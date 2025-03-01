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

#ifndef FRAMEWORK_ALGORITHM_COLORSPACE_CONVERTER_FWK_H
#define FRAMEWORK_ALGORITHM_COLORSPACE_CONVERTER_FWK_H

#include <atomic>
#include <memory>
#include <optional>
#include "colorspace_converter.h"
#include "colorspace_converter_base.h"
#include "extension_base.h"
#include "metadata_generator.h"
#include "metadata_generator_base.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
/**
 * Process
 * 解码后的视频帧转换
 * 单层hdr图片 -> sdr图片
 * sdr图片     -> 单层hdr图片
 * sdr图片     -> sdr图片
 * ComposeImage
 * 双层hdr图片  -> 单层hdr图片
 * DecomposeImage
 * 单层hdr图片  -> 双层hdr图片
 * sdr图片      -> 双层hdr图片
 */
class ColorSpaceConverterFwk : public ColorSpaceConverter {
public:
    ColorSpaceConverterFwk();
    ColorSpaceConverterFwk(std::shared_ptr<OpenGLContext> openglContext,
                           ClContext *opengclContext = nullptr);
    ~ColorSpaceConverterFwk();
    VPEAlgoErrCode SetParameter(const ColorSpaceConverterParameter &parameter) override;
    VPEAlgoErrCode GetParameter(ColorSpaceConverterParameter &parameter) const override;
    VPEAlgoErrCode Process(const sptr<SurfaceBuffer> &input, const sptr<SurfaceBuffer> &output) override;
    VPEAlgoErrCode ComposeImage(const sptr<SurfaceBuffer> &inputSdrImage, const sptr<SurfaceBuffer> &inputGainmap,
        const sptr<SurfaceBuffer> &outputHdrImage, bool legacy) override;
    VPEAlgoErrCode DecomposeImage(const sptr<SurfaceBuffer> &inputImage, const sptr<SurfaceBuffer> &outputSdrImage,
        const sptr<SurfaceBuffer> &outputGainmap) override;

private:
    VPEAlgoErrCode Init(const sptr<SurfaceBuffer> &input, const sptr<SurfaceBuffer> &output, VPEContext context);
    void OpenGLInit();
    void OpenCLInit();

    std::shared_ptr<ColorSpaceConverterBase> impl_ { nullptr };
    std::optional<ColorSpaceConverterParameter> parameter_ { std::nullopt };
    std::atomic<bool> initialized_ { false };
    Extension::ExtensionInfo extensionInfo_;
    std::map<
        std::tuple<ColorSpaceDescription, GraphicPixelFormat, ColorSpaceDescription, GraphicPixelFormat>,
        std::shared_ptr<ColorSpaceConverterBase>> impls_;
    std::tuple<ColorSpaceDescription, GraphicPixelFormat, ColorSpaceDescription, GraphicPixelFormat>
        lastFrameInfoKey_;
    VPEContext context;
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // FRAMEWORK_ALGORITHM_COLORSPACE_CONVERTER_FWK_H
