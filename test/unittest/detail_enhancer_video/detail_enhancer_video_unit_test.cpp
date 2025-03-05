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

#include "detail_enhancer_video_impl.h"
#include "detail_enhancer_video.h"
#include "surface/window.h"
#include "external_window.h"

constexpr int64_t NANOS_IN_SECOND = 1000000000L;
constexpr int64_t NANOS_IN_MICRO = 1000L;
constexpr uint32_t DEFAULT_WIDTH = 1920;
constexpr uint32_t DEFAULT_HEIGHT = 1080;

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

class DetailEnhancerVideoCallbackImpl : public DetailEnhancerVideoCallback {
public:
    DetailEnhancerVideoCallbackImpl() = default;
    ~DetailEnhancerVideoCallbackImpl() override = default;
    DetailEnhancerVideoCallbackImpl(const DetailEnhancerVideoCallbackImpl&) = delete;
    DetailEnhancerVideoCallbackImpl& operator=(const DetailEnhancerVideoCallbackImpl&) = delete;
    DetailEnhancerVideoCallbackImpl(DetailEnhancerVideoCallbackImpl&&) = delete;
    DetailEnhancerVideoCallbackImpl& operator=(DetailEnhancerVideoCallbackImpl&&) = delete;
 
    void OnError(VPEAlgoErrCode errorCode) override;
    void OnState(VPEAlgoState state) override;
    void OnOutputBufferAvailable(uint32_t index, DetailEnhBufferFlag flag) override;
};
void DetailEnhancerVideoCallbackImpl::OnOutputBufferAvailable(uint32_t index, DetailEnhBufferFlag flag)
{
    switch (flag) {
        case DETAIL_ENH_BUFFER_FLAG_NONE:
            std::cout << "OnOutputBufferAvailable: normal" << std::endl;
            break;
        case DETAIL_ENH_BUFFER_FLAG_EOS:
            std::cout << "OnOutputBufferAvailable: end of stream" << std::endl;
            break;
        default:
            std::cout << "OnOutputBufferAvailable: unknown" << std::endl;
            break;
    }
}
void DetailEnhancerVideoCallbackImpl::OnError(VPEAlgoErrCode errorCode)
{
    (void)errorCode;
}
void DetailEnhancerVideoCallbackImpl::OnState(VPEAlgoState state)
{
    (void)state;
}

class DetailEnhancerVideoUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<Surface> surface;
    OHNativeWindow *nativeWindow;
    uint32_t FlushSurf(OHNativeWindowBuffer *ohNativeWindowBuffer);
};

void DetailEnhancerVideoUnitTest::SetUpTestCase(void)
{
    cout << "[SetUpTestCase]: " << endl;
}

void DetailEnhancerVideoUnitTest::TearDownTestCase(void)
{
    cout << "[TearDownTestCase]: " << endl;
}

void DetailEnhancerVideoUnitTest::SetUp(void)
{
    cout << "[SetUp]: SetUp!!!" << endl;
}

void DetailEnhancerVideoUnitTest::TearDown(void)
{
    cout << "[TearDown]: over!!!" << endl;
}

int64_t GetSystemTime()
{
    struct timespec now;
    (void)clock_gettime(CLOCK_BOOTTIME, &now);
    int64_t nanoTime = reinterpret_cast<int64_t>(now.tv_sec) * NANOS_IN_SECOND + now.tv_nsec;

    return nanoTime / NANOS_IN_MICRO;
}

uint32_t DetailEnhancerVideoUnitTest::FlushSurf(OHNativeWindowBuffer *ohNativeWindowBuffer)
{
    struct Region region;
    struct Region::Rect *rect = new Region::Rect();
    rect->x = 0;
    rect->y = 0;
    rect->w = DEFAULT_WIDTH;
    rect->h = DEFAULT_HEIGHT;
    region.rects = rect;
    NativeWindowHandleOpt(nativeWindow, SET_UI_TIMESTAMP, GetSystemTime());
    int32_t err = OH_NativeWindow_NativeWindowFlushBuffer(nativeWindow, ohNativeWindowBuffer, -1, region);
    delete rect;
    if (err != 0) {
        cout << "FlushBuffer failed" << endl;
        return 1;
    }
    return 0;
}

// detail enhancer init
HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_init_01, TestSize.Level1)
{
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    EXPECT_NE(detailEnhVideo, nullptr);
    detailEnhVideo->Release();
}

HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_02, TestSize.Level1)
{
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    std::shared_ptr<DetailEnhancerVideoCallback> cb = nullptr;
    auto ret = detailEnhVideo->RegisterCallback(cb);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
    detailEnhVideo->Release();
}

HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_03, TestSize.Level1)
{
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    auto ret = detailEnhVideo->SetParameter(param, VIDEO);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
    detailEnhVideo->Release();
}

HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_04, TestSize.Level1)
{
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    auto ret = detailEnhVideo->SetParameter(param, VIDEO);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
    detailEnhVideo->Release();
}

HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_05, TestSize.Level1)
{
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    auto ret = detailEnhVideo->SetParameter(param, VIDEO);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
    detailEnhVideo->Release();
}

HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_06, TestSize.Level1)
{
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    auto ret = detailEnhVideo->GetInputSurface();
    EXPECT_NE(ret, nullptr);
    detailEnhVideo->Release();
}

HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_07, TestSize.Level1)
{
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    auto ret = detailEnhVideo->GetInputSurface();
    ret = detailEnhVideo->GetInputSurface();
    EXPECT_EQ(ret, nullptr);
    detailEnhVideo->Release();
}

HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_08, TestSize.Level1)
{
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    sptr<Surface> surface = nullptr;
    auto ret = detailEnhVideo->SetOutputSurface(surface);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
    detailEnhVideo->Release();
}

HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_09, TestSize.Level1)
{
    auto detailEnhVideo = DetailEnhancerVideo::Create();

    auto ret = detailEnhVideo->RenderOutputBuffer(0);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
    detailEnhVideo->Release();
}

HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_10, TestSize.Level1)
{
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    auto ret = detailEnhVideo->NotifyEos();
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
    detailEnhVideo->Release();
}

// set parameter to midium
HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_11, TestSize.Level1)
{
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_MEDIUM,
    };
    auto res = detailEnhVideo->SetParameter(param, VIDEO);
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);
    detailEnhVideo->Release();
}

// set parameter to low
HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_12, TestSize.Level1)
{
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    auto res = detailEnhVideo->SetParameter(param, VIDEO);
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);
    detailEnhVideo->Release();
}

// set parameter to none
HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_13, TestSize.Level1)
{
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_NONE,
    };
    auto res = detailEnhVideo->SetParameter(param, VIDEO);
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);
    detailEnhVideo->Release();
}

HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_14, TestSize.Level1)
{
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    auto res = detailEnhVideo->SetParameter(param, VIDEO);
    res = detailEnhVideo->SetParameter(param, VIDEO);
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);
    detailEnhVideo->Release();
}

HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_15, TestSize.Level1)
{
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    std::shared_ptr<DetailEnhancerVideoCallback> cb = std::make_shared<DetailEnhancerVideoCallbackImpl>();
    auto res = detailEnhVideo->RegisterCallback(cb);
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);
    detailEnhVideo->Release();
}


HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_16, TestSize.Level1)
{
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    std::shared_ptr<DetailEnhancerVideoCallback> cb = std::make_shared<DetailEnhancerVideoCallbackImpl>();
    auto res = detailEnhVideo->RegisterCallback(cb);
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    res = detailEnhVideo->SetParameter(param, VIDEO);
    auto ret = detailEnhVideo->GetInputSurface();
    res = detailEnhVideo->Start();
    EXPECT_NE(res, VPE_ALGO_ERR_OK);
    detailEnhVideo->Release();
}

HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_17, TestSize.Level1)
{
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    auto res = detailEnhVideo->Stop();
    EXPECT_NE(res, VPE_ALGO_ERR_OK);
    detailEnhVideo->Release();
}

HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_18, TestSize.Level1)
{
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    std::shared_ptr<DetailEnhancerVideoCallback> cb = std::make_shared<DetailEnhancerVideoCallbackImpl>();
    auto res = detailEnhVideo->RegisterCallback(cb);
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    res = detailEnhVideo->SetParameter(param, VIDEO);
    auto surface1 = detailEnhVideo->GetInputSurface();
    res = detailEnhVideo->SetOutputSurface(surface1);
    res = detailEnhVideo->Start();
    res = detailEnhVideo->NotifyEos();
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);
    detailEnhVideo->Release();
}

HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_19, TestSize.Level1)
{
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    std::shared_ptr<DetailEnhancerVideoCallback> cb = std::make_shared<DetailEnhancerVideoCallbackImpl>();
    auto res = detailEnhVideo->RegisterCallback(cb);
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    res = detailEnhVideo->SetParameter(param, VIDEO);
    auto surface1 = detailEnhVideo->GetInputSurface();
    res = detailEnhVideo->SetOutputSurface(surface1);
    res = detailEnhVideo->Start();
    res = detailEnhVideo->ReleaseOutputBuffer(100, true); // 100 index
    EXPECT_NE(res, VPE_ALGO_ERR_OK);
    detailEnhVideo->Release();
}

HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_20, TestSize.Level1)
{
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    std::shared_ptr<DetailEnhancerVideoCallback> cb = std::make_shared<DetailEnhancerVideoCallbackImpl>();
    auto res = detailEnhVideo->RegisterCallback(cb);
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    res = detailEnhVideo->SetParameter(param, VIDEO);
    auto surface1 = detailEnhVideo->GetInputSurface();
    res = detailEnhVideo->SetOutputSurface(surface1);
    res = detailEnhVideo->Start();
    res = detailEnhVideo->ReleaseOutputBuffer(100, false); // 100 index
    EXPECT_NE(res, VPE_ALGO_ERR_OK);
    detailEnhVideo->Release();
}

HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_21, TestSize.Level1)
{
    OHNativeWindowBuffer *ohNativeWindowBuffer;
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    std::shared_ptr<DetailEnhancerVideoCallback> cb = std::make_shared<DetailEnhancerVideoCallbackImpl>();
    auto res = detailEnhVideo->RegisterCallback(cb);
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    res = detailEnhVideo->SetParameter(param, VIDEO);
    auto surface = detailEnhVideo->GetInputSurface();
    auto detailEnh2 = DetailEnhancerVideo::Create();
    auto surface2 = detailEnh2->GetInputSurface();
    res = detailEnhVideo->SetOutputSurface(surface2);
    res = detailEnhVideo->Start();
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);

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
    detailEnhVideo->Release();
}

HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_22, TestSize.Level1)
{
    OHNativeWindowBuffer *ohNativeWindowBuffer;
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    std::shared_ptr<DetailEnhancerVideoCallback> cb = std::make_shared<DetailEnhancerVideoCallbackImpl>();
    auto res = detailEnhVideo->RegisterCallback(cb);
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    res = detailEnhVideo->SetParameter(param, VIDEO);
    auto surface = detailEnhVideo->GetInputSurface();
    auto detailEnh2 = DetailEnhancerVideo::Create();
    auto surface2 = detailEnh2->GetInputSurface();
    surface2->SetRequestWidthAndHeight(10, 10);
    res = detailEnhVideo->SetOutputSurface(surface2);
    res = detailEnhVideo->Start();
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);

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
    detailEnhVideo->Release();
}

HWTEST_F(DetailEnhancerVideoUnitTest, detailenhancer_23, TestSize.Level1)
{
    OHNativeWindowBuffer *ohNativeWindowBuffer;
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    std::shared_ptr<DetailEnhancerVideoCallback> cb = std::make_shared<DetailEnhancerVideoCallbackImpl>();
    auto res = detailEnhVideo->RegisterCallback(cb);
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    res = detailEnhVideo->SetParameter(param, VIDEO);
    auto surface = detailEnhVideo->GetInputSurface();
    auto detailEnh2 = DetailEnhancerVideo::Create();
    auto surface2 = detailEnh2->GetInputSurface();
    surface2->SetRequestWidthAndHeight(10, 0);
    res = detailEnhVideo->SetOutputSurface(surface2);
    res = detailEnhVideo->Start();
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);

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
    detailEnhVideo->Release();
}

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
