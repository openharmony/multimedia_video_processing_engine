/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORKS_KITS_TAIHE_INCLUDE_VIDEO_PROCESSING_ENGINE_TAIHE_H
#define FRAMEWORKS_KITS_TAIHE_INCLUDE_VIDEO_PROCESSING_ENGINE_TAIHE_H

#include "ohos.multimedia.videoProcessingEngine.impl.hpp"
#include "ohos.multimedia.videoProcessingEngine.proj.hpp"

#include "detail_enhancer_image.h"
#include "image_type.h"
#include "image_processing_types.h"
#include "pixel_map.h"

#include "taihe/runtime.hpp"
#include "pixel_map_taihe.h"

namespace ANI:Vpe {
using namespace taihe;
using namespace OHOS;
using namespace OHOS::Media;
using namespace OHOS::Media::VideoProcessingEngine;
namespace taiheVpe = ::ohos::multimedia::videoProcessingEngine;
namespace taiheImage = ::ohos::multimedia::image::image;

class ImageProcessorImpl {
public:
    ImageProcessorImpl() = default;
    taihe::PixelMap EnhanceDetailWithRes(taiheImage::weak::PixelMap sourceImage, int width, int height,
        optional_view<taiheVpe::QualityLevel> level);
    taihe::PixelMap EnhanceDetailWithRes(taiheImage::weak::PixelMap sourceImage, double scale,
        optional_view<taiheVpe::QualityLevel> level);
    taihe::PixelMap EnhanceDetailWithRes(taiheImage::weak::PixelMap sourceImage, int width, int height,
        optional_view<taiheVpe::QualityLevel> level);
    taihe::PixelMap EnhanceDetailWithRes(taiheImage::weak::PixelMap sourceImage, double scale,
        optional_view<taiheVpe::QualityLevel> level);
};

private:
    struct DetailEnhanceContext {
        double xArg{};
        double yArg{};
        int32_t qualityLevel{};
        std::shared_ptr<PixelMap> inputPixelMap;
        std::shared_ptr<PixelMap> outputPixelMap;
    }

    void ParseDetailEnhanceParameter(std::unique_ptr<DetailEnhanceContext>& detailContext,
    taiheImage::weak::PixelMap sourceImage, int width, int height, optional_view<taiheVpe::QualityLevel> level);
    void ParseDetailEnhanceParameter(std::unique_ptr<DetailEnhanceContext>& detailContext,
    taiheImage::weak::PixelMap sourceImage, double scale, optional_view<taiheVpe::QualityLevel> level);
    void ParseDetailEnhanceParameter(std::unique_ptr<DetailEnhanceContext>& detailContext,
    taiheImage::weak::PixelMap sourceImage, double scale, optional_view<taiheVpe::QualityLevel> level);
    std::shared_ptr<PixelMap> DetailEnhanceImpl(DetailEnhanceContext* context);
    
}

#endif