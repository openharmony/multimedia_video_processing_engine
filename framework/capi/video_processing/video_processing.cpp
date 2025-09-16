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

#include "video_processing.h"

#include <atomic>
#include <functional>

#include "vpe_log.h"
#include "video_processing_capi_capability.h"
#include "video_environment_native.h"
#include "video_processing_callback_impl.h"
#include "video_processing_impl.h"

using namespace OHOS::Media::VideoProcessingEngine;

// CAPI define
// Video processing feature types:
const int32_t VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION = 0x1;
const int32_t VIDEO_PROCESSING_TYPE_METADATA_GENERATION = 0x2;
const int32_t VIDEO_PROCESSING_TYPE_DETAIL_ENHANCER = 0x4;
// Video processing parameter keys:
// Detail enhancement:
const char* VIDEO_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL = "QualityLevel";
const char* VIDEO_METADATA_GENERATOR_STYLE_CONTROL = "StyleControl";

namespace {
// Call video processing interface
VideoProcessing_ErrorCode CallVideoProcessing(OH_VideoProcessing* videoProcessor,
    std::function<VideoProcessing_ErrorCode(std::shared_ptr<IVideoProcessingNative>&)>&& operation)
{
    CHECK_AND_RETURN_RET_LOG(videoProcessor != nullptr, VIDEO_PROCESSING_ERROR_INVALID_INSTANCE,
        "videoProcessor is null!");
    auto videoProcessing = videoProcessor->GetVideoProcessing();
    CHECK_AND_RETURN_RET_LOG(videoProcessing != nullptr, VIDEO_PROCESSING_ERROR_INVALID_INSTANCE,
        "videoProcessor is invalid!");
    return operation(videoProcessing);
}

// Call video processing callback interface
VideoProcessing_ErrorCode CallVideoProcessingCallback(VideoProcessing_Callback* callback,
    std::function<VideoProcessing_ErrorCode(std::shared_ptr<VideoProcessingCallbackNative>&)>&& operation)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER, "callback is null!");
    auto innerCallback = callback->GetInnerCallback();
    CHECK_AND_RETURN_RET_LOG(innerCallback != nullptr, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER,
        "callback is invalid!");
    return operation(innerCallback);
}
}

VideoProcessing_ErrorCode OH_VideoProcessing_InitializeEnvironment(void)
{
    CHECK_AND_RETURN_RET_LOG(VideoProcessingCapiCapability::OpenGLInit() == VIDEO_PROCESSING_SUCCESS,
        VIDEO_PROCESSING_ERROR_INITIALIZE_FAILED, "OpenGLInit failed!");
    return VideoEnvironmentNative::Get().Initialize();
}

VideoProcessing_ErrorCode OH_VideoProcessing_DeinitializeEnvironment(void)
{
    return VideoEnvironmentNative::Get().Deinitialize();
}

bool OH_VideoProcessing_IsColorSpaceConversionSupported(const VideoProcessing_ColorSpaceInfo* sourceVideoInfo,
    const VideoProcessing_ColorSpaceInfo* destinationVideoInfo)
{
    return VideoProcessingCapiCapability::IsColorSpaceConversionSupported(sourceVideoInfo, destinationVideoInfo);
}

bool OH_VideoProcessing_IsMetadataGenerationSupported(const VideoProcessing_ColorSpaceInfo* sourceVideoInfo)
{
    return VideoProcessingCapiCapability::IsMetadataGenerationSupported(sourceVideoInfo);
}

VideoProcessing_ErrorCode OH_VideoProcessing_Create(OH_VideoProcessing** videoProcessor, int type)
{
    return OH_VideoProcessing::Create(videoProcessor, type, VideoProcessingCapiCapability::GetOpenGLContext());
}

VideoProcessing_ErrorCode OH_VideoProcessing_Destroy(OH_VideoProcessing* videoProcessor)
{
    return OH_VideoProcessing::Destroy(videoProcessor);
}

VideoProcessing_ErrorCode OH_VideoProcessing_RegisterCallback(OH_VideoProcessing* videoProcessor,
    const VideoProcessing_Callback* callback, void* userData)
{
    return CallVideoProcessing(videoProcessor, [&callback, &userData](std::shared_ptr<IVideoProcessingNative>& obj) {
        return obj->RegisterCallback(callback, userData);
    });
}

VideoProcessing_ErrorCode OH_VideoProcessing_SetSurface(OH_VideoProcessing* videoProcessor,
    const OHNativeWindow* window)
{
    return CallVideoProcessing(videoProcessor, [&window](std::shared_ptr<IVideoProcessingNative>& obj) {
        return obj->SetSurface(window);
    });
}

VideoProcessing_ErrorCode OH_VideoProcessing_GetSurface(OH_VideoProcessing* videoProcessor, OHNativeWindow** window)
{
    return CallVideoProcessing(videoProcessor, [&window](std::shared_ptr<IVideoProcessingNative>& obj) {
        return obj->GetSurface(window);
    });
}

VideoProcessing_ErrorCode OH_VideoProcessing_SetParameter(OH_VideoProcessing* videoProcessor,
    const OH_AVFormat* parameter)
{
    return CallVideoProcessing(videoProcessor, [&parameter](std::shared_ptr<IVideoProcessingNative>& obj) {
        return obj->SetParameter(parameter);
    });
}

VideoProcessing_ErrorCode OH_VideoProcessing_GetParameter(OH_VideoProcessing* videoProcessor, OH_AVFormat* parameter)
{
    return CallVideoProcessing(videoProcessor, [&parameter](std::shared_ptr<IVideoProcessingNative>& obj) {
        return obj->GetParameter(parameter);
    });
}

VideoProcessing_ErrorCode OH_VideoProcessing_Start(OH_VideoProcessing* videoProcessor)
{
    return CallVideoProcessing(videoProcessor, [](std::shared_ptr<IVideoProcessingNative>& obj) {
        return obj->Start();
    });
}

VideoProcessing_ErrorCode OH_VideoProcessing_Stop(OH_VideoProcessing* videoProcessor)
{
    return CallVideoProcessing(videoProcessor, [](std::shared_ptr<IVideoProcessingNative>& obj) {
        return obj->Stop();
    });
}

VideoProcessing_ErrorCode OH_VideoProcessing_RenderOutputBuffer(OH_VideoProcessing* videoProcessor, uint32_t index)
{
    return CallVideoProcessing(videoProcessor, [&index](std::shared_ptr<IVideoProcessingNative>& obj) {
        return obj->RenderOutputBuffer(index);
    });
}

VideoProcessing_ErrorCode OH_VideoProcessingCallback_Create(VideoProcessing_Callback** callback)
{
    return VideoProcessing_Callback::Create(callback);
}

VideoProcessing_ErrorCode OH_VideoProcessingCallback_Destroy(VideoProcessing_Callback* callback)
{
    return VideoProcessing_Callback::Destroy(callback);
}

VideoProcessing_ErrorCode OH_VideoProcessingCallback_BindOnError(VideoProcessing_Callback* callback,
    OH_VideoProcessingCallback_OnError onError)
{
    return CallVideoProcessingCallback(callback, [&onError](std::shared_ptr<VideoProcessingCallbackNative>& obj) {
        return obj->BindOnError(onError);
    });
}

VideoProcessing_ErrorCode OH_VideoProcessingCallback_BindOnState(VideoProcessing_Callback* callback,
    OH_VideoProcessingCallback_OnState onState)
{
    return CallVideoProcessingCallback(callback, [&onState](std::shared_ptr<VideoProcessingCallbackNative>& obj) {
        return obj->BindOnState(onState);
    });
}

VideoProcessing_ErrorCode OH_VideoProcessingCallback_BindOnNewOutputBuffer(VideoProcessing_Callback* callback,
    OH_VideoProcessingCallback_OnNewOutputBuffer onNewOutputBuffer)
{
    return CallVideoProcessingCallback(callback,
        [&onNewOutputBuffer](std::shared_ptr<VideoProcessingCallbackNative>& obj) {
            return obj->BindOnNewOutputBuffer(onNewOutputBuffer);
        });
}
