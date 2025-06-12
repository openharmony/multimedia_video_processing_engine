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

#ifndef INTERFACES_INNER_API_ALGORITHM_VIDEO_H
#define INTERFACES_INNER_API_ALGORITHM_VIDEO_H

#include <cinttypes>
#include <memory>

#include "meta/format.h"
#include "refbase.h"
#include "surface.h"

#include "algorithm_errors.h"
#include "algorithm_video_common.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class __attribute__((visibility("default"))) VpeVideo {
public:
    /**
     * @brief Create a VpeVideo object.
     * @param type Use VIDEO_TYPE_XXX to specify the processing type. For details, see {@link VpeVideoType}.
     * @return If successful, returns a pointer to the VpeVideo object. On error, returns a null pointer.
     * @since 5.1
     * @version 5.1
     */
    static std::shared_ptr<VpeVideo> Create(uint32_t type);

    /**
     * @brief Query whether the feature altorithm is supported.
     * @param type Use VIDEO_TYPE_XXX to specify the processing type. For details, see {@link VpeVideoType}.
     * @param parameter The parameter of video processing.
     * @return true if the feature altorithm is supported. false If the feature altorithm is unsupported.
     * @since 5.1
     * @version 5.1
     */
    static bool IsSupported(uint32_t type, const Format& parameter);

    /**
     * @brief Query whether the product is supported.
     * @param type Use VIDEO_TYPE_XXX to specify the processing type. For details, see {@link VpeVideoType}.
     * @param parameter The parameter of video processing.
     * @return true if the product is supported. false If the product is unsupported.
     * @since 6.0
     */
    static bool IsSupported(void);

    /**
     * @brief Register callback object.
     * @param callback Callback object to be registered. For details, see {@link VpeVideoCallback}.
     * @return If successful, returns {@link VPE_ALGO_ERR_OK}. On error, retuns an error code.
     * @since 5.1
     * @version 5.1
     */
    virtual VPEAlgoErrCode RegisterCallback(const std::shared_ptr<VpeVideoCallback>& callback);

    /**
     * @brief Set the output surface for video processing.
     * @param surface The output surface object.
     * @return If successful, returns {@link VPE_ALGO_ERR_OK}. On error, retuns an error code.
     * @since 5.1
     * @version 5.1
     */
    virtual VPEAlgoErrCode SetOutputSurface(const sptr<Surface>& surface);

    /**
     * @brief Create an input surface.
     * @return If successful, returns a pointer to the input surface object. On error, returns a null pointer.
     * @since 5.1
     * @version 5.1
     */
    virtual sptr<Surface> GetInputSurface();

    /**
     * @brief Set parameter for video processing. Add parameter identified by the specified parameter key.
     * @param parameter The parameter for video processing.
     * @return If successful, returns {@link VPE_ALGO_ERR_OK}. On error, retuns an error code.
     * @since 5.1
     * @version 5.1
     */
    virtual VPEAlgoErrCode SetParameter(const Format& parameter);

    /**
     * @brief Get parameter of video processing. Get parameter identified by the specified parameter key.
     * @param parameter The parameter of video processing.
     * @return If successful, returns {@link VPE_ALGO_ERR_OK}. On error, retuns an error code.
     * @since 5.1
     * @version 5.1
     */
    virtual VPEAlgoErrCode GetParameter(Format& parameter);

    /**
     * @brief Start video processing.
     * @return If successful, returns {@link VPE_ALGO_ERR_OK}. On error, retuns an error code.
     * @since 5.1
     * @version 5.1
     */
    virtual VPEAlgoErrCode Start();

    /**
     * @brief Stop video processing.
     *
     * This method must be called during running.
     *
     * @return If successful, returns {@link VPE_ALGO_ERR_OK}. On error, retuns an error code.
     * @since 5.1
     * @version 5.1
     */
    virtual VPEAlgoErrCode Stop();

    /**
     * @brief Releases all resources before destructor. All methods are unavailable after calling this.
     * @return If successful, returns {@link VPE_ALGO_ERR_OK}. On error, retuns an error code.
     * @since 5.1
     * @version 5.1
     */
    virtual VPEAlgoErrCode Release();

    /**
     * @brief Flush both input and output buffers for video processing.
     *
     * This method must be called during running.
     *
     * @return If successful, returns {@link VPE_ALGO_ERR_OK}. On error, retuns an error code.
     * @since 5.1
     * @version 5.1
     */
    virtual VPEAlgoErrCode Flush();

    /**
     * @brief Enable video processing effect. It only can be called after {@link Disable}.
     *
     * This method must be called during running.
     *
     * @return If successful, returns {@link VPE_ALGO_ERR_OK}. On error, retuns an error code.
     * @since 5.1
     * @version 5.1
     */
    virtual VPEAlgoErrCode Enable();

    /**
     * @brief Disable video processing effect. If video processing is disabled, then the processor
     * sends the surfacebuffer of the input surface to the output surface directly.
     *
     * This method must be called during running.
     *
     * @return If successful, returns {@link VPE_ALGO_ERR_OK}. On error, retuns an error code.
     * @since 5.1
     * @version 5.1
     */
    virtual VPEAlgoErrCode Disable();

    /**
     * @brief Notify the end of stream.
     *
     * This method must be called during running.
     *
     * @return If successful, returns {@link VPE_ALGO_ERR_OK}. On error, retuns an error code.
     * @since 5.1
     * @version 5.1
     */
    virtual VPEAlgoErrCode NotifyEos();

    /**
     * @brief Returns the output buffer to video processing.
     *
     * This method must be called during running.
     *
     * @param index The index of the output buffer.
     * @param render Whether to render the buffer.
     * @return If successful, returns {@link VPE_ALGO_ERR_OK}. On error, retuns an error code.
     * @since 5.1
     * @version 5.1
     */
    virtual VPEAlgoErrCode ReleaseOutputBuffer(uint32_t index, bool render);

    /**
     * @brief Send the output buffer out.
     *
     * This method must be called during running.
     *
     * @param index The index of the output buffer.
     * @param renderTimestamp The timestamp is associated with the output buffer when it is sent to the surface.
     * @return If successful, returns {@link VPE_ALGO_ERR_OK}. On error, retuns an error code.
     * @since 5.1
     * @version 5.1
     */
    virtual VPEAlgoErrCode RenderOutputBufferAtTime(uint32_t index, int64_t renderTimestamp);

protected:
    VpeVideo() = default;
    virtual ~VpeVideo() = default;
    VpeVideo(const VpeVideo&) = delete;
    VpeVideo& operator=(const VpeVideo&) = delete;
    VpeVideo(VpeVideo&&) = delete;
    VpeVideo& operator=(VpeVideo&&) = delete;
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // INTERFACES_INNER_API_ALGORITHM_VIDEO_H
