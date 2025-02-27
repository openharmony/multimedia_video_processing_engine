/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B3F
#undef LOG_TAG
#define LOG_TAG "VpeNapi"

#include "detail_enhance_napi_formal.h"

#include <algorithm>

#include "image_processing_types.h"
#include "image_napi_utils.h"
#include "media_errors.h"
#include "memory_manager.h"
#include "native_avformat.h"
#include "pixelmap_native_impl.h"
#include "pixelmap_native.h"

#include "detail_enhancer_common.h"
#include "vpe_log.h"
#include "vpe_trace.h"
#include "vpe_utils.h"

namespace {
constexpr uint32_t NUM_0 = 0;
constexpr uint32_t NUM_1 = 1;
constexpr uint32_t NUM_2 = 2;
constexpr uint32_t NUM_3 = 3;
constexpr uint32_t NUM_4 = 4;
constexpr uint32_t NUM_5 = 5;
constexpr uint32_t NUM_6 = 6;
constexpr int32_t NEW_INSTANCE_ARGC = 1;
const std::string CLASS_NAME = "ImageProcessor";
static std::mutex g_imageProcessorMutex{std::mutex()};
static std::mutex g_detailLock{std::mutex()};
static std::mutex g_contrastLock{std::mutex()};
}

namespace OHOS {
namespace Media {
using namespace VideoProcessingEngine;
thread_local napi_ref VpeNapi::constructor_ = nullptr;
thread_local napi_ref VpeNapi::qualityLevelTypeRef_ = nullptr;
thread_local std::shared_ptr<VpeNapi::DetailEnhanceContext> VpeNapi::detailContext_ = nullptr;
thread_local std::shared_ptr<VpeNapi::ContrastEnhanceContext> VpeNapi::contrastContext_ = nullptr;
static std::shared_ptr<DetailEnhancerImage> g_detailEnh{};
static std::shared_ptr<ContrastEnhancerImage> g_contrastEnh{};

struct QualityLevelEnum {
    std::string name;
    int32_t numVal;
    std::string strVal;
};
static std::vector<struct QualityLevelEnum> g_qualityLevels = {
    {"NONE",   OHOS::Media::VideoProcessingEngine::DETAIL_ENH_LEVEL_NONE,   ""},
    {"LOW",    OHOS::Media::VideoProcessingEngine::DETAIL_ENH_LEVEL_LOW,    ""},
    {"MEDIUM", OHOS::Media::VideoProcessingEngine::DETAIL_ENH_LEVEL_MEDIUM, ""},
    {"HIGH",   OHOS::Media::VideoProcessingEngine::DETAIL_ENH_LEVEL_HIGH,   ""},
};

void VpeNapi::ThrowExceptionError(napi_env env, const int32_t errCode, const std::string errMsg)
{
    std::string errCodeStr = std::to_string(errCode);
    napi_throw_error(env, errCodeStr.c_str(), errMsg.c_str());
}

bool VpeNapi::PrepareNapiEnv(napi_env env, napi_callback_info info, NapiValues* nVal)
{
    CHECK_AND_RETURN_RET_LOG(nVal != nullptr, false, "nVal == nullptr");
    if (napi_get_undefined(env, &(nVal->result)) != napi_ok) {
        VPE_LOGE("Get undefined result failed");
        return false;
    }
    nVal->status = napi_get_cb_info(env, info, &(nVal->argc), nVal->argv, &(nVal->thisVar), nullptr);
    if (nVal->status != napi_ok) {
        VPE_LOGE("fail to napi_get_cb_info");
        return false;
    }
    return true;
}

ImageType VpeNapi::ParserImageType(napi_env env, napi_value argv)
{
    napi_value constructor = nullptr;
    napi_value global = nullptr;
    napi_status ret = napi_invalid_arg;
    napi_get_global(env, &global);
    ret = napi_get_named_property(env, global, "PixelMap", &constructor);
    if (ret != napi_ok) {
        VPE_LOGI("Get VpeNapi property failed!");
    }
    bool isInstance = false;
    ret = napi_instanceof(env, argv, constructor, &isInstance);
    if (ret == napi_ok && isInstance) {
        return ImageType::TYPE_PIXEL_MAP;
    }
    return ImageType::TYPE_UNKNOWN;
}

bool VpeNapi::ConfigResolutionBasedOnRatio(napi_env env, napi_value& nVal,
    std::shared_ptr<VpeNapi::DetailEnhanceContext> context)
{
    CHECK_AND_RETURN_RET_LOG(context != nullptr, false, "context == nullptr");
    double zoomRatio;
    if (napi_ok != napi_get_value_double(env, nVal, &zoomRatio)) {
        VPE_LOGE("Arg 1 type mismatch");
        return false;
    }
    CHECK_AND_RETURN_RET_LOG(context->inputPixelMap != nullptr, false, "context->inputPixelMap == nullptr");
    context->xArg = zoomRatio * context->inputPixelMap->GetWidth();
    context->yArg = zoomRatio * context->inputPixelMap->GetHeight();
    VPE_LOGE("config resolution with ratio :%{public}d, %{public}d",
        static_cast<int>(context->xArg), static_cast<int>(context->yArg));
    return true;
}

bool VpeNapi::ConfigResolution(napi_env env, napi_value& width, napi_value& height,
    std::shared_ptr<VpeNapi::DetailEnhanceContext> context)
{
    CHECK_AND_RETURN_RET_LOG(context != nullptr, false, "context == nullptr");
    CHECK_AND_RETURN_RET_LOG(napi_ok == napi_get_value_double(env, width, &(context->xArg)),
        false, "Arg 1 type mismatch");
    CHECK_AND_RETURN_RET_LOG(napi_ok == napi_get_value_double(env, height, &(context->yArg)),
        false, "Arg 2 type mismatch");
    return true;
}

bool VpeNapi::ParseDetailEnhanceParameter(napi_env env, napi_callback_info info)
{
    VPETrace vpeTrace("VpeNapi::DetailEnhanceParseParameter");
    std::lock_guard<std::mutex> lock(g_detailLock);
    CHECK_AND_RETURN_RET_LOG(detailContext_ != nullptr, false, "detailContext_ == nullptr");
    NapiValues nVal;
    nVal.argc = NUM_4; // Use the maximum value to initialize argc before executing PrepareNapiEnv
    napi_value argValue[NUM_4] = {0};
    nVal.argv = argValue;
    CHECK_AND_RETURN_RET_LOG(PrepareNapiEnv(env, info, &nVal), false, "PrepareNapiEnv failed");
    if (nVal.argc != NUM_2 && nVal.argc != NUM_3 && nVal.argc != NUM_4) {
        VPE_LOGE("Invalid args count %{public}zu", nVal.argc);
        return false;
    }
    if (ParserImageType(env, argValue[NUM_0]) == ImageType::TYPE_PIXEL_MAP) {
        detailContext_->inputPixelMap = PixelMapNapi::GetPixelMap(env, argValue[NUM_0]);
    } else {
        VPE_LOGE("args0 is not pixelMap!");
        return false;
    }
    CHECK_AND_RETURN_RET_LOG(detailContext_->inputPixelMap != nullptr, false, "inputPixelMap is nullptr!");
    if (nVal.argc == NUM_2) { // 2 parameter: pixelmap scaleRatio
        CHECK_AND_RETURN_RET_LOG(ConfigResolutionBasedOnRatio(env, nVal.argv[NUM_1], detailContext_),
            false, "ConfigResolutionBasedOnRatio failed");
        detailContext_->qualityLevel = DETAIL_ENH_LEVEL_LOW; // default as low level
    }
    if (nVal.argc == NUM_3) { // 3 parameter: pixelmap scaleRatio level / pixelmap x y
        double valueToCheck = 0;
        CHECK_AND_RETURN_RET_LOG(napi_get_value_double(env, nVal.argv[NUM_2], &valueToCheck) == napi_ok,
            false, "failed to parse");
        if (valueToCheck >= 0 && valueToCheck <= 3) { // if valueToCheck in [0,3], valueToCheck should be level.
            CHECK_AND_RETURN_RET_LOG(ConfigResolutionBasedOnRatio(env, nVal.argv[NUM_1], detailContext_), false,
                "ConfigResolutionBasedOnRatio failed");
            detailContext_->qualityLevel = static_cast<int>(valueToCheck);
        } else {
            CHECK_AND_RETURN_RET_LOG(ConfigResolution(env, nVal.argv[NUM_1], nVal.argv[NUM_2], detailContext_),
                false, "ConfigResolution failed");
            detailContext_->qualityLevel = DETAIL_ENH_LEVEL_LOW; // default as low level
        }
    }
    if (nVal.argc == NUM_4) { // 4 parameter: pixelmap x y level
        CHECK_AND_RETURN_RET_LOG(ConfigResolution(env, nVal.argv[NUM_1], nVal.argv[NUM_2], detailContext_),
            false, "ConfigResolution failed");
        CHECK_AND_RETURN_RET_LOG(napi_ok == napi_get_value_int32(env, nVal.argv[NUM_3],
            &(detailContext_->qualityLevel)), false, "Arg 3 type mismatch");
    }
    return true;
}

napi_value VpeNapi::InitializeEnvironment(napi_env env, napi_callback_info info)
{
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

napi_value VpeNapi::DeinitializeEnvironment(napi_env env, napi_callback_info info)
{
    VPETrace vpeTrace("VpeNapi::DetailEnhanceDeinitializeEnvironment");
    std::lock_guard<std::mutex> lock(g_detailLock);
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

void VpeNapi::SetDstPixelMapInfo(PixelMap& source, void* dstPixels, uint32_t dstPixelsSize,
    std::unique_ptr<AbsMemory>& memory, PixelMap& dstPixelMap)
{
    AllocatorType sourceType = source.GetAllocatorType();
    if (sourceType != AllocatorType::DMA_ALLOC) {
        VPE_LOGW("only support DMA");
        return;
    }
    if (memory == nullptr) {
        VPE_LOGW("Invalid memory");
        return;
    }
    dstPixelMap.SetPixelsAddr(dstPixels, memory->extend.data, memory->data.size, sourceType, nullptr);
    if (source.GetAllocatorType() == AllocatorType::DMA_ALLOC && source.IsHdr()) {
        sptr<SurfaceBuffer> sourceSurfaceBuffer(reinterpret_cast<SurfaceBuffer*> (source.GetFd()));
        sptr<SurfaceBuffer> dstSurfaceBuffer(reinterpret_cast<SurfaceBuffer*> (dstPixelMap.GetFd()));
        VpeUtils::CopySurfaceBufferInfo(sourceSurfaceBuffer, dstSurfaceBuffer);
    }
    OHOS::ColorManager::ColorSpace colorspace = source.InnerGetGrColorSpace();
    dstPixelMap.InnerSetColorSpace(colorspace);
}

bool VpeNapi::AllocMemory(PixelMap& source, PixelMap& dstPixelMap, const InitializationOptions& opt)
{
    if (source.GetPixels() == nullptr) {
        VPE_LOGE("pixels of source are not available");
        return false;
    }
    int32_t bufferSize = source.GetByteCount();
    if (bufferSize <= 0) {
        VPE_LOGE("CopyPixelMap parameter bufferSize:[%{public}d] error.", bufferSize);
        return false;
    }
    ImageInfo dstImageInfo;
    dstPixelMap.GetImageInfo(dstImageInfo);
    MemoryData memoryData = {nullptr, static_cast<size_t>(bufferSize), "Copy ImageData",
        dstImageInfo.size, dstImageInfo.pixelFormat};
    std::unique_ptr<AbsMemory> memory = MemoryManager::CreateMemory(source.GetAllocatorType(), memoryData);
    if (memory == nullptr) {
        VPE_LOGE("invalid memory");
        return false;
    }
    void* dstPixels = memory->data.data;
    if (dstPixels == nullptr) {
        VPE_LOGE("source crop allocate memory fail allocatetype: %{public}d ", source.GetAllocatorType());
        return false;
    }
    SetDstPixelMapInfo(source, dstPixels, static_cast<size_t>(bufferSize), memory, dstPixelMap);
    return true;
}

std::unique_ptr<PixelMap> VpeNapi::CreateDstPixelMap(PixelMap& source, const InitializationOptions& opts)
{
    if (source.GetAllocatorType() != AllocatorType::DMA_ALLOC) {
        VPE_LOGE("alloc type of source is not dma, create with default method");
        return source.Create(source, opts);
    }
    std::unique_ptr<PixelMap> dstPixelMap = std::make_unique<PixelMap>();
    if (dstPixelMap == nullptr) {
        VPE_LOGE("create pixelmap pointer fail");
        return nullptr;
    }
    ImageInfo srcImageInfo;
    source.GetImageInfo(srcImageInfo);
    ImageInfo dstImageInfo = {
        .size = opts.size,
        .pixelFormat = srcImageInfo.pixelFormat,
        .alphaType = srcImageInfo.alphaType,
    };
    if (dstPixelMap->SetImageInfo(dstImageInfo) != SUCCESS) {
        return nullptr;
    }
    if (!AllocMemory(source, *dstPixelMap.get(), opts)) {
        return nullptr;
    }
    return dstPixelMap;
}

bool VpeNapi::ConvertPixelmapToSurfaceBuffer(const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap,
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

sptr<SurfaceBuffer> VpeNapi::GetSurfaceBufferFromDMAPixelMap(
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

napi_value VpeNapi::Create(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    napi_value constructor = nullptr;
    napi_status status = napi_ok;
    status = napi_get_reference_value(env, constructor_, &constructor);
    if (status == napi_ok) {
        size_t argc = NEW_INSTANCE_ARGC;
        napi_value argv[NEW_INSTANCE_ARGC] = { 0 };
        status = napi_new_instance(env, constructor, argc, argv, &result);
    }
    if (status != napi_ok) {
        VPE_LOGE("create instance failed");
        ThrowExceptionError(env, IMAGE_PROCESSING_ERROR_CREATE_FAILED, "create instance failed");
        return nullptr;
    }
    VPE_LOGE("create done");
    return result;
}

std::shared_ptr<PixelMap> VpeNapi::PrepareDstPixelMap(napi_env env, DetailEnhanceContext* context)
{
    CHECK_AND_RETURN_RET_LOG(context->inputPixelMap->GetWidth() != 0 && context->inputPixelMap->GetHeight() != 0,
        nullptr, "invalid resolution");
    float ratio = std::min(static_cast<float>(context->xArg) / static_cast<float>(context->inputPixelMap->GetWidth()),
        static_cast<float>(context->yArg) / static_cast<float>(context->inputPixelMap->GetHeight()));
    InitializationOptions opts {
        .size = {
            .width = ratio < 1.0 ? static_cast<int>(context->xArg) :
                static_cast<int>(context->inputPixelMap->GetWidth()),
            .height = ratio < 1.0 ? static_cast<int>(context->yArg) :
                static_cast<int>(context->inputPixelMap->GetHeight()),
        },
    };
    std::unique_ptr<PixelMap> outputPtr = CreateDstPixelMap(*context->inputPixelMap, opts);
    if (outputPtr == nullptr) {
        ThrowExceptionError(env, IMAGE_PROCESSING_ERROR_INVALID_VALUE, "create failed");
        return nullptr;
    }
    std::shared_ptr<PixelMap> dstPixelMap{std::move(outputPtr)};
    return dstPixelMap;
}

bool VpeNapi::InitDetailAlgo(napi_env env, int level)
{
    VPETrace vpeTrace("VpeNapi::DetailEnhanceInitAlgo");
    CHECK_AND_RETURN_RET_LOG(g_detailEnh == nullptr, true, "DetailEnhancerImage handle has created");
    g_detailEnh = DetailEnhancerImage::Create();
    CHECK_AND_RETURN_RET_LOG(g_detailEnh != nullptr, false, "create DetailEnhancerImage failed");
    DetailEnhancerParameters param {
        .uri = "",
        .level = static_cast<DetailEnhancerLevel>(level),
    };
    if (g_detailEnh->SetParameter(param)!= VPE_ALGO_ERR_OK) {
        ThrowExceptionError(env, IMAGE_PROCESSING_ERROR_CREATE_FAILED, "set parameter failed");
        return false;
    }
    return true;
}

std::shared_ptr<PixelMap> VpeNapi::DetailEnhanceImpl(napi_env env, DetailEnhanceContext* context)
{
    VPETrace vpeTrace("VpeNapi::DetailEnhanceImpl");
    if (context == nullptr) {
        VPE_LOGE("context == nullptr");
        return nullptr;
    }
    if (!InitDetailAlgo(env, context->qualityLevel)) {
        VPE_LOGE("init algo failed");
        ThrowExceptionError(env, IMAGE_PROCESSING_ERROR_CREATE_FAILED, "init algo failed");
        return nullptr;
    }
    if (context->inputPixelMap == nullptr) {
        VPE_LOGE("*context->inputPixelMap == nullptr");
        return nullptr;
    }
    auto dstPixelMap = PrepareDstPixelMap(env, context);
    if (dstPixelMap == nullptr) {
        VPE_LOGE("move failed");
        return nullptr;
    }
    auto output = GetSurfaceBufferFromDMAPixelMap(dstPixelMap);
    auto input = GetSurfaceBufferFromDMAPixelMap(context->inputPixelMap);
    CHECK_AND_RETURN_RET_LOG((g_detailEnh != nullptr && g_detailEnh->Process(input, output) == VPE_ALGO_ERR_OK),
        nullptr, "process failed");
    VPE_LOGI("process done");
    return dstPixelMap;
}

napi_value VpeNapi::EnhanceDetail(napi_env env, napi_callback_info info)
{
    VPETrace vpeTrace("VpeNapi::DetailEnhanceProcess");
    if (detailContext_ == nullptr) {
        detailContext_ = std::make_shared<DetailEnhanceContext>();
    }
    CHECK_AND_RETURN_RET_LOG(detailContext_ != nullptr, nullptr, "context == nullptr");
    napi_deferred deferred;
    napi_value promise;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    CHECK_AND_RETURN_RET_LOG(ParseDetailEnhanceParameter(env, info), nullptr, "parse parameter failed");
    detailContext_->deferred = deferred;
    napi_value resourceName;
    napi_create_string_latin1(env, "Asynchronous processing", NAPI_AUTO_LENGTH, &resourceName);
    napi_status status = napi_create_async_work(env, nullptr, resourceName,
        [](napi_env env, void* data) {
            DetailEnhanceContext* innerAsyncContext = reinterpret_cast<DetailEnhanceContext*>(data);
            if (innerAsyncContext == nullptr) {
                ThrowExceptionError(env, IMAGE_PROCESSING_ERROR_PROCESS_FAILED, "innerAsyncContext is nullptr");
                return;
            }
            innerAsyncContext->outputPixelMap = DetailEnhanceImpl(env, innerAsyncContext);
        },
        [](napi_env env, napi_status status, void* data) {
            DetailEnhanceContext* innerAsyncContext = reinterpret_cast<DetailEnhanceContext*>(data);
            if (innerAsyncContext == nullptr) {
                ThrowExceptionError(env, IMAGE_PROCESSING_ERROR_PROCESS_FAILED, "innerAsyncContext is nullptr");
                return;
            }
            napi_value outputPixelMapNapi = (innerAsyncContext->outputPixelMap == nullptr) ?
                nullptr : PixelMapNapi::CreatePixelMap(env, innerAsyncContext->outputPixelMap);
            if (outputPixelMapNapi == nullptr) {
                VPE_LOGI("outputPixelMap is nullptr");
                return;
            }
            if (innerAsyncContext->deferred) {
                napi_resolve_deferred(env, innerAsyncContext->deferred, outputPixelMapNapi);
            } else {
                napi_value callback = nullptr;
                napi_get_reference_value(env, innerAsyncContext->callbackRef, &callback);
                napi_call_function(env, nullptr, callback, 1, &(outputPixelMapNapi), nullptr);
                napi_delete_reference(env, innerAsyncContext->callbackRef);
                innerAsyncContext->callbackRef = nullptr;
            }
            napi_delete_async_work(env, innerAsyncContext->asyncWork);
            delete innerAsyncContext;
        }, reinterpret_cast<void*>(detailContext_.get()), &detailContext_->asyncWork);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "create aysnc failed");
    napi_queue_async_work(env, detailContext_->asyncWork);
    return promise;
}

napi_value VpeNapi::EnhanceDetailSync(napi_env env, napi_callback_info info)
{
    VPETrace vpeTrace("VpeNapi::DetailEnhanceProcess");
    if (detailContext_ == nullptr) {
        detailContext_ = std::make_shared<DetailEnhanceContext>();
    }
    CHECK_AND_RETURN_RET_LOG(detailContext_ != nullptr, nullptr, "context == nullptr");
    CHECK_AND_RETURN_RET_LOG(ParseDetailEnhanceParameter(env, info), nullptr, "parse parameter failed");
    std::shared_ptr<PixelMap> outputPixelMap = DetailEnhanceImpl(env, detailContext_.get());
    if (outputPixelMap == nullptr) {
        VPE_LOGE("DetailEnhanceImpl processed failed");
        return nullptr;
    }
    napi_value outputPixelMapNapi = PixelMapNapi::CreatePixelMap(env, outputPixelMap);
    return outputPixelMapNapi;
}

bool VpeNapi::UpdateMetadataBasedOnLcd(ContrastEnhanceContext* context)
{
    sptr<SurfaceBuffer> surfaceBuffer = GetSurfaceBufferFromDMAPixelMap(context->inputPixelMap);
    return g_contrastEnh->UpdateMetadataBasedOnLcd(context->displayArea, context->lcdWidth, context->lcdHeight,
        surfaceBuffer);
}

bool VpeNapi::UpdateMetadataBasedOnDetail(ContrastEnhanceContext* context)
{
    OHOS::Rect completePixelmapArea = {
        .x = 0,
        .y = 0,
        .w = context->oriWidth,
        .h = context->oriHeight,
    };
    sptr<SurfaceBuffer> surfaceBuffer = GetSurfaceBufferFromDMAPixelMap(context->inputPixelMap);
    return g_contrastEnh->UpdateMetadataBasedOnDetail(context->displayArea, context->curPixelmapArea,
        completePixelmapArea, surfaceBuffer, context->fullRatio);
}

napi_value VpeNapi::SetDetailImage(napi_env env, napi_callback_info info)
{
    VPETrace vpeTrace("VpeNapi::DetailEnhanceProcess");
    if (contrastContext_ == nullptr) {
        contrastContext_ = std::make_shared<ContrastEnhanceContext>();
        VPE_LOGI("create new contrast context");
    }
    CHECK_AND_RETURN_RET_LOG(contrastContext_ != nullptr, nullptr, "context == nullptr");
    NapiValues nVal;
    CHECK_AND_RETURN_RET_LOG(ParseDetailImageParameter(env, info, nVal), nullptr, "parse parameter failed");
    // 不管是不是区域解码，都先使用直方图的结果进行展示，之后再转变成bitmap的结果
    UpdateMetadataBasedOnLcd(contrastContext_.get());
    CallCallback(env, contrastContext_.get());
    // 如果在计算完成前坐标位置已经发生了变化，则不需要继续进行计算
    CHECK_AND_RETURN_RET_LOG(!contrastContext_->genFinalEffect, nullptr, "It's still moving. Stop processing");
    if (contrastContext_->callbackRef == nullptr) {
        napi_create_promise(env, &(contrastContext_->deferred), &(nVal.result));
    }
    napi_value resourceName;
    napi_create_string_latin1(env, "Asynchronous processing", NAPI_AUTO_LENGTH, &resourceName);
    napi_status status = napi_create_async_work(
        env, nullptr, resourceName,
        [](napi_env env, void* data) {
            ContrastEnhanceContext* innerAsyncContext = reinterpret_cast<ContrastEnhanceContext*>(data);
            if (innerAsyncContext == nullptr) {
                ThrowExceptionError(env, IMAGE_PROCESSING_ERROR_PROCESS_FAILED, "innerAsyncContext == nullptr");
                return;
            }
            UpdateMetadataBasedOnDetail(innerAsyncContext);
        },
        [](napi_env env, napi_status status, void* data) {
            ContrastEnhanceContext* innerAsyncContext = reinterpret_cast<ContrastEnhanceContext*>(data);
            CallCallback(env, innerAsyncContext);
            if (status != napi_ok) {
                VPE_LOGE("process failed");
            }
            VPE_LOGI("process detail image done");
        },
        (void*)(contrastContext_.get()), &contrastContext_->asyncWork);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "create aysnc failed");
    status = napi_queue_async_work(env, contrastContext_->asyncWork);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "queue aysnc work failed");
    return nullptr;
}

bool VpeNapi::GenerateRegionHist(napi_env env, ContrastEnhanceContext* context)
{
    CHECK_AND_RETURN_RET_LOG(context != nullptr, false, "context == nullptr");
    CHECK_AND_RETURN_RET_LOG(context->lcdPixelMap != nullptr, false, "lcdPixelMap == nullptr");
    auto input = GetSurfaceBufferFromDMAPixelMap(context->lcdPixelMap);
    CHECK_AND_RETURN_RET_LOG(input != nullptr, false, "input == nullptr");
    CHECK_AND_RETURN_RET_LOG(g_contrastEnh != nullptr, false, "g_contrastEnh == nullptr");
    if (g_contrastEnh->GetRegionHist(input) != 0) { // 9ms
        VPE_LOGE("processed failed. Clear LUT history");
        return false;
    }
    return true;
}

bool VpeNapi::InitContrastAlgo(napi_env env)
{
    VPETrace vpeTrace("VpeNapi::DetailEnhanceInitAlgo");
    CHECK_AND_RETURN_RET_LOG(g_contrastEnh == nullptr, true, "ContrastEnhancerImage handle has created");
    g_contrastEnh = ContrastEnhancerImage::Create();
    CHECK_AND_RETURN_RET_LOG(g_contrastEnh != nullptr, false, "create ContrastEnhancerImage failed");
    ContrastEnhancerParameters param {
        .uri = "",
    };
    if (g_contrastEnh->SetParameter(param)!= VPE_ALGO_ERR_OK) {
        ThrowExceptionError(env, IMAGE_PROCESSING_ERROR_CREATE_FAILED, "set parameter failed");
        return false;
    }
    return true;
}

bool VpeNapi::ParseRect(napi_env env, napi_value nVal, OHOS::Rect& rect)
{
    CHECK_AND_RETURN_RET_LOG(GET_INT32_BY_NAME(nVal, "x", rect.x), false, "Failed to parse start pos X");
    CHECK_AND_RETURN_RET_LOG(GET_INT32_BY_NAME(nVal, "y", rect.y), false, "Failed to parse start pos Y");
    napi_value tmpValue = nullptr;
    CHECK_AND_RETURN_RET_LOG(GET_NODE_BY_NAME(nVal, "size", tmpValue), false, "Failed to parse resolution of rect");
    CHECK_AND_RETURN_RET_LOG(GET_INT32_BY_NAME(tmpValue, "height", rect.h), false, "Failed to parse height of rect");
    CHECK_AND_RETURN_RET_LOG(GET_INT32_BY_NAME(tmpValue, "width", rect.w), false, "Failed to parse width of rect");
    return true;
}

bool VpeNapi::ParseSize(napi_env env, napi_value nVal)
{
    napi_value tmpValue = nullptr;
    CHECK_AND_RETURN_RET_LOG(GET_NODE_BY_NAME(nVal, "size", tmpValue), false, "Failed to parse resolution of rect");
    CHECK_AND_RETURN_RET_LOG(GET_INT32_BY_NAME(nVal, "height", contrastContext_->oriHeight),
        false, "Failed to parse height of rect");
    CHECK_AND_RETURN_RET_LOG(GET_INT32_BY_NAME(nVal, "width", contrastContext_->oriWidth),
        false, "Failed to parse width of rect");
    return true;
}

bool VpeNapi::ParseDetailImageParameter(napi_env env, napi_callback_info info, NapiValues& nVal)
{
    VPETrace vpeTrace("VpeNapi::DetailEnhance");
    std::lock_guard<std::mutex> lock(g_contrastLock);
    nVal.argc = NUM_6;
    napi_value argValue[NUM_6] = {0};
    nVal.argv = argValue;
    if (!PrepareNapiEnv(env, info, &nVal)) {
        return false;
    }
    if (nVal.argc != NUM_6) {
        VPE_LOGE("Invalid args count %{public}zu", nVal.argc);
        return false;
    } else {
        CHECK_AND_RETURN_RET_LOG(ParserImageType(env, argValue[NUM_0]) == ImageType::TYPE_PIXEL_MAP,
            false, "Arg 0 type mismatch");
        contrastContext_->inputPixelMap = PixelMapNapi::GetPixelMap(env, argValue[NUM_0]);
        CHECK_AND_RETURN_RET_LOG(contrastContext_->inputPixelMap != nullptr, false,
            "contrastContext_->srcPixelMap == nullptr, resuse history");
        CHECK_AND_RETURN_RET_LOG(napi_get_value_int32(env, nVal.argv[NUM_1],
            &(contrastContext_->pixelmapId)) == napi_ok, false, "Arg 1 type mismatch");
        CHECK_AND_RETURN_RET_LOG(ParseRect(env, argValue[NUM_2], contrastContext_->curPixelmapArea), false,
            "parse pixelmap area failed");
        CHECK_AND_RETURN_RET_LOG(ParseRect(env, argValue[NUM_3], contrastContext_->displayArea), false,
            "parse display area failed");
        CHECK_AND_RETURN_RET_LOG(ParseSize(env, argValue[NUM_4]), false, "parse resolution of original image failed");
        CHECK_AND_RETURN_RET_LOG(napi_get_value_bool(env, nVal.argv[NUM_5],
            &(contrastContext_->genFinalEffect)) == napi_ok, false, "Arg 5 type mismatch");
    }
    contrastContext_->fullRatio = std::min(
        static_cast<float>(contrastContext_->inputPixelMap->GetWidth()) /
            static_cast<float>(contrastContext_->curPixelmapArea.w),
        static_cast<float>(contrastContext_->inputPixelMap->GetHeight()) /
            static_cast<float>(contrastContext_->curPixelmapArea.h));
    return true;
}

bool VpeNapi::ParseLCDParameter(napi_env env, napi_callback_info info, NapiValues& nVal)
{
    VPETrace vpeTrace("VpeNapi::DetailEnhance");
    std::lock_guard<std::mutex> lock(g_contrastLock);
    nVal.argc = NUM_3;
    napi_value argValue[NUM_3] = {0};
    nVal.argv = argValue;
    if (!PrepareNapiEnv(env, info, &nVal)) {
        return false;
    }
    if (nVal.argc != NUM_3) {
        VPE_LOGE("Invalid args count %{public}zu", nVal.argc);
        return false;
    } else {
        CHECK_AND_RETURN_RET_LOG(ParserImageType(env, argValue[NUM_0]) == ImageType::TYPE_PIXEL_MAP,
            false, "Arg 0 type is not pixelmap");
        contrastContext_->lcdPixelMap = PixelMapNapi::GetPixelMap(env, argValue[NUM_0]);
        CHECK_AND_RETURN_RET_LOG(napi_get_value_int32(env, nVal.argv[NUM_1],
            &(contrastContext_->contentId)) == napi_ok, false, "Failed to parse lcd param. Arg 1 type mismatch");
        CHECK_AND_RETURN_RET_LOG(napi_get_value_double(env, nVal.argv[NUM_2],
            &(contrastContext_->defaultRatio)) == napi_ok, false, "Failed to parse lcd param. Arg 2 type mismatch");
        contrastContext_->lcdWidth = contrastContext_->lcdPixelMap->GetWidth();
        contrastContext_->lcdHeight = contrastContext_->lcdPixelMap->GetHeight();
    }
    VPE_LOGI("update content info: lcdWidth:%{public}d, lcdHeight:%{public}d",
        contrastContext_->lcdWidth, contrastContext_->lcdHeight);
    return true;
}

napi_value VpeNapi::SetLcdImage(napi_env env, napi_callback_info info)
{
    VPE_LOGI("set lcd image");
    VPETrace vpeTrace("VpeNapi::DetailEnhanceProcess");
    napi_value result = nullptr;
    if (contrastContext_ == nullptr) {
        contrastContext_ = std::make_shared<ContrastEnhanceContext>();
        VPE_LOGI("create new contrast context");
    }
    if (contrastContext_ == nullptr) {
        VPE_LOGE("context == nullptr");
        return result;
    }
    NapiValues nVal;
    if (!ParseLCDParameter(env, info, nVal)) {
        VPE_LOGE("parse parameter failed");
        return result;
    }
    napi_value resourceName;
    napi_create_string_latin1(env, "Asynchronous processing", NAPI_AUTO_LENGTH, &resourceName);
    napi_status status = napi_create_async_work(
        env, nullptr, resourceName,
        [](napi_env env, void* data) {
            ContrastEnhanceContext* innerAsyncContext = reinterpret_cast<ContrastEnhanceContext*>(data);
            if (innerAsyncContext == nullptr) {
                ThrowExceptionError(env, IMAGE_PROCESSING_ERROR_PROCESS_FAILED, "innerAsyncContext == nullptr");
                return;
            }
            GenerateRegionHist(env, innerAsyncContext);
        },
        [](napi_env env, napi_status status, void* data) {
            ContrastEnhanceContext* innerAsyncContext = reinterpret_cast<ContrastEnhanceContext*>(data);
            CallCallback(env, innerAsyncContext);
            if (status != napi_ok) {
                VPE_LOGE("Failed to generate lut baseline for pcicture %{public}d", contrastContext_->contentId);
            }
            VPE_LOGI("Generate lut baseline for picture %{public}d successfully. Initial scaling ratio: %{public}f",
                contrastContext_->contentId, contrastContext_->defaultRatio);
        },
        (void*)(contrastContext_.get()), &contrastContext_->asyncWork);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "create aysnc failed");
    CHECK_AND_RETURN_RET_LOG(napi_queue_async_work(env, contrastContext_->asyncWork) == napi_ok,
        nullptr, "queue aysnc work failed");
    return result;
}

napi_value VpeNapi::RegisterCallback(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (contrastContext_->callbackRef != nullptr) {
        napi_delete_reference(env, contrastContext_->callbackRef);
    }
    napi_create_reference(env, argv[0], 1, &(contrastContext_->callbackRef));
    return nullptr;
}

napi_value VpeNapi::CallCallback(napi_env env, ContrastEnhanceContext* context)
{
    bool noCall = true;
    if (noCall) {
        return nullptr;
    }

    if (context->callbackRef != nullptr) {
        napi_value global;
        napi_get_global(env, &global);
        napi_value callbackFunc;
        napi_get_reference_value(env, context->callbackRef, &callbackFunc);
        napi_value result;
        napi_call_function(env, global, callbackFunc, 0, nullptr, &result);
    }
    return nullptr;
}

napi_value VpeNapi::Constructor(napi_env env, napi_callback_info info)
{
    napi_value undefineVar = nullptr;
    napi_get_undefined(env, &undefineVar);

    napi_status status;
    napi_value thisVar = nullptr;
    napi_get_undefined(env, &thisVar);
    size_t argc = NEW_INSTANCE_ARGC;
    napi_value argv[NEW_INSTANCE_ARGC] = { 0 };
    status = napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (status != napi_ok || thisVar == nullptr) {
        return undefineVar;
    }
    VpeNapi* pDetailEnhanceNapi = new VpeNapi();
    status = napi_wrap_with_size(env, thisVar, reinterpret_cast<void*>(pDetailEnhanceNapi),
        VpeNapi::Destructor, nullptr, nullptr, static_cast<size_t>(sizeof(VpeNapi)));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, undefineVar, "Failure wrapping js to native napi");
    return thisVar;
}

void VpeNapi::Destructor(napi_env env, void* nativeObject, void* finalize)
{
    if (nativeObject != nullptr) {
        std::lock_guard<std::mutex> lock(g_imageProcessorMutex);
        delete reinterpret_cast<VpeNapi*>(nativeObject);
        nativeObject = nullptr;
    }
}

napi_value VpeNapi::DoInitAfter(napi_env env, napi_value exports, napi_value constructor,
    size_t property_count, const napi_property_descriptor* properties)
{
    napi_value global = nullptr;
    CHECK_AND_RETURN_RET_LOG(napi_get_global(env, &global) == napi_ok, nullptr, "Init:get global fail");
    CHECK_AND_RETURN_RET_LOG(napi_set_named_property(env, global, CLASS_NAME.c_str(), constructor) == napi_ok,
        nullptr, "Init:set global named property fail");
    CHECK_AND_RETURN_RET_LOG(napi_set_named_property(env, exports, CLASS_NAME.c_str(), constructor) == napi_ok,
        nullptr, "set named property fail");
    CHECK_AND_RETURN_RET_LOG(napi_define_properties(env, exports, property_count, properties) == napi_ok,
        nullptr, "define properties fail");
    return exports;
}

napi_value VpeNapi::CreateEnumTypeObject(napi_env env, napi_valuetype type, napi_ref* ref,
    std::vector<struct QualityLevelEnum>& imageEnumMap)
{
    napi_value result = nullptr;
    napi_status status;
    int32_t refCount = 1;
    std::string propName;
    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (auto imgEnum : imageEnumMap) {
            napi_value enumNapiValue = nullptr;
            if (type == napi_string) {
                status = napi_create_string_utf8(env, imgEnum.strVal.c_str(), NAPI_AUTO_LENGTH, &enumNapiValue);
            } else if (type == napi_number) {
                status = napi_create_int32(env, imgEnum.numVal, &enumNapiValue);
            } else {
                VPE_LOGE("Unsupported type %{public}d!", type);
            }
            if (status == napi_ok && enumNapiValue != nullptr) {
                status = napi_set_named_property(env, result, imgEnum.name.c_str(), enumNapiValue);
            }
            if (status != napi_ok) {
                VPE_LOGE("Failed to add named prop!");
                break;
            }
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, refCount, ref);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    VPE_LOGE("CreateEnumTypeObject is Failed!");
    napi_get_undefined(env, &result);
    return result;
}

std::vector<napi_property_descriptor> VpeNapi::RegisterNapi()
{
    std::vector<napi_property_descriptor> props = {
        DECLARE_NAPI_FUNCTION("enhanceDetail", VpeNapi::EnhanceDetail),
        DECLARE_NAPI_FUNCTION("enhanceDetailSync", VpeNapi::EnhanceDetailSync),
        DECLARE_NAPI_FUNCTION("setLcdImage", VpeNapi::SetLcdImage),
        DECLARE_NAPI_FUNCTION("setDetailImage", VpeNapi::SetDetailImage),
        DECLARE_NAPI_FUNCTION("registerCallback", VpeNapi::RegisterCallback)
    };
    return props;
}

napi_value VpeNapi::Init(napi_env env, napi_value exports)
{
    std::vector<napi_property_descriptor> props = VpeNapi::RegisterNapi();
    napi_value constructor = nullptr;
    CHECK_AND_RETURN_RET_LOG(napi_define_class(env, CLASS_NAME.c_str(), NAPI_AUTO_LENGTH,
        VpeNapi::Constructor, nullptr, props.size(), props.data(), &constructor) == napi_ok,
        nullptr, "define class fail");
    CHECK_AND_RETURN_RET_LOG(napi_create_reference(env, constructor, 1, &constructor_) == napi_ok,
        nullptr, "create reference fail");

    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_PROPERTY("QualityLevel",
            CreateEnumTypeObject(env, napi_number, &qualityLevelTypeRef_, g_qualityLevels)),
        DECLARE_NAPI_FUNCTION("create", VpeNapi::Create),
        DECLARE_NAPI_FUNCTION("initializeEnvironment", VpeNapi::InitializeEnvironment),
        DECLARE_NAPI_FUNCTION("deinitializeEnvironment", VpeNapi::DeinitializeEnvironment),
    };
    auto result = DoInitAfter(env, exports, constructor, sizeof(desc) / sizeof(desc[0]), desc);
    return result;
}
}
}