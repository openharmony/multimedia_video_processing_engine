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

#include "aihdr_enhancer_fwk.h"

#include "native_buffer.h"
#include "surface_buffer.h"

#include "extension_manager.h"
#include "video_processing_client.h"
#include "vpe_log.h"
#include "vpe_trace.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
AihdrEnhancerFwk::AihdrEnhancerFwk()
{
    Extension::ExtensionManager::GetInstance().IncreaseInstance();
}

AihdrEnhancerFwk::~AihdrEnhancerFwk()
{
    if (impl_) {
        impl_->Deinit();
        impl_ = nullptr;
    }
    Extension::ExtensionManager::GetInstance().DecreaseInstance();
}

VPEAlgoErrCode AihdrEnhancerFwk::SetParameter(const int& parameter)
{
    parameter_ = parameter;
    VPE_LOGI("AihdrEnhancerFwk SetParameter Succeed");
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode AihdrEnhancerFwk::GetParameter(int& parameter) const
{
    parameter = parameter_;
    VPE_LOGI("AihdrEnhancerFwk GetParameter Succeed");
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode AihdrEnhancerFwk::Process(const sptr<SurfaceBuffer>& input)
{
    CHECK_AND_RETURN_RET_LOG(input != nullptr, VPE_ALGO_ERR_INVALID_VAL, "Input is nullptr");

    VPEAlgoErrCode ret = Init(input);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, ret, "Init failed");

    VPE_SYNC_TRACE;
    ret = impl_->Process(input);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, ret, "Process failed, ret: %{public}d", ret);

    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode AihdrEnhancerFwk::Init(const sptr<SurfaceBuffer>& input)
{
    if (initialized_) {
        impl_->SetParameter(parameter_);
        return VPE_ALGO_ERR_OK;
    }
    auto &manager = Extension::ExtensionManager::GetInstance();

    VPE_SYNC_TRACE;

    FrameInfo info(input);
    impl_ = manager.CreateAihdrEnhancer(info, extensionInfo_);
    CHECK_AND_RETURN_RET_LOG(impl_ != nullptr, VPE_ALGO_ERR_NOT_IMPLEMENTED, "Create failed");

    int32_t ret = impl_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, VPE_ALGO_ERR_NOT_IMPLEMENTED, "Init failed");
    impl_->SetParameter(parameter_);
    initialized_ = true;
    VPE_LOGI("AihdrEnhancerFwk Init Succeed");

    return VPE_ALGO_ERR_OK;
}

std::shared_ptr<AihdrEnhancer> AihdrEnhancer::Create()
{
    auto p = std::make_shared<AihdrEnhancerFwk>();
    CHECK_AND_RETURN_RET_LOG(p != nullptr, nullptr, "Create AihdrEnhancer failed");
    return std::static_pointer_cast<AihdrEnhancer>(p);
}

int32_t AihdrEnhancerCreate(int32_t* instance)
{
    CHECK_AND_RETURN_RET_LOG(instance != nullptr, VPE_ALGO_ERR_INVALID_VAL, "invalid instance");
    auto p = AihdrEnhancer::Create();
    CHECK_AND_RETURN_RET_LOG(p != nullptr, VPE_ALGO_ERR_INVALID_VAL, "cannot create instance");
    Extension::ExtensionManager::InstanceVariableType instanceVar { p };
    int32_t newId = Extension::ExtensionManager::GetInstance().NewInstanceId(instanceVar);
    CHECK_AND_RETURN_RET_LOG(newId != -1, VPE_ALGO_ERR_NO_MEMORY, "cannot create more instance");
    *instance = newId;
    return VPE_ALGO_ERR_OK;
}

int32_t AihdrEnhancerProcessImage(int32_t instance, OHNativeWindowBuffer* inputImage)
{
    CHECK_AND_RETURN_RET_LOG((inputImage != nullptr), VPE_ALGO_ERR_INVALID_VAL,
        "invalid parameters");
    auto someInstance = Extension::ExtensionManager::GetInstance().GetInstance(instance);
    CHECK_AND_RETURN_RET_LOG(someInstance != std::nullopt, VPE_ALGO_ERR_INVALID_VAL, "invalid instance");
 
    VPEAlgoErrCode ret = VPE_ALGO_ERR_INVALID_VAL;
    auto visitFunc = [inputImage, &ret](auto&& var) {
        using VarType = std::decay_t<decltype(var)>;
        if constexpr (std::is_same_v<VarType, std::shared_ptr<AihdrEnhancer>>) {
            OH_NativeBuffer* inputImageNativeBuffer = nullptr;
            CHECK_AND_RETURN_LOG(
                (OH_NativeBuffer_FromNativeWindowBuffer(inputImage, &inputImageNativeBuffer) == GSERROR_OK),
                "invalid input image");
            sptr<SurfaceBuffer> inputImageSurfaceBuffer(
                SurfaceBuffer::NativeBufferToSurfaceBuffer(inputImageNativeBuffer));
            ret = var->Process(inputImageSurfaceBuffer);
        } else {
            VPE_LOGE("instance may be miss used");
        }
    };
    std::visit(visitFunc, *someInstance);
 
    return ret;
}

int32_t AihdrEnhancerDestroy(int32_t* instance)
{
    CHECK_AND_RETURN_RET_LOG(instance != nullptr, VPE_ALGO_ERR_INVALID_VAL, "instance is null");
    return Extension::ExtensionManager::GetInstance().RemoveInstanceReference(*instance);
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
