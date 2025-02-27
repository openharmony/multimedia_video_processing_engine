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

#include "frame_info.h"
#include "vpe_log.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
FrameInfo::FrameInfo(const sptr<SurfaceBuffer> &buffer)
{
    CHECK_AND_RETURN_LOG(nullptr != buffer, "Get an invalid buffer");
    width = static_cast<uint32_t>(buffer->GetWidth());
    height = static_cast<uint32_t>(buffer->GetHeight());
    widthStride = static_cast<uint32_t>(buffer->GetStride());
    // 10 bit sample might be as 2bytes, so stride is 2 * (width + padding)
    pixelFormat = static_cast<GraphicPixelFormat>(buffer->GetFormat());
    bitDepth = BitDepth::BIT_DEPTH_8;
    int numTwo = 2;
    int numThree = 3;
    int numFour = 4;
    OH_NativeBuffer_Planes *planes = nullptr;
    if ((pixelFormat == GRAPHIC_PIXEL_FMT_RGBA_8888) || (pixelFormat == GRAPHIC_PIXEL_FMT_BGRA_8888)) {
        // RGBA8 has 4 channel per pixel which cost 4*8bit. // GetStride = w * sizeof(uint32)
        widthStride = static_cast<uint32_t>(buffer->GetStride() / numFour);
        heightStride = buffer->GetSize() / static_cast<uint32_t>(numFour) / widthStride;
    } else if ((pixelFormat == GRAPHIC_PIXEL_FMT_YCBCR_420_SP) || (pixelFormat == GRAPHIC_PIXEL_FMT_YCRCB_420_SP)
        || (pixelFormat == GRAPHIC_PIXEL_FMT_YCBCR_420_P) || (pixelFormat == GRAPHIC_PIXEL_FMT_YCRCB_420_P)) {
        // GetStride = w * sizeof(uint8) // yuv420(totalsize=3/2*h*w) calculate height stride
        widthStride = static_cast<uint32_t>(buffer->GetStride());
        heightStride = buffer->GetSize() * static_cast<uint32_t>(numTwo) /
                       static_cast<uint32_t>(numThree) / widthStride;
        if ((buffer->GetPlanesInfo(reinterpret_cast<void**>(&planes)) == OHOS::SURFACE_ERROR_OK) &&
            (planes != nullptr)) {
            if (planes->planeCount > 1) {
                heightStride = planes->planes[1].offset / planes->planes[0].columnStride;
            }
        }
    } else if (pixelFormat == GRAPHIC_PIXEL_FMT_RGBA_1010102) {
        // RGBA8 has 4 channel // GetStride = w * sizeof(uint8) (Get wByte nums) 4 channel
        bitDepth = BitDepth::BIT_DEPTH_10;
        widthStride = static_cast<uint32_t>(buffer->GetStride() / numFour);
        heightStride = buffer->GetSize() / static_cast<uint32_t>(numFour) / widthStride;
    } else if ((pixelFormat == GRAPHIC_PIXEL_FMT_YCBCR_P010) || (pixelFormat == GRAPHIC_PIXEL_FMT_YCRCB_P010)) {
        // 2 bit GetStride = w * sizeof(uint8) (Get wByte nums) // yuv420(totalsize=3*h*w) calculate height stride
        bitDepth = BitDepth::BIT_DEPTH_10;
        widthStride = static_cast<uint32_t>(buffer->GetStride() / numTwo);
        heightStride = buffer->GetSize() / static_cast<uint32_t>(numThree) / widthStride;
        if ((buffer->GetPlanesInfo(reinterpret_cast<void**>(&planes)) == OHOS::SURFACE_ERROR_OK) &&
            (planes != nullptr)) {
            if (planes->planeCount > 1) {
                heightStride = planes->planes[1].offset / planes->planes[0].columnStride;
            }
        }
    } else {
        heightStride = height;
    }
    ColorSpaceDescription::Create(buffer, colorSpace);
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
