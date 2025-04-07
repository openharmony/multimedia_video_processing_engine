/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VPE_SA_TYPES_H
#define VPE_SA_TYPES_H

#include <cstdint>

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
struct VideoInfo {
    uint64_t videoIndex;
    uint64_t frameIndex;
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // VPE_SA_TYPES_H
