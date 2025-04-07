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

#ifndef IVIDEO_PROCESSING_ALGORITHM_H
#define IVIDEO_PROCESSING_ALGORITHM_H

#include <cinttypes>
#include <string>
#include <vector>

#include "surface_buffer_info.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class IVideoProcessingAlgorithm {
public:
    virtual int Initialize() = 0;
    virtual int Deinitialize() = 0;
    virtual bool HasClient() const = 0;
    virtual int Add(const std::string& clientName, uint32_t& clientID) = 0;
    virtual int Del(uint32_t clientID) = 0;
    virtual int SetParameter(uint32_t clientID, int tag, const std::vector<uint8_t>& parameter) = 0;
    virtual int GetParameter(uint32_t clientID, int tag, std::vector<uint8_t>& parameter) = 0;
    virtual int UpdateMetadata(uint32_t clientID, SurfaceBufferInfo& image) = 0;
    virtual int Process(uint32_t clientID, const SurfaceBufferInfo& input, SurfaceBufferInfo& output) = 0;
    virtual int ComposeImage(uint32_t clientID, const SurfaceBufferInfo& inputSdrImage,
        const SurfaceBufferInfo& inputGainmap, SurfaceBufferInfo& outputHdrImage, bool legacy) = 0;
    virtual int DecomposeImage(uint32_t clientID, const SurfaceBufferInfo& inputImage,
        SurfaceBufferInfo& outputSdrImage, SurfaceBufferInfo& outputGainmap) = 0;

protected:
    IVideoProcessingAlgorithm() = default;
    virtual ~IVideoProcessingAlgorithm() = default;
    IVideoProcessingAlgorithm(const IVideoProcessingAlgorithm&) = delete;
    IVideoProcessingAlgorithm& operator=(const IVideoProcessingAlgorithm&) = delete;
    IVideoProcessingAlgorithm(IVideoProcessingAlgorithm&&) = delete;
    IVideoProcessingAlgorithm& operator=(IVideoProcessingAlgorithm&&) = delete;
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // IVIDEO_PROCESSING_ALGORITHM_H
