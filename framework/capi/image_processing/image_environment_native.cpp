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

#include "image_environment_native.h"

#include "image_processing_utils.h"
#include "video_processing_client.h"
#include "vpe_log.h"

using namespace OHOS::Media::VideoProcessingEngine;

ImageEnvironmentNative& ImageEnvironmentNative::Get()
{
    static ImageEnvironmentNative instance{};
    return instance;
}

ImageProcessing_ErrorCode ImageEnvironmentNative::Initialize()
{
    std::lock_guard<std::mutex> lock(lock_);
    if (isExplicitInit_) [[unlikely]] {
        VPE_LOGE("Repeated initialization of the image environment!");
        return IMAGE_PROCESSING_ERROR_OPERATION_NOT_PERMITTED;
    }
    isExplicitInit_ = true;
    return InitializeLocked();
}

ImageProcessing_ErrorCode ImageEnvironmentNative::Deinitialize()
{
    std::lock_guard<std::mutex> lock(lock_);
    if (!isExplicitInit_) [[unlikely]] {
        VPE_LOGE("Repeated deinitialization of the image environment!");
        return IMAGE_PROCESSING_ERROR_OPERATION_NOT_PERMITTED;
    }
    isExplicitInit_ = false;
    return DeinitializeLocked();
}

ImageProcessing_ErrorCode ImageEnvironmentNative::InitializeByDefault()
{
    std::lock_guard<std::mutex> lock(lock_);
    return InitializeLocked();
}

ImageProcessing_ErrorCode ImageEnvironmentNative::DeinitializeByDefault()
{
    std::lock_guard<std::mutex> lock(lock_);
    return DeinitializeLocked();
}

ImageProcessing_ErrorCode ImageEnvironmentNative::InitializeLocked()
{
    if (referenceCount_ > 0) [[likely]] {
        VPE_LOGD("already init(cnt:%{public}d)", referenceCount_);
        referenceCount_++;
        return IMAGE_PROCESSING_SUCCESS;
    }
    VPE_LOGD("start to initialize...");
    ImageProcessing_ErrorCode result = InitializeEnvLocked();
    VPE_LOGD("initialize ret:%{public}s", ImageProcessingUtils::ToString(result).c_str());
    if (result == IMAGE_PROCESSING_SUCCESS) [[likely]] {
        referenceCount_++;
    }
    return result;
}

ImageProcessing_ErrorCode ImageEnvironmentNative::DeinitializeLocked()
{
    if (referenceCount_ > 1) [[likely]] {
        VPE_LOGD("environment is still in use(cnt:%{public}d)", referenceCount_);
        referenceCount_--;
        return IMAGE_PROCESSING_SUCCESS;
    }
    VPE_LOGD("start to deinitialize...");
    ImageProcessing_ErrorCode result = DeinitializeEnvLocked();
    VPE_LOGD("deinitialize ret:%{public}s", ImageProcessingUtils::ToString(result).c_str());
    if (result == IMAGE_PROCESSING_SUCCESS) [[likely]] {
        if (referenceCount_ <= 0) {
            VPE_LOGE("referenceCount_ is less than 1, no need to deinitialize");
            return IMAGE_PROCESSING_ERROR_OPERATION_NOT_PERMITTED;
        }
        referenceCount_--;
    }
    return result;
}

ImageProcessing_ErrorCode ImageEnvironmentNative::InitializeEnvLocked()
{
    OHOS::Media::VideoProcessingEngine::VideoProcessingManager::GetInstance().Connect();
    return IMAGE_PROCESSING_SUCCESS;
}

ImageProcessing_ErrorCode ImageEnvironmentNative::DeinitializeEnvLocked()
{
    OHOS::Media::VideoProcessingEngine::VideoProcessingManager::GetInstance().Disconnect();
    return IMAGE_PROCESSING_SUCCESS;
}
