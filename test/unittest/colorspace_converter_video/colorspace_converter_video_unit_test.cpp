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
 
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <dlfcn.h>
#include <fstream>
#include <memory>
#include <string>
#include <gtest/gtest.h>
 
#include "algorithm_common.h"
#include "algorithm_errors.h"
#include "colorspace_converter_video_impl.h"
#include "colorspace_converter_video.h"
#include "colorspace_converter_video_description.h"
 
using namespace std;
using namespace testing::ext;
 
namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

struct CallbackT : public ColorSpaceConverterVideoCallback {
    void OnError(int32_t errorCode) override
    {
        (void)errorCode;
    }

    void OnState(int32_t state) override
    {
        (void)state;
    }

    void OnOutputBufferAvailable(uint32_t index, CscvBufferFlag flag) override
    {
        (void)index;
        (void)flag;
    }
};
 
class ColorSpaceConverterVideoUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp(void) {};
    void TearDown(void) {};
};
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_init_01, TestSize.Level1)
{
    auto handle = ColorSpaceConvertVideoCreate();
    EXPECT_NE(nullptr, handle);
    ColorSpaceConvertVideoDestroy(handle);
    ColorSpaceConvertVideoDestroy(nullptr);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_setcallback_01, TestSize.Level1)
{
    auto handle = ColorSpaceConvertVideoCreate();
    ColorSpaceConverterVideoCallbackImpl::Callback cb;
    auto ret = ColorSpaceConvertVideoSetCallback(handle, &cb, nullptr);
    ColorSpaceConvertVideoDestroy(handle);
    handle = nullptr;
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_setcallback_02, TestSize.Level1)
{
    auto handle = ColorSpaceConvertVideoCreate();
    auto ret = ColorSpaceConvertVideoSetCallback(handle, nullptr, nullptr);
    ColorSpaceConvertVideoDestroy(handle);
    handle = nullptr;
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_setcallback_03, TestSize.Level1)
{
    ColorSpaceConverterVideoCallbackImpl::Callback cb;
    auto ret = ColorSpaceConvertVideoSetCallback(nullptr, &cb, nullptr);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_setcallback_04, TestSize.Level1)
{
    auto ret = ColorSpaceConvertVideoSetCallback(nullptr, nullptr, nullptr);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_setcallback_05, TestSize.Level1)
{
    auto handle = ColorSpaceConvertVideoCreate();
    ColorSpaceConverterVideoCallbackImpl::Callback cb;
    ColorSpaceConverterVideoCallbackImpl::Callback* cbp = &cb;
    if (handle && cbp) {
        auto ret = ColorSpaceConvertVideoSetCallback(handle, cbp, nullptr);
        EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
    }
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_setoutputsurface_02, TestSize.Level1)
{
    auto handle = ColorSpaceConvertVideoCreate();
    auto ret = ColorSpaceConvertVideoSetOutputSurface(handle, nullptr);
    ColorSpaceConvertVideoDestroy(handle);
    handle = nullptr;
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_setoutputsurface_03, TestSize.Level1)
{
    sptr<Surface> surface = nullptr;
    sptr<Surface> userData = nullptr;
    auto sfp = reinterpret_cast<void*>(&surface);
    auto ret = ColorSpaceConvertVideoSetOutputSurface(nullptr, sfp);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_setoutputsurface_04, TestSize.Level1)
{
    auto ret = ColorSpaceConvertVideoSetOutputSurface(nullptr, nullptr);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_setoutputsurface_05, TestSize.Level1)
{
    sptr<Surface> surface = nullptr;
    auto handle = ColorSpaceConvertVideoCreate();
    auto sfp = reinterpret_cast<void*>(&surface);
    auto ret = ColorSpaceConvertVideoSetOutputSurface(handle, sfp);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_createinputsurface_01, TestSize.Level1)
{
    auto handle = ColorSpaceConvertVideoCreate();
    sptr<Surface> surface = nullptr;
    auto sfp = reinterpret_cast<void*>(&surface);
    auto ret = ColorSpaceConvertVideoCreateInputSurface(handle, sfp);
    ColorSpaceConvertVideoDestroy(handle);
    handle = nullptr;
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_createinputsurface_02, TestSize.Level1)
{
    sptr<Surface> surface = nullptr;
    auto sfp = reinterpret_cast<void*>(&surface);
    auto ret = ColorSpaceConvertVideoCreateInputSurface(nullptr, sfp);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_configure_01, TestSize.Level1)
{
    auto handle = ColorSpaceConvertVideoCreate();
    Format parameter;
    parameter.PutIntValue(CscVDescriptionKey::CSCV_KEY_PIXEL_FORMAT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP);
    parameter.PutIntValue(CscVDescriptionKey::CSCV_KEY_HDR_METADATA_TYPE, CM_METADATA_NONE);
    parameter.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_RANGE, RANGE_LIMITED);
    parameter.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_MATRIX, MATRIX_BT709);
    parameter.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_TRANS_FUNC, TRANSFUNC_BT709);
    parameter.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_PRIMARIES, COLORPRIMARIES_BT709);
    parameter.PutIntValue(CscVDescriptionKey::CSCV_KEY_RENDER_INTENT,
        static_cast<int32_t>(RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC));
    parameter.PutDoubleValue(CscVDescriptionKey::CSCV_KEY_SDRUI_BRIGHTNESS_RATIO, 0.6);
    auto parameterP = reinterpret_cast<void*>(&parameter);
    auto ret = ColorSpaceConvertVideoConfigure(handle, parameterP);
    ColorSpaceConvertVideoDestroy(handle);
    handle = nullptr;
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_configure_02, TestSize.Level1)
{
    auto ret = ColorSpaceConvertVideoConfigure(nullptr, nullptr);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_prepare_02, TestSize.Level1)
{
    auto ret = ColorSpaceConvertVideoPrepare(nullptr);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_setparameter_02, TestSize.Level1)
{
    auto handle = ColorSpaceConvertVideoCreate();
 
    sptr<Surface> inSurface = nullptr;
    auto inSfp = reinterpret_cast<void*>(&inSurface);
    auto ret = ColorSpaceConvertVideoCreateInputSurface(handle, inSfp);
 
    auto outSurfaceP = reinterpret_cast<sptr<Surface>*>(inSfp);
    auto outSurface = *outSurfaceP;
    auto producer = outSurface->GetProducer();
    auto producerSurface = Surface::CreateSurfaceAsProducer(producer);
    auto outSfp = reinterpret_cast<void*>(&producerSurface);
    ret = ColorSpaceConvertVideoSetOutputSurface(handle, outSfp);
 
    Format parameter;
    parameter.PutIntValue(CscVDescriptionKey::CSCV_KEY_PIXEL_FORMAT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP);
    parameter.PutIntValue(CscVDescriptionKey::CSCV_KEY_HDR_METADATA_TYPE, CM_METADATA_NONE);
    parameter.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_RANGE, RANGE_LIMITED);
    parameter.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_MATRIX, MATRIX_BT709);
    parameter.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_TRANS_FUNC, TRANSFUNC_BT709);
    parameter.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_PRIMARIES, COLORPRIMARIES_BT709);
    auto parameterP = reinterpret_cast<void*>(&parameter);
    ret = ColorSpaceConvertVideoConfigure(handle, parameterP);
    ret = ColorSpaceConvertVideoPrepare(handle);
 
    ret = ColorSpaceConvertVideoSetParameter(handle, nullptr);
    ColorSpaceConvertVideoDestroy(handle);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_setparameter_03, TestSize.Level1)
{
    Format parameter;
    parameter.PutIntValue(CscVDescriptionKey::CSCV_KEY_RENDER_INTENT,
        static_cast<int32_t>(RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC));
    parameter.PutDoubleValue(CscVDescriptionKey::CSCV_KEY_SDRUI_BRIGHTNESS_RATIO, 0.6);
    auto parameterP = reinterpret_cast<void*>(&parameter);
    auto ret = ColorSpaceConvertVideoSetParameter(nullptr, parameterP);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_setparameter_04, TestSize.Level1)
{
    auto ret = ColorSpaceConvertVideoSetParameter(nullptr, nullptr);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_getparameter_02, TestSize.Level1)
{
    auto ret = ColorSpaceConvertVideoGetParameter(nullptr, nullptr);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_getparameter_03, TestSize.Level1)
{
    auto handle = ColorSpaceConvertVideoCreate();
    auto ret = ColorSpaceConvertVideoGetParameter(handle, nullptr);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_reset_01, TestSize.Level1)
{
    auto handle = ColorSpaceConvertVideoCreate();
    if (handle) {
        auto ret = ColorSpaceConvertVideoReset(handle);
        ColorSpaceConvertVideoDestroy(handle);
        EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
    }
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_reset_02, TestSize.Level1)
{
    auto ret = ColorSpaceConvertVideoReset(nullptr);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_release_01, TestSize.Level1)
{
    auto handle = ColorSpaceConvertVideoCreate();
    if (handle) {
        auto ret = ColorSpaceConvertVideoRelease(handle);
        EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
    }
    ColorSpaceConvertVideoDestroy(handle);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_release_02, TestSize.Level1)
{
    auto ret = ColorSpaceConvertVideoRelease(nullptr);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_notifyEos_01, TestSize.Level1)
{
    auto handle = ColorSpaceConvertVideoCreate();
    auto ret = ColorSpaceConvertVideoNotifyEos(handle);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
    ColorSpaceConvertVideoDestroy(handle);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_notifyEos_02, TestSize.Level1)
{
    auto ret = ColorSpaceConvertVideoNotifyEos(nullptr);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}
 
HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_releaseOutputBuffer_01, TestSize.Level1)
{
    auto ret = ColorSpaceConvertVideoReleaseOutputBuffer(nullptr, 0, 0);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_start_01, TestSize.Level1)
{
    auto handle = ColorSpaceConvertVideoCreate();
    auto ret = ColorSpaceConvertVideoStart(handle);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_start_02, TestSize.Level1)
{
    auto ret = ColorSpaceConvertVideoStart(nullptr);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_stop_01, TestSize.Level1)
{
    auto handle = ColorSpaceConvertVideoCreate();
    auto ret = ColorSpaceConvertVideoStop(handle);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_stop_02, TestSize.Level1)
{
    auto ret = ColorSpaceConvertVideoStop(nullptr);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_releaseOutputBuffer_02, TestSize.Level1)
{
    auto handle = ColorSpaceConvertVideoCreate();
    auto ret = ColorSpaceConvertVideoReleaseOutputBuffer(handle, 0, 0);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_flush_01, TestSize.Level1)
{
    auto handle = ColorSpaceConvertVideoCreate();
    if (handle) {
        auto ret = ColorSpaceConvertVideoFlush(handle);
        EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
    }
}

HWTEST_F(ColorSpaceConverterVideoUnitTest, cscv_flush_02, TestSize.Level1)
{
    auto ret = ColorSpaceConvertVideoFlush(nullptr);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS