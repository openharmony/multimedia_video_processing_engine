/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VIDEO_PROCESSING_ENGINE_UTILS_H
#define VIDEO_PROCESSING_ENGINE_UTILS_H

#include "algorithm_errors.h"
#include "pixel_map.h"
#include "surface_buffer.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class VpeUtils {
public:
    static sptr<SurfaceBuffer> GetSurfaceBufferFromPixelMap(const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap);
    static bool ConvertPixelmapToSurfaceBuffer(
        const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap, sptr<SurfaceBuffer> bufferImpl);
    static bool ConvertSurfaceBufferToPixelmap(const sptr<SurfaceBuffer>& buffer,
        std::shared_ptr<OHOS::Media::PixelMap>& pixelmap);
    static bool SetSurfaceBufferToPixelMap(const sptr<SurfaceBuffer>& buffer,
        std::shared_ptr<OHOS::Media::PixelMap>& pixelmap);
    static bool CopyRGBASurfaceBufferToPixelmap(const sptr<SurfaceBuffer>& buffer,
        std::shared_ptr<OHOS::Media::PixelMap>& pixelmap);
    static bool CopyRGBAPixelmapToSurfaceBuffer(const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap,
        sptr<SurfaceBuffer>& buffer);
    static bool CopyNV12SurfaceBufferToPixelmap(const sptr<SurfaceBuffer>& buffer,
        std::shared_ptr<OHOS::Media::PixelMap>& pixelmap);
    static bool CopyNV12PixelmapToSurfaceBuffer(const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap,
        sptr<SurfaceBuffer>& buffer);
    static VPEAlgoErrCode LoadModel(int modelKey, sptr<SurfaceBuffer>& modelBuffer);
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // VIDEO_PROCESSING_ENGINE_UTILS_H