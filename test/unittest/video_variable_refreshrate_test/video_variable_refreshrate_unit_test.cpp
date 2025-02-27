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
#include <cstdint>
#include <string_view>
#include <fstream>
#include "iostream"
#include "securec.h"
#include "vpe_trace.h"
#include "vpe_log.h"
#include "video_refreshrate_prediction.h"
#include "v2_0/buffer_handle_meta_key_type.h"
#include "v1_2/display_composer_type.h"

using namespace std;
using namespace testing::ext;

namespace {
const std::string MV_FILE = "3840x1608.pmv";
const std::string UT_PROCESS_NAME = "video_variable_refreshrate_unit_test";
} // namespace

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

class VideoVariableRefreshRateUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void VideoVariableRefreshRateUnitTest::SetUpTestCase(void)
{
    cout << "[VideoVariableRefreshRateUnitTest SetUpTestCase]: success!" << endl;
}

void VideoVariableRefreshRateUnitTest::TearDownTestCase(void)
{
    cout << "[VideoVariableRefreshRateUnitTest TearDownTestCase]: " << endl;
}

void VideoVariableRefreshRateUnitTest::SetUp(void)
{
    cout << "[VideoVariableRefreshRateUnitTest SetUp]: SetUp!!!" << endl;
}

void VideoVariableRefreshRateUnitTest::TearDown(void)
{
    cout << "[VideoVariableRefreshRateUnitTest TearDown]: over!!!" << endl;
}

uint32_t CalculateMaxMvBufferSize(uint32_t width, uint32_t height)
{
    uint32_t align = 64;
    uint32_t multCoef = 1;
    uint32_t diviCoef = 4;
    uint32_t offset = 256;
    uint32_t mvWidth = (width + align - 1) & (~(align - 1));
    uint32_t mvHeight = (height + align - 1) & (~(align - 1));
    return mvWidth * mvHeight * multCoef / diviCoef + offset;
}

sptr<SurfaceBuffer> CreateSurfaceBuffer(uint32_t pixelFormat, int32_t width, int32_t height, uint8_t *mvBuffer,
    int32_t mvLength)
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
    inputCfg.usage = BUFFER_USAGE_CPU_READ | BUFFER_USAGE_CPU_WRITE | BUFFER_USAGE_HW_RENDER
    | BUFFER_USAGE_HW_TEXTURE | HDI::Display::Composer::V1_2::HBM_USE_VIDEO_DEC_MV;
    inputCfg.format = pixelFormat;
    inputCfg.timeout = 0;
    GSError err = buffer->Alloc(inputCfg);

    using namespace HDI::Display::Graphic::Common;
    std::vector<uint8_t> vec;
    V2_0::BlobDataType data;
    int32_t ret = buffer->GetMetadata(V2_0::ATTRKEY_VIDEO_DECODER_MV, vec);
    CHECK_AND_RETURN_RET_LOG(ret == GSERROR_OK && (vec.size() == sizeof(V2_0::BlobDataType)),
        nullptr, "VRR got decode mv type from handle failed");
    ret = memcpy_s(&data, sizeof(V2_0::BlobDataType), vec.data(), vec.size());
    if (GSERROR_OK != err) {
        printf("Alloc surface buffer failed\n");
        return nullptr;
    }
    ret = memcpy_s((void *)(data.vaddr + data.offset), mvLength, mvBuffer, mvLength);
    if (GSERROR_OK != err) {
        printf("Alloc surface buffer failed\n");
        return nullptr;
    }
    printf("Alloc surface buffer with motion vecotr success\n");
    return buffer;
}

HWTEST_F(VideoVariableRefreshRateUnitTest, VideoVariableRefreshRate_init_01, TestSize.Level1)
{
    auto vrrPredictor = OHOS::Media::VideoProcessingEngine::VideoRefreshRatePrediction::Create();
    VPEAlgoErrCode ret = vrrPredictor->CheckVRRSupport(UT_PROCESS_NAME);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(VideoVariableRefreshRateUnitTest, VideoVariableRefreshRate_init_02, TestSize.Level1)
{
    VideoRefreshRatePredictionHandle *vrrHandle = VideoRefreshRatePredictionCreate();
    int32_t ret =  VideoRefreshRatePredictionCheckSupport(vrrHandle, UT_PROCESS_NAME.c_str());
    VideoRefreshRatePredictionDestroy(vrrHandle);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(VideoVariableRefreshRateUnitTest, VideoVariableRefreshRate_implProcess_01, TestSize.Level1)
{
    VPEAlgoErrCode ret = VPE_ALGO_ERR_OK;
    int32_t width = 3840;
    int32_t height = 1608;
    auto vrrPredictor = OHOS::Media::VideoProcessingEngine::VideoRefreshRatePrediction::Create();
    int mvLength = CalculateMaxMvBufferSize(width, height);
    uint8_t *mvBuffer = (uint8_t *)malloc(mvLength);
    sptr<SurfaceBuffer> inputFrame = CreateSurfaceBuffer(GRAPHIC_PIXEL_FMT_RGBA_8888,
            width, height, mvBuffer, mvLength);
    ret = vrrPredictor->Process(inputFrame, 60, MOTIONVECTOR_TYPE_HEVC);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
    free(mvBuffer);
}

HWTEST_F(VideoVariableRefreshRateUnitTest, VideoVariableRefreshRate_implProcess_02, TestSize.Level1)
{
    VPEAlgoErrCode ret = VPE_ALGO_ERR_OK;
    int32_t width = 3840;
    int32_t height = 1608;
    auto vrrPredictor = OHOS::Media::VideoProcessingEngine::VideoRefreshRatePrediction::Create();
    int mvLength = CalculateMaxMvBufferSize(width, height);
    uint8_t *mvBuffer = (uint8_t *)malloc(mvLength);
    FILE *fp = fopen(MV_FILE.c_str(), "rb");
    if (fp == nullptr) {
        printf("open motion vector file [%s] fail!\n", MV_FILE.c_str());
        return;
    }
    int frameIndex = 0;
    while (1) {
        int readCnt = fread(mvBuffer, 1, mvLength, fp);
        if (readCnt < mvLength) {
            break;
        }
        sptr<SurfaceBuffer> inputFrame = CreateSurfaceBuffer(GRAPHIC_PIXEL_FMT_RGBA_8888,
            width, height, mvBuffer, mvLength);
        ret = vrrPredictor->Process(inputFrame, 60, MOTIONVECTOR_TYPE_HEVC);
        EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
        frameIndex++;
    }
    free(mvBuffer);
    fclose(fp);
    EXPECT_EQ(ret, VPE_ALGO_ERR_OK);
}

HWTEST_F(VideoVariableRefreshRateUnitTest, VideoVariableRefreshRate_implProcess_03, TestSize.Level1)
{
    int32_t width = 3840;
    int32_t height = 1608;
    VideoRefreshRatePredictionHandle *vrrHandle = VideoRefreshRatePredictionCreate();
    int32_t ret =  VideoRefreshRatePredictionCheckSupport(vrrHandle, UT_PROCESS_NAME.c_str());
    int mvLength = CalculateMaxMvBufferSize(width, height);
    uint8_t *mvBuffer = (uint8_t *)malloc(mvLength);
    sptr<SurfaceBuffer> inputFrame = CreateSurfaceBuffer(GRAPHIC_PIXEL_FMT_RGBA_8888,
            width, height, mvBuffer, mvLength);
    VideoRefreshRatePredictionProcess(vrrHandle, inputFrame->SurfaceBufferToNativeBuffer(), 60, MOTIONVECTOR_TYPE_HEVC);
    VideoRefreshRatePredictionDestroy(vrrHandle);
    free(mvBuffer);
    EXPECT_NE(ret, VPE_ALGO_ERR_OK);
}

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS