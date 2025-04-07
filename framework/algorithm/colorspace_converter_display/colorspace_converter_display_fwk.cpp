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

#include "colorspace_converter_display_fwk.h"
#include "securec.h"
#include "extension_manager.h"
#include "vpe_trace.h"
#include "vpe_log.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

ColorSpaceConverterDisplayFwk::ColorSpaceConverterDisplayFwk()
{
    Extension::ExtensionManager::GetInstance().IncreaseInstance();
}

ColorSpaceConverterDisplayFwk::~ColorSpaceConverterDisplayFwk()
{
    impl_.clear();
    Extension::ExtensionManager::GetInstance().DecreaseInstance();
}

VPEAlgoErrCode ColorSpaceConverterDisplayFwk::Process(const std::shared_ptr<OHOS::Rosen::Drawing::ShaderEffect> &input,
    std::shared_ptr<OHOS::Rosen::Drawing::ShaderEffect> &output, const ColorSpaceConverterDisplayParameter &parameter)
{
    auto ret = Init();
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, VPE_ALGO_ERR_INIT_FAILED, "Init failed");
    VPE_LOGD("size of impl %{public}zu", impl_.size());
    CHECK_AND_RETURN_RET_LOG(!impl_.empty(), VPE_ALGO_ERR_NOT_IMPLEMENTED, "Extension is not found");

    VPE_SYNC_TRACE;
    for (const auto& impl : impl_) {
        ret = impl->Process(input, output, parameter);
        if (ret == VPE_ALGO_ERR_OK) {
            return VPE_ALGO_ERR_OK;
        }
    }

    VPE_LOGE("The operation is not supported");
    return VPE_ALGO_ERR_OPERATION_NOT_SUPPORTED;
}

VPEAlgoErrCode ColorSpaceConverterDisplayFwk::Init()
{
    if (initialized_) {
        return VPE_ALGO_ERR_OK;
    }
    auto& manager = Extension::ExtensionManager::GetInstance();

    VPE_SYNC_TRACE;
    impl_ = manager.CreateColorSpaceConverterDisplay();
    CHECK_AND_RETURN_RET_LOG(!impl_.empty(), VPE_ALGO_ERR_NOT_IMPLEMENTED, "Create impl failed");

    initialized_ = true;
    VPE_LOGI("Successed");
    return VPE_ALGO_ERR_OK;
}

std::shared_ptr<ColorSpaceConverterDisplay> ColorSpaceConverterDisplay::Create()
{
    auto p = std::make_shared<ColorSpaceConverterDisplayFwk>();
    CHECK_AND_RETURN_RET_LOG(p != nullptr, nullptr, "Create failed, maybe caused of no memory");
    return std::static_pointer_cast<ColorSpaceConverterDisplay>(p);
}

struct ColorSpaceConvertDisplayHandleImpl {
    std::shared_ptr<ColorSpaceConverterDisplayFwk> obj;
};

struct VPEShaderEffectHandleImpl {
    std::shared_ptr<OHOS::Rosen::Drawing::ShaderEffect> obj;
};

ColorSpaceConvertDisplayHandle *ColorSpaceConvertDisplayCreate()
{
    std::shared_ptr<ColorSpaceConverterDisplayFwk> impl = std::make_shared<ColorSpaceConverterDisplayFwk>();
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr, "failed to init ColorSpaceConvertDisplayCreate");
    auto handle = new ColorSpaceConvertDisplayHandleImpl;
    handle->obj = impl;
    return static_cast<ColorSpaceConvertDisplayHandle *>(handle);
}

void ColorSpaceConvertDisplayDestroy(ColorSpaceConvertDisplayHandle *handle)
{
    VPE_LOGD("call ColorSpaceConvertDisplayDestroy");
    if (handle != nullptr) {
        auto p = static_cast<ColorSpaceConvertDisplayHandleImpl *>(handle);
        delete p;
    }
}

VPEAlgoErrCode ColorSpaceConvertDisplayProcess(ColorSpaceConvertDisplayHandle *handle, VPEShaderEffectHandle *input,
    VPEShaderEffectHandle *output, const ColorSpaceConverterDisplayParameter &parameter)
{
    CHECK_AND_RETURN_RET_LOG(handle != nullptr, VPE_ALGO_ERR_INVALID_PARAM, "Input DisplayHandle in null!");
    VPEAlgoErrCode ret = VPE_ALGO_ERR_OK;

    auto p = static_cast<ColorSpaceConvertDisplayHandleImpl *>(handle);
    auto in = static_cast<VPEShaderEffectHandleImpl *>(input);
    auto out = static_cast<VPEShaderEffectHandleImpl *>(output);
    ret = p->obj->Process(in->obj, out->obj, parameter);
    return ret;
}

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
