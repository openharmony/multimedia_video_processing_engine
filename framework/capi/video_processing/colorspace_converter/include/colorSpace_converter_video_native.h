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

#ifndef COLORSPACE_CONVERTER_VIDEO_NATIVE_H
#define COLORSPACE_CONVERTER_VIDEO_NATIVE_H

#include <atomic>
#include <functional>
#include <mutex>

#include "video_processing_native_template.h"
#include "colorspace_converter_video.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
/**
 * ColorSpace converter NDK interface implementaion.
 */
class ColorSpaceConverterVideoNative : public VideoProcessingNativeTemplate<ColorSpaceConverterVideoNative> {
public:
    DEFINE_WITH_DISALLOW_COPY_AND_MOVE(ColorSpaceConverterVideoNative);

    VideoProcessing_ErrorCode InitializeInner() override;
    VideoProcessing_ErrorCode DeinitializeInner() override;
    VideoProcessing_ErrorCode RegisterCallback() override;
    VideoProcessing_ErrorCode SetSurface(const sptr<Surface>& surface, const OHNativeWindow& window) override;
    sptr<Surface> GetSurface() override;
    VideoProcessing_ErrorCode SetParameter(const OHOS::Media::Format& parameter) override;
    VideoProcessing_ErrorCode GetParameter(OHOS::Media::Format& parameter) override;
    VideoProcessing_ErrorCode OnStart() override;
    VideoProcessing_ErrorCode OnStop() override;
    VideoProcessing_ErrorCode OnRenderOutputBuffer(uint32_t index) override;

private:
    class NativeCallback : public ColorSpaceConverterVideoCallback {
    public:
        explicit NativeCallback(const std::shared_ptr<ColorSpaceConverterVideoNative>& owner);
        virtual ~NativeCallback() = default;
        NativeCallback(const NativeCallback&) = delete;
        NativeCallback& operator=(const NativeCallback&) = delete;
        NativeCallback(NativeCallback&&) = delete;
        NativeCallback& operator=(NativeCallback&&) = delete;

        void OnError(int32_t errorCode) override;
        void OnState(int32_t state) override;
        void OnOutputBufferAvailable(uint32_t index, CscvBufferFlag flag) override;
        void OnOutputFormatChanged(const Format& format) override;

    private:
        void SendCallback(std::function<void(void)>&& callback) const;

        const std::shared_ptr<ColorSpaceConverterVideoNative> owner_{};
    };

    mutable std::mutex lock_{};
    // Guarded by lock_ begin
    std::atomic<bool> isInitialized_{false};
    std::shared_ptr<ColorSpaceConverterVideo> colorSpaceConverter_{};
    int32_t colorSpaceValue_ = 0;
    int32_t formatValue_ = 0;
    int32_t metadataValue_ = 0;
    // Guarded by lock_ end
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // COLORSPACE_CONVERTER_VIDEO_NATIVE_H
