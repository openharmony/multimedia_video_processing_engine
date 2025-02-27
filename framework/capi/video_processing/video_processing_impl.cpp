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

#include "video_processing_impl.h"

#include "video_processing_factory.h"
#include "vpe_log.h"

using namespace OHOS::Media::VideoProcessingEngine;

VideoProcessing_ErrorCode OH_VideoProcessing::Create(OH_VideoProcessing** instance, int type,
                                                     std::shared_ptr<OpenGLContext> openglContext)
{
    CHECK_AND_RETURN_RET_LOG(instance != nullptr && *instance == nullptr, VIDEO_PROCESSING_ERROR_INVALID_INSTANCE,
        "VPE video processing instance is null or *instance is not null!");
    CHECK_AND_RETURN_RET_LOG(VideoProcessingFactory::IsValid(type), VIDEO_PROCESSING_ERROR_INVALID_PARAMETER,
        "VPE video processing type(%{public}d) is invalid!", type);

    *instance = new(std::nothrow) OH_VideoProcessing(type);
    CHECK_AND_RETURN_RET_LOG(*instance != nullptr, VIDEO_PROCESSING_ERROR_NO_MEMORY,
        "VPE video processing out of memory!");
    auto obj = (*instance)->GetVideoProcessing();
    CHECK_AND_RETURN_RET_LOG(obj != nullptr, VIDEO_PROCESSING_ERROR_CREATE_FAILED,
        "VPE video processing constructor failed!");
    obj->openglContext_ = openglContext;
    return obj->Initialize();
}

VideoProcessing_ErrorCode OH_VideoProcessing::Destroy(OH_VideoProcessing* instance)
{
    CHECK_AND_RETURN_RET_LOG(instance != nullptr, VIDEO_PROCESSING_ERROR_INVALID_INSTANCE,
        "VPE video processing instance is null!");
    auto obj = instance->GetVideoProcessing();
    CHECK_AND_RETURN_RET_LOG(obj != nullptr, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER,
        "VPE video processing instance is empty!");
    auto errorCode = obj->Deinitialize();
    delete instance;
    instance = nullptr;
    return errorCode;
}

OH_VideoProcessing::OH_VideoProcessing(int type)
{
    videoProcessing_ = VideoProcessingFactory::CreateVideoProcessing(type, this);
}

OH_VideoProcessing::~OH_VideoProcessing()
{
    videoProcessing_ = nullptr;
}

std::shared_ptr<IVideoProcessingNative> OH_VideoProcessing::GetVideoProcessing()
{
    return videoProcessing_;
}
