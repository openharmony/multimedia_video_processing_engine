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

#include <cstdint>
#include <unistd.h>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <fuzzer/FuzzedDataProvider.h>

#include "video_processing_client.h"
#include "services_fuzzer.h"
#include "securec.h"

using namespace std;
using namespace OHOS;

using namespace OHOS::Media::VideoProcessingEngine;
#define   ATTRKEY_END 8192
namespace {
uint32_t LoadInfoFuzzerTest(int32_t key)
{
    VPE_LOGI("LoadInfoFuzzerTest begin!");
    SurfaceBufferInfo bufferInfo;
    auto &manager = VideoProcessingManager::GetInstance();
    manager.LoadInfo(key, bufferInfo);
    VPE_LOGI("LoadInfoFuzzerTest end!");
    return ERR_NONE;
}

uint32_t CreateFuzzerTest(std::string feature, std::string clientName, uint32_t clientID)
{
    VPE_LOGI("CreateFuzzerTest begin!");
    auto &manager = VideoProcessingManager::GetInstance();
    manager.Create(feature, clientName, clientID);
    VPE_LOGI("CreateFuzzerTest end!");
    return ERR_NONE;
}

uint32_t DestroyFuzzerTest(uint32_t clientID)
{
    VPE_LOGI("DestroyFuzzerTest begin!");
    auto &manager = VideoProcessingManager::GetInstance();
    manager.Destroy(clientID);
    VPE_LOGI("DestroyFuzzerTest end!");
    return ERR_NONE;
}

uint32_t SetParameterFuzzerTest(uint32_t clientID, int32_t tag, const std::vector<uint8_t>& parameter)
{
    VPE_LOGI("SetParameterFuzzerTest begin!");
    auto &manager = VideoProcessingManager::GetInstance();
    manager.SetParameter(clientID, tag, parameter);
    VPE_LOGI("SetParameterFuzzerTest end!");
    return ERR_NONE;
}

uint32_t GetParameterFuzzerTest(uint32_t clientID, int32_t tag)
{
    VPE_LOGI("GetParameterFuzzerTest begin!");
    std::vector<uint8_t> param;
    auto &manager = VideoProcessingManager::GetInstance();
    manager.GetParameter(clientID, tag, param);
    VPE_LOGI("GetParameterFuzzerTest end!");
    return ERR_NONE;
}

uint32_t UpdateMetadataFuzzerTest(uint32_t clientID, SurfaceBufferInfo& image)
{
    VPE_LOGI("UpdateMetadataFuzzerTest begin!");
    auto &manager = VideoProcessingManager::GetInstance();
    manager.UpdateMetadata(clientID, image);
    VPE_LOGI("UpdateMetadataFuzzerTest end!");
    return ERR_NONE;
}

uint32_t ProcessFuzzerTest(uint32_t clientID, const SurfaceBufferInfo& input)
{
    VPE_LOGI("ProcessFuzzerTest begin!");
    SurfaceBufferInfo output;
    auto &manager = VideoProcessingManager::GetInstance();
    manager.Process(clientID, input, output);
    VPE_LOGI("ProcessFuzzerTest end!");
    return ERR_NONE;
}

uint32_t ComposeImageFuzzerTest(uint32_t clientID, const SurfaceBufferInfo& inputSdrImage,
         const SurfaceBufferInfo& inputGainmap, bool legacy)
{
    VPE_LOGI("ComposeImageFuzzerTest begin!");
    auto &manager = VideoProcessingManager::GetInstance();
    SurfaceBufferInfo outputHdrImage;
    manager.ComposeImage(clientID, inputSdrImage, inputGainmap, outputHdrImage, legacy);
    VPE_LOGI("ComposeImageFuzzerTest end!");
    return ERR_NONE;
}

uint32_t DecomposeImageTest(uint32_t clientID, const SurfaceBufferInfo& inputImage)
{
    VPE_LOGI("DecomposeImageTest begin!");
    SurfaceBufferInfo outputSdrImage;
    SurfaceBufferInfo outputGainmap;
    auto &manager = VideoProcessingManager::GetInstance();
    manager.DecomposeImage(clientID, inputImage, outputSdrImage, outputGainmap);
    VPE_LOGI("DecomposeImageTest end!");
    return ERR_NONE;
}
}


/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FuzzedDataProvider data_provider(data, size);
    SurfaceBufferInfo buffer;
    buffer.videoInfo.videoIndex = data_provider.ConsumeIntegral<uint64_t>();
    buffer.videoInfo.frameIndex = data_provider.ConsumeIntegral<uint64_t>();
    if (data != nullptr) {
        std::vector<uint8_t> vec;
        vec.resize(size);
        if (memcpy_s(vec.data(), vec.size(), data, size) != 0) {
            VPE_LOGE("memcpy_s err");
            return 0;
        }
        uint32_t key = data_provider.ConsumeIntegralInRange <uint32_t>(0, ATTRKEY_END);
        buffer.surfacebuffer = SurfaceBuffer::Create();
        if (buffer.surfacebuffer == nullptr) {
            VPE_LOGI("buffer.surfacebuffer is nullptr");
            return 0;
        }
        buffer.surfacebuffer->SetMetadata(key, vec);
    }

    LoadInfoFuzzerTest(data_provider.ConsumeIntegral<uint32_t>());

    CreateFuzzerTest(data_provider.ConsumeRandomLengthString(), data_provider.ConsumeRandomLengthString(),
        data_provider.ConsumeIntegral<uint32_t>());

    DestroyFuzzerTest(data_provider.ConsumeIntegral<uint32_t>());

    SetParameterFuzzerTest(data_provider.ConsumeIntegral<uint32_t>(), data_provider.ConsumeIntegral<int32_t>(),
        data_provider.ConsumeRemainingBytes<uint8_t>());

    ProcessFuzzerTest(data_provider.ConsumeIntegral<uint32_t>(), buffer);

    GetParameterFuzzerTest(data_provider.ConsumeIntegral<uint32_t>(), data_provider.ConsumeIntegral<int32_t>());

    UpdateMetadataFuzzerTest(data_provider.ConsumeIntegral<uint32_t>(), buffer);

    ComposeImageFuzzerTest(data_provider.ConsumeIntegral<uint32_t>(), buffer, buffer, data_provider.ConsumeBool());

    DecomposeImageTest(data_provider.ConsumeIntegral<uint32_t>(), buffer);
    return 0;
}