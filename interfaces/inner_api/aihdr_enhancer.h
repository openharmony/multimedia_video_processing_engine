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

#ifndef INTERFACES_INNER_API_AIHDR_ENHANCER_H
#define INTERFACES_INNER_API_AIHDR_ENHANCER_H

#include <atomic>
#include <memory>
#include <optional>

#include "external_window.h"

#include "algorithm_common.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class __attribute__((visibility("default"))) AihdrEnhancer {
public:
    /* *
     * @brief Create a AihdrEnhancer object.
     * @syscap
     * @return pointer of the AihdrEnhancer object.
     * @since 14
     */
    static std::shared_ptr<AihdrEnhancer> Create();

    /* *
     * @brief 设置参数
     * @syscap
     * @param parameter 输入参数
     * @return 返回错误码VPEAlgoErrCode
     * @since 14
     */
    virtual VPEAlgoErrCode SetParameter(const int& parameter) = 0;

    /* *
     * @brief 查询参数
     * @syscap
     * @param parameter 输出参数
     * @return 返回错误码VPEAlgoErrCode
     * @since 14
     */
    virtual VPEAlgoErrCode GetParameter(int& parameter) const = 0;

    /* *
     * @brief 用于解码后sdr视频帧生成128位lut曲线。
     * @syscap
     * @param input 输入图片，生成的lut曲线写入该image
     * @return 返回错误码VPEAlgoErrCode
     * @since 14
     */
    virtual VPEAlgoErrCode Process(const sptr<SurfaceBuffer>& input) = 0;

protected:
    virtual ~AihdrEnhancer() = default;
};

extern "C" int32_t AihdrEnhancerCreate(int32_t* instance);
 
extern "C" int32_t AihdrEnhancerProcessImage(int32_t instance, OHNativeWindowBuffer* inputImage);

extern "C" int32_t AihdrEnhancerDestroy(int32_t* instance);

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // INTERFACES_INNER_API_AIHDR_ENHANCER_H
