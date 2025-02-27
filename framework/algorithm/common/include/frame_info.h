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

#ifndef FRAMEWORK_ALGORITHM_COMMON_FRAME_INFO_H
#define FRAMEWORK_ALGORITHM_COMMON_FRAME_INFO_H

#include <stdint.h>
#include "algorithm_common.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
enum class BitDepth {
    BIT_DEPTH_8,   // 8bit位深
    BIT_DEPTH_10,  // 10bit位深
    BIT_DEPTH_FP16 // 16bit位深,fp16格式
};

struct FrameInfo {
    uint32_t width = 0;                 // 输入输出最大图像宽
    uint32_t height = 0;                // 输入输出最大图像高
    uint32_t widthStride = 0;           // input and output img width stride(padding at the end of the lines)
    uint32_t heightStride = 0;          // input and output img height stride
    BitDepth bitDepth = BitDepth::BIT_DEPTH_8;              // 输入图像位宽
    GraphicPixelFormat pixelFormat = GRAPHIC_PIXEL_FMT_RGBA_8888; // 像素格式
    ColorSpaceDescription colorSpace = {GetColorSpaceInfo(CM_SRGB_FULL), CM_METADATA_NONE};

    FrameInfo() = default;
    explicit FrameInfo(const sptr<SurfaceBuffer> &buffer);
};
struct ColorSpaceInfo {
    GraphicPixelFormat pixelFormat = GRAPHIC_PIXEL_FMT_RGBA_8888; // 像素格式
    CM_ColorSpaceType colorSpace = CM_SRGB_FULL;                        // 颜色空间
    CM_HDR_Metadata_Type metadataType = CM_METADATA_NONE;                    // 颜色空间标志
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // FRAMEWORK_ALGORITHM_COMMON_FRAME_INFO_H
