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

#ifndef METADATA_GENERATOR_VIDEO_NATIVE_H
#define METADATA_GENERATOR_VIDEO_NATIVE_H

#include <atomic>
#include <functional>
#include <mutex>

#include "video_processing_native_template.h"
#include "metadata_generator_video.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
/**
 * Metadata generator CAPI interface implementaion.
 */
class MetadataGeneratorVideoNative : public VideoProcessingNativeTemplate<MetadataGeneratorVideoNative> {
public:
    DEFINE_WITH_DISALLOW_COPY_AND_MOVE(MetadataGeneratorVideoNative);

    VideoProcessing_ErrorCode InitializeInner() override;
    VideoProcessing_ErrorCode DeinitializeInner() override;
    VideoProcessing_ErrorCode RegisterCallback() override;
    VideoProcessing_ErrorCode SetSurface(const sptr<Surface>& surface) override;
    sptr<Surface> GetSurface() override;
    VideoProcessing_ErrorCode SetParameter(const OHOS::Media::Format& parameter) override;
    VideoProcessing_ErrorCode GetParameter(OHOS::Media::Format& parameter) override;
    VideoProcessing_ErrorCode OnStart() override;
    VideoProcessing_ErrorCode OnStop() override;
    VideoProcessing_ErrorCode OnRenderOutputBuffer(uint32_t index) override;

private:
    class NativeCallback : public MetadataGeneratorVideoCallback {
    public:
        explicit NativeCallback(const std::shared_ptr<MetadataGeneratorVideoNative>& owner);
        virtual ~NativeCallback() = default;
        NativeCallback(const NativeCallback&) = delete;
        NativeCallback& operator=(const NativeCallback&) = delete;
        NativeCallback(NativeCallback&&) = delete;
        NativeCallback& operator=(NativeCallback&&) = delete;

        void OnError(int32_t errorCode) override;
        void OnState(int32_t state) override;
        void OnOutputBufferAvailable(uint32_t index, MdgBufferFlag flag) override;

    private:
        void SendCallback(std::function<void(void)>&& callback) const;

        const std::shared_ptr<MetadataGeneratorVideoNative> owner_{};
    };
    int CApiStyleToInner(int Style) const;

    mutable std::mutex lock_{};
    // Guarded by lock_ begin
    std::atomic<bool> isInitialized_{false};
    std::shared_ptr<MetadataGeneratorVideo> metadataGenerator_{};
    // Guarded by lock_ end
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // METADATA_GENERATOR_VIDEO_NATIVE_H
