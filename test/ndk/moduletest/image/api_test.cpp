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
#include <atomic>
#include <fstream>
#include <thread>
#include <queue>
#include <string>
#include "gtest/gtest.h"
#include "image_processing.h"
#include "native_color_space_manager.h"
#include "nocopyable.h"
#include "pixelmap_native.h"
#include "native_buffer.h"
using namespace std;
using namespace OHOS;
using namespace testing::ext;
namespace {
constexpr uint32_t DEFAULT_WIDTH = 3840;
constexpr uint32_t DEFAULT_HEIGHT = 2160;
constexpr uint32_t PIX_SIZE = DEFAULT_WIDTH * DEFAULT_HEIGHT * 4;
uint8_t *g_pixData = nullptr;

OH_Pixelmap_InitializationOptions *g_createOpts_RGBA = nullptr;
OH_Pixelmap_InitializationOptions *g_createOpts_BGRA = nullptr;
OH_PixelmapNative *pixelMap_RGBA = nullptr;
OH_PixelmapNative *pixelMap_BGRA = nullptr;
void InitCreatePixelmapParamRGBA()
{
    if (g_createOpts_RGBA == nullptr) {
        OH_PixelmapInitializationOptions_Create(&g_createOpts_RGBA);
        OH_PixelmapInitializationOptions_SetWidth(g_createOpts_RGBA, DEFAULT_WIDTH);
        OH_PixelmapInitializationOptions_SetHeight(g_createOpts_RGBA, DEFAULT_HEIGHT);
        OH_PixelmapInitializationOptions_SetPixelFormat(g_createOpts_RGBA, NATIVEBUFFER_PIXEL_FMT_RGBA_8888);
    }
}
void InitCreatePixelmapParamBGRA()
{
    if (g_createOpts_BGRA == nullptr) {
        OH_PixelmapInitializationOptions_Create(&g_createOpts_BGRA);
        OH_PixelmapInitializationOptions_SetWidth(g_createOpts_BGRA, DEFAULT_WIDTH);
        OH_PixelmapInitializationOptions_SetHeight(g_createOpts_BGRA, DEFAULT_HEIGHT);
        OH_PixelmapInitializationOptions_SetPixelFormat(g_createOpts_BGRA, PIXEL_FORMAT_BGRA_8888);
    }
}


class VpeImageApiTest : public testing::Test {
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


void VpeImageApiTest::SetUpTestCase()
{
    g_pixData = new uint8_t[PIX_SIZE];
    InitCreatePixelmapParamRGBA();
    InitCreatePixelmapParamBGRA();
    OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &pixelMap_RGBA);
    OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_BGRA, &pixelMap_BGRA);
}
void VpeImageApiTest::TearDownTestCase()
{
    if (g_pixData) {
        delete[] g_pixData;
        g_pixData = nullptr;
    }
    if (g_createOpts_RGBA) {
        OH_PixelmapInitializationOptions_Release(g_createOpts_RGBA);
        g_createOpts_RGBA = nullptr;
    }
    if (g_createOpts_BGRA) {
        OH_PixelmapInitializationOptions_Release(g_createOpts_BGRA);
        g_createOpts_BGRA = nullptr;
    }
}
void VpeImageApiTest::SetUp()
{
}
void VpeImageApiTest::TearDown()
{
    OH_ImageProcessing_DeinitializeEnvironment();
}


const ImageProcessing_ColorSpaceInfo CSC_SRC_INFO = {HDR_METADATA_TYPE_NONE,
                                                     SRGB,
                                                     PIXEL_FORMAT_RGBA_8888};
const ImageProcessing_ColorSpaceInfo CSC_DST_INFO = {HDR_METADATA_TYPE_NONE,
                                                     DISPLAY_P3,
                                                     PIXEL_FORMAT_RGBA_8888};
const ImageProcessing_ColorSpaceInfo COMPOSE_SRC_INFO = {HDR_METADATA_TYPE_BASE,
                                                         SRGB,
                                                         PIXEL_FORMAT_RGBA_8888};

const ImageProcessing_ColorSpaceInfo COMPOSE_SRC_GAIN_INFO = {HDR_METADATA_TYPE_GAINMAP,
                                                              SRGB,
                                                              PIXEL_FORMAT_RGBA_8888};
const ImageProcessing_ColorSpaceInfo COMPOSE_DST_INFO = {HDR_METADATA_TYPE_ALTERNATE,
                                                         BT2020_HLG,
                                                         PIXEL_FORMAT_RGBA_1010102};
const ImageProcessing_ColorSpaceInfo UNSUPPORTED_INFO = {HDR_METADATA_TYPE_NONE,
                                                         BT601_EBU,
                                                         PIXEL_FORMAT_BGRA_8888};

}

namespace {
#ifdef ENABLE_ALL_PROCESS
static bool g_suppported = true;
#else
static bool g_suppported = false;
#endif

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0010
 * @tc.name      : first call OH_ImageProcessing_InitializeEnvironment
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0010, TestSize.Level0)
{
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_InitializeEnvironment();
    EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
    OH_ImageProcessing_DeinitializeEnvironment();
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0020
 * @tc.name      : first call OH_ImageProcessing_DeinitializeEnvironment
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0020, TestSize.Level0)
{
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_DeinitializeEnvironment();
    EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_OPERATION_NOT_PERMITTED);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0030
 * @tc.name      : first call OH_ImageProcessing_DeinitializeEnvironment after initialize
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0030, TestSize.Level0)
{
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_InitializeEnvironment();
    EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
    OH_ImageProcessing_DeinitializeEnvironment();
    EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0040
 * @tc.name      : call OH_ImageProcessing_IsColorSpaceConversionSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0040, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsColorSpaceConversionSupported(nullptr, nullptr);
    ASSERT_FALSE(ret);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0050
 * @tc.name      : call OH_ImageProcessing_IsColorSpaceConversionSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0050, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsColorSpaceConversionSupported(&CSC_SRC_INFO, nullptr);
    ASSERT_FALSE(ret);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0060
 * @tc.name      : call OH_ImageProcessing_IsColorSpaceConversionSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0060, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsColorSpaceConversionSupported(nullptr, &CSC_DST_INFO);
    ASSERT_FALSE(ret);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0070
 * @tc.name      : call OH_ImageProcessing_IsColorSpaceConversionSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0070, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsColorSpaceConversionSupported(&CSC_SRC_INFO, &CSC_DST_INFO);
    if (g_suppported) {
        ASSERT_TRUE(ret);
    } else {
        ASSERT_FALSE(ret);
    }
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0080
 * @tc.name      : call OH_ImageProcessing_IsCompositionSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0080, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsCompositionSupported(nullptr, nullptr, nullptr);
    ASSERT_FALSE(ret);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0090
 * @tc.name      : call OH_ImageProcessing_IsCompositionSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0090, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsCompositionSupported(&COMPOSE_SRC_INFO, nullptr, nullptr);
    ASSERT_FALSE(ret);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0100
 * @tc.name      : call OH_ImageProcessing_IsCompositionSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0100, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsCompositionSupported(nullptr, nullptr, &COMPOSE_DST_INFO);
    ASSERT_FALSE(ret);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0110
 * @tc.name      : call OH_ImageProcessing_IsCompositionSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0110, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsCompositionSupported(nullptr, &COMPOSE_SRC_GAIN_INFO, nullptr);
    ASSERT_FALSE(ret);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0120
 * @tc.name      : call OH_ImageProcessing_IsCompositionSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0120, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsCompositionSupported(&COMPOSE_SRC_INFO, nullptr, &COMPOSE_DST_INFO);
    ASSERT_FALSE(ret);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0130
 * @tc.name      : call OH_ImageProcessing_IsCompositionSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0130, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsCompositionSupported(nullptr, &COMPOSE_SRC_GAIN_INFO, &COMPOSE_DST_INFO);
    ASSERT_FALSE(ret);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0131
 * @tc.name      : call OH_ImageProcessing_IsCompositionSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0131, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsCompositionSupported(&COMPOSE_SRC_INFO, &COMPOSE_SRC_GAIN_INFO, nullptr);
    ASSERT_FALSE(ret);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0140
 * @tc.name      : call OH_ImageProcessing_IsCompositionSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0140, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsCompositionSupported(&COMPOSE_SRC_INFO, &COMPOSE_SRC_GAIN_INFO, &COMPOSE_DST_INFO);
    if (g_suppported) {
        ASSERT_TRUE(ret);
    } else {
        ASSERT_FALSE(ret);
    }
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0150
 * @tc.name      : call OH_ImageProcessing_IsDecompositionSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0150, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsDecompositionSupported(nullptr, nullptr, nullptr);
    ASSERT_FALSE(ret);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0160
 * @tc.name      : call OH_ImageProcessing_IsDecompositionSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0160, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsDecompositionSupported(&COMPOSE_DST_INFO, nullptr, nullptr);
    ASSERT_FALSE(ret);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0170
 * @tc.name      : call OH_ImageProcessing_IsDecompositionSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0170, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsDecompositionSupported(nullptr, &COMPOSE_SRC_INFO, nullptr);
    ASSERT_FALSE(ret);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0180
 * @tc.name      : call OH_ImageProcessing_IsDecompositionSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0180, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsDecompositionSupported(nullptr, nullptr, &COMPOSE_SRC_GAIN_INFO);
    ASSERT_FALSE(ret);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0190
 * @tc.name      : call OH_ImageProcessing_IsDecompositionSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0190, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsDecompositionSupported(&COMPOSE_DST_INFO, nullptr, &COMPOSE_SRC_GAIN_INFO);
    ASSERT_FALSE(ret);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0200
 * @tc.name      : call OH_ImageProcessing_IsDecompositionSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0200, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsDecompositionSupported(nullptr, &COMPOSE_SRC_INFO, &COMPOSE_SRC_GAIN_INFO);
    ASSERT_FALSE(ret);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0201
 * @tc.name      : call OH_ImageProcessing_IsDecompositionSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0201, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsDecompositionSupported(&COMPOSE_DST_INFO, &COMPOSE_SRC_INFO, nullptr);
    ASSERT_FALSE(ret);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0210
 * @tc.name      : call OH_ImageProcessing_IsDecompositionSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0210, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsDecompositionSupported(&COMPOSE_DST_INFO,
        &COMPOSE_SRC_INFO, &COMPOSE_SRC_GAIN_INFO);
    if (g_suppported) {
        ASSERT_TRUE(ret);
    } else {
        ASSERT_FALSE(ret);
    }
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0220
 * @tc.name      : call OH_ImageProcessing_IsMetadataGenerationSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0220, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsMetadataGenerationSupported(nullptr);
    ASSERT_FALSE(ret);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0230
 * @tc.name      : call OH_ImageProcessing_IsMetadataGenerationSupported
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0230, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    const ImageProcessing_ColorSpaceInfo HDR_INFO = {HDR_METADATA_TYPE_ALTERNATE, BT2020_HLG,
        PIXEL_FORMAT_RGBA_1010102};
    bool ret = OH_ImageProcessing_IsMetadataGenerationSupported(&HDR_INFO);
    if (g_suppported) {
        ASSERT_TRUE(ret);
    } else {
        ASSERT_FALSE(ret);
    }
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0240
 * @tc.name      : call OH_ImageProcessing_Create
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0240, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(nullptr, INT_MAX);
    EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_INSTANCE);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0250
 * @tc.name      : call OH_ImageProcessing_Create
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0250, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(nullptr,
        IMAGE_PROCESSING_TYPE_METADATA_GENERATION);
    EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_INSTANCE);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0260
 * @tc.name      : call OH_ImageProcessing_Create
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0260, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* imageProcessor = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor, INT_MAX);
    EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER);
    OH_ImageProcessing_Destroy(imageProcessor);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0270
 * @tc.name      : call OH_ImageProcessing_Create
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0270, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* imageProcessor = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor,
        IMAGE_PROCESSING_TYPE_METADATA_GENERATION);
    if (g_suppported) {
        EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
    } else {
        EXPECT_NE(ret, IMAGE_PROCESSING_SUCCESS);
    }
    OH_ImageProcessing_Destroy(imageProcessor);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0280
 * @tc.name      : call OH_ImageProcessing_Destroy
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0280, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Destroy(nullptr);
    EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_INSTANCE);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0290
 * @tc.name      : call OH_ImageProcessing_Destroy
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0290, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* imageProcessor = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor,
        IMAGE_PROCESSING_TYPE_METADATA_GENERATION);
    if (g_suppported) {
        EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
        ret = OH_ImageProcessing_Destroy(imageProcessor);
        EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
    }
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0450
 * @tc.name      : call OH_ImageProcessing_ConvertColorSpace
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0450, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_ConvertColorSpace(nullptr,
        nullptr, nullptr);
    EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_INSTANCE);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0460
 * @tc.name      : call OH_ImageProcessing_ConvertColorSpace
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0460, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* imageProcessor = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor,
        IMAGE_PROCESSING_TYPE_COLOR_SPACE_CONVERSION);
    if (g_suppported) {
        EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
        ret = OH_ImageProcessing_ConvertColorSpace(imageProcessor, nullptr, nullptr);
        EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER);
    }
    OH_ImageProcessing_Destroy(imageProcessor);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0470
 * @tc.name      : call OH_ImageProcessing_ConvertColorSpace
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0470, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_ConvertColorSpace(nullptr, pixelMap_RGBA, nullptr);
    EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_INSTANCE);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0480
 * @tc.name      : call OH_ImageProcessing_ConvertColorSpace
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0480, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_ConvertColorSpace(
        nullptr, nullptr, pixelMap_RGBA);
    EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_INSTANCE);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0490
 * @tc.name      : call OH_ImageProcessing_ConvertColorSpace
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0490, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* imageProcessor = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor,
        IMAGE_PROCESSING_TYPE_COLOR_SPACE_CONVERSION);
    if (g_suppported) {
        EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
        ret = OH_ImageProcessing_ConvertColorSpace(imageProcessor, nullptr, pixelMap_RGBA);
        EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER);
    }
    OH_ImageProcessing_Destroy(imageProcessor);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0500
 * @tc.name      : call OH_ImageProcessing_ConvertColorSpace
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0500, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    if (g_suppported) {
        OH_PixelmapNative *src_pixelMap_RGBA = nullptr;
        OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &src_pixelMap_RGBA);
        OH_PixelmapNative *dst_pixelMap_RGBA = nullptr;
        OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &dst_pixelMap_RGBA);
        ImageProcessing_ErrorCode ret = OH_ImageProcessing_ConvertColorSpace(
        nullptr, src_pixelMap_RGBA, dst_pixelMap_RGBA);
        EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_INSTANCE);
        OH_PixelmapNative_Release(src_pixelMap_RGBA);
        OH_PixelmapNative_Release(dst_pixelMap_RGBA);
    }
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0530
 * @tc.name      : call OH_ImageProcessing_Compose
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0530, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Compose(nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_INSTANCE);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0540
 * @tc.name      : call OH_ImageProcessing_Compose
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0540, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* imageProcessor = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor,
        IMAGE_PROCESSING_TYPE_COMPOSITION);
    if (g_suppported) {
        EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
        ret = OH_ImageProcessing_Compose(imageProcessor, nullptr, nullptr, nullptr);
        EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER);
    }
    OH_ImageProcessing_Destroy(imageProcessor);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0550
 * @tc.name      : call OH_ImageProcessing_Compose
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0550, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_PixelmapNative *src_pixelMap_RGBA = nullptr;
    OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &src_pixelMap_RGBA);
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Compose(nullptr, src_pixelMap_RGBA, nullptr, nullptr);
    EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_INSTANCE);
    OH_PixelmapNative_Release(src_pixelMap_RGBA);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0560
 * @tc.name      : call OH_ImageProcessing_Compose
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0560, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_PixelmapNative *src_gain_pixelMap_RGBA = nullptr;
    OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &src_gain_pixelMap_RGBA);
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Compose(nullptr, nullptr, src_gain_pixelMap_RGBA, nullptr);
    EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_INSTANCE);
    OH_PixelmapNative_Release(src_gain_pixelMap_RGBA);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0570
 * @tc.name      : call OH_ImageProcessing_Compose
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0570, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_PixelmapNative *dst_pixelMap_RGBA = nullptr;
    OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &dst_pixelMap_RGBA);
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Compose(nullptr, nullptr, nullptr, dst_pixelMap_RGBA);
    EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_INSTANCE);

    OH_PixelmapNative_Release(dst_pixelMap_RGBA);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0580
 * @tc.name      : call OH_ImageProcessing_Compose
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0580, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* imageProcessor = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor,
        IMAGE_PROCESSING_TYPE_COMPOSITION);
    if (g_suppported) {
        EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
        OH_PixelmapNative *src_pixelMap_RGBA = nullptr;
        OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &src_pixelMap_RGBA);
        ret = OH_ImageProcessing_Compose(imageProcessor, src_pixelMap_RGBA, nullptr, nullptr);
        EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER);
        OH_ImageProcessing_Destroy(imageProcessor);
        OH_PixelmapNative_Release(src_pixelMap_RGBA);
    }
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0590
 * @tc.name      : call OH_ImageProcessing_Compose
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0590, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* imageProcessor = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor,
        IMAGE_PROCESSING_TYPE_COMPOSITION);
    if (g_suppported) {
        EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
        OH_PixelmapNative *src_gain_pixelMap_RGBA = nullptr;
        OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &src_gain_pixelMap_RGBA);
        ret = OH_ImageProcessing_Compose(imageProcessor, nullptr, src_gain_pixelMap_RGBA, nullptr);
        EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER);
        OH_ImageProcessing_Destroy(imageProcessor);
        OH_PixelmapNative_Release(src_gain_pixelMap_RGBA);
    }
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0600
 * @tc.name      : call OH_ImageProcessing_Compose
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0600, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* imageProcessor = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor,
        IMAGE_PROCESSING_TYPE_COMPOSITION);
    if (g_suppported) {
        EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
        OH_PixelmapNative *dst_pixelMap_RGBA = nullptr;
        OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &dst_pixelMap_RGBA);
        ret = OH_ImageProcessing_Compose(imageProcessor, nullptr, nullptr, dst_pixelMap_RGBA);
        EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER);
        OH_ImageProcessing_Destroy(imageProcessor);
        OH_PixelmapNative_Release(dst_pixelMap_RGBA);
    }
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0610
 * @tc.name      : call OH_ImageProcessing_Compose
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0610, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* imageProcessor = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor,
        IMAGE_PROCESSING_TYPE_COMPOSITION);
    if (g_suppported) {
        EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
        OH_PixelmapNative *src_pixelMap_RGBA = nullptr;
        OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &src_pixelMap_RGBA);
        OH_PixelmapNative *src_gain_pixelMap_RGBA = nullptr;
        OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &src_gain_pixelMap_RGBA);
        ret = OH_ImageProcessing_Compose(imageProcessor, src_pixelMap_RGBA, src_gain_pixelMap_RGBA, nullptr);
        EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER);
        OH_ImageProcessing_Destroy(imageProcessor);
        OH_PixelmapNative_Release(src_pixelMap_RGBA);
        OH_PixelmapNative_Release(src_gain_pixelMap_RGBA);
    }
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0620
 * @tc.name      : call OH_ImageProcessing_Compose
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0620, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* imageProcessor = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor,
        IMAGE_PROCESSING_TYPE_COMPOSITION);
    if (g_suppported) {
        EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
        OH_PixelmapNative *src_gain_pixelMap_RGBA = nullptr;
        OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &src_gain_pixelMap_RGBA);
        OH_PixelmapNative *dst_pixelMap_RGBA = nullptr;
        OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &dst_pixelMap_RGBA);
        ret = OH_ImageProcessing_Compose(imageProcessor, nullptr, src_gain_pixelMap_RGBA, dst_pixelMap_RGBA);
        EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER);
        OH_ImageProcessing_Destroy(imageProcessor);
        OH_PixelmapNative_Release(src_gain_pixelMap_RGBA);
        OH_PixelmapNative_Release(dst_pixelMap_RGBA);
    }
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0650
 * @tc.name      : call OH_ImageProcessing_Decompose
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0650, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Decompose(nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_INSTANCE);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0660
 * @tc.name      : call OH_ImageProcessing_Decompose
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0660, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* imageProcessor = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor,
        IMAGE_PROCESSING_TYPE_DECOMPOSITION);
    if (g_suppported) {
        EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
        ret = OH_ImageProcessing_Decompose(imageProcessor, nullptr, nullptr, nullptr);
        EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER);
    }
    OH_ImageProcessing_Destroy(imageProcessor);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0670
 * @tc.name      : call OH_ImageProcessing_Decompose
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0670, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_PixelmapNative *src_pixelMap_RGBA = nullptr;
    OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &src_pixelMap_RGBA);
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Decompose(nullptr, src_pixelMap_RGBA, nullptr, nullptr);
    EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_INSTANCE);
    OH_PixelmapNative_Release(src_pixelMap_RGBA);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0680
 * @tc.name      : call OH_ImageProcessing_Decompose
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0680, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_PixelmapNative *dst_pixelMap_RGBA = nullptr;
    OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &dst_pixelMap_RGBA);
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Decompose(nullptr, nullptr, dst_pixelMap_RGBA, nullptr);
    EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_INSTANCE);
    OH_PixelmapNative_Release(dst_pixelMap_RGBA);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0690
 * @tc.name      : call OH_ImageProcessing_Decompose
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0690, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_PixelmapNative *dst_gain_pixelMap_RGBA = nullptr;
    OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &dst_gain_pixelMap_RGBA);
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Decompose(nullptr, nullptr, nullptr, dst_gain_pixelMap_RGBA);
    EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_INSTANCE);
    OH_PixelmapNative_Release(dst_gain_pixelMap_RGBA);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0700
 * @tc.name      : call OH_ImageProcessing_Decompose
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0700, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* imageProcessor = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor,
        IMAGE_PROCESSING_TYPE_DECOMPOSITION);
    if (g_suppported) {
        EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
        OH_PixelmapNative *src_pixelMap_RGBA = nullptr;
        OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &src_pixelMap_RGBA);
        ret = OH_ImageProcessing_Decompose(imageProcessor, src_pixelMap_RGBA, nullptr, nullptr);
        EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER);
        OH_PixelmapNative_Release(src_pixelMap_RGBA);
    }
    OH_ImageProcessing_Destroy(imageProcessor);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0710
 * @tc.name      : call OH_ImageProcessing_Decompose
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0710, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* imageProcessor = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor,
        IMAGE_PROCESSING_TYPE_DECOMPOSITION);
    if (g_suppported) {
        EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
        OH_PixelmapNative *dst_pixelMap_RGBA = nullptr;
        OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &dst_pixelMap_RGBA);
        ret = OH_ImageProcessing_Decompose(imageProcessor, nullptr, dst_pixelMap_RGBA, nullptr);
        EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER);
        OH_PixelmapNative_Release(dst_pixelMap_RGBA);
    }
    OH_ImageProcessing_Destroy(imageProcessor);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0720
 * @tc.name      : call OH_ImageProcessing_Decompose
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0720, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* imageProcessor = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor,
        IMAGE_PROCESSING_TYPE_DECOMPOSITION);
    if (g_suppported) {
        EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
        OH_PixelmapNative *dst_gain_pixelMap_RGBA = nullptr;
        OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &dst_gain_pixelMap_RGBA);
        ret = OH_ImageProcessing_Decompose(imageProcessor, nullptr, nullptr, dst_gain_pixelMap_RGBA);
        EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER);
        OH_PixelmapNative_Release(dst_gain_pixelMap_RGBA);
    }
    OH_ImageProcessing_Destroy(imageProcessor);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0730
 * @tc.name      : call OH_ImageProcessing_Decompose
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0730, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* imageProcessor = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor,
        IMAGE_PROCESSING_TYPE_DECOMPOSITION);
    if (g_suppported) {
        EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
        OH_PixelmapNative *src_pixelMap_RGBA = nullptr;
        OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &src_pixelMap_RGBA);
        OH_PixelmapNative *dst_pixelMap_RGBA = nullptr;
        OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &dst_pixelMap_RGBA);
        ret = OH_ImageProcessing_Decompose(imageProcessor, src_pixelMap_RGBA, dst_pixelMap_RGBA, nullptr);
        EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER);
        OH_PixelmapNative_Release(src_pixelMap_RGBA);
        OH_PixelmapNative_Release(dst_pixelMap_RGBA);
    }
    OH_ImageProcessing_Destroy(imageProcessor);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0740
 * @tc.name      : call OH_ImageProcessing_Decompose
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0740, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* imageProcessor = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor,
        IMAGE_PROCESSING_TYPE_DECOMPOSITION);
    if (g_suppported) {
        EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
        OH_PixelmapNative *dst_pixelMap_RGBA = nullptr;
        OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &dst_pixelMap_RGBA);
        OH_PixelmapNative *dst_gain_pixelMap_RGBA = nullptr;
        OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_RGBA, &dst_gain_pixelMap_RGBA);
        ret = OH_ImageProcessing_Decompose(imageProcessor, nullptr,
                                           dst_pixelMap_RGBA, dst_gain_pixelMap_RGBA);
        EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER);
        OH_PixelmapNative_Release(dst_pixelMap_RGBA);
        OH_PixelmapNative_Release(dst_gain_pixelMap_RGBA);
    }
    OH_ImageProcessing_Destroy(imageProcessor);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0770
 * @tc.name      : call OH_ImageProcessing_GenerateMetadata
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0770, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_GenerateMetadata(nullptr, nullptr);
    EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_INSTANCE);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0780
 * @tc.name      : call OH_ImageProcessing_GenerateMetadata
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0780, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* imageProcessor = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor,
        IMAGE_PROCESSING_TYPE_METADATA_GENERATION);
    if (g_suppported) {
        EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
        ret = OH_ImageProcessing_GenerateMetadata(imageProcessor, nullptr);
        EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER);
    }
    OH_ImageProcessing_Destroy(imageProcessor);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0790
 * @tc.name      : call OH_ImageProcessing_GenerateMetadata by the color space
 * of the image is unsupported.
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0790, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* imageProcessor = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor,
        IMAGE_PROCESSING_TYPE_METADATA_GENERATION);
    if (g_suppported) {
        EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
        OH_PixelmapNative *src_pixelMap_BGRA = nullptr;
        OH_PixelmapNative_CreatePixelmap(g_pixData, PIX_SIZE, g_createOpts_BGRA, &src_pixelMap_BGRA);
        ret = OH_ImageProcessing_GenerateMetadata(imageProcessor, src_pixelMap_BGRA);
        EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_UNSUPPORTED_PROCESSING);
        OH_PixelmapNative_Release(src_pixelMap_BGRA);
    }
    OH_ImageProcessing_Destroy(imageProcessor);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0890
 * @tc.name      : call OH_ImageProcessing_IsColorSpaceConversionSupported
 * the the color space conversion is unsupported.
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0890, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsColorSpaceConversionSupported(&UNSUPPORTED_INFO, &CSC_DST_INFO);
    if (g_suppported) {
        ASSERT_FALSE(ret);
    }
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0900
 * @tc.name      : call OH_ImageProcessing_IsCompositionSupported
 * the the color space conversion is unsupported.
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0900, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsCompositionSupported(&UNSUPPORTED_INFO,
        &COMPOSE_SRC_GAIN_INFO, &COMPOSE_DST_INFO);
    if (g_suppported) {
        ASSERT_FALSE(ret);
    }
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0910
 * @tc.name      : call OH_ImageProcessing_IsDecompositionSupported
 * the the color space conversion is unsupported.
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0910, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsDecompositionSupported(&UNSUPPORTED_INFO,
        &COMPOSE_SRC_INFO, &COMPOSE_SRC_GAIN_INFO);
    if (g_suppported) {
        ASSERT_FALSE(ret);
    }
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0920
 * @tc.name      : call OH_ImageProcessing_IsMetadataGenerationSupported
 * the the color space conversion is unsupported.
 * @tc.desc      : function test
 */
HWTEST_F(VpeImageApiTest, VPE_IMAGE_API_TEST_0920, TestSize.Level0)
{
    OH_ImageProcessing_InitializeEnvironment();
    bool ret = OH_ImageProcessing_IsMetadataGenerationSupported(&UNSUPPORTED_INFO);
    if (g_suppported) {
        ASSERT_FALSE(ret);
    }
}
}