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

#include "image_processing_factory.h"

#include <functional>
#include <unordered_map>

#include "vpe_log.h"
#include "image_processing_native_template.h"
#include "metadata_generator_image_native.h"
#include "detail_enhancer_image_native.h"
#include "colorspace_converter_image_native.h"

using namespace OHOS::Media::VideoProcessingEngine;

namespace {
template <typename T>
std::shared_ptr<IImageProcessingNative> Create()
{
    return ImageProcessingNativeTemplate<T>::Create();
}
// NOTE: Add VPE feature type like below.
// VPE feature map begin
const std::unordered_map<int, std::function<std::shared_ptr<IImageProcessingNative>()>> CREATORS = {
    { IMAGE_PROCESSING_TYPE_DETAIL_ENHANCER, Create<DetailEnhancerImageNative> },
    { IMAGE_PROCESSING_TYPE_COLOR_SPACE_CONVERSION, Create<ColorspaceConverterImageNative> },
    { IMAGE_PROCESSING_TYPE_COMPOSITION, Create<ColorspaceConverterImageNative> },
    { IMAGE_PROCESSING_TYPE_DECOMPOSITION, Create<ColorspaceConverterImageNative> },
    { IMAGE_PROCESSING_TYPE_METADATA_GENERATION, Create<MetadataGeneratorImageNative> },
    // ...
};
// VPE feature map end
}

bool ImageProcessingFactory::IsValid(int type)
{
    if (access("/system/lib64/libvideoprocessingengine_ext.z.so", 0)) {
        if (type != VIDEO_PROCESSING_TYPE_DETAIL_ENHANCER) {
            return false;
        }
    }
    return CREATORS.find(type) != CREATORS.end();
}

std::shared_ptr<IImageProcessingNative> ImageProcessingFactory::CreateImageProcessing(int type)
{
    auto it = CREATORS.find(type);
    if (it == CREATORS.end()) {
        VPE_LOGE("Unknown type:%{public}d!", type);
        return nullptr;
    }
    return it->second();
}
