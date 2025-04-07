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

#ifndef VIDEO_PROCESSING_ALGORITHM_FACTORY_COMMON_H
#define VIDEO_PROCESSING_ALGORITHM_FACTORY_COMMON_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "ivideo_processing_algorithm.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
using VpeAlgorithmCreator = std::function<std::shared_ptr<IVideoProcessingAlgorithm>(const std::string&, uint32_t)>;

struct VpeAlgorithmCreatorInfo {
    uint32_t id;
    VpeAlgorithmCreator creator;
};

using VpeAlgorithmCreatorMap = std::unordered_map<std::string, VpeAlgorithmCreatorInfo>;

// NOTE:
// All algorithms MUST be derived from VideoProcessingAlgorithmWithData or VideoProcessingAlgorithmWithoutData.
// And algorithms MUST keep the input parameters of constructor like (const std::string& feature, uint32_t id).
template <typename T>
std::shared_ptr<IVideoProcessingAlgorithm> CreateVpeAlgorithm(const std::string& feature, uint32_t id)
{
    return std::make_shared<T>(feature, id);
}

template <typename T>
VpeAlgorithmCreatorInfo MakeCreator()
{
    VpeAlgorithmCreatorInfo info {
        .id = 0,
        .creator = CreateVpeAlgorithm<T>,
    };
    return info;
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // VIDEO_PROCESSING_ALGORITHM_FACTORY_COMMON_H
