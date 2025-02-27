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

#ifndef VIDEO_PROCESSING_NATIVE_TEMPLATE_H
#define VIDEO_PROCESSING_NATIVE_TEMPLATE_H

#include <memory>

#include "nocopyable.h"
#include "video_processing_native_base.h"

#define DEFINE_WITH_DISALLOW_COPY_AND_MOVE(className) \
    className([[maybe_unused]] Protected mask, OH_VideoProcessing* context) \
        : VideoProcessingNativeTemplate<className>(context) {} \
    virtual ~className() = default; \
    DISALLOW_COPY_AND_MOVE(className)

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
/**
 * Base implementaion for video processing.
 */
template <typename T>
class VideoProcessingNativeTemplate : public VideoProcessingNativeBase, public std::enable_shared_from_this<T> {
public:
    static inline std::shared_ptr<T> Create(OH_VideoProcessing* context)
    {
        return std::make_shared<T>(Protected(), context);
    }

protected:
    struct Protected { explicit Protected() = default; };

    explicit VideoProcessingNativeTemplate(OH_VideoProcessing* context) : VideoProcessingNativeBase(context) {}
    virtual ~VideoProcessingNativeTemplate() = default;
    VideoProcessingNativeTemplate(const VideoProcessingNativeTemplate&) = delete;
    VideoProcessingNativeTemplate& operator=(const VideoProcessingNativeTemplate&) = delete;
    VideoProcessingNativeTemplate(VideoProcessingNativeTemplate&&) = delete;
    VideoProcessingNativeTemplate& operator=(VideoProcessingNativeTemplate&&) = delete;
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // VIDEO_PROCESSING_NATIVE_TEMPLATE_H
