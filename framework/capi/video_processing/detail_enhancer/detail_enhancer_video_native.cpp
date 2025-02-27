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

#include "detail_enhancer_video_native.h"

#include <unordered_map>

#include "vpe_log.h"
#include "video_processing_utils.h"

using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;

namespace {
const std::unordered_map<int, int> LEVEL_MAP = {
    { VIDEO_DETAIL_ENHANCER_QUALITY_LEVEL_NONE,     DETAIL_ENH_LEVEL_NONE },
    { VIDEO_DETAIL_ENHANCER_QUALITY_LEVEL_LOW,      DETAIL_ENH_LEVEL_LOW },
    { VIDEO_DETAIL_ENHANCER_QUALITY_LEVEL_MEDIUM,   DETAIL_ENH_LEVEL_MEDIUM },
    { VIDEO_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH,     DETAIL_ENH_LEVEL_HIGH },
};
}

VideoProcessing_ErrorCode DetailEnhancerVideoNative::InitializeInner()
{
    CHECK_AND_RETURN_RET_LOG(!isInitialized_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already init!");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(!isInitialized_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already init!");
    detailEnhancer_ = DetailEnhancerVideo::Create();
    CHECK_AND_RETURN_RET_LOG(detailEnhancer_ != nullptr, VIDEO_PROCESSING_ERROR_CREATE_FAILED,
        "Create detail enhancement failed!");
    isInitialized_ = true;
    return VIDEO_PROCESSING_SUCCESS;
}

VideoProcessing_ErrorCode DetailEnhancerVideoNative::DeinitializeInner()
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already deinit!");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already deinit!");
    detailEnhancer_ = nullptr;
    isInitialized_ = false;
    return VIDEO_PROCESSING_SUCCESS;
}

VideoProcessing_ErrorCode DetailEnhancerVideoNative::RegisterCallback()
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");

    auto callback = std::make_shared<NativeCallback>(shared_from_this());
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, VIDEO_PROCESSING_ERROR_CREATE_FAILED,
        "Create callback failed!");
    CHECK_AND_RETURN_RET_LOG(detailEnhancer_->RegisterCallback(callback) == VPE_ALGO_ERR_OK,
        VIDEO_PROCESSING_ERROR_PROCESS_FAILED, "RegisterCallback failed!");
    return VIDEO_PROCESSING_SUCCESS;
}

VideoProcessing_ErrorCode DetailEnhancerVideoNative::SetSurface(const sptr<Surface>& surface)
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");
    CHECK_AND_RETURN_RET_LOG(surface != nullptr, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER,
        "surface is null!");
    BufferRequestConfig bufferRequestConfig = surface->GetWindowConfig();
    surface->SetRequestWidthAndHeight(bufferRequestConfig.width, bufferRequestConfig.height);
    detailEnhancer_->SetOutputSurface(surface);
    return VIDEO_PROCESSING_SUCCESS;
}

sptr<Surface> DetailEnhancerVideoNative::GetSurface()
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), nullptr, "Initialization failed!");
    return detailEnhancer_->GetInputSurface();
}

VideoProcessing_ErrorCode DetailEnhancerVideoNative::SetParameter(const OHOS::Media::Format& parameter)
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");

    int level;
    CHECK_AND_RETURN_RET_LOG(parameter.GetIntValue(VIDEO_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL, level),
        VIDEO_PROCESSING_ERROR_INVALID_PARAMETER, "No quality level!");
    int innerLevel = NDKLevelToInner(level);
    CHECK_AND_RETURN_RET_LOG(innerLevel != -1, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER, "Quality level is invalid!");
    DetailEnhancerParameters param{};
    param.level = static_cast<DetailEnhancerLevel>(innerLevel);
    auto result = VideoProcessingUtils::InnerErrorToNDK(detailEnhancer_->SetParameter(param, VIDEO));
    if (result == VIDEO_PROCESSING_SUCCESS) {
        level_ = level;
    }
    return result;
}

VideoProcessing_ErrorCode DetailEnhancerVideoNative::GetParameter(OHOS::Media::Format& parameter)
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");

    int level = level_.load();
    CHECK_AND_RETURN_RET_LOG(level != -1, VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "No quality level set!");
    CHECK_AND_RETURN_RET_LOG(parameter.PutIntValue(VIDEO_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL, level),
        VIDEO_PROCESSING_ERROR_PROCESS_FAILED, "Get parameter failed!");
    return VIDEO_PROCESSING_SUCCESS;
}

VideoProcessing_ErrorCode DetailEnhancerVideoNative::OnStart()
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");

    return VideoProcessingUtils::InnerErrorToNDK(detailEnhancer_->Start());
}

VideoProcessing_ErrorCode DetailEnhancerVideoNative::OnStop()
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");

    return VideoProcessingUtils::InnerErrorToNDK(detailEnhancer_->Stop());
}

VideoProcessing_ErrorCode DetailEnhancerVideoNative::OnRenderOutputBuffer(uint32_t index)
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");

    return VideoProcessingUtils::InnerErrorToNDK(detailEnhancer_->ReleaseOutputBuffer(index, true));
}

int DetailEnhancerVideoNative::NDKLevelToInner(int level) const
{
    auto it = LEVEL_MAP.find(level);
    if (it == LEVEL_MAP.end()) [[unlikely]] {
        VPE_LOGE("Invalid input level:%{public}d", level);
        return -1;
    }
    return it->second;
}

DetailEnhancerVideoNative::NativeCallback::NativeCallback(const std::shared_ptr<DetailEnhancerVideoNative>& owner)
    : owner_(owner)
{
}

void DetailEnhancerVideoNative::NativeCallback::OnError(VPEAlgoErrCode errorCode)
{
    SendCallback([this, &errorCode]() {
        owner_->OnError(VideoProcessingUtils::InnerErrorToNDK(errorCode));
    });
}

void DetailEnhancerVideoNative::NativeCallback::OnState(VPEAlgoState state)
{
    SendCallback([this, &state]() {
        owner_->OnState(VideoProcessingUtils::InnerStateToNDK(state));
    });
}

void DetailEnhancerVideoNative::NativeCallback::OnOutputBufferAvailable(uint32_t index,
    [[maybe_unused]] DetailEnhBufferFlag flag)
{
    SendCallback([this, &index]() {
        owner_->OnNewOutputBuffer(index);
    });
}

void DetailEnhancerVideoNative::NativeCallback::SendCallback(std::function<void(void)>&& callback) const
{
    if (owner_ == nullptr) {
        VPE_LOGE("owner is null!");
        return;
    }

    callback();
}
