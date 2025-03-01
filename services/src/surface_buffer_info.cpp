/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

namespace OHOS {
bool SurfaceBufferInfo::ReadFromParcel(Parcel &parcel)
{
    auto messageParcel = static_cast<MessageParcel*>(&parcel);
    if (surfacebuffer == nullptr) {
        VPE_LOGE("surfacebuffer is null!");
        return false;
    }
    auto status = surfacebuffer->ReadFromMessageParcel(*messageParcel);
    if (status != GSERROR_OK) {
        return false;
    }
    if (surfacebuffer == nullptr) {
        return false;
    }
    return true;
}

bool SurfaceBufferInfo::Marshalling(Parcel &parcel) const
{
    auto messageParcel = static_cast<MessageParcel*>(&parcel);
    auto status = surfacebuffer->WriteToMessageParcel(*messageParcel);
    if (status != GSERROR_OK) {
        return false;
    }
    return true;
}

SurfaceBufferInfo *SurfaceBufferInfo::Unmarshalling(Parcel &parcel)
{
    SurfaceBufferInfo *info = new (std::nothrow) SurfaceBufferInfo();
    if (info == nullptr) {
        return nullptr;
    }
    info->surfacebuffer = SurfaceBuffer::Create();
    if (!info->ReadFromParcel(parcel)) {
        delete info;
        return nullptr;
    }

    return info;
}
} // namespace OHOS