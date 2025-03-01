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

#include "algorithm_video.h"

#include <functional>
#include <unordered_map>

#include "vpe_log.h"

// NOTE: Add feature altorithm header files here
// Feature altorithm header files begin
#include "detail_enhancer_video_fwk.h"
// Feature altorithm header files end

using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;

namespace {
std::unordered_map<uint32_t, std::function<std::shared_ptr<VpeVideo>(void)>> g_creators = {
    // NOTE: Add feature altorithm creator here
    // Feature altorithm header creator begin
    { VIDEO_TYPE_DETAIL_ENHANCER, &DetailEnhancerVideoFwk::Create },
    // Feature altorithm header creator end
};
}

std::shared_ptr<VpeVideo> VpeVideo::Create(uint32_t type)
{
    auto it = g_creators.find(type);
    if (it == g_creators.end()) {
        VPE_LOGE("Unsupported type: 0x%{public}x", type);
        return nullptr;
    }
    return it->second();
}

VPEAlgoErrCode VpeVideo::RegisterCallback([[maybe_unused]] const std::shared_ptr<VpeVideoCallback>& callback)
{
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode VpeVideo::SetOutputSurface([[maybe_unused]] const sptr<Surface>& surface)
{
    return VPE_ALGO_ERR_OK;
}

sptr<Surface> VpeVideo::GetInputSurface()
{
    return nullptr;
}

VPEAlgoErrCode VpeVideo::SetParameter([[maybe_unused]] const Format& parameter)
{
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode VpeVideo::GetParameter([[maybe_unused]] Format& parameter)
{
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode VpeVideo::Start()
{
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode VpeVideo::Stop()
{
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode VpeVideo::Release()
{
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode VpeVideo::Flush()
{
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode VpeVideo::Enable()
{
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode VpeVideo::Disable()
{
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode VpeVideo::NotifyEos()
{
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode VpeVideo::ReleaseOutputBuffer([[maybe_unused]] uint32_t index, [[maybe_unused]] bool render)
{
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode VpeVideo::RenderOutputBufferAtTime([[maybe_unused]] uint32_t index,
    [[maybe_unused]] int64_t renderTimestamp)
{
    return VPE_ALGO_ERR_OK;
}
