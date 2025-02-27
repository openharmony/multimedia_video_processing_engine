/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef DETAIL_ENHANCER_VIDEO_FWK_H
#define DETAIL_ENHANCER_VIDEO_FWK_H

#include <memory>
#include <mutex>

#include "algorithm_video_impl.h"
#include "detail_enhancer_image_fwk.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class DetailEnhancerVideoFwk : public VpeVideoImpl {
public:
    static std::shared_ptr<VpeVideoImpl> Create();

    explicit DetailEnhancerVideoFwk(uint32_t type) : VpeVideoImpl(type) {}
    virtual ~DetailEnhancerVideoFwk() = default;
    DetailEnhancerVideoFwk(const DetailEnhancerVideoFwk&) = delete;
    DetailEnhancerVideoFwk& operator=(const DetailEnhancerVideoFwk&) = delete;
    DetailEnhancerVideoFwk(DetailEnhancerVideoFwk&&) = delete;
    DetailEnhancerVideoFwk& operator=(DetailEnhancerVideoFwk&&) = delete;

    VPEAlgoErrCode SetParameter(const Format& parameter) final;
    VPEAlgoErrCode GetParameter(Format& parameter) final;

protected:
    VPEAlgoErrCode OnInitialize() final;
    VPEAlgoErrCode OnDeinitialize() final;
    VPEAlgoErrCode Process(const sptr<SurfaceBuffer>& sourceImage, sptr<SurfaceBuffer>& destinationImage) final;
    bool IsProducerSurfaceValid(const sptr<Surface>& surface) final;
    VPEAlgoErrCode UpdateRequestCfg(const sptr<Surface>& surface, BufferRequestConfig& requestCfg) final;
    void UpdateRequestCfg(const sptr<SurfaceBuffer>& consumerBuffer, BufferRequestConfig& requestCfg) final;

private:
    enum ParamError {
        PARAM_ERR_INVALID = -1,
        PARAM_ERR_NOT_FOUND = 0,
        PARAM_ERR_OK = 1,
    };

    ParamError SetLevel(const Format& parameter);
    ParamError SetTargetSize(const Format& parameter);
    ParamError SetAutoDownshift(const Format& parameter);

    std::mutex lock_{};
    // Guarded by lock_ begin
    DetailEnhancerQualityLevel level_{DETAIL_ENHANCER_LEVEL_LOW};
    VpeBufferSize size_{};
    DetailEnhancerQualityLevel lastEffectiveLevel_{};
    VpeBufferSize lastEffectiveSize_{};
    bool isAutoDownshift_{true};
    // Guarded by lock_ end

    std::shared_ptr<DetailEnhancerImage> detailEnh_{};
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // DETAIL_ENHANCER_VIDEO_FWK_H
