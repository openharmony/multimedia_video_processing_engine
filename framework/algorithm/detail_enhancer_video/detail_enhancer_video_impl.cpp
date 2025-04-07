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

#include "detail_enhancer_video_impl.h"

#include "vpe_log.h"
#include "vpe_trace.h"

constexpr int MAX_URL_LENGTH = 100;

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
std::shared_ptr<DetailEnhancerVideo> DetailEnhancerVideo::Create()
{
    std::shared_ptr<DetailEnhancerVideoImpl> impl = std::make_shared<DetailEnhancerVideoImpl>();
    int32_t ret = impl->Init();
    CHECK_AND_RETURN_RET_LOG(ret == static_cast<int32_t>(VPE_ALGO_ERR_OK), nullptr,
        "failed to init DetailEnhancerVideoImpl");
    return impl;
}

VPEAlgoErrCode DetailEnhancerVideoImpl::RegisterCallback(const std::shared_ptr<VpeVideoCallback>& callback)
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VPE_ALGO_ERR_INIT_FAILED, "Initialization was NOT successful!");
    return detailEnhancerVideo_->RegisterCallback(callback);
}

VPEAlgoErrCode DetailEnhancerVideoImpl::SetOutputSurface(const sptr<Surface>& surface)
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VPE_ALGO_ERR_INIT_FAILED, "Initialization was NOT successful!");
    return detailEnhancerVideo_->SetOutputSurface(surface);
}

sptr<Surface> DetailEnhancerVideoImpl::GetInputSurface()
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), nullptr, "Initialization was NOT successful!");
    return detailEnhancerVideo_->GetInputSurface();
}

VPEAlgoErrCode DetailEnhancerVideoImpl::SetParameter(const DetailEnhancerParameters& parameter, SourceType type)
{
    CHECK_AND_RETURN_RET_LOG(parameter.level >= DETAIL_ENH_LEVEL_NONE && parameter.level <= DETAIL_ENH_LEVEL_HIGH &&
        parameter.uri.length() < MAX_URL_LENGTH, VPE_ALGO_ERR_INVALID_VAL, "Invalid algo level");
    CHECK_AND_RETURN_RET_LOG(type == VIDEO, VPE_ALGO_ERR_INVALID_VAL, "Invalid source type");

    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VPE_ALGO_ERR_INIT_FAILED, "Initialization was NOT successful!");

    Format fmt;
    CHECK_AND_RETURN_RET_LOG(fmt.PutIntValue(ParameterKey::DETAIL_ENHANCER_QUALITY_LEVEL, parameter.level),
        VPE_ALGO_ERR_INVALID_VAL, "Failed to put level(%{public}d) to format!", parameter.level);
    return detailEnhancerVideo_->SetParameter(fmt);
}

VPEAlgoErrCode DetailEnhancerVideoImpl::Start()
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VPE_ALGO_ERR_INIT_FAILED, "Initialization was NOT successful!");
    return detailEnhancerVideo_->Start();
}

VPEAlgoErrCode DetailEnhancerVideoImpl::Stop()
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VPE_ALGO_ERR_INIT_FAILED, "Initialization was NOT successful!");
    return detailEnhancerVideo_->Stop();
}

VPEAlgoErrCode DetailEnhancerVideoImpl::Release()
{
    return detailEnhancerVideo_->Release();
}

VPEAlgoErrCode DetailEnhancerVideoImpl::RenderOutputBuffer([[maybe_unused]] uint32_t index)
{
    return VPE_ALGO_ERR_OK;
}

int32_t DetailEnhancerVideoImpl::Init()
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(!isInitialized_.load(), VPE_ALGO_ERR_INVALID_STATE, "Already initialized!");
    detailEnhancerVideo_ = DetailEnhancerVideoFwk::CreateEx(false);
    CHECK_AND_RETURN_RET_LOG(detailEnhancerVideo_ != nullptr, VPE_ALGO_ERR_UNKNOWN,
        "Failed to create video detail enhancer!");
    isInitialized_ = true;
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode DetailEnhancerVideoImpl::NotifyEos()
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VPE_ALGO_ERR_INIT_FAILED, "Initialization was NOT successful!");
    return detailEnhancerVideo_->NotifyEos();
}

VPEAlgoErrCode DetailEnhancerVideoImpl::ReleaseOutputBuffer(uint32_t index, bool render)
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VPE_ALGO_ERR_INIT_FAILED, "Initialization was NOT successful!");
    return detailEnhancerVideo_->ReleaseOutputBuffer(index, render);
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
