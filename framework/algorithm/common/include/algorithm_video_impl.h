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

#ifndef ALGORITHM_VIDEO_IMPL_H
#define ALGORITHM_VIDEO_IMPL_H

#include <atomic>
#include <cinttypes>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>

#include "refbase.h"
#include "surface.h"

#include "algorithm_errors.h"
#include "algorithm_video.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class VpeVideoImpl : public VpeVideo, public std::enable_shared_from_this<VpeVideoImpl> {
public:
    VPEAlgoErrCode RegisterCallback(const std::shared_ptr<VpeVideoCallback>& callback) override;
    VPEAlgoErrCode SetOutputSurface(const sptr<Surface>& surface) override;
    sptr<Surface> GetInputSurface() override;
    VPEAlgoErrCode Start() override;
    VPEAlgoErrCode Stop() override;
    VPEAlgoErrCode Release() override;
    VPEAlgoErrCode Flush() override;
    VPEAlgoErrCode Enable() override;
    VPEAlgoErrCode Disable() override;
    VPEAlgoErrCode NotifyEos() override;
    VPEAlgoErrCode ReleaseOutputBuffer(uint32_t index, bool render) override;
    VPEAlgoErrCode RenderOutputBufferAtTime(uint32_t index, int64_t renderTimestamp) override;

protected:
    explicit VpeVideoImpl(uint32_t type) : type_(type) {}
    virtual ~VpeVideoImpl();
    VpeVideoImpl(const VpeVideoImpl&) = delete;
    VpeVideoImpl& operator=(const VpeVideoImpl&) = delete;
    VpeVideoImpl(VpeVideoImpl&&) = delete;
    VpeVideoImpl& operator=(VpeVideoImpl&&) = delete;

    bool IsInitialized() const;
    VPEAlgoErrCode Initialize();
    VPEAlgoErrCode Deinitialize();
    void RefreshBuffers();
    void OnOutputFormatChanged(const Format& format);

    virtual VPEAlgoErrCode OnInitialize();
    virtual VPEAlgoErrCode OnDeinitialize();
    virtual VPEAlgoErrCode Process(const sptr<SurfaceBuffer>& sourceImage, sptr<SurfaceBuffer>& destinationImage);
    virtual bool IsProducerSurfaceValid(const sptr<Surface>& surface);
    virtual VPEAlgoErrCode UpdateRequestCfg(const sptr<Surface>& surface, BufferRequestConfig& requestCfg);
    virtual void UpdateRequestCfg(const sptr<SurfaceBuffer>& consumerBuffer, BufferRequestConfig& requestCfg);

private:
    enum class VPEState : int {
        IDLE = 0,
        RUNNING,
        STOPPING
    };

    struct SurfaceBufferInfo {
        sptr<SurfaceBuffer> buffer{};
        VpeBufferFlag bufferFlag{VPE_BUFFER_FLAG_NONE};
        int64_t timestamp{};
    };

    class ConsumerListener : public IBufferConsumerListener {
    public:
        explicit ConsumerListener(const std::shared_ptr<VpeVideoImpl>& owner) : owner_(owner) {}
        virtual ~ConsumerListener() = default;
        ConsumerListener(const ConsumerListener&) = delete;
        ConsumerListener& operator=(const ConsumerListener&) = delete;
        ConsumerListener(ConsumerListener&&) = delete;
        ConsumerListener& operator=(ConsumerListener&&) = delete;

        void OnBufferAvailable() final;

    private:
        std::weak_ptr<VpeVideoImpl> owner_;
    };

    void OnError(VPEAlgoErrCode errorCode);
    void OnStateLocked(VPEAlgoState state);
    void OnEffectChange(uint32_t type);
    void OnOutputBufferAvailable(uint32_t index, const VpeBufferInfo& info);

    GSError OnConsumerBufferAvailable();
    GSError OnProducerBufferReleased();

    VPEAlgoErrCode UpdateProducerLocked();
    VPEAlgoErrCode RenderOutputBuffer(uint32_t index, int64_t renderTimestamp, bool render);
    sptr<Surface> CreateConsumerSurfaceLocked();
    bool RequestBuffer(SurfaceBufferInfo& bufferInfo, GSError& errorCode);
    void PrepareBuffers();
    void AttachAndRefreshProducerBuffers(const sptr<Surface>& producer);
    void AttachBuffers(const sptr<Surface>& producer, std::queue<SurfaceBufferInfo>& bufferQueue);
    void RefreshProducerBuffers(std::queue<SurfaceBufferInfo>& bufferQueue,
        std::function<bool(SurfaceBufferInfo&)>&& refresher);
    void ProcessBuffers();
    bool ProcessBuffer(SurfaceBufferInfo& srcBufferInfo, SurfaceBufferInfo& dstBufferInfo);
    void BypassBuffer(SurfaceBufferInfo& srcBufferInfo, SurfaceBufferInfo& dstBufferInfo);
    void OutputBuffer(const SurfaceBufferInfo& bufferInfo, const SurfaceBufferInfo& bufferImage,
        std::function<void(void)>&& getReadyToRender);
    void NotifyEnableStatus(uint32_t type, const std::string& status);
    bool PopBuffer(std::queue<SurfaceBufferInfo>& bufferQueue, uint32_t index, SurfaceBufferInfo& bufferInfo,
        std::function<void(sptr<SurfaceBuffer>&)>&& func);
    void PrintBufferSize() const;
    void SetRequestCfgLocked(const sptr<SurfaceBuffer>& buffer);
    bool WaitTrigger();
    void CheckSpuriousWakeup();
    bool CheckStopping();
    bool CheckStoppingLocked();
    void ClearQueue(std::queue<SurfaceBufferInfo>& bufferQueue);
    void ClearConsumerLocked(std::queue<SurfaceBufferInfo>& bufferQueue);
    void ClearBufferQueues();

    VPEAlgoErrCode ExecuteWhenIdle(std::function<VPEAlgoErrCode(void)>&& operation, const std::string& errorMessage);
    VPEAlgoErrCode ExecuteWhenNotIdle(std::function<VPEAlgoErrCode(void)>&& operation, const std::string& errorMessage);
    VPEAlgoErrCode ExecuteWhenRunning(std::function<VPEAlgoErrCode(void)>&& operation, const std::string& errorMessage);
    VPEAlgoErrCode ExecuteWithCheck(std::function<bool(void)>&& checker,
        std::function<VPEAlgoErrCode(void)>&& operation, const std::string& errorMessage);

    // Common
    uint32_t type_{};

    // For thread control
    std::condition_variable cvTrigger_{};
    std::condition_variable cvDone_{};

    mutable std::mutex lock_{};
    // Guarded by lock_ begin
    std::atomic<bool> isInitialized_{false};
    std::atomic<bool> isRunning_{false};
    std::atomic<bool> isEnable_{true};
    std::atomic<bool> isEnableChange_{true};
    std::atomic<bool> hasConsumer_{false};
    std::atomic<VPEState> state_{VPEState::IDLE};
    uint32_t lastConsumerBufferId_{};
    GraphicTransformType lastTransform_{};
    ScalingMode lastScalingMode_{};
    bool isForceUpdateProducer_{true};
    std::thread worker_{};
    std::shared_ptr<VpeVideoCallback> cb_{};
    sptr<Surface> consumer_{};
    sptr<Surface> producer_{}; // Also guarded by producerLock_
    BufferRequestConfig requestCfg_{};
    BufferRequestConfig orgRequestCfg_{};
    // Guarded by lock_ end
    mutable std::mutex producerLock_{};

    mutable std::mutex taskLock_{};
    // Guarded by taskLock_ begin
    std::atomic<bool> isProcessing_{false};
    // Guarded by taskLock_ end

    mutable std::mutex consumerBufferLock_{};
    // Guarded by consumerBufferLock_ begin
    std::atomic<bool> isBufferQueueReady_{false};
    std::atomic<bool> needPrepareBuffers_{false};
    std::queue<SurfaceBufferInfo> consumerBufferQueue_{};
    // Guarded by consumerBufferLock_ end

    mutable std::mutex bufferLock_{};
    // Guarded by bufferLock_ begin
    std::queue<SurfaceBufferInfo> producerBufferQueue_{};
    std::unordered_map<uint32_t, SurfaceBufferInfo> renderBufferQueue_{};
    std::queue<SurfaceBufferInfo> flushBufferQueue_{};
    std::queue<SurfaceBufferInfo> attachBufferQueue_{};
    std::set<uint32_t> attachBufferIDs_{};
    // Guarded by bufferLock_ end
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // ALGORITHM_VIDEO_IMPL_H
