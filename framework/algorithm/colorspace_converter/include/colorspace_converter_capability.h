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

#ifndef FRAMEWORK_ALGORITHM_COLORSPACE_CONVERTER_COLORSPACE_CONVERTER_CAPABILITY_H
#define FRAMEWORK_ALGORITHM_COLORSPACE_CONVERTER_COLORSPACE_CONVERTER_CAPABILITY_H

#include <functional>
#include <vector>
#include "frame_info.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
struct ColorSpaceConverterCapability {
    ColorSpaceDescription inputColorSpaceDesc;
    ColorSpaceDescription outputColorSpaceDesc;
    /*
    The key is a supported input pixel format, and the value is a supported output pixel formats' vector.
    In json format:
    {
        supportedInputPixelFormat: [supportedOutputPixelFormat, supportedOutputPixelFormat, ...],
        ......
    }

    eg.
    Supported input pixel formats are nv21, nv12 and rgba.
    When the input pixel format is nv21, the supported output pixel format can be nv21, nv12.
    When the input pixel format is nv12, the supported output pixel format can be nv21, nv12 and rgba.
    When the input pixel format is rgba, the supported output pixel format can be rgba.
    So the keys are nv21, nv12 and rgba, the corresponding value is a vector contains the supported output
    pixel formats.
    In json format:
    {
        nv21: [nv21, nv12],
        nv12: [nv21, nv12, rgba],
        rgba: [rgba],
        ......
    }
    */
    std::map<GraphicPixelFormat, std::vector<GraphicPixelFormat>> pixelFormatMap;
    uint32_t rank;
    int32_t version;
};

using ColorSpaceConverterCapabilitiesBuilder = std::function<std::vector<ColorSpaceConverterCapability>()>;
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // FRAMEWORK_ALGORITHM_COLORSPACE_CONVERTER_COLORSPACE_CONVERTER_CAPABILITY_H
