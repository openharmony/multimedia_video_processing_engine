/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef VPE_TEST_DETAIL_ENH_SAMPLE_H
#define VPE_TEST_DETAIL_ENH_SAMPLE_H

#include "detail_enhancer_image.h"

#define TEST_LOG(msg, ...) printf("%s: %s:%d " msg "\n", LOG_TAG, __FUNCTION__, __LINE__, ##__VA_ARGS__)

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
void ReadYuvFile(sptr<SurfaceBuffer> &buffer, std::unique_ptr<std::ifstream> &yuvFile, int32_t frameSize);
sptr<SurfaceBuffer> CreateSurfaceBuffer(uint32_t pixelFormat, int32_t width, int32_t height);
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
#endif // VPE_TEST_CSC_SAMPLE_H
