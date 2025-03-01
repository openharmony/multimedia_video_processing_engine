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

#ifndef DETAIL_ENHANCER_VIDEO_IMPL_H
#define DETAIL_ENHANCER_VIDEO_IMPL_H

#include <atomic>
#include <memory>
#include <mutex>

#include "algorithm_video_common.h"
#include "detail_enhancer_video.h"
#include "detail_enhancer_video_fwk.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class DetailEnhancerVideoImpl : public DetailEnhancerVideo {
public:
    DetailEnhancerVideoImpl() = default;
    ~DetailEnhancerVideoImpl() = default;

    int32_t Init();

    VPEAlgoErrCode RegisterCallback(const std::shared_ptr<VpeVideoCallback>& callback) override;
    VPEAlgoErrCode SetOutputSurface(const sptr<Surface>& surface) override;
    sptr<Surface> GetInputSurface() override;
    VPEAlgoErrCode SetParameter(const DetailEnhancerParameters& parameter, SourceType type) override;
    VPEAlgoErrCode Start() override;
    VPEAlgoErrCode Stop() override;
    VPEAlgoErrCode RenderOutputBuffer(uint32_t index) override;
    VPEAlgoErrCode ReleaseOutputBuffer(uint32_t index, bool render) override;

    VPEAlgoErrCode NotifyEos() override;

private:
    std::mutex lock_{};
    // Guarded by lock_ begin
    std::atomic<bool> isInitialized_{false};
    std::shared_ptr<VpeVideoImpl> detailEnhancerVideo_{};
    // Guarded by lock_ end
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // DETAIL_ENHANCER_VIDEO_IMPL_H
