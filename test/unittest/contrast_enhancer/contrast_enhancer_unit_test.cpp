#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <dlfcn.h>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include "algorithm_common.h"
#include "algorithm_errors.h"
#include "graphic_common_c.h"

#include "configuration_helper.h"
#include "contrast_enhancer_common.h"
#include "contrast_enhancer_image_fwk.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS::Media::VideoProcessingEngine;

class ContrastEnhancerUnitTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化必要的对象和变量
    }

    void TearDown() override {
        // 清理工作
    }
};

sptr<SurfaceBuffer> CreateSurfaceBuffer(uint32_t pixelFormat, int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0) {
        cout << "Invalid resolution" << endl;
        return nullptr;
    }
    auto buffer = SurfaceBuffer::Create();
    if (buffer == nullptr) {
        cout << "Create surface buffer failed" << endl;
        return nullptr;
    }
    BufferRequestConfig inputCfg;
    inputCfg.width = width;
    inputCfg.height = height;
    inputCfg.strideAlignment = width;
    inputCfg.usage = BUFFER_USAGE_CPU_READ | BUFFER_USAGE_CPU_WRITE
        | BUFFER_USAGE_HW_RENDER | BUFFER_USAGE_HW_TEXTURE | BUFFER_USAGE_MEM_MMZ_CACHE;
    inputCfg.format = pixelFormat;
    inputCfg.timeout = 0;
    if (GSERROR_OK != buffer->Alloc(inputCfg)) {
        cout << "Alloc surface buffer{" + std::to_string(inputCfg.width) + std::to_string(inputCfg.strideAlignment) + 
             "x" + std::to_string(inputCfg.height)  + "format:" + std::to_string(inputCfg.format) + "} failed" << endl;
        return nullptr;
    }
    return buffer;
}

bool IsSupported()
{
    std::string xmlFilePath = "/sys_prod/etc/display/effect/displayengine/featureprocessor/adaptive_fov_cfg.xml";
    if (access(xmlFilePath.c_str(), R_OK) != 0) [[unlikely]] {
        cout << xmlFilePath + "is NOT exist!" << endl;
        return false;
    }
    return true;
}

// 测试SetParameter函数
HWTEST_F(ContrastEnhancerUnitTest, SetParameter_NormalCase, TestSize.Level1)
{
    ContrastEnhancerParameters param;
    param.type = ADAPTIVE_FOV;
    // 设置参数
    auto contrastEnhancer = ContrastEnhancerImage::Create();
    EXPECT_NE(contrastEnhancer, nullptr);
    if (contrastEnhancer == nullptr) {
        return;
    }
    VPEAlgoErrCode ret = contrastEnhancer->SetParameter(param);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// 测试GetParameter函数
HWTEST_F(ContrastEnhancerUnitTest, GetParameter_NormalCase, TestSize.Level1)
{
    ContrastEnhancerParameters param;
    auto contrastEnhancer = ContrastEnhancerImage::Create();
    EXPECT_NE(contrastEnhancer, nullptr);
    if (contrastEnhancer == nullptr) {
        return;
    }
    VPEAlgoErrCode ret = contrastEnhancer->GetParameter(param);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// 测试GetRegionHist函数
HWTEST_F(ContrastEnhancerUnitTest, GetRegionHist_NormalCase, TestSize.Level1)
{
    sptr<SurfaceBuffer> input = CreateSurfaceBuffer(OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102, 1024, 768);
    // 初始化input
    auto contrastEnhancer = ContrastEnhancerImage::Create();
    EXPECT_NE(contrastEnhancer, nullptr);
    if (contrastEnhancer == nullptr) {
        return;
    }
    ContrastEnhancerParameters param;
    param.type = ADAPTIVE_FOV;
    VPEAlgoErrCode ret = contrastEnhancer->SetParameter(param);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);

    if (!IsSupported()) {
        return;
    }

    ret = contrastEnhancer->GetRegionHist(input);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(ContrastEnhancerUnitTest, GetRegionHist_InvalidFormat, TestSize.Level1)
{
    sptr<SurfaceBuffer> input = CreateSurfaceBuffer(OHOS::GRAPHIC_PIXEL_FMT_RGBA_8888, 1024, 768);
    EXPECT_NE(input, nullptr);
    // 初始化input
    auto contrastEnhancer = ContrastEnhancerImage::Create();
    EXPECT_NE(contrastEnhancer, nullptr);
    if (contrastEnhancer == nullptr) {
        return;
    }
    ContrastEnhancerParameters param;
    param.type = ADAPTIVE_FOV;
    VPEAlgoErrCode ret = contrastEnhancer->SetParameter(param);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);

    if (!IsSupported()) {
        return;
    }

    ret = contrastEnhancer->GetRegionHist(input);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// 测试UpdateMetadataBasedOnHist函数
HWTEST_F(ContrastEnhancerUnitTest, UpdateMetadataBasedOnHist_ImplDirectly, TestSize.Level1)
{
    OHOS::Rect displayArea = {
        .x = 0,
        .y = 0,
        .w = 1920,
        .h = 1080,
    };
    sptr<SurfaceBuffer> surfaceBuffer = CreateSurfaceBuffer(OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102, 1920, 1080);
    EXPECT_NE(surfaceBuffer, nullptr);
    if (surfaceBuffer == nullptr) {
        return;
    }
    // 初始化surfaceBuffer
    auto contrastEnhancer = ContrastEnhancerImage::Create();
    EXPECT_NE(contrastEnhancer, nullptr);
    if (contrastEnhancer == nullptr) {
        return;
    }
    ContrastEnhancerParameters param;
    param.type = ADAPTIVE_FOV;
    VPEAlgoErrCode ret = contrastEnhancer->SetParameter(param);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);

    if (!IsSupported()) {
        return;
    }

    // UniqueId, contentId, curRatio, maxRatio, minRatio, animationDuration
    std::tuple<int, int, double, double, double, int> pixelmapInfo = std::make_tuple(1, 1, 0.3, 0.5, 0.2, 0);
    ret = contrastEnhancer->UpdateMetadataBasedOnHist(displayArea, surfaceBuffer, pixelmapInfo);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(ContrastEnhancerUnitTest, UpdateMetadataBasedOnHist_InvalidSurfaceBuffer, TestSize.Level1)
{
    OHOS::Rect displayArea = {
        .x = 0,
        .y = 0,
        .w = 1920,
        .h = 1080,
    };
    // 创建一个无效的SurfaceBuffer对象
    sptr<SurfaceBuffer> invalidSurfaceBuffer = nullptr; // 或者使用其他方式创建无效对象
    auto contrastEnhancer = ContrastEnhancerImage::Create();
    EXPECT_NE(contrastEnhancer, nullptr);
    if (contrastEnhancer == nullptr) {
        return;
    }
    ContrastEnhancerParameters param;
    param.type = ADAPTIVE_FOV;
    VPEAlgoErrCode ret = contrastEnhancer->SetParameter(param);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);

    if (!IsSupported()) {
        return;
    }

    // UniqueId, contentId, curRatio, maxRatio, minRatio, animationDuration
    std::tuple<int, int, double, double, double, int> pixelmapInfo = std::make_tuple(1, 1, 0.3, 0.5, 0.2, 0);
    ret = contrastEnhancer->UpdateMetadataBasedOnHist(displayArea, invalidSurfaceBuffer, pixelmapInfo);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// unsupport format
HWTEST_F(ContrastEnhancerUnitTest, UpdateMetadataBasedOnHist_InvalidFormat, TestSize.Level1)
{
    OHOS::Rect displayArea = {
        .x = 0,
        .y = 0,
        .w = 1920,
        .h = 1080,
    };
    sptr<SurfaceBuffer> surfaceBuffer = CreateSurfaceBuffer(OHOS::GRAPHIC_PIXEL_FMT_RGBA_8888, 1024, 768);
    EXPECT_NE(surfaceBuffer, nullptr);
    if (surfaceBuffer == nullptr) {
        return;
    }
    // 初始化surfaceBuffer
    auto contrastEnhancer = ContrastEnhancerImage::Create();
    EXPECT_NE(contrastEnhancer, nullptr);
    if (contrastEnhancer == nullptr) {
        return;
    }
    ContrastEnhancerParameters param;
    param.type = ADAPTIVE_FOV;
    VPEAlgoErrCode ret = contrastEnhancer->SetParameter(param);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);

    if (!IsSupported()) {
        return;
    }

    // UniqueId, contentId, curRatio, maxRatio, minRatio, animationDuration
    std::tuple<int, int, double, double, double, int> pixelmapInfo = std::make_tuple(1, 1, 0.3, 0.5, 0.2, 0);
    ret = contrastEnhancer->UpdateMetadataBasedOnHist(displayArea, surfaceBuffer, pixelmapInfo);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}
 
// 测试UpdateMetadataBasedOnPixel函数
HWTEST_F(ContrastEnhancerUnitTest, UpdateMetadataBasedOnPixel_NormalCase, TestSize.Level1)
{
    OHOS::Rect displayArea = {
        .x = 0,
        .y = 0,
        .w = 1920,
        .h = 1080,
    };
    OHOS::Rect curPixelmapArea = {
        .x = 0,
        .y = 0,
        .w = 960,
        .h = 540,
    };
    OHOS::Rect completePixelmapArea = {
        .x = 0,
        .y = 0,
        .w = 1920,
        .h = 1080,
    };
    sptr<SurfaceBuffer> surfaceBuffer = CreateSurfaceBuffer(OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102, 1024, 768);
    EXPECT_NE(surfaceBuffer, nullptr);
    if (surfaceBuffer == nullptr) {
        return;
    }
    // 初始化surfaceBuffer
    float fullRatio = 1.0f; // 示例值
    auto contrastEnhancer = ContrastEnhancerImage::Create();
    EXPECT_NE(contrastEnhancer, nullptr);
    if (contrastEnhancer == nullptr) {
        return;
    }
    ContrastEnhancerParameters param;
    param.type = ADAPTIVE_FOV;
    VPEAlgoErrCode ret = contrastEnhancer->SetParameter(param);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);

    if (!IsSupported()) {
        return;
    }

    ret = contrastEnhancer->UpdateMetadataBasedOnPixel(displayArea, curPixelmapArea,
        completePixelmapArea, surfaceBuffer, fullRatio);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// invalid surfacebuffer
HWTEST_F(ContrastEnhancerUnitTest, UpdateMetadataBasedOnPixel_InvalidSurfaceBuffer, TestSize.Level1)
{
    OHOS::Rect displayArea = {
        .x = 0,
        .y = 0,
        .w = 1920,
        .h = 1080,
    };
    OHOS::Rect curPixelmapArea = {
        .x = 0,
        .y = 0,
        .w = 960,
        .h = 540,
    };
    OHOS::Rect completePixelmapArea = {
        .x = 0,
        .y = 0,
        .w = 1920,
        .h = 1080,
    };
    float fullRatio = 1.0f;
    // 创建一个无效的SurfaceBuffer对象
    sptr<SurfaceBuffer> invalidSurfaceBuffer = nullptr; // 或者使用其他方式创建无效对象
    auto contrastEnhancer = ContrastEnhancerImage::Create();
    EXPECT_NE(contrastEnhancer, nullptr);
    if (contrastEnhancer == nullptr) {
        return;
    }
    ContrastEnhancerParameters param;
    param.type = ADAPTIVE_FOV;
    VPEAlgoErrCode ret = contrastEnhancer->SetParameter(param);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);

    if (!IsSupported()) {
        return;
    }

    ret = contrastEnhancer->UpdateMetadataBasedOnPixel(displayArea, curPixelmapArea,
        completePixelmapArea, invalidSurfaceBuffer, fullRatio);
    EXPECT_EQ(ret, VPE_ALGO_ERR_UNKNOWN);
}

// over max ratio
HWTEST_F(ContrastEnhancerUnitTest, UpdateMetadataBasedOnHist_MaxRatio, TestSize.Level1)
{
    OHOS::Rect displayArea = {
        .x = 0,
        .y = 0,
        .w = 1920,
        .h = 1080,
    };
    sptr<SurfaceBuffer> surfaceBuffer = CreateSurfaceBuffer(OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102, 1920, 1080);
    EXPECT_NE(surfaceBuffer, nullptr);
    if (surfaceBuffer == nullptr) {
        return;
    }
    // 初始化surfaceBuffer
    auto contrastEnhancer = ContrastEnhancerImage::Create();
    EXPECT_NE(contrastEnhancer, nullptr);
    if (contrastEnhancer == nullptr) {
        return;
    }
    ContrastEnhancerParameters param;
    param.type = ADAPTIVE_FOV;
    VPEAlgoErrCode ret = contrastEnhancer->SetParameter(param);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);

    if (!IsSupported()) {
        return;
    }

    // UniqueId, contentId, curRatio, maxRatio, minRatio, animationDuration
    std::tuple<int, int, double, double, double, int> pixelmapInfo = std::make_tuple(1, 1, 0.6, 0.5, 0.2, 0);
    ret = contrastEnhancer->UpdateMetadataBasedOnHist(displayArea, surfaceBuffer, pixelmapInfo);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// smaller than min ratio
HWTEST_F(ContrastEnhancerUnitTest, UpdateMetadataBasedOnHist_MinRatio, TestSize.Level1)
{
    OHOS::Rect displayArea = {
        .x = 0,
        .y = 0,
        .w = 1920,
        .h = 1080,
    };
    sptr<SurfaceBuffer> surfaceBuffer = CreateSurfaceBuffer(OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102, 1920, 1080);
    EXPECT_NE(surfaceBuffer, nullptr);
    if (surfaceBuffer == nullptr) {
        return;
    }
    // 初始化surfaceBuffer
    auto contrastEnhancer = ContrastEnhancerImage::Create();
    EXPECT_NE(contrastEnhancer, nullptr);
    if (contrastEnhancer == nullptr) {
        return;
    }
    ContrastEnhancerParameters param;
    param.type = ADAPTIVE_FOV;
    VPEAlgoErrCode ret = contrastEnhancer->SetParameter(param);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);

    if (!IsSupported()) {
        return;
    }

    // UniqueId, contentId, curRatio, maxRatio, minRatio, animationDuration
    std::tuple<int, int, double, double, double, int> pixelmapInfo = std::make_tuple(1, 1, 0.1, 0.5, 0.2, 0);
    ret = contrastEnhancer->UpdateMetadataBasedOnHist(displayArea, surfaceBuffer, pixelmapInfo);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// double click, animation duration > 0
HWTEST_F(ContrastEnhancerUnitTest, UpdateMetadataBasedOnHist_DoubleClick, TestSize.Level1)
{
    OHOS::Rect displayArea = {
        .x = 0,
        .y = 0,
        .w = 1920,
        .h = 1080,
    };
    sptr<SurfaceBuffer> surfaceBuffer = CreateSurfaceBuffer(OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102, 1920, 1080);
    EXPECT_NE(surfaceBuffer, nullptr);
    if (surfaceBuffer == nullptr) {
        return;
    }
    // 初始化surfaceBuffer
    auto contrastEnhancer = ContrastEnhancerImage::Create();
    EXPECT_NE(contrastEnhancer, nullptr);
    if (contrastEnhancer == nullptr) {
        return;
    }
    ContrastEnhancerParameters param;
    param.type = ADAPTIVE_FOV;
    VPEAlgoErrCode ret = contrastEnhancer->SetParameter(param);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);

    if (!IsSupported()) {
        return;
    }

    // UniqueId, contentId, curRatio, maxRatio, minRatio, animationDuration
    std::tuple<int, int, double, double, double, int> pixelmapInfo = std::make_tuple(1, 1, 0.3, 0.5, 0.2, 200);
    ret = contrastEnhancer->UpdateMetadataBasedOnHist(displayArea, surfaceBuffer, pixelmapInfo);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// invalid animation duration
HWTEST_F(ContrastEnhancerUnitTest, UpdateMetadataBasedOnHist_InvalidDuration, TestSize.Level1)
{
    OHOS::Rect displayArea = {
        .x = 0,
        .y = 0,
        .w = 1920,
        .h = 1080,
    };
    sptr<SurfaceBuffer> surfaceBuffer = CreateSurfaceBuffer(OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102, 1920, 1080);
    EXPECT_NE(surfaceBuffer, nullptr);
    if (surfaceBuffer == nullptr) {
        return;
    }
    // 初始化surfaceBuffer
    auto contrastEnhancer = ContrastEnhancerImage::Create();
    EXPECT_NE(contrastEnhancer, nullptr);
    if (contrastEnhancer == nullptr) {
        return;
    }
    ContrastEnhancerParameters param;
    param.type = ADAPTIVE_FOV;
    VPEAlgoErrCode ret = contrastEnhancer->SetParameter(param);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);

    if (!IsSupported()) {
        return;
    }

    // UniqueId, contentId, curRatio, maxRatio, minRatio, animationDuration
    std::tuple<int, int, double, double, double, int> pixelmapInfo = std::make_tuple(1, 1, 0.3, 0.5, 0.2, -200);
    ret = contrastEnhancer->UpdateMetadataBasedOnHist(displayArea, surfaceBuffer, pixelmapInfo);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

void threadFunction(std::shared_ptr<ContrastEnhancerImage> impl, OHOS::Rect rect, sptr<SurfaceBuffer> surfaceBuffer,
    std::tuple<int, int, double, double, double, int> pixelmapInfo) {
    impl->UpdateMetadataBasedOnHist(rect, surfaceBuffer, pixelmapInfo);
}


// 多线程、无动画
HWTEST_F(ContrastEnhancerUnitTest, Concurrency_scenarios, TestSize.Level1)
{
    OHOS::Rect displayArea = {
        .x = 0,
        .y = 0,
        .w = 1920,
        .h = 1080,
    };
    sptr<SurfaceBuffer> surfaceBuffer = CreateSurfaceBuffer(OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102, 1920, 1080);
    EXPECT_NE(surfaceBuffer, nullptr);
    if (surfaceBuffer == nullptr) {
        return;
    }
    // 初始化surfaceBuffer
    std::shared_ptr<ContrastEnhancerImage> contrastEnhancer = std::make_shared<ContrastEnhancerImageFwk>();
    EXPECT_NE(contrastEnhancer, nullptr);
    if (contrastEnhancer == nullptr) {
        return;
    }
    ContrastEnhancerParameters param;
    param.type = ADAPTIVE_FOV;
    VPEAlgoErrCode ret = contrastEnhancer->SetParameter(param);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);

    if (!IsSupported()) {
        return;
    }

    // UniqueId, contentId, curRatio, maxRatio, minRatio, animationDuration
    std::tuple<int, int, double, double, double, int> pixelmapInfo = std::make_tuple(1, 1, 0.3, 0.5, 0.2, 0);
    // 创建一个线程向量来存储线程对象
    std::thread threads[5];
    // 创建并启动5个线程
    for (int i = 0; i < 5; ++i) {
        threads[i] = std::thread(threadFunction, std::ref(contrastEnhancer), displayArea, surfaceBuffer, pixelmapInfo);
    }
    // 等待所有线程完成
    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }
}

// 多线程、有动画
HWTEST_F(ContrastEnhancerUnitTest, Concurrency_scenarios_with_animation, TestSize.Level1)
{
    OHOS::Rect displayArea = {
        .x = 0,
        .y = 0,
        .w = 1920,
        .h = 1080,
    };
    sptr<SurfaceBuffer> surfaceBuffer = CreateSurfaceBuffer(OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102, 1920, 1080);
    EXPECT_NE(surfaceBuffer, nullptr);
    if (surfaceBuffer == nullptr) {
        return;
    }
    // 初始化surfaceBuffer
    std::shared_ptr<ContrastEnhancerImage> contrastEnhancer = std::make_shared<ContrastEnhancerImageFwk>();
    EXPECT_NE(contrastEnhancer, nullptr);
    if (contrastEnhancer == nullptr) {
        return;
    }
    ContrastEnhancerParameters param;
    param.type = ADAPTIVE_FOV;
    VPEAlgoErrCode ret = contrastEnhancer->SetParameter(param);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);

    if (!IsSupported()) {
        return;
    }

    // UniqueId, contentId, curRatio, maxRatio, minRatio, animationDuration
    std::tuple<int, int, double, double, double, int> pixelmapInfo = std::make_tuple(1, 1, 0.3, 0.5, 0.2, 200);
    // 创建一个线程向量来存储线程对象
    std::thread threads[5];
    // 创建并启动5个线程
    for (int i = 0; i < 5; ++i) {
        threads[i] = std::thread(threadFunction, std::ref(contrastEnhancer), displayArea, surfaceBuffer, pixelmapInfo);
    }
    // 等待所有线程完成
    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }
}

// impl twice
HWTEST_F(ContrastEnhancerUnitTest, Get_Algo_Twice, TestSize.Level1)
{
    OHOS::Rect displayArea = {
        .x = 0,
        .y = 0,
        .w = 1920,
        .h = 1080,
    };
    sptr<SurfaceBuffer> surfaceBuffer = CreateSurfaceBuffer(OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102, 1920, 1080);
    EXPECT_NE(surfaceBuffer, nullptr);
    if (surfaceBuffer == nullptr) {
        return;
    }
    // 初始化surfaceBuffer
    std::shared_ptr<ContrastEnhancerImageFwk> contrastEnhancer = std::make_shared<ContrastEnhancerImageFwk>();
    EXPECT_NE(contrastEnhancer, nullptr);
    if (contrastEnhancer == nullptr) {
        return;
    }
    ContrastEnhancerParameters param;
    param.type = ADAPTIVE_FOV;
    VPEAlgoErrCode ret = contrastEnhancer->SetParameter(param);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);

    if (!IsSupported()) {
        return;
    }

    // UniqueId, contentId, curRatio, maxRatio, minRatio, animationDuration
    std::tuple<int, int, double, double, double, int> pixelmapInfo = std::make_tuple(1, 1, 0.3, 0.5, 0.2, 2200);
    ret = contrastEnhancer->UpdateMetadataBasedOnHist(displayArea, surfaceBuffer, pixelmapInfo);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// get algo failed over 5 times due to invalid enhancer type
HWTEST_F(ContrastEnhancerUnitTest, Get_Algo_Failed, TestSize.Level1)
{
    OHOS::Rect displayArea = {
        .x = 0,
        .y = 0,
        .w = 1920,
        .h = 1080,
    };
    sptr<SurfaceBuffer> surfaceBuffer = CreateSurfaceBuffer(OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102, 1920, 1080);
    EXPECT_NE(surfaceBuffer, nullptr);
    if (surfaceBuffer == nullptr) {
        return;
    }
    // 初始化surfaceBuffer
    std::shared_ptr<ContrastEnhancerImageFwk> contrastEnhancer = std::make_shared<ContrastEnhancerImageFwk>();
    EXPECT_NE(contrastEnhancer, nullptr);
    if (contrastEnhancer == nullptr) {
        return;
    }

    if (!IsSupported()) {
        return;
    }

    // UniqueId, contentId, curRatio, maxRatio, minRatio, animationDuration
    std::tuple<int, int, double, double, double, int> pixelmapInfo = std::make_tuple(1, 1, 0.3, 0.5, 0.2, 2200);
    VPEAlgoErrCode ret = contrastEnhancer->UpdateMetadataBasedOnHist(displayArea, surfaceBuffer, pixelmapInfo);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
    ret = contrastEnhancer->UpdateMetadataBasedOnHist(displayArea, surfaceBuffer, pixelmapInfo);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
    ret = contrastEnhancer->UpdateMetadataBasedOnHist(displayArea, surfaceBuffer, pixelmapInfo);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
    ret = contrastEnhancer->UpdateMetadataBasedOnHist(displayArea, surfaceBuffer, pixelmapInfo);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
    ret = contrastEnhancer->UpdateMetadataBasedOnHist(displayArea, surfaceBuffer, pixelmapInfo);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
    ret = contrastEnhancer->UpdateMetadataBasedOnHist(displayArea, surfaceBuffer, pixelmapInfo);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}