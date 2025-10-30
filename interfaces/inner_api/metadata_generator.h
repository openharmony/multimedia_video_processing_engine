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

#ifndef INTERFACES_INNER_API_ALGORITHM_METADATA_GENERATOR_H
#define INTERFACES_INNER_API_ALGORITHM_METADATA_GENERATOR_H

#include <atomic>
#include <memory>
#include <optional>
#include "external_window.h"
#include "algorithm_common.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class __attribute__((visibility("default"))) MetadataGenerator {
public:
    /* *
     * @brief Create a MetadataGenerator object.
     * @syscap
     * @return pointer of the MetadataGenerator object.
     * @since 11
     */
    static std::shared_ptr<MetadataGenerator> Create();
    static std::shared_ptr<MetadataGenerator> Create(std::shared_ptr<OpenGLContext> openglContext);

    /* *
     * @brief 设置参数
     * @syscap
     * @param parameter 输入参数
     * @return 返回错误码VPEAlgoErrCode
     * @since 11
     */
    virtual VPEAlgoErrCode SetParameter(const MetadataGeneratorParameter &parameter) = 0;

    /* *
     * @brief 查询参数
     * @syscap
     * @param parameter 输出参数
     * @return 返回错误码VPEAlgoErrCode
     * @since 11
     */
    virtual VPEAlgoErrCode GetParameter(MetadataGeneratorParameter &parameter) const = 0;

    /* *
     * @brief 用于解码后视频帧、sdr和单层hdr图片元数据生成。
     * @syscap
     * @param input 输入图片，生成的元数据写入该image
     * @return 返回错误码VPEAlgoErrCode
     * @since 11
     */
    virtual VPEAlgoErrCode Process(const sptr<SurfaceBuffer> &input) = 0;

protected:
    virtual ~MetadataGenerator() = default;
};

extern "C" int32_t MetadataGeneratorCreate(int32_t* instance);
 
extern "C" int32_t MetadataGeneratorProcessImage(int32_t instance, OHNativeWindowBuffer* inputImage);

extern "C" int32_t MetadataGeneratorProcessVideo(int32_t instance, OHNativeWindowBuffer* inputImage);

extern "C" int32_t MetadataGeneratorDestroy(int32_t* instance);

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // INTERFACES_INNER_API_ALGORITHM_METADATA_GENERATOR_H
