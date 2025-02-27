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

#include "video_processing_server.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <iremote_object.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "surface_buffer.h"
#include "event_handler_factory.h"
#include "vpe_log.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
namespace {
    const int VPE_INFO_FILE_MAX_LENGTH = 20485760;
    const int32_t VIDEO_PROCESSING_SERVER_SA_ID = 0x00010256;
    const std::string UNLOAD_TASK_ID = "unload_vpe_svr";
    constexpr int32_t DELAY_TIME = 180000;
    REGISTER_SYSTEM_ABILITY_BY_ID(VideoProcessingServer, VIDEO_PROCESSING_SERVER_SA_ID, false);
}

VideoProcessingServer::VideoProcessingServer(int32_t saId, bool runOnCreate) : SystemAbility(saId, runOnCreate)
{
    VPE_LOGD("VideoProcessingServer construct!");
}
VideoProcessingServer::~VideoProcessingServer()
{
    VPE_LOGD("VideoProcessingServer destruction!");
    UnInit();
}

int32_t VideoProcessingServer::Init()
{
    VPE_LOGD("VideoProcessingServer init begin");
    if (EventHandlerFactory::GetInstance().Init() != ERR_NONE) {
        VPE_LOGE("EventHandlerFactory init failed");
        return ERR_NULL_OBJECT;
    }
    VPE_LOGD("VideoProcessingServer init finish");
    return ERR_NONE;
}

bool VideoProcessingServer::IsInited()
{
    return isInited_;
}

int32_t VideoProcessingServer::UnInit()
{
    isInited_ = false;
    if (EventHandlerFactory::GetInstance().UnInit() != ERR_NONE) {
        VPE_LOGE("EventHandlerFactory UnInit failed");
        return ERR_NULL_OBJECT;
    }
    VPE_LOGD("VideoProcessingServer uninit succeeded");
    return ERR_NONE;
}

int32_t VideoProcessingServer::CreateUnloadHandler()
{
    std::lock_guard<std::mutex> lock(unloadMutex_);
    if (unloadHandler_ == nullptr) {
        unloadHandler_ = EventHandlerFactory::GetInstance().CreateEventHandler("unload_vpe_sa_handler");
    }
    if (unloadHandler_ == nullptr) {
        VPE_LOGE("UnloadHandler is nullptr!");
        return ERR_NULL_OBJECT;
    }
    return ERR_NONE;
}

int32_t VideoProcessingServer::DestroyUnloadHandler()
{
    std::lock_guard<std::mutex> lock(unloadMutex_);
    if (unloadHandler_ == nullptr) {
        VPE_LOGE("UnloadHandler is nullptr!");
        return ERR_NULL_OBJECT;
    }
    unloadHandler_->RemoveTask(UNLOAD_TASK_ID);
    unloadHandler_ = nullptr;
    return ERR_NONE;
}

ErrCode VideoProcessingServer::LoadInfo(int32_t key, SurfaceBufferInfo& bufferInfo)
{
    if (key < 0 || key >= VPEMODEL_PATHS_LENGTH) {
        VPE_LOGE("Input key %{public}d is invalid!", key);
        UnloadVideoProcessingSA();
        return ERR_INVALID_DATA;
    }
    std::string path = VPE_MODEL_PATHS[key];
    VPE_LOGD("LoadInfoForVpe %{public}s", path.c_str());
    bufferInfo.surfacebuffer = SurfaceBuffer::Create();
    if (bufferInfo.surfacebuffer == nullptr) {
        VPE_LOGE("Create surface buffer failed");
        UnloadVideoProcessingSA();
        return ERR_NULL_OBJECT;
    }
    std::unique_ptr<std::ifstream> fileStream = std::make_unique<std::ifstream>(path, std::ios::binary);
    if (!fileStream->is_open()) {
        VPE_LOGE("file is not open %{public}s", path.c_str());
        UnloadVideoProcessingSA();
        return ERR_NULL_OBJECT;
    }
    fileStream->seekg(0, std::ios::end);
    int fileLength = fileStream->tellg();
    fileStream->seekg(0, std::ios::beg);
    if (fileLength < 0 || fileLength > VPE_INFO_FILE_MAX_LENGTH) {
        VPE_LOGE("fileLength %{public}d is too short or too long!", fileLength);
        UnloadVideoProcessingSA();
        return ERR_INVALID_DATA;
    }

    BufferRequestConfig inputCfg;
    inputCfg.width = fileLength;
    inputCfg.height = 1;
    VPE_LOGD("FileLength: %{public}d", fileLength);
    inputCfg.strideAlignment = fileLength;
    inputCfg.usage = BUFFER_USAGE_CPU_READ | BUFFER_USAGE_CPU_WRITE | BUFFER_USAGE_MEM_DMA;
    inputCfg.format = GRAPHIC_PIXEL_FMT_YCBCR_420_SP;
    inputCfg.timeout = 0;
    GSError err = bufferInfo.surfacebuffer->Alloc(inputCfg);
    if (err != GSERROR_OK) {
        VPE_LOGE("Alloc surface buffer failed");
        UnloadVideoProcessingSA();
        return ERR_INVALID_DATA;
    }
    fileStream->read(reinterpret_cast<char*>(bufferInfo.surfacebuffer->GetVirAddr()), fileLength);
    fileStream->close();
    UnloadVideoProcessingSA();
    return ERR_NONE;
}

void VideoProcessingServer::UnloadVideoProcessingSA()
{
    if (CreateUnloadHandler() == ERR_NONE) {
        VPE_LOGI("CreateUnloadHandler success!");
        DelayUnloadTask();
    } else {
        return;
    }
    VPE_LOGD("Start/Update Delay Time Unload VPE SA!");
    return;
}

void VideoProcessingServer::DelayUnloadTask()
{
    VPE_LOGD("delay unload task begin");
    auto task = []() {
        VPE_LOGD("do unload task");
        auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgrProxy == nullptr) {
            VPE_LOGE("get samgr failed");
            return;
        }
        int32_t unloadResult = samgrProxy->UnloadSystemAbility(VIDEO_PROCESSING_SERVER_SA_ID);
        if (unloadResult != ERR_OK) {
            VPE_LOGE("remove system ability failed");
            return;
        }
        VPE_LOGI("kill VPE service success!");
    };
    unloadHandler_->RemoveTask(UNLOAD_TASK_ID);
    VPE_LOGD("delay unload task post task");
    unloadHandler_->PostTask(task, UNLOAD_TASK_ID, DELAY_TIME);
}

void VideoProcessingServer::OnStart()
{
    VPE_LOGD("VPE SA Onstart!");
    Init();
    Publish(this);
}

void VideoProcessingServer::OnStop()
{
    VPE_LOGD("VPE SA Onstop!");
    DestroyUnloadHandler();
    if (UnInit() != ERR_NONE) {
        VPE_LOGE("Uninit failed");
        return;
    }
}
}
}
}