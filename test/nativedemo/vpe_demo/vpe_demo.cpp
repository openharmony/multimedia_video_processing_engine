/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <cstdio>
#include <fstream>
#include <memory>
#include <vector>
#include <string>
#include "csc_sample.h"
#include "csc_sample_define.h"
#include "graphic_common_c.h"
#include "algorithm_common.h"
#include "algorithm_errors.h"

using namespace OHOS;
using namespace Media;
using namespace VideoProcessingEngine;
using namespace CSCSampleDefine;

namespace {
struct DemoArg {
    int32_t repeatTimes = 1;
    bool needPrintMetadata = true;
    bool needDumpOutputBuffer = false;
};

void GetCmdArg(int argc, char* argv[], DemoArg &demoArg)
{
    constexpr int32_t maxRepeatTimes = 1000;
    constexpr int8_t needDumpOutputBufferOffset = 4;
    constexpr int8_t needPrintMetadataOffset = 3;
    constexpr int8_t repeatTimesOffset = 2;
    switch (argc) {
        case needDumpOutputBufferOffset:
            demoArg.needDumpOutputBuffer = atoi(argv[needDumpOutputBufferOffset - 1]);
        case needPrintMetadataOffset:
            demoArg.needPrintMetadata = atoi(argv[needPrintMetadataOffset - 1]);
        case repeatTimesOffset:
            demoArg.repeatTimes = atoi(argv[repeatTimesOffset - 1]);
            if (demoArg.repeatTimes > maxRepeatTimes || demoArg.repeatTimes < 1) {
                printf("Demo repeats range: [1, %d], err input: %d, set 1\n", maxRepeatTimes, demoArg.repeatTimes);
                demoArg.repeatTimes = 1;
            }
        default:
            break;
    }
    printf("Dump output buffer: %s\n", demoArg.needDumpOutputBuffer ? "true" : "false");
    printf("Print metadata: %s\n", demoArg.needPrintMetadata ? "true" : "false");
    printf("Demo repeat times: %d\n", demoArg.repeatTimes);
}

std::shared_ptr<ColorSpaceConverter> CreateColorSpaceConverter()
{
    auto csc = ColorSpaceConverter::Create();
    if (csc == nullptr) {
        printf("Create colorspace converter failed");
        return nullptr;
    }
    ColorSpaceConverterParameter parameter;
    parameter.renderIntent = RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC;
    int32_t ret = csc->SetParameter(parameter);
    printf("SetParameter %d\n", ret);

    return csc;
}
}

int32_t main(int argc, char* argv[])
{
    DemoArg demoArg;
    GetCmdArg(argc, argv, demoArg);

    std::unique_ptr<std::ifstream> metadataFile =
        std::make_unique<std::ifstream>(METADATA_FILE.data(), std::ios::binary | std::ios::in);
    std::unique_ptr<std::ifstream> yuvFile =
        std::make_unique<std::ifstream>(YUV_FILE.data(), std::ios::binary | std::ios::in);
    auto csc = CreateColorSpaceConverter();

    for (int times = 0; times < demoArg.repeatTimes; times++) {
        std::vector<sptr<SurfaceBuffer>> inputList(YUV_FILE_FRAME_NUM,
            CreateSurfaceBuffer(YUV_FILE_PIXEL_FORMAT, WIDTH, HEIGHT));
        auto output = CreateSurfaceBuffer(OUTPUT_PIXEL_FORMAT, WIDTH, HEIGHT);
        SetMeatadata(output, OUTPUT_COLORSPACE_INFO);
        SetMeatadata(output, CM_METADATA_NONE);

        int frame = times * 2;
        for (auto &input : inputList) {
            SetMeatadata(input, INPUT_COLORSPACE_INFO);
            SetMeatadata(input, CM_VIDEO_HDR_VIVID);
            SetMeatadata(input, metadataFile);
            ReadYuvFile(input, yuvFile, ONE_FRAME_SIZE);

            int32_t ret = csc->Process(input, output);
            printf("frame: %4d, csc process, ret: %d\n", ++frame, ret);

            if (demoArg.needPrintMetadata) {
                PrintMetadataType(input,  ATTRKEY_COLORSPACE_TYPE);
                PrintMetadataType(input,  ATTRKEY_HDR_METADATA_TYPE);
                PrintMetadataType(output, ATTRKEY_COLORSPACE_TYPE);
                PrintMetadataType(output, ATTRKEY_HDR_METADATA_TYPE);
                PrintMetadataKey(input);
                PrintMetadataKey(output);
            }
            if (demoArg.needDumpOutputBuffer) {
                std::unique_ptr<std::ofstream> outputImage = std::make_unique<std::ofstream>(
                    "frame_output_" + std::to_string(frame), std::ios::binary | std::ios::out | std::ios::trunc);
                outputImage->write(static_cast<const char *>(output->GetVirAddr()), output->GetSize());
            }
        }
        metadataFile->seekg(0);
        yuvFile->seekg(0);
    }
    return 0;
}
