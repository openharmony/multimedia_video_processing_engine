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

#ifndef INTERFACES_INNER_API_ALGORITHM_ALGORITHM_UTILS_H
#define INTERFACES_INNER_API_ALGORITHM_ALGORITHM_UTILS_H

#include <string>

#include "algorithm_errors.h"
#include "algorithm_video_common.h"
#include "surface_buffer.h"

#define VPE_TO_STR(val) (std::string(#val) + "(" + std::to_string(static_cast<int>(val)) + ")")

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class AlgorithmUtils {
public:
    static std::string ToString(VPEAlgoErrCode errorCode);
    static std::string ToString(VPEAlgoState state);
    static bool CopySurfaceBufferToSurfaceBuffer(const sptr<SurfaceBuffer>& srcBuffer,
        sptr<SurfaceBuffer>& destBuffer);
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // INTERFACES_INNER_API_ALGORITHM_ALGORITHM_UTILS_H
