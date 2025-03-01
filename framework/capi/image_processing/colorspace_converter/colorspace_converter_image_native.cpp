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

#include "colorspace_converter_image_native.h"

#include <unordered_map>

#include "image_processing_capi_impl.h"
#include "detail_enhancer_common.h"
#include "detail_enhancer_image_fwk.h"
#include "image_processing_utils.h"
#include "surface_buffer.h"
#include "surface_buffer_impl.h"
#include "surface_type.h"
#include "vpe_log.h"

using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

ImageProcessing_ErrorCode ColorspaceConverterImageNative::InitializeInner()
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(!isInitialized_.load(), IMAGE_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already init!");
    colorspaceConverter_ = ColorSpaceConverter::Create(nullptr, opengclContext_);
    CHECK_AND_RETURN_RET_LOG(colorspaceConverter_ != nullptr, IMAGE_PROCESSING_ERROR_CREATE_FAILED,
        "Create detail colorspaceConverter failed!");
    isInitialized_ = true;
    return IMAGE_PROCESSING_SUCCESS;
}

ImageProcessing_ErrorCode ColorspaceConverterImageNative::DeinitializeInner()
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), IMAGE_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already deinit!");
    colorspaceConverter_ = nullptr;
    isInitialized_ = false;
    return IMAGE_PROCESSING_SUCCESS;
}

ImageProcessing_ErrorCode ColorspaceConverterImageNative::SetParameter(const OHOS::Media::Format& parameter)
{
    return IMAGE_PROCESSING_SUCCESS;
}

ImageProcessing_ErrorCode ColorspaceConverterImageNative::GetParameter(OHOS::Media::Format& parameter)
{
    return IMAGE_PROCESSING_SUCCESS;
}

ImageProcessing_ErrorCode ColorspaceConverterImageNative::Process(
    const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
    std::shared_ptr<OHOS::Media::PixelMap>& destinationImage)
{
    return IMAGE_PROCESSING_SUCCESS;
}
CM_ColorSpaceInfo GetColorSpaceInfo(const uint32_t colorSpaceType)
{
    CM_ColorSpaceInfo info;
    info.primaries = static_cast<CM_ColorPrimaries>((colorSpaceType & COLORPRIMARIES_MASK) >> COLORPRIMARIES_OFFSET);
    info.transfunc = static_cast<CM_TransFunc>((colorSpaceType & TRANSFUNC_MASK) >> TRANSFUNC_OFFSET);
    info.matrix = static_cast<CM_Matrix>((colorSpaceType & MATRIX_MASK) >> MATRIX_OFFSET);
    info.range = static_cast<CM_Range>((colorSpaceType & RANGE_MASK) >> RANGE_OFFSET);
    return info;
}

ImageProcessing_ErrorCode ColorspaceConverterImageNative::ConvertColorSpace(
    const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
    std::shared_ptr<OHOS::Media::PixelMap>& destinationImage)
{
    ColorSpaceConverterParameter parameter;
    parameter.renderIntent = RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC;
    auto ret = ImageProcessingUtils::InnerErrorToCAPI(colorspaceConverter_->SetParameter(parameter));
    CHECK_AND_RETURN_RET_LOG(ret == IMAGE_PROCESSING_SUCCESS, ret, "cSetParameter failed!");
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), IMAGE_PROCESSING_ERROR_INITIALIZE_FAILED,
        "ConvertColorSpace image is not initialized!");
    CHECK_AND_RETURN_RET_LOG(sourceImage != nullptr && destinationImage != nullptr,
        IMAGE_PROCESSING_ERROR_INVALID_PARAMETER, "sourceImage or destinationImage is null!");
    sptr<SurfaceBuffer> sourceImageSurfaceBuffer = nullptr;
    sptr<SurfaceBuffer> destinationImageSurfaceBuffer = nullptr;
    ret = ConvertPixelmapToSurfaceBuffer(sourceImage, sourceImageSurfaceBuffer);
    CHECK_AND_RETURN_RET_LOG(ret == IMAGE_PROCESSING_SUCCESS, ret, "convert to surface buffer failed!");
    ret = ConvertPixelmapToSurfaceBuffer(destinationImage, destinationImageSurfaceBuffer);
    CHECK_AND_RETURN_RET_LOG(ret == IMAGE_PROCESSING_SUCCESS, ret, "convert to surface buffer failed!");
    ret = ImageProcessingUtils::InnerErrorToCAPI(
        colorspaceConverter_->Process(sourceImageSurfaceBuffer, destinationImageSurfaceBuffer));
    CHECK_AND_RETURN_RET_LOG(ret == IMAGE_PROCESSING_SUCCESS, ret, "process failed!");
    return IMAGE_PROCESSING_SUCCESS;
}

ImageProcessing_ErrorCode ColorspaceConverterImageNative::Compose(
    const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
    const std::shared_ptr<OHOS::Media::PixelMap>& sourceGainmap,
    std::shared_ptr<OHOS::Media::PixelMap>& destinationImage)
{
    ColorSpaceConverterParameter parameter;
    parameter.renderIntent = RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC;
    auto ret = ImageProcessingUtils::InnerErrorToCAPI(colorspaceConverter_->SetParameter(parameter));
    CHECK_AND_RETURN_RET_LOG(ret == IMAGE_PROCESSING_SUCCESS, ret, "cSetParameter failed!");
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), IMAGE_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Compose image is not initialized!");
    CHECK_AND_RETURN_RET_LOG(sourceImage != nullptr && destinationImage != nullptr && sourceGainmap != nullptr,
        IMAGE_PROCESSING_ERROR_INVALID_PARAMETER, "sourceImage or destinationImage is null!");
    sptr<SurfaceBuffer> sourceImageSurfaceBuffer = nullptr;
    sptr<SurfaceBuffer> sourceGainmapSurfaceBuffer = nullptr;
    sptr<SurfaceBuffer> destinationImageSurfaceBuffer = nullptr;
    ret = ConvertPixelmapToSurfaceBuffer(sourceImage, sourceImageSurfaceBuffer);
    CHECK_AND_RETURN_RET_LOG(ret == IMAGE_PROCESSING_SUCCESS, ret, "convert to surface buffer failed!");
    ret = ConvertPixelmapToSurfaceBuffer(sourceGainmap, sourceGainmapSurfaceBuffer);
    CHECK_AND_RETURN_RET_LOG(ret == IMAGE_PROCESSING_SUCCESS, ret, "convert to surface buffer failed!");
    ret = ConvertPixelmapToSurfaceBuffer(destinationImage, destinationImageSurfaceBuffer);
    CHECK_AND_RETURN_RET_LOG(ret == IMAGE_PROCESSING_SUCCESS, ret, "convert to surface buffer failed!");
    ret = ImageProcessingUtils::InnerErrorToCAPI(
        colorspaceConverter_->ComposeImage(sourceImageSurfaceBuffer, sourceGainmapSurfaceBuffer,
        destinationImageSurfaceBuffer, false));
    CHECK_AND_RETURN_RET_LOG(ret == IMAGE_PROCESSING_SUCCESS, ret, "process failed!");
    return ret;
}

ImageProcessing_ErrorCode ColorspaceConverterImageNative::Decompose(
    const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
    std::shared_ptr<OHOS::Media::PixelMap>& destinationImage,
    std::shared_ptr<OHOS::Media::PixelMap>& destinationGainmap)
{
    ColorSpaceConverterParameter parameter;
    parameter.renderIntent = RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC;
    auto ret = ImageProcessingUtils::InnerErrorToCAPI(colorspaceConverter_->SetParameter(parameter));
    CHECK_AND_RETURN_RET_LOG(ret == IMAGE_PROCESSING_SUCCESS, ret, "cSetParameter failed!");
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), IMAGE_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Decompose image is not initialized!");
    CHECK_AND_RETURN_RET_LOG(sourceImage != nullptr && destinationImage != nullptr && destinationGainmap != nullptr,
        IMAGE_PROCESSING_ERROR_INVALID_PARAMETER, "sourceImage or destinationImage is null!");
    sptr<SurfaceBuffer> sourceImageSurfaceBuffer = nullptr;
    sptr<SurfaceBuffer> destinationImageSurfaceBuffer = nullptr;
    sptr<SurfaceBuffer> destinationGainmapSurfaceBuffer = nullptr;
    ret = ConvertPixelmapToSurfaceBuffer(sourceImage, sourceImageSurfaceBuffer);
    CHECK_AND_RETURN_RET_LOG(ret == IMAGE_PROCESSING_SUCCESS, ret, "convert to surface buffer failed!");
    ret = ConvertPixelmapToSurfaceBuffer(destinationImage, destinationImageSurfaceBuffer);
    CHECK_AND_RETURN_RET_LOG(ret == IMAGE_PROCESSING_SUCCESS, ret, "convert to surface buffer failed!");
    ret = ConvertPixelmapToSurfaceBuffer(destinationGainmap, destinationGainmapSurfaceBuffer);
    CHECK_AND_RETURN_RET_LOG(ret == IMAGE_PROCESSING_SUCCESS, ret, "convert to surface buffer failed!");
    ret = ImageProcessingUtils::InnerErrorToCAPI(
        colorspaceConverter_->DecomposeImage(sourceImageSurfaceBuffer, destinationImageSurfaceBuffer,
        destinationGainmapSurfaceBuffer));
    CHECK_AND_RETURN_RET_LOG(ret == IMAGE_PROCESSING_SUCCESS, ret, "process failed!");
    return ret;
}

ImageProcessing_ErrorCode ColorspaceConverterImageNative::GenerateMetadata(
    const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage)
{
    return IMAGE_PROCESSING_SUCCESS;
}

ImageProcessing_ErrorCode ColorspaceConverterImageNative::EnhanceDetail(
    const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
    std::shared_ptr<OHOS::Media::PixelMap>& destinationImage)
{
    return IMAGE_PROCESSING_SUCCESS;
}
// LCOV_EXCL_START
static CM_ColorSpaceType ConvertColorSpaceType(ColorManager::ColorSpaceName colorSpace, bool base)
{
    switch (colorSpace) {
        case ColorManager::ColorSpaceName::SRGB :
            return CM_SRGB_FULL;
        case ColorManager::ColorSpaceName::SRGB_LIMIT :
            return CM_SRGB_LIMIT;
        case ColorManager::ColorSpaceName::DISPLAY_P3 :
            return CM_P3_FULL;
        case ColorManager::ColorSpaceName::DISPLAY_P3_LIMIT :
            return CM_P3_LIMIT;
        case ColorManager::ColorSpaceName::BT2020 :
        case ColorManager::ColorSpaceName::BT2020_HLG :
            return CM_BT2020_HLG_FULL;
        case ColorManager::ColorSpaceName::BT2020_HLG_LIMIT :
            return CM_BT2020_HLG_LIMIT;
        case ColorManager::ColorSpaceName::BT2020_PQ :
            return CM_BT2020_PQ_FULL;
        case ColorManager::ColorSpaceName::BT2020_PQ_LIMIT :
            return CM_BT2020_PQ_LIMIT;
        default:
            return base ? CM_SRGB_FULL : CM_BT2020_HLG_FULL;
    }
    return base ? CM_SRGB_FULL : CM_BT2020_HLG_FULL;
}

ImageProcessing_ErrorCode ColorspaceConverterImageNative::ConvertPixelmapToSurfaceBuffer(
    const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap, sptr<SurfaceBuffer>& bufferImpl)
{
    auto it = IMAGE_FORMAT_MAP.find(pixelmap->GetPixelFormat());
    CHECK_AND_RETURN_RET_LOG(it != IMAGE_FORMAT_MAP.end(), IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING,
        "unsupported format: %{public}d", pixelmap->GetPixelFormat());
    CHECK_AND_RETURN_RET_LOG(pixelmap->GetAllocatorType() == AllocatorType::DMA_ALLOC,
        IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING, "GetAllocatorType: %{public}d", pixelmap->GetAllocatorType());
    bufferImpl = reinterpret_cast<SurfaceBuffer*>(pixelmap->GetFd());
    CHECK_AND_RETURN_RET_LOG(bufferImpl != nullptr, IMAGE_PROCESSING_ERROR_PROCESS_FAILED,
        "bufferImpl is nullptr");
    auto colorspace = ConvertColorSpaceType(pixelmap->InnerGetGrColorSpace().GetColorSpaceName(), true);
    auto colorspaceinfo = GetColorSpaceInfo(colorspace);
    VPE_LOGD("colorspace : %{public}d", colorspace);
    std::vector<uint8_t> colorSpaceInfoVec;
    colorSpaceInfoVec.resize(sizeof(CM_ColorSpaceInfo));
    auto ret = memcpy_s(colorSpaceInfoVec.data(), colorSpaceInfoVec.size(), &colorspaceinfo,
        sizeof(CM_ColorSpaceInfo));
    CHECK_AND_RETURN_RET_LOG(ret == EOK, IMAGE_PROCESSING_ERROR_PROCESS_FAILED,
        "memcpy_s, err: %{public}d", ret);
    auto err = bufferImpl->SetMetadata(ATTRKEY_COLORSPACE_INFO, colorSpaceInfoVec);
    CHECK_AND_RETURN_RET_LOG(GSERROR_OK == err, IMAGE_PROCESSING_ERROR_PROCESS_FAILED,
        "Get hdr metadata type failed, err: %{public}d", err);
    return IMAGE_PROCESSING_SUCCESS;
}

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS