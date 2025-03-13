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
#include "gtest/gtest.h"
#include "video_processing.h"
#include "native_buffer.h"

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
    OH_VideoProcessing_InitializeEnvironment();
}
void VpeVideoCapTest::TearDownTestCase()
{
    OH_VideoProcessing_DeinitializeEnvironment();
}
void VpeVideoCapTest::SetUp() {}
void VpeVideoCapTest::TearDown() {}
}

namespace {
/**
 * @tc.number    : COLORSPACE_SUPPORT_001
 * @tc.name      : HDR10 to HLG
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_001, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_HDR10;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_HLG;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(true, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_002
 * @tc.name      : HDR10 to HLG
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_002, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_HDR10;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_HLG;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_FULL;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_003
 * @tc.name      : HDR10 to HLG
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_003, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_HDR10;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_004
 * @tc.name      : HDR10 to HLG
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_004, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_HDR10;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_HLG;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_RGB_565;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_005
 * @tc.name      : HDR10 to HLG
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_005, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_HDR10;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_FULL;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_HLG;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_006
 * @tc.name      : HDR10 to HLG
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_006, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_HDR10;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_FULL;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_HLG;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_FULL;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_007
 * @tc.name      : HDR10 to HLG
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_007, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_HDR10;
    inputFormat.colorSpace = OH_COLORSPACE_BT709_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_HLG;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_008
 * @tc.name      : HDR10 to HLG
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_008, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_HDR10;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_P;
    outputFormat.metadataType = OH_VIDEO_HDR_HLG;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_009
 * @tc.name      : HDR vivid(PQ) to HLG
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_009, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_HLG;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(true, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0010
 * @tc.name      : HDR vivid(PQ) to HLG
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0010, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_HLG;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_FULL;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0011
 * @tc.name      : HDR vivid(PQ) to HLG
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0011, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_HDR10;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0012
 * @tc.name      : HDR vivid(PQ) to HLG
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0012, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_FULL;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_HLG;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0013
 * @tc.name      : HDR vivid(PQ) to HLG
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0013, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_P;
    outputFormat.metadataType = OH_VIDEO_HDR_HLG;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0014
 * @tc.name      : HDR vivid(HLG) to HDR vivid(PQ)
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0014, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(true, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0015
 * @tc.name      : HDR vivid(HLG) to HDR vivid(PQ)
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0015, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_FULL;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_FULL;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0016
 * @tc.name      : HDR vivid(HLG) to HDR vivid(PQ)
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0016, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0017
 * @tc.name      : HDR vivid(HLG) to HDR vivid(PQ)
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0017, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0018
 * @tc.name      : HLG to HDR10/HDR vivid
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0018, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_HLG;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_HDR10;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0019
 * @tc.name      : HLG to HDR10/HDR vivid
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0019, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_HLG;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0020
 * @tc.name      : HLG to HDR10/HDR vivid
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0020, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_HLG;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0021
 * @tc.name      : HLG to HDR10/HDR vivid
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0021, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_HLG;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_FULL;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_HDR10;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0022
 * @tc.name      : HDR2SDR
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0022, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;

    outputFormat.colorSpace = OH_COLORSPACE_BT709_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(true, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0023
 * @tc.name      : HDR2SDR
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0023, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;

    outputFormat.colorSpace = OH_COLORSPACE_BT709_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_P;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0024
 * @tc.name      : HDR2SDR
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0024, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;

    outputFormat.colorSpace = OH_COLORSPACE_BT601_EBU_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0025
 * @tc.name      : HDR2SDR
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0025, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_HDR10;
    outputFormat.colorSpace = OH_COLORSPACE_BT709_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0026
 * @tc.name      : HDR2SDR
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0026, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;

    outputFormat.colorSpace = OH_COLORSPACE_BT709_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(true, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0027
 * @tc.name      : HDR2SDR
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0027, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;

    outputFormat.colorSpace = OH_COLORSPACE_BT709_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_P;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0028
 * @tc.name      : HDR2SDR
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0028, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;

    outputFormat.colorSpace = OH_COLORSPACE_BT601_EBU_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0029
 * @tc.name      : HDR2SDR
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0029, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.metadataType = OH_VIDEO_HDR_VIVID;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.metadataType = OH_VIDEO_HDR_HDR10;
    outputFormat.colorSpace = OH_COLORSPACE_BT709_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0030
 * @tc.name      : SDR2SDR
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0030, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;

    inputFormat.colorSpace = OH_COLORSPACE_BT601_EBU_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;

    outputFormat.colorSpace = OH_COLORSPACE_BT709_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(true, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0031
 * @tc.name      : SDR2SDR
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0031, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;

    inputFormat.colorSpace = OH_COLORSPACE_BT601_EBU_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;

    outputFormat.colorSpace = OH_COLORSPACE_BT601_SMPTE_C_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0032
 * @tc.name      : SDR2SDR
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0032, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;

    inputFormat.colorSpace = OH_COLORSPACE_BT601_SMPTE_C_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;

    outputFormat.colorSpace = OH_COLORSPACE_BT709_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(true, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0033
 * @tc.name      : SDR2SDR
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0033, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;

    inputFormat.colorSpace = OH_COLORSPACE_BT601_SMPTE_C_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;

    outputFormat.colorSpace = OH_COLORSPACE_BT601_EBU_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0034
 * @tc.name      : SDR2SDR
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0034, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;

    inputFormat.colorSpace = OH_COLORSPACE_BT601_EBU_FULL;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;

    outputFormat.colorSpace = OH_COLORSPACE_BT709_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0035
 * @tc.name      : SDR2SDR
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0035, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.colorSpace = OH_COLORSPACE_BT709_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.colorSpace = OH_COLORSPACE_BT601_EBU_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0036
 * @tc.name      : SDR2SDR
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0036, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.colorSpace = OH_COLORSPACE_BT601_SMPTE_C_FULL;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.colorSpace = OH_COLORSPACE_BT709_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
/**
 * @tc.number    : COLORSPACE_SUPPORT_0037
 * @tc.name      : 异常组合
 * @tc.desc      : api test
 */
HWTEST_F(VpeVideoCapTest, COLORSPACE_SUPPORT_0037, TestSize.Level2)
{
    VideoProcessing_ColorSpaceInfo inputFormat;
    VideoProcessing_ColorSpaceInfo outputFormat;
    inputFormat.colorSpace = OH_COLORSPACE_BT2020_PQ_LIMIT;
    inputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    outputFormat.colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    outputFormat.pixelFormat = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;
    if (!access("/system/lib64/ndk/libvideo_processing_capi_impl.so", 0)) {
        if (!access("/system/lib64/", 0)) {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        } else {
            ASSERT_EQ(false, OH_VideoProcessing_IsColorSpaceConversionSupported(&inputFormat, &outputFormat));
        }
    }
}
}
} // namespace