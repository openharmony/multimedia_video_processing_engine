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

#include "image_processing_native_base.h"

#include "common/native_mfmagic.h"
#include "pixelmap_native_impl.h"

#include "image_environment_native.h"
#include "vpe_log.h"

using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;

ImageProcessingNativeBase::ImageProcessingNativeBase()
{
}

ImageProcessing_ErrorCode ImageProcessingNativeBase::Initialize()
{
    CHECK_AND_RETURN_RET_LOG(!isInitialized_.load(), IMAGE_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already init!");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(!isInitialized_.load(), IMAGE_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already init!");
    auto result = InitializeInner();
    isInitialized_ = true;
    return result;
}

ImageProcessing_ErrorCode ImageProcessingNativeBase::Deinitialize()
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), IMAGE_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already deinit!");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), IMAGE_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already deinit!");
    auto result = DeinitializeInner();
    isInitialized_ = false;
    return result;
}

ImageProcessing_ErrorCode ImageProcessingNativeBase::SetParameter(const OH_AVFormat* parameter)
{
    CHECK_AND_RETURN_RET_LOG(parameter != nullptr, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER, "parameter is null!");
    return SetParameter(parameter->format_);
}

ImageProcessing_ErrorCode ImageProcessingNativeBase::GetParameter(OH_AVFormat* parameter)
{
    CHECK_AND_RETURN_RET_LOG(parameter != nullptr, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER, "parameter is null!");
    return GetParameter(parameter->format_);
}

ImageProcessing_ErrorCode ImageProcessingNativeBase::Process(OH_PixelmapNative* sourceImage,
    OH_PixelmapNative* destinationImage)
{
    CHECK_AND_RETURN_RET_LOG(sourceImage != nullptr && destinationImage != nullptr,
        IMAGE_PROCESSING_ERROR_INVALID_PARAMETER, "parameter is null!");
    auto destPixelmap = destinationImage->GetInnerPixelmap();
    auto sourePixelmap = sourceImage->GetInnerPixelmap();
    return Process(sourePixelmap, destPixelmap);
}

ImageProcessing_ErrorCode ImageProcessingNativeBase::ConvertColorSpace(OH_PixelmapNative* sourceImage,
    OH_PixelmapNative* destinationImage)
{
    CHECK_AND_RETURN_RET_LOG(sourceImage != nullptr && destinationImage != nullptr,
        IMAGE_PROCESSING_ERROR_INVALID_PARAMETER, "parameter is null!");
    auto destPixelmap = destinationImage->GetInnerPixelmap();
    return ConvertColorSpace(sourceImage->GetInnerPixelmap(), destPixelmap);
}

ImageProcessing_ErrorCode ImageProcessingNativeBase::Compose(OH_PixelmapNative* sourceImage,
    OH_PixelmapNative* sourceGainmap, OH_PixelmapNative* destinationImage)
{
    CHECK_AND_RETURN_RET_LOG(sourceImage != nullptr && destinationImage != nullptr && sourceGainmap != nullptr,
        IMAGE_PROCESSING_ERROR_INVALID_PARAMETER, "parameter is null!");
    auto destPixelmap = destinationImage->GetInnerPixelmap();
    return Compose(sourceImage->GetInnerPixelmap(), sourceGainmap->GetInnerPixelmap(), destPixelmap);
}

ImageProcessing_ErrorCode ImageProcessingNativeBase::Decompose(OH_PixelmapNative* sourceImage,
    OH_PixelmapNative* destinationImage, OH_PixelmapNative* destinationGainmap)
{
    CHECK_AND_RETURN_RET_LOG(sourceImage != nullptr && destinationImage != nullptr && destinationGainmap != nullptr,
        IMAGE_PROCESSING_ERROR_INVALID_PARAMETER, "parameter is null!");
    auto destPixelmap = destinationImage->GetInnerPixelmap();
    auto destGainmapPixelmap = destinationGainmap->GetInnerPixelmap();
    return Decompose(sourceImage->GetInnerPixelmap(), destPixelmap, destGainmapPixelmap);
}

ImageProcessing_ErrorCode ImageProcessingNativeBase::GenerateMetadata(OH_PixelmapNative* sourceImage)
{
    CHECK_AND_RETURN_RET_LOG(sourceImage != nullptr,
        IMAGE_PROCESSING_ERROR_INVALID_PARAMETER, "parameter is null!");
    return GenerateMetadata(sourceImage->GetInnerPixelmap());
}

ImageProcessing_ErrorCode ImageProcessingNativeBase::EnhanceDetail(OH_PixelmapNative* sourceImage,
    OH_PixelmapNative* destinationImage)
{
    CHECK_AND_RETURN_RET_LOG(sourceImage != nullptr && destinationImage != nullptr,
        IMAGE_PROCESSING_ERROR_INVALID_PARAMETER, "parameter is null!");
    auto destPixelmap = destinationImage->GetInnerPixelmap();
    return EnhanceDetail(sourceImage->GetInnerPixelmap(), destPixelmap);
}

ImageProcessing_ErrorCode ImageProcessingNativeBase::InitializeInner()
{
    return ImageEnvironmentNative::Get().InitializeByDefault();
}

ImageProcessing_ErrorCode ImageProcessingNativeBase::DeinitializeInner()
{
    return ImageEnvironmentNative::Get().DeinitializeByDefault();
}

ImageProcessing_ErrorCode ImageProcessingNativeBase::SetParameter([[maybe_unused]] const OHOS::Media::Format& parameter)
{
    return IMAGE_PROCESSING_SUCCESS;
}

ImageProcessing_ErrorCode ImageProcessingNativeBase::GetParameter([[maybe_unused]] OHOS::Media::Format& parameter)
{
    return IMAGE_PROCESSING_SUCCESS;
}

ImageProcessing_ErrorCode ImageProcessingNativeBase::Process(
    [[maybe_unused]] const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
    [[maybe_unused]] std::shared_ptr<OHOS::Media::PixelMap>& destinationImage)
{
    return IMAGE_PROCESSING_SUCCESS;
}

ImageProcessing_ErrorCode ImageProcessingNativeBase::ConvertColorSpace(
    [[maybe_unused]] const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
    [[maybe_unused]] std::shared_ptr<OHOS::Media::PixelMap>& destinationImage)
{
    return IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING;
}

ImageProcessing_ErrorCode ImageProcessingNativeBase::Compose(
    [[maybe_unused]] const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
    [[maybe_unused]] const std::shared_ptr<OHOS::Media::PixelMap>& sourceGainmap,
    [[maybe_unused]] std::shared_ptr<OHOS::Media::PixelMap>& destinationImage)
{
    return IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING;
}

ImageProcessing_ErrorCode ImageProcessingNativeBase::Decompose(
    [[maybe_unused]] const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
    [[maybe_unused]] std::shared_ptr<OHOS::Media::PixelMap>& destinationImage,
    [[maybe_unused]] std::shared_ptr<OHOS::Media::PixelMap>& destinationGainmap)
{
    return IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING;
}

ImageProcessing_ErrorCode ImageProcessingNativeBase::GenerateMetadata(
    [[maybe_unused]] const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage)
{
    return IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING;
}

ImageProcessing_ErrorCode ImageProcessingNativeBase::EnhanceDetail(
    [[maybe_unused]] const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
    [[maybe_unused]] std::shared_ptr<OHOS::Media::PixelMap>& destinationImage)
{
    return IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING;
}
