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

#ifndef VIDEO_PROCESSING_CAPI_CAPABILITY_H
#define VIDEO_PROCESSING_CAPI_CAPABILITY_H

#include <unordered_map>

#include "v1_0/cm_color_space.h"
#include "native_buffer.h"
#include "surface_type.h"

#include "video_processing_types.h"
#include "algorithm_common.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
using namespace HDI::Display::Graphic::Common::V1_0;

const std::unordered_map<OH_NativeBuffer_ColorSpace, CM_ColorSpaceType> NATIVE_COLORSPACE_TO_CM_MAP = {
    {OH_COLORSPACE_BT601_EBU_LIMIT, CM_BT601_EBU_LIMIT},
    {OH_COLORSPACE_BT601_SMPTE_C_LIMIT, CM_BT601_SMPTE_C_LIMIT},
    {OH_COLORSPACE_BT709_LIMIT, CM_BT709_LIMIT},
    {OH_COLORSPACE_BT2020_HLG_LIMIT, CM_BT2020_HLG_LIMIT},
    {OH_COLORSPACE_BT2020_PQ_LIMIT, CM_BT2020_PQ_LIMIT},
    {OH_COLORSPACE_BT601_EBU_FULL, CM_BT601_EBU_FULL},
    {OH_COLORSPACE_BT601_SMPTE_C_FULL, CM_BT601_SMPTE_C_FULL},
    {OH_COLORSPACE_BT709_FULL, CM_BT709_FULL},
    {OH_COLORSPACE_BT2020_HLG_FULL, CM_BT2020_HLG_FULL},
    {OH_COLORSPACE_BT2020_PQ_FULL, CM_BT2020_PQ_FULL},
};
const std::unordered_map<OH_NativeBuffer_MetadataType, CM_HDR_Metadata_Type> NATIVE_METADATATYPE_TO_CM_MAP = {
    {OH_VIDEO_HDR_HLG, CM_VIDEO_HLG},
    {OH_VIDEO_HDR_HDR10, CM_VIDEO_HDR10},
    {OH_VIDEO_HDR_VIVID, CM_VIDEO_HDR_VIVID},
};
const std::unordered_map<OH_NativeBuffer_Format, GraphicPixelFormat> NATIVE_FORMAT_TO_GRAPHIC_MAP = {
    {NATIVEBUFFER_PIXEL_FMT_RGBA_8888, GRAPHIC_PIXEL_FMT_RGBA_8888},
    {NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, GRAPHIC_PIXEL_FMT_YCBCR_420_SP},
    {NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP, GRAPHIC_PIXEL_FMT_YCRCB_420_SP},
    {NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, GRAPHIC_PIXEL_FMT_RGBA_1010102},
    {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, GRAPHIC_PIXEL_FMT_YCBCR_P010},
    {NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, GRAPHIC_PIXEL_FMT_YCRCB_P010},
};

class VideoProcessingCapiCapability {
public:
    static bool IsColorSpaceConversionSupported(const VideoProcessing_ColorSpaceInfo* sourceVideoInfo,
        const VideoProcessing_ColorSpaceInfo* destinationVideoInfo);
    static bool IsMetadataGenerationSupported(const VideoProcessing_ColorSpaceInfo* sourceVideoInfo);
    static VideoProcessing_ErrorCode OpenGLInit();
    static std::shared_ptr<OpenGLContext> GetOpenGLContext();
private:
    static std::shared_ptr<OpenGLContext> openglContext_;
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // VIDEO_PROCESSING_CAPI_CAPABILITY_H