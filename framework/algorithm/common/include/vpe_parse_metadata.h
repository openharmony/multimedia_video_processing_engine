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

#ifndef VPE_FRAMEWORK_ALGORITHM_COMMON_PARSE_METADATA_H
#define VPE_FRAMEWORK_ALGORITHM_COMMON_PARSE_METADATA_H

#include <vector>
#include "hdr_vivid_metadata_v1.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
constexpr int METADATA_MAX_LENGTH = 512;
constexpr int AIHDR_METADATA_MAX_LENGTH = 128;
// 以下变量根据CUVA标准7.4确定了每个元数据所占用的bit数
using CuvaMetadataBitNum = enum {
    SYSTEM_START_CODE_BIT = 8,
    MINIMUM_MAXRGB_BIT = 12,
    AVERAGE_MAXRGB_BIT = 12,
    VARIANCE_MAXRGB_BIT = 12,
    MAXIMUM_MAXRGB_BIT = 12,
    TONE_MAPPING_MODE_BIT = 1,
    TONE_MAPPING_PARAM_NUM_BIT = 1,
    TARGETED_SYSTEM_DISPLAY_BIT = 12,
    BASE_FLAG_BIT = 1,
    BASE_PARAM_MP_BIT = 14,
    BASE_PARAM_MM_BIT = 6,
    BASE_PARAM_MA_BIT = 10,
    BASE_PARAM_MB_BIT = 10,
    BASE_PARAM_MN_BIT = 6,
    BASE_PARAM_K1_BIT = 2,
    BASE_PARAM_K2_BIT = 2,
    BASE_PARAM_K3_BIT = 4,
    BASE_PARAM_DELTA_MODE_BIT = 3,
    BASE_PARAM_DELTA_BIT = 7,
    P3SPLINE_FLAG_BIT = 1,
    P3SPLINE_NUM_BIT = 1,
    P3SPLINE_TH_MODE_BIT = 2,
    P3SPLINE_TH_MB_BIT = 8,
    P3SPLINE_TH_OFFSET_BIT = 2,
    P3SPLINE_TH1_BIT = 12,
    P3SPLINE_TH2_BIT = 10,
    P3SPLINE_TH3_BIT = 10,
    P3SPLINE_STRENGTH_BIT = 8,
    COLOR_SATURATION_BIT = 1,
    COLOR_SATURATION_NUM_BIT = 3,
    COLOR_SATURATION_GAIN_BIT = 8,
    CUVA_TWELVE_BIT_NUM = 4095,
    CUVA_LOW_AREA_CURVE = 0,
    CUVA_SPLINE_AREA_CURVE = 1,
    CUVA_HIGH_AREA_CURVE = 2,
    CUVA_TYPE_BIT = 4,
    MARKERBIT_BIT = 1,
    DISPLAY_3BIT = 3,
    DISPLAY_8BIT = 8,
    DISPLAY_MARKERBIT = 1,
    DISPLAY_16BIT = 16
};

int PraseMetadataFromArray(const std::vector<uint8_t> &cuvaInfo, HdrVividMetadataV1 &tmoCuvaMetadata,
    HwDisplayMeta &displayMeta);
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // VPE_FRAMEWORK_ALGORITHM_COMMON_PARSE_METADATA_H
