# VPE引擎（multimedia_video_processing_engine）

## 简介
VPE（Video Processing Engine）引擎是处理视频和图像数据的媒体引擎，包括细节增强、对比度增强、亮度增强、动态范围增强等基础能力，为转码、分享、显示后处理等提供色彩空间转换、缩放超分、动态元数据集生成等基础算法。

VPE引擎的主要结构如下图所示：

![VPE引擎架构图](./figures/videoProcessingEngine_architecture.png)

#### 各模块功能说明

<table>
<tr>
<td bgcolor=#F5F5F5> 层级 </td>
<td bgcolor=#F5F5F5> 模块 </td>
<td bgcolor=#F5F5F5> 功能描述 </td>
</tr>
<tr>
<td rowspan="7" colspan="1" > Interface </td>
<td> 视频色彩空间CAPI </td>
<td> 提供用于视频场景色彩空间转换相关接口 </td>
</tr>
<tr>
<td> 图片色彩空间CAPI </td>
<td> 提供用于图片场景色彩空间转换相关接口 </td>
</tr>
<tr>
<td> 视频细节增强CAPI </td>
<td> 提供视频超分算法、锐化算法的相关接口 </td>
</tr>
<tr>
<td> 图片细节增强CAPI </td>
<td> 提供图片超分算法、锐化算法的相关接口 </td>
</tr>
<tr>
<td> 视频动态元数据CAPI </td>
<td> 提供接口可用于视频内容动态元数据生成算法调用 </td>
</tr>
<tr>
<td> 图片动态元数据CAPI </td>
<td> 提供接口可用于图片内容动态元数据生成算法调用 </td>
</tr>
<tr>
<td>  图片细节增强TS 接口 </td>
<td> 提供图片超分算法、锐化算法的TS接口 </td>
</tr>
<tr>
<td rowspan="6" colspan="1" > 原子能力封装层 </td>
<td> 视频色彩空间原子能力 </td>
<td> 实现视频场景色彩空间转换软件通路调度及上下文管理，实现视频流过程控制 </td>
</tr>
<tr>
<td> 图片色彩空间原子能力 </td>
<td> 实现图片场景色彩空间转换软件通路调度 </td>
</tr>
<tr>
<td> 视频细节增强原子能力 </td>
<td> 实现视频场景清晰度增强及缩放算法软件通路调度及上下文管理，实现视频流过程控制 </td>
</tr>
<tr>
<td> 图片细节增强原子能力 </td>
<td> 实现图片场景清晰度增强及缩放算法软件通路调度 </td>
</tr>
<tr>
<td> 视频动态元数据原子能力 </td>
<td> 实现视频场景动态元数据生成软件通路调度 </td>
</tr>
<tr>
<td> 图片动态元数据原子能力 </td>
<td> 实现图片场景动态元数据生成软件通路调度 </td>
</tr>
<tr>
<td rowspan="6" colspan="1" > 算法插件层 </td>
<td> 视频色彩空间处理算法插件 </td>
<td> 实现视频色彩空间转换算法功能，具体包括SDR2SDR、HDR2SDR、HDR2HDR </td>
</tr>
<tr>
<td> 图片色彩空间算法插件 </td>
<td> 实现图片色彩空间转换算法功能，具体包括SDR2SDR、单层转双层、双层转单层 </td>
</tr>
<tr>
<td> 视频细节增强算法插件 </td>
<td> 实现视频缩放、画质增强算法 </td>
</tr>
<tr>
<td> 图片细节增强算法插件 </td>
<td> 实现图片缩放、画质增强算法 </td>
</tr>
<tr>
<td> 视频动态元数据算法插件 </td>
<td> 实现视频动态元数据生产算法 </td>
</tr>
<tr>
<td> 图片动态元数据算法插件 </td>
<td> 实现图片动态元数据生成算法 </td>
</tr>
<tr>
<td rowspan="3" colspan="1"> 插件管理 </td>
<td> 插件注册 </td>
<td> 提供系统开发者插件注册的功能 </td>
</tr>
<tr>
<td> 能力查询 </td>
<td> 应用开发者可通过能力查询功能确认给定插件是否在该设备或系统支持 </td>
</tr>
<tr>
<td> 插件调用 </td>
<td> 调用具体的插件能力完成相关算法功能 </td>
</tr>
<tr>
<td rowspan="2" colspan="1"> 服务管理 </td>
<td> 资源管理 </td>
<td> 进行资源调度及算法的上下文管理，例如视频内容前后帧的相关信息 </td>
</tr>
<tr>
<td> 进程管理 </td>
<td> 完成跨进程通信等功能 </td>
</tr>
</table>


| 依赖模块 | 功能描述 |
| :-- | :-- |
| graphic_graphic_surface | 提供视频surface支持 |
| graphic_graphic_2d | 提供图片surfacebuffer支持 |
| multimedia_media_foundation | 提供pixelmap支持 |
| multimedia_image_framework | 提供Format参数设置支持 |
| third_party_skia | 提供缩放算法 |

## 目录

仓目录结构如下：

```
/foundation/multimedia/video_processing_engine/
├── framework                                  # 框架代码
│   ├── algorithm                              # 算法框架
│       ├── aihdr_enhancer                     # 图像HDR增强算法框架
│       ├── aihdr_enhancer_video               # 视频HDR增强算法框架
│       ├── colorspace_converter               # 图像颜色空间转换算法框架
│       ├── colorspace_converter_display       # 图像颜色空间显示算法框架
│       ├── colorspace_converter_video         # 视频颜色空间转换算法框架
│       ├── common                             # 算法框架公共部分
│       ├── contrast_enhancer                  # 对比度增强算法框架
│       ├── detail_enhancer                    # 图像细节增强算法框架
│       ├── detail_enhancer_video              # 视频细节增强算法框架
│       ├── extension_manager                  # 插件管理
│       ├── extensions                         # 插件算法
│       ├── metadata_generator                 # 图像元数据生成算法框架
│       ├── metadata_generator_video           # 视频元数据生成算法框架
│       ├── video_variable_refresh_rate        # 视频可变帧率算法框架
│   ├── capi                                   # CAPI层
│       ├── image_processing                   # 图像CAPI
│       ├── video_processing                   # 视频CAPI
│   ├── dfx                                    # dfx代码
├── interfaces                                 # 接口层
│   ├── inner_api                              # 系统内部接口
│   ├── kits                                   # 应用接口
├── services                                   # 服务代码
├── sertestvices                               # 测试代码
```

## 编译构建

编译32位ARM系统VPE引擎
```
./build.sh --product-name {product_name} --ccache --build-target video_processing_engine
```

编译64位ARM系统VPE引擎
```
./build.sh --product-name {product_name} --ccache --target-cpu arm64 --build-target video_processing_engine
```

{product_name}为当前支持的平台，比如rk3568。

## 说明

### 使用说明
VPE引擎作为OpenHarmony的组件，提供系统的视频图像能力，包含视频处理算法框架，色彩空间转换、动态元数据生成以及细节增强等插件，支持开发者在插件中注册自定义算法，实现更多高阶图像和视频处理操作。

#### 应用开发者调用图像缩放示例
以下步骤详细说明了具体的开发流程。
1. 添加头文件。
    ```cpp
    #include <hilog/log.h>
    #include <multimedia/image_framework/image_pixel_map_mdk.h>
    #include <multimedia/image_framework/image/pixelmap_native.h>
    #include <multimedia/video_processing_engine/image_processing.h>
    #include <multimedia/video_processing_engine/image_processing_types.h>
    #include <multimedia/player_framework/native_avformat.h>
    #include <napi/native_api.h>
    ```
2. （可选）初始化环境。
  一般在进程内第一次使用时调用，可提前完成部分耗时操作。
    ```cpp
    ImageProcessing_ErrorCode ret =  OH_ImageProcessing_InitializeEnvironment();
    ```
3. 创建细节增强模块。
  应用可以通过图片处理引擎模块类型来创建图片细节增强模块。示例中的变量说明如下：
  imageProcessor：细节增强模块实例。
  IMAGE_PROCESSING_TYPE_DETAIL_ENHANCER：细节增强类型。
  预期返回值：IMAGE_PROCESSING_SUCCESS
    ```cpp
    // 创建图片细节增强模块实例
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor, IMAGE_PROCESSING_TYPE_DETAIL_ENHANCER);
    ```
4. （可选）配置细节增强档位，当前有高中低三档及NONE可选，若不配置则默认档位为LOW档。
    ```cpp
    // 创建format实例
    OH_AVFormat* parameter = OH_AVFormat_Create();
    // 指定档位
    OH_AVFormat_SetIntValue(parameter, IMAGE_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL,
        IMAGE_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH);
    // 配置参数
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_SetParameter(imageProcessor,parameter);
    ```
5. 启动细节增强处理。
    ```cpp
    // 启动细节增强处理
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_EnhanceDetail(imageProcessor, srcImage, dstImage);
    ```
6. 释放处理实例。
    ```cpp
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Destroy(imageProcessor);
    ```
7. 释放处理资源。
    ```cpp
    OH_ImageProcessing_DeinitializeEnvironment();
    ```
#### 应用开发者调用视频缩放示例
以下步骤详细说明了具体的开发流程。
1. 添加头文件。
    ```cpp
    #include <ace/xcomponent/native_interface_xcomponent.h>
    #include <multimedia/player_framework/native_avformat.h>
    #include <multimedia/video_processing_engine/video_processing.h>
    #include <multimedia/video_processing_engine/video_processing_types.h>
    #include <native_window/external_window.h>
    #include <native_buffer/native_buffer.h>
    ```
2. （可选）创建解码实例。
  细节增强模块的输入可以是来自系统解码的视频流，也可以由应用自行往window填充视频数据（例如：应用内部软解后直接将数据填充到window中）。若选择系统解码器对视频文件或视频流媒体进行处理，则可以创建解码实例来作为细节增强模块的输入。
    ```cpp
    // 创建Demuxer（媒体多路分解器）解析音视频信息(详见代码示例)
    OH_AVSource* source_ = OH_AVSource_CreateWithFD(inputFd, inputFileOffset, inputFileSize);
    OH_AVDemuxer* demuxer_ = OH_AVDemuxer_CreateWithSource(source_);
    auto sourceFormat = std::shared_ptr<OH_AVFormat>(OH_AVSource_GetSourceFormat(source_), OH_AVFormat_Destroy);
    // 创建视频解码器
    OH_AVCodec * decoder_ = OH_VideoDecoder_CreateByMime(videoCodecMime.c_str());
    // 配置视频信息
    OH_AVFormat *format = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_WIDTH, videoWidth);
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_HEIGHT, videoHeight);
    OH_AVFormat_SetDoubleValue(format, OH_MD_KEY_FRAME_RATE, frameRate);
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_PIXEL_FORMAT, pixelFormat);
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_ROTATION, rotation);
    int ret = OH_VideoDecoder_Configure(decoder_, format);
    OH_AVFormat_Destroy(format);
    // 配置回调，维护视频解码器buffer队列（详见代码示例）
    OH_VideoDecoder_RegisterCallback(decoder_,
            {SampleCallback::OnCodecError, SampleCallback::OnCodecFormatChange,
            SampleCallback::OnNeedInputBuffer, SampleCallback::OnNewOutputBuffer}, videoDecContext_);
    // 准备视频解码器
    int ret = OH_VideoDecoder_Prepare(decoder_);
    // 创建解码上下文
    videoDecContext_ = new CodecUserData;
    ```
3. （可选）初始化环境。
  一般在进程内第一次使用时调用，可提前完成部分耗时操作。
    ```cpp
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    ```
4. 创建细节增强模块。
  应用可以通过视频处理引擎模块类型来创建细节增强模块。示例中的变量说明如下：
  videoProcessor：细节增强模块实例。
  VIDEO_PROCESSING_TYPE_DETAIL_ENHANCER：细节增强类型。
  预期返回值：VIDEO_PROCESSING_SUCCESS
    ```cpp
    // 通过指定视频处理引擎类型创建细节增强模块实例
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_Create(&videoProcessor， VIDEO_PROCESSING_TYPE_DETAIL_ENHANCER);
    ```
5. 配置异步回调函数。
    ```cpp
    // 创建回调实例
    ret = OH_VideoProcessingCallback_Create(&callback);
    // 绑定回调函数
    OH_VideoProcessingCallback_BindOnError(callback, OnError);
    OH_VideoProcessingCallback_BindOnState(callback, OnState);
    OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, OnNewOutputBuffer);
    // 注册回调函数
    ret = OH_VideoProcessing_RegisterCallback(videoProcessor, callback, this);
    // 回调函数声明（其中userData会传递注册回调时传入的用户数据，如：this指针）
    void OnError(OH_VideoProcessing* videoProcessor, VideoProcessing_ErrorCode error, void* userData);
    void OnState(OH_VideoProcessing* videoProcessor, VideoProcessing_State state, void* userData);
    void OnNewOutputBuffer(OH_VideoProcessing* videoProcessor, uint32_t index, void* userData);
    ```
6. （可选）配置细节增强档位，当前有高中低三档及NONE可选，若不配置则默认档位为LOW档。
    ```cpp
    // 创建format实例
    OH_AVFormat* parameter = OH_AVFormat_Create();
    // 指定档位
    OH_AVFormat_SetIntValue(parameter, VIDEO_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL, VIDEO_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH);
    // 配置参数
    OH_VideoProcessing_SetParameter(videoProcessor, parameter);
    ```
7. 获取Surface。
    ```cpp
    //配置算法的输入
    ret = OH_VideoProcessing_GetSurface(videoProcessor, inputWindow);
    // 将解码器的输出与算法的输入进行绑定，解码器输出的window分辨率即为算法输入分辨率
    OH_VideoDecoder_SetSurface(decoder_,  inputWindow_);
    ```
8. 设置Surface（配置送显）。
    ```cpp
    // 配置算法的输出，配置的输出window的分辨率即为算法输出分辨率
    ret = OH_VideoProcessing_SetSurface(videoProcessor, outWindow);
    ```
9. 创建解码器输入输出线程。
    ```cpp
    std::unique_ptr<std::thread> videoDecInputThread_ = std::make_unique<std::thread>(&Player::VideoDecInputThread, this);
    std::unique_ptr<std::thread> videoDecOutputThread_ = std::make_unique<std::thread>(&Player::VideoDecOutputThread, this);
    ```
10. 启动细节增强处理。
    ```cpp
    // 启动解码
    int ret = OH_VideoDecoder_Start(decoder_);
    // 启动细节增强处理
    ret = OH_VideoProcessing_Start(videoProcessor);
    ```
11. 调用OH_VideoProcessing_Stop()停止细节增强。
    ```cpp
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_Stop(videoProcessor);
    ```
12. 释放处理实例。
    ```cpp
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_Destroy(videoProcessor)；
    VideoProcessing_ErrorCode ret = OH_VideoProcessingCallback_Destroy(callback);
    ```
13. 释放处理资源。
    ```cpp
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_DeinitializeEnvironment();
    ```

#### 系统开发者实现自定义算法插件注册示例
视频处理引擎提供色彩空间转换、动态元数据生成以及细节增强等算法插件注册接口，系统用户可以将自己的算法注册其中，丰富算法插件。视频处理引擎定义了各类算法的基类，具体如下：
| 功能 | 基类 | 头文件 |
| :-- | :-- | :-- |
| 视频色彩空间转换 | ColorSpaceConverterBase | colorspace_converter_base.h |
| 图片色彩空间转换 | ColorSpaceConverterBase | colorspace_converter_base.h |
| 视频细节增强 | DetailEnhancerBase | detail_enhancer_base.h |
| 图片细节增强 | DetailEnhancerBase | detail_enhancer_base.h |
| 视频动态元数据 | MetadataGeneratorBase | metadata_generator_base.h |
| 图片动态元数据 | MetadataGeneratorBase | metadata_generator_base.h |

<br>
以缩放算法为例，该插件算法在framework/algorithm/extensions/skia中已经实现，以下描述具体开发步骤

1. 在framework/algorithm/extensions目录下新建一个文件夹，例如skia，以及对应的cpp和h文件

2. 缩放算法头文件实现，具体可参考framework/algorithm/extensions/skia/include/skia_impl.h

```cpp
#ifndef SKIA_IMPL_H
#define SKIA_IMPL_H

#include "surface_buffer.h"
#include "include/core/SkYUVAPixmaps.h"

#include "algorithm_errors.h"
#include "detail_enhancer_base.h"
#include "detail_enhancer_capability.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
// 由基类DetailEnhancerBase创建一个自定义缩放算法类，例如Skia
// 其它算法对应关系如上表所示
class Skia : public DetailEnhancerBase {
public:
    Skia() = default;
    virtual ~Skia() = default;
    Skia(const Skia&) = delete;
    Skia& operator=(const Skia&) = delete;
    Skia(Skia&&) = delete;
    Skia& operator=(Skia&&) = delete;
    // 以下函数必须定义，基类虚函数必须实现
    // 定义Create函数，函数名可以自定义，这里以Create()为例，用于实例创建, 返回值必须是DetailEnhancerBase指针。
    static std::unique_ptr<DetailEnhancerBase> Create();
    // 定义能力构建函数，函数名可以自定义，这里以BuildCapabilities()为例，用于定义算法所支持的能力，返回值必须是
    static DetailEnhancerCapability BuildCapabilities();
    VPEAlgoErrCode Init() override;
    VPEAlgoErrCode Deinit() override;
    VPEAlgoErrCode SetParameter(const DetailEnhancerParameters& parameter, int type, bool flag) override;
    VPEAlgoErrCode Process(const sptr<SurfaceBuffer>& input, const sptr<SurfaceBuffer>& output) override;
};
// 注册函数，函数名可自定义，满足void XXX(uintptr_t XXX)形式，如下：
void RegisterSkiaExtensions(uintptr_t extensionListAddr);
} // VideoProcessingEngine
} // Media
} // OHOS
#endif // SKIA_IMPL_H
```
3. 插件算法能力注册与插件算法实现,具体可参考framework/algorithm/extensions/skia/skia_impl.cpp

```cpp
#include "skia_impl.h"

#include <array>
#include <chrono>
#include <dlfcn.h>
#include "detail_enhancer_extension.h"
#include "utils.h"

#include "vpe_log.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

namespace {
// 定义算法优先级，默认RANK_DEFAULT，若设置RANK_HIGH，则存在同类型算法时，优先使用RANK_HIGH算法。
constexpr Extension::Rank RANK = Extension::Rank::RANK_DEFAULT;
constexpr uint32_t VERSION = 0;
} // namespace

std::unique_ptr<DetailEnhancerBase> Skia::Create()
{
    return std::make_unique<Skia>();
}

// 算法能力注册
DetailEnhancerCapability Skia::BuildCapabilities()
{
    // 通过RANK设置当前算法高优先级，当存在同样能力的算法时，优先使用RANK_HIGH算法，默认为RANK_DEFAULT。
    std::vector<uint32_t> levels = { DETAIL_ENH_LEVEL_NONE, DETAIL_ENH_LEVEL_LOW, DETAIL_ENH_LEVEL_MEDIUM,
        DETAIL_ENH_LEVEL_HIGH_EVE, DETAIL_ENH_LEVEL_HIGH_AISR, DETAIL_ENH_LEVEL_VIDEO};
    DetailEnhancerCapability capability = { levels, RANK, VERSION };
    return capability;
}

VPEAlgoErrCode Skia::Init()
{
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode Skia::Deinit()
{
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode Skia::SetParameter([[maybe_unused]] const DetailEnhancerParameters& parameter,
    [[maybe_unused]] int type, [[maybe_unused]] bool flag)
{
    return VPE_ALGO_ERR_OK;
}

// 算法实现，具体可参考framework/algorithm/extensions/skia/skia_impl.cpp
VPEAlgoErrCode Skia::Process(const sptr<SurfaceBuffer>& input, const sptr<SurfaceBuffer>& output)
{
    return errCode;
}

// 实现注册函数，注册函数名可自定义修改，需与下面DoRegisterExtensions中的注册函数名一致
static std::vector<std::shared_ptr<Extension::ExtensionBase>> RegisterExtensions()
{
    std::vector<std::shared_ptr<Extension::ExtensionBase>> extensions;

    auto extension = std::make_shared<Extension::DetailEnhancerExtension>();
    CHECK_AND_RETURN_RET_LOG(extension != nullptr, extensions, "null pointer");
    // 填写插件算法信息，当前算法为缩放算法，类型则填写DETAIL_ENHANCER，后面两个字符串分别为算法名称以及版本号，可自定义修改。
    extension->info = { Extension::ExtensionType::DETAIL_ENHANCER, "SKIA", "0.0.1" };
    extension->creator = Skia::Create;
    extension->capabilitiesBuilder = Skia::BuildCapabilities;
    extensions.push_back(std::static_pointer_cast<Extension::ExtensionBase>(extension));

    return extensions;
}
// 注册函数，函数名可自定义
void RegisterSkiaExtensions(uintptr_t extensionListAddr)
{
    Extension::DoRegisterExtensions(extensionListAddr, RegisterExtensions);
}
} // VideoProcessingEngine
} // Media
} // OHOS
```

4. 添加注册插件回调函数，VPE插件管理会遍历所有插件注册函数。
  在framework/algorithm/extension_manager/include/static_extension_list.h文件staticExtensionsRegisterMap中添加算法插件。
```cpp
const std::unordered_map<std::string, RegisterExtensionFunc> staticExtensionsRegisterMap = {
    // 添加自定义插件算法名和注册函数，注册函数需与cpp中定义的注册函数同名
    { "Skia", RegisterSkiaExtensions },
};
```

再以图像动态元数据生成为例，以下描述了具体开发步骤。
1. 在framework/algorithm/extensions目录下新建一个文件夹，例如image_metadata_generator，以及对应的cpp和h文件
```
    /foundation/multimedia/video_processing_engine/
    ├── framework                                           # 框架代码
    │   ├── algorithm                                       # 算法框架
    │       ├── extensions                                  # 插件算法
    │           ├── image_metadata_generator                # 图像动态元数据生成
    │               ├── image_metadata_gen_impl.h
    │               ├── image_metadata_gen_impl.cpp
```
2. 图像动态元数据生成头文件实现

```cpp
#ifndef IMAGE_METADATA_GEN_IMPL_H
#define IMAGE_METADATA_GEN_IMPL_H

#include "metadata_generator_base.h"
#include "metadata_generator_capability.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
// 由基类MetadataGeneratorBase创建一个自定义元数据生成类，例如ImageMetadataGen
// 其余算法对应关系如上表所示

class ImageMetadataGen : public MetadataGeneratorBase {
public:
    // 定义Create函数，函数名可以自定义，这里以Create()为例，用于实例创建, 返回值必须是MetadataGeneratorBase指针。
    static std::unique_ptr<MetadataGeneratorBase> Create();
    // 定义能力构建函数，函数名可以自定义，这里以BuildCapabilities()为例，用于定义算法所支持的能力，返回值必须是vector<MetadataGeneratorCapability>。
    static std::vector<MetadataGeneratorCapability> BuildCapabilities();
    // 基类虚函数必须实现
    VPEAlgoErrCode Init(VPEContext context) override;
    VPEAlgoErrCode Deinit() override;
    VPEAlgoErrCode SetParameter(const MetadataGeneratorParameter &parameter) override;
    VPEAlgoErrCode GetParameter(MetadataGeneratorParameter &parameter) override;
    VPEAlgoErrCode Process(const sptr<SurfaceBuffer> &input) override;

private:
    MetadataGeneratorParameter parameter_;

};
// 注册函数，函数名可自定义，满足void XXX(uintptr_t XXX)形式，如下：
void RegisterImageMetadataGeneratorExtensions(uintptr_t extensionListAddr);
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
#endif // IMAGE_METADATA_GEN_IMPL_H

```
3. 插件算法能力注册与插件算法实现
```cpp
#include "image_metadata_gen_impl.h"
#include "metadata_generator_extension.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
// 实现create函数，创建实例
std::unique_ptr<MetadataGeneratorBase> ImageMetadataGen::Create()
{
    return std::make_unique<ImageMetadataGen>();
}

// 算法能力注册
std::vector<MetadataGeneratorCapability> ImageMetadataGen::BuildCapabilities()
{   
    // 设置算法支持的输入色彩空间，具体可参考framework/capi/image_processing/include/image_processing_capi_capability.h中定义
    std::vector<ColorSpaceDescription> inColorspaceList = {
        { GetColorSpaceInfo(CM_BT2020_PQ_LIMIT), CM_IMAGE_HDR_VIVID_SINGLE }};
    // 设置算法支持的pixelmap
    std::vector<GraphicPixelFormat> pixelFormatMap;
    pixelFormatMap.emplace_back(GRAPHIC_PIXEL_FMT_YCBCR_P010);   // NV12
    pixelFormatMap.emplace_back(GRAPHIC_PIXEL_FMT_YCRCB_P010);   // NV21
    pixelFormatMap.emplace_back(GRAPHIC_PIXEL_FMT_RGBA_1010102); // rgba1010102

    // 遍历色彩空间和pixelmap格式的所有组合，表示算法支持的所有能力，
    std::vector<MetadataGeneratorCapability> capabilities;
    for (const auto &inColorspace : inColorspaceList) {
        // 通过RANK_HIGH设置当前算法高优先级，当存在同样能力的算法时，优先使用RANK_HIGH算法，默认为RANK_DEFAULT。
        MetadataGeneratorCapability capability = { inColorspace, pixelFormatMap, Extension::Rank::RANK_HIGH, 0 };
        capabilities.emplace_back(capability);
    }
    return capabilities;
}

// 实现Init函数，可以根据实际算法，用于一些资源的初始化
VPEAlgoErrCode ImageMetadataGen::Init(VPEContext context)
{
    return VPE_ALGO_ERR_OK;
}

// 实现Deinit函数，可以根据实际算法，释放初始化的资源
VPEAlgoErrCode ImageMetadataGen::Deinit()
{
    return VPE_ALGO_ERR_OK;
}
// 实现参数设置函数
VPEAlgoErrCode ImageMetadataGen::SetParameter(const MetadataGeneratorParameter &parameter)
{
    parameter_ = parameter;
    return VPE_ALGO_ERR_OK;
}
// 实现参数获取函数
VPEAlgoErrCode ImageMetadataGen::GetParameter(MetadataGeneratorParameter &parameter)
{
    parameter = parameter_;
    return VPE_ALGO_ERR_OK;
}
// 算法功能实现
VPEAlgoErrCode ImageMetadataGen::Process(const sptr<SurfaceBuffer> &input)
{
    return VPE_ALGO_ERR_OK;
}

// 实现注册函数，注册函数名可自定义修改，需与下面DoRegisterExtensions中的注册函数名一致
static std::vector<std::shared_ptr<OHOS::Media::VideoProcessingEngine::Extension::ExtensionBase>> RegisterExtensions()
{
    std::vector<std::shared_ptr<OHOS::Media::VideoProcessingEngine::Extension::ExtensionBase>> extensions;

    auto extension = std::make_shared<OHOS::Media::VideoProcessingEngine::Extension::MetadataGeneratorExtension>();
    // 填写插件算法信息，当前算法为图像元数据生成，类型则填写METADATA_GENERATOR，后面两个字符串分别为算法名称以及版本号，可自定义修改。
    extension->info = { OHOS::Media::VideoProcessingEngine::Extension::ExtensionType::METADATA_GENERATOR,
        "ImageMetadataGen", "V1.0" };
    extension->creator = OHOS::Media::VideoProcessingEngine::ImageMetadataGen::Create;
    extension->capabilitiesBuilder = OHOS::Media::VideoProcessingEngine::ImageMetadataGen::BuildCapabilities;
    extensions.push_back(
        std::static_pointer_cast<OHOS::Media::VideoProcessingEngine::Extension::ExtensionBase>(extension));
    return extensions;
}

// 注册函数，函数名可自定义
void RegisterImageMetadataGeneratorExtensions(uintptr_t extensionListAddr)
{
    OHOS::Media::VideoProcessingEngine::Extension::DoRegisterExtensions(extensionListAddr, RegisterExtensions);
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
}
```
4. 添加注册插件回调函数，VPE插件管理会遍历所有插件注册函数。
  在framework/algorithm/extension_manager/include/static_extension_list.h文件staticExtensionsRegisterMap中添加算法插件。
```cpp
const std::unordered_map<std::string, RegisterExtensionFunc> staticExtensionsRegisterMap = {
    // 添加自定义插件算法名和注册函数，注册函数需与cpp中定义的注册函数同名
    {"ImgMetadataGeneratorExtensions", RegisterImageMetadataGeneratorExtensions}
};
```
## 相关仓

- [graphic_graphic_2d](https://gitee.com/openharmony/graphic_graphic_2d)
- [graphic_graphic_surface](https://gitee.com/openharmony/graphic_graphic_surface)
- [multimedia_image_framework](https://gitee.com/openharmony/multimedia_image_framework)
- [multimedia_media_foundation](https://gitee.com/openharmony/multimedia_media_foundation)
- [third_party_skia](https://gitee.com/openharmony/third_party_skia)