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
#include <fstream>
#include "video_sample.h"
using namespace OHOS;
using namespace OHOS::Media;
using namespace std;
static uint32_t g_onErrorCount = 0;
static VideoProcessing_State g_state = VIDEO_PROCESSING_STATE_STOPPED;
static std::mutex g_Mutex;
static std::condition_variable g_Cond;
constexpr std::chrono::seconds STOP_TIMEOUT(10);
constexpr int64_t NANOS_IN_SECOND = 1000000000L;
constexpr int64_t NANOS_IN_MICRO = 1000L;

class TestConsumerListener : public IBufferConsumerListener {
public:
    TestConsumerListener(sptr<Surface> cs, std::string_view name) : cs(cs) {};
    ~TestConsumerListener() {}
    void OnBufferAvailable() override
    {
        sptr<SurfaceBuffer> buffer;
        int32_t flushFence;
        cs->AcquireBuffer(buffer, flushFence, timestamp, damage);

        cs->ReleaseBuffer(buffer, -1);
    }

private:
    int64_t timestamp = 0;
    Rect damage = {};
    sptr<Surface> cs {nullptr};
};

static int64_t GetSystemTimeUs()
{
    struct timespec now;
    (void)clock_gettime(CLOCK_BOOTTIME, &now);
    int64_t nanoTime = static_cast<int64_t>(now.tv_sec) * NANOS_IN_SECOND + now.tv_nsec;
    return nanoTime / NANOS_IN_MICRO;
}

static void OnError(OH_VideoProcessing* videoProcessor, VideoProcessing_ErrorCode error, void* userData)
{
    g_onErrorCount++;
    std::cout << "OnError callback recv errorcode:" << error << std::endl;
}

static void OnState(OH_VideoProcessing* videoProcessor, VideoProcessing_State state, void* userData)
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

VideoSample::~VideoSample()
{
    if (inFile != nullptr) {
        if (inFile->is_open()) {
            inFile->close();
        }
        inFile.reset();
        inFile = nullptr;
    }
    if (callback) {
        OH_VideoProcessingCallback_Destroy(callback);
        callback = nullptr;
    }
    if (rect) {
        delete rect;
        rect = nullptr;
    }
    if (metaData) {
        delete[] metaData;
        metaData = nullptr;
    }

    OH_VideoProcessing_Destroy(videoProcessor);
}

int32_t VideoSample::InitVideoSample(const int32_t type, int32_t width, int32_t height, VideoProcessParam param)
{
    width_ = width;
    height_ = height;
    param_ = param;
    viewer = std::make_unique<YuvViewer>();
    int32_t ret = OH_VideoProcessing_Create(&videoProcessor, type);
    CheckAndRet(ret == VIDEO_PROCESSING_SUCCESS, ret, "OH_VideoProcessing_Create failed.");

    cs = Surface::CreateSurfaceAsConsumer();
    sptr<IBufferConsumerListener> listener = new TestConsumerListener(cs, OUT_DIR);
    cs->RegisterConsumerListener(listener);
    auto p = cs->GetProducer();
    ps = Surface::CreateSurfaceAsProducer(p);
    outWindow = CreateNativeWindowFromSurface(&ps);
    (void)OH_NativeWindow_NativeWindowHandleOpt(outWindow, SET_BUFFER_GEOMETRY, width_, height_);
    (void)OH_NativeWindow_NativeWindowHandleOpt(outWindow, SET_USAGE,
        NATIVEBUFFER_USAGE_CPU_READ | NATIVEBUFFER_USAGE_CPU_WRITE |
        NATIVEBUFFER_USAGE_MEM_DMA | NATIVEBUFFER_USAGE_HW_RENDER);
    (void)OH_NativeWindow_NativeWindowHandleOpt(outWindow, SET_FORMAT, param_.outFmt);

    ret = OH_VideoProcessing_SetSurface(videoProcessor, outWindow);
    CheckAndRet(ret == VIDEO_PROCESSING_SUCCESS, ret, "OH_VideoProcessing_SetSurface failed.");
    ret = OH_VideoProcessing_GetSurface(videoProcessor, &inWindow);
    CheckAndRet(ret == VIDEO_PROCESSING_SUCCESS, ret, "OH_VideoProcessing_GetSurface failed.");
    SetInputWindowParam();
    ret = OH_VideoProcessingCallback_Create(&callback);
    CheckAndRet(ret == VIDEO_PROCESSING_SUCCESS, ret, "OH_VideoProcessingCallback_Create failed.");
    OH_VideoProcessingCallback_BindOnError(callback, OnError);
    OH_VideoProcessingCallback_BindOnState(callback, OnState);
    OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, OnNewOutputBuffer);
    ret = OH_VideoProcessing_RegisterCallback(videoProcessor, callback, this);
    CheckAndRet(ret == VIDEO_PROCESSING_SUCCESS, ret, "OH_VideoProcessing_RegisterCallback failed.");
    return VIDEO_PROCESSING_SUCCESS;
}

void VideoSample::SetInputWindowParam()
{
    (void)OH_NativeWindow_NativeWindowHandleOpt(inWindow, SET_BUFFER_GEOMETRY, width_, height_);
    (void)OH_NativeWindow_NativeWindowHandleOpt(inWindow, SET_USAGE,
        NATIVEBUFFER_USAGE_CPU_READ | NATIVEBUFFER_USAGE_CPU_WRITE |
        NATIVEBUFFER_USAGE_MEM_DMA | NATIVEBUFFER_USAGE_HW_RENDER);
    (void)OH_NativeWindow_NativeWindowHandleOpt(inWindow, SET_FORMAT, param_.inFmt);

    if (isHDRVivid) {
        metaDataFile = std::make_unique<std::ifstream>(inputMetaPath);
        metaDataFile->seekg(0, ios::end);
        metadataSize = metaDataFile->tellg();
        metaDataFile->seekg(0, ios::beg);
        if (metadataSize > 0) {
            metaData = new uint8_t[metadataSize];
            metaDataFile->read(reinterpret_cast<char*>(metaData), metadataSize);
        }
    }
    rect = new Region::Rect();
    rect->x = 0;
    rect->y = 0;
    rect->w = width_;
    rect->h = height_;
    region.rects = rect;
}

int32_t VideoSample::InputFunc()
{
    inFile = std::make_unique<std::ifstream>(inputFilePath);
    for (int32_t i = 0; i < inputFrameNumber; i++) {
        int fenceFd = -1;
        OHNativeWindowBuffer *ohNativeWindowBuffer;
        int32_t err = OH_NativeWindow_NativeWindowRequestBuffer(inWindow, &ohNativeWindowBuffer, &fenceFd);
        CheckAndRet(err == 0, err, "OH_NativeWindow_NativeWindowRequestBuffer failed.");
        if (fenceFd > 0) {
            close(fenceFd);
        }
        OH_NativeBuffer *nativeBuffer = nullptr;
        err = OH_NativeBuffer_FromNativeWindowBuffer(ohNativeWindowBuffer, &nativeBuffer);
        CheckAndRet(err == 0, err, "OH_NativeBuffer_FromNativeWindowBuffer failed.");
        void *virAddr = nullptr;
        OH_NativeBuffer_Config config;
        OH_NativeBuffer_GetConfig(nativeBuffer, &config);
        err = OH_NativeBuffer_Map(nativeBuffer, &virAddr);
        CheckAndRet(err == 0, err, "OH_NativeBuffer_Map failed.");
        if (inFile->is_open()) {
            if (param_.inFmt == NATIVEBUFFER_PIXEL_FMT_YCBCR_P010 ||
            param_.inFmt == NATIVEBUFFER_PIXEL_FMT_YCRCB_P010) {
                ReadOneFrameP010(reinterpret_cast<uint8_t *>(virAddr), config);
            } else if (param_.inFmt == NATIVEBUFFER_PIXEL_FMT_RGBA_1010102) {
                ReadOneFrameRGBA10(reinterpret_cast<uint8_t *>(virAddr), config);
            }
            inFile->seekg(0, ios::beg);
        }
        NativeWindowHandleOpt(inWindow, SET_UI_TIMESTAMP, GetSystemTimeUs());
        err = OH_NativeBuffer_Unmap(nativeBuffer);
        CheckAndRet(err == 0, err, "OH_NativeBuffer_Unmap failed.");
        err = OH_NativeWindow_NativeWindowFlushBuffer(inWindow, ohNativeWindowBuffer, -1, region);
        CheckAndRet(err == 0, err, "OH_NativeWindow_NativeWindowFlushBuffer failed.");
        err = OH_NativeWindow_SetColorSpace(inWindow, param_.inColorSpace);
        CheckAndRet(err == 0, err, "OH_NativeWindow_SetColorSpace failed.");
        if (isHDRVivid) {
            uint8_t val = OH_VIDEO_HDR_VIVID;
            err = OH_NativeWindow_SetMetadataValue(inWindow, OH_HDR_METADATA_TYPE, sizeof(uint8_t), &val);
            CheckAndRet(err == 0, err, "set OH_HDR_METADATA_TYPE failed.");
            err = OH_NativeWindow_SetMetadataValue(inWindow, OH_HDR_DYNAMIC_METADATA, metadataSize, metaData);
            CheckAndRet(err == 0, err, "set OH_VIDEO_HDR_VIVID failed.");
        }
    }
    return 0;
}

int32_t VideoSample::StartProcess()
{
    int32_t ret = OH_VideoProcessing_Start(videoProcessor);
    CheckAndRet(ret == VIDEO_PROCESSING_SUCCESS, ret, "OH_VideoProcessing_Start failed.");
    inputLoop_ = make_unique<thread>(&VideoSample::InputFunc, this);
    return VIDEO_PROCESSING_SUCCESS;
}

int32_t VideoSample::WaitAndStopSample()
{
    inputLoop_->join();
    int32_t ret = OH_VideoProcessing_Stop(videoProcessor);
    CheckAndRet(ret == VIDEO_PROCESSING_SUCCESS, ret, "OH_VideoProcessing_Stop failed.");
    unique_lock<mutex> lock(g_Mutex);
    if (g_Cond.wait_for(lock, STOP_TIMEOUT) == std::cv_status::timeout) {
        std::cout << "waiting stop state timeout" << std::endl;
    }
    return g_onErrorCount;
}

int32_t VideoSample::ReadOneFrameP010(uint8_t *addr, OH_NativeBuffer_Config config)
{
    uint8_t *start = addr;
    // copy Y
    for (uint32_t i = 0; i < config.height; i++) {
        inFile->read(reinterpret_cast<char *>(addr), width_ * sizeof(uint16_t));
        addr += config.stride;
    }
    // copy UV
    for (uint32_t i = 0; i < config.height / sizeof(uint16_t); i++) {
        inFile->read(reinterpret_cast<char *>(addr),  width_ * sizeof(uint16_t));
        addr += config.stride;
    }
    return addr - start;
}

int32_t VideoSample::ReadOneFrameRGBA10(uint8_t *addr, OH_NativeBuffer_Config config)
{
    uint8_t *start = addr;
    for (uint32_t i = 0; i < height_; i++) {
        inFile->read(reinterpret_cast<char *>(addr), width_ * sizeof(uint32_t));
        addr += config.stride;
    }
    return addr - start;
}
