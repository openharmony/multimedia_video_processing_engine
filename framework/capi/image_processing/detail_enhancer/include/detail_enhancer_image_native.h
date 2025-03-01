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

#ifndef DETAIL_ENHANCER_IMAGE_NATIVE_H
#define DETAIL_ENHANCER_IMAGE_NATIVE_H

#include <atomic>
#include <functional>
#include <mutex>

#include "image_processing_native_template.h"
#include "image_processing_types.h"
#include "pixelmap_native_impl.h"

#include "detail_enhancer_image.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

class DetailEnhancerImageNative : public ImageProcessingNativeTemplate<DetailEnhancerImageNative> {
public:
    DEFINE_WITH_DISALLOW_COPY_AND_MOVE(DetailEnhancerImageNative);

    ImageProcessing_ErrorCode InitializeInner() override;
    ImageProcessing_ErrorCode DeinitializeInner() override;
    ImageProcessing_ErrorCode SetParameter(const OHOS::Media::Format& parameter) override;
    ImageProcessing_ErrorCode GetParameter(OHOS::Media::Format& parameter) override;
    ImageProcessing_ErrorCode Process(const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
        std::shared_ptr<OHOS::Media::PixelMap>& destinationImage) override;

private:
    ImageProcessing_ErrorCode CheckParameter();
    int LevelTransfer(int level, const std::unordered_map<int, int> levelMap) const;

    mutable std::mutex lock_{};
    // Guarded by lock_ begin
    std::atomic<bool> isInitialized_{false};
    std::shared_ptr<DetailEnhancerImage> detailEnhancer_{};
    // Guarded by lock_ end
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // DETAIL_ENHANCER_IMAGE_NATIVE_H