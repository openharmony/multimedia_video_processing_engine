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

#ifndef VIDEO_PROCESSING_ALGORITHM_WITH_DATA_H
#define VIDEO_PROCESSING_ALGORITHM_WITH_DATA_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "algorithm_errors.h"
#include "video_processing_algorithm_base.h"
#include "vpe_log.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
template <typename T>
class VideoProcessingAlgorithmWithData : public VideoProcessingAlgorithmBase {
public:
    int SetParameter(uint32_t clientID, int tag, const std::vector<uint8_t>& parameter) final
    {
        auto set = &VideoProcessingAlgorithmWithData::OnSetParameter;
        return Execute(clientID, std::bind(set, this, std::placeholders::_1, tag, parameter), VPE_LOG_INFO);
    }

    int GetParameter(uint32_t clientID, int tag, std::vector<uint8_t>& parameter) final
    {
        auto get = &VideoProcessingAlgorithmWithData::OnGetParameter;
        return Execute(clientID, std::bind(get, this, std::placeholders::_1, tag, parameter), VPE_LOG_INFO);
    }

    int DoUpdateMetadata(uint32_t clientID, SurfaceBufferInfo& image) final
    {
        auto update = &VideoProcessingAlgorithmWithData::OnUpdateMetadata;
        return Execute(clientID, std::bind(update, this, std::placeholders::_1, image), VPE_LOG_INFO);
    }

    int DoProcess(uint32_t clientID, const SurfaceBufferInfo& input, SurfaceBufferInfo& output) final
    {
        auto process = &VideoProcessingAlgorithmWithData::OnProcess;
        return Execute(clientID, std::bind(process, this, std::placeholders::_1, input, output), VPE_LOG_INFO);
    }

    int DoComposeImage(uint32_t clientID,
        const SurfaceBufferInfo& inputSdrImage,
        const SurfaceBufferInfo& inputGainmap,
        SurfaceBufferInfo& outputHdrImage, bool legacy) final
    {
        auto compose = &VideoProcessingAlgorithmWithData::OnComposeImage;
        return Execute(clientID,
            std::bind(compose, this, std::placeholders::_1, inputSdrImage, inputGainmap, outputHdrImage, legacy),
            VPE_LOG_INFO);
    }

    int DoDecomposeImage(uint32_t clientID,
        const SurfaceBufferInfo& inputImage,
        SurfaceBufferInfo& outputSdrImage,
        SurfaceBufferInfo& outputGainmap) final
    {
        auto decompose = &VideoProcessingAlgorithmWithData::OnDecomposeImage;
        return Execute(clientID,
            std::bind(decompose, this, std::placeholders::_1, inputImage, outputSdrImage, outputGainmap), VPE_LOG_INFO);
    }

protected:
    using ClientInfo = std::pair<std::string, std::shared_ptr<T>>;

    VideoProcessingAlgorithmWithData(const std::string& feature, uint32_t id)
        : VideoProcessingAlgorithmBase(feature, id) {}
    virtual ~VideoProcessingAlgorithmWithData() = default;
    VideoProcessingAlgorithmWithData(const VideoProcessingAlgorithmWithData&) = delete;
    VideoProcessingAlgorithmWithData& operator=(const VideoProcessingAlgorithmWithData&) = delete;
    VideoProcessingAlgorithmWithData(VideoProcessingAlgorithmWithData&&) = delete;
    VideoProcessingAlgorithmWithData& operator=(VideoProcessingAlgorithmWithData&&) = delete;

    // NOTE: Would be override by the actual algorithm when necessary.
    //      The input ClientInfo& clientInfo is already checked for all OnXXX functions.
    //      It's NOT necessary to check clientInfo.second whether is nullptr.
    // Actual methods begin
    // int OnInitializeLocked()     - declared by VideoProcessingAlgorithmBase
    // int OnDeinitializeLocked()   - declared by VideoProcessingAlgorithmBase
    virtual int OnContextInitializeLocked([[maybe_unused]] ClientInfo& clientInfo)
    {
        return VPE_ALGO_ERR_OK;
    }

    virtual int OnContextDeinitializeLocked([[maybe_unused]] ClientInfo& clientInfo)
    {
        return VPE_ALGO_ERR_OK;
    }

    virtual int OnSetParameter([[maybe_unused]] ClientInfo& clientInfo,
        [[maybe_unused]] int tag, [[maybe_unused]] const std::vector<uint8_t>& parameter)
    {
        return ReturnNotSupport(VPE_LOG_INFO);
    }

    virtual int OnGetParameter([[maybe_unused]] ClientInfo& clientInfo,
        [[maybe_unused]] int tag, [[maybe_unused]] std::vector<uint8_t>& parameter)
    {
        return ReturnNotSupport(VPE_LOG_INFO);
    }

    virtual int OnUpdateMetadata([[maybe_unused]] ClientInfo& clientInfo,
        [[maybe_unused]] SurfaceBufferInfo& image)
    {
        return ReturnNotSupport(VPE_LOG_INFO);
    }

    virtual int OnProcess([[maybe_unused]] ClientInfo& clientInfo,
        [[maybe_unused]] const SurfaceBufferInfo& input,
        [[maybe_unused]] SurfaceBufferInfo& output)
    {
        return ReturnNotSupport(VPE_LOG_INFO);
    }

    virtual int OnComposeImage([[maybe_unused]] ClientInfo& clientInfo,
        [[maybe_unused]] const SurfaceBufferInfo& inputSdrImage,
        [[maybe_unused]] const SurfaceBufferInfo& inputGainmap,
        [[maybe_unused]] SurfaceBufferInfo& outputHdrImage, [[maybe_unused]] bool legacy)
    {
        return ReturnNotSupport(VPE_LOG_INFO);
    }

    virtual int OnDecomposeImage([[maybe_unused]] ClientInfo& clientInfo,
        [[maybe_unused]] const SurfaceBufferInfo& inputImage,
        [[maybe_unused]] SurfaceBufferInfo& outputSdrImage,
        [[maybe_unused]] SurfaceBufferInfo& outputGainmap)
    {
        return ReturnNotSupport(VPE_LOG_INFO);
    }
    // Actual methods end

    // Utils methods would be called by the derived algorithm class
    ClientInfo GetContext(uint32_t clientID)
    {
        std::lock_guard<std::mutex>lock(lock_);
        return GetContextLocked(clientID);
    }

    ClientInfo GetContextLocked(uint32_t clientID)
    {
        auto it = contexts_.find(clientID);
        if (it == contexts_.end()) [[unlikely]] {
            return std::make_pair("", nullptr);
        }
        return it->second;
    }

private:
    int AddClientIDLocked(const std::string& clientName, uint32_t& clientID) final
    {
        uint32_t id;
        if (!GenerateClientID([this](uint32_t id) { return contexts_.find(id) != contexts_.end(); }, id)) {
            return VPE_ALGO_ERR_INVALID_STATE;
        }
        auto context = std::make_shared<T>();
        CHECK_AND_RETURN_RET_LOG(context != nullptr, VPE_ALGO_ERR_NO_MEMORY, "Failed to create context for %{public}s!",
            clientName.c_str());
        contexts_[id] = std::make_pair(clientName, context);
        clientID = id;
        return OnContextInitializeLocked(contexts_[id]);
    }

    int DelClientIDLocked(uint32_t clientID, bool& isEmpty) final
    {
        auto it = contexts_.find(clientID);
        CHECK_AND_RETURN_RET_LOG(it != contexts_.end(), VPE_ALGO_ERR_INVALID_PARAM,
            "Invalid input: clientID:%{public}u!", clientID);
        int ret = OnContextDeinitializeLocked(it->second);
        contexts_.erase(it);
        isEmpty = contexts_.empty();
        return ret;
    }

    size_t GetClientSizeLocked() const final
    {
        return contexts_.size();
    }

    void ClearClientsLocked() final
    {
        for (auto& [id, info] : contexts_) {
            CHECK_AND_LOG(OnContextDeinitializeLocked(info) == VPE_ALGO_ERR_OK,
                "Failed to deinitialize(id:%{public}d, client:%{public}s)", id, info.first.c_str());
        }
        contexts_.clear();
    }

    int Execute(uint32_t clientID, std::function<int(ClientInfo&)>&& operation,
        const LogInfo& logInfo)
    {
        ClientInfo ctxt;
        {
            std::lock_guard<std::mutex>lock(lock_);
            if (!isInitialized_) [[unlikely]] {
                VPE_ORG_LOGE(logInfo, "Oops! Not initialized!");
                return VPE_ALGO_ERR_INVALID_STATE;
            }
            ctxt = GetContextLocked(clientID);
            if (ctxt.first.empty() || ctxt.second == nullptr) [[unlikely]] {
                VPE_ORG_LOGE(logInfo, "Invalid input: client(%{public}u) for '%{public}s'!",
                    clientID, ctxt.first.c_str());
                return VPE_ALGO_ERR_INVALID_PARAM;
            }
        }
        return operation(ctxt);
    }

    // Guarded by lock_ begin
    std::unordered_map<uint32_t, ClientInfo> contexts_{};
    // Guarded by lock_ end
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // VIDEO_PROCESSING_ALGORITHM_WITH_DATA_H
