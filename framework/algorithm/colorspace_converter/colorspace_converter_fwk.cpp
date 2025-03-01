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

#include <dlfcn.h>
#include "colorspace_converter_fwk.h"
#include "extension_manager.h"
#include "native_buffer.h"
#include "surface_buffer.h"
#include "vpe_trace.h"
#include "vpe_log.h"
#include "EGL/egl.h"
#include "surface_buffer_info.h"
#include "video_processing_client.h"
#include <unistd.h>

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
ColorSpaceConverterFwk::ColorSpaceConverterFwk()
{
    OpenCLInit();
    OpenGLInit();
    Extension::ExtensionManager::GetInstance().IncreaseInstance();
}

void ColorSpaceConverterFwk::OpenCLInit()
{
    void *OpenclFoundationHandle = nullptr;
    std::string path = "/sys_prod/lib64/VideoProcessingEngine/libaihdr_engine.so";
    auto ret = access(path.c_str(), F_OK);
    if (ret != 0) {
        VPE_LOGW("access = %d path = %s", ret, path.c_str());
    } else {
        constexpr int DEVICE_NAME_LENGTH = 32; // 32 max name length
        char deviceName[DEVICE_NAME_LENGTH];
        auto status = SetupOpencl(&OpenclFoundationHandle, "HUA", deviceName);
        if (status != static_cast<int>(CL_SUCCESS)) {
            VPE_LOGE("%{public}s, Error: setupOpencl status=%{public}d\n", __FUNCTION__, status);
        }
        OHOS::Media::VideoProcessingEngine::VideoProcessingManager::GetInstance().Connect();
        VPE_LOGI("VPE Framework connect and load SA!");
        OHOS::Media::VideoProcessingEngine::VideoProcessingManager::GetInstance().Disconnect();
    }
    context.clContext = reinterpret_cast<ClContext *>(OpenclFoundationHandle);
}

void ColorSpaceConverterFwk::OpenGLInit()
{
    context.glDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (context.glDisplay == EGL_NO_DISPLAY || eglGetError() != EGL_SUCCESS) {
        VPE_LOGE("ColorSpaceConverterFwk Get display failed!");
    }
    EGLint major;
    EGLint minor;
    if (eglInitialize(context.glDisplay, &major, &minor) == EGL_FALSE || eglGetError() != EGL_SUCCESS) {
        VPE_LOGE("ColorSpaceConverterFwk eglInitialize failed!");
    }
}

ColorSpaceConverterFwk::ColorSpaceConverterFwk(std::shared_ptr<OpenGLContext> openglContext,
                                               ClContext *opengclContext)
{
    if (opengclContext != nullptr) {
        context.clContext = opengclContext;
    }
    if (openglContext != nullptr) {
        if (openglContext->display != EGL_NO_DISPLAY) {
            context.glDisplay = openglContext->display;
        }
    }
    OHOS::Media::VideoProcessingEngine::VideoProcessingManager::GetInstance().Connect();
    VPE_LOGI("VPE Framework connect and load SA!");
    OHOS::Media::VideoProcessingEngine::VideoProcessingManager::GetInstance().Disconnect();
    Extension::ExtensionManager::GetInstance().IncreaseInstance();
}

ColorSpaceConverterFwk::~ColorSpaceConverterFwk()
{
    impl_ = nullptr;
    for (auto it = impls_.begin(); it != impls_.end(); ++it) {
        if (it->second) {
            it->second->Deinit();
            it->second = nullptr;
        }
    }
    impls_.clear();
    CleanOpencl(context.clContext);
    Extension::ExtensionManager::GetInstance().DecreaseInstance();
}

VPEAlgoErrCode ColorSpaceConverterFwk::SetParameter(const ColorSpaceConverterParameter &parameter)
{
    CHECK_AND_RETURN_RET_LOG(parameter.renderIntent == RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC,
        VPE_ALGO_ERR_INVALID_VAL, "Absolute colorimetric is the only supported render intent");
    parameter_ = parameter;
    VPE_LOGI("ColorSpaceConverterFwk SetParameter Succeed");
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode ColorSpaceConverterFwk::GetParameter(ColorSpaceConverterParameter &parameter) const
{
    CHECK_AND_RETURN_RET_LOG(parameter_ != std::nullopt, VPE_ALGO_ERR_INVALID_VAL, "Parameter is not set");

    parameter = *parameter_;
    VPE_LOGI("ColorSpaceConverterFwk GetParameter Succeed");
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode ColorSpaceConverterFwk::Process(const sptr<SurfaceBuffer> &input, const sptr<SurfaceBuffer> &output)
{
    CHECK_AND_RETURN_RET_LOG(parameter_ != std::nullopt, VPE_ALGO_ERR_INVALID_VAL, "Parameter is not set");
    CHECK_AND_RETURN_RET_LOG((input != nullptr) && (output != nullptr), VPE_ALGO_ERR_INVALID_VAL,
        "Input or output is nullptr");
    VPEAlgoErrCode ret;

    ret = Init(input, output, context);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, ret, "Init failed, ret: %{public}d", ret);
    ret = impl_->SetParameter(*parameter_);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, VPE_ALGO_ERR_OPERATION_NOT_SUPPORTED,
        "Set parameter failed, ret: %{public}d", ret);

    VPE_SYNC_TRACE;
    ret = impl_->Process(input, output);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, ret, "Process failed, ret: %{public}d", ret);

    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode ColorSpaceConverterFwk::ComposeImage(const sptr<SurfaceBuffer> &inputSdrImage,
    const sptr<SurfaceBuffer> &inputGainmap, const sptr<SurfaceBuffer> &outputHdrImage, bool legacy)
{
    CHECK_AND_RETURN_RET_LOG(parameter_ != std::nullopt, VPE_ALGO_ERR_INVALID_VAL, "Parameter is not set");
    CHECK_AND_RETURN_RET_LOG((inputSdrImage != nullptr) && (inputGainmap != nullptr) && (outputHdrImage != nullptr),
        VPE_ALGO_ERR_INVALID_VAL, "Input or output is nullptr");

    VPEAlgoErrCode ret = Init(inputSdrImage, outputHdrImage, context);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, ret, "Init failed, ret: %{public}d", ret);

    ret = impl_->SetParameter(*parameter_);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, VPE_ALGO_ERR_OPERATION_NOT_SUPPORTED,
        "Set parameter failed, ret: %{public}d", ret);

    VPE_SYNC_TRACE;
    ret = impl_->ComposeImage(inputSdrImage, inputGainmap, outputHdrImage, legacy);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, VPE_ALGO_ERR_UNKNOWN, "Compose image failed, ret: %{public}d",
        ret);
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode ColorSpaceConverterFwk::DecomposeImage(const sptr<SurfaceBuffer> &inputImage,
    const sptr<SurfaceBuffer> &outputSdrImage, const sptr<SurfaceBuffer> &outputGainmap)
{
    CHECK_AND_RETURN_RET_LOG(parameter_ != std::nullopt, VPE_ALGO_ERR_INVALID_VAL, "Parameter is not set");
    CHECK_AND_RETURN_RET_LOG((inputImage != nullptr) && (outputSdrImage != nullptr) && (outputGainmap != nullptr),
        VPE_ALGO_ERR_INVALID_VAL, "Input or output is nullptr");
    VPEAlgoErrCode ret;

    ret = Init(inputImage, outputSdrImage, context);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, ret, "Init failed, ret: %{public}d", ret);

    ret = impl_->SetParameter(*parameter_);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, VPE_ALGO_ERR_OPERATION_NOT_SUPPORTED,
        "Set parameter failed, ret: %{public}d", ret);

    VPE_SYNC_TRACE;
    ret = impl_->DecomposeImage(inputImage, outputSdrImage, outputGainmap);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, VPE_ALGO_ERR_UNKNOWN, "Decompose image failed, ret: %{public}d",
        ret);
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode ColorSpaceConverterFwk::Init(const sptr<SurfaceBuffer> &input, const sptr<SurfaceBuffer> &output,
    VPEContext ctx)
{
    CHECK_AND_RETURN_RET_LOG(context.clContext != nullptr || context.glDisplay != EGL_NO_DISPLAY,
        VPE_ALGO_ERR_OPERATION_NOT_SUPPORTED, "opencl or opengl is not initialized!");
    auto &manager = Extension::ExtensionManager::GetInstance();
    VPE_SYNC_TRACE;
    FrameInfo inputInfo(input);
    FrameInfo outputInfo(output);
    auto currentKey =
        std::make_tuple(inputInfo.colorSpace, inputInfo.pixelFormat, outputInfo.colorSpace, outputInfo.pixelFormat);
    auto it = impls_.find(currentKey);
    if (it != impls_.end()) {
        impl_ = it->second;
        VPE_LOGD("ColorSpaceConverterFwk::Init find exist impl");
        return VPE_ALGO_ERR_OK;
    }
    impl_ = manager.CreateColorSpaceConverter(inputInfo, outputInfo, extensionInfo_);
    CHECK_AND_RETURN_RET_LOG(impl_ != nullptr, VPE_ALGO_ERR_NOT_IMPLEMENTED,
        "Extension create failed, get a empty impl");
    VPEAlgoErrCode ret = impl_->Init(inputInfo, outputInfo, ctx);
    CHECK_AND_RETURN_RET_LOG(ret == 0, VPE_ALGO_ERR_EXTENSION_INIT_FAILED,
        "Init failed, extension: %{public}s, version: %{public}s", extensionInfo_.name.c_str(),
        extensionInfo_.version.c_str());
    VPE_LOGI("Init successfully, extension: %{public}s, version: %{public}s", extensionInfo_.name.c_str(),
        extensionInfo_.version.c_str());
    impls_.insert(std::make_pair(currentKey, impl_));
    return VPE_ALGO_ERR_OK;
}

std::shared_ptr<ColorSpaceConverter> ColorSpaceConverter::Create()
{
    auto p = std::make_shared<ColorSpaceConverterFwk>();
    CHECK_AND_RETURN_RET_LOG(p != nullptr, nullptr, "Create failed, maybe caused of no memory");
    return std::static_pointer_cast<ColorSpaceConverter>(p);
}

std::shared_ptr<ColorSpaceConverter> ColorSpaceConverter::Create(std::shared_ptr<OpenGLContext> openglContext,
                                                                 ClContext *opengclContext)
{
    auto p = std::make_shared<ColorSpaceConverterFwk>(openglContext, opengclContext);
    CHECK_AND_RETURN_RET_LOG(p != nullptr, nullptr, "Create failed, maybe caused of no memory");
    return std::static_pointer_cast<ColorSpaceConverter>(p);
}

int32_t ColorSpaceConverterCreate(int32_t* instance)
{
    CHECK_AND_RETURN_RET_LOG(instance != nullptr, VPE_ALGO_ERR_INVALID_VAL, "invalid instance");
    auto p = ColorSpaceConverter::Create();
    CHECK_AND_RETURN_RET_LOG(p != nullptr, VPE_ALGO_ERR_INVALID_VAL, "cannot create instance");
    Extension::ExtensionManager::InstanceVariableType instanceVar { p };
    int32_t newId = Extension::ExtensionManager::GetInstance().NewInstanceId(instanceVar);
    CHECK_AND_RETURN_RET_LOG(newId != -1, VPE_ALGO_ERR_NO_MEMORY, "cannot create more instance");
    *instance = newId;
    return VPE_ALGO_ERR_OK;
}

int32_t ColorSpaceConverterProcessImage(int32_t instance, OHNativeWindowBuffer* inputImage,
    OHNativeWindowBuffer* outputImage)
{
    CHECK_AND_RETURN_RET_LOG((inputImage != nullptr && outputImage != nullptr), VPE_ALGO_ERR_INVALID_VAL,
        "invalid parameters");
    auto someInstance = Extension::ExtensionManager::GetInstance().GetInstance(instance);
    CHECK_AND_RETURN_RET_LOG(someInstance != std::nullopt, VPE_ALGO_ERR_INVALID_VAL, "invalid instance");

    VPEAlgoErrCode ret = VPE_ALGO_ERR_INVALID_VAL;
    auto visitFunc = [inputImage, outputImage, &ret](auto&& var) {
        using VarType = std::decay_t<decltype(var)>;
        if constexpr (std::is_same_v<VarType, std::shared_ptr<ColorSpaceConverter>>) {
            OH_NativeBuffer* inputImageNativeBuffer = nullptr;
            OH_NativeBuffer* outputImageNativeBuffer = nullptr;
            CHECK_AND_RETURN_LOG(
                (OH_NativeBuffer_FromNativeWindowBuffer(inputImage, &inputImageNativeBuffer) == GSERROR_OK) &&
                (OH_NativeBuffer_FromNativeWindowBuffer(outputImage, &outputImageNativeBuffer) == GSERROR_OK),
                "invalid input or output image");
            sptr<SurfaceBuffer> inputImageSurfaceBuffer(
                SurfaceBuffer::NativeBufferToSurfaceBuffer(inputImageNativeBuffer));
            sptr<SurfaceBuffer> outputImageSurfaceBuffer(
                SurfaceBuffer::NativeBufferToSurfaceBuffer(outputImageNativeBuffer));
            (void)var->SetParameter({ RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC, std::nullopt });
            ret = var->Process(inputImageSurfaceBuffer, outputImageSurfaceBuffer);
        } else {
            VPE_LOGE("instance may be miss used");
        }
    };
    std::visit(visitFunc, *someInstance);

    return ret;
}

int32_t ColorSpaceConverterComposeImage(int32_t instance, OHNativeWindowBuffer* inputSdrImage,
    OHNativeWindowBuffer* inputGainmap, OHNativeWindowBuffer* outputHdrImage, bool legacy)
{
    CHECK_AND_RETURN_RET_LOG((inputSdrImage != nullptr && inputGainmap != nullptr && outputHdrImage != nullptr),
        VPE_ALGO_ERR_INVALID_VAL, "invalid parameters");

    auto someInstance = Extension::ExtensionManager::GetInstance().GetInstance(instance);
    CHECK_AND_RETURN_RET_LOG(someInstance != std::nullopt, VPE_ALGO_ERR_INVALID_VAL, "invalid instance");

    VPEAlgoErrCode ret = VPE_ALGO_ERR_INVALID_VAL;
    auto visitFunc = [inputSdrImage, inputGainmap, outputHdrImage, legacy, &ret](auto&& var) {
        using VarType = std::decay_t<decltype(var)>;
        if constexpr (std::is_same_v<VarType, std::shared_ptr<ColorSpaceConverter>>) {
            OH_NativeBuffer* inputSdrImageNativeBuffer = nullptr;
            OH_NativeBuffer* inputGainmapNativeBuffer = nullptr;
            OH_NativeBuffer* outputHdrImageNativeBuffer = nullptr;
            CHECK_AND_RETURN_LOG(
                (OH_NativeBuffer_FromNativeWindowBuffer(inputSdrImage, &inputSdrImageNativeBuffer) == GSERROR_OK) &&
                (OH_NativeBuffer_FromNativeWindowBuffer(inputGainmap, &inputGainmapNativeBuffer) == GSERROR_OK) &&
                (OH_NativeBuffer_FromNativeWindowBuffer(outputHdrImage, &outputHdrImageNativeBuffer) == GSERROR_OK),
                "invalid input or output image");
            sptr<SurfaceBuffer> inputSdrImageSurfaceBuffer(
                SurfaceBuffer::NativeBufferToSurfaceBuffer(inputSdrImageNativeBuffer));
            sptr<SurfaceBuffer> inputGainmapSurfaceBuffer(
                SurfaceBuffer::NativeBufferToSurfaceBuffer(inputGainmapNativeBuffer));
            sptr<SurfaceBuffer> outputHdrImageSurfaceBuffer(
                SurfaceBuffer::NativeBufferToSurfaceBuffer(outputHdrImageNativeBuffer));
            (void)var->SetParameter({ RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC, std::nullopt });
            ret = var->ComposeImage(inputSdrImageSurfaceBuffer, inputGainmapSurfaceBuffer, outputHdrImageSurfaceBuffer,
                                    legacy);
        } else {
            VPE_LOGE("instance may be miss used");
        }
    };
    std::visit(visitFunc, *someInstance);

    return ret;
}

int32_t ColorSpaceConverterDecomposeImage(int32_t instance, OHNativeWindowBuffer* inputImage,
    OHNativeWindowBuffer* outputSdrImage, OHNativeWindowBuffer* outputGainmap)
{
    CHECK_AND_RETURN_RET_LOG((inputImage != nullptr && outputSdrImage != nullptr && outputGainmap != nullptr),
        VPE_ALGO_ERR_INVALID_VAL, "invalid parameters");
    auto someInstance = Extension::ExtensionManager::GetInstance().GetInstance(instance);
    CHECK_AND_RETURN_RET_LOG(someInstance != std::nullopt, VPE_ALGO_ERR_INVALID_VAL, "invalid instance");

    VPEAlgoErrCode ret = VPE_ALGO_ERR_INVALID_VAL;
    auto visitFunc = [inputImage, outputSdrImage, outputGainmap, &ret](auto&&  var) {
        using VarType = std::decay_t<decltype(var)>;
        if constexpr (std::is_same_v<VarType, std::shared_ptr<ColorSpaceConverter>>) {
            OH_NativeBuffer* inputImageNativeBuffer = nullptr;
            OH_NativeBuffer* outputSdrImageNativeBuffer = nullptr;
            OH_NativeBuffer* outputGainmapNativeBuffer = nullptr;
            CHECK_AND_RETURN_LOG(
                (OH_NativeBuffer_FromNativeWindowBuffer(inputImage, &inputImageNativeBuffer) == GSERROR_OK) &&
                (OH_NativeBuffer_FromNativeWindowBuffer(outputSdrImage, &outputSdrImageNativeBuffer) == GSERROR_OK) &&
                (OH_NativeBuffer_FromNativeWindowBuffer(outputGainmap, &outputGainmapNativeBuffer) == GSERROR_OK),
                "invalid input or output image");
            sptr<SurfaceBuffer> inputImageSurfaceBuffer(
                SurfaceBuffer::NativeBufferToSurfaceBuffer(inputImageNativeBuffer));
            sptr<SurfaceBuffer> outputSdrImageSurfaceBuffer(
                SurfaceBuffer::NativeBufferToSurfaceBuffer(outputSdrImageNativeBuffer));
            sptr<SurfaceBuffer> outputGainmapSurfaceBuffer(
                SurfaceBuffer::NativeBufferToSurfaceBuffer(outputGainmapNativeBuffer));
            (void)var->SetParameter({ RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC, std::nullopt });
            ret = var->DecomposeImage(inputImageSurfaceBuffer, outputSdrImageSurfaceBuffer, outputGainmapSurfaceBuffer);
        } else  {
            VPE_LOGE("instance may be miss used");
        }
    };
    std::visit(visitFunc, *someInstance);

    return ret;
}

int32_t ColorSpaceConverterDestroy(int32_t* instance)
{
    CHECK_AND_RETURN_RET_LOG(instance != nullptr, VPE_ALGO_ERR_INVALID_VAL, "instance is null");
    return Extension::ExtensionManager::GetInstance().RemoveInstanceReference(*instance);
}

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
