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

#ifndef INTERFACES_INNER_API_ALGORITHM_COLORSPACE_CONVERTER_H
#define INTERFACES_INNER_API_ALGORITHM_COLORSPACE_CONVERTER_H

#include <atomic>
#include <memory>
#include <optional>
#include "external_window.h"
#include "algorithm_common.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
/**
 * Process
 * 解码后的视频帧转换
 * 单层hdr图片 -> sdr图片
 * sdr图片     -> 单层hdr图片
 * sdr图片     -> sdr图片
 * ComposeImage
 * 双层hdr图片  -> 单层hdr图片
 * DecomposeImage
 * 单层hdr图片  -> 双层hdr图片
 * sdr图片      -> 双层hdr图片
 */
class __attribute__((visibility("default"))) ColorSpaceConverter {
public:
    /* *
     * @brief Create a ColorspaceConverter object.
     * @syscap
     * @return pointer of the ColorspaceConverter object.
     * @since 11
     */
    static std::shared_ptr<ColorSpaceConverter> Create();
    static std::shared_ptr<ColorSpaceConverter> Create(std::shared_ptr<OpenGLContext> openglContext,
                                                       ClContext *opengclContext = nullptr);

    /* *
     * @brief 设置色彩空间转换参数。可被多次调用，但只有最接近Process的一次调用设置的参数
     * 会在Process时生效。
     * @syscap
     * @param parameter 转换参数
     * @return 返回错误码VPEAlgoErrCode
     * @since 11
     */
    virtual VPEAlgoErrCode SetParameter(const ColorSpaceConverterParameter &parameter) = 0;

    /* *
     * @brief 查询参数
     * @syscap
     * @param parameter 输出参数
     * @return 返回错误码VPEAlgoErrCode
     * @since 11
     */
    virtual VPEAlgoErrCode GetParameter(ColorSpaceConverterParameter &parameter) const = 0;

    /* *
     * @brief 用于解码后的视频帧，以及单层图片的转换，
     * 解码视频帧转换
     * 单层hdr图片 -> sdr图片
     * sdr图片     -> 单层hdr图片
     * sdr图片     -> sdr图片
     * @syscap
     * @param input 输入的源视频帧、sdr图片或单层hdr图片
     * @param output 输出的视频帧、sdr图片或单层hdr图片
     * @return 返回错误码VPEAlgoErrCode
     * @since 11
     */
    virtual VPEAlgoErrCode Process(const sptr<SurfaceBuffer> &input, const sptr<SurfaceBuffer> &output) = 0;

    /* *
     * @brief 用于双层hdr图片转单层hdr图片。
     * @syscap
     * @param inputSdrImage 输入的双层hdr图片的sdr图片部分
     * @param inputGainmap 输入的双层hdr图片的gainmap部分
     * @param outputHdrImage 输出的单层hdr图片
     * @param legacy 如果输入的双层hdr图片是老双层的格式，则应当设置为true；否则设置为false
     * @return 返回错误码VPEAlgoErrCode
     * @since 11
     */
    virtual VPEAlgoErrCode ComposeImage(const sptr<SurfaceBuffer> &inputSdrImage,
        const sptr<SurfaceBuffer> &inputGainmap, const sptr<SurfaceBuffer> &outputHdrImage, bool legacy) = 0;

    /* *
     * @brief 用于sdr图片或单层hdr图片转双层hdr图片。输出为新双层hdr图片格式。
     * @syscap
     * @param inputImage 输入的sdr图片或单层hdr图片
     * @param outputSdrImage 输出的双层hdr图片的sdr图片部分
     * @param outputGainmap 输出的双层hdr图片的gainmap部分
     * @return 返回错误码VPEAlgoErrCode
     * @since 11
     */
    virtual VPEAlgoErrCode DecomposeImage(const sptr<SurfaceBuffer> &inputImage,
        const sptr<SurfaceBuffer> &outputSdrImage, const sptr<SurfaceBuffer> &outputGainmap) = 0;

protected:
    virtual ~ColorSpaceConverter() = default;
};

extern "C" int32_t ColorSpaceConverterCreate(int32_t* instance);

extern "C" int32_t ColorSpaceConverterProcessImage(int32_t instance, OHNativeWindowBuffer* inputImage,
    OHNativeWindowBuffer* outputImage);

extern "C" int32_t ColorSpaceConverterComposeImage(int32_t instance, OHNativeWindowBuffer* inputSdrImage,
    OHNativeWindowBuffer* inputGainmap, OHNativeWindowBuffer* outputHdrImage, bool legacy);

extern "C" int32_t ColorSpaceConverterDecomposeImage(int32_t instance, OHNativeWindowBuffer* inputImage,
    OHNativeWindowBuffer* outputSdrImage, OHNativeWindowBuffer* outputGainmap);

extern "C" int32_t ColorSpaceConverterDestroy(int32_t* instance);

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // INTERFACES_INNER_API_ALGORITHM_COLORSPACE_CONVERTER_H
