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

#include "video_processing_callback_impl.h"

#include "vpe_log.h"

using namespace OHOS::Media::VideoProcessingEngine;

VideoProcessing_ErrorCode VideoProcessing_Callback::Create(VideoProcessing_Callback** instance)
{
    CHECK_AND_RETURN_RET_LOG(instance != nullptr, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER, "instance is null!");

    *instance = new(std::nothrow) VideoProcessing_Callback();
    CHECK_AND_RETURN_RET_LOG(*instance != nullptr, VIDEO_PROCESSING_ERROR_NO_MEMORY, "new callback out of memory!");
    return VIDEO_PROCESSING_SUCCESS;
}

VideoProcessing_ErrorCode VideoProcessing_Callback::Destroy(VideoProcessing_Callback* instance)
{
    CHECK_AND_RETURN_RET_LOG(instance != nullptr, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER, "instance is null!");
    auto innerCallback = instance->GetInnerCallback();
    if (innerCallback != nullptr && !innerCallback->IsModifiable()) {
        VPE_LOGW("Callback is still in use, please stop before destory.");
        return VIDEO_PROCESSING_ERROR_INVALID_INSTANCE;
    }
    delete instance;
    return VIDEO_PROCESSING_SUCCESS;
}

VideoProcessing_Callback::VideoProcessing_Callback()
{
    videoProcessingCallback_ = std::make_shared<VideoProcessingCallbackNative>();
}

VideoProcessing_Callback::~VideoProcessing_Callback()
{
    videoProcessingCallback_ = nullptr;
}

std::shared_ptr<VideoProcessingCallbackNative> VideoProcessing_Callback::GetInnerCallback() const
{
    return videoProcessingCallback_;
}
