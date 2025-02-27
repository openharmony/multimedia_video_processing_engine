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

#include "video_refreshrate_prediction_fwk.h"
#include "extension_manager.h"
#include "surface_buffer.h"
#include "vpe_trace.h"
#include "vpe_log.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
VideoRefreshRatePredictionFwk::VideoRefreshRatePredictionFwk()
{
    Extension::ExtensionManager::GetInstance().IncreaseInstance();
}

VideoRefreshRatePredictionFwk::~VideoRefreshRatePredictionFwk()
{
    impl_ = nullptr;
    Extension::ExtensionManager::GetInstance().DecreaseInstance();
}

VPEAlgoErrCode VideoRefreshRatePredictionFwk::Process(const sptr<SurfaceBuffer> &input, int videoFps, int codecType)
{
    VPEAlgoErrCode ret = Init();
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, ret, "VRR Init failed");
    ret = impl_->Process(input, videoFps, codecType);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, ret, "Process failed, ret: %{public}d", ret);
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode VideoRefreshRatePredictionFwk::CheckVRRSupport(std::string processName)
{
    VPEAlgoErrCode ret = Init();
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, ret, "VRR Init failed");
    ret = impl_->CheckVRRSupport(processName);
    return ret;
}

VPEAlgoErrCode VideoRefreshRatePredictionFwk::Init()
{
    if (initialized_) {
        return VPE_ALGO_ERR_OK;
    }
    auto& manager = Extension::ExtensionManager::GetInstance();
    VPE_SYNC_TRACE;
    impl_ = manager.CreateVideoRefreshRatePredictor();
    CHECK_AND_RETURN_RET_LOG(impl_ != nullptr, VPE_ALGO_ERR_NOT_IMPLEMENTED, "Create impl failed");
    initialized_ = true;
    VPE_LOGI("create VideoRefreshRatePredictionFwk Successed");
    return VPE_ALGO_ERR_OK;
}


std::shared_ptr<VideoRefreshRatePrediction> VideoRefreshRatePrediction::Create()
{
    auto p = std::make_shared<VideoRefreshRatePredictionFwk>();
    CHECK_AND_RETURN_RET_LOG(p != nullptr, nullptr, "Create VideoRefreshRatePrediction failed");
    return std::static_pointer_cast<VideoRefreshRatePrediction>(p);
}

struct VideoRefreshRatePredictionHandleImpl {
    std::shared_ptr<VideoRefreshRatePredictionFwk> obj;
};

VideoRefreshRatePredictionHandle *VideoRefreshRatePredictionCreate()
{
    auto impl = std::make_shared<VideoRefreshRatePredictionFwk>();
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr, "Create VideoRefreshRatePrediction failed");
    auto handle = new VideoRefreshRatePredictionHandleImpl;
    handle->obj = impl;
    return static_cast<VideoRefreshRatePredictionHandle *>(handle);
}

void VideoRefreshRatePredictionDestroy(VideoRefreshRatePredictionHandle *handle)
{
    VPE_LOGD("VideoRefreshRatePredictionFwk Destroy");
    if (handle != nullptr) {
        auto p = static_cast<VideoRefreshRatePredictionHandleImpl *>(handle);
        delete p;
    }
}

int32_t VideoRefreshRatePredictionCheckSupport(VideoRefreshRatePredictionHandle *handle, const char *processName)
{
    auto p = static_cast<VideoRefreshRatePredictionHandleImpl *>(handle);
    int32_t ret = p->obj->CheckVRRSupport(processName);
    return ret;
}

void VideoRefreshRatePredictionProcess(VideoRefreshRatePredictionHandle *handle,
    OH_NativeBuffer* inputImageNativeBuffer, int videoFps, int codecType)
{
    auto p = static_cast<VideoRefreshRatePredictionHandleImpl *>(handle);
    sptr<SurfaceBuffer> inputImageSurfaceBuffer(SurfaceBuffer::NativeBufferToSurfaceBuffer(inputImageNativeBuffer));
    p->obj->Process(inputImageSurfaceBuffer, videoFps, codecType);
}

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
