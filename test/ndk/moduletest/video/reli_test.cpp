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
#include "yuv_viewer.h"
#include "enum_list.h"
#include "video_sample.h"
using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace {
constexpr uint32_t DEFAULT_WIDTH = 3840;
constexpr uint32_t DEFAULT_HEIGHT = 2160;


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
}
void VpeVideoReliTest::TearDown()
{
}
}

namespace {
int32_t TestUnsupportedOutput(int32_t inColorSpace, int32_t inPixFmt)
{
    for (int i : g_nativeBufferColorSpace) {
        for (int j : g_nativeBufferFormat) {
            for (int k : g_nativeBufferMetadataType) {
                std::unique_ptr<VideoSample> sample = std::make_unique<VideoSample>();
                sample->inputFrameNumber = 1;
                VideoProcessParam param = {inPixFmt, inColorSpace, j, i};
                int32_t ret = sample->InitVideoSample(VIDEO_PROCESSING_TYPE_COLOR_SPACE_CONVERSION,
                                        DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
                if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", F_OK)) {
                    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
                    sample->StartProcess();
                    EXPECT_NE(sample->WaitAndStopSample(), VIDEO_PROCESSING_SUCCESS);
                }
            }
        }
    }
}

bool ValidatePixelFormat(ImageProcessing_ColorSpaceInfo formatImage)
{
    if (formatImage == nullptr) {
        return false;
    }
    bool ret = (formatImage.pixelFormat == NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP ||
                formatImage.pixelFormat == NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP ||
                formatImage.pixelFormat == NATIVEBUFFER_PIXEL_FMT_RGBA_8888);
    return ret;
}

/**
 * @tc.number    : VPE_VIDEO_RELI_TEST_0010
 * @tc.name      : test all unsupported convert options
 * @tc.desc      : function test
 */
HWTEST(VpeVideoReliTest, VPE_VIDEO_RELI_TEST_0010, TestSize.Level0)
{
    for (int i : g_nativeBufferColorSpace) {
        for (int j : g_nativeBufferFormat) {
            TestUnsupportedOutput(i, j);
        }
    }
}

/**
 * @tc.number    : METADATASUPPORT_001
 * @tc.name      : test all unsupported metadata generation
 * @tc.desc      : function test
 */
HWTEST_F(VpeVideoReliTest, METADATASUPPORT_001, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    for (int i : g_nativeBufferMetadataType) {
        for (int j: g_nativeBufferColorSpace) {
            for (int k : g_nativeBufferFormat) {
                inputFormat.metadataType = i;
                inputFormat.colorSpace = j;
                inputFormat.pixelFormat = k;
                bool ret = OH_VideoProcessing_IsMetadataGenerationSupported(inputFormat);
            }
        }
    }
}


void CheckCapability(VideoProcessing_ColorSpaceInfo inputFormat)
{
    if (formatImage.colorSpace == OH_COLORSPACE_SRGB_FULL ||
        formatImage.colorSpace == OH_COLORSPACE_SRGB_LIMIT ||
        formatImage.colorSpace == OH_COLORSPACE_LINEAR_SRGB ||
        formatImage.colorSpace == OH_COLORSPACE_DISPLAY_SRGB ||
        formatImage.colorSpace == OH_COLORSPACE_DISPLAY_P3_SRGB ||
        formatImage.colorSpace == OH_COLORSPACE_DISPLAY_BT2020_SRGB) {
            if (ValidatePixelFormat(formatImage)) {
                if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", F_OK)) {
                    if (!access("/system/lib64/media/", 0)) {
                        ASSERT_EQ(true, OH_ImageProcessing_IsMetadataGenerationSupported(formatImage));
                    } else {
                        ASSERT_EQ(false, OH_ImageProcessing_IsMetadataGenerationSupported(formatImage));
                    }
                }
            }
    }
    if (formatImage.colorSpace == OH_COLORSPACE_DISPLAY_P3_SRGB ||
        formatImage.colorSpace == OH_COLORSPACE_DISPLAY_P3_HLG ||
        formatImage.colorSpace == OH_COLORSPACE_DISPLAY_P3_PQ) {
        if (ValidatePixelFormat(formatImage)) {
            if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", F_OK)) {
                if (!access("/system/lib64/media/", 0)) {
                    ASSERT_EQ(true, OH_ImageProcessing_IsMetadataGenerationSupported(formatImage));
                } else {
                    ASSERT_EQ(false, OH_ImageProcessing_IsMetadataGenerationSupported(formatImage));
                }
            }
        }
    }
    if (formatImage.colorSpace == OH_COLORSPACE_ADOBERGB_FULL ||
        formatImage.colorSpace == OH_COLORSPACE_ADOBERGB_LIMIT) {
        if (ValidatePixelFormat(formatImage)) {
            if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", F_OK)) {
                if (!access("/system/lib64/media/", 0)) {
                    ASSERT_EQ(true, OH_ImageProcessing_IsMetadataGenerationSupported(formatImage));
                } else {
                    ASSERT_EQ(false, OH_ImageProcessing_IsMetadataGenerationSupported(formatImage));
                }
            }
        }
    }
}

HWTEST_F(VpeVideoReliTest, METADATASUPPORT_002, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo formatImage;
    for (int i : g_nativeBufferMetadataType) {
        for (int j: g_nativeBufferColorSpace) {
            for (int k : g_nativeBufferFormat) {
                formatImage.metadataType = i;
                formatImage.colorSpace = j;
                formatImage.pixelFormat = k;
                CheckCapability(formatImage);
            }
        }
    }
}

HWTEST_F(VpeVideoReliTest, METADATASUPPORT_003, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo formatImage;
    for (int i : g_nativeBufferMetadataType) {
        for (int j: g_nativeBufferColorSpace) {
            for (int k : g_nativeBufferFormat) {
                formatImage.metadataType = i;
                formatImage.colorSpace = j;
                formatImage.pixelFormat = k;
                cout<<"--metadataType--" << i << "--colorSpace--" << j << "--pixelFormat--" << k << endl;
            }
        }
        if (formatImage.colorSpace == OH_COLORSPACE_SRGB_FULL ||
            formatImage.colorSpace == OH_COLORSPACE_SRGB_LIMIT ||
            formatImage.colorSpace == OH_COLORSPACE_LINEAR_SRGB ||
            formatImage.colorSpace == OH_COLORSPACE_DISPLAY_SRGB ||
            formatImage.colorSpace == OH_COLORSPACE_DISPLAY_P3_SRGB ||
            formatImage.colorSpace == OH_COLORSPACE_DISPLAY_BT2020_SRGB) {
            if (ValidatePixelFormat(formatImage)){
                if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", F_OK)) {
                    if (!access("/system/lib64/media/", 0)) {
                        cout<<"return true"<< endl;
                        ASSERT_EQ(true, OH_ImageProcessing_IsMetadataGenerationSupported(formatImage));
                    } else {
                        cout<<"return false"<< endl;
                        ASSERT_EQ(false, OH_ImageProcessing_IsMetadataGenerationSupported(formatImage));
                    }
                }
            }
        } else if (formatImage.colorSpace == OH_COLORSPACE_DISPLAY_P3_SRGB ||
                   formatImage.colorSpace == OH_COLORSPACE_DISPLAY_P3_HLG ||
                   formatImage.colorSpace == OH_COLORSPACE_DISPLAY_P3_PQ) {
            if (ValidatePixelFormat(formatImage)){
                if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", F_OK)) {
                    if (!access("/system/lib64/media/", 0)) {
                        cout<<"return true"<< endl;
                        ASSERT_EQ(true, OH_ImageProcessing_IsMetadataGenerationSupported(formatImage));
                    } else {
                        cout<<"return false"<< endl;
                        ASSERT_EQ(false, OH_ImageProcessing_IsMetadataGenerationSupported(formatImage));
                    }
                }
            }
        } else if (formatImage.colorSpace == OH_COLORSPACE_ADOBERGB_FULL ||
                   formatImage.colorSpace == OH_COLORSPACE_ADOBERGB_LIMIT) {
            if (ValidatePixelFormat(formatImage)){
                if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", F_OK)) {
                    if (!access("/system/lib64/media/", 0)) {
                        ASSERT_EQ(true, OH_ImageProcessing_IsMetadataGenerationSupported(formatImage));
                    } else {
                        cout<<"return false"<< endl;
                        ASSERT_EQ(false, OH_ImageProcessing_IsMetadataGenerationSupported(formatImage));
                    }
                }
            }
        } else {
            cout<<"return false"<< endl;
            ASSERT_EQ(false, OH_ImageProcessing_IsMetadataGenerationSupported(formatImage));
        }
    }
}
}