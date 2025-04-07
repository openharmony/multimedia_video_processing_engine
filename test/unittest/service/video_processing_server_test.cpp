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

#include <fstream>
#include <iostream>
#include <sstream>
#include <iservice_registry.h>
#include <system_ability_definition.h>
#include "algorithm_errors.h"
#include "surface_buffer.h"
#include "vpe_model_path.h"
#include "vpe_sa_constants.h"

#include "video_processing_server_test.h"
#include "video_processing_server.h"
// #include "system_ability_ondemand_reason.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
const std::string UNLOAD_HANLDER = "unload_vpe_sa_handler";
const std::string UNLOAD_TASK_ID = "unload_vpe_sa";
class VideoProcessingServerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void VideoProcessingServerTest::SetUpTestCase(void)
{
    cout << "[SetUpTestCase]: " << endl;
}

void VideoProcessingServerTest::TearDownTestCase(void)
{
    cout << "[TearDownTestCase]: " << endl;
}

void VideoProcessingServerTest::SetUp(void)
{
    cout << "[SetUp]: SetUp!!!" << endl;
}

void VideoProcessingServerTest::TearDown(void)
{
    cout << "[TearDown]: over!!!" << endl;
}

/**
 * @tc.name  : LoadInfo_Test_01
 * @tc.number: VideoProcessingServerTest_01
 * @tc.desc  : Test LoadInfo function when key is invalid.
 */
HWTEST_F(VideoProcessingServerTest, LoadInfo_Test_01, TestSize.Level0)
{
    VPE_LOGI("LoadInfo_Test_01: begin!");
    VideoProcessingServer videoProcessingServer(1, true);
    SurfaceBufferInfo bufferInfo;
    ErrCode result = videoProcessingServer.LoadInfo(-1, bufferInfo);
    EXPECT_EQ(result, ERR_INVALID_DATA);
    VPE_LOGI("LoadInfo_Test_01: end!");
}

/**
 * @tc.name  : LoadInfo_Test_02
 * @tc.number: VideoProcessingServerTest_02
 * @tc.desc  : Test LoadInfo function when key is valid but file is not open.
 */
HWTEST_F(VideoProcessingServerTest, LoadInfo_Test_02, TestSize.Level0)
{
    VPE_LOGI("LoadInfo_Test_02: begin!");
    VideoProcessingServer videoProcessingServer(1, true);
    SurfaceBufferInfo bufferInfo;
    ErrCode result = videoProcessingServer.LoadInfo(0, bufferInfo);
    EXPECT_NE(result, ERR_NULL_OBJECT);
    VPE_LOGI("LoadInfo_Test_02: end!");
}

/**
 * @tc.name  : LoadInfo_Test_03
 * @tc.number: VideoProcessingServerTest_03
 * @tc.desc  : Test LoadInfo function when file length is too short or too long.
 */
HWTEST_F(VideoProcessingServerTest, LoadInfo_Test_03, TestSize.Level0)
{
    VPE_LOGI("LoadInfo_Test_03: begin!");
    VideoProcessingServer videoProcessingServer(1, true);
    SurfaceBufferInfo bufferInfo;
    ErrCode result = videoProcessingServer.LoadInfo(1, bufferInfo);
    EXPECT_NE(result, ERR_INVALID_DATA);
    VPE_LOGI("LoadInfo_Test_03: end!");
}

/**
 * @tc.name  : LoadInfo_Test_04
 * @tc.number: VideoProcessingServerTest_04
 * @tc.desc  : Test LoadInfo function when Alloc surface buffer failed.
 */
HWTEST_F(VideoProcessingServerTest, LoadInfo_Test_04, TestSize.Level0)
{
    VPE_LOGI("LoadInfo_Test_04: begin!");
    VideoProcessingServer videoProcessingServer(1, true);
    SurfaceBufferInfo bufferInfo;
    ErrCode result = videoProcessingServer.LoadInfo(2, bufferInfo);
    EXPECT_EQ(result, 0);
    VPE_LOGI("LoadInfo_Test_04: end!");
}

/**
 * @tc.name  : LoadInfo_Test_05
 * @tc.number: VideoProcessingServerTest_05
 * @tc.desc  : Test LoadInfo function when everything is fine.
 */
HWTEST_F(VideoProcessingServerTest, LoadInfo_Test_05, TestSize.Level0)
{
    VPE_LOGI("LoadInfo_Test_05: begin!");
    VideoProcessingServer videoProcessingServer(1, true);
    SurfaceBufferInfo bufferInfo;
    ErrCode result = videoProcessingServer.LoadInfo(3, bufferInfo);
    EXPECT_EQ(result, ERR_NONE);
    VPE_LOGI("LoadInfo_Test_05: end!");
}


/**
 * @tc.name  : Create_Success_WhenValidFeatureAndClientName
 * @tc.number: VideoProcessingServerTest_001
 * @tc.desc  : Test Create method when valid feature and clientName are provided.
 */
HWTEST_F(VideoProcessingServerTest, Create_Success_WhenValidFeatureAndClientName, TestSize.Level0)
{
    VPE_LOGI("Create_Success_WhenValidFeatureAndClientName: begin!");
    VideoProcessingServer server(1, true);
    std::string feature = "testFeature";
    std::string clientName = "testClient";
    int32_t clientID = 0;
    EXPECT_NE(server.Create(feature, clientName, clientID), VPE_ALGO_ERR_OK);
    VPE_LOGI("Create_Success_WhenValidFeatureAndClientName: end!");
}

/**
 * @tc.name  : Create_Fail_WhenNoMemory
 * @tc.number: VideoProcessingServerTest_002
 * @tc.desc  : Test Create method when no memory is available.
 */
HWTEST_F(VideoProcessingServerTest, Create_Fail_WhenNoMemory, TestSize.Level0)
{
    VPE_LOGI("Create_Fail_WhenNoMemory: begin!");
    VideoProcessingServer server(1, true);
    std::string feature = "testFeature";
    std::string clientName = "testClient";
    int32_t clientID = 0;
    EXPECT_EQ(server.Create(feature, clientName, clientID), VPE_ALGO_ERR_NO_MEMORY);
    VPE_LOGI("Create_Fail_WhenNoMemory: end!");
}

/**
 * @tc.name  : Create_Fail_WhenInitializeFails
 * @tc.number: VideoProcessingServerTest_003
 * @tc.desc  : Test Create method when initialization fails.
 */
HWTEST_F(VideoProcessingServerTest, Create_Fail_WhenInitializeFails, TestSize.Level0)
{
    VPE_LOGI("Create_Fail_WhenInitializeFails: begin!");
    VideoProcessingServer server(1, true);
    std::string feature = "testFeature";
    std::string clientName = "testClient";
    int32_t clientID = 0;
    EXPECT_NE(server.Create(feature, clientName, clientID), ERR_INVALID_STATE);
    VPE_LOGI("Create_Fail_WhenInitializeFails: end!");
}

/**
 * @tc.name  : Create_Fail_WhenAddClientFails
 * @tc.number: VideoProcessingServerTest_004
 * @tc.desc  : Test Create method when adding client fails.
 */
HWTEST_F(VideoProcessingServerTest, Create_Fail_WhenAddClientFails, TestSize.Level0)
{
    VPE_LOGI("Create_Fail_WhenAddClientFails: begin!");
    VideoProcessingServer server(1, true);
    std::string feature = "testFeature";
    std::string clientName = "testClient";
    int32_t clientID = 0;
    EXPECT_NE(server.Create(feature, clientName, clientID), ERR_INVALID_DATA);
    VPE_LOGI("Create_Fail_WhenAddClientFails: end!");
}

HWTEST_F(VideoProcessingServerTest, Destroy_ShouldReturnErrOk_WhenDestroyLockedReturnsErrOk, TestSize.Level0)
{
    VPE_LOGI("Destroy_ShouldReturnErrOk_WhenDestroyLockedReturnsErrOk: begin!");
    // Arrange
    VideoProcessingServer videoProcessingServer(1, true);
    // Act
    auto result = videoProcessingServer.Destroy(1);

    // Assert
    EXPECT_NE(result, ERR_OK);
    VPE_LOGI("Destroy_ShouldReturnErrOk_WhenDestroyLockedReturnsErrOk: end!");
}

HWTEST_F(VideoProcessingServerTest, UpdateMetadata_ShouldReturnErrOk_WhenImageIsNotNull, TestSize.Level0)
{
    VPE_LOGI("UpdateMetadata_ShouldReturnErrOk_WhenImageIsNotNull: begin!");
    // Arrange
    int32_t clientID = 1;
    SurfaceBufferInfo image;
    image.surfacebuffer = SurfaceBuffer::Create();
    VideoProcessingServer videoProcessingServer(1, true);

    // Act
    ErrCode result = videoProcessingServer.UpdateMetadata(clientID, image);

    // Assert
    EXPECT_NE(result, ERR_OK);
    VPE_LOGI("UpdateMetadata_ShouldReturnErrOk_WhenImageIsNotNull: end!");
}

/**
 * @tc.name  : UpdateMetadata_ShouldReturnErrFail_WhenImageIsNull
 * @tc.number: VideoProcessingServerTest_002
 * @tc.desc  : Test UpdateMetadata function when image is null.
 */
HWTEST_F(VideoProcessingServerTest, UpdateMetadata_ShouldReturnErrFail_WhenImageIsNull, TestSize.Level0)
{
    VPE_LOGI("UpdateMetadata_ShouldReturnErrFail_WhenImageIsNull: begin!");
    // Arrange
    int32_t clientID = 1;
    SurfaceBufferInfo image;
    VideoProcessingServer videoProcessingServer(1, true);

    // Act
    ErrCode result = videoProcessingServer.UpdateMetadata(clientID, image);

    // Assert
    EXPECT_EQ(result, VPE_ALGO_ERR_INVALID_PARAM);
    VPE_LOGI("UpdateMetadata_ShouldReturnErrFail_WhenImageIsNull: end!");
}

HWTEST_F(VideoProcessingServerTest, Process_ShouldReturnErrOk_WhenInputOutputNotNull, TestSize.Level0)
{
    VPE_LOGI("Process_ShouldReturnErrOk_WhenInputOutputNotNull: begin!");
    VideoProcessingServer videoProcessingServer(1, true);
    SurfaceBufferInfo input;
    SurfaceBufferInfo output;
    output.surfacebuffer = SurfaceBuffer::Create();
    EXPECT_NE(videoProcessingServer.Process(1, input, output), ERR_OK);
    VPE_LOGI("Process_ShouldReturnErrOk_WhenInputOutputNotNull: end!");
}

/**
 * @tc.name  : Process_ShouldReturnErrInvalidParam_WhenOutputIsNull
 * @tc.number: VideoProcessingServerTest_002
 * @tc.desc  : Test if Process method returns ERR_FAIL when output is null.
 */
HWTEST_F(VideoProcessingServerTest, Process_ShouldReturnErrInvalidParam_WhenOutputIsNull, TestSize.Level0)
{
    VPE_LOGI("Process_ShouldReturnErrInvalidParam_WhenOutputIsNull: begin!");
    VideoProcessingServer videoProcessingServer(1, true);
    SurfaceBufferInfo input;
    SurfaceBufferInfo output;
    EXPECT_EQ(videoProcessingServer.Process(1, input, output), VPE_ALGO_ERR_INVALID_PARAM);
    VPE_LOGI("Process_ShouldReturnErrInvalidParam_WhenOutputIsNull: end!");
}

/**
 * @tc.name  : UnloadVideoProcessingSA_Success
 * @tc.number: VideoProcessingServerTest_001
 * @tc.desc  : Test when CreateUnloadHandler returns true then UnloadVideoProcessingSA should call DelayUnloadTask
 */
HWTEST_F(VideoProcessingServerTest, UnloadVideoProcessingSA_Success, TestSize.Level0)
{
    VPE_LOGI("UnloadVideoProcessingSA_Success: begin!");
    int32_t systemAbilityId = 123;
    VideoProcessingServer videoProcessingServer(1, true);
    // Verify that DelayUnloadTask was called
    videoProcessingServer.UnloadVideoProcessingSA();
    EXPECT_EQ(systemAbilityId, 123);
    VPE_LOGI("UnloadVideoProcessingSA_Success: end!");
}

HWTEST_F(VideoProcessingServerTest, ComposeImage_ShouldReturnErrOk_WhenInputIsValid, TestSize.Level0)
{
    VPE_LOGI("ComposeImage_ShouldReturnErrOk_WhenInputIsValid: begin!");
    // Arrange
    int32_t clientID = 1;
    SurfaceBufferInfo inputSdrImage;
    SurfaceBufferInfo inputGainmap;
    SurfaceBufferInfo outputHdrImage;
    outputHdrImage.surfacebuffer = SurfaceBuffer::Create();
    bool legacy = false;

    VideoProcessingServer videoProcessingServer(clientID, legacy);

    // Act
    ErrCode result = videoProcessingServer.ComposeImage(clientID, inputSdrImage, inputGainmap, outputHdrImage, legacy);

    // Assert
    EXPECT_NE(result, ERR_OK);
    VPE_LOGI("ComposeImage_ShouldReturnErrOk_WhenInputIsValid: end!");
}

/**
 * @tc.name  : ComposeImage_ShouldReturnErrInvalidParam_WhenOutputIsNull
 * @tc.number: VideoProcessingServerTest_002
 * @tc.desc  : Test ComposeImage function when output is null.
 */
HWTEST_F(VideoProcessingServerTest, ComposeImage_ShouldReturnErrInvalidParam_WhenOutputIsNull, TestSize.Level0)
{
    VPE_LOGI("ComposeImage_ShouldReturnErrInvalidParam_WhenOutputIsNull: begin!");
    // Arrange
    int32_t clientID = 1;
    SurfaceBufferInfo inputSdrImage;
    SurfaceBufferInfo inputGainmap;
    SurfaceBufferInfo outputHdrImage;
    bool legacy = false;

    VideoProcessingServer videoProcessingServer(clientID, legacy);

    // Act
    ErrCode result = videoProcessingServer.ComposeImage(clientID, inputSdrImage, inputGainmap, outputHdrImage, legacy);

    // Assert
    EXPECT_EQ(result, VPE_ALGO_ERR_INVALID_PARAM);
    VPE_LOGI("ComposeImage_ShouldReturnErrInvalidParam_WhenOutputIsNull: end!");
}


/**
 * @tc.name  : DecomposeImage_ShouldReturnErrOk_WhenInputIsValid
 * @tc.number: VideoProcessingServerTest_001
 * @tc.desc  : Test DecomposeImage function when input is valid.
 */
HWTEST_F(VideoProcessingServerTest, DecomposeImage_ShouldReturnErrOk_WhenInputIsValid, TestSize.Level0)
{
    VPE_LOGI("DecomposeImage_ShouldReturnErrOk_WhenInputIsValid: begin!");
    // Arrange
    int32_t clientID = 1;
    SurfaceBufferInfo inputImage;
    SurfaceBufferInfo outputSdrImage;
    SurfaceBufferInfo outputGainmap;
    VideoProcessingServer videoProcessingServer(clientID, true);

    // Act
    ErrCode result = videoProcessingServer.DecomposeImage(clientID, inputImage, outputSdrImage, outputGainmap);

    // Assert
    EXPECT_NE(result, ERR_OK);
    VPE_LOGI("DecomposeImage_ShouldReturnErrOk_WhenInputIsValid: end!");
}

/**
 * @tc.name  : DecomposeImage_ShouldReturnErrInvalidParam_WhenOutputIsNull
 * @tc.number: VideoProcessingServerTest_002
 * @tc.desc  : Test DecomposeImage function when output is null.
 */
HWTEST_F(VideoProcessingServerTest, DecomposeImage_ShouldReturnErrInvalidParam_WhenOutputIsNull, TestSize.Level0)
{
    VPE_LOGI("DecomposeImage_ShouldReturnErrInvalidParam_WhenOutputIsNull: begin!");
    // Arrange
    int32_t clientID = 1;
    SurfaceBufferInfo inputImage;
    SurfaceBufferInfo outputSdrImage;
    SurfaceBufferInfo outputGainmap;
    outputSdrImage.surfacebuffer = nullptr;
    outputGainmap.surfacebuffer = nullptr;
    VideoProcessingServer videoProcessingServer(clientID, true);

    // Act
    ErrCode result = videoProcessingServer.DecomposeImage(clientID, inputImage, outputSdrImage, outputGainmap);

    // Assert
    EXPECT_EQ(result, VPE_ALGO_ERR_INVALID_PARAM);
    VPE_LOGI("DecomposeImage_ShouldReturnErrInvalidParam_WhenOutputIsNull: end!");
}

/**
 * @tc.name  : DestroyLocked_ShouldReturnErrOk_WhenClientExistsAndAlgoNotNull
 * @tc.number: VideoProcessingServerTest_001
 * @tc.desc  : Test DestroyLocked function when client exists and algorithm is not null.
 */
HWTEST_F(VideoProcessingServerTest, DestroyLocked_ShouldReturnErrOk_WhenClientExistsAndAlgoNotNull, TestSize.Level0)
{
    VPE_LOGI("DestroyLocked_ShouldReturnErrOk_WhenClientExistsAndAlgoNotNull: begin!");
    VideoProcessingServer server(1, true);
    uint32_t id = 1;
    server.DestroyLocked(id);
    EXPECT_EQ(id, 1);
    VPE_LOGI("DestroyLocked_ShouldReturnErrOk_WhenClientExistsAndAlgoNotNull: end!");
}

/**
 * @tc.name  : DestroyLocked_ShouldReturnErrInvalidVal_WhenClientNotExists
 * @tc.number: VideoProcessingServerTest_002
 * @tc.desc  : Test DestroyLocked function when client not exists.
 */
HWTEST_F(VideoProcessingServerTest, DestroyLocked_ShouldReturnErrInvalidVal_WhenClientNotExists, TestSize.Level0)
{
    VPE_LOGI("DestroyLocked_ShouldReturnErrInvalidVal_WhenClientNotExists: begin!");
    VideoProcessingServer server(1, true);
    uint32_t id = 2;
    EXPECT_EQ(server.DestroyLocked(id), VPE_ALGO_ERR_INVALID_CLIENT_ID);
    VPE_LOGI("DestroyLocked_ShouldReturnErrInvalidVal_WhenClientNotExists: end!");
}

/**
 * @tc.name  : DestroyLocked_ShouldReturnErrInvalidVal_WhenAlgoNotExists
 * @tc.number: VideoProcessingServerTest_003
 * @tc.desc  : Test DestroyLocked function when algorithm not exists.
 */
HWTEST_F(VideoProcessingServerTest, DestroyLocked_ShouldReturnErrInvalidVal_WhenAlgoNotExists, TestSize.Level0)
{
    VPE_LOGI("DestroyLocked_ShouldReturnErrInvalidVal_WhenAlgoNotExists: begin!");
    VideoProcessingServer server(1, true);
    uint32_t id = 1;
    std::string feature = "feature1";
    server.clients_[id] = feature;
    EXPECT_EQ(server.DestroyLocked(id), VPE_ALGO_ERR_INVALID_VAL);
    VPE_LOGI("DestroyLocked_ShouldReturnErrInvalidVal_WhenAlgoNotExists: end!");
}

/**
 * @tc.name  : DestroyLocked_ShouldReturnErrInvalidVal_WhenAlgoIsNull
 * @tc.number: VideoProcessingServerTest_004
 * @tc.desc  : Test DestroyLocked function when algorithm is null.
 */
HWTEST_F(VideoProcessingServerTest, DestroyLocked_ShouldReturnErrInvalidVal_WhenAlgoIsNull, TestSize.Level0)
{
    VPE_LOGI("DestroyLocked_ShouldReturnErrInvalidVal_WhenAlgoIsNull: begin!");
    VideoProcessingServer server(1, true);
    uint32_t id = 1;
    std::string feature = "feature1";
    server.clients_[id] = feature;
    server.algorithms_[feature] = nullptr;
    EXPECT_EQ(server.DestroyLocked(id), VPE_ALGO_ERR_INVALID_VAL);
    VPE_LOGI("DestroyLocked_ShouldReturnErrInvalidVal_WhenAlgoIsNull: end!");
}

/**
 * @tc.name  : CreateUnloadHandler_ShouldCreateHandler_WhenHandlerIsNull
 * @tc.number: VideoProcessingServerTest_001
 * @tc.desc  : Test if CreateUnloadHandler creates a handler when the handler is null.
 */
HWTEST_F(VideoProcessingServerTest, CreateUnloadHandler_ShouldCreateHandler_WhenHandlerIsNull, TestSize.Level0)
{
    VPE_LOGI("CreateUnloadHandler_ShouldCreateHandler_WhenHandlerIsNull: begin!");
    // Arrange
    VideoProcessingServer videoProcessingServer(1, true);
    videoProcessingServer.unloadHandler_ = nullptr;

    // Act
    bool result = videoProcessingServer.CreateUnloadHandler();

    // Assert
    EXPECT_TRUE(result);
    EXPECT_NE(videoProcessingServer.unloadHandler_, nullptr);
    VPE_LOGI("CreateUnloadHandler_ShouldCreateHandler_WhenHandlerIsNull: end!");
}

/**
 * @tc.name  : DestroyUnloadHandler_ShouldNotRemoveAllEventsAndRemoveTask_WhenUnloadHandlerIsNull
 * @tc.number: VideoProcessingServerTest_002
 * @tc.desc  : Test DestroyUnloadHandler method when unloadHandler is null
 */
HWTEST_F(VideoProcessingServerTest, DestroyUnloadHandler_ShouldNotRemoveAllEventsAndRemoveTask_WhenUnloadHandlerIsNull, TestSize.Level0)
{
    VPE_LOGI("DestroyUnloadHandler_ShouldNotRemoveAllEventsAndRemoveTask_WhenUnloadHandlerIsNull: begin!");
    // Arrange
    VideoProcessingServer videoProcessingServer(1, true);
    videoProcessingServer.unloadHandler_ = nullptr;

    // Act
    videoProcessingServer.DestroyUnloadHandler();

    // Assert
    EXPECT_EQ(videoProcessingServer.unloadHandler_, nullptr);
    VPE_LOGI("DestroyUnloadHandler_ShouldNotRemoveAllEventsAndRemoveTask_WhenUnloadHandlerIsNull: end!");
}

HWTEST_F(VideoProcessingServerTest, DestroyUnloadHandler_ShouldNotRemoveAllEventsAndRemoveTask_WhenUnloadHandlerNotNull, TestSize.Level0)
{
    VPE_LOGI("DestroyUnloadHandler_ShouldNotRemoveAllEventsAndRemoveTask_WhenUnloadHandlerIsNull: begin!");
    // Arrange
    VideoProcessingServer videoProcessingServer(1, true);
    videoProcessingServer.unloadHandler_ =  std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create(UNLOAD_HANLDER));

    // Act
    videoProcessingServer.DestroyUnloadHandler();

    // Assert
    EXPECT_EQ(videoProcessingServer.unloadHandler_, nullptr);
    VPE_LOGI("DestroyUnloadHandler_ShouldNotRemoveAllEventsAndRemoveTask_WhenUnloadHandlerIsNull: end!");
}

/**
 * @tc.name  : DelayUnloadTask_ShouldLockAndUnlock_WhenCalled
 * @tc.number: VideoProcessingServerTest_001
 * @tc.desc  : Test that DelayUnloadTask locks and unlocks the mutex
 */
HWTEST_F(VideoProcessingServerTest, DelayUnloadTask_ShouldLockAndUnlock_WhenCalled, TestSize.Level0)
{
    VPE_LOGI("DelayUnloadTask_ShouldLockAndUnlock_WhenCalled: begin!");
    VideoProcessingServer videoProcessingServer(1, true);
    videoProcessingServer.DelayUnloadTask();
    VPE_LOGI("DelayUnloadTask_ShouldLockAndUnlock_WhenCalled: end!");
}

/**
 * @tc.name  : DelayUnloadTaskLocked_WhenIsWorkingIsTrue
 * @tc.number: VideoProcessingServerTest_001
 * @tc.desc  : Test when isWorking_ is true, the function will return immediately
 */
HWTEST_F(VideoProcessingServerTest, DelayUnloadTaskLocked_WhenIsWorkingIsTrue, TestSize.Level0)
{
    VPE_LOGI("DelayUnloadTaskLocked_WhenIsWorkingIsTrue: begin!");
    VideoProcessingServer videoProcessingServer(1, true);
    videoProcessingServer.isWorking_ = true;
    videoProcessingServer.DelayUnloadTaskLocked();
    ASSERT_TRUE(videoProcessingServer.isWorking_);
    VPE_LOGI("DelayUnloadTaskLocked_WhenIsWorkingIsTrue: end!");
}

/**
 * @tc.name  : DelayUnloadTaskLocked_WhenUnloadHandlerIsNull
 * @tc.number: VideoProcessingServerTest_002
 * @tc.desc  : Test when unloadHandler_ is null, the function will return immediately
 */
HWTEST_F(VideoProcessingServerTest, DelayUnloadTaskLocked_WhenUnloadHandlerIsNull, TestSize.Level0)
{
    VPE_LOGI("DelayUnloadTaskLocked_WhenUnloadHandlerIsNull: begin!");
    VideoProcessingServer videoProcessingServer(1, true);
    videoProcessingServer.unloadHandler_ = nullptr;
    videoProcessingServer.DelayUnloadTaskLocked();
    ASSERT_TRUE(videoProcessingServer.unloadHandler_ != nullptr);
    VPE_LOGI("DelayUnloadTaskLocked_WhenUnloadHandlerIsNull: end!");
}

HWTEST_F(VideoProcessingServerTest, DelayUnloadTaskLocked_WhenUnloadHandlerNotNull, TestSize.Level0)
{
    VPE_LOGI("DelayUnloadTaskLocked_WhenUnloadHandlerIsNull: begin!");
    VideoProcessingServer videoProcessingServer(1, true);
    videoProcessingServer.unloadHandler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create(UNLOAD_HANLDER));
    videoProcessingServer.DelayUnloadTaskLocked();
    ASSERT_TRUE(videoProcessingServer.unloadHandler_ != nullptr);
    VPE_LOGI("DelayUnloadTaskLocked_WhenUnloadHandlerIsNull: end!");
}

HWTEST_F(VideoProcessingServerTest, ClearAlgorithms_ShouldClearAlgorithms_WhenCalled, TestSize.Level0)
{
    VPE_LOGI("ClearAlgorithms_ShouldClearAlgorithms_WhenCalled: begin!");
    VideoProcessingServer server(1, true);
    server.algorithms_["test_algo"] = nullptr;
    server.clients_[1] = "test_client";
    server.isWorking_ = true;

    server.ClearAlgorithms();

    EXPECT_TRUE(server.algorithms_.empty());
    EXPECT_TRUE(server.clients_.empty());
    EXPECT_FALSE(server.isWorking_.load());
    VPE_LOGI("ClearAlgorithms_ShouldClearAlgorithms_WhenCalled: end!");
}

TEST_F(VideoProcessingServerTest, SetParameter_ShouldReturnError_WhenInvalidClientID)
{
    VideoProcessingServer videoProcessingServer(1, true);
    int32_t clientID = -1;
    int32_t tag = 2;
    std::vector<uint8_t> parameter = {1, 2, 3, 4, 5};

    ErrCode result = videoProcessingServer.SetParameter(clientID, tag, parameter);

    EXPECT_NE(result, VPE_ALGO_ERR_OK);
}
TEST_F(VideoProcessingServerTest, SetParameter_ShouldReturnError_WhenEmptyParameter)
{
    VideoProcessingServer videoProcessingServer(1, true);
    int32_t clientID = 1;
    int32_t tag = 2;
    std::vector<uint8_t> parameter = {};

    ErrCode result = videoProcessingServer.SetParameter(clientID, tag, parameter);

    EXPECT_NE(result, VPE_ALGO_ERR_OK);
}

TEST_F(VideoProcessingServerTest, GetParameter_ShouldReturnSuccess_WhenValidParameters)
{
    VideoProcessingServer videoProcessingServer(1, true);
    int32_t clientID = 1;
    int32_t tag = 1;
    std::vector<uint8_t> parameter;

    ErrCode result = videoProcessingServer.GetParameter(clientID, tag, parameter);

    EXPECT_NE(result, VPE_ALGO_ERR_OK);
}

TEST_F(VideoProcessingServerTest, GetParameter_ShouldReturnError_WhenInvalidClientID)
{
    VideoProcessingServer videoProcessingServer(1, true);
    int32_t clientID = -1;
    int32_t tag = 1;
    std::vector<uint8_t> parameter;

    ErrCode result = videoProcessingServer.GetParameter(clientID, tag, parameter);

    EXPECT_NE(result, VPE_ALGO_ERR_OK);
}

TEST_F(VideoProcessingServerTest, GetParameter_ShouldReturnError_WhenInvalidTag)
{
    VideoProcessingServer videoProcessingServer(1, true);
    int32_t clientID = 1;
    int32_t tag = -1;
    std::vector<uint8_t> parameter;

    ErrCode result = videoProcessingServer.GetParameter(clientID, tag, parameter);

    EXPECT_NE(result, VPE_ALGO_ERR_OK);
}

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
