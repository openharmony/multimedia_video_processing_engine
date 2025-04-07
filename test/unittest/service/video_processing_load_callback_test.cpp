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

#include "video_processing_load_callback.h"
#include "video_processing_client.h"
#include "vpe_log.h"


using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

class VideoProcessingLoadCallbackTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void VideoProcessingLoadCallbackTest::SetUpTestCase(void)
{
    cout << "[SetUpTestCase]: " << endl;
}

void VideoProcessingLoadCallbackTest::TearDownTestCase(void)
{
    cout << "[TearDownTestCase]: " << endl;
}

void VideoProcessingLoadCallbackTest::SetUp(void)
{
    cout << "[SetUp]: SetUp!!!" << endl;
    // 在每个测试用例开始前调用
}

void VideoProcessingLoadCallbackTest::TearDown(void)
{
    cout << "[TearDown]: over!!!" << endl;
    // 在每个测试用例结束后调用
}


TEST_F(VideoProcessingLoadCallbackTest, OnLoadSystemAbilitySuccess) {
    VPE_LOGI("OnLoadSystemAbilitySuccess");
    int32_t systemAbilityId = 123;
    sptr<OHOS::IRemoteObject> remoteObject;
    VideoProcessingLoadCallback callback;
    callback.OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);
    EXPECT_EQ(systemAbilityId, 123);
    VPE_LOGI("OnLoadSystemAbilitySuccess end");
}

TEST_F(VideoProcessingLoadCallbackTest, OnLoadSystemAbilityFail) {
    VPE_LOGI("OnLoadSystemAbilityFail");
    int32_t systemAbilityId = 123;
    VideoProcessingLoadCallback callback;
    callback.OnLoadSystemAbilityFail(systemAbilityId);

    EXPECT_EQ(systemAbilityId, 123);
    VPE_LOGI("OnLoadSystemAbilityFail end");
}

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
