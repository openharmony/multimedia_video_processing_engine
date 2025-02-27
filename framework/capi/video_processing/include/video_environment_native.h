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

#ifndef VIDEO_ENVIRONMENT_NATIVE_H
#define VIDEO_ENVIRONMENT_NATIVE_H

#include <cinttypes>
#include <mutex>

#include "video_processing_types.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
/**
 * Implementation for video processing environment.
 */
class VideoEnvironmentNative {
public:
    static VideoEnvironmentNative& Get();

    VideoProcessing_ErrorCode Initialize();
    VideoProcessing_ErrorCode Deinitialize();
    VideoProcessing_ErrorCode InitializeByDefault();
    VideoProcessing_ErrorCode DeinitializeByDefault();

private:
    VideoEnvironmentNative() = default;
    virtual ~VideoEnvironmentNative() = default;
    VideoEnvironmentNative(const VideoEnvironmentNative&) = delete;
    VideoEnvironmentNative& operator=(const VideoEnvironmentNative&) = delete;
    VideoEnvironmentNative(VideoEnvironmentNative&&) = delete;
    VideoEnvironmentNative& operator=(VideoEnvironmentNative&&) = delete;

    VideoProcessing_ErrorCode InitializeLocked();
    VideoProcessing_ErrorCode DeinitializeLocked();
    VideoProcessing_ErrorCode InitializeEnvLocked();
    VideoProcessing_ErrorCode DeinitializeEnvLocked();

    std::mutex lock_{};
    // Guarded by lock_ begin
    bool isExplicitInit_{};
    uint32_t referenceCount_{};
    // Guarded by lock_ end
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // VIDEO_ENVIRONMENT_NATIVE_H
