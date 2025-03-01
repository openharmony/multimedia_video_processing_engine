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

#include "video_environment_native.h"

#include "vpe_log.h"
#include "video_processing_client.h"
#include "video_processing_utils.h"

using namespace OHOS::Media::VideoProcessingEngine;

VideoEnvironmentNative& VideoEnvironmentNative::Get()
{
    static VideoEnvironmentNative instance{};
    return instance;
}

VideoProcessing_ErrorCode VideoEnvironmentNative::Initialize()
{
    std::lock_guard<std::mutex> lock(lock_);
    if (isExplicitInit_) [[unlikely]] {
        VPE_LOGE("Repeated initialization of the video environment!");
        return VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED;
    }
    isExplicitInit_ = true;
    return InitializeLocked();
}

VideoProcessing_ErrorCode VideoEnvironmentNative::Deinitialize()
{
    std::lock_guard<std::mutex> lock(lock_);
    if (!isExplicitInit_) [[unlikely]] {
        VPE_LOGE("Repeated deinitialization of the video environment!");
        return VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED;
    }
    isExplicitInit_ = false;
    return DeinitializeLocked();
}

VideoProcessing_ErrorCode VideoEnvironmentNative::InitializeByDefault()
{
    std::lock_guard<std::mutex> lock(lock_);
    return InitializeLocked();
}

VideoProcessing_ErrorCode VideoEnvironmentNative::DeinitializeByDefault()
{
    std::lock_guard<std::mutex> lock(lock_);
    return DeinitializeLocked();
}

VideoProcessing_ErrorCode VideoEnvironmentNative::InitializeLocked()
{
    if (referenceCount_ > 0) [[likely]] {
        VPE_LOGD("already init(cnt:%{public}d)", referenceCount_);
        referenceCount_++;
        return VIDEO_PROCESSING_SUCCESS;
    }
    VPE_LOGD("start to initialize...");
    VideoProcessing_ErrorCode result = InitializeEnvLocked();
    VPE_LOGD("initialize ret:%{public}s", VideoProcessingUtils::ToString(result).c_str());
    if (result == VIDEO_PROCESSING_SUCCESS) [[likely]] {
        referenceCount_++;
    }
    return result;
}

VideoProcessing_ErrorCode VideoEnvironmentNative::DeinitializeLocked()
{
    if (referenceCount_ > 1) [[likely]] {
        VPE_LOGD("environment is still in use(cnt:%{public}d)", referenceCount_);
        referenceCount_--;
        return VIDEO_PROCESSING_SUCCESS;
    }
    VPE_LOGD("start to deinitialize...");
    VideoProcessing_ErrorCode result = DeinitializeEnvLocked();
    VPE_LOGD("deinitialize ret:%{public}s", VideoProcessingUtils::ToString(result).c_str());
    if (result == VIDEO_PROCESSING_SUCCESS) [[likely]] {
        if (referenceCount_ <= 0) {
            VPE_LOGE("referenceCount_ is less than 1, no need to deinitialize");
            return VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED;
        }
        referenceCount_--;
    }
    return result;
}

VideoProcessing_ErrorCode VideoEnvironmentNative::InitializeEnvLocked()
{
    OHOS::Media::VideoProcessingEngine::VideoProcessingManager::GetInstance().Connect();
    return VIDEO_PROCESSING_SUCCESS;
}

VideoProcessing_ErrorCode VideoEnvironmentNative::DeinitializeEnvLocked()
{
    OHOS::Media::VideoProcessingEngine::VideoProcessingManager::GetInstance().Disconnect();
    return VIDEO_PROCESSING_SUCCESS;
}
