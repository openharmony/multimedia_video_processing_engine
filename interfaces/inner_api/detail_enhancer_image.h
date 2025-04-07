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

#ifndef INTERFACES_INNER_API_VPE_DETAIL_ENHANCER_IMAGE_H
#define INTERFACES_INNER_API_VPE_DETAIL_ENHANCER_IMAGE_H

#include <memory>

#include "algorithm_errors.h"
#include "detail_enhancer_common.h"
#include "external_window.h"
#include "refbase.h"
#include "surface_buffer.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
/**
 * Process
 *      执行计算画质算法对图像质量进行增强，如：超分、AIHDR等：
 *      1. AISR超分:
 *          原始图片 -> 缩放后的目标尺寸图片
 *      2. AIHDR:
 *          原始图片 -> 处理后的单层HDR图片
 */
class __attribute__((visibility("default"))) DetailEnhancerImage {
public:
    /**
     * @brief Create a DetailEnhancerImage object.
     * @syscap
     * @param type 超分的类型为图片还是视频
     * @return pointer of the DetailEnhancerImage object.
     * @since 12
     */
    static std::shared_ptr<DetailEnhancerImage> Create(int type = IMAGE);

    /**
     * @brief 设置计算画质参数。可被多次调用，但只有最接近Process的一次调用设置的参数
     * 会在Process时生效。
     * @syscap
     * @param parameter 转换参数
     * @return 返回错误码VPEAlgoErrCode
     * @since 12
     */
    virtual VPEAlgoErrCode SetParameter(const DetailEnhancerParameters& parameter) = 0;

    /**
     * @brief 查询参数
     * @syscap
     * @param parameter 输出参数
     * @return 返回错误码VPEAlgoErrCode
     * @since 12
     */
    virtual VPEAlgoErrCode GetParameter(DetailEnhancerParameters& parameter) const = 0;

    /**
     * @brief 用于解码后的单层图片画质增强处理，如：超分、AIHDR等
     * @syscap
     * @param input 输入的sdr图片或单层hdr图片
     * @param output 输出画质增强后的sdr图片或单层hdr图片
     * @return 返回错误码VPEAlgoErrCode
     * @since 12
     */
    virtual VPEAlgoErrCode Process(const sptr<SurfaceBuffer>& input, const sptr<SurfaceBuffer>& output) = 0;

    virtual VPEAlgoErrCode EnableProtection(bool enable) = 0;
    virtual VPEAlgoErrCode ResetProtectionStatus() = 0;

protected:
    DetailEnhancerImage() = default;
    virtual ~DetailEnhancerImage() = default;
    DetailEnhancerImage(const DetailEnhancerImage&) = delete;
    DetailEnhancerImage& operator=(const DetailEnhancerImage&) = delete;
    DetailEnhancerImage(DetailEnhancerImage&&) = delete;
    DetailEnhancerImage& operator=(DetailEnhancerImage&&) = delete;
};

extern "C" __attribute__((visibility("default"))) int32_t DetailEnhancerCreate(int32_t* instance);
extern "C" __attribute__((visibility("default"))) int32_t DetailEnhancerProcessImage(int32_t instance,
    OHNativeWindowBuffer* inputImage, OHNativeWindowBuffer* outputImage, int32_t level);
extern "C" __attribute__((visibility("default"))) int32_t DetailEnhancerDestroy(int32_t* instance);
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // INTERFACES_INNER_API_VPE_DETAIL_ENHANCER_IMAGE_H
