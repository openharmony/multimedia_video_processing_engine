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

#ifndef VIDEO_PROCESSING_CAPI_IMPL_H
#define VIDEO_PROCESSING_CAPI_IMPL_H

#include "video_processing_capi_interface.h"
#include "algorithm_common.h"

class VideoProcessingCapiImpl : public IVideoProcessingNdk {
public:
    VideoProcessingCapiImpl() = default;
    virtual ~VideoProcessingCapiImpl() = default;
    VideoProcessingCapiImpl(const VideoProcessingCapiImpl&) = delete;
    VideoProcessingCapiImpl& operator=(const VideoProcessingCapiImpl&) = delete;
    VideoProcessingCapiImpl(VideoProcessingCapiImpl&&) = delete;
    VideoProcessingCapiImpl& operator=(VideoProcessingCapiImpl&&) = delete;

    VideoProcessing_ErrorCode InitializeEnvironment() final;
    VideoProcessing_ErrorCode DeinitializeEnvironment() final;
    bool IsColorSpaceConversionSupported(const VideoProcessing_ColorSpaceInfo* sourceVideoInfo,
        const VideoProcessing_ColorSpaceInfo* destinationVideoInfo) final;
    bool IsMetadataGenerationSupported(const VideoProcessing_ColorSpaceInfo* sourceVideoInfo) final;
    VideoProcessing_ErrorCode Create(OH_VideoProcessing** videoProcessor, int type) final;
    VideoProcessing_ErrorCode Destroy(OH_VideoProcessing* videoProcessor) final;
    VideoProcessing_ErrorCode RegisterCallback(OH_VideoProcessing* videoProcessor,
        const VideoProcessing_Callback* callback, void* userData) final;
    VideoProcessing_ErrorCode SetSurface(OH_VideoProcessing* videoProcessor, const OHNativeWindow* window) final;
    VideoProcessing_ErrorCode GetSurface(OH_VideoProcessing* videoProcessor, OHNativeWindow** window) final;
    VideoProcessing_ErrorCode SetParameter(OH_VideoProcessing* videoProcessor,
        const OH_AVFormat* parameter) final;
    VideoProcessing_ErrorCode GetParameter(OH_VideoProcessing* videoProcessor, OH_AVFormat* parameter) final;
    VideoProcessing_ErrorCode Start(OH_VideoProcessing* videoProcessor) final;
    VideoProcessing_ErrorCode Stop(OH_VideoProcessing* videoProcessor) final;
    VideoProcessing_ErrorCode RenderOutputBuffer(OH_VideoProcessing* videoProcessor, uint32_t index) final;

    VideoProcessing_ErrorCode Create(VideoProcessing_Callback** callback) final;
    VideoProcessing_ErrorCode Destroy(VideoProcessing_Callback* callback) final;
    VideoProcessing_ErrorCode BindOnError(VideoProcessing_Callback* callback,
        OH_VideoProcessingCallback_OnError onError) final;
    VideoProcessing_ErrorCode BindOnState(VideoProcessing_Callback* callback,
        OH_VideoProcessingCallback_OnState onState) final;
    VideoProcessing_ErrorCode BindOnNewOutputBuffer(VideoProcessing_Callback* callback,
        OH_VideoProcessingCallback_OnNewOutputBuffer onNewOutputBuffer) final;
private:
    std::shared_ptr<OpenGLContext> openglContext_ {nullptr};
    VideoProcessing_ErrorCode OpenGLInit();
};

#endif // VIDEO_PROCESSING_CAPI_IMPL_H
