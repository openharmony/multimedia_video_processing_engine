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

#include <iostream>
#include <gtest/gtest.h>
#include "plugin_manager.h"
#include "video_processing_engine.h"
#include "vpe_errors.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class VPEFrameworkUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * tc.number desc
 *                      |01|01|01|
 *         ClassNo ------|  |  |------ FuncTestNo
 *                          |
 *                        FuncNo
 */

/**
 * @tc.number    : 010101
 * @tc.name      : PluginManager_Init
 * @tc.desc      : Test for PluginManager Init
 */
HWTEST_F(VPEFrameworkUnitTest, PluginManager_Init_010101, TestSize.Level1)
{
    std::unique_ptr<Plugin::PluginManager> pluginManager = std::make_unique<Plugin::PluginManager>();
    ASSERT_EQ(VPE_ERR_OK, pluginManager->Init());
}

/**
 * @tc.number    : 010201
 * @tc.name      : PluginManager_FindColorSpaceConverterPlugin
 * @tc.desc      : Test for PluginManager FindColorSpaceConverterPlugin
 */
HWTEST_F(VPEFrameworkUnitTest, PluginManager_FindColorSpaceConverterPlugin_010201, TestSize.Level1)
{
    std::unique_ptr<Plugin::PluginManager> pluginManager = std::make_unique<Plugin::PluginManager>();
    ASSERT_EQ(VPE_ERR_OK, pluginManager->Init());

    ColorSpaceDescription inputDesc = {}, outputDesc = {};
    auto ret = pluginManager->FindColorSpaceConverterPlugin(inputDesc, outputDesc);
    std::cout << "Find plugin " << (ret == nullptr ? "false" : "successfully");
}

/**
 * @tc.number    : 010301
 * @tc.name      : PluginManager_FindColorSpaceConverterDisplayPlugin
 * @tc.desc      : Test for PluginManager FindColorSpaceConverterDisplayPlugin
 */
HWTEST_F(VPEFrameworkUnitTest, PluginManager_FindColorSpaceConverterDisplayPlugin_010301, TestSize.Level1)
{
    std::unique_ptr<Plugin::PluginManager> pluginManager = std::make_unique<Plugin::PluginManager>();
    ASSERT_EQ(VPE_ERR_OK, pluginManager->Init());

    auto ret = pluginManager->FindColorSpaceConverterDisplayPlugin();
    std::cout << "Find plugin " << (ret.empty() ? "false" : "successfully") << endl;
}

/**
 * @tc.number    : 010401
 * @tc.name      : PluginManager_FindMetadataGeneratorPlugin
 * @tc.desc      : Test for PluginManager FindMetadataGeneratorPlugin
 */
HWTEST_F(VPEFrameworkUnitTest, PluginManager_FindMetadataGeneratorPlugin_010401, TestSize.Level1)
{
    std::unique_ptr<Plugin::PluginManager> pluginManager = std::make_unique<Plugin::PluginManager>();
    ASSERT_EQ(VPE_ERR_OK, pluginManager->Init());

    ColorSpaceDescription inputDesc = {};
    auto ret = pluginManager->FindMetadataGeneratorPlugin(inputDesc);
    std::cout << "Find plugin " << (ret == nullptr ? "false" : "successfully") << endl;
}

/**
 * @tc.number    : 020101
 * @tc.name      : VideoProcessingEngine_CreateColorSpaceConverter
 * @tc.desc      : Test for VideoProcessingEngine CreateColorSpaceConverter
 */
HWTEST_F(VPEFrameworkUnitTest, VideoProcessingEngine_CreateColorSpaceConverter_020101, TestSize.Level1)
{
    int ret = 0;
    auto &vpe = VideoProcessingEngine::GetInstance();

    ColorSpaceDescription inputDesc = {}, outputDesc = {};
    auto impl = vpe.CreateColorSpaceConverter(inputDesc, outputDesc);
    if (impl == nullptr) {
        ret = -1;
    }
    cout << "Create ColorSpaceConverter " << (impl == nullptr ? "failed" : "successfully") << endl;
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number    : 020201
 * @tc.name      : VideoProcessingEngine_CreateColorSpaceConverterDisplay
 * @tc.desc      : Test for VideoProcessingEngine CreateColorSpaceConverterDisplay
 */
HWTEST_F(VPEFrameworkUnitTest, VideoProcessingEngine_CreateColorSpaceConverterDisplay_020201, TestSize.Level1)
{
    int ret = 0;
    auto &vpe = VideoProcessingEngine::GetInstance();

    auto ret = vpe.CreateColorSpaceConverterDisplay();
    if (ret.empty()) {
        ret = -1;
    }
    cout << "Create CreateColorSpaceConverterDisplay " << (ret.empty() ? "failed" : "successfully") << endl;
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number    : 020301
 * @tc.name      : VideoProcessingEngine_CreateMetadataGenerator
 * @tc.desc      : Test for VideoProcessingEngine CreateMetadataGenerator
 */
HWTEST_F(VPEFrameworkUnitTest, VideoProcessingEngine_CreateMetadataGenerator_020301, TestSize.Level1)
{
    int ret = 0;
    auto &vpe = VideoProcessingEngine::GetInstance();

    ColorSpaceDescription inputDesc = {};
    auto impl = vpe.CreateMetadataGenerator(inputDesc);
    if (impl == nullptr) {
        ret = -1;
    }
    cout << "Create CreateMetadataGenerator " << (impl == nullptr ? "failed" : "successfully") << endl;
    EXPECT_EQ(ret, 0);
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS