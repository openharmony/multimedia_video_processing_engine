/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef VPE_TEST_DETAIL_ENH_SAMPLE_DEFINE_H
#define VPE_TEST_DETAIL_ENH_SAMPLE_DEFINE_H

#include "detail_enhancer_image.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
inline constexpr std::string_view NV12_FILE = "/data/test/media/nv12Image_4096x3072_1.yuv";
inline constexpr std::string_view I420_FILE = "/data/test/media/yuv420Image_4096x3072_1.yuv";
inline constexpr std::string_view RGBA_FILE = "/data/test/media/rgbaImage_4096x3072_1.yuv";
inline constexpr std::string_view BGRA_FILE = "/data/test/media/bgraImage_4096x3072_1.yuv";
inline constexpr int32_t WIDTH = 1024;
inline constexpr int32_t HEIGHT = 1024;
inline constexpr int32_t MIN_WIDTH = 100;
inline constexpr int32_t MIN_HEIGHT = 100;

enum SUPPORT_FORMAT {
    YUV400 = 0,
    YVU420,
    YUV422,
    YUV444,
    RGB,
    RGBA,
    NV12,
    I420,
    BGRA,
    RGBA1010102
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
#endif // VPE_TEST_CSC_SAMPLE_DEFINE_H