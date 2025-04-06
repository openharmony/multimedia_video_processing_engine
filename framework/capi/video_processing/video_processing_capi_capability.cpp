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

#include "video_processing_capi_capability.h"

#include <cstdio>
#include <iostream>
#include <string>
#include <unordered_set>

#include "vpe_log.h"

using namespace OHOS::Media::VideoProcessingEngine;
using namespace OHOS::HDI::Display::Graphic::Common::V1_0;

struct MetaSupportKey {
    int32_t metadata;
    int32_t colorSpace;
    int32_t format;

    bool operator<(const MetaSupportKey& k) const
    {
        return metadata < k.metadata ||
            (metadata == k.metadata && colorSpace < k.colorSpace) ||
            (metadata == k.metadata && colorSpace == k.colorSpace && format < k.format);
    }

    bool operator==(const MetaSupportKey& k) const
    {
        return metadata == k.metadata && colorSpace == k.colorSpace && format == k.format;
    }
};
template<> struct std::hash<MetaSupportKey> {
    std::size_t operator()(const MetaSupportKey& k) const
    {
        return std::hash<uint32_t>()(k.metadata) ^ std::hash<uint32_t>()(k.colorSpace) ^
            std::hash<uint32_t>()(k.format);
    }
};

struct CscvSupportKey {
    int32_t metadataIn;
    int32_t colorSpaceIn;
    int32_t formatIn;
    int32_t metadataOut;
    int32_t colorSpaceOut;
    int32_t formatOut;

    bool operator<(const CscvSupportKey& k) const
    {
        return metadataIn < k.metadataIn ||
            (metadataIn == k.metadataIn && colorSpaceIn < k.colorSpaceIn) ||
            (metadataIn == k.metadataIn && colorSpaceIn == k.colorSpaceIn && formatIn < k.formatIn) ||
            (metadataIn == k.metadataIn && colorSpaceIn == k.colorSpaceIn  && formatIn == k.formatIn &&
                metadataOut < k.metadataOut) ||
            (metadataIn == k.metadataIn && colorSpaceIn == k.colorSpaceIn  && formatIn == k.formatIn  &&
                metadataOut == k.metadataOut && colorSpaceOut < k.colorSpaceOut) ||
            (metadataIn == k.metadataIn && colorSpaceIn == k.colorSpaceIn  && formatIn == k.formatIn  &&
                metadataOut == k.metadataOut  && colorSpaceOut == k.colorSpaceOut && formatOut < k.formatOut);
    }

    bool operator==(const CscvSupportKey& k) const
    {
        return metadataIn == k.metadataIn && colorSpaceIn == k.colorSpaceIn && formatIn == k.formatIn &&
            metadataOut == k.metadataOut && colorSpaceOut == k.colorSpaceOut && formatOut == k.formatOut;
    }
};
template<> struct std::hash<CscvSupportKey> {
    std::size_t operator()(const CscvSupportKey& k) const
    {
        return std::hash<uint32_t>()(k.metadataIn) ^ std::hash<uint32_t>()(k.colorSpaceIn) ^
            std::hash<uint32_t>()(k.formatIn) ^ std::hash<uint32_t>()(k.metadataOut) ^
            std::hash<uint32_t>()(k.colorSpaceOut) ^ std::hash<uint32_t>()(k.formatOut);
    }
};

namespace {
const std::unordered_set<MetaSupportKey> VIDEO_META_SUPPORT_MAP = {
    { CM_VIDEO_HDR10, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010 },
    { CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010 },
    { CM_VIDEO_HDR10, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010 },
    { CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010 },
    { CM_VIDEO_HDR10, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102 },
    { CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102 },
};
const std::unordered_set<CscvSupportKey> VIDEO_COLORSPACE_CONVERTER_SUPPORT_SDRTOSDR_MAP = {
    { CM_METADATA_NONE, CM_BT601_EBU_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP },
    { CM_METADATA_NONE, CM_BT601_SMPTE_C_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP },
    { CM_METADATA_NONE, CM_BT601_EBU_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_420_SP },
    { CM_METADATA_NONE, CM_BT601_SMPTE_C_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_420_SP },
    { CM_METADATA_NONE, CM_BT601_EBU_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_8888 },
    { CM_METADATA_NONE, CM_BT601_SMPTE_C_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_8888 },
    { CM_METADATA_NONE, CM_BT601_EBU_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_420_SP,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP },
    { CM_METADATA_NONE, CM_BT601_SMPTE_C_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_420_SP,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP },
    { CM_METADATA_NONE, CM_BT601_EBU_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_420_SP,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_420_SP },
    { CM_METADATA_NONE, CM_BT601_SMPTE_C_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_420_SP,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_420_SP },
    { CM_METADATA_NONE, CM_BT601_EBU_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_420_SP,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_8888 },
    { CM_METADATA_NONE, CM_BT601_SMPTE_C_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_420_SP,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_8888 },
    { CM_METADATA_NONE, CM_BT601_EBU_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_8888,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP },
    { CM_METADATA_NONE, CM_BT601_SMPTE_C_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_8888,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP },
    { CM_METADATA_NONE, CM_BT601_EBU_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_8888,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_420_SP },
    { CM_METADATA_NONE, CM_BT601_SMPTE_C_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_8888,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_420_SP },
    { CM_METADATA_NONE, CM_BT601_EBU_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_8888,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_8888 },
    { CM_METADATA_NONE, CM_BT601_SMPTE_C_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_8888,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_8888 },
};
const std::unordered_set<CscvSupportKey> VIDEO_COLORSPACE_CONVERTER_SUPPORT_HDRTOSDR_MAP = {
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_420_SP },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_420_SP },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_8888 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_8888 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_420_SP },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_420_SP },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_8888 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_8888 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_420_SP },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_420_SP },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_8888 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102,
      CM_METADATA_NONE, CM_BT709_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_8888 },
};
const std::unordered_set<CscvSupportKey> VIDEO_COLORSPACE_CONVERTER_SUPPORT_HDRTOHDR_MAP = {
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010,
      CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010 },
    { CM_VIDEO_HDR10, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010,
      CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010,
      CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010,
      CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010,
      CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010 },
    { CM_VIDEO_HDR10, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010,
      CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010,
      CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010,
      CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010,
      CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102 },
    { CM_VIDEO_HDR10, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010,
      CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010,
      CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010,
      CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010,
      CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010 },
    { CM_VIDEO_HDR10, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010,
      CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010,
      CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010,
      CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010,
      CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010 },
    { CM_VIDEO_HDR10, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010,
      CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010,
      CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010,
      CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010,
      CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102 },
    { CM_VIDEO_HDR10, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010,
      CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010,
      CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010,
      CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102,
      CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010 },
    { CM_VIDEO_HDR10, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102,
      CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102,
      CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102,
      CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCBCR_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102,
      CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010 },
    { CM_VIDEO_HDR10, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102,
      CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102,
      CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102,
      CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_YCRCB_P010 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102,
      CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102 },
    { CM_VIDEO_HDR10, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102,
      CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102,
      CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102 },
    { CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102,
      CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, GRAPHIC_PIXEL_FMT_RGBA_1010102 },
};
}

CscvSupportKey VideoColorSpaceInfoToCscvKey(const VideoProcessing_ColorSpaceInfo* sourceVideoInfo,
    const VideoProcessing_ColorSpaceInfo* destinationVideoInfo)
{
    int32_t metadataValeSrc = static_cast<int32_t>(CM_METADATA_NONE);
    int32_t colorSpaceValeSrc = static_cast<int32_t>(OH_COLORSPACE_NONE);
    int32_t formatValeSrc = static_cast<int32_t>(NATIVEBUFFER_PIXEL_FMT_BUTT);
    int32_t metadataValeDst = static_cast<int32_t>(CM_METADATA_NONE);
    int32_t colorSpaceValeDst = static_cast<int32_t>(OH_COLORSPACE_NONE);
    int32_t formatValeDst = static_cast<int32_t>(NATIVEBUFFER_PIXEL_FMT_BUTT);
    OH_NativeBuffer_Format formatIn = static_cast<OH_NativeBuffer_Format>(sourceVideoInfo->pixelFormat);
    auto itInFormat = NATIVE_FORMAT_TO_GRAPHIC_MAP.find(formatIn);
    if (itInFormat != NATIVE_FORMAT_TO_GRAPHIC_MAP.end()) {
        formatValeSrc = static_cast<int32_t>(itInFormat->second);
    }
    OH_NativeBuffer_Format formatOut = static_cast<OH_NativeBuffer_Format>(destinationVideoInfo->pixelFormat);
    auto itOutFormat = NATIVE_FORMAT_TO_GRAPHIC_MAP.find(formatOut);
    if (itOutFormat != NATIVE_FORMAT_TO_GRAPHIC_MAP.end()) {
        formatValeDst = static_cast<int32_t>(itOutFormat->second);
    }
    OH_NativeBuffer_ColorSpace colorIn = static_cast<OH_NativeBuffer_ColorSpace>(sourceVideoInfo->colorSpace);
    auto itInColorSpace = NATIVE_COLORSPACE_TO_CM_MAP.find(colorIn);
    if (itInColorSpace != NATIVE_COLORSPACE_TO_CM_MAP.end()) {
        colorSpaceValeSrc = static_cast<int32_t>(itInColorSpace->second);
    }
    OH_NativeBuffer_ColorSpace colorOut = static_cast<OH_NativeBuffer_ColorSpace>(destinationVideoInfo->colorSpace);
    auto itOutColorSpace = NATIVE_COLORSPACE_TO_CM_MAP.find(colorOut);
    if (itOutColorSpace != NATIVE_COLORSPACE_TO_CM_MAP.end()) {
        colorSpaceValeDst = static_cast<int32_t>(itOutColorSpace->second);
    }
    OH_NativeBuffer_MetadataType metaIn = static_cast<OH_NativeBuffer_MetadataType>(sourceVideoInfo->metadataType);
    auto itInMetadata = NATIVE_METADATATYPE_TO_CM_MAP.find(metaIn);
    if (itInMetadata != NATIVE_METADATATYPE_TO_CM_MAP.end()) {
        metadataValeSrc = static_cast<int32_t>(itInMetadata->second);
    }
    OH_NativeBuffer_MetadataType metaOut = static_cast<OH_NativeBuffer_MetadataType>(
        destinationVideoInfo->metadataType);
    auto itOutMetadata = NATIVE_METADATATYPE_TO_CM_MAP.find(metaOut);
    if (itOutMetadata != NATIVE_METADATATYPE_TO_CM_MAP.end()) {
        metadataValeDst = static_cast<int32_t>(itOutMetadata->second);
    }
    CscvSupportKey keyReturn{ metadataValeSrc, colorSpaceValeSrc, formatValeSrc,
        metadataValeDst, colorSpaceValeDst, formatValeDst };
    return keyReturn;
}
bool VideoProcessingCapiCapability::IsColorSpaceConversionSupported(
    const VideoProcessing_ColorSpaceInfo* sourceVideoInfo,
    const VideoProcessing_ColorSpaceInfo* destinationVideoInfo)
{
    if (!access("/system/lib64/libvideoprocessingengine_ext.z.so", 0)) {
        return false;
    }
    CHECK_AND_RETURN_RET_LOG(sourceVideoInfo != nullptr, false, "sourceVideoInfo is null!");
    CHECK_AND_RETURN_RET_LOG(destinationVideoInfo != nullptr, false, "destinationVideoInfo is null!");
    CscvSupportKey keySource = VideoColorSpaceInfoToCscvKey(sourceVideoInfo, destinationVideoInfo);
    auto it = VIDEO_COLORSPACE_CONVERTER_SUPPORT_SDRTOSDR_MAP.find(keySource);
    if (it != VIDEO_COLORSPACE_CONVERTER_SUPPORT_SDRTOSDR_MAP.end()) {
        return true;
    }
    it = VIDEO_COLORSPACE_CONVERTER_SUPPORT_HDRTOSDR_MAP.find(keySource);
    if (it != VIDEO_COLORSPACE_CONVERTER_SUPPORT_HDRTOSDR_MAP.end()) {
        return true;
    }
    it = VIDEO_COLORSPACE_CONVERTER_SUPPORT_HDRTOHDR_MAP.find(keySource);
    if (it != VIDEO_COLORSPACE_CONVERTER_SUPPORT_HDRTOHDR_MAP.end()) {
        return true;
    }
    VPE_LOGE("IsColorSpaceConversionSupported false (FormatIn:%{public}d, ColorIn:%{public}d, MetaIn:%{public}d, \
        FormatOut:%{public}d, ColorOut:%{public}d, MetaOut:%{public}d) !",
        sourceVideoInfo->pixelFormat, sourceVideoInfo->colorSpace, sourceVideoInfo->metadataType,
        destinationVideoInfo->pixelFormat, destinationVideoInfo->colorSpace, destinationVideoInfo->metadataType);
    return false;
}

MetaSupportKey VideoColorSpaceInfoToMetaKey(const VideoProcessing_ColorSpaceInfo* sourceVideoInfo)
{
    int32_t metadataVale = static_cast<int32_t>(CM_METADATA_NONE);
    int32_t colorSpaceVale = static_cast<int32_t>(OH_COLORSPACE_NONE);
    int32_t formatVale = static_cast<int32_t>(NATIVEBUFFER_PIXEL_FMT_BUTT);
    OH_NativeBuffer_Format formatIn = static_cast<OH_NativeBuffer_Format>(sourceVideoInfo->pixelFormat);
    auto itInFormat = NATIVE_FORMAT_TO_GRAPHIC_MAP.find(formatIn);
    if (itInFormat != NATIVE_FORMAT_TO_GRAPHIC_MAP.end()) {
        formatVale = static_cast<int32_t>(itInFormat->second);
    }
    OH_NativeBuffer_ColorSpace colorIn = static_cast<OH_NativeBuffer_ColorSpace>(sourceVideoInfo->colorSpace);
    auto itInColorSpace = NATIVE_COLORSPACE_TO_CM_MAP.find(colorIn);
    if (itInColorSpace != NATIVE_COLORSPACE_TO_CM_MAP.end()) {
        colorSpaceVale = static_cast<int32_t>(itInColorSpace->second);
    }
    OH_NativeBuffer_MetadataType metaIn = static_cast<OH_NativeBuffer_MetadataType>(sourceVideoInfo->metadataType);
    auto itInMetadata = NATIVE_METADATATYPE_TO_CM_MAP.find(metaIn);
    if (itInMetadata != NATIVE_METADATATYPE_TO_CM_MAP.end()) {
        metadataVale = static_cast<int32_t>(itInMetadata->second);
    }
    MetaSupportKey keyReturn{ metadataVale, colorSpaceVale, formatVale };
    return keyReturn;
}
bool VideoProcessingCapiCapability::IsMetadataGenerationSupported(
    const VideoProcessing_ColorSpaceInfo* sourceVideoInfo)
{
    if (!access("/system/lib64/libvideoprocessingengine_ext.z.so", 0)) {
        return false;
    }
    CHECK_AND_RETURN_RET_LOG(sourceVideoInfo != nullptr, false, "sourceVideoInfo is null!");
    MetaSupportKey keySource = VideoColorSpaceInfoToMetaKey(sourceVideoInfo);
    auto it = VIDEO_META_SUPPORT_MAP.find(keySource);
    if (it != VIDEO_META_SUPPORT_MAP.end()) {
        return true;
    }
    VPE_LOGE("IsMetadataGenerationSupported false (Format:%{public}d, Color:%{public}d, Meta:%{public}d) !",
        sourceVideoInfo->pixelFormat, sourceVideoInfo->colorSpace, sourceVideoInfo->metadataType);
    return false;
}

std::shared_ptr<OpenGLContext> VideoProcessingCapiCapability::openglContext_ = nullptr;

VideoProcessing_ErrorCode VideoProcessingCapiCapability::OpenGLInit()
{
    auto status = SetupOpengl(openglContext_);
    CHECK_AND_RETURN_RET_LOG(status == static_cast<int>(VIDEO_PROCESSING_SUCCESS),
                             VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
                             "OpenGLInit SetupOpengl fail!");
    return VIDEO_PROCESSING_SUCCESS;
}

std::shared_ptr<OpenGLContext> VideoProcessingCapiCapability::GetOpenGLContext()
{
    return openglContext_;
}
