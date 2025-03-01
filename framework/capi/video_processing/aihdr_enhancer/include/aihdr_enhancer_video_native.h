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

#ifndef AIHDR_ENHANCER_VIDEO_NATIVE_H
#define AIHDR_ENHANCER_VIDEO_NATIVE_H

#include <atomic>
#include <functional>
#include <mutex>

#include "external_window.h"

#include "aihdr_enhancer_video.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
typedef void (*VideoProcessingCallback_OnError)(int32_t error);
typedef void (*VideoProcessingCallback_OnState)(int32_t state);
typedef void (*VideoProcessingCallback_OnNewOutputBuffer)(uint32_t index);

class  __attribute__((visibility("default"))) AihdrEnhancerVideoNative :
    public std::enable_shared_from_this<AihdrEnhancerVideoNative> {
public:
    int32_t InitializeInner();
    int32_t DeinitializeInner();
    int32_t RegisterCallback();
    int32_t SetSurface(const OHNativeWindow* window);
    int32_t GetSurface(OHNativeWindow** window);
    int32_t SetParameter(const int& parameter);
    int32_t GetParameter(int& parameter);
    int32_t OnStart();
    int32_t OnStop();
    int32_t OnRenderOutputBuffer(uint32_t index);

    void BindOnError(VideoProcessingCallback_OnError onError);
    void BindOnState(VideoProcessingCallback_OnState onState);
    void BindOnNewOutputBuffer(VideoProcessingCallback_OnNewOutputBuffer onNewOutputBuffer);

    void OnError(int32_t errorCode);
    void OnState(int32_t state);
    void OnNewOutputBuffer(uint32_t index);

private:
    class NativeCallback : public AihdrEnhancerVideoCallback {
    public:
        explicit NativeCallback(const std::shared_ptr<AihdrEnhancerVideoNative>& owner);
        virtual ~NativeCallback() = default;
        NativeCallback(const NativeCallback&) = delete;
        NativeCallback& operator=(const NativeCallback&) = delete;
        NativeCallback(NativeCallback&&) = delete;
        NativeCallback& operator=(NativeCallback&&) = delete;

        void OnError(int32_t errorCode) override;
        void OnState(int32_t state) override;
        void OnOutputBufferAvailable(uint32_t index, AihdrEnhancerBufferFlag flag) override;

    private:
        void SendCallback(std::function<void(void)>&& callback) const;

        const std::shared_ptr<AihdrEnhancerVideoNative> owner_{};
    };

    VideoProcessingCallback_OnError onError_{};
    VideoProcessingCallback_OnState onState_{};
    VideoProcessingCallback_OnNewOutputBuffer onNewOutputBuffer_{};
    mutable std::mutex lock_{};
    // Guarded by lock_ begin
    std::atomic<bool> isInitialized_{false};
    std::shared_ptr<AihdrEnhancerVideo> aihdrEnhancer_{};
    // Guarded by lock_ end
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // AIHDR_ENHANCER_VIDEO_NATIVE_H
