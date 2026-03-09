/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "video_processing_engine_taihe.h"

#include "detail_enhance_napi_formal.h"

#include <algorithm>
#include <unordered_set>

#include "image_napi_utils.h"
#include "media_errors.h"
#include "memory_manager.h"
#include "native_avformat.h"
#include "pixelmap_native_impl.h"
#include "pixelmap_native.h"
#include "surface_buffer.h"

#include "detail_enhancer_common.h"
#include "vpe_log.h"
#include "vpe_trace.h"
#include "vpe_utils.h"

namespace {
constexpr int32_t MIN_RESOLUTION_DETAIL = 32; // min support resolution 32, consistent with fwk
constexpr int32_t MAX_RESOLUTION_DETAIL = 8192; // max support resolution 8192, consistent with fwk
static std::shared_ptr<OHOS::Media::VideoProcessingEngine::DetailEnhancerImage> g_detailEnh{};
static std::mutex g_detailTaskLock{std::mutex()};
}

using namespace taihe;
using namespace OHOS::Media;
using namespace VideoProcessingEngine;
using namespace ANI;
using namespace ANI::Vpe;

namespace ANI::Vpe {
void InitializeEnvironment() {}
void DeinitializeEnvironment() {}

void ImageProcessorImpl::ThrowExceptionError(const int32_t errCode, const std::string& errMsg)
{
    VPE_LOGE("errCode: %{public}d, errMsg: %{public}s", errCode, errMsg.c_str());
    taihe::set_business_error(errCode, errMsg);
}

taiheVpe::ImageProcessor create()
{
    return make_holder<ImageProcessorImpl, taiheVpe::ImageProcessor>();
}
} // namespace

void ImageProcessorImpl::ParseDetailEnhanceParameter(std::unique_ptr<DetailEnhanceContext>& detailContext,
    taiheImage::weak::PixelMap sourceImage, int width, int height, optional_view<taiheVpe::QualityLevel> level)
{
    if (detailContext == nullptr) {
        VPE_LOGE("detailContext == nullptr");
        return;
    }
    ANI::Image::PixelMapImpl* pixelMapImpl = reinterpret_cast<ANI::Image::PixelMapImpl*>(sourceImage->GetImplPtr());
    detailContext->xArg = width;
    detailContext->yArg = height;
    if (level.has_value()) {
        detailContext->qualityLevel = level.value();
    }
    detailContext->inputPixelMap = pixelMapImpl->GetNativePtr();
}

void ImageProcessorImpl::ParseDetailEnhanceParameter(std::unique_ptr<DetailEnhanceContext>& detailContext,
    taiheImage::weak::PixelMap sourceImage, double scale, optional_view<taiheVpe::QualityLevel> level)
{
    if (detailContext == nullptr) {
        VPE_LOGE("detailContext == nullptr");
        return;
    }
    ANI::Image::PixelMapImpl* pixelMapImpl = reinterpret_cast<ANI::Image::PixelMapImpl*>(sourceImage->GetImplPtr());
    std::shared_ptr<Media::PixelMap> inputPixelMap = pixelMapImpl->GetNativePtr();
    detailContext->xArg = inputPixelMap->GetWidth() * scale;
    detailContext->yArg = inputPixelMap->GetHeight() * scale;
    if (level.has_value()) {
        detailContext->qualityLevel = level.value();
    }
    detailContext->inputPixelMap = pixelMapImpl->GetNativePtr();
}

std::shared_ptr<PixelMap> ImageProcessorImpl::PrepareDstPixelMap(std::unique_ptr<DetailEnhanceContext>& context)
{
    CHECK_AND_RETURN_RET_LOG(context->inputPixelMap->GetWidth() >= MIN_RESOLUTION_DETAIL &&
        context->inputPixelMap->GetHeight() >= MIN_RESOLUTION_DETAIL &&
        context->inputPixelMap->GetWidth() <= MAX_RESOLUTION_DETAIL &&
        context->inputPixelMap->GetHeight() <= MAX_RESOLUTION_DETAIL,
        nullptr, "invalid resolution");
    InitializationOptions opts {
        .size = {
            .width = static_cast<int>(context->xArg),
            .height = static_cast<int>(context->yArg),
        },
    };
    VPE_LOGD("res:w %{public}d, h %{public}d, -> w %{public}d, h %{public}d",
        context->inputPixelMap->GetWidth(), context->inputPixelMap->GetHeight(),
        static_cast<int>(context->xArg), static_cast<int>(context->yArg));
    std::unique_ptr<PixelMap> outputPtr = context->inputPixelMap->Create(*context->inputPixelMap, opts);
    if (outputPtr == nullptr) {
        ThrowExceptionError(IMAGE_PROCESSING_ERROR_INVALID_VALUE, "create failed");
        return nullptr;
    }
    std::shared_ptr<PixelMap> dstPixelMap{std::move(outputPtr)};
    return dstPixelMap;
}

bool ImageProcessorImpl::SetDetailAlgoParam(int level)
{
    DetailEnhancerParameters param {
        .uri = "",
        .level = static_cast<DetailEnhancerLevel>(level),
    };
    if (g_detailEnh->SetParameter(param)!= VPE_ALGO_ERR_OK) {
        ThrowExceptionError(IMAGE_PROCESSING_ERROR_CREATE_FAILED, "set parameter failed");
        return false;
    }
    return true;
}

std::shared_ptr<OHOS::Media::PixelMap> ImageProcessorImpl::EnhanceDetail(
    std::unique_ptr<DetailEnhanceContext>& detailContext)
{
    if (detailContext == nullptr) {
        VPE_LOGE("detail context is nullptr");
        return nullptr;
    }
    std::shared_ptr<OHOS::Media::PixelMap> outputPixelMap = EnhanceDetailImpl(detailContext);
    detailContext->inputPixelMap = nullptr; // Dereferencing prevents memory leaks
    if (outputPixelMap == nullptr) {
        VPE_LOGE("DetailEnhance processed failed");
        return nullptr;
    }
    return outputPixelMap;
}

bool ImageProcessorImpl::InitDetailAlgo()
{
    VPETrace vpeTrace("ImageProcessorImpl::DetailEnhanceInitAlgo");
    if (g_detailEnh != nullptr) {
        VPE_LOGW("DetailEnhancerImage handle has created");
        return true;
    }
    g_detailEnh = DetailEnhancerImage::Create();
    CHECK_AND_RETURN_RET_LOG(g_detailEnh != nullptr, false, "create DetailEnhancerImage failed");
    return true;
}

bool ImageProcessorImpl::ConvertPixelmapToSurfaceBuffer(const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap,
    sptr<SurfaceBuffer>& bufferImpl)
{
    BufferRequestConfig bfConfig = {};
    bfConfig.width = pixelmap->GetWidth();
    bfConfig.height = pixelmap->GetHeight();
    bfConfig.usage = BUFFER_USAGE_CPU_READ | BUFFER_USAGE_CPU_WRITE | BUFFER_USAGE_MEM_DMA | BUFFER_USAGE_MEM_MMZ_CACHE;
    bfConfig.strideAlignment = bfConfig.width;
    bfConfig.format = GraphicPixelFormat::GRAPHIC_PIXEL_FMT_RGBA_8888;
    bfConfig.timeout = 0;
    bfConfig.colorGamut = GraphicColorGamut::GRAPHIC_COLOR_GAMUT_SRGB;
    bfConfig.transform = GraphicTransformType::GRAPHIC_ROTATE_NONE;
    CHECK_AND_RETURN_RET_LOG((bufferImpl->Alloc(bfConfig) == GSERROR_OK), false, "invalid OH_PixelmapNative image");
    return true;
}

sptr<SurfaceBuffer> ImageProcessorImpl::GetSurfaceBufferFromDMAPixelMap(
    const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap)
{
    CHECK_AND_RETURN_RET_LOG(pixelmap != nullptr, nullptr, "pixelmap == nullptr");
    if (pixelmap->GetAllocatorType() == AllocatorType::DMA_ALLOC) {
        return reinterpret_cast<SurfaceBuffer*>(pixelmap->GetFd());
    }
    auto buffer = SurfaceBuffer::Create();
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, nullptr, "get surface buffer failed!");
    CHECK_AND_RETURN_RET_LOG(ConvertPixelmapToSurfaceBuffer(pixelmap, buffer), nullptr,
        "get surface buffer failed!");
    return buffer;
}

std::shared_ptr<OHOS::Media::PixelMap> ImageProcessorImpl::EnhanceDetailImpl(
    std::unique_ptr<DetailEnhanceContext>& context)
{
    VPETrace vpeTrace("ImageProcessorImpl::DetailEnhanceImpl");
    if (context == nullptr) {
        VPE_LOGE("context == nullptr");
        return nullptr;
    }
    if (context->inputPixelMap->GetPixelFormat() == PixelFormat::YCBCR_P010 ||
        context->inputPixelMap->GetPixelFormat() == PixelFormat::YCRCB_P010) {
        VPE_LOGI("not support P010");
        return context->inputPixelMap;
    }
    if (!InitDetailAlgo()) {
        VPE_LOGE("init algo failed");
        ThrowExceptionError(IMAGE_PROCESSING_ERROR_CREATE_FAILED, "init algo failed");
        return nullptr;
    }
    if (!SetDetailAlgoParam(context->qualityLevel)) {
        VPE_LOGE("set detail param failed");
        return nullptr;
    }
    if (context->inputPixelMap == nullptr) {
        VPE_LOGE("*context->inputPixelMap == nullptr");
        return nullptr;
    }
    auto dstPixelMap = PrepareDstPixelMap(context);
    if (dstPixelMap == nullptr) {
        VPE_LOGE("move failed");
        return nullptr;
    }
    auto output = GetSurfaceBufferFromDMAPixelMap(dstPixelMap);
    auto input = GetSurfaceBufferFromDMAPixelMap(context->inputPixelMap);
    CHECK_AND_RETURN_RET_LOG((g_detailEnh != nullptr && g_detailEnh->Process(input, output) == VPE_ALGO_ERR_OK),
        nullptr, "process failed");
    return dstPixelMap;
}

taiheImage::PixelMap ImageProcessorImpl::EnhanceDetailWithRes(taiheImage::weak::PixelMap sourceImage, int width,
    int height, optional_view<taiheVpe::QualityLevel> level)
{
    VPETrace vpeTrace("VpeAni::DeatailEnhanceProcessRes");
    std::unique_ptr<DetailEnhanceContext> detailContext = std::make_unique<DetailEnhanceContext>();
    if (detailContext == nullptr) {
        VPE_LOGE("invalid context");
        return make_holder<ANI::Image::PixelMapImpl, taiheImage::PixelMap>();
    }
    ParseDetailEnhanceParameter(detailContext, sourceImage, width, height, level);
    return make_holder<ANI::Image::PixelMapImpl, taiheImage::PixelMap>(EnhanceDetail(detailContext));
}

taiheImage::PixelMap ImageProcessorImpl::EnhanceDetailWithRatio(taiheImage::weak::PixelMap sourceImage, double scale,
    optional_view<taiheVpe::QualityLevel> level)
{
    VPETrace vpeTrace("VpeAni::DeatailEnhanceProcessRatio");
    std::unique_ptr<DetailEnhanceContext> detailContext = std::make_unique<DetailEnhanceContext>();
    if (detailContext == nullptr) {
        VPE_LOGE("invalid context");
        return make_holder<ANI::Image::PixelMapImpl, taiheImage::PixelMap>();
    }
    ParseDetailEnhanceParameter(detailContext, sourceImage, scale, level);
    return make_holder<ANI::Image::PixelMapImpl, taiheImage::PixelMap>(EnhanceDetail(detailContext));
}

taiheImage::PixelMap ImageProcessorImpl::EnhanceDetailSyncWithRes(taiheImage::weak::PixelMap sourceImage, int width,
    int height, optional_view<taiheVpe::QualityLevel> level)
{
    VPETrace vpeTrace("VpeAni::DeatailEnhanceProcessSyncRes");
    std::lock_guard<std::mutex> lock(g_detailTaskLock);
    std::unique_ptr<DetailEnhanceContext> detailContext = std::make_unique<DetailEnhanceContext>();
    if (detailContext == nullptr) {
        VPE_LOGE("invalid context");
        return make_holder<ANI::Image::PixelMapImpl, taiheImage::PixelMap>();
    }
    ParseDetailEnhanceParameter(detailContext, sourceImage, width, height, level);
    return make_holder<ANI::Image::PixelMapImpl, taiheImage::PixelMap>(EnhanceDetail(detailContext));
}

taiheImage::PixelMap ImageProcessorImpl::EnhanceDetailSyncWithRatio(taiheImage::weak::PixelMap sourceImage,
    double scale, optional_view<taiheVpe::QualityLevel> level)
{
    VPETrace vpeTrace("VpeAni::DeatailEnhanceProcessSyncRatio");
    std::lock_guard<std::mutex> lock(g_detailTaskLock);
    std::unique_ptr<DetailEnhanceContext> detailContext = std::make_unique<DetailEnhanceContext>();
    if (detailContext == nullptr) {
        VPE_LOGE("invalid context");
        return make_holder<ANI::Image::PixelMapImpl, taiheImage::PixelMap>();
    }
    ParseDetailEnhanceParameter(detailContext, sourceImage, scale, level);
    return make_holder<ANI::Image::PixelMapImpl, taiheImage::PixelMap>(EnhanceDetail(detailContext));
}

// NOLINTBEGIN
TH_EXPORT_CPP_API_InitializeEnvironment(InitializeEnvironment);
TH_EXPORT_CPP_API_DeinitializeEnvironment(DeinitializeEnvironment);
TH_EXPORT_CPP_API_create(create);
// NOLINTEND