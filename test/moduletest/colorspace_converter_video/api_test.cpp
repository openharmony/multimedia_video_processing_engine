/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <memory>
#include "colorspace_converter_video.h"
#include "colorspace_converter_video_description.h"
#include "colorspace_converter_video_common.h"
#include "v1_0/cm_color_space.h"
#include "v1_0/hdr_static_metadata.h"
#include "algorithm_errors.h"
#include "media_description.h"
#include "algorithm_common.h"
#include "surface/window.h"
#include "external_window.h"
#include "colorspace_converter_video_impl.h"
#include "securec.h"

constexpr int64_t NANOS_IN_SECOND = 1000000000L;
constexpr int64_t NANOS_IN_MICRO = 1000L;
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
class CscVCB : public ColorSpaceConverterVideoCallback {
public:
    void OnError(int32_t errorCode) override;
    void OnState(int32_t state) override;
    void OnOutputBufferAvailable(uint32_t index, CscvBufferFlag flag) override;
    void OnOutputFormatChanged(const Format& formatOutput) override;
};
void CscVCB::OnOutputBufferAvailable(uint32_t index, CscvBufferFlag flag)
{
    (void)index;
    (void)flag;
}
void CscVCB::OnError(int32_t errorCode)
{
    (void)errorCode;
}
void CscVCB::OnState(int32_t state)
{
    (void)state;
}
void CscVCB::OnOutputFormatChanged(const Format& formatOutput)
{
    (void)formatOutput;
}

constexpr CM_ColorSpaceInfo COLORSPACE_INFO_HDR_BT2020_HLG = {
    COLORPRIMARIES_BT2020,
    TRANSFUNC_HLG,
    MATRIX_BT2020,
    RANGE_LIMITED
};

class CSCVInnerApiTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp()
    {
        cscv_ = ColorSpaceConverterVideo::Create();
        cscvCb_ = std::make_shared<CscVCB>();
        format.PutIntValue(CscVDescriptionKey::CSCV_KEY_PIXEL_FORMAT, int(GRAPHIC_PIXEL_FMT_YCBCR_420_SP));
        format.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_PRIMARIES, COLORPRIMARIES_BT709);
        format.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_TRANS_FUNC, TRANSFUNC_BT709);
        format.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_MATRIX, MATRIX_BT709);
        format.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_RANGE, RANGE_LIMITED);
        format.PutIntValue(CscVDescriptionKey::CSCV_KEY_HDR_METADATA_TYPE, CM_METADATA_NONE);
        format.PutIntValue(CscVDescriptionKey::CSCV_KEY_RENDER_INTENT,
            int(RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC));
        format.PutDoubleValue(CscVDescriptionKey::CSCV_KEY_SDRUI_BRIGHTNESS_RATIO, 0.0);

        format2.PutIntValue(CscVDescriptionKey::CSCV_KEY_PIXEL_FORMAT, int(GRAPHIC_PIXEL_FMT_YCBCR_P010));
        format2.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_PRIMARIES, COLORPRIMARIES_BT2020);
        format2.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_TRANS_FUNC, TRANSFUNC_HLG);
        format2.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_MATRIX, MATRIX_BT2020);
        format2.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_RANGE, RANGE_LIMITED);
        format2.PutIntValue(CscVDescriptionKey::CSCV_KEY_HDR_METADATA_TYPE, CM_METADATA_NONE);
    };
    void TearDown()
    {
        if (cscv_)
            cscv_->Release();
    };
    sptr<Surface> surface;
    OHNativeWindow *nativeWindow;
    BufferFlushConfig flushCfg_{};
    BufferRequestConfig requestCfg_{};
    uint32_t FlushSurf(OHNativeWindowBuffer *ohNativeWindowBuffer);
    GSError SetMeatadata(sptr<SurfaceBuffer> &buffer, uint32_t value);
    GSError SetMeatadata(sptr<SurfaceBuffer> &buffer, CM_ColorSpaceInfo &colorspaceInfo);
    void InitBufferConfig();
    std::shared_ptr<ColorSpaceConverterVideo> cscv_ = nullptr;
    std::shared_ptr<CscVCB> cscvCb_ = nullptr;
    OHOS::Media::Format format;
    OHOS::Media::Format format2;
};

int64_t GetSystemTimeUs()
{
    struct timespec now;
    (void)clock_gettime(CLOCK_BOOTTIME, &now);
    int64_t nanoTime = reinterpret_cast<int64_t>(now.tv_sec) * NANOS_IN_SECOND + now.tv_nsec;

    return nanoTime / NANOS_IN_MICRO;
}

GSError CSCVInnerApiTest::SetMeatadata(sptr<SurfaceBuffer> &buffer, uint32_t value)
{
    std::vector<uint8_t> metadata;
    metadata.resize(sizeof(value));
    (void)memcpy_s(metadata.data(), metadata.size(), &value, sizeof(value));
    GSError err = buffer->SetMetadata(ATTRKEY_HDR_METADATA_TYPE, metadata);
    return err;
}

GSError CSCVInnerApiTest::SetMeatadata(sptr<SurfaceBuffer> &buffer, CM_ColorSpaceInfo &colorspaceInfo)
{
    std::vector<uint8_t> metadata;
    metadata.resize(sizeof(CM_ColorSpaceInfo));
    (void)memcpy_s(metadata.data(), metadata.size(), &colorspaceInfo, sizeof(CM_ColorSpaceInfo));
    GSError err = buffer->SetMetadata(ATTRKEY_COLORSPACE_INFO, metadata);
    return err;
}

void CSCVInnerApiTest::InitBufferConfig()
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
}

uint32_t CSCVInnerApiTest::FlushSurf(OHNativeWindowBuffer *ohNativeWindowBuffer)
{
    struct Region region;
    struct Region::Rect *rect = new Region::Rect();
    rect->x = 0;
    rect->y = 0;
    rect->w = DEFAULT_WIDTH;
    rect->h = DEFAULT_HEIGHT;
    region.rects = rect;
    NativeWindowHandleOpt(nativeWindow, SET_UI_TIMESTAMP, GetSystemTimeUs());
    int32_t err = OH_NativeWindow_NativeWindowFlushBuffer(nativeWindow, ohNativeWindowBuffer, -1, region);
    delete rect;
    if (err != 0) {
        cout << "FlushBuffer failed" << endl;
        return 1;
    }
    return 0;
}

/**
 * @tc.number    : CSCVInnerApiTest_CSCV_API_0010
 * @tc.name      : release output buffer api with illegal parameter
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerApiTest, CSCV_API_0010, TestSize.Level2)
{
    int32_t ret = 0;
    std::shared_ptr<ColorSpaceConverterVideo> cscv2;
    sptr<Surface> surface2;
    ret = cscv_->SetCallback(cscvCb_);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    surface = cscv_->CreateInputSurface();
    ASSERT_NE(surface, nullptr);
    cscv2 = ColorSpaceConverterVideo::Create();
    surface2 = cscv2->CreateInputSurface();
    ret = cscv_->SetOutputSurface(surface2);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->Configure(format);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->ReleaseOutputBuffer(100000, true);
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_PARAM);
}

/**
 * @tc.number    : CSCV_API_0020
 * @tc.name      : SetParameter test
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerApiTest, CSCV_API_0020, TestSize.Level2)
{
    int32_t ret = 0;
    std::shared_ptr<ColorSpaceConverterVideo> cscv2;
    sptr<Surface> surface2;
    ret = cscv_->SetCallback(cscvCb_);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    surface = cscv_->CreateInputSurface();
    ASSERT_NE(surface, nullptr);
    cscv2 = ColorSpaceConverterVideo::Create();
    surface2 = cscv2->CreateInputSurface();
    ret = cscv_->SetOutputSurface(surface2);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->Configure(format);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);

    ret = cscv_->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->SetParameter(format);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    OHOS::Media::Format format2;
    ret = cscv_->SetParameter(format2);
    ASSERT_NE(ret, VPE_ALGO_ERR_OK);
}
/**
 * @tc.number    : CSCV_API_0030
 * @tc.name      : NotifyEOS test
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerApiTest, CSCV_API_0030, TestSize.Level2)
{
    int32_t ret = 0;
    OHNativeWindowBuffer *ohNativeWindowBuffer;
    ret = cscv_->SetCallback(cscvCb_);
    surface = cscv_->CreateInputSurface();
    std::shared_ptr<ColorSpaceConverterVideo> cscv2 = ColorSpaceConverterVideo::Create();
    sptr<Surface> surface2 = cscv2->CreateInputSurface();
    ret = cscv_->SetOutputSurface(surface2);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->Configure(format);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->Start();

    int fenceFd = -1;
    nativeWindow = CreateNativeWindowFromSurface(&surface);
    ret = OH_NativeWindow_NativeWindowHandleOpt(nativeWindow, SET_FORMAT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = OH_NativeWindow_NativeWindowHandleOpt(nativeWindow, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = OH_NativeWindow_NativeWindowRequestBuffer(nativeWindow, &ohNativeWindowBuffer, &fenceFd);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);

    ret = FlushSurf(ohNativeWindowBuffer);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    OH_NativeWindow_DestroyNativeWindow(nativeWindow);
}

/**
 * @tc.number    : CSCVInnerApiTest_CSCV_API_0040
 * @tc.name      : OnProducerBufferReleased api test
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerApiTest, CSCV_API_0040, TestSize.Level2)
{
    int32_t ret = 0;
    std::shared_ptr<ColorSpaceConverterVideoImpl> cscvImpl;
    cscvImpl = make_shared<ColorSpaceConverterVideoImpl>();
    ret = cscv_->SetCallback(cscvCb_);
    sptr<Surface> surface = cscv_->CreateInputSurface();
    ret = cscvImpl->Init();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscvImpl->SetOutputSurface(surface);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscvImpl->OnProducerBufferReleased();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerApiTest_CSCV_API_0050
 * @tc.name      : OnProducerBufferReleased api test
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerApiTest, CSCV_API_0050, TestSize.Level2)
{
        int32_t ret = 0;
    OHNativeWindowBuffer *ohNativeWindowBuffer;
    ret = cscv_->SetCallback(cscvCb_);
    surface = cscv_->CreateInputSurface();
    std::shared_ptr<ColorSpaceConverterVideo> cscv2 = ColorSpaceConverterVideo::Create();
    sptr<Surface> surface2 = cscv2->CreateInputSurface();
    ret = cscv_->SetOutputSurface(surface2);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->Configure(format);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->Start();

    int fenceFd = -1;
    nativeWindow = CreateNativeWindowFromSurface(&surface);
    ret = OH_NativeWindow_NativeWindowHandleOpt(nativeWindow, SET_FORMAT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = OH_NativeWindow_NativeWindowHandleOpt(nativeWindow, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = OH_NativeWindow_NativeWindowRequestBuffer(nativeWindow, &ohNativeWindowBuffer, &fenceFd);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = FlushSurf(ohNativeWindowBuffer);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->ReleaseOutputBuffer(0, true);
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_PARAM);
    OH_NativeWindow_DestroyNativeWindow(nativeWindow);
}

/**
 * @tc.number    : CSCVInnerApiTest_CSCV_API_0060
 * @tc.name      : OnProducerBufferReleased api test
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerApiTest, CSCV_API_0060, TestSize.Level2)
{
    int32_t ret = 0;
    ret = cscv_->SetCallback(cscvCb_);
    sptr<Surface> surface1 = cscv_->CreateInputSurface();
    std::shared_ptr<ColorSpaceConverterVideo> cscv2;
    cscv2 = ColorSpaceConverterVideo::Create();
    sptr<Surface> surface2 = cscv2->CreateInputSurface();
    ret = cscv_->SetOutputSurface(surface1);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->Configure(format);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->SetOutputSurface(surface1);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->SetOutputSurface(surface2);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->ReleaseOutputBuffer(0, 0);
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_PARAM);
}

/**
 * @tc.number    : CSCVInnerApiTest_CSCV_API_0061
 * @tc.name      : OnProducerBufferReleased api test
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerApiTest, CSCV_API_0061, TestSize.Level2)
{
    int32_t ret = 0;
    std::queue<sptr<SurfaceBuffer>> AppInBufferAvilQue;
    sptr<SurfaceBuffer> buffer;
    sptr<SurfaceBuffer> buffer2;
    int32_t fence = -1;
    GraphicPixelFormat surfacePixelFmt;
    format.GetIntValue(CscVDescriptionKey::CSCV_KEY_PIXEL_FORMAT, *(int *)&surfacePixelFmt);
    requestCfg_.format = surfacePixelFmt;
    InitBufferConfig();
    sptr<Surface> surface1 = cscv_->CreateInputSurface();
    std::shared_ptr<ColorSpaceConverterVideo> cscv2;
    cscv2 = ColorSpaceConverterVideo::Create();
    sptr<Surface> surface2 = cscv2->CreateInputSurface();
    ret = cscv_->SetCallback(cscvCb_);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->SetOutputSurface(surface2);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->Configure(format);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    GSError err = surface1->RequestBuffer(buffer, fence, requestCfg_);
    ASSERT_EQ(err, GSERROR_OK);
    AppInBufferAvilQue.push(buffer);
    err = surface1->FlushBuffer(buffer, -1, flushCfg_);
    ASSERT_EQ(err, GSERROR_OK);
    ret = cscv_->NotifyEos();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    sleep(2);
    cscv2->Release();
}

/**
 * @tc.number    : CSCVInnerApiTest_CSCV_API_0062
 * @tc.name      : OnProducerBufferReleased api test
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerApiTest, CSCV_API_0062, TestSize.Level2)
{
    int32_t ret = 0;
    std::queue<sptr<SurfaceBuffer>> AppInBufferAvilQue;
    sptr<SurfaceBuffer> buffer;
    sptr<SurfaceBuffer> buffer2;
    int32_t fence = -1;
    GraphicPixelFormat surfacePixelFmt;
    format.GetIntValue(CscVDescriptionKey::CSCV_KEY_PIXEL_FORMAT, *(int *)&surfacePixelFmt);
    requestCfg_.format = surfacePixelFmt;
    InitBufferConfig();
    sptr<Surface> surface1 = cscv_->CreateInputSurface();
    std::shared_ptr<ColorSpaceConverterVideo> cscv2;
    cscv2 = ColorSpaceConverterVideo::Create();
    sptr<Surface> surface2 = cscv2->CreateInputSurface();
    std::shared_ptr<ColorSpaceConverterVideo> cscv3;
    cscv3 = ColorSpaceConverterVideo::Create();
    sptr<Surface> surface3 = cscv3->CreateInputSurface();
    ret = cscv_->SetCallback(cscvCb_);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->SetOutputSurface(surface2);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->Configure(format);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    GSError err = surface1->RequestBuffer(buffer, fence, requestCfg_);
    ASSERT_EQ(err, GSERROR_OK);
    AppInBufferAvilQue.push(buffer);
    err = surface1->FlushBuffer(buffer, -1, flushCfg_);
    ASSERT_EQ(err, GSERROR_OK);
    ret = cscv_->SetOutputSurface(surface3);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv_->NotifyEos();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    sleep(2);
    cscv2->Release();
    cscv3->Release();
}

/**
 * @tc.number    : CSCVInnerApiTest_CSCV_API_0063
 * @tc.name      : OnProducerBufferReleased api test
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerApiTest, CSCV_API_0063, TestSize.Level2)
{
    int32_t ret = 0;
    GSError err = GSERROR_OK;
    std::queue<sptr<SurfaceBuffer>> AppInBufferAvilQue;
    sptr<SurfaceBuffer> buffer;
    sptr<SurfaceBuffer> buffer2;
    int32_t fence = -1;
    CM_ColorSpaceInfo inColspcInfo = COLORSPACE_INFO_HDR_BT2020_HLG;
    CM_HDR_Metadata_Type inMetaType = CM_METADATA_NONE;
    GraphicPixelFormat surfacePixelFmt;
    format2.GetIntValue(CscVDescriptionKey::CSCV_KEY_PIXEL_FORMAT, *(int *)&surfacePixelFmt);
    requestCfg_.format = surfacePixelFmt;
    InitBufferConfig();
    sptr<Surface> surface1 = cscv_->CreateInputSurface();
    std::shared_ptr<ColorSpaceConverterVideo> cscv2;
    cscv2 = ColorSpaceConverterVideo::Create();
    sptr<Surface> surface2 = cscv2->CreateInputSurface();
    std::shared_ptr<ColorSpaceConverterVideo> cscv3;
    cscv3 = ColorSpaceConverterVideo::Create();
    sptr<Surface> surface3 = cscv3->CreateInputSurface();
    cscv_->SetCallback(cscvCb_);
    cscv_->SetOutputSurface(surface2);
    cscv_->Configure(format);
    cscv_->Prepare();
    ret = cscv_->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    for (int i = 0; i < 3; i++) {
        sptr<Surface> surfaceTmp = (i % 2) ? surface2 : surface3;
        err = surface1->RequestBuffer(buffer, fence, requestCfg_);
        ASSERT_EQ(err, GSERROR_OK);
        err = SetMeatadata(buffer, inColspcInfo);
        ASSERT_EQ(err, GSERROR_OK);
        err = SetMeatadata(buffer, (uint32_t)inMetaType);
        ASSERT_EQ(err, GSERROR_OK);
        AppInBufferAvilQue.push(buffer);
        err = surface1->FlushBuffer(buffer, -1, flushCfg_);
        ASSERT_EQ(err, GSERROR_OK);
        ret = cscv_->SetOutputSurface(surfaceTmp);
        ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
        ret = cscv_->NotifyEos();
        sleep(2);
    }
    cscv2->Release();
    cscv3->Release();
}
}