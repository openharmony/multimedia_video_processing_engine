/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef IMAGE_PROCESSING_INTERFACE_H
#define IMAGE_PROCESSING_INTERFACE_H

#include "image_processing_types.h"
#include "algorithm_common.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class IImageProcessingNative {
public:
    virtual ImageProcessing_ErrorCode Initialize() = 0;
    virtual ImageProcessing_ErrorCode Deinitialize() = 0;
    virtual ImageProcessing_ErrorCode SetParameter(const OH_AVFormat* parameter) = 0;
    virtual ImageProcessing_ErrorCode GetParameter(OH_AVFormat* parameter) = 0;
    virtual ImageProcessing_ErrorCode Process(OH_PixelmapNative* sourceImage,
        OH_PixelmapNative* destinationImage) = 0;
    virtual ImageProcessing_ErrorCode ConvertColorSpace(OH_PixelmapNative* sourceImage,
        OH_PixelmapNative* destinationImage) = 0;
    virtual ImageProcessing_ErrorCode Compose(OH_PixelmapNative* sourceImage, OH_PixelmapNative* sourceGainmap,
        OH_PixelmapNative* destinationImage) = 0;
    virtual ImageProcessing_ErrorCode Decompose(OH_PixelmapNative* sourceImage, OH_PixelmapNative* destinationImage,
        OH_PixelmapNative* destinationGainmap) = 0;
    virtual ImageProcessing_ErrorCode GenerateMetadata(OH_PixelmapNative* sourceImage) = 0;
    virtual ImageProcessing_ErrorCode EnhanceDetail(OH_PixelmapNative* sourceImage,
        OH_PixelmapNative* destinationImage) = 0;

protected:
    IImageProcessingNative() = default;
    virtual ~IImageProcessingNative() = default;
    IImageProcessingNative(const IImageProcessingNative&) = delete;
    IImageProcessingNative& operator=(const IImageProcessingNative&) = delete;
    IImageProcessingNative(IImageProcessingNative&&) = delete;
    IImageProcessingNative& operator=(IImageProcessingNative&&) = delete;

public:
    ClContext *opengclContext_ {nullptr};
    std::shared_ptr<OpenGLContext> openglContext_ {nullptr};
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
 
#endif // IMAGE_PROCESSING_INTERFACE_H