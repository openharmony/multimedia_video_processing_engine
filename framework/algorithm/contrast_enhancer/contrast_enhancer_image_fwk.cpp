/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "contrast_enhancer_image_fwk.h"

#include <dlfcn.h>

#include "contrast_enhancer_common.h"
#include "extension_manager.h"
#include "native_buffer.h"
#include "surface_buffer.h"
#include "video_processing_client.h"
#include "vpe_log.h"
#include "vpe_trace.h"

#include "frame_info.h"
#include "v2_1/buffer_handle_meta_key_type.h"

namespace {
const int MAX_URL_LENGTH = 100;
const int SUPPORTED_MIN_WIDTH = 720; // consistent with napi
const int SUPPORTED_MIN_HEIGHT = 720; // consistent with napi
const int SUPPORTED_MAX_WIDTH = 20000; // consistent with napi
const int SUPPORTED_MAX_HEIGHT = 20000; // consistent with napi
const std::unordered_set<int32_t> SUPPORTED_FORMATS = {
    OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102, // RGBA_1010102
    OHOS::GRAPHIC_PIXEL_FMT_YCBCR_P010,
};
constexpr int MAX_FAILURE_NUM = 5;
}

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
ContrastEnhancerImageFwk::ContrastEnhancerImageFwk()
{
    Extension::ExtensionManager::GetInstance().IncreaseInstance();
}

ContrastEnhancerImageFwk::~ContrastEnhancerImageFwk()
{
    {
        std::lock_guard<std::mutex> lock(lock_);
        algorithms_.clear();
    }
    Extension::ExtensionManager::GetInstance().DecreaseInstance();
}

std::shared_ptr<ContrastEnhancerImage> ContrastEnhancerImage::Create()
{
    std::shared_ptr<ContrastEnhancerImage> impl = std::make_shared<ContrastEnhancerImageFwk>();
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr, "failed to init ContrastEnhancerImage");
    return impl;
}

std::shared_ptr<ContrastEnhancerBase> ContrastEnhancerImageFwk::GetAlgorithm(ContrastEnhancerType level)
{
    if (!getAlgoLock_.try_lock()) {
        VPE_LOGI("get algo lock failed");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(lock_);
    auto createdImpl = algorithms_.find(level);
    if (createdImpl != algorithms_.end()) [[likely]] {
        getAlgoLock_.unlock();
        return createdImpl->second;
    }
    if (failureCount_.load() > MAX_FAILURE_NUM) {
        VPE_LOGW("has failed many times");
        getAlgoLock_.unlock();
        return nullptr;
    }
    std::shared_ptr<ContrastEnhancerBase> algo = CreateAlgorithm(level);
    if (algo.get() == nullptr) {
        failureCount_++;
        VPE_LOGE("create algo failed");
        getAlgoLock_.unlock();
        return nullptr;
    }
    algorithms_[level] = algo;
    getAlgoLock_.unlock();
    return algorithms_[level];
}

std::shared_ptr<ContrastEnhancerBase> ContrastEnhancerImageFwk::CreateAlgorithm(ContrastEnhancerType type)
{
    auto& manager = Extension::ExtensionManager::GetInstance();
    VPE_SYNC_TRACE;
    std::shared_ptr<ContrastEnhancerBase> algoImpl = manager.CreateContrastEnhancer(type);
    if (algoImpl == nullptr) {
        VPE_LOGE("Extension create failed, get a empty impl, type: %{public}d", type);
        return nullptr;
    }
    if (algoImpl->Init() != VPE_ALGO_ERR_OK) {
        VPE_LOGE("Init failed, extension type: %{public}d", type);
        return nullptr;
    }
    return algoImpl;
}

VPEAlgoErrCode ContrastEnhancerImageFwk::SetParameter(const ContrastEnhancerParameters& parameter)
{
    CHECK_AND_RETURN_RET_LOG(parameter.uri.length() < MAX_URL_LENGTH, VPE_ALGO_ERR_INVALID_VAL, "Invalid parameter");
    std::lock_guard<std::mutex> lock(lock_);
    parameter_ = parameter;
    VPE_LOGI("ContrastEnhancerImageFwk SetParameter Succeed");
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode ContrastEnhancerImageFwk::GetParameter(ContrastEnhancerParameters& parameter) const
{
    std::lock_guard<std::mutex> lock(lock_);
    parameter = parameter_;
    VPE_LOGI("ContrastEnhancerImageFwk GetParameter Succeed");
    return VPE_ALGO_ERR_OK;
}

bool ContrastEnhancerImageFwk::IsValidProcessedObject(const sptr<SurfaceBuffer>& buffer)
{
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, false, "buffer is nullptr!!");
    return SUPPORTED_FORMATS.find(buffer->GetFormat()) != SUPPORTED_FORMATS.end() &&
        buffer->GetWidth() > SUPPORTED_MIN_WIDTH && buffer->GetHeight() > SUPPORTED_MIN_HEIGHT &&
        buffer->GetWidth() <= SUPPORTED_MAX_WIDTH && buffer->GetHeight() <= SUPPORTED_MAX_HEIGHT;
}

VPEAlgoErrCode ContrastEnhancerImageFwk::GetRegionHist(const sptr<SurfaceBuffer>& input)
{
    auto algoImpl = GetAlgorithm(parameter_.type);
    CHECK_AND_RETURN_RET_LOG(algoImpl != nullptr, VPE_ALGO_ERR_UNKNOWN, "set parameter failed!");
    return algoImpl->GetRegionHist(input);
}

VPEAlgoErrCode ContrastEnhancerImageFwk::UpdateMetadataBasedOnHist(OHOS::Rect rect, sptr<SurfaceBuffer> surfaceBuffer,
    std::tuple<int, int, double, double, double, int> pixelmapInfo)
{
    VPETrace vpeTrace("VpeFwk::ContrastProcess");
    CHECK_AND_RETURN_RET_LOG(IsValidProcessedObject(surfaceBuffer), VPE_ALGO_ERR_UNKNOWN, "Invalid input");
    auto algoImpl = GetAlgorithm(parameter_.type);
    CHECK_AND_RETURN_RET_LOG(algoImpl != nullptr, VPE_ALGO_ERR_UNKNOWN, "set parameter failed!");
    return algoImpl->UpdateMetadataBasedOnHist(rect, surfaceBuffer, pixelmapInfo);
}

VPEAlgoErrCode ContrastEnhancerImageFwk::UpdateMetadataBasedOnPixel(OHOS::Rect displayArea, OHOS::Rect curPixelmapArea,
    OHOS::Rect completePixelmapArea, sptr<SurfaceBuffer> surfaceBuffer, float fullRatio)
{
    CHECK_AND_RETURN_RET_LOG(IsValidProcessedObject(surfaceBuffer), VPE_ALGO_ERR_UNKNOWN, "Invalid input");
    auto algoImpl = GetAlgorithm(parameter_.type);
    CHECK_AND_RETURN_RET_LOG(algoImpl != nullptr, VPE_ALGO_ERR_UNKNOWN, "set parameter failed!");
    return algoImpl->UpdateMetadataBasedOnPixel(displayArea, curPixelmapArea, completePixelmapArea,
        surfaceBuffer, fullRatio);
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
