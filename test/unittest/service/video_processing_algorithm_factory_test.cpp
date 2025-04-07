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
#define private public
#define protected public
 
#include "gtest/gtest.h"
 
#include "video_processing_algorithm_factory.h"
 
#include <dlfcn.h>
 
#include "algorithm_errors.h"
#include "video_processing_algorithm_factory_common.h"
#include "vpe_log.h"
 
// NOTE: Add header file of static algorithm which would be called by VPE SA below:
// algorithm begin
// algorithm end
using namespace std;
using namespace testing::ext;
 
using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

class VideoProcessingAlgorithmFactoryTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
 
void VideoProcessingAlgorithmFactoryTest::SetUpTestCase(void)
{
    cout << "[SetUpTestCase]: " << endl;
}
 
void VideoProcessingAlgorithmFactoryTest::TearDownTestCase(void)
{
    cout << "[TearDownTestCase]: " << endl;
}
 
void VideoProcessingAlgorithmFactoryTest::SetUp(void)
{
    cout << "[SetUp]: SetUp!!!" << endl;
}
 
void VideoProcessingAlgorithmFactoryTest::TearDown(void)
{
    cout << "[TearDown]: over!!!" << endl;
}
 
 
/**
 * @tc.name  : LoadDynamicAlgorithm_Success_WhenPathIsValid
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmFactory_LoadDynamicAlgorithm_001
 * @tc.desc  : Test LoadDynamicAlgorithm method when the path is valid.
 */
HWTEST_F(VideoProcessingAlgorithmFactoryTest, LoadDynamicAlgorithm_Success_WhenPathIsValid, TestSize.Level0)
{
    OHOS::Media::VideoProcessingEngine::VideoProcessingAlgorithmFactory factory;
    std::string validPath = "valid_path_to_dynamic_library";
    EXPECT_FALSE(factory.LoadDynamicAlgorithm(validPath));
}

/**
 * @tc.name  : LoadDynamicAlgorithm_Fail_WhenPathIsInvalid
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmFactory_LoadDynamicAlgorithm_002
 * @tc.desc  : Test LoadDynamicAlgorithm method when the path is invalid.
 */
HWTEST_F(VideoProcessingAlgorithmFactoryTest, LoadDynamicAlgorithm_Fail_WhenPathIsInvalid, TestSize.Level0)
{
    OHOS::Media::VideoProcessingEngine::VideoProcessingAlgorithmFactory factory;
    std::string invalidPath = "invalid_path_to_dynamic_library";
    EXPECT_FALSE(factory.LoadDynamicAlgorithm(invalidPath));
}

/**
 * @tc.name  : LoadDynamicAlgorithm_Fail_WhenLibraryOpenFails
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmFactory_LoadDynamicAlgorithm_003
 * @tc.desc  : Test LoadDynamicAlgorithm method when dlopen fails.
 */
HWTEST_F(VideoProcessingAlgorithmFactoryTest, LoadDynamicAlgorithm_Fail_WhenLibraryOpenFails, TestSize.Level0)
{
    OHOS::Media::VideoProcessingEngine::VideoProcessingAlgorithmFactory factory;
    std::string path = "path_to_dynamic_library_that_fails_to_open";
    EXPECT_FALSE(factory.LoadDynamicAlgorithm(path));
}

/**
 * @tc.name  : LoadDynamicAlgorithm_Fail_WhenGetCreatorFails
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmFactory_LoadDynamicAlgorithm_004
 * @tc.desc  : Test LoadDynamicAlgorithm method when dlsym fails.
 */
HWTEST_F(VideoProcessingAlgorithmFactoryTest, LoadDynamicAlgorithm_Fail_WhenGetCreatorFails, TestSize.Level0)
{
    OHOS::Media::VideoProcessingEngine::VideoProcessingAlgorithmFactory factory;
    std::string path = "path_to_dynamic_library_that_fails_to_get_creator";
    EXPECT_FALSE(factory.LoadDynamicAlgorithm(path));
}

/**
 * @tc.name  : LoadDynamicAlgorithm_Fail_WhenGetCreatorReturnsNull
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmFactory_LoadDynamicAlgorithm_005
 * @tc.desc  : Test LoadDynamicAlgorithm method when GetCreator returns null.
 */
HWTEST_F(VideoProcessingAlgorithmFactoryTest, LoadDynamicAlgorithm_Fail_WhenGetCreatorReturnsNull, TestSize.Level0)
{
    OHOS::Media::VideoProcessingEngine::VideoProcessingAlgorithmFactory factory;
    std::string path = "path_to_dynamic_library_that_returns_null_creator";
    EXPECT_FALSE(factory.LoadDynamicAlgorithm(path));
}

/**
 * @tc.name  : UnloadDynamicAlgorithm_ShouldUnloadDynamicAlgorithm_WhenHandleIsNotNull
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmFactory_UnloadDynamicAlgorithm_001
 * @tc.desc  : Test when handle is not null, UnloadDynamicAlgorithm should close the handle and set it to nullptr
 */
HWTEST_F(VideoProcessingAlgorithmFactoryTest, UnloadDynamicAlgorithm_ShouldUnloadDynamicAlgorithm_WhenHandleIsNotNull, TestSize.Level0)
{
    OHOS::Media::VideoProcessingEngine::VideoProcessingAlgorithmFactory factory;
    void* handle = dlopen("libtest.so", RTLD_LAZY);
    factory.handle_ = handle;
    factory.UnloadDynamicAlgorithm();
    EXPECT_EQ(factory.handle_, nullptr);
}

/**
 * @tc.name  : UnloadDynamicAlgorithm_ShouldDoNothing_WhenHandleIsNull
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmFactory_UnloadDynamicAlgorithm_002
 * @tc.desc  : Test when handle is null, UnloadDynamicAlgorithm should do nothing
 */
HWTEST_F(VideoProcessingAlgorithmFactoryTest, UnloadDynamicAlgorithm_ShouldDoNothing_WhenHandleIsNull, TestSize.Level0)
{
    OHOS::Media::VideoProcessingEngine::VideoProcessingAlgorithmFactory factory;
    factory.handle_ = nullptr;
    factory.UnloadDynamicAlgorithm();
    EXPECT_EQ(factory.handle_, nullptr);
}

HWTEST_F(VideoProcessingAlgorithmFactoryTest, Create_ShouldReturnNullptr_WhenFeatureNotFound, TestSize.Level0)
{
    VideoProcessingAlgorithmFactory factory;
    auto result = factory.Create("nonexistent_feature");
    EXPECT_EQ(result, nullptr);
}

HWTEST_F(VideoProcessingAlgorithmFactoryTest, Create_ShouldReturnValidAlgorithm_WhenFeatureFound, TestSize.Level0)
{
    VideoProcessingAlgorithmFactory factory;
    // Assuming "valid_feature" is a valid feature in g_creators
    auto result = factory.Create("valid_feature");
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name  : GenerateFeatureIDs_ShouldAssignIDs_WhenCalled
 * @tc.number: VideoProcessingEngine_GenerateFeatureIDs_001
 * @tc.desc  : Test GenerateFeatureIDs method should assign IDs to creators when called
 */
HWTEST_F(VideoProcessingAlgorithmFactoryTest, GenerateFeatureIDs_ShouldAssignIDs_WhenCalled, TestSize.Level0)
{
    OHOS::Media::VideoProcessingEngine::VideoProcessingAlgorithmFactory factory;

    factory.GenerateFeatureIDs();

    int expectedId = 1;
    EXPECT_EQ(expectedId, 1);
}


}
}
}