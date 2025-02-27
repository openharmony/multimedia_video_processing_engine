/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef IMAGE_PROCESSING_UTILS_H
#define IMAGE_PROCESSING_UTILS_H

#include "pixelmap_native_impl.h"
#include "surface_buffer.h"

#include "algorithm_errors.h"
#include "image_processing_types.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class ImageProcessingUtils {
public:
    static ImageProcessing_ErrorCode InnerErrorToNDK(VPEAlgoErrCode errorCode);
    static std::string ToString(ImageProcessing_ErrorCode errorCode);
    static sptr<SurfaceBuffer> GetSurfaceBufferFromPixelMap(const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap);
    static ImageProcessing_ErrorCode ConvertPixelmapToSurfaceBuffer(
        const std::shared_ptr<OHOS::Media::PixelMap>& pixelmap, sptr<SurfaceBuffer> bufferImpl);
    static ImageProcessing_ErrorCode ConvertSurfaceBufferToPixelmap(const sptr<SurfaceBuffer>& buffer,
        std::shared_ptr<OHOS::Media::PixelMap>& pixelmap);
    static ImageProcessing_ErrorCode SetSurfaceBufferToPixelMap(const sptr<SurfaceBuffer>& buffer,
        std::shared_ptr<OHOS::Media::PixelMap>& pixelmap);
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
 
#endif // IMAGE_PROCESSING_UTILS_H