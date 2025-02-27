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
#ifndef VPE_UTIL_DEFINE_H
#define VPE_UTIL_DEFINE_H

#include <string>
#include <cstdio>
#include "graphic_common_c.h"
#include "algorithm_common.h"
#include "algorithm_errors.h"
#include "v1_0/cm_color_space.h"
#include "vpe_context.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
struct ParameterBase {
    int modelType;
    int width;
    int height;
    int widthGainmap;
    int heightGainmap;
    int metadatInPut;
    CM_HDR_Metadata_Type inMetaType; // CM_METADATA_NONE, CM_VIDEO_HLG, CM_VIDEO_HDR10, CM_VIDEO_HDR_VIVID
    CM_ColorSpaceInfo inColspcInfo;
    GraphicPixelFormat inPixFmt; // nv12 10bit
    CM_HDR_Metadata_Type outMetaType;
    CM_ColorSpaceInfo outColspcInfo;
    GraphicPixelFormat outPixFmt;
    CM_HDR_Metadata_Type gainmapMetaType;
    CM_ColorSpaceInfo gainmapColspcInfo;
    GraphicPixelFormat gainmapPixFmt;
    std::string yuvFilePath;
    std::string metadataFilePath;
    VPEContext context;
};

void ReadFileYuv42016(std::string yuvFilePath, sptr<SurfaceBuffer> &buffer, int width, int height);
void ReadFileYuv42010ToNv1216(std::string yuvFilePath, sptr<SurfaceBuffer> &buffer, int width, int height);
void ReadInputFile(std::string yuvFilePath, sptr<SurfaceBuffer> &buffer, int frameSize);

void SaveMetadataFromSurBuffer(const sptr<SurfaceBuffer> &input, int frame, const std::string &metadataBin);
void WriteOutFile(int frame, const std::string &outYuvFileName, const sptr<SurfaceBuffer> &output, int frameSize);
std::string GetPixFmtString(GraphicPixelFormat pixfmt);
std::string GetMetadataString(CM_HDR_Metadata_Type metaType);
std::string GetColspcString(CM_ColorSpaceInfo colorSpaceInfo);
std::string GetOutFileName(const std::string &baseName, const ParameterBase &param);
void SetMeatadata(sptr<SurfaceBuffer> &buffer, uint32_t value);
void SetMeatadata(sptr<SurfaceBuffer> &buffer, const CM_ColorSpaceInfo &colorspaceInfo);
void SetMeatadata(sptr<SurfaceBuffer> &buffer, int key, const float &data);
void SetMeatadata(sptr<SurfaceBuffer> &buffer, int key, const int &data);
void SetMeatadata(sptr<SurfaceBuffer> &buffer, std::unique_ptr<std::ifstream> &metadataFile);
sptr<SurfaceBuffer> CreateSurfaceBuffer(uint32_t pixelFormat, int32_t width, int32_t height);
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
#endif // VPE_UTIL_DEFINE_H