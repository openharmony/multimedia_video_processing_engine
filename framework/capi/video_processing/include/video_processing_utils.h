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

#ifndef VIDEO_PROCESSING_UTILS_H
#define VIDEO_PROCESSING_UTILS_H

#include <string>

#include "algorithm_errors.h"
#include "algorithm_video_common.h"
#include "video_processing_types.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
/**
 * Utility funcions for video processing CAPI.
 */
class VideoProcessingUtils {
public:
    static VideoProcessing_ErrorCode InnerErrorToCAPI(VPEAlgoErrCode errorCode);
    static VideoProcessing_State InnerStateToCAPI(VPEAlgoState state);
    static std::string ToString(VideoProcessing_ErrorCode errorCode);
    static std::string ToString(VideoProcessing_State state);
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // VIDEO_PROCESSING_UTILS_H
