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

#include "metadata_generator_fwk.h"
#include "video_processing_client.h"
#include "extension_manager.h"
#include "native_buffer.h"
#include "surface_buffer.h"
#include "vpe_trace.h"
#include "vpe_log.h"
#include "EGL/egl.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
MetadataGeneratorFwk::MetadataGeneratorFwk()
{
    OpenGLInit();
    OHOS::Media::VideoProcessingEngine::VideoProcessingManager::GetInstance().Connect();
    VPE_LOGI("VPE Framework connect and load SA!");
    OHOS::Media::VideoProcessingEngine::VideoProcessingManager::GetInstance().Disconnect();
    Extension::ExtensionManager::GetInstance().IncreaseInstance();
}

void MetadataGeneratorFwk::OpenGLInit()
{
    context.glDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (context.glDisplay == EGL_NO_DISPLAY || eglGetError() != EGL_SUCCESS) {
        VPE_LOGE("MetadataGeneratorFwk Get display failed!");
    }
    EGLint major;
    EGLint minor;
    if (eglInitialize(context.glDisplay, &major, &minor) == EGL_FALSE || eglGetError() != EGL_SUCCESS) {
        VPE_LOGE("MetadataGeneratorFwk eglInitialize failed!");
    }
}

MetadataGeneratorFwk::MetadataGeneratorFwk(std::shared_ptr<OpenGLContext> openglContext)
{
    if (openglContext != nullptr) {
        context.glDisplay = openglContext->display;
    }
    OHOS::Media::VideoProcessingEngine::VideoProcessingManager::GetInstance().Connect();
    VPE_LOGI("VPE Framework connect and load SA!");
    OHOS::Media::VideoProcessingEngine::VideoProcessingManager::GetInstance().Disconnect();
    Extension::ExtensionManager::GetInstance().IncreaseInstance();
}

MetadataGeneratorFwk::~MetadataGeneratorFwk()
{
    if (impl_) {
        impl_->Deinit();
        impl_ = nullptr;
    }
    if (context.glDisplay != EGL_NO_DISPLAY) {
        eglTerminate(context.glDisplay);
    }
    Extension::ExtensionManager::GetInstance().DecreaseInstance();
}

VPEAlgoErrCode MetadataGeneratorFwk::SetParameter(const MetadataGeneratorParameter &parameter)
{
    parameter_ = parameter;
    VPE_LOGI("MetadataGeneratorFwk SetParameter Succeed");
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode MetadataGeneratorFwk::GetParameter(MetadataGeneratorParameter &parameter) const
{
    parameter = parameter_;
    VPE_LOGI("MetadataGeneratorFwk GetParameter Succeed");
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode MetadataGeneratorFwk::Process(const sptr<SurfaceBuffer> &input)
{
    CHECK_AND_RETURN_RET_LOG(input != nullptr, VPE_ALGO_ERR_INVALID_VAL, "Input is nullptr");
    CHECK_AND_RETURN_RET_LOG((input->GetUsage() & (BUFFER_USAGE_CPU_WRITE | BUFFER_USAGE_HW_RENDER)) != 0,
        VPE_ALGO_ERR_INVALID_VAL, "Input surfacebuffer usage need NATIVEBUFFER_USAGE_HW_RENDER");
    VPEAlgoErrCode ret = Init(input);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, ret, "Init failed");

    VPE_SYNC_TRACE;
    ret = impl_->Process(input);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, ret, "Process failed, ret: %{public}d", ret);

    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode MetadataGeneratorFwk::Init(const sptr<SurfaceBuffer> &input)
{
    if (initialized_) {
        impl_->SetParameter(parameter_);
        return VPE_ALGO_ERR_OK;
    }
    auto &manager = Extension::ExtensionManager::GetInstance();

    VPE_SYNC_TRACE;

    FrameInfo info(input);
    impl_ = manager.CreateMetadataGenerator(info, extensionInfo_, parameter_.algoType);
    CHECK_AND_RETURN_RET_LOG(impl_ != nullptr, VPE_ALGO_ERR_NOT_IMPLEMENTED, "Create failed");

    int32_t ret = impl_->Init(context);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, VPE_ALGO_ERR_NOT_IMPLEMENTED, "Init failed");
    impl_->SetParameter(parameter_);
    initialized_ = true;
    VPE_LOGI("MetadataGeneratorFwk Init Succeed");

    return VPE_ALGO_ERR_OK;
}

std::shared_ptr<MetadataGenerator> MetadataGenerator::Create()
{
    auto p = std::make_shared<MetadataGeneratorFwk>();
    CHECK_AND_RETURN_RET_LOG(p != nullptr, nullptr, "Create MetadataGenerator failed");
    return std::static_pointer_cast<MetadataGenerator>(p);
}

std::shared_ptr<MetadataGenerator> MetadataGenerator::Create(std::shared_ptr<OpenGLContext> openglContext)
{
    auto p = std::make_shared<MetadataGeneratorFwk>(openglContext);
    CHECK_AND_RETURN_RET_LOG(p != nullptr, nullptr, "Create MetadataGenerator failed");
    return std::static_pointer_cast<MetadataGenerator>(p);
}

int32_t MetadataGeneratorCreate(int32_t* instance)
{
    CHECK_AND_RETURN_RET_LOG(instance != nullptr, VPE_ALGO_ERR_INVALID_VAL, "invalid instance");
    auto p = MetadataGenerator::Create();
    CHECK_AND_RETURN_RET_LOG(p != nullptr, VPE_ALGO_ERR_INVALID_VAL, "cannot create instance");
    Extension::ExtensionManager::InstanceVariableType instanceVar { p };
    int32_t newId = Extension::ExtensionManager::GetInstance().NewInstanceId(instanceVar);
    CHECK_AND_RETURN_RET_LOG(newId != -1, VPE_ALGO_ERR_NO_MEMORY, "cannot create more instance");
    *instance = newId;
    return VPE_ALGO_ERR_OK;
}
int32_t MetadataGeneratorProcessImage(int32_t instance, OHNativeWindowBuffer* inputImage)
{
    CHECK_AND_RETURN_RET_LOG((inputImage != nullptr), VPE_ALGO_ERR_INVALID_VAL,
        "invalid parameters");
    auto someInstance = Extension::ExtensionManager::GetInstance().GetInstance(instance);
    CHECK_AND_RETURN_RET_LOG(someInstance != std::nullopt, VPE_ALGO_ERR_INVALID_VAL, "invalid instance");
 
    VPEAlgoErrCode ret = VPE_ALGO_ERR_INVALID_VAL;
    auto visitFunc = [inputImage, &ret](auto&& var) {
        using VarType = std::decay_t<decltype(var)>;
        if constexpr (std::is_same_v<VarType, std::shared_ptr<MetadataGenerator>>) {
            OH_NativeBuffer* inputImageNativeBuffer = nullptr;
            CHECK_AND_RETURN_LOG(
                (OH_NativeBuffer_FromNativeWindowBuffer(inputImage, &inputImageNativeBuffer) == GSERROR_OK),
                "invalid input image");
            sptr<SurfaceBuffer> inputImageSurfaceBuffer(
                SurfaceBuffer::NativeBufferToSurfaceBuffer(inputImageNativeBuffer));
            (void)var->SetParameter({ MetadataGeneratorAlgoType::META_GEN_ALGO_TYPE_IMAGE});
            ret = var->Process(inputImageSurfaceBuffer);
        } else {
            VPE_LOGE("instance may be miss used");
        }
    };
    std::visit(visitFunc, *someInstance);
 
    return ret;
}

int32_t MetadataGeneratorProcessVideo(int32_t instance, OHNativeWindowBuffer* inputImage)
{
    CHECK_AND_RETURN_RET_LOG((inputImage != nullptr), VPE_ALGO_ERR_INVALID_VAL,
        "invalid parameters");
    auto someInstance = Extension::ExtensionManager::GetInstance().GetInstance(instance);
    CHECK_AND_RETURN_RET_LOG(someInstance != std::nullopt, VPE_ALGO_ERR_INVALID_VAL, "invalid instance");
 
    VPEAlgoErrCode ret = VPE_ALGO_ERR_INVALID_VAL;
    auto visitFunc = [inputImage, &ret](auto&& var) {
        using VarType = std::decay_t<decltype(var)>;
        if constexpr (std::is_same_v<VarType, std::shared_ptr<MetadataGenerator>>) {
            OH_NativeBuffer* inputImageNativeBuffer = nullptr;
            CHECK_AND_RETURN_LOG(
                (OH_NativeBuffer_FromNativeWindowBuffer(inputImage, &inputImageNativeBuffer) == GSERROR_OK),
                "invalid input image");
            sptr<SurfaceBuffer> inputImageSurfaceBuffer(
                SurfaceBuffer::NativeBufferToSurfaceBuffer(inputImageNativeBuffer));
            MetadataGeneratorParameter param;
            param.algoType = MetadataGeneratorAlgoType::META_GEN_ALGO_TYPE_VIDEO;
            (void)var->SetParameter(param);
            ret = var->Process(inputImageSurfaceBuffer);
        } else {
            VPE_LOGE("instance may be miss used");
        }
    };
    std::visit(visitFunc, *someInstance);
 
    return ret;
}

int32_t MetadataGeneratorDestroy(int32_t* instance)
{
    CHECK_AND_RETURN_RET_LOG(instance != nullptr, VPE_ALGO_ERR_INVALID_VAL, "instance is null");
    return Extension::ExtensionManager::GetInstance().RemoveInstanceReference(*instance);
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
