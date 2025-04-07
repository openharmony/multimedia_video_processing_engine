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

#ifndef VPE_VIDEO_PROCESSING_SERVER_H
#define VPE_VIDEO_PROCESSING_SERVER_H

#include <atomic>
#include <cinttypes>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include <refbase.h>
#include <system_ability.h>

#include "event_handler.h"
#include "event_runner.h"
#include "ipc_types.h"

#include "video_processing_algorithm_factory.h"
#include "video_processing_service_manager_stub.h"
#include "vpe_log.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class VideoProcessingServer : public SystemAbility, public VideoProcessingServiceManagerStub {
    DECLARE_SYSTEM_ABILITY(VideoProcessingServer);

public:
    VideoProcessingServer(int32_t saId, bool runOnCreate);
    ~VideoProcessingServer();
    VideoProcessingServer(const VideoProcessingServer&) = delete;
    VideoProcessingServer& operator=(const VideoProcessingServer&) = delete;
    VideoProcessingServer(VideoProcessingServer&&) = delete;
    VideoProcessingServer& operator=(VideoProcessingServer&&) = delete;

    ErrCode LoadInfo(int32_t key, SurfaceBufferInfo& bufferInfo) override;

    // For optimized SA
    ErrCode Create(const std::string& feature, const std::string& clientName, int32_t& clientID) final;
    ErrCode Destroy(int32_t clientID) final;
    ErrCode SetParameter(int32_t clientID, int32_t tag, const std::vector<uint8_t>& parameter) final;
    ErrCode GetParameter(int32_t clientID, int32_t tag, std::vector<uint8_t>& parameter) final;
    ErrCode UpdateMetadata(int32_t clientID, SurfaceBufferInfo& image) final;
    ErrCode Process(int32_t clientID, const SurfaceBufferInfo& input, SurfaceBufferInfo& output) final;
    ErrCode ComposeImage(int32_t clientID, const SurfaceBufferInfo& inputSdrImage,
        const SurfaceBufferInfo& inputGainmap, SurfaceBufferInfo& outputHdrImage, bool legacy) final;
    ErrCode DecomposeImage(int32_t clientID, const SurfaceBufferInfo& inputImage, SurfaceBufferInfo& outputSdrImage,
        SurfaceBufferInfo& outputGainmap) final;

private:
    using AlgoPtr = std::shared_ptr<IVideoProcessingAlgorithm>;

    void OnStart(const SystemAbilityOnDemandReason& startReason) final;
    void OnStop(const SystemAbilityOnDemandReason& stopReason) final;

    void UnloadVideoProcessingSA();

    // For optimized SA
    ErrCode CreateLocked(const std::string& feature, const std::string& clientName, uint32_t& id);
    ErrCode DestroyLocked(uint32_t id);
    bool CreateUnloadHandler();
    bool CreateUnloadHandlerLocked();
    void DestroyUnloadHandler();
    void DelayUnloadTask();
    void DelayUnloadTaskLocked();
    void ClearAlgorithms();
    ErrCode Execute(int clientID, std::function<int(AlgoPtr&, uint32_t)>&& operation, const LogInfo& logInfo);

    VideoProcessingAlgorithmFactory factory_{};
    mutable std::mutex lock_{};
    // Guarded by lock_ begin
    std::atomic<bool> isWorking_{false};
    std::shared_ptr<AppExecFwk::EventHandler> unloadHandler_{};
    std::unordered_map<uint32_t, std::string> clients_{};
    std::unordered_map<std::string, AlgoPtr> algorithms_{};
    // Guarded by lock_ end
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // VPE_VIDEO_PROCESSING_SERVER_H
