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
#include "v1_0/cm_color_space.h"
#include "v1_0/hdr_static_metadata.h"
#include "algorithm_common.h"
#include "algorithm_errors.h"
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::Media;
using namespace OHOS::Media::VideoProcessingEngine;
using namespace OHOS::HDI::Display::Graphic::Common::V1_0;
using namespace std;
namespace {
constexpr int32_t NONEXIST_VAL = 1000;
class CscVCB : public ColorSpaceConverterVideoCallback {
public:
    void OnError(int32_t errorCode) override;
    void OnState(int32_t state) override;
    void OnOutputBufferAvailable(uint32_t index, CscvBufferFlag flag) override;
    void OnOutputFormatChanged(const Format& format) override;
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
void CscVCB::OnOutputFormatChanged(const Format& format)
{
    (void)format;
}
void CscVCB::OnState(int32_t state)
{
    (void)state;
}

class CSCVInnerFuncTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp()
    {
        cscv_ = ColorSpaceConverterVideo::Create();
        cscvCb_ = std::make_shared<CscVCB>();
        cscv_->SetCallback(cscvCb_);
        surface = cscv_->CreateInputSurface();
        outSurface = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
        sptr<IBufferProducer> producer = outSurface->GetProducer();
        sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
        cscv_->SetOutputSurface(producerSurface);
    };
    void TearDown()
    {
        cscv_->Stop();
        cscv_->Release();
    };
    void SetMeta(int32_t csPrimaries, int32_t csTransFunc, int32_t csMatrix, int32_t csRange, int32_t metaType);
    void SetMetaPixFmt(int32_t pixFmt);
    void SetMetaExtra();
    void SetRenderIntent();
    std::shared_ptr<ColorSpaceConverterVideo> cscv_ = nullptr;
    std::shared_ptr<CscVCB> cscvCb_ = nullptr;
    OHOS::Media::Format format_;
    sptr<Surface> surface;
    sptr<Surface> outSurface;
};

void CSCVInnerFuncTest::SetMeta(int32_t csPrimaries, int32_t csTransFunc,
    int32_t csMatrix, int32_t csRange, int32_t metaType)
{
    if (csPrimaries >= 0)
        format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_PRIMARIES, csPrimaries);
    if (csTransFunc >= 0)
        format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_TRANS_FUNC, csTransFunc);
    if (csMatrix >= 0)
        format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_MATRIX, csMatrix);
    if (csRange >= 0)
        format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_RANGE, csRange);
    if (metaType >= 0)
        format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_HDR_METADATA_TYPE, metaType);
}

void CSCVInnerFuncTest::SetMetaPixFmt(int32_t pixFmt)
{
    format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_PIXEL_FORMAT, pixFmt);
}

void CSCVInnerFuncTest::SetRenderIntent()
{
    format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_RENDER_INTENT,
        int(RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC));
}

void CSCVInnerFuncTest::SetMetaExtra()
{
    format_.PutDoubleValue(CscVDescriptionKey::CSCV_KEY_SDRUI_BRIGHTNESS_RATIO, 0.0);
}

/**
 * @tc.number    : CSCVInnerFuncTest_CSCV_FUNC_0010
 * @tc.name      : call Configure with non COLORPRIMARIES_BT709 value
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerFuncTest, CSCV_FUNC_0010, TestSize.Level2)
{
    SetMeta(COLORPRIMARIES_BT601_P, TRANSFUNC_BT709, MATRIX_BT709, RANGE_LIMITED, CM_METADATA_NONE);
    SetMetaPixFmt(int(GRAPHIC_PIXEL_FMT_YCBCR_420_SP));
    SetRenderIntent();
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Configure(format_));
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Prepare());
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Start());
}

/**
 * @tc.number    : CSCVInnerFuncTest_CSCV_FUNC_0020
 * @tc.name      : call Configure with non TRANSFUNC_BT709 value
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerFuncTest, CSCV_FUNC_0020, TestSize.Level2)
{
    SetMeta(COLORPRIMARIES_BT709, TRANSFUNC_HLG, MATRIX_BT709, RANGE_LIMITED, CM_METADATA_NONE);
    SetMetaPixFmt(int(GRAPHIC_PIXEL_FMT_YCBCR_420_SP));
    SetRenderIntent();
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Configure(format_));
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Prepare());
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Start());
}

/**
 * @tc.number    : CSCVInnerFuncTest_CSCV_FUNC_0030
 * @tc.name      : call Configure with non MATRIX_BT709 value
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerFuncTest, CSCV_FUNC_0030, TestSize.Level2)
{
    SetMeta(COLORPRIMARIES_BT709, TRANSFUNC_BT709, MATRIX_BT601_N, RANGE_LIMITED, CM_METADATA_NONE);
    SetMetaPixFmt(int(GRAPHIC_PIXEL_FMT_YCBCR_420_SP));
    SetRenderIntent();
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Configure(format_));
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Prepare());
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Start());
}

/**
 * @tc.number    : CSCVInnerFuncTest_CSCV_FUNC_0040
 * @tc.name      : call Configure with non RANGE_LIMITED value
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerFuncTest, CSCV_FUNC_0040, TestSize.Level2)
{
    SetMeta(COLORPRIMARIES_BT709, TRANSFUNC_BT709, MATRIX_BT709, RANGE_FULL, CM_METADATA_NONE);
    SetMetaPixFmt(int(GRAPHIC_PIXEL_FMT_YCBCR_420_SP));
    SetRenderIntent();
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Configure(format_));
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Prepare());
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Start());
}

/**
 * @tc.number    : CSCVInnerFuncTest_CSCV_FUNC_0050
 * @tc.name      : call Configure with non CM_METADATA_NONE value
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerFuncTest, CSCV_FUNC_0050, TestSize.Level2)
{
    SetMeta(COLORPRIMARIES_BT709, TRANSFUNC_BT709, MATRIX_BT709, RANGE_LIMITED, CM_VIDEO_HDR_VIVID);
    SetMetaPixFmt(int(GRAPHIC_PIXEL_FMT_YCBCR_420_SP));
    SetRenderIntent();
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Configure(format_));
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Prepare());
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Start());
}

/**
 * @tc.number    : CSCVInnerFuncTest_CSCV_FUNC_0060
 * @tc.name      : call Configure with non NV12 pix_fmt value
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerFuncTest, CSCV_FUNC_0060, TestSize.Level2)
{
    SetMeta(COLORPRIMARIES_BT709, TRANSFUNC_BT709, MATRIX_BT709, RANGE_LIMITED, CM_METADATA_NONE);
    SetMetaPixFmt(GRAPHIC_PIXEL_FMT_BGRA_8888);
    SetRenderIntent();
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Configure(format_));
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Prepare());
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Start());
}

/**
 * @tc.number    : CSCVInnerFuncTest_CSCV_FUNC_0070
 * @tc.name      : call Configure with nonexist value
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerFuncTest, CSCV_FUNC_0070, TestSize.Level2)
{
    SetMeta(NONEXIST_VAL, TRANSFUNC_BT709, MATRIX_BT709, RANGE_LIMITED, CM_METADATA_NONE);
    SetMetaPixFmt(int(GRAPHIC_PIXEL_FMT_YCBCR_420_SP));
    SetRenderIntent();
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Configure(format_));
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Prepare());
}

/**
 * @tc.number    : CSCVInnerFuncTest_CSCV_FUNC_0080
 * @tc.name      : Configure with BT709 value and check getParameter return value
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerFuncTest, CSCV_FUNC_0080, TestSize.Level1)
{
    SetMeta(COLORPRIMARIES_BT709, TRANSFUNC_BT709, MATRIX_BT709, RANGE_LIMITED, CM_METADATA_NONE);
    SetMetaPixFmt(int(GRAPHIC_PIXEL_FMT_YCBCR_420_SP));
    SetRenderIntent();
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Configure(format_));
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Prepare());
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Start());
    OHOS::Media::Format ret_meta;
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->GetParameter(ret_meta));
    int32_t primaries = 0;
    ASSERT_EQ(true, ret_meta.GetIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_PRIMARIES, primaries));
    ASSERT_EQ(COLORPRIMARIES_BT709, primaries);
    int32_t transFunc = 0;
    ASSERT_EQ(true, ret_meta.GetIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_TRANS_FUNC, transFunc));
    ASSERT_EQ(TRANSFUNC_BT709, transFunc);
    int32_t matrix = 0;
    ASSERT_EQ(true, ret_meta.GetIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_MATRIX, matrix));
    ASSERT_EQ(MATRIX_BT709, matrix);
    int32_t range = 0;
    ASSERT_EQ(true, ret_meta.GetIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_RANGE, range));
    ASSERT_EQ(RANGE_LIMITED, range);
    int32_t metaVal = 0;
    ASSERT_EQ(true, ret_meta.GetIntValue(CscVDescriptionKey::CSCV_KEY_HDR_METADATA_TYPE, metaVal));
    ASSERT_EQ(CM_METADATA_NONE, metaVal);
}

/**
 * @tc.number    : CSCVInnerFuncTest_CSCV_FUNC_0090
 * @tc.name      : Configure with BT709 value, skip metatype, and check getParameter return value
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerFuncTest, CSCV_FUNC_0090, TestSize.Level1)
{
    SetMeta(COLORPRIMARIES_BT709, TRANSFUNC_BT709, MATRIX_BT709, RANGE_LIMITED, -1);
    SetMetaPixFmt(int(GRAPHIC_PIXEL_FMT_YCBCR_420_SP));
    SetRenderIntent();
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Configure(format_));
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Prepare());
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->Start());
    OHOS::Media::Format retMeta;
    ASSERT_EQ(VPE_ALGO_ERR_OK, cscv_->GetParameter(retMeta));
    int32_t primaries = 0;
    ASSERT_EQ(true, retMeta.GetIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_PRIMARIES, primaries));
    ASSERT_EQ(COLORPRIMARIES_BT709, primaries);
    int32_t transFunc = 0;
    ASSERT_EQ(true, retMeta.GetIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_TRANS_FUNC, transFunc));
    ASSERT_EQ(TRANSFUNC_BT709, transFunc);
    int32_t matrix = 0;
    ASSERT_EQ(true, retMeta.GetIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_MATRIX, matrix));
    ASSERT_EQ(MATRIX_BT709, matrix);
    int32_t range = 0;
    ASSERT_EQ(true, retMeta.GetIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_RANGE, range));
    ASSERT_EQ(RANGE_LIMITED, range);
    int32_t metaVal = 0;
    ASSERT_EQ(false, retMeta.GetIntValue(CscVDescriptionKey::CSCV_KEY_HDR_METADATA_TYPE, metaVal));
}

/**
 * @tc.number    : CSCVInnerFuncTest_CSCV_FUNC_0100
 * @tc.name      : Configure with BT709 value ,skip colorspace
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerFuncTest, CSCV_FUNC_0100, TestSize.Level1)
{
    SetMeta(-1, TRANSFUNC_BT709, MATRIX_BT709, RANGE_LIMITED, CM_METADATA_NONE);
    SetMetaPixFmt(int(GRAPHIC_PIXEL_FMT_YCBCR_420_SP));
    SetRenderIntent();
    ASSERT_NE(VPE_ALGO_ERR_OK, cscv_->Configure(format_));
}

/**
 * @tc.number    : CSCVInnerFuncTest_CSCV_FUNC_0110
 * @tc.name      : Configure with BT709 value ,skip all colorspace
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerFuncTest, CSCV_FUNC_0110, TestSize.Level1)
{
    SetMeta(-1, -1, -1, -1, CM_METADATA_NONE);
    SetMetaPixFmt(int(GRAPHIC_PIXEL_FMT_YCBCR_420_SP));
    SetRenderIntent();
    ASSERT_NE(VPE_ALGO_ERR_OK, cscv_->Configure(format_));
}

/**
 * @tc.number    : CSCVInnerFuncTest_CSCV_FUNC_0120
 * @tc.name      : Configure with BT709 value ,skip pixel format
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerFuncTest, CSCV_FUNC_0120, TestSize.Level1)
{
    SetMeta(COLORPRIMARIES_BT709, TRANSFUNC_BT709, MATRIX_BT709, RANGE_LIMITED, CM_METADATA_NONE);
    SetRenderIntent();
    ASSERT_NE(VPE_ALGO_ERR_OK, cscv_->Configure(format_));
}

/**
 * @tc.number    : CSCVInnerFuncTest_CSCV_FUNC_0130
 * @tc.name      : Configure with BT709 value ,skip transfunc/matrix/range
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerFuncTest, CSCV_FUNC_0130, TestSize.Level1)
{
    SetMeta(COLORPRIMARIES_BT709, -1, -1, -1, CM_METADATA_NONE);
    SetMetaPixFmt(int(GRAPHIC_PIXEL_FMT_YCBCR_420_SP));
    SetRenderIntent();
    ASSERT_NE(VPE_ALGO_ERR_OK, cscv_->Configure(format_));
}

/**
 * @tc.number    : CSCVInnerFuncTest_CSCV_FUNC_0140
 * @tc.name      : Configure with BT709 value ,skip matrix/range
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerFuncTest, CSCV_FUNC_0140, TestSize.Level1)
{
    SetMeta(COLORPRIMARIES_BT709, TRANSFUNC_BT709, -1, -1, CM_METADATA_NONE);
    SetMetaPixFmt(int(GRAPHIC_PIXEL_FMT_YCBCR_420_SP));
    SetRenderIntent();
    ASSERT_NE(VPE_ALGO_ERR_OK, cscv_->Configure(format_));
}

/**
 * @tc.number    : CSCVInnerFuncTest_CSCV_FUNC_0150
 * @tc.name      : Meta test
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerFuncTest, CSCV_FUNC_0150, TestSize.Level1)
{
    SetMeta(COLORPRIMARIES_BT709, TRANSFUNC_BT709, -1, -1, CM_METADATA_NONE);
    SetMetaPixFmt(int(GRAPHIC_PIXEL_FMT_YCBCR_420_SP));
    ASSERT_NE(VPE_ALGO_ERR_OK, cscv_->Configure(format_));
}

/**
 * @tc.number    : CSCVInnerFuncTest_CSCV_FUNC_0160
 * @tc.name      : Meta test
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerFuncTest, CSCV_FUNC_0160, TestSize.Level1)
{
    SetMeta(COLORPRIMARIES_BT709, TRANSFUNC_BT709, MATRIX_BT709, -1, CM_METADATA_NONE);
    SetMetaPixFmt(int(GRAPHIC_PIXEL_FMT_YCBCR_420_SP));
    SetRenderIntent();
    ASSERT_NE(VPE_ALGO_ERR_OK, cscv_->Configure(format_));
}

/**
 * @tc.number    : CSCV_FUNC_0170
 * @tc.name      : Meta test
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerFuncTest, CSCV_FUNC_0170, TestSize.Level1)
{
    SetMeta(COLORPRIMARIES_BT709, TRANSFUNC_BT709, MATRIX_BT709, RANGE_LIMITED, CM_METADATA_NONE);
    SetMetaExtra();
    ASSERT_NE(VPE_ALGO_ERR_OK, cscv_->Configure(format_));
}

/**
 * @tc.number    : CSCV_FUNC_0180
 * @tc.name      : Meta test
 * @tc.desc      : function test
 */
HWTEST_F(CSCVInnerFuncTest, CSCV_FUNC_0180, TestSize.Level1)
{
    SetMeta(COLORPRIMARIES_BT709, TRANSFUNC_BT709, MATRIX_BT709, RANGE_LIMITED, CM_METADATA_NONE);
    SetMetaExtra();
    SetMetaPixFmt(int(GRAPHIC_PIXEL_FMT_YCBCR_420_SP));
    ASSERT_NE(VPE_ALGO_ERR_OK, cscv_->Configure(format_));
}
}