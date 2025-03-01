/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef DETAIL_ENHANCER_BASE_H
#define DETAIL_ENHANCER_BASE_H

#include "algorithm_errors.h"
#include "detail_enhancer_common.h"
#include "refbase.h"
#include "surface_buffer.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
enum DetailEnhancerHighLevel {
    // The high level adopts downshift logic and uses AISR by default. If AISR processing fails, downshift to EVE.
    // Do not modify the high-level logical sequence
    DETAIL_ENH_LEVEL_HIGH_EVE = DETAIL_ENH_LEVEL_HIGH,
    DETAIL_ENH_LEVEL_HIGH_AISR = DETAIL_ENH_LEVEL_HIGH + 1,
    DETAIL_ENH_LEVEL_VIDEO = DETAIL_ENH_LEVEL_HIGH + 2,
};

class DetailEnhancerBase {
public:
    enum DetailEnhancerType {
        DETAIL_ENH_TYPE_IMAGE = 0,
        DETAIL_ENH_TYPE_VIDEO,
    };
 
    DetailEnhancerBase() = default;
    virtual ~DetailEnhancerBase() = default;
    DetailEnhancerBase(const DetailEnhancerBase&) = delete;
    DetailEnhancerBase& operator=(const DetailEnhancerBase&) = delete;
    DetailEnhancerBase(DetailEnhancerBase&&) = delete;
    DetailEnhancerBase& operator=(DetailEnhancerBase&&) = delete;

    virtual VPEAlgoErrCode Init() = 0;
    virtual VPEAlgoErrCode Deinit() = 0;
    virtual VPEAlgoErrCode SetParameter(const DetailEnhancerParameters& parameter, int type, bool flag) = 0;
    virtual VPEAlgoErrCode Process(const sptr<SurfaceBuffer>& input, const sptr<SurfaceBuffer>& output) = 0;
};

using DetailEnhancerCreator = std::function<std::shared_ptr<DetailEnhancerBase>()>;
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif /* DETAIL_ENHANCER_BASE_H */
