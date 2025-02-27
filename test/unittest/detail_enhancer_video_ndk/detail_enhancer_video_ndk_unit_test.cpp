/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <condition_variable>
#include <mutex>

#include <unistd.h>

#include <gtest/gtest.h>

#include "native_avformat.h"

#include "video_processing.h"
#include "video_processing_types.h"
#include "video_processing_callback_impl.h"
#include "video_processing_callback_native.h"
#include "video_environment_native.h"
#include "video_processing_impl.h"

#include "detail_enhancer_video_impl.h"
#include "detail_enhancer_video.h"
#include "surface/window.h"
#include "external_window.h"

namespace {
constexpr int64_t NANOS_IN_SECOND = 1000000000L;
constexpr int64_t NANOS_IN_MICRO = 1000L;
constexpr uint32_t DEFAULT_WIDTH = 1920;
constexpr uint32_t DEFAULT_HEIGHT = 1080;

std::condition_variable g_cvStop{};
std::mutex g_lock{};
bool g_isStoped = true;

void WaitForStop()
{
    std::cout << "wait for stop flag" << std::endl;
    std::unique_lock<std::mutex> lock(g_lock);
    g_cvStop.wait(lock, [] { return g_isStoped; });
    std::cout << "stop flag is " << g_isStoped << std::endl;
}
}

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

void OnError(OH_VideoProcessing *handle, VideoProcessing_ErrorCode errorCode, void* userData)
{
    // do something
}
void OnState(OH_VideoProcessing* instance, VideoProcessing_State state, void* userData)
{
    if (state == VIDEO_PROCESSING_STATE_STOPPED) {
        std::cout << "state is VIDEO_PROCESSING_STATE_STOPPED" << std::endl;
        {
            std::lock_guard<std::mutex> lock(g_lock);
            g_isStoped = true;
        }
        g_cvStop.notify_one();
    } else {
        std::cout << "state is VIDEO_PROCESSING_STATE_RUNNING" << std::endl;
        std::lock_guard<std::mutex> lock(g_lock);
        g_isStoped = false;
        std::cout << "start and stop flag is false" << std::endl;
    }
    if (userData != nullptr) {
        VideoProcessing_State* userState = static_cast<VideoProcessing_State*>(userData);
        *userState = state;
    }
}

void OnNewOutputBuffer(OH_VideoProcessing* instance, uint32_t index, void* userData)
{
        (void)userData;
        (void)OH_VideoProcessing_RenderOutputBuffer(instance, index);
}


class DetailEnhancerVideoNdkUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<Surface> surface;
    uint32_t FlushSurf(OHNativeWindowBuffer *ohNativeWindowBuffer, OHNativeWindow *window);
};

void DetailEnhancerVideoNdkUnitTest::SetUpTestCase(void)
{
    cout << "[SetUpTestCase]: " << endl;
}

void DetailEnhancerVideoNdkUnitTest::TearDownTestCase(void)
{
    cout << "[TearDownTestCase]: " << endl;
}

void DetailEnhancerVideoNdkUnitTest::SetUp(void)
{
    cout << "[SetUp]: SetUp!!!" << endl;
}

void DetailEnhancerVideoNdkUnitTest::TearDown(void)
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

uint32_t DetailEnhancerVideoNdkUnitTest::FlushSurf(OHNativeWindowBuffer *ohNativeWindowBuffer, OHNativeWindow *window)
{
    struct Region region;
    struct Region::Rect *rect = new Region::Rect();
    rect->x = 0;
    rect->y = 0;
    rect->w = DEFAULT_WIDTH;
    rect->h = DEFAULT_HEIGHT;
    region.rects = rect;
    NativeWindowHandleOpt(window, SET_UI_TIMESTAMP, GetSystemTime());
    int32_t err = OH_NativeWindow_NativeWindowFlushBuffer(window, ohNativeWindowBuffer, -1, region);
    delete rect;
    if (err != 0) {
        cout << "FlushBuffer failed" << endl;
        return 1;
    }
    return 0;
}

// context init nullptr
HWTEST_F(DetailEnhancerVideoNdkUnitTest, create_instance_01, TestSize.Level1)
{
    int createType = 0x4;
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessing_Destroy(instance);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessing_DeinitializeEnvironment();
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
}

// context create without init
HWTEST_F(DetailEnhancerVideoNdkUnitTest, create_instance_02, TestSize.Level1)
{
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    auto ret = OH_VideoProcessing_Create(&instance, createType);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessing_Destroy(instance);
}

// context init 2
HWTEST_F(DetailEnhancerVideoNdkUnitTest, create_instance_03, TestSize.Level1)
{
    int createType = 0x2;
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessing_Destroy(instance);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessing_DeinitializeEnvironment();
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
}

// context init 1
HWTEST_F(DetailEnhancerVideoNdkUnitTest, create_instance_04, TestSize.Level1)
{
    int createType = 0x1;
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessing_Destroy(instance);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessing_DeinitializeEnvironment();
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
}

// context init -1
HWTEST_F(DetailEnhancerVideoNdkUnitTest, create_instance_05, TestSize.Level1)
{
    int createType = -1;
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    EXPECT_NE(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessing_Destroy(instance);
    EXPECT_NE(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessing_DeinitializeEnvironment();
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
}

// context init repeat
HWTEST_F(DetailEnhancerVideoNdkUnitTest, create_instance_06, TestSize.Level1)
{
    int createType = 0x4;
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    ret = OH_VideoProcessing_InitializeEnvironment();
    ret = OH_VideoProcessing_InitializeEnvironment();
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessing_Destroy(instance);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessing_Destroy(instance);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessing_DeinitializeEnvironment();
    ret = OH_VideoProcessing_DeinitializeEnvironment();
    ret = OH_VideoProcessing_DeinitializeEnvironment();
    ret = OH_VideoProcessing_InitializeEnvironment();
    ret = OH_VideoProcessing_DeinitializeEnvironment();
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
}

// deinit
HWTEST_F(DetailEnhancerVideoNdkUnitTest, deinit_01, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    ret = OH_VideoProcessing_DeinitializeEnvironment();
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
}

// context init and destroy
HWTEST_F(DetailEnhancerVideoNdkUnitTest, destroy_instance_01, TestSize.Level1)
{
    int createType = 0x4;
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    ret = OH_VideoProcessing_Destroy(instance);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// context no create but destroy
HWTEST_F(DetailEnhancerVideoNdkUnitTest, destroy_instance_02, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Destroy(instance);
    EXPECT_NE(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// context no create but destroy without initialize
HWTEST_F(DetailEnhancerVideoNdkUnitTest, destroy_instance_03, TestSize.Level1)
{
    OH_VideoProcessing* instance = nullptr;
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_Destroy(instance);
    EXPECT_NE(ret, VIDEO_PROCESSING_SUCCESS);
}

// color space converter support check
HWTEST_F(DetailEnhancerVideoNdkUnitTest, check_support_01, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    const VideoProcessing_ColorSpaceInfo* sourceVideoInfo = nullptr;
    const VideoProcessing_ColorSpaceInfo* destinationVideoInfo = nullptr;
    auto res = OH_VideoProcessing_IsColorSpaceConversionSupported(sourceVideoInfo, destinationVideoInfo);
    EXPECT_EQ(res, false);
    ret = OH_VideoProcessing_DeinitializeEnvironment();
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
}

// Set param nullptr
HWTEST_F(DetailEnhancerVideoNdkUnitTest, set_parameter_01, TestSize.Level1)
{
    int createType = 0x4;
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameter = nullptr;
    ret = OH_VideoProcessing_SetParameter(instance, parameter);
    EXPECT_NE(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// set normal
HWTEST_F(DetailEnhancerVideoNdkUnitTest, set_parameter_02, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, VIDEO_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL,
        VIDEO_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH);
    ret = OH_VideoProcessing_SetParameter(instance, parameter);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// get parameter but param not null
HWTEST_F(DetailEnhancerVideoNdkUnitTest, get_parameter_01, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    ret = OH_VideoProcessing_GetParameter(instance, parameter);
    EXPECT_NE(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// get parameter normal
HWTEST_F(DetailEnhancerVideoNdkUnitTest, get_parameter_02, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameter = nullptr;
    ret = OH_VideoProcessing_GetParameter(instance, parameter);
    EXPECT_NE(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// get parameter normal after set
HWTEST_F(DetailEnhancerVideoNdkUnitTest, get_parameter_03, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameterSetted = OH_AVFormat_Create();
    OH_AVFormat* parameterGetted = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameterSetted, VIDEO_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL,
        VIDEO_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH);
    ret = OH_VideoProcessing_SetParameter(instance, parameterSetted);
    ret = OH_VideoProcessing_GetParameter(instance, parameterGetted);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// set surface with nullptr
HWTEST_F(DetailEnhancerVideoNdkUnitTest, set_surface_01, TestSize.Level1)
{
    OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    OH_VideoProcessing* instance2 = nullptr;
    OHNativeWindow* window;
    OHNativeWindow* window2;
    OH_VideoProcessing_Create(&instance, createType);
    OH_VideoProcessing_Create(&instance2, createType);
    OH_VideoProcessing_GetSurface(instance, &window);
    OH_VideoProcessing_GetSurface(instance2, &window2);
    auto ret = OH_VideoProcessing_SetSurface(instance, window2);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// set surface with a non nullptr window
HWTEST_F(DetailEnhancerVideoNdkUnitTest, set_surface_02, TestSize.Level1)
{
    OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    OHNativeWindow* window = nullptr;
    OH_VideoProcessing_Create(&instance, createType);
    auto ret = OH_VideoProcessing_SetSurface(instance, window);
    EXPECT_NE(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// get surface nullptr
HWTEST_F(DetailEnhancerVideoNdkUnitTest, get_surface_01, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    OHNativeWindow* window;
    ret = OH_VideoProcessing_Create(&instance, createType);
    ret = OH_VideoProcessing_GetSurface(instance, &window);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// set surfaceto a non nullptr window
HWTEST_F(DetailEnhancerVideoNdkUnitTest, get_surface_02, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    auto detailEnhVideo = DetailEnhancerVideo::Create();
    auto surface = detailEnhVideo->GetInputSurface();
    OHNativeWindow* window = CreateNativeWindowFromSurface(&surface);
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, VIDEO_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL,
        VIDEO_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH);
    ret = OH_VideoProcessing_SetParameter(instance, parameter);
    ret = OH_VideoProcessing_SetSurface(instance, window);
    ret = OH_VideoProcessing_GetSurface(instance, &window);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// create callback and register
HWTEST_F(DetailEnhancerVideoNdkUnitTest, callback_01, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, VIDEO_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL,
        VIDEO_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH);
    OH_VideoProcessing_SetParameter(instance, parameter);
    VideoProcessing_Callback* callback = nullptr;
    ret = OH_VideoProcessingCallback_Create(&callback);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessingCallback_BindOnError(callback, OnError);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessingCallback_BindOnState(callback, OnState);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, OnNewOutputBuffer);
    OH_VideoProcessing_RenderOutputBuffer(instance, 0);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    auto userData = VIDEO_PROCESSING_STATE_STOPPED;
    OH_VideoProcessing_RegisterCallback(instance, callback, &userData);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// create callback and register null func
HWTEST_F(DetailEnhancerVideoNdkUnitTest, callback_02, TestSize.Level1)
{
    OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, VIDEO_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL,
        VIDEO_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH);
    OH_VideoProcessing_SetParameter(instance, parameter);
    VideoProcessing_Callback* callback = nullptr;
    OH_VideoProcessingCallback_Create(&callback);
    OH_VideoProcessingCallback_BindOnError(callback, nullptr);
    OH_VideoProcessingCallback_BindOnState(callback, nullptr);
    OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, nullptr);
    auto userData = VIDEO_PROCESSING_STATE_STOPPED;
    auto ret = OH_VideoProcessing_RegisterCallback(instance, callback, &userData);
    OH_VideoProcessing_RenderOutputBuffer(instance, 0);
    EXPECT_NE(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
    OH_VideoProcessingCallback_Destroy(callback);
}

// create and destroy
HWTEST_F(DetailEnhancerVideoNdkUnitTest, callback_03, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, VIDEO_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL,
        VIDEO_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH);
    ret = OH_VideoProcessing_SetParameter(instance, parameter);
    VideoProcessing_Callback* callback = nullptr;
    ret = OH_VideoProcessingCallback_Create(&callback);
    ret = OH_VideoProcessingCallback_BindOnError(callback, OnError);
    ret = OH_VideoProcessingCallback_BindOnState(callback, OnState);
    OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, OnNewOutputBuffer);
    void* userData = nullptr;
    ret = OH_VideoProcessing_RegisterCallback(instance, callback, userData);
    OH_VideoProcessing_RenderOutputBuffer(instance, 0);
    ret = OH_VideoProcessingCallback_Destroy(callback);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// only destroy call back
HWTEST_F(DetailEnhancerVideoNdkUnitTest, callback_04, TestSize.Level1)
{
    VideoProcessing_Callback* callback = nullptr;
    VideoProcessing_ErrorCode ret = OH_VideoProcessingCallback_Destroy(callback);
    EXPECT_NE(ret, VIDEO_PROCESSING_SUCCESS);
}

// create callback and register but instance is nullptr
HWTEST_F(DetailEnhancerVideoNdkUnitTest, callback_05, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    OH_VideoProcessing* instance = nullptr;
    VideoProcessing_Callback* callback = nullptr;
    ret = OH_VideoProcessingCallback_Create(&callback);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessingCallback_BindOnError(callback, OnError);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessingCallback_BindOnState(callback, OnState);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, OnNewOutputBuffer);
    EXPECT_EQ(ret, VIDEO_PROCESSING_SUCCESS);
    auto userData = VIDEO_PROCESSING_STATE_STOPPED;
    OH_VideoProcessing_RegisterCallback(instance, callback, &userData);
    OH_VideoProcessing_RenderOutputBuffer(instance, 0);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// create callback and register but callback is nullptr
HWTEST_F(DetailEnhancerVideoNdkUnitTest, callback_06, TestSize.Level1)
{
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    ret = OH_VideoProcessing_Create(&instance, createType);
    VideoProcessing_Callback* callback = nullptr;
    ret = OH_VideoProcessingCallback_BindOnError(callback, OnError);
    EXPECT_NE(ret, VIDEO_PROCESSING_SUCCESS);
    ret = OH_VideoProcessingCallback_BindOnState(callback, OnState);
    EXPECT_NE(ret, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, OnNewOutputBuffer);
    EXPECT_NE(ret, VIDEO_PROCESSING_SUCCESS);
    auto userData = VIDEO_PROCESSING_STATE_STOPPED;
    OH_VideoProcessing_RegisterCallback(instance, callback, &userData);
    OH_VideoProcessing_RenderOutputBuffer(instance, 0);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// start processing with flush surface
HWTEST_F(DetailEnhancerVideoNdkUnitTest, start_01, TestSize.Level1)
{
    OHNativeWindowBuffer *ohNativeWindowBuffer;
    OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    OH_VideoProcessing* instance2 = nullptr;
    OH_VideoProcessing_Create(&instance, createType);
    OH_VideoProcessing_Create(&instance2, createType);
    VideoProcessing_Callback* callback = nullptr;
    OH_VideoProcessingCallback_Create(&callback);
    OH_VideoProcessingCallback_BindOnError(callback, OnError);
    OH_VideoProcessingCallback_BindOnState(callback, OnState);
    OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, OnNewOutputBuffer);
    auto userData = VIDEO_PROCESSING_STATE_STOPPED;
    OH_VideoProcessing_RegisterCallback(instance, callback, &userData);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, VIDEO_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL,
        VIDEO_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH);
    OH_VideoProcessing_SetParameter(instance, parameter);

    OHNativeWindow* window = nullptr;
    OHNativeWindow* window2 = nullptr;

    VideoProcessing_ErrorCode res = OH_VideoProcessing_GetSurface(instance, &window);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    res = OH_VideoProcessing_GetSurface(instance2, &window2);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    res = OH_VideoProcessing_SetSurface(instance, window2);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);

    res = OH_VideoProcessing_Start(instance);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_RenderOutputBuffer(instance, 0);
    int fenceFd = -1;
    auto ret1 = OH_NativeWindow_NativeWindowHandleOpt(window, SET_FORMAT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP);
    ASSERT_EQ(ret1, VPE_ALGO_ERR_OK);
    auto ret2 = OH_NativeWindow_NativeWindowHandleOpt(window, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    ASSERT_EQ(ret2, VPE_ALGO_ERR_OK);
    auto ret3 = OH_NativeWindow_NativeWindowRequestBuffer(window, &ohNativeWindowBuffer, &fenceFd);
    ASSERT_EQ(ret3, VPE_ALGO_ERR_OK);
    auto ret4 = FlushSurf(ohNativeWindowBuffer, window);
    ASSERT_EQ(ret4, VPE_ALGO_ERR_OK);
    OH_VideoProcessing_RenderOutputBuffer(instance, 1);
}

// start processing with flush surface then stop
HWTEST_F(DetailEnhancerVideoNdkUnitTest, start_02, TestSize.Level1)
{
    OHNativeWindowBuffer *ohNativeWindowBuffer;
    OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    OH_VideoProcessing* instance2 = nullptr;
    OH_VideoProcessing_Create(&instance, createType);
    OH_VideoProcessing_Create(&instance2, createType);
    VideoProcessing_Callback* callback = nullptr;
    OH_VideoProcessingCallback_Create(&callback);
    OH_VideoProcessingCallback_BindOnError(callback, OnError);
    OH_VideoProcessingCallback_BindOnState(callback, OnState);
    OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, OnNewOutputBuffer);
    auto userData = VIDEO_PROCESSING_STATE_STOPPED;
    OH_VideoProcessing_RegisterCallback(instance, callback, &userData);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, VIDEO_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL,
        VIDEO_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH);
    OH_VideoProcessing_SetParameter(instance, parameter);

    OHNativeWindow* window = nullptr;
    OHNativeWindow* window2 = nullptr;

    VideoProcessing_ErrorCode res = OH_VideoProcessing_GetSurface(instance, &window);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    res = OH_VideoProcessing_GetSurface(instance2, &window2);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    res = OH_VideoProcessing_SetSurface(instance, window2);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);

    res = OH_VideoProcessing_Start(instance);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_RenderOutputBuffer(instance, 0);

    int fenceFd = -1;
    auto ret1 = OH_NativeWindow_NativeWindowHandleOpt(window, SET_FORMAT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP);
    ASSERT_EQ(ret1, VPE_ALGO_ERR_OK);
    auto ret2 = OH_NativeWindow_NativeWindowHandleOpt(window, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    ASSERT_EQ(ret2, VPE_ALGO_ERR_OK);
    auto ret3 = OH_NativeWindow_NativeWindowRequestBuffer(window, &ohNativeWindowBuffer, &fenceFd);
    ASSERT_EQ(ret3, VPE_ALGO_ERR_OK);
    auto ret4 = FlushSurf(ohNativeWindowBuffer, window);
    ASSERT_EQ(ret4, VPE_ALGO_ERR_OK);
    
    OH_VideoProcessing_Stop(instance);
    WaitForStop();
    OH_VideoProcessingCallback_Destroy(callback);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// start repeatedly
HWTEST_F(DetailEnhancerVideoNdkUnitTest, start_03, TestSize.Level1)
{
    OHNativeWindowBuffer *ohNativeWindowBuffer;
    OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    OH_VideoProcessing* instance2 = nullptr;
    OH_VideoProcessing_Create(&instance, createType);
    OH_VideoProcessing_Create(&instance2, createType);
    VideoProcessing_Callback* callback = nullptr;
    OH_VideoProcessingCallback_Create(&callback);
    OH_VideoProcessingCallback_BindOnError(callback, OnError);
    OH_VideoProcessingCallback_BindOnState(callback, OnState);
    OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, OnNewOutputBuffer);
    void* userData = nullptr;
    OH_VideoProcessing_RegisterCallback(instance, callback, userData);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, VIDEO_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL,
        VIDEO_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH);
    OH_VideoProcessing_SetParameter(instance, parameter);

    OHNativeWindow* window = nullptr;
    OHNativeWindow* window2 = nullptr;

    VideoProcessing_ErrorCode res = OH_VideoProcessing_GetSurface(instance, &window);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    res = OH_VideoProcessing_GetSurface(instance2, &window2);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    res = OH_VideoProcessing_SetSurface(instance, window2);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);

    res = OH_VideoProcessing_Start(instance);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    res = OH_VideoProcessing_Start(instance);
    EXPECT_NE(res, VIDEO_PROCESSING_SUCCESS);

    int fenceFd = -1;
    auto ret1 = OH_NativeWindow_NativeWindowHandleOpt(window, SET_FORMAT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP);
    ASSERT_EQ(ret1, VPE_ALGO_ERR_OK);
    auto ret2 = OH_NativeWindow_NativeWindowHandleOpt(window, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    ASSERT_EQ(ret2, VPE_ALGO_ERR_OK);
    auto ret3 = OH_NativeWindow_NativeWindowRequestBuffer(window, &ohNativeWindowBuffer, &fenceFd);
    ASSERT_EQ(ret3, VPE_ALGO_ERR_OK);
    auto ret4 = FlushSurf(ohNativeWindowBuffer, window);
    ASSERT_EQ(ret4, VPE_ALGO_ERR_OK);
    OH_VideoProcessing_RenderOutputBuffer(instance, 0);

    OH_VideoProcessing_Stop(instance);
    WaitForStop();
    OH_VideoProcessing_Stop(instance);
    OH_VideoProcessingCallback_Destroy(callback);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// start but not have buffer callbacks
HWTEST_F(DetailEnhancerVideoNdkUnitTest, start_04, TestSize.Level1)
{
    OHNativeWindowBuffer *ohNativeWindowBuffer;
    OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    OH_VideoProcessing* instance2 = nullptr;
    OH_VideoProcessing_Create(&instance, createType);
    OH_VideoProcessing_Create(&instance2, createType);
    VideoProcessing_Callback* callback = nullptr;
    OH_VideoProcessingCallback_Create(&callback);
    OH_VideoProcessingCallback_BindOnError(callback, OnError);
    OH_VideoProcessingCallback_BindOnState(callback, OnState);
    OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, nullptr);
    void* userData = nullptr;
    OH_VideoProcessing_RegisterCallback(instance, callback, userData);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, VIDEO_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL,
        VIDEO_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH);
    OH_VideoProcessing_SetParameter(instance, parameter);

    OHNativeWindow* window = nullptr;
    OHNativeWindow* window2 = nullptr;

    VideoProcessing_ErrorCode res = OH_VideoProcessing_GetSurface(instance, &window);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    res = OH_VideoProcessing_GetSurface(instance2, &window2);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    res = OH_VideoProcessing_SetSurface(instance, window2);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);

    res = OH_VideoProcessing_Start(instance);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);

    int fenceFd = -1;
    auto ret1 = OH_NativeWindow_NativeWindowHandleOpt(window, SET_FORMAT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP);
    ASSERT_EQ(ret1, VPE_ALGO_ERR_OK);
    auto ret2 = OH_NativeWindow_NativeWindowHandleOpt(window, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    ASSERT_EQ(ret2, VPE_ALGO_ERR_OK);
    auto ret3 = OH_NativeWindow_NativeWindowRequestBuffer(window, &ohNativeWindowBuffer, &fenceFd);
    ASSERT_EQ(ret3, VPE_ALGO_ERR_OK);
    auto ret4 = FlushSurf(ohNativeWindowBuffer, window);
    ASSERT_EQ(ret4, VPE_ALGO_ERR_OK);
    OH_VideoProcessing_RenderOutputBuffer(instance, 0);

    OH_VideoProcessing_Stop(instance);
    WaitForStop();
    OH_VideoProcessingCallback_Destroy(callback);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// start and destroy while still running
HWTEST_F(DetailEnhancerVideoNdkUnitTest, start_05, TestSize.Level1)
{
    OHNativeWindowBuffer *ohNativeWindowBuffer;
    OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    OH_VideoProcessing* instance2 = nullptr;
    OH_VideoProcessing_Create(&instance, createType);
    OH_VideoProcessing_Create(&instance2, createType);
    VideoProcessing_Callback* callback = nullptr;
    OH_VideoProcessingCallback_Create(&callback);
    OH_VideoProcessingCallback_BindOnError(callback, OnError);
    OH_VideoProcessingCallback_BindOnState(callback, OnState);
    OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, OnNewOutputBuffer);
    void* userData = nullptr;
    OH_VideoProcessing_RegisterCallback(instance, callback, userData);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, VIDEO_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL,
        VIDEO_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH);
    OH_VideoProcessing_SetParameter(instance, parameter);

    OHNativeWindow* window = nullptr;
    OHNativeWindow* window2 = nullptr;

    VideoProcessing_ErrorCode res = OH_VideoProcessing_GetSurface(instance, &window);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    res = OH_VideoProcessing_GetSurface(instance2, &window2);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    res = OH_VideoProcessing_SetSurface(instance, window2);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);

    res = OH_VideoProcessing_Start(instance);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);

    int fenceFd = -1;
    auto ret1 = OH_NativeWindow_NativeWindowHandleOpt(window, SET_FORMAT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP);
    ASSERT_EQ(ret1, VPE_ALGO_ERR_OK);
    auto ret2 = OH_NativeWindow_NativeWindowHandleOpt(window, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    ASSERT_EQ(ret2, VPE_ALGO_ERR_OK);
    auto ret3 = OH_NativeWindow_NativeWindowRequestBuffer(window, &ohNativeWindowBuffer, &fenceFd);
    ASSERT_EQ(ret3, VPE_ALGO_ERR_OK);
    auto ret4 = FlushSurf(ohNativeWindowBuffer, window);
    ASSERT_EQ(ret4, VPE_ALGO_ERR_OK);
    OH_VideoProcessing_RenderOutputBuffer(instance, 0);
    res = OH_VideoProcessingCallback_Destroy(callback);
    EXPECT_NE(res, VIDEO_PROCESSING_SUCCESS);
    res = OH_VideoProcessingCallback_Destroy(nullptr);
    EXPECT_NE(res, VIDEO_PROCESSING_SUCCESS);

    OH_VideoProcessing_Stop(instance);
    WaitForStop();
    OH_VideoProcessingCallback_Destroy(callback);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// start but no callbacks binded
HWTEST_F(DetailEnhancerVideoNdkUnitTest, start_06, TestSize.Level1)
{
    OHNativeWindowBuffer *ohNativeWindowBuffer;
    OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    OH_VideoProcessing* instance2 = nullptr;
    OH_VideoProcessing_Create(&instance, createType);
    OH_VideoProcessing_Create(&instance2, createType);
    VideoProcessing_Callback* callback = nullptr;
    OH_VideoProcessingCallback_Create(&callback);
    void* userData = nullptr;
    OH_VideoProcessing_RegisterCallback(instance, callback, userData);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, VIDEO_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL,
        VIDEO_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH);
    OH_VideoProcessing_SetParameter(instance, parameter);

    OHNativeWindow* window = nullptr;
    OHNativeWindow* window2 = nullptr;

    VideoProcessing_ErrorCode res = OH_VideoProcessing_GetSurface(instance, &window);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    res = OH_VideoProcessing_GetSurface(instance2, &window2);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    res = OH_VideoProcessing_SetSurface(instance, window2);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);

    res = OH_VideoProcessing_Start(instance);
    EXPECT_NE(res, VIDEO_PROCESSING_SUCCESS);

    int fenceFd = -1;
    auto ret1 = OH_NativeWindow_NativeWindowHandleOpt(window, SET_FORMAT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP);
    ASSERT_EQ(ret1, VPE_ALGO_ERR_OK);
    auto ret2 = OH_NativeWindow_NativeWindowHandleOpt(window, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    ASSERT_EQ(ret2, VPE_ALGO_ERR_OK);
    auto ret3 = OH_NativeWindow_NativeWindowRequestBuffer(window, &ohNativeWindowBuffer, &fenceFd);
    ASSERT_EQ(ret3, VPE_ALGO_ERR_OK);
    auto ret4 = FlushSurf(ohNativeWindowBuffer, window);
    ASSERT_EQ(ret4, VPE_ALGO_ERR_OK);
    OH_VideoProcessing_RenderOutputBuffer(instance, 1);

    OH_VideoProcessing_Stop(instance);
    WaitForStop();
    OH_VideoProcessingCallback_Destroy(callback);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// start processing with flush surface then stop and start again
HWTEST_F(DetailEnhancerVideoNdkUnitTest, start_07, TestSize.Level1)
{
    OHNativeWindowBuffer *ohNativeWindowBuffer;
    OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    OH_VideoProcessing* instance2 = nullptr;
    OH_VideoProcessing_Create(&instance, createType);
    OH_VideoProcessing_Create(&instance2, createType);
    VideoProcessing_Callback* callback = nullptr;
    OH_VideoProcessingCallback_Create(&callback);
    OH_VideoProcessingCallback_BindOnError(callback, OnError);
    OH_VideoProcessingCallback_BindOnState(callback, OnState);
    OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, OnNewOutputBuffer);
    auto userData = VIDEO_PROCESSING_STATE_STOPPED;
    OH_VideoProcessing_RegisterCallback(instance, callback, &userData);
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, VIDEO_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL,
        VIDEO_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH);
    OH_VideoProcessing_SetParameter(instance, parameter);

    OHNativeWindow* window = nullptr;
    OHNativeWindow* window2 = nullptr;

    VideoProcessing_ErrorCode res = OH_VideoProcessing_GetSurface(instance, &window);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    res = OH_VideoProcessing_GetSurface(instance2, &window2);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    res = OH_VideoProcessing_SetSurface(instance, window2);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);

    res = OH_VideoProcessing_Start(instance);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_RenderOutputBuffer(instance, 0);

    int fenceFd = -1;
    auto ret1 = OH_NativeWindow_NativeWindowHandleOpt(window, SET_FORMAT, GRAPHIC_PIXEL_FMT_YCBCR_420_SP);
    ASSERT_EQ(ret1, VPE_ALGO_ERR_OK);
    auto ret2 = OH_NativeWindow_NativeWindowHandleOpt(window, SET_BUFFER_GEOMETRY, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    ASSERT_EQ(ret2, VPE_ALGO_ERR_OK);
    auto ret3 = OH_NativeWindow_NativeWindowRequestBuffer(window, &ohNativeWindowBuffer, &fenceFd);
    ASSERT_EQ(ret3, VPE_ALGO_ERR_OK);
    auto ret4 = FlushSurf(ohNativeWindowBuffer, window);
    ASSERT_EQ(ret4, VPE_ALGO_ERR_OK);
    
    OH_VideoProcessing_Stop(instance);
    WaitForStop();
    res = OH_VideoProcessing_Start(instance);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    FlushSurf(ohNativeWindowBuffer, window);
    OH_VideoProcessing_Stop(instance);
    WaitForStop();
    OH_VideoProcessingCallback_Destroy(callback);
    OH_VideoProcessing_Destroy(instance);
    OH_VideoProcessing_DeinitializeEnvironment();
}

// simple output buffer check
HWTEST_F(DetailEnhancerVideoNdkUnitTest, output_buffer_1, TestSize.Level1)
{
    OH_VideoProcessing_InitializeEnvironment();
    int createType = 0x4;
    OH_VideoProcessing* instance = nullptr;
    auto res = OH_VideoProcessing_Create(&instance, createType);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    OH_VideoProcessing_RenderOutputBuffer(instance, 0);
    res = OH_VideoProcessing_Destroy(instance);
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
    res = OH_VideoProcessing_DeinitializeEnvironment();
    EXPECT_EQ(res, VIDEO_PROCESSING_SUCCESS);
}

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
