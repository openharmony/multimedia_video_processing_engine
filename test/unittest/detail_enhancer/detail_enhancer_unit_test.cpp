/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "graphic_common_c.h"
#include "detailEnh_sample.h"
#include "detailEnh_sample_define.h"
#include "detail_enhancer_image.h"

using namespace std;
using namespace testing::ext;

constexpr int32_t DEFAULT_FORMAT_RGBAEIGHT = OHOS::GRAPHIC_PIXEL_FMT_RGBA_8888;
constexpr int32_t DEFAULT_FORMAT_YCBCRSP = OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_SP;

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

int32_t GetFileSize(int32_t width, int32_t height, int32_t format)
{
    int32_t size = width * height;
    switch (format) {
        case OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_SP:
        case OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_P:
            size = size * 3 / 2; // 3; 2
            break;
        case OHOS::GRAPHIC_PIXEL_FMT_RGBA_8888:
            size *= 4; // 4
            break;
        default:
            size *= 3; // 3
            break;
    }
    return size;
}

class DetailEnhancerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DetailEnhancerUnitTest::SetUpTestCase(void)
{
    cout << "[SetUpTestCase]: " << endl;
}

void DetailEnhancerUnitTest::TearDownTestCase(void)
{
    cout << "[TearDownTestCase]: " << endl;
}

void DetailEnhancerUnitTest::SetUp(void)
{
    cout << "[SetUp]: SetUp!!!" << endl;
}

void DetailEnhancerUnitTest::TearDown(void)
{
    cout << "[TearDown]: over!!!" << endl;
}

// detail enhancer init
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_init_01, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    EXPECT_NE(detailEnh, nullptr);
}

// detail enhancer init meultiple times
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_init_02, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    detailEnh = DetailEnhancerImage::Create();
    detailEnh = DetailEnhancerImage::Create();
    detailEnh = DetailEnhancerImage::Create();
    detailEnh = DetailEnhancerImage::Create();
    EXPECT_NE(detailEnh, nullptr);
}

// set parameter to high
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_init_04, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    auto res = detailEnh->SetParameter(param);
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);
}

// set parameter to medium
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_init_05, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_MEDIUM,
    };
    auto res = detailEnh->SetParameter(param);
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);
}

// set parameter to low
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_init_06, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    auto res = detailEnh->SetParameter(param);
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);
}

// set parameter to none
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_init_07, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_NONE,
    };
    auto res = detailEnh->SetParameter(param);
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);
}

// get parameters after set to high
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_init_09, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .level = DETAIL_ENH_LEVEL_HIGH,
        .uri = "",
    };
    auto res_set = detailEnh->SetParameter(param);
    auto res_get = detailEnh->GetParameter(param);
    EXPECT_EQ(res_set, VPE_ALGO_ERR_OK);
    EXPECT_EQ(res_get, VPE_ALGO_ERR_OK);
}

// get parameters after set to medium
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_init_10, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .level = DETAIL_ENH_LEVEL_MEDIUM,
        .uri = "",
    };
    auto res_set = detailEnh->SetParameter(param);
    auto res_get = detailEnh->GetParameter(param);
    EXPECT_EQ(res_set, VPE_ALGO_ERR_OK);
    EXPECT_EQ(res_get, VPE_ALGO_ERR_OK);
}

// get parameters after set to low
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_init_11, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .level = DETAIL_ENH_LEVEL_LOW,
        .uri = "",
    };
    auto res_set = detailEnh->SetParameter(param);
    auto res_get = detailEnh->GetParameter(param);
    EXPECT_EQ(res_set, VPE_ALGO_ERR_OK);
    EXPECT_EQ(res_get, VPE_ALGO_ERR_OK);
}

// get parameters after set to none
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_init_12, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .level = DETAIL_ENH_LEVEL_NONE,
        .uri = "",
    };
    auto res_set = detailEnh->SetParameter(param);
    auto res_get = detailEnh->GetParameter(param);
    EXPECT_EQ(res_set, VPE_ALGO_ERR_OK);
    EXPECT_EQ(res_get, VPE_ALGO_ERR_OK);
}

// get parameters
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_init_13, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .level = DETAIL_ENH_LEVEL_HIGH,
        .uri = "",
    };
    auto res = detailEnh->GetParameter(param);
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);
}

// test getBuffer with bad size
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_init_14, TestSize.Level1)
{
    auto createdBuffer = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, -1, -1);
    EXPECT_EQ(createdBuffer, nullptr);
}

// set parameter uri to abnormal value
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_init_15, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "x",
        .level = DETAIL_ENH_LEVEL_NONE,
    };
    auto res = detailEnh->SetParameter(param);
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);
}

// process RGB to RGB, aisr
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_01, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 4096;
    int32_t outputHeight = 3072;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 1024, 768);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// process RGB to RGB, high
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_02, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// process RGB to RGB, medium
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_03, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_MEDIUM,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process RGB to RGB, low
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_04, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process RGB to RGB, none
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_05, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_NONE,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process RGB to RGB different resolution 0.5x, aisr MOVED TO VPE_EXT
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_06, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 1024, 768);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// process RGB to RGB different resolution 0.5x, high
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_07, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 1024;
    int32_t outputHeight = 768;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// process RGB to RGB different resolution 0.5x, medium
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_08, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_MEDIUM,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 1024;
    int32_t outputHeight = 768;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process RGB to RGB different resolution 0.5x, low
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_09, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 1024;
    int32_t outputHeight = 768;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process RGB to RGB different resolution 0.5x, none
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_10, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_NONE,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 1024;
    int32_t outputHeight = 768;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process RGB to RGB different resolution 2x, aisr
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_11, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 8192;
    int32_t outputHeight = 6144;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 1024, 768);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// process RGB to RGB different resolution 2x, high
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_12, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 4096;
    int32_t outputHeight = 3072;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// process RGB to RGB different resolution 2x, medium
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_13, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_MEDIUM,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 4096;
    int32_t outputHeight = 3072;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// process RGB to RGB different resolution 2x, low
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_14, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 4096;
    int32_t outputHeight = 3072;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// process RGB to RGB different resolution 2x, none
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_15, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_NONE,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 4096;
    int32_t outputHeight = 3072;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}


// process RGB to RGB unusually resolution, aisr exceed x32 scales
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_16, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 3145728;
    int32_t outputHeight = 1;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 1024, 768);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process RGB to RGB unusually resolution, high
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_17, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 3145728;
    int32_t outputHeight = 1;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process RGB to RGB unusually resolution, medium
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_18, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_MEDIUM,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 3145728;
    int32_t outputHeight = 1;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process RGB to RGB unusually resolution, low
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_19, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 3145728;
    int32_t outputHeight = 1;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process RGB to RGB unusually resolution, none
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_20, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_NONE,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 3145728;
    int32_t outputHeight = 1;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to I420; different format, aisr
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_21, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_P;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 1024, 768);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to I420; different format, high
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_22, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_P;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to I420; different format, medium
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_23, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_MEDIUM,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to I420; different format, low
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_24, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to I420; different format, none
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_25, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_NONE,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process with unsupported input Format
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_26, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_422_SP; // unsupported format
    auto input = CreateSurfaceBuffer(inputFormat, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process with unsupported output Format
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_27, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_422_SP; // unsupported format
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process RGB to RGB witout setting parameters first
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_28, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to NV12 extremly small image input, aisr
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_29, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 1;
    int32_t outputHeight = 1;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 1024, 768);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to NV12 extremly small image input, high
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_30, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 1;
    int32_t outputHeight = 1;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to NV12 extremly small image input, medium
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_31, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_MEDIUM,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 1;
    int32_t outputHeight = 1;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to NV12 extremly small image input, low
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_32, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 1;
    int32_t outputHeight = 1;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to NV12 extremly small image input, none
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_33, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_NONE,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 1;
    int32_t outputHeight = 1;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to NV12 extremly big image input, aisr
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_34, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 30720;
    int32_t outputHeight = 17280;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 1024, 768);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to NV12 extremly big image input, high
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_35, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 30720;
    int32_t outputHeight = 17280;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to NV12 extremly big image input, medium
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_36, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_MEDIUM,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 30720;
    int32_t outputHeight = 17280;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to NV12 extremly big image input, low
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_37, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 30720;
    int32_t outputHeight = 17280;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to NV12 extremly big image input, none
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_38, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_NONE,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 30720;
    int32_t outputHeight = 17280;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to NV12 odd number input, aisr MOVED TO VPE_EXT
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_39, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 1025;
    int32_t outputHeight = 767;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 1024, 768);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to NV12 odd number input, high
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_40, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 1025;
    int32_t outputHeight = 767;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to NV12 odd number input, medium
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_41, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_MEDIUM,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 1025;
    int32_t outputHeight = 767;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to NV12 odd number input, low
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_42, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 1025;
    int32_t outputHeight = 767;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to NV12 odd number input, nonw
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_43, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_NONE,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 1025;
    int32_t outputHeight = 767;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to NV12 input output different width/height ratio, aisr MOVED TO VPE_EXT, unsupport resolution
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_44, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 1024;
    int32_t outputHeight = 480;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 1024, 768);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to NV12 input output different width/height ratio, high
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_45, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 1024;
    int32_t outputHeight = 480;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to NV12 input output different width/height ratio, medium
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_46, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_MEDIUM,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 1024;
    int32_t outputHeight = 480;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to NV12 input output different width/height ratio, low
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_47, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 1024;
    int32_t outputHeight = 480;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process NV12 to NV12 input output different width/height ratio, none
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_48, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_NONE,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 1024;
    int32_t outputHeight = 480;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process but buffer is nullptr
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_49, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t ret = detailEnh->Process(nullptr, nullptr);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// repeating process I420 to I420 with set parameters
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_50, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    // repeat again with AISR
    DetailEnhancerParameters param2 {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param2);
    int32_t ret2 = detailEnh->Process(input, output);
    // repeat again with medium
    DetailEnhancerParameters param3 {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_MEDIUM,
    };
    detailEnh->SetParameter(param3);
    int32_t ret3 = detailEnh->Process(input, output);
    // repeat again with low
    DetailEnhancerParameters param4 {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    detailEnh->SetParameter(param4);
    int32_t ret4 = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
    EXPECT_EQ(ret2, VPE_ALGO_ERR_OK);
    EXPECT_NE(ret3, VPE_ALGO_ERR_OK);
    EXPECT_NE(ret4, VPE_ALGO_ERR_OK);
}

// process twice
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_51, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_YCBCRSP, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// check extern c interface
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_52, TestSize.Level1)
{
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 2048, 1536);
    DetailEnhancerLevel level = DETAIL_ENH_LEVEL_HIGH;
    void* lib = dlopen("/system/lib64/libvideoprocessingengine.z.so", RTLD_LAZY);
    if (lib == nullptr) {
        printf("cannot load vpe lib\n");
        return;
    }

    typedef int32_t (*DetailEnhancerCreate)(int32_t*);
    typedef int32_t (*DetailEnhancerProcessImage)(int32_t,
        OHNativeWindowBuffer*, OHNativeWindowBuffer*, int32_t);
    typedef int32_t (*DetailEnhancerDestroy)(int32_t*);

    auto detailEnhCreate = reinterpret_cast<DetailEnhancerCreate>(dlsym(lib, "DetailEnhancerCreate"));
    auto detailEnhProcessImage =
        reinterpret_cast<DetailEnhancerProcessImage>(dlsym(lib, "DetailEnhancerProcessImage"));
    auto detailEnhDestroy = reinterpret_cast<DetailEnhancerDestroy>(dlsym(lib, "DetailEnhancerDestroy"));

    int32_t instanceSrId = -1;
    int32_t res = detailEnhCreate(&instanceSrId);
    if (res != 0 || instanceSrId == -1) {
        detailEnhDestroy(&instanceSrId);
        dlclose(lib);
        return;
    }
    if (input == nullptr || output == nullptr) {
        detailEnhDestroy(&instanceSrId);
        dlclose(lib);
        return;
    }
    OHNativeWindowBuffer* srIn = OH_NativeWindow_CreateNativeWindowBufferFromSurfaceBuffer(&input);
    OHNativeWindowBuffer* srOut = OH_NativeWindow_CreateNativeWindowBufferFromSurfaceBuffer(&output);
    res = detailEnhProcessImage(instanceSrId, srIn, srOut, static_cast<int>(level));
    if (res != 0) {
        detailEnhDestroy(&instanceSrId);
        dlclose(lib);
        return;
    }
    res = detailEnhDestroy(&instanceSrId);
    if (res != 0) {
        dlclose(lib);
        return;
    }
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);
    res = detailEnhDestroy(&instanceSrId); // destroy twice
    dlclose(lib);
    EXPECT_NE(res, VPE_ALGO_ERR_OK);
}

// check extern c interface, create with null instance
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_53, TestSize.Level1)
{
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 2048, 1536);
    void* lib = dlopen("/system/lib64/libvideoprocessingengine.z.so", RTLD_LAZY);
    if (lib == nullptr) {
        printf("cannot load vpe lib\n");
        return;
    }
    typedef int32_t (*DetailEnhancerCreate)(int32_t*);
    auto detailEnhCreate = reinterpret_cast<DetailEnhancerCreate>(dlsym(lib, "DetailEnhancerCreate"));
    int32_t ret = detailEnhCreate(nullptr);
    dlclose(lib);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// check extern c interface, process with null output buffer
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_54, TestSize.Level1)
{
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 2048, 1536);
    DetailEnhancerLevel level = DETAIL_ENH_LEVEL_HIGH;
    void* lib = dlopen("/system/lib64/libvideoprocessingengine.z.so", RTLD_LAZY);
    if (lib == nullptr) {
        printf("cannot load vpe lib\n");
        return;
    }

    typedef int32_t (*DetailEnhancerCreate)(int32_t*);
    typedef int32_t (*DetailEnhancerProcessImage)(int32_t,
        OHNativeWindowBuffer*, OHNativeWindowBuffer*, int32_t);
    typedef int32_t (*DetailEnhancerDestroy)(int32_t*);

    auto detailEnhCreate = reinterpret_cast<DetailEnhancerCreate>(dlsym(lib, "DetailEnhancerCreate"));
    auto detailEnhProcessImage =
        reinterpret_cast<DetailEnhancerProcessImage>(dlsym(lib, "DetailEnhancerProcessImage"));
    auto detailEnhDestroy = reinterpret_cast<DetailEnhancerDestroy>(dlsym(lib, "DetailEnhancerDestroy"));

    int32_t instanceSrId = -1;
    int32_t res = detailEnhCreate(&instanceSrId);
    if (res != 0 || instanceSrId == -1) {
        detailEnhDestroy(&instanceSrId);
        dlclose(lib);
        return;
    }
    if (input == nullptr || output == nullptr) {
        detailEnhDestroy(&instanceSrId);
        dlclose(lib);
        return;
    }
    OHNativeWindowBuffer* srIn = OH_NativeWindow_CreateNativeWindowBufferFromSurfaceBuffer(&input);
    res = detailEnhProcessImage(instanceSrId, srIn, nullptr, static_cast<int>(level));
    if (res != 0) {
        detailEnhDestroy(&instanceSrId);
        dlclose(lib);
        return;
    }
    res = detailEnhDestroy(&instanceSrId);
    if (res != 0) {
        dlclose(lib);
        return;
    }
    dlclose(lib);
    EXPECT_NE(res, VPE_ALGO_ERR_OK);
}

// check extern c interface, destroy nullptr
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_55, TestSize.Level1)
{
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 2048, 1536);
    DetailEnhancerLevel level = DETAIL_ENH_LEVEL_HIGH;
    void* lib = dlopen("/system/lib64/libvideoprocessingengine.z.so", RTLD_LAZY);
    if (lib == nullptr) {
        printf("cannot load vpe lib\n");
        return;
    }

    typedef int32_t (*DetailEnhancerCreate)(int32_t*);
    typedef int32_t (*DetailEnhancerProcessImage)(int32_t,
        OHNativeWindowBuffer*, OHNativeWindowBuffer*, int32_t);
    typedef int32_t (*DetailEnhancerDestroy)(int32_t*);

    auto detailEnhCreate = reinterpret_cast<DetailEnhancerCreate>(dlsym(lib, "DetailEnhancerCreate"));
    auto detailEnhProcessImage =
        reinterpret_cast<DetailEnhancerProcessImage>(dlsym(lib, "DetailEnhancerProcessImage"));
    auto detailEnhDestroy = reinterpret_cast<DetailEnhancerDestroy>(dlsym(lib, "DetailEnhancerDestroy"));

    int32_t instanceSrId = -1;
    int32_t res = detailEnhCreate(&instanceSrId);
    if (res != 0 || instanceSrId == -1) {
        detailEnhDestroy(&instanceSrId);
        dlclose(lib);
        return;
    }
    if (input == nullptr || output == nullptr) {
        detailEnhDestroy(&instanceSrId);
        dlclose(lib);
        return;
    }
    OHNativeWindowBuffer* srIn = OH_NativeWindow_CreateNativeWindowBufferFromSurfaceBuffer(&input);
    OHNativeWindowBuffer* srOut = OH_NativeWindow_CreateNativeWindowBufferFromSurfaceBuffer(&output);
    res = detailEnhProcessImage(instanceSrId, srIn, srOut, static_cast<int>(level));
    if (res != 0) {
        detailEnhDestroy(&instanceSrId);
        dlclose(lib);
        return;
    }
    res = detailEnhDestroy(nullptr);
    if (res != 0) {
        dlclose(lib);
        return;
    }
    dlclose(lib);
    EXPECT_NE(res, VPE_ALGO_ERR_OK);
}

// process BGR to BGR, aisr
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_56, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 4096;
    int32_t outputHeight = 3072;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
    auto input = CreateSurfaceBuffer(inputFormat, 1024, 768);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// process BGR to BGR, high
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_57, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
    auto input = CreateSurfaceBuffer(inputFormat, 4096, 3072);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// process BGR to BGR, medium
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_58, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_MEDIUM,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
    auto input = CreateSurfaceBuffer(inputFormat, 4096, 3072);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process BGR to BGR, low
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_59, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
    auto input = CreateSurfaceBuffer(inputFormat, 4096, 3072);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// process BGR to BGR, none
HWTEST_F(DetailEnhancerUnitTest, detailenhancer_process_60, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_NONE,
    };
    detailEnh->SetParameter(param);
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
    auto input = CreateSurfaceBuffer(inputFormat, 4096, 3072);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process YCRCB_420_SP, aisr MOVED TO VPE_EXT
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_01, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 1024;
    int32_t inputHeight = 768;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process YCRCB_420_SP, high
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_02, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process YCRCB_420_SP, medium
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_03, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_MEDIUM,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process YCRCB_420_SP, low
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_04, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process YCRCB_420_SP, none
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_05, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_NONE,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process YCBCR_420_P, aisr MOVED TO VPE_EXT
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_06, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 1024;
    int32_t inputHeight = 768;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_P;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_P;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process YCBCR_420_P, high
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_07, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_P;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_P;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process YCBCR_420_P, medium
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_08, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_MEDIUM,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_P;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_P;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process YCBCR_420_P, low
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_09, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_P;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_P;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process YCBCR_420_P, none
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_10, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_NONE,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_P;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_P;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process YCRCB_420_P, aisr MOVED TO VPE_EXT
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_11, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 1024;
    int32_t inputHeight = 768;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_P;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_P;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process YCRCB_420_P, high
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_12, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_P;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_P;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process YCRCB_420_P, medium
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_13, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_MEDIUM,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_P;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_P;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process YCRCB_420_P, low
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_14, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_P;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_P;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process YCRCB_420_P, none
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_15, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_NONE,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_P;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_P;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process invalid resolution
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_16, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 0;
    int32_t inputHeight = 0;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process invalid resolution
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_17, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 1024;
    int32_t inputHeight = 768;
    int32_t outputWidth = 0;
    int32_t outputHeight = 0;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process null output buffer
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_18, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_P;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_P;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, nullptr);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// Exceeds minimum reduction ratio
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_19, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2;
    int32_t outputHeight = 1;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_P;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_P;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, nullptr);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process BGRA_8888, aisr MOVED TO VPE_EXT
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_20, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 1024;
    int32_t inputHeight = 768;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process BGRA_8888, high
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_21, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process BGRA_8888, medium
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_22, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_MEDIUM,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process BGRA_8888, low
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_23, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process BGRA_8888, none
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_24, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_NONE,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// check enforce extension extream vision engine, process RGBA1010102, HIGH
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_25, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// check enforce extension extream vision engine, process RGBA1010102, MEDIUM
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_26, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_MEDIUM,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);  // medium not support RGB1010102
}

// check enforce extension extream vision engine, process RGBA1010102, LOW
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_27, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK); // low not support RGB1010102
}

// check enforce extension extream vision engine, process RGBA1010102, NONE
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_28, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_NONE,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK); // none not support RGB1010102
}

// check extension extream vision engine, process RGBA1010102, HIGH
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_29, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// check extension extream vision engine, process RGBA1010102, HIGH, format is not consistent
HWTEST_F(DetailEnhancerUnitTest, extream_vision_engine_process_30, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// normal process of aisr, AISR MOVED TO VPE_EXT
HWTEST_F(DetailEnhancerUnitTest, aisr_process_9, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 1024;
    int32_t inputHeight = 768;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = detailEnh->Process(input, output); // try to process twice
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// normal process of aisr, high
HWTEST_F(DetailEnhancerUnitTest, aisr_process_10, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_HIGH,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
    ret = detailEnh->Process(input, output); // try to process twice
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// normal process of aisr, medium
HWTEST_F(DetailEnhancerUnitTest, aisr_process_11, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_MEDIUM,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
    ret = detailEnh->Process(input, output); // try to process twice
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// normal process of aisr, low
HWTEST_F(DetailEnhancerUnitTest, aisr_process_12, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_LOW,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
    ret = detailEnh->Process(input, output); // try to process twice
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// normal process of aisr, none
HWTEST_F(DetailEnhancerUnitTest, aisr_process_13, TestSize.Level1)
{
    auto detailEnh = DetailEnhancerImage::Create();
    DetailEnhancerParameters param {
        .uri = "",
        .level = DETAIL_ENH_LEVEL_NONE,
    };
    detailEnh->SetParameter(param);
    int32_t inputWidth = 4096;
    int32_t inputHeight = 3072;
    int32_t outputWidth = 2048;
    int32_t outputHeight = 1536;
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    int32_t outputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP;
    auto input = CreateSurfaceBuffer(inputFormat, inputWidth, inputHeight);
    auto output = CreateSurfaceBuffer(outputFormat, outputWidth, outputHeight);
    int32_t ret = detailEnh->Process(input, output);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
    ret = detailEnh->Process(input, output); // try to process twice
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

// destroy twice
HWTEST_F(DetailEnhancerUnitTest, aisr_process_1, TestSize.Level1)
{
    auto input = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 4096, 3072);
    auto output = CreateSurfaceBuffer(DEFAULT_FORMAT_RGBAEIGHT, 2048, 1536);
    DetailEnhancerLevel level = DETAIL_ENH_LEVEL_HIGH;
    void* lib = dlopen("/system/lib64/libvideoprocessingengine.z.so", RTLD_LAZY);
    if (lib == nullptr) {
        printf("cannot load vpe lib\n");
        return;
    }

    typedef int32_t (*DetailEnhancerCreate)(int32_t*);
    typedef int32_t (*DetailEnhancerProcessImage)(int32_t,
        OHNativeWindowBuffer*, OHNativeWindowBuffer*, int32_t);
    typedef int32_t (*DetailEnhancerDestroy)(int32_t*);

    auto detailEnhCreate = reinterpret_cast<DetailEnhancerCreate>(dlsym(lib, "DetailEnhancerCreate"));
    auto detailEnhProcessImage =
        reinterpret_cast<DetailEnhancerProcessImage>(dlsym(lib, "DetailEnhancerProcessImage"));
    auto detailEnhDestroy = reinterpret_cast<DetailEnhancerDestroy>(dlsym(lib, "DetailEnhancerDestroy"));

    int32_t instanceSrId = -1;
    int32_t res = detailEnhCreate(&instanceSrId);
    if (res != 0 || instanceSrId == -1) {
        detailEnhDestroy(&instanceSrId);
        dlclose(lib);
        return;
    }
    if (input == nullptr || output == nullptr) {
        detailEnhDestroy(&instanceSrId);
        dlclose(lib);
        return;
    }
    OHNativeWindowBuffer* srIn = OH_NativeWindow_CreateNativeWindowBufferFromSurfaceBuffer(&input);
    OHNativeWindowBuffer* srOut = OH_NativeWindow_CreateNativeWindowBufferFromSurfaceBuffer(&output);
    res = detailEnhProcessImage(instanceSrId, srIn, srOut, static_cast<int>(level));
    if (res != 0) {
        detailEnhDestroy(&instanceSrId);
        dlclose(lib);
        return;
    }
    res = detailEnhDestroy(&instanceSrId);
    if (res != 0) {
        EXPECT_EQ(res, VPE_ALGO_ERR_OK);
        dlclose(lib);
        return;
    }
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);
    res = detailEnhDestroy(&instanceSrId); // destroy twice
    dlclose(lib);
    EXPECT_NE(res, VPE_ALGO_ERR_OK);
}

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
