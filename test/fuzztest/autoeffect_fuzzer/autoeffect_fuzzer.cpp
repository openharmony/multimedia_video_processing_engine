/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdint>
#include <string>
#include <fuzzer/FuzzedDataProvider.h>

#include "video_processing.h"
#include "video_processing_types.h"
#include "native_avformat.h"
#include "autoeffect_fuzzer.h"

namespace {
bool IsAutoEffectSupportedFuzzTest(uint32_t type)
{
    return OH_VideoProcessing_IsAutoEffectSupported(type);
}

VideoProcessing_ErrorCode UseAutoEffectFuzzTest(uint32_t type, bool enable, const char* name)
{
    return OH_VideoProcessing_UseAutoEffect(type, enable, name);
}

VideoProcessing_ErrorCode SetAutoEffectParamFuzzTest(uint32_t type, const char* name, const OH_AVFormat* param)
{
    return OH_VideoProcessing_SetAutoEffectParam(type, name, param);
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FuzzedDataProvider dataProvider(data, size);

    uint32_t type = dataProvider.ConsumeBool()
        ? static_cast<uint32_t>(VIDEO_PROCESSING_TYPE_AUTOEFFECT_AISR)
        : dataProvider.ConsumeIntegral<uint32_t>();

    bool enable = dataProvider.ConsumeBool();

    std::string nameStr = dataProvider.ConsumeRandomLengthString(64);
    const char* name = dataProvider.ConsumeBool() ? nullptr : nameStr.c_str();

    OH_AVFormat* param = nullptr;
    if (!dataProvider.ConsumeBool()) {
        param = OH_AVFormat_Create();
        if (param != nullptr) {
            int32_t enableVal = dataProvider.ConsumeIntegralInRange<int32_t>(-2, 2);
            float strength = dataProvider.ConsumeFloatingPointInRange<float>(-1.0f, 2.0f);
            uint8_t paramMode = dataProvider.ConsumeIntegralInRange<uint8_t>(0, 3);
            if (paramMode & 0x1) {
                OH_AVFormat_SetIntValue(param, VIDEO_AUTOEFFECT_ENABLE, enableVal);
            }
            if (paramMode & 0x2) {
                OH_AVFormat_SetFloatValue(param, VIDEO_AUTOEFFECT_AISR_STRENGTH, strength);
            }
        }
    }

    (void)IsAutoEffectSupportedFuzzTest(type);

    (void)UseAutoEffectFuzzTest(type, enable, name);

    (void)SetAutoEffectParamFuzzTest(type, name, param);

    if (param != nullptr) {
        OH_AVFormat_Destroy(param);
    }
    return 0;
}
