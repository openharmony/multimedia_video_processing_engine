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

#include "yuv_viewer.h"
#include <sys/mman.h>
#include <unistd.h>
#include "refbase.h"
#include "surface/window.h"
#include "surface.h"
#include "ui/rs_surface_node.h"
#include "window_option.h"
using namespace OHOS;

YuvViewer::~YuvViewer()
{
    if (window_) {
        OH_NativeWindow_DestroyNativeWindow(window_);
        window_ = nullptr;
    }
    if (rosenWindow_) {
        rosenWindow_->Destroy();
        rosenWindow_ = nullptr;
    }
}

OHNativeWindow *YuvViewer::CreateWindow(uint32_t width, uint32_t height,
    OH_NativeBuffer_Format pix_fmt, OH_NativeBuffer_ColorSpace color_space, bool isHDRVivid)
{
    sptr<OHOS::Surface> surfaceProducer;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_FLOAT);
    option->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FULLSCREEN);
    rosenWindow_ = Rosen::Window::Create("VpeDemo", option);
    if (rosenWindow_ == nullptr) {
        std::cout << "rosen window create failed" << std::endl;
        return nullptr;
    }
    rosenWindow_->SetTurnScreenOn(!rosenWindow_->IsTurnScreenOn());
    rosenWindow_->SetKeepScreenOn(true);
    rosenWindow_->Show();
    surfaceProducer = rosenWindow_->GetSurfaceNode()->GetSurface();

    window_ = CreateNativeWindowFromSurface(&surfaceProducer);

    (void)OH_NativeWindow_NativeWindowHandleOpt(window_, SET_TRANSFORM, 1); // 1: rotation 90°
    (void)OH_NativeWindow_NativeWindowHandleOpt(window_, SET_BUFFER_GEOMETRY,
        width, height);
    (void)OH_NativeWindow_NativeWindowHandleOpt(window_, SET_USAGE,
        NATIVEBUFFER_USAGE_CPU_READ | NATIVEBUFFER_USAGE_CPU_WRITE |
        NATIVEBUFFER_USAGE_MEM_DMA | NATIVEBUFFER_USAGE_HW_RENDER);
    (void)OH_NativeWindow_NativeWindowHandleOpt(window_, SET_FORMAT, pix_fmt);
    return window_;
}