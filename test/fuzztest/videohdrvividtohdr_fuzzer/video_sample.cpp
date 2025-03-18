/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef VIDEO_SAMPLE_H
#define VIDEO_SAMPLE_H

#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include "video_sample.h"
#include "securec.h"
#include "sync_fence.h"

using namespace OHOS;
using namespace std;

constexpr int64_t NANOS_IN_SECOND = 1000000000L;
constexpr int64_t NANOS_IN_MICRO = 1000L;
constexpr int64_t SLEEP_MICROSECONDS = 33333L;
constexpr int THREE = 3;

namespace OHOS {
int64_t GetSystemTimeUs()
{
    struct timespec now;
    (void)clock_gettime(CLOCK_BOOTTIME, &now);
    int64_t nanoTime = static_cast<int64_t>(now.tv_sec) * NANOS_IN_SECOND + now.tv_nsec;
    return nanoTime / NANOS_IN_MICRO;
}

void OnError(OH_VideoProcessing* videoProcessor, VideoProcessing_ErrorCode error, void* userData)
{
    (void)videoProcessor;
    (void)error;
    (void)userData;
}

void OnState(OH_VideoProcessing* videoProcessor, VideoProcessing_State state, void* userData)
{
    g_state = state;
    if (state == VIDEO_PROCESSING_STATE_STOPPED) {
        g_Cond.notify_all();
    }
    std::cout << "OnState callback called, new state is "<< state << std::endl;
}

void OnNewOutputBuffer(OH_VideoProcessing* videoProcessor, uint32_t index, void* userData)
{
    OH_VideoProcessing_RenderOutputBuffer(videoProcessor, index);
}
}

class VPEConsumerListener : public IBufferConsumerListener {
public:
    explicit VPEConsumerListener(sptr<Surface> cs) : cs(cs) {};
    ~VPEConsumerListener() {}
    void OnBufferAvailable() override
    {
        sptr<SurfaceBuffer> buffer;
        Rect damage = {};
        cs->AcquireBuffer(buffer, fence, timestamp, damage);
        cs->ReleaseBuffer(buffer, -1);
    }

private:
    int64_t timestamp = 0;
    int32_t fence = -1;
    sptr<Surface> cs {nullptr};
};

VideoSample::~VideoSample()
{
    if (callback) {
        OH_VideoProcessingCallback_Destroy(callback);
        callback = nullptr;
    }
    if (rect) {
        delete rect;
        rect = nullptr;
    }
    OH_VideoProcessing_Destroy(videoProcessor);
    OH_NativeWindow_DestroyNativeWindow(outWindow);
    OH_NativeWindow_DestroyNativeWindow(inWindow);
    if (cs) {
        cs->UnregisterConsumerListener();
    }
}

int32_t VideoSample::InitVideoSample(int32_t width, int32_t height, VideoProcessParam param)
{
    width_ = width;
    height_ = height;
    isRunning = true;
    param_ = param;
    int32_t ret = OH_VideoProcessing_Create(&videoProcessor, VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION);
    CHECK_AND_RETURN_RET(ret == VIDEO_PROCESSING_SUCCESS, ret, "OH_VideoProcessing_Create failed.");
    cs = Surface::CreateSurfaceAsConsumer();
    sptr<IBufferConsumerListener> listener = new VPEConsumerListener(cs);
    cs->RegisterConsumerListener(listener);
    auto p = cs->GetProducer();
    sptr<Surface> ps = Surface::CreateSurfaceAsProducer(p);
    outWindow = CreateNativeWindowFromSurface(&ps);
    (void)OH_NativeWindow_NativeWindowHandleOpt(outWindow, SET_FORMAT, param_.outFmt);
    ret = OH_NativeWindow_SetColorSpace(outWindow, param_.outColorSpace);

    ret = OH_VideoProcessing_SetSurface(videoProcessor, outWindow);
    CHECK_AND_RETURN_RET(ret == VIDEO_PROCESSING_SUCCESS, ret, "OH_VideoProcessing_SetSurface failed.");
    ret = OH_VideoProcessing_GetSurface(videoProcessor, &inWindow);
    CHECK_AND_RETURN_RET(ret == VIDEO_PROCESSING_SUCCESS, ret, "OH_VideoProcessing_GetSurface failed.");
    SetInputWindowParam();
    ret = OH_VideoProcessingCallback_Create(&callback);
    CHECK_AND_RETURN_RET(ret == VIDEO_PROCESSING_SUCCESS, ret, "OH_VideoProcessingCallback_Create failed.");
    OH_VideoProcessingCallback_BindOnError(callback, OnError);
    OH_VideoProcessingCallback_BindOnState(callback, OnState);
    OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, OnNewOutputBuffer);
    ret = OH_VideoProcessing_RegisterCallback(videoProcessor, callback, this);
    CHECK_AND_RETURN_RET(ret == VIDEO_PROCESSING_SUCCESS, ret, "OH_VideoProcessing_RegisterCallback failed.");
    return VIDEO_PROCESSING_SUCCESS;
}

void VideoSample::SetInputWindowParam()
{
    (void)OH_NativeWindow_NativeWindowHandleOpt(inWindow, SET_BUFFER_GEOMETRY, width_, height_);
    (void)OH_NativeWindow_NativeWindowHandleOpt(inWindow, SET_USAGE,
        NATIVEBUFFER_USAGE_CPU_READ | NATIVEBUFFER_USAGE_CPU_WRITE |
        NATIVEBUFFER_USAGE_MEM_DMA | NATIVEBUFFER_USAGE_HW_RENDER);
    (void)OH_NativeWindow_NativeWindowHandleOpt(inWindow, SET_FORMAT, param_.inFmt);
    OH_NativeWindow_SetColorSpace(inWindow, param_.inColorSpace);
    int32_t err = 0;
    metaDataFile = std::make_unique<std::ifstream>(inputMetaPath);
    metaDataFile->seekg(0, ios::end);
    metadataSize = metaDataFile->tellg();
    metaDataFile->seekg(0, ios::beg);
    if (metadataSize > 0) {
        metaData = new uint8_t[metadataSize];
        metaDataFile->read(reinterpret_cast<char*>(metaData), metadataSize);
    } else {
        cout << "invalid metadata size" << endl;
        metaData = nullptr;
        return
    }
    uint8_t val = OH_VIDEO_HDR_VIVID;
    err = OH_NativeWindow_SetMetadataValue(inWindow, OH_HDR_METADATA_TYPE, sizeof(uint8_t), &val);
    if (err != 0) {
        cout << "set metadata type failed" << endl;
    }

    rect = new Region::Rect();
    rect->x = 0;
    rect->y = 0;
    rect->w = width_;
    rect->h = height_;
    region.rects = rect;
}

int32_t VideoSample::InputFunc(const uint8_t *data, size_t size)
{
    int fenceFd = -1;
    OHNativeWindowBuffer *ohNativeWindowBuffer;
    int32_t ret = OH_NativeWindow_SetMetadataValue(inWindow, OH_HDR_DYNAMIC_METADATA, metadataSize, metaData);
    CHECK_AND_RETURN_RET(ret == 0, ret, "set metadata value failed");
    int32_t err = OH_NativeWindow_NativeWindowRequestBuffer(inWindow, &ohNativeWindowBuffer, &fenceFd);
    CHECK_AND_RETURN_RET(err == 0, err, "OH_NativeWindow_NativeWindowRequestBuffer failed.");
    if (fenceFd > 0) {
        close(fenceFd);
    }
    OH_NativeBuffer *nativeBuffer = nullptr;
    err = OH_NativeBuffer_FromNativeWindowBuffer(ohNativeWindowBuffer, &nativeBuffer);
    CHECK_AND_RETURN_RET(err == 0, err, "OH_NativeBuffer_FromNativeWindowBuffer failed.");
    void *virAddr = nullptr;
    OH_NativeBuffer_Config config;
    OH_NativeBuffer_GetConfig(nativeBuffer, &config);
    err = OH_NativeBuffer_Map(nativeBuffer, &virAddr);
    CHECK_AND_RETURN_RET(err == 0, err, "OH_NativeBuffer_Map failed.");
    uint8_t *addr = reinterpret_cast<uint8_t *>(virAddr);
    memcpy_s(addr, config.stride * config.height * THREE, data, size);
    NativeWindowHandleOpt(inWindow, SET_UI_TIMESTAMP, GetSystemTimeUs());
    err = OH_NativeBuffer_Unmap(nativeBuffer);
    CHECK_AND_RETURN_RET(err == 0, err, "OH_NativeBuffer_Unmap failed.");
    err = OH_NativeWindow_NativeWindowFlushBuffer(inWindow, ohNativeWindowBuffer, -1, region);
    CHECK_AND_RETURN_RET(err == 0, err, "OH_NativeWindow_NativeWindowFlushBuffer failed.");
    err = OH_NativeWindow_SetColorSpace(inWindow, param_.inColorSpace);
    CHECK_AND_RETURN_RET(err == 0, err, "OH_NativeWindow_SetColorSpace failed.");
    usleep(SLEEP_MICROSECONDS);
    return err;
}

int32_t VideoSample::StartProcess()
{
    int32_t ret = OH_VideoProcessing_Start(videoProcessor);
    CHECK_AND_RETURN_RET(ret == VIDEO_PROCESSING_SUCCESS, ret, "OH_VideoProcessing_Start failed.");
    return VIDEO_PROCESSING_SUCCESS;
}

#endif