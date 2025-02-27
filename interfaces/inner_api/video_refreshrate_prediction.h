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

#ifndef INTERFACES_INNER_API_VPE_VIDEO_REFRESHRATE_PREDICTION_H
#define INTERFACES_INNER_API_VPE_VIDEO_REFRESHRATE_PREDICTION_H

#include <memory>

#include "algorithm_errors.h"
#include "external_window.h"
#include "refbase.h"
#include "surface_buffer.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

enum MotionVectorType : int32_t {
    MOTIONVECTOR_TYPE_NONE  = 0,
    MOTIONVECTOR_TYPE_AVC  = 1,
    MOTIONVECTOR_TYPE_HEVC = 2
};

class __attribute__((visibility("default"))) VideoRefreshRatePrediction {
public:
    /**
     * @brief Create a VideoRefreshRatePrediction object.
     * @syscap
     * @return pointer of the VideoRefreshRatePrediction object.
     * @since 13
     */
    static std::shared_ptr<VideoRefreshRatePrediction> Create();

    /**
     * @brief 硬件LTPO支持校验，调用方检查返回值
     * @syscap
     * @return 返回错误码VPEAlgoErrCode
     * @since 13
     */
    virtual VPEAlgoErrCode CheckVRRSupport(std::string processName) = 0;

    /**
     * @brief 执行视频可变帧率算法
     * @syscap
     * @param input 输入的解码帧，算法将决策的帧率信息写入SurfaceBuffer的ExtraData中
     * @param videoFps 输入视频帧的帧率
     * @param codecType 输入视频帧编码格式 MotionVectorType
     * @return 返回错误码VPEAlgoErrCode
     * @since 13
     */
    virtual VPEAlgoErrCode Process(const sptr<SurfaceBuffer>& input, int videoFps, int codecType) = 0;
protected:
    virtual ~VideoRefreshRatePrediction() = default;
};

#ifdef __cplusplus
extern "C" {
#endif

using VideoRefreshRatePredictionHandle = void;

VideoRefreshRatePredictionHandle *VideoRefreshRatePredictionCreate();
void VideoRefreshRatePredictionDestroy(VideoRefreshRatePredictionHandle *handle);
int32_t VideoRefreshRatePredictionCheckSupport(VideoRefreshRatePredictionHandle *handle, const char *processName);
void VideoRefreshRatePredictionProcess(VideoRefreshRatePredictionHandle *handle,
    OH_NativeBuffer* inputImageNativeBuffer, int videoFps, int codecType);

#ifdef __cplusplus
}
#endif

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // INTERFACES_INNER_API_VPE_VIDEO_REFRESHRATE_PREDICTION_H
