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

#ifndef COLORSPACE_CONVERTER_VIDEO_H
#define COLORSPACE_CONVERTER_VIDEO_H
#include <string>
#include "surface.h"
#include "meta/format.h"
#include "colorspace_converter_video_common.h"
#include "algorithm_common.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class __attribute__((visibility("default"))) ColorSpaceConverterVideo {
public:
    static std::shared_ptr<ColorSpaceConverterVideo> Create();
    static std::shared_ptr<ColorSpaceConverterVideo> Create(std::shared_ptr<OpenGLContext> openglContext);
    virtual ~ColorSpaceConverterVideo() = default;
    /* *
     * @brief Registers a ColorSpaceConverterVideo callback.
     *
     * This function must be called before {@link Prepare}
     *
     * @param callback Indicates the callback to register. For details, see {@link ColorSpaceConverterVideoCallback}.
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 4.1
     */
    virtual int32_t SetCallback(const std::shared_ptr<ColorSpaceConverterVideoCallback> &callback) = 0;

    /* *
     * @brief Sets the surface on which to render the output of this ColorSpaceConverterVideo.
     *
     * This function must be called before {@link Prepare}
     *
     * @param surface The output surface.
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 4.1
     */
    virtual int32_t SetOutputSurface(sptr<Surface> surface) = 0;

    /* *
     * @brief Obtains the surface from ColorSpaceConverterVideo.
     *
     * This function can only be called before {@link Prepare}
     *
     * @return Returns the pointer to the surface.
     * @since 4.1
     */
    virtual sptr<Surface> CreateInputSurface() = 0;

    /* *
     * @brief Sets the parameters to the ColorSpaceConverterVideo.
     *
     * This function must be called after {@link Prepare}
     *
     * @param parameter
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 4.1
     */
    virtual int32_t SetParameter(const Format &parameter) = 0;

    /* *
     * @brief Gets the parameters of the ColorSpaceConverterVideo.
     *
     * This function must be called after {@link Prepare}
     *
     * @param parameter
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 4.1
     */
    virtual int32_t GetParameter(Format &parameter) = 0;

    /* *
     * @brief Configure the ColorSpaceConverterVideo.
     *
     * @param format The format of the input data and the desired format of the output data.
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 4.1
     */
    virtual int32_t Configure(const Format &format) = 0;

    /* *
     * @brief Prepare for ColorSpaceConverterVideo.
     *
     * This function must be called before {@link Start}
     *
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 4.1
     */
    virtual int32_t Prepare() = 0;

    /* *
     * @brief Start ColorSpaceConverterVideo.
     *
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 4.1
     */
    virtual int32_t Start() = 0;

    /* *
     * @brief Stop ColorSpaceConverterVideo.
     *
     * This function must be called during running
     *
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 4.1
     */
    virtual int32_t Stop() = 0;

    /* *
     * @brief Restores the ColorSpaceConverterVideo to the initial state.
     *
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 4.1
     */
    virtual int32_t Reset() = 0;

    /* *
     * @brief Releases ColorSpaceConverterVideo resources. All methods are unavailable after calling this.
     *
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 4.1
     */
    virtual int32_t Release() = 0;

    /* *
     * @brief Notify eos of the ColorSpaceConverterVideo.
     *
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 4.1
     */
    virtual int32_t NotifyEos() = 0;

    /* *
     * @brief Returns the output buffer to the ColorSpaceConverterVideo.
     *
     * This function must be called during running
     *
     * @param index The index of the output buffer.
     * @param render Whether to render the buffer.
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 4.1
     */
    virtual int32_t ReleaseOutputBuffer(uint32_t index, bool render) = 0;

    virtual int32_t Flush() = 0;
    /* *
     * @brief Gets the format of the output data.
     *
     * @param format Obtain the required output data format.
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 5.0
     */
    virtual int32_t GetOutputFormat(Format &format) = 0;
};

#ifdef __cplusplus
extern "C" {
#endif

using ColorSpaceConvertVideoHandle = void;
using ArgumentType = void;

int32_t ColorSpaceConvertVideoIsColorSpaceConversionSupported(const ArgumentType* input, const ArgumentType* output);
ColorSpaceConvertVideoHandle* ColorSpaceConvertVideoCreate();
void ColorSpaceConvertVideoDestroy(ColorSpaceConvertVideoHandle* handle);
int32_t ColorSpaceConvertVideoSetCallback(ColorSpaceConvertVideoHandle* handle, ArgumentType* callback,
    ArgumentType* userData);
int32_t ColorSpaceConvertVideoSetOutputSurface(ColorSpaceConvertVideoHandle* handle, ArgumentType* surface);
int32_t ColorSpaceConvertVideoCreateInputSurface(ColorSpaceConvertVideoHandle* handle, ArgumentType* surface);
int32_t ColorSpaceConvertVideoSetParameter(ColorSpaceConvertVideoHandle* handle, ArgumentType* parameter);
int32_t ColorSpaceConvertVideoGetParameter(ColorSpaceConvertVideoHandle* handle, ArgumentType* parameter);
int32_t ColorSpaceConvertVideoConfigure(ColorSpaceConvertVideoHandle* handle, ArgumentType* configuration);
int32_t ColorSpaceConvertVideoPrepare(ColorSpaceConvertVideoHandle* handle);
int32_t ColorSpaceConvertVideoStart(ColorSpaceConvertVideoHandle* handle);
int32_t ColorSpaceConvertVideoStop(ColorSpaceConvertVideoHandle* handle);
int32_t ColorSpaceConvertVideoFlush(ColorSpaceConvertVideoHandle* handle);
int32_t ColorSpaceConvertVideoReset(ColorSpaceConvertVideoHandle* handle);
int32_t ColorSpaceConvertVideoRelease(ColorSpaceConvertVideoHandle* handle);
int32_t ColorSpaceConvertVideoNotifyEos(ColorSpaceConvertVideoHandle* handle);
int32_t ColorSpaceConvertVideoReleaseOutputBuffer(ColorSpaceConvertVideoHandle* handle, uint32_t index, bool render);
int32_t ColorSpaceConvertVideoGetOutputFormat(ColorSpaceConvertVideoHandle* handle, ArgumentType* format);
int32_t ColorSpaceConvertVideoOnProducerBufferReleased(ColorSpaceConvertVideoHandle *handle);

#ifdef __cplusplus
}
#endif

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
#endif // COLORSPACE_CONVERTER_VIDEO_H