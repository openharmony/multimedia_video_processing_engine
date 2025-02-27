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

#ifndef FRAMEWORK_ALGORITHM_EXTENSION_MANAGER_UTILS_H
#define FRAMEWORK_ALGORITHM_EXTENSION_MANAGER_UTILS_H

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>
#include "extension_base.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
namespace Extension {
namespace Utils {
using RegisterExtensionFunc = std::function<std::vector<std::shared_ptr<ExtensionBase>>()>;
} // namespace Utils

using ExtensionList = std::vector<std::shared_ptr<ExtensionBase>>;

void DoRegisterExtensions(uintptr_t addr, Utils::RegisterExtensionFunc func);
} // namespace Extension
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#define EXTENSION_EXPORT extern "C" __attribute__((visibility("default")))

#define REGISTER_EXTENSIONS(libName, registerFunc)                                                              \
    EXTENSION_EXPORT void Register##libName##Extensions(uintptr_t extensionListAddr)                            \
    {                                                                                                           \
        OHOS::Media::VideoProcessingEngine::Extension::DoRegisterExtensions(extensionListAddr, (registerFunc)); \
    }

#endif // FRAMEWORK_ALGORITHM_EXTENSION_MANAGER_UTILS_H
