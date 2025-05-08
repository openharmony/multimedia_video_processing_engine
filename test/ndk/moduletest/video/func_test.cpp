/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "video_sample.h"
using namespace std;
using namespace OHOS;
using namespace testing::ext;
namespace {
bool g_isHardware = true;
class VpeVideoFuncTest : public testing::Test {
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

void VpeVideoFuncTest::SetUpTestCase()
{
    OH_VideoProcessing_InitializeEnvironment();
    if (!access("/vendor/bin/himediacomm_bak", 0)) {
        g_isHardware = false;
    }
}
void VpeVideoFuncTest::TearDownTestCase()
{
    OH_VideoProcessing_DeinitializeEnvironment();
}
void VpeVideoFuncTest::SetUp()
{
}
void VpeVideoFuncTest::TearDown()
{
}
}

namespace {
#ifdef ENABLE_ALL_PROCESS
/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0010
 * @tc.name      : test HDRVivid2SDR ,src colorspace PQ@10bit NV12,convert to BT709@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0010, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_pq_nv12.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_pq.bin";
    sample->outputFilePath = "/data/test/media/vivid_pq_nv12_out_bt709_nv12.yuv";
    sample->isHDRVivid = true;
    sample->defaultPixelFormat = "nv12";
    sample->convertType = "hdr2sdr_nv12_nv12";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
        DEFAULT_WIDTH, DEFAULT_HEIGHT, param);

    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0020
 * @tc.name      : test HDRVivid2SDR ,src colorspace PQ@10bit NV12,convert to BT709@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0020, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_pq_nv12.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_pq.bin";
    sample->outputFilePath = "/data/test/media/vivid_pq_nv12_out_bt709_nv21.yuv";
    sample->isHDRVivid = true;
    sample->defaultPixelFormat = "nv12";
    sample->convertType = "hdr2sdr_nv12_nv21";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
        DEFAULT_WIDTH, DEFAULT_HEIGHT, param);

    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0030
 * @tc.name      : test HDRVivid2SDR ,src colorspace PQ@10bit NV12,convert to BT709@RGBA8888
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0030, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_pq_nv12.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_pq.bin";
    sample->outputFilePath = "/data/test/media/vivid_pq_nv12_out_bt709_rgba8888.yuv";
    sample->isHDRVivid = true;
    sample->defaultPixelFormat = "nv12";
    sample->convertType = "hdr2sdr_nv12_rgba8888";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_8888, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0040
 * @tc.name      : test HDRVivid2SDR ,src colorspace PQ@10bit NV21,convert to BT709@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0040, TestSize.Level0)
{

    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_pq_nv21.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_pq.bin";
    sample->outputFilePath = "/data/test/media/vivid_pq_nv21_out_bt709_nv12.yuv";
    sample->isHDRVivid = true;
    sample->defaultPixelFormat = "nv21";
    sample->convertType = "hdr2sdr_nv21_nv12";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0050
 * @tc.name      : test HDRVivid2SDR ,src colorspace PQ@10bit NV21,convert to BT709@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0050, TestSize.Level0)
{

    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_pq_nv21.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_pq.bin";
    sample->outputFilePath = "/data/test/media/vivid_pq_nv21_out_bt709_nv21.yuv";
    sample->isHDRVivid = true;
    sample->defaultPixelFormat = "nv21";
    sample->convertType = "hdr2sdr_nv21_nv21";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0060
 * @tc.name      : test HDRVivid2SDR ,src colorspace PQ@10bit NV21,convert to BT709@RGBA8888
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0060, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_pq_nv21.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_pq.bin";
    sample->outputFilePath = "/data/test/media/vivid_pq_nv21_out_bt709_rgba8888.yuv";
    sample->isHDRVivid = true;
    sample->defaultPixelFormat = "nv21";
    sample->convertType = "hdr2sdr_nv21_rgba8888";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_8888, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0070
 * @tc.name      : test HDRVivid2SDR ,src colorspace PQ@10bit RGBA,convert to BT709@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0070, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_pq_rgba.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_pq.bin";
    sample->outputFilePath = "/data/test/media/vivid_pq_rgba_out_bt709_nv12.yuv";
    sample->isHDRVivid = true;
    sample->defaultPixelFormat = "rgba";
    sample->convertType = "hdr2sdr_rgba_nv12";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0080
 * @tc.name      : test HDRVivid2SDR ,src colorspace PQ@10bit RGBA,convert to BT709@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0080, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_pq_rgba.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_pq.bin";
    sample->outputFilePath = "/data/test/media/vivid_pq_rgba_out_bt709_nv21.yuv";
    sample->isHDRVivid = true;
    sample->defaultPixelFormat = "rgba";
    sample->convertType = "hdr2sdr_rgba_nv21";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0090
 * @tc.name      : test HDRVivid2SDR ,src colorspace PQ@10bit RGBA,convert to BT709@RGBA8888
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0090, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->isHDRVivid = true;
    sample->inputFilePath = "/data/test/media/vivid_pq_rgba.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_pq.bin";
    sample->outputFilePath = "/data/test/media/vivid_pq_rgba_out_bt709_rgba8888.yuv";
    sample->defaultPixelFormat = "rgba";
    sample->convertType = "hdr2sdr_rgba_rgba8888";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_HLG_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_8888, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0100
 * @tc.name      : test HDRVivid2SDR ,src colorspace HLG@10bit NV12,convert to BT709@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0100, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->isHDRVivid = true;
    sample->inputFilePath = "/data/test/media/vivid_hlg_nv12.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_hlg.bin";
    sample->outputFilePath = "/data/test/media/vivid_hlg_nv12_out_bt709_nv12.yuv";
    sample->defaultPixelFormat = "nv12";
    if (g_isHardware) {
        sample->convertType = "hdr2sdr_nv12_nv12";
    } else {
        sample->convertType = "hdr2sdr_nv12_nv12_sw";
    }
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_HLG_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0110
 * @tc.name      : test HDRVivid2SDR ,src colorspace HLG@10bit NV12,convert to BT709@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0110, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->isHDRVivid = true;
    sample->inputFilePath = "/data/test/media/vivid_hlg_nv12.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_hlg.bin";
    sample->outputFilePath = "/data/test/media/vivid_hlg_nv12_out_bt709_nv21.yuv";
    sample->defaultPixelFormat = "nv12";
    if (g_isHardware) {
        sample->convertType = "hdr2sdr_nv12_nv21";
    } else {
        sample->convertType = "hdr2sdr_nv12_nv21_sw";
    }
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_HLG_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0120
 * @tc.name      : test HDRVivid2SDR ,src colorspace HLG@10bit NV12,convert to BT709@RGBA8888
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0120, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->isHDRVivid = true;
    sample->inputFilePath = "/data/test/media/vivid_hlg_nv12.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_hlg.bin";
    sample->outputFilePath = "/data/test/media/vivid_hlg_nv12_out_bt709_rgba8888.yuv";
    sample->defaultPixelFormat = "nv12";
    if (g_isHardware) {
        sample->convertType = "hdr2sdr_nv12_rgba8888";
    } else {
        sample->convertType = "hdr2sdr_nv12_rgba8888_sw";
    }
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_HLG_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_8888, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0130
 * @tc.name      : test HDRVivid2SDR ,src colorspace HLG@10bit NV21,convert to BT709@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0130, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->isHDRVivid = true;
    sample->inputFilePath = "/data/test/media/vivid_hlg_nv21.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_hlg.bin";
    sample->outputFilePath = "/data/test/media/vivid_hlg_nv21_out_bt709_nv12.yuv";
    sample->defaultPixelFormat = "nv21";
    if (g_isHardware) {
        sample->convertType = "hdr2sdr_nv21_nv12";
    } else {
        sample->convertType = "hdr2sdr_nv21_nv12_sw";
    }
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_HLG_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0140
 * @tc.name      : test HDRVivid2SDR ,src colorspace HLG@10bit NV21,convert to BT709@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0140, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->isHDRVivid = true;
    sample->inputFilePath = "/data/test/media/vivid_hlg_nv21.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_hlg.bin";
    sample->outputFilePath = "/data/test/media/vivid_hlg_nv21_out_bt709_nv21.yuv";
    sample->defaultPixelFormat = "nv21";
    if (g_isHardware) {
        sample->convertType = "hdr2sdr_nv21_nv21";
    } else {
        sample->convertType = "hdr2sdr_nv21_nv21_sw";
    }
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_HLG_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0150
 * @tc.name      : test HDRVivid2SDR ,src colorspace HLG@10bit NV21,convert to BT709@RGBA8888
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0150, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->isHDRVivid = true;
    sample->inputFilePath = "/data/test/media/vivid_hlg_nv21.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_hlg.bin";
    sample->outputFilePath = "/data/test/media/vivid_hlg_nv21_out_bt709_rgba8888.yuv";
    sample->defaultPixelFormat = "nv21";
    if (g_isHardware) {
        sample->convertType = "hdr2sdr_nv21_rgba8888";
    } else {
        sample->convertType = "hdr2sdr_nv21_rgba8888_sw";
    }
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_HLG_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_8888, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0160
 * @tc.name      : test HDRVivid2SDR ,src colorspace HLG@10bit RGBA,convert to BT709@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0160, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->isHDRVivid = true;
    sample->inputFilePath = "/data/test/media/vivid_hlg_rgba.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_hlg.bin";
    sample->outputFilePath = "/data/test/media/vivid_hlg_rgba_out_bt709_nv12.yuv";
    sample->defaultPixelFormat = "rgba";
    if (g_isHardware) {
        sample->convertType = "hdr2sdr_rgba_nv12";
    } else {
        sample->convertType = "hdr2sdr_rgba_nv12_sw";
    }
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_HLG_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0170
 * @tc.name      : test HDRVivid2SDR ,src colorspace HLG@10bit RGBA,convert to BT709@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0170, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->isHDRVivid = true;
    sample->inputFilePath = "/data/test/media/vivid_hlg_rgba.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_hlg.bin";
    sample->outputFilePath = "/data/test/media/vivid_hlg_rgba_out_bt709_nv21.yuv";
    sample->defaultPixelFormat = "rgba";
    if (g_isHardware) {
        sample->convertType = "hdr2sdr_rgba_nv21";
    } else {
        sample->convertType = "hdr2sdr_rgba_nv21_sw";
    }
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_HLG_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0180
 * @tc.name      : test HDRVivid2SDR ,src colorspace HLG@10bit RGBA,convert to BT709@RGBA8888
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0180, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->isHDRVivid = true;
    sample->inputFilePath = "/data/test/media/vivid_hlg_rgba.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_hlg.bin";
    sample->outputFilePath = "/data/test/media/vivid_hlg_rgba_out_bt709_rgba8888.yuv";
    sample->defaultPixelFormat = "rgba";
    if (g_isHardware) {
        sample->convertType = "hdr2sdr_rgba_rgba8888";
    } else {
        sample->convertType = "hdr2sdr_rgba_rgba8888_sw";
    }
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_HLG_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_8888, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0190
 * @tc.name      : test SDR2SDR ,src colorspace EBU@NV12,convert to BT709@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0190, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/ebu_nv12.yuv";
    sample->outputFilePath = "/data/test/media/ebu_nv12_out_bt709_nv12.yuv";
    sample->defaultPixelFormat = "nv12";
    sample->convertType = "sdr2sdr_nv12_nv12";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, OH_COLORSPACE_BT601_EBU_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0200
 * @tc.name      : test SDR2SDR ,src colorspace EBU@NV12,convert to BT709@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0200, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/ebu_nv12.yuv";
    sample->outputFilePath = "/data/test/media/ebu_nv12_out_bt709_nv21.yuv";
    sample->defaultPixelFormat = "nv12";
    sample->convertType = "sdr2sdr_nv12_nv21";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, OH_COLORSPACE_BT601_EBU_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0210
 * @tc.name      : test SDR2SDR ,src colorspace EBU@NV12,convert to BT709@RGBA
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0210, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/ebu_nv12.yuv";
    sample->outputFilePath = "/data/test/media/ebu_nv12_out_bt709_rgba.yuv";
    sample->defaultPixelFormat = "nv12";
    sample->convertType = "sdr2sdr_nv12_rgba";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, OH_COLORSPACE_BT601_EBU_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_8888, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0220
 * @tc.name      : test SDR2SDR ,src colorspace EBU@NV21,convert to BT709@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0220, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/ebu_nv21.yuv";
    sample->outputFilePath = "/data/test/media/ebu_nv21_out_bt709_nv12.yuv";
    sample->defaultPixelFormat = "nv21";
    sample->convertType = "sdr2sdr_nv21_nv12";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP, OH_COLORSPACE_BT601_EBU_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0230
 * @tc.name      : test SDR2SDR ,src colorspace EBU@NV21,convert to BT709@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0230, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/ebu_nv21.yuv";
    sample->outputFilePath = "/data/test/media/ebu_nv21_out_bt709_nv21.yuv";
    sample->defaultPixelFormat = "nv21";
    sample->convertType = "sdr2sdr_nv21_nv21";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP, OH_COLORSPACE_BT601_EBU_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0240
 * @tc.name      : test SDR2SDR ,src colorspace EBU@NV21,convert to BT709@RGBA
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0240, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/ebu_nv21.yuv";
    sample->outputFilePath = "/data/test/media/ebu_nv21_out_bt709_rgba.yuv";
    sample->defaultPixelFormat = "nv21";
    sample->convertType = "sdr2sdr_nv21_rgba";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP, OH_COLORSPACE_BT601_EBU_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_8888, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0250
 * @tc.name      : test SDR2SDR ,src colorspace EBU@RGBA,convert to BT709@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0250, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/ebu_rgba.yuv";
    sample->outputFilePath = "/data/test/media/ebu_rgba_out_bt709_nv12.yuv";
    sample->defaultPixelFormat = "rgba";
    sample->convertType = "sdr2sdr_rgba_nv12";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_8888, OH_COLORSPACE_BT601_EBU_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0260
 * @tc.name      : test SDR2SDR ,src colorspace EBU@RGBA,convert to BT709@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0260, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/ebu_rgba.yuv";
    sample->outputFilePath = "/data/test/media/ebu_rgba_out_bt709_nv21.yuv";
    sample->defaultPixelFormat = "rgba";
    sample->convertType = "sdr2sdr_rgba_nv21";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_8888, OH_COLORSPACE_BT601_EBU_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0270
 * @tc.name      : test SDR2SDR ,src colorspace EBU@RGBA,convert to BT709@RGBA
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0270, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/ebu_rgba.yuv";
    sample->outputFilePath = "/data/test/media/ebu_rgba_out_bt709_rgba.yuv";
    sample->defaultPixelFormat = "rgba";
    sample->convertType = "sdr2sdr_rgba_rgba";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_8888, OH_COLORSPACE_BT601_EBU_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_8888, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0280
 * @tc.name      : test SDR2SDR ,src colorspace SMPTEC@NV12,convert to BT709@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0280, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/smptec_nv12.yuv";
    sample->outputFilePath = "/data/test/media/smptec_nv12_out_bt709_nv12.yuv";
    sample->defaultPixelFormat = "nv12";
    sample->convertType = "sdr2sdr_nv12_nv12";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, OH_COLORSPACE_BT601_SMPTE_C_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0290
 * @tc.name      : test SDR2SDR ,src colorspace SMPTEC@NV12,convert to BT709@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0290, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/smptec_nv12.yuv";
    sample->outputFilePath = "/data/test/media/smptec_nv12_out_bt709_nv21.yuv";
    sample->defaultPixelFormat = "nv12";
    sample->convertType = "sdr2sdr_nv12_nv21";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, OH_COLORSPACE_BT601_SMPTE_C_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0300
 * @tc.name      : test SDR2SDR ,src colorspace SMPTEC@NV12,convert to BT709@RGBA
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0300, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/smptec_nv12.yuv";
    sample->outputFilePath = "/data/test/media/smptec_nv12_out_bt709_rgba.yuv";
    sample->defaultPixelFormat = "nv12";
    sample->convertType = "sdr2sdr_nv12_rgba";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, OH_COLORSPACE_BT601_SMPTE_C_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_8888, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0310
 * @tc.name      : test SDR2SDR ,src colorspace SMPTEC@NV21,convert to BT709@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0310, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/smptec_nv21.yuv";
    sample->outputFilePath = "/data/test/media/smptec_nv21_out_bt709_nv12.yuv";
    sample->defaultPixelFormat = "nv21";
    sample->convertType = "sdr2sdr_nv21_nv12";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP, OH_COLORSPACE_BT601_SMPTE_C_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0320
 * @tc.name      : test SDR2SDR ,src colorspace SMPTEC@NV21,convert to BT709@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0320, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/smptec_nv21.yuv";
    sample->outputFilePath = "/data/test/media/smptec_nv21_out_bt709_nv21.yuv";
    sample->defaultPixelFormat = "nv21";
    sample->convertType = "sdr2sdr_nv21_nv21";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP, OH_COLORSPACE_BT601_SMPTE_C_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0330
 * @tc.name      : test SDR2SDR ,src colorspace SMPTEC@NV21,convert to BT709@RGBA
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0330, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/smptec_nv21.yuv";
    sample->outputFilePath = "/data/test/media/smptec_nv21_out_bt709_rgba.yuv";
    sample->defaultPixelFormat = "nv21";
    sample->convertType = "sdr2sdr_nv21_rgba";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP, OH_COLORSPACE_BT601_SMPTE_C_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_8888, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0340
 * @tc.name      : test SDR2SDR ,src colorspace SMPTEC@RGBA,convert to BT709@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0340, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/smptec_rgba.yuv";
    sample->outputFilePath = "/data/test/media/smptec_rgba_out_bt709_nv12.yuv";
    sample->defaultPixelFormat = "rgba";
    sample->convertType = "sdr2sdr_rgba_nv12";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_8888, OH_COLORSPACE_BT601_SMPTE_C_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0350
 * @tc.name      : test SDR2SDR ,src colorspace SMPTEC@RGBA,convert to BT709@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0350, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/smptec_rgba.yuv";
    sample->outputFilePath = "/data/test/media/smptec_rgba_out_bt709_nv21.yuv";
    sample->defaultPixelFormat = "rgba";
    sample->convertType = "sdr2sdr_rgba_nv21";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_8888, OH_COLORSPACE_BT601_SMPTE_C_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0360
 * @tc.name      : test SDR2SDR ,src colorspace SMPTEC@RGBA,convert to BT709@RGBA
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0360, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/smptec_rgba.yuv";
    sample->outputFilePath = "/data/test/media/smptec_rgba_out_bt709_rgba.yuv";
    sample->defaultPixelFormat = "rgba";
    sample->convertType = "sdr2sdr_rgba_rgba";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_8888, OH_COLORSPACE_BT601_SMPTE_C_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_8888, OH_COLORSPACE_BT709_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0370
 * @tc.name      : test HDR2HDR ,src colorspace PQ@NV12,convert to HLG@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0370, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/pq_nv12.yuv";
    sample->outputFilePath = "/data/test/media/pq_nv12_out_hlg_nv12.yuv";
    sample->defaultPixelFormat = "nv12";
    sample->convertType = "hdr2hdr_nv12_nv12";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0380
 * @tc.name      : test HDR2HDR ,src colorspace PQ@NV12,convert to HLG@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0380, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/pq_nv12.yuv";
    sample->outputFilePath = "/data/test/media/pq_nv12_out_hlg_nv21.yuv";
    sample->defaultPixelFormat = "nv12";
    sample->convertType = "hdr2hdr_nv12_nv21";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0390
 * @tc.name      : test HDR2HDR ,src colorspace PQ@NV12,convert to HLG@RGBA
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0390, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/pq_nv12.yuv";
    sample->outputFilePath = "/data/test/media/pq_nv12_out_hlg_rgba.yuv";
    sample->defaultPixelFormat = "nv12";
    sample->convertType = "hdr2hdr_nv12_rgba";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0400
 * @tc.name      : test HDR2HDR ,src colorspace PQ@NV21,convert to HLG@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0400, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/pq_nv21.yuv";
    sample->outputFilePath = "/data/test/media/pq_nv21_out_hlg_nv12.yuv";
    sample->defaultPixelFormat = "nv21";
    sample->convertType = "hdr2hdr_nv21_nv12";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0410
 * @tc.name      : test HDR2HDR ,src colorspace PQ@NV21,convert to HLG@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0410, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/pq_nv21.yuv";
    sample->outputFilePath = "/data/test/media/pq_nv21_out_hlg_nv21.yuv";
    sample->defaultPixelFormat = "nv21";
    sample->convertType = "hdr2hdr_nv21_nv21";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0420
 * @tc.name      : test HDR2HDR ,src colorspace PQ@NV21,convert to HLG@RGBA
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0420, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/pq_nv21.yuv";
    sample->outputFilePath = "/data/test/media/pq_nv21_out_hlg_rgba.yuv";
    sample->defaultPixelFormat = "nv21";
    sample->convertType = "hdr2hdr_nv21_rgba";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0430
 * @tc.name      : test HDR2HDR ,src colorspace PQ@RGBA,convert to HLG@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0430, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/pq_rgba.yuv";
    sample->outputFilePath = "/data/test/media/pq_rgba_out_hlg_nv12.yuv";
    sample->defaultPixelFormat = "rgba";
    sample->convertType = "hdr2hdr_rgba_nv12";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0440
 * @tc.name      : test HDR2HDR ,src colorspace PQ@RGBA,convert to HLG@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0440, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/pq_rgba.yuv";
    sample->outputFilePath = "/data/test/media/pq_rgba_out_hlg_nv21.yuv";
    sample->defaultPixelFormat = "rgba";
    sample->convertType = "hdr2hdr_rgba_nv21";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}


/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0450
 * @tc.name      : test HDR2HDR ,src colorspace PQ@RGBA,convert to HLG@RGBA
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0450, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/pq_rgba.yuv";
    sample->outputFilePath = "/data/test/media/pq_rgba_out_hlg_rgba.yuv";
    sample->defaultPixelFormat = "rgba";
    sample->convertType = "hdr2hdr_rgba_rgba";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0460
 * @tc.name      : test HDR2HDR ,src colorspace Vivid PQ@NV12,convert to Vivid HLG@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0460, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_pq_nv12.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_pq.bin";
    sample->outputFilePath = "/data/test/media/vivid_pq_nv12_out_hlg_nv12.yuv";
    sample->isHDRVivid = true;
    sample->isHDRVividOut = true;
    sample->defaultPixelFormat = "nv12";
    sample->convertType = "hdr2hdr_nv12_nv12";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0470
 * @tc.name      : test HDR2HDR ,src colorspace Vivid PQ@NV12,convert to Vivid HLG@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0470, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_pq_nv12.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_pq.bin";
    sample->outputFilePath = "/data/test/media/vivid_pq_nv12_out_hlg_nv21.yuv";
    sample->isHDRVivid = true;
    sample->isHDRVividOut = true;
    sample->defaultPixelFormat = "nv12";
    sample->convertType = "hdr2hdr_nv12_nv21";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0480
 * @tc.name      : test HDR2HDR ,src colorspace Vivid PQ@NV12,convert to Vivid HLG@RGBA
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0480, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_pq_nv12.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_pq.bin";
    sample->outputFilePath = "/data/test/media/vivid_pq_nv12_out_hlg_rgba.yuv";
    sample->isHDRVivid = true;
    sample->isHDRVividOut = true;
    sample->defaultPixelFormat = "nv12";
    sample->convertType = "hdr2hdr_nv12_rgba";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0490
 * @tc.name      : test HDR2HDR ,src colorspace Vivid PQ@NV21,convert to Vivid HLG@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0490, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_pq_nv21.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_pq.bin";
    sample->outputFilePath = "/data/test/media/vivid_pq_nv21_out_hlg_nv12.yuv";
    sample->isHDRVivid = true;
    sample->isHDRVividOut = true;
    sample->defaultPixelFormat = "nv21";
    sample->convertType = "hdr2hdr_nv21_nv12";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0500
 * @tc.name      : test HDR2HDR ,src colorspace Vivid PQ@NV21,convert to Vivid HLG@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0500, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_pq_nv21.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_pq.bin";
    sample->outputFilePath = "/data/test/media/vivid_pq_nv21_out_hlg_nv21.yuv";
    sample->isHDRVivid = true;
    sample->isHDRVividOut = true;
    sample->defaultPixelFormat = "nv21";
    sample->convertType = "hdr2hdr_nv21_nv21";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0510
 * @tc.name      : test HDR2HDR ,src colorspace Vivid PQ@NV21,convert to Vivid HLG@RGBA
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0510, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_pq_nv21.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_pq.bin";
    sample->outputFilePath = "/data/test/media/vivid_pq_nv21_out_hlg_rgba.yuv";
    sample->isHDRVivid = true;
    sample->isHDRVividOut = true;
    sample->defaultPixelFormat = "nv21";
    sample->convertType = "hdr2hdr_nv21_rgba";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0520
 * @tc.name      : test HDR2HDR ,src colorspace Vivid PQ@RGBA,convert to Vivid HLG@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0520, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_pq_rgba.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_pq.bin";
    sample->outputFilePath = "/data/test/media/vivid_pq_rgba_out_hlg_nv12.yuv";
    sample->isHDRVivid = true;
    sample->isHDRVividOut = true;
    sample->defaultPixelFormat = "rgba";
    sample->convertType = "hdr2hdr_rgba_nv12";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0530
 * @tc.name      : test HDR2HDR ,src colorspace Vivid PQ@RGBA,convert to Vivid HLG@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0530, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_pq_rgba.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_pq.bin";
    sample->outputFilePath = "/data/test/media/vivid_pq_rgba_out_hlg_nv21.yuv";
    sample->isHDRVivid = true;
    sample->isHDRVividOut = true;
    sample->defaultPixelFormat = "rgba";
    sample->convertType = "hdr2hdr_rgba_nv21";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0540
 * @tc.name      : test HDR2HDR ,src colorspace Vivid PQ@RGBA,convert to Vivid HLG@RGBA
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0540, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_pq_rgba.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_pq.bin";
    sample->outputFilePath = "/data/test/media/vivid_pq_rgba_out_hlg_rgba.yuv";
    sample->isHDRVivid = true;
    sample->isHDRVividOut = true;
    sample->defaultPixelFormat = "rgba";
    sample->convertType = "hdr2hdr_rgba_rgba";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_PQ_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0550
 * @tc.name      : test HDR2HDR ,src colorspace Vivid HLG@NV12,convert to Vivid PQ@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0550, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_hlg_nv12.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_hlg.bin";
    sample->outputFilePath = "/data/test/media/vivid_hlg_nv12_out_pq_nv12.yuv";
    sample->isHDRVivid = true;
    sample->isHDRVividOut = true;
    sample->defaultPixelFormat = "nv12";
    sample->convertType = "hdr2hdr_nv12_nv12";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_HLG_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_PQ_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0560
 * @tc.name      : test HDR2HDR ,src colorspace Vivid HLG@NV12,convert to Vivid PQ@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0560, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_hlg_nv12.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_hlg.bin";
    sample->outputFilePath = "/data/test/media/vivid_hlg_nv12_out_pq_nv21.yuv";
    sample->isHDRVivid = true;
    sample->isHDRVividOut = true;
    sample->defaultPixelFormat = "nv12";
    sample->convertType = "hdr2hdr_nv12_nv21";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_HLG_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_PQ_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0570
 * @tc.name      : test HDR2HDR ,src colorspace Vivid HLG@NV12,convert to Vivid PQ@RGBA
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0570, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_hlg_nv12.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_hlg.bin";
    sample->outputFilePath = "/data/test/media/vivid_hlg_nv12_out_pq_rgba.yuv";
    sample->isHDRVivid = true;
    sample->isHDRVividOut = true;
    sample->defaultPixelFormat = "nv12";
    sample->convertType = "hdr2hdr_nv12_rgba";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_HLG_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_PQ_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0580
 * @tc.name      : test HDR2HDR ,src colorspace Vivid HLG@NV21,convert to Vivid PQ@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0580, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_hlg_nv21.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_hlg.bin";
    sample->outputFilePath = "/data/test/media/vivid_hlg_nv21_out_pq_nv12.yuv";
    sample->isHDRVivid = true;
    sample->isHDRVividOut = true;
    sample->defaultPixelFormat = "nv21";
    sample->convertType = "hdr2hdr_nv21_nv12";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_HLG_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_PQ_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0590
 * @tc.name      : test HDR2HDR ,src colorspace Vivid HLG@NV21,convert to Vivid PQ@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0590, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_hlg_nv21.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_hlg.bin";
    sample->outputFilePath = "/data/test/media/vivid_hlg_nv21_out_pq_nv21.yuv";
    sample->isHDRVivid = true;
    sample->isHDRVividOut = true;
    sample->defaultPixelFormat = "nv21";
    sample->convertType = "hdr2hdr_nv21_nv21";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_HLG_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_PQ_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}


/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0600
 * @tc.name      : test HDR2HDR ,src colorspace Vivid HLG@NV21,convert to Vivid PQ@RGBA
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0600, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_hlg_nv21.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_hlg.bin";
    sample->outputFilePath = "/data/test/media/vivid_hlg_nv21_out_pq_rgba.yuv";
    sample->isHDRVivid = true;
    sample->isHDRVividOut = true;
    sample->defaultPixelFormat = "nv21";
    sample->convertType = "hdr2hdr_nv21_rgba";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_HLG_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_PQ_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0610
 * @tc.name      : test HDR2HDR ,src colorspace Vivid HLG@RGBA,convert to Vivid PQ@NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0610, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_hlg_rgba.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_hlg.bin";
    sample->outputFilePath = "/data/test/media/vivid_hlg_rgba_out_pq_nv12.yuv";
    sample->isHDRVivid = true;
    sample->isHDRVividOut = true;
    sample->defaultPixelFormat = "rgba";
    sample->convertType = "hdr2hdr_rgba_nv12";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_HLG_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_PQ_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0620
 * @tc.name      : test HDR2HDR ,src colorspace Vivid HLG@RGBA,convert to Vivid PQ@NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0620, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_hlg_rgba.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_hlg.bin";
    sample->outputFilePath = "/data/test/media/vivid_hlg_rgba_out_pq_nv21.yuv";
    sample->isHDRVivid = true;
    sample->isHDRVividOut = true;
    sample->defaultPixelFormat = "rgba";
    sample->convertType = "hdr2hdr_rgba_nv21";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_HLG_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_PQ_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0630
 * @tc.name      : test HDR2HDR ,src colorspace Vivid HLG@RGBA,convert to Vivid PQ@RGBA
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0630, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/vivid_hlg_rgba.yuv";
    sample->inputMetaPath = "/data/test/media/vivid_hlg.bin";
    sample->outputFilePath = "/data/test/media/vivid_hlg_rgba_out_pq_rgba.yuv";
    sample->isHDRVivid = true;
    sample->isHDRVividOut = true;
    sample->defaultPixelFormat = "rgba";
    sample->convertType = "hdr2hdr_rgba_rgba";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_HLG_LIMIT,
                            NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_PQ_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0640
 * @tc.name      : test metadata Generate, input PQ NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0640, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/pq_nv12.yuv";
    sample->outputMetaPath = "/data/test/media/pq_nv12_out.bin";
    sample->defaultPixelFormat = "nv12";
    sample->convertType = "metadata_generate";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                               NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_PQ_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_METADATA_GENERATION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0650
 * @tc.name      : test metadata Generate, input PQ NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0650, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/pq_nv21.yuv";
    sample->outputMetaPath = "/data/test/media/pq_nv21_out.bin";
    sample->defaultPixelFormat = "nv21";
    sample->convertType = "metadata_generate";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                               NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_PQ_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_METADATA_GENERATION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}
/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0660
 * @tc.name      : test metadata Generate, input PQ RGBA
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0660, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/pq_rgba.yuv";
    sample->outputMetaPath = "/data/test/media/pq_rgba_out.bin";
    sample->defaultPixelFormat = "rgba";
    sample->convertType = "metadata_generate";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_PQ_LIMIT,
                               NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_PQ_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_METADATA_GENERATION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}
/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0670
 * @tc.name      : test metadata Generate, input HLG NV12
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0670, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/hlg_nv12.yuv";
    sample->outputMetaPath = "/data/test/media/hlg_nv12_out.bin";
    sample->defaultPixelFormat = "nv12";
    sample->convertType = "metadata_generate";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_HLG_LIMIT,
                               NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_METADATA_GENERATION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}
/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0680
 * @tc.name      : test metadata Generate, input HLG NV21
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0680, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/hlg_nv21.yuv";
    sample->outputMetaPath = "/data/test/media/hlg_nv21_out.bin";
    sample->defaultPixelFormat = "nv21";
    sample->convertType = "metadata_generate";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_HLG_LIMIT,
                               NATIVEBUFFER_PIXEL_FMT_YCRCB_P010, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_METADATA_GENERATION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}
/**
 * @tc.number    : VPE_VIDEO_FUNC_TEST_0690
 * @tc.name      : test metadata Generate, input HLG RGBA
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoFuncTest, VPE_VIDEO_FUNC_TEST_0690, TestSize.Level0)
{
    std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
    sample->inputFilePath = "/data/test/media/hlg_rgba.yuv";
    sample->outputMetaPath = "/data/test/media/hlg_rgba_out.bin";
    sample->defaultPixelFormat = "rgba";
    sample->convertType = "metadata_generate";
    VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_HLG_LIMIT,
                              NATIVEBUFFER_PIXEL_FMT_RGBA_1010102, OH_COLORSPACE_BT2020_HLG_LIMIT};
    int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_METADATA_GENERATION,
                            DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
    ASSERT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    sample->StartProcess();
    ASSERT_EQ(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
    ASSERT_TRUE(sample->md5Equal);
}

#endif
}