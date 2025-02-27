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
#include <memory>
#include "metadata_generator_video.h"
#include "metadata_generator_video_common.h"
#include "metadata_generator_video_impl.h"
#include "algorithm_errors.h"
#include "media_description.h"
#include "algorithm_common.h"
#include "surface/window.h"
#include "external_window.h"
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
class CscVCB : public MetadataGeneratorVideoCallback {
public:
    void OnError(int32_t errorCode) override;
    void OnState(int32_t state) override;
    void OnOutputBufferAvailable(uint32_t index, MdgBufferFlag flag) override;
};
void CscVCB::OnOutputBufferAvailable(uint32_t index, MdgBufferFlag flag)
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
        mdg_ = MetadataGeneratorVideo::Create();
        cscvCb_ = std::make_shared<CscVCB>();
    };
    void TearDown()
    {
        if (mdg_)
            mdg_->Release();
    };
    sptr<Surface> surface;
    OHNativeWindow *nativeWindow;
    BufferFlushConfig flushCfg_{};
    BufferRequestConfig requestCfg_{};
    uint32_t FlushSurf(OHNativeWindowBuffer *ohNativeWindowBuffer);
    GSError SetMeatadata(sptr<SurfaceBuffer> &buffer, uint32_t value);
    GSError SetMeatadata(sptr<SurfaceBuffer> &buffer, CM_ColorSpaceInfo &colorspaceInfo);
    void InitBufferConfig();
    std::shared_ptr<MetadataGeneratorVideo> mdg_ = nullptr;
    std::shared_ptr<CscVCB> cscvCb_ = nullptr;
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
    std::shared_ptr<MetadataGeneratorVideo> mdg2;
    sptr<Surface> surface2;
    ret = mdg_->SetCallback(cscvCb_);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    surface = mdg_->CreateInputSurface();
    ASSERT_NE(surface, nullptr);
    mdg2 = MetadataGeneratorVideo::Create();
    surface2 = mdg2->CreateInputSurface();
    ret = mdg_->SetOutputSurface(surface2);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->Configure();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->ReleaseOutputBuffer(100000, true);
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
    std::shared_ptr<MetadataGeneratorVideo> mdg2;
    sptr<Surface> surface2;
    ret = mdg_->SetCallback(cscvCb_);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    surface = mdg_->CreateInputSurface();
    ASSERT_NE(surface, nullptr);
    mdg2 = MetadataGeneratorVideo::Create();
    surface2 = mdg2->CreateInputSurface();
    ret = mdg_->SetOutputSurface(surface2);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->Configure();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);

    ret = mdg_->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
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
    ret = mdg_->SetCallback(cscvCb_);
    surface = mdg_->CreateInputSurface();
    std::shared_ptr<MetadataGeneratorVideo> mdg2 = MetadataGeneratorVideo::Create();
    sptr<Surface> surface2 = mdg2->CreateInputSurface();
    ret = mdg_->SetOutputSurface(surface2);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->Configure();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->Start();

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
    std::shared_ptr<MetadataGeneratorVideoImpl> cscvImpl;
    cscvImpl = make_shared<MetadataGeneratorVideoImpl>();
    ret = mdg_->SetCallback(cscvCb_);
    sptr<Surface> surface = mdg_->CreateInputSurface();
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
    ret = mdg_->SetCallback(cscvCb_);
    surface = mdg_->CreateInputSurface();
    std::shared_ptr<MetadataGeneratorVideo> mdg2 = MetadataGeneratorVideo::Create();
    sptr<Surface> surface2 = mdg2->CreateInputSurface();
    ret = mdg_->SetOutputSurface(surface2);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->Configure();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->Start();

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
    ret = mdg_->ReleaseOutputBuffer(0, true);
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
    ret = mdg_->SetCallback(cscvCb_);
    sptr<Surface> surface1 = mdg_->CreateInputSurface();
    std::shared_ptr<MetadataGeneratorVideo> mdg2;
    mdg2 = MetadataGeneratorVideo::Create();
    sptr<Surface> surface2 = mdg2->CreateInputSurface();
    ret = mdg_->SetOutputSurface(surface1);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->Configure();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->SetOutputSurface(surface1);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->SetOutputSurface(surface2);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->ReleaseOutputBuffer(0, 0);
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
    requestCfg_.format = GRAPHIC_PIXEL_FMT_YCBCR_P010;
    InitBufferConfig();
    sptr<Surface> surface1 = mdg_->CreateInputSurface();
    std::shared_ptr<MetadataGeneratorVideo> mdg2;
    mdg2 = MetadataGeneratorVideo::Create();
    sptr<Surface> surface2 = mdg2->CreateInputSurface();
    ret = mdg_->SetCallback(cscvCb_);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->SetOutputSurface(surface2);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->Configure();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    GSError err = surface1->RequestBuffer(buffer, fence, requestCfg_);
    ASSERT_EQ(err, GSERROR_OK);
    AppInBufferAvilQue.push(buffer);
    err = surface1->FlushBuffer(buffer, -1, flushCfg_);
    ASSERT_EQ(err, GSERROR_OK);
    ret = mdg_->NotifyEos();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    mdg2->Release();
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
    requestCfg_.format = GRAPHIC_PIXEL_FMT_YCBCR_P010;
    InitBufferConfig();
    sptr<Surface> surface1 = mdg_->CreateInputSurface();
    std::shared_ptr<MetadataGeneratorVideo> mdg2;
    mdg2 = MetadataGeneratorVideo::Create();
    sptr<Surface> surface2 = mdg2->CreateInputSurface();
    std::shared_ptr<MetadataGeneratorVideo> mdg3;
    mdg3 = MetadataGeneratorVideo::Create();
    sptr<Surface> surface3 = mdg3->CreateInputSurface();
    ret = mdg_->SetCallback(cscvCb_);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->SetOutputSurface(surface2);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->Configure();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    GSError err = surface1->RequestBuffer(buffer, fence, requestCfg_);
    ASSERT_EQ(err, GSERROR_OK);
    AppInBufferAvilQue.push(buffer);
    err = surface1->FlushBuffer(buffer, -1, flushCfg_);
    ASSERT_EQ(err, GSERROR_OK);
    ret = mdg_->SetOutputSurface(surface3);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg_->NotifyEos();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    mdg2->Release();
    mdg3->Release();
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
    requestCfg_.format = GRAPHIC_PIXEL_FMT_YCBCR_P010;
    InitBufferConfig();
    sptr<Surface> surface1 = mdg_->CreateInputSurface();
    std::shared_ptr<MetadataGeneratorVideo> mdg2;
    mdg2 = MetadataGeneratorVideo::Create();
    sptr<Surface> surface2 = mdg2->CreateInputSurface();
    std::shared_ptr<MetadataGeneratorVideo> mdg3;
    mdg3 = MetadataGeneratorVideo::Create();
    sptr<Surface> surface3 = mdg3->CreateInputSurface();
    mdg_->SetCallback(cscvCb_);
    mdg_->SetOutputSurface(surface2);
    mdg_->Configure();
    mdg_->Prepare();
    ret = mdg_->Start();
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
        ret = mdg_->SetOutputSurface(surfaceTmp);
        ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
        ret = mdg_->NotifyEos();
        sleep(2);
    }
    mdg2->Release();
    mdg3->Release();
}
}