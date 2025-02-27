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
#include <fstream>
#include <memory>
#include "csc_sample.h"
#include "csc_sample_define.h"
#include "refbase.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
using namespace CSCSampleDefine;
class CSCModuleTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};

    void SetParameter(std::shared_ptr<ColorSpaceConverter> plugin);
    sptr<SurfaceBuffer> PrepareOneFrame();
};

void CSCModuleTest::SetParameter(std::shared_ptr<ColorSpaceConverter> plugin)
{
    ColorSpaceConverterParameter parameterSet;
    parameterSet.renderIntent = RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC;
    int32_t ret = plugin->SetParameter(parameterSet);
    ASSERT_EQ(VPE_ALGO_ERR_OK, ret);
}

sptr<SurfaceBuffer> CSCModuleTest::PrepareOneFrame()
{
    std::unique_ptr<std::ifstream> metadataFile =
        std::make_unique<std::ifstream>(METADATA_FILE.data(), std::ios::binary | std::ios::in);
    std::unique_ptr<std::ifstream> yuvFile =
        std::make_unique<std::ifstream>(YUV_FILE.data(), std::ios::binary | std::ios::in);

    auto buffer = CreateSurfaceBuffer(YUV_FILE_PIXEL_FORMAT, WIDTH, HEIGHT);
    if (buffer == nullptr) {
        return nullptr;
    }
    SetMeatadata(buffer, INPUT_COLORSPACE_INFO);
    SetMeatadata(buffer, CM_VIDEO_HDR_VIVID);
    SetMeatadata(buffer, metadataFile);
    ReadYuvFile(buffer, yuvFile, ONE_FRAME_SIZE);

    return buffer;
}

/**
 * @tc.number    : 0101
 * @tc.func      : Create
 * @tc.desc      : Test for ColorSpaceConverter Create
 */
HWTEST_F(CSCModuleTest, Create_0101, TestSize.Level1)
{
    auto plugin = ColorSpaceConverter::Create();
    ASSERT_NE(nullptr, plugin);
}

/**
 * @tc.number    : 0201
 * @tc.func      : SetParameter
 * @tc.desc      : parameter.renderIntent != RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC
 */
HWTEST_F(CSCModuleTest, SetParameter_0201, TestSize.Level1)
{
    auto plugin = ColorSpaceConverter::Create();
    ASSERT_NE(nullptr, plugin);
    
    ColorSpaceConverterParameter parameterSet;
    parameterSet.renderIntent = RenderIntent::RENDER_INTENT_PERCEPTUAL;
    int32_t ret = plugin->SetParameter(parameterSet);
    ASSERT_NE(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0202
 * @tc.func      : SetParameter
 * @tc.desc      : parameter.renderIntent = RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC
 */
HWTEST_F(CSCModuleTest, SetParameter_0202, TestSize.Level1)
{
    auto plugin = ColorSpaceConverter::Create();
    ASSERT_NE(nullptr, plugin);
    
    SetParameter(plugin);
}

/**
 * @tc.number    : 0301
 * @tc.func      : GetParameter
 * @tc.desc      : Call after Create
 */
HWTEST_F(CSCModuleTest, GetParameter_0301, TestSize.Level1)
{
    auto plugin = ColorSpaceConverter::Create();
    ASSERT_NE(nullptr, plugin);

    ColorSpaceConverterParameter parameterGet;
    int32_t ret = plugin->GetParameter(parameterGet);
    ASSERT_NE(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0302
 * @tc.func      : GetParameter
 * @tc.desc      : Call after SetParameter
 */
HWTEST_F(CSCModuleTest, GetParameter_0302, TestSize.Level1)
{
    auto plugin = ColorSpaceConverter::Create();
    ASSERT_NE(nullptr, plugin);
    
    SetParameter(plugin);

    ColorSpaceConverterParameter parameterGet;
    int32_t ret = plugin->GetParameter(parameterGet);
    ASSERT_EQ(VPE_ALGO_ERR_OK, ret);
    ASSERT_EQ(RenderIntent::RENDER_INTENT_ABSOLUTE_COLORIMETRIC, parameterGet.renderIntent);
}

/**
 * @tc.number    : 0401
 * @tc.func      : Process
 * @tc.desc      : Call after Create
 */
HWTEST_F(CSCModuleTest, Process_0401, TestSize.Level1)
{
    auto plugin = ColorSpaceConverter::Create();
    ASSERT_NE(nullptr, plugin);
    
    int32_t ret = plugin->Process(SurfaceBuffer::Create(), SurfaceBuffer::Create());
    ASSERT_NE(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0402
 * @tc.func      : Process
 * @tc.desc      : Call after SetParameter, input is null
 */
HWTEST_F(CSCModuleTest, Process_0402, TestSize.Level1)
{
    auto plugin = ColorSpaceConverter::Create();
    ASSERT_NE(nullptr, plugin);
    
    SetParameter(plugin);

    int32_t ret = plugin->Process(nullptr, SurfaceBuffer::Create());
    ASSERT_NE(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0403
 * @tc.func      : Process
 * @tc.desc      : Call after SetParameter, output is null
 */
HWTEST_F(CSCModuleTest, Process_0403, TestSize.Level1)
{
    auto plugin = ColorSpaceConverter::Create();
    ASSERT_NE(nullptr, plugin);
    
    SetParameter(plugin);

    int32_t ret = plugin->Process(SurfaceBuffer::Create(), nullptr);
    ASSERT_NE(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0404
 * @tc.func      : Process
 * @tc.desc      : Call after SetParameter, input and output is not null
 */
HWTEST_F(CSCModuleTest, Process_0404, TestSize.Level1)
{
    auto plugin = ColorSpaceConverter::Create();
    ASSERT_NE(nullptr, plugin);
    
    SetParameter(plugin);

    auto input = PrepareOneFrame();
    ASSERT_NE(nullptr, input);
    auto output = CreateSurfaceBuffer(OUTPUT_PIXEL_FORMAT, WIDTH, HEIGHT);
    ASSERT_NE(nullptr, output);
    SetMeatadata(output, OUTPUT_COLORSPACE_INFO);

    int32_t ret = plugin->Process(input, output);
    ASSERT_EQ(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0501
 * @tc.func      : ComposeImage
 * @tc.desc      : Call after Create
 */
HWTEST_F(CSCModuleTest, ComposeImage_0501, TestSize.Level1)
{
    auto plugin = ColorSpaceConverter::Create();
    ASSERT_NE(nullptr, plugin);

    int32_t ret = plugin->ComposeImage(
        SurfaceBuffer::Create(), SurfaceBuffer::Create(), SurfaceBuffer::Create(), false);
    ASSERT_NE(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0502
 * @tc.func      : ComposeImage
 * @tc.desc      : Call after SetParameter, inputSdrImage is null
 */
HWTEST_F(CSCModuleTest, ComposeImage_0502, TestSize.Level1)
{
    auto plugin = ColorSpaceConverter::Create();
    ASSERT_NE(nullptr, plugin);
    
    SetParameter(plugin);
    
    int32_t ret = plugin->ComposeImage(nullptr, SurfaceBuffer::Create(), SurfaceBuffer::Create(), false);
    ASSERT_NE(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0503
 * @tc.func      : ComposeImage
 * @tc.desc      : Call after SetParameter, inputGainmap is null
 */
HWTEST_F(CSCModuleTest, ComposeImage_0503, TestSize.Level1)
{
    auto plugin = ColorSpaceConverter::Create();
    ASSERT_NE(nullptr, plugin);
    
    SetParameter(plugin);
    
    int32_t ret = plugin->ComposeImage(SurfaceBuffer::Create(), nullptr, SurfaceBuffer::Create(), false);
    ASSERT_NE(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0504
 * @tc.func      : ComposeImage
 * @tc.desc      : Call after SetParameter, outputHdrImage is null
 */
HWTEST_F(CSCModuleTest, ComposeImage_0504, TestSize.Level1)
{
    auto plugin = ColorSpaceConverter::Create();
    ASSERT_NE(nullptr, plugin);
    
    SetParameter(plugin);
    
    int32_t ret = plugin->ComposeImage(SurfaceBuffer::Create(), SurfaceBuffer::Create(), nullptr, false);
    ASSERT_NE(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0505
 * @tc.func      : ComposeImage
 * @tc.desc      : Call after SetParameter, inputSdrImage, inputGainmap and outputHdrImage is not null, legacy is false
 */
HWTEST_F(CSCModuleTest, ComposeImage_0505, TestSize.Level1)
{
    auto plugin = ColorSpaceConverter::Create();
    ASSERT_NE(nullptr, plugin);
    
    SetParameter(plugin);

    auto inputSdrImage = CreateSurfaceBuffer(YUV_FILE_PIXEL_FORMAT, WIDTH, HEIGHT);
    auto inputGainmap = CreateSurfaceBuffer(YUV_FILE_PIXEL_FORMAT, WIDTH, HEIGHT);
    auto outputHdrImage = CreateSurfaceBuffer(OUTPUT_PIXEL_FORMAT, WIDTH, HEIGHT);
    ASSERT_NE(nullptr, inputSdrImage);
    ASSERT_NE(nullptr, inputGainmap);
    ASSERT_NE(nullptr, outputHdrImage);

    int32_t ret = plugin->ComposeImage(inputSdrImage, inputGainmap, outputHdrImage, false);
    ASSERT_NE(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0506
 * @tc.func      : ComposeImage
 * @tc.desc      : Call after SetParameter, inputSdrImage, inputGainmap and outputHdrImage is not null, legacy is true
 */
HWTEST_F(CSCModuleTest, ComposeImage_0506, TestSize.Level1)
{
    auto plugin = ColorSpaceConverter::Create();
    ASSERT_NE(nullptr, plugin);
    
    SetParameter(plugin);

    auto inputSdrImage = CreateSurfaceBuffer(YUV_FILE_PIXEL_FORMAT, WIDTH, HEIGHT);
    auto inputGainmap = CreateSurfaceBuffer(YUV_FILE_PIXEL_FORMAT, WIDTH, HEIGHT);
    auto outputHdrImage = CreateSurfaceBuffer(OUTPUT_PIXEL_FORMAT, WIDTH, HEIGHT);
    ASSERT_NE(nullptr, inputSdrImage);
    ASSERT_NE(nullptr, inputGainmap);
    ASSERT_NE(nullptr, outputHdrImage);

    int32_t ret = plugin->ComposeImage(inputSdrImage, inputGainmap, outputHdrImage, true);
    ASSERT_NE(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0601
 * @tc.func      : DecomposeImage
 * @tc.desc      : Call after Create
 */
HWTEST_F(CSCModuleTest, DecomposeImage_0601, TestSize.Level1)
{
    auto plugin = ColorSpaceConverter::Create();
    ASSERT_NE(nullptr, plugin);
    
    int32_t ret = plugin->DecomposeImage(
        SurfaceBuffer::Create(), SurfaceBuffer::Create(), SurfaceBuffer::Create());
    ASSERT_NE(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0602
 * @tc.func      : DecomposeImage
 * @tc.desc      : Call after SetParameter, inputImage is null
 */
HWTEST_F(CSCModuleTest, DecomposeImage_0602, TestSize.Level1)
{
    auto plugin = ColorSpaceConverter::Create();
    ASSERT_NE(nullptr, plugin);
    
    SetParameter(plugin);

    auto inputImage = CreateSurfaceBuffer(YUV_FILE_PIXEL_FORMAT, WIDTH, HEIGHT);
    ASSERT_NE(nullptr, inputImage);

    int32_t ret = plugin->DecomposeImage(inputImage, SurfaceBuffer::Create(), SurfaceBuffer::Create());
    ASSERT_NE(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0603
 * @tc.func      : DecomposeImage
 * @tc.desc      : Call after SetParameter, outputSdrImage is null
 */
HWTEST_F(CSCModuleTest, DecomposeImage_0603, TestSize.Level1)
{
    auto plugin = ColorSpaceConverter::Create();
    ASSERT_NE(nullptr, plugin);
    
    SetParameter(plugin);

    auto outputSdrImage = CreateSurfaceBuffer(YUV_FILE_PIXEL_FORMAT, WIDTH, HEIGHT);
    ASSERT_NE(nullptr, outputSdrImage);

    int32_t ret = plugin->DecomposeImage(SurfaceBuffer::Create(), outputSdrImage, SurfaceBuffer::Create());
    ASSERT_NE(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0604
 * @tc.func      : DecomposeImage
 * @tc.desc      : Call after SetParameter, outputGainmap is null
 */
HWTEST_F(CSCModuleTest, DecomposeImage_0604, TestSize.Level1)
{
    auto plugin = ColorSpaceConverter::Create();
    ASSERT_NE(nullptr, plugin);
    
    SetParameter(plugin);

    auto outputGainmap = CreateSurfaceBuffer(YUV_FILE_PIXEL_FORMAT, WIDTH, HEIGHT);
    ASSERT_NE(nullptr, outputGainmap);

    int32_t ret = plugin->DecomposeImage(SurfaceBuffer::Create(), SurfaceBuffer::Create(), outputGainmap);
    ASSERT_NE(VPE_ALGO_ERR_OK, ret);
}

/**
 * @tc.number    : 0605
 * @tc.func      : DecomposeImage
 * @tc.desc      : Call after SetParameter, inputImage, outputSdrImage and outputGainmap is not null
 */
HWTEST_F(CSCModuleTest, DecomposeImage_0605, TestSize.Level1)
{
    auto plugin = ColorSpaceConverter::Create();
    ASSERT_NE(nullptr, plugin);
    
    SetParameter(plugin);

    auto inputImage = CreateSurfaceBuffer(YUV_FILE_PIXEL_FORMAT, WIDTH, HEIGHT);
    auto outputSdrImage = CreateSurfaceBuffer(YUV_FILE_PIXEL_FORMAT, WIDTH, HEIGHT);
    auto outputGainmap = CreateSurfaceBuffer(OUTPUT_PIXEL_FORMAT, WIDTH, HEIGHT);
    ASSERT_NE(nullptr, inputImage);
    ASSERT_NE(nullptr, outputSdrImage);
    ASSERT_NE(nullptr, outputGainmap);

    int32_t ret = plugin->DecomposeImage(inputImage, outputSdrImage, outputGainmap);
    ASSERT_NE(VPE_ALGO_ERR_OK, ret);
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS