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
#include "refbase.h"
#include "surface.h"
#include "metadata_generator.h"
#include "algorithm_errors.h"

using namespace std;
using namespace testing::ext;

namespace {
constexpr int32_t WIDTH = 1920;
constexpr int32_t HEIGHT = 1080;
}

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class MGModuleTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};

    void SetParameter(std::shared_ptr<MetadataGenerator> plugin);
    sptr<SurfaceBuffer> CreateSurfaceBuffer();
};

void MGModuleTest::SetParameter(std::shared_ptr<MetadataGenerator> plugin)
{
    MetadataGeneratorParameter parameterSet;
    int32_t ret = plugin->SetParameter(parameterSet);
    ASSERT_EQ(VPE_ALGO_ERR_OK, ret);
}

sptr<SurfaceBuffer> MGModuleTest::CreateSurfaceBuffer()
{
    auto buffer = SurfaceBuffer::Create();
    if (nullptr == buffer) {
        return nullptr;
    }
    BufferRequestConfig inputCfg;
    inputCfg.width = WIDTH;
    inputCfg.height = HEIGHT;
    inputCfg.strideAlignment = WIDTH;
    inputCfg.usage = BUFFER_USAGE_HW_TEXTURE;
    inputCfg.format = GRAPHIC_PIXEL_FMT_YCBCR_420_SP;
    inputCfg.timeout = 0;
    GSError err = buffer->Alloc(inputCfg);
    if (GSERROR_OK != err) {
        return nullptr;
    }

    return buffer;
}

/**
 * @tc.number    : 0101
 * @tc.func      : Create
 * @tc.desc      : Test for MetadataGenerator Create
 */
HWTEST_F(MGModuleTest, Create_0101, TestSize.Level1)
{
    auto plugin = MetadataGenerator::Create();
    ASSERT_NE(nullptr, plugin);
}

/**
 * @tc.number    : 0201
 * @tc.func      : SetParameter
 * @tc.desc      : parameter.renderIntent != RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC
 */
HWTEST_F(MGModuleTest, SetParameter_0201, TestSize.Level1)
{
    auto plugin = MetadataGenerator::Create();
    ASSERT_NE(nullptr, plugin);
    
    MetadataGeneratorParameter parameterSet;
    int32_t ret = plugin->SetParameter(parameterSet);
    ASSERT_EQ(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0202
 * @tc.func      : SetParameter
 * @tc.desc      : parameter.renderIntent = RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC
 */
HWTEST_F(MGModuleTest, SetParameter_0202, TestSize.Level1)
{
    auto plugin = MetadataGenerator::Create();
    ASSERT_NE(nullptr, plugin);
    
    SetParameter(plugin);
}

/**
 * @tc.number    : 0301
 * @tc.func      : GetParameter
 * @tc.desc      : Call after Create
 */
HWTEST_F(MGModuleTest, GetParameter_0301, TestSize.Level1)
{
    auto plugin = MetadataGenerator::Create();
    ASSERT_NE(nullptr, plugin);

    MetadataGeneratorParameter parameterGet;
    int32_t ret = plugin->GetParameter(parameterGet);
    ASSERT_EQ(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0302
 * @tc.func      : GetParameter
 * @tc.desc      : Call after SetParameter
 */
HWTEST_F(MGModuleTest, GetParameter_0302, TestSize.Level1)
{
    auto plugin = MetadataGenerator::Create();
    ASSERT_NE(nullptr, plugin);
    
    SetParameter(plugin);

    MetadataGeneratorParameter parameterGet;
    int32_t ret = plugin->GetParameter(parameterGet);
    ASSERT_EQ(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0401
 * @tc.func      : Process
 * @tc.desc      : Call after Create
 */
HWTEST_F(MGModuleTest, Process_0401, TestSize.Level1)
{
    auto plugin = MetadataGenerator::Create();
    ASSERT_NE(nullptr, plugin);
    
    int32_t ret = plugin->Process(SurfaceBuffer::Create());
    ASSERT_NE(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0402
 * @tc.func      : Process
 * @tc.desc      : Call after SetParameter, input is null
 */
HWTEST_F(MGModuleTest, Process_0402, TestSize.Level1)
{
    auto plugin = MetadataGenerator::Create();
    ASSERT_NE(nullptr, plugin);
    
    SetParameter(plugin);

    int32_t ret = plugin->Process(nullptr);
    ASSERT_NE(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0404
 * @tc.func      : Process
 * @tc.desc      : Call after SetParameter, input is not null
 */
HWTEST_F(MGModuleTest, Process_0404, TestSize.Level1)
{
    auto plugin = MetadataGenerator::Create();
    ASSERT_NE(nullptr, plugin);
    
    SetParameter(plugin);

    auto input = CreateSurfaceBuffer();
    ASSERT_NE(nullptr, input);

    int32_t ret = plugin->Process(input);
    ASSERT_NE(VPE_ALGO_ERR_OK, ret);
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS