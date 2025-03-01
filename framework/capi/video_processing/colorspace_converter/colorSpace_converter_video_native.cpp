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

#include <unordered_map>
#include "video_processing_utils.h"
#include "colorSpace_converter_video_native.h"
#include "colorspace_converter_video_description.h"
#include "video_processing_capi_capability.h"
#include "surface_type.h"
#include "algorithm_common.h"
#include "v1_0/cm_color_space.h"
#include "native_buffer.h"
#include "window.h"
#include "vpe_log.h"

using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;

VideoProcessing_ErrorCode ColorSpaceConverterVideoNative::InitializeInner()
{
    CHECK_AND_RETURN_RET_LOG(!isInitialized_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already init!");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(!isInitialized_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already init!");
    colorSpaceConverter_ = ColorSpaceConverterVideo::Create(openglContext_);
    CHECK_AND_RETURN_RET_LOG(colorSpaceConverter_ != nullptr, VIDEO_PROCESSING_ERROR_CREATE_FAILED,
        "Create colorSpace converter failed!");
    isInitialized_ = true;
    return VIDEO_PROCESSING_SUCCESS;
}

VideoProcessing_ErrorCode ColorSpaceConverterVideoNative::DeinitializeInner()
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already deinit!");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already deinit!");
    colorSpaceConverter_ = nullptr;
    isInitialized_ = false;
    return VIDEO_PROCESSING_SUCCESS;
}

VideoProcessing_ErrorCode ColorSpaceConverterVideoNative::RegisterCallback()
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");

    auto callback = std::make_shared<NativeCallback>(shared_from_this());
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, VIDEO_PROCESSING_ERROR_CREATE_FAILED,
        "Create callback failed!");
    CHECK_AND_RETURN_RET_LOG(colorSpaceConverter_->SetCallback(callback) == VPE_ALGO_ERR_OK,
        VIDEO_PROCESSING_ERROR_PROCESS_FAILED, "RegisterCallback failed!");

    return VIDEO_PROCESSING_SUCCESS;
}

VideoProcessing_ErrorCode ColorSpaceConverterVideoNative::SetSurface(const sptr<Surface>& surface,
    const OHNativeWindow& window)
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");
    CHECK_AND_RETURN_RET_LOG(surface != nullptr, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER,
        "window surface is null!");
    colorSpaceConverter_->SetOutputSurface(surface);

    colorSpaceValue_ = static_cast<int>(CM_BT709_LIMIT);
    formatValue_ = static_cast<int>(GRAPHIC_PIXEL_FMT_YCBCR_420_SP);
    metadataValue_ = static_cast<int>(CM_METADATA_NONE);
    int32_t formatGet = 0;
    auto ret = OH_NativeWindow_NativeWindowHandleOpt(const_cast<OHNativeWindow*>(&window), GET_FORMAT, &formatGet);
    if ((ret == 0) && (formatGet != 0)) {
        OH_NativeBuffer_Format formatGetResult = static_cast<OH_NativeBuffer_Format>(formatGet);
        auto itFormat = NATIVE_FORMAT_TO_GRAPHIC_MAP.find(formatGetResult);
        if (itFormat != NATIVE_FORMAT_TO_GRAPHIC_MAP.end()) {
            formatValue_ = static_cast<int>(itFormat->second);
        }
    }

    CHECK_AND_RETURN_RET_LOG(const_cast<OHNativeWindow*>(&window) != nullptr,
        VIDEO_PROCESSING_ERROR_INVALID_PARAMETER, "SetSurface window is null!");
    OH_NativeBuffer_ColorSpace colorSpaceOut = OH_COLORSPACE_NONE;
    ret = OH_NativeWindow_GetColorSpace(const_cast<OHNativeWindow*>(&window), &colorSpaceOut);
    CHECK_AND_LOG(ret == 0, "OH_NativeWindow_GetColorSpace fail!");
    if ((ret == 0) && (colorSpaceOut != 0)) {
        auto itColorSpace = NATIVE_COLORSPACE_TO_CM_MAP.find(colorSpaceOut);
        if (itColorSpace != NATIVE_COLORSPACE_TO_CM_MAP.end()) {
            colorSpaceValue_ = static_cast<int>(itColorSpace->second);
        }
    }
    int32_t buffSize;
    uint8_t *checkMetaData;
    ret = OH_NativeWindow_GetMetadataValue(const_cast<OHNativeWindow*>(&window), OH_HDR_METADATA_TYPE,
        &buffSize, &checkMetaData);
    if (ret != 0) {
        VPE_LOGE("OH_NativeWindow_GetMetadataValue failed, ret:%{public}d", ret);
        return VIDEO_PROCESSING_SUCCESS;
    }
    OH_NativeBuffer_MetadataType metadataGetResult = static_cast<OH_NativeBuffer_MetadataType>(checkMetaData[0]);
    auto itMetadata = NATIVE_METADATATYPE_TO_CM_MAP.find(metadataGetResult);
    if (itMetadata != NATIVE_METADATATYPE_TO_CM_MAP.end()) {
        metadataValue_ = static_cast<int>(itMetadata->second);
    }
    delete[] checkMetaData;
    return VIDEO_PROCESSING_SUCCESS;
}

sptr<Surface> ColorSpaceConverterVideoNative::GetSurface()
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), nullptr, "Initialization failed!");
    return colorSpaceConverter_->CreateInputSurface();
}

VideoProcessing_ErrorCode ColorSpaceConverterVideoNative::SetParameter(const OHOS::Media::Format& parameter)
{
    return VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED;
}

VideoProcessing_ErrorCode ColorSpaceConverterVideoNative::GetParameter(OHOS::Media::Format& parameter)
{
    return VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED;
}

VideoProcessing_ErrorCode ColorSpaceConverterVideoNative::OnStart()
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");
    int32_t ret = 0;
    OHOS::Media::Format format;
    format.PutIntValue(CscVDescriptionKey::CSCV_KEY_PIXEL_FORMAT, formatValue_);
    format.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_PRIMARIES,
        ((static_cast<uint32_t>(colorSpaceValue_) & COLORPRIMARIES_MASK) >> COLORPRIMARIES_OFFSET));
    format.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_TRANS_FUNC,
        ((static_cast<uint32_t>(colorSpaceValue_) & TRANSFUNC_MASK) >> TRANSFUNC_OFFSET));
    format.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_MATRIX,
        ((static_cast<uint32_t>(colorSpaceValue_) & MATRIX_MASK) >> MATRIX_OFFSET));
    format.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_RANGE,
        ((static_cast<uint32_t>(colorSpaceValue_) & RANGE_MASK) >> RANGE_OFFSET));
    format.PutIntValue(CscVDescriptionKey::CSCV_KEY_HDR_METADATA_TYPE, metadataValue_);
    format.PutIntValue(CscVDescriptionKey::CSCV_KEY_RENDER_INTENT,
        int(RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC));
    ret = colorSpaceConverter_->Configure(format);
    if (ret != VPE_ALGO_ERR_OK) {
        return VideoProcessingUtils::InnerErrorToCAPI(static_cast<VPEAlgoErrCode>(ret));
    }
    ret = colorSpaceConverter_->Prepare();
    if (ret != VPE_ALGO_ERR_OK) {
        return VideoProcessingUtils::InnerErrorToCAPI(static_cast<VPEAlgoErrCode>(ret));
    }
    ret = colorSpaceConverter_->Start();
    if (ret != VPE_ALGO_ERR_OK) {
        return VideoProcessingUtils::InnerErrorToCAPI(static_cast<VPEAlgoErrCode>(ret));
    }
    return VIDEO_PROCESSING_SUCCESS;
}

VideoProcessing_ErrorCode ColorSpaceConverterVideoNative::OnStop()
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");

    return VideoProcessingUtils::InnerErrorToCAPI(
        static_cast<VPEAlgoErrCode>(colorSpaceConverter_->Stop()));
}

VideoProcessing_ErrorCode ColorSpaceConverterVideoNative::OnRenderOutputBuffer(uint32_t index)
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");

    return VideoProcessingUtils::InnerErrorToCAPI(
        static_cast<VPEAlgoErrCode>(colorSpaceConverter_->ReleaseOutputBuffer(index, true)));
}

ColorSpaceConverterVideoNative::NativeCallback::NativeCallback(
    const std::shared_ptr<ColorSpaceConverterVideoNative>& owner)
    : owner_(owner)
{
}

void ColorSpaceConverterVideoNative::NativeCallback::OnError(int32_t errorCode)
{
    SendCallback([this, &errorCode]() {
        owner_->OnError(VideoProcessingUtils::InnerErrorToCAPI(static_cast<VPEAlgoErrCode>(errorCode)));
    });
}

void ColorSpaceConverterVideoNative::NativeCallback::OnState(int32_t state)
{
    SendCallback([this, &state]() {
        owner_->OnState(VideoProcessingUtils::InnerStateToCAPI(static_cast<VPEAlgoState>(state)));
    });
}

void ColorSpaceConverterVideoNative::NativeCallback::OnOutputBufferAvailable(uint32_t index,
    [[maybe_unused]] CscvBufferFlag flag)
{
    SendCallback([this, &index]() {
        owner_->OnNewOutputBuffer(index);
    });
}

void ColorSpaceConverterVideoNative::NativeCallback::OnOutputFormatChanged(const Format& format)
{
    (void)format;
}

void ColorSpaceConverterVideoNative::NativeCallback::SendCallback(std::function<void(void)>&& callback) const
{
    if (owner_ == nullptr) {
        VPE_LOGE("owner is null!");
        return;
    }

    callback();
}
