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
#include <iostream>
#include <string>
#include <thread>
#include <queue>
#include <fstream>
#include <memory>
#include "video_processing.h"
#include "surface/window.h"
#include "native_buffer.h"
#include "iconsumer_surface.h"
#include "sync_fence.h"
#include "openssl/crypto.h"
#include "openssl/sha.h"

typedef struct VideoProcessParam {
    OH_NativeBuffer_Format inFmt;
    OH_NativeBuffer_ColorSpace inColorSpace;
    OH_NativeBuffer_Format outFmt;
    OH_NativeBuffer_ColorSpace outColorSpace;
}VideoProcessParam;

constexpr uint32_t DEFAULT_WIDTH = 3840;
constexpr uint32_t DEFAULT_HEIGHT = 2160;
namespace OHOS {
class VideoSample {
public:
    VideoSample() = default;
    ~VideoSample();
    std::string inputFilePath = "";
    std::string inputMetaPath = "";
    std::string outputFilePath = "";
    std::string outputMetaPath = "";
    std::string hashValJsonPath = "/data/test/media/hash_val.json";
    std::string DEFAULT = "default";
    std::string defaultPixelFormat = "";
    std::string outputPixelFormat = "";
    std::string convertType = "";
    bool isHDRVivid = false;
    bool isHDRVividOut = false;
    bool md5Equal = true;
    bool inputFuncFinished = false;
    bool isEOF = false;
    uint32_t inCount = 0;
    uint32_t outCount = 0;
    int32_t InputFunc();
    void OutputThread();
    int32_t InitVideoSample(const int32_t type, int32_t width, int32_t height, VideoProcessParam param);
    int32_t StartProcess();
    void ProcessOutputData(SurfaceBuffer *buffer);
    int32_t WaitAndStopSample();
    bool MdCompare(uint8_t source[]);
    std::vector<uint8_t> LoadHashFile();
    int32_t errCount = 0;
    std::queue<sptr<SurfaceBuffer>> outputBufferAvilQue_;
    sptr<Surface> cs = nullptr;
    bool isRunning = false;
    std::mutex mutex2_;
    std::mutex mutex_;
    bool isStopped = false;
private:
    void SetInputWindowParam();
    void GetMetadata(OH_NativeBuffer *buffer);
    void ShowHashValue(uint8_t source[]);
    std::unique_ptr<std::ofstream> outFile;
    int32_t width_ = 0;
    int32_t height_ = 0;
    VideoProcessParam param_;
    int32_t ReadOneFrameYUV420SP(uint8_t *addr, OH_NativeBuffer_Config config);
    int32_t ReadOneFrameP010(uint8_t *addr, OH_NativeBuffer_Config config);
    int32_t ReadOneFrameRGBA10(uint8_t *addr, OH_NativeBuffer_Config config);
    OH_VideoProcessing* videoProcessor = nullptr;
    bool isMetadataGen = false;
    OHNativeWindow *inWindow = nullptr;
    OHNativeWindow *outWindow = nullptr;
    std::unique_ptr<std::ifstream> inFile;
    std::unique_ptr<std::ifstream> metaDataFile;
    std::unique_ptr<std::thread> inputLoop_;
    std::unique_ptr<std::thread> outputLoop_;
    VideoProcessing_Callback* callback = nullptr;
    struct Region region;
    struct Region::Rect *rect;
    uint8_t *metaData = nullptr;
    int32_t metadataSize = 0;
    SHA512_CTX ctx;
    uint8_t md[SHA512_DIGEST_LENGTH];
};
}
#endif