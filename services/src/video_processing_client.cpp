/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <mutex>

#include "video_processing_service_manager_proxy.h"
#include "iservice_registry.h"
#include "surface_buffer.h"
#include "event_handler.h"
#include "event_runner.h"
#include "video_processing_load_callback.h"
#include "vpe_log.h"

namespace OHOS { class IRemoteObject; }
namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
namespace {
std::mutex g_proxyLock;
const int32_t VIDEO_PROCESSING_SERVER_SA_ID = 0x00010256;
const int LOADSA_TIMEOUT_MS = 1000;
}

VideoProcessingManager& VideoProcessingManager::GetInstance()
{
    static VideoProcessingManager instance;
    return instance;
}

void VideoProcessingManager::Connect()
{
    std::unique_lock<std::mutex> lock(g_proxyLock);
    g_proxy = nullptr;
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_LOG(samgr != nullptr, "GetSystemAbilityManager return null");
    VPE_LOGD("GetSystemAbilityManager return is not null, success");
    auto object = samgr->CheckSystemAbility(VIDEO_PROCESSING_SERVER_SA_ID);
    if (object != nullptr) {
        VPE_LOGD("object is not null, get service succeed");
        g_proxy = iface_cast<IVideoProcessingServiceManager>(object);
        return;
    }
    VPE_LOGD("object is null, Start load SA");
    sptr<VideoProcessingLoadCallback> loadCallback = new VideoProcessingLoadCallback();
    CHECK_AND_RETURN_LOG(loadCallback != nullptr, "New VideoProcessingLoadCallback fail!");
    int32_t ret = samgr->LoadSystemAbility(VIDEO_PROCESSING_SERVER_SA_ID, loadCallback);
    CHECK_AND_RETURN_LOG(ret == ERR_OK, "LoadSystemAbility %{public}d failed!", VIDEO_PROCESSING_SERVER_SA_ID);
    VPE_LOGD("VideoProcessingService SA load start!");
    auto waitStatus = g_proxyConVar.wait_for(lock, std::chrono::milliseconds(LOADSA_TIMEOUT_MS),
                                             [this]() { return g_proxy != nullptr; });
    VPE_LOGD("VideoProcessingService SA load end!");
    if (!waitStatus) {
        VPE_LOGE("VideoProcessingService SA load timeout!");
        return;
    }
    if (waitStatus && g_proxy != nullptr) {
        VPE_LOGI("Load VideoProcessingService success");
    }
    return;
}

void VideoProcessingManager::Disconnect()
{
    VPE_LOGD("VideoProcessingManager Disconnect!");
    return;
}

ErrCode VideoProcessingManager::LoadInfo(int32_t key, SurfaceBufferInfo& bufferInfo)
{
    std::lock_guard<std::mutex> lock(g_proxyLock);
    CHECK_AND_RETURN_RET_LOG(g_proxy != nullptr, ERR_NULL_OBJECT, "LoadInfo: g_proxy is nullptr!");
    return g_proxy->LoadInfo(key, bufferInfo);
}

void VideoProcessingManager::LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject)
{
    VPE_LOGI("Get VideoProcessingService SA success!");
    std::unique_lock<std::mutex> lock(g_proxyLock);
    if (remoteObject != nullptr) {
        VPE_LOGD("remoteObject is not null.");
        g_proxy = iface_cast<IVideoProcessingServiceManager>(remoteObject);
        g_proxyConVar.notify_one();
    }
}

void VideoProcessingManager::LoadSystemAbilityFail()
{
    VPE_LOGE("Get VideoProcessingService SA failed!");
    std::unique_lock<std::mutex> lock(g_proxyLock);
    g_proxy = nullptr;
}
}
}
}