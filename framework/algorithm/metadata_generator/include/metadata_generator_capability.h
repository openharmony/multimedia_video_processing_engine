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

#ifndef FRAMEWORK_ALGORITHM_METADATA_GENERATOR_METADATA_GENERATOR_CAPABILITY_H
#define FRAMEWORK_ALGORITHM_METADATA_GENERATOR_METADATA_GENERATOR_CAPABILITY_H

#include <functional>
#include <vector>
#include "frame_info.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
struct MetadataGeneratorCapability {
    ColorSpaceDescription colorspaceDesc;
    std::vector<GraphicPixelFormat> pixelFormats;
    uint32_t rank;
    int32_t version;
};

using MetadataGeneratorCapabilitiesBuilder = std::function<std::vector<MetadataGeneratorCapability>()>;
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // FRAMEWORK_ALGORITHM_METADATA_GENERATOR_METADATA_GENERATOR_CAPABILITY_H
