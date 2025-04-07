/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "vpe_sa_utils.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "vpe_log.h"

using namespace OHOS::Media::VideoProcessingEngine;

namespace {
constexpr uint32_t DEV_VALUE_SIZE = 256;
}

std::string VpeSaUtils::GetProcessName()
{
    std::string pid = std::to_string(getpid());
    std::string devPath = "/proc/" + pid + "/cmdline";
    pid = "pid:" + pid;
    int fd = open(devPath.c_str(), O_RDONLY);
    if (fd < 0) [[unlikely]] {
        VPE_LOGW("Failed to open %{public}s! %{public}s", devPath.c_str(), strerror(errno));
        return pid;
    }
    std::string name;
    char text[DEV_VALUE_SIZE]{};
    if (read(fd, text, sizeof(text) - 1) <= 0) [[unlikely]] {
        VPE_LOGW("Failed to read %{public}s! %{public}s", devPath.c_str(), strerror(errno));
        name = pid;
    } else [[likely]] {
        // Video app may have several process like below, and we use "tv.danmaku.bili" as the app name,
        // and trim the suffix after ":" such as ":ijkservice" and etc.
        // tv.danmaku.bili
        // tv.danmaku.bili:pushservice
        // tv.danmaku.bili:ijkservice
        // tv.danmaku.bili:download
        name = text;
        auto pos = name.find_first_of(':');
        if (pos != std::string::npos) {
            name = name.substr(0, pos);
        }
    }
    close(fd);
    return name;
}
