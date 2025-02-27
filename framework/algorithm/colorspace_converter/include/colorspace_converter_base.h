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

#ifndef FRAMEWORK_ALGORITHM_COLORSPACE_CONVERTER_COLORSPACE_CONVERTER_BASE_H
#define FRAMEWORK_ALGORITHM_COLORSPACE_CONVERTER_COLORSPACE_CONVERTER_BASE_H

#include <functional>
#include <memory>
#include "nocopyable.h"
#include "frame_info.h"
#include "vpe_context.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class ColorSpaceConverterBase : public NoCopyable {
public:
    virtual ~ColorSpaceConverterBase() = default;
    virtual VPEAlgoErrCode Init(const FrameInfo &inputFrameInfo, const FrameInfo &outputFrameInfo,
        VPEContext context) = 0;
    virtual VPEAlgoErrCode Deinit() = 0;
    virtual VPEAlgoErrCode SetParameter(const ColorSpaceConverterParameter &parameter) = 0;
    virtual VPEAlgoErrCode GetParameter(ColorSpaceConverterParameter &parameter) = 0;
    virtual VPEAlgoErrCode Process(const sptr<SurfaceBuffer> &input, const sptr<SurfaceBuffer> &output) = 0;
    virtual VPEAlgoErrCode ComposeImage(const sptr<SurfaceBuffer> &inputSdrImage,
        const sptr<SurfaceBuffer> &inputGainmap, const sptr<SurfaceBuffer> &outputHdrImage, bool legacy) = 0;
    virtual VPEAlgoErrCode DecomposeImage(const sptr<SurfaceBuffer> &inputImage,
        const sptr<SurfaceBuffer> &outputSdrImage, const sptr<SurfaceBuffer> &outputGainmap) = 0;
};

using ColorSpaceConverterCreator = std::function<std::shared_ptr<ColorSpaceConverterBase>()>;
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // FRAMEWORK_ALGORITHM_COLORSPACE_CONVERTER_COLORSPACE_CONVERTER_BASE_H
