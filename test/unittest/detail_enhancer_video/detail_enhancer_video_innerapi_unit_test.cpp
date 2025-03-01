/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <memory>
#include <string>

#include <dlfcn.h>

#include <gtest/gtest.h>

#include "external_window.h"
#include "surface/window.h"

#include "algorithm_errors.h"
#include "algorithm_utils.h"
#include "algorithm_video.h"

using namespace testing::ext;

namespace {
constexpr int64_t NANOS_IN_SECOND = 1000000000L;
constexpr int64_t NANOS_IN_MICRO = 1000L;
constexpr uint32_t DEFAULT_WIDTH = 1920;
constexpr uint32_t DEFAULT_HEIGHT = 1080;

int64_t GetSystemTime()
{
    struct timespec now;
    clock_gettime(CLOCK_BOOTTIME, &now);
    int64_t nanoTime = reinterpret_cast<int64_t>(now.tv_sec) * NANOS_IN_SECOND + now.tv_nsec;

    return nanoTime / NANOS_IN_MICRO;
}
} // namespace

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class DetailEnhancerVideoCallbackImpl : public VpeVideoCallback {
public:
    DetailEnhancerVideoCallbackImpl() = default;
    ~DetailEnhancerVideoCallbackImpl() override = default;
    DetailEnhancerVideoCallbackImpl(const DetailEnhancerVideoCallbackImpl&) = delete;
    DetailEnhancerVideoCallbackImpl& operator=(const DetailEnhancerVideoCallbackImpl&) = delete;
    DetailEnhancerVideoCallbackImpl(DetailEnhancerVideoCallbackImpl&&) = delete;
    DetailEnhancerVideoCallbackImpl& operator=(DetailEnhancerVideoCallbackImpl&&) = delete;

    void OnError(VPEAlgoErrCode errorCode) final;
    void OnState(VPEAlgoState state) final;
    void OnEffectChange(uint32_t type) final;
    void OnOutputFormatChanged(const Format& format) final;
    void OnOutputBufferAvailable(uint32_t index, VpeBufferFlag flag) final;
    void OnOutputBufferAvailable(uint32_t index, const VpeBufferInfo& info) final;
};

void DetailEnhancerVideoCallbackImpl::OnError(VPEAlgoErrCode errorCode)
{
    std::cout << "OnError:" << AlgorithmUtils::ToString(errorCode) << std::endl;
}

void DetailEnhancerVideoCallbackImpl::OnState(VPEAlgoState state)
{
    std::cout << "OnState:" << AlgorithmUtils::ToString(state) << std::endl;
}

void DetailEnhancerVideoCallbackImpl::OnEffectChange(uint32_t type)
{
    std::cout << "OnEffectChange:0x" << std::hex << type << std::endl;
}

void DetailEnhancerVideoCallbackImpl::OnOutputFormatChanged(const Format& format)
{
    std::cout << "OnOutputFormatChanged:" << format.Stringify() << std::endl;
}

void DetailEnhancerVideoCallbackImpl::OnOutputBufferAvailable(uint32_t index, VpeBufferFlag flag)
{
    std::cout << "OnOutputBufferAvailable: index=" << index << " flag=" << flag << std::endl;
}

void DetailEnhancerVideoCallbackImpl::OnOutputBufferAvailable(uint32_t index, const VpeBufferInfo& info)
{
    std::cout << "OnOutputBufferAvailable: index=" << index << " flag=" << info.flag <<
        " pts=" << info.presentationTimestamp << std::endl;
}

class DetailEnhancerVideoInnerAPIUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);

    void SetUp();
    void TearDown();

protected:
    uint32_t FlushSurf(OHNativeWindowBuffer* ohNativeWindowBuffer);

    sptr<Surface> surface{};
    OHNativeWindow* nativeWindow{};
};

void DetailEnhancerVideoInnerAPIUnitTest::SetUpTestCase(void)
{
    std::cout << "[SetUpTestCase]: " << std::endl;
}

void DetailEnhancerVideoInnerAPIUnitTest::TearDownTestCase(void)
{
    std::cout << "[TearDownTestCase]: " << std::endl;
}

void DetailEnhancerVideoInnerAPIUnitTest::SetUp(void)
{
    std::cout << "[SetUp]: SetUp!!!" << std::endl;
}

void DetailEnhancerVideoInnerAPIUnitTest::TearDown(void)
{
    std::cout << "[TearDown]: over!!!" << std::endl;
}

uint32_t DetailEnhancerVideoInnerAPIUnitTest::FlushSurf(OHNativeWindowBuffer* ohNativeWindowBuffer)
{
    struct Region region;
    struct Region::Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = DEFAULT_WIDTH;
    rect.h = DEFAULT_HEIGHT;
    region.rects = &rect;
    NativeWindowHandleOpt(nativeWindow, SET_UI_TIMESTAMP, GetSystemTime());
    int32_t err = OH_NativeWindow_NativeWindowFlushBuffer(nativeWindow, ohNativeWindowBuffer, -1, region);
    if (err != 0) {
        std::cout << "FlushBuffer failed" << std::endl;
        return 1;
    }
    return 0;
}

// detail enhancer init
HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_init_01, TestSize.Level1)
{
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    EXPECT_NE(detailEnh, nullptr);
}

HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_02, TestSize.Level1)
{
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    std::shared_ptr<VpeVideoCallback> cb = nullptr;
    auto ret = detailEnh->RegisterCallback(cb);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_03, TestSize.Level1)
{
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    Format param{};
    EXPECT_EQ(param.PutIntValue(ParameterKey::DETAIL_ENHANCER_QUALITY_LEVEL, DETAIL_ENHANCER_LEVEL_HIGH), true);
    EXPECT_EQ(detailEnh->SetParameter(param), VPE_ALGO_ERR_OK);
}

HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_04, TestSize.Level1)
{
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    Format param{};
    EXPECT_EQ(param.PutIntValue(ParameterKey::DETAIL_ENHANCER_QUALITY_LEVEL, DETAIL_ENHANCER_LEVEL_MEDIUM), true);
    EXPECT_EQ(detailEnh->SetParameter(param), VPE_ALGO_ERR_OK);
}

HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_05, TestSize.Level1)
{
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    Format param{};
    EXPECT_EQ(param.PutIntValue(ParameterKey::DETAIL_ENHANCER_QUALITY_LEVEL, DETAIL_ENHANCER_LEVEL_LOW), true);
    EXPECT_EQ(detailEnh->SetParameter(param), VPE_ALGO_ERR_OK);
}

HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_06, TestSize.Level1)
{
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    auto ret = detailEnh->GetInputSurface();
    EXPECT_NE(ret, nullptr);
}

HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_07, TestSize.Level1)
{
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    auto ret = detailEnh->GetInputSurface();
    ret = detailEnh->GetInputSurface();
    EXPECT_EQ(ret, nullptr);
}

HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_08, TestSize.Level1)
{
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    sptr<Surface> surface = nullptr;
    auto ret = detailEnh->SetOutputSurface(surface);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_09, TestSize.Level1)
{
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    auto ret = detailEnh->ReleaseOutputBuffer(0, true);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_10, TestSize.Level1)
{
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    int64_t renderTimestamp = 0;
    auto ret = detailEnh->RenderOutputBufferAtTime(0, renderTimestamp);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// set parameter to midium
HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_11, TestSize.Level1)
{
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    Format param{};
    EXPECT_EQ(param.PutIntValue(ParameterKey::DETAIL_ENHANCER_QUALITY_LEVEL, DETAIL_ENHANCER_LEVEL_MEDIUM), true);
    EXPECT_EQ(detailEnh->SetParameter(param), VPE_ALGO_ERR_OK);
}

// set parameter to low
HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_12, TestSize.Level1)
{
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    Format param{};
    EXPECT_EQ(param.PutIntValue(ParameterKey::DETAIL_ENHANCER_QUALITY_LEVEL, DETAIL_ENHANCER_LEVEL_LOW), true);
    EXPECT_EQ(detailEnh->SetParameter(param), VPE_ALGO_ERR_OK);
}

// set parameter to none
HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_13, TestSize.Level1)
{
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    Format param{};
    EXPECT_EQ(param.PutIntValue(ParameterKey::DETAIL_ENHANCER_QUALITY_LEVEL, DETAIL_ENHANCER_LEVEL_NONE), true);
    EXPECT_EQ(detailEnh->SetParameter(param), VPE_ALGO_ERR_OK);
}

HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_14, TestSize.Level1)
{
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    Format param{};
    EXPECT_EQ(param.PutIntValue(ParameterKey::DETAIL_ENHANCER_QUALITY_LEVEL, DETAIL_ENHANCER_LEVEL_HIGH), true);
    EXPECT_EQ(detailEnh->SetParameter(param), VPE_ALGO_ERR_OK);
}

HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_15, TestSize.Level1)
{
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    std::shared_ptr<VpeVideoCallback> cb = std::make_shared<DetailEnhancerVideoCallbackImpl>();
    auto res = detailEnh->RegisterCallback(cb);
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);
}

HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_16, TestSize.Level1)
{
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    std::shared_ptr<VpeVideoCallback> cb = std::make_shared<DetailEnhancerVideoCallbackImpl>();
    EXPECT_EQ(detailEnh->RegisterCallback(cb), VPE_ALGO_ERR_OK);
    Format param{};
    EXPECT_EQ(param.PutIntValue(ParameterKey::DETAIL_ENHANCER_QUALITY_LEVEL, DETAIL_ENHANCER_LEVEL_HIGH), true);
    EXPECT_EQ(detailEnh->SetParameter(param), VPE_ALGO_ERR_OK);
    EXPECT_NE(detailEnh->GetInputSurface(), nullptr);
    EXPECT_NE(detailEnh->Start(), VPE_ALGO_ERR_OK);
}

HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_17, TestSize.Level1)
{
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    auto res = detailEnh->Stop();
    EXPECT_NE(res, VPE_ALGO_ERR_OK);
}

HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_18, TestSize.Level1)
{
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    std::shared_ptr<VpeVideoCallback> cb = std::make_shared<DetailEnhancerVideoCallbackImpl>();
    EXPECT_EQ(detailEnh->RegisterCallback(cb), VPE_ALGO_ERR_OK);
    Format param{};
    EXPECT_EQ(param.PutIntValue(ParameterKey::DETAIL_ENHANCER_QUALITY_LEVEL, DETAIL_ENHANCER_LEVEL_HIGH), true);
    EXPECT_EQ(detailEnh->SetParameter(param), VPE_ALGO_ERR_OK);
    auto surface1 = detailEnh->GetInputSurface();
    EXPECT_EQ(detailEnh->SetOutputSurface(surface1), VPE_ALGO_ERR_OK);
    EXPECT_EQ(detailEnh->Start(), VPE_ALGO_ERR_OK);
}

HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_19, TestSize.Level1)
{
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    std::shared_ptr<VpeVideoCallback> cb = std::make_shared<DetailEnhancerVideoCallbackImpl>();
    EXPECT_EQ(detailEnh->RegisterCallback(cb), VPE_ALGO_ERR_OK);
    Format param{};
    EXPECT_EQ(param.PutIntValue(ParameterKey::DETAIL_ENHANCER_QUALITY_LEVEL, DETAIL_ENHANCER_LEVEL_HIGH), true);
    EXPECT_EQ(detailEnh->SetParameter(param), VPE_ALGO_ERR_OK);
    auto surface1 = detailEnh->GetInputSurface();
    EXPECT_EQ(detailEnh->SetOutputSurface(surface1), VPE_ALGO_ERR_OK);
    EXPECT_EQ(detailEnh->Start(), VPE_ALGO_ERR_OK);
    EXPECT_NE(detailEnh->ReleaseOutputBuffer(100, true), VPE_ALGO_ERR_OK);
}

HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_20, TestSize.Level1)
{
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    std::shared_ptr<VpeVideoCallback> cb = std::make_shared<DetailEnhancerVideoCallbackImpl>();
    EXPECT_EQ(detailEnh->RegisterCallback(cb), VPE_ALGO_ERR_OK);
    Format param{};
    EXPECT_EQ(param.PutIntValue(ParameterKey::DETAIL_ENHANCER_QUALITY_LEVEL, DETAIL_ENHANCER_LEVEL_HIGH), true);
    EXPECT_EQ(detailEnh->SetParameter(param), VPE_ALGO_ERR_OK);
    auto surface1 = detailEnh->GetInputSurface();
    EXPECT_EQ(detailEnh->SetOutputSurface(surface1), VPE_ALGO_ERR_OK);
    EXPECT_EQ(detailEnh->Start(), VPE_ALGO_ERR_OK);
    EXPECT_NE(detailEnh->RenderOutputBufferAtTime(100, 0), VPE_ALGO_ERR_OK);
}

HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_21, TestSize.Level1)
{
    OHNativeWindowBuffer* ohNativeWindowBuffer;
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    std::shared_ptr<VpeVideoCallback> cb = std::make_shared<DetailEnhancerVideoCallbackImpl>();
    EXPECT_EQ(detailEnh->RegisterCallback(cb), VPE_ALGO_ERR_OK);
    Format param{};
    EXPECT_EQ(param.PutIntValue(ParameterKey::DETAIL_ENHANCER_QUALITY_LEVEL, DETAIL_ENHANCER_LEVEL_HIGH), true);
    EXPECT_EQ(detailEnh->SetParameter(param), VPE_ALGO_ERR_OK);
    auto surface = detailEnh->GetInputSurface();
    auto detailEnh2 = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    auto surface2 = detailEnh2->GetInputSurface();
    EXPECT_EQ(detailEnh->SetOutputSurface(surface2), VPE_ALGO_ERR_OK);
    EXPECT_EQ(detailEnh->Start(), VPE_ALGO_ERR_OK);

    int fenceFd = -1;
    nativeWindow = CreateNativeWindowFromSurface(&surface);
    auto ret = OH_NativeWindow_NativeWindowHandleOpt(nativeWindow, SET_FORMAT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = OH_NativeWindow_NativeWindowHandleOpt(nativeWindow, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = OH_NativeWindow_NativeWindowRequestBuffer(nativeWindow, &ohNativeWindowBuffer, &fenceFd);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = FlushSurf(ohNativeWindowBuffer);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    OH_NativeWindow_DestroyNativeWindow(nativeWindow);
}

HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_22, TestSize.Level1)
{
    OHNativeWindowBuffer* ohNativeWindowBuffer;
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    std::shared_ptr<VpeVideoCallback> cb = std::make_shared<DetailEnhancerVideoCallbackImpl>();
    EXPECT_EQ(detailEnh->RegisterCallback(cb), VPE_ALGO_ERR_OK);
    Format param{};
    EXPECT_EQ(param.PutIntValue(ParameterKey::DETAIL_ENHANCER_QUALITY_LEVEL, DETAIL_ENHANCER_LEVEL_HIGH), true);
    EXPECT_EQ(detailEnh->SetParameter(param), VPE_ALGO_ERR_OK);
    auto surface = detailEnh->GetInputSurface();
    auto detailEnh2 = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    auto surface2 = detailEnh2->GetInputSurface();
    surface2->SetRequestWidthAndHeight(10, 10);
    EXPECT_EQ(detailEnh->SetOutputSurface(surface2), VPE_ALGO_ERR_OK);
    EXPECT_EQ(detailEnh->Start(), VPE_ALGO_ERR_OK);

    int fenceFd = -1;
    nativeWindow = CreateNativeWindowFromSurface(&surface);
    auto ret = OH_NativeWindow_NativeWindowHandleOpt(nativeWindow, SET_FORMAT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = OH_NativeWindow_NativeWindowHandleOpt(nativeWindow, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = OH_NativeWindow_NativeWindowRequestBuffer(nativeWindow, &ohNativeWindowBuffer, &fenceFd);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = FlushSurf(ohNativeWindowBuffer);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    OH_NativeWindow_DestroyNativeWindow(nativeWindow);
}

HWTEST_F(DetailEnhancerVideoInnerAPIUnitTest, detailenhancer_23, TestSize.Level1)
{
    OHNativeWindowBuffer* ohNativeWindowBuffer;
    auto detailEnh = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    std::shared_ptr<VpeVideoCallback> cb = std::make_shared<DetailEnhancerVideoCallbackImpl>();
    EXPECT_EQ(detailEnh->RegisterCallback(cb), VPE_ALGO_ERR_OK);
    Format param{};
    EXPECT_EQ(param.PutIntValue(ParameterKey::DETAIL_ENHANCER_QUALITY_LEVEL, DETAIL_ENHANCER_LEVEL_HIGH), true);
    EXPECT_EQ(detailEnh->SetParameter(param), VPE_ALGO_ERR_OK);
    auto surface = detailEnh->GetInputSurface();
    auto detailEnh2 = VpeVideo::Create(VIDEO_TYPE_DETAIL_ENHANCER);
    auto surface2 = detailEnh2->GetInputSurface();
    surface2->SetRequestWidthAndHeight(10, 0);
    EXPECT_EQ(detailEnh->SetOutputSurface(surface2), VPE_ALGO_ERR_OK);
    EXPECT_EQ(detailEnh->Start(), VPE_ALGO_ERR_OK);

    int fenceFd = -1;
    nativeWindow = CreateNativeWindowFromSurface(&surface);
    auto ret = OH_NativeWindow_NativeWindowHandleOpt(nativeWindow, SET_FORMAT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = OH_NativeWindow_NativeWindowHandleOpt(nativeWindow, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = OH_NativeWindow_NativeWindowRequestBuffer(nativeWindow, &ohNativeWindowBuffer, &fenceFd);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = FlushSurf(ohNativeWindowBuffer);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    OH_NativeWindow_DestroyNativeWindow(nativeWindow);
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
