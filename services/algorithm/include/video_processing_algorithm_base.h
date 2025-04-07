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

#ifndef VIDEO_PROCESSING_ALGORITHM_BASE_H
#define VIDEO_PROCESSING_ALGORITHM_BASE_H

#include <atomic>
#include <functional>
#include <mutex>
#include <string>

#include "algorithm_errors.h"
#include "ivideo_processing_algorithm.h"
#include "vpe_log.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class VideoProcessingAlgorithmBase : public IVideoProcessingAlgorithm {
public:
    int Initialize() final;
    int Deinitialize() final;
    bool HasClient() const final;
    int Add(const std::string& clientName, uint32_t& clientID) final;
    int Del(uint32_t clientID) final;
    int UpdateMetadata(uint32_t clientID, SurfaceBufferInfo& image) final;
    int Process(uint32_t clientID, const SurfaceBufferInfo& input, SurfaceBufferInfo& output) final;
    int ComposeImage(uint32_t clientID, const SurfaceBufferInfo& inputSdrImage,
        const SurfaceBufferInfo& inputGainmap, SurfaceBufferInfo& outputHdrImage, bool legacy) final;
    int DecomposeImage(uint32_t clientID, const SurfaceBufferInfo& inputImage,
        SurfaceBufferInfo& outputSdrImage, SurfaceBufferInfo& outputGainmap) final;

protected:
    VideoProcessingAlgorithmBase(const std::string& feature, uint32_t id) : feature_(feature), featureID_(id) {}
    virtual ~VideoProcessingAlgorithmBase() = default;
    VideoProcessingAlgorithmBase(const VideoProcessingAlgorithmBase&) = delete;
    VideoProcessingAlgorithmBase& operator=(const VideoProcessingAlgorithmBase&) = delete;
    VideoProcessingAlgorithmBase(VideoProcessingAlgorithmBase&&) = delete;
    VideoProcessingAlgorithmBase& operator=(VideoProcessingAlgorithmBase&&) = delete;

    // Note: called by algorithm derived classes
    virtual inline bool IsValid(const SurfaceBufferInfo& info) const // Derived classes can modify the checker if needed
    {
        return info.surfacebuffer != nullptr;
    }

    template <typename T>
    static inline int CallSetParameter(std::function<int(const T&)>&& setter, const std::vector<uint8_t>& parameter,
        const LogInfo& logInfo)
    {
        if (parameter.size() != sizeof(T)) [[unlikely]] {
            VPE_ORG_LOGE(logInfo, "Invalid input: size=%{public}zu(Expected:%{public}zu)!",
                parameter.size(), sizeof(T));
            return VPE_ALGO_ERR_INVALID_VAL;
        }
        return setter(*reinterpret_cast<const T*>(parameter.data()));
    }

    template <typename T>
    static inline int CallGetParameter(std::function<int(T&)>&& getter, std::vector<uint8_t>& parameter)
    {
        parameter.resize(sizeof(T));
        return getter(*reinterpret_cast<T*>(parameter.data()));
    }

    // Note: implement by algorithm derived classes
    virtual int OnInitializeLocked();
    virtual int OnDeinitializeLocked();

    virtual int AddClientIDLocked(const std::string& clientName, uint32_t& clientID);
    virtual int DelClientIDLocked(uint32_t clientID, bool& isEmpty);
    virtual size_t GetClientSizeLocked() const;
    virtual void ClearClientsLocked();

    virtual int DoUpdateMetadata(uint32_t clientID, SurfaceBufferInfo& image);
    virtual int DoProcess(uint32_t clientID, const SurfaceBufferInfo& input, SurfaceBufferInfo& output);
    virtual int DoComposeImage(uint32_t clientID, const SurfaceBufferInfo& inputSdrImage,
        const SurfaceBufferInfo& inputGainmap, SurfaceBufferInfo& outputHdrImage, bool legacy);
    virtual int DoDecomposeImage(uint32_t clientID, const SurfaceBufferInfo& inputImage,
        SurfaceBufferInfo& outputSdrImage, SurfaceBufferInfo& outputGainmap);

    int ReturnNotSupport(const LogInfo& logInfo) const;
    bool GenerateClientID(std::function<bool(uint32_t)>&& isDuplicate, uint32_t& clientID);

    // Note: Data members to be accessed by algorithm derived classes
    const std::string feature_{};
    uint32_t featureID_{};
    mutable std::mutex lock_{};
    // Guarded by lock_ begin
    bool isInitialized_{};
    // Guarded by lock_ end

private:
    uint32_t GenerateClientID();

    // Guarded by lock_ begin
    std::atomic<bool> hasClient_{};
    uint32_t clientIdBase_{};
    // Guarded by lock_ end
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // VIDEO_PROCESSING_ALGORITHM_BASE_H
