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
        std::shared_ptr<VpeVideoImpl> owner_;
    };

    void OnErrorLocked(VPEAlgoErrCode errorCode);
    void OnStateLocked(VPEAlgoState state);
    void OnEffectChange(uint32_t type);
    void OnOutputBufferAvailable(uint32_t index, const VpeBufferInfo& info);

    GSError OnConsumerBufferAvailable();
    GSError OnProducerBufferReleased();

    VPEAlgoErrCode RenderOutputBufferLocked(uint32_t index, int64_t renderTimestamp, bool render);
    sptr<Surface> CreateConsumerSurfaceLocked();
    bool RequestBuffer(GSError& errorCode);
    void PrepareBuffers();
    void ProcessBuffers();
    bool ProcessBuffer(sptr<Surface>& consumer, SurfaceBufferInfo& srcBufferInfo, SurfaceBufferInfo& dstBufferInfo);
    void BypassBuffer(SurfaceBufferInfo& srcBufferInfo, SurfaceBufferInfo& dstBufferInfo);
    void OutputBuffer(const SurfaceBufferInfo& bufferInfo, const SurfaceBufferInfo& bufferImage, bool isProcessed,
        std::function<void(void)>&& getReadyToRender);
    bool PopBuffer(std::queue<SurfaceBufferInfo>& bufferQueue, uint32_t index, SurfaceBufferInfo& bufferInfo,
        std::function<void(sptr<SurfaceBuffer>&)>&& func);
    void SetRequestCfgLocked(const sptr<SurfaceBuffer>& buffer);
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
    std::condition_variable cv_{};

    mutable std::mutex lock_{};
    // Guarded by lock_ begin
    std::atomic<bool> isInitialized_{false};
    std::atomic<bool> isRunning_{false};
    std::atomic<bool> isEnable_{true};
    std::atomic<bool> isEnableChange_{true};
    std::atomic<VPEState> state_{VPEState::IDLE};
    std::thread worker_{};
    std::shared_ptr<VpeVideoCallback> cb_{};
    sptr<Surface> consumer_{};
    sptr<Surface> producer_{};
    BufferRequestConfig requestCfg_{};
    // Guarded by lock_ end

    mutable std::mutex bufferLock_{};
    // Guarded by bufferLock_ begin
    bool isBufferQueueReady_{};
    std::queue<SurfaceBufferInfo> consumerBufferQueue_{};
    std::queue<SurfaceBufferInfo> producerBufferQueue_{};
    std::queue<SurfaceBufferInfo> renderBufferQueue_{};
    std::queue<SurfaceBufferInfo> attachBufferQueue_{};
    std::set<uint32_t> attachBufferIDs_{};
    // Guarded by bufferLock_ end
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // ALGORITHM_VIDEO_IMPL_H
