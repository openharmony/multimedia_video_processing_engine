/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
 
#include "auto_effect_aisr_video.h"
 
#include "algorithm_utils.h"
#include "extension_manager.h"
#include "securec.h"
#include "vpe_controller_common.h"
#include "vpe_log.h"
#include "vpe_sa_utils.h"
#include "video_processing_client.h"
#include "v2_2/buffer_handle_meta_key_type.h"
 
using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;
using namespace OHOS::HDI::Display::Graphic::Common;
 
namespace {
std::mutex g_autoEffectLock{};
std::unordered_map<std::string, bool> g_autoEffectMap{};
std::unordered_map<std::string, std::weak_ptr<VpeVideo>> g_vpeMap{};
constexpr int FEATURE_PARAM_GET_SUPPORTED = 1;
constexpr int PRODUCT_PARAM_GET_SUPPORTED = 3;
} // namespace
 
std::shared_ptr<VpeVideoImpl> AutoEffectAisrVideo::Create()
{
    VPE_LOGI("AutoEffectAisrVideo::Create start");
    auto obj = std::make_shared<AutoEffectAisrVideo>(VIDEO_TYPE_AUTO_EFFECT_AISR, true);
    CHECK_AND_RETURN_RET_LOG(obj != nullptr, nullptr, "Failed to create auto effect!");
    CHECK_AND_RETURN_RET_LOG(obj->Initialize() == VPE_ALGO_ERR_OK, nullptr, "Failed to initialize auto effect!");
    VPE_LOGI("AutoEffectAisrVideo::Create success");
    return obj;
}
 
bool AutoEffectAisrVideo::IsSupported(const OHOS::Media::Format& parameter)
{
    std::string name;
    if (!parameter.GetStringValue(ParameterKey::SURFACE_NODE_NAME, name)) {
        VPE_LOGE("IsSupported: autoEffectName not found in parameter!");
        return false;
    }
    std::lock_guard<std::mutex> lock(g_autoEffectLock);
    auto it = g_autoEffectMap.find(name);
    if (it == g_autoEffectMap.end() || !it->second) {
        VPE_LOGI("IsSupported: name %{public}s not enabled in local map", name.c_str());
        return false;
    }
    // Check if VPE instance already exists for this name
    auto vpeIt = g_vpeMap.find(name);
    if (vpeIt != g_vpeMap.end() && !vpeIt->second.expired()) {
        VPE_LOGI("IsSupported: VPE instance already exists for name: %{public}s", name.c_str());
        return false;
    }
    return true;
}
 
bool AutoEffectAisrVideo::IsSurfaceSupported(const sptr<Surface>& surface)
{
    return true;
}
 
bool AutoEffectAisrVideo::IsProductSupported()
{
    static std::atomic<bool> cached{false};
    static std::atomic<bool> queried{false};
    if (queried.load()) {
        return cached.load();
    }
    struct FeatureParameter param = { false, "" };
    std::vector<uint8_t> parameter = VpeSaUtils::StructToVector(param);
    ErrCode ret = VideoProcessingManager::GetInstance().GetFeatureParameter(VIDEO_TYPE_DETAIL_ENHANCER,
        CTRL_TAG_AUTO_EFFECT_AISR_ENABLE, parameter);
    if (ret == VPE_ALGO_ERR_SA_NOT_READY) {
        VPE_LOGI("IsSystemSupported: SA not ready, wait for SA sync load");
        VideoProcessingManager::GetInstance().LoadSync();
        ret = VideoProcessingManager::GetInstance().GetFeatureParameter(VIDEO_TYPE_DETAIL_ENHANCER,
            CTRL_TAG_AUTO_EFFECT_AISR_ENABLE, parameter);
    }
    if (ret != VPE_ALGO_ERR_OK) {
        VPE_LOGE("IsSystemSupported: GetFeatureParameter failed %{public}d", ret);
        return false;
    }
    if (parameter.size() != sizeof(struct FeatureParameter)) {
        VPE_LOGE("IsSystemSupported: vector size mismatch");
        return false;
    }
    auto* getPara = reinterpret_cast<struct FeatureParameter*>(parameter.data());
    cached = getPara->isSupported;
    queried = true;
    VPE_LOGI("IsSystemSupported: %{public}d", cached.load());
    return cached.load();
}
 
VPEAlgoErrCode AutoEffectAisrVideo::UseAutoEffect(uint32_t type, bool enable, const char* name)
{
    if (name == nullptr) {
        VPE_LOGE("UseAutoEffect name is null!");
        return VPE_ALGO_ERR_INVALID_VAL;
    }
    std::string mapKey = std::string(name) + "Surface";
    std::lock_guard<std::mutex> lock(g_autoEffectLock);
    if (g_autoEffectMap.find(mapKey) != g_autoEffectMap.end()) {
        VPE_LOGE("UseAutoEffect name already registered: %{public}s", name);
        return VPE_ALGO_ERR_INVALID_OPERATION;
    }
    g_autoEffectMap[mapKey] = enable;
    VPE_LOGI("UseAutoEffect type: 0x%{public}x, enable: %{public}d, name: %{public}s, mapKey: %{public}s",
        type, enable, name, mapKey.c_str());
    return VPE_ALGO_ERR_OK;
}
 
VPEAlgoErrCode AutoEffectAisrVideo::SetAutoEffectEnabled(bool enable, bool hasEnable,
    const char* name, float strength, bool hasStrength)
{
    VPE_LOGI("SetAutoEffectEnabled enable: %{public}d (has:%{public}d), name: %{public}s, "
        "strength: %{public}f (has:%{public}d)", enable, hasEnable, name, strength, hasStrength);
    if (name == nullptr) {
        VPE_LOGE("SetAutoEffectEnabled name is null!");
        return VPE_ALGO_ERR_INVALID_VAL;
    }
    std::string mapKey = std::string(name) + "Surface";
    std::lock_guard<std::mutex> lock(g_autoEffectLock);
 
    auto vpeIt = g_vpeMap.find(mapKey);
    if (vpeIt == g_vpeMap.end() || vpeIt->second.expired()) {
        VPE_LOGE("SetAutoEffectEnabled: no valid vpe instance for name: %{public}s, mapSize: %{public}zu",
            mapKey.c_str(), g_vpeMap.size());
        return VPE_ALGO_ERR_INVALID_STATE;
    }
    auto instance = std::static_pointer_cast<AutoEffectAisrVideo>(vpeIt->second.lock());
    if (instance == nullptr) {
        VPE_LOGE("SetAutoEffectEnabled: failed to lock vpe instance for name: %{public}s", name);
        return VPE_ALGO_ERR_INVALID_STATE;
    }
 
    if (!enable) {
        instance->SetAutoEffectEnabledInner(false, hasEnable, strength, hasStrength);
        return VPE_ALGO_ERR_OK;
    }
    // Disable all other VPE instances first
    for (auto& [key, weakPtr] : g_vpeMap) {
        if (weakPtr.expired()) {
            continue;
        }
        auto inst = std::static_pointer_cast<AutoEffectAisrVideo>(weakPtr.lock());
        if (inst != nullptr) {
            inst->SetAutoEffectAisrEnable(false);
        }
    }
    instance->SetAutoEffectEnabledInner(true, hasEnable, strength, hasStrength);
    return VPE_ALGO_ERR_OK;
}
 
AutoEffectAisrVideo::~AutoEffectAisrVideo()
{
    // Unregister from g_vpeMap and clear corresponding entry from g_autoEffectMap
    {
        std::lock_guard<std::mutex> lock(g_autoEffectLock);
        if (!effectName_.empty()) {
            std::string mapKey = effectName_ + "Surface";
            g_autoEffectMap.erase(mapKey);
            VPE_LOGI("~AutoEffectAisrVideo: erased g_autoEffectMap for %{public}s", mapKey.c_str());
            auto vpeIt = g_vpeMap.find(mapKey);
            if (vpeIt != g_vpeMap.end()) {
                g_vpeMap.erase(vpeIt);
                VPE_LOGI("~AutoEffectAisrVideo: erased g_vpeMap for %{public}s", mapKey.c_str());
            }
            effectName_.clear();
        }
    }
    Release();
}
 
VPEAlgoErrCode AutoEffectAisrVideo::OnInitialize()
{
    VPE_LOGI("AutoEffectAisrVideo::OnInitialize start");
    std::shared_ptr<DetailEnhancerBase> algo;
    {
        std::lock_guard<std::mutex> lock(lock_);
        if (algo_ != nullptr) {
            VPE_LOGI("AutoEffectAisrVideo::OnInitialize already initialized, skip");
            return VPE_ALGO_ERR_OK;
        }
        algo_ = Extension::ExtensionManager::GetInstance().CreateAutoEffectAisr();
        algo = algo_;
        CHECK_AND_RETURN_RET_LOG(algo != nullptr, VPE_ALGO_ERR_UNKNOWN, "Failed to create algorithm!");
    }
 
    CHECK_AND_RETURN_RET_LOG(algo->Init() == VPE_ALGO_ERR_OK, VPE_ALGO_ERR_UNKNOWN, "Failed to init algorithm!");
 
    // Connect to VideoProcessingManager and register server listener for policy control
    VideoProcessingManager::GetInstance().Connect();
    CHECK_AND_RETURN_RET_LOG(RegisterServerListener(), VPE_ALGO_ERR_UNKNOWN, "Failed to register server listener!");
 
    // Enable protection if auto-disable is set
    if (isAutoDisable_) {
        CHECK_AND_RETURN_RET_LOG(algo->EnableProtection(true) == VPE_ALGO_ERR_OK,
            VPE_ALGO_ERR_UNKNOWN, "Failed to enable protection!");
    }
    VPE_LOGI("AutoEffectAisrVideo::OnInitialize success, algo: %{public}p", algo.get());
    return VPE_ALGO_ERR_OK;
}
 
VPEAlgoErrCode AutoEffectAisrVideo::OnDeinitialize()
{
    std::lock_guard<std::mutex> lock(lock_);
    UnRegisterAlgorithmCallbackLocked();
    UnregisterServerListener();
    if (algo_ != nullptr) {
        VPE_LOGI("OnDeinitialize");
        algo_->Deinit();
        algo_ = nullptr;
    }
    return VPE_ALGO_ERR_OK;
}
 
VPEAlgoErrCode AutoEffectAisrVideo::Process(const sptr<SurfaceBuffer>& sourceImage,
    sptr<SurfaceBuffer>& destinationImage, sptr<SyncFence> infenceFd)
{
    VPE_LOGI("AutoEffectAisrVideo::Process start, strength: %{public}f", strength_.load());
    CHECK_AND_RETURN_RET_LOG(sourceImage != nullptr && destinationImage != nullptr, VPE_ALGO_ERR_INVALID_VAL,
        "Invalid input: source or destination image is null!");
    CHECK_AND_RETURN_RET_LOG(IsInitialized(), VPE_ALGO_ERR_INVALID_OPERATION, "NOT initialized!");
    if (!RegisterAlgorithmCallback()) {
        VPE_LOGE("Process: RegisterAlgorithmCallback failed");
        return VPE_ALGO_ERR_INVALID_STATE;
    }
    std::shared_ptr<DetailEnhancerBase> algo;
    {
        std::lock_guard<std::mutex> lock(lock_);
        algo = algo_;
    }
    CHECK_AND_RETURN_RET_LOG(algo != nullptr, VPE_ALGO_ERR_INVALID_STATE, "algo_ is null!");
    // Execute algorithm: input cache invalidation -> process -> output cache flush
#ifndef USE_VIDEO_PROCESSING_ENGINE_HVISIONSR
    GSError gsErr = sourceImage->InvalidateCache();
    if (gsErr != GSERROR_OK) {
        VPE_LOGE("InvalidateCache failed, GSError = %{public}d", gsErr);
        return VPE_ALGO_ERR_UNKNOWN;
    }
    gsErr = sourceImage->FlushCache();
    if (gsErr != GSERROR_OK) {
        VPE_LOGE("FlushCache failed, GSError = %{public}d", gsErr);
        return VPE_ALGO_ERR_UNKNOWN;
    }
#endif
    VPEAlgoErrCode ret = algo->Process(sourceImage, destinationImage);
    VPE_LOGI("AutoEffectAisrVideo::Process algo_->Process ret: %{public}d", ret);
#ifndef USE_VIDEO_PROCESSING_ENGINE_HVISIONSR
    gsErr = destinationImage->InvalidateCache();
    if (gsErr != GSERROR_OK) {
        VPE_LOGE("InvalidateCache failed, GSError = %{public}d", gsErr);
        return VPE_ALGO_ERR_UNKNOWN;
    }
    gsErr = destinationImage->FlushCache();
    if (gsErr != GSERROR_OK) {
        VPE_LOGE("FlushCache failed, GSError = %{public}d", gsErr);
        return VPE_ALGO_ERR_UNKNOWN;
    }
#endif
    VPE_LOGI("AutoEffectAisrVideo::Process done, ret:%{public}d, strength:%{public}f", ret, strength_.load());
    return ret;
}
 
VPEAlgoErrCode AutoEffectAisrVideo::SetParameter(const Format& parameter)
{
    std::function<ParamError(const Format&)> setters[] = {
        [this](const Format& parameter) { return SetAutoEffectName(parameter); },
        [this](const Format& parameter) { return SetNodeId(parameter); },
    };
 
    CHECK_AND_RETURN_RET_LOG(IsInitialized(), VPE_ALGO_ERR_INVALID_OPERATION, "NOT initialized!");
 
    std::lock_guard<std::mutex> lock(lock_);
    int setCount = 0;
    for (auto& setter : setters) {
        int err = setter(parameter);
        if (err == PARAM_ERR_INVALID) {
            return VPE_ALGO_ERR_INVALID_VAL;
        }
        setCount += err;
    }
    CHECK_AND_RETURN_RET_LOG(setCount > 0, VPE_ALGO_ERR_INVALID_VAL, "Invalid input: NO valid parameters!");
    return VPE_ALGO_ERR_OK;
}
 
VPEAlgoErrCode AutoEffectAisrVideo::SetAutoEffectEnabledInner(bool enable, bool hasEnable, float strength,
    bool hasStrength)
{
    // Use previously stored value if not provided
    bool actualEnable = hasEnable ? enable : GetAutoEffectAisrEnable();
    float actualStrength = hasStrength ? strength : strength_.load();
    VPE_LOGI("SetAutoEffectEnabledInner enable: %{public}d (has:%{public}d), strength: %{public}f (has:%{public}d)",
        actualEnable, hasEnable, actualStrength, hasStrength);
    if (actualEnable) {
        strength_ = actualStrength;
        // Re-initialize algo_ if cleared (e.g. after OnServerDied)
        bool needInit = false;
        {
            std::lock_guard<std::mutex> lock(lock_);
            needInit = (algo_ == nullptr);
        }
        if (needInit) {
            VPE_LOGI("SetAutoEffectEnabledInner: algo_ is null, re-initializing");
            VPEAlgoErrCode ret = OnInitialize();
            if (ret != VPE_ALGO_ERR_OK) {
                VPE_LOGE("SetAutoEffectEnabledInner: OnInitialize failed: %{public}d", ret);
                return ret;
            }
        }
        SetAutoEffectAisrEnable(actualEnable);
        auto startResult = Start();
        if (startResult == VPE_ALGO_ERR_OK) {
            VPE_LOGI("SetAutoEffectEnabledInner Start success");
        }
    } else {
        SetAutoEffectAisrEnable(actualEnable);
    }
    return VPE_ALGO_ERR_OK;
}
 
VPEAlgoErrCode AutoEffectAisrVideo::GernerateAisrMetadata(sptr<SurfaceBuffer>& srcBuffer,
    sptr<SurfaceBuffer>& dstBuffer, sptr<SyncFence> infenceFd)
{
    if (infenceFd != nullptr && infenceFd->IsValid()) {
        infenceFd->Wait(WAIT_FOR_EVER);
        srcBuffer->InvalidateCache();
    }
    if (strength_.load() < 0.0f) {
        return Process(srcBuffer, srcBuffer);
    }
    std::vector<uint8_t> param;
    param.resize(sizeof(float));
    *reinterpret_cast<float*>(param.data()) = strength_.load();
    GSError result = srcBuffer->SetMetadata(V2_2::ATTRKEY_HDR_DYNAMIC_METADATA, param);
    VPE_LOGD("GernerateAisrMetadata: strength set, SetMetadata result:%{public}d", result);
    return VPE_ALGO_ERR_OK;
}
 
bool AutoEffectAisrVideo::RegisterAlgorithmCallback()
{
    std::lock_guard<std::mutex> lock(lock_);
    return RegisterAlgorithmCallbackLocked();
}
 
bool AutoEffectAisrVideo::RegisterAlgorithmCallbackLocked()
{
    if (algo_ == nullptr) {
        VPE_LOGE("RegisterAlgorithmCallbackLocked: algo_ is null");
        return false;
    }
    if (listener_ != nullptr) {
        VPE_LOGI("RegisterAlgorithmCallbackLocked: already registered");
        return true;
    }
    listener_ = std::make_shared<AlgorithmCallback>(*this);
    CHECK_AND_RETURN_RET_LOG(listener_ != nullptr, false, "Failed to create algorithm callback!");
    VPEAlgoErrCode err = algo_->RegisterCallback(listener_);
    CHECK_AND_RETURN_RET_LOG(err == VPE_ALGO_ERR_OK, false, "Failed to register callback, ret=%{public}d!", err);
    VPE_LOGI("RegisterAlgorithmCallbackLocked success");
    return true;
}
 
bool AutoEffectAisrVideo::UnRegisterAlgorithmCallback()
{
    std::lock_guard<std::mutex> lock(lock_);
    return UnRegisterAlgorithmCallbackLocked();
}
 
bool AutoEffectAisrVideo::UnRegisterAlgorithmCallbackLocked()
{
    if (listener_ == nullptr) {
        return true;
    }
    if (algo_ == nullptr) {
        VPE_LOGE("UnRegisterAlgorithmCallbackLocked: algo_ is null");
        listener_ = nullptr;
        return true;
    }
    VPEAlgoErrCode err = algo_->UnregisterCallback();
    listener_ = nullptr;
    CHECK_AND_RETURN_RET_LOG(err == VPE_ALGO_ERR_OK, false, "Failed to unregister callback, ret=%{public}d!", err);
    return true;
}
 
bool AutoEffectAisrVideo::RegisterServerListener()
{
    std::lock_guard<std::mutex> lock(lock_);
    if (serverListener_ != nullptr) {
        return true;
    }
    serverListener_ = std::make_shared<ServerListener>(*this);
    CHECK_AND_RETURN_RET_LOG(serverListener_ != nullptr, false, "Failed to create server listener!");
    VPEAlgoErrCode err = VideoProcessingManager::GetInstance().RegisterServerListener(serverListener_);
    CHECK_AND_RETURN_RET_LOG(err == VPE_ALGO_ERR_OK, false, "Failed to register server listener, ret=%{public}d!", err);
    return true;
}
 
void AutoEffectAisrVideo::UnregisterServerListener()
{
    if (serverListener_ == nullptr) {
        return;
    }
    VPEAlgoErrCode err = VideoProcessingManager::GetInstance().UnregisterServerListener(serverListener_);
    serverListener_ = nullptr;
    if (err != VPE_ALGO_ERR_OK) {
        VPE_LOGE("Failed to unregister server listener, ret=%{public}d!", err);
    }
}
 
void AutoEffectAisrVideo::ServerListener::OnServerDied()
{
    VPE_LOGI("ServerListener::OnServerDied");
    std::lock_guard<std::mutex> lock(owner_.lock_);
    owner_.UnRegisterAlgorithmCallbackLocked();
    if (owner_.algo_ != nullptr) {
        owner_.algo_->Deinit();
        owner_.algo_ = nullptr;
    }
    owner_.UnregisterServerListener();
}
 
void AutoEffectAisrVideo::AlgorithmCallback::OnPolicyControl(bool isAlgorithmEnable)
{
    VPE_LOGI("OnPolicyControl: %{public}d", isAlgorithmEnable);
    if (isAlgorithmEnable) {
        owner_.Enable();
    } else {
        owner_.Disable();
    }
}
 
void AutoEffectAisrVideo::RegisterInVpeMap(const std::string& name)
{
    std::lock_guard<std::mutex> lock(g_autoEffectLock);
    effectName_ = name;
    g_vpeMap.emplace(name, shared_from_this());
    VPE_LOGI("RegisterInVpeMap: name=%{public}s, mapSize=%{public}zu", name.c_str(), g_vpeMap.size());
}
 
AutoEffectAisrVideo::ParamError AutoEffectAisrVideo::SetAutoEffectName(const Format& parameter)
{
    VPE_LOGI("SetAutoEffectName start");
    std::string name;
    if (!parameter.GetStringValue(ParameterKey::SURFACE_NODE_NAME, name)) {
        return PARAM_ERR_NOT_FOUND;
    }
    VPE_LOGI("SetAutoEffectName: %{public}s", name.c_str());
    RegisterInVpeMap(name);
    return PARAM_ERR_OK;
}
 
AutoEffectAisrVideo::ParamError AutoEffectAisrVideo::SetNodeId(const Format& parameter)
{
    int64_t nodeId = 0;
    if (!parameter.GetLongValue(ParameterKey::DETAIL_ENHANCER_NODE_ID, nodeId)) {
        return PARAM_ERR_NOT_FOUND;
    }
    VPE_LOGI("SetNodeId: %{public}" PRId64, nodeId);
    if (!RegisterAlgorithmCallbackLocked()) {
        VPE_LOGE("SetNodeId: RegisterAlgorithmCallbackLocked failed");
        return PARAM_ERR_INVALID;
    }
    CHECK_AND_RETURN_RET_LOG(algo_ != nullptr, PARAM_ERR_INVALID, "SetNodeId: algo_ is null");
    VPEAlgoErrCode ret = algo_->SetNodeId(static_cast<uint64_t>(nodeId));
    if (ret == VPE_ALGO_ERR_INVALID_VAL) {
        return PARAM_ERR_NOT_FOUND;
    }
    if (ret != VPE_ALGO_ERR_OK) {
        VPE_LOGE("Failed to set nodeId to algorithm, ret=%{public}d", ret);
        return PARAM_ERR_INVALID;
    }
    return PARAM_ERR_OK;
}
 
VPEAlgoErrCode AutoEffectAisrVideo::UpdateRequestCfg(const sptr<Surface>& surface,
    BufferRequestConfig& requestCfg)
{
    CHECK_AND_RETURN_RET_LOG(surface != nullptr, VPE_ALGO_ERR_INVALID_VAL, "surface is null!");
    requestCfg.width = surface->GetRequestWidth();
    requestCfg.height = surface->GetRequestHeight();
    if (requestCfg.width == 0 || requestCfg.height == 0) {
        requestCfg.width = surface->GetDefaultWidth();
        requestCfg.height = surface->GetDefaultHeight();
    }
    return VPE_ALGO_ERR_OK;
}
 
void AutoEffectAisrVideo::UpdateRequestCfg(const sptr<SurfaceBuffer>& consumerBuffer,
    BufferRequestConfig& requestCfg)
{
    CHECK_AND_RETURN_LOG(consumerBuffer != nullptr, "surface buffer is null!");
    if (requestCfg.width == 0 || requestCfg.height == 0) {
        requestCfg.width = consumerBuffer->GetWidth();
        requestCfg.height = consumerBuffer->GetHeight();
    }
    requestCfg.format = consumerBuffer->GetFormat();
}