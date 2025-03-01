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

#ifndef INTERFACES_INNER_API_ALGORITHM_ALGORITHM_ERRORS_H
#define INTERFACES_INNER_API_ALGORITHM_ALGORITHM_ERRORS_H

#include <map>
#include <string>
#include "errors.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
// bit 28~21 is subsys, bit 20~16 is Module. bit 15~0 is code
constexpr ErrCode VPE_ALGO_MODULE = 11;
constexpr ErrCode VPE_ALGO_ERR_OFFSET = ErrCodeOffset(SUBSYS_MULTIMEDIA, VPE_ALGO_MODULE);
typedef enum VPEAlgoErrCode : ErrCode {
    VPE_ALGO_ERR_OK = ERR_OK,
    VPE_ALGO_ERR_NO_MEMORY = VPE_ALGO_ERR_OFFSET + ENOMEM,         // no memory
    VPE_ALGO_ERR_INVALID_OPERATION = VPE_ALGO_ERR_OFFSET + ENOSYS, // opertation not be permitted
    VPE_ALGO_ERR_INVALID_VAL = VPE_ALGO_ERR_OFFSET + EINVAL,       // invalid argument
    VPE_ALGO_ERR_UNKNOWN = VPE_ALGO_ERR_OFFSET + 0x200,            // unkown error.
    VPE_ALGO_ERR_INIT_FAILED,                                      // video processing engine init failed
    VPE_ALGO_ERR_EXTENSION_NOT_FOUND,                              // extension not found
    VPE_ALGO_ERR_EXTENSION_INIT_FAILED,                            // extension init failed
    VPE_ALGO_ERR_EXTENSION_PROCESS_FAILED,                         // extension process failed
    VPE_ALGO_ERR_NOT_IMPLEMENTED,                                  // extension is not implemented
    VPE_ALGO_ERR_OPERATION_NOT_SUPPORTED,                          // not supported operation
    VPE_ALGO_ERR_INVALID_STATE,                                    // the state no support this operation
    VPE_ALGO_ERR_INVALID_PARAM,                                    // invalid parameter.

    VPE_ALGO_ERR_EXTEND_START = VPE_ALGO_ERR_OFFSET + 0xF000, // extend err start.
} VPEAlgoErrCode;
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
#endif // INTERFACES_INNER_API_ALGORITHM_ALGORITHM_ERRORS_H
