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
#ifndef IMAGE_OPENCL_SETUP_H
#define IMAGE_OPENCL_SETUP_H
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <CL/opencl.h>
#include <CL/cl_egl.h>

struct ClContext {
    // Platform and device
    cl_platform_id *platforms{};
    cl_uint numPlatforms{};   // Total platforms
    cl_uint idxPlatforms{};   // Current platform
    cl_device_id **devices{}; // Devices[idxPlatforms] is link list header of platform[idxPlatforms]
    cl_uint *numDevices{};    // each numDevices[i] record device number of each platform.
    cl_uint *idxDevices{};    // Current device under current platform

    // CL Context and Command queue
    cl_context context{};
    cl_command_queue cmdQueueGPU{};
    cl_command_queue cmdQueueCPU{};
    cl_command_queue cmdQueueDSP{};
    cl_command_queue cmdQueueOther{};
};
#define INFO_BUFFER_LENGTH 128

extern "C" {
namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
int SetupOpencl(void **pHandle, const char *vendorName, char *deviceName);
void CleanOpencl(ClContext *pCtx);
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
}

#endif // IMAGE_OPENCL_SETUP_H
