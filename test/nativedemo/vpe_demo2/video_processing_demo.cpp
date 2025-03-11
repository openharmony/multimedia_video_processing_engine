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
#include "avcodec_e2e_demo.h"

#include <iostream>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <list>
#include "av_common.h"
#include "avcodec_common.h"
#include "avcodec_errors.h"
#include "native_avcodec_videodecoder.h"
#include "native_avcodec_videoencoder.h"
#include "media_description.h"
#include "native_avformat.h"
#include "native_avcodec_base.h"

using namespace OHOS;
using namespace std;
constexpr int64_t MICRO_IN_SECOND = 1000000L;
constexpr float FRAME_INTERVAL_TIMES = 1.5;
constexpr int32_t AUDIO_BUFFER_SIZE = 1024 * 1024;
constexpr int8_t ARGV_PARAM1 = 1;
constexpr int8_t ARGV_PARAM2 = 2;
constexpr int8_t ARGV_PARAM3 = 3;
constexpr int8_t ARGV_PARAM4 = 4;
constexpr int8_t ARGV_PARAM5 = 5;
constexpr int8_t ARGV_PARAM6 = 6;
constexpr int8_t ARGV_MIN_LENGTH = 7;
constexpr double DEFAULT_FRAME_RATE = 25.0;
constexpr std::chrono::seconds STOP_TIMEOUT(10);

static uint32_t g_onErrorCount = 0;
static VideoProcessing_State g_state = VIDEO_PROCESSING_STATE_STOPPED;
static std::mutex g_Mutex;
static std::condition_variable g_Cond;

static int64_t GetFileSize(const char *fileName)
{
    int64_t fileSize = 0;
    if (fileName != nullptr) {
        struct stat fileStatus {};
        if (stat(fileName, &fileStatus) == 0) {
            fileSize = static_cast<int64_t>(fileStatus.st_size);
        }
    }
    return fileSize;
}

static void OnError(OH_AVCodec *codec, int32_t errorCode, void *userData)
{
    (void)codec;
    (void)userData;
    cout<<"error :"<<errorCode<<endl;
}

static void OnDecStreamChanged(OH_AVCodec *codec, OH_AVFormat *format, void *userData)
{
    (void)codec;
    (void)format;
    (void)userData;
}

static void OnDecInputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVBuffer *buffer, void *userData)
{
    VideoProcessingDemo *demo = static_cast<VideoProcessingDemo*>(userData);
    OH_AVDemuxer_ReadSampleBuffer(demo->demuxer, demo->videoTrackID, buffer);
    OH_VideoDecoder_PushInputBuffer(codec, index);
}

static void OnDecOutputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVBuffer *buffer, void *userData)
{
    VideoProcessingDemo *demo = static_cast<VideoProcessingDemo*>(userData);
    OH_AVCodecBufferAttr attr;
    OH_AVBuffer_GetBufferAttr(buffer, &attr);
    if (attr.flags & AVCODEC_BUFFER_FLAGS_EOS) {
        OH_VideoEncoder_NotifyEndOfStream(demo->enc);
    }
    OH_VideoDecoder_RenderOutputBuffer(codec, index);
}

static void OnEncStreamChanged(OH_AVCodec *codec, OH_AVFormat *format, void *userData)
{
    cout<<"format changed"<<endl;
}

static void OnEncInputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVBuffer *buffer, void *userData)
{
    (void)codec;
    (void)index;
    (void)buffer;
    (void)userData;
}

static void OnEncOutputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVBuffer *buffer, void *userData)
{
    VideoProcessingDemo *demo = static_cast<VideoProcessingDemo*>(userData);
    OH_AVCodecBufferAttr attr;
    OH_AVBuffer_GetBufferAttr(buffer, &attr);
    if (attr.flags & AVCODEC_BUFFER_FLAGS_EOS) {
        demo->isFinish.store(true);
        demo->waitCond.notify_all();
        return;
    }
    OH_AVMuxer_WriteSampleBuffer(demo->muxer, 0, buffer);
    OH_VideoEncoder_FreeOutputBuffer(codec, index);
}


static void OnVPEError(OH_VideoProcessing* videoProcessor, VideoProcessing_ErrorCode error, void* userData)
{
    g_onErrorCount++;
    std::cout << "OnError callback recv errorcode:" << error << std::endl;
}

static void OnVPEState(OH_VideoProcessing* videoProcessor, VideoProcessing_State state, void* userData)
{
    g_state = state;
    if (state == VIDEO_PROCESSING_STATE_STOPPED) {
        g_Cond.notify_all();
    }
    std::cout << "OnState callback called, new state is "<< state << std::endl;
}

static void OnNewOutputBuffer(OH_VideoProcessing* videoProcessor, uint32_t index, void* userData)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_RenderOutputBuffer(videoProcessor, index);
    if (ret != VIDEO_PROCESSING_SUCCESS) {
        g_onErrorCount++;
        std::cout << "Render output buffer failed,errcode: "<< ret << std::endl;
    }
}

VideoProcessingDemo::VideoProcessingDemo(const char *file, VideoProcessing_ColorSpaceInfo inInfo,
                                         VideoProcessing_ColorSpaceInfo outInfo)
{
    inInfo_ = inInfo;
    outInfo_ = outInfo;
    fd = open(file, O_RDONLY);
    outFd = open("./output.mp4", O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    int64_t size = GetFileSize(file);
    inSource = OH_AVSource_CreateWithFD(fd, 0, size);
    if (!inSource) {
        cout << "create source failed" << endl;
    }
    OH_VideoProcessing_InitializeEnvironment();
    demuxer = OH_AVDemuxer_CreateWithSource(inSource);
    muxer = OH_AVMuxer_Create(outFd, AV_OUTPUT_FORMAT_MPEG_4);
    if (!muxer || !demuxer) {
        cout << "create muxer demuxer failed" << endl;
    }
    OH_AVFormat *sourceFormat = OH_AVSource_GetSourceFormat(inSource);
    OH_AVFormat_GetIntValue(sourceFormat, OH_MD_KEY_TRACK_COUNT, &trackCount);
    for (int32_t index = 0; index < trackCount; index++) {
        SetTrackFormat(index);
    }
    OH_AVFormat_Destroy(sourceFormat);
}

void VideoProcessingDemo::SetTrackFormat(int32_t index)
{
    OH_AVFormat *trackFormat = OH_AVSource_GetTrackFormat(inSource, index);
    OH_AVDemuxer_SelectTrackByID(demuxer, index);
    int32_t trackType = -1;
    OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_TRACK_TYPE, &trackType);
    if (trackType == MEDIA_TYPE_VID) {
        videoTrackID = index;
        OH_AVMuxer_AddTrack(muxer, &muxVideoTrackID, trackFormat);
        OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_TRACK_TYPE, &trackType);
        char *mime = nullptr;
        OH_AVFormat_GetStringValue(trackFormat, OH_MD_KEY_CODEC_MIME, &mime);
        dec = OH_VideoDecoder_CreateByMime(mime);
        if (!needRender) {
            enc = OH_VideoEncoder_CreateByMime(OH_AVCODEC_MIMETYPE_VIDEO_HEVC);
        }
        if (isMetadataGen) {
            OH_VideoProcessing_Create(&videoProcessor, VIDEO_PROCESSING_TYPE_METADATA_GENERATION);
        } else {
            OH_VideoProcessing_Create(&videoProcessor, VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION);
        }
        if (!enc || !dec || !videoProcessor) {
            cout << "create codec or processor failed" << endl;
            return;
        }
        int32_t isVividIn = false;
        OH_AVFormat_GetIntValue(trackFormat, OH_MD_KEY_VIDEO_IS_HDR_VIVID, &isVividIn);
        viewer = std::make_unique<YuvViewer>();
        OH_AVFormat_SetIntValue(trackFormat, OH_MD_KEY_PIXEL_FORMAT, inInfo_.pixelFormat);
        OH_VideoDecoder_Configure(dec, trackFormat);
        OH_AVFormat_SetIntValue(trackFormat, OH_MD_KEY_PIXEL_FORMAT, outInfo_.pixelFormat);
        if (outInfo_.colorSpace == OH_COLORSPACE_BT2020_HLG_LIMIT ||
            outInfo_.colorSpace == OH_COLORSPACE_BT2020_PQ_LIMIT) {
            OH_AVFormat_SetIntValue(trackFormat, OH_MD_KEY_PROFILE, HEVC_PROFILE_MAIN_10);
            isHDRVividOut = isVividIn;
        } else {
            OH_AVFormat_SetIntValue(trackFormat, OH_MD_KEY_PROFILE, HEVC_PROFILE_MAIN);
        }
        if (!needRender) {
            OH_VideoEncoder_Configure(enc, trackFormat);
        }
    } else if (trackType == MEDIA_TYPE_AUD) {
        audioTrackID = index;
        OH_AVMuxer_AddTrack(muxer, &muxAudioTrackID, trackFormat);
    }
    OH_AVFormat_Destroy(trackFormat);
}


VideoProcessingDemo::~VideoProcessingDemo()
{
    OH_VideoProcessing_Destroy(videoProcessor);
    OH_VideoProcessing_DeinitializeEnvironment();
    if (dec) {
        OH_VideoDecoder_Destroy(dec);
    }
    if (enc) {
        OH_VideoEncoder_Destroy(enc);
    }
    if (muxer) {
        OH_AVMuxer_Destroy(muxer);
    }
    if (demuxer) {
        OH_AVDemuxer_Destroy(demuxer);
    }
    if (inSource) {
        OH_AVSource_Destroy(inSource);
    }
    close(fd);
    close(outFd);
}

void VideoProcessingDemo::ConfigureCodec()
{
    if (!needRender) {
        OH_AVCodecCallback encCallback;
        encCallback.onError = OnError;
        encCallback.onStreamChanged = OnEncStreamChanged;
        encCallback.onNeedInputBuffer = OnEncInputBufferAvailable;
        encCallback.onNewOutputBuffer = OnEncOutputBufferAvailable;
        OH_VideoEncoder_RegisterCallback(enc, encCallback, this);
    }
    OH_AVCodecCallback decCallback;
    decCallback.onError = OnError;
    decCallback.onStreamChanged = OnDecStreamChanged;
    decCallback.onNeedInputBuffer = OnDecInputBufferAvailable;
    decCallback.onNewOutputBuffer = OnDecOutputBufferAvailable;
    OH_VideoDecoder_RegisterCallback(dec, decCallback, this);
    if (needRender) {
        outWindow = viewer->CreateWindow(width_, height_, param.outFmt, param.outColorSpace, isHDRVividOut);
    } else {
        OH_VideoEncoder_GetSurface(enc, &outWindow);
    }
    int32_t ret = OH_VideoProcessing_SetSurface(videoProcessor, outWindow);
    if (ret != VIDEO_PROCESSING_SUCCESS) {
        std::cout<<" get vpe input surface failed"<<std::endl;
        return;
    }
    int32_t ret = OH_VideoProcessing_GetSurface(videoProcessor, &inWindow);
    if (ret != VIDEO_PROCESSING_SUCCESS) {
        std::cout<<" get vpe input surface failed"<<std::endl;
        return;
    }
    ret = OH_VideoDecoder_SetSurface(dec, inWindow);
    if (ret != AV_ERR_OK) {
        std::cout<<" set decoder output surface failed"<<std::endl;
        return;
    }
    isFinish.store(false);
}

void VideoProcessingDemo::ConfigureProcessor()
{
    OH_VideoProcessingCallback_Create(&callback);
    OH_VideoProcessingCallback_BindOnError(callback, OnVPEError);
    OH_VideoProcessingCallback_BindOnState(callback, OnVPEState);
    OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, OnNewOutputBuffer);
    int32_t ret = OH_VideoProcessing_RegisterCallback(videoProcessor, callback, this);
    if (ret != VIDEO_PROCESSING_SUCCESS) {
        std::cout<<"register vpe callback failed"<<std::endl;
        return;
    }
}

void VideoProcessingDemo::WriteAudioTrack()
{
    OH_AVBuffer *buffer = nullptr;
    buffer = OH_AVBuffer_Create(AUDIO_BUFFER_SIZE);
    while (!isFinish.load()) {
        OH_AVDemuxer_ReadSampleBuffer(demuxer, audioTrackID, buffer);
        OH_AVCodecBufferAttr attr;
        OH_AVBuffer_GetBufferAttr(buffer, &attr);
        if (attr.flags & AVCODEC_BUFFER_FLAGS_EOS) {
            break;
        }
        OH_AVMuxer_WriteSampleBuffer(muxer, audioTrackID, buffer);
    }
    OH_AVBuffer_Destroy(buffer);
}

void VideoProcessingDemo::Start()
{
    OH_VideoProcessing_Start(videoProcessor);
    OH_VideoDecoder_Prepare(dec);
    if (!needRender) {
        OH_VideoEncoder_Prepare(enc);
    }
    OH_AVMuxer_Start(muxer);
    if (!needRender) {
        OH_VideoEncoder_Start(enc);
    }
    OH_VideoDecoder_Start(dec);
    if (audioTrackID != -1) {
        audioThread = make_unique<thread>(&VideoProcessingDemo::WriteAudioTrack, this);
    }
}

void VideoProcessingDemo::WaitForEOS()
{
    std::mutex waitMtx;
    unique_lock<mutex> lock(waitMtx);
    waitCond.wait(lock, [this]() {
        return isFinish.load();
    });
    if (audioThread) {
        audioThread->join();
    }
    cout << "task finish" << endl;
}

void VideoProcessingDemo::Stop()
{
    OH_VideoDecoder_Stop(dec);
    if (!needRender) {
        OH_VideoEncoder_Stop(enc);
    }
    OH_VideoProcessing_Stop(videoProcessor);
    unique_lock<mutex> lock(g_Mutex);
    if (g_Cond.wait_for(lock, STOP_TIMEOUT) == std::cv_status::timeout) {
        std::cout << "waiting stop state timeout" << std::endl;
    }
    OH_AVMuxer_Stop(muxer);
}



int main(int32_t argc, char *argv[])
{
    if (argc < ARGV_MIN_LENGTH) {
        std::cout<< "parameter not enough."<<std::endl;
        return 0;
    }
    char *path = argv[ARGV_PARAM1];
    bool isMetadataGen = bool(atoi(argv[ARGV_PARAM2]));
    inInfo.pixelFormat = atoi(argv[ARGV_PARAM3]);
    outInfo.colorSpace = atoi(argv[ARGV_PARAM4]);
    outInfo.pixelFormat = atoi(argv[ARGV_PARAM5]);
    VideoProcessing_ColorSpaceInfo inInfo;
    VideoProcessing_ColorSpaceInfo outInfo;
    auto vpeDemo = std::make_unique<VideoProcessingDemo>(path, isMetadataGen, inInfo, outInfo);
    vpeDemo->needRender = atoi(argv[ARGV_PARAM6]);
    vpeDemo->Configure();
    vpeDemo->Start();
    vpeDemo->WaitForEOS();
    vpeDemo->Stop();
    return 0;
}