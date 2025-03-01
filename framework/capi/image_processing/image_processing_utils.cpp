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

#include "image_processing_utils.h"

#include <unordered_map>

#include "algorithm_utils.h"
#include "surface_buffer_impl.h"
#include "surface_type.h"
#include "vpe_log.h"
#include "vpe_utils_common.h"

using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;

namespace {
const std::unordered_map<VPEAlgoErrCode, ImageProcessing_ErrorCode> ERROR_MAP = {
    { VPE_ALGO_ERR_OK,                          IMAGE_PROCESSING_SUCCESS },
    { VPE_ALGO_ERR_NO_MEMORY,                   IMAGE_PROCESSING_ERROR_NO_MEMORY },
    { VPE_ALGO_ERR_INVALID_OPERATION,           IMAGE_PROCESSING_ERROR_OPERATION_NOT_PERMITTED },
    { VPE_ALGO_ERR_INVALID_VAL,                 IMAGE_PROCESSING_ERROR_INVALID_VALUE },
    { VPE_ALGO_ERR_UNKNOWN,                     IMAGE_PROCESSING_ERROR_UNKNOWN },
    { VPE_ALGO_ERR_INVALID_PARAM,               IMAGE_PROCESSING_ERROR_INVALID_PARAMETER },
    { VPE_ALGO_ERR_INIT_FAILED,                 IMAGE_PROCESSING_ERROR_INITIALIZE_FAILED },
    { VPE_ALGO_ERR_EXTENSION_NOT_FOUND,         IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING },
    { VPE_ALGO_ERR_EXTENSION_INIT_FAILED,       IMAGE_PROCESSING_ERROR_INITIALIZE_FAILED },
    { VPE_ALGO_ERR_EXTENSION_PROCESS_FAILED,    IMAGE_PROCESSING_ERROR_PROCESS_FAILED },
    { VPE_ALGO_ERR_NOT_IMPLEMENTED,             IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING },
    { VPE_ALGO_ERR_OPERATION_NOT_SUPPORTED,     IMAGE_PROCESSING_ERROR_OPERATION_NOT_PERMITTED },
    { VPE_ALGO_ERR_INVALID_STATE,               IMAGE_PROCESSING_ERROR_OPERATION_NOT_PERMITTED },
    { VPE_ALGO_ERR_EXTEND_START,                IMAGE_PROCESSING_ERROR_OPERATION_NOT_PERMITTED },
};
const std::unordered_map<ImageProcessing_ErrorCode, std::string> CAPI_ERROR_STR_MAP = {
    { IMAGE_PROCESSING_SUCCESS,                         VPE_TO_STR(IMAGE_PROCESSING_SUCCESS) },
    { IMAGE_PROCESSING_ERROR_INVALID_PARAMETER,         VPE_TO_STR(IMAGE_PROCESSING_ERROR_INVALID_PARAMETER) },
    { IMAGE_PROCESSING_ERROR_UNKNOWN,                   VPE_TO_STR(IMAGE_PROCESSING_ERROR_UNKNOWN) },
    { IMAGE_PROCESSING_ERROR_INITIALIZE_FAILED,         VPE_TO_STR(IMAGE_PROCESSING_ERROR_INITIALIZE_FAILED) },
    { IMAGE_PROCESSING_ERROR_CREATE_FAILED,             VPE_TO_STR(IMAGE_PROCESSING_ERROR_CREATE_FAILED) },
    { IMAGE_PROCESSING_ERROR_PROCESS_FAILED,            VPE_TO_STR(IMAGE_PROCESSING_ERROR_PROCESS_FAILED) },
    { IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING,    VPE_TO_STR(IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING) },
    { IMAGE_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,   VPE_TO_STR(IMAGE_PROCESSING_ERROR_OPERATION_NOT_PERMITTED) },
    { IMAGE_PROCESSING_ERROR_NO_MEMORY,                 VPE_TO_STR(IMAGE_PROCESSING_ERROR_NO_MEMORY) },
    { IMAGE_PROCESSING_ERROR_INVALID_INSTANCE,          VPE_TO_STR(IMAGE_PROCESSING_ERROR_INVALID_INSTANCE) },
    { IMAGE_PROCESSING_ERROR_INVALID_VALUE,             VPE_TO_STR(IMAGE_PROCESSING_ERROR_INVALID_VALUE) }
};
}

ImageProcessing_ErrorCode ImageProcessingUtils::InnerErrorToCAPI(VPEAlgoErrCode errorCode)
{
    auto it = ERROR_MAP.find(errorCode);
    if (it == ERROR_MAP.end()) [[unlikely]] {
        VPE_LOGE("Invalid error code:%{public}d", errorCode);
        return IMAGE_PROCESSING_ERROR_UNKNOWN;
    }
    return it->second;
}

std::string ImageProcessingUtils::ToString(ImageProcessing_ErrorCode errorCode)
{
    auto it = CAPI_ERROR_STR_MAP.find(errorCode);
    if (it == CAPI_ERROR_STR_MAP.end()) [[unlikely]] {
        VPE_LOGE("Invalid error code:%{public}d", errorCode);
        return "Unsupported error:" + std::to_string(static_cast<int>(errorCode));
    }
    return it->second;
}

sptr<SurfaceBuffer> ImageProcessingUtils::GetSurfaceBufferFromPixelMap(
    const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap)
{
    return VpeUtils::GetSurfaceBufferFromPixelMap(pixelmap);
}

ImageProcessing_ErrorCode ImageProcessingUtils::ConvertPixelmapToSurfaceBuffer(
    const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap, sptr<SurfaceBuffer> bufferImpl)
{
    return VpeUtils::ConvertPixelmapToSurfaceBuffer(pixelmap, bufferImpl) ?
        IMAGE_PROCESSING_SUCCESS : IMAGE_PROCESSING_ERROR_PROCESS_FAILED;
}

ImageProcessing_ErrorCode ImageProcessingUtils::ConvertSurfaceBufferToPixelmap(const sptr<SurfaceBuffer>& buffer,
    std::shared_ptr<OHOS::Media::PixelMap>& pixelmap)
{
    return VpeUtils::ConvertSurfaceBufferToPixelmap(buffer, pixelmap) ?
        IMAGE_PROCESSING_SUCCESS : IMAGE_PROCESSING_ERROR_PROCESS_FAILED;
}

ImageProcessing_ErrorCode ImageProcessingUtils::SetSurfaceBufferToPixelMap(const sptr<SurfaceBuffer>& buffer,
    std::shared_ptr<OHOS::Media::PixelMap>& pixelmap)
{
    return VpeUtils::SetSurfaceBufferToPixelMap(buffer, pixelmap) ?
        IMAGE_PROCESSING_SUCCESS : IMAGE_PROCESSING_ERROR_PROCESS_FAILED;
}
