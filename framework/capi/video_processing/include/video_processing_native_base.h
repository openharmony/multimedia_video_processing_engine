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

#ifndef VIDEO_PROCESSING_NATIVE_BASE_H
#define VIDEO_PROCESSING_NATIVE_BASE_H

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <mutex>
#include <set>

#include "common/native_mfmagic.h"
#include "surface.h"

#include "video_processing_callback_native.h"
#include "video_processing_interface.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
/**
 * Base implementaion for video processing.
 */
class VideoProcessingNativeBase : public IVideoProcessingNative {
public:
    VideoProcessing_ErrorCode Initialize() final;
    VideoProcessing_ErrorCode Deinitialize() final;
    VideoProcessing_ErrorCode RegisterCallback(const VideoProcessing_Callback* callback, void* userData) final;
    VideoProcessing_ErrorCode SetSurface(const OHNativeWindow* window) final;
    VideoProcessing_ErrorCode GetSurface(OHNativeWindow** window) final;
    VideoProcessing_ErrorCode SetParameter(const OH_AVFormat* parameter) final;
    VideoProcessing_ErrorCode GetParameter(OH_AVFormat* parameter) final;
    VideoProcessing_ErrorCode Start() final;
    VideoProcessing_ErrorCode Stop() final;
    VideoProcessing_ErrorCode RenderOutputBuffer(uint32_t index) final;

protected:
    explicit VideoProcessingNativeBase(OH_VideoProcessing* context);
    virtual ~VideoProcessingNativeBase() = default;
    VideoProcessingNativeBase(const VideoProcessingNativeBase&) = delete;
    VideoProcessingNativeBase& operator=(const VideoProcessingNativeBase&) = delete;
    VideoProcessingNativeBase(VideoProcessingNativeBase&&) = delete;
    VideoProcessingNativeBase& operator=(VideoProcessingNativeBase&&) = delete;

    virtual VideoProcessing_ErrorCode InitializeInner();
    virtual VideoProcessing_ErrorCode DeinitializeInner();
    virtual VideoProcessing_ErrorCode RegisterCallback();
    virtual VideoProcessing_ErrorCode SetSurface(const sptr<Surface>& surface, const OHNativeWindow& window);
    virtual VideoProcessing_ErrorCode SetSurface(const sptr<Surface>& surface);
    virtual sptr<Surface> GetSurface();
    virtual VideoProcessing_ErrorCode SetParameter(const OHOS::Media::Format& parameter);
    virtual VideoProcessing_ErrorCode GetParameter(OHOS::Media::Format& parameter);
    virtual VideoProcessing_ErrorCode OnStart();
    virtual VideoProcessing_ErrorCode OnStop();
    virtual VideoProcessing_ErrorCode OnRenderOutputBuffer(uint32_t index);

    // Called by child classes of features(Such as DetailEnhancerVideoNative) to send information to users' callbacks
    void OnError(VideoProcessing_ErrorCode errorCode);
    void OnState(VideoProcessing_State state);
    void OnNewOutputBuffer(uint32_t index);

private:
    struct CallbackInfo {
        std::shared_ptr<VideoProcessingCallbackNative> callback{};
        void* userData{};
        bool operator<(const CallbackInfo& other) const
        {
            return (callback == other.callback) ? (userData < other.userData) : (callback < other.callback);
        }
    };

    VideoProcessing_ErrorCode ExecuteWhenIdle(std::function<VideoProcessing_ErrorCode(void)>&& task,
        const std::string& errLog);
    VideoProcessing_ErrorCode ExecuteWhenRunning(std::function<VideoProcessing_ErrorCode(void)>&& task,
        const std::string& errLog);
    void OnCallback(std::function<void(std::shared_ptr<VideoProcessingCallbackNative>&, void*)>&& task,
        const std::string& name);
    void TraverseCallbacksLocked(std::function<void(std::shared_ptr<VideoProcessingCallbackNative>&, void*)>&& task);
    void TraverseCallbacksExLocked(std::function<bool(std::shared_ptr<VideoProcessingCallbackNative>&, void*)>&& task);
    VideoProcessing_ErrorCode PrepareRegistrationLocked();

    OH_VideoProcessing* context_{};
    mutable std::mutex lock_{};
    // Guarded by lock_ begin
    std::atomic<bool> isInitialized_{false};
    std::atomic<bool> isRunning_{false};
    std::atomic<bool> hasInputSurface_{false};
    std::atomic<bool> hasOutputSurface_{false};
    // Guarded by lock_ end
    std::atomic<bool> isOnNewOutputBuffer_{};
    mutable std::mutex callbackLock_{};
    // Guarded by callbackLock_ begin
    std::atomic<bool> hasCallback_{false};
    std::atomic<bool> hasOnRenderOutputBuffer_{false};
    bool isInnerCallbackReady_{false};
    std::set<CallbackInfo> callbacks_{};
    // Guarded by callbackLock_ end
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // VIDEO_PROCESSING_NATIVE_BASE_H
