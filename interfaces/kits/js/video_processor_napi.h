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
 
#ifndef VIDEO_PROCESSOR_NAPI_H
#define VIDEO_PROCESSOR_NAPI_H
 
#include "algorithm_video.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
 
namespace OHOS {
namespace Media {
 
class VideoProcessorNapi {
public:
    VideoProcessorNapi() = default;
    ~VideoProcessorNapi() = default;
 
    static napi_value Init(napi_env env, napi_value exports);
    static napi_value CreateVideoProcessor(napi_env env, napi_callback_info info);
    static napi_value GetStatus(napi_env env, napi_callback_info info);
    static napi_value OnStatusChange(napi_env env, napi_callback_info info);
    static napi_value OffStatusChange(napi_env env, napi_callback_info info);
 
private:
    static thread_local napi_ref constructor_;
    static std::vector<napi_ref> g_statusCallbacks;
    static std::mutex g_statusCallbacksLock;
    static napi_threadsafe_function g_statusTsfn;
 
    static napi_value Constructor(napi_env env, napi_callback_info info);
    static void Destructor(napi_env env, void* nativeObject, void* finalize);
    static void ThrowExceptionError(napi_env env, const int32_t errCode, const std::string errMsg);
    static void NotifyCallback(napi_env env, napi_ref callbackRef, bool aiHdrEnabled);
    static void StatusTsfnCallJs(napi_env env, napi_value jsCallback, void* context, void* data);
    static void StatusTsfnFinalize(napi_env env, void* data, void* hint);
 
    static bool ValidateCallback(napi_env env, napi_value callbackValue);
    static void RegisterObserver(napi_env env);
    static void UnregisterObserver();
    static void RemoveCallbackRef(napi_env env, napi_value callback);
};
}
}
 
#endif
