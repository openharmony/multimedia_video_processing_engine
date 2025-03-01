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

#ifndef INTERFACES_KITS_JS_COMMON_INCLUDE_DETAIL_ENHANCE_NAPI_FORMAL_H
#define INTERFACES_KITS_JS_COMMON_INCLUDE_DETAIL_ENHANCE_NAPI_FORMAL_H

#include "detail_enhancer_image.h"
#include "contrast_enhancer_image.h"
#include "image_type.h"
#include "pixel_map_napi.h"

namespace OHOS {
namespace Media {
class VpeNapi {
public:
    VpeNapi() = default;
    ~VpeNapi() = default;

    static napi_value Init(napi_env env, napi_value exports);
    static napi_value Create(napi_env env, napi_callback_info info);
    static napi_value EnhanceDetail(napi_env env, napi_callback_info info);
    static napi_value EnhanceDetailSync(napi_env env, napi_callback_info info);
    static napi_value SetDetailImage(napi_env env, napi_callback_info info);
    static napi_value SetLcdImage(napi_env env, napi_callback_info info);
    static napi_value RegisterCallback(napi_env env, napi_callback_info info);
    static napi_value InitializeEnvironment(napi_env env, napi_callback_info info);
    static napi_value DeinitializeEnvironment(napi_env env, napi_callback_info info);

private:
    struct DetailEnhanceContext {
        napi_async_work asyncWork{}; // asynchronous work object
        napi_deferred deferred{}; // Delayed execution object (used to return calculation results in Promise)
        napi_ref callbackRef{}; // Reference object of callback func(used to return calculation results)
        double xArg{};
        double yArg{};
        int32_t qualityLevel{};
        std::shared_ptr<PixelMap> inputPixelMap{};
        std::shared_ptr<PixelMap> outputPixelMap{};
    };

    struct ContrastEnhanceContext {
        napi_env env;
        napi_async_work asyncWork{}; // asynchronous work object
        napi_deferred deferred;
        napi_ref callbackRef;
        std::shared_ptr<PixelMap> inputPixelMap{};
        std::shared_ptr<PixelMap> scaledPixelMap{};
        std::shared_ptr<PixelMap> lcdPixelMap{};
        sptr<SurfaceBuffer> pazzleBuffer{};
        OHOS::Rect curPixelmapArea;
        OHOS::Rect displayArea;
        int pixelmapId = -1;
        double fullRatio{};; // 传入的pixelmap 相对于原图的缩放比例
        int sharpnessIntensity{};;
        int32_t contentId = -1;
        int oriHeight{};
        int oriWidth{};
        unsigned int lcdHeight{};
        unsigned int lcdWidth{};
        bool isLocalDecoding{};
        napi_ref callbackFunc;
        double defaultRatio{};
        bool isLCDLutFinished{};
        bool isCanceled{};
        bool genFinalEffect{};
    };

    struct NapiValues {
        napi_status status = napi_ok;
        napi_value thisVar{};
        napi_value result{};
        napi_value* argv{};
        size_t argc = 0;
    };

    static thread_local napi_ref constructor_;
    static thread_local napi_ref qualityLevelTypeRef_;
    static thread_local std::shared_ptr<ContrastEnhanceContext> contrastContext_;
    static thread_local std::shared_ptr<DetailEnhanceContext> detailContext_;

    static bool PrepareNapiEnv(napi_env env, napi_callback_info info, NapiValues* nVal);
    static bool ParseRect(napi_env env, napi_value nVal, OHOS::Rect& rect);
    static bool ParseSize(napi_env env, napi_value nVal);
    static std::shared_ptr<PixelMap> PrepareDstPixelMap(napi_env env, VpeNapi::DetailEnhanceContext* context);
    static ImageType ParserImageType(napi_env env, napi_value argv);

    static void SetDstPixelMapInfo(OHOS::Media::PixelMap& source, void* dstPixels, uint32_t dstPixelsSize,
        std::unique_ptr<OHOS::Media::AbsMemory>& memory, OHOS::Media::PixelMap& dstPixelMap);
    static bool AllocMemory(OHOS::Media::PixelMap& source, OHOS::Media::PixelMap& dstPixelMap,
        const InitializationOptions& opt);
    static bool ConvertPixelmapToSurfaceBuffer(const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap,
        sptr<SurfaceBuffer>& bufferImpl);
    static std::unique_ptr<OHOS::Media::PixelMap> CreateDstPixelMap(OHOS::Media::PixelMap& source,
        const OHOS::Media::InitializationOptions& opts);
    static sptr<OHOS::SurfaceBuffer> GetSurfaceBufferFromDMAPixelMap(
        const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap);
    static napi_value CreateEnumTypeObject(napi_env env,
        napi_valuetype type, napi_ref* ref, std::vector<struct QualityLevelEnum>& imageEnumMap);

    // detail enhancer
    static bool InitDetailAlgo(napi_env env, int level);
    static bool ConfigResolutionBasedOnRatio(napi_env env, napi_value& nVal,
        std::shared_ptr<VpeNapi::DetailEnhanceContext> context);
    static bool ConfigResolution(napi_env env, napi_value& width, napi_value& height,
        std::shared_ptr<VpeNapi::DetailEnhanceContext> context);
    static bool ParseDetailEnhanceParameter(napi_env env, napi_callback_info info);
    static std::shared_ptr<PixelMap> DetailEnhanceImpl(napi_env env, VpeNapi::DetailEnhanceContext* context);

    // contrast enhancer
    static bool InitContrastAlgo(napi_env env);
    static bool ParseLCDParameter(napi_env env, napi_callback_info info, NapiValues& nVal);
    static bool ParseDetailImageParameter(napi_env env, napi_callback_info info, NapiValues& nVal);
    static bool GenerateRegionHist(napi_env env, ContrastEnhanceContext* context);
    static bool UpdateMetadataBasedOnLcd(ContrastEnhanceContext* context);
    static bool UpdateMetadataBasedOnDetail(ContrastEnhanceContext* context);
    static napi_value CallCallback(napi_env env, ContrastEnhanceContext* context);

    static napi_value Constructor(napi_env env, napi_callback_info info);
    static void Destructor(napi_env env, void* nativeObject, void* finalize);
    static void ThrowExceptionError(napi_env env, const int32_t errCode, const std::string errMsg);
    static napi_value DoInitAfter(napi_env env, napi_value exports, napi_value constructor, size_t property_count,
        const napi_property_descriptor* properties);
    static std::vector<napi_property_descriptor> RegisterNapi();
};
}
}
#endif // INTERFACES_KITS_JS_COMMON_INCLUDE_DETAIL_ENHANCE_NAPI_FORMAL_H