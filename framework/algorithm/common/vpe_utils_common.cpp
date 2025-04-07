/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "vpe_utils_common.h"

#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include "securec.h"
#include "algorithm_errors.h"
#include "algorithm_utils.h"
#include "image_type.h"
#include "surface_buffer_impl.h"
#include "surface_type.h"
#include "vpe_log.h"
#include "vpe_model_path.h"

using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;

namespace {
constexpr int VPE_MODEL_FILE_MAX_SIZE = 20485760;
const std::unordered_map<OHOS::Media::PixelFormat, GraphicPixelFormat> FORMAT_MAP = {
    { OHOS::Media::PixelFormat::RGBA_8888,  GraphicPixelFormat::GRAPHIC_PIXEL_FMT_RGBA_8888 },
    { OHOS::Media::PixelFormat::BGRA_8888,  GraphicPixelFormat::GRAPHIC_PIXEL_FMT_BGRA_8888 },
    { OHOS::Media::PixelFormat::NV21,       GraphicPixelFormat::GRAPHIC_PIXEL_FMT_YCRCB_420_SP },
    { OHOS::Media::PixelFormat::NV12,       GraphicPixelFormat::GRAPHIC_PIXEL_FMT_YCBCR_420_SP },
};
float GetPixelFormatCoefficient(OHOS::Media::PixelFormat format)
{
    float coefficient;
    switch (format) {
        case OHOS::Media::PixelFormat::RGBA_8888:
        case OHOS::Media::PixelFormat::BGRA_8888:
            coefficient = 4.0; // 4.0 size coefficient
            break;
        case OHOS::Media::PixelFormat::NV12:
        case OHOS::Media::PixelFormat::NV21:
            coefficient = 1.0; // 1.0 size coefficient
            break;
        default:
            coefficient = 3.0; // 3.0 size coefficient
            break;
    }
    return coefficient;
}
}

sptr<SurfaceBuffer> VpeUtils::GetSurfaceBufferFromPixelMap(
    const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap)
{
    if (pixelmap->GetAllocatorType() == AllocatorType::DMA_ALLOC) {
        return reinterpret_cast<SurfaceBuffer*>(pixelmap->GetFd());
    }
    auto buffer = SurfaceBuffer::Create();
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, nullptr, "get surface buffer failed!");
    CHECK_AND_RETURN_RET_LOG(ConvertPixelmapToSurfaceBuffer(pixelmap, buffer), nullptr,
        "get surface buffer failed!");
    return buffer;
}

bool VpeUtils::ConvertPixelmapToSurfaceBuffer(
    const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap, sptr<SurfaceBuffer> bufferImpl)
{
    auto it = FORMAT_MAP.find(pixelmap->GetPixelFormat());
    if (it == FORMAT_MAP.end()) [[unlikely]] {
        VPE_LOGE("unsupported format: %{public}d", pixelmap->GetPixelFormat());
        return false;
    }
    BufferRequestConfig bfConfig = {};
    bfConfig.width = pixelmap->GetWidth();
    bfConfig.height = pixelmap->GetHeight();
    bfConfig.usage = BUFFER_USAGE_CPU_READ | BUFFER_USAGE_CPU_WRITE | BUFFER_USAGE_MEM_DMA | BUFFER_USAGE_MEM_MMZ_CACHE;
    bfConfig.strideAlignment = bfConfig.width;
    bfConfig.format = it->second;
    bfConfig.timeout = 0;
    bfConfig.colorGamut = GraphicColorGamut::GRAPHIC_COLOR_GAMUT_SRGB;
    bfConfig.transform = GraphicTransformType::GRAPHIC_ROTATE_NONE;
    CHECK_AND_RETURN_RET_LOG((bufferImpl->Alloc(bfConfig) == GSERROR_OK), false, "invalid OH_PixelmapNative image");
    bool ret;
    switch (pixelmap->GetPixelFormat()) {
        case OHOS::Media::PixelFormat::RGBA_8888:
        case OHOS::Media::PixelFormat::BGRA_8888:
            ret = CopyRGBAPixelmapToSurfaceBuffer(pixelmap, bufferImpl);
            break;
        case OHOS::Media::PixelFormat::NV12:
        case OHOS::Media::PixelFormat::NV21:
            ret = CopyNV12PixelmapToSurfaceBuffer(pixelmap, bufferImpl);
            break;
        default:
            ret = false;
            break;
    }
    return ret;
}

bool VpeUtils::ConvertSurfaceBufferToPixelmap(const sptr<SurfaceBuffer>& buffer,
    std::shared_ptr<OHOS::Media::PixelMap>& pixelmap)
{
    bool ret;
    switch (pixelmap->GetPixelFormat()) {
        case OHOS::Media::PixelFormat::RGBA_8888:
        case OHOS::Media::PixelFormat::BGRA_8888:
            ret = CopyRGBASurfaceBufferToPixelmap(buffer, pixelmap);
            break;
        case OHOS::Media::PixelFormat::NV12:
        case OHOS::Media::PixelFormat::NV21:
            ret = CopyNV12SurfaceBufferToPixelmap(buffer, pixelmap);
            break;
        default:
            ret = false;
            break;
    }
    return ret;
}

bool VpeUtils::CopyRGBASurfaceBufferToPixelmap(const sptr<SurfaceBuffer>& buffer,
    std::shared_ptr<OHOS::Media::PixelMap>& pixelmap)
{
    int32_t pixelmapStride = pixelmap->GetRowStride() > 1.5 * buffer->GetStride() ? // 1.5
        pixelmap->GetRowStride() / 2 : pixelmap->GetRowStride(); // 2
    int rowSize = std::min(buffer->GetStride(), pixelmapStride);
    for (int i = 0; i < buffer->GetHeight(); ++i) {
        if (memcpy_s(static_cast<uint8_t*>(pixelmap->GetWritablePixels()) + i * pixelmapStride, rowSize,
            static_cast<uint8_t*>(buffer->GetVirAddr()) + i * buffer->GetStride(), rowSize) != EOK) {
            VPE_LOGE("Failed to copy image buffer!");
            return false;
        }
    }
    return true;
}

bool VpeUtils::CopyRGBAPixelmapToSurfaceBuffer(const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap,
    sptr<SurfaceBuffer>& buffer)
{
    int32_t rowBufferSize = pixelmap->GetWidth() * GetPixelFormatCoefficient(pixelmap->GetPixelFormat());
    int32_t pixelmapStride = pixelmap->GetRowStride() > 1.5 * buffer->GetStride() ? // 1.5
        pixelmap->GetRowStride() / 2 : pixelmap->GetRowStride(); // 2
    for (int i = 0; i < pixelmap->GetHeight(); ++i) {
        if (memcpy_s(static_cast<uint8_t*>(buffer->GetVirAddr()) + i * buffer->GetStride(),
            rowBufferSize, pixelmap->GetPixels() + i * pixelmapStride, rowBufferSize) != EOK) {
            VPE_LOGE("copy data failed!");
            return false;
        }
    }
    return true;
}

bool VpeUtils::CopyNV12SurfaceBufferToPixelmap(const sptr<SurfaceBuffer>& buffer,
    std::shared_ptr<OHOS::Media::PixelMap>& pixelmap)
{
    YUVDataInfo yuvInfo;
    pixelmap->GetImageYUVInfo(yuvInfo);
    int32_t pixelmapStride = pixelmap->GetRowStride() > 1.5 * buffer->GetStride() ? // 1.5
        pixelmap->GetRowStride() / 2 : pixelmap->GetRowStride(); // 2
    int32_t pixelmapUvStride = static_cast<int32_t>((yuvInfo.uvStride > 0) ?
        static_cast<int32_t>(yuvInfo.uvStride) : pixelmapStride);
    int32_t pixelmapUvOffset = static_cast<int32_t>((yuvInfo.uvOffset > 0) ?
        static_cast<int32_t>(yuvInfo.uvOffset) : pixelmapStride * pixelmap->GetHeight());
    int32_t pixelmapUvHeight = static_cast<int32_t>((yuvInfo.uvHeight > 0) ?
        static_cast<int32_t>(yuvInfo.uvHeight) : (pixelmap->GetHeight() + 1) / 2); // 2
    
    int32_t bufferUvStride = buffer->GetStride();
    int32_t bufferUvOffset = buffer->GetStride() * buffer->GetHeight();
    int32_t bufferUvHeight = (buffer->GetHeight() + 1) / 2;
    void *planesInfoPtr = nullptr;
    buffer->GetPlanesInfo(&planesInfoPtr);
    auto planesInfo = static_cast<OH_NativeBuffer_Planes*>(planesInfoPtr);

    if (planesInfo != nullptr) {
        int idx = (pixelmap->GetPixelFormat() == OHOS::Media::PixelFormat::NV12) ? 1 : 2; // 1 NV12, 2 NV21
        bufferUvStride = static_cast<int32_t>(planesInfo->planes[idx].columnStride);
        bufferUvOffset = static_cast<int32_t>(planesInfo->planes[idx].offset);
        if (bufferUvStride != 0) {
            bufferUvHeight = static_cast<int32_t>(planesInfo->planes[idx].offset) / bufferUvStride;
        }
    }
    
    int32_t yStride = std::min(pixelmapStride, buffer->GetStride());
    for (int i = 0; i < pixelmap->GetHeight(); ++i) {
        if (memcpy_s(static_cast<uint8_t*>(pixelmap->GetWritablePixels()) + i * pixelmapStride, yStride,
            static_cast<uint8_t*>(buffer->GetVirAddr()) + i * buffer->GetStride(), yStride) != EOK) {
            VPE_LOGE("Failed to copy NV12 buffer to pixelmap!");
            return false;
        }
    }

    int32_t uvStride = std::min(pixelmapUvStride, bufferUvStride);
    int32_t uvHeight = std::min(pixelmapUvHeight, bufferUvHeight);
    for (int i = 0; i < uvHeight; ++i) {
        if (memcpy_s(static_cast<uint8_t*>(pixelmap->GetWritablePixels()) + pixelmapUvOffset + i * pixelmapUvStride,
            uvStride, static_cast<uint8_t*>(buffer->GetVirAddr()) + bufferUvOffset + i * bufferUvStride,
            uvStride) != EOK) {
            VPE_LOGE("Failed to copy NV12 buffer to pixelmap!");
            return false;
        }
    }
    return true;
}

bool VpeUtils::CopyNV12PixelmapToSurfaceBuffer(const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap,
    sptr<SurfaceBuffer>& buffer)
{
    YUVDataInfo yuvInfo;
    pixelmap->GetImageYUVInfo(yuvInfo);
    int32_t pixelmapStride = pixelmap->GetRowStride() > 1.5 * buffer->GetStride() ? // 1.5
        pixelmap->GetRowStride() / 2 : pixelmap->GetRowStride(); // 2
    int32_t pixelmapUvStride = static_cast<int32_t>((yuvInfo.uvStride > 0) ?
        static_cast<int32_t>(yuvInfo.uvStride) : pixelmapStride);
    int32_t pixelmapUvOffset = static_cast<int32_t>((yuvInfo.uvOffset > 0) ?
        static_cast<int32_t>(yuvInfo.uvOffset) : pixelmapStride * pixelmap->GetHeight());
    int32_t pixelmapUvHeight = static_cast<int32_t>((yuvInfo.uvHeight > 0) ?
        static_cast<int32_t>(yuvInfo.uvHeight) : (pixelmap->GetHeight() + 1) / 2);
    
    int32_t bufferUvStride = buffer->GetStride();
    int32_t bufferUvOffset = buffer->GetStride() * buffer->GetHeight();
    int32_t bufferUvHeight = (buffer->GetHeight() + 1) / 2; // 2
    void *planesInfoPtr = nullptr;
    buffer->GetPlanesInfo(&planesInfoPtr);
    auto planesInfo = static_cast<OH_NativeBuffer_Planes*>(planesInfoPtr);

    if (planesInfo != nullptr) {
        int idx = (pixelmap->GetPixelFormat() == OHOS::Media::PixelFormat::NV12) ? 1 : 2; // 1 NV12, 2 NV21
        bufferUvStride = static_cast<int32_t>(planesInfo->planes[idx].columnStride);
        bufferUvOffset = static_cast<int32_t>(planesInfo->planes[idx].offset);
        if (bufferUvStride != 0) {
            bufferUvHeight = static_cast<int32_t>(planesInfo->planes[idx].offset) / bufferUvStride;
        }
    }
    int32_t yStride = std::min(pixelmapStride, buffer->GetStride());
    for (int i = 0; i < pixelmap->GetHeight(); ++i) {
        if (memcpy_s(static_cast<uint8_t*>(buffer->GetVirAddr()) + i * buffer->GetStride(), yStride,
            static_cast<uint8_t*>(pixelmap->GetWritablePixels()) + i * pixelmapStride, yStride) != EOK) {
            VPE_LOGE("Failed to copy NV12 buffer to pixelmap!");
            return false;
        }
    }
    
    int32_t uvStride = std::min(pixelmapUvStride, bufferUvStride);
    int32_t uvHeight = std::min(pixelmapUvHeight, bufferUvHeight);
    for (int i = 0; i < uvHeight; ++i) {
        if (memcpy_s(static_cast<uint8_t*>(buffer->GetVirAddr()) + bufferUvOffset + i * bufferUvStride, uvStride,
            static_cast<uint8_t*>(pixelmap->GetWritablePixels()) + pixelmapUvOffset + i * pixelmapUvStride,
            uvStride) != EOK) {
            VPE_LOGE("Failed to copy NV12 buffer to pixelmap!");
            return false;
        }
    }
    return true;
}

bool VpeUtils::SetSurfaceBufferToPixelMap(const sptr<SurfaceBuffer>& buffer,
    std::shared_ptr<OHOS::Media::PixelMap>& pixelmap)
{
    if (pixelmap->GetAllocatorType() == AllocatorType::DMA_ALLOC) {
        return true;
    }
    return ConvertSurfaceBufferToPixelmap(buffer, pixelmap);
}

VPEAlgoErrCode VpeUtils::LoadModel(int modelKey, sptr<SurfaceBuffer>& modelBuffer)
{
    CHECK_AND_RETURN_RET_LOG(modelKey >= 0 && modelKey < VPE_MODEL_KEY_NUM, VPE_ALGO_ERR_INVALID_VAL,
        "Invalid input: modelKey is %{public}d(Expected:[0,%{public}d)!", modelKey, VPE_MODEL_KEY_NUM);
    std::string path = VPE_MODEL_PATHS[modelKey];
    std::unique_ptr<std::ifstream> fileStream = std::make_unique<std::ifstream>(path, std::ios::binary);
    CHECK_AND_RETURN_RET_LOG(fileStream->is_open(), VPE_ALGO_ERR_NO_MEMORY, "Failed to open %{public}s!", path.c_str());
    fileStream->seekg(0, std::ios::end);
    int fileSize = fileStream->tellg();
    fileStream->seekg(0, std::ios::beg);
    CHECK_AND_RETURN_RET_LOG(fileSize > 0 && fileSize <= VPE_MODEL_FILE_MAX_SIZE, VPE_ALGO_ERR_INVALID_VAL,
        "Invalid input: '%{public}s' size is %{public}d(Expected:(0,%{public}d]!", path.c_str(), fileSize,
        VPE_MODEL_FILE_MAX_SIZE);

    modelBuffer = SurfaceBuffer::Create();
    CHECK_AND_RETURN_RET_LOG(modelBuffer != nullptr, VPE_ALGO_ERR_NO_MEMORY,
        "Failed to create surface buffer for '%{public}s'!", path.c_str());
    BufferRequestConfig requestCfg;
    requestCfg.width = fileSize;
    requestCfg.height = 1;
    requestCfg.strideAlignment = fileSize;
    requestCfg.usage = BUFFER_USAGE_CPU_READ | BUFFER_USAGE_CPU_WRITE | BUFFER_USAGE_MEM_DMA;
    requestCfg.format = GRAPHIC_PIXEL_FMT_YCBCR_420_SP;
    requestCfg.timeout = 0;
    CHECK_AND_RETURN_RET_LOG(modelBuffer->Alloc(requestCfg) == GSERROR_OK, VPE_ALGO_ERR_NO_MEMORY,
        "Failed to alloc surface buffer size:%{public}d for '%{public}s'!", fileSize, path.c_str());
    fileStream->read(reinterpret_cast<char*>(modelBuffer->GetVirAddr()), fileSize);
    fileStream->close();
    VPE_LOGD("Load model '%{public}s' size:%{public}d", path.c_str(), fileSize);
    return VPE_ALGO_ERR_OK;
}
