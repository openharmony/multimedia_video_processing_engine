/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef DETAIL_ENHANCER_IMAGE_FWK_H
#define DETAIL_ENHANCER_IMAGE_FWK_H

#include <unordered_set>

#include "detail_enhancer_image.h"
#include "detail_enhancer_base.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class DetailEnhancerImageFwk : public DetailEnhancerImage {
public:
    explicit DetailEnhancerImageFwk(int type);
    ~DetailEnhancerImageFwk() override;
    DetailEnhancerImageFwk(const DetailEnhancerImageFwk&) = delete;
    DetailEnhancerImageFwk& operator=(const DetailEnhancerImageFwk&) = delete;
    DetailEnhancerImageFwk(DetailEnhancerImageFwk&&) = delete;
    DetailEnhancerImageFwk& operator=(DetailEnhancerImageFwk&&) = delete;

    VPEAlgoErrCode SetParameter(const DetailEnhancerParameters& parameter) override;
    VPEAlgoErrCode GetParameter(DetailEnhancerParameters& parameter) const override;
    VPEAlgoErrCode Process(const sptr<SurfaceBuffer>& input, const sptr<SurfaceBuffer>& output) override;
    VPEAlgoErrCode EnableProtection(bool enable) final;
    VPEAlgoErrCode ResetProtectionStatus() final;

private:
    std::shared_ptr<DetailEnhancerBase> GetAlgorithm(int feature);
    std::shared_ptr<DetailEnhancerBase> CreateAlgorithm(int feature);
    bool IsValidProcessedObject(const sptr<SurfaceBuffer>& input, const sptr<SurfaceBuffer>& output);
    int EvaluateTargetLevel(const sptr<SurfaceBuffer>& input, const sptr<SurfaceBuffer>& output,
        float widthRatio, float heightRatio) const;
    VPEAlgoErrCode DoProcess(const sptr<SurfaceBuffer>& input, const sptr<SurfaceBuffer>& output);
    VPEAlgoErrCode ProcessVideo(const sptr<SurfaceBuffer>& input, const sptr<SurfaceBuffer>& output);
    void UpdateLastAlgorithm(const std::shared_ptr<DetailEnhancerBase>& algorithm);
    void Clear();
    VPEAlgoErrCode ProcessAlgorithm(const std::shared_ptr<DetailEnhancerBase>& algo, const sptr<SurfaceBuffer>& input,
        const sptr<SurfaceBuffer>& output);

    DetailEnhancerParameters parameter_{};
    mutable std::mutex lock_{};
    std::unordered_map<int, std::shared_ptr<DetailEnhancerBase>> algorithms_{};
    std::shared_ptr<DetailEnhancerBase> lastAlgorithm_{};
    int type_;
    std::atomic<bool> parameterUpdated{};
    bool hasParameter_{};
    bool enableProtection_{};
    mutable std::mutex restoreLock_{};
    bool needRestore_{}; // Guarded by restoreLock_
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // DETAIL_ENHANCER_IMAGE_IMPL_H
