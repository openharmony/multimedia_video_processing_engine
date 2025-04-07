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

#ifndef VIDEO_PROCESSING_ALGORITHM_WITHOUT_DATA_H
#define VIDEO_PROCESSING_ALGORITHM_WITHOUT_DATA_H

#include <functional>
#include <string>
#include <unordered_map>

#include "video_processing_algorithm_base.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class VideoProcessingAlgorithmWithoutData : public VideoProcessingAlgorithmBase {
public:
    int SetParameter(uint32_t clientID, int tag, const std::vector<uint8_t>& parameter) final;
    int GetParameter(uint32_t clientID, int tag, std::vector<uint8_t>& parameter) final;
    int DoUpdateMetadata(uint32_t clientID, SurfaceBufferInfo& image) final;
    int DoProcess(uint32_t clientID, const SurfaceBufferInfo& input, SurfaceBufferInfo& output) final;
    int DoComposeImage(uint32_t clientID, const SurfaceBufferInfo& inputSdrImage,
        const SurfaceBufferInfo& inputGainmap, SurfaceBufferInfo& outputHdrImage, bool legacy) final;
    int DoDecomposeImage(uint32_t clientID, const SurfaceBufferInfo& inputImage,
        SurfaceBufferInfo& outputSdrImage, SurfaceBufferInfo& outputGainmap) final;

protected:
    VideoProcessingAlgorithmWithoutData(const std::string& feature, uint32_t id)
        : VideoProcessingAlgorithmBase(feature, id) {}
    virtual ~VideoProcessingAlgorithmWithoutData() = default;
    VideoProcessingAlgorithmWithoutData(const VideoProcessingAlgorithmWithoutData&) = delete;
    VideoProcessingAlgorithmWithoutData& operator=(const VideoProcessingAlgorithmWithoutData&) = delete;
    VideoProcessingAlgorithmWithoutData(VideoProcessingAlgorithmWithoutData&&) = delete;
    VideoProcessingAlgorithmWithoutData& operator=(VideoProcessingAlgorithmWithoutData&&) = delete;

    // NOTE: Would be override by the actual algorithm when necessary
    // Actual methods begin
    // int OnInitializeLocked()     - declared by VideoProcessingAlgorithmBase
    // int OnDeinitializeLocked()   - declared by VideoProcessingAlgorithmBase
    virtual int OnSetParameter(const std::string& clientName, int tag, const std::vector<uint8_t>& parameter);
    virtual int OnGetParameter(const std::string& clientName, int tag, std::vector<uint8_t>& parameter);
    virtual int OnUpdateMetadata(const std::string& clientName, SurfaceBufferInfo& image);
    virtual int OnProcess(const std::string& clientName, const SurfaceBufferInfo& input, SurfaceBufferInfo& output);
    virtual int OnComposeImage(const std::string& clientName, const SurfaceBufferInfo& inputSdrImage,
        const SurfaceBufferInfo& inputGainmap, SurfaceBufferInfo& outputHdrImage, bool legacy);
    virtual int OnDecomposeImage(const std::string& clientName, const SurfaceBufferInfo& inputImage,
        SurfaceBufferInfo& outputSdrImage, SurfaceBufferInfo& outputGainmap);
    // Actual methods end

private:
    int AddClientIDLocked(const std::string& clientName, uint32_t& clientID) final;
    int DelClientIDLocked(uint32_t clientID, bool& isEmpty) final;
    void ClearClientsLocked() final;

    int Execute(uint32_t clientID, std::function<int(const std::string&)>&& operation, const LogInfo& logInfo);

    // Guarded by lock_ begin
    std::unordered_map<uint32_t, std::string> clients_{};
    // Guarded by lock_ end
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // VIDEO_PROCESSING_ALGORITHM_WITHOUT_DATA_H
