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
#define USE_OPENCL_WRAPPER
#ifdef USE_OPENCL_WRAPPER

#include "image_opencl_wrapper.h"
#include <algorithm>
#include <dlfcn.h>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <iostream>
#include "vpe_log.h"

namespace OHOS {
namespace Media {
// default opencl library path
static const std::vector<std::string> g_opencl_library_paths = {
#if defined(__APPLE__) || defined(__MACOSX)
    "libOpenCL.so", "/System/Library/Frameworks/OpenCL.framework/OpenCL"
#else
    "/vendor/lib64/chipsetsdk/libGLES_mali.so",
    "/system/lib64/libGLES_mali.so",
    "libGLES_mali.so",
    "/vendor/lib64/chipsetsdk/libhvgr_v200.so",
    "/vendor/lib64/chipsetsdk/libEGL_impl.so",
#endif
};

static std::mutex g_initMutex;
static bool g_isInit = false;
static bool g_loadSuccess = false;
static void *g_handle{nullptr};
// load default library path
    static bool LoadLibraryFromPath(const std::string &libraryPath, void **handlePtr)
    {
        CHECK_AND_RETURN_RET_LOG(handlePtr != nullptr, false, "handlePtr null!");

        char path[PATH_MAX] = "";
        if (realpath(libraryPath.c_str(), path) == nullptr) {
            return false;
        }
        if (strcmp(path, "") == 0) {
            return false;
        }
        *handlePtr = dlopen(path, RTLD_NOW | RTLD_LOCAL);
        CHECK_AND_RETURN_RET_LOG(*handlePtr != nullptr, false, "*handlePtr null!");

        // load function ptr use dlopen and dlsym.
        clImportMemory = reinterpret_cast<clImportMemoryFunc>(dlsym(*handlePtr, "clImportMemoryARM"));
        CHECK_AND_RETURN_RET_LOG(clImportMemory != nullptr, false, "clImportMemory null!");

        return true;
    }

    bool LoadOpenCLLibrary(void **handlePtr)
    {
        CHECK_AND_RETURN_RET_LOG(handlePtr != nullptr, false, "handlePtr null!");
        auto it =
                std::find_if(g_opencl_library_paths.begin(), g_opencl_library_paths.end(),
                             [&](const std::string &lib_path) {
                                 return OHOS::Media::LoadLibraryFromPath(lib_path, handlePtr);
                             });
        CHECK_AND_RETURN_RET_LOG(it == g_opencl_library_paths.end(), true, "LoadOpenCLLibrary true!");
        return false;
    }

    bool InitOpenCL()
    {
        std::lock_guard <std::mutex> lock(g_initMutex);
        CHECK_AND_RETURN_RET_LOG(!g_isInit, g_loadSuccess, "InitOpenCL g_loadSuccess!");
        g_isInit = true;
        g_loadSuccess = LoadOpenCLLibrary(&g_handle);
        return g_loadSuccess;
    }

    void DeInitOpenCL()
    {
        std::lock_guard <std::mutex> lock(g_initMutex);
        if (g_handle != nullptr) {
            dlclose(g_handle);
            g_handle = nullptr;
        }
    }

    clImportMemoryFunc clImportMemory = nullptr;

}  // namespace Media
}  // namespace OHOS

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
// clImportMemory wrapper, use OpenCLWrapper function.
cl_mem ClImportMemory(ClImportMemoryParam clImportMemoryParam)
{
    auto status = OHOS::Media::InitOpenCL();
    CHECK_AND_RETURN_RET_LOG(status, nullptr, "clImportMemory InitOpenCL null!");
    auto func = OHOS::Media::clImportMemory;
    MS_ASSERT(func != nullptr);
    return func(clImportMemoryParam.context, clImportMemoryParam.flags, clImportMemoryParam.properties,
        clImportMemoryParam.fd, clImportMemoryParam.size, clImportMemoryParam.errcode_ret);
}
void ClDeInitOpenCL()
{
    OHOS::Media::DeInitOpenCL();
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif  // USE_OPENCL_WRAPPER
