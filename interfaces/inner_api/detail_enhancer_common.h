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

#ifndef DETAIL_ENHANCER_COMMON_H
#define DETAIL_ENHANCER_COMMON_H

#include <string>

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
enum DetailEnhancerLevel {
    DETAIL_ENH_LEVEL_NONE = 0,
    DETAIL_ENH_LEVEL_LOW,
    DETAIL_ENH_LEVEL_MEDIUM,
     // High gear is the highest gear. If you need to add a higher gear, you should
     // pay attention to the level configuration of the high gear in impl.
    DETAIL_ENH_LEVEL_HIGH,
    DETAIL_ENH_LEVEL_NUM
};

enum SourceType {
    SOURCE_TYPE_INVALID = -1,
    IMAGE,
    VIDEO,
    SOURCE_TYPE_NUM
};

struct DetailEnhancerParameters {
    std::string uri{};
    DetailEnhancerLevel level{DETAIL_ENH_LEVEL_LOW};
    int contentId;
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // DETAIL_ENHANCER_COMMON_H

