/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "vpe_parse_metadata.h"
#include <cstdint>
#include <vector>
#include "vpe_log.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

// 做了字节颠倒ˇ把4个字节逆序排列
static inline uint32_t ConvertEndian(uint32_t x)
{
    // 8 & 24: bit offset; 0x0000ff00 & 0x00ff0000 & 0x000000ff: bit mask
    return ((x << 24) | ((x & 0x0000ff00) << 8) | ((x & 0x00ff0000) >> 8) | ((x >> 24) & 0x000000ff));
}

struct BsInfo {
    uint8_t *head;
    uint8_t *tail;
    uint32_t bufA;
    uint32_t bufB;
    int32_t bsLen;
    int32_t bufPos;
    uint32_t totalPos;
};

static void InitBs(BsInfo *bs, const uint8_t *input, int32_t length)
{
    uint32_t data;
    uint64_t alignWord = 0x3; // 使首地址4字节对齐

    bs->head = reinterpret_cast<uint8_t *>(reinterpret_cast<uintptr_t>(input) & ~alignWord);
    bs->tail = bs->head + 8; // 8 :将尾地址初始化为首地址8字节之后
    bs->bsLen = length;

    data = *reinterpret_cast<uint32_t *>(bs->head);
    bs->bufA = ConvertEndian(data);
    // 4 :读取后4个字节的值传给bufBˇ当要取的元数据跨越两段4字节时会用到
    data = *reinterpret_cast<uint32_t *>(bs->head + 4);
    bs->bufB = ConvertEndian(data);
    // 3 is bit ofsˇ记录对齐地址和实际地址的偏差量
    bs->bufPos = (reinterpret_cast<uintptr_t>(input) & 0x3) << 3;
    bs->totalPos = 0;

    return;
}

static int32_t ShowBs(const BsInfo *bs, int32_t currentBitNum)
{
    uint32_t pos = static_cast<uint32_t>(bs->bufPos);
    // 0x20:4字节ˇ如果对齐后和实际地址的偏差量大于4字节ˇ或要读取的位数为0
    // 或要取的位数大于4字节ˇ直接返回0
    if (pos >= 0x20 || currentBitNum == 0 || currentBitNum > 0x20) {
        return 0;
    }
    // 要读取的位数超出剩余码流的位数时，直接返回0
    // 8 :一个字节占8位ˇ当前读取位小于传入数组总位数才合理
    if (static_cast<uint32_t>(bs->totalPos + currentBitNum) > static_cast<uint32_t>(bs->bsLen * 8)) {
        return 0;
    }
    // 0x20ˇ32ˇ如果要取的元数据要跨越两段4个字节ˇ则要取两次值进行拼接。
    uint32_t data = 0;
    int32_t abPos = currentBitNum + bs->bufPos;
    if (abPos > 0x20) {
        data = bs->bufA << pos;
        // 0x20-pos为高四节数值和低四节数值的位偏移量ˇ移位后data1和data对齐
        uint32_t data1 = (pos == 0) ? 0 : (bs->bufB >> (0x20 - pos));
        data |= data1;
        data >>= static_cast<uint32_t>(0x20 - currentBitNum); // 0x20:4字节
    } else { // 如果要取的元数据没有跨越两个4字节ˇ则直接读取bufA移位后的值。0x20:4字节
        data = (bs->bufA << pos) >> static_cast<uint32_t>(0x20 - currentBitNum);
    }
    return static_cast<int32_t>(data);
}

static int32_t SkipBs(BsInfo *bs, int32_t currentBitNum)
{
    bs->totalPos += static_cast<uint32_t>(currentBitNum);
    // 8 :一个字节占8位ˇ当前读取位小于传入数组总位数才合理
    if (bs->totalPos < static_cast<uint32_t>(bs->bsLen * 8)) {
        int32_t abPos = currentBitNum + bs->bufPos;
        if (abPos >= 32) {           // 32 :如果ˇ地址偏移量+读取位数ˇ大于4字节
            bs->bufPos = abPos - 32; // 32 :则更新偏移量为(地址偏移量+读取位数)%32
            bs->bufA = bs->bufB;     // 前四字节的buf更新为后4字节的buf
            uint32_t data = *reinterpret_cast<uint32_t *>(bs->tail);
            bs->bufB = ConvertEndian(data);
            bs->tail += 4; // 4 :整体ˇ后移4字节
        } else {
            bs->bufPos += currentBitNum;
        }
    }
    return currentBitNum;
}

static __inline int32_t GetBs(BsInfo *bs, int32_t currentBitNum)
{
    uint32_t data = static_cast<uint32_t>(ShowBs(bs, currentBitNum));
    SkipBs(bs, currentBitNum);
    return static_cast<int32_t>(data);
}

static uint32_t HevcUv(BsInfo *bsPtr, int32_t bitNum)
{
    uint32_t code = static_cast<uint32_t>(GetBs(bsPtr, bitNum));
    return code;
}

void SetThreeSpline(BsInfo *bsPtr, HdrVividMetadataV1 &tmoCuvaMetadata, int index)
{
    for (unsigned int mode_i = 0; mode_i < tmoCuvaMetadata.threeSplineNum[index]; mode_i++) {
        tmoCuvaMetadata.threeSplineThMode[index][mode_i] = HevcUv(bsPtr, P3SPLINE_TH_MODE_BIT);
        // 对应标准7.3
        if ((tmoCuvaMetadata.threeSplineThMode[index][mode_i] == 0) ||
            (tmoCuvaMetadata.threeSplineThMode[index][mode_i] == 2)) { // 2:
            tmoCuvaMetadata.threeSplineThMb[index][mode_i] = HevcUv(bsPtr, P3SPLINE_TH_MB_BIT);
        }
        tmoCuvaMetadata.threeSplineTh[index][mode_i][0] = HevcUv(bsPtr, P3SPLINE_TH1_BIT);
        tmoCuvaMetadata.threeSplineTh[index][mode_i][1] = HevcUv(bsPtr, P3SPLINE_TH2_BIT);
        tmoCuvaMetadata.threeSplineTh[index][mode_i][2] = HevcUv(bsPtr, P3SPLINE_TH3_BIT); // 2:
        tmoCuvaMetadata.threeSplineStrength[index][mode_i] = HevcUv(bsPtr, P3SPLINE_STRENGTH_BIT);
    }
}

int SetToneMappingMode(BsInfo *bsPtr, HdrVividMetadataV1 &tmoCuvaMetadata)
{
    if (!tmoCuvaMetadata.toneMappingMode) {
        return 0;
    }

    tmoCuvaMetadata.toneMappingParamNum = HevcUv(bsPtr, TONE_MAPPING_PARAM_NUM_BIT);
    // test
    tmoCuvaMetadata.toneMappingParamNum++;
    // For Memory Safety
    // tone_mapping_param_num占用1bitˇ最大为1ˇ加1后最大为2
    if (tmoCuvaMetadata.toneMappingParamNum > 2) {
        VPE_LOGE("ToneMappingParamNum Out Of Range, value: %i", tmoCuvaMetadata.toneMappingParamNum);
        return 1;
    }
    for (unsigned int j = 0; j < tmoCuvaMetadata.toneMappingParamNum; j++) {
        tmoCuvaMetadata.targetedSystemDisplayMaximumLuminance[j] = HevcUv(bsPtr, TARGETED_SYSTEM_DISPLAY_BIT);
        tmoCuvaMetadata.baseFlag[j] = HevcUv(bsPtr, BASE_FLAG_BIT);
        if (tmoCuvaMetadata.baseFlag[j]) {
            tmoCuvaMetadata.baseParamMp[j] = HevcUv(bsPtr, BASE_PARAM_MP_BIT);
            tmoCuvaMetadata.baseParamMm[j] = HevcUv(bsPtr, BASE_PARAM_MM_BIT);
            tmoCuvaMetadata.baseParamMa[j] = HevcUv(bsPtr, BASE_PARAM_MA_BIT);
            tmoCuvaMetadata.baseParamMb[j] = HevcUv(bsPtr, BASE_PARAM_MB_BIT);
            tmoCuvaMetadata.baseParamMn[j] = HevcUv(bsPtr, BASE_PARAM_MN_BIT);
            tmoCuvaMetadata.baseParamK1[j] = HevcUv(bsPtr, BASE_PARAM_K1_BIT);
            tmoCuvaMetadata.baseParamK2[j] = HevcUv(bsPtr, BASE_PARAM_K2_BIT);
            tmoCuvaMetadata.baseParamK3[j] = HevcUv(bsPtr, BASE_PARAM_K3_BIT);
            tmoCuvaMetadata.baseParamDeltaMode[j] = HevcUv(bsPtr, BASE_PARAM_DELTA_MODE_BIT);
            tmoCuvaMetadata.baseParamDelta[j] = HevcUv(bsPtr, BASE_PARAM_DELTA_BIT);
        }
        tmoCuvaMetadata.threeSplineFlag[j] = HevcUv(bsPtr, P3SPLINE_FLAG_BIT);
        if (tmoCuvaMetadata.threeSplineFlag[j]) {
            tmoCuvaMetadata.threeSplineNum[j] = HevcUv(bsPtr, P3SPLINE_NUM_BIT);
            tmoCuvaMetadata.threeSplineNum[j]++;
            // For Memory Safety
            // p3SplineNum占用1bitˇ最大为1ˇ加1后最大为2
            if (tmoCuvaMetadata.threeSplineNum[j] > 2) {
                VPE_LOGE("ThreeSplineNum Out Of Range, value: %i", tmoCuvaMetadata.threeSplineNum[j]);
                return 1;
            }
            SetThreeSpline(bsPtr, tmoCuvaMetadata, j);
        }
    }
    return 0;
}

int SetcolorSaturationMapping(BsInfo *bsPtr, HdrVividMetadataV1 &tmoCuvaMetadata)
{
    if (tmoCuvaMetadata.colorSaturationMappingFlag) {
        tmoCuvaMetadata.colorSaturationNum = HevcUv(bsPtr, COLOR_SATURATION_NUM_BIT);
        // For Memory Safety
        if (tmoCuvaMetadata.colorSaturationNum > 8) { // 8:
            VPE_LOGE("ColorSaturationNum Out Of Range, value: %i", tmoCuvaMetadata.colorSaturationNum);
            return 1;
        }
        for (unsigned int mode_i = 0; mode_i < tmoCuvaMetadata.colorSaturationNum; mode_i++) {
            tmoCuvaMetadata.colorSaturationGain[mode_i] = HevcUv(bsPtr, COLOR_SATURATION_GAIN_BIT);
        }
    }
    return 0;
}
void PraseDisplayMetadataFromArray(BsInfo *bsPtr, HwDisplayMeta &displayMeta)
{
    // HwDisplayMeta displayMeta;
    displayMeta.type = HevcUv(bsPtr, DISPLAY_3BIT);
    VPE_LOGD("displayMeta type = %{public}d", displayMeta.type);
    if (displayMeta.type > 0) {
        displayMeta.size = HevcUv(bsPtr, DISPLAY_16BIT);
        auto markerbit = HevcUv(bsPtr, DISPLAY_MARKERBIT);
        displayMeta.startX = HevcUv(bsPtr, DISPLAY_16BIT);
        markerbit = HevcUv(bsPtr, DISPLAY_MARKERBIT);
        displayMeta.startY = HevcUv(bsPtr, DISPLAY_16BIT);
        markerbit = HevcUv(bsPtr, DISPLAY_MARKERBIT);
        displayMeta.endX = HevcUv(bsPtr, DISPLAY_16BIT);
        markerbit = HevcUv(bsPtr, DISPLAY_MARKERBIT);
        displayMeta.endY = HevcUv(bsPtr, DISPLAY_16BIT);
        markerbit = HevcUv(bsPtr, DISPLAY_MARKERBIT);
        for (int i = 0; i < 17; i++) { // 17
            displayMeta.lutMap[i] = HevcUv(bsPtr, DISPLAY_8BIT);
            if (i % 2 == 1) { // 2
                markerbit = HevcUv(bsPtr, DISPLAY_MARKERBIT);
            }
        }
        displayMeta.averageLuma = HevcUv(bsPtr, DISPLAY_16BIT);
        markerbit = HevcUv(bsPtr, DISPLAY_MARKERBIT);
        displayMeta.scene = HevcUv(bsPtr, DISPLAY_8BIT);
        markerbit = HevcUv(bsPtr, DISPLAY_MARKERBIT);
        displayMeta.expo = HevcUv(bsPtr, DISPLAY_16BIT);
        markerbit = HevcUv(bsPtr, DISPLAY_MARKERBIT);
        displayMeta.iso = HevcUv(bsPtr, DISPLAY_16BIT);
        markerbit = HevcUv(bsPtr, DISPLAY_MARKERBIT);
        displayMeta.flash = HevcUv(bsPtr, DISPLAY_8BIT);
        for (int i = 0; i < 32; i++) { // 32
            displayMeta.hdrHistgram[i] = HevcUv(bsPtr, DISPLAY_8BIT);
            if (i % 2 == 0) { // 2
                markerbit = HevcUv(bsPtr, DISPLAY_MARKERBIT);
            }
        }
        displayMeta.noiseLevel = HevcUv(bsPtr, DISPLAY_8BIT);
        markerbit = HevcUv(bsPtr, DISPLAY_MARKERBIT);
        displayMeta.apetureValue = HevcUv(bsPtr, DISPLAY_8BIT);
        displayMeta.aelv = HevcUv(bsPtr, DISPLAY_8BIT);
        displayMeta.maxHeadroom = 5115; // 5115 = 5 * 1023;
        VPE_LOGD("CLink displayMeta.markerbit = %{public}d\n", markerbit);
    }
    if (displayMeta.type > 1) {
        auto markerbit = HevcUv(bsPtr, DISPLAY_MARKERBIT);
        displayMeta.maxHeadroom = HevcUv(bsPtr, DISPLAY_16BIT);
        VPE_LOGD("CLink displayMeta.markerbit = %{public}d\n", markerbit);
        VPE_LOGD("CLink displayMeta.maxHeadroom = %{public}d\n", displayMeta.maxHeadroom);
    }
    return;
}
uint32_t PraseMatadataType(BsInfo *bsPtr)
{
    HevcUv(bsPtr, MARKERBIT_BIT);
    uint32_t type = HevcUv(bsPtr, CUVA_TYPE_BIT);
    VPE_LOGD("HDR: extension metadata type = %{public}d", type);
    return type;
}

int PraseMetadataFromArray(const std::vector<uint8_t> &cuvaInfo, HdrVividMetadataV1 &tmoCuvaMetadata,
    HwDisplayMeta &displayMeta)
{
    BsInfo bs {};
    BsInfo *bsPtr = &bs;
    int cuvaInfoSize = static_cast<int>(cuvaInfo.size());
    if (cuvaInfoSize == AIHDR_METADATA_MAX_LENGTH) {
        tmoCuvaMetadata.gtmLut = cuvaInfo;
    }
    VPE_LOGD("cuvaInfoSize = %{public}d\n", cuvaInfoSize);
    std::vector<uint8_t> cuvaInfoCopy;
    if (cuvaInfoSize > METADATA_MAX_LENGTH) { // 最大metadata长度512
        cuvaInfoSize = METADATA_MAX_LENGTH;
    }
    cuvaInfoCopy.resize(METADATA_MAX_LENGTH); // 最大metadata长度512
    errno_t res = memcpy_s(cuvaInfoCopy.data(), cuvaInfoCopy.size(), cuvaInfo.data(), cuvaInfo.size());
    CHECK_AND_RETURN_RET_LOG(res == EOK, res, "memcpy_s failed");
    InitBs(bsPtr, &cuvaInfoCopy[0], cuvaInfoSize);
    tmoCuvaMetadata.systemStartCode = HevcUv(bsPtr, SYSTEM_START_CODE_BIT);
    int numWindows = 0;
    if (tmoCuvaMetadata.systemStartCode == 1) {
        numWindows = 1;
    }

    for (int i = 0; i < numWindows; i++) {
        tmoCuvaMetadata.minimumMaxRgbPq = HevcUv(bsPtr, MINIMUM_MAXRGB_BIT);
        tmoCuvaMetadata.averageMaxRgbPq = HevcUv(bsPtr, AVERAGE_MAXRGB_BIT);
        tmoCuvaMetadata.varianceMaxRgbPq = HevcUv(bsPtr, VARIANCE_MAXRGB_BIT);
        tmoCuvaMetadata.maximumMaxRgbPq = HevcUv(bsPtr, MAXIMUM_MAXRGB_BIT);
        tmoCuvaMetadata.toneMappingMode = HevcUv(bsPtr, TONE_MAPPING_MODE_BIT);
        int ret = SetToneMappingMode(bsPtr, tmoCuvaMetadata);
        if (ret != 0) { return ret; }
        tmoCuvaMetadata.colorSaturationMappingFlag = HevcUv(bsPtr, COLOR_SATURATION_BIT);
        ret = SetcolorSaturationMapping(bsPtr, tmoCuvaMetadata);
        if (ret != 0) { return ret; }
    }
    // 8 :一个字节占8位ˇ当前读取位小于传入数组总位数才合理
    while (bsPtr->totalPos + 8 < static_cast<uint32_t>(bsPtr->bsLen * 8)) { // 存在后续元数据内容
        uint32_t metadataType = PraseMatadataType(bsPtr);
        if ((metadataType & 0x3) == 1) {
            // ToDo: Prase LocalToneMapping Metadata
            VPE_LOGD("LTM Metadata Prase!");
        } else if ((metadataType & 0x3) == 3) { // 3 拍显元数据标志位
            VPE_LOGD("CLink Metadata Prase!");
            PraseDisplayMetadataFromArray(bsPtr, displayMeta);
        } else {
            VPE_LOGD("Unknown Extension Metadata type!");
            break;
        }
    }
    return 0;
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
