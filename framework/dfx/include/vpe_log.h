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

#ifndef FRAMEWORK_DFX_LOG_H
#define FRAMEWORK_DFX_LOG_H

#include <cinttypes>
#include "hilog/log.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
namespace VPELogFlag {
extern const bool VERBOSE_LOG;
} // VPELogFlag

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B3F

#undef LOG_TAG
#define LOG_TAG "VIDEOPROCESSENGINE"
#undef LOG_FMT
#define LOG_FMT "[%{public}s][%{public}s %{public}d] "

#define VPE_LOGF(fmt, ...) HILOG_FATAL(LOG_CORE, LOG_FMT fmt, __FILE_NAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define VPE_LOGE(fmt, ...) HILOG_ERROR(LOG_CORE, LOG_FMT fmt, __FILE_NAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define VPE_LOGW(fmt, ...) HILOG_WARN(LOG_CORE, LOG_FMT fmt, __FILE_NAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define VPE_LOGI(fmt, ...) HILOG_INFO(LOG_CORE, LOG_FMT fmt, __FILE_NAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define VPE_LOGD(fmt, ...) HILOG_DEBUG(LOG_CORE, LOG_FMT fmt, __FILE_NAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define VPE_LOGD_LIMIT(frequency, fmt, ...)                  \
    do {                                                     \
        thread_local uint64_t currentTimes = 0;              \
        if (currentTimes++ % ((uint64_t)(frequency)) == 0) { \
            VPE_LOGD(fmt, ##__VA_ARGS__);                    \
        }                                                    \
    } while (0)

#define VPE_LOGV(fmt, ...)            \
    if (VPELogFlag::VERBOSE_LOG) {    \
        VPE_LOGD(fmt, ##__VA_ARGS__); \
    }

#define CHECK_AND_RETURN_RET_LOG(cond, ret, fmt, ...) \
    do {                                              \
        if (!(cond)) {                                \
            VPE_LOGE(fmt, ##__VA_ARGS__);             \
            return ret;                               \
        }                                             \
    } while (0)

#define CHECK_AND_RETURN_LOG(cond, fmt, ...) \
    do {                                     \
        if (!(cond)) {                       \
            VPE_LOGE(fmt, ##__VA_ARGS__);    \
            return;                          \
        }                                    \
    } while (0)

#define CHECK_AND_BREAK_LOG(cond, fmt, ...) \
    if (1) {                                \
        if (!(cond)) {                      \
            VPE_LOGE(fmt, ##__VA_ARGS__);   \
            break;                          \
        }                                   \
    } else                                  \
        void(0)

#define CHECK_AND_CONTINUE_LOG(cond, fmt, ...) \
    if (1) {                                   \
        if (!(cond)) {                         \
            VPE_LOGE(fmt, ##__VA_ARGS__);      \
            continue;                          \
        }                                      \
    } else                                     \
        void(0)

#define CHECK_AND_LOG(cond, fmt, ...) \
    do {                                     \
        if (!(cond)) {                       \
            VPE_LOGE(fmt, ##__VA_ARGS__);    \
        }                                    \
    } while (0)

#define VPE_LOG_PRINT_COLOR_SPACE_CAPBILITY(CSDesc, pixelFormat)                                           \
    if (VPELogFlag::VERBOSE_LOG) {                                                                         \
        VPE_LOGD("Cap: (pri=%{public}3d,trans=%{public}3d,mat=%{public}3d,range=%{public}3d,pixel=%{public}3d)", \
            (CSDesc).primaries, (CSDesc).transfunc, (CSDesc).matrix, (CSDesc).range, (pixelFormat));             \
    }

#define VPE_LOG_PRINT_METADATA_GEN_CAPBILITY(CSDesc, pixelFormat, algoType)                                    \
    if (VPELogFlag::VERBOSE_LOG) {                                                                             \
        VPE_LOGI("Cap: (%{public}3d,%{public}3d,%{public}3d,%{public}3d,%{public}3d, %{public}3d)",            \
            (CSDesc).primaries, (CSDesc).transfunc, (CSDesc).matrix, (CSDesc).range, (pixelFormat), algoType); \
    }

#define POINTER_MASK 0x00FFFFFF
#define FAKE_POINTER(addr) (POINTER_MASK & reinterpret_cast<uintptr_t>(addr))

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
#endif // FRAMEWORK_DFX_LOG_H
