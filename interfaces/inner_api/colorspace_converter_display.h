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

#ifndef INTERFACES_INNER_API_ALGORITHM_COLORSPACE_CONVERTER_DISPLAY_H
#define INTERFACES_INNER_API_ALGORITHM_COLORSPACE_CONVERTER_DISPLAY_H

#include <atomic>
#include <memory>
#include <set>
#include <vector>
#include "effect/shader_effect.h"
#include "algorithm_common.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class __attribute__((visibility("default"))) ColorSpaceConverterDisplay {
public:
    /* *
     * @brief Create a ColorspaceConverterDisplay object.
     * @syscap
     * @param
     * @return pointer of the ColorspaceConverterDisplay object.
     * @since 4.1
     */
    static std::shared_ptr<ColorSpaceConverterDisplay> Create();

    /* *
     * @brief 构造显示场景中用于色彩空间转换的shader。
     * @syscap
     * @param input 当前已经存在的shader
     * @param output 在inputShader基础上添加了色彩空间转换的shader的新shader
     * @param Parameter 构建色彩空间转换shader需要的参数
     * @return 返回错误码VPEAlgoErrCode
     * @since 4.1
     */
    virtual VPEAlgoErrCode Process(const std::shared_ptr<OHOS::Rosen::Drawing::ShaderEffect>& input,
        std::shared_ptr<OHOS::Rosen::Drawing::ShaderEffect>& output,
        const ColorSpaceConverterDisplayParameter& parameter) = 0;
protected:
    virtual ~ColorSpaceConverterDisplay() = default;
};

#ifdef __cplusplus
extern "C" {
#endif

using ColorSpaceConvertDisplayHandle = void;
using VPEShaderEffectHandle = void;

ColorSpaceConvertDisplayHandle *ColorSpaceConvertDisplayCreate();
VPEAlgoErrCode ColorSpaceConvertDisplayProcess(ColorSpaceConvertDisplayHandle *handle, VPEShaderEffectHandle *input,
    VPEShaderEffectHandle *output, const ColorSpaceConverterDisplayParameter &parameter);
void ColorSpaceConvertDisplayDestroy(ColorSpaceConvertDisplayHandle *handle);

#ifdef __cplusplus
}
#endif

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // INTERFACES_INNER_API_ALGORITHM_COLORSPACE_CONVERTER_DISPLAY_H
