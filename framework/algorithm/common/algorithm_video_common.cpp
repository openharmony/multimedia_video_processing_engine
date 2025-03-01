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

#include "algorithm_video_common.h"

#include "vpe_log.h"

using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;

void VpeVideoCallback::OnError([[maybe_unused]] VPEAlgoErrCode errorCode)
{
}

void VpeVideoCallback::OnState([[maybe_unused]] VPEAlgoState state)
{
}

void VpeVideoCallback::OnOutputFormatChanged([[maybe_unused]] const Format& format)
{
}

void VpeVideoCallback::OnEffectChange([[maybe_unused]] uint32_t type)
{
}

void VpeVideoCallback::OnOutputBufferAvailable([[maybe_unused]] uint32_t index, [[maybe_unused]] VpeBufferFlag flag)
{
}

void VpeVideoCallback::OnOutputBufferAvailable(uint32_t index, const VpeBufferInfo& info)
{
    VPE_LOGD("Call OnOutputBufferAvailable(%{public}u,%{public}d)", index, info.flag);
    OnOutputBufferAvailable(index, info.flag);
}
