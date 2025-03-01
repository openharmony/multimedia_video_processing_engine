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

#include "image_processing_capi_impl.h"

#include "vpe_log.h"

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

ImageProcessing_ErrorCode ImageProcessingCapiImpl::OpenCLInit()
{
    void *OpenclFoundationHandle = nullptr;
    std::string path = "/sys_prod/lib64/VideoProcessingEngine/libaihdr_engine.so";
    auto ret = access(path.c_str(), F_OK);
    if (ret != 0) {
        VPE_LOGW("access = %d path = %s", ret, path.c_str());
    } else {
        constexpr int DEVICE_NAME_LENGTH = 32; // 32 max name length
        char deviceName[DEVICE_NAME_LENGTH];
        auto status = SetupOpencl(&OpenclFoundationHandle, "HUA", deviceName);
        CHECK_AND_RETURN_RET_LOG(status == static_cast<int>(IMAGE_PROCESSING_SUCCESS),
                                 IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING,
                                 "GetOpenCLContext SetupOpencl fail!");
    }
    openclContext_ = reinterpret_cast<ClContext *>(OpenclFoundationHandle);
    return IMAGE_PROCESSING_SUCCESS;
}

ImageProcessing_ErrorCode ImageProcessingCapiImpl::OpenGLInit()
{
    auto status = SetupOpengl(openglContext_);
    CHECK_AND_RETURN_RET_LOG(status == static_cast<int>(IMAGE_PROCESSING_SUCCESS),
                             IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING,
                             "OpenGLInit SetupOpengl fail!");
    return IMAGE_PROCESSING_SUCCESS;
}

ImageProcessing_ErrorCode ImageProcessingCapiImpl::InitializeEnvironment()
{
    CHECK_AND_RETURN_RET_LOG(OpenCLInit() == IMAGE_PROCESSING_SUCCESS, IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING,
                             "OpenCLInit failed!");
    CHECK_AND_RETURN_RET_LOG(OpenGLInit() == IMAGE_PROCESSING_SUCCESS, IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING,
                             "OpenGLInit failed!");

    return ImageEnvironmentNative::Get().Initialize();
}

ImageProcessing_ErrorCode ImageProcessingCapiImpl::DeinitializeEnvironment()
{
    return ImageEnvironmentNative::Get().Deinitialize();
}

void ImageProcessingCapiImpl::LoadLibrary()
{
    std::lock_guard<std::mutex> lock(lock_);
    if (usedInstance_ == 0 && mLibHandle == nullptr) {
        std::string path = "libvideoprocessingengine.z.so";
        mLibHandle = dlopen(path.c_str(), RTLD_NOW);
    }
    usedInstance_++;
}
void ImageProcessingCapiImpl::UnloadLibrary()
{
    std::lock_guard<std::mutex> lock(lock_);
    usedInstance_--;
    if ((usedInstance_ == 0) && (mLibHandle != nullptr)) {
        dlclose(mLibHandle);
        mLibHandle = nullptr;
    }
}

ImageProcessing_ErrorCode ImageProcessingCapiImpl::LoadAlgo()
{
    CHECK_AND_RETURN_RET_LOG(mLibHandle != nullptr, IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING,
        "Library is nullptr!");
    std::pair<std::string, LibFunction&> funcs[] = {
        { "ImageProcessing_IsColorSpaceConversionSupported", isColorSpaceConversionSupported_},
        { "ImageProcessing_IsCompositionSupported", isCompositionSupported_ },
        { "ImageProcessing_IsDecompositionSupported", isDecompositionSupported_ },
    };
    for (auto& func : funcs) {
        func.second = reinterpret_cast<LibFunction>(dlsym(mLibHandle, func.first.c_str()));
        CHECK_AND_RETURN_RET_LOG(func.second != nullptr, IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING,
            "Failed to locate %s in - %s", func.first.c_str(), dlerror());
    }
    isMetadataGenSupported_ = reinterpret_cast<LibMetaFunction>(dlsym(mLibHandle,
        "ImageProcessing_IsMetadataGenerationSupported"));
    CHECK_AND_RETURN_RET_LOG(isMetadataGenSupported_ != nullptr, IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING,
        "Failed to locate %s in - %s", "ImageProcessing_IsMetadataGenerationSupported",
        dlerror());
    return IMAGE_PROCESSING_SUCCESS;
}

bool ImageProcessingCapiImpl::CheckColorSpaceConversionSupport(
    const ImageProcessing_ColorSpaceInfo* sourceImageInfo,
    const ImageProcessing_ColorSpaceInfo* destinationImageInfo)
{
    CHECK_AND_RETURN_RET_LOG(sourceImageInfo != nullptr, false, "sourceImageInfo is nullptr!");
    CHECK_AND_RETURN_RET_LOG(destinationImageInfo != nullptr, false, "destinationImageInfo is nullptr!");
    auto status = LoadAlgo();
    CHECK_AND_RETURN_RET_LOG(status == IMAGE_PROCESSING_SUCCESS, false, "LoadAlgo faild");
    OHOS::Media::VideoProcessingEngine::ColorSpaceInfo inputInfo;
    OHOS::Media::VideoProcessingEngine::ColorSpaceInfo outputInfo;
    auto iterPixelFormat = IMAGE_FORMAT_MAP.find(static_cast<OHOS::Media::PixelFormat>(sourceImageInfo->pixelFormat));
    CHECK_AND_RETURN_RET_LOG(iterPixelFormat != IMAGE_FORMAT_MAP.end(), false, "src IMAGE_FORMAT_MAP find failed!");
    inputInfo.pixelFormat = iterPixelFormat->second;
    iterPixelFormat = IMAGE_FORMAT_MAP.find(static_cast<OHOS::Media::PixelFormat>(destinationImageInfo->pixelFormat));
    CHECK_AND_RETURN_RET_LOG(iterPixelFormat != IMAGE_FORMAT_MAP.end(), false, "dst IMAGE_FORMAT_MAP find failed!");
    outputInfo.pixelFormat = iterPixelFormat->second;
    auto iterMetadataType = HDR_METADATA_TYPE_MAP.find(static_cast<ImagePixelmapHdrMetadataType>(
        sourceImageInfo->metadataType));
    CHECK_AND_RETURN_RET_LOG(iterMetadataType != HDR_METADATA_TYPE_MAP.end(), false,
        "src HDR_METADATA_TYPE_MAP find failed!");
    inputInfo.metadataType = iterMetadataType->second;
    iterMetadataType = HDR_METADATA_TYPE_MAP.find(static_cast<ImagePixelmapHdrMetadataType>(
        destinationImageInfo->metadataType));
    CHECK_AND_RETURN_RET_LOG(iterMetadataType != HDR_METADATA_TYPE_MAP.end(), false,
        "dst HDR_METADATA_TYPE_MAP find failed!");
    outputInfo.metadataType = iterMetadataType->second;
    auto iterColorSpace = COLORSPACE_MAP.find(static_cast<ImagePixelmapColorspace>(sourceImageInfo->colorSpace));
    CHECK_AND_RETURN_RET_LOG(iterColorSpace != COLORSPACE_MAP.end(), false, "dst colorSpace find failed!");
    inputInfo.colorSpace = iterColorSpace->second;
    iterColorSpace = COLORSPACE_MAP.find(static_cast<ImagePixelmapColorspace>(destinationImageInfo->colorSpace));
    CHECK_AND_RETURN_RET_LOG(iterColorSpace != COLORSPACE_MAP.end(), false, "dst colorSpace find failed!");
    outputInfo.colorSpace = iterColorSpace->second;
    return isColorSpaceConversionSupported_(inputInfo, outputInfo);
}

bool ImageProcessingCapiImpl::CheckCompositionSupport(
    const ImageProcessing_ColorSpaceInfo* sourceImageInfo,
    const ImageProcessing_ColorSpaceInfo* sourceGainmapInfo,
    const ImageProcessing_ColorSpaceInfo* destinationImageInfo)
{
    CHECK_AND_RETURN_RET_LOG(sourceImageInfo != nullptr, false, "sourceImageInfo is nullptr!");
    CHECK_AND_RETURN_RET_LOG(destinationImageInfo != nullptr, false, "destinationImageInfo is nullptr!");
    CHECK_AND_RETURN_RET_LOG(sourceGainmapInfo != nullptr, false, "sourceGainmapInfo is nullptr!");
    auto status = LoadAlgo();
    CHECK_AND_RETURN_RET_LOG(status == IMAGE_PROCESSING_SUCCESS, false, "LoadAlgo faild");
    OHOS::Media::VideoProcessingEngine::ColorSpaceInfo inputInfo;
    OHOS::Media::VideoProcessingEngine::ColorSpaceInfo outputInfo;
    auto iterPixelFormat = IMAGE_FORMAT_MAP.find(static_cast<OHOS::Media::PixelFormat>(sourceImageInfo->pixelFormat));
    CHECK_AND_RETURN_RET_LOG(iterPixelFormat != IMAGE_FORMAT_MAP.end(), false, "src IMAGE_FORMAT_MAP find failed!");
    inputInfo.pixelFormat = iterPixelFormat->second;
    iterPixelFormat = IMAGE_FORMAT_MAP.find(static_cast<OHOS::Media::PixelFormat>(destinationImageInfo->pixelFormat));
    CHECK_AND_RETURN_RET_LOG(iterPixelFormat != IMAGE_FORMAT_MAP.end(), false, "dst IMAGE_FORMAT_MAP find failed!");
    outputInfo.pixelFormat = iterPixelFormat->second;
    auto iterMetadataType = HDR_METADATA_TYPE_MAP.find(static_cast<ImagePixelmapHdrMetadataType>(
        sourceImageInfo->metadataType));
    CHECK_AND_RETURN_RET_LOG(iterMetadataType != HDR_METADATA_TYPE_MAP.end(), false,
        "src HDR_METADATA_TYPE_MAP find failed!");
    inputInfo.metadataType = iterMetadataType->second;
    iterMetadataType = HDR_METADATA_TYPE_MAP.find(static_cast<ImagePixelmapHdrMetadataType>(
        destinationImageInfo->metadataType));
    CHECK_AND_RETURN_RET_LOG(iterMetadataType != HDR_METADATA_TYPE_MAP.end(), false,
        "dst HDR_METADATA_TYPE_MAP find failed!");
    outputInfo.metadataType = iterMetadataType->second;
    auto iterColorSpace = COLORSPACE_MAP.find(static_cast<ImagePixelmapColorspace>(sourceImageInfo->colorSpace));
    CHECK_AND_RETURN_RET_LOG(iterColorSpace != COLORSPACE_MAP.end(), false, "dst colorSpace find failed!");
    inputInfo.colorSpace = iterColorSpace->second;
    iterColorSpace = COLORSPACE_MAP.find(static_cast<ImagePixelmapColorspace>(destinationImageInfo->colorSpace));
    CHECK_AND_RETURN_RET_LOG(iterColorSpace != COLORSPACE_MAP.end(), false, "dst colorSpace find failed!");
    outputInfo.colorSpace = iterColorSpace->second;
    return isCompositionSupported_(inputInfo, outputInfo);
}

bool ImageProcessingCapiImpl::CheckDecompositionSupport(
    const ImageProcessing_ColorSpaceInfo* sourceImageInfo,
    const ImageProcessing_ColorSpaceInfo* destinationImageInfo,
    const ImageProcessing_ColorSpaceInfo* destinationGainmapInfo)
{
    CHECK_AND_RETURN_RET_LOG(sourceImageInfo != nullptr, false, "sourceImageInfo is nullptr!");
    CHECK_AND_RETURN_RET_LOG(destinationImageInfo != nullptr, false, "destinationImageInfo is nullptr!");
    CHECK_AND_RETURN_RET_LOG(destinationGainmapInfo != nullptr, false, "destinationGainmapInfo is nullptr!");
    auto status = LoadAlgo();
    CHECK_AND_RETURN_RET_LOG(status == IMAGE_PROCESSING_SUCCESS, false, "LoadAlgo faild");
    OHOS::Media::VideoProcessingEngine::ColorSpaceInfo inputInfo;
    OHOS::Media::VideoProcessingEngine::ColorSpaceInfo outputInfo;
    auto iterPixelFormat = IMAGE_FORMAT_MAP.find(static_cast<OHOS::Media::PixelFormat>(sourceImageInfo->pixelFormat));
    CHECK_AND_RETURN_RET_LOG(iterPixelFormat != IMAGE_FORMAT_MAP.end(), false, "src IMAGE_FORMAT_MAP find failed!");
    inputInfo.pixelFormat = iterPixelFormat->second;
    iterPixelFormat = IMAGE_FORMAT_MAP.find(static_cast<OHOS::Media::PixelFormat>(destinationImageInfo->pixelFormat));
    CHECK_AND_RETURN_RET_LOG(iterPixelFormat != IMAGE_FORMAT_MAP.end(), false, "dst IMAGE_FORMAT_MAP find failed!");
    outputInfo.pixelFormat = iterPixelFormat->second;
    auto iterMetadataType = HDR_METADATA_TYPE_MAP.find(static_cast<ImagePixelmapHdrMetadataType>(
        sourceImageInfo->metadataType));
    CHECK_AND_RETURN_RET_LOG(iterMetadataType != HDR_METADATA_TYPE_MAP.end(), false,
        "src HDR_METADATA_TYPE_MAP find failed!");
    inputInfo.metadataType = iterMetadataType->second;
    iterMetadataType = HDR_METADATA_TYPE_MAP.find(static_cast<ImagePixelmapHdrMetadataType>(
        destinationImageInfo->metadataType));
    CHECK_AND_RETURN_RET_LOG(iterMetadataType != HDR_METADATA_TYPE_MAP.end(), false,
        "dst HDR_METADATA_TYPE_MAP find failed!");
    outputInfo.metadataType = iterMetadataType->second;
    auto iterColorSpace = COLORSPACE_MAP.find(static_cast<ImagePixelmapColorspace>(sourceImageInfo->colorSpace));
    CHECK_AND_RETURN_RET_LOG(iterColorSpace != COLORSPACE_MAP.end(), false, "src colorSpace find failed!");
    inputInfo.colorSpace = iterColorSpace->second;
    iterColorSpace = COLORSPACE_MAP.find(static_cast<ImagePixelmapColorspace>(destinationImageInfo->colorSpace));
    CHECK_AND_RETURN_RET_LOG(iterColorSpace != COLORSPACE_MAP.end(), false, "dst colorSpace find failed!");
    outputInfo.colorSpace = iterColorSpace->second;
    return isDecompositionSupported_(inputInfo, outputInfo);
}

bool ImageProcessingCapiImpl::CheckMetadataGenerationSupport(const ImageProcessing_ColorSpaceInfo* sourceImageInfo)
{
    CHECK_AND_RETURN_RET_LOG(sourceImageInfo != nullptr, false, "sourceImageInfo is nullptr!");
    auto status = LoadAlgo();
    CHECK_AND_RETURN_RET_LOG(status == IMAGE_PROCESSING_SUCCESS, false, "LoadAlgo faild");
    OHOS::Media::VideoProcessingEngine::ColorSpaceInfo inputInfo;
    auto iterPixelFormat = IMAGE_FORMAT_MAP.find(static_cast<OHOS::Media::PixelFormat>(sourceImageInfo->pixelFormat));
    CHECK_AND_RETURN_RET_LOG(iterPixelFormat != IMAGE_FORMAT_MAP.end(), false, "src IMAGE_FORMAT_MAP find failed!");
    inputInfo.pixelFormat = iterPixelFormat->second;

    auto iterMetadataType = HDR_METADATA_TYPE_MAP.find(static_cast<ImagePixelmapHdrMetadataType>(
        sourceImageInfo->metadataType));
    CHECK_AND_RETURN_RET_LOG(iterMetadataType != HDR_METADATA_TYPE_MAP.end(), false,
        "src HDR_METADATA_TYPE_MAP find failed!");
    inputInfo.metadataType = iterMetadataType->second;

    auto iterColorSpace = COLORSPACE_MAP.find(static_cast<ImagePixelmapColorspace>(sourceImageInfo->colorSpace));
    CHECK_AND_RETURN_RET_LOG(iterColorSpace != COLORSPACE_MAP.end(), false, "dst colorSpace find failed!");
    inputInfo.colorSpace = iterColorSpace->second;
    return isMetadataGenSupported_(inputInfo);
}

bool ImageProcessingCapiImpl::IsColorSpaceConversionSupported(
    const ImageProcessing_ColorSpaceInfo* sourceImageInfo,
    const ImageProcessing_ColorSpaceInfo* destinationImageInfo)
{
    LoadLibrary();
    auto flag = CheckColorSpaceConversionSupport(sourceImageInfo, destinationImageInfo);
    UnloadLibrary();
    return flag;
}

bool ImageProcessingCapiImpl::IsCompositionSupported(
    const ImageProcessing_ColorSpaceInfo* sourceImageInfo,
    const ImageProcessing_ColorSpaceInfo* sourceGainmapInfo,
    const ImageProcessing_ColorSpaceInfo* destinationImageInfo)
{
    LoadLibrary();
    auto flag = CheckCompositionSupport(sourceImageInfo, sourceGainmapInfo, destinationImageInfo);
    UnloadLibrary();
    return flag;
}

bool ImageProcessingCapiImpl::IsDecompositionSupported(
    const ImageProcessing_ColorSpaceInfo* sourceImageInfo,
    const ImageProcessing_ColorSpaceInfo* destinationImageInfo,
    const ImageProcessing_ColorSpaceInfo* destinationGainmapInfo)
{
    LoadLibrary();
    auto flag = CheckDecompositionSupport(sourceImageInfo, destinationImageInfo, destinationGainmapInfo);
    UnloadLibrary();
    return flag;
}

bool ImageProcessingCapiImpl::IsMetadataGenerationSupported(const ImageProcessing_ColorSpaceInfo* sourceImageInfo)
{
    LoadLibrary();
    auto flag = CheckMetadataGenerationSupport(sourceImageInfo);

    return flag;
}

ImageProcessing_ErrorCode ImageProcessingCapiImpl::Create(OH_ImageProcessing** imageProcessor, int type)
{
    return OH_ImageProcessing::Create(imageProcessor, type, openglContext_, openclContext_);
}

ImageProcessing_ErrorCode ImageProcessingCapiImpl::Destroy(OH_ImageProcessing* imageProcessor)
{
    return OH_ImageProcessing::Destroy(imageProcessor);
}

ImageProcessing_ErrorCode ImageProcessingCapiImpl::SetParameter(OH_ImageProcessing* imageProcessor,
    const OH_AVFormat* parameter)
{
    return CallImageProcessing(imageProcessor, [&parameter](std::shared_ptr<IImageProcessingNative>& obj) {
        return obj->SetParameter(parameter);
    });
}

ImageProcessing_ErrorCode ImageProcessingCapiImpl::GetParameter(OH_ImageProcessing* imageProcessor,
    OH_AVFormat* parameter)
{
    return CallImageProcessing(imageProcessor, [&parameter](std::shared_ptr<IImageProcessingNative>& obj) {
        return obj->GetParameter(parameter);
    });
}

ImageProcessing_ErrorCode ImageProcessingCapiImpl::ConvertColorSpace(OH_ImageProcessing* imageProcessor,
    OH_PixelmapNative* sourceImage, OH_PixelmapNative* destinationImage)
{
    return CallImageProcessing(imageProcessor, [&sourceImage, &destinationImage](
        std::shared_ptr<IImageProcessingNative>& obj) {
        return obj->ConvertColorSpace(sourceImage, destinationImage);
    });
}

ImageProcessing_ErrorCode ImageProcessingCapiImpl::Compose(OH_ImageProcessing* imageProcessor,
    OH_PixelmapNative* sourceImage, OH_PixelmapNative* sourceGainmap, OH_PixelmapNative* destinationImage)
{
    return CallImageProcessing(imageProcessor, [&sourceImage, &sourceGainmap, &destinationImage](
        std::shared_ptr<IImageProcessingNative>& obj) {
        return obj->Compose(sourceImage, sourceGainmap, destinationImage);
    });
}

ImageProcessing_ErrorCode ImageProcessingCapiImpl::Decompose(OH_ImageProcessing* imageProcessor,
    OH_PixelmapNative* sourceImage, OH_PixelmapNative* destinationImage,
    OH_PixelmapNative* destinationGainmap)
{
    return CallImageProcessing(imageProcessor, [&sourceImage, &destinationImage, &destinationGainmap](
        std::shared_ptr<IImageProcessingNative>& obj) {
        return obj->Decompose(sourceImage, destinationImage, destinationGainmap);
    });
}

ImageProcessing_ErrorCode ImageProcessingCapiImpl::GenerateMetadata(OH_ImageProcessing* imageProcessor,
    OH_PixelmapNative* sourceImage)
{
    return CallImageProcessing(imageProcessor, [&sourceImage](
        std::shared_ptr<IImageProcessingNative>& obj) {
        return obj->GenerateMetadata(sourceImage);
    });
}

ImageProcessing_ErrorCode ImageProcessingCapiImpl::EnhanceDetail(OH_ImageProcessing* imageProcessor,
    OH_PixelmapNative* sourceImage, OH_PixelmapNative* destinationImage)
{
    return CallImageProcessing(imageProcessor, [&sourceImage, &destinationImage](
        std::shared_ptr<IImageProcessingNative>& obj) {
        return obj->Process(sourceImage, destinationImage);
    });
}

IImageProcessingNdk* CreateImageProcessingNdk()
{
    return new(std::nothrow) ImageProcessingCapiImpl();
}

void DestroyImageProcessingNdk(IImageProcessingNdk* obj)
{
    CHECK_AND_RETURN_LOG(obj != nullptr, "VPE image processing is null!");
    ImageProcessingCapiImpl* impl = static_cast<ImageProcessingCapiImpl*>(obj);
    delete impl;
}
