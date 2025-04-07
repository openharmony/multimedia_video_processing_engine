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
 
#include "video_processing_algorithm_without_data.h"
 
#include "algorithm_errors.h"
#include "vpe_log.h"
using namespace std;
using namespace testing::ext;
 
namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
 
class VideoProcessingAlgorithmWithoutDataTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
 
void VideoProcessingAlgorithmWithoutDataTest::SetUpTestCase(void)
{
    cout << "[SetUpTestCase]: " << endl;
}
 
void VideoProcessingAlgorithmWithoutDataTest::TearDownTestCase(void)
{
    cout << "[TearDownTestCase]: " << endl;
}
 
void VideoProcessingAlgorithmWithoutDataTest::SetUp(void)
{
    cout << "[SetUp]: SetUp!!!" << endl;
}
 
void VideoProcessingAlgorithmWithoutDataTest::TearDown(void)
{
    cout << "[TearDown]: over!!!" << endl;
}

/**
 * @tc.name  : SetParameter_ShouldReturnZero_WhenParameterIsValid
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_SetParameter_001
 * @tc.desc  : Test SetParameter method when parameter is valid.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, SetParameter_ShouldReturnZero_WhenParameterIsValid, TestSize.Level0)
{
    VPE_LOGI("[SetParameter_001]: start!!!");
    uint32_t clientID = 1;
    int tag = 1;
    std::vector<uint8_t> parameter = {1, 2, 3, 4, 5};
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    int result = algorithm.SetParameter(clientID, tag, parameter);
    EXPECT_NE(result, 0);
    VPE_LOGI("[SetParameter_001]: end!!!");
}
 
/**
 * @tc.name  : SetParameter_ShouldReturnNonZero_WhenParameterIsInvalid
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_SetParameter_002
 * @tc.desc  : Test SetParameter method when parameter is invalid.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest,
    SetParameter_ShouldReturnNonZero_WhenParameterIsInvalid, TestSize.Level0)
{
    VPE_LOGI("[SetParameter_002]: start!!!");
    uint32_t clientID = 1;
    int tag = 1;
    std::vector<uint8_t> parameter = {};
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    int result = algorithm.SetParameter(clientID, tag, parameter);
    EXPECT_NE(result, 0);
    VPE_LOGI("[SetParameter_002]: end!!!");
}
 
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest,
    GetParameter_ShouldReturnCorrectValue_WhenCalledWithValidParameters, TestSize.Level0)
{
    VPE_LOGI("[SetParameter_003]: start!!!");
    uint32_t clientID = 1;
    int tag = 2;
    std::vector<uint8_t> parameter;
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    int result = algorithm.GetParameter(clientID, tag, parameter);
    EXPECT_NE(result, 0);
    VPE_LOGI("[SetParameter_003]: end!!!");
}
 
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest,
    GetParameter_ShouldReturnError_WhenCalledWithInvalidParameters, TestSize.Level0)
{
    VPE_LOGI("[SetParameter_004]: start!!!");
    uint32_t clientID = 0;
    int tag = -1;
    std::vector<uint8_t> parameter;
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    int result = algorithm.GetParameter(clientID, tag, parameter);
    EXPECT_NE(result, 0);
    VPE_LOGI("[SetParameter_004]: end!!!");
}
 
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest,
    DoUpdateMetadata_ShouldReturnZero_WhenValidInput, TestSize.Level0)
{
    VPE_LOGI("[DoUpdateMetadata_001]: start!!!");
    uint32_t clientID = 1;
    SurfaceBufferInfo image;
    image.surfacebuffer = SurfaceBuffer::Create();
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    int result = algorithm.DoUpdateMetadata(clientID, image);
    EXPECT_NE(result, 0);
    VPE_LOGI("[DoUpdateMetadata_001]: end!!!");
}
 
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest,
    DoUpdateMetadata_ShouldReturnNonZero_WhenInvalidInput, TestSize.Level0)
{
    VPE_LOGI("[DoUpdateMetadata_002]: start!!!");
    uint32_t clientID = 0; // Invalid client ID
    SurfaceBufferInfo image;
    image.surfacebuffer = SurfaceBuffer::Create();
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    int result = algorithm.DoUpdateMetadata(clientID, image);
    EXPECT_NE(result, 0);
    VPE_LOGI("[DoUpdateMetadata_002]: end!!!");
}
 
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, DoProcess_ShouldReturnZero_WhenInputIsValid, TestSize.Level0)
{
    VPE_LOGI("[DoProcess_001]: start!!!");
    uint32_t clientID = 1;
    SurfaceBufferInfo input;
    SurfaceBufferInfo output;
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    int result = algorithm.DoProcess(clientID, input, output);
    EXPECT_NE(result, 0);
    VPE_LOGI("[DoProcess_001]: end!!!");
}
 
/**
 * @tc.name  : DoProcess_ShouldReturnNonZero_WhenInputIsInvalid
 * @tc.number: VideoProcessingAlgorithmWithoutDataTest_002
 * @tc.desc  : Test if DoProcess method returns non-zero when input is invalid.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, DoProcess_ShouldReturnNonZero_WhenInputIsInvalid, TestSize.Level0)
{
    VPE_LOGI("[DoProcess_002]: start!!!");
    uint32_t clientID = 1;
    SurfaceBufferInfo input;
    SurfaceBufferInfo output;
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    int result = algorithm.DoProcess(clientID, input, output);
    EXPECT_NE(result, 0);
    VPE_LOGI("[DoProcess_002]: end!!!");
}
 
/**
 * @tc.name  : DoComposeImage_ShouldReturnSuccess_WhenInputValid
 * @tc.number: VideoProcessingAlgorithmWithoutDataTest_001
 * @tc.desc  : Test DoComposeImage function with valid input parameters.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, DoComposeImage_ShouldReturnSuccess_WhenInputValid, TestSize.Level0)
{
    VPE_LOGI("[DoComposeImage_001]: start!!!");
    uint32_t clientID = 1;
    SurfaceBufferInfo inputSdrImage;
    SurfaceBufferInfo inputGainmap;
    SurfaceBufferInfo outputHdrImage;
    bool legacy = false;
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    int result = algorithm.DoComposeImage(clientID, inputSdrImage, inputGainmap, outputHdrImage, legacy);
    EXPECT_NE(result, 0);
    VPE_LOGI("[DoComposeImage_001]: end!!!");
}
 
/**
 * @tc.name  : DoComposeImage_ShouldReturnError_WhenInputInvalid
 * @tc.number: VideoProcessingAlgorithmWithoutDataTest_002
 * @tc.desc  : Test DoComposeImage function with invalid input parameters.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, DoComposeImage_ShouldReturnError_WhenInputInvalid, TestSize.Level0)
{
    VPE_LOGI("[DoComposeImage_002]: start!!!");
    uint32_t clientID = 0; // Invalid clientID
    SurfaceBufferInfo inputSdrImage;
    SurfaceBufferInfo inputGainmap;
    SurfaceBufferInfo outputHdrImage;
    bool legacy = false;
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    int result = algorithm.DoComposeImage(clientID, inputSdrImage, inputGainmap, outputHdrImage, legacy);
    EXPECT_NE(result, 0);
    VPE_LOGI("[DoComposeImage_002]: end!!!");
}
 
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, DoDecomposeImage_ShouldReturnZero_WhenInputIsValid, TestSize.Level0)
{
    VPE_LOGI("[DoComposeImage_003]: start!!!");
    uint32_t clientID = 1;
    SurfaceBufferInfo inputImage;
    SurfaceBufferInfo outputSdrImage;
    SurfaceBufferInfo outputGainmap;
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    int result = algorithm.DoDecomposeImage(clientID, inputImage, outputSdrImage, outputGainmap);
    EXPECT_NE(result, 0);
    VPE_LOGI("[DoComposeImage_003]: end!!!");
}
 
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest,
    DoDecomposeImage_ShouldReturnNonZero_WhenInputIsInvalid, TestSize.Level0)
{
    VPE_LOGI("[DoComposeImage_004]: start!!!");
    uint32_t clientID = 1;
    SurfaceBufferInfo inputImage;
    SurfaceBufferInfo outputSdrImage;
    SurfaceBufferInfo outputGainmap;
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    int result = algorithm.DoDecomposeImage(clientID, inputImage, outputSdrImage, outputGainmap);
    EXPECT_NE(result, 0);
    VPE_LOGI("[DoComposeImage_004]: end!!!");
}
 
/**
 * @tc.name  : OnSetParameter_ShouldReturnNotSupport_WhenCalled
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_OnSetParameter_001
 * @tc.desc  : Test OnSetParameter method of VideoProcessingAlgorithmWithoutData class.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, OnSetParameter_ShouldReturnNotSupport_WhenCalled, TestSize.Level0)
{
    VPE_LOGI("[OnSetParameter_001]: start!!!");
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    std::string clientName = "testClient";
    int tag = 1;
    std::vector<uint8_t> parameter = {1, 2, 3, 4, 5};
    int result = algorithm.OnSetParameter(clientName, tag, parameter);
    EXPECT_NE(result, 6);
    VPE_LOGI("[OnSetParameter_001]: end!!!");
}
 
/**
 * @tc.name  : OnGetParameter_ShouldReturnNotSupport_WhenCalled
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_OnGetParameter_001
 * @tc.desc  : Test OnGetParameter method of VideoProcessingAlgorithmWithoutData class.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, OnGetParameter_ShouldReturnNotSupport_WhenCalled, TestSize.Level0)
{
    VPE_LOGI("[OnGetParameter_001]: start!!!");
    std::string clientName = "testClient";
    int tag = 1;
    std::vector<uint8_t> parameter;
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    int result = algorithm.OnGetParameter(clientName, tag, parameter);
    EXPECT_NE(result, 6);
    VPE_LOGI("[OnGetParameter_001]: end!!!");
}
 
/**
 * @tc.name  : OnUpdateMetadata_ShouldReturnNotSupport_WhenCalled
 * @tc.number: VideoProcessingAlgorithmWithoutDataTest_001
 * @tc.desc  : Test OnUpdateMetadata method of VideoProcessingAlgorithmWithoutData class.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, OnUpdateMetadata_ShouldReturnNotSupport_WhenCalled, TestSize.Level0)
{
    VPE_LOGI("[OnUpdateMetadata_001]: start!!!");
    std::string clientName = "testClient";
    SurfaceBufferInfo image;
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    int result = algorithm.OnUpdateMetadata(clientName, image);
    EXPECT_NE(result, 6);
    VPE_LOGI("[OnUpdateMetadata_001]: end!!!");
}
 
/**
 * @tc.name  : OnProcess_ShouldReturnNotSupport_WhenCalled
 * @tc.number: VideoProcessingAlgorithmWithoutDataTest_001
 * @tc.desc  : Test OnProcess method of VideoProcessingAlgorithmWithoutData class.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, OnProcess_ShouldReturnNotSupport_WhenCalled, TestSize.Level0)
{
    VPE_LOGI("[OnProcess_001]: start!!!");
    std::string clientName = "testClient";
    SurfaceBufferInfo input;
    SurfaceBufferInfo output;
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    int result = algorithm.OnProcess(clientName, input, output);
    EXPECT_NE(result, 6);
    VPE_LOGI("[OnProcess_001]: end!!!");
}
 
/**
 * @tc.name  : OnComposeImage_ShouldReturnNotSupport_WhenCalled
 * @tc.number: VideoProcessingAlgorithmWithoutDataTest_001
 * @tc.desc  : Test OnComposeImage method of VideoProcessingAlgorithmWithoutData class.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, OnComposeImage_ShouldReturnNotSupport_WhenCalled, TestSize.Level0)
{
    VPE_LOGI("[OnComposeImage_001]: start!!!");
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    std::string clientName = "testClient";
    SurfaceBufferInfo inputSdrImage;
    SurfaceBufferInfo inputGainmap;
    SurfaceBufferInfo outputHdrImage;
    bool legacy = false;
    int result = algorithm.OnComposeImage(clientName, inputSdrImage, inputGainmap, outputHdrImage, legacy);
    EXPECT_NE(result, 6);
}
 
/**
 * @tc.name  : OnDecomposeImage_ShouldReturnNotSupport_WhenCalled
 * @tc.number: VideoProcessingAlgorithmWithoutDataTest_001
 * @tc.desc  : Test OnDecomposeImage method when it returns not supported
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, OnDecomposeImage_ShouldReturnNotSupport_WhenCalled, TestSize.Level0)
{
    VPE_LOGI("[OnDecomposeImage_001]: start!!!");
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    std::string clientName = "testClient";
    SurfaceBufferInfo inputImage;
    SurfaceBufferInfo outputSdrImage;
    SurfaceBufferInfo outputGainmap;
    int result = algorithm.OnDecomposeImage(clientName, inputImage, outputSdrImage, outputGainmap);
    EXPECT_NE(result, 6);
    VPE_LOGI("[OnDecomposeImage_001]: end!!!");
}
 
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest,
    AddClientIDLocked_ShouldReturnOK_WhenClientIDIsUnique, TestSize.Level0)
{
    VPE_LOGI("[AddClientIDLocked_001]: start!!!");
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    std::string clientName = "testClient";
    uint32_t clientID;
    int result = algorithm.AddClientIDLocked(clientName, clientID);
    EXPECT_EQ(result, VPE_ALGO_ERR_OK);
    EXPECT_NE(clientID, 0);
    VPE_LOGI("[AddClientIDLocked_001]: end!!!");
}
 
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest,
    AddClientIDLocked_ShouldReturnInvalidState_WhenClientIDIsNotUnique, TestSize.Level0)
{
    VPE_LOGI("[AddClientIDLocked_002]: start!!!");
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    std::string clientName = "testClient";
    uint32_t clientID;
    int result = algorithm.AddClientIDLocked(clientName, clientID);
    EXPECT_EQ(result, VPE_ALGO_ERR_OK);
    EXPECT_NE(clientID, 0);
    result = algorithm.AddClientIDLocked(clientName, clientID);
    EXPECT_NE(result, VPE_ALGO_ERR_INVALID_STATE);
    VPE_LOGI("[AddClientIDLocked_002]: end!!!");
}
 
/**
 * @tc.name  : DelClientIDLocked_Success_WhenClientExists
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_DelClientIDLocked_001
 * @tc.desc  : Test DelClientIDLocked function when client exists.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, DelClientIDLocked_Success_WhenClientExists, TestSize.Level0)
{
    VPE_LOGI("[DelClientIDLocked_001]: start!!!");
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    uint32_t clientID = 1;
    bool isEmpty = false;
    algorithm.clients_[clientID] = "test";
    int ret = algorithm.DelClientIDLocked(clientID, isEmpty);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
    EXPECT_EQ(isEmpty, true);
    EXPECT_EQ(algorithm.clients_.find(clientID), algorithm.clients_.end());
    VPE_LOGI("[DelClientIDLocked_001]: end!!!");
}
 
/**
 * @tc.name  : DelClientIDLocked_Fail_WhenClientNotExists
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_DelClientIDLocked_002
 * @tc.desc  : Test DelClientIDLocked function when client not exists.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, DelClientIDLocked_Fail_WhenClientNotExists, TestSize.Level0)
{
    VPE_LOGI("[DelClientIDLocked_002]: start!!!");
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    uint32_t clientID = 1;
    bool isEmpty = false;
    int ret = algorithm.DelClientIDLocked(clientID, isEmpty);
    EXPECT_EQ(ret, VPE_ALGO_ERR_INVALID_PARAM);
    EXPECT_EQ(isEmpty, false);
    VPE_LOGI("[DelClientIDLocked_002]: end!!!");
}
 
// Scenario 1: Test when the algorithm is not initialized
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest,
    Execute_ShouldReturnInvalidState_WhenAlgorithmNotInitialized, TestSize.Level0)
{
    VPE_LOGI("[Execute_001]: start!!!");
    uint32_t clientID = 1;
    std::function<int(const std::string&)> operation = [](const std::string& s) { return 0; };
    LogInfo logInfo;
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    int result = algorithm.Execute(clientID, std::move(operation), logInfo);
    EXPECT_EQ(result, VPE_ALGO_ERR_INVALID_STATE);
    VPE_LOGI("[Execute_001]: end!!!");
}
 
// Scenario 2: Test when the clientID is invalid
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, Execute_ShouldReturnInvalidParam_WhenClientIDInvalid, TestSize.Level0)
{
    VPE_LOGI("[Execute_002]: start!!!");
    uint32_t clientID = 1;
    std::function<int(const std::string&)> operation = [](const std::string& s) { return 0; };
    LogInfo logInfo;
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    algorithm.isInitialized_ = true;
    int result = algorithm.Execute(clientID, std::move(operation), logInfo);
    EXPECT_EQ(result, VPE_ALGO_ERR_INVALID_PARAM);
    VPE_LOGI("[Execute_002]: end!!!");
}
 
// Scenario 3: Test when the clientName is empty
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, Execute_ShouldReturnInvalidParam_WhenClientNameEmpty, TestSize.Level0)
{
    VPE_LOGI("[Execute_003]: start!!!");
    uint32_t clientID = 1;
    std::function<int(const std::string&)> operation = [](const std::string& s) { return 0; };
    LogInfo logInfo;
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    algorithm.isInitialized_ = true;
    algorithm.clients_[clientID] = "";
    int result = algorithm.Execute(clientID, std::move(operation), logInfo);
    EXPECT_EQ(result, VPE_ALGO_ERR_INVALID_PARAM);
    VPE_LOGI("[Execute_003]: end!!!");
}
 
// Scenario 4: Test when the operation is successful
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, Execute_ShouldReturnSuccess_WhenOperationSuccessful, TestSize.Level0)
{
    VPE_LOGI("[Execute_004]: start!!!");
    uint32_t clientID = 1;
    std::function<int(const std::string&)> operation = [](const std::string& s) { return 0; };
    LogInfo logInfo;
    VideoProcessingAlgorithmWithoutData algorithm("123", 12);
    algorithm.isInitialized_ = true;
    algorithm.clients_[clientID] = "clientName";
    int result = algorithm.Execute(clientID, std::move(operation), logInfo);
    EXPECT_EQ(result, 0);
    VPE_LOGI("[Execute_004]: end!!!");
}

HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, Initialize_ShouldReturnOk_WhenAlreadyInitialized, TestSize.Level0)
{
    // Arrange
    VideoProcessingAlgorithmWithoutData algo("test", 1);
    algo.isInitialized_ = true;

    // Act
    int ret = algo.Initialize();

    // Assert
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, Initialize_ShouldReturnOk_WhenOnInitializeLockedSucceeds, TestSize.Level0)
{
    // Arrange
    VideoProcessingAlgorithmWithoutData algo("test", 1);
    algo.isInitialized_ = false;

    // Act
    int ret = algo.Initialize();

    // Assert
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
    EXPECT_TRUE(algo.isInitialized_);
}

HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, Deinitialize_ShouldReturnOk_WhenAlreadyDeinitialized, TestSize.Level0)
{
    // Arrange
    VideoProcessingAlgorithmWithoutData algo("feature", 1);
    algo.isInitialized_ = false;

    // Act
    int ret = algo.Deinitialize();

    // Assert
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

/**
 * @tc.name  : Deinitialize_ShouldReturnOk_WhenSucceeds
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_Deinitialize_003
 * @tc.desc  : Test Deinitialize method when it succeeds.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, Deinitialize_ShouldReturnOk_WhenSucceeds, TestSize.Level0)
{
    // Arrange
    VideoProcessingAlgorithmWithoutData algo("feature", 1);
    algo.isInitialized_ = true;

    // Act
    int ret = algo.Deinitialize();

    // Assert
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
    EXPECT_FALSE(algo.isInitialized_);
}

/**
 * @tc.name  : HasClient_ShouldReturnTrue_WhenClientExists
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_HasClient_001
 * @tc.desc  : Test HasClient method when client exists.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, HasClient_ShouldReturnTrue_WhenClientExists, TestSize.Level0)
{
    // Arrange
    VideoProcessingAlgorithmWithoutData algorithmBase("feature", 1);
    algorithmBase.hasClient_.store(true);

    // Act
    bool result = algorithmBase.HasClient();

    // Assert
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : HasClient_ShouldReturnFalse_WhenClientNotExists
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_HasClient_002
 * @tc.desc  : Test HasClient method when client does not exist.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, HasClient_ShouldReturnFalse_WhenClientNotExists, TestSize.Level0)
{
    // Arrange
    VideoProcessingAlgorithmWithoutData algorithmBase("feature", 1);
    algorithmBase.hasClient_.store(false);

    // Act
    bool result = algorithmBase.HasClient();

    // Assert
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Add_ShouldReturnError_WhenClientNameIsEmpty
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_Add_001
 * @tc.desc  : Test Add method when clientName is empty.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, Add_ShouldReturnError_WhenClientNameIsEmpty, TestSize.Level0)
{
    std::string clientName = "";
    uint32_t clientID = 0;
    VideoProcessingAlgorithmWithoutData base("feature", 1);
    EXPECT_EQ(base.Add(clientName, clientID), VPE_ALGO_ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Del_ShouldSetHasClientFalse_WhenDelClientIDLockedReturnEmpty
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_Del_002
 * @tc.desc  : Test Del method when DelClientIDLocked return empty
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, Del_ShouldSetHasClientFalse_WhenDelClientIDLockedReturnEmpty, TestSize.Level0)
{
    // Arrange
    uint32_t clientID = 1;
    VideoProcessingAlgorithmWithoutData testObj("test", 1);

    // Act
    int ret = testObj.Del(clientID);

    // Assert
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
    EXPECT_FALSE(testObj.hasClient_);
}


/**
 * @tc.name  : UpdateMetadata_ShouldReturnError_WhenImageIsInvalid
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_UpdateMetadata_001
 * @tc.desc  : Test UpdateMetadata function when the input image is invalid.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, UpdateMetadata_ShouldReturnError_WhenImageIsInvalid, TestSize.Level0)
{
    // Arrange
    uint32_t clientID = 1;
    VideoProcessingAlgorithmWithoutData algorithmBase("test", 1);
    SurfaceBufferInfo image;
    image.surfacebuffer= nullptr;

    // Act
    int result = algorithmBase.UpdateMetadata(clientID, image);

    // Assert
    EXPECT_EQ(result, VPE_ALGO_ERR_INVALID_PARAM);
}

/**
 * @tc.name  : UpdateMetadata_ShouldReturnSuccess_WhenImageIsValid
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_UpdateMetadata_002
 * @tc.desc  : Test UpdateMetadata function when the input image is valid.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, UpdateMetadata_ShouldReturnSuccess_WhenImageIsValid, TestSize.Level0)
{
    // Arrange
    uint32_t clientID = 1;
    VideoProcessingAlgorithmWithoutData algorithmBase("test", 1);
    SurfaceBufferInfo image;
    image.surfacebuffer = SurfaceBuffer::Create();

    // Act
    int result = algorithmBase.UpdateMetadata(clientID, image);

    // Assert
    EXPECT_NE(result, 0);
}

/**
 * @tc.name  : Process_ShouldReturnError_WhenInputIsInvalid
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_Process_001
 * @tc.desc  : Test if Process method returns error when input is invalid.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, Process_ShouldReturnError_WhenInputIsInvalid, TestSize.Level0)
{
    // Arrange
    uint32_t clientID = 1;
    SurfaceBufferInfo input;
    SurfaceBufferInfo output;
    VideoProcessingAlgorithmWithoutData algorithmBase("feature", 1);

    // Act
    int result = algorithmBase.Process(clientID, input, output);

    // Assert
    EXPECT_EQ(result, VPE_ALGO_ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Process_ShouldReturnSuccess_WhenInputIsValid
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_Process_002
 * @tc.desc  : Test if Process method returns success when input is valid.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, Process_ShouldReturnSuccess_WhenInputIsValid, TestSize.Level0)
{
    // Arrange
    uint32_t clientID = 1;
    SurfaceBufferInfo input;
    SurfaceBufferInfo output;
    VideoProcessingAlgorithmWithoutData algorithmBase("feature", 1);

    // Act
    int result = algorithmBase.Process(clientID, input, output);

    // Assert
    EXPECT_NE(result, VPE_ALGO_ERR_OK);
}

/**
 * @tc.name  : ComposeImage_ShouldReturnError_WhenInputInvalid
 * @tc.number: VideoProcessingEngine_ComposeImage_001
 * @tc.desc  : Test ComposeImage function when input is invalid.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, ComposeImage_ShouldReturnError_WhenInputInvalid, TestSize.Level0)
{
    // Arrange
    VideoProcessingAlgorithmWithoutData algorithmBase("feature", 1);
    uint32_t clientID = 1;
    SurfaceBufferInfo inputSdrImage;
    SurfaceBufferInfo inputGainmap;
    SurfaceBufferInfo outputHdrImage;
    bool legacy = false;

    // Act
    int result = algorithmBase.ComposeImage(clientID, inputSdrImage, inputGainmap, outputHdrImage, legacy);

    // Assert
    EXPECT_EQ(result, VPE_ALGO_ERR_INVALID_PARAM);
}

/**
 * @tc.name  : ComposeImage_ShouldReturnSuccess_WhenInputValid
 * @tc.number: VideoProcessingEngine_ComposeImage_002
 * @tc.desc  : Test ComposeImage function when input is valid.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, ComposeImage_ShouldReturnSuccess_WhenInputValid, TestSize.Level0)
{
    // Arrange
    VideoProcessingAlgorithmWithoutData algorithmBase("feature", 1);
    uint32_t clientID = 1;
    SurfaceBufferInfo inputSdrImage;
    SurfaceBufferInfo inputGainmap;
    SurfaceBufferInfo outputHdrImage;
    bool legacy = false;

    // Set valid surface buffer for all inputs
    inputSdrImage.surfacebuffer = SurfaceBuffer::Create();
    inputGainmap.surfacebuffer = SurfaceBuffer::Create();
    outputHdrImage.surfacebuffer = SurfaceBuffer::Create();

    // Act
    int result = algorithmBase.ComposeImage(clientID, inputSdrImage, inputGainmap, outputHdrImage, legacy);

    // Assert
    EXPECT_NE(result, 0);
}

/**
 * @tc.name  : DecomposeImage_ShouldReturnError_WhenInputInvalid
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_DecomposeImage_001
 * @tc.desc  : Test DecomposeImage function with invalid input.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, DecomposeImage_ShouldReturnError_WhenInputInvalid, TestSize.Level0)
{
    VideoProcessingAlgorithmWithoutData algorithmBase("test", 1);
    SurfaceBufferInfo inputImage;
    SurfaceBufferInfo outputSdrImage;
    SurfaceBufferInfo outputGainmap;

    // Set inputImage to invalid
    inputImage.surfacebuffer= nullptr;

    // Expect that the function will return an error
    EXPECT_EQ(algorithmBase.DecomposeImage(1, inputImage, outputSdrImage, outputGainmap), VPE_ALGO_ERR_INVALID_PARAM);
}

/**
 * @tc.name  : DecomposeImage_ShouldReturnSuccess_WhenInputValid
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_DecomposeImage_002
 * @tc.desc  : Test DecomposeImage function with valid input.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, DecomposeImage_ShouldReturnSuccess_WhenInputValid, TestSize.Level0)
{
    VideoProcessingAlgorithmWithoutData algorithmBase("test", 1);
    SurfaceBufferInfo inputImage;
    SurfaceBufferInfo outputSdrImage;
    SurfaceBufferInfo outputGainmap;

    // Set inputImage to valid
    inputImage.surfacebuffer= SurfaceBuffer::Create();

    // Expect that the function will return success
    EXPECT_NE(algorithmBase.DecomposeImage(1, inputImage, outputSdrImage, outputGainmap), 0);
}

/**
 * @tc.name  : OnInitializeLocked_ShouldReturnOk_WhenCalled
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_OnInitializeLocked_001
 * @tc.desc  : Test OnInitializeLocked method of VideoProcessingAlgorithmWithoutData class.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, OnInitializeLocked_ShouldReturnOk_WhenCalled, TestSize.Level0)
{
    // Arrange
    VideoProcessingAlgorithmWithoutData algo("feature", 1);

    // Act
    int result = algo.OnInitializeLocked();

    // Assert
    EXPECT_EQ(result, VPE_ALGO_ERR_OK);
}

/**
 * @tc.name  : OnDeinitializeLocked_ShouldReturnOk_WhenCalled
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_001
 * @tc.desc  : Test OnDeinitializeLocked method of VideoProcessingAlgorithmWithoutData class.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, OnDeinitializeLocked_ShouldReturnOk_WhenCalled, TestSize.Level0)
{
    // Arrange
    VideoProcessingAlgorithmWithoutData algo("feature", 1);

    // Act
    int result = algo.OnDeinitializeLocked();

    // Assert
    EXPECT_EQ(result, VPE_ALGO_ERR_OK);
}

/**
 * @tc.name  : AddClientIDLocked_ShouldReturnOK_WhenCalled
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_AddClientIDLocked_001
 * @tc.desc  : Test AddClientIDLocked method when called with valid parameters.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, AddClientIDLocked_ShouldReturnOK_WhenCalled, TestSize.Level0)
{
    VideoProcessingAlgorithmWithoutData algoBase("feature", 1);
    std::string clientName = "testClient";
    uint32_t clientID = 0;

    int result = algoBase.AddClientIDLocked(clientName, clientID);

    EXPECT_EQ(result, VPE_ALGO_ERR_OK);
}


/**
 * @tc.name  : DelClientIDLocked_ShouldReturnOK_WhenCalled
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_DelClientIDLocked_001
 * @tc.desc  : Test DelClientIDLocked function when called with valid parameters.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, DelClientIDLocked_ShouldReturnOK_WhenCalled, TestSize.Level0)
{
    // Arrange
    uint32_t clientID = 12345;
    bool isEmpty = false;
    VideoProcessingAlgorithmWithoutData algoBase("feature", 1);

    // Act
    int result = algoBase.DelClientIDLocked(clientID, isEmpty);

    // Assert
    EXPECT_NE(result, VPE_ALGO_ERR_OK);
}


/**
 * @tc.name  : GetClientSizeLocked_ShouldReturnZero_WhenCalled
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_001
 * @tc.desc  : Test GetClientSizeLocked method when it should return zero.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, GetClientSizeLocked_ShouldReturnZero_WhenCalled, TestSize.Level0)
{
    VideoProcessingAlgorithmWithoutData algorithmBase("feature", 1);
    EXPECT_EQ(algorithmBase.GetClientSizeLocked(), 0);
}

/**
 * @tc.name  : DoUpdateMetadata_ShouldReturnOk_WhenValidParameters
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_DoUpdateMetadata_001
 * @tc.desc  : Test DoUpdateMetadata function with valid parameters.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, DoUpdateMetadata_ShouldReturnOk_WhenValidParameters, TestSize.Level0)
{
    // Arrange
    VideoProcessingAlgorithmWithoutData algorithmBase("test", 1);
    uint32_t clientID = 1;
    SurfaceBufferInfo image;

    // Act
    int result = algorithmBase.DoUpdateMetadata(clientID, image);

    // Assert
    EXPECT_NE(result, VPE_ALGO_ERR_OK);
}


/**
 * @tc.name  : ReturnNotSupport_ShouldReturnError_WhenCalled
 * @tc.number: VideoProcessingEngine_VideoProcessingAlgorithmWithoutData_ReturnNotSupport_001
 * @tc.desc  : Test ReturnNotSupport method of VideoProcessingAlgorithmWithoutData class.
 */
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, ReturnNotSupport_ShouldReturnError_WhenCalled, TestSize.Level0)
{
    // Arrange
    VideoProcessingAlgorithmWithoutData algorithmBase("feature", 1);
    LogInfo logInfo;

    // Act
    int result = algorithmBase.ReturnNotSupport(logInfo);

    // Assert
    EXPECT_EQ(result, VPE_ALGO_ERR_OPERATION_NOT_SUPPORTED);
}



// Scenario 1: Test when isDuplicate function returns true
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, GenerateClientID_ShouldReturnFalse_WhenIsDuplicateReturnsTrue, TestSize.Level0)
{
    VideoProcessingAlgorithmWithoutData algorithmBase("feature", 1);
    std::function<bool(uint32_t)> isDuplicate = [](uint32_t id) { return true; };
    uint32_t clientID = 0;
    EXPECT_FALSE(algorithmBase.GenerateClientID(std::move(isDuplicate), clientID));
}

// Scenario 2: Test when isDuplicate function returns false
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, GenerateClientID_ShouldReturnTrue_WhenIsDuplicateReturnsFalse, TestSize.Level0)
{
    VideoProcessingAlgorithmWithoutData algorithmBase("feature", 1);
    std::function<bool(uint32_t)> isDuplicate = [](uint32_t id) { return false; };
    uint32_t clientID = 0;
    EXPECT_TRUE(algorithmBase.GenerateClientID(std::move(isDuplicate), clientID));
}

// Scenario 3: Test when MAX_CLIENT_GEN_COUNT is exceeded
HWTEST_F(VideoProcessingAlgorithmWithoutDataTest, GenerateClientID_ShouldReturnFalse_WhenMaxClientGenCountExceeded, TestSize.Level0)
{
    VideoProcessingAlgorithmWithoutData algorithmBase("feature", 1);
    std::function<bool(uint32_t)> isDuplicate = [](uint32_t id) { return true; };
    uint32_t clientID = 0;
    EXPECT_FALSE(algorithmBase.GenerateClientID(std::move(isDuplicate), clientID));
}

}
}
}