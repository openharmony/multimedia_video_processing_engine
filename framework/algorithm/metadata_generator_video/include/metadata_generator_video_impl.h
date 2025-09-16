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

#ifndef METADATA_GENERATOR_VIDEO_IMPL_H
#define METADATA_GENERATOR_VIDEO_IMPL_H

#include <functional>
#include <queue>
#include <mutex>
#include <limits>
#include <map>
#include "metadata_generator_video.h"
#include "surface.h"
#include "sync_fence.h"
#include "metadata_generator_video_common.h"
#include "metadata_generator.h"
#include "algorithm_video_common.h"
namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class MetadataGeneratorVideoImpl : public MetadataGeneratorVideo {
public:
    MetadataGeneratorVideoImpl();
    ~MetadataGeneratorVideoImpl();
    int32_t Init();
    int32_t Init(std::shared_ptr<OpenGLContext> openglContext);
    // 北向调用接口
    int32_t SetCallback(const std::shared_ptr<MetadataGeneratorVideoCallback> &callback) override;
    int32_t SetOutputSurface(sptr<Surface> surface) override;
    sptr<Surface> CreateInputSurface() override;
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
        MdgBufferFlag bufferFlag{MDG_BUFFER_FLAG_NONE};
        sptr<SyncFence> fence{nullptr};
        int64_t timestamp;
    };
    void InitBuffers();
    bool WaitProcessing();
    bool AcquireInputBuffers(std::shared_ptr<SurfaceBufferWrapper> &inputBuffer);
    void DoTask();
    void OnTriggered();
    void CheckRequestCfg(sptr<SurfaceBuffer> inputBuffer);
    void Process(std::shared_ptr<SurfaceBufferWrapper> inputBuffer);
    int32_t AttachToNewSurface(sptr<Surface> newSurface);
    int32_t SetOutputSurfaceConfig(sptr<Surface> surface);
    int32_t SetOutputSurfaceRunning(sptr<Surface> newSurface);
    int32_t GetReleaseOutBuffer();
    std::atomic<VPEAlgoState> state_{VPEAlgoState::UNINITIALIZED};
    std::shared_ptr<MetadataGeneratorVideoCallback> cb_{nullptr};
    std::shared_ptr<MetadataGenerator> csc_{nullptr};
    std::mutex mutex_;
    bool getUsage_{false};
    std::atomic<bool> initBuffer_{false};

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
    std::mutex outputQueMutex_;
    sptr<Surface> inputSurface_{nullptr};
    sptr<Surface> outputSurface_{nullptr};
    static constexpr size_t MAX_INPUT_BUFFER_CNT{5};
    static constexpr size_t MAX_OUTPUT_BUFFER_CNT{12};
    uint32_t outBufferCnt_{MAX_OUTPUT_BUFFER_CNT};
    uint32_t inBufferCnt_{MAX_INPUT_BUFFER_CNT};
    static constexpr size_t MAX_SURFACE_SEQUENCE{std::numeric_limits<uint32_t>::max()};
    uint32_t lastSurfaceSequence_{MAX_SURFACE_SEQUENCE};
    BufferRequestConfig requestCfg_{};
    BufferFlushConfig flushCfg_{};
};

class MetadataGeneratorBufferConsumerListener : public OHOS::IBufferConsumerListener {
public:
    explicit MetadataGeneratorBufferConsumerListener(MetadataGeneratorVideoImpl *process) : process_(process) {}
    void OnBufferAvailable() override;

private:
    MetadataGeneratorVideoImpl *process_;
};

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
#endif // METADATA_GENERATOR_VIDEO_IMPL_H
