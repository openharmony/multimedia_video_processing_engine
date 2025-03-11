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

#include "videohdrvividtohdr_fuzzer.h"
#include "video_sample.h"
#include <cstddef>
#include <cstdint>

namespace OHOS {
VideoSample *sample = nullptr;
constexpr uint32_t DEFAULT_WIDTH = 3840;
constexpr uint32_t DEFAULT_HEIGHT = 2160;
bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (!sample) {
        sample = new VideoSample();
        VideoProcessParam param = {NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_PQ_LIMIT,
                                   NATIVEBUFFER_PIXEL_FMT_YCBCR_P010, OH_COLORSPACE_BT2020_PQ_LIMIT};
        sample->InitVideoSample(DEFAULT_WIDTH, DEFAULT_HEIGHT, param);
        sample->StartProcess();
    }
    return sample->InputFunc(data, size);
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}