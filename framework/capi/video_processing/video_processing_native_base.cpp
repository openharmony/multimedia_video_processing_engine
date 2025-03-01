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

#include "video_processing_native_base.h"

#include "common/native_mfmagic.h"
#include "native_window.h"

#include "video_environment_native.h"
#include "video_processing_callback_impl.h"
#include "vpe_log.h"

using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;

VideoProcessingNativeBase::VideoProcessingNativeBase(OH_VideoProcessing* context) : context_(context)
{
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::Initialize()
{
    CHECK_AND_RETURN_RET_LOG(!isInitialized_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already init!");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(!isInitialized_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already init!");
    auto result = InitializeInner();
    isInitialized_ = true;
    return result;
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::Deinitialize()
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already deinit!");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already deinit!");
    isInitialized_ = false;
    auto result = DeinitializeInner();
    return result;
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::RegisterCallback(const VideoProcessing_Callback* callback,
    void* userData)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER, "Callback is null!");
    CHECK_AND_RETURN_RET_LOG(callback->GetInnerCallback() != nullptr && callback->GetInnerCallback()->IsValid(),
        VIDEO_PROCESSING_ERROR_INVALID_VALUE, "Callback is empty!");

    return ExecuteWhenIdle([this, &callback, &userData]() {
        std::lock_guard<std::mutex> lock(callbackLock_);
        VideoProcessing_ErrorCode errorCode = PrepareRegistrationLocked();
        if (errorCode != VIDEO_PROCESSING_SUCCESS) {
            return errorCode;
        }
        auto [it, result]  = callbacks_.insert({ callback->GetInnerCallback(), userData });
        CHECK_AND_RETURN_RET_LOG(result, VIDEO_PROCESSING_ERROR_PROCESS_FAILED, "Add callback failed!");
        hasCallback_ = true;
        if (callback->GetInnerCallback()->HasOnNewOutputBuffer()) {
            hasOnRenderOutputBuffer_ = true;
        }
        return VIDEO_PROCESSING_SUCCESS;
    }, "Registration during running is not allowed!");
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::SetSurface(const OHNativeWindow* window)
{
    CHECK_AND_RETURN_RET_LOG(window != nullptr && window->surface != nullptr, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER,
        "window is null or surface buffer is null!");

    std::lock_guard<std::mutex> lock(lock_);
    auto result = SetSurface(window->surface, *window);
    hasOutputSurface_ = (result == VIDEO_PROCESSING_SUCCESS);
    return result;
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::GetSurface(OHNativeWindow** window)
{
    CHECK_AND_RETURN_RET_LOG(window != nullptr, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER, "window is null!");
    return ExecuteWhenIdle([this, &window]() {
        if (hasInputSurface_.load()) {
            VPE_LOGE("Input surface is already created!");
            return VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED;
        }

        sptr<Surface> surface = GetSurface();
        CHECK_AND_RETURN_RET_LOG(surface != nullptr, VIDEO_PROCESSING_ERROR_CREATE_FAILED, "get surface failed!");
        *window = CreateNativeWindowFromSurface(&surface);
        CHECK_AND_RETURN_RET_LOG(*window != nullptr, VIDEO_PROCESSING_ERROR_CREATE_FAILED, "create window failed!");
        hasInputSurface_ = true;
        return VIDEO_PROCESSING_SUCCESS;
    }, "Surface getting during running is not allowed!");
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::SetParameter(const OH_AVFormat* parameter)
{
    CHECK_AND_RETURN_RET_LOG(parameter != nullptr, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER, "parameter is null!");
    return SetParameter(parameter->format_);
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::GetParameter(OH_AVFormat* parameter)
{
    CHECK_AND_RETURN_RET_LOG(parameter != nullptr, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER, "parameter is null!");
    return GetParameter(parameter->format_);
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::Start()
{
    return ExecuteWhenIdle([this]() {
        CHECK_AND_RETURN_RET_LOG(hasInputSurface_.load() && hasOutputSurface_.load() && hasCallback_.load(),
            VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
            "Input/Output surface is not ready or the callback is not registered!");

        TraverseCallbacksLocked([](std::shared_ptr<VideoProcessingCallbackNative>& cb, void*) { cb->LockModifiers(); });
        isRunning_ = true;
        return OnStart();
    }, "Already start!");
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::Stop()
{
    return ExecuteWhenRunning([this]() {
        TraverseCallbacksLocked(
            [](std::shared_ptr<VideoProcessingCallbackNative>& cb, void*) { cb->UnlockModifiers(); });
        isRunning_ = false;
        return OnStop();
    }, "Already stop!");
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::RenderOutputBuffer(uint32_t index)
{
    CHECK_AND_RETURN_RET_LOG(hasOnRenderOutputBuffer_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "The operation is not permitted without binding OnRenderOutputBuffer!");
    if (isOnNewOutputBuffer_.load()) {
        return OnRenderOutputBuffer(index);
    }
    return ExecuteWhenRunning([this, &index]() {
        return OnRenderOutputBuffer(index);
    }, "RenderOutputBuffer must be called during running!");
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::InitializeInner()
{
    return VideoEnvironmentNative::Get().InitializeByDefault();
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::DeinitializeInner()
{
    return VideoEnvironmentNative::Get().DeinitializeByDefault();
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::RegisterCallback()
{
    return VIDEO_PROCESSING_SUCCESS;
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::SetSurface(const sptr<Surface>& surface,
    [[maybe_unused]] const OHNativeWindow& window)
{
    return SetSurface(surface);
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::SetSurface([[maybe_unused]] const sptr<Surface>& surface)
{
    return VIDEO_PROCESSING_SUCCESS;
}

sptr<Surface> VideoProcessingNativeBase::GetSurface()
{
    return nullptr;
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::SetParameter([[maybe_unused]] const OHOS::Media::Format& parameter)
{
    return VIDEO_PROCESSING_SUCCESS;
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::GetParameter([[maybe_unused]] OHOS::Media::Format& parameter)
{
    return VIDEO_PROCESSING_SUCCESS;
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::OnStart()
{
    return VIDEO_PROCESSING_SUCCESS;
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::OnStop()
{
    return VIDEO_PROCESSING_SUCCESS;
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::OnRenderOutputBuffer([[maybe_unused]] uint32_t index)
{
    return VIDEO_PROCESSING_SUCCESS;
}

void VideoProcessingNativeBase::OnError(VideoProcessing_ErrorCode errorCode)
{
    OnCallback([this, &errorCode](std::shared_ptr<VideoProcessingCallbackNative>& callback, void* userData) {
        callback->OnError(context_, errorCode, userData);
    }, "OnError");
}

void VideoProcessingNativeBase::OnState(VideoProcessing_State state)
{
    OnCallback([this, &state](std::shared_ptr<VideoProcessingCallbackNative>& callback, void* userData) {
        callback->OnState(context_, state, userData);
    }, "OnState");
}

void VideoProcessingNativeBase::OnNewOutputBuffer(uint32_t index)
{
    if (!isInitialized_.load()) {
        VPE_LOGI("Skip index:%{public}u buffer because VPE has been deinitialized.", index);
        return;
    }
    OnCallback([this, &index](std::shared_ptr<VideoProcessingCallbackNative>& callback, void* userData) {
        if (callback->HasOnNewOutputBuffer()) {
            isOnNewOutputBuffer_ = true;
            callback->OnNewOutputBuffer(context_, index, userData);
            isOnNewOutputBuffer_ = false;
        } else {
            OnRenderOutputBuffer(index);
        }
    }, "OnNewOutputBuffer");
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::ExecuteWhenIdle(
    std::function<VideoProcessing_ErrorCode(void)>&& task, const std::string& errLog)
{
    if (isRunning_.load()) {
        VPE_LOGW("%{public}s", errLog.c_str());
        return VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED;
    }

    std::lock_guard<std::mutex> lock(lock_);
    if (isRunning_.load()) {
        VPE_LOGW("%{public}s", errLog.c_str());
        return VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED;
    }
    return task();
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::ExecuteWhenRunning(
    std::function<VideoProcessing_ErrorCode(void)>&& task, const std::string& errLog)
{
    if (!isRunning_.load()) {
        VPE_LOGW("%{public}s", errLog.c_str());
        return VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED;
    }

    std::lock_guard<std::mutex> lock(lock_);
    if (!isRunning_.load()) {
        VPE_LOGW("%{public}s", errLog.c_str());
        return VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED;
    }
    return task();
}

void VideoProcessingNativeBase::OnCallback(
    std::function<void(std::shared_ptr<VideoProcessingCallbackNative>&, void*)>&& task,
    const std::string& name)
{
    std::lock_guard<std::mutex> lock(callbackLock_);
    TraverseCallbacksLocked(std::move(task));
}

void VideoProcessingNativeBase::TraverseCallbacksLocked(
    std::function<void(std::shared_ptr<VideoProcessingCallbackNative>&, void*)>&& task)
{
    TraverseCallbacksExLocked([&task](std::shared_ptr<VideoProcessingCallbackNative>& cb, void* userData) {
        task(cb, userData);
        return false;
    });
}

void VideoProcessingNativeBase::TraverseCallbacksExLocked(
    std::function<bool(std::shared_ptr<VideoProcessingCallbackNative>&, void*)>&& task)
{
    for (auto& callback : callbacks_) {
        std::shared_ptr<VideoProcessingCallbackNative> cb = callback.callback;
        if (cb == nullptr) [[unlikely]] {
            VPE_LOGW("callback is null!");
            continue;
        }
        if (task(cb, callback.userData)) {
            break;
        }
    }
}

VideoProcessing_ErrorCode VideoProcessingNativeBase::PrepareRegistrationLocked()
{
    if (isInnerCallbackReady_) {
        return VIDEO_PROCESSING_SUCCESS;
    }
    VideoProcessing_ErrorCode errorCode = RegisterCallback();
    if (errorCode == VIDEO_PROCESSING_SUCCESS) {
        VPE_LOGD("Inner RegisterCallback success.");
        isInnerCallbackReady_ = true;
    }
    return errorCode;
}
