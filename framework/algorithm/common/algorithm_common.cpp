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

#include "algorithm_common.h"
#include "securec.h"
#include "vpe_log.h"
#include "vpe_context.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

int SetupOpengl(std::shared_ptr<OpenGLContext> &openglHandle)
{
    std::shared_ptr<OpenGLContext> openglContextPtr = std::make_shared<OpenGLContext>();
    openglContextPtr->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (openglContextPtr->display == EGL_NO_DISPLAY || eglGetError() != EGL_SUCCESS) {
        VPE_LOGE("Get display failed!");
        return VPE_ALGO_ERR_OPERATION_NOT_SUPPORTED;
    }
    EGLint major;
    EGLint minor;
    if (eglInitialize(openglContextPtr->display, &major, &minor) == EGL_FALSE || eglGetError() != EGL_SUCCESS) {
        VPE_LOGE("eglInitialize failed!");
        return VPE_ALGO_ERR_OPERATION_NOT_SUPPORTED;
    }
    openglHandle = openglContextPtr;
    return static_cast<int>(VPE_ALGO_ERR_OK);
}

VPEAlgoErrCode ColorSpaceDescription::Create(const sptr<SurfaceBuffer> &buffer, ColorSpaceDescription &desc)
{
    CHECK_AND_RETURN_RET_LOG(nullptr != buffer, VPE_ALGO_ERR_INVALID_VAL, "Get an invalid buffer");

    std::vector<uint8_t> vec;
    int32_t err = buffer->GetMetadata(ATTRKEY_COLORSPACE_INFO, vec);
    CHECK_AND_RETURN_RET_LOG(GSERROR_OK == err, VPE_ALGO_ERR_OPERATION_NOT_SUPPORTED,
        "Get metadata colorspace info failed, err: %{public}d", err);
    CHECK_AND_RETURN_RET_LOG(sizeof(desc.colorSpaceInfo) == (vec.size()), VPE_ALGO_ERR_INVALID_VAL,
        "memcpy_s failed, desc.colorSpaceInfo size != vec size;");
    errno_t ret = memcpy_s(&desc.colorSpaceInfo, sizeof(desc.colorSpaceInfo), vec.data(), vec.size());
    if (ret != EOK) {
        VPE_LOGE("memcpy_s failed, err = %d\n", ret);
        return VPE_ALGO_ERR_INVALID_VAL;
    }

    vec.clear();
    err = buffer->GetMetadata(ATTRKEY_HDR_METADATA_TYPE, vec);
    CHECK_AND_RETURN_RET_LOG(GSERROR_OK == err, VPE_ALGO_ERR_OPERATION_NOT_SUPPORTED,
        "Get hdr metadata type failed, err: %{public}d", err);
    CHECK_AND_RETURN_RET_LOG(sizeof(desc.metadataType) == (vec.size()), VPE_ALGO_ERR_INVALID_VAL,
        "memcpy_s failed, desc.metadataType size != vec size;");
    ret = memcpy_s(&desc.metadataType, sizeof(desc.metadataType), vec.data(), vec.size());
    if (ret != EOK) {
        VPE_LOGE("memcpy_s failed, err = %d\n", ret);
        return VPE_ALGO_ERR_INVALID_VAL;
    }

    return VPE_ALGO_ERR_OK;
}

uint32_t GetColorSpaceType(const CM_ColorSpaceInfo &colorSpaceInfo)
{
    CHECK_AND_LOG(colorSpaceInfo.primaries <= CM_ColorPrimaries::COLORPRIMARIES_ADOBERGB &&
        colorSpaceInfo.transfunc <= CM_TransFunc::TRANSFUNC_GAMMA2_4 &&
        colorSpaceInfo.matrix <= CM_Matrix::MATRIX_BT2100_ICTCP &&
        colorSpaceInfo.range <= CM_Range::RANGE_EXTEND,
        "Invalid colorSpaceInfo, primaries : %{public}d, transfunc: %{public}d, matrix: %{public}d, range: %{public}d",
        colorSpaceInfo.primaries, colorSpaceInfo.transfunc, colorSpaceInfo.matrix, colorSpaceInfo.range);
    return ((static_cast<unsigned int>(colorSpaceInfo.primaries) << COLORPRIMARIES_OFFSET) +
            (static_cast<unsigned int>(colorSpaceInfo.transfunc) << TRANSFUNC_OFFSET) +
            (static_cast<unsigned int>(colorSpaceInfo.matrix) << MATRIX_OFFSET) +
            (static_cast<unsigned int>(colorSpaceInfo.range) << RANGE_OFFSET));
}

CM_ColorSpaceInfo GetColorSpaceInfo(const uint32_t colorSpaceType)
{
    CM_ColorSpaceInfo info;
    info.primaries = static_cast<CM_ColorPrimaries>((colorSpaceType & COLORPRIMARIES_MASK) >> COLORPRIMARIES_OFFSET);
    info.transfunc = static_cast<CM_TransFunc>((colorSpaceType & TRANSFUNC_MASK) >> TRANSFUNC_OFFSET);
    info.matrix = static_cast<CM_Matrix>((colorSpaceType & MATRIX_MASK) >> MATRIX_OFFSET);
    info.range = static_cast<CM_Range>((colorSpaceType & RANGE_MASK) >> RANGE_OFFSET);
    return info;
}

bool ColorSpaceDescription::operator < (const ColorSpaceDescription &desc) const
{
    uint32_t thisColorSpaceType = GetColorSpaceType(colorSpaceInfo);
    uint32_t descColorSpaceType = GetColorSpaceType(desc.colorSpaceInfo);
    return (thisColorSpaceType < descColorSpaceType ||
        ((thisColorSpaceType == descColorSpaceType) && (metadataType < desc.metadataType)));
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
