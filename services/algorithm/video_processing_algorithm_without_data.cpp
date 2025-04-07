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

#include "video_processing_algorithm_without_data.h"

#include "algorithm_errors.h"
#include "vpe_log.h"

using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;
using namespace std::placeholders;

int VideoProcessingAlgorithmWithoutData::SetParameter(uint32_t clientID, int tag, const std::vector<uint8_t>& parameter)
{
    return Execute(clientID, std::bind(&VideoProcessingAlgorithmWithoutData::OnSetParameter, this, _1, tag, parameter),
        VPE_LOG_INFO);
}

int VideoProcessingAlgorithmWithoutData::GetParameter(uint32_t clientID, int tag, std::vector<uint8_t>& parameter)
{
    return Execute(clientID, std::bind(&VideoProcessingAlgorithmWithoutData::OnGetParameter, this, _1, tag, parameter),
        VPE_LOG_INFO);
}

int VideoProcessingAlgorithmWithoutData::DoUpdateMetadata(uint32_t clientID, SurfaceBufferInfo& image)
{
    return Execute(clientID, std::bind(&VideoProcessingAlgorithmWithoutData::OnUpdateMetadata, this, _1, image),
        VPE_LOG_INFO);
}

int VideoProcessingAlgorithmWithoutData::DoProcess(uint32_t clientID,
    const SurfaceBufferInfo& input, SurfaceBufferInfo& output)
{
    return Execute(clientID, std::bind(&VideoProcessingAlgorithmWithoutData::OnProcess, this, _1, input, output),
        VPE_LOG_INFO);
}

int VideoProcessingAlgorithmWithoutData::DoComposeImage(uint32_t clientID,
    const SurfaceBufferInfo& inputSdrImage,
    const SurfaceBufferInfo& inputGainmap,
    SurfaceBufferInfo& outputHdrImage, bool legacy)
{
    auto compose = &VideoProcessingAlgorithmWithoutData::OnComposeImage;
    return Execute(clientID, std::bind(compose, this, _1, inputSdrImage, inputGainmap, outputHdrImage, legacy),
        VPE_LOG_INFO);
}

int VideoProcessingAlgorithmWithoutData::DoDecomposeImage(uint32_t clientID,
    const SurfaceBufferInfo& inputImage,
    SurfaceBufferInfo& outputSdrImage,
    SurfaceBufferInfo& outputGainmap)
{
    auto decompose = &VideoProcessingAlgorithmWithoutData::OnDecomposeImage;
    return Execute(clientID, std::bind(decompose, this, _1, inputImage, outputSdrImage, outputGainmap),
        VPE_LOG_INFO);
}

int VideoProcessingAlgorithmWithoutData::OnSetParameter([[maybe_unused]] const std::string& clientName,
    [[maybe_unused]] int tag, [[maybe_unused]] const std::vector<uint8_t>& parameter)
{
    return ReturnNotSupport(VPE_LOG_INFO);
}

int VideoProcessingAlgorithmWithoutData::OnGetParameter([[maybe_unused]] const std::string& clientName,
    [[maybe_unused]] int tag, [[maybe_unused]] std::vector<uint8_t>& parameter)
{
    return ReturnNotSupport(VPE_LOG_INFO);
}

int VideoProcessingAlgorithmWithoutData::OnUpdateMetadata([[maybe_unused]] const std::string& clientName,
    [[maybe_unused]] SurfaceBufferInfo& image)
{
    return ReturnNotSupport(VPE_LOG_INFO);
}

int VideoProcessingAlgorithmWithoutData::OnProcess([[maybe_unused]] const std::string& clientName,
    [[maybe_unused]] const SurfaceBufferInfo& input,
    [[maybe_unused]] SurfaceBufferInfo& output)
{
    return ReturnNotSupport(VPE_LOG_INFO);
}

int VideoProcessingAlgorithmWithoutData::OnComposeImage([[maybe_unused]] const std::string& clientName,
    [[maybe_unused]] const SurfaceBufferInfo& inputSdrImage,
    [[maybe_unused]] const SurfaceBufferInfo& inputGainmap,
    [[maybe_unused]] SurfaceBufferInfo& outputHdrImage, [[maybe_unused]] bool legacy)
{
    return ReturnNotSupport(VPE_LOG_INFO);
}

int VideoProcessingAlgorithmWithoutData::OnDecomposeImage([[maybe_unused]] const std::string& clientName,
    [[maybe_unused]] const SurfaceBufferInfo& inputImage,
    [[maybe_unused]] SurfaceBufferInfo& outputSdrImage,
    [[maybe_unused]] SurfaceBufferInfo& outputGainmap)
{
    return ReturnNotSupport(VPE_LOG_INFO);
}

int VideoProcessingAlgorithmWithoutData::AddClientIDLocked(const std::string& clientName, uint32_t& clientID)
{
    uint32_t id;
    if (!GenerateClientID([this](uint32_t id) { return clients_.find(id) != clients_.end(); }, id)) {
        return VPE_ALGO_ERR_INVALID_STATE;
    }
    clients_[id] = clientName;
    clientID = id;
    return VPE_ALGO_ERR_OK;
}

int VideoProcessingAlgorithmWithoutData::DelClientIDLocked(uint32_t clientID, bool& isEmpty)
{
    auto it = clients_.find(clientID);
    CHECK_AND_RETURN_RET_LOG(it != clients_.end(), VPE_ALGO_ERR_INVALID_PARAM,
        "Invalid input: clientID:%{public}u!", clientID);
    clients_.erase(it);
    isEmpty = clients_.empty();
    return VPE_ALGO_ERR_OK;
}

void VideoProcessingAlgorithmWithoutData::ClearClientsLocked()
{
    clients_.clear();
}

int VideoProcessingAlgorithmWithoutData::Execute(uint32_t clientID, std::function<int(const std::string&)>&& operation,
    const LogInfo& logInfo)
{
    std::string clientName;
    {
        std::lock_guard<std::mutex> lock(lock_);
        if (!isInitialized_) [[unlikely]] {
            VPE_ORG_LOGE(logInfo, "Oops! Not initialized!");
            return VPE_ALGO_ERR_INVALID_STATE;
        }
        auto it = clients_.find(clientID);
        if (it == clients_.end()) [[unlikely]] {
            VPE_ORG_LOGE(logInfo, "Invalid input: client(%{public}u)!", clientID);
            return VPE_ALGO_ERR_INVALID_PARAM;
        }
        clientName = it->second;
        if (clientName.empty()) [[unlikely]] {
            VPE_ORG_LOGE(logInfo, "Invalid input: client(%{public}u) for ''!", clientID);
            return VPE_ALGO_ERR_INVALID_PARAM;
        }
    }
    return operation(clientName);
}
