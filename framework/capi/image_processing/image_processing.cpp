/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "image_processing.h"

#include <atomic>
#include <functional>

#include "vpe_log.h"
#include "image_processing_capi_capability.h"
#include "image_environment_native.h"
#include "image_processing_impl.h"

using namespace OHOS::Media::VideoProcessingEngine;

const int32_t IMAGE_PROCESSING_TYPE_COLOR_SPACE_CONVERSION = 0x1;
const int32_t IMAGE_PROCESSING_TYPE_COMPOSITION = 0x2;
const int32_t IMAGE_PROCESSING_TYPE_DECOMPOSITION = 0x4;
const int32_t IMAGE_PROCESSING_TYPE_METADATA_GENERATION = 0x8;
const int32_t IMAGE_PROCESSING_TYPE_DETAIL_ENHANCER = 0x10;
const char* IMAGE_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL = "QualityLevel";

namespace {
ImageProcessing_ErrorCode CallImageProcessing(OH_ImageProcessing* imageProcessor,
    std::function<ImageProcessing_ErrorCode(std::shared_ptr<IImageProcessingNative>&)>&& operation)
{
    CHECK_AND_RETURN_RET_LOG(imageProcessor != nullptr, IMAGE_PROCESSING_ERROR_INVALID_INSTANCE,
        "imageProcessor is null!");
    auto imageProcessing = imageProcessor->GetImageProcessing();
    CHECK_AND_RETURN_RET_LOG(imageProcessing != nullptr, IMAGE_PROCESSING_ERROR_INVALID_INSTANCE,
        "imageProcessor is invalid!");
    return operation(imageProcessing);
}
}

ImageProcessing_ErrorCode OH_ImageProcessing_InitializeEnvironment(void)
{
    CHECK_AND_RETURN_RET_LOG(ImageProcessingCapiCapability::Get().OpenCLInit() == IMAGE_PROCESSING_SUCCESS,
        IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING, "OpenCLInit failed!");
    CHECK_AND_RETURN_RET_LOG(ImageProcessingCapiCapability::Get().OpenGLInit() == IMAGE_PROCESSING_SUCCESS,
        IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING, "OpenGLInit failed!");
    return ImageEnvironmentNative::Get().Initialize();
}

ImageProcessing_ErrorCode OH_ImageProcessing_DeinitializeEnvironment(void)
{
    return ImageEnvironmentNative::Get().Deinitialize();
}

bool OH_ImageProcessing_IsColorSpaceConversionSupported(
    const ImageProcessing_ColorSpaceInfo* sourceImageInfo,
    const ImageProcessing_ColorSpaceInfo* destinationImageInfo)
{
    ImageProcessingCapiCapability::Get().LoadLibrary();
    auto flag = ImageProcessingCapiCapability::Get().CheckColorSpaceConversionSupport(sourceImageInfo,
        destinationImageInfo);
    ImageProcessingCapiCapability::Get().UnloadLibrary();
    return flag;
}

bool OH_ImageProcessing_IsCompositionSupported(
    const ImageProcessing_ColorSpaceInfo* sourceImageInfo,
    const ImageProcessing_ColorSpaceInfo* sourceGainmapInfo,
    const ImageProcessing_ColorSpaceInfo* destinationImageInfo)
{
    ImageProcessingCapiCapability::Get().LoadLibrary();
    auto flag = ImageProcessingCapiCapability::Get().CheckCompositionSupport(sourceImageInfo,
        sourceGainmapInfo, destinationImageInfo);
    ImageProcessingCapiCapability::Get().UnloadLibrary();
    return flag;
}

bool OH_ImageProcessing_IsDecompositionSupported(
    const ImageProcessing_ColorSpaceInfo* sourceImageInfo,
    const ImageProcessing_ColorSpaceInfo* destinationImageInfo,
    const ImageProcessing_ColorSpaceInfo* destinationGainmapInfo)
{
    ImageProcessingCapiCapability::Get().LoadLibrary();
    auto flag = ImageProcessingCapiCapability::Get().CheckDecompositionSupport(sourceImageInfo,
        destinationImageInfo, destinationGainmapInfo);
    ImageProcessingCapiCapability::Get().UnloadLibrary();
    return flag;
}

bool OH_ImageProcessing_IsMetadataGenerationSupported(const ImageProcessing_ColorSpaceInfo* sourceImageInfo)
{
    ImageProcessingCapiCapability::Get().LoadLibrary();
    auto flag = ImageProcessingCapiCapability::Get().CheckMetadataGenerationSupport(sourceImageInfo);
    ImageProcessingCapiCapability::Get().UnloadLibrary();
    return flag;
}

ImageProcessing_ErrorCode OH_ImageProcessing_Create(OH_ImageProcessing** imageProcessor, int type)
{
    return OH_ImageProcessing::Create(imageProcessor, type, ImageProcessingCapiCapability::Get().GetOpenGLContext(),
        ImageProcessingCapiCapability::Get().GetClContext());
}

ImageProcessing_ErrorCode OH_ImageProcessing_Destroy(OH_ImageProcessing* imageProcessor)
{
    return OH_ImageProcessing::Destroy(imageProcessor);
}

ImageProcessing_ErrorCode OH_ImageProcessing_SetParameter(OH_ImageProcessing* imageProcessor,
    const OH_AVFormat* parameter)
{
    return CallImageProcessing(imageProcessor, [&parameter](std::shared_ptr<IImageProcessingNative>& obj) {
        return obj->SetParameter(parameter);
    });
}

ImageProcessing_ErrorCode OH_ImageProcessing_GetParameter(OH_ImageProcessing* imageProcessor, OH_AVFormat* parameter)
{
    return CallImageProcessing(imageProcessor, [&parameter](std::shared_ptr<IImageProcessingNative>& obj) {
        return obj->GetParameter(parameter);
    });
}

ImageProcessing_ErrorCode OH_ImageProcessing_ConvertColorSpace(OH_ImageProcessing* imageProcessor,
    OH_PixelmapNative* sourceImage, OH_PixelmapNative* destinationImage)
{
    return CallImageProcessing(imageProcessor, [&sourceImage, &destinationImage](
        std::shared_ptr<IImageProcessingNative>& obj) {
        return obj->ConvertColorSpace(sourceImage, destinationImage);
    });
}

ImageProcessing_ErrorCode OH_ImageProcessing_Compose(OH_ImageProcessing* imageProcessor,
    OH_PixelmapNative* sourceImage, OH_PixelmapNative* sourceGainmap, OH_PixelmapNative* destinationImage)
{
    return CallImageProcessing(imageProcessor, [&sourceImage, &sourceGainmap, &destinationImage](
        std::shared_ptr<IImageProcessingNative>& obj) {
        return obj->Compose(sourceImage, sourceGainmap, destinationImage);
    });
}

ImageProcessing_ErrorCode OH_ImageProcessing_Decompose(OH_ImageProcessing* imageProcessor,
    OH_PixelmapNative* sourceImage, OH_PixelmapNative* destinationImage, OH_PixelmapNative* destinationGainmap)
{
    return CallImageProcessing(imageProcessor, [&sourceImage, &destinationImage, &destinationGainmap](
        std::shared_ptr<IImageProcessingNative>& obj) {
        return obj->Decompose(sourceImage, destinationImage, destinationGainmap);
    });
}

ImageProcessing_ErrorCode OH_ImageProcessing_GenerateMetadata(OH_ImageProcessing* imageProcessor,
    OH_PixelmapNative* sourceImage)
{
    return CallImageProcessing(imageProcessor, [&sourceImage](
        std::shared_ptr<IImageProcessingNative>& obj) {
        return obj->GenerateMetadata(sourceImage);
    });
}

ImageProcessing_ErrorCode OH_ImageProcessing_EnhanceDetail(OH_ImageProcessing* imageProcessor,
    OH_PixelmapNative* sourceImage, OH_PixelmapNative* destinationImage)
{
    return CallImageProcessing(imageProcessor, [&sourceImage, &destinationImage](
        std::shared_ptr<IImageProcessingNative>& obj) {
        return obj->Process(sourceImage, destinationImage);
    });
}
