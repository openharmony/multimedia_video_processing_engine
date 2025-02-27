/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "csc_sample.h"
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
    if (!yuvFile->is_open()) {
        printf("Yuv file is not open\n");
        return;
    }
    yuvFile->read(reinterpret_cast<char *>(buffer->GetVirAddr()), frameSize);
}

sptr<SurfaceBuffer> CreateSurfaceBuffer(uint32_t pixelFormat, int32_t width, int32_t height)
{
    auto buffer = SurfaceBuffer::Create();
    if (nullptr == buffer) {
        printf("Create surface buffer failed\n");
        return nullptr;
    }
    BufferRequestConfig inputCfg;
    inputCfg.width = width;
    inputCfg.height = height;
    inputCfg.strideAlignment = width;
    inputCfg.usage = BUFFER_USAGE_CPU_READ | BUFFER_USAGE_CPU_WRITE | BUFFER_USAGE_HW_RENDER
    | BUFFER_USAGE_HW_TEXTURE;
    inputCfg.format = pixelFormat;
    inputCfg.timeout = 0;
    GSError err = buffer->Alloc(inputCfg);
    if (GSERROR_OK != err) {
        printf("Alloc surface buffer failed\n");
        return nullptr;
    }
    return buffer;
}

void SetMeatadata(sptr<SurfaceBuffer> &buffer, uint32_t value)
{
    std::vector<uint8_t> metadata;
    metadata.resize(sizeof(value));
    (void)memcpy_s(metadata.data(), metadata.size(), &value, sizeof(value));
    GSError err = buffer->SetMetadata(ATTRKEY_HDR_METADATA_TYPE, metadata);
    if (err != 0) {
        printf("Buffer set metadata type, ret: %d\n", static_cast<int32_t>(err));
    }
}

void SetMeatadata(sptr<SurfaceBuffer> &buffer, const CM_ColorSpaceInfo &colorspaceInfo)
{
    std::vector<uint8_t> metadata;
    metadata.resize(sizeof(CM_ColorSpaceInfo));
    (void)memcpy_s(metadata.data(), metadata.size(), &colorspaceInfo, sizeof(CM_ColorSpaceInfo));
    GSError err = buffer->SetMetadata(ATTRKEY_COLORSPACE_INFO, metadata);
    if (err != 0) {
        printf("Buffer set colorspace info, ret: %d\n", static_cast<int32_t>(err));
    }
}

void SetMeatadata(sptr<SurfaceBuffer> &buffer, std::unique_ptr<std::ifstream> &metadataFile)
{
    if (!metadataFile->is_open()) {
        printf("Metadata file is not open\n");
        return;
    }

    std::vector<uint8_t> metadata;
    uint32_t metadataSize = 0;

    metadataFile->read(reinterpret_cast<char *>(&metadataSize), sizeof(uint32_t));
    if (metadataSize == 0) {
        printf("Read metadata failed, get a size: %u\n", metadataSize);
        return;
    }
    metadata.resize(metadataSize);
    metadataFile->read(reinterpret_cast<char *>(metadata.data()), metadataSize);

    GSError err = buffer->SetMetadata(ATTRKEY_HDR_DYNAMIC_METADATA, metadata);
    if (err != 0) {
        printf("Buffer hdr dynamic metadata type, ret: %d\n", static_cast<int32_t>(err));
    }
}

void PrintMetadataType(sptr<SurfaceBuffer> &buffer, int32_t bufferHandleAttrKey)
{
    std::vector<uint8_t> metadata;
    GSError err = buffer->GetMetadata(bufferHandleAttrKey, metadata);
    if (err != 0) {
        printf("Get metadata failed, err: %d\n", static_cast<int32_t>(err));
        return;
    }
    int32_t value;
    (void)memcpy_s(&value, sizeof(value), metadata.data(), metadata.size());

    std::string bufferHandleStr = bufferHandleAttrKey == ATTRKEY_COLORSPACE_TYPE ? "colorspace" : "metadata";
    printf("Buffer %s type %d\n", bufferHandleStr.c_str(), value);
}

void PrintMetadataKey(sptr<SurfaceBuffer> &buffer)
{
    std::vector<uint32_t> keys;
    GSError err = buffer->ListMetadataKeys(keys);
    printf("List buffer key %d\n", static_cast<int32_t>(err));
    for (auto i : keys) {
        printf("Buffer metadata key %u\n", i);
    }

    err = buffer->EraseMetadataKey(ATTRKEY_COLORSPACE_TYPE);
    printf("Buffer erase metadata key %d\n", static_cast<int32_t>(err));
    err = buffer->EraseMetadataKey(ATTRKEY_HDR_METADATA_TYPE);
    printf("Buffer erase metadata key %d\n", static_cast<int32_t>(err));
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
