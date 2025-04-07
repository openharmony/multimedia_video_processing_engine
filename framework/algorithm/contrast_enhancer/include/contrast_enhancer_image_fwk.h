/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CONTRAST_ENHANCER_IMAGE_FWK_H
#define CONTRAST_ENHANCER_IMAGE_FWK_H

#include <unordered_set>

#include "contrast_enhancer_image.h"
#include "contrast_enhancer_base.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class ContrastEnhancerImageFwk : public ContrastEnhancerImage {
public:
    explicit ContrastEnhancerImageFwk();
    ~ContrastEnhancerImageFwk() override;
    ContrastEnhancerImageFwk(const ContrastEnhancerImageFwk&) = delete;
    ContrastEnhancerImageFwk& operator=(const ContrastEnhancerImageFwk&) = delete;
    ContrastEnhancerImageFwk(ContrastEnhancerImageFwk&&) = delete;
    ContrastEnhancerImageFwk& operator=(ContrastEnhancerImageFwk&&) = delete;

    VPEAlgoErrCode SetParameter(const ContrastEnhancerParameters& parameter) override;
    VPEAlgoErrCode GetParameter(ContrastEnhancerParameters& parameter) const override;
    VPEAlgoErrCode GetRegionHist(const sptr<SurfaceBuffer>& input) override;
    VPEAlgoErrCode UpdateMetadataBasedOnHist(OHOS::Rect displayArea, sptr<SurfaceBuffer> surfaceBuffer,
        std::tuple<int, int, double, double, double, int> pixelmapInfo) override;
    VPEAlgoErrCode UpdateMetadataBasedOnPixel(OHOS::Rect displayArea, OHOS::Rect curPixelmapArea,
        OHOS::Rect completePixelmapArea, sptr<SurfaceBuffer> surfaceBuffer, float fullRatio) override;
private:
    std::shared_ptr<ContrastEnhancerBase> GetAlgorithm(ContrastEnhancerType feature);
    std::shared_ptr<ContrastEnhancerBase> CreateAlgorithm(ContrastEnhancerType feature);
    bool IsValidProcessedObject(const sptr<SurfaceBuffer>& buffer);

    ContrastEnhancerParameters parameter_{};
    mutable std::mutex lock_{};
    std::mutex getAlgoLock_{};
    std::unordered_map<ContrastEnhancerType, std::shared_ptr<ContrastEnhancerBase>> algorithms_{};
    std::atomic<int> failureCount_{};
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // CONTRAST_ENHANCER_IMAGE_IMPL_H
