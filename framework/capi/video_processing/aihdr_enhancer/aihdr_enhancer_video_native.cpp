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

#include "aihdr_enhancer_video_native.h"

#include <unordered_map>

#include "native_window.h"

#include "video_processing_utils.h"
#include "vpe_log.h"

using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;

int32_t AihdrEnhancerVideoNative::InitializeInner()
{
    CHECK_AND_RETURN_RET_LOG(!isInitialized_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already init!");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(!isInitialized_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already init!");
    aihdrEnhancer_ = AihdrEnhancerVideo::Create();
    CHECK_AND_RETURN_RET_LOG(aihdrEnhancer_ != nullptr, VIDEO_PROCESSING_ERROR_CREATE_FAILED,
        "Create colorSpace converter failed!");
    isInitialized_ = true;
    return 0;
}

int32_t AihdrEnhancerVideoNative::DeinitializeInner()
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already deinit!");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already deinit!");
    aihdrEnhancer_ = nullptr;
    isInitialized_ = false;
    return 0;
}

int32_t AihdrEnhancerVideoNative::RegisterCallback()
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");
    auto callback = std::make_shared<NativeCallback>(shared_from_this());
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, VIDEO_PROCESSING_ERROR_CREATE_FAILED,
        "Create callback failed!");
    CHECK_AND_RETURN_RET_LOG(aihdrEnhancer_->SetCallback(callback) == 0,
        VIDEO_PROCESSING_ERROR_PROCESS_FAILED, "RegisterCallback failed!");
    return 0;
}

int32_t AihdrEnhancerVideoNative::SetSurface(const OHNativeWindow* window)
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");
    CHECK_AND_RETURN_RET_LOG(window != nullptr && window->surface != nullptr,
        VIDEO_PROCESSING_ERROR_INVALID_PARAMETER, "window is null or surface buffer is null!");
    return aihdrEnhancer_->SetSurface(window);
}

int32_t AihdrEnhancerVideoNative::GetSurface(OHNativeWindow** window)
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");
    CHECK_AND_RETURN_RET_LOG(window != nullptr, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER, "window is null!");
    return aihdrEnhancer_->GetSurface(window);
}

int32_t AihdrEnhancerVideoNative::SetParameter(const int& parameter)
{
    return VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED;
}

int32_t AihdrEnhancerVideoNative::GetParameter(int& parameter)
{
    return VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED;
}

int32_t AihdrEnhancerVideoNative::OnStart()
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");

    int32_t ret = 0;
    ret = aihdrEnhancer_->Configure();
    CHECK_AND_RETURN_RET_LOG(ret == 0, VideoProcessingUtils::InnerErrorToNDK(static_cast<VPEAlgoErrCode>(ret)),
        "Configure fail!");
    ret = aihdrEnhancer_->Prepare();
    CHECK_AND_RETURN_RET_LOG(ret == 0, VideoProcessingUtils::InnerErrorToNDK(static_cast<VPEAlgoErrCode>(ret)),
        "Prepare fail!");
    ret = aihdrEnhancer_->Start();
    CHECK_AND_RETURN_RET_LOG(ret == 0, VideoProcessingUtils::InnerErrorToNDK(static_cast<VPEAlgoErrCode>(ret)),
        "Start fail!");
    return 0;
}

int32_t AihdrEnhancerVideoNative::OnStop()
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");

    return static_cast<int32_t>(aihdrEnhancer_->Stop());
}

int32_t AihdrEnhancerVideoNative::OnRenderOutputBuffer(uint32_t index)
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");

    return static_cast<int32_t>(aihdrEnhancer_->ReleaseOutputBuffer(index, true));
}

void AihdrEnhancerVideoNative::OnError(int32_t errorCode)
{
    CHECK_AND_RETURN_LOG(onError_ != nullptr, "onError_ is null!");
    onError_(errorCode);
}

void AihdrEnhancerVideoNative::OnState(int32_t state)
{
    CHECK_AND_RETURN_LOG(onState_ != nullptr, "onState_ is null!");
    onState_(state);
}

void AihdrEnhancerVideoNative::OnNewOutputBuffer(uint32_t index)
{
    OnRenderOutputBuffer(index);
    CHECK_AND_RETURN_LOG(onNewOutputBuffer_ != nullptr, "onNewOutputBuffer_ is null!");
    onNewOutputBuffer_(index);
}

void AihdrEnhancerVideoNative::BindOnError(VideoProcessingCallback_OnError onError)
{
    onError_ = onError;
}

void AihdrEnhancerVideoNative::BindOnState(VideoProcessingCallback_OnState onState)
{
    onState_ = onState;
}

void AihdrEnhancerVideoNative::BindOnNewOutputBuffer(VideoProcessingCallback_OnNewOutputBuffer onNewOutputBuffer)
{
    onNewOutputBuffer_ = onNewOutputBuffer;
}

AihdrEnhancerVideoNative::NativeCallback::NativeCallback(
    const std::shared_ptr<AihdrEnhancerVideoNative>& owner)
    : owner_(owner)
{
}

void AihdrEnhancerVideoNative::NativeCallback::OnError(int32_t errorCode)
{
    SendCallback([this, &errorCode]() {
        owner_->OnError(static_cast<int32_t>(errorCode));
    });
}

void AihdrEnhancerVideoNative::NativeCallback::OnState(int32_t state)
{
    SendCallback([this, &state]() {
        owner_->OnState(static_cast<int32_t>(state));
    });
}


void AihdrEnhancerVideoNative::NativeCallback::OnOutputBufferAvailable(uint32_t index,
    [[maybe_unused]] AihdrEnhancerBufferFlag flag)
{
    SendCallback([this, &index]() {
        owner_->OnNewOutputBuffer(index);
    });
}

void AihdrEnhancerVideoNative::NativeCallback::SendCallback(std::function<void(void)>&& callback) const
{
    CHECK_AND_RETURN_LOG(owner_ != nullptr, "owner_ is null!");
    callback();
}
