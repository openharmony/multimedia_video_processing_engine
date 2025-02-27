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

#ifndef FRAMEWORK_ALGORITHM_VIDEO_REFRESHRATE_PREDICTION_BASE_H
#define FRAMEWORK_ALGORITHM_VIDEO_REFRESHRATE_PREDICTION_BASE_H

#include <functional>
#include <memory>
#include "nocopyable.h"
#include "frame_info.h"


namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class VideoRefreshRatePredictionBase : public NoCopyable {
public:
    virtual ~VideoRefreshRatePredictionBase() = default;
    virtual VPEAlgoErrCode CheckVRRSupport(std::string processName) = 0;
    virtual VPEAlgoErrCode Process(const sptr<SurfaceBuffer> &input, int videoFps, int codecType) = 0;
};

using VideoRefreshRatePredictionCreator = std::function<std::shared_ptr<VideoRefreshRatePredictionBase>()>;
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // FRAMEWORK_ALGORITHM_VIDEO_REFRESHRATE_PREDICTION_BASE_H
