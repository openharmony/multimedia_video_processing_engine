/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AIHDR_ENHANCER_VIDEO_IMPL_H
#define AIHDR_ENHANCER_VIDEO_IMPL_H

#include <functional>
#include <limits>
#include <map>
#include <mutex>
#include <queue>

#include "native_window.h"
#include "surface.h"
#include "sync_fence.h"

#include "aihdr_enhancer.h"
#include "aihdr_enhancer_video.h"
#include "aihdr_enhancer_video_common.h"
#include "algorithm_video_common.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class AihdrEnhancerVideoImpl : public AihdrEnhancerVideo {
public:
    AihdrEnhancerVideoImpl();
    ~AihdrEnhancerVideoImpl();
    int32_t Init();
    // 北向调用接口
    int32_t SetCallback(const std::shared_ptr<AihdrEnhancerVideoCallback> &callback) override;
    int32_t SetOutputSurface(sptr<Surface> surface);
    sptr<Surface> CreateInputSurface();
    int32_t SetSurface(const OHNativeWindow* window) override;
    int32_t GetSurface(OHNativeWindow** window) override;
    int32_t Configure() override;
    int32_t Prepare() override;
    int32_t Start() override;
    int32_t Stop() override;
    int32_t Reset() override;
    int32_t Release() override;
    int32_t NotifyEos() override;
    int32_t ReleaseOutputBuffer(uint32_t index, bool render) override;
    int32_t Flush() override;

    GSError OnConsumerBufferAvailable();
    GSError OnProducerBufferReleased();
private:
    struct SurfaceBufferWrapper {
    public:
        SurfaceBufferWrapper() = default;
        ~SurfaceBufferWrapper() = default;

        sptr<SurfaceBuffer> memory{nullptr};
        AihdrEnhancerBufferFlag bufferFlag{AIHDR_ENHANCER_BUFFER_FLAG_NONE};
        sptr<SyncFence> fence{nullptr};
        int64_t timestamp;
    };
    void InitBuffers();
    bool WaitProcessing();
    bool AcquireInputOutputBuffers(
        std::shared_ptr<SurfaceBufferWrapper> &inputBuffer, std::shared_ptr<SurfaceBufferWrapper> &outputBuffer);
    void DoTask();
    void OnTriggered();
    void Process(std::shared_ptr<SurfaceBufferWrapper> inputBuffer, std::shared_ptr<SurfaceBufferWrapper> outputBuffer);
    int32_t AttachToNewSurface(sptr<Surface> newSurface);
    int32_t SetOutputSurfaceConfig(sptr<Surface> surface);
    int32_t SetOutputSurfaceRunning(sptr<Surface> newSurface);
    int32_t GetReleaseOutBuffer();
    std::atomic<VPEAlgoState> state_{VPEAlgoState::UNINITIALIZED};
    std::shared_ptr<AihdrEnhancerVideoCallback> cb_{nullptr};
    std::shared_ptr<AihdrEnhancer> csc_{nullptr};
    std::mutex mutex_;
    bool getUsage_{false};

    // task相关
    std::mutex mtxTaskDone_;
    std::condition_variable cvTaskDone_;
    std::shared_ptr<std::thread> taskThread_{nullptr};
    std::condition_variable cvTaskStart_;
    std::mutex mtxTaskStart_;
    std::atomic<bool> isRunning_{false};
    std::atomic<bool> isProcessing_{false};
    std::atomic<bool> isEos_{false};

    // surface相关
    std::queue<std::shared_ptr<SurfaceBufferWrapper>> outputBufferAvilQue_;
    std::queue<std::shared_ptr<SurfaceBufferWrapper>> inputBufferAvilQue_;
    std::queue<std::shared_ptr<SurfaceBufferWrapper>> renderBufferAvilQue_;
    using RenderBufferAvilMapType = std::map<uint64_t, std::shared_ptr<SurfaceBufferWrapper>>;
    RenderBufferAvilMapType renderBufferAvilMap_;
    RenderBufferAvilMapType renderBufferMapBak_;
    RenderBufferAvilMapType outputBufferAvilQueBak_;
    std::mutex onBqMutex_;       // inputsruface buffer
    std::mutex renderQueMutex_;  // outputsruface buffer
    std::mutex surfaceChangeMutex_;
    std::mutex surfaceChangeMutex2_;
    sptr<Surface> inputSurface_{nullptr};
    sptr<Surface> outputSurface_{nullptr};
    static constexpr size_t MAX_BUFFER_CNT{5};
    uint32_t outBufferCnt_{MAX_BUFFER_CNT};
    uint32_t inBufferCnt_{MAX_BUFFER_CNT};
    static constexpr size_t MAX_SURFACE_SEQUENCE{std::numeric_limits<uint32_t>::max()};
    uint32_t lastSurfaceSequence_{MAX_SURFACE_SEQUENCE};
    BufferRequestConfig requestCfg_{};
    BufferFlushConfig flushCfg_{};
};

class AihdrEnhancerBufferConsumerListener : public OHOS::IBufferConsumerListener {
public:
    explicit AihdrEnhancerBufferConsumerListener(AihdrEnhancerVideoImpl *process) : process_(process) {}
    void OnBufferAvailable() override;

private:
    AihdrEnhancerVideoImpl *process_;
};

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
#endif // AIHDR_ENHANCER_VIDEO_IMPL_H
