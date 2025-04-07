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

#include "video_processing_server.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "algorithm_errors.h"
#include "surface_buffer.h"
#include "vpe_model_path.h"
#include "vpe_sa_constants.h"

using namespace OHOS::Media::VideoProcessingEngine;
using namespace OHOS;
using namespace std::placeholders;

using VpeAlgo = IVideoProcessingAlgorithm;

namespace {
const int VPE_INFO_FILE_MAX_LENGTH = 20485760;
const std::string UNLOAD_HANLDER = "unload_vpe_sa_handler";
const std::string UNLOAD_TASK_ID = "unload_vpe_sa";
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
}

ErrCode VideoProcessingServer::LoadInfo(int32_t key, SurfaceBufferInfo& bufferInfo)
{
    if (key < 0 || key >= VPE_MODEL_KEY_NUM) {
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

// For optimized SA
ErrCode VideoProcessingServer::Create(const std::string& feature, const std::string& clientName, int32_t& clientID)
{
    VPE_LOGD(">>> feature:%{public}s client:%{public}s", feature.c_str(), clientName.c_str());
    uint32_t id;
    std::lock_guard<std::mutex> lock(lock_);
    auto ret = CreateLocked(feature, clientName, id);
    if (ret == VPE_ALGO_ERR_OK) {
        clientID = static_cast<int>(id);
        VPE_LOGD("<<< feature:%{public}s client:%{public}s isWorking_:%{public}d {new ID:%{public}u}",
            feature.c_str(), clientName.c_str(), isWorking_.load(), id);
    }
    DelayUnloadTaskLocked();
    return ret;
}

ErrCode VideoProcessingServer::Destroy(int32_t clientID)
{
    std::lock_guard<std::mutex> lock(lock_);
    auto ret = DestroyLocked(static_cast<uint32_t>(clientID));
    DelayUnloadTaskLocked();
    return ret;
}

ErrCode VideoProcessingServer::SetParameter(int32_t clientID, int32_t tag, const std::vector<uint8_t>& parameter)
{
    return Execute(clientID, std::bind(&VpeAlgo::SetParameter, _1, _2, tag, parameter), VPE_LOG_INFO);
}

ErrCode VideoProcessingServer::GetParameter(int32_t clientID, int32_t tag, std::vector<uint8_t>& parameter)
{
    return Execute(clientID, std::bind(&VpeAlgo::GetParameter, _1, _2, tag, parameter), VPE_LOG_INFO);
}

ErrCode VideoProcessingServer::UpdateMetadata(int32_t clientID, SurfaceBufferInfo& image)
{
    CHECK_AND_RETURN_RET_LOG(image.surfacebuffer != nullptr, VPE_ALGO_ERR_INVALID_PARAM,
        "Invalid input: image is null!");
    return Execute(clientID, std::bind(&VpeAlgo::UpdateMetadata, _1, _2, image), VPE_LOG_INFO);
}

ErrCode VideoProcessingServer::Process(int32_t clientID, const SurfaceBufferInfo& input, SurfaceBufferInfo& output)
{
    CHECK_AND_RETURN_RET_LOG(input.surfacebuffer != nullptr && output.surfacebuffer != nullptr,
        VPE_ALGO_ERR_INVALID_PARAM, "Invalid input: input or output is null!");
    return Execute(clientID, std::bind(&VpeAlgo::Process, _1, _2, input, output), VPE_LOG_INFO);
}

ErrCode VideoProcessingServer::ComposeImage(int32_t clientID, const SurfaceBufferInfo& inputSdrImage,
    const SurfaceBufferInfo& inputGainmap, SurfaceBufferInfo& outputHdrImage, bool legacy)
{
    CHECK_AND_RETURN_RET_LOG(inputSdrImage.surfacebuffer != nullptr && inputGainmap.surfacebuffer != nullptr &&
        outputHdrImage.surfacebuffer != nullptr, VPE_ALGO_ERR_INVALID_PARAM, "Invalid input: input or output is null!");
    return Execute(clientID,
        std::bind(&VpeAlgo::ComposeImage, _1, _2, inputSdrImage, inputGainmap, outputHdrImage, legacy), VPE_LOG_INFO);
}

ErrCode VideoProcessingServer::DecomposeImage(int32_t clientID, const SurfaceBufferInfo& inputImage,
    SurfaceBufferInfo& outputSdrImage, SurfaceBufferInfo& outputGainmap)
{
    CHECK_AND_RETURN_RET_LOG(inputImage.surfacebuffer != nullptr && outputSdrImage.surfacebuffer != nullptr &&
        outputGainmap.surfacebuffer != nullptr, VPE_ALGO_ERR_INVALID_PARAM, "Invalid input: input or output is null!");
    return Execute(clientID,
        std::bind(&VpeAlgo::DecomposeImage, _1, _2, inputImage, outputSdrImage, outputGainmap), VPE_LOG_INFO);
}

void VideoProcessingServer::UnloadVideoProcessingSA()
{
    if (CreateUnloadHandler()) {
        VPE_LOGI("CreateUnloadHandler success!");
        DelayUnloadTask();
    } else {
        return;
    }
    VPE_LOGD("Start/Update Delay Time Unload VPE SA!");
    return;
}

void VideoProcessingServer::OnStart(const SystemAbilityOnDemandReason& startReason)
{
    VPE_LOGD("Start VPE SA because %{public}s.", startReason.GetName().c_str());
    if (CreateUnloadHandler()) {
        VPE_LOGI("CreateUnloadHandler success!");
        DelayUnloadTask();
    }
    CHECK_AND_RETURN_LOG(Publish(this), "Failed to publish SA!");
}

void VideoProcessingServer::OnStop(const SystemAbilityOnDemandReason& stopReason)
{
    VPE_LOGD("Stop VPE SA because %{public}s.", stopReason.GetName().c_str());
    DestroyUnloadHandler();
    ClearAlgorithms();
}

ErrCode VideoProcessingServer::CreateLocked(const std::string& feature, const std::string& clientName, uint32_t& id)
{
    AlgoPtr algo = nullptr;
    bool isNew = false;
    auto it = algorithms_.find(feature);
    if (it == algorithms_.end() || it->second == nullptr) {
        algo = factory_.Create(feature);
        CHECK_AND_RETURN_RET_LOG(algo != nullptr, VPE_ALGO_ERR_NO_MEMORY,
            "Failed to create '%{public}s' for '%{public}s'!", feature.c_str(), clientName.c_str());
        CHECK_AND_RETURN_RET_LOG(algo->Initialize() == VPE_ALGO_ERR_OK, ERR_INVALID_STATE,
            "Failed to initialize '%{public}s' for '%{public}s'!", feature.c_str(), clientName.c_str());
        isNew = true;
    } else {
        algo = it->second;
    }
    CHECK_AND_RETURN_RET_LOG(algo->Add(clientName, id) == VPE_ALGO_ERR_OK, ERR_INVALID_DATA,
        "Failed to add client to '%{public}s' for '%{public}s'!", feature.c_str(), clientName.c_str());
    clients_[id] = feature;
    if (isNew) {
        algorithms_[feature] = algo;
    }
    isWorking_ = true;
    return VPE_ALGO_ERR_OK;
}

ErrCode VideoProcessingServer::DestroyLocked(uint32_t id)
{
    auto it = clients_.find(id);
    if (it == clients_.end()) [[unlikely]] {
        VPE_LOGE("Invalid input: no client for ID=%{public}d", id);
        return VPE_ALGO_ERR_INVALID_CLIENT_ID;
    }
    std::string feature = it->second;
    clients_.erase(it);
    isWorking_ = !clients_.empty();
    VPE_LOGD("isWorking_:%{public}d", isWorking_.load());
    auto itAlgo = algorithms_.find(feature);
    if (itAlgo == algorithms_.end()) [[unlikely]] {
        VPE_LOGE("Invalid input: no '%{public}s' for ID=%{public}d", feature.c_str(), id);
        return VPE_ALGO_ERR_INVALID_VAL;
    }
    if (itAlgo->second == nullptr) [[unlikely]] {
        VPE_LOGE("Invalid input: null '%{public}s' for ID=%{public}d", feature.c_str(), id);
        algorithms_.erase(itAlgo);
        return VPE_ALGO_ERR_INVALID_VAL;
    }
    auto algo = itAlgo->second;
    auto ret = algo->Del(id);
    CHECK_AND_LOG(ret == VPE_ALGO_ERR_OK, "Failed to del(ID=%{public}d) of '%{public}s'", id, feature.c_str());
    if (!algo->HasClient()) {
        ret = algo->Deinitialize();
        CHECK_AND_LOG(ret == VPE_ALGO_ERR_OK, "Failed to deinitialize of '%{public}s' for ID=%{public}d",
            feature.c_str(), id);
        algorithms_.erase(itAlgo);
    }
    return ret;
}

bool VideoProcessingServer::CreateUnloadHandler()
{
    std::lock_guard<std::mutex> lock(lock_);
    return CreateUnloadHandlerLocked();
}

bool VideoProcessingServer::CreateUnloadHandlerLocked()
{
    if (unloadHandler_ != nullptr) {
        return true;
    }
    unloadHandler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create(UNLOAD_HANLDER));
    return unloadHandler_ != nullptr;
}

void VideoProcessingServer::DestroyUnloadHandler()
{
    std::lock_guard<std::mutex> lock(lock_);
    if (unloadHandler_ != nullptr) {
        unloadHandler_->RemoveAllEvents();
        unloadHandler_->RemoveTask(UNLOAD_TASK_ID);
        unloadHandler_ = nullptr;
    }
}

void VideoProcessingServer::DelayUnloadTask()
{
    std::lock_guard<std::mutex> lock(lock_);
    DelayUnloadTaskLocked();
}

void VideoProcessingServer::DelayUnloadTaskLocked()
{
    VPE_LOGD("delay unload task begin, isWorking_:%{public}d", isWorking_.load());
    CHECK_AND_RETURN_LOG(CreateUnloadHandlerLocked(), "unloadHandler_ is NOT created!");
    unloadHandler_->RemoveTask(UNLOAD_TASK_ID);
    VPE_LOGD("delay unload task post task(wait %{public}dms)", DELAY_TIME);
    auto task = [this]() {
        VPE_LOGD("do unload task, isWorking_:%{public}d", isWorking_.load());
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        CHECK_AND_RETURN_LOG(samgr != nullptr, "Failed to GetSystemAbilityManager!");
        CHECK_AND_RETURN_LOG(samgr->UnloadSystemAbility(VIDEO_PROCESSING_SERVER_SA_ID) == ERR_OK,
            "Failed to unload VPE SA!");
        VPE_LOGI("kill VPE service success!");
    };
    unloadHandler_->PostTask(task, UNLOAD_TASK_ID, DELAY_TIME);
}

void VideoProcessingServer::ClearAlgorithms()
{
    std::lock_guard<std::mutex> lock(lock_);
    for (auto& [feature, algo] : algorithms_) {
        if (algo == nullptr) [[unlikely]] {
            VPE_LOGE("algorithm is null of '%{public}s'!", feature.c_str());
            continue;
        }
        if (algo->Deinitialize() != VPE_ALGO_ERR_OK) [[unlikely]] {
            VPE_LOGE("Failed to deinitialize of '%{public}s'!", feature.c_str());
            continue;
        }
    }
    algorithms_.clear();
    clients_.clear();
    isWorking_ = false;
    VPE_LOGD("isWorking_:%{public}d", isWorking_.load());
}

ErrCode VideoProcessingServer::Execute(int clientID, std::function<int(AlgoPtr&, uint32_t)>&& operation,
    const LogInfo& logInfo)
{
    uint32_t id = static_cast<uint32_t>(clientID);
    AlgoPtr algorithm;
    {
        std::lock_guard<std::mutex> lock(lock_);
        auto it = clients_.find(id);
        if (it == clients_.end()) [[unlikely]] {
            VPE_ORG_LOGE(logInfo, "Invalid input: no client for ID=%{public}d!", id);
            DelayUnloadTaskLocked();
            return VPE_ALGO_ERR_INVALID_CLIENT_ID;
        }
        auto itAlgo = algorithms_.find(it->second);
        if (itAlgo == algorithms_.end() || itAlgo->second == nullptr) [[unlikely]] {
            VPE_ORG_LOGE(logInfo, "Invalid input: no '%{public}s' for ID=%{public}d!", it->second.c_str(), id);
            DelayUnloadTaskLocked();
            return VPE_ALGO_ERR_INVALID_VAL;
        }
        algorithm = itAlgo->second;
    }
    auto err = operation(algorithm, id);
    DelayUnloadTask();
    return err;
}
