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

#ifndef FRAMEWORK_DFX_TRACE_H
#define FRAMEWORK_DFX_TRACE_H

#include <string>
#include "nocopyable.h"
#include "refbase.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
#define VPE_SYNC_TRACE VPETrace trace(std::string(__FUNCTION__))

class __attribute__((visibility("default"))) VPETrace : public NoCopyable {
public:
    explicit VPETrace(const std::string& funcName);
    static void TraceBegin(const std::string& funcName, int32_t taskId);
    static void TraceEnd(const std::string& funcName, int32_t taskId);
    static void CounterTrace(const std::string& varName, int32_t val);
    ~VPETrace();

private:
    bool isSync_ = false;
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
#endif // FRAMEWORK_DFX_TRACE_H
