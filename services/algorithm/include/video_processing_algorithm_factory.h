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

#ifndef VIDEO_PROCESSING_ALGORITHM_FACTORY_H
#define VIDEO_PROCESSING_ALGORITHM_FACTORY_H

#include <memory>
#include <string>

#include "ivideo_processing_algorithm.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class VideoProcessingAlgorithmFactory {
public:
    VideoProcessingAlgorithmFactory();
    ~VideoProcessingAlgorithmFactory();
    VideoProcessingAlgorithmFactory(const VideoProcessingAlgorithmFactory&) = delete;
    VideoProcessingAlgorithmFactory& operator=(const VideoProcessingAlgorithmFactory&) = delete;
    VideoProcessingAlgorithmFactory(VideoProcessingAlgorithmFactory&&) = delete;
    VideoProcessingAlgorithmFactory& operator=(VideoProcessingAlgorithmFactory&&) = delete;

    std::shared_ptr<IVideoProcessingAlgorithm> Create(const std::string& feature) const;

private:
    bool LoadDynamicAlgorithm(const std::string& path);
    void UnloadDynamicAlgorithm();
    void GenerateFeatureIDs();

    void* handle_{};
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // VIDEO_PROCESSING_ALGORITHM_FACTORY_H
