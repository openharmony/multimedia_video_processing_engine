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

#include "video_processing_algorithm_factory.h"

#include <dlfcn.h>

#include "algorithm_errors.h"
#include "video_processing_algorithm_factory_common.h"
#include "vpe_log.h"

// NOTE: Add header file of static algorithm which would be called by VPE SA below:
// algorithm begin
// algorithm end

using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;

namespace {
const std::string DYNAMIC_ALGORITHM_LIBRARY_PATH = "libvideoprocessingengineservice_ext.z.so";
VpeAlgorithmCreatorMap g_creators = {
    // NOTE: Add static algorithm which would be called by VPE SA below:
    // algorithm begin
    // algorithm end
};
}

VideoProcessingAlgorithmFactory::VideoProcessingAlgorithmFactory()
{
    VPE_LOGD("step in");
    if (!LoadDynamicAlgorithm(DYNAMIC_ALGORITHM_LIBRARY_PATH)) {
        UnloadDynamicAlgorithm();
    }
    GenerateFeatureIDs();
}

VideoProcessingAlgorithmFactory::~VideoProcessingAlgorithmFactory()
{
    UnloadDynamicAlgorithm();
}

std::shared_ptr<IVideoProcessingAlgorithm> VideoProcessingAlgorithmFactory::Create(const std::string& feature) const
{
    auto it = g_creators.find(feature);
    if (it == g_creators.end()) {
        return nullptr;
    }
    return it->second.creator(feature, it->second.id);
}

bool VideoProcessingAlgorithmFactory::LoadDynamicAlgorithm(const std::string& path)
{
    handle_ = dlopen(path.c_str(), RTLD_NOW);
    if (handle_ == nullptr) {
        VPE_LOGD("Can't open library '%{public}s' - %{public}s", path.c_str(), dlerror());
        return false;
    }

    using GetCreator = VpeAlgorithmCreatorMap* (*)();
    auto getCreator = reinterpret_cast<GetCreator>(dlsym(handle_, "GetDynamicAlgorithmCreator"));
    if (getCreator == nullptr) {
        VPE_LOGD("Failed to locate GetDynamicAlgorithmCreator in '%{public}s' - %{public}s", path.c_str(), dlerror());
        return false;
    }

    auto dynamicAlgorithms = getCreator();
    if (dynamicAlgorithms == nullptr) {
        VPE_LOGD("Failed to GetDynamicAlgorithmCreator() from '%{public}s'", path.c_str());
        return false;
    }

    auto staticSize = g_creators.size();
    auto dynamicSize = dynamicAlgorithms->size();
    g_creators.merge(*dynamicAlgorithms);
    VPE_LOGI("Algorithms: { static:%{public}zu + dynamic:%{public}zu -> total:%{public}zu }",
        staticSize, dynamicSize, g_creators.size());
    return true;
}

void VideoProcessingAlgorithmFactory::UnloadDynamicAlgorithm()
{
    if (handle_ != nullptr) {
        dlclose(handle_);
        handle_ = nullptr;
    }
}

void VideoProcessingAlgorithmFactory::GenerateFeatureIDs()
{
    // When GenerateFeatureIDs be called, the g_creators size is fixed.
    // And we fill algorithm feature ID here from 1 to N.
    uint32_t id = 0;
    for (auto& creatorInfo : g_creators) {
        creatorInfo.second.id = ++id;
    }
}
