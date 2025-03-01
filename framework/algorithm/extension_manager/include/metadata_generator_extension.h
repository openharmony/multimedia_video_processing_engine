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

#ifndef FRAMEWORK_ALGORITHM_EXTENSION_MANAGER_METADATA_GENERATOR_EXTENSION_H
#define FRAMEWORK_ALGORITHM_EXTENSION_MANAGER_METADATA_GENERATOR_EXTENSION_H

#include <vector>
#include "metadata_generator_base.h"
#include "metadata_generator_capability.h"
#include "extension_base.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
namespace Extension {
struct MetadataGeneratorExtension : public ExtensionBase {
    MetadataGeneratorCreator creator;
    MetadataGeneratorCapabilitiesBuilder capabilitiesBuilder;
};
} // namespace Extension
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // FRAMEWORK_ALGORITHM_EXTENSION_MANAGER_METADATA_GENERATOR_EXTENSION_H
