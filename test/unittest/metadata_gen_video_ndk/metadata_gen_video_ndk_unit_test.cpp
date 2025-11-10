/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <unistd.h>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include "securec.h"
#include <sys/time.h>

#include "native_avformat.h"
#include "native_window.h"
#include "surface/window.h"
#include "external_window.h"
#include "v1_0/cm_color_space.h"

#include "video_processing.h"
#include "video_processing_types.h"
#include "metadata_generator_video.h"
#include "algorithm_common.h"
#include "algorithm_common.h"

constexpr uint32_t DEFAULT_WIDTH = 1920;
constexpr uint32_t DEFAULT_HEIGHT = 1080;
constexpr uint32_t DEFAULT_BYTE = 32;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::Media;
using namespace std;
using namespace OHOS::Media::VideoProcessingEngine;
using namespace OHOS::HDI::Display::Graphic::Common::V1_0;
namespace {

class MetadataGeneratorVideoNdkImplUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp()
    {
        requestCfg_.usage = BUFFER_USAGE_CPU_READ | BUFFER_USAGE_CPU_WRITE
        | BUFFER_USAGE_HW_RENDER | BUFFER_USAGE_HW_TEXTURE | BUFFER_USAGE_MEM_MMZ_CACHE;
        requestCfg_.width = DEFAULT_WIDTH;
        requestCfg_.height = DEFAULT_HEIGHT;
        requestCfg_.timeout = 0;
        requestCfg_.strideAlignment = DEFAULT_BYTE;
        flushCfg_.damage.x = 0;
        flushCfg_.damage.y = 0;
        flushCfg_.damage.w = DEFAULT_WIDTH;
        flushCfg_.damage.h = DEFAULT_HEIGHT;
        flushCfg_.timestamp = 0;
    };
    void TearDown()
    {
    };

    BufferFlushConfig flushCfg_{};
    BufferRequestConfig requestCfg_{};
    int32_t fence_ = -1;
    CM_ColorSpaceType inColspc_ = CM_BT2020_PQ_LIMIT;
    CM_HDR_Metadata_Type inMetaType_ = CM_VIDEO_HDR_VIVID;
    GraphicPixelFormat surfacePixelFmt_ = GRAPHIC_PIXEL_FMT_YCBCR_P010;

    void SetMeatadata(sptr<SurfaceBuffer> &buffer, uint32_t value);
    void SetMeatadata(sptr<SurfaceBuffer> &buffer, CM_ColorSpaceInfo &colorspaceInfo);
    VideoProcessing_ErrorCode ProcessContrastStyle();
    VideoProcessing_ErrorCode ProcessBrightStyle();
};

void OnError(OH_VideoProcessing *handle, VideoProcessing_ErrorCode errorCode, void* userData)
{
    (void)handle;
    (void)errorCode;
    (void)userData;
}
void OnState(OH_VideoProcessing *handle, VideoProcessing_State state, void* userData)
{
    (void)handle;
    (void)state;
    (void)userData;
}
void OnNewOutputBuffer(OH_VideoProcessing *handle, uint32_t index, void* userData)
{
    (void)handle;
    (void)index;
    (void)userData;
}

void MetadataGeneratorVideoNdkImplUnitTest::SetMeatadata(sptr<SurfaceBuffer> &buffer, uint32_t value)
{
    std::vector<uint8_t> metadata;
    metadata.resize(sizeof(value));
    (void)memcpy_s(metadata.data(), metadata.size(), &value, sizeof(value));
    uint32_t err = buffer->SetMetadata(ATTRKEY_HDR_METADATA_TYPE, metadata);
    if (err != 0) {
        printf("Buffer set metadata info, ret: %d\n", err);
    }
}

void MetadataGeneratorVideoNdkImplUnitTest::SetMeatadata(sptr<SurfaceBuffer> &buffer,
    CM_ColorSpaceInfo &colorspaceInfo)
{
    std::vector<uint8_t> metadata;
    metadata.resize(sizeof(CM_ColorSpaceInfo));
    (void)memcpy_s(metadata.data(), metadata.size(), &colorspaceInfo, sizeof(CM_ColorSpaceInfo));
    uint32_t err = buffer->SetMetadata(ATTRKEY_COLORSPACE_INFO, metadata);
    if (err != 0) {
        printf("Buffer set colorspace info, ret: %d\n", err);
    }
}

VideoProcessing_ErrorCode MetadataGeneratorVideoNdkImplUnitTest::ProcessContrastStyle()
{
    VideoProcessing_ErrorCode ret = VIDEO_PROCESSING_SUCCESS;
    sptr<SurfaceBuffer> buffer;
    CM_ColorSpaceInfo inColspcInfo = {
        static_cast<CM_ColorPrimaries>((inColspc_ & COLORPRIMARIES_MASK) >> COLORPRIMARIES_OFFSET),
        static_cast<CM_TransFunc>((inColspc_ & TRANSFUNC_MASK) >> TRANSFUNC_OFFSET),
        static_cast<CM_Matrix>((inColspc_ & MATRIX_MASK) >> MATRIX_OFFSET),
        static_cast<CM_Range>((inColspc_ & RANGE_MASK) >> RANGE_OFFSET)
    };
    ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x2;
    OH_VideoProcessing* instance = nullptr;
    OH_VideoProcessing* instance2 = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    ret = OH_VideoProcessing_Create(&instance2, createType);
    OH_AVFormat* paramSetted = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(paramSetted, VIDEO_METADATA_GENERATOR_STYLE_CONTROL,
        VIDEO_METADATA_GENERATOR_CONTRAST_MODE);
    ret = OH_VideoProcessing_SetParameter(instance, paramSetted);
    VideoProcessing_Callback* callback = nullptr;
    ret = OH_VideoProcessingCallback_Create(&callback);
    ret = OH_VideoProcessingCallback_BindOnError(callback, OnError);
    ret = OH_VideoProcessingCallback_BindOnState(callback, OnState);
    ret = OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, OnNewOutputBuffer);
    OHNativeWindow* window = nullptr;
    OHNativeWindow* window2 = nullptr;
    ret = OH_VideoProcessing_GetSurface(instance, &window);
    ret = OH_VideoProcessing_GetSurface(instance2, &window2);
    auto userData = VIDEO_PROCESSING_STATE_STOPPED;
    ret = OH_VideoProcessing_RegisterCallback(instance, callback, &userData);
    ret = OH_VideoProcessing_SetSurface(instance, window2);
    ret = OH_VideoProcessing_Start(instance);
    if (ret != VIDEO_PROCESSING_SUCCESS) {
        return ret;
    }
    int videoSurfaceBuffNum = 1;
    int sleepTime = 2;
    for (int i = 0; i < videoSurfaceBuffNum; i++) {
        window->surface->RequestBuffer(buffer, fence_, requestCfg_);
        SetMeatadata(buffer, inColspcInfo);
        SetMeatadata(buffer, (uint32_t)inMetaType_);
        window->surface->FlushBuffer(buffer, fence_, flushCfg_);
        OH_VideoProcessing_SetSurface(instance, window2);
        sleep(sleepTime);
    }
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_Destroy(instance2);
    OH_VideoProcessing_DeinitializeEnvironment();
    return ret;
}

VideoProcessing_ErrorCode MetadataGeneratorVideoNdkImplUnitTest::ProcessBrightStyle()
{
    VideoProcessing_ErrorCode ret = VIDEO_PROCESSING_SUCCESS;
    sptr<SurfaceBuffer> buffer;
    CM_ColorSpaceInfo inColspcInfo = {
        static_cast<CM_ColorPrimaries>((inColspc_ & COLORPRIMARIES_MASK) >> COLORPRIMARIES_OFFSET),
        static_cast<CM_TransFunc>((inColspc_ & TRANSFUNC_MASK) >> TRANSFUNC_OFFSET),
        static_cast<CM_Matrix>((inColspc_ & MATRIX_MASK) >> MATRIX_OFFSET),
        static_cast<CM_Range>((inColspc_ & RANGE_MASK) >> RANGE_OFFSET)
    };
    ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x2;
    OH_VideoProcessing* instance = nullptr;
    OH_VideoProcessing* instance2 = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    ret = OH_VideoProcessing_Create(&instance2, createType);
    OH_AVFormat* paramSetted = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(paramSetted, VIDEO_METADATA_GENERATOR_STYLE_CONTROL,
        VIDEO_METADATA_GENERATOR_BRIGHT_MODE);
    ret = OH_VideoProcessing_SetParameter(instance, paramSetted);
    VideoProcessing_Callback* callback = nullptr;
    ret = OH_VideoProcessingCallback_Create(&callback);
    ret = OH_VideoProcessingCallback_BindOnError(callback, OnError);
    ret = OH_VideoProcessingCallback_BindOnState(callback, OnState);
    ret = OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, OnNewOutputBuffer);
    OHNativeWindow* window = nullptr;
    OHNativeWindow* window2 = nullptr;
    ret = OH_VideoProcessing_GetSurface(instance, &window);
    ret = OH_VideoProcessing_GetSurface(instance2, &window2);
    auto userData = VIDEO_PROCESSING_STATE_STOPPED;
    ret = OH_VideoProcessing_RegisterCallback(instance, callback, &userData);
    ret = OH_VideoProcessing_SetSurface(instance, window2);
    ret = OH_VideoProcessing_Start(instance);
    if (ret != VIDEO_PROCESSING_SUCCESS) {
        return ret;
    }
    int videoSurfaceBuffNum = 1;
    int sleepTime = 2;
    for (int i = 0; i < videoSurfaceBuffNum; i++) {
        window->surface->RequestBuffer(buffer, fence_, requestCfg_);
        SetMeatadata(buffer, inColspcInfo);
        SetMeatadata(buffer, (uint32_t)inMetaType_);
        window->surface->FlushBuffer(buffer, fence_, flushCfg_);
        OH_VideoProcessing_SetSurface(instance, window2);
        sleep(sleepTime);
    }
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_Destroy(instance2);
    OH_VideoProcessing_DeinitializeEnvironment();
    return ret;
}

HWTEST_F(MetadataGeneratorVideoNdkImplUnitTest, testVideoProcess_testFunSupportedMetadata, TestSize.Level1)
{
    bool resultSupported = false;
    resultSupported = OH_VideoProcessing_IsColorSpaceConversionSupported(nullptr, nullptr);
    EXPECT_EQ(resultSupported, false);
    resultSupported = OH_VideoProcessing_IsMetadataGenerationSupported(nullptr);
    EXPECT_EQ(resultSupported, false);
    const int formatListNum = 3;
    int formatListHDRNative[formatListNum] = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, NATIVEBUFFER_PIXEL_FMT_YCRCB_P010,
        NATIVEBUFFER_PIXEL_FMT_RGBA_1010102};
    const int colorMetaMetaListNum = 4;
    const int colorMetaParaNum = 2;
    const int colorMetaParaInMetaNum = 0;
    const int colorMetaParaInColorNum = 1;
    int colorMetaListSdr2Sdr[colorMetaMetaListNum][colorMetaParaNum] = {
        {OH_VIDEO_HDR_HDR10, OH_COLORSPACE_BT2020_PQ_LIMIT},
        {OH_VIDEO_HDR_VIVID, OH_COLORSPACE_BT2020_PQ_LIMIT},
        {OH_VIDEO_HDR_HLG, OH_COLORSPACE_BT2020_HLG_LIMIT},
        {OH_VIDEO_HDR_VIVID, OH_COLORSPACE_BT2020_HLG_LIMIT}
    };
    VideoProcessing_ColorSpaceInfo sourceVideoInfo = {-1, -1, -1};
    for (int n = 0; n < colorMetaMetaListNum; n++) {
        sourceVideoInfo.metadataType = static_cast<int32_t>(colorMetaListSdr2Sdr[n][colorMetaParaInMetaNum]);
        sourceVideoInfo.colorSpace = static_cast<int32_t>(colorMetaListSdr2Sdr[n][colorMetaParaInColorNum]);
        for (int i = 0; i < formatListNum; i++) {
            sourceVideoInfo.pixelFormat = static_cast<int32_t>(formatListHDRNative[i]);
            resultSupported = OH_VideoProcessing_IsMetadataGenerationSupported(&sourceVideoInfo);
            EXPECT_EQ(resultSupported, true);
        }
    }
    sourceVideoInfo.metadataType = static_cast<int32_t>(OH_VIDEO_HDR_HLG);
    sourceVideoInfo.colorSpace = static_cast<int32_t>(OH_COLORSPACE_BT2020_PQ_LIMIT);
    sourceVideoInfo.pixelFormat = static_cast<int32_t>(NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP);
    resultSupported = OH_VideoProcessing_IsMetadataGenerationSupported(&sourceVideoInfo);
    EXPECT_EQ(resultSupported, false);
}

HWTEST_F(MetadataGeneratorVideoNdkImplUnitTest, testVideoProcess_testFun, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    int createType = 0x2;
    OH_VideoProcessing* instance = nullptr;
    OH_VideoProcessing* instance2 = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    ret = OH_VideoProcessing_Create(&instance2, createType);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    VideoProcessing_Callback* callback = nullptr;
    ret = OH_VideoProcessingCallback_Create(&callback);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessingCallback_BindOnError(callback, OnError);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessingCallback_BindOnState(callback, OnState);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, OnNewOutputBuffer);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    OHNativeWindow* window = nullptr;
    OHNativeWindow* window2 = nullptr;
    ret = OH_VideoProcessing_GetSurface(instance, &window);
    ret = OH_VideoProcessing_GetSurface(instance2, &window2);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    auto userData = VIDEO_PROCESSING_STATE_STOPPED;
    ret = OH_VideoProcessing_RegisterCallback(instance, callback, &userData);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    ret = OH_VideoProcessing_SetParameter(instance, parameter);
    EXPECT_EQ(ret, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER);
    ret = OH_VideoProcessing_GetParameter(instance, parameter);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessing_SetSurface(instance, window2);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    for (int i = 0; i < 2; i++) {
        ret = OH_VideoProcessing_Start(instance);
        EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
        ret = OH_VideoProcessing_Stop(instance);
        EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    }
    ret = OH_VideoProcessing_RenderOutputBuffer(instance, 0);
    EXPECT_EQ(ret, VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED);
    ret = OH_VideoProcessingCallback_Destroy(callback);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessing_Destroy(instance);
    ret = OH_VideoProcessing_Destroy(instance2);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessing_DeinitializeEnvironment();
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
}

// set parameter but param not value
HWTEST_F(MetadataGeneratorVideoNdkImplUnitTest, set_parameter_01, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x2;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    ret = OH_VideoProcessing_SetParameter(instance, parameter);
    EXPECT_EQ(ret, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// Set param nullptr
HWTEST_F(MetadataGeneratorVideoNdkImplUnitTest, set_parameter_02, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x2;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameter = nullptr;
    ret = OH_VideoProcessing_SetParameter(instance, parameter);
    EXPECT_EQ(ret, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// set normal
HWTEST_F(MetadataGeneratorVideoNdkImplUnitTest, set_parameter_03, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x2;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, VIDEO_METADATA_GENERATOR_STYLE_CONTROL,
        VIDEO_METADATA_GENERATOR_BRIGHT_MODE);
    ret = OH_VideoProcessing_SetParameter(instance, parameter);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// set normal
HWTEST_F(MetadataGeneratorVideoNdkImplUnitTest, set_parameter_04, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x2;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, VIDEO_METADATA_GENERATOR_STYLE_CONTROL,
        VIDEO_METADATA_GENERATOR_CONTRAST_MODE);
    ret = OH_VideoProcessing_SetParameter(instance, parameter);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// set invalid value
HWTEST_F(MetadataGeneratorVideoNdkImplUnitTest, set_parameter_05, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x2;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, VIDEO_METADATA_GENERATOR_STYLE_CONTROL, -1);
    ret = OH_VideoProcessing_SetParameter(instance, parameter);
    EXPECT_EQ(ret, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// set invalid value
HWTEST_F(MetadataGeneratorVideoNdkImplUnitTest, set_parameter_06, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x2;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, VIDEO_METADATA_GENERATOR_STYLE_CONTROL, 2);
    ret = OH_VideoProcessing_SetParameter(instance, parameter);
    EXPECT_EQ(ret, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// set normal
HWTEST_F(MetadataGeneratorVideoNdkImplUnitTest, set_parameter_07, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x2;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat* parameterGetted = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, VIDEO_METADATA_GENERATOR_STYLE_CONTROL,
        VIDEO_METADATA_GENERATOR_BRIGHT_MODE);
    ret = OH_VideoProcessing_SetParameter(instance, parameter);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_GetParameter(instance, parameterGetted);
    int32_t mode = -1;
    OH_AVFormat_GetIntValue(parameterGetted, VIDEO_METADATA_GENERATOR_STYLE_CONTROL, &mode);
    EXPECT_EQ(mode, VIDEO_METADATA_GENERATOR_BRIGHT_MODE);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// set normal
HWTEST_F(MetadataGeneratorVideoNdkImplUnitTest, set_parameter_08, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x2;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat* parameterGetted = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, VIDEO_METADATA_GENERATOR_STYLE_CONTROL,
        VIDEO_METADATA_GENERATOR_CONTRAST_MODE);
    ret = OH_VideoProcessing_SetParameter(instance, parameter);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_GetParameter(instance, parameterGetted);
    int32_t mode = -1;
    OH_AVFormat_GetIntValue(parameterGetted, VIDEO_METADATA_GENERATOR_STYLE_CONTROL, &mode);
    EXPECT_EQ(mode, VIDEO_METADATA_GENERATOR_CONTRAST_MODE);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// set invalid value
HWTEST_F(MetadataGeneratorVideoNdkImplUnitTest, set_parameter_09, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x2;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat* parameterGetted = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, VIDEO_METADATA_GENERATOR_STYLE_CONTROL, -1);
    ret = OH_VideoProcessing_SetParameter(instance, parameter);
    EXPECT_EQ(ret, VIDEO_PROCESSING_ERROR_INVALID_PARAMETER);
    OH_VideoProcessing_GetParameter(instance, parameterGetted);
    int32_t mode = -1;
    OH_AVFormat_GetIntValue(parameterGetted, VIDEO_METADATA_GENERATOR_STYLE_CONTROL, &mode);
    EXPECT_EQ(mode, VIDEO_METADATA_GENERATOR_CONTRAST_MODE);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// get parameter but param not null
HWTEST_F(MetadataGeneratorVideoNdkImplUnitTest, get_parameter_01, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x2;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    ret = OH_VideoProcessing_GetParameter(instance, parameter);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// get parameter normal
HWTEST_F(MetadataGeneratorVideoNdkImplUnitTest, get_parameter_02, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x2;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameter = nullptr;
    ret = OH_VideoProcessing_GetParameter(instance, parameter);
    EXPECT_NE(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// get parameter normal after set
HWTEST_F(MetadataGeneratorVideoNdkImplUnitTest, get_parameter_03, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x2;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameterSetted = OH_AVFormat_Create();
    OH_AVFormat* parameterGetted = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameterSetted, VIDEO_METADATA_GENERATOR_STYLE_CONTROL,
        VIDEO_METADATA_GENERATOR_BRIGHT_MODE);
    ret = OH_VideoProcessing_SetParameter(instance, parameterSetted);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessing_GetParameter(instance, parameterGetted);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// get parameter normal after set
HWTEST_F(MetadataGeneratorVideoNdkImplUnitTest, get_parameter_04, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x2;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameterSetted = OH_AVFormat_Create();
    OH_AVFormat* parameterGetted = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameterSetted, VIDEO_METADATA_GENERATOR_STYLE_CONTROL,
        VIDEO_METADATA_GENERATOR_CONTRAST_MODE);
    ret = OH_VideoProcessing_SetParameter(instance, parameterSetted);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessing_GetParameter(instance, parameterGetted);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// get default parameter
HWTEST_F(MetadataGeneratorVideoNdkImplUnitTest, get_parameter_05, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x2;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    ret = OH_VideoProcessing_GetParameter(instance, parameter);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    int32_t mode = -1;
    OH_AVFormat_GetIntValue(parameter, VIDEO_METADATA_GENERATOR_STYLE_CONTROL, &mode);
    EXPECT_EQ(mode, VIDEO_METADATA_GENERATOR_CONTRAST_MODE);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// get parameter normal after set
HWTEST_F(MetadataGeneratorVideoNdkImplUnitTest, get_parameter_06, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x2;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameterSetted = OH_AVFormat_Create();
    OH_AVFormat* parameterGetted = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameterSetted, VIDEO_METADATA_GENERATOR_STYLE_CONTROL,
        VIDEO_METADATA_GENERATOR_BRIGHT_MODE);
    ret = OH_VideoProcessing_SetParameter(instance, parameterSetted);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessing_GetParameter(instance, parameterGetted);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    int32_t mode = -1;
    OH_AVFormat_GetIntValue(parameterGetted, VIDEO_METADATA_GENERATOR_STYLE_CONTROL, &mode);
    EXPECT_EQ(mode, VIDEO_METADATA_GENERATOR_BRIGHT_MODE);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// get parameter normal after set
HWTEST_F(MetadataGeneratorVideoNdkImplUnitTest, get_parameter_07, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x2;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameterSetted = OH_AVFormat_Create();
    OH_AVFormat* parameterGetted = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameterSetted, VIDEO_METADATA_GENERATOR_STYLE_CONTROL,
        VIDEO_METADATA_GENERATOR_CONTRAST_MODE);
    ret = OH_VideoProcessing_SetParameter(instance, parameterSetted);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessing_GetParameter(instance, parameterGetted);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    int32_t mode = -1;
    OH_AVFormat_GetIntValue(parameterGetted, VIDEO_METADATA_GENERATOR_STYLE_CONTROL, &mode);
    EXPECT_EQ(mode, VIDEO_METADATA_GENERATOR_CONTRAST_MODE);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

HWTEST_F(MetadataGeneratorVideoNdkImplUnitTest, testVideoProcess_metadataGen_01, TestSize.Level1)
{
    const int formatListNum = 3;
    int formatListHDR[formatListNum] = {GRAPHIC_PIXEL_FMT_YCBCR_P010, GRAPHIC_PIXEL_FMT_YCRCB_P010,
        GRAPHIC_PIXEL_FMT_RGBA_1010102};
    const int colorMetaMetaListNum = 4;
    const int colorMetaParaNum = 2;
    const int colorMetaParaInMetaNum = 0;
    const int colorMetaParaInColorNum = 1;
    int colorMetaList[colorMetaMetaListNum][colorMetaParaNum] = {
        {CM_VIDEO_HDR10, CM_BT2020_PQ_LIMIT},
        {CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT},
        {CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT},
        {CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT}
    };
    for (int n = 0; n < colorMetaMetaListNum; n++) {
        inMetaType_ = static_cast<CM_HDR_Metadata_Type>(colorMetaList[n][colorMetaParaInMetaNum]);
        inColspc_ = static_cast<CM_ColorSpaceType>(colorMetaList[n][colorMetaParaInColorNum]);
        for (int i = 0; i < formatListNum; i++) {
            surfacePixelFmt_ = static_cast<GraphicPixelFormat>(formatListHDR[i]);
            requestCfg_.format = surfacePixelFmt_;
            VideoProcessing_ErrorCode ret = ProcessContrastStyle();
            EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
        }
    }
}

HWTEST_F(MetadataGeneratorVideoNdkImplUnitTest, testVideoProcess_metadataGen_02, TestSize.Level1)
{
    const int formatListNum = 3;
    int formatListHDR[formatListNum] = {GRAPHIC_PIXEL_FMT_YCBCR_P010, GRAPHIC_PIXEL_FMT_YCRCB_P010,
        GRAPHIC_PIXEL_FMT_RGBA_1010102};
    const int colorMetaMetaListNum = 4;
    const int colorMetaParaNum = 2;
    const int colorMetaParaInMetaNum = 0;
    const int colorMetaParaInColorNum = 1;
    int colorMetaList[colorMetaMetaListNum][colorMetaParaNum] = {
        {CM_VIDEO_HDR10, CM_BT2020_PQ_LIMIT},
        {CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT},
        {CM_VIDEO_HLG, CM_BT2020_HLG_LIMIT},
        {CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT}
    };
    for (int n = 0; n < colorMetaMetaListNum; n++) {
        inMetaType_ = static_cast<CM_HDR_Metadata_Type>(colorMetaList[n][colorMetaParaInMetaNum]);
        inColspc_ = static_cast<CM_ColorSpaceType>(colorMetaList[n][colorMetaParaInColorNum]);
        for (int i = 0; i < formatListNum; i++) {
            surfacePixelFmt_ = static_cast<GraphicPixelFormat>(formatListHDR[i]);
            requestCfg_.format = surfacePixelFmt_;
            VideoProcessing_ErrorCode ret = ProcessBrightStyle();
            EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
        }
    }
}
}