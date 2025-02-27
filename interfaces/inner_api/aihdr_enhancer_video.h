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

#ifndef AIHDR_ENHANCER_VIDEO_H
#define AIHDR_ENHANCER_VIDEO_H

#include <string>
#include <stdio.h>

#include "external_window.h"

#include "aihdr_enhancer_video_common.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
// typedef struct Surface Surface;

class __attribute__((visibility("default"))) AihdrEnhancerVideo {
public:
    static std::shared_ptr<AihdrEnhancerVideo> Create();
    virtual ~AihdrEnhancerVideo() = default;
    /* *
     * @brief Registers a AihdrEnhancerVideo callback.
     *
     * This function must be called before {@link Prepare}
     *
     * @param callback Indicates the callback to register. For details, see {@link AihdrEnhancerVideoCallback}.
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 5.0
     */
    virtual int32_t SetCallback(const std::shared_ptr<AihdrEnhancerVideoCallback> &callback) = 0;

    /* *
     * @brief Sets the window on which to render the output of this AihdrEnhancerVideo.
     *
     * This function must be called before {@link Prepare}
     *
     * @param window The output window.
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 5.0
     */
    virtual int32_t SetSurface(const OHNativeWindow* window) = 0;

    /* *
     * @brief Obtains the surface from AihdrEnhancerVideo.
     *
     * This function can only be called before {@link Prepare}
     *
     * @return Returns the pointer to the surface.
     * @since 5.0
     */
    virtual int32_t GetSurface(OHNativeWindow** window) = 0;

    /* *
     * @brief Configure the AihdrEnhancerVideo.
     *
     * @param window The input window.
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 5.0
     */
    virtual int32_t Configure() = 0;

    /* *
     * @brief Prepare for AihdrEnhancerVideo.
     *
     * This function must be called before {@link Start}
     *
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 5.0
     */
    virtual int32_t Prepare() = 0;

    /* *
     * @brief Start AihdrEnhancerVideo.
     *
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 5.0
     */
    virtual int32_t Start() = 0;

    /* *
     * @brief Stop AihdrEnhancerVideo.
     *
     * This function must be called during running
     *
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 5.0
     */
    virtual int32_t Stop() = 0;

    /* *
     * @brief Restores the AihdrEnhancerVideo to the initial state.
     *
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 5.0
     */
    virtual int32_t Reset() = 0;

    /* *
     * @brief Releases AihdrEnhancerVideo resources. All methods are unavailable after calling this.
     *
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 5.0
     */
    virtual int32_t Release() = 0;

    /* *
     * @brief Notify eos of the AihdrEnhancerVideo.
     *
     * @return Returns {@link VPE_ALGO_ERR_OK} if success; returns an error code otherwise.
     * @since 5.0
     */
    virtual int32_t NotifyEos() = 0;

    /* *
     * @brief Returns the output buffer to the AihdrEnhancerVideo.
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
};
using ArgumentType = void;
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
#endif // AIHDR_ENHANCER_VIDEO_H