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

#include "aihdr_enhancer_video_impl.h"
#include "aihdr_enhancer_video.h"
#include "surface/window.h"
#include "external_window.h"

using namespace std;
using namespace testing::ext;
constexpr int64_t NANOS_IN_SECOND = 1000000000L;
constexpr int64_t NANOS_IN_MICRO = 1000L;
constexpr uint32_t DEFAULT_WIDTH = 1920;
constexpr uint32_t DEFAULT_HEIGHT = 1080;

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
std::shared_ptr<AihdrEnhancerVideo> aev = nullptr;
class AihdrEnhancerVideoCallbackImpl : public AihdrEnhancerVideoCallback {
public:
    AihdrEnhancerVideoCallbackImpl() = default;
    ~AihdrEnhancerVideoCallbackImpl() override = default;
    AihdrEnhancerVideoCallbackImpl(const AihdrEnhancerVideoCallbackImpl&) = delete;
    AihdrEnhancerVideoCallbackImpl& operator=(const AihdrEnhancerVideoCallbackImpl&) = delete;
    AihdrEnhancerVideoCallbackImpl(AihdrEnhancerVideoCallbackImpl&&) = delete;
    AihdrEnhancerVideoCallbackImpl& operator=(AihdrEnhancerVideoCallbackImpl&&) = delete;
 
    void OnError(int32_t errorCode) override;
    void OnState(int32_t state) override;
    void OnOutputBufferAvailable(uint32_t index, AihdrEnhancerBufferFlag flag) override;
};
void AihdrEnhancerVideoCallbackImpl::OnOutputBufferAvailable(uint32_t index, AihdrEnhancerBufferFlag flag)
{
    if (aev != nullptr) {
        aev->ReleaseOutputBuffer(index, flag);
    }
}
void AihdrEnhancerVideoCallbackImpl::OnError(int32_t errorCode)
{
    (void)errorCode;
}
void AihdrEnhancerVideoCallbackImpl::OnState(int32_t state)
{
    (void)state;
}

class AihdrEnhancerVideoUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<Surface> surface;
    OHNativeWindow *nativeWindow;
    uint32_t FlushSurf(OHNativeWindowBuffer *ohNativeWindowBuffer);
};

void AihdrEnhancerVideoUnitTest::SetUpTestCase(void)
{
    cout << "[SetUpTestCase]: " << endl;
}

void AihdrEnhancerVideoUnitTest::TearDownTestCase(void)
{
    cout << "[TearDownTestCase]: " << endl;
}

void AihdrEnhancerVideoUnitTest::SetUp(void)
{
    cout << "[SetUp]: SetUp!!!" << endl;
}

void AihdrEnhancerVideoUnitTest::TearDown(void)
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

uint32_t AihdrEnhancerVideoUnitTest::FlushSurf(OHNativeWindowBuffer *ohNativeWindowBuffer)
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

HWTEST_F(AihdrEnhancerVideoUnitTest, aihdrEnhancer_init_01, TestSize.Level1)
{
    auto aihdrEnhancerVideo = AihdrEnhancerVideo::Create();
    EXPECT_NE(aihdrEnhancerVideo, nullptr);
}

HWTEST_F(AihdrEnhancerVideoUnitTest, aihdrEnhancer_02, TestSize.Level1)
{
    auto aihdrEnhancerVideo = AihdrEnhancerVideo::Create();
    std::shared_ptr<AihdrEnhancerVideoCallback> cb = nullptr;
    auto ret = aihdrEnhancerVideo->SetCallback(cb);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(AihdrEnhancerVideoUnitTest, aihdrEnhancer_03, TestSize.Level1)
{
    auto aihdrEnhancerVideo = AihdrEnhancerVideo::Create();
    OHNativeWindow* window{};
    auto ret = aihdrEnhancerVideo->GetSurface(&window);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(AihdrEnhancerVideoUnitTest, aihdrEnhancer_04, TestSize.Level1)
{
    auto aihdrEnhancerVideo = AihdrEnhancerVideo::Create();
    OHNativeWindow* window{};
    auto ret = aihdrEnhancerVideo->GetSurface(&window);
    ret = aihdrEnhancerVideo->GetSurface(&window);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(AihdrEnhancerVideoUnitTest, aihdrEnhancer_05, TestSize.Level1)
{
    auto aihdrEnhancerVideo = AihdrEnhancerVideo::Create();
    OHNativeWindow* window{};
    auto ret = aihdrEnhancerVideo->SetSurface(window);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(AihdrEnhancerVideoUnitTest, aihdrEnhancer_06, TestSize.Level1)
{
    auto aihdrEnhancerVideo = AihdrEnhancerVideo::Create();

    auto ret = aihdrEnhancerVideo->ReleaseOutputBuffer(0, true);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(AihdrEnhancerVideoUnitTest, aihdrEnhancer_07, TestSize.Level1)
{
    auto aihdrEnhancerVideo = AihdrEnhancerVideo::Create();
    auto ret = aihdrEnhancerVideo->NotifyEos();
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(AihdrEnhancerVideoUnitTest, aihdrEnhancer_08, TestSize.Level1)
{
    auto aihdrEnhancerVideo = AihdrEnhancerVideo::Create();
    std::shared_ptr<AihdrEnhancerVideoCallback> cb = std::make_shared<AihdrEnhancerVideoCallbackImpl>();
    auto res = aihdrEnhancerVideo->SetCallback(cb);
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);
}

HWTEST_F(AihdrEnhancerVideoUnitTest, aihdrEnhancer_09, TestSize.Level1)
{
    auto aihdrEnhancerVideo = AihdrEnhancerVideo::Create();
    auto res = aihdrEnhancerVideo->Stop();
    EXPECT_NE(res, VPE_ALGO_ERR_OK);
}

HWTEST_F(AihdrEnhancerVideoUnitTest, aihdrEnhancer_10, TestSize.Level1)
{
    OHNativeWindowBuffer *ohNativeWindowBuffer;
    auto aihdrEnhancerVideo = AihdrEnhancerVideo::Create();
    std::shared_ptr<AihdrEnhancerVideoCallback> cb = std::make_shared<AihdrEnhancerVideoCallbackImpl>();
    auto res = aihdrEnhancerVideo->SetCallback(cb);
    aihdrEnhancerVideo->GetSurface(&nativeWindow);

    auto aihdrEnhancerVideo2 = AihdrEnhancerVideo::Create();
    OHNativeWindow* window2{};
    aihdrEnhancerVideo2->GetSurface(&window2);
    OH_NativeWindow_NativeWindowHandleOpt(window2, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    res = aihdrEnhancerVideo->SetSurface(window2);
    res = aihdrEnhancerVideo->Prepare();
    res = aihdrEnhancerVideo->Start();
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);

    int fenceFd = -1;
    OH_NativeWindow_NativeWindowHandleOpt(nativeWindow, SET_FORMAT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP);
    OH_NativeWindow_NativeWindowHandleOpt(nativeWindow, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    auto ret = OH_NativeWindow_NativeWindowRequestBuffer(nativeWindow, &ohNativeWindowBuffer, &fenceFd);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = FlushSurf(ohNativeWindowBuffer);
    ASSERT_EQ(ret, VPE_ALGO_ERR_OK);
    OH_NativeWindow_DestroyNativeWindow(nativeWindow);
}

HWTEST_F(AihdrEnhancerVideoUnitTest, aihdrEnhancer_11, TestSize.Level1)
{
    auto aihdrEnhancerVideo = AihdrEnhancerVideo::Create();
    std::shared_ptr<AihdrEnhancerVideoCallback> cb = std::make_shared<AihdrEnhancerVideoCallbackImpl>();
    auto res = aihdrEnhancerVideo->SetCallback(cb);
    aihdrEnhancerVideo->GetSurface(&nativeWindow);

    auto aihdrEnhancerVideo2 = AihdrEnhancerVideo::Create();
    OHNativeWindow* window2{};
    aihdrEnhancerVideo2->GetSurface(&window2);
    OH_NativeWindow_NativeWindowHandleOpt(window2, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    res = aihdrEnhancerVideo->SetSurface(window2);
    res = aihdrEnhancerVideo->Prepare();
    res = aihdrEnhancerVideo->Start();
    res = aihdrEnhancerVideo->Stop();
    res = aihdrEnhancerVideo->Prepare();
    res = aihdrEnhancerVideo->Stop();
    EXPECT_NE(res, VPE_ALGO_ERR_OK);
}

HWTEST_F(AihdrEnhancerVideoUnitTest, aihdrEnhancer_12, TestSize.Level1)
{
    auto aihdrEnhancerVideo = AihdrEnhancerVideo::Create();
    std::shared_ptr<AihdrEnhancerVideoCallback> cb = std::make_shared<AihdrEnhancerVideoCallbackImpl>();
    auto res = aihdrEnhancerVideo->SetCallback(cb);
    aihdrEnhancerVideo->GetSurface(&nativeWindow);

    auto aihdrEnhancerVideo2 = AihdrEnhancerVideo::Create();
    OHNativeWindow* window2{};
    aihdrEnhancerVideo2->GetSurface(&window2);
    OH_NativeWindow_NativeWindowHandleOpt(window2, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    res = aihdrEnhancerVideo->SetSurface(window2);
    res = aihdrEnhancerVideo->Prepare();
    res = aihdrEnhancerVideo->Start();
    res = aihdrEnhancerVideo->Stop();
    res = aihdrEnhancerVideo->Prepare();
    res = aihdrEnhancerVideo->Stop();
    EXPECT_NE(res, VPE_ALGO_ERR_OK);
}

HWTEST_F(AihdrEnhancerVideoUnitTest, aihdrEnhancer_13, TestSize.Level1)
{
    auto aihdrEnhancerVideo = AihdrEnhancerVideo::Create();
    std::shared_ptr<AihdrEnhancerVideoCallback> cb = std::make_shared<AihdrEnhancerVideoCallbackImpl>();
    auto res = aihdrEnhancerVideo->SetCallback(cb);
    aihdrEnhancerVideo->GetSurface(&nativeWindow);

    auto aihdrEnhancerVideo2 = AihdrEnhancerVideo::Create();
    OHNativeWindow* window2{};
    aihdrEnhancerVideo2->GetSurface(&window2);

    auto aihdrEnhancerVideo3 = AihdrEnhancerVideo::Create();
    OHNativeWindow* window3{};
    aihdrEnhancerVideo3->GetSurface(&window3);

    OH_NativeWindow_NativeWindowHandleOpt(window2, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    res = aihdrEnhancerVideo->SetSurface(window2);
    res = aihdrEnhancerVideo->Prepare();
    res = aihdrEnhancerVideo->Start();
    res = aihdrEnhancerVideo->SetSurface(window3);
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);
}

HWTEST_F(AihdrEnhancerVideoUnitTest, aihdrEnhancer_14, TestSize.Level1)
{
    auto aihdrEnhancerVideo = AihdrEnhancerVideo::Create();
    std::shared_ptr<AihdrEnhancerVideoCallback> cb = std::make_shared<AihdrEnhancerVideoCallbackImpl>();
    auto res = aihdrEnhancerVideo->SetCallback(cb);
    aihdrEnhancerVideo->GetSurface(&nativeWindow);

    auto aihdrEnhancerVideo2 = AihdrEnhancerVideo::Create();
    OHNativeWindow* window2{};
    aihdrEnhancerVideo2->GetSurface(&window2);
    OH_NativeWindow_NativeWindowHandleOpt(window2, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    res = aihdrEnhancerVideo->SetSurface(window2);
    res = aihdrEnhancerVideo->Prepare();
    res = aihdrEnhancerVideo->Start();
    res = aihdrEnhancerVideo->Stop();
    res = aihdrEnhancerVideo->SetSurface(window2);
    EXPECT_NE(res, VPE_ALGO_ERR_OK);
}

HWTEST_F(AihdrEnhancerVideoUnitTest, aihdrEnhancer_15, TestSize.Level1)
{
    auto aihdrEnhancerVideo = AihdrEnhancerVideo::Create();
    std::shared_ptr<AihdrEnhancerVideoCallback> cb = std::make_shared<AihdrEnhancerVideoCallbackImpl>();
    auto res = aihdrEnhancerVideo->SetCallback(cb);
    aihdrEnhancerVideo->GetSurface(&nativeWindow);

    auto aihdrEnhancerVideo2 = AihdrEnhancerVideo::Create();
    OHNativeWindow* window2{};
    aihdrEnhancerVideo2->GetSurface(&window2);

    auto aihdrEnhancerVideo3 = AihdrEnhancerVideo::Create();
    OHNativeWindow* window3{};
    aihdrEnhancerVideo3->GetSurface(&window3);

    OH_NativeWindow_NativeWindowHandleOpt(window2, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    res = aihdrEnhancerVideo->SetSurface(window2);
    res = aihdrEnhancerVideo->Prepare();
    res = aihdrEnhancerVideo->Start();
    res = aihdrEnhancerVideo->Stop();
    res = aihdrEnhancerVideo->SetSurface(window3);
    EXPECT_NE(res, VPE_ALGO_ERR_OK);
}

HWTEST_F(AihdrEnhancerVideoUnitTest, aihdrEnhancer_16, TestSize.Level1)
{
    auto aihdrEnhancerVideo = AihdrEnhancerVideo::Create();
    std::shared_ptr<AihdrEnhancerVideoCallback> cb = std::make_shared<AihdrEnhancerVideoCallbackImpl>();
    auto res = aihdrEnhancerVideo->SetCallback(cb);
    aihdrEnhancerVideo->GetSurface(&nativeWindow);

    auto aihdrEnhancerVideo2 = AihdrEnhancerVideo::Create();
    OHNativeWindow* window2{};
    aihdrEnhancerVideo2->GetSurface(&window2);

    OH_NativeWindow_NativeWindowHandleOpt(window2, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    res = aihdrEnhancerVideo->SetSurface(window2);
    res = aihdrEnhancerVideo->Prepare();
    res = aihdrEnhancerVideo->Start();
    res = aihdrEnhancerVideo->ReleaseOutputBuffer(0, true);

    EXPECT_NE(res, VPE_ALGO_ERR_OK);
}

HWTEST_F(AihdrEnhancerVideoUnitTest, aihdrEnhancer_17, TestSize.Level1)
{
    std::queue<sptr<SurfaceBuffer>> AppInBufferAvilQue;
    sptr<SurfaceBuffer> buffer;
    int32_t fence = -1;
    BufferFlushConfig flushCfg_{};
    BufferRequestConfig requestCfg_{};
    requestCfg_.width = DEFAULT_WIDTH;
    requestCfg_.height = DEFAULT_HEIGHT;
    requestCfg_.timeout = 0;
    requestCfg_.strideAlignment = 32;
    flushCfg_.damage.x = 0;
    flushCfg_.damage.y = 0;
    flushCfg_.damage.w = DEFAULT_WIDTH;
    flushCfg_.damage.h = DEFAULT_HEIGHT;
    flushCfg_.timestamp = 0;

    aev = AihdrEnhancerVideo::Create();
    std::shared_ptr<AihdrEnhancerVideoCallback> cb = std::make_shared<AihdrEnhancerVideoCallbackImpl>();
    auto res = aev->SetCallback(cb);
    OHNativeWindow* window1{};
    aev->GetSurface(&window1);

    auto aihdrEnhancerVideo2 = AihdrEnhancerVideo::Create();
    OHNativeWindow* window2{};
    aihdrEnhancerVideo2->GetSurface(&window2);

    auto aihdrEnhancerVideo3 = AihdrEnhancerVideo::Create();
    OHNativeWindow* window3{};
    aihdrEnhancerVideo3->GetSurface(&window3);

    OH_NativeWindow_NativeWindowHandleOpt(window2, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    res = aev->SetSurface(window2);
    res = aev->Prepare();
    res = aev->Start();
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);
    for (int i = 0; i < 3; i++) {
        OHNativeWindow* windowTmp = (i % 2) ? window2 : window3;
        window1->surface->RequestBuffer(buffer, fence, requestCfg_);
        AppInBufferAvilQue.push(buffer);
        window1->surface->FlushBuffer(buffer, -1, flushCfg_);
        aev->SetSurface(windowTmp);
        aev->NotifyEos();
        sleep(2);
    }
    aihdrEnhancerVideo2->Release();
    aihdrEnhancerVideo3->Release();
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
