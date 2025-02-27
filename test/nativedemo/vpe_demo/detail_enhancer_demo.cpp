/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#define LOG_TAG "DetailEnhDemo"

#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <dlfcn.h>
#include <fstream>
#include <memory>
#include <unistd.h>
#include <string>

#include "algorithm_common.h"
#include "algorithm_errors.h"
#include "graphic_common_c.h"
#include "detailEnh_sample.h"
#include "detailEnh_sample_define.h"
#include "detail_enhancer_image.h"

using namespace OHOS;
using namespace Media;
using namespace VideoProcessingEngine;

namespace {
const float SIZE_COEF_YUV420 = 1.5;
const float SIZE_COEF_RGBA8888 = 4;
const float SIZE_COEF_YUV444 = 3;

std::shared_ptr<DetailEnhancerImage> DetailEnhancerImageCreate()
{
    auto detailEnh = DetailEnhancerImage::Create();
    if (detailEnh == nullptr) {
        TEST_LOG("Create Detail enhancer failed");
        return nullptr;
    }
    return detailEnh;
}

std::string GetFormatName(int32_t format)
{
    std::string formatName = "UNKNOWN";
    switch (format) {
        case OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_SP:
            formatName = "NV12";
            break;
        case OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_P:
            formatName = "I420";
            break;
        case OHOS::GRAPHIC_PIXEL_FMT_RGBA_8888:
            formatName = "RGBA";
            break;
        case OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888:
            formatName = "BGRA";
            break;
        case OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102:
            formatName = "RGBA10";
            break;
        default:
            TEST_LOG("Unknow format!");
    }
    return formatName;
}

int32_t GetFileSize(int32_t width, int32_t height, int32_t format)
{
    int32_t size = width * height;
    switch (format) {
        case OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_SP:
        case OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_P:
            size = size * SIZE_COEF_YUV420;
            break;
        case OHOS::GRAPHIC_PIXEL_FMT_RGBA_8888:
        case OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888:
        case OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102:
            size *= SIZE_COEF_RGBA8888;
            break;
        default:
            TEST_LOG("Unknow format:%d", format);
            size *= SIZE_COEF_YUV444;
            break;
    }
    return size;
}

int32_t GetImageType(int32_t format)
{
    int32_t imageType = 0;
    switch (format) {
        case RGBA:
            imageType = OHOS::GRAPHIC_PIXEL_FMT_RGBA_8888;
            break;
        case BGRA:
            imageType = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
            break;
        case NV12:
            imageType = OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_SP;
            break;
        case I420:
            imageType = OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_P;
            break;
        case RGBA1010102:
            imageType = OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102;
            break;
        default:
            imageType = OHOS::GRAPHIC_PIXEL_FMT_RGBA_8888;
    }
    return imageType;
}

void Process(std::shared_ptr<DetailEnhancerImage> detailEnh, std::string_view inputFile,
    std::vector<int32_t> inputParam, std::vector<int32_t> outputParam, bool needDump)
{
    if (inputFile == "UNKNOWN") {
        TEST_LOG("Invalid input");
        return;
    }
    int32_t inputFormat = inputParam[0];
    int32_t inputWidth = inputParam[1];
    int32_t inputHeight = inputParam[2];
    int32_t outputFormat = outputParam[0];
    int32_t outputWidth = outputParam[1];
    int32_t outputHeight = outputParam[2];
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    TEST_LOG("inputFile:%s", inputFile.data());
    std::unique_ptr<std::ifstream> yuvFile =
        std::make_unique<std::ifstream>(inputFile.data(), std::ios::binary | std::ios::in);
    ReadYuvFile(input, yuvFile, GetFileSize(inputWidth, inputHeight, inputFormat));
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    yuvFile->seekg(0);
    yuvFile->close();

    int32_t ret = detailEnh->Process(input, output);
    if (ret != VPE_ALGO_ERR_OK) {
        TEST_LOG("Processed failed");
        return;
    }
    if (needDump) {
        std::unique_ptr<std::ofstream> outputImage = std::make_unique<std::ofstream>(
            "/data/test/media/output/" + GetFormatName(inputFormat) + "To" + GetFormatName(outputFormat) + "_" +
            std::to_string(outputWidth) + "x" + std::to_string(outputHeight) + "_" +
            std::to_string(output->GetStride()) + ".yuv",
            std::ios::binary | std::ios::out | std::ios::trunc);
        outputImage->write(static_cast<const char *>(output->GetVirAddr()), output->GetSize());
        outputImage->close();
    }
}

void RunWithSo(sptr<SurfaceBuffer> & input, sptr<SurfaceBuffer> & output, DetailEnhancerLevel level)
{
    void* lib = dlopen("/system/lib64/libvideoprocessingengine.z.so", RTLD_LAZY);
    if (lib == nullptr) {
        printf("cannot load vpe lib\n");
        return;
    }

    typedef int32_t (*DetailEnhancerCreate)(int32_t*);
    typedef int32_t (*DetailEnhancerProcessImage)(int32_t,
        OHNativeWindowBuffer*, OHNativeWindowBuffer*, int32_t);
    typedef int32_t (*DetailEnhancerDestroy)(int32_t*);

    auto detailEnhCreate = reinterpret_cast<DetailEnhancerCreate>(dlsym(lib, "DetailEnhancerCreate"));
    auto detailEnhProcessImage =
        reinterpret_cast<DetailEnhancerProcessImage>(dlsym(lib, "DetailEnhancerProcessImage"));
    auto detailEnhDestroy = reinterpret_cast<DetailEnhancerDestroy>(dlsym(lib, "DetailEnhancerDestroy"));

    int32_t instanceSrId = -1;
    int32_t res = detailEnhCreate(&instanceSrId);
    if (res != 0 || instanceSrId == -1) {
        TEST_LOG("create failed, res:%d, insta:%d\n", res, instanceSrId);
        dlclose(lib);
        return;
    }
    if (input == nullptr || output == nullptr) {
        TEST_LOG("input invalid\n");
        return;
    }
    OHNativeWindowBuffer* srIn = OH_NativeWindow_CreateNativeWindowBufferFromSurfaceBuffer(&input);
    OHNativeWindowBuffer* srOut = OH_NativeWindow_CreateNativeWindowBufferFromSurfaceBuffer(&output);
    res = detailEnhProcessImage(instanceSrId, srIn, srOut, static_cast<int>(level));
    if (res != 0) {
        TEST_LOG("process failed\n");
        return;
    }
    res = detailEnhDestroy(&instanceSrId);
    if (res != 0) {
        TEST_LOG("destroy failed\n");
        return;
    }
    std::unique_ptr<std::ofstream> outputImage = std::make_unique<std::ofstream>(
        "/data/test/media/output/" + GetFormatName(input->GetFormat()) + "To" + GetFormatName(output->GetFormat()) +
        "_" + std::to_string(output->GetWidth()) + "x" + std::to_string(output->GetHeight())+ "_" +
        std::to_string(output->GetStride()) + "_level"+ std::to_string(level) + "externC.yuv",
        std::ios::binary | std::ios::out | std::ios::trunc);
    outputImage->write(static_cast<const char *>(output->GetVirAddr()), output->GetSize());
    outputImage->close();
}
}

int32_t main([[maybe_unused]]int argc, char* argv[])
{
    TEST_LOG("USAGE exe inputWidth inputHeight outputWidth outputHeight pixelFormat\
        checkHightLevel checkSuperLevel checkDlopen needDump processTime enterTime\n");
    TEST_LOG("format: 5 <---> RGBA8888, 6 <---> NV12, 7 <---> YUV420, 8 <---> BGRA 9 <---> RGBA1010102\n");
    int32_t inputWidth = atoi(argv[1]);
    int32_t inputHeight = atoi(argv[2]);
    int32_t outputWidth = atoi(argv[3]);
    int32_t outputHeight = atoi(argv[4]);
    SUPPORT_FORMAT pixelFormat = static_cast<SUPPORT_FORMAT>(atoi(argv[5]));
    bool checkDlopen = atoi(argv[6]);
    bool needDump = atoi(argv[7]);
    int32_t levelToProcess = atoi(argv[8]);
    std::string inputFilePath = argv[9];
    int32_t processTime = atoi(argv[10]);
    int32_t enterTime = atoi(argv[11]);

    if (!checkDlopen) {
        for (int i = 0; i < enterTime; i++) {
            auto detailEnh = DetailEnhancerImageCreate();
            if (detailEnh == nullptr) {
                printf("detailEnh == nullptr");
                return -1;
            }
            DetailEnhancerParameters param {
                .uri = "",
                .level = static_cast<DetailEnhancerLevel>(levelToProcess),
                .forceEve = 1,
            };
            if (detailEnh->SetParameter(param)!= VPE_ALGO_ERR_OK) {
                printf("Init failed!");
                return -1;
            }
            for (int j = 0; j < processTime; j++) {
                Process(detailEnh, inputFilePath, { GetImageType(pixelFormat), inputWidth, inputHeight },
                    { GetImageType(pixelFormat), outputWidth, outputHeight }, needDump);
            }
            detailEnh = nullptr;
        }
    } else {
        auto input = CreateSurfaceBuffer(GetImageType(pixelFormat), inputWidth, inputHeight);
        std::string_view inputFile = inputFilePath;
        std::unique_ptr<std::ifstream> yuvFile =
            std::make_unique<std::ifstream>(inputFile.data(), std::ios::binary | std::ios::in);
        ReadYuvFile(input, yuvFile, GetFileSize(inputWidth, inputHeight, GetImageType(pixelFormat)));
        auto output = CreateSurfaceBuffer(GetImageType(pixelFormat), outputWidth, outputHeight);
        RunWithSo(input, output, static_cast<DetailEnhancerLevel>(levelToProcess));
        yuvFile->close();
    }
    return 0;
}
