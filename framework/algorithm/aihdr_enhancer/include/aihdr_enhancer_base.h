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

#ifndef AIHDR_ENHANCER_BASE_H
#define AIHDR_ENHANCER_BASE_H

#include <functional>
#include <memory>

#include "nocopyable.h"

#include "frame_info.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class AihdrEnhancerBase : public NoCopyable {
public:
    virtual ~AihdrEnhancerBase() = default;
    virtual VPEAlgoErrCode Init() = 0;
    virtual VPEAlgoErrCode Deinit() = 0;
    virtual VPEAlgoErrCode SetParameter(const int& parameter) = 0;
    virtual VPEAlgoErrCode GetParameter(int& parameter) = 0;
    virtual VPEAlgoErrCode Process(const sptr<SurfaceBuffer>& input) = 0;
};

using AihdrEnhancerCreator = std::function<std::shared_ptr<AihdrEnhancerBase>()>;
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // AIHDR_ENHANCER_BASE_H
