/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_INNER_API_ALGORITHM_ALGORITHM_VIDEO_COMMON_H
#define INTERFACES_INNER_API_ALGORITHM_ALGORITHM_VIDEO_COMMON_H

#include <cstdint>
#include <memory>
#include <string_view>

#include "meta/format.h"

#include "algorithm_errors.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
enum class VPEAlgoState : int32_t {
    UNINITIALIZED,
    INITIALIZED,
    CONFIGURING,
    CONFIGURED,
    STOPPED,
    RUNNING,
    EOS,
    FLUSHED,
    ERROR,
};

/**
 * @brief Feature type of video processing.
 *
 * @since 5.1
 * @version 5.1
 */
enum VpeVideoType : uint32_t {
    /**
     * @brief Used to create an video processing object of detail enhancement.
     *
     * Scale or resize video with the specified quality or just enhance details for rendering without changing its
     * resolution.
     *
     * @since 5.1
     * @version 5.1
     */
    VIDEO_TYPE_DETAIL_ENHANCER = 0x4,
    VIDEO_TYPE_AIHDR_ENHANCER = 0x8,
};

/**
 * @brief Flag of video processing buffer.
 *
 * @since 5.1
 * @version 5.1
 */
enum VpeBufferFlag : uint32_t {
    VPE_BUFFER_FLAG_NONE = 0,
    /** This signals the end of stream */
    VPE_BUFFER_FLAG_EOS = 1 << 0,
};

/**
 * @brief Information of video processing buffer.
 *
 * @since 5.1
 * @version 5.1
 */
struct VpeBufferInfo {
    /** The flag of the available output buffer. For details, see {@link VpeBufferFlag}. */
    VpeBufferFlag flag{VPE_BUFFER_FLAG_NONE};
    /** presentationTimestamp The presentation timestamp for the buffer. */
    int64_t presentationTimestamp{-1};
};

/**
 * @brief Video processing callback base class, you can inherited it and only override partial methods as needed.
 *
 * @since 5.1
 * @version 5.1
 */
class __attribute__((visibility("default"))) VpeVideoCallback {
public:
    /**
     * Called when an error occurred.
     *
     * @param errorCode Error code. For details, see {@link VPEAlgoErrCode}.
     * @since 5.1
     * @version 5.1
     */
    virtual void OnError(VPEAlgoErrCode errorCode);

    /**
     * Called when switch new state.
     *
     * @param state Current state. For details, see {@link VPEAlgoState}.
     * @since 5.1
     * @version 5.1
     */
    virtual void OnState(VPEAlgoState state);

    /**
     * Called when one of the features enable or disable effect.
     *
     * @param type Current enable processing type. For details, see {@link VpeVideoType}.
     * If type is 0, no effect is enabled now.
     * @since 5.1
     * @version 5.1
     */
    virtual void OnEffectChange(uint32_t type);

    /**
     * Called when an output format changed.
     *
     * @param format Output surfacebuffer format.
     * @since 5.1
     * @version 5.1
     */
    virtual void OnOutputFormatChanged(const Format& format);

    /**
     * Called when an output buffer becomes available.
     *
     * @param index The index of the available output buffer.
     * @param flag The flag of the available output buffer. For details, see {@link VpeBufferFlag}.
     * @since 5.1
     * @version 5.1
     */
    virtual void OnOutputBufferAvailable(uint32_t index, VpeBufferFlag flag);

    /**
     * Called when an output buffer becomes available.
     *
     * @param index The index of the available output buffer.
     * @param info The information of the available output buffer. For details, see {@link VpeBufferInfo}.
     * @since 5.1
     * @version 5.1
     */
    virtual void OnOutputBufferAvailable(uint32_t index, const VpeBufferInfo& info);

protected:
    VpeVideoCallback() = default;
    virtual ~VpeVideoCallback() = default;
    VpeVideoCallback(const VpeVideoCallback&) = delete;
    VpeVideoCallback& operator=(const VpeVideoCallback&) = delete;
    VpeVideoCallback(VpeVideoCallback&&) = delete;
    VpeVideoCallback& operator=(VpeVideoCallback&&) = delete;
};

/**
 * @brief Width and height of the image buffer.
 *
 * It is the value of the key parameter {@link ParameterKey::DETAIL_ENHANCER_TARGET_SIZE}.
 *
 * @see VpeVideo::SetParameter
 * @see VpeVideo::GetParameter
 * @since 5.1
 * @version 5.1
 */
struct VpeBufferSize {
    int width{};
    int height{};
};

/**
 * @brief The quality level is used for detail enhancement.
 *
 * It is the value of the key parameter {@link ParameterKey::DETAIL_ENHANCER_QUALITY_LEVEL}.
 *
 * @see VpeVideo::SetParameter
 * @see VpeVideo::GetParameter
 * @since 5.1
 * @version 5.1
 */
enum DetailEnhancerQualityLevel {
    /** No detail enhancement */
    DETAIL_ENHANCER_LEVEL_NONE = 0,
    /** A low level of detail enhancement quality but with a fast speed. It's the default level */
    DETAIL_ENHANCER_LEVEL_LOW,
    /** A medium level of detail enhancement quality. Its speed is between the low setting and high setting */
    DETAIL_ENHANCER_LEVEL_MEDIUM,
    /** A high level of detail enhancement quality but with a relatively slow speed */
    DETAIL_ENHANCER_LEVEL_HIGH,
};

/**
 * @brief Contains the key corresponding to each paramter value.
 *
 * @see VpeVideo::SetParameter
 * @see VpeVideo::GetParameter
 * @since 5.1
 * @version 5.1
 */
class ParameterKey {
public:
    /**
     * @brief The key is used to specify the quality level for video detail enhancement.
     *
     * See {@link DetailEnhancerQualityLevel} for its values.
     * Use {@link VpeVideo::SetParameter} and {@link Format::SetIntValue} to set the quality level.
     * Use {@link VpeVideo::GetParameter} and {@link Format::GetIntValue} to get the current quality level.
     *
     * @since 5.1
     * @version 5.1
     */
    static constexpr std::string_view DETAIL_ENHANCER_QUALITY_LEVEL{"QualityLevel"};

    /**
     * @brief The key is used to specify width and height of the target image.
     *
     * See {@link VpeBufferSize} for its values.
     * Use {@link VpeVideo::SetParameter} and {@link Format::SetBuffer} to set the size of the target image.
     * Use {@link VpeVideo::GetParameter} and {@link Format::GetBuffer} to get the current size of the target image.
     *
     * @since 5.1
     * @version 5.1
     */
    static constexpr std::string_view DETAIL_ENHANCER_TARGET_SIZE{"TargetSize"};

    /**
     * @brief The key is used to specify whether automatically downshift the quality level for detail
     * enhancement or not. Default value is true.
     *
     * Use {@link VpeVideo::SetParameter} and {@link Format::SetIntValue} to set whether automatic downshift or not.
     * Use {@link VpeVideo::GetParameter} and {@link Format::GetIntValue} to get whether automatic downshift or not.
     *
     * @since 5.1
     * @version 5.1
     */
    static constexpr std::string_view DETAIL_ENHANCER_AUTO_DOWNSHIFT{"AutoDownshift"};

    /**
     * @brief The key is used to identifies surface nodeId
     * enhancement or not. Default value is true.
     *
     * Use {@link VpeVideo::SetParameter} and {@link Format::SetLongValue} to set whether automatic downshift or not.
     * Use {@link VpeVideo::GetParameter} and {@link Format::GetLongValue} to get whether automatic downshift or not.
     *
     * @since 5.1
     * @version 5.1
     */
    static constexpr std::string_view DETAIL_ENHANCER_NODE_ID{"NodeId"};

private:
    ParameterKey() = delete;
    ~ParameterKey() = delete;
    ParameterKey(const ParameterKey&) = delete;
    ParameterKey& operator=(const ParameterKey&) = delete;
    ParameterKey(ParameterKey&&) = delete;
    ParameterKey& operator=(ParameterKey&&) = delete;
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // INTERFACES_INNER_API_ALGORITHM_ALGORITHM_VIDEO_COMMON_H
