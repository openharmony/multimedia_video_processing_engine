/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#define LOG_DOMAIN LOG_TAG_DOMAIN_ID_IMAGE
#undef LOG_TAG
#define LOG_TAG "DetailEnhanceNapi"

#include "detail_enhance_napi.h"

#include <chrono>

#include "image_log.h"
#include "image_trace.h"
#include "image_utils.h"
#include "log_tags.h"
#include "media_errors.h"
#include "memory_manager.h"
#include "pixelmap_native_impl.h"
#include "pixelmap_native.h"
#include "vpe_utils.h"

#if defined(__OHOS__)
    #include "hitrace_meter.h"
    #define ATRACE_CALL() HITRACE_METER_NAME(HITRACE_TAG_GRAPHIC_AGP, __func__)
    #define ATRACE_BEGIN(name) StartTrace(HITRACE_TAG_GRAPHIC_AGP, name)
    #define ATRACE_END() FinishTrace(HITRACE_TAG_GRAPHIC_AGP)
    #define ATRACE_INT(name, value) CountTrace(HITRACE_TAG_GRAPHIC_AGP, name, value)
#endif

namespace {
    constexpr uint32_t NUM_0 = 0;
    constexpr uint32_t NUM_1 = 1;
    constexpr uint32_t NUM_2 = 2;
    constexpr uint32_t NUM_3 = 3;
}

namespace OHOS {
namespace Media {
using namespace VideoProcessingEngine;
ImageType DetailEnhanceNapi::ParserImageType(napi_env env, napi_value argv)
{
    napi_value constructor = nullptr;
    napi_value global = nullptr;
    napi_status ret = napi_invalid_arg;
    napi_get_global(env, &global);
    ret = napi_get_named_property(env, global, "PixelMap", &constructor);
    if (ret != napi_ok) {
        IMAGE_LOGI("Get DetailEnhanceNapi property failed!");
    }
    bool isInstance = false;
    ret = napi_instanceof(env, argv, constructor, &isInstance);
    if (ret == napi_ok && isInstance) {
        return ImageType::TYPE_PIXEL_MAP;
    }
    return ImageType::TYPE_UNKNOWN;
}

bool DetailEnhanceNapi::PrepareNapiEnv(napi_env env, napi_callback_info info, struct NapiValues* nVal)
{
    napi_get_undefined(env, &(nVal->result));
    nVal->status = napi_get_cb_info(env, info, &(nVal->argc), nVal->argv, &(nVal->thisVar), nullptr);
    if (nVal->status != napi_ok) {
        IMAGE_LOGE("fail to napi_get_cb_info");
        return false;
    }
    nVal->context = std::make_unique<DetailEnhanceContext>();
    return true;
}

std::shared_ptr<DetailEnhancerImage> DetailEnhancerImageCreate()
{
    auto detailEnh = DetailEnhancerImage::Create();
    if (detailEnh == nullptr) {
        return nullptr;
    }
    return detailEnh;
}

napi_value DetailEnhanceNapi::Init(napi_env env, napi_callback_info info)
{
    ATRACE_BEGIN("DetailEnhanceNapi::OH_ImageProcessing_Create");
    std::lock_guard<std::mutex> lock(lock_);
    napi_value result;
    if (mDetailEnh != nullptr) {
        napi_get_boolean(env, true, &result);
        return result;
    }
    mDetailEnh = DetailEnhancerImageCreate();
    if (mDetailEnh == nullptr) {
        IMAGE_LOGE("mDetailEnh == nullptr");
        return nullptr;
    }
    DetailEnhancerParameters param {
        .uri = "",
        .level = static_cast<DetailEnhancerLevel>(DETAIL_ENH_LEVEL_HIGH),
    };
    if (mDetailEnh->SetParameter(param)!= VPE_ALGO_ERR_OK) {
        printf("Init failed!");
        napi_get_boolean(env, false, &result);
        return result;
    }
    ATRACE_END();
    napi_get_boolean(env, true, &result);
    return result;
}

napi_value DetailEnhanceNapi::Destroy(napi_env env, napi_callback_info info)
{
    std::lock_guard<std::mutex> lock(lock_);
    ATRACE_BEGIN("DetailEnhanceNapi::Destroy");
    napi_value result;
    if (mDetailEnh != nullptr) {
        mDetailEnh = nullptr;
    }
    ATRACE_END();
    napi_get_boolean(env, true, &result);
    return result;
}

void DetailEnhanceNapi::SetDstPixelMapInfo(PixelMap &source, void* dstPixels, uint32_t dstPixelsSize,
    std::unique_ptr<AbsMemory>& memory, PixelMap &dstPixelMap)
{
    AllocatorType sourceType = source.GetAllocatorType();
    if (sourceType != AllocatorType::DMA_ALLOC) {
        IMAGE_LOGW("only support DMA");
        return;
    }
    if (memory == nullptr) {
        IMAGE_LOGW("Invalid memory");
        return;
    }
    dstPixelMap.SetPixelsAddr(dstPixels, memory->extend.data, memory->data.size, sourceType, nullptr);
    if (source.GetAllocatorType() == AllocatorType::DMA_ALLOC && source.IsHdr()) {
        sptr<SurfaceBuffer> sourceSurfaceBuffer(reinterpret_cast<SurfaceBuffer*> (source.GetFd()));
        sptr<SurfaceBuffer> dstSurfaceBuffer(reinterpret_cast<SurfaceBuffer*> (dstPixelMap.GetFd()));
        VpeUtils::CopySurfaceBufferInfo(sourceSurfaceBuffer, dstSurfaceBuffer);
    }
#ifdef IMAGE_COLORSPACE_FLAG
    OHOS::ColorManager::ColorSpace colorspace = source.InnerGetGrColorSpace();
    dstPixelMap.InnerSetColorSpace(colorspace);
#endif
}

bool DetailEnhanceNapi::AllocMemory(PixelMap &source, PixelMap &dstPixelMap)
{
    if (source.GetPixels() == nullptr || source.GetAllocatorType() != AllocatorType::DMA_ALLOC) {
        IMAGE_LOGE("source pixelMap data invalid");
        return false;
    }
    int32_t bufferSize = source.GetByteCount();
    if (bufferSize <= 0) {
        IMAGE_LOGE("CopyPixelMap parameter bufferSize:[%{public}d] error.", bufferSize);
        return false;
    }
    ImageInfo dstImageInfo;
    dstPixelMap.GetImageInfo(dstImageInfo);
    MemoryData memoryData = {nullptr, static_cast<size_t>(bufferSize), "Copy ImageData",
        dstImageInfo.size, dstImageInfo.pixelFormat};
    std::unique_ptr<AbsMemory> memory = MemoryManager::CreateMemory(source.GetAllocatorType(), memoryData);
    if (memory == nullptr) {
        IMAGE_LOGE("invalid memory");
        return false;
    }
    void *dstPixels = memory->data.data;
    if (dstPixels == nullptr) {
        IMAGE_LOGE("source crop allocate memory fail allocatetype: %{public}d ", source.GetAllocatorType());
        return false;
    }
    SetDstPixelMapInfo(source, dstPixels, static_cast<size_t>(bufferSize), memory, dstPixelMap);
    return true;
}

std::unique_ptr<PixelMap> DetailEnhanceNapi::CreateDstPixelMap(PixelMap &source, const InitializationOptions &opts)
{
    std::unique_ptr<PixelMap> dstPixelMap = std::make_unique<PixelMap>();
    if (dstPixelMap == nullptr) {
        IMAGE_LOGE("create pixelmap pointer fail");
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
    if (!AllocMemory(source, *dstPixelMap.get())) {
        return nullptr;
    }
    return dstPixelMap;
}

sptr<SurfaceBuffer> DetailEnhanceNapi::GetSurfaceBufferFromDMAPixelMap(
    const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap)
{
    if (pixelmap == nullptr || pixelmap->GetAllocatorType() != AllocatorType::DMA_ALLOC) {
        return nullptr;
    }
    return reinterpret_cast<SurfaceBuffer*>(pixelmap->GetFd());
}

napi_value DetailEnhanceNapi::DetailEnhanceImpl(napi_env env, std::unique_ptr<DetailEnhanceContext>& context)
{
    if (context == nullptr) {
        IMAGE_LOGE("context == nullptr");
        return nullptr;
    }
    if (mDetailEnh == nullptr) {
        IMAGE_LOGE("mDetailEnh == nullptr");
        return nullptr;
    }
    ATRACE_BEGIN("DetailEnhanceNapi::CreatePixelMap");
    InitializationOptions opts {
        .size = {
            .width = static_cast<int>(context->xArg),
            .height = static_cast<int>(context->yArg),
        },
    };
    std::unique_ptr<PixelMap> outputPtr = CreateDstPixelMap(*context->inputPixelMap, opts);
    if (outputPtr == nullptr) {
        IMAGE_LOGE("create failed");
        return nullptr;
    }
    std::shared_ptr<PixelMap> dstPixelMap{std::move(outputPtr)};
    if (dstPixelMap == nullptr) {
        IMAGE_LOGE("move failed");
        return nullptr;
    }
    auto input = GetSurfaceBufferFromDMAPixelMap(context->inputPixelMap);
    auto output = GetSurfaceBufferFromDMAPixelMap(dstPixelMap);
    if (mDetailEnh->Process(input, output) != VPE_ALGO_ERR_OK) {
        IMAGE_LOGE("process failed");
        return nullptr;
    }
    return PixelMapNapi::CreatePixelMap(env, dstPixelMap);
}

napi_value DetailEnhanceNapi::Process(napi_env env, napi_callback_info info)
{
    ImageTrace imageTrace("DetailEnhanceNapi::DetailEnhance");
    ATRACE_BEGIN("DetailEnhanceNapi::ProcessCheckEnv");
    std::lock_guard<std::mutex> lock(lock_);
    if (mDetailEnh == nullptr) {
        napi_value result;
        napi_get_boolean(env, false, &result);
        return result;
    }
    ATRACE_END();
    ATRACE_BEGIN("DetailEnhanceNapi::prepareNapi");
    NapiValues nVal;
    nVal.argc = NUM_3;
    napi_value argValue[NUM_3] = {0};
    nVal.argv = argValue;
    if (!PrepareNapiEnv(env, info, &nVal)) {
        return nVal.result;
    }
    if (nVal.argc != NUM_3) {
        IMAGE_LOGE("Invalid args count %{public}zu", nVal.argc);
        return nullptr;
    } else {
        if (napi_ok != napi_get_value_double(env, nVal.argv[NUM_0], &(nVal.context->xArg))) {
            IMAGE_LOGE("Arg 0 type mismatch");
            return nullptr;
        }
        if (napi_ok != napi_get_value_double(env, nVal.argv[NUM_1], &(nVal.context->yArg))) {
            IMAGE_LOGE("Arg 1 type mismatch");
            return nullptr;
        }
        if (ParserImageType(env, argValue[NUM_2]) == ImageType::TYPE_PIXEL_MAP) {
            nVal.context->inputPixelMap = PixelMapNapi::GetPixelMap(env, argValue[NUM_2]);
        }
        if (nVal.context->inputPixelMap == nullptr) {
            return nullptr;
        }
    }
    if (nVal.context->callbackRef == nullptr) {
        napi_create_promise(env, &(nVal.context->deferred), &(nVal.result));
    }
    ATRACE_END();
    std::chrono::steady_clock::time_point clProcess = std::chrono::steady_clock::now();
    napi_value outPixelmap = DetailEnhanceImpl(env, nVal.context);
    std::chrono::duration<float, std::milli> clProcessD = std::chrono::steady_clock::now() - clProcess;
    IMAGE_LOGI("detail enhance total cost: %{public}f ms", clProcessD.count());
    return outPixelmap;
}

static napi_value Init(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("init", DetailEnhanceNapi::Init),
        DECLARE_NAPI_FUNCTION("process", DetailEnhanceNapi::Process),
        DECLARE_NAPI_FUNCTION("destroy", DetailEnhanceNapi::Destroy),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module detailEnhanceModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "multimedia.detailEnhancer",
    .nm_priv = ((void *)0),
    .reserved = {0},
};
 
extern "C" __attribute__((constructor)) void DetailEnhanceRegisterModule(void)
{
    napi_module_register(&detailEnhanceModule);
}
} // namespace Media
} // namespace OHOS