/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "algorithm_utils.h"

#include <unordered_map>
#include "vpe_log.h"
using namespace OHOS::Media::VideoProcessingEngine;

namespace {
const std::unordered_map<VPEAlgoErrCode, std::string> ERROR_STR_MAP = {
    { VPE_ALGO_ERR_OK,                          VPE_TO_STR(VPE_ALGO_ERR_OK) },
    { VPE_ALGO_ERR_NO_MEMORY,                   VPE_TO_STR(VPE_ALGO_ERR_NO_MEMORY) },
    { VPE_ALGO_ERR_INVALID_OPERATION,           VPE_TO_STR(VPE_ALGO_ERR_INVALID_OPERATION) },
    { VPE_ALGO_ERR_INVALID_VAL,                 VPE_TO_STR(VPE_ALGO_ERR_INVALID_VAL) },
    { VPE_ALGO_ERR_UNKNOWN,                     VPE_TO_STR(VPE_ALGO_ERR_UNKNOWN) },
    { VPE_ALGO_ERR_INIT_FAILED,                 VPE_TO_STR(VPE_ALGO_ERR_INIT_FAILED) },
    { VPE_ALGO_ERR_EXTENSION_NOT_FOUND,         VPE_TO_STR(VPE_ALGO_ERR_EXTENSION_NOT_FOUND) },
    { VPE_ALGO_ERR_EXTENSION_INIT_FAILED,       VPE_TO_STR(VPE_ALGO_ERR_EXTENSION_INIT_FAILED) },
    { VPE_ALGO_ERR_EXTENSION_PROCESS_FAILED,    VPE_TO_STR(VPE_ALGO_ERR_EXTENSION_PROCESS_FAILED) },
    { VPE_ALGO_ERR_NOT_IMPLEMENTED,             VPE_TO_STR(VPE_ALGO_ERR_NOT_IMPLEMENTED) },
    { VPE_ALGO_ERR_OPERATION_NOT_SUPPORTED,     VPE_TO_STR(VPE_ALGO_ERR_OPERATION_NOT_SUPPORTED) },
    { VPE_ALGO_ERR_INVALID_STATE,               VPE_TO_STR(VPE_ALGO_ERR_INVALID_STATE) },
    { VPE_ALGO_ERR_EXTEND_START,                VPE_TO_STR(VPE_ALGO_ERR_EXTEND_START) },
};
const std::unordered_map<VPEAlgoState, std::string> STATE_STR_MAP = {
    { VPEAlgoState::UNINITIALIZED,  VPE_TO_STR(VPEAlgoState::UNINITIALIZED) },
    { VPEAlgoState::INITIALIZED,    VPE_TO_STR(VPEAlgoState::INITIALIZED) },
    { VPEAlgoState::CONFIGURING,    VPE_TO_STR(VPEAlgoState::CONFIGURING) },
    { VPEAlgoState::CONFIGURED,     VPE_TO_STR(VPEAlgoState::CONFIGURED) },
    { VPEAlgoState::STOPPED,        VPE_TO_STR(VPEAlgoState::STOPPED) },
    { VPEAlgoState::RUNNING,        VPE_TO_STR(VPEAlgoState::RUNNING) },
    { VPEAlgoState::EOS,            VPE_TO_STR(VPEAlgoState::EOS) },
    { VPEAlgoState::ERROR,          VPE_TO_STR(VPEAlgoState::ERROR) },
};
}

std::string AlgorithmUtils::ToString(VPEAlgoErrCode errorCode)
{
    auto it = ERROR_STR_MAP.find(errorCode);
    if (it == ERROR_STR_MAP.end()) [[unlikely]] {
        VPE_LOGE("Invalid error code:%{public}d", errorCode);
        return "Unsupported error:" + std::to_string(static_cast<int>(errorCode));
    }
    return it->second;
}

std::string AlgorithmUtils::ToString(VPEAlgoState state)
{
    auto it = STATE_STR_MAP.find(state);
    if (it == STATE_STR_MAP.end()) [[unlikely]] {
        VPE_LOGE("Invalid state:%{public}d", state);
        return "Unsupported state:" + std::to_string(static_cast<int>(state));
    }
    return it->second;
}

bool AlgorithmUtils::CopySurfaceBufferToSurfaceBuffer(const sptr<SurfaceBuffer>& srcBuffer,
    sptr<SurfaceBuffer>& destBuffer)
{
    CHECK_AND_RETURN_RET_LOG(srcBuffer != nullptr && destBuffer != nullptr, false,
        "srcBuffer or destBuffer is nullptr");
    CHECK_AND_RETURN_RET_LOG(srcBuffer->GetFormat() == destBuffer->GetFormat(), false, "buffer format is not same."\
        "input format:%{public}d,output format:%{public}d", srcBuffer->GetFormat(), destBuffer->GetFormat());
    CHECK_AND_RETURN_RET_LOG((srcBuffer->GetStride() == destBuffer->GetStride()) &&
        (srcBuffer->GetHeight() == destBuffer->GetHeight()) && (srcBuffer->GetWidth() == destBuffer->GetWidth()),
        false, "buffer stride and height is not same. input height:%{public}d,output height:%{public}d,"\
        "input width:%{public}d,output width:%{public}d,input stride:%{public}d,output stride:%{public}d",
        srcBuffer->GetHeight(), destBuffer->GetHeight(), srcBuffer->GetWidth(), destBuffer->GetWidth(),
        srcBuffer->GetStride(), destBuffer->GetStride());
    CHECK_AND_RETURN_RET_LOG(srcBuffer->GetSize() == destBuffer->GetSize(), false, "buffer size is not same."\
        "input size:%{public}u,output size:%{public}u", srcBuffer->GetSize(), destBuffer->GetSize());
    if (memcpy_s(static_cast<uint8_t*>(destBuffer->GetVirAddr()), destBuffer->GetSize(),
        static_cast<uint8_t*>(srcBuffer->GetVirAddr()), srcBuffer->GetSize()) != EOK) {
        VPE_LOGE("Fail to copy surfaceBuffer to surfaceBuffer!");
        return false;
    }
    std::vector<uint8_t> attrInfo{};
    std::vector<uint32_t> keys{};
    GSError ret;
    if (srcBuffer->ListMetadataKeys(keys) == GSERROR_OK && !keys.empty()) {
        for (size_t i = 0; i < keys.size(); i++) {
            if (srcBuffer->GetMetadata(keys[i], attrInfo) == GSERROR_OK && !attrInfo.empty()) {
                ret = destBuffer->SetMetadata(keys[i], attrInfo);
                CHECK_AND_RETURN_RET_LOG(ret == GSERROR_OK, false, "Fail to set metadata.");
            }
            attrInfo.clear();
        }
    }
    return true;
}