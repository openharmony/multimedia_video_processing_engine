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

#ifndef IMAGE_OPENCL_WRAPPER_H_
#define IMAGE_OPENCL_WRAPPER_H_

#include <memory>
#include <string>
#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>
#include <CL/cl_ext.h>

#define MS_ASSERT(f) ((void)0)

namespace OHOS {
// This is a opencl function wrapper.
namespace Media {

using clImportMemoryFunc = cl_mem (*)(cl_context, cl_mem_flags, const cl_import_properties_arm *, void *, size_t,
    cl_int *);

extern clImportMemoryFunc clImportMemory;
}  // namespace Media
}  // namespace OHOS
struct ClImportMemoryParam {
    cl_context context;
    cl_mem_flags flags;
    const cl_import_properties_arm* properties;
    void* fd;
    size_t size;
    cl_int* errcode_ret;
};
namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
extern  cl_mem  ClImportMemory(ClImportMemoryParam clImportMemoryParam);
extern  void  ClDeInitOpenCL();
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif  // IMAGE_OPENCL_WRAPPER_H_
