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

#ifndef SKIA_IMPL_H
#define SKIA_IMPL_H

#include "surface_buffer.h"
#include "include/core/SkYUVAPixmaps.h"

#include "algorithm_errors.h"
#include "detail_enhancer_base.h"
#include "detail_enhancer_capability.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class Skia : public DetailEnhancerBase {
public:
    Skia() = default;
    virtual ~Skia() = default;
    Skia(const Skia&) = delete;
    Skia& operator=(const Skia&) = delete;
    Skia(Skia&&) = delete;
    Skia& operator=(Skia&&) = delete;

    static std::unique_ptr<DetailEnhancerBase> Create();
    static DetailEnhancerCapability BuildCapabilities();
    VPEAlgoErrCode Init() override;
    VPEAlgoErrCode Deinit() override;
    VPEAlgoErrCode SetParameter(const DetailEnhancerParameters& parameter, int type, bool flag) override;
    VPEAlgoErrCode Process(const sptr<SurfaceBuffer>& input, const sptr<SurfaceBuffer>& output) override;
};

void RegisterSkiaExtensions(uintptr_t extensionListAddr);
} // VideoProcessingEngine
} // Media
} // OHOS

#endif // SKIA_IMPL_H