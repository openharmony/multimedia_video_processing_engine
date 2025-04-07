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

#include "video_processing_factory.h"

#include <functional>
#include <unordered_map>

#include "vpe_log.h"
#include "video_processing_native_template.h"
// NOTE: Add VPE feature header files like below.
// VPE feature header begin
#include "detail_enhancer_video_native.h"
#include "colorSpace_converter_video_native.h"
#include "metadata_generator_video_native.h"
// VPE feature header end

using namespace OHOS::Media::VideoProcessingEngine;

namespace {
template <typename T>
std::shared_ptr<IVideoProcessingNative> Create(OH_VideoProcessing* context)
{
    return VideoProcessingNativeTemplate<T>::Create(context);
}
// NOTE: Add VPE feature type like below.
// VPE feature map begin
const std::unordered_map<int, std::function<std::shared_ptr<IVideoProcessingNative>(OH_VideoProcessing*)>> CREATORS = {
    { VIDEO_PROCESSING_TYPE_DETAIL_ENHANCER, Create<DetailEnhancerVideoNative> },
    { VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION, Create<ColorSpaceConverterVideoNative> },
    { VIDEO_PROCESSING_TYPE_METADATA_GENERATION, Create<MetadataGeneratorVideoNative> },
    // ...
};
// VPE feature map end
}

bool VideoProcessingFactory::IsValid(int type)
{
    if (access("/system/lib64/libvideoprocessingengine_ext.z.so", 0)) {
        if (type != VIDEO_PROCESSING_TYPE_DETAIL_ENHANCER) {
            return false;
        }
    }
    return CREATORS.find(type) != CREATORS.end();
}

std::shared_ptr<IVideoProcessingNative> VideoProcessingFactory::CreateVideoProcessing(int type,
    OH_VideoProcessing* context)
{
    if (context == nullptr) [[unlikely]] {
        VPE_LOGE("Invalid input: context is null!");
        return nullptr;
    }
    auto it = CREATORS.find(type);
    if (it == CREATORS.end()) {
        VPE_LOGE("Unknown type:%{public}d!", type);
        return nullptr;
    }
    return it->second(context);
}
