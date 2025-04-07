/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "detail_enhancer_image_native.h"

#include <unordered_map>

#include "detail_enhancer_common.h"
#include "detail_enhancer_image_fwk.h"
#include "image_processing_utils.h"
#include "surface_buffer.h"
#include "vpe_log.h"

using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;

namespace {
const std::unordered_map<int, int> CAPI_TO_INNER_LEVEL_MAP = {
    { IMAGE_DETAIL_ENHANCER_QUALITY_LEVEL_NONE,     DETAIL_ENH_LEVEL_NONE },
    { IMAGE_DETAIL_ENHANCER_QUALITY_LEVEL_LOW,      DETAIL_ENH_LEVEL_LOW },
    { IMAGE_DETAIL_ENHANCER_QUALITY_LEVEL_MEDIUM,   DETAIL_ENH_LEVEL_MEDIUM },
    { IMAGE_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH,     DETAIL_ENH_LEVEL_HIGH },
};
const std::unordered_map<int, int> INNER_TO_CAPI_LEVEL_MAP = {
    { DETAIL_ENH_LEVEL_NONE,    IMAGE_DETAIL_ENHANCER_QUALITY_LEVEL_NONE },
    { DETAIL_ENH_LEVEL_LOW,     IMAGE_DETAIL_ENHANCER_QUALITY_LEVEL_LOW },
    { DETAIL_ENH_LEVEL_MEDIUM,  IMAGE_DETAIL_ENHANCER_QUALITY_LEVEL_MEDIUM },
    { DETAIL_ENH_LEVEL_HIGH,    IMAGE_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH },
};
}

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

ImageProcessing_ErrorCode DetailEnhancerImageNative::InitializeInner()
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(!isInitialized_.load(), IMAGE_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already init!");
    detailEnhancer_ = DetailEnhancerImage::Create();
    CHECK_AND_RETURN_RET_LOG(detailEnhancer_ != nullptr, IMAGE_PROCESSING_ERROR_CREATE_FAILED,
        "Create detail enhancement failed!");
    isInitialized_ = true;
    return IMAGE_PROCESSING_SUCCESS;
}

ImageProcessing_ErrorCode DetailEnhancerImageNative::DeinitializeInner()
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), IMAGE_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already deinit!");
    detailEnhancer_ = nullptr;
    isInitialized_ = false;
    return IMAGE_PROCESSING_SUCCESS;
}

ImageProcessing_ErrorCode DetailEnhancerImageNative::SetParameter(const OHOS::Media::Format& parameter)
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), IMAGE_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Detail enhancer image is not initialized!");
    int level;
    CHECK_AND_RETURN_RET_LOG(parameter.GetIntValue(IMAGE_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL, level),
        IMAGE_PROCESSING_ERROR_INVALID_PARAMETER, "No quality level!");
    int innerLevel = LevelTransfer(level, CAPI_TO_INNER_LEVEL_MAP);
    CHECK_AND_RETURN_RET_LOG(innerLevel != -1, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER, "Quality level is invalid!");
    DetailEnhancerParameters param{};
    param.level = static_cast<DetailEnhancerLevel>(innerLevel);
    return ImageProcessingUtils::InnerErrorToCAPI(detailEnhancer_->SetParameter(param));
}

ImageProcessing_ErrorCode DetailEnhancerImageNative::GetParameter(OHOS::Media::Format& parameter)
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), IMAGE_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Detail enhancer image is not initialized!");
    DetailEnhancerParameters param {
        .uri = "",
        .level{},
    };
    auto ret = detailEnhancer_->GetParameter(param);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, ImageProcessingUtils::InnerErrorToCAPI(ret), "param is not set");
    int level = LevelTransfer(param.level, INNER_TO_CAPI_LEVEL_MAP);
    CHECK_AND_RETURN_RET_LOG(level != -1, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER, "Quality level is invalid!");
    parameter.PutIntValue(IMAGE_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL, level);
    return IMAGE_PROCESSING_SUCCESS;
}

ImageProcessing_ErrorCode DetailEnhancerImageNative::Process(const std::shared_ptr<OHOS::Media::PixelMap>& sourceImage,
    std::shared_ptr<OHOS::Media::PixelMap>& destinationImage)
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), IMAGE_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Detail enhancer image is not initialized!");
    CHECK_AND_RETURN_RET_LOG(sourceImage != nullptr && destinationImage != nullptr,
        IMAGE_PROCESSING_ERROR_INVALID_PARAMETER, "sourceImage or destinationImage is null!");
    auto sourceImageSurfaceBuffer = ImageProcessingUtils::GetSurfaceBufferFromPixelMap(sourceImage);
    CHECK_AND_RETURN_RET_LOG(sourceImageSurfaceBuffer != nullptr, IMAGE_PROCESSING_ERROR_PROCESS_FAILED,
        "sourceImageSurfaceBuffer create failed!");
    auto destinationImageSurfaceBuffer = ImageProcessingUtils::GetSurfaceBufferFromPixelMap(destinationImage);
    CHECK_AND_RETURN_RET_LOG(destinationImageSurfaceBuffer != nullptr, IMAGE_PROCESSING_ERROR_PROCESS_FAILED,
        "destinationImageSurfaceBuffer create failed!");
    auto ret = CheckParameter();
    CHECK_AND_RETURN_RET_LOG(ret == IMAGE_PROCESSING_SUCCESS, ret, "check parameter failed!");
    ret = ImageProcessingUtils::InnerErrorToCAPI(
        detailEnhancer_->Process(sourceImageSurfaceBuffer, destinationImageSurfaceBuffer));
    CHECK_AND_RETURN_RET_LOG(ret == IMAGE_PROCESSING_SUCCESS, ret, "process failed!");
    ret = ImageProcessingUtils::SetSurfaceBufferToPixelMap(destinationImageSurfaceBuffer, destinationImage);
    CHECK_AND_RETURN_RET_LOG(ret == IMAGE_PROCESSING_SUCCESS, ret, "convert to pixelmap failed!");
    return ret;
}

ImageProcessing_ErrorCode DetailEnhancerImageNative::CheckParameter()
{
    DetailEnhancerParameters param {
        .uri = "",
        .level{},
    };
    if (detailEnhancer_->GetParameter(param) == VPE_ALGO_ERR_OK) {
        return IMAGE_PROCESSING_SUCCESS;
    }
    param.level = static_cast<DetailEnhancerLevel>(IMAGE_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH);
    CHECK_AND_RETURN_RET_LOG(detailEnhancer_->SetParameter(param) == VPE_ALGO_ERR_OK,
        IMAGE_PROCESSING_ERROR_PROCESS_FAILED, "set default enhance level failed!!");
    return IMAGE_PROCESSING_SUCCESS;
}

int DetailEnhancerImageNative::LevelTransfer(int level, const std::unordered_map<int, int> levelMap) const
{
    auto it = levelMap.find(level);
    if (it == levelMap.end()) [[unlikely]] {
        VPE_LOGE("Invalid input level:%{public}d", level);
        return -1;
    }
    return it->second;
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
