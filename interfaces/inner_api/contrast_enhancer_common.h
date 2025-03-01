/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef CONTRAST_ENHANCER_COMMON_H
#define CONTRAST_ENHANCER_COMMON_H

#include <string>

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
enum ContrastEnhancerLevel {
    CONTRAST_ENHANCER = 0,
};

struct ContrastEnhancerParameters {
    std::string uri{};
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // CONTRAST_ENHANCER_COMMON_H

