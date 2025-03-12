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
#ifndef VIDEO_SAMPLE_H
#define VIDEO_SAMPLE_H
#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#include <memory>
#include "yuv_viewer.h"
#include "video_processing.h"
#include "surface/window.h"
#include "native_buffer.h"

int CheckAndRet(bool cond, int ret, char* msg) {
    if (!cond) {                                          
        std::cout << msg <<"    ErrCode:"<< ret << std::endl;
        return ret;
    }
}

typedef struct VideoProcessParam {
    OH_NativeBuffer_Format inFmt;
    OH_NativeBuffer_ColorSpace inColorSpace;
    OH_NativeBuffer_Format outFmt;
    OH_NativeBuffer_ColorSpace outColorSpace;
}VideoProcessParam;

namespace OHOS {
class VideoSample {
public:
    VideoSample() = default;
    ~VideoSample();

    int32_t inputFrameNumber = 100;
    std::string inputFilePath = "";
    std::string inputMetaPath = "";
    bool isHDRVivid = false;
    bool isHDRVividOut = false;
    int32_t InputFunc();
    int32_t InitVideoSample(const int32_t type, int32_t width, int32_t height, VideoProcessParam param);
    int32_t StartProcess();
    int32_t WaitAndStopSample();
    int32_t errCount = 0;
    std::string OUT_DIR = "";
private:
    void SetInputWindowParam();

    int32_t width_ = 0;
    int32_t height_ = 0;
    VideoProcessParam param_;
    int32_t ReadOneFrameP010(uint8_t *addr, OH_NativeBuffer_Config config);
    int32_t ReadOneFrameRGBA10(uint8_t *addr, OH_NativeBuffer_Config config);
    OH_VideoProcessing* videoProcessor = nullptr;
    std::unique_ptr<YuvViewer> viewer;
    OHNativeWindow *inWindow = nullptr;
    OHNativeWindow *outWindow = nullptr;
    std::unique_ptr<std::ifstream> inFile;
    std::unique_ptr<std::ifstream> metaDataFile;
    std::unique_ptr<std::thread> inputLoop_;
    VideoProcessing_Callback* callback = nullptr;
    struct Region region;
    struct Region::Rect *rect;
    uint8_t *metaData = nullptr;
    int32_t metadataSize = 0;
    };
}
#endif