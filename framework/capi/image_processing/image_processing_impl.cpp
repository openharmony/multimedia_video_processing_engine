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

#include "image_processing_impl.h"

#include "image_processing_factory.h"
#include "vpe_log.h"

using namespace OHOS::Media::VideoProcessingEngine;

ImageProcessing_ErrorCode OH_ImageProcessing::Create(OH_ImageProcessing** instance, int type,
                                                     std::shared_ptr<OpenGLContext> openglContext,
                                                     ClContext *openclContext)
{
    CHECK_AND_RETURN_RET_LOG(instance != nullptr && *instance == nullptr, IMAGE_PROCESSING_ERROR_INVALID_INSTANCE,
        "VPE image processing instance is null or *instance is not null!");
    CHECK_AND_RETURN_RET_LOG(ImageProcessingFactory::IsValid(type), IMAGE_PROCESSING_ERROR_INVALID_PARAMETER,
        "VPE image processing type(%{public}d) is invalid!", type);

    *instance = new(std::nothrow) OH_ImageProcessing(type);
    CHECK_AND_RETURN_RET_LOG(*instance != nullptr, IMAGE_PROCESSING_ERROR_NO_MEMORY,
        "VPE image processing out of memory!");
    auto obj = (*instance)->GetImageProcessing();
    CHECK_AND_RETURN_RET_LOG(obj != nullptr, IMAGE_PROCESSING_ERROR_CREATE_FAILED,
        "VPE image processing constructor failed!");
    obj->opengclContext_ = openclContext;
    obj->openglContext_ = openglContext;
    return obj->Initialize();
}

ImageProcessing_ErrorCode OH_ImageProcessing::Destroy(OH_ImageProcessing* instance)
{
    CHECK_AND_RETURN_RET_LOG(instance != nullptr, IMAGE_PROCESSING_ERROR_INVALID_INSTANCE,
        "VPE image processing instance is null!");
    auto obj = instance->GetImageProcessing();
    CHECK_AND_RETURN_RET_LOG(obj != nullptr, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER,
        "VPE image processing instance is empty!");
    auto errorCode = obj->Deinitialize();
    delete instance;
    instance = nullptr;
    return errorCode;
}

OH_ImageProcessing::OH_ImageProcessing(int type)
{
    imageProcessing_ = ImageProcessingFactory::CreateImageProcessing(type);
}

OH_ImageProcessing::~OH_ImageProcessing()
{
    imageProcessing_ = nullptr;
}

std::shared_ptr<IImageProcessingNative> OH_ImageProcessing::GetImageProcessing()
{
    return imageProcessing_;
}
