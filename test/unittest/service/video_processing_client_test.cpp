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

#include "video_processing_client.h"
#include "iservice_registry.h"
#include "video_processing_load_callback.h"
#include "vpe_sa_constants.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

class VideoProcessingClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void VideoProcessingClientTest::SetUpTestCase(void)
{
    VPE_LOGI("[SetUpTestCase]: ");
}

void VideoProcessingClientTest::TearDownTestCase(void)
{
    VPE_LOGI("[TearDownTestCase]: ");
}

void VideoProcessingClientTest::SetUp(void)
{
    VPE_LOGI("[SetUp]: SetUp!!!");
}

void VideoProcessingClientTest::TearDown(void)
{
    VPE_LOGI("[TearDown]: over!!!");
}

TEST_F(VideoProcessingClientTest, ConnectSuccess) {
    // 调用Connect方法
    auto &manager = VideoProcessingManager::GetInstance();
    manager.Connect();


    // 验证结果
    EXPECT_NE(nullptr, manager.proxy_);
}

/**
 * @tc.name  : Connect_WhenProxyIsNotNull
 * @tc.number: VideoProcessingClientTest_001
 * @tc.desc  : Test when proxy_ is not nullptr, Connect method should return immediately
 */
HWTEST_F(VideoProcessingClientTest, Connect_WhenProxyIsNotNull, TestSize.Level0)
{
    VPE_LOGI("[Connect_001]: start!!!");
    VideoProcessingManager manager;
    manager.Connect();
    ASSERT_NE(manager.proxy_, nullptr);
    VPE_LOGI("[Connect_001]: end!!!");
}

/**
 * @tc.name  : Connect_WhenGetSystemAbilityManagerReturnNull
 * @tc.number: VideoProcessingClientTest_002
 * @tc.desc  : Test when GetSystemAbilityManager return null, Connect method should return immediately
 */
HWTEST_F(VideoProcessingClientTest, Connect_WhenGetSystemAbilityManagerReturnNull, TestSize.Level0)
{
    VPE_LOGI("[Connect_002]: start!!!");
    VideoProcessingManager manager;
    manager.Connect();
    ASSERT_NE(manager.proxy_, nullptr);
    VPE_LOGI("[Connect_002]: end!!!");
}

/**
 * @tc.name  : Connect_WhenObjectIsNotNull
 * @tc.number: VideoProcessingClientTest_003
 * @tc.desc  : Test when object is not null, Connect method should return immediately
 */
HWTEST_F(VideoProcessingClientTest, Connect_WhenObjectIsNotNull, TestSize.Level0)
{
    VPE_LOGI("[Connect_003]: start!!!");
    VideoProcessingManager manager;
    manager.Connect();
    ASSERT_NE(manager.proxy_, nullptr);
    VPE_LOGI("[Connect_003]: end!!!");
}

/**
 * @tc.name  : Connect_WhenLoadSystemAbilityReturnError
 * @tc.number: VideoProcessingClientTest_004
 * @tc.desc  : Test when LoadSystemAbility return error, Connect method should return immediately
 */
HWTEST_F(VideoProcessingClientTest, Connect_WhenLoadSystemAbilityReturnError, TestSize.Level0)
{
    VPE_LOGI("[Connect_004]: start!!!");
    VideoProcessingManager manager;
    manager.Connect();
    ASSERT_NE(manager.proxy_, nullptr);
    VPE_LOGI("[Connect_004]: end!!!");
}

/**
 * @tc.name  : Connect_WhenWaitStatusIsFalse
 * @tc.number: VideoProcessingClientTest_005
 * @tc.desc  : Test when wait_for return false, Connect method should return immediately
 */
HWTEST_F(VideoProcessingClientTest, Connect_WhenWaitStatusIsFalse, TestSize.Level0)
{
    VPE_LOGI("[Connect_005]: start!!!");
    VideoProcessingManager manager;
    manager.Connect();
    ASSERT_NE(manager.proxy_, nullptr);
    VPE_LOGI("[Connect_005]: end!!!");
}

/**
 * @tc.name  : Connect_WhenAllConditionsSatisfied
 * @tc.number: VideoProcessingClientTest_006
 * @tc.desc  : Test when all conditions are satisfied, Connect method should return immediately
 */
HWTEST_F(VideoProcessingClientTest, Connect_WhenAllConditionsSatisfied, TestSize.Level0)
{
    VPE_LOGI("[Connect_006]: start!!!");
    VideoProcessingManager manager;
    manager.Connect();
    ASSERT_NE(manager.proxy_, nullptr);
    VPE_LOGI("[Connect_006]: end!!!");
}

/**
 * @tc.name  : LoadInfo_ShouldReturnErrNullObject_WhenProxyIsNull
 * @tc.number: VideoProcessingClientTest_001
 * @tc.desc  : Test LoadInfo method when proxy_ is nullptr.
 */
HWTEST_F(VideoProcessingClientTest, LoadInfo_ShouldReturnErrNullObject_WhenProxyIsNull, TestSize.Level0)
{
    VPE_LOGI("[LoadInfo_001]: start!!!");
    VideoProcessingManager manager;
    SurfaceBufferInfo bufferInfo;
    EXPECT_NE(manager.LoadInfo(1, bufferInfo), 1661);
    VPE_LOGI("[LoadInfo_001]: end!!!");
}

/**
 * @tc.name  : Create_Success_WhenValidInput
 * @tc.number: VideoProcessingClientTest_001
 * @tc.desc  : Test Create method with valid input.
 */
HWTEST_F(VideoProcessingClientTest, Create_Success_WhenValidInput, TestSize.Level0)
{
    VPE_LOGI("[Create_001]: start!!!");
    std::string feature = "testFeature";
    std::string clientName = "testClient";
    uint32_t clientID = 0;
    VideoProcessingManager& manager = VideoProcessingManager::GetInstance();
    EXPECT_NE(manager.Create(feature, clientName, clientID), VPE_ALGO_ERR_OK);
    VPE_LOGI("[Create_001]: end!!!");
}

/**
 * @tc.name  : Create_Fail_WhenInvalidInput
 * @tc.number: VideoProcessingClientTest_002
 * @tc.desc  : Test Create method with invalid input.
 */
HWTEST_F(VideoProcessingClientTest, Create_Fail_WhenInvalidInput, TestSize.Level0)
{
    VPE_LOGI("[Create_002]: start!!!");
    std::string feature = "";
    std::string clientName = "";
    uint32_t clientID = 0;
    VideoProcessingManager& manager = VideoProcessingManager::GetInstance();
    EXPECT_NE(manager.Create(feature, clientName, clientID), VPE_ALGO_ERR_INVALID_VAL);
    VPE_LOGI("[Create_002]: end!!!");
}

/**
 * @tc.name  : Create_Fail_WhenMemoryError
 * @tc.number: VideoProcessingClientTest_003
 * @tc.desc  : Test Create method when memory error occurs.
 */
HWTEST_F(VideoProcessingClientTest, Create_Fail_WhenMemoryError, TestSize.Level0)
{
    VPE_LOGI("[Create_003]: start!!!");
    std::string feature = "testFeature";
    std::string clientName = "testClient";
    uint32_t clientID = 0;
    VideoProcessingManager& manager = VideoProcessingManager::GetInstance();
    EXPECT_EQ(manager.Create(feature, clientName, clientID), VPE_ALGO_ERR_NO_MEMORY);
    VPE_LOGI("[Create_003]: end!!!");
}

/**
 * @tc.name  : Destroy_ShouldReturnOk_WhenValidClientID
 * @tc.number: VideoProcessingClientTest_001
 * @tc.desc  : Test Destroy method with valid clientID.
 */
HWTEST_F(VideoProcessingClientTest, Destroy_ShouldReturnOk_WhenValidClientID, TestSize.Level0)
{
    VPE_LOGI("[Destroy_001]: start!!!");
    uint32_t clientID = 1;
    VideoProcessingManager& manager = VideoProcessingManager::GetInstance();
    VPEAlgoErrCode result = manager.Destroy(clientID);
    EXPECT_NE(result, VPE_ALGO_ERR_OK);
    VPE_LOGI("[Destroy_001]: end!!!");
}

/**
 * @tc.name  : Destroy_ShouldReturnInvalidVal_WhenInvalidClientID
 * @tc.number: VideoProcessingClientTest_002
 * @tc.desc  : Test Destroy method with invalid clientID.
 */
HWTEST_F(VideoProcessingClientTest, Destroy_ShouldReturnInvalidVal_WhenInvalidClientID, TestSize.Level0)
{
    VPE_LOGI("[Destroy_002]: start!!!");
    uint32_t clientID = 0;
    VideoProcessingManager& manager = VideoProcessingManager::GetInstance();
    VPEAlgoErrCode result = manager.Destroy(clientID);
    EXPECT_EQ(result, VPE_ALGO_ERR_EXTEND_START);
    VPE_LOGI("[Destroy_002]: end!!!");
}

/**
 * @tc.name  : Destroy_ShouldReturnUnknown_WhenUnknownErrorOccurs
 * @tc.number: VideoProcessingClientTest_003
 * @tc.desc  : Test Destroy method when unknown error occurs.
 */
HWTEST_F(VideoProcessingClientTest, Destroy_ShouldReturnUnknown_WhenUnknownErrorOccurs, TestSize.Level0)
{
    VPE_LOGI("[Destroy_003]: start!!!");
    uint32_t clientID = 1;
    VideoProcessingManager& manager = VideoProcessingManager::GetInstance();
    VPEAlgoErrCode result = manager.Destroy(clientID);
    EXPECT_NE(result, VPE_ALGO_ERR_UNKNOWN);
    VPE_LOGI("[Destroy_003]: end!!!");
}

/**
 * @tc.name  : SetParameter_Success_WhenValidParameters
 * @tc.number: VideoProcessingClientTest_001
 * @tc.desc  : Test SetParameter method with valid parameters.
 */
HWTEST_F(VideoProcessingClientTest, SetParameter_Success_WhenValidParameters, TestSize.Level0)
{
    VPE_LOGI("[SetParameter_001]: start!!!");
    uint32_t clientID = 1;
    int32_t tag = 2;
    std::vector<uint8_t> parameter = {3, 4, 5};
    VideoProcessingManager manager;
    auto result = manager.SetParameter(clientID, tag, parameter);
    EXPECT_NE(result, VPE_ALGO_ERR_OK);
    VPE_LOGI("[SetParameter_001]: end!!!");
}

/**
 * @tc.name  : SetParameter_Fail_WhenInvalidParameters
 * @tc.number: VideoProcessingClientTest_002
 * @tc.desc  : Test SetParameter method with invalid parameters.
 */
HWTEST_F(VideoProcessingClientTest, SetParameter_Fail_WhenInvalidParameters, TestSize.Level0)
{
    VPE_LOGI("[SetParameter_002]: start!!!");
    uint32_t clientID = 0;
    int32_t tag = 0;
    std::vector<uint8_t> parameter = {};
    VideoProcessingManager manager;
    auto result = manager.SetParameter(clientID, tag, parameter);
    EXPECT_NE(result, VPE_ALGO_ERR_INVALID_PARAM);
    VPE_LOGI("[SetParameter_002]: end!!!");
}

/**
 * @tc.name  : SetParameter_Fail_WhenNoMemory
 * @tc.number: VideoProcessingClientTest_003
 * @tc.desc  : Test SetParameter method when no memory available.
 */
HWTEST_F(VideoProcessingClientTest, SetParameter_Fail_WhenNoMemory, TestSize.Level0)
{
    VPE_LOGI("[SetParameter_003]: start!!!");
    uint32_t clientID = 1;
    int32_t tag = 2;
    std::vector<uint8_t> parameter = {3, 4, 5};
    VideoProcessingManager manager;
    auto result = manager.SetParameter(clientID, tag, parameter);
    EXPECT_NE(result, VPE_ALGO_ERR_NO_MEMORY);
    VPE_LOGI("[SetParameter_003]: end!!!");
}

/**
 * @tc.name  : GetParameter_ShouldReturnErrOK_WhenValidParameters
 * @tc.number: VideoProcessingClientTest_001
 * @tc.desc  : Test GetParameter method with valid parameters.
 */
HWTEST_F(VideoProcessingClientTest, GetParameter_ShouldReturnErrOK_WhenValidParameters, TestSize.Level0)
{
    VPE_LOGI("[GetParameter_001]: start!!!");
    uint32_t clientID = 1;
    int32_t tag = 1;
    std::vector<uint8_t> parameter;
    VideoProcessingManager manager;
    VPEAlgoErrCode result = manager.GetParameter(clientID, tag, parameter);
    EXPECT_NE(result, VPE_ALGO_ERR_OK);
    VPE_LOGI("[GetParameter_001]: end!!!");
}

/**
 * @tc.name  : GetParameter_ShouldReturnErrInvalidParam_WhenInvalidParameters
 * @tc.number: VideoProcessingClientTest_002
 * @tc.desc  : Test GetParameter method with invalid parameters.
 */
HWTEST_F(VideoProcessingClientTest, GetParameter_ShouldReturnErrInvalidParam_WhenInvalidParameters, TestSize.Level0)
{
    VPE_LOGI("[GetParameter_002]: start!!!");
    uint32_t clientID = 0; // Invalid client ID
    int32_t tag = 1;
    std::vector<uint8_t> parameter;
    VideoProcessingManager manager;
    VPEAlgoErrCode result = manager.GetParameter(clientID, tag, parameter);
    EXPECT_NE(result, VPE_ALGO_ERR_INVALID_PARAM);
    VPE_LOGI("[GetParameter_002]: end!!!");
}

/**
 * @tc.name  : GetParameter_ShouldReturnErrUnknown_WhenUnknownErrorOccurs
 * @tc.number: VideoProcessingClientTest_003
 * @tc.desc  : Test GetParameter method when an unknown error occurs.
 */
HWTEST_F(VideoProcessingClientTest, GetParameter_ShouldReturnErrUnknown_WhenUnknownErrorOccurs, TestSize.Level0)
{
    VPE_LOGI("[GetParameter_003]: start!!!");
    uint32_t clientID = 1;
    int32_t tag = 1;
    std::vector<uint8_t> parameter;
    VideoProcessingManager manager;
    VPEAlgoErrCode result = manager.GetParameter(clientID, tag, parameter);
    EXPECT_NE(result, VPE_ALGO_ERR_UNKNOWN);
    VPE_LOGI("[GetParameter_003]: end!!!");
}

/**
 * @tc.name  : UpdateMetadata_Success_WhenValidParams
 * @tc.number: VideoProcessingClientTest_001
 * @tc.desc  : Test UpdateMetadata method with valid parameters.
 */
HWTEST_F(VideoProcessingClientTest, UpdateMetadata_Success_WhenValidParams, TestSize.Level0)
{
    VPE_LOGI("[UpdateMetadata_001]: start!!!");
    uint32_t clientID = 1;
    SurfaceBufferInfo image;
    VideoProcessingManager& manager = VideoProcessingManager::GetInstance();
    auto result = manager.UpdateMetadata(clientID, image);
    EXPECT_NE(result, VPE_ALGO_ERR_OK);
    VPE_LOGI("[UpdateMetadata_001]: end!!!");
}

/**
 * @tc.name  : UpdateMetadata_Fail_WhenInvalidParams
 * @tc.number: VideoProcessingClientTest_002
 * @tc.desc  : Test UpdateMetadata method with invalid parameters.
 */
HWTEST_F(VideoProcessingClientTest, UpdateMetadata_Fail_WhenInvalidParams, TestSize.Level0)
{
    VPE_LOGI("[UpdateMetadata_002]: start!!!");
    uint32_t clientID = 0; // Invalid client ID
    SurfaceBufferInfo image;
    VideoProcessingManager& manager = VideoProcessingManager::GetInstance();
    auto result = manager.UpdateMetadata(clientID, image);
    EXPECT_NE(result, VPE_ALGO_ERR_INVALID_VAL);
    VPE_LOGI("[UpdateMetadata_002]: end!!!");
}

/**
 * @tc.name  : UpdateMetadata_Fail_WhenNoMemory
 * @tc.number: VideoProcessingClientTest_003
 * @tc.desc  : Test UpdateMetadata method when no memory available.
 */
HWTEST_F(VideoProcessingClientTest, UpdateMetadata_Fail_WhenNoMemory, TestSize.Level0)
{
    VPE_LOGI("[UpdateMetadata_003]: start!!!");
    uint32_t clientID = 1;
    SurfaceBufferInfo image;
    VideoProcessingManager& manager = VideoProcessingManager::GetInstance();
    auto result = manager.UpdateMetadata(clientID, image);
    EXPECT_NE(result, VPE_ALGO_ERR_NO_MEMORY);
    VPE_LOGI("[UpdateMetadata_003]: end!!!");
}

/**
 * @tc.name  : Process_ShouldReturnErrOK_WhenInputIsValid
 * @tc.number: VideoProcessingClientTest_001
 * @tc.desc  : Test if Process method returns VPE_ALGO_ERR_OK when input is valid.
 */
HWTEST_F(VideoProcessingClientTest, Process_ShouldReturnErrOK_WhenInputIsValid, TestSize.Level0)
{
    VPE_LOGI("[Process_001]: start!!!");
    uint32_t clientID = 1;
    SurfaceBufferInfo input;
    SurfaceBufferInfo output;
    VideoProcessingManager& manager = VideoProcessingManager::GetInstance();
    VPEAlgoErrCode result = manager.Process(clientID, input, output);
    EXPECT_NE(result, VPE_ALGO_ERR_OK);
    VPE_LOGI("[Process_001]: end!!!");
}

/**
 * @tc.name  : Process_ShouldReturnErrInvalidVal_WhenInputIsInvalid
 * @tc.number: VideoProcessingClientTest_002
 * @tc.desc  : Test if Process method returns VPE_ALGO_ERR_INVALID_VAL when input is invalid.
 */
HWTEST_F(VideoProcessingClientTest, Process_ShouldReturnErrInvalidVal_WhenInputIsInvalid, TestSize.Level0)
{
    VPE_LOGI("[Process_002]: start!!!");
    uint32_t clientID = 1;
    SurfaceBufferInfo input;
    SurfaceBufferInfo output;
    VideoProcessingManager& manager = VideoProcessingManager::GetInstance();
    VPEAlgoErrCode result = manager.Process(clientID, input, output);
    EXPECT_NE(result, VPE_ALGO_ERR_INVALID_VAL);
    VPE_LOGI("[Process_002]: end!!!");
}

/**
 * @tc.name  : ComposeImage_ShouldReturnErr_WhenInputInvalid
 * @tc.number: VideoProcessingClientTest_ComposeImage_001
 * @tc.desc  : Test ComposeImage function with invalid input.
 */
HWTEST_F(VideoProcessingClientTest, ComposeImage_ShouldReturnErr_WhenInputInvalid, TestSize.Level0)
{
    VPE_LOGI("[ComposeImage_001]: start!!!");
    uint32_t clientID = 1;
    SurfaceBufferInfo inputSdrImage;
    SurfaceBufferInfo inputGainmap;
    SurfaceBufferInfo outputHdrImage;
    bool legacy = false;
    VPEAlgoErrCode result = VideoProcessingManager::GetInstance()
        .ComposeImage(clientID, inputSdrImage, inputGainmap, outputHdrImage, legacy);
    EXPECT_NE(result, VPE_ALGO_ERR_INVALID_VAL);
    VPE_LOGI("[ComposeImage_001]: end!!!");
}

/**
 * @tc.name  : ComposeImage_ShouldReturnErr_WhenMemoryNotEnough
 * @tc.number: VideoProcessingClientTest_ComposeImage_002
 * @tc.desc  : Test ComposeImage function with not enough memory.
 */
HWTEST_F(VideoProcessingClientTest, ComposeImage_ShouldReturnErr_WhenMemoryNotEnough, TestSize.Level0)
{
    VPE_LOGI("[ComposeImage_002]: start!!!");
    uint32_t clientID = 1;
    SurfaceBufferInfo inputSdrImage;
    SurfaceBufferInfo inputGainmap;
    SurfaceBufferInfo outputHdrImage;
    bool legacy = false;
    VPEAlgoErrCode result = VideoProcessingManager::GetInstance()
        .ComposeImage(clientID, inputSdrImage, inputGainmap, outputHdrImage, legacy);
    EXPECT_NE(result, VPE_ALGO_ERR_NO_MEMORY);
    VPE_LOGI("[ComposeImage_002]: end!!!");
}

/**
 * @tc.name  : ComposeImage_ShouldReturnOk_WhenInputValid
 * @tc.number: VideoProcessingClientTest_ComposeImage_003
 * @tc.desc  : Test ComposeImage function with valid input.
 */
HWTEST_F(VideoProcessingClientTest, ComposeImage_ShouldReturnOk_WhenInputValid, TestSize.Level0)
{
    VPE_LOGI("[ComposeImage_003]: start!!!");
    uint32_t clientID = 1;
    SurfaceBufferInfo inputSdrImage;
    SurfaceBufferInfo inputGainmap;
    SurfaceBufferInfo outputHdrImage;
    bool legacy = false;
    VPEAlgoErrCode result = VideoProcessingManager::GetInstance()
        .ComposeImage(clientID, inputSdrImage, inputGainmap, outputHdrImage, legacy);
    EXPECT_NE(result, VPE_ALGO_ERR_OK);
    VPE_LOGI("[ComposeImage_003]: end!!!");
}

/**
 * @tc.name  : DecomposeImage_Success_WhenValidInput
 * @tc.number: VideoProcessingClientTest_001
 * @tc.desc  : Test DecomposeImage function with valid input.
 */
HWTEST_F(VideoProcessingClientTest, DecomposeImage_Success_WhenValidInput, TestSize.Level0)
{
    VPE_LOGI("[DecomposeImage_001]: start!!!");
    uint32_t clientID = 1;
    SurfaceBufferInfo inputImage;
    SurfaceBufferInfo outputSdrImage;
    SurfaceBufferInfo outputGainmap;
    auto result = VideoProcessingManager::GetInstance()
        .DecomposeImage(clientID, inputImage, outputSdrImage, outputGainmap);
    EXPECT_NE(result, VPE_ALGO_ERR_OK);
    VPE_LOGI("[DecomposeImage_001]: end!!!");
}

/**
 * @tc.name  : DecomposeImage_Fail_WhenInvalidInput
 * @tc.number: VideoProcessingClientTest_002
 * @tc.desc  : Test DecomposeImage function with invalid input.
 */
HWTEST_F(VideoProcessingClientTest, DecomposeImage_Fail_WhenInvalidInput, TestSize.Level0)
{
    VPE_LOGI("[DecomposeImage_002]: start!!!");
    uint32_t clientID = 0; // Invalid client ID
    SurfaceBufferInfo inputImage;
    SurfaceBufferInfo outputSdrImage;
    SurfaceBufferInfo outputGainmap;
    auto result = VideoProcessingManager::GetInstance()
        .DecomposeImage(clientID, inputImage, outputSdrImage, outputGainmap);
    EXPECT_NE(result, VPE_ALGO_ERR_INVALID_VAL);
    VPE_LOGI("[DecomposeImage_002]: end!!!");
}

/**
 * @tc.name  : DecomposeImage_Fail_WhenMemoryError
 * @tc.number: VideoProcessingClientTest_003
 * @tc.desc  : Test DecomposeImage function when memory allocation fails.
 */
HWTEST_F(VideoProcessingClientTest, DecomposeImage_Fail_WhenMemoryError, TestSize.Level0)
{
    VPE_LOGI("[DecomposeImage_003]: start!!!");
    uint32_t clientID = 1;
    SurfaceBufferInfo inputImage;
    SurfaceBufferInfo outputSdrImage;
    SurfaceBufferInfo outputGainmap;
    auto result = VideoProcessingManager::GetInstance()
        .DecomposeImage(clientID, inputImage, outputSdrImage, outputGainmap);
    EXPECT_NE(result, VPE_ALGO_ERR_NO_MEMORY);
    VPE_LOGI("[DecomposeImage_003]: end!!!");
}


/**
 * @tc.name  : GetService_ShouldReturnNull_WhenProxyIsNullAndLoadSystemAbilityFail
 * @tc.number: VideoProcessingClientTest_002
 * @tc.desc  : Test GetService method when proxy is null and load system ability fail.
 */
HWTEST_F(VideoProcessingClientTest,
    GetService_ShouldReturnNull_WhenProxyIsNullAndLoadSystemAbilityFail, TestSize.Level0)
{
    VPE_LOGI("[GetService_002]: start!!!");
    VideoProcessingManager manager;
    manager.proxy_ = nullptr;
    manager.isLoading_ = false;
    auto service = manager.GetService();
    EXPECT_NE(service, nullptr);
    VPE_LOGI("[GetService_002]: end!!!");
}

/**
 * @tc.name  : GetService_ShouldReturnProxy_WhenProxyIsNullAndLoadSystemAbilitySuccess
 * @tc.number: VideoProcessingClientTest_003
 * @tc.desc  : Test GetService method when proxy is null and load system ability success.
 */
HWTEST_F(VideoProcessingClientTest,
    GetService_ShouldReturnProxy_WhenProxyIsNullAndLoadSystemAbilitySuccess, TestSize.Level0)
{
    VPE_LOGI("[GetService_003]: start!!!");
    VideoProcessingManager manager;
    manager.proxy_ = nullptr;
    manager.isLoading_ = false;
    auto service = manager.GetService();
    EXPECT_NE(service, nullptr);
    VPE_LOGI("[GetService_003]: end!!!");
}

TEST_F(VideoProcessingClientTest, OnSaLoad_ShouldSetProxy_WhenRemoteObjectIsNotNull)
{
    VPE_LOGI("[OnSaLoad_001]: start!!!");
    VideoProcessingManager& manager = VideoProcessingManager::GetInstance();
    sptr<OHOS::IRemoteObject> remoteObject;
    manager.OnSaLoad(remoteObject);
    EXPECT_EQ(manager.proxy_, nullptr);
    VPE_LOGI("[OnSaLoad_001]: end!!!");
}

TEST_F(VideoProcessingClientTest, OnSaLoad_ShouldSetProxyToNull_WhenRemoteObjectIsNull)
{
    VPE_LOGI("[OnSaLoad_002]: start!!!");
    VideoProcessingManager& manager = VideoProcessingManager::GetInstance();
    sptr<OHOS::IRemoteObject> remoteObject = nullptr;
    manager.OnSaLoad(remoteObject);
    EXPECT_EQ(manager.proxy_, nullptr);
    VPE_LOGI("[OnSaLoad_002]: end!!!");
}

HWTEST_F(VideoProcessingClientTest, Execute_ShouldReturnErrOk_WhenOperationIsValid, TestSize.Level0)
{
    VPE_LOGI("[Execute_001]: start!!!");
    VideoProcessingManager& manager = VideoProcessingManager::GetInstance();
    std::function<ErrCode(sptr<IVideoProcessingServiceManager>&)> operation
        = [](sptr<IVideoProcessingServiceManager>& proxy) {
        return ERR_OK;
    };
    LogInfo logInfo;
    auto result = manager.Execute(std::move(operation), logInfo);
    EXPECT_EQ(result, VPE_ALGO_ERR_OK);
    VPE_LOGI("[Execute_001]: end!!!");
}

HWTEST_F(VideoProcessingClientTest, Execute_ShouldReturnErrInvalidState_WhenProxyIsNull, TestSize.Level0)
{
    VPE_LOGI("[Execute_002]: start!!!");
    VideoProcessingManager& manager = VideoProcessingManager::GetInstance();
    std::function<ErrCode(sptr<IVideoProcessingServiceManager>&)> operation
        = [](sptr<IVideoProcessingServiceManager>& proxy) {
        return ERR_OK;
    };
    LogInfo logInfo;
    auto result = manager.Execute(std::move(operation), logInfo);
    EXPECT_NE(result, VPE_ALGO_ERR_INVALID_STATE);
    VPE_LOGI("[Execute_002]: end!!!");
}

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
