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
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "colorspace_converter_video.h"
#include "media_description.h"
#include "colorspace_converter_video_description.h"
#include "colorspace_converter_video_common.h"
#include "v1_0/cm_color_space.h"
#include "v1_0/hdr_static_metadata.h"
#include "algorithm_common.h"
#include "algorithm_errors.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::Media;
using namespace Media::VideoProcessingEngine;
using namespace OHOS::HDI::Display::Graphic::Common::V1_0;
using namespace std;
namespace {
OHOS::Media::Format format;
const std::string DEFAULT_FILE = "/data/test/media/1080p_Vivid.mp4";

enum CSCV_API_NAME {
    SETCALLBACK = 1,
    CREATEINPUTSURFACE,
    SETOUTPUTSURFACE,
    CONFIGURE,
};

uint32_t g_errorCount = 0;
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
    g_errorCount++;
}
void CscVCB::OnOutputFormatChanged(const Format& formatOutput)
{
    (void)formatOutput;
}
void CscVCB::OnState(int32_t state)
{
    (void)state;
}

class CSCVInnerStateTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        format.PutIntValue(CscVDescriptionKey::CSCV_KEY_PIXEL_FORMAT, int(GRAPHIC_PIXEL_FMT_YCBCR_420_SP));
        format.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_PRIMARIES, COLORPRIMARIES_BT709);
        format.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_TRANS_FUNC, TRANSFUNC_BT709);
        format.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_MATRIX, MATRIX_BT709);
        format.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_RANGE, RANGE_LIMITED);
        format.PutIntValue(CscVDescriptionKey::CSCV_KEY_HDR_METADATA_TYPE, CM_METADATA_NONE);
        format.PutIntValue(CscVDescriptionKey::CSCV_KEY_RENDER_INTENT,
            int(RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC));
    };
    static void TearDownTestCase(void) {};
    void SetUp()
    {
        cscv = ColorSpaceConverterVideo::Create();
        cscvCb_ = std::make_shared<CscVCB>();
        g_errorCount = 0;
    };
    void TearDown()
    {
        if (cscv) {
            cscv->Release();
            cscv = nullptr;
        }
    };

    void PrepareFunc(int num);
    void AllPrepareFunc();

    std::shared_ptr<CscVCB> cscvCb_ = nullptr;
    std::shared_ptr<ColorSpaceConverterVideo> cscv = nullptr;
};

void CSCVInnerStateTest::PrepareFunc(int num)
{
    int32_t ret = 0;
    std::shared_ptr<ColorSpaceConverterVideo> cscv2;
    sptr<Surface> surface2;
    sptr<Surface> surface;
    switch (num) {
        case SETCALLBACK:
            ret = cscv->SetCallback(cscvCb_);
            ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
            break;
        case CREATEINPUTSURFACE:
            surface = cscv->CreateInputSurface();
            ASSERT_NE(surface, nullptr);
            break;
        case SETOUTPUTSURFACE:
            cscv2 = ColorSpaceConverterVideo::Create();
            surface2 = cscv2->CreateInputSurface();
            ret = cscv->SetOutputSurface(surface2);
            ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
            break;
        case CONFIGURE:
            ret = cscv->Configure(format);
            ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
            break;
        default:
            break;
    }
}

void CSCVInnerStateTest::AllPrepareFunc()
{
    int32_t ret = 0;
    std::shared_ptr<ColorSpaceConverterVideo> cscv2;
    sptr<Surface> surface2;
    sptr<Surface> surface;
    ret = cscv->SetCallback(cscvCb_);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    surface = cscv->CreateInputSurface();
    ASSERT_NE(surface, nullptr);
    cscv2 = ColorSpaceConverterVideo::Create();
    surface2 = cscv2->CreateInputSurface();
    ret = cscv->SetOutputSurface(surface2);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->Configure(format);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0010
 * @tc.name      : call all combination of prepare-state func
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0010, TestSize.Level1)
{
    std::vector<int> nums = {1, 2, 3, 4};
    int32_t ret = 0;
    do {
        ret = cscv->Release();
        ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
        cscv = nullptr;
        cscv = ColorSpaceConverterVideo::Create();
        ASSERT_NE(cscv, nullptr);
        for (int num:nums)
            PrepareFunc(num);
        ret = cscv->Prepare();
        ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
        ret = cscv->Start();
        ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
        ret = cscv->NotifyEos();
        ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    } while (std::next_permutation(nums.begin(), nums.end()));
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0020
 * @tc.name      : call start func without fully prepare(configure)
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0020, TestSize.Level2)
{
    int32_t ret = 0;
    sptr<Surface> surface = cscv->CreateInputSurface();
    std::shared_ptr<ColorSpaceConverterVideo> cscv2 = ColorSpaceConverterVideo::Create();
    ASSERT_NE(cscv2, nullptr);
    sptr<Surface> surface2 = cscv2->CreateInputSurface();
    ret = cscv->SetOutputSurface(surface2);
    ASSERT_NE(surface, nullptr);
    ret = cscv->SetCallback(cscvCb_);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_OPERATION);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0030
 * @tc.name      : call start func without fully prepare(SetCallback)
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0030, TestSize.Level2)
{
    int32_t ret = 0;
    std::shared_ptr<ColorSpaceConverterVideo> cscv2 = ColorSpaceConverterVideo::Create();
    ASSERT_NE(cscv2, nullptr);
    sptr<Surface> surface2 = cscv2->CreateInputSurface();
    ret = cscv->SetOutputSurface(surface2);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    sptr<Surface> surface = cscv->CreateInputSurface();
    ASSERT_NE(surface, nullptr);
    ret = cscv->Configure(format);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_OPERATION);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0040
 * @tc.name      : call start func without fully prepare(SetOutputSurface)
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0040, TestSize.Level2)
{
    int32_t ret = 0;
    sptr<Surface> surface = cscv->CreateInputSurface();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->Configure(format);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->SetCallback(cscvCb_);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_OPERATION);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0050
 * @tc.name      : call start func without fully prepare(CreateInputSurface)
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0050, TestSize.Level2)
{
    int32_t ret = 0;
    std::shared_ptr<ColorSpaceConverterVideo> cscv2 = ColorSpaceConverterVideo::Create();
    sptr<Surface> surface2 = cscv2->CreateInputSurface();
    ret = cscv->SetOutputSurface(surface2);
    ret = cscv->Configure(format);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->SetCallback(cscvCb_);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_OPERATION);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0060
 * @tc.name      : call prepare/start/stop/notifyeos without initialize
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0060, TestSize.Level2)
{
    int32_t ret = 0;
    ret = cscv->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = cscv->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = cscv->Stop();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = cscv->NotifyEos();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0070
 * @tc.name      : repeat call CreateInputSurface
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0070, TestSize.Level1)
{
    ASSERT_NE(cscv->CreateInputSurface(), nullptr);
    ASSERT_EQ(cscv->CreateInputSurface(), nullptr);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0080
 * @tc.name      : repeat call SetOutputSurface
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0080, TestSize.Level1)
{
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(cscv->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0090
 * @tc.name      : repeat call SetCallback
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0090, TestSize.Level1)
{
    ASSERT_EQ(cscv->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0100
 * @tc.name      : repeat call Configure with different value
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0100, TestSize.Level1)
{
    sptr<Surface> surface = cscv->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv->SetOutputSurface(producerSurface));

    OHOS::Media::Format format2, format3;
    format2.PutIntValue(CscVDescriptionKey::CSCV_KEY_PIXEL_FORMAT, int(GRAPHIC_PIXEL_FMT_YCBCR_420_SP));
    format2.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_PRIMARIES, COLORPRIMARIES_BT601_P);
    format2.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_TRANS_FUNC, TRANSFUNC_BT709);
    format2.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_MATRIX, MATRIX_BT709);
    format2.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_RANGE, RANGE_LIMITED);
    format2.PutIntValue(CscVDescriptionKey::CSCV_KEY_HDR_METADATA_TYPE, CM_METADATA_NONE);
    format2.PutIntValue(CscVDescriptionKey::CSCV_KEY_RENDER_INTENT,
        int(RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC));
    format3.PutIntValue(CscVDescriptionKey::CSCV_KEY_PIXEL_FORMAT, int(GRAPHIC_PIXEL_FMT_YCBCR_420_SP));
    format3.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_PRIMARIES, COLORPRIMARIES_BT709);
    format3.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_TRANS_FUNC, TRANSFUNC_HLG);
    format3.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_MATRIX, MATRIX_BT709);
    format3.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_RANGE, RANGE_LIMITED);
    format3.PutIntValue(CscVDescriptionKey::CSCV_KEY_HDR_METADATA_TYPE, CM_METADATA_NONE);
    format3.PutIntValue(CscVDescriptionKey::CSCV_KEY_RENDER_INTENT,
        int(RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC));

    ASSERT_EQ(cscv->Configure(format2), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Configure(format3), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Configure(format), VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0110
 * @tc.name      : call configure -> start -> stop -> eos
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0110, TestSize.Level2)
{
    int32_t ret = 0;
    AllPrepareFunc();
    ret = cscv->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = cscv->Stop();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = cscv->NotifyEos();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0120
 * @tc.name      : call configure -> reset -> configure -> prepare -> start ->eos ->release
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0120, TestSize.Level1)
{
    int32_t ret = 0;
    AllPrepareFunc();
    ret = cscv->Reset();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->Configure(format);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->NotifyEos();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->Release();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0130
 * @tc.name      : call configure -> reset -> reset ->release
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0130, TestSize.Level1)
{
    int32_t ret = 0;
    AllPrepareFunc();
    ret = cscv->Reset();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->Reset();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->Release();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0140
 * @tc.name      : call configure -> reset ->release
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0140, TestSize.Level1)
{
    int32_t ret = 0;
    AllPrepareFunc();
    ret = cscv->Reset();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->Release();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0150
 * @tc.name      : call configure -> reset -> CreateInputSurface ->
                   SetOutputSurface -> SetCallback -> configure -> prepare ->start -> eos ->release
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0150, TestSize.Level1)
{
    int32_t ret = 0;
    AllPrepareFunc();
    ret = cscv->Reset();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    auto surface = cscv->CreateInputSurface();
    ASSERT_EQ(surface, nullptr);
    std::shared_ptr<ColorSpaceConverterVideo> cscv2 = ColorSpaceConverterVideo::Create();
    sptr<Surface> surface2 = cscv2->CreateInputSurface();
    ret = cscv->SetOutputSurface(surface2);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->SetCallback(cscvCb_);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->Configure(format);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->NotifyEos();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->Release();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ASSERT_EQ(g_errorCount, 0);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0160
 * @tc.name      : call configure -> reset -> prepare -> start -> stop -> eos
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0160, TestSize.Level2)
{
    int32_t ret = 0;
    AllPrepareFunc();
    ret = cscv->Reset();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = cscv->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = cscv->Stop();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = cscv->NotifyEos();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0170
 * @tc.name      : call configure -> release ->(prepare-reset)
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0170, TestSize.Level2)
{
    int32_t ret = 0;
    AllPrepareFunc();
    ret = cscv->Release();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = cscv->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = cscv->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = cscv->Stop();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = cscv->NotifyEos();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = cscv->Reset();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0180
 * @tc.name      : repeat call start/stop in processing
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0180, TestSize.Level1)
{
    sptr<Surface> surface = cscv->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(cscv->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Configure(format), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(cscv->Stop(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(cscv->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(cscv->Stop(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(cscv->Start(), VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0190
 * @tc.name      : configure -> prepare -> start -> stop -> release
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0190, TestSize.Level1)
{
    sptr<Surface> surface = cscv->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(cscv->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Configure(format), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(cscv->Stop(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Release(), VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0200
 * @tc.name      : configure -> prepare -> start -> stop -> reset -> release
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0200, TestSize.Level1)
{
    sptr<Surface> surface = cscv->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(cscv->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Configure(format), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(cscv->Stop(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Reset(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Release(), VPE_ALGO_ERR_OK);
    cscv = nullptr;
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0210
 * @tc.name      : configure -> prepare -> start -> stop -> (error)configure/prepare/stop/eos
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0210, TestSize.Level2)
{
    sptr<Surface> surface = cscv->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(cscv->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Configure(format), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(cscv->Stop(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(cscv->Stop(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->SetOutputSurface(producerSurface), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(cscv->SetCallback(cscvCb_), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(cscv->Configure(format), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Stop(), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(cscv->NotifyEos(), VPE_ALGO_ERR_INVALID_STATE);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0220
 * @tc.name      : configure -> prepare -> start -> eos -> release
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0220, TestSize.Level1)
{
    sptr<Surface> surface = cscv->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(cscv->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Configure(format), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(cscv->NotifyEos(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Release(), VPE_ALGO_ERR_OK);
    cscv = nullptr;
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0230
 * @tc.name      : configure -> prepare -> start -> eos -> reset -> release
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0230, TestSize.Level1)
{
    sptr<Surface> surface = cscv->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(cscv->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Configure(format), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(cscv->NotifyEos(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Reset(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Release(), VPE_ALGO_ERR_OK);
    cscv = nullptr;
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0240
 * @tc.name      : configure -> prepare -> start -> eos -> (error)configure/prepare/start/stop/eos
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0240, TestSize.Level2)
{
    sptr<Surface> surface = cscv->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(cscv->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Configure(format), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(cscv->NotifyEos(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->SetCallback(cscvCb_), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(cscv->Configure(format), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(cscv->Prepare(), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(cscv->Start(), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(cscv->Stop(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->NotifyEos(), VPE_ALGO_ERR_INVALID_STATE);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0250
 * @tc.name      : configure -> prepare -> start -> release
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0250, TestSize.Level1)
{
    sptr<Surface> surface = cscv->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(cscv->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Configure(format), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(cscv->Release(), VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0260
 * @tc.name      : configure -> prepare -> start -> reset ->release
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0260, TestSize.Level1)
{
    sptr<Surface> surface = cscv->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(cscv->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Configure(format), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(cscv->Reset(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Release(), VPE_ALGO_ERR_OK);
    cscv = nullptr;
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0270
 * @tc.name      : configure -> prepare -> start -> configure
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0270, TestSize.Level1)
{
    sptr<Surface> surface = cscv->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(cscv->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Configure(format), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(cscv->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->SetCallback(cscvCb_), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(cscv->Configure(format), VPE_ALGO_ERR_INVALID_STATE);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0280
 * @tc.name      : configure -> prepare -> (error)configure/stop/eos/reset/release
 * @tc.desc      : state test
 */
HWTEST_F(CSCVInnerStateTest, CSCV_STATE_0280, TestSize.Level2)
{
    sptr<Surface> surface = cscv->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(cscv->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Configure(format), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Start(), VPE_ALGO_ERR_OK);

    ASSERT_EQ(cscv->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->SetCallback(cscvCb_), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(cscv->Configure(format), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(cscv->Stop(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->NotifyEos(), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(cscv->Reset(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(cscv->Release(), VPE_ALGO_ERR_OK);
    cscv = nullptr;
}

}