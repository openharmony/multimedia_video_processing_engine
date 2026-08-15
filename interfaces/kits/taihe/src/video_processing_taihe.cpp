/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "video_processing_taihe.h"
#include "algorithm_video.h"
#include "vpe_log.h"
#include "vpe_trace.h"
#include "taihe/runtime.hpp"

using namespace taihe;
using namespace OHOS::Media::VideoProcessingEngine;
using namespace ANI;
using namespace ANI::Vp;

namespace ANI::Vp {
std::vector<taihe::callback<void(const taiheVp::VideoProcessorStatus&)>> VideoProcessorImpl::g_statusCallbacks;
std::mutex VideoProcessorImpl::g_statusCallbacksLock;

taiheVp::VideoProcessor createVideoProcessor()
{
    return taihe::make_holder<VideoProcessorImpl, taiheVp::VideoProcessor>();
}
}

bool VideoProcessorImpl::GetCurrentAiHdrEnabled()
{
    OHOS::Media::Format parameter;
    return VpeVideo::IsSupported(VIDEO_TYPE_AIHDR_ENHANCER, parameter);
}

void VideoProcessorImpl::StatusChangeCallback(int32_t feature, int32_t tag, int32_t param)
{
    VPE_LOGI("StatusChangeCallback: feature=%{public}d, param=%{public}d", feature, param);
    bool aiHdrEnabled = (param != 0);
    
    std::lock_guard<std::mutex> lock(g_statusCallbacksLock);
    taiheVp::VideoProcessorAiHdrStatus aiHdrStatus;
    aiHdrStatus.enabled.emplace(aiHdrEnabled);
    taiheVp::VideoProcessorStatus status;
    status.aiHdr.emplace(aiHdrStatus);
    for (auto& callback : g_statusCallbacks) {
        callback(status);
    }
    VPE_LOGI("StatusChangeCallback: notified %{public}zu callbacks, aiHdrEnabled=%{public}d",
        g_statusCallbacks.size(), aiHdrEnabled);
}

void VideoProcessorImpl::TriggerSingleCallback(
    const taihe::callback<void(const taiheVp::VideoProcessorStatus&)>& callback, bool aiHdrEnabled)
{
    taiheVp::VideoProcessorAiHdrStatus aiHdrStatus;
    aiHdrStatus.enabled.emplace(aiHdrEnabled);
    taiheVp::VideoProcessorStatus status;
    status.aiHdr.emplace(aiHdrStatus);
    callback(status);
    VPE_LOGI("TriggerSingleCallback: aiHdrEnabled=%{public}d", aiHdrEnabled);
}

taiheVp::VideoProcessorStatus VideoProcessorImpl::GetStatus()
{
    taiheVp::VideoProcessorAiHdrStatus aiHdrStatus;
    bool aiHdrEnabled = GetCurrentAiHdrEnabled();
    aiHdrStatus.enabled.emplace(aiHdrEnabled);
    taiheVp::VideoProcessorStatus status;
    status.aiHdr.emplace(aiHdrStatus);
    VPE_LOGI("GetStatus: aiHdrEnabled=%{public}d", aiHdrEnabled);
    return status;
}

void VideoProcessorImpl::OnStatusChange(
    taihe::callback_view<void(const taiheVp::VideoProcessorStatus&)> callback)
{
    VPETrace vpeTrace("VideoProcessorImpl::OnStatusChange");
    size_t callbackSize = 0;
    {
        std::lock_guard<std::mutex> lock(g_statusCallbacksLock);
        if (g_statusCallbacks.empty()) {
            VpeVideo::RegisterSettingsChangeCallback(VIDEO_TYPE_AIHDR_ENHANCER,
                [this](int32_t feature, int32_t tag, int32_t param) {
                    StatusChangeCallback(feature, tag, param);
            });
        }
        g_statusCallbacks.push_back(callback);
        callbackSize = g_statusCallbacks.size();
    }
    
    bool aiHdrEnabled = GetCurrentAiHdrEnabled();
    TriggerSingleCallback(callback, aiHdrEnabled);
    VPE_LOGI("OnStatusChange done, callbackCount=%{public}zu", callbackSize);
}

void VideoProcessorImpl::OffStatusChange(
    taihe::optional_view<taihe::callback<void(const taiheVp::VideoProcessorStatus&)>> callback)
{
    VPETrace vpeTrace("VideoProcessorImpl::OffStatusChange");
    std::lock_guard<std::mutex> lock(g_statusCallbacksLock);
    if (callback.has_value()) {
        auto it = std::find_if(g_statusCallbacks.begin(), g_statusCallbacks.end(),
            [&callback](const taihe::callback<void(const taiheVp::VideoProcessorStatus&)>& item) {
                return item == callback.value();
            });
        if (it != g_statusCallbacks.end()) {
            g_statusCallbacks.erase(it);
        }
    } else {
        g_statusCallbacks.clear();
    }
    
    if (g_statusCallbacks.empty()) {
        VpeVideo::UnregisterSettingsChangeCallback(VIDEO_TYPE_AIHDR_ENHANCER);
    }
    VPE_LOGI("OffStatusChange done, remainingCallbacks=%{public}zu", g_statusCallbacks.size());
}

// NOLINTBEGIN
TH_EXPORT_CPP_API_createVideoProcessor(createVideoProcessor);
// NOLINTEND
