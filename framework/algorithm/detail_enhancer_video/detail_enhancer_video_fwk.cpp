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

#include "detail_enhancer_video_fwk.h"

#include <functional>

#include "vpe_log.h"
#include "vpe_trace.h"

using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;
using namespace std::chrono_literals;

namespace {
constexpr int MAX_TARGET_WIDTH = 2000;
constexpr int MAX_TARGET_HEIGHT = 2000;
constexpr int MIN_SOURCE_WIDTH = 320;
constexpr int MIN_SOURCE_HEIGHT = 320;
} // namespace

std::shared_ptr<VpeVideoImpl> DetailEnhancerVideoFwk::Create()
{
    return CreateEx(true);
}

std::shared_ptr<VpeVideoImpl> DetailEnhancerVideoFwk::CreateEx(bool disable)
{
    auto obj = std::make_shared<DetailEnhancerVideoFwk>(VIDEO_TYPE_DETAIL_ENHANCER, disable);
    CHECK_AND_RETURN_RET_LOG(obj != nullptr, nullptr, "Failed to create detail enhancer!");
    CHECK_AND_RETURN_RET_LOG(obj->Initialize() == VPE_ALGO_ERR_OK, nullptr, "Failed to initialize detail enhancer!");
    return obj;
}

VPEAlgoErrCode DetailEnhancerVideoFwk::SetParameter(const Format& parameter)
{
    std::function<ParamError(const Format&)> setters[] = {
        [this] (const Format& parameter) { return SetLevel(parameter); },
        [this] (const Format& parameter) { return SetTargetSize(parameter); },
        [this] (const Format& parameter) { return SetAutoDownshift(parameter); },
    };

    CHECK_AND_RETURN_RET_LOG(IsInitialized(), VPE_ALGO_ERR_INVALID_OPERATION, "NOT initialized!");

    std::lock_guard<std::mutex> lock(lock_);
    int setCount = 0;
    for (auto& setter : setters) {
        int err = setter(parameter);
        if (err == PARAM_ERR_INVALID) {
            return VPE_ALGO_ERR_INVALID_VAL;
        }
        setCount += err;
    }
    CHECK_AND_RETURN_RET_LOG(setCount > 0, VPE_ALGO_ERR_INVALID_VAL, "Invalid input: NO valid parameters!");
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode DetailEnhancerVideoFwk::GetParameter([[maybe_unused]] Format& parameter)
{
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode DetailEnhancerVideoFwk::OnInitialize()
{
    detailEnh_ = DetailEnhancerImage::Create(VIDEO);
    CHECK_AND_RETURN_RET_LOG(detailEnh_ != nullptr, VPE_ALGO_ERR_UNKNOWN, "Failed to create DetailEnhancer!");
    return detailEnh_->EnableProtection(isAutoDisable_);
}

VPEAlgoErrCode DetailEnhancerVideoFwk::OnDeinitialize()
{
    detailEnh_ = nullptr;
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode DetailEnhancerVideoFwk::Process(const sptr<SurfaceBuffer>& sourceImage,
    sptr<SurfaceBuffer>& destinationImage)
{
    CHECK_AND_RETURN_RET_LOG(sourceImage != nullptr && destinationImage != nullptr, VPE_ALGO_ERR_INVALID_VAL,
        "Invalid input: source or destination image is null!");
    CHECK_AND_RETURN_RET_LOG(IsInitialized(), VPE_ALGO_ERR_INVALID_OPERATION, "NOT initialized!");
    auto ret = detailEnh_->Process(sourceImage, destinationImage);
    VPE_LOGD("scale %{public}dx%{public}d -> %{public}dx%{public}d ret:%{public}d",
        sourceImage->GetWidth(), sourceImage->GetHeight(),
        destinationImage->GetWidth(), destinationImage->GetHeight(), ret);
    std::lock_guard<std::mutex> lock(lock_);
    if (ret == VPE_ALGO_ERR_OK && (lastEffectiveLevel_ != level_ ||
        lastEffectiveSize_.width != destinationImage->GetWidth() ||
        lastEffectiveSize_.height != destinationImage->GetHeight())) {
        VPE_LOGD("level:%{public}d->%{public}d size:%{public}dx%{public}d->%{public}dx%{public}d",
            lastEffectiveLevel_, level_, lastEffectiveSize_.width, lastEffectiveSize_.height,
            destinationImage->GetWidth(), destinationImage->GetHeight());
        Format fmt;
        lastEffectiveLevel_ = level_;
        fmt.PutIntValue(ParameterKey::DETAIL_ENHANCER_QUALITY_LEVEL, lastEffectiveLevel_);
        lastEffectiveSize_.width = destinationImage->GetWidth();
        lastEffectiveSize_.height = destinationImage->GetHeight();
        fmt.PutBuffer(ParameterKey::DETAIL_ENHANCER_TARGET_SIZE, reinterpret_cast<uint8_t*>(&lastEffectiveSize_),
            sizeof(lastEffectiveSize_));
        OnOutputFormatChanged(fmt);
    }
    return ret;
}

VPEAlgoErrCode DetailEnhancerVideoFwk::ResetAfterDisable()
{
    CHECK_AND_RETURN_RET_LOG(IsInitialized(), VPE_ALGO_ERR_INVALID_OPERATION, "NOT initialized!");
    return detailEnh_->ResetProtectionStatus();
}

bool DetailEnhancerVideoFwk::IsDisableAfterProcessFail()
{
    return isAutoDisable_;
}

bool DetailEnhancerVideoFwk::IsProducerSurfaceValid([[maybe_unused]] const sptr<Surface>& surface)
{
    // Check resolution for detail enhancer is valid or not
    return true;
}

bool DetailEnhancerVideoFwk::IsConsumerBufferValid(const sptr<SurfaceBuffer>& buffer)
{
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, false, "buffer is null!");
    CHECK_AND_RETURN_RET_LOG(buffer->GetWidth() > MIN_SOURCE_WIDTH && buffer->GetHeight() > MIN_SOURCE_HEIGHT, false,
        "Invalid input: %{public}dx%{public}d!", buffer->GetWidth(), buffer->GetHeight());
    return true;
}

VPEAlgoErrCode DetailEnhancerVideoFwk::UpdateRequestCfg(const sptr<Surface>& surface, BufferRequestConfig& requestCfg)
{
    CHECK_AND_RETURN_RET_LOG(surface != nullptr, VPE_ALGO_ERR_INVALID_VAL, "surface is null!");

    // If the target size is set, the target size takes precedence.
    std::lock_guard<std::mutex> lock(lock_);
    if (size_.width != 0 && size_.height != 0) {
        requestCfg.width = size_.width;
        requestCfg.height = size_.height;
    } else {
        // Update buffer request configuration, use default resolution of output surface as the resize destination
        // resolution when the request resolution is invalid.
        requestCfg.width = surface->GetRequestWidth();
        requestCfg.height = surface->GetRequestHeight();
        if (requestCfg.width == 0 || requestCfg.height == 0) {
            requestCfg.width = surface->GetDefaultWidth();
            requestCfg.height = surface->GetDefaultHeight();
        }
    }
    return VPE_ALGO_ERR_OK;
}

void DetailEnhancerVideoFwk::UpdateRequestCfg(const sptr<SurfaceBuffer>& consumerBuffer,
    BufferRequestConfig& requestCfg)
{
    CHECK_AND_RETURN_LOG(consumerBuffer != nullptr, "surface buffer is null!");

    if (size_.width != 0 && size_.height != 0) {
        requestCfg.width = size_.width;
        requestCfg.height = size_.height;
    } else {
        if (requestCfg.width == 0 || requestCfg.height == 0) {
            requestCfg.width = consumerBuffer->GetWidth();
            requestCfg.height = consumerBuffer->GetHeight();
        }
    }
    requestCfg.format = consumerBuffer->GetFormat();
}

DetailEnhancerVideoFwk::ParamError DetailEnhancerVideoFwk::SetLevel(const Format& parameter)
{
    int level;
    if (!parameter.GetIntValue(ParameterKey::DETAIL_ENHANCER_QUALITY_LEVEL, level)) {
        return PARAM_ERR_NOT_FOUND;
    }
    CHECK_AND_RETURN_RET_LOG(level >= DETAIL_ENHANCER_LEVEL_NONE && level <= DETAIL_ENHANCER_LEVEL_HIGH,
        PARAM_ERR_INVALID, "Invalid input: level=%{public}d(Expected:[%{public}d,%{public}d])!",
        level, DETAIL_ENHANCER_LEVEL_NONE, DETAIL_ENHANCER_LEVEL_HIGH);
    VPE_LOGI("level:%{public}d->%{public}d", level_, level);
    level_ = static_cast<DetailEnhancerQualityLevel>(level);
    DetailEnhancerParameters param{};
    param.level = static_cast<DetailEnhancerLevel>(level_);
    CHECK_AND_RETURN_RET_LOG(detailEnh_->SetParameter(param) == VPE_ALGO_ERR_OK, PARAM_ERR_INVALID,
        "Failed to set parameter(level:%{public}d) to detail enhancer!", param.level);
    return PARAM_ERR_OK;
}

DetailEnhancerVideoFwk::ParamError DetailEnhancerVideoFwk::SetTargetSize(const Format& parameter)
{
    size_t addrSize;
    uint8_t* addr = nullptr;
    if (!parameter.GetBuffer(ParameterKey::DETAIL_ENHANCER_TARGET_SIZE, &addr, addrSize)) {
        return PARAM_ERR_NOT_FOUND;
    }
    CHECK_AND_RETURN_RET_LOG(addr != nullptr && addrSize == sizeof(VpeBufferSize), PARAM_ERR_INVALID,
        "Invalid input: addr is null or addrSize=%{public}zu(Expected:%{public}zu)!",
        addrSize, sizeof(VpeBufferSize));
    auto size = reinterpret_cast<VpeBufferSize*>(addr);
    CHECK_AND_RETURN_RET_LOG(size->width > 0 && size->width <= MAX_TARGET_WIDTH &&
        size->height > 0 && size->height <= MAX_TARGET_HEIGHT, PARAM_ERR_INVALID,
        "Invalid input: target buffer size width=%{public}d(Expected:(0,%{public}d])"
        " height=%{public}d(Expected:(0,%{public}d])!",
        size->width, MAX_TARGET_WIDTH, size->height, MAX_TARGET_HEIGHT);
    VPE_LOGI("target size:%{public}dx%{public}d->%{public}dx%{public}d",
        size_.width, size_.height, size->width, size->height);
    size_ = *size;

    RefreshBuffers();
    return PARAM_ERR_OK;
}

DetailEnhancerVideoFwk::ParamError DetailEnhancerVideoFwk::SetAutoDownshift(const Format& parameter)
{
    int autoDownshift;
    if (!parameter.GetIntValue(ParameterKey::DETAIL_ENHANCER_AUTO_DOWNSHIFT, autoDownshift)) {
        return PARAM_ERR_NOT_FOUND;
    }
    CHECK_AND_RETURN_RET_LOG(autoDownshift == 0 || autoDownshift == 1, PARAM_ERR_INVALID,
        "Invalid input: autoDownshift=%{public}d(Expected: 0 or 1)!", autoDownshift);
    VPE_LOGI("auto downshift:%{public}d->%{public}d", isAutoDownshift_, (autoDownshift == 1));
    isAutoDownshift_ = (autoDownshift == 1);
    return PARAM_ERR_OK;
}
