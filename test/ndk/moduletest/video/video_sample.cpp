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
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include "video_sample.h"
#include "sync_fence.h"
#include "nlohmann/json.hpp"
#include "securec.h"
using namespace OHOS;
using namespace std;
using namespace nlohmann;

constexpr int64_t NANOS_IN_SECOND = 1000000000L;
constexpr int64_t NANOS_IN_MICRO = 1000L;
constexpr int32_t THREE = 3;
constexpr int32_t FOUR = 4;
constexpr int32_t SIXTEEN = 16;

static int64_t GetSystemTimeUs()
{
    struct timespec now;
    (void)clock_gettime(CLOCK_BOOTTIME, &now);
    int64_t nanoTime = static_cast<int64_t>(now.tv_sec) * NANOS_IN_SECOND + now.tv_nsec;
    return nanoTime / NANOS_IN_MICRO;
}

vector<uint8_t> VideoSample::LoadHashFile()
{
    ifstream f(hashValJsonPath, ios::in);
    vector<uint8_t> ret;
    if (f) {
        json data = json::parse(f);
        filesystem::path filePath = inputFilePath;
        string fileName = filePath.filename();
        string hashValue = data[fileName.c_str()][defaultPixelFormat][convertType];
        stringstream ss(hashValue);
        string item;
        while (getline(ss, item, ',')) {
            if (!item.empty()) {
                ret.push_back(stol(item, nullptr, SIXTEEN));
            }
        }
    }
    return ret;
}

void VideoSample::ShowHashValue(uint8_t source[])
{
    for (int32_t i = 0; i < SHA512_DIGEST_LENGTH; i++) {
        cout << std::hex << std::setfill('0') << std::setw(2) << int(source[i]);
        if (i != SHA512_DIGEST_LENGTH - 1) {
            cout << ",";
        }
    }
    cout << std::dec;
    cout << endl;
}

bool VideoSample::MdCompare(uint8_t source[])
{
    std::vector<uint8_t> srcHashVal = LoadHashFile();
    if (srcHashVal.size() != SHA512_DIGEST_LENGTH) {
        cout << "get hash value failed, size " <<srcHashVal.size() << endl;
        return false;
    }
    for (int32_t i = 0; i < SHA512_DIGEST_LENGTH; i++) {
        if (source[i] != srcHashVal[i]) {
            cout << "decoded hash value mismatch" << endl;
            ShowHashValue(source);
            return false;
        }
    }
    return true;
}

void VideoSample::ProcessOutputData(SurfaceBuffer *buffer)
{
    int32_t picWidth = buffer->GetWidth();
    int32_t picHeight = buffer->GetHeight();
    uint8_t *bufferAddr = reinterpret_cast<uint8_t *>(buffer->GetVirAddr());
    uint32_t cropSize = buffer->GetSize();
    if (param_.outFmt == NATIVEBUFFER_PIXEL_FMT_YCBCR_P010 ||
        param_.outFmt == NATIVEBUFFER_PIXEL_FMT_YCRCB_P010) {
        cropSize = picWidth * picHeight * THREE;
    } else if (param_.outFmt == NATIVEBUFFER_PIXEL_FMT_RGBA_1010102 ||
        param_.outFmt == NATIVEBUFFER_PIXEL_FMT_RGBA_8888) {
        cropSize = picWidth * picHeight * FOUR;
    } else if (param_.outFmt == NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP ||
        param_.outFmt == NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP) {
        cropSize = picWidth * picHeight * THREE >> 1;
    }
    uint8_t *cropBuffer = new uint8_t[cropSize];
    if (param_.outFmt == NATIVEBUFFER_PIXEL_FMT_RGBA_1010102 ||
        param_.outFmt == NATIVEBUFFER_PIXEL_FMT_RGBA_8888) {
        memcpy_s(cropBuffer, cropSize, bufferAddr, picWidth * picHeight * FOUR);
    } else if (param_.outFmt == NATIVEBUFFER_PIXEL_FMT_YCBCR_P010 ||
        param_.outFmt == NATIVEBUFFER_PIXEL_FMT_YCRCB_P010) {
        memcpy_s(cropBuffer, cropSize, bufferAddr, picWidth * picHeight * THREE);
    } else if (param_.outFmt == NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP ||
        param_.outFmt == NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP) {
        memcpy_s(cropBuffer, cropSize, bufferAddr, picWidth * picHeight * THREE >> 1);
    }
    outFile->write(reinterpret_cast<char *>(cropBuffer), cropSize);
    outFile->close();
    SHA512_Update(&ctx, cropBuffer, cropSize);
    delete[] cropBuffer;
}

static void OnError(OH_VideoProcessing* videoProcessor, VideoProcessing_ErrorCode error, void* userData)
{
    VideoSample *sample = reinterpret_cast<VideoSample*>(userData);
    sample->isRunning = false;
    sample->errCount++;
    std::cout << "OnError callback recv errorcode:" << error << std::endl;
}

static void OnState(OH_VideoProcessing* videoProcessor, VideoProcessing_State state, void* userData)
{
    std::cout << "OnState callback called, new state is "<< state << std::endl;
}

static void OnNewOutputBuffer(OH_VideoProcessing* videoProcessor, uint32_t index, void* userData)
{
    VideoSample *sample = reinterpret_cast<VideoSample*>(userData);
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_RenderOutputBuffer(videoProcessor, index);
    if (ret != VIDEO_PROCESSING_SUCCESS) {
        sample->errCount++;
        std::cout << "Render output buffer failed,errcode: "<< ret << std::endl;
    }
}


class VPEConsumerListener : public IBufferConsumerListener {
public:
    explicit VPEConsumerListener(sptr<Surface> cs, VideoSample *sample) : sample(sample), cs(cs){};
    ~VPEConsumerListener() {}
    void OnBufferAvailable() override
    {
        sptr<SurfaceBuffer> buffer;
        cs->AcquireBuffer(buffer, flushFence, timestamp, damage);
        unique_lock<mutex> lock(sample->mutex2_);
        if (buffer) {
            sample->outputBufferAvilQue_.push(buffer);
        } else {
            cout << "acquire buffer failed" <<endl;
        }
    }

private:
    int64_t timestamp = 0;
    Rect damage = {};
    VideoSample *sample = nullptr;
    int32_t flushFence = -1;
    sptr<Surface> cs {nullptr};
};


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
    if (metaDataFile != nullptr) {
        if (metaDataFile->is_open()) {
            metaDataFile->close();
        }
        metaDataFile.reset();
        metaDataFile = nullptr;
    }
    OH_VideoProcessing_Destroy(videoProcessor);
    OH_NativeWindow_DestroyNativeWindow(outWindow);
    OH_NativeWindow_DestroyNativeWindow(inWindow);
    if (cs) {
        for (; !outputBufferAvilQue_.empty(); outputBufferAvilQue_.pop()) {
            cs->ReleaseBuffer(outputBufferAvilQue_.front(), -1);
        }
        cs->UnregisterConsumerListener();
    }
}

int32_t VideoSample::InitVideoSample(const int32_t type, int32_t width, int32_t height, VideoProcessParam param)
{
    width_ = width;
    height_ = height;
    isRunning = true;
    param_ = param;
    int32_t ret = OH_VideoProcessing_Create(&videoProcessor, type);
    if (type == VIDEO_PROCESSING_TYPE_METADATA_GENERATION) {
        isMetadataGen = true;
    }
    CHECK_AND_RETURN_RET(ret == VIDEO_PROCESSING_SUCCESS, ret, "OH_VideoProcessing_Create failed");
    cs = Surface::CreateSurfaceAsConsumer();
    sptr<IBufferConsumerListener> listener = new VPEConsumerListener(cs, this);
    cs->RegisterConsumerListener(listener);
    auto p = cs->GetProducer();
    sptr<Surface> ps = Surface::CreateSurfaceAsProducer(p);
    outWindow = CreateNativeWindowFromSurface(&ps);
    (void)OH_NativeWindow_NativeWindowHandleOpt(outWindow, SET_FORMAT, param_.outFmt);
    if (!isMetadataGen) {
        (void)OH_NativeWindow_NativeWindowHandleOpt(outWindow, SET_FORMAT, param_.outFmt);
        ret = OH_NativeWindow_SetColorSpace(outWindow, param_.outColorSpace);
    }
    if (isHDRVividOut) {
        uint8_t val = OH_VIDEO_HDR_VIVID;
        ret = OH_NativeWindow_SetMetadataValue(outWindow, OH_HDR_METADATA_TYPE, sizeof(uint8_t), &val);
        if (ret != 0) {
            cout << "set metadata failed" << endl;
        }
    }
    if (!isHDRVivid && param_.inColorSpace == OH_COLORSPACE_BT2020_PQ_LIMIT) {
        uint8_t outMeta = OH_VIDEO_HDR_HLG;
        OH_NativeWindow_SetMetadataValue(outWindow, OH_HDR_METADATA_TYPE, sizeof(uint8_t), &outMeta);
    }
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
    (void)OH_NativeWindow_NativeWindowHandleOpt(inWindow, SET_FORMAT, param_.inFmt);
    OH_NativeWindow_SetColorSpace(inWindow, param_.inColorSpace);
    int32_t err = 0;
    if (isMetadataGen) {
        uint8_t val = OH_VIDEO_HDR_VIVID;
        err = OH_NativeWindow_SetMetadataValue(inWindow, OH_HDR_METADATA_TYPE, sizeof(uint8_t), &val);
        if (err != 0) {
            cout << "set metadata type failed" << endl;
        }
    }
    if (isHDRVivid) {
        metaDataFile = std::make_unique<std::ifstream>(inputMetaPath);
        metaDataFile->seekg(0, ios::end);
        metadataSize = metaDataFile->tellg();
        metaDataFile->seekg(0, ios::beg);
        metaData = new uint8_t[metadataSize];
        metaDataFile->read(reinterpret_cast<char*>(metaData), metadataSize);
        uint8_t val = OH_VIDEO_HDR_VIVID;
        err = OH_NativeWindow_SetMetadataValue(inWindow, OH_HDR_METADATA_TYPE, sizeof(uint8_t), &val);
        if (err != 0) {
            cout << "set metadata type failed" << endl;
        }
    } else if (!isHDRVivid && param_.inColorSpace == OH_COLORSPACE_BT2020_PQ_LIMIT) {
        uint8_t inMeta = OH_VIDEO_HDR_HDR10;
        OH_NativeWindow_SetMetadataValue(inWindow, OH_HDR_METADATA_TYPE, sizeof(uint8_t), &inMeta);
    }
    rect = new Region::Rect();
    rect->x = 0;
    rect->y = 0;
    rect->w = width_;
    rect->h = height_;
    region.rects = rect;
}

void VideoSample::GetMetadata(OH_NativeBuffer *buffer)
{
    uint8_t *staticMetadata = nullptr;
    int32_t staticSize = 0;
    uint8_t *dynamicMetadata = nullptr;
    int32_t dynamicSize = 0;
    int32_t ret = OH_NativeBuffer_GetMetadataValue(buffer, OH_HDR_STATIC_METADATA, &staticSize, &staticMetadata);
    if (ret) {
        cout << "get static metadata failed" << ret << endl;
        errCount++;
    }
    ret = OH_NativeBuffer_GetMetadataValue(buffer, OH_HDR_DYNAMIC_METADATA, &dynamicSize, &dynamicMetadata);
    if (ret) {
        cout << "get dynamic metadata failed" << ret << endl;
        errCount++;
        return;
    }
    outFile->write(reinterpret_cast<char *>(dynamicMetadata), dynamicSize);
    outFile->close();
}

int32_t VideoSample::InputFunc()
{
    inFile = std::make_unique<std::ifstream>(inputFilePath);
    CHECK_AND_RETURN_RET(inFile->is_open(), -1, "open input file failed");
    int fenceFd = -1;
    OHNativeWindowBuffer *ohNativeWindowBuffer;
    if (isHDRVivid) {
        int32_t ret = OH_NativeWindow_SetMetadataValue(inWindow, OH_HDR_DYNAMIC_METADATA, metadataSize, metaData);
        CHECK_AND_RETURN_RET(ret == 0, ret, "set metadata value failed");
    }
    int32_t err = OH_NativeWindow_NativeWindowRequestBuffer(inWindow, &ohNativeWindowBuffer, &fenceFd);
    CHECK_AND_RETURN_RET(err == 0, err, "OH_NativeWindow_NativeWindowRequestBuffer failed");
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

    if (param_.inFmt == NATIVEBUFFER_PIXEL_FMT_YCBCR_P010 ||
        param_.inFmt == NATIVEBUFFER_PIXEL_FMT_YCRCB_P010) {
        ReadOneFrameP010(reinterpret_cast<uint8_t *>(virAddr), config);
    } else if (param_.inFmt == NATIVEBUFFER_PIXEL_FMT_RGBA_1010102 ||
        param_.inFmt == NATIVEBUFFER_PIXEL_FMT_RGBA_8888) {
        ReadOneFrameRGBA10(reinterpret_cast<uint8_t *>(virAddr), config);
    } else if (param_.inFmt == NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP ||
        param_.inFmt == NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP) {
        ReadOneFrameYUV420SP(reinterpret_cast<uint8_t *>(virAddr), config);
    }
    NativeWindowHandleOpt(inWindow, SET_UI_TIMESTAMP, GetSystemTimeUs());
    err = OH_NativeBuffer_Unmap(nativeBuffer);
    CHECK_AND_RETURN_RET(err == 0, err, "OH_NativeBuffer_Unmap failed.");
    err = OH_NativeWindow_NativeWindowFlushBuffer(inWindow, ohNativeWindowBuffer, -1, region);
    inCount++;
    CHECK_AND_RETURN_RET(err == 0, err, "OH_NativeWindow_NativeWindowFlushBuffer failed.");
    inputFuncFinished = true;
    inFile->close();
    return 0;
}

void VideoSample::OutputThread()
{
    if (isMetadataGen) {
        outFile = std::make_unique<std::ofstream>(outputMetaPath);
    } else {
        outFile = std::make_unique<std::ofstream>(outputFilePath);
    }
    SHA512_Init(&ctx);
    while (isRunning) {
        if (outputBufferAvilQue_.size() > 0) {
            unique_lock<mutex> lock(mutex2_);
            auto buffer = outputBufferAvilQue_.front();
            ProcessOutputData(buffer);
            outCount++;
            lock.unlock();
            cs->ReleaseBuffer(buffer, -1);
            outputBufferAvilQue_.pop();
        }
        if (outputBufferAvilQue_.size() == 0 && inputFuncFinished && (inCount == outCount)) {
            break;
        }
        usleep(NANOS_IN_MICRO);
    }
    SHA512_Final(md, &ctx);
    OPENSSL_cleanse(&ctx, sizeof(ctx));
    if (!MdCompare(md)) {
        md5Equal = false;
    }
}

int32_t VideoSample::StartProcess()
{
    int32_t ret = OH_VideoProcessing_Start(videoProcessor);
    CHECK_AND_RETURN_RET(ret == VIDEO_PROCESSING_SUCCESS, ret, "OH_VideoProcessing_Start failed.");
    inputLoop_ = make_unique<thread>(&VideoSample::InputFunc, this);
    outputLoop_ = make_unique<thread>(&VideoSample::OutputThread, this);
    return VIDEO_PROCESSING_SUCCESS;
}

int32_t VideoSample::WaitAndStopSample()
{
    inputLoop_->join();
    outputLoop_->join();
    int32_t ret = OH_VideoProcessing_Stop(videoProcessor);
    CHECK_AND_RETURN_RET(ret == VIDEO_PROCESSING_SUCCESS, ret, "OH_VideoProcessing_Stop failed.");
    return 0;
}

int32_t VideoSample::ReadOneFrameYUV420SP(uint8_t *addr, OH_NativeBuffer_Config config)
{
    uint8_t *start = addr;
    for (uint32_t i = 0; i < config.height; i++) {
        inFile->read(reinterpret_cast<char *>(addr), width_);
        addr += config.stride;
    }
    for (uint32_t i = 0; i < config.height / sizeof(uint16_t); i++) {
        inFile->read(reinterpret_cast<char *>(addr), width_);
        addr += config.stride;
    }
    return addr - start;
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
