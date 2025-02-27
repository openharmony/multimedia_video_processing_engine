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

#ifndef VPE_FRAMEWORK_ALGORITHM_COMMON_HDR_VIVID_METADATA_V1_H
#define VPE_FRAMEWORK_ALGORITHM_COMMON_HDR_VIVID_METADATA_V1_H

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
struct HdrVividMetadataV1 {
    unsigned int systemStartCode; // 表示系统版本号
    // minimum_maxrgb_pq,表示显示内容的最小亮度，pq域，范围从0~4095
    unsigned int minimumMaxRgbPq;
    unsigned int averageMaxRgbPq; // average_maxrgb_pq,表示显示内容的平均亮度，pq域，范围从0.0~4095
    // variance_maxrgb_pq,表示显示内容的变化范围，pq域，范围从0.0~4095
    unsigned int varianceMaxRgbPq;
    unsigned int maximumMaxRgbPq; // maximum_maxrgb_pq,表示显示内容的最大亮度，pq域，范围从0.0~4095
    // tone_mapping_enable_mode_flag，取值为0或1，若为0，则不传曲线参数，否则传输参数
    unsigned int toneMappingMode;
    // tone_mapping_para_enable_num，表示当前色调映射参数组的数目减1，
    // 为0时色调映射参数组的数目为1，为1时色调映射参数组的数目为2
    unsigned int toneMappingParamNum;
    // targeted_system_display_maximum_luminancePq,参考目标显示器最高亮度,范围从0.0~4095,
    // 数组长度为2说明有2组参数，tone_mapping_param_num
    unsigned int targetedSystemDisplayMaximumLuminance[2];
    // base_enable_flag,基础曲线标识，取值为0或1，为0则不传输基础曲线参数，为1则传输参数
    unsigned int baseFlag[4];
    unsigned int baseParamMp[2]; // 范围0~16383
    unsigned int baseParamMm[2]; // 范围0~63
    unsigned int baseParamMa[2]; // 范围0~1023
    unsigned int baseParamMb[2]; // 范围0~1023
    unsigned int baseParamMn[2]; // 范围0~63
    unsigned int baseParamK1[2]; // 分小于等于1 和 大于1两种情况
    unsigned int baseParamK2[2]; // 分小于等于1 和 大于1两种情况
    unsigned int baseParamK3[2]; // 分小于等于1 和 1~2 和 大于2两种情况
    // base_param_delta_enable_mode,标识当前基础曲线映射参数的调整系数模式
    unsigned int baseParamDeltaMode[2];
    // base_param_enable_delta,标识当前基础曲线映射参数的调整系数值，范围0~127
    unsigned int baseParamDelta[2];
    // 3Spline_enable_flag,二值变量，为1时标识传输三次样条参数，为0时不传
    unsigned int threeSplineFlag[2];
    unsigned int threeSplineNum[2]; // 3Spline_enable_num,标识三次样条区间数量，取值为0和1
    // 3Spline_TH_enable_mode,标识色调映射的三次样条模式，范围为0~3，
    // 这里数组长度4是由于P3Spline_num
    unsigned int threeSplineThMode[2][4];
    // 3Spline_TH_enable_MB,指示色调映射的三次样条区间参数的斜率和暗区偏移量
    unsigned int threeSplineThMb[2][4];
    // 3Spline_TH_enable, 指示色调映射的三次样条区间参数，范围0~4095，第三维度表示三次样
    // 条区间参数（0~4095）、三次样条区间1偏移量（0~1023）和三次样条区间2偏移量（0~1023）
    unsigned int threeSplineTh[2][4][3];
    // 3Spline_enable_Strength,指示色调映射的三次样条区间的修正幅度参数，范围0~255
    unsigned int threeSplineStrength[2][4];
    unsigned int colorSaturationMappingFlag; // 对应标准中color_saturation_mapping_enable_flag
    unsigned int colorSaturationNum;         // 对应标准中color_saturation_enable_num
    unsigned int colorSaturationGain[16];    // 对应标准中color_saturation_enable_gain
    std::vector<uint8_t> gtmLut;  // 存储aihdr生成的lut
};
// C-Link paramter
typedef struct HwDisplayMeta {
    unsigned int type; // 0x03
    unsigned int size; // meta大小
    unsigned int startX;
    unsigned int startY;
    unsigned int endX;
    unsigned int endY;
    unsigned int lutMap[17];
    unsigned int averageLuma;
    unsigned int scene; // 0 风景  1 人像
    unsigned int expo;
    unsigned int iso;
    unsigned int flash;
    unsigned int hdrHistgram[32];
    unsigned int noiseLevel;
    unsigned int apetureValue;
    unsigned int aelv;
    unsigned int maxHeadroom;
} HwDisplayMeta;
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // VPE_FRAMEWORK_ALGORITHM_COMMON_HDR_VIVID_METADATA_V1_H
