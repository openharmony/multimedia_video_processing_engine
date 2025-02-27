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

#ifndef IMAGE_ENVIRONMENT_NATIVE_H
#define IMAGE_ENVIRONMENT_NATIVE_H

#include <cinttypes>
#include <mutex>

#include "image_processing_types.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class ImageEnvironmentNative {
public:
    static ImageEnvironmentNative& Get();

    ImageProcessing_ErrorCode Initialize();
    ImageProcessing_ErrorCode Deinitialize();
    ImageProcessing_ErrorCode InitializeByDefault();
    ImageProcessing_ErrorCode DeinitializeByDefault();

private:
    ImageEnvironmentNative() = default;
    virtual ~ImageEnvironmentNative() = default;
    ImageEnvironmentNative(const ImageEnvironmentNative&) = delete;
    ImageEnvironmentNative& operator=(const ImageEnvironmentNative&) = delete;
    ImageEnvironmentNative(ImageEnvironmentNative&&) = delete;
    ImageEnvironmentNative& operator=(ImageEnvironmentNative&&) = delete;

    ImageProcessing_ErrorCode InitializeLocked();
    ImageProcessing_ErrorCode DeinitializeLocked();
    ImageProcessing_ErrorCode InitializeEnvLocked();
    ImageProcessing_ErrorCode DeinitializeEnvLocked();

    std::mutex lock_{};
    // Guarded by lock_ begin
    bool isExplicitInit_{};
    uint32_t referenceCount_{};
    // Guarded by lock_ end
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // IMAGE_ENVIRONMENT_NATIVE_H
