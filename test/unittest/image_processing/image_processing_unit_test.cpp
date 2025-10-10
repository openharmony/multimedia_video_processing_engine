/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "image_processing.h"
#include "image_processing_types.h"
#include "native_avformat.h"
#include "pixelmap_native.h"
#include "image_processing_factory.h"
#include "metadata_generator.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

class ImageProcessingUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ImageProcessingUnitTest::SetUpTestCase(void)
{
    cout << "[SetUpTestCase]: " << endl;
}

void ImageProcessingUnitTest::TearDownTestCase(void)
{
    cout << "[TearDownTestCase]: " << endl;
}

void ImageProcessingUnitTest::SetUp(void)
{
    cout << "[SetUp]: SetUp!!!" << endl;
}

void ImageProcessingUnitTest::TearDown(void)
{
    cout << "[TearDown]: over!!!" << endl;
}

void CreateEmptyPixelmap(OH_PixelmapNative** pixelMap, int32_t width, int32_t height, int format)
{
    OH_Pixelmap_InitializationOptions* options = nullptr;
    (void)OH_PixelmapInitializationOptions_Create(&options);
    (void)OH_PixelmapInitializationOptions_SetWidth(options, width);
    (void)OH_PixelmapInitializationOptions_SetHeight(options, height);
    (void)OH_PixelmapInitializationOptions_SetPixelFormat(options, format);
    (void)OH_PixelmapNative_CreateEmptyPixelmap(options, pixelMap);
}

HWTEST_F(ImageProcessingUnitTest, create_instance_01, TestSize.Level1)
{
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* instance = nullptr;
    ret = OH_ImageProcessing_Create(&instance, IMAGE_PROCESSING_TYPE_DETAIL_ENHANCER);
    EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
}

HWTEST_F(ImageProcessingUnitTest, deinitialize_02, TestSize.Level1)
{
    auto ret = OH_ImageProcessing_DeinitializeEnvironment();
    EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
}

HWTEST_F(ImageProcessingUnitTest, create_instance_02, TestSize.Level1)
{
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* instance = nullptr;
    ret = OH_ImageProcessing_Create(&instance, 11);
    EXPECT_NE(ret, IMAGE_PROCESSING_SUCCESS);
}

HWTEST_F(ImageProcessingUnitTest, destroy_instance_01, TestSize.Level1)
{
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* instance = nullptr;
    ret = OH_ImageProcessing_Create(&instance, IMAGE_PROCESSING_TYPE_DETAIL_ENHANCER);
    ret = OH_ImageProcessing_Destroy(instance);
    EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
}

HWTEST_F(ImageProcessingUnitTest, destroy_instance_02, TestSize.Level1)
{
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* instance = nullptr;
    ret = OH_ImageProcessing_Destroy(instance);
    EXPECT_NE(ret, IMAGE_PROCESSING_SUCCESS);
}

/**
 * @tc.number    : VPE_IMAGE_API_TEST_0140
 * @tc.name      : call OH_ImageProcessing_IsCompositionSupported
 * @tc.desc      : function test
 */
HWTEST(VpeImageApiTest, VPE_IMAGE_API_TEST_0140, TestSize.Level0)
{
    ImageProcessing_ColorSpaceInfo SRC_INFO;
    ImageProcessing_ColorSpaceInfo SRC_GAIN_INFO;
    ImageProcessing_ColorSpaceInfo DST_INFO;
    DST_INFO.colorSpace = 9;
    DST_INFO.metadataType = 3;
    DST_INFO.pixelFormat = 10;
    SRC_INFO.colorSpace = 3;
    SRC_INFO.metadataType = 1;
    SRC_INFO.pixelFormat = 3;
    bool ret = OH_ImageProcessing_IsCompositionSupported(&SRC_INFO, &SRC_GAIN_INFO, &DST_INFO);
    ASSERT_TRUE(ret);
}
HWTEST(VpeImageApiTest, VPE_IMAGE_API_TEST_0141, TestSize.Level0)
{
    ImageProcessing_ColorSpaceInfo SRC_INFO;
    ImageProcessing_ColorSpaceInfo SRC_GAIN_INFO;
    ImageProcessing_ColorSpaceInfo DST_INFO;
    SRC_INFO.colorSpace = 9;
    SRC_INFO.metadataType = 3;
    SRC_INFO.pixelFormat = 10;
    DST_INFO.colorSpace = 3;
    DST_INFO.metadataType = 1;
    DST_INFO.pixelFormat = 3;
    bool ret = OH_ImageProcessing_IsDecompositionSupported(&SRC_INFO, &DST_INFO, &SRC_GAIN_INFO);
    ASSERT_TRUE(ret);
}
HWTEST(VpeImageApiTest, VPE_IMAGE_API_TEST_0142, TestSize.Level0)
{
    ImageProcessing_ColorSpaceInfo SRC_INFO;
    ImageProcessing_ColorSpaceInfo DST_INFO;
    SRC_INFO.colorSpace = 3;
    SRC_INFO.metadataType = 0;
    SRC_INFO.pixelFormat = 3;
    DST_INFO.colorSpace = 4;
    DST_INFO.metadataType = 0;
    DST_INFO.pixelFormat = 3;
    bool ret = OH_ImageProcessing_IsColorSpaceConversionSupported(&SRC_INFO, &DST_INFO);
    ASSERT_TRUE(ret);
}
HWTEST(VpeImageApiTest, VPE_IMAGE_API_TEST_0143, TestSize.Level0)
{
    ImageProcessing_ColorSpaceInfo SRC_INFO;
    SRC_INFO.colorSpace = 9;
    SRC_INFO.metadataType = 3;
    SRC_INFO.pixelFormat = 10;
    bool ret = OH_ImageProcessing_IsMetadataGenerationSupported(&SRC_INFO);
    ASSERT_TRUE(ret);
}
HWTEST_F(ImageProcessingUnitTest, set_parameter_01, TestSize.Level1)
{
    OH_ImageProcessing* instance = nullptr;
    OH_AVFormat* parameter = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_SetParameter(instance, parameter);
    EXPECT_NE(ret, IMAGE_PROCESSING_SUCCESS);
}

HWTEST_F(ImageProcessingUnitTest, set_parameter_02, TestSize.Level1)
{
    OH_ImageProcessing* instance = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&instance, IMAGE_PROCESSING_TYPE_DETAIL_ENHANCER);
    OH_AVFormat* parameter = nullptr;
    ret = OH_ImageProcessing_SetParameter(instance, parameter);
    EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER);
}

HWTEST_F(ImageProcessingUnitTest, set_parameter_03, TestSize.Level1)
{
    OH_ImageProcessing* instance = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&instance, IMAGE_PROCESSING_TYPE_DETAIL_ENHANCER);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, IMAGE_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL, 10);
    ret = OH_ImageProcessing_SetParameter(instance, parameter);
    EXPECT_NE(ret, IMAGE_PROCESSING_SUCCESS);
}

HWTEST_F(ImageProcessingUnitTest, get_parameter_01, TestSize.Level1)
{
    OH_ImageProcessing* instance = nullptr;
    OH_AVFormat* parameter = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_GetParameter(instance, parameter);
    EXPECT_NE(ret, IMAGE_PROCESSING_SUCCESS);
}

HWTEST_F(ImageProcessingUnitTest, get_parameter_02, TestSize.Level1)
{
    OH_ImageProcessing* instance = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&instance, IMAGE_PROCESSING_TYPE_DETAIL_ENHANCER);
    OH_AVFormat* parameter = nullptr;
    ret = OH_ImageProcessing_GetParameter(instance, parameter);
    EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_INVALID_PARAMETER);
}

HWTEST_F(ImageProcessingUnitTest, get_parameter_03, TestSize.Level1)
{
    OH_ImageProcessing* instance = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&instance, IMAGE_PROCESSING_TYPE_DETAIL_ENHANCER);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, IMAGE_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL,
        IMAGE_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH);
    ret = OH_ImageProcessing_SetParameter(instance, parameter);
    ret = OH_ImageProcessing_GetParameter(instance, parameter);
    EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
}

HWTEST_F(ImageProcessingUnitTest, get_parameter_04, TestSize.Level1)
{
    OH_ImageProcessing* instance = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&instance, IMAGE_PROCESSING_TYPE_DETAIL_ENHANCER);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    ret = OH_ImageProcessing_GetParameter(instance, parameter);
    EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
}

HWTEST_F(ImageProcessingUnitTest, process_01, TestSize.Level1)
{
    OH_ImageProcessing* instance = nullptr;
    OH_PixelmapNative* srcImg = nullptr;
    OH_PixelmapNative* dstImg = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_EnhanceDetail(instance, srcImg, dstImg);
    EXPECT_NE(ret, IMAGE_PROCESSING_SUCCESS);
}

HWTEST_F(ImageProcessingUnitTest, process_02, TestSize.Level1)
{
    OH_ImageProcessing* instance = nullptr;
    OH_PixelmapNative* srcImg = nullptr;
    OH_PixelmapNative* dstImg = nullptr;
    CreateEmptyPixelmap(&srcImg, 720, 960, 4);
    CreateEmptyPixelmap(&dstImg, 2880, 3840, 4);
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_EnhanceDetail(instance, srcImg, dstImg);
    EXPECT_NE(ret, IMAGE_PROCESSING_SUCCESS);
}

HWTEST_F(ImageProcessingUnitTest, process_03, TestSize.Level1)
{
    OH_ImageProcessing* instance = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&instance, IMAGE_PROCESSING_TYPE_DETAIL_ENHANCER);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, IMAGE_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL,
        IMAGE_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH);
    ret = OH_ImageProcessing_SetParameter(instance, parameter);
    OH_PixelmapNative* srcImg = nullptr;
    OH_PixelmapNative* dstImg = nullptr;
    CreateEmptyPixelmap(&srcImg, 720, 960, 4);
    CreateEmptyPixelmap(&dstImg, 2880, 3840, 4);
    ret = OH_ImageProcessing_EnhanceDetail(instance, srcImg, dstImg);
    EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
}

HWTEST_F(ImageProcessingUnitTest, process_04, TestSize.Level1)
{
    OH_ImageProcessing* instance = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&instance, IMAGE_PROCESSING_TYPE_DETAIL_ENHANCER);
    OH_PixelmapNative* srcImg = nullptr;
    OH_PixelmapNative* dstImg = nullptr;
    CreateEmptyPixelmap(&srcImg, 720, 960, 4);
    CreateEmptyPixelmap(&dstImg, 2880, 3840, 4);
    ret = OH_ImageProcessing_EnhanceDetail(instance, srcImg, dstImg);
    EXPECT_NE(ret, IMAGE_PROCESSING_SUCCESS);
}

HWTEST_F(ImageProcessingUnitTest, process_05, TestSize.Level1)
{
    OH_ImageProcessing* instance = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&instance, IMAGE_PROCESSING_TYPE_DETAIL_ENHANCER);
    OH_PixelmapNative* srcImg = nullptr;
    OH_PixelmapNative* dstImg = nullptr;
    CreateEmptyPixelmap(&srcImg, 720, 960, 9);
    CreateEmptyPixelmap(&dstImg, 2880, 3840, 9);
    ret = OH_ImageProcessing_EnhanceDetail(instance, srcImg, dstImg);
    EXPECT_NE(ret, IMAGE_PROCESSING_SUCCESS);
}

HWTEST_F(ImageProcessingUnitTest, initialize, TestSize.Level1)
{
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_InitializeEnvironment();
    EXPECT_EQ(ret, IMAGE_PROCESSING_ERROR_OPERATION_NOT_PERMITTED);
}

HWTEST_F(ImageProcessingUnitTest, deinitialize, TestSize.Level1)
{
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_InitializeEnvironment();
    ret = OH_ImageProcessing_DeinitializeEnvironment();
    EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
}

HWTEST_F(ImageProcessingUnitTest, process_07, TestSize.Level1)
{
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* instance = nullptr;
    ret = OH_ImageProcessing_Create(&instance, IMAGE_PROCESSING_TYPE_COMPOSITION);
    OH_PixelmapNative* srcImg = nullptr;
    OH_PixelmapNative* dstImg = nullptr;
    OH_PixelmapNative* gainmap = nullptr;
    CreateEmptyPixelmap(&srcImg, 3840, 2160, 3);
    CreateEmptyPixelmap(&dstImg, 3840, 2160, 10);
    CreateEmptyPixelmap(&gainmap, 1920, 1080, 3);
    if (dstImg == nullptr) {
        return;
    }
    ret = OH_ImageProcessing_Compose(instance, srcImg, gainmap, dstImg);
    EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
    OH_ImageProcessing_DeinitializeEnvironment();
}
HWTEST_F(ImageProcessingUnitTest, process_08, TestSize.Level1)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* instance = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&instance, IMAGE_PROCESSING_TYPE_DECOMPOSITION);
    OH_PixelmapNative* srcImg = nullptr;
    OH_PixelmapNative* dstImg = nullptr;
    OH_PixelmapNative* gainmap = nullptr;
    CreateEmptyPixelmap(&srcImg, 3840, 2160, 10);
    CreateEmptyPixelmap(&dstImg, 3840, 2160, 3);
    CreateEmptyPixelmap(&gainmap, 1920, 1080, 3);
    if (srcImg == nullptr) {
        return;
    }
    ret = OH_ImageProcessing_Decompose(instance, srcImg, dstImg, gainmap);
    EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
    OH_ImageProcessing_DeinitializeEnvironment();
}
HWTEST_F(ImageProcessingUnitTest, process_09, TestSize.Level1)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* instance = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&instance, IMAGE_PROCESSING_TYPE_COLOR_SPACE_CONVERSION);
    OH_PixelmapNative* srcImg = nullptr;
    OH_PixelmapNative* dstImg = nullptr;
    CreateEmptyPixelmap(&srcImg, 3840, 2160, 3);
    CreateEmptyPixelmap(&dstImg, 3840, 2160, 3);
    ret = OH_ImageProcessing_ConvertColorSpace(instance, srcImg, dstImg);
    if (ret != 0) {
        return;
    }
    EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
    OH_ImageProcessing_DeinitializeEnvironment();
}
HWTEST_F(ImageProcessingUnitTest, process_10, TestSize.Level1)
{
    OH_ImageProcessing_InitializeEnvironment();
    OH_ImageProcessing* instance = nullptr;
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&instance, IMAGE_PROCESSING_TYPE_METADATA_GENERATION);
    OH_PixelmapNative* srcImg = nullptr;
    CreateEmptyPixelmap(&srcImg, 3840, 2160, 10);
    if (srcImg == nullptr) {
        return;
    }
    ret = OH_ImageProcessing_GenerateMetadata(instance, srcImg);
    EXPECT_EQ(ret, IMAGE_PROCESSING_SUCCESS);
    OH_ImageProcessing_DeinitializeEnvironment();
}

HWTEST_F(ImageProcessingUnitTest, apitest_01, TestSize.Level1)
{
    int32_t instanceSrId = -1;
    int32_t* tmp = &instanceSrId;
    auto ret = MetadataGeneratorCreate(tmp);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
    EXPECT_NE(instanceSrId, (int32_t)(-1));
    auto input = CreateSurfaceBuffer(GRAPHIC_PIXEL_FMT_RGBA_1010102, 1920, 1080);
    EXPECT_NE(input, nullptr);
    OHNativeWindowBuffer* srTmp = OH_NativeWindow_CreateNativeWindowBufferFromSurfaceBuffer(&input);
    EXPECT_NE(srTmp, nullptr);
    OH_NativeBuffer* inputImageNativeBuffer = nullptr;
    OH_NativeBuffer_FromNativeWindowBuffer(srTmp, &inputImageNativeBuffer);
    EXPECT_NE(inputImageNativeBuffer, nullptr);
    uint8_t metadataType = CM_IMAGE_HDR_VIVID_DUAL;
    OH_NativeBuffer_SetMetadataValue(inputImageNativeBuffer, OH_HDR_METADATA_TYPE, sizeof(uint8_t), &metadataType);
    OH_NativeBuffer_ColorSpace colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    OH_NativeBuffer_SetColorSpace(inputImageNativeBuffer, colorSpace);
    OHNativeWindowBuffer* srIn = OH_NativeWindow_CreateNativeWindowBufferFromNativeBuffer(inputImageNativeBuffer);
    EXPECT_NE(srIn, nullptr);
    ret = MetadataGeneratorProcessImage(instanceSrId, srIn);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = MetadataGeneratorDestroy(tmp);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(ImageProcessingUnitTest, apitest_02, TestSize.Level1)
{
    int32_t instanceSrId = -1;
    int32_t* tmp = &instanceSrId;
    auto ret = MetadataGeneratorCreate(tmp);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
    EXPECT_NE(instanceSrId, (int32_t)(-1));
    auto input = CreateSurfaceBuffer(GRAPHIC_PIXEL_FMT_RGBA_1010102, 1920, 1080);
    EXPECT_NE(input, nullptr);
    OHNativeWindowBuffer* srTmp = OH_NativeWindow_CreateNativeWindowBufferFromSurfaceBuffer(&input);
    EXPECT_NE(srTmp, nullptr);
    OH_NativeBuffer* inputVideoNativeBuffer = nullptr;
    OH_NativeBuffer_FromNativeWindowBuffer(srTmp, &inputVideoNativeBuffer);
    EXPECT_NE(inputVideoNativeBuffer, nullptr);
    uint8_t metadataType = OH_VIDEO_HDR_HLG;
    OH_NativeBuffer_SetMetadataValue(inputVideoNativeBuffer, OH_HDR_METADATA_TYPE, sizeof(uint8_t), &metadataType);
    OH_NativeBuffer_ColorSpace colorSpace = OH_COLORSPACE_BT2020_HLG_LIMIT;
    OH_NativeBuffer_SetColorSpace(inputVideoNativeBuffer, colorSpace);
    OHNativeWindowBuffer* srIn = OH_NativeWindow_CreateNativeWindowBufferFromNativeBuffer(inputVideoNativeBuffer);
    EXPECT_NE(srIn, nullptr);
    ret = MetadataGeneratorProcessVideo(instanceSrId, srIn);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = MetadataGeneratorDestroy(tmp);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
