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
#include "colorspace_converter_video.h"
#include "colorspace_converter_video_description.h"
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

class ColorSpaceConverterVideoNdkImplUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp()
    {
        requestCfg_.usage =
            BUFFER_USAGE_CPU_READ | BUFFER_USAGE_CPU_WRITE | BUFFER_USAGE_HW_RENDER | BUFFER_USAGE_HW_TEXTURE;
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
    GraphicPixelFormat inSurfacePixelFmt_ = GRAPHIC_PIXEL_FMT_YCBCR_P010;
    OH_NativeBuffer_ColorSpace outColspcInfo_ = OH_COLORSPACE_BT709_LIMIT;
    OH_NativeBuffer_MetadataType outMetaType_ = OH_VIDEO_HDR_VIVID;
    OH_NativeBuffer_Format outSurfacePixelFmt_ = NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP;

    void SetMeatadata(sptr<SurfaceBuffer> &buffer, uint32_t value);
    void SetMeatadata(sptr<SurfaceBuffer> &buffer, CM_ColorSpaceInfo &colorspaceInfo);
    VideoProcessing_ErrorCode ProcessToHDR();
    VideoProcessing_ErrorCode ProcessToSDR();
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

void ColorSpaceConverterVideoNdkImplUnitTest::SetMeatadata(sptr<SurfaceBuffer> &buffer, uint32_t value)
{
    std::vector<uint8_t> metadata;
    metadata.resize(sizeof(value));
    (void)memcpy_s(metadata.data(), metadata.size(), &value, sizeof(value));
    uint32_t err = buffer->SetMetadata(ATTRKEY_HDR_METADATA_TYPE, metadata);
    if (err != 0) {
        printf("Buffer set metadata info, ret: %d\n", err);
    }
}

void ColorSpaceConverterVideoNdkImplUnitTest::SetMeatadata(sptr<SurfaceBuffer> &buffer,
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

VideoProcessing_ErrorCode ColorSpaceConverterVideoNdkImplUnitTest::ProcessToSDR()
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
    int createType = 0x1;
    OH_VideoProcessing* instance = nullptr;
    OH_VideoProcessing* instance2 = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    ret = OH_VideoProcessing_Create(&instance2, createType);
    VideoProcessing_Callback* callback = nullptr;
    ret = OH_VideoProcessingCallback_Create(&callback);
    ret = OH_VideoProcessingCallback_BindOnError(callback, OnError);
    ret = OH_VideoProcessingCallback_BindOnState(callback, OnState);
    ret = OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, OnNewOutputBuffer);
    OHNativeWindow* window = nullptr;
    OHNativeWindow* window2 = nullptr;
    ret = OH_VideoProcessing_GetSurface(instance, &window);
    ret = OH_VideoProcessing_GetSurface(instance2, &window2);
    int32_t err1 = OH_NativeWindow_NativeWindowHandleOpt(window2, SET_FORMAT, outSurfacePixelFmt_);
    err1 = OH_NativeWindow_SetColorSpace(window2, outColspcInfo_);
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

VideoProcessing_ErrorCode ColorSpaceConverterVideoNdkImplUnitTest::ProcessToHDR()
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
    int createType = 0x1;
    OH_VideoProcessing* instance = nullptr;
    OH_VideoProcessing* instance2 = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    ret = OH_VideoProcessing_Create(&instance2, createType);
    VideoProcessing_Callback* callback = nullptr;
    ret = OH_VideoProcessingCallback_Create(&callback);
    ret = OH_VideoProcessingCallback_BindOnError(callback, OnError);
    ret = OH_VideoProcessingCallback_BindOnState(callback, OnState);
    ret = OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, OnNewOutputBuffer);
    OHNativeWindow* window = nullptr;
    OHNativeWindow* window2 = nullptr;
    ret = OH_VideoProcessing_GetSurface(instance, &window);
    ret = OH_VideoProcessing_GetSurface(instance2, &window2);
    int32_t err1 = OH_NativeWindow_NativeWindowHandleOpt(window2, SET_FORMAT, outSurfacePixelFmt_);
    err1 = OH_NativeWindow_SetColorSpace(window2, outColspcInfo_);
    uint8_t val = static_cast<uint8_t>(outMetaType_);
    err1 = OH_NativeWindow_SetMetadataValue(window2, OH_HDR_METADATA_TYPE, sizeof(uint8_t), &val);
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

HWTEST_F(ColorSpaceConverterVideoNdkImplUnitTest, testVideoProcess_testFunSupportedSDR2SDR, TestSize.Level1)
{
    bool resultSupported = false;
    resultSupported = OH_VideoProcessing_IsColorSpaceConversionSupported(nullptr, nullptr);
    EXPECT_EQ(resultSupported, false);
    resultSupported = OH_VideoProcessing_IsMetadataGenerationSupported(nullptr);
    EXPECT_EQ(resultSupported, false);
    const int formatListNum = 3;
    int formatListSDRNative[formatListNum] = {NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP,
        NATIVEBUFFER_PIXEL_FMT_RGBA_8888};
    const int colorMetaSdr2SdrListNum = 2;
    const int colorMetaParaNum = 4;
    const int colorMetaParaInColorNum = 1;
    const int colorMetaParaOutColorNum = 3;
    int colorMetaListSdr2Sdr[colorMetaSdr2SdrListNum][colorMetaParaNum] = {
        {CM_METADATA_NONE, OH_COLORSPACE_BT601_EBU_LIMIT, CM_METADATA_NONE, OH_COLORSPACE_BT709_LIMIT},
        {CM_METADATA_NONE, OH_COLORSPACE_BT601_SMPTE_C_LIMIT, CM_METADATA_NONE, OH_COLORSPACE_BT709_LIMIT}
    };
    VideoProcessing_ColorSpaceInfo sourceVideoInfo = {-1, -1, -1};
    VideoProcessing_ColorSpaceInfo destinationVideoInfo = {-1, -1, -1};
    for (int n = 0; n < colorMetaSdr2SdrListNum; n++) {
        sourceVideoInfo.colorSpace = static_cast<int32_t>(colorMetaListSdr2Sdr[n][colorMetaParaInColorNum]);
        destinationVideoInfo.colorSpace = static_cast<int32_t>(colorMetaListSdr2Sdr[n][colorMetaParaOutColorNum]);
        for (int i = 0; i < formatListNum; i++) {
            sourceVideoInfo.pixelFormat = static_cast<int32_t>(formatListSDRNative[i]);
            for (int j = 0; j < formatListNum; j++) {
                destinationVideoInfo.pixelFormat = static_cast<int32_t>(formatListSDRNative[j]);
                resultSupported = OH_VideoProcessing_IsColorSpaceConversionSupported(
                    &sourceVideoInfo, &destinationVideoInfo);
                EXPECT_EQ(resultSupported, true);
            }
        }
    }
    sourceVideoInfo.metadataType = static_cast<int32_t>(OH_VIDEO_HDR_VIVID);
    sourceVideoInfo.colorSpace = static_cast<int32_t>(OH_COLORSPACE_BT2020_HLG_LIMIT);
    sourceVideoInfo.pixelFormat = static_cast<int32_t>(NATIVEBUFFER_PIXEL_FMT_YCBCR_P010);
    destinationVideoInfo.metadataType = static_cast<int32_t>(OH_VIDEO_HDR_VIVID);
    destinationVideoInfo.colorSpace = static_cast<int32_t>(OH_COLORSPACE_BT2020_HLG_LIMIT);
    destinationVideoInfo.pixelFormat = static_cast<int32_t>(NATIVEBUFFER_PIXEL_FMT_YCBCR_P010);
    resultSupported = OH_VideoProcessing_IsColorSpaceConversionSupported(
        &sourceVideoInfo, &destinationVideoInfo);
    EXPECT_EQ(resultSupported, false);
}
HWTEST_F(ColorSpaceConverterVideoNdkImplUnitTest, testVideoProcess_testFunSupportedHDR2SDR, TestSize.Level1)
{
    bool resultSupported = false;
    const int formatListNum = 3;
    int formatListSDRNative[formatListNum] = {NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP,
        NATIVEBUFFER_PIXEL_FMT_RGBA_8888};
    int formatListHDRNative[formatListNum] = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, NATIVEBUFFER_PIXEL_FMT_YCRCB_P010,
        NATIVEBUFFER_PIXEL_FMT_RGBA_1010102};
    const int colorMetaHdr2SdrListNum = 2;
    const int colorMetaParaNum = 4;
    const int colorMetaParaInMetaNum = 0;
    const int colorMetaParaInColorNum = 1;
    const int colorMetaParaOutColorNum = 3;
    int colorMetaListHdr2Sdr[colorMetaHdr2SdrListNum][colorMetaParaNum] = {
        {OH_VIDEO_HDR_VIVID, OH_COLORSPACE_BT2020_PQ_LIMIT, CM_METADATA_NONE, OH_COLORSPACE_BT709_LIMIT},
        {OH_VIDEO_HDR_VIVID, OH_COLORSPACE_BT2020_HLG_LIMIT, CM_METADATA_NONE, OH_COLORSPACE_BT709_LIMIT}
    };
    VideoProcessing_ColorSpaceInfo sourceVideoInfo = {-1, -1, -1};
    VideoProcessing_ColorSpaceInfo destinationVideoInfo = {-1, -1, -1};
    for (int n = 0; n < colorMetaHdr2SdrListNum; n++) {
        sourceVideoInfo.metadataType = static_cast<int32_t>(colorMetaListHdr2Sdr[n][colorMetaParaInMetaNum]);
        sourceVideoInfo.colorSpace = static_cast<int32_t>(colorMetaListHdr2Sdr[n][colorMetaParaInColorNum]);
        destinationVideoInfo.colorSpace = static_cast<int32_t>(colorMetaListHdr2Sdr[n][colorMetaParaOutColorNum]);
        for (int i = 0; i < formatListNum; i++) {
            sourceVideoInfo.pixelFormat = static_cast<int32_t>(formatListHDRNative[i]);
            for (int j = 0; j < formatListNum; j++) {
                destinationVideoInfo.pixelFormat = static_cast<int32_t>(formatListSDRNative[j]);
                resultSupported = OH_VideoProcessing_IsColorSpaceConversionSupported(
                    &sourceVideoInfo, &destinationVideoInfo);
                EXPECT_EQ(resultSupported, true);
            }
        }
    }
    sourceVideoInfo.metadataType = static_cast<int32_t>(OH_VIDEO_HDR_VIVID);
    sourceVideoInfo.colorSpace = static_cast<int32_t>(OH_COLORSPACE_BT2020_HLG_LIMIT);
    sourceVideoInfo.pixelFormat = static_cast<int32_t>(NATIVEBUFFER_PIXEL_FMT_YCBCR_P010);
    destinationVideoInfo.metadataType = static_cast<int32_t>(OH_VIDEO_HDR_VIVID);
    destinationVideoInfo.colorSpace = static_cast<int32_t>(OH_COLORSPACE_BT2020_HLG_LIMIT);
    destinationVideoInfo.pixelFormat = static_cast<int32_t>(NATIVEBUFFER_PIXEL_FMT_YCBCR_P010);
    resultSupported = OH_VideoProcessing_IsColorSpaceConversionSupported(
        &sourceVideoInfo, &destinationVideoInfo);
    EXPECT_EQ(resultSupported, false);
}
HWTEST_F(ColorSpaceConverterVideoNdkImplUnitTest, testVideoProcess_testFunSupportedHDR2HDR, TestSize.Level1)
{
    bool resultSupported = false;
    const int formatListNum = 3;
    int formatListHDRNative[formatListNum] = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, NATIVEBUFFER_PIXEL_FMT_YCRCB_P010,
        NATIVEBUFFER_PIXEL_FMT_RGBA_1010102};
    const int colorMetaHdr2HdrListNum = 4;
    const int colorMetaParaNum = 4;
    const int colorMetaParaInMetaNum = 0;
    const int colorMetaParaInColorNum = 1;
    const int colorMetaParaOutMetaNum = 2;
    const int colorMetaParaOutColorNum = 3;
    int colorMetaListHdr2Hdr[colorMetaHdr2HdrListNum][colorMetaParaNum] = {
        {OH_VIDEO_HDR_VIVID, OH_COLORSPACE_BT2020_PQ_LIMIT, OH_VIDEO_HDR_VIVID, OH_COLORSPACE_BT2020_HLG_LIMIT},
        {OH_VIDEO_HDR_HDR10, OH_COLORSPACE_BT2020_PQ_LIMIT, OH_VIDEO_HDR_HLG, OH_COLORSPACE_BT2020_HLG_LIMIT},
        {OH_VIDEO_HDR_VIVID, OH_COLORSPACE_BT2020_PQ_LIMIT, OH_VIDEO_HDR_HLG, OH_COLORSPACE_BT2020_HLG_LIMIT},
        {OH_VIDEO_HDR_VIVID, OH_COLORSPACE_BT2020_HLG_LIMIT, OH_VIDEO_HDR_VIVID, OH_COLORSPACE_BT2020_PQ_LIMIT}
    };
    VideoProcessing_ColorSpaceInfo sourceVideoInfo = {-1, -1, -1};
    VideoProcessing_ColorSpaceInfo destinationVideoInfo = {-1, -1, -1};
    for (int n = 0; n < colorMetaHdr2HdrListNum; n++) {
        sourceVideoInfo.metadataType = static_cast<int32_t>(colorMetaListHdr2Hdr[n][colorMetaParaInMetaNum]);
        sourceVideoInfo.colorSpace = static_cast<int32_t>(colorMetaListHdr2Hdr[n][colorMetaParaInColorNum]);
        destinationVideoInfo.metadataType = static_cast<int32_t>(colorMetaListHdr2Hdr[n][colorMetaParaOutMetaNum]);
        destinationVideoInfo.colorSpace = static_cast<int32_t>(colorMetaListHdr2Hdr[n][colorMetaParaOutColorNum]);
        for (int i = 0; i < formatListNum; i++) {
            sourceVideoInfo.pixelFormat = static_cast<int32_t>(formatListHDRNative[i]);
            for (int j = 0; j < formatListNum; j++) {
                destinationVideoInfo.pixelFormat = static_cast<int32_t>(formatListHDRNative[j]);
                resultSupported = OH_VideoProcessing_IsColorSpaceConversionSupported(
                    &sourceVideoInfo, &destinationVideoInfo);
                EXPECT_EQ(resultSupported, true);
            }
        }
    }
    sourceVideoInfo.metadataType = static_cast<int32_t>(OH_VIDEO_HDR_VIVID);
    sourceVideoInfo.colorSpace = static_cast<int32_t>(OH_COLORSPACE_BT709_FULL);
    sourceVideoInfo.pixelFormat = static_cast<int32_t>(NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP);
    destinationVideoInfo.metadataType = static_cast<int32_t>(OH_VIDEO_HDR_VIVID);
    destinationVideoInfo.colorSpace = static_cast<int32_t>(OH_COLORSPACE_BT709_FULL);
    destinationVideoInfo.pixelFormat = static_cast<int32_t>(NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP);
    resultSupported = OH_VideoProcessing_IsColorSpaceConversionSupported(
        &sourceVideoInfo, &destinationVideoInfo);
    EXPECT_EQ(resultSupported, false);
}

HWTEST_F(ColorSpaceConverterVideoNdkImplUnitTest, testVideoProcess_testFun, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    int createType = 0x1;
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
    EXPECT_EQ(ret, VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED);
    ret = OH_VideoProcessing_GetParameter(instance, parameter);
    EXPECT_EQ(ret, VIDEO_PROCESSING_ERROR_OPERATION_NOT_PERMITTED);
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

HWTEST_F(ColorSpaceConverterVideoNdkImplUnitTest, testVideoProcess_sdr2sdr, TestSize.Level1)
{
    const int formatListNum = 3;
    int formatListSDR[formatListNum] = {GRAPHIC_PIXEL_FMT_YCBCR_420_SP, GRAPHIC_PIXEL_FMT_YCRCB_420_SP,
        GRAPHIC_PIXEL_FMT_RGBA_8888};
    int formatListSDRNative[formatListNum] = {NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP,
        NATIVEBUFFER_PIXEL_FMT_RGBA_8888};
    const int colorMetaSdr2SdrListNum = 2;
    const int colorMetaParaNum = 4;
    const int colorMetaParaInMetaNum = 0;
    const int colorMetaParaInColorNum = 1;
    const int colorMetaParaOutColorNum = 3;
    int colorMetaListSdr2Sdr[colorMetaSdr2SdrListNum][colorMetaParaNum] = {
        {CM_METADATA_NONE, CM_BT601_EBU_LIMIT, CM_METADATA_NONE, OH_COLORSPACE_BT709_LIMIT},
        {CM_METADATA_NONE, CM_BT601_SMPTE_C_LIMIT, CM_METADATA_NONE, OH_COLORSPACE_BT709_LIMIT}
    };
    for (int n = 0; n < colorMetaSdr2SdrListNum; n++) {
        inMetaType_ = static_cast<CM_HDR_Metadata_Type>(colorMetaListSdr2Sdr[n][colorMetaParaInMetaNum]);
        inColspc_ = static_cast<CM_ColorSpaceType>(colorMetaListSdr2Sdr[n][colorMetaParaInColorNum]);
        outColspcInfo_ = static_cast<OH_NativeBuffer_ColorSpace>(colorMetaListSdr2Sdr[n][colorMetaParaOutColorNum]);
        for (int i = 0; i < formatListNum; i++) {
            inSurfacePixelFmt_ = static_cast<GraphicPixelFormat>(formatListSDR[i]);
            requestCfg_.format = inSurfacePixelFmt_;
            for (int j = 0; j < formatListNum; j++) {
                outSurfacePixelFmt_ = static_cast<OH_NativeBuffer_Format>(formatListSDRNative[j]);
                VideoProcessing_ErrorCode ret = ProcessToSDR();
                EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
            }
        }
    }
}

HWTEST_F(ColorSpaceConverterVideoNdkImplUnitTest, testVideoProcess_hdr2sdr, TestSize.Level1)
{
    const int formatListNum = 3;
    int formatListHDR[formatListNum] = {GRAPHIC_PIXEL_FMT_YCBCR_P010, GRAPHIC_PIXEL_FMT_YCRCB_P010,
        GRAPHIC_PIXEL_FMT_RGBA_1010102};
    int formatListSDRNative[formatListNum] = {NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP, NATIVEBUFFER_PIXEL_FMT_YCRCB_420_SP,
        NATIVEBUFFER_PIXEL_FMT_RGBA_8888};
    const int colorMetaHdr2SdrListNum = 2;
    const int colorMetaParaNum = 4;
    const int colorMetaParaInMetaNum = 0;
    const int colorMetaParaInColorNum = 1;
    const int colorMetaParaOutColorNum = 3;
    int colorMetaListHdr2Sdr[colorMetaHdr2SdrListNum][colorMetaParaNum] = {
        {CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, CM_METADATA_NONE, OH_COLORSPACE_BT709_LIMIT},
        {CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, CM_METADATA_NONE, OH_COLORSPACE_BT709_LIMIT}
    };
    for (int n = 0; n < colorMetaHdr2SdrListNum; n++) {
        inMetaType_ = static_cast<CM_HDR_Metadata_Type>(colorMetaListHdr2Sdr[n][colorMetaParaInMetaNum]);
        inColspc_ = static_cast<CM_ColorSpaceType>(colorMetaListHdr2Sdr[n][colorMetaParaInColorNum]);
        outColspcInfo_ = static_cast<OH_NativeBuffer_ColorSpace>(colorMetaListHdr2Sdr[n][colorMetaParaOutColorNum]);
        for (int i = 0; i < formatListNum; i++) {
            inSurfacePixelFmt_ = static_cast<GraphicPixelFormat>(formatListHDR[i]);
            requestCfg_.format = inSurfacePixelFmt_;
            for (int j = 0; j < formatListNum; j++) {
                outSurfacePixelFmt_ = static_cast<OH_NativeBuffer_Format>(formatListSDRNative[j]);
                VideoProcessing_ErrorCode ret = ProcessToSDR();
                EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
            }
        }
    }
}

HWTEST_F(ColorSpaceConverterVideoNdkImplUnitTest, testVideoProcess_hdr2hdr, TestSize.Level1)
{
    const int formatListNum = 3;
    int formatListHDR[formatListNum] = {GRAPHIC_PIXEL_FMT_YCBCR_P010, GRAPHIC_PIXEL_FMT_YCRCB_P010,
        GRAPHIC_PIXEL_FMT_RGBA_1010102};
    int formatListHDRNative[formatListNum] = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, NATIVEBUFFER_PIXEL_FMT_YCRCB_P010,
        NATIVEBUFFER_PIXEL_FMT_RGBA_1010102};
    const int colorMetaHdr2HdrListNum = 4;
    const int colorMetaParaNum = 4;
    const int colorMetaParaInMetaNum = 0;
    const int colorMetaParaInColorNum = 1;
    const int colorMetaParaOutMetaNum = 2;
    const int colorMetaParaOutColorNum = 3;
    int colorMetaListSdr2Sdr[colorMetaHdr2HdrListNum][colorMetaParaNum] = {
        {CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, OH_VIDEO_HDR_VIVID, OH_COLORSPACE_BT2020_HLG_LIMIT},
        {CM_VIDEO_HDR10, CM_BT2020_PQ_LIMIT, OH_VIDEO_HDR_HLG, OH_COLORSPACE_BT2020_HLG_LIMIT},
        {CM_VIDEO_HDR_VIVID, CM_BT2020_PQ_LIMIT, OH_VIDEO_HDR_HLG, OH_COLORSPACE_BT2020_HLG_LIMIT},
        {CM_VIDEO_HDR_VIVID, CM_BT2020_HLG_LIMIT, OH_VIDEO_HDR_VIVID, OH_COLORSPACE_BT2020_PQ_LIMIT}
    };
    for (int n = 0; n < colorMetaHdr2HdrListNum; n++) {
        inMetaType_ = static_cast<CM_HDR_Metadata_Type>(colorMetaListSdr2Sdr[n][colorMetaParaInMetaNum]);
        inColspc_ = static_cast<CM_ColorSpaceType>(colorMetaListSdr2Sdr[n][colorMetaParaInColorNum]);
        outMetaType_ = static_cast<OH_NativeBuffer_MetadataType>(colorMetaListSdr2Sdr[n][colorMetaParaOutMetaNum]);
        outColspcInfo_ = static_cast<OH_NativeBuffer_ColorSpace>(colorMetaListSdr2Sdr[n][colorMetaParaOutColorNum]);
        for (int i = 0; i < formatListNum; i++) {
            inSurfacePixelFmt_ = static_cast<GraphicPixelFormat>(formatListHDR[i]);
            requestCfg_.format = inSurfacePixelFmt_;
            for (int j = 0; j < formatListNum; j++) {
                outSurfacePixelFmt_ = static_cast<OH_NativeBuffer_Format>(formatListHDRNative[j]);
                VideoProcessing_ErrorCode ret = ProcessToHDR();
                EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
            }
        }
    }
}

HWTEST_F(ColorSpaceConverterVideoNdkImplUnitTest, codeCoverageComponent_10, TestSize.Level1)
{
    inMetaType_ = static_cast<CM_HDR_Metadata_Type>(CM_IMAGE_HDR_ISO_SINGLE);
    inColspc_ = static_cast<CM_ColorSpaceType>(CM_DISPLAY_P3_HLG);
    outColspcInfo_ = static_cast<OH_NativeBuffer_ColorSpace>(OH_COLORSPACE_DISPLAY_P3_HLG);
    inSurfacePixelFmt_ = static_cast<GraphicPixelFormat>(GRAPHIC_PIXEL_FMT_YUYV_422_PKG);
    requestCfg_.format = inSurfacePixelFmt_;
    outSurfacePixelFmt_ = static_cast<OH_NativeBuffer_Format>(NATIVEBUFFER_PIXEL_FMT_YUYV_422_PKG);
    VideoProcessing_ErrorCode ret = ProcessToSDR();
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
}
HWTEST_F(ColorSpaceConverterVideoNdkImplUnitTest, codeCoverageComponent_20, TestSize.Level1)
{
    inMetaType_ = static_cast<CM_HDR_Metadata_Type>(CM_VIDEO_HDR_VIVID);
    inColspc_ = static_cast<CM_ColorSpaceType>(CM_BT2020_PQ_LIMIT);
    outColspcInfo_ = static_cast<OH_NativeBuffer_ColorSpace>(OH_COLORSPACE_BT709_LIMIT);
    inSurfacePixelFmt_ = static_cast<GraphicPixelFormat>(GRAPHIC_PIXEL_FMT_YCBCR_P010);
    requestCfg_.format = inSurfacePixelFmt_;
    outSurfacePixelFmt_ = static_cast<OH_NativeBuffer_Format>(NATIVEBUFFER_PIXEL_FMT_YCBCR_420_SP);
    VideoProcessing_ErrorCode ret = VIDEO_PROCESSING_SUCCESS;
    sptr<SurfaceBuffer> buffer;
    CM_ColorSpaceInfo inColspcInfo = {
        static_cast<CM_ColorPrimaries>((inColspc_ & COLORPRIMARIES_MASK) >> COLORPRIMARIES_OFFSET),
        static_cast<CM_TransFunc>((inColspc_ & TRANSFUNC_MASK) >> TRANSFUNC_OFFSET),
        static_cast<CM_Matrix>((inColspc_ & MATRIX_MASK) >> MATRIX_OFFSET),
        static_cast<CM_Range>((inColspc_ & RANGE_MASK) >> RANGE_OFFSET)
    };
    ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x1;
    OH_VideoProcessing* instance = nullptr;
    OH_VideoProcessing* instance2 = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    ret = OH_VideoProcessing_Create(&instance2, createType);
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
    window->surface->RequestBuffer(buffer, fence_, requestCfg_);
    SetMeatadata(buffer, inColspcInfo);
    SetMeatadata(buffer, (uint32_t)inMetaType_);
    window->surface->FlushBuffer(buffer, fence_, flushCfg_);
    OH_VideoProcessing_SetSurface(instance, window2);
    int sleepTime = 2;
    sleep(sleepTime);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_Destroy(instance2);
    OH_VideoProcessing_DeinitializeEnvironment();
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
}

}