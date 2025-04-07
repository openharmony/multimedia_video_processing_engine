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

#include "video_processing_client.h"

#include "iservice_registry.h"
#include "video_processing_load_callback.h"
#include "vpe_sa_constants.h"

using namespace OHOS::Media::VideoProcessingEngine;
using namespace OHOS;
using namespace std::chrono_literals;
using namespace std::placeholders;

using VpeSa = IVideoProcessingServiceManager;

namespace {
std::mutex g_proxyLock;
constexpr int ERROR_DEAD_REPLY = 29189;
constexpr int DEAD_RETRY_COUNT = 5;
}

VideoProcessingManager& VideoProcessingManager::GetInstance()
{
    static VideoProcessingManager instance;
    return instance;
}

void VideoProcessingManager::Connect()
{
    VPE_LOGD("call GetService");
    GetService();
}

void VideoProcessingManager::Disconnect()
{
    VPE_LOGD("VideoProcessingManager Disconnect!");
    return;
}

ErrCode VideoProcessingManager::LoadInfo(int32_t key, SurfaceBufferInfo& bufferInfo)
{
    std::lock_guard<std::mutex> lock(g_proxyLock);
    return Execute([&key, &bufferInfo](sptr<VpeSa>& proxy) { return proxy->LoadInfo(key, bufferInfo); }, VPE_LOG_INFO);
}

void VideoProcessingManager::LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject)
{
    VPE_LOGI("Get VideoProcessingService SA success!");
    std::unique_lock<std::mutex> lock(g_proxyLock);
    if (remoteObject != nullptr) {
        VPE_LOGD("remoteObject is not null.");
        proxy_ = iface_cast<IVideoProcessingServiceManager>(remoteObject);
        cvProxy_.notify_one();
    }
}

void VideoProcessingManager::LoadSystemAbilityFail()
{
    VPE_LOGE("Get VideoProcessingService SA failed!");
    std::unique_lock<std::mutex> lock(g_proxyLock);
    proxy_ = nullptr;
}

VPEAlgoErrCode VideoProcessingManager::Create(const std::string& feature, const std::string& clientName,
    uint32_t& clientID)
{
    int id;
    auto ret = Execute(std::bind(&VpeSa::Create, _1, feature, clientName, std::ref(id)), VPE_LOG_INFO);
    if (ret == ERR_NONE) {
        clientID = static_cast<uint32_t>(id);
        VPE_LOGD("feature:%{public}s client:%{public}s -> ID:%{public}d", feature.c_str(), clientName.c_str(), id);
    } else {
        VPE_LOGE("feature:%{public}s client:%{public}s failed! ret:%{public}d", feature.c_str(), clientName.c_str(),
            ret);
    }
    return ret;
}

VPEAlgoErrCode VideoProcessingManager::Destroy(uint32_t clientID)
{
    return Execute(std::bind(&VpeSa::Destroy, _1, clientID), VPE_LOG_INFO);
}

VPEAlgoErrCode VideoProcessingManager::SetParameter(uint32_t clientID, int32_t tag,
    const std::vector<uint8_t>& parameter)
{
    return Execute(std::bind(&VpeSa::SetParameter, _1, clientID, tag, parameter), VPE_LOG_INFO);
}

VPEAlgoErrCode VideoProcessingManager::SetParameter(uint32_t clientID, int32_t tag)
{
    std::vector<uint8_t> param;
    return SetParameter(clientID, tag, param);
}

VPEAlgoErrCode VideoProcessingManager::GetParameter(uint32_t clientID, int32_t tag, std::vector<uint8_t>& parameter)
{
    return Execute(std::bind(&VpeSa::GetParameter, _1, clientID, tag, parameter), VPE_LOG_INFO);
}

VPEAlgoErrCode VideoProcessingManager::UpdateMetadata(uint32_t clientID, SurfaceBufferInfo& image)
{
    return Execute(std::bind(&VpeSa::UpdateMetadata, _1, clientID, image), VPE_LOG_INFO);
}

VPEAlgoErrCode VideoProcessingManager::Process(uint32_t clientID, const SurfaceBufferInfo& input,
    SurfaceBufferInfo& output)
{
    return Execute(std::bind(&VpeSa::Process, _1, clientID, input, output), VPE_LOG_INFO);
}

VPEAlgoErrCode VideoProcessingManager::ComposeImage(uint32_t clientID, const SurfaceBufferInfo& inputSdrImage,
    const SurfaceBufferInfo& inputGainmap, SurfaceBufferInfo& outputHdrImage, bool legacy)
{
    return Execute(std::bind(&VpeSa::ComposeImage, _1, clientID, inputSdrImage, inputGainmap, outputHdrImage, legacy),
        VPE_LOG_INFO);
}

VPEAlgoErrCode VideoProcessingManager::DecomposeImage(uint32_t clientID, const SurfaceBufferInfo& inputImage,
    SurfaceBufferInfo& outputSdrImage, SurfaceBufferInfo& outputGainmap)
{
    return Execute(std::bind(&VpeSa::DecomposeImage, _1, clientID, inputImage, outputSdrImage, outputGainmap),
        VPE_LOG_INFO);
}

sptr<IVideoProcessingServiceManager> VideoProcessingManager::GetService()
{
    do {
        std::lock_guard<std::mutex> lock(lock_);
        if (proxy_ != nullptr) {
            if (proxy_->AsObject() != nullptr && !proxy_->AsObject()->IsObjectDead()) [[likely]] {
                return proxy_;
            }
            VPE_LOGD("SA remote died.");
            ClearSaLocked();
        }

        if (isLoading_.load()) {
            VPE_LOGD("SA is loading, so wait for the result directly .");
            break;
        }

        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr, "Failed to GetSystemAbilityManager!");
        VPE_LOGD("Try to check VPE SA.");
        auto object = samgr->CheckSystemAbility(VIDEO_PROCESSING_SERVER_SA_ID);
        if (object != nullptr) {
            proxy_ = iface_cast<IVideoProcessingServiceManager>(object);
            VPE_LOGD("SA is already start");
            return proxy_;
        }

        sptr<LoadCallback> loadCallback = new(std::nothrow) LoadCallback(
            std::bind(&VideoProcessingManager::OnSaLoad, this, _1),
            std::bind(&VideoProcessingManager::OnSaLoad, this, nullptr));
        CHECK_AND_RETURN_RET_LOG(loadCallback != nullptr, nullptr, "Failed to create LoadCallback!");
        VPE_LOGD("Loading VPE SA...");
        CHECK_AND_RETURN_RET_LOG(samgr->LoadSystemAbility(VIDEO_PROCESSING_SERVER_SA_ID, loadCallback) == ERR_OK,
            nullptr, "Failed to load VPE SA!");
        isLoading_ = true;
    } while (false);

    std::unique_lock lock(lock_);
    CHECK_AND_RETURN_RET_LOG(cvProxy_.wait_for(lock, 1s, [this] { return !isLoading_.load(); }), nullptr,
        "load SA timeout!");
    if (proxy_ != nullptr) {
        VPE_LOGD("Finish loading VPE SA success.");
    } else {
        VPE_LOGE("Finish loading VPE SA fail!");
    }
    return proxy_;
}

void VideoProcessingManager::OnSaLoad(const sptr<IRemoteObject>& remoteObject)
{
    {
        std::lock_guard<std::mutex> lock(lock_);
        if (remoteObject != nullptr) {
            sptr<DeathObserver> observer = new(std::nothrow) DeathObserver(
                std::bind(&VideoProcessingManager::OnSaDied, this, _1));
            CHECK_AND_RETURN_LOG(observer != nullptr, "Failed to create DeathObserver!");
            CHECK_AND_RETURN_LOG(remoteObject->AddDeathRecipient(observer), "Failed to AddDeathRecipient!");
            VPE_LOGD("AddDeathRecipient success.");
            proxy_ = iface_cast<IVideoProcessingServiceManager>(remoteObject);
            VPE_LOGI("SA load success.");
        } else {
            proxy_ = nullptr;
            VPE_LOGE("SA load fail!");
        }
        isLoading_ = false;
    }
    cvProxy_.notify_all();
}

void VideoProcessingManager::OnSaDied([[maybe_unused]] const wptr<IRemoteObject>& remoteObject)
{
    std::lock_guard<std::mutex> lock(lock_);
    proxy_ = nullptr;
}

VPEAlgoErrCode VideoProcessingManager::Execute(
    std::function<ErrCode(sptr<IVideoProcessingServiceManager>&)>&& operation, const LogInfo& logInfo)
{
    auto proxy = GetService();
    if (proxy == nullptr) [[unlikely]] {
        VPE_ORG_LOGE(logInfo, "proxy is null!");
        return VPE_ALGO_ERR_INVALID_STATE;
    }
    auto err = static_cast<VPEAlgoErrCode>(operation(proxy));
    if (err == ERROR_DEAD_REPLY) {
        ClearSa();
        if (deadRetryCount_.load() < DEAD_RETRY_COUNT) {
            deadRetryCount_++;
            VPE_ORG_LOGD(logInfo, "<%{public}d> Retry to check SA again for dead reply.", deadRetryCount_.load());
            return Execute(std::move(operation), logInfo);
        }
    } else {
        deadRetryCount_.store(0);
    }
    return err;
}

void VideoProcessingManager::ClearSa()
{
    std::lock_guard<std::mutex> lock(lock_);
    ClearSaLocked();
}

void VideoProcessingManager::ClearSaLocked()
{
    proxy_ = nullptr;
    isLoading_ = false;
}

void VideoProcessingManager::LoadCallback::OnLoadSystemAbilitySuccess([[maybe_unused]] int32_t systemAbilityId,
    const sptr<IRemoteObject>& remoteObject)
{
    onSuccess_(remoteObject);
}

void VideoProcessingManager::LoadCallback::OnLoadSystemAbilityFail([[maybe_unused]] int32_t systemAbilityId)
{
    onFail_();
}

void VideoProcessingManager::DeathObserver::OnRemoteDied(const wptr<IRemoteObject>& remoteObject)
{
    onRemoteDied_(remoteObject);
}