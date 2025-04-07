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

#ifndef INTERFACES_INNER_API_VPE_CONTRAST_ENHANCER_IMAGE_H
#define INTERFACES_INNER_API_VPE_CONTRAST_ENHANCER_IMAGE_H

#include <memory>

#include "algorithm_errors.h"
#include "contrast_enhancer_common.h"
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
class __attribute__((visibility("default"))) ContrastEnhancerImage {
public:
    /**
     * @brief Create a ContrastEnhancerImage object.
     * @syscap
     * @param type 超分的类型为图片还是视频
     * @return pointer of the ContrastEnhancerImage object.
     * @since 16
     */
    static std::shared_ptr<ContrastEnhancerImage> Create();

    /**
     * @brief 设置计算画质参数。可被多次调用，但只有最接近Process的一次调用设置的参数
     * 会在Process时生效。
     * @syscap
     * @param parameter 转换参数
     * @return 返回错误码VPEAlgoErrCode
     * @since 16
     */
    virtual VPEAlgoErrCode SetParameter(const ContrastEnhancerParameters& parameter) = 0;

    /**
     * @brief 查询参数
     * @syscap
     * @param parameter 输出参数
     * @return 返回错误码VPEAlgoErrCode
     * @since 12
     */
    virtual VPEAlgoErrCode GetParameter(ContrastEnhancerParameters& parameter) const = 0;

    /**
     * @brief 初始化区域直方图
     * @syscap
     * @param input 输入的sdr图片或单层hdr图片
     * @return 返回错误码VPEAlgoErrCode
     * @since 16
     */
    virtual VPEAlgoErrCode GetRegionHist(const sptr<SurfaceBuffer>& input) = 0;

    /**
     * @brief 基于像素值更新元数据
     * @syscap
     * @param displayArea 送显区域
     * @param curPixelmapArea 当前输入pixelmap在完整原图中的区域
     * @param completePixelmapArea 完整原图的分辨率信息
     * @param surfaceBuffer 当前输入pixelmap的surfacebuffer
     * @param fullRatio 缩放比例
     * @return 返回错误码VPEAlgoErrCode
     * @since 16
     */
    virtual VPEAlgoErrCode UpdateMetadataBasedOnPixel(OHOS::Rect displayArea, OHOS::Rect curPixelmapArea,
        OHOS::Rect completePixelmapArea, sptr<SurfaceBuffer> surfaceBuffer, float fullRatio) = 0;

    /**
     * @brief 基于LCD图更新元数据
     * @syscap
     * @param displayArea 送显区域
     * @param lcdWidth lcd图的宽度
     * @param lcdHeight lcd图的高度
     * @param surfaceBuffer 当前输入pixelmap的surfacebuffer
     * @return 返回错误码VPEAlgoErrCode
     * @since 16
     */
    virtual VPEAlgoErrCode UpdateMetadataBasedOnHist(OHOS::Rect displayArea, sptr<SurfaceBuffer> surfaceBuffer,
        std::tuple<int, int, double, double, double, int> pixelmapInfo) = 0;

protected:
    ContrastEnhancerImage() = default;
    virtual ~ContrastEnhancerImage() = default;
    ContrastEnhancerImage(const ContrastEnhancerImage&) = delete;
    ContrastEnhancerImage& operator=(const ContrastEnhancerImage&) = delete;
    ContrastEnhancerImage(ContrastEnhancerImage&&) = delete;
    ContrastEnhancerImage& operator=(ContrastEnhancerImage&&) = delete;
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // INTERFACES_INNER_API_VPE_CONTRAST_ENHANCER_IMAGE_H
