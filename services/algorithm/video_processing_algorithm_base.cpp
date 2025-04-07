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

#include "video_processing_algorithm_base.h"

#include "algorithm_errors.h"
#include "video_processing_algorithm_factory.h"
#include "vpe_sa_types.h"

using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;

namespace {
constexpr uint32_t MAX_CLIENT_GEN_COUNT = 500;
}

int VideoProcessingAlgorithmBase::Initialize()
{
    std::lock_guard<std::mutex> lock(lock_);
    if (isInitialized_) {
        VPE_LOGW("Already initialized!");
        return VPE_ALGO_ERR_OK;
    }
    int ret = OnInitializeLocked();
    if (ret != VPE_ALGO_ERR_OK) {
        VPE_LOGE("Failed to initialize!");
        return ret;
    }
    isInitialized_ = true;
    return VPE_ALGO_ERR_OK;
}

int VideoProcessingAlgorithmBase::Deinitialize()
{
    std::lock_guard<std::mutex> lock(lock_);
    if (!isInitialized_) {
        VPE_LOGW("Already deinitialized!");
        return VPE_ALGO_ERR_OK;
    }
    ClearClientsLocked();
    int ret = OnDeinitializeLocked();
    if (ret != VPE_ALGO_ERR_OK) {
        VPE_LOGE("Failed to deinitialize!");
        return ret;
    }
    isInitialized_ = false;
    return VPE_ALGO_ERR_OK;
}

bool VideoProcessingAlgorithmBase::HasClient() const
{
    return hasClient_.load();
}

int VideoProcessingAlgorithmBase::Add(const std::string& clientName, uint32_t& clientID)
{
    CHECK_AND_RETURN_RET_LOG(!clientName.empty(), VPE_ALGO_ERR_INVALID_PARAM, "Invalid input: clientName is empty!");

    std::lock_guard<std::mutex> lock(lock_);
    int ret = AddClientIDLocked(clientName, clientID);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, ret, "Failed to add client for %{public}s!", clientName.c_str());

    hasClient_ = true;
    return VPE_ALGO_ERR_OK;
}

int VideoProcessingAlgorithmBase::Del(uint32_t clientID)
{
    std::lock_guard<std::mutex> lock(lock_);
    bool isEmpty;
    int ret = DelClientIDLocked(clientID, isEmpty);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, ret, "Failed to del client(%{public}u)!", clientID);
    if (isEmpty) {
        hasClient_ = false;
    }
    return VPE_ALGO_ERR_OK;
}

int VideoProcessingAlgorithmBase::UpdateMetadata(uint32_t clientID, SurfaceBufferInfo& image)
{
    CHECK_AND_RETURN_RET_LOG(IsValid(image), VPE_ALGO_ERR_INVALID_PARAM, "Invalid input: %{public}s!",
        image.str().c_str());
    return DoUpdateMetadata(clientID, image);
}

int VideoProcessingAlgorithmBase::Process(uint32_t clientID,
    const SurfaceBufferInfo& input, SurfaceBufferInfo& output)
{
    CHECK_AND_RETURN_RET_LOG(IsValid(input) && IsValid(output), VPE_ALGO_ERR_INVALID_PARAM,
        "Invalid input: input={%{public}s} output={%{public}s}!", input.str().c_str(), output.str().c_str());
    return DoProcess(clientID, input, output);
}

int VideoProcessingAlgorithmBase::ComposeImage(uint32_t clientID,
    const SurfaceBufferInfo& inputSdrImage,
    const SurfaceBufferInfo& inputGainmap,
    SurfaceBufferInfo& outputHdrImage, bool legacy)
{
    CHECK_AND_RETURN_RET_LOG(IsValid(inputSdrImage) && IsValid(inputGainmap) && IsValid(outputHdrImage),
        VPE_ALGO_ERR_INVALID_PARAM,
        "Invalid input: inputSdrImage={%{public}s} inputGainmap={%{public}s} outputHdrImage={%{public}s}!",
        inputSdrImage.str().c_str(), inputGainmap.str().c_str(), outputHdrImage.str().c_str());
    return DoComposeImage(clientID, inputSdrImage, inputGainmap, outputHdrImage, legacy);
}

int VideoProcessingAlgorithmBase::DecomposeImage(uint32_t clientID,
    const SurfaceBufferInfo& inputImage,
    SurfaceBufferInfo& outputSdrImage,
    SurfaceBufferInfo& outputGainmap)
{
    CHECK_AND_RETURN_RET_LOG(IsValid(inputImage) && IsValid(outputSdrImage) && IsValid(outputGainmap),
        VPE_ALGO_ERR_INVALID_PARAM,
        "Invalid input: inputImage={%{public}s} outputSdrImage={%{public}s} outputGainmap={%{public}s}!",
        inputImage.str().c_str(), outputSdrImage.str().c_str(), outputGainmap.str().c_str());
    return DoDecomposeImage(clientID, inputImage, outputSdrImage, outputGainmap);
}

int VideoProcessingAlgorithmBase::OnInitializeLocked()
{
    return VPE_ALGO_ERR_OK;
}

int VideoProcessingAlgorithmBase::OnDeinitializeLocked()
{
    return VPE_ALGO_ERR_OK;
}

int VideoProcessingAlgorithmBase::AddClientIDLocked([[maybe_unused]] const std::string& clientName,
    [[maybe_unused]] uint32_t& clientID)
{
    return VPE_ALGO_ERR_OK;
}

int VideoProcessingAlgorithmBase::DelClientIDLocked([[maybe_unused]] uint32_t clientID, [[maybe_unused]] bool& isEmpty)
{
    return VPE_ALGO_ERR_OK;
}

size_t VideoProcessingAlgorithmBase::GetClientSizeLocked() const
{
    return 0;
}

void VideoProcessingAlgorithmBase::ClearClientsLocked()
{
}

int VideoProcessingAlgorithmBase::DoUpdateMetadata([[maybe_unused]] uint32_t clientID,
    [[maybe_unused]] SurfaceBufferInfo& image)
{
    return VPE_ALGO_ERR_OK;
}

int VideoProcessingAlgorithmBase::DoProcess([[maybe_unused]] uint32_t clientID,
    [[maybe_unused]] const SurfaceBufferInfo& input, [[maybe_unused]] SurfaceBufferInfo& output)
{
    return VPE_ALGO_ERR_OK;
}

int VideoProcessingAlgorithmBase::DoComposeImage([[maybe_unused]] uint32_t clientID,
    [[maybe_unused]] const SurfaceBufferInfo& inputSdrImage,
    [[maybe_unused]] const SurfaceBufferInfo& inputGainmap,
    [[maybe_unused]] SurfaceBufferInfo& outputHdrImage, [[maybe_unused]] bool legacy)
{
    return VPE_ALGO_ERR_OK;
}

int VideoProcessingAlgorithmBase::DoDecomposeImage([[maybe_unused]] uint32_t clientID,
    [[maybe_unused]] const SurfaceBufferInfo& inputImage,
    [[maybe_unused]] SurfaceBufferInfo& outputSdrImage,
    [[maybe_unused]] SurfaceBufferInfo& outputGainmap)
{
    return VPE_ALGO_ERR_OK;
}

int VideoProcessingAlgorithmBase::ReturnNotSupport(const LogInfo& logInfo) const
{
    VPE_ORG_LOGE(logInfo, "Not support this operation!");
    return VPE_ALGO_ERR_OPERATION_NOT_SUPPORTED;
}

bool VideoProcessingAlgorithmBase::GenerateClientID(std::function<bool(uint32_t)>&& isDuplicate, uint32_t& clientID)
{
    uint32_t id;
    uint32_t i = 0;
    do {
        // MAX_CLIENT_GEN_COUNT will control the ID generation times,
        // it will avoid an infinite loop during ID generation.
        if (i++ > MAX_CLIENT_GEN_COUNT) {
            VPE_LOGE("Failed to generate new client ID, maybe too many clients(size:%{public}zu).",
                GetClientSizeLocked());
            return false;
        }
        id = GenerateClientID();
    } while (isDuplicate(id));
    clientID = id;
    return true;
}

uint32_t VideoProcessingAlgorithmBase::GenerateClientID()
{
    // clientIdBase_ CAN overflow after self-adding, because the ID value maybe reused.
    // And MAX_CLIENT_GEN_COUNT will control the ID generation times,
    // it will avoid an infinite loop during ID generation.
    // +---------------------------------------+
    // |  High  <---------------------->  Low  |
    // +---------+-----------------------------+
    // |  1 Byte |           3 Bytes           |
    // +---------+---------+---------+---------+
    // |FeatureID|          ClientID           |
    // +---------+---------+---------+---------+
    return (featureID_ << 24) | (clientIdBase_++ & 0xFFFFFF);
}
