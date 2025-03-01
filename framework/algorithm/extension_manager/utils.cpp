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

#include "utils.h"
#include <algorithm>
#include <iterator>
#include "vpe_log.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
namespace Extension {

void DoRegisterExtensions(uintptr_t addr, Utils::RegisterExtensionFunc func)
{
    CHECK_AND_RETURN_LOG(func, "Register function is invalid");
    auto extensionList = reinterpret_cast<ExtensionList *>(addr);
    CHECK_AND_RETURN_LOG(extensionList != nullptr, "Extensionlist is nullptr");
    auto extensions = func();
    std::copy(extensions.begin(), extensions.end(), std::back_inserter(*extensionList));

    VPE_LOGD("%{public}zu extensions loaded", extensions.size());
    return;
}
} // namespace Extension
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
