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

#ifndef FRAMEWORKS_KITS_TAIHE_INCLUDE_VIDEO_PROCESSING_TAIHE_H
#define FRAMEWORKS_KITS_TAIHE_INCLUDE_VIDEO_PROCESSING_TAIHE_H

#include "ohos.multimedia.videoProcessing.impl.hpp"
#include "ohos.multimedia.videoProcessing.proj.hpp"
#include <mutex>
#include <vector>
#include <functional>

#include "taihe/runtime.hpp"

namespace ANI::Vp {
using namespace taihe;
namespace taiheVp = ::ohos::multimedia::videoProcessing;

class VideoProcessorImpl {
public:
    VideoProcessorImpl() = default;
    ~VideoProcessorImpl() = default;

    taiheVp::VideoProcessorStatus GetStatus();
    void OnStatusChange(
        taihe::callback_view<void(const taiheVp::VideoProcessorStatus&)> callback);
    void OffStatusChange(
        taihe::optional_view<taihe::callback<void(const taiheVp::VideoProcessorStatus&)>> callback);

private:
    bool GetCurrentAiHdrEnabled();
    void TriggerSingleCallback(const taihe::callback<void(const taiheVp::VideoProcessorStatus&)>& callback,
        bool aiHdrEnabled);
    void StatusChangeCallback(int32_t feature, int32_t tag, int32_t param);

    static std::vector<taihe::callback<void(const taiheVp::VideoProcessorStatus&)>> g_statusCallbacks;
    static std::mutex g_statusCallbacksLock;
    static bool g_isListenerRegistered;
};

}
#endif