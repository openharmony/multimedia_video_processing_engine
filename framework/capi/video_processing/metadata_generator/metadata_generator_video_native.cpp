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
#include "metadata_generator_video_native.h"
#include "vpe_log.h"

using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;
namespace {
const std::unordered_map<int, int> styleMap = {
    {VIDEO_METADATA_GENERATOR_BRIGHT_MODE,         META_GEN_BRIGHT_STYLE},
    {VIDEO_METADATA_GENERATOR_CONTRAST_MODE,       META_GEN_CONTRAST_STYLE},
};
}

VideoProcessing_ErrorCode MetadataGeneratorVideoNative::InitializeInner()
{
    CHECK_AND_RETURN_RET_LOG(!isInitialized_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already init!");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(!isInitialized_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already init!");
    metadataGenerator_ = MetadataGeneratorVideo::Create(openglContext_);
    CHECK_AND_RETURN_RET_LOG(metadataGenerator_ != nullptr, VIDEO_PROCESSING_ERROR_CREATE_FAILED,
        "Create colorSpace converter failed!");
    isInitialized_ = true;
    return VIDEO_PROCESSING_SUCCESS;
}

VideoProcessing_ErrorCode MetadataGeneratorVideoNative::DeinitializeInner()
{
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already deinit!");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED,
        "Already deinit!");
    metadataGenerator_ = nullptr;
    isInitialized_ = false;
    return VIDEO_PROCESSING_SUCCESS;
}

VideoProcessing_ErrorCode MetadataGeneratorVideoNative::RegisterCallback()
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");

    auto callback = std::make_shared<NativeCallback>(shared_from_this());
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, VIDEO_PROCESSING_ERROR_CREATE_FAILED,
        "Create callback failed!");
    CHECK_AND_RETURN_RET_LOG(metadataGenerator_->SetCallback(callback) == VPE_ALGO_ERR_OK,
        VIDEO_PROCESSING_ERROR_PROCESS_FAILED, "RegisterCallback failed!");
    return VIDEO_PROCESSING_SUCCESS;
}

VideoProcessing_ErrorCode MetadataGeneratorVideoNative::SetSurface(const sptr<Surface>& surface)
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");
    CHECK_AND_RETURN_RET_LOG(surface != nullptr, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER,
        "surface is null!");
    metadataGenerator_->SetOutputSurface(surface);
    return VIDEO_PROCESSING_SUCCESS;
}

sptr<Surface> MetadataGeneratorVideoNative::GetSurface()
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), nullptr, "Initialization failed!");
    return metadataGenerator_->CreateInputSurface();
}

VideoProcessing_ErrorCode MetadataGeneratorVideoNative::SetParameter(const OHOS::Media::Format& parameter)
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");

    int style;
    CHECK_AND_RETURN_RET_LOG(parameter.GetIntValue(VIDEO_METADATA_GENERATOR_STYLE_CONTROL, style),
        VIDEO_PROCESSING_ERROR_INVALID_PARAMETER, "No metadata generator style!");
    int styleType = CApiStyleToInner(style);
    CHECK_AND_RETURN_RET_LOG(styleType != -1, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER,
        "Metadata generator style is invalid!");
    MetadataGeneratorParameter param{};
    param.styleType = static_cast<VideoMetadataGeneratorStyle>(styleType);
    auto result = VideoProcessingUtils::InnerErrorToCAPI(metadataGenerator_->SetParameter(param));
    return result;
}

VideoProcessing_ErrorCode MetadataGeneratorVideoNative::GetParameter(OHOS::Media::Format& parameter)
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");

    MetadataGeneratorParameter param{};
    auto result = VideoProcessingUtils::InnerErrorToCAPI(metadataGenerator_->GetParameter(param));
    if (result == VIDEO_PROCESSING_SUCCESS) {
        int styleType = static_cast<int>(param.styleType);
        CHECK_AND_RETURN_RET_LOG(parameter.PutIntValue(VIDEO_METADATA_GENERATOR_STYLE_CONTROL, styleType),
            VIDEO_PROCESSING_ERROR_PROCESS_FAILED, "Get parameter failed!");
    }
    return result;
}

VideoProcessing_ErrorCode MetadataGeneratorVideoNative::OnStart()
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");

    int32_t ret = 0;
    ret = metadataGenerator_->Configure();
    if (ret != VPE_ALGO_ERR_OK) {
        return VideoProcessingUtils::InnerErrorToCAPI(static_cast<VPEAlgoErrCode>(ret));
    }
    ret = metadataGenerator_->Prepare();
    if (ret != VPE_ALGO_ERR_OK) {
        return VideoProcessingUtils::InnerErrorToCAPI(static_cast<VPEAlgoErrCode>(ret));
    }
    ret = metadataGenerator_->Start();
    if (ret != VPE_ALGO_ERR_OK) {
        return VideoProcessingUtils::InnerErrorToCAPI(static_cast<VPEAlgoErrCode>(ret));
    }
    return VIDEO_PROCESSING_SUCCESS;
}

VideoProcessing_ErrorCode MetadataGeneratorVideoNative::OnStop()
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");

    return VideoProcessingUtils::InnerErrorToCAPI(static_cast<VPEAlgoErrCode>(metadataGenerator_->Stop()));
}

VideoProcessing_ErrorCode MetadataGeneratorVideoNative::OnRenderOutputBuffer(uint32_t index)
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED,
        "Initialization failed!");

    return VideoProcessingUtils::InnerErrorToCAPI(
        static_cast<VPEAlgoErrCode>(metadataGenerator_->ReleaseOutputBuffer(index, true)));
}

MetadataGeneratorVideoNative::NativeCallback::NativeCallback(
    const std::shared_ptr<MetadataGeneratorVideoNative>& owner)
    : owner_(owner)
{
}

int MetadataGeneratorVideoNative::CApiStyleToInner(int Style) const
{
    auto it = styleMap.find(Style);
    if (it == styleMap.end()) {
        VPE_LOGE("Invalid input style:%{public}d", Style);
        return -1;
    }
    return it->second;
}

void MetadataGeneratorVideoNative::NativeCallback::OnError(int32_t errorCode)
{
    SendCallback([this, &errorCode]() {
        owner_->OnError(VideoProcessingUtils::InnerErrorToCAPI(static_cast<VPEAlgoErrCode>(errorCode)));
    });
}

void MetadataGeneratorVideoNative::NativeCallback::OnState(int32_t state)
{
    SendCallback([this, &state]() {
        owner_->OnState(VideoProcessingUtils::InnerStateToCAPI(static_cast<VPEAlgoState>(state)));
    });
}

void MetadataGeneratorVideoNative::NativeCallback::OnOutputBufferAvailable(uint32_t index,
    [[maybe_unused]] MdgBufferFlag flag)
{
    SendCallback([this, &index]() {
        owner_->OnNewOutputBuffer(index);
    });
}

void MetadataGeneratorVideoNative::NativeCallback::SendCallback(std::function<void(void)>&& callback) const
{
    if (owner_ == nullptr) {
        VPE_LOGE("owner is null!");
        return;
    }

    callback();
}
