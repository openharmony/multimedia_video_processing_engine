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

#ifndef INTERFACES_INNER_API_VPE_DETAIL_ENHANCER_VIDEO_COMMON_H
#define INTERFACES_INNER_API_VPE_DETAIL_ENHANCER_VIDEO_COMMON_H

#include "algorithm_video_common.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
using DetailEnhBufferFlag = VpeBufferFlag;
#define DETAIL_ENH_BUFFER_FLAG_NONE VPE_BUFFER_FLAG_NONE
#define DETAIL_ENH_BUFFER_FLAG_EOS  VPE_BUFFER_FLAG_EOS

class __attribute__((visibility("default"))) DetailEnhancerVideoCallback : public VpeVideoCallback {
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // INTERFACES_INNER_API_VPE_DETAIL_ENHANCER_VIDEO_COMMON_H
