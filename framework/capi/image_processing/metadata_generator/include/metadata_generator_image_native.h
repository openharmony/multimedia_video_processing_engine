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

#ifndef METADATA_GENERATOR_IMAGE_NATIVE_H
#define METADATA_GENERATOR_IMAGE_NATIVE_H

#include <atomic>
#include <functional>
#include <mutex>
#include <unordered_map>

#include "detail_enhancer_common.h"
#include "detail_enhancer_image_fwk.h"
#include "image_processing_utils.h"
#include "surface_buffer.h"
#include "surface_buffer_impl.h"
#include "surface_type.h"
#include "vpe_log.h"
#include "image_processing_native_template.h"
#include "image_processing_types.h"
#include "pixelmap_native_impl.h"
#include "detail_enhancer_image.h"
#include "metadata_generator.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class MetadataGeneratorImageNative : public ImageProcessingNativeTemplate<MetadataGeneratorImageNative> {
public:
    DEFINE_WITH_DISALLOW_COPY_AND_MOVE(MetadataGeneratorImageNative);

    ImageProcessing_ErrorCode InitializeInner() override;
    ImageProcessing_ErrorCode DeinitializeInner() override;
    ImageProcessing_ErrorCode SetParameter(const OHOS::Media::Format& parameter) override;
    ImageProcessing_ErrorCode GetParameter(OHOS::Media::Format& parameter) override;
    ImageProcessing_ErrorCode Process(const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
        std::shared_ptr<OHOS::Media::PixelMap>& destinationImage) override;
    ImageProcessing_ErrorCode ConvertColorSpace(const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
        std::shared_ptr<OHOS::Media::PixelMap>& destinationImage) override;
    ImageProcessing_ErrorCode Compose(const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
        const std::shared_ptr<OHOS::Media::PixelMap>& sourceGainmap,
        std::shared_ptr<OHOS::Media::PixelMap>& destinationImage) override;
    ImageProcessing_ErrorCode Decompose(const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
        std::shared_ptr<OHOS::Media::PixelMap>& destinationImage,
        std::shared_ptr<OHOS::Media::PixelMap>& destinationGainmap) override;
    ImageProcessing_ErrorCode GenerateMetadata(const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage) override;
    ImageProcessing_ErrorCode EnhanceDetail(const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
        std::shared_ptr<OHOS::Media::PixelMap>& destinationImage) override;
private:
    ImageProcessing_ErrorCode CheckParameter();
    ImageProcessing_ErrorCode ConvertPixelmapToSurfaceBuffer(const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap,
        sptr<SurfaceBuffer>& bufferImpl);
    uint32_t GetColorSpaceType(const CM_ColorSpaceInfo &colorSpaceInfo);
    CM_ColorSpaceInfo GetColorSpaceInfo(const uint32_t colorSpaceType);
    mutable std::mutex lock_{};
    // Guarded by lock_ begin
    std::atomic<bool> isInitialized_{false};
    std::shared_ptr<MetadataGenerator> metadataGenerator_{};
    // Guarded by lock_ end
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // METADATA_GENERATOR_IMAGE_NATIVE_H