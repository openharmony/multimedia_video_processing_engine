/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef VPE_TEST_CSC_SAMPLE_DEFINE_H
#define VPE_TEST_CSC_SAMPLE_DEFINE_H

#include "colorspace_converter.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
namespace CSCSampleDefine {
inline constexpr std::string_view METADATA_FILE = "/data/test/media/csc_metadata.bin";

inline constexpr std::string_view YUV_FILE = "/data/test/media/csc_yuv42010_2_frames.yuv";
inline constexpr int32_t YUV_FILE_PIXEL_FORMAT = GRAPHIC_PIXEL_FMT_RGBA_1010102 + 1;
inline constexpr int32_t OUTPUT_PIXEL_FORMAT = GRAPHIC_PIXEL_FMT_YCBCR_420_SP;
inline constexpr uint32_t YUV_FILE_FRAME_NUM = 2;
inline constexpr int32_t WIDTH = 3840;
inline constexpr int32_t HEIGHT = 2160;
inline constexpr int32_t ONE_FRAME_SIZE = 24883200;    // GRAPHIC_PIXEL_FMT_RGBA_1010102 + 1
inline constexpr CM_ColorSpaceInfo INPUT_COLORSPACE_INFO = {
    COLORPRIMARIES_BT2020, TRANSFUNC_HLG, MATRIX_BT2020, RANGE_LIMITED
};
inline constexpr CM_ColorSpaceInfo OUTPUT_COLORSPACE_INFO = {
    COLORPRIMARIES_BT709, TRANSFUNC_BT709, MATRIX_BT709, RANGE_LIMITED
};
} // namespace CSCSampleDefine
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
#endif // VPE_TEST_CSC_SAMPLE_DEFINE_H