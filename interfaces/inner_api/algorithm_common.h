/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_INNER_API_ALGORITHM_ALGORITHM_COMMON_H
#define INTERFACES_INNER_API_ALGORITHM_ALGORITHM_COMMON_H

#include <optional>
#include <stdint.h>
#include "refbase.h"
#include "surface_buffer.h"
#include "v1_0/buffer_handle_meta_key_type.h"
#include "v1_0/cm_color_space.h"
#include "v1_0/hdr_static_metadata.h"
#include "algorithm_errors.h"

typedef struct OpenGLContext OpenGLContext;
typedef struct ClContext ClContext;
extern "C" int SetupOpencl(void **pHandle, const char *vendorName, char *deviceName);

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
constexpr uint8_t COLORPRIMARIES_OFFSET = 0;
constexpr uint8_t TRANSFUNC_OFFSET = 8;
constexpr uint8_t MATRIX_OFFSET = 16;
constexpr uint8_t RANGE_OFFSET = 21;
constexpr uint32_t COLORPRIMARIES_MASK = 0x000000FF;
constexpr uint32_t TRANSFUNC_MASK = 0x0000FF00;
constexpr uint32_t MATRIX_MASK = 0x001F0000;
constexpr uint32_t RANGE_MASK = 0xFFE00000;
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

typedef struct {
    unsigned int systemStartCode; // 表示系统版本号
    // minimum_maxrgb_pq,表示显示内容的最小亮度，pq域，范围从0~4095
    unsigned int minimumMaxRgbPq;
    unsigned int averageMaxRgbPq; // average_maxrgb_pq,表示显示内容的平均亮度，pq域，范围从0.0~4095
    // variance_maxrgb_pq,表示显示内容的变化范围，pq域，范围从0.0~4095
    unsigned int varianceMaxRgbPq;
    unsigned int maximumMaxRgbPq; // maximum_maxrgb_pq,表示显示内容的最大亮度，pq域，范围从0.0~4095
    // tone_mapping_enable_mode_flag，取值为0或1，若为0，则不传曲线参数，否则传输参数
    unsigned int toneMappingMode;
    // tone_mapping_para_enable_num，表示当前色调映射参数组的数目减1，
    // 为0时色调映射参数组的数目为1，为1时色调映射参数组的数目为2
    unsigned int toneMappingParamNum;
    // targeted_system_display_maximum_luminancePq,参考目标显示器最高亮度,范围从0.0~4095,
    // 数组长度为2说明有2组参数，tone_mapping_param_num
    unsigned int targetedSystemDisplayMaximumLuminance[2];
    // base_enable_flag,基础曲线标识，取值为0或1，为0则不传输基础曲线参数，为1则传输参数
    unsigned int baseFlag[4];
    unsigned int baseParamMp[2]; // 范围0~16383
    unsigned int baseParamMm[2]; // 范围0~63
    unsigned int baseParamMa[2]; // 范围0~1023
    unsigned int baseParamMb[2]; // 范围0~1023
    unsigned int baseParamMn[2]; // 范围0~63
    unsigned int baseParamK1[2]; // 分小于等于1 和 大于1两种情况
    unsigned int baseParamK2[2]; // 分小于等于1 和 大于1两种情况
    unsigned int baseParamK3[2]; // 分小于等于1 和 1~2 和 大于2两种情况
    // base_param_delta_enable_mode,标识当前基础曲线映射参数的调整系数模式
    unsigned int baseParamDeltaMode[2];
    // base_param_enable_delta,标识当前基础曲线映射参数的调整系数值，范围0~127
    unsigned int baseParamDelta[2];
    // 3Spline_enable_flag,二值变量，为1时标识传输三次样条参数，为0时不传
    unsigned int threeSplineFlag[2];
    unsigned int threeSplineNum[2]; // 3Spline_enable_num,标识三次样条区间数量，取值为0和1
    // 3Spline_TH_enable_mode,标识色调映射的三次样条模式，范围为0~3，
    // 这里数组长度4是由于P3Spline_num
    unsigned int threeSplineThMode[2][4];
    // 3Spline_TH_enable_MB,指示色调映射的三次样条区间参数的斜率和暗区偏移量
    unsigned int threeSplineThMb[2][4];
    // 3Spline_TH_enable, 指示色调映射的三次样条区间参数，范围0~4095，第三维度表示三次样
    // 条区间参数（0~4095）、三次样条区间1偏移量（0~1023）和三次样条区间2偏移量（0~1023）
    unsigned int threeSplineTh[2][4][3];
    // 3Spline_enable_Strength,指示色调映射的三次样条区间的修正幅度参数，范围0~255
    unsigned int threeSplineStrength[2][4];
    unsigned int colorSaturationMappingFlag; // 对应标准中color_saturation_mapping_enable_flag
    unsigned int colorSaturationNum;         // 对应标准中color_saturation_enable_num
    unsigned int colorSaturationGain[16];    // 对应标准中color_saturation_enable_gain
} HdrVividMetadataV1;

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
constexpr int MAX_IMAGE_SIZE = 8880;
using namespace HDI::Display::Graphic::Common::V1_0;

struct ColorSpaceDescription {
    CM_ColorSpaceInfo colorSpaceInfo;
    CM_HDR_Metadata_Type metadataType;

    static VPEAlgoErrCode Create(const sptr<SurfaceBuffer> &buffer, ColorSpaceDescription &desc);
    bool operator < (const ColorSpaceDescription &desc) const;
};

enum class RenderIntent {
    RENDER_INTENT_PERCEPTUAL,            // 感性意图
    RENDER_INTENT_RELATIVE_COLORIMETRIC, // 相对比色意图
    RENDER_INTENT_ABSOLUTE_COLORIMETRIC, // 绝对比色渲染意图
    RENDER_INTENT_SATURATION             // 饱和度意图
};

enum class MetadataGeneratorAlgoType {
    META_GEN_ALGO_TYPE_IMAGE, // image
    META_GEN_ALGO_TYPE_VIDEO  // video
};

enum class VideoMetadataGeneratorStyle {
    META_GEN_BRIGHT_STYLE = 0,
    META_GEN_CONTRAST_STYLE = 1
};

struct ColorSpaceConverterParameter {
    RenderIntent renderIntent; // 渲染意图
    std::optional<double> sdrUIBrightnessRatio {
        std::nullopt
    }; // 为当输入sdr ui，需要配置值，亮度打折，sdr亮度调节系数。范围[1, 6].
    bool isVideo = false; // 当为视频抽帧资源时，配置为true不生成元数据。图片默认均生成元数据
};

struct ColorSpaceConverterDisplayParameter {
    ColorSpaceDescription inputColorSpace;    // 色彩空间信息
    ColorSpaceDescription outputColorSpace;   // 色彩空间信息
    std::vector<uint8_t> staticMetadata;      // 静态元数据
    std::vector<uint8_t> dynamicMetadata;     // 动态元数据
    std::vector<float> layerLinearMatrix;     // 线性域转换矩阵，作用在原色域上
    float tmoNits;                            // TMO目标亮度
    float currentDisplayNits;                 // 屏幕当前亮度，和tmoNits相除得到sdr亮度打折比
    float sdrNits;                            // SDR亮度
    int32_t width;                            // 宽度
    int32_t height;                           // 高度
    bool disableHdrFloatHeadRoom;             // 不使用HDRfp16方案做额外提亮
    std::vector<uint8_t> adaptiveFOVMetadata; // 缩放重适应元数据
};

struct MetadataGeneratorParameter {
    MetadataGeneratorAlgoType algoType = MetadataGeneratorAlgoType::META_GEN_ALGO_TYPE_IMAGE;
    bool isOldHdrVivid = false;
    float avgGainmapGray = 0.0;
    VideoMetadataGeneratorStyle styleType = VideoMetadataGeneratorStyle::META_GEN_CONTRAST_STYLE;
};

uint32_t GetColorSpaceType(const CM_ColorSpaceInfo &colorSpaceInfo);
CM_ColorSpaceInfo GetColorSpaceInfo(const uint32_t colorSpaceType);
int SetupOpengl(std::shared_ptr<OpenGLContext> &openglHandle);
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
#endif // INTERFACES_INNER_API_ALGORITHM_ALGORITHM_COMMON_H
