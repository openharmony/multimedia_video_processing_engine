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

#ifndef INTERFACES_KITS_JS_COMMON_INCLUDE_DETAIL_ENHANCE_NAPI_H
#define INTERFACES_KITS_JS_COMMON_INCLUDE_DETAIL_ENHANCE_NAPI_H

#include "detail_enhancer_image.h"
#include "image_type.h"
#include "pixel_map_napi.h"

namespace OHOS {
namespace Media {
class DetailEnhanceNapi {
public:
    DetailEnhanceNapi();
    ~DetailEnhanceNapi();

    static napi_value Process(napi_env env, napi_callback_info info);
    static napi_value Init(napi_env env, napi_callback_info info);
    static napi_value Destroy(napi_env env, napi_callback_info info);
private:
    struct DetailEnhanceContext {
        napi_env env;
        napi_deferred deferred;
        napi_ref callbackRef;
        std::shared_ptr<PixelMap> inputPixelMap = nullptr;
        double xArg = 0;
        double yArg = 0;
    };
    struct NapiValues {
        napi_status status;
        napi_value thisVar = nullptr;
        napi_value result = nullptr;
        napi_value* argv = nullptr;
        size_t argc;
        std::unique_ptr<DetailEnhanceContext> context;
    };

    static ImageType ParserImageType(napi_env env, napi_value argv);
    static bool PrepareNapiEnv(napi_env env, napi_callback_info info, struct NapiValues* nVal);
    static napi_value DetailEnhanceImpl(napi_env env, std::unique_ptr<DetailEnhanceContext>& context);
    static void SetDstPixelMapInfo(OHOS::Media::PixelMap &source, void* dstPixels, uint32_t dstPixelsSize,
        std::unique_ptr<OHOS::Media::AbsMemory>& memory, OHOS::Media::PixelMap &dstPixelMap);
    static bool AllocMemory(OHOS::Media::PixelMap &source, OHOS::Media::PixelMap &dstPixelMap);
    static std::unique_ptr<OHOS::Media::PixelMap> CreateDstPixelMap(OHOS::Media::PixelMap &source,
        const OHOS::Media::InitializationOptions &opts);
    static sptr<OHOS::SurfaceBuffer> GetSurfaceBufferFromDMAPixelMap(
        const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap);
};

static std::shared_ptr<OHOS::Media::VideoProcessingEngine::DetailEnhancerImage> mDetailEnh;
static std::mutex lock_{std::mutex()};
}
}
#endif // INTERFACES_KITS_JS_COMMON_INCLUDE_DETAIL_ENHANCE_NAPI_H