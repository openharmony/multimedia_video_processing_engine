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
#include "gtest/gtest.h"
#include "video_processing.h"
#include "image_processing_types.h"
#include "image_processing.h"
#include "enum_list.h"
#include "pixelmap_native.h"
#include "native_color_space_manager.h"

using namespace std;
using namespace testing::ext;
namespace {
class VpeVideoCapTest : public testing::Test {
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
void VpeVideoCapTest::SetUpTestCase()
{
    OH_ImageProcessing_InitializeEnvironment();
}
void VpeVideoCapTest::TearDownTestCase()
{
    OH_ImageProcessing_DeinitializeEnvironment();
}
void VpeVideoCapTest::SetUp() {}
void VpeVideoCapTest::TearDown() {}
}
namespace {
#ifdef ENABLE_ALL_PROCESS
static bool g_suppported = true;
#else
static bool g_suppported = false;
#endif

/**
 * @tc.number    : COLORSPACE_SUPPORT_001
 * @tc.name      : adobergb to srgb
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_001, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    ImageProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = HDR_METADATA_TYPE_NONE;
    inputFormat.colorSpace = ADOBE_RGB;
    inputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;
    outputFormat.metadataType = HDR_METADATA_TYPE_NONE;
    outputFormat.colorSpace = SRGB;
    outputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
    }
}


/**
 * @tc.number    : COLORSPACE_SUPPORT_002
 * @tc.name      : adobergb to display p3
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_002, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    ImageProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = HDR_METADATA_TYPE_NONE;
    inputFormat.colorSpace = ADOBE_RGB;
    inputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;
    outputFormat.metadataType = HDR_METADATA_TYPE_NONE;
    outputFormat.colorSpace = DISPLAY_P3;
    outputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
    }
}

/**
 * @tc.number    : COLORSPACE_SUPPORT_003
 * @tc.name      : srgb to display p3
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_003, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    ImageProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = HDR_METADATA_TYPE_NONE;
    inputFormat.colorSpace = SRGB;
    inputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;
    outputFormat.metadataType = HDR_METADATA_TYPE_NONE;
    outputFormat.colorSpace = DISPLAY_P3;
    outputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
    }
}

/**
 * @tc.number    : COLORSPACE_SUPPORT_004
 * @tc.name      : display p3 to srgb
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_004, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    ImageProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = HDR_METADATA_TYPE_NONE;
    inputFormat.colorSpace = DISPLAY_P3;
    inputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;
    outputFormat.metadataType = HDR_METADATA_TYPE_NONE;
    outputFormat.colorSpace = SRGB;
    outputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
    }
}

/**
 * @tc.number    : COLORSPACE_SUPPORT_005
 * @tc.name      : srgb to hlg rgba
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_005, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    ImageProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = HDR_METADATA_TYPE_NONE;
    inputFormat.colorSpace = SRGB;
    inputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;
    outputFormat.metadataType = HDR_METADATA_TYPE_ALTERNATE;
    outputFormat.colorSpace = BT2020_HLG;
    outputFormat.pixelFormat = PIXEL_FORMAT_RGBA_1010102;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
    }
}

/**
 * @tc.number    : COLORSPACE_SUPPORT_006
 * @tc.name      : srgb to hlg p010
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_006, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    ImageProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = HDR_METADATA_TYPE_NONE;
    inputFormat.colorSpace = SRGB;
    inputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;
    outputFormat.metadataType = HDR_METADATA_TYPE_ALTERNATE;
    outputFormat.colorSpace = BT2020_HLG;
    outputFormat.pixelFormat = PIXEL_FORMAT_YCBCR_P010;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
    }
}


/**
 * @tc.number    : COLORSPACE_SUPPORT_007
 * @tc.name      : p3 to hlg rgba
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_007, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    ImageProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = HDR_METADATA_TYPE_NONE;
    inputFormat.colorSpace = DISPLAY_P3;
    inputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;
    outputFormat.metadataType = HDR_METADATA_TYPE_ALTERNATE;
    outputFormat.colorSpace = BT2020_HLG;
    outputFormat.pixelFormat = PIXEL_FORMAT_RGBA_1010102;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
    }
}

/**
 * @tc.number    : COLORSPACE_SUPPORT_008
 * @tc.name      : p3 to hlg p010
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_008, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    ImageProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = HDR_METADATA_TYPE_NONE;
    inputFormat.colorSpace = DISPLAY_P3;
    inputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;
    outputFormat.metadataType = HDR_METADATA_TYPE_ALTERNATE;
    outputFormat.colorSpace = BT2020_HLG;
    outputFormat.pixelFormat = PIXEL_FORMAT_YCBCR_P010;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
    }
}

/**
 * @tc.number    : COLORSPACE_DECOMPOSE_0010
 * @tc.name      : hlg 10bit rgba to p3 8bit rgba
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_DECOMPOSE_0010, TestSize.Level0)
{
    ImageProcessing_ColorSpaceInfo sourceImageInfo = {HDR_METADATA_TYPE_ALTERNATE,
                                        BT2020_HLG,
                                        PIXEL_FORMAT_RGBA_1010102};
    ImageProcessing_ColorSpaceInfo destinationImageInfo = {HDR_METADATA_TYPE_BASE,
                                            DISPLAY_P3,
                                            PIXEL_FORMAT_RGBA_8888};
    ImageProcessing_ColorSpaceInfo destinationGainmapInfo = {HDR_METADATA_TYPE_GAINMAP,
                                        DISPLAY_P3,
                                        PIXEL_FORMAT_RGBA_8888};
    if (g_suppported) {
        ASSERT_TRUE(OH_ImageProcessing_IsDecompositionSupported(&sourceImageInfo,
            &destinationImageInfo, &destinationGainmapInfo));
    }
}
/**
 * @tc.number    : COLORSPACE_DECOMPOSE_0020
 * @tc.name      : hlg 10bit rgba to srgb 8bit rgba
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_DECOMPOSE_0020, TestSize.Level1)
{
    ImageProcessing_ColorSpaceInfo sourceImageInfo = {HDR_METADATA_TYPE_ALTERNATE,
                                        BT2020_HLG,
                                        PIXEL_FORMAT_RGBA_1010102};
    ImageProcessing_ColorSpaceInfo destinationImageInfo = {HDR_METADATA_TYPE_BASE,
                                            DISPLAY_SRGB,
                                            PIXEL_FORMAT_RGBA_8888};
    ImageProcessing_ColorSpaceInfo destinationGainmapInfo = {HDR_METADATA_TYPE_GAINMAP,
                                        DISPLAY_SRGB,
                                        PIXEL_FORMAT_RGBA_8888};
    if (g_suppported) {
        ASSERT_TRUE(OH_ImageProcessing_IsDecompositionSupported(&sourceImageInfo,
            &destinationImageInfo, &destinationGainmapInfo));
    }
}
/**
 * @tc.number    : COLORSPACE_DECOMPOSE_0030
 * @tc.name      : hlg 10bit nv12 to p3 8bit rgba
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_DECOMPOSE_0030, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo sourceImageInfo = {HDR_METADATA_TYPE_ALTERNATE,
                                        BT2020_HLG,
                                        PIXEL_FORMAT_YCBCR_P010};
    ImageProcessing_ColorSpaceInfo destinationImageInfo = {HDR_METADATA_TYPE_BASE,
                                            DISPLAY_P3,
                                            PIXEL_FORMAT_RGBA_8888};
    ImageProcessing_ColorSpaceInfo destinationGainmapInfo = {HDR_METADATA_TYPE_GAINMAP,
                                        DISPLAY_P3,
                                        PIXEL_FORMAT_RGBA_8888};
    if (g_suppported) {
        ASSERT_TRUE(OH_ImageProcessing_IsDecompositionSupported(&sourceImageInfo,
            &destinationImageInfo, &destinationGainmapInfo));
    }
}

/**
 * @tc.number    : COLORSPACE_DECOMPOSE_0040
 * @tc.name      : hlg 10bit nv12 to hlg 8bit rgba
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_DECOMPOSE_0040, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo sourceImageInfo = {HDR_METADATA_TYPE_ALTERNATE,
                                        BT2020_HLG,
                                        PIXEL_FORMAT_YCBCR_P010};
    ImageProcessing_ColorSpaceInfo destinationImageInfo = {HDR_METADATA_TYPE_BASE,
                                            DISPLAY_SRGB,
                                            PIXEL_FORMAT_RGBA_8888};
    ImageProcessing_ColorSpaceInfo destinationGainmapInfo = {HDR_METADATA_TYPE_GAINMAP,
                                        DISPLAY_SRGB,
                                        PIXEL_FORMAT_RGBA_8888};
    if (g_suppported) {
        ASSERT_TRUE(OH_ImageProcessing_IsDecompositionSupported(&sourceImageInfo,
            &destinationImageInfo, &destinationGainmapInfo));
    }
}

/**
 * @tc.number    : COLORSPACE_DECOMPOSE_0050
 * @tc.name      : hlg 10bit nv21 to P3 8bit rgba
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_DECOMPOSE_0050, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo sourceImageInfo = {HDR_METADATA_TYPE_ALTERNATE,
                                        BT2020_HLG,
                                        PIXEL_FORMAT_YCRCB_P010};
    ImageProcessing_ColorSpaceInfo destinationImageInfo = {HDR_METADATA_TYPE_BASE,
                                            DISPLAY_P3,
                                            PIXEL_FORMAT_RGBA_8888};
    ImageProcessing_ColorSpaceInfo destinationGainmapInfo = {HDR_METADATA_TYPE_GAINMAP,
                                        DISPLAY_P3,
                                        PIXEL_FORMAT_RGBA_8888};
    if (g_suppported) {
        ASSERT_TRUE(OH_ImageProcessing_IsDecompositionSupported(&sourceImageInfo,
            &destinationImageInfo, &destinationGainmapInfo));
    }
}

/**
 * @tc.number    : COLORSPACE_DECOMPOSE_0060
 * @tc.name      : hlg 10bit nv21 to hlg 8bit rgba
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_DECOMPOSE_0060, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo sourceImageInfo = {HDR_METADATA_TYPE_ALTERNATE,
                                        BT2020_HLG,
                                        PIXEL_FORMAT_YCRCB_P010};
    ImageProcessing_ColorSpaceInfo destinationImageInfo = {HDR_METADATA_TYPE_BASE,
                                            DISPLAY_SRGB,
                                            PIXEL_FORMAT_RGBA_8888};
    ImageProcessing_ColorSpaceInfo destinationGainmapInfo = {HDR_METADATA_TYPE_GAINMAP,
                                        DISPLAY_SRGB,
                                        PIXEL_FORMAT_RGBA_8888};
    if (g_suppported) {
        ASSERT_TRUE(OH_ImageProcessing_IsDecompositionSupported(&sourceImageInfo,
            &destinationImageInfo, &destinationGainmapInfo));
    }
}

/**
 * @tc.number    : COLORSPACE_DECOMPOSE_0070
 * @tc.name      : pq 10bit rgba to p3 8bit rgba
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_DECOMPOSE_0070, TestSize.Level0)
{
    ImageProcessing_ColorSpaceInfo sourceImageInfo = {HDR_METADATA_TYPE_ALTERNATE,
                                        BT2020_PQ,
                                        PIXEL_FORMAT_RGBA_1010102};
    ImageProcessing_ColorSpaceInfo destinationImageInfo = {HDR_METADATA_TYPE_BASE,
                                            DISPLAY_P3,
                                            PIXEL_FORMAT_RGBA_8888};
    ImageProcessing_ColorSpaceInfo destinationGainmapInfo = {HDR_METADATA_TYPE_GAINMAP,
                                        DISPLAY_P3,
                                        PIXEL_FORMAT_RGBA_8888};
    if (g_suppported) {
        ASSERT_TRUE(OH_ImageProcessing_IsDecompositionSupported(&sourceImageInfo,
            &destinationImageInfo, &destinationGainmapInfo));
    }
}

/**
 * @tc.number    : COLORSPACE_DECOMPOSE_0080
 * @tc.name      : pq 10bit rgba to srgb 8bit rgba
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_DECOMPOSE_0080, TestSize.Level1)
{
    ImageProcessing_ColorSpaceInfo sourceImageInfo = {HDR_METADATA_TYPE_ALTERNATE,
                                        BT2020_PQ,
                                        PIXEL_FORMAT_RGBA_1010102};
    ImageProcessing_ColorSpaceInfo destinationImageInfo = {HDR_METADATA_TYPE_BASE,
                                            DISPLAY_SRGB,
                                            PIXEL_FORMAT_RGBA_8888};
    ImageProcessing_ColorSpaceInfo destinationGainmapInfo = {HDR_METADATA_TYPE_GAINMAP,
                                        DISPLAY_SRGB,
                                        PIXEL_FORMAT_RGBA_8888};
    if (g_suppported) {
        ASSERT_TRUE(OH_ImageProcessing_IsDecompositionSupported(&sourceImageInfo,
            &destinationImageInfo, &destinationGainmapInfo));
    }
}

/**
 * @tc.number    : COLORSPACE_DECOMPOSE_0090
 * @tc.name      : pq 10bit nv12 to P3 8bit rgba
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_DECOMPOSE_0090, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo sourceImageInfo = {HDR_METADATA_TYPE_ALTERNATE,
                                        BT2020_PQ,
                                        PIXEL_FORMAT_YCBCR_P010};
    ImageProcessing_ColorSpaceInfo destinationImageInfo = {HDR_METADATA_TYPE_BASE,
                                            DISPLAY_P3,
                                            PIXEL_FORMAT_RGBA_8888};
    ImageProcessing_ColorSpaceInfo destinationGainmapInfo = {HDR_METADATA_TYPE_GAINMAP,
                                        DISPLAY_P3,
                                        PIXEL_FORMAT_RGBA_8888};
    if (g_suppported) {
        ASSERT_TRUE(OH_ImageProcessing_IsDecompositionSupported(&sourceImageInfo,
            &destinationImageInfo, &destinationGainmapInfo));
    }
}

/**
 * @tc.number    : COLORSPACE_DECOMPOSE_0100
 * @tc.name      : pq 10bit nv12 to hlg 8bit rgba
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_DECOMPOSE_0100, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo sourceImageInfo = {HDR_METADATA_TYPE_ALTERNATE,
                                                      BT2020_PQ,
                                                      PIXEL_FORMAT_YCBCR_P010};
    ImageProcessing_ColorSpaceInfo destinationImageInfo = {HDR_METADATA_TYPE_BASE,
                                                           DISPLAY_SRGB,
                                                           PIXEL_FORMAT_RGBA_8888};
    ImageProcessing_ColorSpaceInfo destinationGainmapInfo = {HDR_METADATA_TYPE_GAINMAP,
                                                             DISPLAY_SRGB,
                                                             PIXEL_FORMAT_RGBA_8888};
    if (g_suppported) {
        ASSERT_TRUE(OH_ImageProcessing_IsDecompositionSupported(&sourceImageInfo,
            &destinationImageInfo, &destinationGainmapInfo));
    }
}

/**
 * @tc.number    : COLORSPACE_DECOMPOSE_0110
 * @tc.name      : pq 10bit nv21 to p3 8bit rgba
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_DECOMPOSE_0110, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo sourceImageInfo = {HDR_METADATA_TYPE_ALTERNATE,
                                                    BT2020_PQ,
                                                    PIXEL_FORMAT_YCRCB_P010};
    ImageProcessing_ColorSpaceInfo destinationImageInfo = {HDR_METADATA_TYPE_BASE,
                                                        DISPLAY_P3,
                                                        PIXEL_FORMAT_RGBA_8888};
    ImageProcessing_ColorSpaceInfo destinationGainmapInfo = {HDR_METADATA_TYPE_GAINMAP,
                                                            DISPLAY_P3,
                                                            PIXEL_FORMAT_RGBA_8888};
    if (g_suppported) {
        ASSERT_TRUE(OH_ImageProcessing_IsDecompositionSupported(&sourceImageInfo,
            &destinationImageInfo, &destinationGainmapInfo));
    }
}

/**
 * @tc.number    : COLORSPACE_DECOMPOSE_0120
 * @tc.name      : pq 10bit nv21 to hlg 8bit rgba
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_DECOMPOSE_0120, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo sourceImageInfo = {HDR_METADATA_TYPE_ALTERNATE,
                                        BT2020_PQ,
                                        PIXEL_FORMAT_YCRCB_P010};
    ImageProcessing_ColorSpaceInfo destinationImageInfo = {HDR_METADATA_TYPE_BASE,
                                            DISPLAY_SRGB,
                                            PIXEL_FORMAT_RGBA_8888};
    ImageProcessing_ColorSpaceInfo destinationGainmapInfo = {HDR_METADATA_TYPE_GAINMAP,
                                        DISPLAY_SRGB,
                                        PIXEL_FORMAT_RGBA_8888};
    if (g_suppported) {
        ASSERT_TRUE(OH_ImageProcessing_IsDecompositionSupported(&sourceImageInfo,
                                                            &destinationImageInfo, &destinationGainmapInfo));
    }
}

/**
 * @tc.number    : METADATAGENERATE_SUPPORT_001
 * @tc.name      : hlg rgba1010102 metadata generate
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, METADATAGENERATE_SUPPORT_001, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    inputFormat.metadataType = HDR_METADATA_TYPE_ALTERNATE;
    inputFormat.colorSpace = BT2020_HLG;
    inputFormat.pixelFormat = PIXEL_FORMAT_RGBA_1010102;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsMetadataGenerationSupported(&inputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsMetadataGenerationSupported(&inputFormat));
    }
}

/**
 * @tc.number    : METADATAGENERATE_SUPPORT_002
 * @tc.name      : hlg p010 metadata generate
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, METADATAGENERATE_SUPPORT_002, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    inputFormat.metadataType = HDR_METADATA_TYPE_ALTERNATE;
    inputFormat.colorSpace = BT2020_HLG;
    inputFormat.pixelFormat = PIXEL_FORMAT_YCBCR_P010;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsMetadataGenerationSupported(&inputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsMetadataGenerationSupported(&inputFormat));
    }
}

/**
 * @tc.number    : METADATAGENERATE_SUPPORT_003
 * @tc.name      : hlg p010_NV21 metadata generate
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, METADATAGENERATE_SUPPORT_003, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    inputFormat.metadataType = HDR_METADATA_TYPE_ALTERNATE;
    inputFormat.colorSpace = BT2020_HLG;
    inputFormat.pixelFormat = PIXEL_FORMAT_YCRCB_P010;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsMetadataGenerationSupported(&inputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsMetadataGenerationSupported(&inputFormat));
    }
}

/**
 * @tc.number    : METADATAGENERATE_SUPPORT_004
 * @tc.name      : pq rgba1010102 metadata generate
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, METADATAGENERATE_SUPPORT_004, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    inputFormat.metadataType = HDR_METADATA_TYPE_ALTERNATE;
    inputFormat.colorSpace = BT2020_PQ;
    inputFormat.pixelFormat = PIXEL_FORMAT_RGBA_1010102;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsMetadataGenerationSupported(&inputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsMetadataGenerationSupported(&inputFormat));
    }
}

/**
 * @tc.number    : METADATAGENERATE_SUPPORT_005
 * @tc.name      : pq p010 metadata generate
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, METADATAGENERATE_SUPPORT_005, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    inputFormat.metadataType = HDR_METADATA_TYPE_ALTERNATE;
    inputFormat.colorSpace = BT2020_PQ;
    inputFormat.pixelFormat = PIXEL_FORMAT_YCBCR_P010;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsMetadataGenerationSupported(&inputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsMetadataGenerationSupported(&inputFormat));
    }
}

/**
 * @tc.number    : METADATAGENERATE_SUPPORT_006
 * @tc.name      : pq p010_NV21 metadata generate
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, METADATAGENERATE_SUPPORT_006, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    inputFormat.metadataType = HDR_METADATA_TYPE_ALTERNATE;
    inputFormat.colorSpace = BT2020_PQ;
    inputFormat.pixelFormat = PIXEL_FORMAT_YCRCB_P010;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsMetadataGenerationSupported(&inputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsMetadataGenerationSupported(&inputFormat));
    }
}

/**
 * @tc.number    : Composition_SUPPORT_001
 * @tc.name      : srgb + srgb gainmap to PQ P010
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, Composition_SUPPORT_001, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    ImageProcessing_ColorSpaceInfo inputGainmapFormat;
    ImageProcessing_ColorSpaceInfo outputFormat;

    inputFormat.metadataType = HDR_METADATA_TYPE_BASE;
    inputFormat.colorSpace = SRGB;
    inputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;

    inputGainmapFormat.metadataType = HDR_METADATA_TYPE_GAINMAP;
    inputGainmapFormat.colorSpace = SRGB;
    inputGainmapFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;

    outputFormat.metadataType = HDR_METADATA_TYPE_ALTERNATE;
    outputFormat.colorSpace = BT2020_PQ;
    outputFormat.pixelFormat = PIXEL_FORMAT_YCBCR_P010;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsCompositionSupported(&inputFormat, &inputGainmapFormat, &outputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsCompositionSupported(&inputFormat, &inputGainmapFormat, &outputFormat));
    }
}

/**
 * @tc.number    : Composition_SUPPORT_002
 * @tc.name      : p3 + p3 gainmap to PQ P010
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, Composition_SUPPORT_002, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    ImageProcessing_ColorSpaceInfo inputGainmapFormat;
    ImageProcessing_ColorSpaceInfo outputFormat;

    inputFormat.metadataType = HDR_METADATA_TYPE_BASE;
    inputFormat.colorSpace = DISPLAY_P3;
    inputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;

    inputGainmapFormat.metadataType = HDR_METADATA_TYPE_GAINMAP;
    inputGainmapFormat.colorSpace = DISPLAY_P3;
    inputGainmapFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;

    outputFormat.metadataType = HDR_METADATA_TYPE_ALTERNATE;
    outputFormat.colorSpace = BT2020_PQ;
    outputFormat.pixelFormat = PIXEL_FORMAT_YCBCR_P010;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsCompositionSupported(&inputFormat, &inputGainmapFormat, &outputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsCompositionSupported(&inputFormat, &inputGainmapFormat, &outputFormat));
    }
}

/**
 * @tc.number    : Composition_SUPPORT_005
 * @tc.name      : srgb + srgb gainmap to PQ rgba1010102
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, Composition_SUPPORT_005, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    ImageProcessing_ColorSpaceInfo inputGainmapFormat;
    ImageProcessing_ColorSpaceInfo outputFormat;

    inputFormat.metadataType = HDR_METADATA_TYPE_BASE;
    inputFormat.colorSpace = SRGB;
    inputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;

    inputGainmapFormat.metadataType = HDR_METADATA_TYPE_GAINMAP;
    inputGainmapFormat.colorSpace = SRGB;
    inputGainmapFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;

    outputFormat.metadataType = HDR_METADATA_TYPE_ALTERNATE;
    outputFormat.colorSpace = BT2020_PQ;
    outputFormat.pixelFormat = PIXEL_FORMAT_RGBA_1010102;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsCompositionSupported(&inputFormat, &inputGainmapFormat, &outputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsCompositionSupported(&inputFormat, &inputGainmapFormat, &outputFormat));
    }
}

/**
 * @tc.number    : Composition_SUPPORT_006
 * @tc.name      : p3 + p3 gainmap to PQ rgba1010102
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, Composition_SUPPORT_006, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    ImageProcessing_ColorSpaceInfo inputGainmapFormat;
    ImageProcessing_ColorSpaceInfo outputFormat;

    inputFormat.metadataType = HDR_METADATA_TYPE_BASE;
    inputFormat.colorSpace = DISPLAY_P3;
    inputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;

    inputGainmapFormat.metadataType = HDR_METADATA_TYPE_GAINMAP;
    inputGainmapFormat.colorSpace = DISPLAY_P3;
    inputGainmapFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;

    outputFormat.metadataType = HDR_METADATA_TYPE_ALTERNATE;
    outputFormat.colorSpace = BT2020_PQ;
    outputFormat.pixelFormat = PIXEL_FORMAT_RGBA_1010102;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsCompositionSupported(&inputFormat, &inputGainmapFormat, &outputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsCompositionSupported(&inputFormat, &inputGainmapFormat, &outputFormat));
    }
}

/**
 * @tc.number    : Composition_SUPPORT_007
 * @tc.name      : srgb + srgb gainmap to HLG P010
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, Composition_SUPPORT_007, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    ImageProcessing_ColorSpaceInfo inputGainmapFormat;
    ImageProcessing_ColorSpaceInfo outputFormat;

    inputFormat.metadataType = HDR_METADATA_TYPE_BASE;
    inputFormat.colorSpace = SRGB;
    inputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;

    inputGainmapFormat.metadataType = HDR_METADATA_TYPE_GAINMAP;
    inputGainmapFormat.colorSpace = SRGB;
    inputGainmapFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;

    outputFormat.metadataType = HDR_METADATA_TYPE_ALTERNATE;
    outputFormat.colorSpace = BT2020_HLG;
    outputFormat.pixelFormat = PIXEL_FORMAT_YCBCR_P010;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsCompositionSupported(&inputFormat, &inputGainmapFormat, &outputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsCompositionSupported(&inputFormat, &inputGainmapFormat, &outputFormat));
    }
}

/**
 * @tc.number    : Composition_SUPPORT_008
 * @tc.name      : p3 + p3 gainmap to HLG P010
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, Composition_SUPPORT_008, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    ImageProcessing_ColorSpaceInfo inputGainmapFormat;
    ImageProcessing_ColorSpaceInfo outputFormat;

    inputFormat.metadataType = HDR_METADATA_TYPE_BASE;
    inputFormat.colorSpace = DISPLAY_P3;
    inputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;

    inputGainmapFormat.metadataType = HDR_METADATA_TYPE_GAINMAP;
    inputGainmapFormat.colorSpace = DISPLAY_P3;
    inputGainmapFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;

    outputFormat.metadataType = HDR_METADATA_TYPE_ALTERNATE;
    outputFormat.colorSpace = BT2020_HLG;
    outputFormat.pixelFormat = PIXEL_FORMAT_YCBCR_P010;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsCompositionSupported(&inputFormat, &inputGainmapFormat, &outputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsCompositionSupported(&inputFormat, &inputGainmapFormat, &outputFormat));
    }
}

/**
 * @tc.number    : Composition_SUPPORT_011
 * @tc.name      : srgb + srgb gainmap to HLG rgba1010102
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, Composition_SUPPORT_011, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    ImageProcessing_ColorSpaceInfo inputGainmapFormat;
    ImageProcessing_ColorSpaceInfo outputFormat;

    inputFormat.metadataType = HDR_METADATA_TYPE_BASE;
    inputFormat.colorSpace = SRGB;
    inputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;

    inputGainmapFormat.metadataType = HDR_METADATA_TYPE_GAINMAP;
    inputGainmapFormat.colorSpace = SRGB;
    inputGainmapFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;

    outputFormat.metadataType = HDR_METADATA_TYPE_ALTERNATE;
    outputFormat.colorSpace = BT2020_HLG;
    outputFormat.pixelFormat = PIXEL_FORMAT_RGBA_1010102;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsCompositionSupported(&inputFormat, &inputGainmapFormat, &outputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsCompositionSupported(&inputFormat, &inputGainmapFormat, &outputFormat));
    }
}

/**
 * @tc.number    : Composition_SUPPORT_012
 * @tc.name      : p3 + p3 gainmap to HLG rgba1010102
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, Composition_SUPPORT_012, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    ImageProcessing_ColorSpaceInfo inputGainmapFormat;
    ImageProcessing_ColorSpaceInfo outputFormat;

    inputFormat.metadataType = HDR_METADATA_TYPE_BASE;
    inputFormat.colorSpace = DISPLAY_P3;
    inputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;

    inputGainmapFormat.metadataType = HDR_METADATA_TYPE_GAINMAP;
    inputGainmapFormat.colorSpace = DISPLAY_P3;
    inputGainmapFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;

    outputFormat.metadataType = HDR_METADATA_TYPE_ALTERNATE;
    outputFormat.colorSpace = BT2020_HLG;
    outputFormat.pixelFormat = PIXEL_FORMAT_RGBA_1010102;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsCompositionSupported(&inputFormat, &inputGainmapFormat, &outputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsCompositionSupported(&inputFormat, &inputGainmapFormat, &outputFormat));
    }
}

/**
 * @tc.number    : COLORSPACE_SUPPORT_013
 * @tc.name      : srgb to display p3
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_013, TestSize.Level2)
{
    ImageProcessing_ColorSpaceInfo inputFormat;
    ImageProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = HDR_METADATA_TYPE_NONE;
    inputFormat.colorSpace = SRGB;
    inputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;
    outputFormat.metadataType = HDR_METADATA_TYPE_NONE;
    outputFormat.colorSpace = DISPLAY_P3;
    outputFormat.pixelFormat = PIXEL_FORMAT_RGBA_8888;
    if (g_suppported) {
        ASSERT_EQ(true, OH_ImageProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
    } else {
        ASSERT_EQ(false, OH_ImageProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
    }
}
} // namespace