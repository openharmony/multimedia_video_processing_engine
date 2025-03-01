/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef COLORSPACE_CONVERTER_VIDEO_IMPL_H
#define COLORSPACE_CONVERTER_VIDEO_IMPL_H

#include <functional>
#include <queue>
#include <mutex>
#include <limits>
#include <map>
#include "colorspace_converter_video.h"
#include "meta/format.h"
#include "surface.h"
#include "sync_fence.h"
#include "colorspace_converter_video_common.h"
#include "colorspace_converter.h"
#include "algorithm_video_common.h"
namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class ColorSpaceConverterVideoImpl : public ColorSpaceConverterVideo {
public:
    ColorSpaceConverterVideoImpl();
    ~ColorSpaceConverterVideoImpl();
    int32_t Init();
    int32_t Init(std::shared_ptr<OpenGLContext> openglContext);
    int32_t SetCallback(const std::shared_ptr<ColorSpaceConverterVideoCallback> &callback) override;
    int32_t SetOutputSurface(sptr<Surface> surface) override;
    sptr<Surface> CreateInputSurface() override;
    int32_t Configure(const Format &format) override;
    int32_t Prepare() override;
    int32_t Start() override;
    int32_t Stop() override;
    int32_t Reset() override;
    int32_t Release() override;
    int32_t NotifyEos() override;
    int32_t ReleaseOutputBuffer(uint32_t index, bool render) override;
    int32_t SetParameter(const Format &parameter) override;
    int32_t GetParameter(Format &parameter) override;
    int32_t Flush() override;
    int32_t GetOutputFormat(Format &format) override;
    GSError OnConsumerBufferAvailable();
    GSError OnProducerBufferReleased();

private:
    struct SurfaceBufferWrapper {
    public:
        SurfaceBufferWrapper() = default;
        ~SurfaceBufferWrapper() = default;

        sptr<SurfaceBuffer> memory{nullptr};
        CscvBufferFlag bufferFlag{CSCV_BUFFER_FLAG_NONE};
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
    int32_t ConfigureColorSpace(const Format &format);
    int32_t AttachToNewSurface(sptr<Surface> newSurface);
    int32_t SetOutputSurfaceConfig(sptr<Surface> surface);
    int32_t SetOutputSurfaceRunning(sptr<Surface> newSurface);
    int32_t GetReleaseOutBuffer();

    std::atomic<VPEAlgoState> state_{VPEAlgoState::UNINITIALIZED};
    std::shared_ptr<ColorSpaceConverterVideoCallback> cb_{nullptr};
    std::shared_ptr<ColorSpaceConverter> csc_{nullptr};
    std::mutex mutex_;
    Format format_;
    Format outputFormat_;
    bool getUsage_{false};
    std::atomic<bool> initBuffer_{false};

      // task
    std::mutex mtxTaskDone_;
    std::condition_variable cvTaskDone_;
    std::shared_ptr<std::thread> taskThread_{nullptr};
    std::condition_variable cvTaskStart_;
    std::mutex mtxTaskStart_;
    std::atomic<bool> isRunning_{false};
    std::atomic<bool> isProcessing_{false};
    std::atomic<bool> isEos_{false};

      // surface
    std::queue<std::shared_ptr<SurfaceBufferWrapper>> outputBufferAvilQue_;
    std::queue<std::shared_ptr<SurfaceBufferWrapper>> inputBufferAvilQue_;
    std::queue<std::shared_ptr<SurfaceBufferWrapper>> renderBufferAvilQue_;
    using RenderBufferAvilMapType = std::map<uint64_t, std::shared_ptr<SurfaceBufferWrapper>>;
    RenderBufferAvilMapType renderBufferAvilMap_;
    RenderBufferAvilMapType outputBufferAvilQueBak_;
    RenderBufferAvilMapType renderBufferMapBak_;
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

      // colorsapce
    std::vector<uint8_t> colorSpaceVec_;
    int32_t hdrType_{0};
    std::vector<uint8_t> hdrVec_;
    int32_t lastOutputWidth_ = 0;
    int32_t lastOutputHeight_ = 0;
};

class ImageProcessBufferConsumerListener : public OHOS::IBufferConsumerListener {
public:
    explicit ImageProcessBufferConsumerListener(ColorSpaceConverterVideoImpl *process) : process_(process)
    {}
    void OnBufferAvailable() override;

private:
    ColorSpaceConverterVideoImpl *process_;
};

class ColorSpaceConverterVideoCallbackImpl : public ColorSpaceConverterVideoCallback {
public:
    struct Callback {
        using OnErrorCallback = std::function<void(int32_t, ArgumentType *)>;
        using OnOutputBufferAvailableCallback = std::function<void(uint32_t, int32_t, ArgumentType *)>;
        using OnOutputFormatChangedCallback = std::function<void(Format, ArgumentType *)>;

        OnErrorCallback onError;
        OnOutputBufferAvailableCallback onOutputBufferAvailable;
        OnOutputFormatChangedCallback OnOutputFormatChanged;
    };

    ColorSpaceConverterVideoCallbackImpl(Callback *callback, ArgumentType *userData);
    void OnError(int32_t errorCode);
    void OnState(int32_t state);
    void OnOutputBufferAvailable(uint32_t index, CscvBufferFlag flag);
    void OnOutputFormatChanged(const Format& format);

private:
    std::unique_ptr<Callback> callback_{nullptr};
    ArgumentType *userData_{nullptr};
};
}  // namespace VideoProcessingEngine
}  // namespace Media
}  // namespace OHOS
#endif  // COLORSPACE_CONVERTER_VIDEO_IMPL_H
