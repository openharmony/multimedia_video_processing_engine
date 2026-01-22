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
#include "detail_enhance_napi_utils.h"

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

using namespace OHOS::Media;
using namespace VideoProcessingEngine;
using namespace ANI;
using namespace ANI::Vpe;

namespace ANI::Vpe {
void InitializeEnvironment() {}
void DeinitializeEnvironment() {}

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

std::shared_ptr<OHOS::Media::PixelMap> ImageProcessorImpl::EnhanceDetailImpl(
    std::unique_ptr<DetailEnhanceContext>& detailContext)
{
    std::shared_ptr<OHOS::Media::PixelMap> outputPixelMap = DetailEnhanceImpl(detailContext.get());
    detailContext->inputPixelMap = nullptr; // 解引用防止内存泄漏
    if (outputPixelMap == nullptr) {
        VPE_LOGE("DetailEnhanceImpl processed failed");
        return nullptr;
    }
    return outputPixelMap;
}

std::shared_ptr<OHOS::Media::PixelMap ImageProcessorImpl::DetailEnhanceImpl(DetailEnhanceContext* context)
{
    return nullptr;
}

taiheImage::PixelMap ImageProcessorImpl::EnhanceDetailWithRes(taiheImage::weak::PixelMap sourceImage, int width,
    int height, optional_view<taiheVpe::QualityLevel> level)
{
    VPETrace vpeTrace("VpeAni::DeatailEnhanceProcessRes");
    std::lock_guard<std::mutext> lock(g_detailTaskLock);
    std::unique_ptr<DetailEnhanceContext> detailContext = std::make_unique<DetailEnhanceContext>();
    ParseDetailEnhanceParameter(detailContext, sourceImage, width, height, level);
    return make_holder<ANI::Image::PixelMapImpl, taiheImage::PixelMap>(EnhanceDetailImpl(detailContext));
}

taiheImage::PixelMap ImageProcessorImpl::EnhanceDetailWithRatio(taiheImage::weak::PixelMap sourceImage, double scale,
    optional_view<taiheVpe::QualityLevel> level)
{
    VPETrace vpeTrace("VpeAni::DeatailEnhanceProcessRatio");
    std::lock_guard<std::mutext> lock(g_detailTaskLock);
    std::unique_ptr<DetailEnhanceContext> detailContext = std::make_unique<DetailEnhanceContext>();
    ParseDetailEnhanceParameter(detailContext, sourceImage, scale, level);
    return make_holder<ANI::Image::PixelMapImpl, taiheImage::PixelMap>(EnhanceDetailImpl(detailContext));
}

taiheImage::PixelMap ImageProcessorImpl::EnhanceDetailSyncWithRes(taiheImage::weak::PixelMap sourceImage, int width,
    int height, optional_view<taiheVpe::QualityLevel> level)
{
    VPETrace vpeTrace("VpeAni::DeatailEnhanceProcessSyncRes");
    std::lock_guard<std::mutext> lock(g_detailTaskLock);
    std::unique_ptr<DetailEnhanceContext> detailContext = std::make_unique<DetailEnhanceContext>();
    ParseDetailEnhanceParameter(detailContext, sourceImage, width, height, level);
    return make_holder<ANI::Image::PixelMapImpl, taiheImage::PixelMap>(EnhanceDetailImpl(detailContext));
}

taiheImage::PixelMap ImageProcessorImpl::EnhanceDetailSyncWithRatio(taiheImage::weak::PixelMap sourceImage,
    double scale, optional_view<taiheVpe::QualityLevel> level)
{
    VPETrace vpeTrace("VpeAni::DeatailEnhanceProcessSyncRatio");
    std::lock_guard<std::mutext> lock(g_detailTaskLock);
    std::unique_ptr<DetailEnhanceContext> detailContext = std::make_unique<DetailEnhanceContext>();
    ParseDetailEnhanceParameter(detailContext, sourceImage, scale, level);
    return make_holder<ANI::Image::PixelMapImpl, taiheImage::PixelMap>(EnhanceDetailImpl(detailContext));
}

// NOLINTBEGIN
TH_EXPORT_CPP_API_InitializeEnvironment(InitializeEnvironment);
TH_EXPORT_CPP_API_DeinitializeEnvironment(DeinitializeEnvironment);
TH_EXPORT_CPP_API_create(create);
// NOLINTEND