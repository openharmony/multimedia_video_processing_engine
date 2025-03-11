/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "iamgedecompose_fuzzer.h"
#include "image_processing.h"
#include "image/pixelmap_native.h"
#include "native_color_space_manager.h"
#include <cstddef>
#include <cstdint>

constexpr uint32_t DEFAULT_WIDTH = 3840;
constexpr uint32_t DEFAULT_HEIGHT = 2160;

static void CreatePixelmap(OH_PixelmapNative **pixelmap, int32_t width, int32_t height, int format,
                            OH_NativeColorSpaceManager *colorSpaceNative)
{
    OH_Pixelmap_InintializationOptions *options = nullptr;
    (void)OH_Pixelmap_InintializationOptions_Create(&options);
    (void)OH_Pixelmap_InintializationOptions_SetWidth(options, width);
    (void)OH_Pixelmap_InintializationOptions_SetHeight(options, height);
    (void)OH_Pixelmap_InintializationOptions_SetPixelFormat(options, format);
    (void)OH_PixelmapNative_CreateEmptyPixelmap(options, pixelmap);
    (void)OH_PixelmapNative_SetColorSpaceNative(*pixelmap, colorSpaceNative);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    OH_ImageProcessing* imageProcessor = nullptr;
    OH_ImageProcessing_Create(&imageProcessor, IMAGE_PROCESSING_TYPE_DECOMPOSITION);
    OH_NativeColorSpaceManager *srcColorSpace = OH_NativeColorSpaceManager_CreateForName(BT2020_PQ);
    OH_NativeColorSpaceManager *dstColorSpace = OH_NativeColorSpaceManager_CreateForName(SRGB);
    OH_PixelmapNative *src = nullptr;
    OH_PixelmapNative *dstPic = nullptr;
    OH_PixelmapNative *dstGainmap = nullptr;
    CreatePixelmap(&src, DEFAULT_WIDTH, DEFAULT_HEIGHT, PIXEL_FORMAT_RGBA_1010102, srcColorSpace);
    CreatePixelmap(&dstPic, DEFAULT_WIDTH, DEFAULT_HEIGHT, PIXEL_FORMAT_RGBA_8888, dstColorSpace);
    CreatePixelmap(&dstGainmap, DEFAULT_WIDTH, DEFAULT_HEIGHT, PIXEL_FORMAT_RGBA_8888, dstColorSpace);
    OH_PixelmapNative_WritePixels(src, const_cast<unit8_t *>(data), size);
    bool ret = OH_ImageProcessing_Decompose(imageProcessor, src, dstPic, dstGainmap);
    OH_PixelmapNative_Release(src);
    OH_PixelmapNative_Release(dstPic);
    OH_PixelmapNative_Release(dstGainmap);
    OH_NativeColorSpaceManager_Destroy(srcColorSpace);
    OH_NativeColorSpaceManager_Destroy(dstColorSpace);
    OH_ImageProcessing_Destroy(imageProcessor);
    return ret;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}