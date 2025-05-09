/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include <iostream>
#include <chrono>
#include "gtest/gtest.h"
#include "video_processing.h"
#include "enum_list.h"
#include "video_sample.h"
using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace {
static uint32_t g_allTestCount = 0;
static uint32_t g_supportedCount = 0;
static uint32_t g_unsupportedCount = 0;
std::unique_ptr<std::ofstream> outputListFile = nullptr;

class VpeVideoReliTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
};

void VpeVideoReliTest::SetUpTestCase()
{
    OH_VideoProcessing_InitializeEnvironment();
}
void VpeVideoReliTest::TearDownTestCase()
{
    OH_VideoProcessing_DeinitializeEnvironment();
}
void VpeVideoReliTest::SetUp()
{
    g_allTestCount = 0;
    g_supportedCount = 0;
    g_unsupportedCount = 0;
}
void VpeVideoReliTest::TearDown()
{
}
}
namespace {
#ifdef ENABLE_ALL_PROCESS
static bool CheckHDRColorSpaceAndPixFmt(VideoProcessing_ColorSpaceInfo info, bool isHDR2SDR = false)
{
    if ((info.metadataType == OH_VIDEO_HDR_HLG) && (info.colorSpace != OH_COLORSPACE_BT2020_HLG_LIMIT)) {
        return false;
    }
    if ((info.metadataType == OH_VIDEO_HDR_HDR10) && (info.colorSpace != OH_COLORSPACE_BT2020_PQ_LIMIT)) {
        return false;
    }
    if (!isHDR2SDR) {
        switch (info.colorSpace) {
            case OH_COLORSPACE_BT2020_HLG_LIMIT:
            case OH_COLORSPACE_BT2020_PQ_LIMIT:
                break;
            default:
                return false;
        }
    } else {
        switch (info.colorSpace) {
            case OH_COLORSPACE_BT2020_HLG_LIMIT:
            case OH_COLORSPACE_BT2020_PQ_LIMIT:
            case OH_COLORSPACE_BT2020_HLG_FULL:
                break;
            default:
                return false;
        }
    }
    switch (info.pixelFormat) {
        case NATIVEBUFFER_PIXEL_FMT_YCBCR_P010:
        case NATIVEBUFFER_PIXEL_FMT_YCRCB_P010:
        case NATIVEBUFFER_PIXEL_FMT_RGBA_1010102:
            return true;
        default:
            return false;
    }
}

static bool CheckSupportedMetadataGen(VideoProcessing_ColorSpaceInfo info)
{
    switch (info.metadataType) {
        case OH_VIDEO_HDR_HLG:
        case OH_VIDEO_HDR_HDR10:
        case OH_VIDEO_HDR_VIVID:
            break;
        default:
            return false;
    }
    return CheckHDRColorSpaceAndPixFmt(info);
}

static std::string BoolToString(bool val)
{
    if (val) {
        return "true";
    }
    return "false";
}

void CheckMetadataGen(VideoProcessing_ColorSpaceInfo info)
{
    bool expectVal = CheckSupportedMetadataGen(info);
    bool realVal = OH_VideoProcessing_IsMetadataGenerationSupported(&info);
    if (expectVal != realVal) {
        string msg = "-----------------------------------------------\n";
        msg += "metadata gen type:" + metadataString[info.metadataType] + "\n";
        msg += "colorspace:" + colorString[info.colorSpace] + "\n";
        msg += "format:" + formatString[info.pixelFormat] + "\n";
        msg += "expect" + BoolToString(expectVal) + " actual:" + BoolToString(realVal) + "\n";
        msg += "-----------------------------------------------\n";
        outputListFile->write(msg.c_str(), msg.size());
        g_unsupportedCount++;
    } else {
        g_supportedCount++;
    }
    g_allTestCount++;
}

static bool CheckVideoHDRVivid2SDR(VideoProcessing_ColorSpaceInfo outInfo)
{
    switch (outInfo.metadataType) {
        case OH_VIDEO_HDR_HLG:
        case OH_VIDEO_HDR_HDR10:
        case OH_VIDEO_HDR_VIVID:
            return false;
        default:
            break;
    }
    switch (outInfo.colorSpace) {
        case OH_COLORSPACE_BT709_LIMIT:
            break;
        default:
            return false;
    }
    switch (outInfo.pixelFormat) {
        case NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP:
        case NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP:
        case NATIVEBUFFER_PIXEL_FMT_RGBA_8888:
            return true;
        default:
            return false;
    }
}

static bool CheckVideoVivid2Vivid(VideoProcessing_ColorSpaceInfo  outInfo)
{
    if (outInfo.metadataType != OH_VIDEO_HDR_VIVID) {
        return false;
    }
    return CheckHDRColorSpaceAndPixFmt(outInfo);
}

static bool CheckVideoVivid2HDR(VideoProcessing_ColorSpaceInfo outInfo)
{
    if (outInfo.metadataType != OH_VIDEO_HDR_HLG) {
        return false;
    }
    return CheckHDRColorSpaceAndPixFmt(outInfo);
}

static bool CheckVideoHDR2HDR(VideoProcessing_ColorSpaceInfo outInfo)
{
    if (outInfo.metadataType != OH_VIDEO_HDR_HLG) {
        return false;
    }
    return CheckHDRColorSpaceAndPixFmt(outInfo);
}

static bool IsHDRConvert(VideoProcessing_ColorSpaceInfo inInfo, VideoProcessing_ColorSpaceInfo outInfo)
{
    bool isVivid2SDR = false;
    if (inInfo.metadataType == OH_VIDEO_HDR_VIVID && outInfo.metadataType == OH_VIDEO_NONE) {
        isVivid2SDR = true;
    }
    if (!CheckHDRColorSpaceAndPixFmt(inInfo, isVivid2SDR)) {
        return false;
    }
    if (inInfo.metadataType == OH_VIDEO_HDR_VIVID) {
        if (inInfo.colorSpace == outInfo.colorSpace) {
            return false;
        }
        bool isHDR2SDR = CheckVideoHDRVivid2SDR(outInfo);
        bool isHDR2HDR = CheckVideoVivid2Vivid(outInfo);
        bool isHDRVivid2HDR = false;
        if (inInfo.colorSpace == OH_COLORSPACE_BT2020_PQ_LIMIT) {
            isHDRVivid2HDR = CheckVideoVivid2HDR(outInfo);
        }
        return (isHDR2SDR || isHDR2HDR || isHDRVivid2HDR);
    } else if (inInfo.metadataType == OH_VIDEO_HDR_HLG) {
        return false;
    } else {
        if (inInfo.colorSpace == outInfo.colorSpace) {
            return false;
        }
        return CheckVideoHDR2HDR(outInfo);
    }
}

static bool IsSDRConvert(VideoProcessing_ColorSpaceInfo inInfo, VideoProcessing_ColorSpaceInfo outInfo)
{
    if (inInfo.colorSpace != OH_COLORSPACE_BT601_EBU_LIMIT && inInfo.colorSpace != OH_COLORSPACE_BT601_SMPTE_C_LIMIT) {
        return false;
    }
    switch (inInfo.pixelFormat) {
        case NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP:
        case NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP:
        case NATIVEBUFFER_PIXEL_FMT_RGBA_8888:
            break;
        default:
            return false;
    }
    switch (outInfo.metadataType) {
        case OH_VIDEO_HDR_HLG:
        case OH_VIDEO_HDR_HDR10:
        case OH_VIDEO_HDR_VIVID:
            return false;
        default:
            break;
    }
    if (outInfo.colorSpace != OH_COLORSPACE_BT709_LIMIT) {
        return false;
    }
    switch (outInfo.pixelFormat) {
        case NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP:
        case NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP:
        case NATIVEBUFFER_PIXEL_FMT_RGBA_8888:
            return true;
        default:
            return false;
    }
}

static void CheckColorSpaceConvert(VideoProcessing_ColorSpaceInfo inInfo, VideoProcessing_ColorSpaceInfo outInfo)
{
    bool expectVal = false;
    switch (inInfo.metadataType) {
        case OH_VIDEO_HDR_HLG:
        case OH_VIDEO_HDR_HDR10:
        case OH_VIDEO_HDR_VIVID:
            expectVal = IsHDRConvert(inInfo, outInfo);
            break;
        default:
            expectVal = IsSDRConvert(inInfo, outInfo);
            break;
    }
    bool realVal = OH_VideoProcessing_IsColorSpaceConversionSupported(&inInfo, &outInfo);
    if (expectVal != realVal) {
        string msg = "-----------------------------------------------\n";
        msg += "convert in type:" + metadataString[inInfo.metadataType] + "\n";
        msg += "colorspace:" + colorString[inInfo.colorSpace] + "\n";
        msg += "format:" + formatString[inInfo.pixelFormat] + "\n";
        msg += "convert out type:" + metadataString[outInfo.metadataType] + "\n";
        msg += "colorspace:" + colorString[outInfo.colorSpace] + "\n";
        msg += "format:" + formatString[outInfo.pixelFormat] + "\n";
        msg += "expect" + BoolToString(expectVal) + " actual:" + BoolToString(realVal) + "\n";
        msg += "-----------------------------------------------\n";
        outputListFile->write(msg.c_str(), msg.size());
        g_unsupportedCount++;
    } else {
        g_supportedCount++;
    }
    g_allTestCount++;
}

static void GenOutputOptions(VideoProcessing_ColorSpaceInfo inInfo)
{
    for (int i :NativeBuffer_MetadataType) {
        for (int j : NativeBuffer_ColorSpace) {
            for (int k : NativeBuffer_Format) {
                VideoProcessing_ColorSpaceInfo outInfo;
                outInfo.metadataType = i;
                outInfo.colorSpace = j;
                outInfo.pixelFormat = k;
                CheckColorSpaceConvert(inInfo, outInfo);
            }
        }
    }
}

/**
 * @tc.number    : VPE_VIDEO_RELI_TEST_0010
 * @tc.name      : test all metadataGenerate options
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoReliTest, VPE_VIDEO_RELI_TEST_0010, TestSize.Level2)
{
    outputListFile = std::make_unique<std::ofstream>("/data/test/media/metadataSupportList.txt");
    for (int i : NativeBuffer_MetadataType) {
        for (int j : NativeBuffer_ColorSpace) {
            for (int k : NativeBuffer_Format) {
                VideoProcessing_ColorSpaceInfo info;
                info.metadataType = i;
                info.colorSpace = j;
                info.pixelFormat = k;
                CheckMetadataGen(info);
            }
        }
    }
    cout << "all test " << g_allTestCount <<endl;
    cout << "matched test " << g_supportedCount <<endl;
    cout<< "mismatched test" << g_unsupportedCount <<endl;
    outputListFile->close();
    outputListFile = nullptr;
    ASSERT_EQ(g_unsupportedCount, 0);
}

/**
 * @tc.number    : VPE_VIDEO_RELI_TEST_0020
 * @tc.name      : test all colorspace convert options
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoReliTest, VPE_VIDEO_RELI_TEST_0020, TestSize.Level2)
{
    outputListFile = std::make_unique<std::ofstream>("/data/test/media/convertSupportList.txt");
    for (int i : NativeBuffer_MetadataType) {
        for (int j : NativeBuffer_ColorSpace) {
            for (int k : NativeBuffer_Format) {
                VideoProcessing_ColorSpaceInfo info;
                info.metadataType = i;
                info.colorSpace = j;
                info.pixelFormat = k;
                GenOutputOptions(info);
            }
        }
    }
    cout << "all test " << g_allTestCount <<endl;
    cout << "matched test " << g_supportedCount <<endl;
    cout<< "mismatched test" << g_unsupportedCount <<endl;
    outputListFile->close();
    outputListFile = nullptr;
    ASSERT_EQ(g_unsupportedCount, 0);
}
#endif
}