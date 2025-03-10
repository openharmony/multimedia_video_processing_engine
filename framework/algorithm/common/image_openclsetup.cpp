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
#include "image_openclsetup.h"
#include "image_opencl_wrapper.h"
#include <cstdio>
#include <cstring>
#include "securec.h"
#include "vpe_log.h"

// OpenCL Init
// Enable MY_DEBUG to support printf in kernel.
#define MY_TUNING
#ifdef MY_DEBUG
static void printf_callback(const char *buffer, size_t len, size_t complete, void *usr_data)
{
    VPE_LOGI("output length: %{public}d\noutput data: \n%{public}s\n", (cl_uint)len, buffer);
}
#endif

static cl_int InitContext(ClContext *pCtx)
{
    cl_int status = CL_SUCCESS;
    // Get platform number
    pCtx->numPlatforms = 0;
    status = clGetPlatformIDs(0, nullptr, &pCtx->numPlatforms);  // ~{DZ4fP9B)~}
    CHECK_AND_RETURN_RET_LOG(!((status != CL_SUCCESS) || (pCtx->numPlatforms == 0)), status,
        "[GPU]: Fail to get platform IDs1. (clGetPlatformIDs).");
    VPE_LOGI("(clGetPlatformIDs). status = %{public}d, numPlatforms = %{public}d\n", status, pCtx->numPlatforms);

    // Allocate tables for platform IDs, devices list header, devices number and device index
    cl_uint numValue = pCtx->numPlatforms;
    pCtx->platforms = reinterpret_cast<cl_platform_id *>(malloc(pCtx->numPlatforms * sizeof(cl_platform_id)));
    CHECK_AND_RETURN_RET_LOG(pCtx->platforms != nullptr, CL_INVALID_VALUE, "pCtx platforms null!");
    CHECK_AND_LOG(memset_sp(pCtx->platforms, numValue * sizeof(cl_platform_id), 0,
        numValue * sizeof(cl_platform_id)) == 0, "memsetFail");
    pCtx->numDevices = reinterpret_cast<cl_uint *>(malloc(pCtx->numPlatforms * sizeof(cl_uint)));
    CHECK_AND_RETURN_RET_LOG(pCtx->numDevices != nullptr, CL_INVALID_VALUE, "pCtx numDevices null!");
    CHECK_AND_LOG(memset_sp(pCtx->numDevices, numValue * sizeof(cl_uint), 0, numValue * sizeof(cl_uint)) == 0,
        "memsetFail");
    pCtx->idxDevices = reinterpret_cast<cl_uint *>(malloc(pCtx->numPlatforms * sizeof(cl_uint)));
    CHECK_AND_RETURN_RET_LOG(pCtx->idxDevices != nullptr, CL_INVALID_VALUE, "pCtx idxDevices null!");
    CHECK_AND_LOG(memset_sp(pCtx->idxDevices, numValue * sizeof(cl_uint), 0, numValue * sizeof(cl_uint)) == 0,
        "memsetFail");
    pCtx->devices = reinterpret_cast<cl_device_id **>(malloc(pCtx->numPlatforms * sizeof(cl_device_id *)));
    CHECK_AND_RETURN_RET_LOG(pCtx->devices != nullptr, CL_INVALID_VALUE, "pCtx devices null!");
    CHECK_AND_LOG(memset_sp(pCtx->devices, numValue * sizeof(cl_device_id *), 0,
        numValue * sizeof(cl_device_id *)) == 0, "memsetFail");
    for (cl_uint i = 0; i < pCtx->numPlatforms; i++) {
        pCtx->devices[i] = nullptr;
    }
    return status;
}

static cl_int GetDevice(ClContext *pCtx)
{
    cl_int status = CL_SUCCESS;
    // Get platforms
    status = clGetPlatformIDs(pCtx->numPlatforms, pCtx->platforms, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == CL_SUCCESS, status,
        "[GPU]: Fail to get platform IDs2. (clGetPlatformIDs).");

    // Get all devices for each of platform
    for (cl_uint i = 0; i < pCtx->numPlatforms; i++) {
        cl_uint numDevices = 0;
        status = clGetDeviceIDs(pCtx->platforms[i], CL_DEVICE_TYPE_ALL, 0, nullptr, &numDevices);
        CHECK_AND_RETURN_RET_LOG(status == CL_SUCCESS, status,
            "[GPU]: Fail to get device IDs3. (clGetDeviceIDs).");

        pCtx->numDevices[i] = numDevices;
        pCtx->idxDevices[i] = (cl_uint)(-1);

        if (numDevices == 0) {
            pCtx->devices[i] = nullptr;
            continue;
        }

        pCtx->devices[i] = reinterpret_cast<cl_device_id *>(malloc(numDevices * sizeof(cl_device_id)));
        CHECK_AND_RETURN_RET_LOG(pCtx->devices[i] != nullptr, -1, "pCtx devices[i] null!");

        status = clGetDeviceIDs(pCtx->platforms[i], CL_DEVICE_TYPE_ALL, numDevices, pCtx->devices[i], nullptr);
        CHECK_AND_RETURN_RET_LOG(status == CL_SUCCESS, status,
            "[GPU]: Fail to get device IDs4. (clGetDeviceIDs).");
    }
    return status;
}

static cl_int ChoosePlatform(ClContext *pCtx, cl_uchar infoBuf[], size_t lenInfoBuf, const char *vendorName)
{
    cl_int status = CL_SUCCESS;
    CHECK_AND_RETURN_RET_LOG(lenInfoBuf <= INFO_BUFFER_LENGTH, CL_DEVICE_NOT_FOUND, "lenInfoBuf>128!");
    // Choose vendorName platform to work
    pCtx->idxPlatforms = (cl_uint)(-1);
    if (pCtx->numPlatforms == 1) {
        pCtx->idxPlatforms = 0;
    } else {
        for (cl_uint i = 0; i < pCtx->numPlatforms; i++) {
            // Get vendor name
            status = clGetPlatformInfo(pCtx->platforms[i], CL_PLATFORM_VENDOR, INFO_BUFFER_LENGTH, infoBuf, nullptr);
            CHECK_AND_RETURN_RET_LOG(status == CL_SUCCESS, status,
                "[GPU]: Fail to get platform Info. (clGetPlatformInfo). ");
            if (!strcmp(static_cast<const char *>(static_cast<void *>(infoBuf)), vendorName)) {
                pCtx->idxPlatforms = i;
                break;
            }
        }
    }

    // exit to host program if no vendorName is found.
    if ((cl_uint)(-1) == pCtx->idxPlatforms) {
        VPE_LOGE("[GPU]: There is no  platform to use. exit.\n");
        status = CL_DEVICE_NOT_AVAILABLE;
        return status;
    }
    return status;
}

static cl_int ChooseDevice(ClContext *pCtx, cl_device_id *targetDevice, cl_uchar infoBuf[], size_t lenInfoBuf,
    char *deviceName)
{
    cl_int status = CL_SUCCESS;
    CHECK_AND_RETURN_RET_LOG(lenInfoBuf <= INFO_BUFFER_LENGTH, CL_DEVICE_NOT_FOUND, "lenInfoBuf>128!");
    // Choose GPU device to work
    pCtx->idxDevices[pCtx->idxPlatforms] = (cl_uint)(-1);
    for (cl_uint i = 0; i < pCtx->numDevices[pCtx->idxPlatforms]; i++) {
        status =
            clGetDeviceInfo(pCtx->devices[pCtx->idxPlatforms][i], CL_DEVICE_TYPE, INFO_BUFFER_LENGTH, infoBuf, nullptr);
        CHECK_AND_RETURN_RET_LOG(status == CL_SUCCESS, status,
            "[GPU]: Fail to get device Info. (clGetDeviceInfo).");
        {
            *targetDevice = pCtx->devices[pCtx->idxPlatforms][i];
            pCtx->idxDevices[pCtx->idxPlatforms] = i;
            break;
        }
    }
    if (deviceName != nullptr) {
        constexpr int deviceLength = 32;
        status = clGetDeviceInfo(pCtx->devices[pCtx->idxPlatforms][pCtx->idxDevices[pCtx->idxPlatforms]],
            CL_DEVICE_NAME,
            deviceLength,
            deviceName,
            nullptr);
        CHECK_AND_RETURN_RET_LOG(status == CL_SUCCESS, status,
            "[GPU]: Fail to get device Info. (clGetDeviceInfo).");
        deviceName[strlen(deviceName)] = '\0';
    }
    // exit to host program if no GPU device of vendorName is found.
    CHECK_AND_RETURN_RET_LOG(!((cl_uint)(-1) == pCtx->idxDevices[pCtx->idxPlatforms]), CL_DEVICE_NOT_FOUND,
        "[GPU]: There is no  GPU to use. exit.");
    return status;
}

static cl_int CreateContext(ClContext *pCtx, cl_device_id targetDevice)
{
    cl_int status = CL_SUCCESS;
    pCtx->context = clCreateContext(
        nullptr, pCtx->numDevices[pCtx->idxPlatforms], pCtx->devices[pCtx->idxPlatforms], nullptr, nullptr, &status);

    CHECK_AND_RETURN_RET_LOG(((status == CL_SUCCESS) || (pCtx->context == nullptr)), status,
        "Error: Fail to create CL context.");

    const cl_bitfield props[] = {CL_QUEUE_PRIORITY_KHR, CL_QUEUE_PRIORITY_LOW_KHR, 0};
    pCtx->cmdQueueGPU = clCreateCommandQueueWithProperties(pCtx->context, targetDevice, props, &status);
    CHECK_AND_RETURN_RET_LOG(!((status != CL_SUCCESS) || (pCtx->cmdQueueGPU == nullptr)), status,
        "Error: Fail to create CL cmdQueueGPU.");

    return status;
}

// These code need to be modified in the future if there are more platforms and devices
// and need other platform/device select policies.
static cl_int InitOpenCL(ClContext *pCtx, const char *vendorName, char *deviceName)
{
    cl_uchar infoBuf[INFO_BUFFER_LENGTH] = {0};
    cl_int status;
    cl_device_id targetDevice = (cl_device_id)0;

    status = InitContext(pCtx);
    CHECK_AND_RETURN_RET_LOG(status == CL_SUCCESS, status, "InitContext fail!");

    status = GetDevice(pCtx);
    CHECK_AND_RETURN_RET_LOG(status == CL_SUCCESS, status, "GetDevice fail!");

    status = ChoosePlatform(pCtx, infoBuf, sizeof(infoBuf), vendorName);
    CHECK_AND_RETURN_RET_LOG(status == CL_SUCCESS, status, "ChoosePlatform fail!");

    status = ChooseDevice(pCtx, &targetDevice, infoBuf, sizeof(infoBuf), deviceName);
    CHECK_AND_RETURN_RET_LOG(status == CL_SUCCESS, status, "ChooseDevice fail!");

    // Create context
    status = CreateContext(pCtx, targetDevice);
    CHECK_AND_RETURN_RET_LOG(status == CL_SUCCESS, status, "CreateContext fail!");

    return status;
}

static void CleanDevice(ClContext *pCtx)
{
    if (pCtx->cmdQueueGPU) {
        // Get cmd queue ref count to ensure no memory leak.
        cl_uchar infoBuf[INFO_BUFFER_LENGTH] = {0};
        int status =
            clGetCommandQueueInfo(pCtx->cmdQueueGPU, CL_QUEUE_REFERENCE_COUNT, sizeof(infoBuf), infoBuf, nullptr);
        CHECK_AND_LOG(status == CL_SUCCESS, "[GPU] clGetCommandQueueInfo Failed!");
        cl_int *infoBufTmp = reinterpret_cast<cl_int *>(infoBuf);
        VPE_LOGI("[GPU]: cmd Queue ref count before release it : %{public}d\n", *infoBufTmp);
        status = clReleaseCommandQueue(pCtx->cmdQueueGPU);
        CHECK_AND_LOG(status == CL_SUCCESS, "[GPU] clReleaseCommandQueue Failed!");
    }
    if (pCtx->context) {
        cl_uchar infoBuf[INFO_BUFFER_LENGTH] = {0};
        // Get context ref count to ensure no memory leak.
        int status = clGetContextInfo(pCtx->context, CL_CONTEXT_REFERENCE_COUNT, sizeof(infoBuf), infoBuf, nullptr);
        CHECK_AND_LOG(status == CL_SUCCESS, "[GPU] clGetContextInfo Failed!");
        cl_int *infoBufTmp = reinterpret_cast<cl_int *>(infoBuf);
        VPE_LOGI("[GPU]: Context ref count before release it : %{public}d\n", *infoBufTmp);
        status = clReleaseContext(pCtx->context);
        CHECK_AND_LOG(status == CL_SUCCESS, "[GPU] clReleaseContext Failed!");
    }
    if (pCtx->devices != nullptr) {
        for (cl_uint i = 0; i < pCtx->numPlatforms; i++) {
            if (pCtx->devices[i] != nullptr) {
                free(pCtx->devices[i]);
                pCtx->devices[i] = nullptr;
            }
        }
        free(pCtx->devices);
        pCtx->devices = nullptr;
    }
    if (pCtx->idxDevices != nullptr) {
        free(pCtx->idxDevices);
        pCtx->idxDevices = nullptr;
    }
    if (pCtx->numDevices != nullptr) {
        free(pCtx->numDevices);
        pCtx->numDevices = nullptr;
    }
    if (pCtx->platforms != nullptr) {
        free(pCtx->platforms);
        pCtx->platforms = nullptr;
    }
    if (pCtx != nullptr) {
        free(pCtx);
        pCtx = nullptr;
    }
}

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
void CleanOpencl(ClContext *pCtx)
{
    if (pCtx == nullptr) {
        VPE_LOGI("[GPU]: WARNING: ClneanOpencl input hadle is nullptr.\n");
    } else {
        // Release infrastructure like CmdQ/context. Show ref count to see if there is memory leak.
        // Free the whole clContext created by setupOpencl.
        CleanDevice(pCtx);
    }
    OHOS::Media::VideoProcessingEngine::ClDeInitOpenCL();
}

int SetupOpencl(void **pHandle, const char *vendorName, char *deviceName)
{
    ClContext *pCtx = reinterpret_cast<ClContext *>(calloc(1, sizeof(ClContext)));
    if (pCtx == nullptr) {
        VPE_LOGE("[GPU]: SetupOpencl FAIL to allocate OpenCL Foundation context.\n");
        *pHandle = nullptr;
        return CL_MEM_OBJECT_ALLOCATION_FAILURE;
    }

    // Init CL infrastructure
    cl_int status = InitOpenCL(pCtx, vendorName, deviceName);
    if (status != CL_SUCCESS) {
        CleanOpencl(pCtx);
        *pHandle = nullptr;
        return status;
    }

    *(reinterpret_cast<ClContext **>(pHandle)) = pCtx;
    return static_cast<int>(status);
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS