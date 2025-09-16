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

#ifndef METADATA_GENERATOR_VIDEO_H
#define METADATA_GENERATOR_VIDEO_H
#include <string>
#include "surface.h"
#include "metadata_generator_video_common.h"
#include "algorithm_common.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class __attribute__((visibility("default"))) MetadataGeneratorVideo {
public:
    static std::shared_ptr<MetadataGeneratorVideo> Create();
    static std::shared_ptr<MetadataGeneratorVideo> Create(std::shared_ptr<OpenGLContext> openglContext);
    virtual ~MetadataGeneratorVideo() = default;
    /* *
     * @brief Registers a MetadataGeneratorVideo callback.
     *
     * This function must be called before {@link Prepare}
     *
     * @param callback Indicates the callback to register. For details, see {@link MetadataGeneratorVideoCallback}.
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 5.0
     */
    virtual int32_t SetCallback(const std::shared_ptr<MetadataGeneratorVideoCallback> &callback) = 0;

    /* *
     * @brief Sets the surface on which to render the output of this MetadataGeneratorVideo.
     *
     * This function must be called before {@link Prepare}
     *
     * @param surface The output surface.
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 5.0
     */
    virtual int32_t SetOutputSurface(sptr<Surface> surface) = 0;

    /* *
     * @brief Obtains the surface from MetadataGeneratorVideo.
     *
     * This function can only be called before {@link Prepare}
     *
     * @return Returns the pointer to the surface.
     * @since 5.0
     */
    virtual sptr<Surface> CreateInputSurface() = 0;

    /* *
     * @brief Configure the MetadataGeneratorVideo.
     *
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 5.0
     */
    virtual int32_t Configure() = 0;

    /* *
     * @brief Prepare for MetadataGeneratorVideo.
     *
     * This function must be called before {@link Start}
     *
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 5.0
     */
    virtual int32_t Prepare() = 0;

    /* *
     * @brief Start MetadataGeneratorVideo.
     *
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 5.0
     */
    virtual int32_t Start() = 0;

    /* *
     * @brief Stop MetadataGeneratorVideo.
     *
     * This function must be called during running
     *
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 5.0
     */
    virtual int32_t Stop() = 0;

    /* *
     * @brief Restores the MetadataGeneratorVideo to the initial state.
     *
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 5.0
     */
    virtual int32_t Reset() = 0;

    /* *
     * @brief Releases MetadataGeneratorVideo resources. All methods are unavailable after calling this.
     *
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 5.0
     */
    virtual int32_t Release() = 0;

    /* *
     * @brief Notify eos of the MetadataGeneratorVideo.
     *
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 5.0
     */
    virtual int32_t NotifyEos() = 0;

    /* *
     * @brief Returns the output buffer to the MetadataGeneratorVideo.
     *
     * This function must be called during running
     *
     * @param index The index of the output buffer.
     * @param render Whether to render the buffer.
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 5.0
     */
    virtual int32_t ReleaseOutputBuffer(uint32_t index, bool render) = 0;

    virtual int32_t Flush() = 0;

    virtual VPEAlgoErrCode SetParameter(const MetadataGeneratorParameter& parameter) = 0;

    virtual VPEAlgoErrCode GetParameter(MetadataGeneratorParameter& parameter) = 0;
};
using ArgumentType = void;
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
#endif // METADATA_GENERATOR_VIDEO_H