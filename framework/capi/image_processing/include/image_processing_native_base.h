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

#ifndef IMAGE_PROCESSING_NATIVE_BASE_H
#define IMAGE_PROCESSING_NATIVE_BASE_H

#include "common/native_mfmagic.h"
#include "pixelmap_native_impl.h"
#include "pixel_map.h"

#include "image_processing_interface.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
/**
 * Base implementaion for image processing.
 */
class ImageProcessingNativeBase : public IImageProcessingNative {
public:
    ImageProcessing_ErrorCode Initialize() final;
    ImageProcessing_ErrorCode Deinitialize() final;
    ImageProcessing_ErrorCode SetParameter(const OH_AVFormat* parameter) final;
    ImageProcessing_ErrorCode GetParameter(OH_AVFormat* parameter) final;
    ImageProcessing_ErrorCode Process(OH_PixelmapNative* sourceImage,
    OH_PixelmapNative* destinationImage) final;
    ImageProcessing_ErrorCode ConvertColorSpace(OH_PixelmapNative* sourceImage,
        OH_PixelmapNative* destinationImage) final;
    ImageProcessing_ErrorCode Compose(OH_PixelmapNative* sourceImage,
        OH_PixelmapNative* sourceGainmap, OH_PixelmapNative* destinationImage) final;
    ImageProcessing_ErrorCode Decompose(OH_PixelmapNative* sourceImage,
        OH_PixelmapNative* destinationImage, OH_PixelmapNative* destinationGainmap) final;
    ImageProcessing_ErrorCode GenerateMetadata(OH_PixelmapNative* sourceImage) final;
    ImageProcessing_ErrorCode EnhanceDetail(OH_PixelmapNative* sourceImage, OH_PixelmapNative* destinationImage) final;
protected:
    explicit ImageProcessingNativeBase();
    virtual ~ImageProcessingNativeBase() = default;
    ImageProcessingNativeBase(const ImageProcessingNativeBase&) = delete;
    ImageProcessingNativeBase& operator=(const ImageProcessingNativeBase&) = delete;
    ImageProcessingNativeBase(ImageProcessingNativeBase&&) = delete;
    ImageProcessingNativeBase& operator=(ImageProcessingNativeBase&&) = delete;
 
    virtual ImageProcessing_ErrorCode InitializeInner();
    virtual ImageProcessing_ErrorCode DeinitializeInner();
    virtual ImageProcessing_ErrorCode SetParameter(const OHOS::Media::Format& parameter);
    virtual ImageProcessing_ErrorCode GetParameter(OHOS::Media::Format& parameter);
    virtual ImageProcessing_ErrorCode Process(const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
        std::shared_ptr<OHOS::Media::PixelMap>& destinationImage);
    virtual ImageProcessing_ErrorCode ConvertColorSpace(const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
        std::shared_ptr<OHOS::Media::PixelMap>& destinationImage);
    virtual ImageProcessing_ErrorCode Compose(const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
        const std::shared_ptr<OHOS::Media::PixelMap>& sourceGainmap,
        std::shared_ptr<OHOS::Media::PixelMap>& destinationImage);
    virtual ImageProcessing_ErrorCode Decompose(const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
        std::shared_ptr<OHOS::Media::PixelMap>& destinationImage,
        std::shared_ptr<OHOS::Media::PixelMap>& destinationGainmap);
    virtual ImageProcessing_ErrorCode GenerateMetadata(const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage);
    virtual ImageProcessing_ErrorCode EnhanceDetail(const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
    std::shared_ptr<OHOS::Media::PixelMap>& destinationImage);

private:
    mutable std::mutex lock_{};
    // Guarded by lock_ begin
    std::atomic<bool> isInitialized_{false};
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
 
#endif // IMAGE_PROCESSING_NATIVE_BASE_H