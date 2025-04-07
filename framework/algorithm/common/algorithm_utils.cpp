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
#include "securec.h"
#include "vpe_log.h"

using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;

namespace {
const std::unordered_map<GSError, std::string> GSERROR_STR_MAP = {
    { GSERROR_OK,                              VPE_TO_STR(GSERROR_OK) },
    { GSERROR_MEM_OPERATION_ERROR,             VPE_TO_STR(GSERROR_MEM_OPERATION_ERROR) },
    { GSERROR_INVALID_ARGUMENTS,               VPE_TO_STR(GSERROR_INVALID_ARGUMENTS) },
    { GSERROR_NO_PERMISSION,                   VPE_TO_STR(GSERROR_NO_PERMISSION) },
    { GSERROR_CONNOT_CONNECT_SAMGR,            VPE_TO_STR(GSERROR_CONNOT_CONNECT_SAMGR) },
    { GSERROR_CONNOT_CONNECT_SERVER,           VPE_TO_STR(GSERROR_CONNOT_CONNECT_SERVER) },
    { GSERROR_CONNOT_CONNECT_WESTON,           VPE_TO_STR(GSERROR_CONNOT_CONNECT_WESTON) },
    { GSERROR_NO_BUFFER,                       VPE_TO_STR(GSERROR_NO_BUFFER) },
    { GSERROR_NO_ENTRY,                        VPE_TO_STR(GSERROR_NO_ENTRY) },
    { GSERROR_OUT_OF_RANGE,                    VPE_TO_STR(GSERROR_OUT_OF_RANGE) },
    { GSERROR_NO_SCREEN,                       VPE_TO_STR(GSERROR_NO_SCREEN) },
    { GSERROR_NO_BUFFER_READY,                 VPE_TO_STR(GSERROR_NO_BUFFER_READY) },
    { GSERROR_INVALID_OPERATING,               VPE_TO_STR(GSERROR_INVALID_OPERATING) },
    { GSERROR_NO_CONSUMER,                     VPE_TO_STR(GSERROR_NO_CONSUMER) },
    { GSERROR_NOT_INIT,                        VPE_TO_STR(GSERROR_NOT_INIT) },
    { GSERROR_TYPE_ERROR,                      VPE_TO_STR(GSERROR_TYPE_ERROR) },
    { GSERROR_DESTROYED_OBJECT,                VPE_TO_STR(GSERROR_DESTROYED_OBJECT) },
    { GSERROR_CONSUMER_IS_CONNECTED,           VPE_TO_STR(GSERROR_CONSUMER_IS_CONNECTED) },
    { GSERROR_BUFFER_STATE_INVALID,            VPE_TO_STR(GSERROR_BUFFER_STATE_INVALID) },
    { GSERROR_BUFFER_IS_INCACHE,               VPE_TO_STR(GSERROR_BUFFER_IS_INCACHE) },
    { GSERROR_BUFFER_QUEUE_FULL,               VPE_TO_STR(GSERROR_BUFFER_QUEUE_FULL) },
    { GSERROR_BUFFER_NOT_INCACHE,              VPE_TO_STR(GSERROR_BUFFER_NOT_INCACHE) },
    { GSERROR_CONSUMER_DISCONNECTED,           VPE_TO_STR(GSERROR_CONSUMER_DISCONNECTED) },
    { GSERROR_CONSUMER_UNREGISTER_LISTENER,    VPE_TO_STR(GSERROR_CONSUMER_UNREGISTER_LISTENER) },
    { GSERROR_API_FAILED,                      VPE_TO_STR(GSERROR_API_FAILED) },
    { GSERROR_INTERNAL,                        VPE_TO_STR(GSERROR_INTERNAL) },
    { GSERROR_NO_MEM,                          VPE_TO_STR(GSERROR_NO_MEM) },
    { GSERROR_PROXY_NOT_INCLUDE,               VPE_TO_STR(GSERROR_PROXY_NOT_INCLUDE) },
    { GSERROR_SERVER_ERROR,                    VPE_TO_STR(GSERROR_SERVER_ERROR) },
    { GSERROR_ANIMATION_RUNNING,               VPE_TO_STR(GSERROR_ANIMATION_RUNNING) },
    { GSERROR_HDI_ERROR,                       VPE_TO_STR(GSERROR_HDI_ERROR) },
    { GSERROR_NOT_IMPLEMENT,                   VPE_TO_STR(GSERROR_NOT_IMPLEMENT) },
    { GSERROR_NOT_SUPPORT,                     VPE_TO_STR(GSERROR_NOT_SUPPORT) },
    { GSERROR_BINDER,                          VPE_TO_STR(GSERROR_BINDER) },
    { GSERROR_EGL_STATE_UNKOWN,                VPE_TO_STR(GSERROR_EGL_STATE_UNKOWN) },
    { GSERROR_EGL_API_FAILED,                  VPE_TO_STR(GSERROR_EGL_API_FAILED) },
};
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

template <typename T>
std::string CodeToString(T code, const std::unordered_map<T, std::string>& codeMap, const std::string& name)
{
    auto it = codeMap.find(code);
    if (it == codeMap.end()) [[unlikely]] {
        return "unknown " + name + "(" + std::to_string(static_cast<int>(code)) + ")";
    }
    return it->second;
}
} // namespace

std::string AlgorithmUtils::ToString(GSError errorCode)
{
    return CodeToString(errorCode, GSERROR_STR_MAP, "GSError");
}

std::string AlgorithmUtils::ToString(VPEAlgoErrCode errorCode)
{
    return CodeToString(errorCode, ERROR_STR_MAP, "VPEAlgoErrCode");
}

std::string AlgorithmUtils::ToString(VPEAlgoState state)
{
    return CodeToString(state, STATE_STR_MAP, "VPEAlgoState");
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