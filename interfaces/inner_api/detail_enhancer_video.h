/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_INNER_API_VPE_DETAIL_ENHANCER_VIDEO_H
#define INTERFACES_INNER_API_VPE_DETAIL_ENHANCER_VIDEO_H

#include <memory>

#include "algorithm_errors.h"
#include "algorithm_video.h"
#include "detail_enhancer_common.h"
#include "detail_enhancer_video_common.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class __attribute__((visibility("default"))) DetailEnhancerVideo : public VpeVideo {
public:
    using VpeVideo::SetParameter;

    /**
     * @brief Create a DetailEnhancerVideo object.
     * @syscap
     * @return pointer of the DetailEnhancerVideo object.
     * @since 12
     */
    static std::shared_ptr<DetailEnhancerVideo> Create();

    /**
     * @brief 设置计算画质参数。可被多次调用，但只有最接近Process的一次调用设置的参数
     * 会在Process时生效。
     * @syscap
     * @param parameter 转换参数
     * @return 返回错误码VPEAlgoErrCode
     * @since 12
     */
    virtual VPEAlgoErrCode SetParameter(const DetailEnhancerParameters& parameter, SourceType type) = 0;

    /**
     * @brief 渲染输出buffer.
     * @syscap
     * @param input 索引
     * @return 返回错误码VPEAlgoErrCode
     * @since 12
     */
    virtual VPEAlgoErrCode RenderOutputBuffer(uint32_t index) = 0;

protected:
    DetailEnhancerVideo() = default;
    virtual ~DetailEnhancerVideo() = default;
    DetailEnhancerVideo(const DetailEnhancerVideo&) = delete;
    DetailEnhancerVideo& operator=(const DetailEnhancerVideo&) = delete;
    DetailEnhancerVideo(DetailEnhancerVideo&&) = delete;
    DetailEnhancerVideo& operator=(DetailEnhancerVideo&&) = delete;
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // INTERFACES_INNER_API_VPE_DETAIL_ENHANCER_VIDEO_H
