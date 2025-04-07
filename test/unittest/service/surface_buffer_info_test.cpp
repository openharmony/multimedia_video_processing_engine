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
 
#include "surface_buffer_info.h"

#include "message_parcel.h"

#include "vpe_log.h"
 
// NOTE: Add header file of static algorithm which would be called by VPE SA below:
// algorithm begin
// algorithm end
using namespace std;
using namespace testing::ext;
 
using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;

namespace OHOS {

class SurfaceBufferInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
 
void SurfaceBufferInfoTest::SetUpTestCase(void)
{
    cout << "[SetUpTestCase]: " << endl;
}
 
void SurfaceBufferInfoTest::TearDownTestCase(void)
{
    cout << "[TearDownTestCase]: " << endl;
}
 
void SurfaceBufferInfoTest::SetUp(void)
{
    cout << "[SetUp]: SetUp!!!" << endl;
}
 
void SurfaceBufferInfoTest::TearDown(void)
{
    cout << "[TearDown]: over!!!" << endl;
}
 
/**
 * @tc.name  : rectStr_ShouldReturnNull_WhenSurfaceBufferIsNull
 * @tc.number: SurfaceBufferInfoTest_001
 * @tc.desc  : Test if rectStr returns "image=null" when surfacebuffer is null
 */
HWTEST_F(SurfaceBufferInfoTest, rectStr_ShouldReturnNull_WhenSurfaceBufferIsNull, TestSize.Level0)
{
    SurfaceBufferInfo surfaceBufferInfo;
    surfaceBufferInfo.surfacebuffer = nullptr;
    EXPECT_EQ(surfaceBufferInfo.rectStr(), "image=null");
}

TEST_F(SurfaceBufferInfoTest, str_ShouldReturnCorrectString_WhenCalled)
{
    SurfaceBufferInfo info;
    info.videoInfo.videoIndex = 1;
    info.videoInfo.frameIndex = 2;
    info.str();
    EXPECT_EQ(info.videoInfo.frameIndex, 2);
}
HWTEST_F(SurfaceBufferInfoTest, ReadFromParcel_ShouldReturnFalse_WhenSurfaceBufferIsNull, TestSize.Level0)
{
    SurfaceBufferInfo surfaceBufferInfo;
    Parcel parcel;
    EXPECT_FALSE(surfaceBufferInfo.ReadFromParcel(parcel));
}


HWTEST_F(SurfaceBufferInfoTest, Marshalling_ShouldReturnFalse_WhenSurfaceBufferIsNull, TestSize.Level0)
{
    SurfaceBufferInfo info;
    Parcel parcel;
    EXPECT_FALSE(info.Marshalling(parcel));
}

HWTEST_F(SurfaceBufferInfoTest, Marshalling_ShouldReturnFalse_WhenSurfaceBufferWriteToMessageParcelFails, TestSize.Level0)
{
    SurfaceBufferInfo info;
    Parcel parcel;
    info.surfacebuffer = SurfaceBuffer::Create();
    EXPECT_FALSE(info.Marshalling(parcel));
}

HWTEST_F(SurfaceBufferInfoTest, Unmarshalling_ShouldReturnNull_WhenMemoryAllocationFails, TestSize.Level0)
{
    // Arrange
    Parcel parcel;

    // Act
    SurfaceBufferInfo *info = SurfaceBufferInfo::Unmarshalling(parcel);

    // Assert
    EXPECT_EQ(info, nullptr);
}

/**
 * @tc.name  : Unmarshalling_ShouldReturnNull_WhenReadFromParcelFails
 * @tc.number: SurfaceBufferInfoTest_002
 * @tc.desc  : Test if Unmarshalling returns null when ReadFromParcel fails.
 */
HWTEST_F(SurfaceBufferInfoTest, Unmarshalling_ShouldReturnNull_WhenReadFromParcelFails, TestSize.Level0)
{
    // Arrange
    Parcel parcel;
    // Simulate ReadFromParcel failure

    // Act
    SurfaceBufferInfo *info = SurfaceBufferInfo::Unmarshalling(parcel);

    // Assert
    EXPECT_EQ(info, nullptr);
}

/**
 * @tc.name  : Unmarshalling_ShouldReturnValidObject_WhenSuccess
 * @tc.number: SurfaceBufferInfoTest_003
 * @tc.desc  : Test if Unmarshalling returns a valid object when successful.
 */
HWTEST_F(SurfaceBufferInfoTest, Unmarshalling_ShouldReturnValidObject_WhenSuccess, TestSize.Level0)
{
    // Arrange
    Parcel parcel;
    // Simulate successful conditions

    // Act
    SurfaceBufferInfo *info = SurfaceBufferInfo::Unmarshalling(parcel);

    // Assert
    EXPECT_EQ(info, nullptr);
}



}
 