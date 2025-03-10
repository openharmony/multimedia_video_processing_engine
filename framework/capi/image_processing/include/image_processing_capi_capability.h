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

#ifndef IMAGE_PROCESSING_CAPI_CAPABILITY_H
#define IMAGE_PROCESSING_CAPI_CAPABILITY_H

#include <atomic>
#include <dlfcn.h>
#include <functional>
#include <mutex>
#include <unordered_map>

#include "algorithm_common.h"
#include "frame_info.h"
#include "image_processing_native_template.h"
#include "image_processing_types.h"
#include "pixelmap_native_impl.h"
#include "detail_enhancer_image.h"
#include "colorspace_converter.h"
#include "colorspace_converter_image_native.h"
#include "detail_enhancer_common.h"
#include "detail_enhancer_image_fwk.h"
#include "image_processing_utils.h"
#include "surface_buffer.h"
#include "surface_buffer_impl.h"
#include "surface_type.h"
#include "vpe_log.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

const std::map<OHOS::Media::PixelFormat, OHOS::GraphicPixelFormat> IMAGE_FORMAT_MAP = {
    { OHOS::Media::PixelFormat::RGBA_8888, OHOS::GraphicPixelFormat::GRAPHIC_PIXEL_FMT_RGBA_8888 },
    { OHOS::Media::PixelFormat::BGRA_8888, OHOS::GraphicPixelFormat::GRAPHIC_PIXEL_FMT_BGRA_8888 },
    { OHOS::Media::PixelFormat::RGBA_1010102, OHOS::GraphicPixelFormat::GRAPHIC_PIXEL_FMT_RGBA_1010102 },
    { OHOS::Media::PixelFormat::YCBCR_P010, OHOS::GraphicPixelFormat::GRAPHIC_PIXEL_FMT_YCBCR_P010 },
    { OHOS::Media::PixelFormat::YCRCB_P010, OHOS::GraphicPixelFormat::GRAPHIC_PIXEL_FMT_YCRCB_P010 },
};

typedef enum {
    NONE = 0,
    BASE = 1,
    GAINMAP = 2,
    ALTERNATE = 3,
} ImagePixelmapHdrMetadataType;

const std::map<ImagePixelmapHdrMetadataType,
    OHOS::HDI::Display::Graphic::Common::V1_0::CM_HDR_Metadata_Type> HDR_METADATA_TYPE_MAP = {
    { NONE, OHOS::HDI::Display::Graphic::Common::V1_0::CM_METADATA_NONE },
    { BASE, OHOS::HDI::Display::Graphic::Common::V1_0::CM_IMAGE_HDR_VIVID_DUAL },
    { GAINMAP, OHOS::HDI::Display::Graphic::Common::V1_0::CM_IMAGE_HDR_VIVID_DUAL },
    { ALTERNATE, OHOS::HDI::Display::Graphic::Common::V1_0::CM_IMAGE_HDR_VIVID_SINGLE },
};

typedef enum {
    UNKNOWN = 0,
    ADOBE_RGB_1998 = 1,
    DCI_P3 = 2,
    DISPLAY_P3 = 3,
    SRGB = 4,
    BT709 = 5,
    BT601_EBU = 6,
    BT2020_HLG = 9,
    BT2020_PQ = 10,
    P3_HLG = 11,
    DISPLAY_P3_LIMIT = 14,
    SRGB_LIMIT = 15,
    BT2020_HLG_LIMIT = 19,
    BT2020_PQ_LIMIT = 20,
} ImagePixelmapColorspace;

const std::map<ImagePixelmapColorspace, OHOS::HDI::Display::Graphic::Common::V1_0::CM_ColorSpaceType> COLORSPACE_MAP = {
    { SRGB, OHOS::HDI::Display::Graphic::Common::V1_0::CM_SRGB_FULL },
    { SRGB_LIMIT, OHOS::HDI::Display::Graphic::Common::V1_0::CM_SRGB_LIMIT },
    { DISPLAY_P3, OHOS::HDI::Display::Graphic::Common::V1_0::CM_P3_FULL },
    { DISPLAY_P3_LIMIT, OHOS::HDI::Display::Graphic::Common::V1_0::CM_P3_LIMIT },
    { BT2020_HLG, OHOS::HDI::Display::Graphic::Common::V1_0::CM_BT2020_HLG_FULL },
    { BT2020_HLG_LIMIT, OHOS::HDI::Display::Graphic::Common::V1_0::CM_BT2020_HLG_LIMIT },
    { BT2020_PQ, OHOS::HDI::Display::Graphic::Common::V1_0::CM_BT2020_PQ_FULL },
    { BT2020_PQ_LIMIT, OHOS::HDI::Display::Graphic::Common::V1_0::CM_BT2020_PQ_LIMIT },
    { P3_HLG, OHOS::HDI::Display::Graphic::Common::V1_0::CM_P3_HLG_LIMIT },
    { ADOBE_RGB_1998, OHOS::HDI::Display::Graphic::Common::V1_0::CM_ADOBERGB_FULL },
};

class ImageProcessingCapiCapability {
public:
    static ImageProcessingCapiCapability& Get();

    ImageProcessingCapiCapability() = default;
    virtual ~ImageProcessingCapiCapability() = default;
    ImageProcessingCapiCapability(const ImageProcessingCapiCapability&) = delete;
    ImageProcessingCapiCapability& operator=(const ImageProcessingCapiCapability&) = delete;
    ImageProcessingCapiCapability(ImageProcessingCapiCapability&&) = delete;
    ImageProcessingCapiCapability& operator=(ImageProcessingCapiCapability&&) = delete;

    ImageProcessing_ErrorCode OpenCLInit();
    ImageProcessing_ErrorCode OpenGLInit();
    std::shared_ptr<OpenGLContext> GetOpenGLContext();
    ClContext* GetClContext();
    void LoadLibrary();
    void UnloadLibrary();
    bool CheckColorSpaceConversionSupport(
        const ImageProcessing_ColorSpaceInfo* sourceImageInfo,
        const ImageProcessing_ColorSpaceInfo* destinationImageInfo);
    bool CheckCompositionSupport(const ImageProcessing_ColorSpaceInfo* sourceImageInfo,
        const ImageProcessing_ColorSpaceInfo* sourceGainmapInfo,
        const ImageProcessing_ColorSpaceInfo* destinationImageInfo);
    bool CheckDecompositionSupport(const ImageProcessing_ColorSpaceInfo* sourceImageInfo,
        const ImageProcessing_ColorSpaceInfo* destinationImageInfo,
        const ImageProcessing_ColorSpaceInfo* destinationGainmapInfo);
    bool CheckMetadataGenerationSupport(const ImageProcessing_ColorSpaceInfo* sourceImageInfo);
    
private:
    using LibFunction = bool (*)(const OHOS::Media::VideoProcessingEngine::ColorSpaceInfo inputInfo,
        const OHOS::Media::VideoProcessingEngine::ColorSpaceInfo outputInfo);
    using LibMetaFunction = bool (*)(const OHOS::Media::VideoProcessingEngine::ColorSpaceInfo inputInfo);

    ImageProcessing_ErrorCode LoadAlgo();

    std::shared_ptr<OpenGLContext> openglContext_{nullptr};
    ClContext *openclContext_{nullptr};
    std::mutex lock_;
    int32_t usedInstance_ {0};
    void* mLibHandle{};
    LibFunction isDecompositionSupported_{nullptr};
    LibFunction isColorSpaceConversionSupported_{nullptr};
    LibFunction isCompositionSupported_{nullptr};
    LibMetaFunction isMetadataGenSupported_{nullptr};
};

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif