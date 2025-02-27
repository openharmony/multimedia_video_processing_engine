/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "native_module_ohos_imageprocessing.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN LOG_TAG_DOMAIN_ID_IMAGE

#undef LOG_TAG
#define LOG_TAG 0xD002B3F

namespace OHOS {
namespace Media {
/*
* Function registering all props and functions of multimedia.videoProcessingEngine module
*/
static napi_value Export(napi_env env, napi_value exports)
{
    VpeNapi::Init(env, exports);
    return exports;
}

/*
 * module define
 */
static napi_module videoProcessingModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Export,
    .nm_modname = "multimedia.videoProcessingEngine",
    .nm_priv = nullptr,
    .reserved = {0},
};

/*
 * module register
 */
extern "C" __attribute__((constructor)) void VideoProcessingModule(void)
{
    napi_module_register(&videoProcessingModule);
}
} // namespace Media
} // namespace OHOS
