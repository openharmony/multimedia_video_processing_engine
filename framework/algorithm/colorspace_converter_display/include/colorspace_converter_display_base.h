/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FRAMEWORK_ALGORITHM_COLORSPACE_CONVERTER_DISPLAY_COLORSPACE_CONVERTER_DISPLAY_BASE_H
#define FRAMEWORK_ALGORITHM_COLORSPACE_CONVERTER_DISPLAY_COLORSPACE_CONVERTER_DISPLAY_BASE_H

#include <functional>
#include <memory>
#include "nocopyable.h"
#include "effect/shader_effect.h"
#include "algorithm_common.h"
#include "deserialized_display_parameter.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

using namespace HDI::Display::Graphic::Common::V1_0;

class ColorSpaceConverterDisplayBase : public NoCopyable {
public:
    virtual ~ColorSpaceConverterDisplayBase() = default;
    virtual VPEAlgoErrCode Process(const std::shared_ptr<OHOS::Rosen::Drawing::ShaderEffect>& input,
        std::shared_ptr<OHOS::Rosen::Drawing::ShaderEffect>& output,
        const DeserializedDisplayParameter& parameter) = 0;
};

using ColorSpaceConverterDisplayCreator = std::function<std::shared_ptr<ColorSpaceConverterDisplayBase>()>;

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // FRAMEWORK_ALGORITHM_COLORSPACE_CONVERTER_DISPLAY_COLORSPACE_CONVERTER_DISPLAY_BASE_H
