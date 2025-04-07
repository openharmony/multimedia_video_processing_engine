/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "surface_buffer_info.h"

#include "message_parcel.h"

#include "vpe_log.h"

using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;

std::string SurfaceBufferInfo::rectStr() const
{
    return surfacebuffer == nullptr ? "image=null" :
        std::to_string(surfacebuffer->GetWidth()) + "(" + std::to_string(surfacebuffer->GetStride()) + ")x" +
        std::to_string(surfacebuffer->GetHeight());
}

std::string SurfaceBufferInfo::str() const
{
    return "buffer:{ id=" + std::to_string(videoInfo.videoIndex) + " idx=" + std::to_string(videoInfo.frameIndex) +
        " " + rectStr() + " }";
}

void SurfaceBufferInfo::CopyInfo(const SurfaceBufferInfo& input)
{
    // Copy information from input except SurfaceBuffer
    videoInfo = input.videoInfo;
}

bool SurfaceBufferInfo::ReadFromParcel(Parcel &parcel)
{
    CHECK_AND_RETURN_RET_LOG(surfacebuffer != nullptr, false, "surfacebuffer is null!");
    return parcel.ReadUint64(videoInfo.videoIndex) && parcel.ReadUint64(videoInfo.frameIndex) &&
        surfacebuffer->ReadFromMessageParcel(*static_cast<MessageParcel*>(&parcel)) == GSERROR_OK;
}

bool SurfaceBufferInfo::Marshalling(Parcel &parcel) const
{
    CHECK_AND_RETURN_RET_LOG(surfacebuffer != nullptr, false, "surfacebuffer is null!");
    return parcel.WriteUint64(videoInfo.videoIndex) && parcel.WriteUint64(videoInfo.frameIndex) &&
        surfacebuffer->WriteToMessageParcel(*static_cast<MessageParcel*>(&parcel)) == GSERROR_OK;
}

SurfaceBufferInfo *SurfaceBufferInfo::Unmarshalling(Parcel &parcel)
{
    SurfaceBufferInfo *info = new (std::nothrow) SurfaceBufferInfo();
    if (info == nullptr) [[unlikely]] {
        return nullptr;
    }
    info->surfacebuffer = SurfaceBuffer::Create();
    if (!info->ReadFromParcel(parcel)) [[unlikely]] {
        delete info;
        return nullptr;
    }

    return info;
}