/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef VIDEO_PROCESSING_DEMO_H
#define VIDEO_PROCESSING_DEMO_H

#include <condition_variable>
#include <atomic>
#include <thread>

#include "native_avcodec_base.h"
#include "native_avdemuxer.h"
#include "native_avmuxer.h"
#include "native_avsource.h"
#include "native_avformat.h"
#include "video_processing.h"
namespace OHOS {
class VideoProcessingDemo {
public:
    VideoProcessingDemo(const char *file, bool isMetaDataGen,
        VideoProcessing_ColorSpaceInfo inInfo,
        VideoProcessing_ColorSpaceInfo outInfo);
    ~VideoProcessingDemo();
    void SetTrackFormat();
    void ConfigureCodec();
    viod ConfigureProcessor();
    void Start();
    void WaitForEOS();
    void Stop();
    void WriteAudioTrack();
    OH_AVDemuxer *demuxer = nullptr;
    OH_AVMuxer *muxer = nullptr;
    uint32_t videoTrackID = -1;
    uint32_t audioTrackID = -1;
    uint32_t muxVideoTrackID = -1;
    uint32_t muxAudioTrackID = -1;
    OH_AVCodec *dec = nullptr;
    OH_AVCodec *enc = nullptr;
    std::condition_variable waitCond;
    std::atomic<bool> isFinish;
    uint32_t frameDuration = 0;
    std::unique_ptr<std::thread> audioThread;
    bool needRender = false;
private:
    int32_t width_ = 0;
    int32_t height_ = 0;
    int32_t isHDRVividOut = 0;
    VideoProcessing_ColorSpaceInfo inInfo_;
    VideoProcessing_ColorSpaceInfo outInfo_;
    OH_AVSource *inSource = nullptr;
    int32_t trackCount = 0;
    int32_t fd;
    int32_t outFd;
    OH_VideoProcessing* videoProcessor = nullptr;
    VideoProcessing_Callback* callback = nullptr;
    OHNativeWindow *inWindow = nullptr;
    const OHNativeWindow *outWindow = nullptr;
    std::unique_ptr<YuvViewer> viewer;
};
}

#endif
