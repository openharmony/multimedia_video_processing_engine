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

#include "video_processing_utils.h"

#include <unordered_map>

#include "algorithm_utils.h"
#include "vpe_log.h"

using namespace OHOS::Media::VideoProcessingEngine;

namespace {
const std::unordered_map<VPEAlgoErrCode, VideoProcessing_ErrorCode> ERROR_MAP = {
    { VPE_ALGO_ERR_OK,                          VIDEO_PROCESSING_SUCCESS },
    { VPE_ALGO_ERR_NO_MEMORY,                   VIDEO_PROCESSING_ERROR_NO_MEMORY },
    { VPE_ALGO_ERR_INVALID_OPERATION,           VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED },
    { VPE_ALGO_ERR_INVALID_VAL,                 VIDEO_PROCESSING_ERROR_INVALID_VALUE },
    { VPE_ALGO_ERR_UNKNOWN,                     VIDEO_PROCESSING_ERROR_UNKNOWN },
    { VPE_ALGO_ERR_INVALID_PARAM,               VIDEO_PROCESSING_ERROR_INVALID_PARAMETER },
    { VPE_ALGO_ERR_INIT_FAILED,                 VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED },
    { VPE_ALGO_ERR_EXTENSION_NOT_FOUND,         VIDEO_PROCESSING_ERROR_UNSUPPORTED_PROCESSING },
    { VPE_ALGO_ERR_EXTENSION_INIT_FAILED,       VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED },
    { VPE_ALGO_ERR_EXTENSION_PROCESS_FAILED,    VIDEO_PROCESSING_ERROR_PROCESS_FAILED },
    { VPE_ALGO_ERR_NOT_IMPLEMENTED,             VIDEO_PROCESSING_ERROR_UNSUPPORTED_PROCESSING },
    { VPE_ALGO_ERR_OPERATION_NOT_SUPPORTED,     VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED },
    { VPE_ALGO_ERR_INVALID_STATE,               VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED },
    { VPE_ALGO_ERR_EXTEND_START,                VIDEO_PROCESSING_ERROR_UNKNOWN },
};
const std::unordered_map<VPEAlgoState, VideoProcessing_State> STATE_MAP = {
    { VPEAlgoState::UNINITIALIZED,  VIDEO_PROCESSING_STATE_STOPPED },
    { VPEAlgoState::INITIALIZED,    VIDEO_PROCESSING_STATE_STOPPED },
    { VPEAlgoState::CONFIGURING,    VIDEO_PROCESSING_STATE_STOPPED },
    { VPEAlgoState::CONFIGURED,     VIDEO_PROCESSING_STATE_STOPPED },
    { VPEAlgoState::STOPPED,        VIDEO_PROCESSING_STATE_STOPPED },
    { VPEAlgoState::RUNNING,        VIDEO_PROCESSING_STATE_RUNNING },
    { VPEAlgoState::EOS,            VIDEO_PROCESSING_STATE_RUNNING },
    { VPEAlgoState::ERROR,          VIDEO_PROCESSING_STATE_STOPPED },
};
const std::unordered_map<VideoProcessing_ErrorCode, std::string> NDK_ERROR_STR_MAP = {
    { VIDEO_PROCESSING_SUCCESS,                         VPE_TO_STR(VIDEO_PROCESSING_SUCCESS) },
    { VIDEO_PROCESSING_ERROR_INVALID_PARAMETER,         VPE_TO_STR(VIDEO_PROCESSING_ERROR_INVALID_PARAMETER) },
    { VIDEO_PROCESSING_ERROR_UNKNOWN,                   VPE_TO_STR(VIDEO_PROCESSING_ERROR_UNKNOWN) },
    { VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,         VPE_TO_STR(VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED) },
    { VIDEO_PROCESSING_ERROR_CREATE_FAILED,             VPE_TO_STR(VIDEO_PROCESSING_ERROR_CREATE_FAILED) },
    { VIDEO_PROCESSING_ERROR_PROCESS_FAILED,            VPE_TO_STR(VIDEO_PROCESSING_ERROR_PROCESS_FAILED) },
    { VIDEO_PROCESSING_ERROR_UNSUPPORTED_PROCESSING,    VPE_TO_STR(VIDEO_PROCESSING_ERROR_UNSUPPORTED_PROCESSING) },
    { VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,   VPE_TO_STR(VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED) },
    { VIDEO_PROCESSING_ERROR_NO_MEMORY,                 VPE_TO_STR(VIDEO_PROCESSING_ERROR_NO_MEMORY) },
    { VIDEO_PROCESSING_ERROR_INVALID_INSTANCE,          VPE_TO_STR(VIDEO_PROCESSING_ERROR_INVALID_INSTANCE) },
    { VIDEO_PROCESSING_ERROR_INVALID_VALUE,             VPE_TO_STR(VIDEO_PROCESSING_ERROR_INVALID_VALUE) },
};
const std::unordered_map<VideoProcessing_State, std::string> NDK_STATE_STR_MAP = {
    { VIDEO_PROCESSING_STATE_RUNNING, VPE_TO_STR(VIDEO_PROCESSING_STATE_RUNNING) },
    { VIDEO_PROCESSING_STATE_STOPPED, VPE_TO_STR(VIDEO_PROCESSING_STATE_STOPPED) },
};
}

VideoProcessing_ErrorCode VideoProcessingUtils::InnerErrorToNDK(VPEAlgoErrCode errorCode)
{
    auto it = ERROR_MAP.find(errorCode);
    if (it == ERROR_MAP.end()) [[unlikely]] {
        VPE_LOGE("Invalid error code:%{public}d", errorCode);
        return VIDEO_PROCESSING_ERROR_UNKNOWN;
    }
    return it->second;
}

VideoProcessing_State VideoProcessingUtils::InnerStateToNDK(VPEAlgoState state)
{
    auto it = STATE_MAP.find(state);
    if (it == STATE_MAP.end()) [[unlikely]] {
        VPE_LOGE("Invalid state:%{public}d", state);
        return VIDEO_PROCESSING_STATE_STOPPED;
    }
    return it->second;
}

std::string VideoProcessingUtils::ToString(VideoProcessing_ErrorCode errorCode)
{
    auto it = NDK_ERROR_STR_MAP.find(errorCode);
    if (it == NDK_ERROR_STR_MAP.end()) [[unlikely]] {
        VPE_LOGE("Invalid error code:%{public}d", errorCode);
        return "Unsupported error:" + std::to_string(static_cast<int>(errorCode));
    }
    return it->second;
}

std::string VideoProcessingUtils::ToString(VideoProcessing_State state)
{
    auto it = NDK_STATE_STR_MAP.find(state);
    if (it == NDK_STATE_STR_MAP.end()) [[unlikely]] {
        VPE_LOGE("Invalid state:%{public}d", state);
        return "Unsupported state:" + std::to_string(static_cast<int>(state));
    }
    return it->second;
}
