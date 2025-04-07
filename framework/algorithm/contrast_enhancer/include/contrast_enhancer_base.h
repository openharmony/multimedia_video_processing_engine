/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef CONTRAST_ENHANCER_BASE_H
#define CONTRAST_ENHANCER_BASE_H

#include "algorithm_errors.h"
#include "refbase.h"
#include "surface_buffer.h"

#include "contrast_enhancer_image.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class ContrastEnhancerBase {
public:
    ContrastEnhancerBase() = default;
    virtual ~ContrastEnhancerBase() = default;
    ContrastEnhancerBase(const ContrastEnhancerBase&) = delete;
    ContrastEnhancerBase& operator=(const ContrastEnhancerBase&) = delete;
    ContrastEnhancerBase(ContrastEnhancerBase&&) = delete;
    ContrastEnhancerBase& operator=(ContrastEnhancerBase&&) = delete;

    virtual VPEAlgoErrCode Init() = 0;
    virtual VPEAlgoErrCode Deinit() = 0;
    virtual VPEAlgoErrCode SetParameter(const ContrastEnhancerParameters& parameter) = 0;

    virtual VPEAlgoErrCode GetRegionHist(const sptr<SurfaceBuffer>& input) = 0;
    virtual VPEAlgoErrCode UpdateMetadataBasedOnHist(OHOS::Rect rect, sptr<SurfaceBuffer> surfaceBuffer,
        std::tuple<int, int, double, double, double, int> pixelmapInfo) = 0;
    virtual VPEAlgoErrCode UpdateMetadataBasedOnPixel(OHOS::Rect displayArea, OHOS::Rect curPixelmapArea,
        OHOS::Rect completePixelmapArea, sptr<SurfaceBuffer> surfaceBuffer, float fullRatio) = 0;
};

using ContrastEnhancerCreator = std::function<std::shared_ptr<ContrastEnhancerBase>()>;
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif /* CONTRAST_ENHANCER_BASE_H */
