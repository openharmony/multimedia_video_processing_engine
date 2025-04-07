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

#ifndef FRMEWORK_ALGORITHM_EXTENSION_MANAGER_EXTENSION_BASE_H
#define FRMEWORK_ALGORITHM_EXTENSION_MANAGER_EXTENSION_BASE_H

#include <string>

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
namespace Extension {
enum class ExtensionType {
    COLORSPACE_CONVERTER,
    COLORSPACE_CONVERTER_DISPLAY,
    METADATA_GENERATOR,
    DETAIL_ENHANCER,
    VIDEO_REFRESHRATE_PREDICTION,
    AIHDR_ENHANCER,
    CONTRAST_ENHANCER,
};

enum class Rank : uint32_t {
    RANK_DEFAULT = 0,
    RANK_HIGH,
};

struct ExtensionInfo {
    ExtensionType type {ExtensionType::COLORSPACE_CONVERTER};
    std::string name;
    std::string version;
};

struct ExtensionBase {
    virtual ~ExtensionBase() = default;
    ExtensionInfo info;
};
} // namespace Extension
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // FRMEWORK_ALGORITHM_EXTENSION_MANAGER_EXTENSION_BASE_H
