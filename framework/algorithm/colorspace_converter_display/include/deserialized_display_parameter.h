/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef VPE_FRAMEWORK_ALGORITHM_COLORSPACE_CONVERTER_DISPLAY_DESERIALIZED_DISPLAY_PARAMETER_H
#define VPE_FRAMEWORK_ALGORITHM_COLORSPACE_CONVERTER_DISPLAY_DESERIALIZED_DISPLAY_PARAMETER_H

#include <optional>
#include "v1_0/hdr_static_metadata.h"
#include "algorithm_common.h"
#include "hdr_vivid_metadata_v1.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

using namespace HDI::Display::Graphic::Common::V1_0;

struct DeserializedDisplayParameter {
    ColorSpaceDescription               inputColorSpace;         // 色彩空间信息
    ColorSpaceDescription               outputColorSpace;        // 色彩空间信息
    std::optional<HdrStaticMetadata>    staticMetadata;          // 静态元数据
    std::optional<HdrVividMetadataV1>   hdrVividMetadata;        // 动态元数据
    float                               sdrNits;                 // SDR亮度
    float                               tmoNits;                 // TMO目标亮度
    float                               currentDisplayNits;      // 屏幕当前亮度，和tmoNits相除得到sdr亮度打折比
    bool                                disableHdrFloatHeadRoom; // 不使用HDRfp16方案做额外提亮
    HwDisplayMeta                       displayMeta;             // 拍显协同元数据
    int32_t                             width;                   // 宽度
    int32_t                             height;                  // 高度
    std::vector<float>                  linearMatrix;            // 线性域转换矩阵，作用在原色域上
};

} // namespace VideoProcessingEngine
} // Media
} // OHOS

#endif // VPE_FRAMEWORK_ALGORITHM_COLORSPACE_CONVERTER_DISPLAY_DESERIALIZED_DISPLAY_PARAMETER_H
