/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "util_define.h"
#include <fstream>
#include <string>
#include <string_view>
#include <sstream>
#include <iostream>
#include <vector>
#include <memory>
#include <cstdlib>
#include "securec.h"
#include "vpe_log.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
using namespace std;
constexpr int32_t STRIDE = 2;

void ReadFileYuv42016(std::string yuvFilePath, sptr<SurfaceBuffer> &buffer, int width, int height)
{
    unique_ptr<ifstream> yuvFile = make_unique<ifstream>();
    CHECK_AND_RETURN_LOG(yuvFile != nullptr, "Fatal: No memory");
    yuvFile->open(yuvFilePath, ios::in | ios::binary);
    yuvFile->seekg(0, ios::beg);
    long ySize = static_cast<long>(width) * static_cast<long>(height);
    long uvSize = ySize / 4;
    uint16_t *yBuffer = new uint16_t[ySize + STRIDE * uvSize];
    uint16_t *uBuffer = new uint16_t[uvSize];
    uint16_t *vBuffer = new uint16_t[uvSize];
    yuvFile->read(reinterpret_cast<char *>(yBuffer), ySize * sizeof(uint16_t));
    yuvFile->read(reinterpret_cast<char *>(uBuffer), uvSize * sizeof(uint16_t));
    yuvFile->read(reinterpret_cast<char *>(vBuffer), uvSize * sizeof(uint16_t));

    errno_t ret = memcpy_s(reinterpret_cast<char *>(buffer->GetVirAddr()), (ySize + uvSize * STRIDE) * sizeof(uint16_t),
        yBuffer, (ySize + uvSize * STRIDE) * sizeof(uint16_t));
    if (ret != EOK) {
        printf("memcpy_s failed, err = %d\n", ret);
    }
    delete[] yBuffer;
    delete[] uBuffer;
    delete[] vBuffer;
    yuvFile->close();
}

void ReadFileYuv42010ToNv1216(std::string yuvFilePath, sptr<SurfaceBuffer> &buffer, int width, int height)
{
    unique_ptr<ifstream> yuvFile = make_unique<ifstream>();
    CHECK_AND_RETURN_LOG(yuvFile != nullptr, "Fatal: No memory");
    yuvFile->open(yuvFilePath, ios::in | ios::binary);
    yuvFile->seekg(0, ios::beg);
    long ySize = static_cast<long>(width) * static_cast<long>(height);
    long uvSize = ySize / 4;
    uint16_t *yBuffer = new uint16_t[ySize + STRIDE * uvSize];
    uint16_t *uBuffer = new uint16_t[uvSize];
    uint16_t *vBuffer = new uint16_t[uvSize];
    yuvFile->read(reinterpret_cast<char *>(yBuffer), ySize * sizeof(uint16_t));
    yuvFile->read(reinterpret_cast<char *>(uBuffer), uvSize * sizeof(uint16_t));
    yuvFile->read(reinterpret_cast<char *>(vBuffer), uvSize * sizeof(uint16_t));

    int num10To16 = 6;
    int numTwo = 2;
    uint16_t *uvBufferPtr = &yBuffer[ySize];
    for (int i = 0; i < ySize; i++) {
        yBuffer[i] = yBuffer[i] << num10To16;
    }
    for (int j = 0; j < uvSize; j++) {
        uvBufferPtr[numTwo * j] = uBuffer[j] << num10To16;
        uvBufferPtr[numTwo * j + 1] = vBuffer[j] << num10To16;
    }

    errno_t ret = memcpy_s(reinterpret_cast<char *>(buffer->GetVirAddr()), (ySize + uvSize * STRIDE) * sizeof(uint16_t),
        yBuffer, (ySize + uvSize * STRIDE) * sizeof(uint16_t));
    if (ret != EOK) {
        printf("memcpy_s failed, err = %d\n", ret);
    }
    delete[] yBuffer;
    delete[] uBuffer;
    delete[] vBuffer;
    yuvFile->close();
}

void ReadInputFile(std::string yuvFilePath, sptr<SurfaceBuffer> &buffer, int frameSize)
{
    unique_ptr<ifstream> yuvFile = make_unique<ifstream>();
    CHECK_AND_RETURN_LOG(yuvFile != nullptr, "Fatal: No memory");
    yuvFile->open(yuvFilePath, ios::in | ios::binary);
    yuvFile->seekg(0, ios::beg);
    yuvFile->read(reinterpret_cast<char *>(buffer->GetVirAddr()), frameSize);
    yuvFile->close();
}

static void SaveMetaDataToBin(int frameId, const char *fileName, unsigned char *metadataPayload,
    int metadataPayloadSize)
{
    FILE *fileOut = nullptr;
    if (frameId == 0) {
        fileOut = fopen(fileName, "wb");
    } else {
        fileOut = fopen(fileName, "ab+");
    }
    if (fileOut == nullptr) {
        printf("open file[%s] Error:%s!", fileName, "error");
        return;
    }

    uint32_t mdLen = static_cast<uint32_t>(metadataPayloadSize);
    int len = fwrite(&mdLen, sizeof(uint32_t), 1, fileOut);
    if (len != 1) {
        printf("write file Error:%s with mdLen!", fileName);
    }
    len = fwrite(metadataPayload, sizeof(uint8_t), metadataPayloadSize, fileOut);
    if (len != metadataPayloadSize) {
        printf("write file Error:%s!", fileName);
    }
    int fcloseResult = fclose(fileOut);
    if (fcloseResult != 0) {
        printf("fclose Error:%s!", fileName);
    }
}

void SaveMetadataFromSurBuffer(const sptr<SurfaceBuffer> &input, int frame, const string &metadataBin)
{
    std::vector<uint8_t> inMetaData;
    input->GetMetadata(ATTRKEY_HDR_DYNAMIC_METADATA, inMetaData);
    unsigned char *metaData = inMetaData.data();
    uint32_t meteDataLength = inMetaData.size();
    printf("frame=%d, imeteDataLength_demo = %u\n", frame, meteDataLength);
    SaveMetaDataToBin(frame, metadataBin.c_str(), metaData, meteDataLength);
}

void WriteOutFile(int frame, const string &outYuvFileName, const sptr<SurfaceBuffer> &output, int frameSize)
{
    std::unique_ptr<std::ofstream> outputYuv;
    if (frame == 0) {
        outputYuv =
            std::make_unique<std::ofstream>(outYuvFileName.c_str(), std::ios::binary | std::ios::out | std::ios::trunc);
    } else {
        outputYuv = std::make_unique<std::ofstream>(outYuvFileName.c_str(), std::ios::binary | std::ios::app);
    }
    outputYuv->write(static_cast<const char *>(output->GetVirAddr()), frameSize);
}


std::string GetPixFmtString(GraphicPixelFormat pixfmt)
{
    switch (pixfmt) {
        case GRAPHIC_PIXEL_FMT_YCBCR_P010:
            return "nv12_10";
        case GRAPHIC_PIXEL_FMT_YCRCB_P010:
            return "nv21_10";
        case GRAPHIC_PIXEL_FMT_RGBA_1010102:
            return "rgba1010102";
        default:
            return "none";
    }
}

std::string GetColspcStringPrimaries(CM_ColorSpaceInfo colorSpaceInfo)
{
    std::string str = "";
    switch (colorSpaceInfo.primaries) {
        case COLORPRIMARIES_BT709:
            str = "_709";
            break;
        case COLORPRIMARIES_BT601_P:
            str = "_601p";
            break;
        case COLORPRIMARIES_BT601_N:
            str = "_601n";
            break;
        case COLORPRIMARIES_BT2020:
            str = "_2020";
            break;
        default:
            str = "_none";
            break;
    }
    return str;
}
std::string GetColspcStringTrans(CM_ColorSpaceInfo colorSpaceInfo)
{
    std::string str = "";
    switch (colorSpaceInfo.transfunc) {
        case TRANSFUNC_BT709:
            str = "_709";
            break;
        case TRANSFUNC_SRGB:
            str = "_srgb";
            break;
        case TRANSFUNC_LINEAR:
            str = "_linear";
            break;
        case TRANSFUNC_PQ:
            str = "_pq";
            break;
        case TRANSFUNC_HLG:
            str = "_hlg";
            break;
        case TRANSFUNC_ADOBERGB:
            str = "_adobergb";
            break;
        case TRANSFUNC_GAMMA2_2:
            str = "_gamma22";
            break;
        case TRANSFUNC_GAMMA2_4:
            str = "_gamma24";
            break;
        default:
            str = "_none";
            break;
    }
    return str;
}
std::string GetColspcStringRange(CM_ColorSpaceInfo colorSpaceInfo)
{
    std::string str = "";
    switch (colorSpaceInfo.range) {
        case RANGE_FULL:
            str = "_full";
            break;
        case RANGE_LIMITED:
            str = "_limited";
            break;
        default:
            str = "_none";
            break;
    }
    return str;
}
std::string GetColspcString(CM_ColorSpaceInfo colorSpaceInfo)
{
    std::string str = "";
    std::string strPrima = GetColspcStringPrimaries(colorSpaceInfo);
    str = strPrima;
    std::string strTrans = GetColspcStringTrans(colorSpaceInfo);
    str += strTrans;
    std::string strRange = GetColspcStringRange(colorSpaceInfo);
    str += strRange;
    return str;
}

std::string GetMetadataString(CM_HDR_Metadata_Type metaType)
{
    switch (metaType) {
        case CM_VIDEO_HLG:
            return "_vHlg";
        case CM_VIDEO_HDR10:
            return "_vHdr10";
        case CM_VIDEO_HDR_VIVID:
            return "_vHdrVivid";
        case CM_IMAGE_HDR_VIVID_DUAL:
            return "_iHdrVividD";
        case CM_IMAGE_HDR_VIVID_SINGLE:
            return "_iHdrVividS";
        case CM_IMAGE_HDR_ISO_DUAL:
            return "_iHdrIsoD";
        case CM_IMAGE_HDR_ISO_SINGLE:
            return "_iHdrIsoS";
        default:
            return "none";
    }
}

std::string GetOutFileName(const string &baseName, const ParameterBase &param)
{
    string outputName = "";
    outputName = baseName + "_iFmt" + GetPixFmtString(param.inPixFmt) + "_iCol" + GetColspcString(param.inColspcInfo) +
        "_iMetaT" + GetMetadataString(param.inMetaType) + "_oFmt" + GetPixFmtString(param.outPixFmt) + "_oCol" +
        GetColspcString(param.outColspcInfo) + "_oMeta" + GetMetadataString(param.outMetaType) + ".yuv";
    return outputName;
}

void SetMeatadata(sptr<SurfaceBuffer> &buffer, uint32_t value)
{
    std::vector<uint8_t> metadata;
    metadata.resize(sizeof(value));
    errno_t ret = memcpy_s(metadata.data(), metadata.size(), &value, sizeof(value));
    if (ret != EOK) {
        printf("memcpy_s failed, err = %d\n", ret);
        return;
    }
    uint32_t err = buffer->SetMetadata(ATTRKEY_HDR_METADATA_TYPE, metadata);
    printf("Buffer set metadata type, ret: %u\n", err);
}
void SetMeatadata(sptr<SurfaceBuffer> &buffer, const CM_ColorSpaceInfo &colorspaceInfo)
{
    std::vector<uint8_t> metadata;
    metadata.resize(sizeof(CM_ColorSpaceInfo));
    errno_t ret = memcpy_s(metadata.data(), metadata.size(), &colorspaceInfo, sizeof(CM_ColorSpaceInfo));
    if (ret != EOK) {
        printf("memcpy_s failed, err = %d\n", ret);
        return;
    }
    uint32_t err = buffer->SetMetadata(ATTRKEY_COLORSPACE_INFO, metadata);
    printf("Buffer set colorspace info, ret: %u\n", err);
}
void SetMeatadata(sptr<SurfaceBuffer> &buffer, int key, const float &data)
{
    std::vector<uint8_t> metadata;
    metadata.resize(sizeof(float));
    errno_t ret = memcpy_s(metadata.data(), metadata.size(), &data, sizeof(float));
    if (ret != EOK) {
        printf("memcpy_s failed, err = %d\n", ret);
        return;
    }
    uint32_t err = buffer->SetMetadata(key, metadata);
    printf("Buffer set colorspace info, ret: %u\n", err);
}
void SetMeatadata(sptr<SurfaceBuffer> &buffer, int key, const int &data)
{
    std::vector<uint8_t> metadata;
    metadata.resize(sizeof(int));
    errno_t ret = memcpy_s(metadata.data(), metadata.size(), &data, sizeof(int));
    if (ret != EOK) {
        printf("memcpy_s failed, err = %d\n", ret);
        return;
    }
    uint32_t err = buffer->SetMetadata(key, metadata);
    printf("Buffer set colorspace info, ret: %u\n", err);
}
void SetMeatadata(sptr<SurfaceBuffer> &buffer, std::unique_ptr<std::ifstream> &metadataFile)
{
    if (!metadataFile->is_open()) {
        printf("Metadata file is not open\n");
        return;
    }

    std::vector<uint8_t> metadata;
    uint32_t metadataSize = 0;

    metadataFile->read(reinterpret_cast<char *>(&metadataSize), sizeof(uint32_t));
    if (metadataSize == 0) {
        printf("Read metadata failed, get a size: %u\n", metadataSize);
        return;
    }
    metadata.resize(metadataSize);
    metadataFile->read(reinterpret_cast<char *>(metadata.data()), metadataSize);

    // Dump metadata
    for (size_t idx = 0; idx < metadata.size(); idx++) {
        uint32_t data =
            (metadata[idx] << 24) + (metadata[idx + 1] << 16) + (metadata[idx + 2] << 8) + (metadata[idx + 3]);
        (void)data;
    }

    int32_t err = buffer->SetMetadata(ATTRKEY_HDR_DYNAMIC_METADATA, metadata);
    printf("Buffer hdr dynamic metadata type, ret: %d\n", err);
}
sptr<SurfaceBuffer> CreateSurfaceBuffer(uint32_t pixelFormat, int32_t width, int32_t height)
{
    auto buffer = SurfaceBuffer::Create();
    if (nullptr == buffer) {
        printf("Create surface buffer failed\n");
        return nullptr;
    }
    BufferRequestConfig inputCfg;
    inputCfg.width = width;
    inputCfg.height = height;
    inputCfg.strideAlignment = width;
    inputCfg.usage = BUFFER_USAGE_CPU_READ | BUFFER_USAGE_CPU_WRITE
        | BUFFER_USAGE_HW_RENDER | BUFFER_USAGE_HW_TEXTURE | BUFFER_USAGE_MEM_MMZ_CACHE;
    inputCfg.format = pixelFormat;
    inputCfg.timeout = 0;
    GSError err = buffer->Alloc(inputCfg);
    if (GSERROR_OK != err) {
        printf("Alloc surface buffer failed\n");
        return nullptr;
    }
    return buffer;
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
