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
#include "aihdr_enhancer_fwk.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

sptr<SurfaceBuffer> CreateSurfaceBuffer(uint32_t pixelFormat, int32_t width, int32_t height)
{
    auto buffer = SurfaceBuffer::Create();
    if (nullptr == buffer) {
        printf("Create surface buffer failed\n");
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
    GSError err = buffer->Alloc(inputCfg);
    if (GSERROR_OK != err) {
        printf("Alloc surface buffer failed\n");
        return nullptr;
    }
    return buffer;
}

class AihdrEnhancerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AihdrEnhancerUnitTest::SetUpTestCase(void)
{
    cout << "[SetUpTestCase]: " << endl;
}

void AihdrEnhancerUnitTest::TearDownTestCase(void)
{
    cout << "[TearDownTestCase]: " << endl;
}

void AihdrEnhancerUnitTest::SetUp(void)
{
    cout << "[SetUp]: SetUp!!!" << endl;
}

void AihdrEnhancerUnitTest::TearDown(void)
{
    cout << "[TearDown]: over!!!" << endl;
}

// aihdr enhancer create
HWTEST_F(AihdrEnhancerUnitTest, aihdrenhancer_create_01, TestSize.Level1)
{
    auto aihdrEnh = AihdrEnhancer::Create();
    EXPECT_NE(aihdrEnh, nullptr);
}

// aihdr enhancer create meultiple times
HWTEST_F(AihdrEnhancerUnitTest, aihdrenhancer_create_02, TestSize.Level1)
{
    auto aihdrEnh = AihdrEnhancer::Create();
    aihdrEnh = AihdrEnhancer::Create();
    aihdrEnh = AihdrEnhancer::Create();
    aihdrEnh = AihdrEnhancer::Create();
    aihdrEnh = AihdrEnhancer::Create();
    EXPECT_NE(aihdrEnh, nullptr);
}

// set parameter
HWTEST_F(AihdrEnhancerUnitTest, aihdrenhancer_setparameter_01, TestSize.Level1)
{
    auto aihdrEnh = AihdrEnhancer::Create();
    int param = 1;
    auto res = aihdrEnh->SetParameter(param);
    EXPECT_EQ(res, VPE_ALGO_ERR_OK);
}

// get parameter
HWTEST_F(AihdrEnhancerUnitTest, aihdrenhancer_getparameter_01, TestSize.Level1)
{
    auto aihdrEnh = AihdrEnhancer::Create();
    int param = 1;
    int paramTemp = 0;
    auto resSet = aihdrEnh->SetParameter(param);
    auto resGet = aihdrEnh->GetParameter(paramTemp);
    EXPECT_EQ(resSet, VPE_ALGO_ERR_OK);
    EXPECT_EQ(resGet, VPE_ALGO_ERR_OK);
    EXPECT_EQ(param, paramTemp);
}

// process
HWTEST_F(AihdrEnhancerUnitTest, aihdrenhancer_process_01, TestSize.Level1)
{
    auto aihdrEnh = AihdrEnhancer::Create();
    int param = 1;
    aihdrEnh->SetParameter(param);
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_SP;
    sptr<SurfaceBuffer> input = CreateSurfaceBuffer(inputFormat, 1024, 768);
    auto ret = aihdrEnh->Process(input);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

// process
HWTEST_F(AihdrEnhancerUnitTest, aihdrenhancer_process_02, TestSize.Level1)
{
    auto aihdrEnh = AihdrEnhancer::Create();
    int param = 1;
    aihdrEnh->SetParameter(param);
    int32_t inputFormat = OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_SP;
    sptr<SurfaceBuffer> input = CreateSurfaceBuffer(inputFormat, 1024, 768);
    input = nullptr;
    auto ret = aihdrEnh->Process(input);
    EXPECT_EQ(ret, VPE_ALGO_ERR_INVALID_VAL);
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
