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
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "metadata_generator_video.h"
#include "metadata_generator_video_common.h"
#include "algorithm_common.h"
#include "algorithm_errors.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::Media;
using namespace Media::VideoProcessingEngine;
using namespace OHOS::HDI::Display::Graphic::Common::V1_0;
using namespace std;
namespace {
const std::string DEFAULT_FILE = "/data/test/media/1080p_Vivid.mp4";

enum CSCV_API_NAME {
    SETCALLBACK = 1,
    CREATEINPUTSURFACE,
    SETOUTPUTSURFACE,
    CONFIGURE,
};

uint32_t g_errorCount = 0;
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
    g_errorCount++;
}
void CscVCB::OnState(int32_t state)
{
    (void)state;
}

class MDGInnerStateTest : public testing::Test {
public:
    static void TearDownTestCase(void) {};
    void SetUp()
    {
        mdg = MetadataGeneratorVideo::Create();
        cscvCb_ = std::make_shared<CscVCB>();
        g_errorCount = 0;
    };
    void TearDown()
    {
        if (mdg) {
            mdg->Release();
            mdg = nullptr;
        }
    };

    void PrepareFunc(int num);
    void AllPrepareFunc();

    std::shared_ptr<CscVCB> cscvCb_ = nullptr;
    std::shared_ptr<MetadataGeneratorVideo> mdg = nullptr;
};

void MDGInnerStateTest::PrepareFunc(int num)
{
    int32_t ret = 0;
    std::shared_ptr<MetadataGeneratorVideo> mdg2;
    sptr<Surface> surface2;
    sptr<Surface> surface;
    switch (num) {
        case SETCALLBACK:
            ret = mdg->SetCallback(cscvCb_);
            ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
            break;
        case CREATEINPUTSURFACE:
            surface = mdg->CreateInputSurface();
            ASSERT_NE(surface, nullptr);
            break;
        case SETOUTPUTSURFACE:
            mdg2 = MetadataGeneratorVideo::Create();
            surface2 = mdg2->CreateInputSurface();
            ret = mdg->SetOutputSurface(surface2);
            ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
            break;
        case CONFIGURE:
            ret = mdg->Configure();
            ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
            break;
        default:
            break;
    }
}

void MDGInnerStateTest::AllPrepareFunc()
{
    int32_t ret = 0;
    std::shared_ptr<MetadataGeneratorVideo> mdg2;
    sptr<Surface> surface2;
    sptr<Surface> surface;
    ret = mdg->SetCallback(cscvCb_);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    surface = mdg->CreateInputSurface();
    ASSERT_NE(surface, nullptr);
    mdg2 = MetadataGeneratorVideo::Create();
    surface2 = mdg2->CreateInputSurface();
    ret = mdg->SetOutputSurface(surface2);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->Configure();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0010
 * @tc.name      : call all combination of prepare-state func
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0010, TestSize.Level1)
{
    std::vector<int> nums = {1, 2, 3, 4};
    int32_t ret = 0;
    do {
        ret = mdg->Release();
        ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
        mdg = nullptr;
        mdg = MetadataGeneratorVideo::Create();
        ASSERT_NE(mdg, nullptr);
        for (int num:nums)
            PrepareFunc(num);
        ret = mdg->Prepare();
        ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
        ret = mdg->Start();
        ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
        ret = mdg->NotifyEos();
        ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    } while (std::next_permutation(nums.begin(), nums.end()));
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0030
 * @tc.name      : call start func without fully prepare(SetCallback)
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0030, TestSize.Level2)
{
    int32_t ret = 0;
    std::shared_ptr<MetadataGeneratorVideo> mdg2 = MetadataGeneratorVideo::Create();
    ASSERT_NE(mdg2, nullptr);
    sptr<Surface> surface2 = mdg2->CreateInputSurface();
    ret = mdg->SetOutputSurface(surface2);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    sptr<Surface> surface = mdg->CreateInputSurface();
    ASSERT_NE(surface, nullptr);
    ret = mdg->Configure();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_OPERATION);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0040
 * @tc.name      : call start func without fully prepare(SetOutputSurface)
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0040, TestSize.Level2)
{
    int32_t ret = 0;
    sptr<Surface> surface = mdg->CreateInputSurface();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->Configure();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->SetCallback(cscvCb_);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_OPERATION);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0050
 * @tc.name      : call start func without fully prepare(CreateInputSurface)
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0050, TestSize.Level2)
{
    int32_t ret = 0;
    std::shared_ptr<MetadataGeneratorVideo> mdg2 = MetadataGeneratorVideo::Create();
    sptr<Surface> surface2 = mdg2->CreateInputSurface();
    ret = mdg->SetOutputSurface(surface2);
    ret = mdg->Configure();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->SetCallback(cscvCb_);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_OPERATION);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0060
 * @tc.name      : call prepare/start/stop/notifyeos without initialize
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0060, TestSize.Level2)
{
    int32_t ret = 0;
    ret = mdg->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = mdg->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = mdg->Stop();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = mdg->NotifyEos();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0070
 * @tc.name      : repeat call CreateInputSurface
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0070, TestSize.Level1)
{
    ASSERT_NE(mdg->CreateInputSurface(), nullptr);
    ASSERT_EQ(mdg->CreateInputSurface(), nullptr);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0080
 * @tc.name      : repeat call SetOutputSurface
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0080, TestSize.Level1)
{
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(mdg->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0090
 * @tc.name      : repeat call SetCallback
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0090, TestSize.Level1)
{
    ASSERT_EQ(mdg->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0100
 * @tc.name      : repeat call Configure with different value
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0100, TestSize.Level1)
{
    sptr<Surface> surface = mdg->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(VPE_ALGO_ERR_OK, mdg->SetOutputSurface(producerSurface));
    ASSERT_EQ(mdg->Configure(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Configure(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Configure(), VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0110
 * @tc.name      : call configure -> start -> stop -> eos
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0110, TestSize.Level2)
{
    int32_t ret = 0;
    AllPrepareFunc();
    ret = mdg->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = mdg->Stop();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = mdg->NotifyEos();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0120
 * @tc.name      : call configure -> reset -> configure -> prepare -> start ->eos ->release
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0120, TestSize.Level1)
{
    int32_t ret = 0;
    AllPrepareFunc();
    ret = mdg->Reset();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->Configure();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->NotifyEos();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->Release();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0130
 * @tc.name      : call configure -> reset -> reset ->release
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0130, TestSize.Level1)
{
    int32_t ret = 0;
    AllPrepareFunc();
    ret = mdg->Reset();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->Reset();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->Release();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0140
 * @tc.name      : call configure -> reset ->release
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0140, TestSize.Level1)
{
    int32_t ret = 0;
    AllPrepareFunc();
    ret = mdg->Reset();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->Release();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0150
 * @tc.name      : call configure -> reset -> CreateInputSurface ->
                   SetOutputSurface -> SetCallback -> configure -> prepare ->start -> eos ->release
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0150, TestSize.Level1)
{
    int32_t ret = 0;
    AllPrepareFunc();
    ret = mdg->Reset();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    auto surface = mdg->CreateInputSurface();
    ASSERT_EQ(surface, nullptr);
    std::shared_ptr<MetadataGeneratorVideo> mdg2 = MetadataGeneratorVideo::Create();
    sptr<Surface> surface2 = mdg2->CreateInputSurface();
    ret = mdg->SetOutputSurface(surface2);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->SetCallback(cscvCb_);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->Configure();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->NotifyEos();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->Release();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ASSERT_EQ(g_errorCount, 0);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0160
 * @tc.name      : call configure -> reset -> prepare -> start -> stop -> eos
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0160, TestSize.Level2)
{
    int32_t ret = 0;
    AllPrepareFunc();
    ret = mdg->Reset();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = mdg->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = mdg->Stop();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = mdg->NotifyEos();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0170
 * @tc.name      : call configure -> release ->(prepare-reset)
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0170, TestSize.Level2)
{
    int32_t ret = 0;
    AllPrepareFunc();
    ret = mdg->Release();
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = mdg->Prepare();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = mdg->Start();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = mdg->Stop();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = mdg->NotifyEos();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
    ret = mdg->Reset();
    ASSERT_EQ(ret, VPE_ALGO_ERR_INVALID_STATE);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0180
 * @tc.name      : repeat call start/stop in processing
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0180, TestSize.Level1)
{
    sptr<Surface> surface = mdg->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(mdg->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Configure(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(mdg->Stop(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(mdg->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(mdg->Stop(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(mdg->Start(), VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0190
 * @tc.name      : configure -> prepare -> start -> stop -> release
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0190, TestSize.Level1)
{
    sptr<Surface> surface = mdg->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(mdg->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Configure(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(mdg->Stop(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Release(), VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0200
 * @tc.name      : configure -> prepare -> start -> stop -> reset -> release
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0200, TestSize.Level1)
{
    sptr<Surface> surface = mdg->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(mdg->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Configure(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(mdg->Stop(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Reset(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Release(), VPE_ALGO_ERR_OK);
    mdg = nullptr;
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0210
 * @tc.name      : configure -> prepare -> start -> stop -> (error)configure/prepare/stop/eos
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0210, TestSize.Level2)
{
    sptr<Surface> surface = mdg->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(mdg->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Configure(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(mdg->Stop(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(mdg->Stop(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->SetOutputSurface(producerSurface), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(mdg->SetCallback(cscvCb_), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(mdg->Configure(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Stop(), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(mdg->NotifyEos(), VPE_ALGO_ERR_INVALID_STATE);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0220
 * @tc.name      : configure -> prepare -> start -> eos -> release
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0220, TestSize.Level1)
{
    sptr<Surface> surface = mdg->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(mdg->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Configure(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(mdg->NotifyEos(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Release(), VPE_ALGO_ERR_OK);
    mdg = nullptr;
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0230
 * @tc.name      : configure -> prepare -> start -> eos -> reset -> release
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0230, TestSize.Level1)
{
    sptr<Surface> surface = mdg->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(mdg->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Configure(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(mdg->NotifyEos(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Reset(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Release(), VPE_ALGO_ERR_OK);
    mdg = nullptr;
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0240
 * @tc.name      : configure -> prepare -> start -> eos -> (error)configure/prepare/start/stop/eos
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0240, TestSize.Level2)
{
    sptr<Surface> surface = mdg->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(mdg->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Configure(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(mdg->NotifyEos(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->SetCallback(cscvCb_), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(mdg->Configure(), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(mdg->Prepare(), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(mdg->Start(), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(mdg->Stop(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->NotifyEos(), VPE_ALGO_ERR_INVALID_STATE);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0250
 * @tc.name      : configure -> prepare -> start -> release
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0250, TestSize.Level1)
{
    sptr<Surface> surface = mdg->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(mdg->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Configure(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(mdg->Release(), VPE_ALGO_ERR_OK);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0260
 * @tc.name      : configure -> prepare -> start -> reset ->release
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0260, TestSize.Level1)
{
    sptr<Surface> surface = mdg->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(mdg->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Configure(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(mdg->Reset(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Release(), VPE_ALGO_ERR_OK);
    mdg = nullptr;
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0270
 * @tc.name      : configure -> prepare -> start -> configure
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0270, TestSize.Level1)
{
    sptr<Surface> surface = mdg->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(mdg->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Configure(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Start(), VPE_ALGO_ERR_OK);
    sleep(1);
    ASSERT_EQ(mdg->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->SetCallback(cscvCb_), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(mdg->Configure(), VPE_ALGO_ERR_INVALID_STATE);
}

/**
 * @tc.number    : CSCVInnerStateTest_CSCV_STATE_0280
 * @tc.name      : configure -> prepare -> (error)configure/stop/eos/reset/release
 * @tc.desc      : state test
 */
HWTEST_F(MDGInnerStateTest, CSCV_STATE_0280, TestSize.Level2)
{
    sptr<Surface> surface = mdg->CreateInputSurface();
    sptr<Surface> outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    sptr<IBufferProducer> producer = outSurface->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    ASSERT_EQ(mdg->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->SetCallback(cscvCb_), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Configure(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Prepare(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Start(), VPE_ALGO_ERR_OK);

    ASSERT_EQ(mdg->SetOutputSurface(producerSurface), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->SetCallback(cscvCb_), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(mdg->Configure(), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(mdg->Stop(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->NotifyEos(), VPE_ALGO_ERR_INVALID_STATE);
    ASSERT_EQ(mdg->Reset(), VPE_ALGO_ERR_OK);
    ASSERT_EQ(mdg->Release(), VPE_ALGO_ERR_OK);
    mdg = nullptr;
}

}