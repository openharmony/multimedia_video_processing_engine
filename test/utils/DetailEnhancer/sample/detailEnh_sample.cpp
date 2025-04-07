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

#define LOG_TAG "detailEnh"

#include "detailEnh_sample.h"

#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <cstdlib>

#include "securec.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
void ReadYuvFile(sptr<SurfaceBuffer> &buffer, std::unique_ptr<std::ifstream> &yuvFile, int32_t frameSize)
{
    if (buffer == nullptr) {
        TEST_LOG("null ptr");
        return;
    }
    if (frameSize < 0) {
        TEST_LOG("Invalid size");
        return;
    }
    if (!yuvFile->is_open()) {
        TEST_LOG("Yuv file is not open");
        return;
    }
    yuvFile->read(reinterpret_cast<char *>(buffer->GetVirAddr()), frameSize);
}

sptr<SurfaceBuffer> CreateSurfaceBuffer(uint32_t pixelFormat, int32_t width, int32_t height)
{
    auto buffer = SurfaceBuffer::Create();
    if (buffer == nullptr) {
        TEST_LOG("Create surface buffer failed");
        return nullptr;
    }
    if (width <= 0 || height <= 0) {
        TEST_LOG("Invalid resolution");
        return nullptr;
    }
    BufferRequestConfig inputCfg;
    inputCfg.width = width;
    inputCfg.height = height;
    inputCfg.strideAlignment = width;
    inputCfg.usage = BUFFER_USAGE_CPU_READ | BUFFER_USAGE_CPU_WRITE
        | BUFFER_USAGE_HW_RENDER | BUFFER_USAGE_HW_TEXTURE | BUFFER_USAGE_MEM_MMZ_CACHE;
    inputCfg.format = pixelFormat;
    inputCfg.timeout = 0;
    GSError err = buffer->Alloc(inputCfg);
    if (GSERROR_OK != err) {
        TEST_LOG("Alloc surface buffer{ %d(%d)x%d format:%d } failed:%d",
            inputCfg.width, inputCfg.strideAlignment, inputCfg.height, inputCfg.format, err);
        return nullptr;
    }
    return buffer;
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
