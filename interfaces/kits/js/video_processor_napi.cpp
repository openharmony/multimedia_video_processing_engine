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
#define LOG_TAG "VideoProcessorNapi"
 
#include "video_processor_napi.h"
 
#include <algorithm>
#include "algorithm_video.h"
#include "video_processing_types.h"
#include "vpe_log.h"
#include "vpe_trace.h"
 
 
namespace {
constexpr uint32_t NUM_1 = 1;
const char VIDEO_PROCESSOR_CLASS_NAME[] = "VideoProcessor";
}
 
namespace OHOS {
namespace Media {
using namespace VideoProcessingEngine;
using namespace std::chrono;
thread_local napi_ref VideoProcessorNapi::constructor_ = nullptr;
std::vector<napi_ref> VideoProcessorNapi::g_statusCallbacks{};
std::mutex VideoProcessorNapi::g_statusCallbacksLock{};
napi_threadsafe_function VideoProcessorNapi::g_statusTsfn = nullptr;
 
struct StatusCallbackData {
    bool aiHdrEnabled;
};
 
void VideoProcessorNapi::ThrowExceptionError(napi_env env, const int32_t errCode, const std::string errMsg)
{
    std::string errCodeStr = std::to_string(errCode);
    napi_throw_error(env, errCodeStr.c_str(), errMsg.c_str());
}
 
napi_value VideoProcessorNapi::Constructor(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value thisVar = nullptr;
    size_t argc = NUM_1;
    napi_value argv[NUM_1] = {0};
    status = napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (status != napi_ok || thisVar == nullptr) {
        return nullptr;
    }
    VideoProcessorNapi* pVideoProcessorNapi = new VideoProcessorNapi();
    CHECK_AND_RETURN_RET_LOG(pVideoProcessorNapi != nullptr, nullptr, "pVideoProcessorNapi == nullptr");
    status = napi_wrap_with_size(env, thisVar, reinterpret_cast<void*>(pVideoProcessorNapi),
        VideoProcessorNapi::Destructor, nullptr, nullptr, static_cast<size_t>(sizeof(VideoProcessorNapi)));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "Failure wrapping js to native napi");
    return thisVar;
}
 
void VideoProcessorNapi::Destructor(napi_env env, void* nativeObject, void* finalize)
{
    if (nativeObject != nullptr) {
        delete reinterpret_cast<VideoProcessorNapi*>(nativeObject);
        nativeObject = nullptr;
    }
}
 
static napi_value BuildStatusObject(napi_env env, bool aiHdrEnabled)
{
    napi_value jsStatus = nullptr;
    CHECK_AND_RETURN_RET_LOG(napi_create_object(env, &jsStatus) == napi_ok, nullptr,
        "BuildStatusObject: create jsStatus failed");
    napi_value jsAiHdrStatus;
    CHECK_AND_RETURN_RET_LOG(napi_create_object(env, &jsAiHdrStatus) == napi_ok, nullptr,
        "BuildStatusObject: create jsAiHdrStatus failed");
    napi_value jsAiHdrEnabled;
    CHECK_AND_RETURN_RET_LOG(napi_get_boolean(env, aiHdrEnabled, &jsAiHdrEnabled) == napi_ok, nullptr,
        "BuildStatusObject: get boolean failed");
    CHECK_AND_RETURN_RET_LOG(napi_set_named_property(env, jsAiHdrStatus, "enabled", jsAiHdrEnabled) == napi_ok, nullptr,
        "BuildStatusObject: set enabled failed");
    CHECK_AND_RETURN_RET_LOG(napi_set_named_property(env, jsStatus, "aiHdr", jsAiHdrStatus) == napi_ok, nullptr,
        "BuildStatusObject: set aiHdr failed");
    return jsStatus;
}
 
static void InvokeCallback(napi_env env, napi_ref callbackRef, napi_value jsStatus)
{
    CHECK_AND_RETURN_LOG(jsStatus != nullptr, "jsStatus is nullptr");
    napi_value global;
    CHECK_AND_RETURN_LOG(napi_get_global(env, &global) == napi_ok, "InvokeCallback: get global failed");
    napi_value callbackFunc;
    CHECK_AND_RETURN_LOG(napi_get_reference_value(env, callbackRef, &callbackFunc) == napi_ok,
        "InvokeCallback: get reference value failed");
    napi_value argv[NUM_1] = {jsStatus};
    napi_value callbackResult;
    CHECK_AND_RETURN_LOG(napi_call_function(env, global, callbackFunc, NUM_1, argv, &callbackResult) == napi_ok,
        "InvokeCallback: call function failed");
}
 
void VideoProcessorNapi::NotifyCallback(napi_env env, napi_ref callbackRef, bool aiHdrEnabled)
{
    napi_value jsStatus = BuildStatusObject(env, aiHdrEnabled);
    CHECK_AND_RETURN_LOG(jsStatus != nullptr, "NotifyCallback: jsStatus is nullptr");
    InvokeCallback(env, callbackRef, jsStatus);
    VPE_LOGI("NotifyCallback: aiHdrEnabled=%{public}d", aiHdrEnabled);
}
 
void VideoProcessorNapi::StatusTsfnCallJs(napi_env env, napi_value jsCallback, void* context, void* data)
{
    std::unique_ptr<StatusCallbackData> callbackDataPtr;
    CHECK_AND_RETURN_LOG(data != nullptr, "StatusTsfnCallJs data is nullptr");
    callbackDataPtr.reset(static_cast<StatusCallbackData*>(data));
    napi_value jsStatus = BuildStatusObject(env, callbackDataPtr->aiHdrEnabled);
    CHECK_AND_RETURN_LOG(jsStatus != nullptr, "StatusTsfnCallJs: jsStatus is nullptr");
    
    std::lock_guard<std::mutex> lock(g_statusCallbacksLock);
    for (auto& callbackRef : g_statusCallbacks) {
        InvokeCallback(env, callbackRef, jsStatus);
    }
    VPE_LOGI("StatusTsfnCallJs: aiHdrEnabled=%{public}d, callbackCount=%{public}zu",
        callbackDataPtr->aiHdrEnabled, g_statusCallbacks.size());
}
 
void VideoProcessorNapi::StatusTsfnFinalize(napi_env env, void* data, void* hint)
{
    VPE_LOGI("StatusTsfnFinalize");
}
 
napi_value VideoProcessorNapi::GetStatus(napi_env env, napi_callback_info info)
{
    VPETrace vpeTrace("VideoProcessorNapi::GetStatus");
    napi_value result;
    napi_get_undefined(env, &result);
    napi_deferred deferred;
    napi_value promise;
    CHECK_AND_RETURN_RET_LOG(napi_create_promise(env, &deferred, &promise) == napi_ok, result,
        "GetStatus: create promise failed");
    napi_value jsStatus;
    CHECK_AND_RETURN_RET_LOG(napi_create_object(env, &jsStatus) == napi_ok, result,
        "GetStatus: create jsStatus failed");
    napi_value jsAiHdrStatus;
    CHECK_AND_RETURN_RET_LOG(napi_create_object(env, &jsAiHdrStatus) == napi_ok, result,
        "GetStatus: create jsAiHdrStatus failed");
    Media::Format parameter{};
    bool aiHdrEnabled = VpeVideo::IsSupported(VIDEO_TYPE_AIHDR_ENHANCER, parameter);
    napi_value jsAiHdrEnabled;
    CHECK_AND_RETURN_RET_LOG(napi_get_boolean(env, aiHdrEnabled, &jsAiHdrEnabled) == napi_ok, result,
        "GetStatus: get boolean failed");
    CHECK_AND_RETURN_RET_LOG(napi_set_named_property(env, jsAiHdrStatus, "enabled", jsAiHdrEnabled) == napi_ok, result,
        "GetStatus: set enabled failed");
    CHECK_AND_RETURN_RET_LOG(napi_set_named_property(env, jsStatus, "aiHdr", jsAiHdrStatus) == napi_ok, result,
        "GetStatus: set aiHdr failed");
    CHECK_AND_RETURN_RET_LOG(napi_resolve_deferred(env, deferred, jsStatus) == napi_ok, result,
        "GetStatus: resolve deferred failed");
    VPE_LOGI("GetStatus: aiHdrEnabled=%{public}d", aiHdrEnabled);
    return promise;
}
 
bool VideoProcessorNapi::ValidateCallback(napi_env env, napi_value callbackValue)
{
    napi_valuetype callbackType;
    napi_typeof(env, callbackValue, &callbackType);
    if (callbackType != napi_function) {
        VPE_LOGE("Callback arg is not function");
        ThrowExceptionError(env, VIDEO_PROCESSING_ERROR_INVALID_VALUE, "Callback arg is not function");
        return false;
    }
    return true;
}
 
void VideoProcessorNapi::RegisterObserver(napi_env env)
{
    CHECK_AND_RETURN_LOG(g_statusTsfn == nullptr, "RegisterObserver: already registered");
    napi_value cbName;
    std::string callbackName = "StatusChange";
    CHECK_AND_RETURN_LOG(napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName) == napi_ok,
        "RegisterObserverIf: create string failed");
    CHECK_AND_RETURN_LOG(napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0,
        1, nullptr, StatusTsfnFinalize, nullptr, StatusTsfnCallJs, &g_statusTsfn) == napi_ok,
        "RegisterObserverIf: create threadsafe function failed");
    napi_threadsafe_function tsfnCopy = g_statusTsfn;
    VpeVideo::RegisterSettingsChangeCallback(VIDEO_TYPE_AIHDR_ENHANCER,
        [tsfnCopy](int32_t feature, int32_t tag, int32_t param) {
            VPE_LOGI("Settings changed, calling TSFN");
            auto* callbackData = new StatusCallbackData{(param != 0)};
            if (tsfnCopy != nullptr) {
                napi_call_threadsafe_function(tsfnCopy, callbackData, napi_tsfn_blocking);
            }
        });
}
 
void VideoProcessorNapi::UnregisterObserver()
{
    if (g_statusCallbacks.empty()) {
        VpeVideo::UnregisterSettingsChangeCallback(VIDEO_TYPE_AIHDR_ENHANCER);
        if (g_statusTsfn != nullptr) {
            napi_release_threadsafe_function(g_statusTsfn, napi_tsfn_release);
            g_statusTsfn = nullptr;
        }
    }
}
 
napi_value VideoProcessorNapi::OnStatusChange(napi_env env, napi_callback_info info)
{
    VPETrace vpeTrace("VideoProcessorNapi::OnStatusChange");
    napi_value result;
    napi_get_undefined(env, &result);
    size_t argc = NUM_1;
    napi_value argv[NUM_1] = {0};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != NUM_1 || !ValidateCallback(env, argv[0])) {
        return result;
    }
    
    napi_ref callbackRef = nullptr;
    if (napi_create_reference(env, argv[0], 1, &callbackRef) != napi_ok) {
        VPE_LOGE("Create callback reference failed");
        ThrowExceptionError(env, VIDEO_PROCESSING_ERROR_INVALID_VALUE, "Create callback reference failed");
        return result;
    }
    size_t callbackSize = 0;
    {
        std::lock_guard<std::mutex> lock(g_statusCallbacksLock);
        if (g_statusCallbacks.empty()) {
            RegisterObserver(env);
        }
        g_statusCallbacks.push_back(callbackRef);
        callbackSize = g_statusCallbacks.size();
    }
    Media::Format parameter{};
    bool aiHdrEnabled = VpeVideo::IsSupported(VIDEO_TYPE_AIHDR_ENHANCER, parameter);
    NotifyCallback(env, callbackRef, aiHdrEnabled);
    VPE_LOGI("OnStatusChange done, callbackCount=%{public}zu", callbackSize);
    return result;
}
 
void VideoProcessorNapi::RemoveCallbackRef(napi_env env, napi_value callback)
{
    auto it = std::find_if(g_statusCallbacks.begin(), g_statusCallbacks.end(),
        [env, callback](napi_ref ref) {
            napi_value refFunc;
            CHECK_AND_RETURN_RET_LOG(napi_get_reference_value(env, ref, &refFunc) == napi_ok, false,
                "RemoveCallbackRef: get reference value failed");
            bool isEqual = false;
            CHECK_AND_RETURN_RET_LOG(napi_strict_equals(env, callback, refFunc, &isEqual) == napi_ok, false,
                "RemoveCallbackRef: strict equals failed");
            return isEqual;
        });
    CHECK_AND_RETURN_LOG(it != g_statusCallbacks.end(), "RemoveCallbackRef: callback not found");
    CHECK_AND_RETURN_LOG(napi_delete_reference(env, *it) == napi_ok,
        "RemoveCallbackRef: delete reference failed");
    g_statusCallbacks.erase(it);
}
 
napi_value VideoProcessorNapi::OffStatusChange(napi_env env, napi_callback_info info)
{
    VPETrace vpeTrace("VideoProcessorNapi::OffStatusChange");
    napi_value result;
    napi_get_undefined(env, &result);
    size_t argc = NUM_1;
    napi_value argv[NUM_1] = {0};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > NUM_1) {
        VPE_LOGE("Invalid args count %{public}zu", argc);
        ThrowExceptionError(env, VIDEO_PROCESSING_ERROR_INVALID_VALUE, "Invalid args count");
        return result;
    }
    
    std::lock_guard<std::mutex> lock(g_statusCallbacksLock);
    if (argc == NUM_1) {
        if (!ValidateCallback(env, argv[0])) {
            return result;
        }
        RemoveCallbackRef(env, argv[0]);
    } else {
        for (auto& ref : g_statusCallbacks) {
            napi_delete_reference(env, ref);
        }
        g_statusCallbacks.clear();
    }
    UnregisterObserver();
    VPE_LOGI("OffStatusChange done, remainingCallbacks=%{public}zu", g_statusCallbacks.size());
    return result;
}
 
napi_value VideoProcessorNapi::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor props[] = {
        DECLARE_NAPI_FUNCTION("getStatus", VideoProcessorNapi::GetStatus),
        DECLARE_NAPI_FUNCTION("onStatusChange", VideoProcessorNapi::OnStatusChange),
        DECLARE_NAPI_FUNCTION("offStatusChange", VideoProcessorNapi::OffStatusChange)
    };
 
    napi_value constructor = nullptr;
    CHECK_AND_RETURN_RET_LOG(napi_define_class(env, VIDEO_PROCESSOR_CLASS_NAME, sizeof(VIDEO_PROCESSOR_CLASS_NAME),
        VideoProcessorNapi::Constructor, nullptr, sizeof(props) / sizeof(props[0]), props, &constructor) == napi_ok,
        nullptr, "define class fail");
    CHECK_AND_RETURN_RET_LOG(napi_set_named_property(env, exports, VIDEO_PROCESSOR_CLASS_NAME, constructor) == napi_ok,
        nullptr, "set named property fail");
    CHECK_AND_RETURN_RET_LOG(napi_create_reference(env, constructor, 1, &constructor_) == napi_ok,
        nullptr, "create reference fail");
    napi_property_descriptor moduleFuncs[] = {
        DECLARE_NAPI_FUNCTION("createVideoProcessor", VideoProcessorNapi::CreateVideoProcessor),
    };
    napi_define_properties(env, exports, sizeof(moduleFuncs) / sizeof(moduleFuncs[0]), moduleFuncs);
    return exports;
}
 
napi_value VideoProcessorNapi::CreateVideoProcessor(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_value constructor = nullptr;
    CHECK_AND_RETURN_RET_LOG(napi_get_reference_value(env, constructor_, &constructor) == napi_ok,
        nullptr, "CreateVideoProcessor: get constructor failed");
    size_t argc = NUM_1;
    napi_value argv[NUM_1] = {0};
    CHECK_AND_RETURN_RET_LOG(napi_new_instance(env, constructor, argc, argv, &result) == napi_ok,
        nullptr, "CreateVideoProcessor: new instance failed");
    VPE_LOGI("create done");
    return result;
}
}
}
