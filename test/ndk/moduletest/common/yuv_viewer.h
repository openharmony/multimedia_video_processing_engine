/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef VPE_TEST_YUV_VIEWER_H
#define VPE_TEST_YUV_VIEWER_H
#include "wm/window.h"
#include "surface/native_image.h"
#include "surface/native_buffer.h"
namespace OHOS {
class YuvViewer {
public:
    ~YuvViewer();
    OHNativeWindow *CreateWindow(uint32_t width, uint32_t height,
        OH_NativeBuffer_Format pix_fmt, OH_NativeBuffer_ColorSpace color_space, bool isHDRVivid);

private:
    sptr<Rosen::Window> rosenWindow_;
    OHNativeWindow *window_ = nullptr;
    
};
}
#endif // VPE_TEST_YUV_VIEWER_H