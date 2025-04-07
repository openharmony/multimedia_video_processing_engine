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

#ifndef SURFACE_BUFFER_INFO_H
#define SURFACE_BUFFER_INFO_H

#include <string>

#include "parcel.h"
#include "surface_buffer.h"

#include "vpe_sa_types.h"

namespace OHOS {
struct SurfaceBufferInfo : public Parcelable {
    // Data
    Media::VideoProcessingEngine::VideoInfo videoInfo;
    sptr<SurfaceBuffer> surfacebuffer;

    // Get rectangle string of the surface buffer
    std::string rectStr() const;
    // Get rectangle and id, index string of the surface buffer
    std::string str() const;
    // Copy all information except surface buffer
    void CopyInfo(const SurfaceBufferInfo& input);

    // For IPC transfer
    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static SurfaceBufferInfo *Unmarshalling(Parcel &parcel);
};
} // namespace OHOS

#endif // SURFACE_BUFFER_INFO_H
