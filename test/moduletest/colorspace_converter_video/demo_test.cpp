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
#include <sys/time.h>
#include <gtest/gtest.h>
#include <memory>
#include <fstream>
#include "colorspace_converter_video_description.h"
#include "v1_0/cm_color_space.h"
#include "v1_0/hdr_static_metadata.h"
#include "algorithm_errors.h"
#include "media_description.h"
#include "algorithm_common.h"
#include "surface/window.h"
#include "external_window.h"
#include "colorspace_converter_video_impl.h"
#include "colorspace_converter_video.h"
#include "colorspace_converter_video_description.h"
#include "colorspace_converter_video_common.h"
#include "vpe_log.h"
#include "securec.h"

constexpr uint32_t DEFAULT_WIDTH = 3840; // 1920;
constexpr uint32_t DEFAULT_HEIGHT = 2160; // 1080;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::Media;
using namespace std;
using namespace OHOS::Media::VideoProcessingEngine;
using namespace OHOS::HDI::Display::Graphic::Common::V1_0;
namespace {

std::shared_ptr<ColorSpaceConverterVideo> cscv_ = nullptr;
sptr<Surface> outSurface_;
sptr<Surface> surface2;
BufferRequestConfig requestCfg_{};
sptr<SurfaceBuffer> outputBuffer = nullptr;
class CscVCB : public ColorSpaceConverterVideoCallback {
public:
    void OnError(int32_t errorCode) override;
    void OnState(int32_t state) override;
    void OnOutputBufferAvailable(uint32_t index, CscvBufferFlag flag) override;
    void OnOutputFormatChanged(const Format& format) override;
};

void CscVCB::OnOutputFormatChanged(const Format& format)
{
    (void)format;
}
void CscVCB::OnOutputBufferAvailable(uint32_t index, CscvBufferFlag flag)
{
    (void)flag;
    cscv_->ReleaseOutputBuffer(index, true);
}

void CscVCB::OnError(int32_t errorCode)
{
    (void)errorCode;
}

void CscVCB::OnState(int32_t state)
{
    (void)state;
}

constexpr CM_ColorSpaceInfo COLORSPACE_INFO_HDR_BT2020 = {
    COLORPRIMARIES_BT2020,  // 2
    TRANSFUNC_HLG,         // 1
    MATRIX_BT2020,          // 2
    RANGE_LIMITED            // 2
};


class CscvDemoTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp()
    {
        cscv_ = ColorSpaceConverterVideo::Create();
        cscvCb_ = std::make_shared<CscVCB>();
        format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_PIXEL_FORMAT, int(GRAPHIC_PIXEL_FMT_YCBCR_420_SP));
        format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_PRIMARIES, COLORPRIMARIES_BT709);
        format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_TRANS_FUNC, TRANSFUNC_BT709);
        format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_MATRIX, MATRIX_BT709);
        format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_RANGE, RANGE_LIMITED);
        format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_HDR_METADATA_TYPE, CM_METADATA_NONE);
        format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_RENDER_INTENT,
            int(RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC));
        format_.PutDoubleValue(CscVDescriptionKey::CSCV_KEY_SDRUI_BRIGHTNESS_RATIO, 0.0);
    };
    void TearDown()
    {
        if (cscv_)
            cscv_->Release();
    };
    sptr<Surface> surface;
    OHNativeWindow *nativeWindow;
    GSError SetMeatadata(sptr<SurfaceBuffer> &buffer, uint32_t value);
    GSError SetMeatadata(sptr<SurfaceBuffer> &buffer, CM_ColorSpaceInfo &colorspaceInfo);
    void OutputBufferAvailable(uint32_t index, CscvBufferFlag flag);
    GSError OnProducerBufferReleased();
    std::shared_ptr<CscVCB> cscvCb_ = nullptr;
    OHOS::Media::Format format_;
};


class ImageProcessBufferConsumerListener : public OHOS::IBufferConsumerListener {
public:
    void OnBufferAvailable() override;
};

void ImageProcessBufferConsumerListener::OnBufferAvailable()
{
    sptr<SyncFence> fence{nullptr};
    int64_t timestamp = 0;
    OHOS::Rect damage;
    outSurface_->AcquireBuffer(outputBuffer, fence, timestamp, damage);
}

GSError CscvDemoTest::SetMeatadata(sptr<SurfaceBuffer> &buffer, uint32_t value)
{
    std::vector<uint8_t> metadata;
    metadata.resize(sizeof(value));
    (void)memcpy_s(metadata.data(), metadata.size(), &value, sizeof(value));
    GSError err = buffer->SetMetadata(ATTRKEY_HDR_METADATA_TYPE, metadata);
    return err;
}

GSError CscvDemoTest::SetMeatadata(sptr<SurfaceBuffer> &buffer, CM_ColorSpaceInfo &colorspaceInfo)
{
    std::vector<uint8_t> metadata;
    metadata.resize(sizeof(CM_ColorSpaceInfo));
    (void)memcpy_s(metadata.data(), metadata.size(), &colorspaceInfo, sizeof(CM_ColorSpaceInfo));
    GSError err = buffer->SetMetadata(ATTRKEY_COLORSPACE_INFO, metadata);
    return err;
}

void CscvDemoTest::OutputBufferAvailable(uint32_t index, CscvBufferFlag flag)
{
    (void)flag;
    cscv_->ReleaseOutputBuffer(index, true);
}

GSError CscvDemoTest::OnProducerBufferReleased()
{
    sptr<SurfaceBuffer> buffertmp;
    sptr<SyncFence> fencetmp{nullptr};
    surface2->RequestBuffer(buffertmp, fencetmp, requestCfg_);
    return GSERROR_OK;
}

HWTEST_F(CscvDemoTest, DEMO_RUN_TEST, TestSize.Level1)
{
    BufferFlushConfig flushCfg_{};
    sptr<SurfaceBuffer> buffer;
    // input SDR
    CM_ColorSpaceInfo inColspcInfo = COLORSPACE_INFO_HDR_BT2020;
    CM_HDR_Metadata_Type inMetaType = CM_VIDEO_HDR_VIVID;
    GraphicPixelFormat surfacePixelFmt = GRAPHIC_PIXEL_FMT_YCBCR_P010;
    requestCfg_.format = surfacePixelFmt;
    requestCfg_.usage = BUFFER_USAGE_CPU_READ | BUFFER_USAGE_CPU_WRITE
        | BUFFER_USAGE_HW_RENDER | BUFFER_USAGE_HW_TEXTURE | BUFFER_USAGE_MEM_MMZ_CACHE;
    requestCfg_.width = DEFAULT_WIDTH;
    requestCfg_.height = DEFAULT_HEIGHT;
    requestCfg_.timeout = 0;
    requestCfg_.strideAlignment = 16;
    flushCfg_.damage.x = 0;
    flushCfg_.damage.y = 0;
    flushCfg_.damage.w = DEFAULT_WIDTH;
    flushCfg_.damage.h = DEFAULT_HEIGHT;
    flushCfg_.timestamp = 0;
    sptr<Surface> surface1 = cscv_->CreateInputSurface();
    outSurface_ = Surface::CreateSurfaceAsConsumer("ConvertInputSurface2");
    sptr<IBufferConsumerListener> listener = new ImageProcessBufferConsumerListener();
    outSurface_->RegisterConsumerListener(listener);
    sptr<IBufferProducer> producer = outSurface_->GetProducer();
    surface2 = Surface::CreateSurfaceAsProducer(producer);
    outSurface_->SetQueueSize(5);
    surface2->RegisterReleaseListener([this](sptr<SurfaceBuffer> &buffer) {
        (void)buffer;
        return OnProducerBufferReleased();
    });
    ASSERT_EQ(cscv_->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv_->SetOutputSurface(surface2), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv_->Configure(format_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv_->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv_->Start(), VPE_ALGO_ERR_OK);
    int frames = 5; // 5
    while (frames) {
        sptr<SyncFence> fence{nullptr};
        sptr<SurfaceBuffer> buffer2;
        surface1->RequestBuffer(buffer2, fence, requestCfg_);
        SetMeatadata(buffer2, inColspcInfo);
        SetMeatadata(buffer2, (uint32_t)inMetaType);
        surface1->FlushBuffer(buffer2, -1, flushCfg_);
        frames--;
        outSurface_->ReleaseBuffer(outputBuffer, -1);
        usleep(1 * 100 * 1000);
    }
    ASSERT_EQ(cscv_->NotifyEos(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv_->Stop(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv_->Release(), VPE_ALGO_ERR_OK);
}

}