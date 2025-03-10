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

#include "image_processing_capi_capability.h"

using namespace OHOS::Media::VideoProcessingEngine;

ImageProcessingCapiCapability& ImageProcessingCapiCapability::Get()
{
    static ImageProcessingCapiCapability instance{};
    return instance;
}

ImageProcessing_ErrorCode ImageProcessingCapiCapability::OpenCLInit()
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

ImageProcessing_ErrorCode ImageProcessingCapiCapability::OpenGLInit()
{
    auto status = SetupOpengl(openglContext_);
    CHECK_AND_RETURN_RET_LOG(status == static_cast<int>(IMAGE_PROCESSING_SUCCESS),
                             IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING,
                             "OpenGLInit SetupOpengl fail!");
    return IMAGE_PROCESSING_SUCCESS;
}

ClContext* ImageProcessingCapiCapability::GetClContext()
{
    return openclContext_;
}

std::shared_ptr<OpenGLContext> ImageProcessingCapiCapability::GetOpenGLContext()
{
    return openglContext_;
}

void ImageProcessingCapiCapability::LoadLibrary()
{
    std::lock_guard<std::mutex> lock(lock_);
    if (usedInstance_ == 0 && mLibHandle == nullptr) {
        std::string path = "libvideoprocessingengine.z.so";
        mLibHandle = dlopen(path.c_str(), RTLD_NOW);
    }
    usedInstance_++;
}

void ImageProcessingCapiCapability::UnloadLibrary()
{
    std::lock_guard<std::mutex> lock(lock_);
    usedInstance_--;
    if ((usedInstance_ == 0) && (mLibHandle != nullptr)) {
        dlclose(mLibHandle);
        mLibHandle = nullptr;
    }
}

bool ImageProcessingCapiCapability::CheckColorSpaceConversionSupport(
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

bool ImageProcessingCapiCapability::CheckCompositionSupport(
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

bool ImageProcessingCapiCapability::CheckDecompositionSupport(
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

bool ImageProcessingCapiCapability::CheckMetadataGenerationSupport(
    const ImageProcessing_ColorSpaceInfo* sourceImageInfo)
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

ImageProcessing_ErrorCode ImageProcessingCapiCapability::LoadAlgo()
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

