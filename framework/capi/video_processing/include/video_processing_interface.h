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

#ifndef VIDEO_PROCESSING_INTERFACE_H
#define VIDEO_PROCESSING_INTERFACE_H

#include "video_processing_types.h"
#include "algorithm_common.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
/**
 * Interface for video processing.
 */
class IVideoProcessingNative {
public:
    virtual VideoProcessing_ErrorCode Initialize() = 0;
    virtual VideoProcessing_ErrorCode Deinitialize() = 0;
    virtual VideoProcessing_ErrorCode RegisterCallback(const VideoProcessing_Callback* callback, void* userData) = 0;
    virtual VideoProcessing_ErrorCode SetSurface(const OHNativeWindow* window) = 0;
    virtual VideoProcessing_ErrorCode GetSurface(OHNativeWindow** window) = 0;
    virtual VideoProcessing_ErrorCode SetParameter(const OH_AVFormat* parameter) = 0;
    virtual VideoProcessing_ErrorCode GetParameter(OH_AVFormat* parameter) = 0;
    virtual VideoProcessing_ErrorCode Start() = 0;
    virtual VideoProcessing_ErrorCode Stop() = 0;
    virtual VideoProcessing_ErrorCode RenderOutputBuffer(uint32_t index) = 0;

protected:
    IVideoProcessingNative() = default;
    virtual ~IVideoProcessingNative() = default;
    IVideoProcessingNative(const IVideoProcessingNative&) = delete;
    IVideoProcessingNative& operator=(const IVideoProcessingNative&) = delete;
    IVideoProcessingNative(IVideoProcessingNative&&) = delete;
    IVideoProcessingNative& operator=(IVideoProcessingNative&&) = delete;

public:
    std::shared_ptr<OpenGLContext> openglContext_ {nullptr};
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // VIDEO_PROCESSING_INTERFACE_H
