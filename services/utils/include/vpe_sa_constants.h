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

#ifndef VPE_SA_CONSTANTS_H
#define VPE_SA_CONSTANTS_H

#include <cstdint>

#include "algorithm_errors.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
constexpr int32_t VIDEO_PROCESSING_SERVER_SA_ID = 0x00010256;
enum VPEAlgoErrExCode : ErrCode {
    VPE_ALGO_ERR_INVALID_CLIENT_ID = VPE_ALGO_ERR_EXTEND_START,
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // VPE_SA_CONSTANTS_H
