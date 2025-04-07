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
 
#include "vpe_sa_utils.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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

class VpeSaUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
 
void VpeSaUtilsTest::SetUpTestCase(void)
{
    cout << "[SetUpTestCase]: " << endl;
}
 
void VpeSaUtilsTest::TearDownTestCase(void)
{
    cout << "[TearDownTestCase]: " << endl;
}
 
void VpeSaUtilsTest::SetUp(void)
{
    cout << "[SetUp]: SetUp!!!" << endl;
}
 
void VpeSaUtilsTest::TearDown(void)
{
    cout << "[TearDown]: over!!!" << endl;
}

/**
 * @tc.name  : GetProcessName_ShouldReturnProcessName_WhenProcessExists
 * @tc.number: VpeSaUtilsTest_001
 * @tc.desc  : Test GetProcessName method when process exists.
 */
TEST_F(VpeSaUtilsTest, GetProcessName_ShouldReturnProcessName_WhenProcessExists)
{
    VpeSaUtils vpeSaUtils;
    std::string processName = vpeSaUtils.GetProcessName();
    EXPECT_FALSE(processName.empty());
}

}
}
}
 