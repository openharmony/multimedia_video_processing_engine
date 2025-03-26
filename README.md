# VPE(multimedia_video_processing_engine)

## Overview
The Video Processing Engine (VPE) is a media engine for processing video and image data. It offers a range of fundamental capabilities including enhancements to details, contrast, luminance, and dynamic ranges. It also supports essential algorithms for color space conversion, scaling and upscaling, and dynamic metadata generation for transcoding, sharing, and post-processing for display.

The following figure demonstrates the VPE architecture.

![VPE architecture](./figures/videoProcessingEngine_architecture_english.png)

## Directory Structure

The structure of the repository directory is as follows:

```
/foundation/multimedia/video_processing_engine/
├── framework                                  # Framework code
│   ├── algorithm                              # Algorithm framework
│       ├── aihdr_enhancer                     # Image HDR enhancement algorithm framework
│       ├── aihdr_enhancer_video               # Video HDR enhancement algorithm framework
│       ├── colorspace_converter               # Image color space conversion algorithm framework
│       ├── colorspace_converter_display       # Image color space display algorithm framework
│       ├── colorspace_converter_video         # Video color space conversion algorithm framework
│       ├── detail_enhancer                    # Image detail enhancement algorithm framework
│       ├── detail_enhancer_video              # Video detail enhancement algorithm framework
│       ├── extension_manager                  # Plugin management
│       ├── metadata_generator                 # Image metadata generation algorithm framework
│       ├── metadata_generator_video           # Video metadata generation algorithm framework
│       ├── video_variable_refresh_rate        # Video variable frame rate algorithm framework
│   ├── capi                                   # CAPI layer
│       ├── image_processing                   # Image CAPI
│       ├── video_processing                   # Video CAPI
│   ├── dfx                                    # DFX code
├── interfaces                                 # API layer
│   ├── inner_api                              # Internal APIs
│   ├── kits                                   # Application APIs
├── services                                   # Service code
├── sertestvices                               # Test code
```

## Build

Run the following command to build the VPE for the 32-bit ARM system:
```
./build.sh --product-name {product_name} --ccache --build-target video_processing_engine
```

Run the following command to build the VPE for the 64-bit ARM system:
```
./build.sh --product-name {product_name} --ccache --target-cpu arm64 --build-target video_processing_engine
```

**product_name** indicates the product supported, for example, **rk3568**.

## Description

### How to Use
As a component of OpenHarmony, the VPE provides video and image processing capabilities, including color space conversion, dynamic metadata generation, and detail enhancement.

#### Image Scale
1. Add a header file.
    ```cpp
    #include <hilog/log.h>
    #include <multimedia/image_framework/image_pixel_map_mdk.h>
    #include <multimedia/image_framework/image/pixelmap_native.h>
    #include <multimedia/video_processing_engine/image_processing.h>
    #include <multimedia/video_processing_engine/image_processing_types.h>
    #include <multimedia/player_framework/native_avformat.h>
    #include <napi/native_api.h>
    ```
2. Optionally, initialize the environment.
  Generally, it is called when it is used for the first time in the process, and some time-consuming operations can be completed in advance.
    ```cpp
    ImageProcessing_ErrorCode ret =  OH_ImageProcessing_InitializeEnvironment();
    ```
3. Create a detail enhancement module.
  Applications can create image detail enhancement modules by image processing engine module types. The variables in the example are described as follows:
  imageProcessor：an instance of the Detail Enhancement module.
  IMAGE_PROCESSING_TYPE_DETAIL_ENHANCER：Detail enhancement type.
  Expected return value: IMAGE_PROCESSING_SUCCESS
    ```cpp
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Create(&imageProcessor, IMAGE_PROCESSING_TYPE_DETAIL_ENHANCER);
    ```
4. (Optional) Configure the detail enhancement gear, there are currently three gears of high, medium and low and NONE optional, if not configured, the default gear is LOW.
    ```cpp
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, IMAGE_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL,
        IMAGE_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH);
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_SetParameter(imageProcessor,parameter);
    ```
5. Initiates detail enhancement processing.
    ```cpp
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_EnhanceDetail(imageProcessor, srcImage, dstImage);
    ```
6. Release the processing instance.
    ```cpp
    ImageProcessing_ErrorCode ret = OH_ImageProcessing_Destroy(imageProcessor);
    ```
7. Free up processing resources.
    ```cpp
    OH_ImageProcessing_DeinitializeEnvironment();
    ```
#### video zoom
1. Add a header file.
    ```cpp
    #include <ace/xcomponent/native_interface_xcomponent.h>
    #include <multimedia/player_framework/native_avformat.h>
    #include <multimedia/video_processing_engine/video_processing.h>
    #include <multimedia/video_processing_engine/video_processing_types.h>
    #include <native_window/external_window.h>
    #include <native_buffer/native_buffer.h>
    ```
2. Optionally, create a decode instance.
  The input of the detail enhancement module can be a video stream decoded by the system, or the application can fill the window with video data (for example, the application directly fills the data into the window after internal soft decoding). If you select a system decoder to process a video file or video stream, you can create a decoder instance as input to the Detail Enhancement module.
    ```cpp
    OH_AVSource* source_ = OH_AVSource_CreateWithFD(inputFd, inputFileOffset, inputFileSize);
    OH_AVDemuxer* demuxer_ = OH_AVDemuxer_CreateWithSource(source_);
    auto sourceFormat = std::shared_ptr<OH_AVFormat>(OH_AVSource_GetSourceFormat(source_), OH_AVFormat_Destroy);
    OH_AVCodec * decoder_ = OH_VideoDecoder_CreateByMime(videoCodecMime.c_str());
    OH_AVFormat *format = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_WIDTH, videoWidth);
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_HEIGHT, videoHeight);
    OH_AVFormat_SetDoubleValue(format, OH_MD_KEY_FRAME_RATE, frameRate);
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_PIXEL_FORMAT, pixelFormat);
    OH_AVFormat_SetIntValue(format, OH_MD_KEY_ROTATION, rotation);
    int ret = OH_VideoDecoder_Configure(decoder_, format);
    OH_AVFormat_Destroy(format);
    OH_VideoDecoder_RegisterCallback(decoder_,
            {SampleCallback::OnCodecError, SampleCallback::OnCodecFormatChange,
            SampleCallback::OnNeedInputBuffer, SampleCallback::OnNewOutputBuffer}, videoDecContext_);

    int ret = OH_VideoDecoder_Prepare(decoder_);

    videoDecContext_ = new CodecUserData;
    ```
3. Optionally, initialize the environment.
  Generally, it is called when it is used for the first time in the process, and some time-consuming operations can be completed in advance.
    ```cpp
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_InitializeEnvironment();
    ```
4. Create a detail enhancement module.
  Applications can create detail enhancement modules by video processing engine module types. The variables in the example are described as follows:
  videoProcessor：an instance of the Detail Enhancement module.
  VIDEO_PROCESSING_TYPE_DETAIL_ENHANCER：Detail enhancement type.
  Expected return value: VIDEO_PROCESSING_SUCCESS
    ```cpp
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_Create(&videoProcessor， VIDEO_PROCESSING_TYPE_DETAIL_ENHANCER);
    ```
5. Configure an asynchronous callback function.
    ```cpp
    ret = OH_VideoProcessingCallback_Create(&callback);
    OH_VideoProcessingCallback_BindOnError(callback, OnError);
    OH_VideoProcessingCallback_BindOnState(callback, OnState);
    OH_VideoProcessingCallback_BindOnNewOutputBuffer(callback, OnNewOutputBuffer);
    ret = OH_VideoProcessing_RegisterCallback(videoProcessor, callback, this);
    void OnError(OH_VideoProcessing* videoProcessor, VideoProcessing_ErrorCode error, void* userData);
    void OnState(OH_VideoProcessing* videoProcessor, VideoProcessing_State state, void* userData);
    void OnNewOutputBuffer(OH_VideoProcessing* videoProcessor, uint32_t index, void* userData);
    ```
6. (Optional) Configure the detail enhancement gear, there are currently three gears of high, medium and low and NONE optional, if not configured, the default gear is LOW.
    ```cpp
    OH_AVFormat* parameter = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(parameter, VIDEO_DETAIL_ENHANCER_PARAMETER_KEY_QUALITY_LEVEL, VIDEO_DETAIL_ENHANCER_QUALITY_LEVEL_HIGH);
    OH_VideoProcessing_SetParameter(videoProcessor, parameter);
    ```
7. Get your Surface.
    ```cpp
    ret = OH_VideoProcessing_GetSurface(videoProcessor, inputWindow);
    OH_VideoDecoder_SetSurface(decoder_,  inputWindow_);
    ```
8. Set Surface
    ```cpp
    ret = OH_VideoProcessing_SetSurface(videoProcessor, outWindow);
    ```
9. Create a decoder input and output thread.
    ```cpp
    std::unique_ptr<std::thread> videoDecInputThread_ = std::make_unique<std::thread>(&Player::VideoDecInputThread, this);
    std::unique_ptr<std::thread> videoDecOutputThread_ = std::make_unique<std::thread>(&Player::VideoDecOutputThread, this);
    ```
10. Initiates detail enhancement processing.
    ```cpp
    int ret = OH_VideoDecoder_Start(decoder_);
    ret = OH_VideoProcessing_Start(videoProcessor);
    ```
11. Call OH_VideoProcessing_Stop()
    ```cpp
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_Stop(videoProcessor);
    ```
12. Release the processing instance.
    ```cpp
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_Destroy(videoProcessor)；
    VideoProcessing_ErrorCode ret = OH_VideoProcessingCallback_Destroy(callback);
    ```
13. Free up processing resources.
    ```cpp
    VideoProcessing_ErrorCode ret = OH_VideoProcessing_DeinitializeEnvironment();
    ```
#### Custom algorithm plugin registration
1. Implement plugin registration function
    ```cpp
    std::vector<MetadataGeneratorCapability> ImageMetadataGen::BuildCapabilities()
    {
        std::vector<MetadataGeneratorCapability> capabilities;
        for (const auto &inColorspace : inColorspaceList) {
            MetadataGeneratorCapability capability = { inColorspace, pixelFormatMap, RANK, IMAGEMETAGENVERSION };
            capabilities.emplace_back(capability);
        }
        return capabilities;
    }

    static std::vector<std::shared_ptr<OHOS::Media::VideoProcessingEngine::Extension::ExtensionBase>> RegisterExtensions()
    {
        std::vector<std::shared_ptr<OHOS::Media::VideoProcessingEngine::Extension::ExtensionBase>> extensions;
        auto extension = std::make_shared<OHOS::Media::VideoProcessingEngine::Extension::MetadataGeneratorExtension>();
        extension->info = { OHOS::Media::VideoProcessingEngine::Extension::ExtensionType::METADATA_GENERATOR,
        "ImageMetadataGen", "v1" };
        extension->capabilitiesBuilder = OHOS::Media::VideoProcessingEngine::ImageMetadataGen::BuildCapabilities;
        extensions.push_back(
            std::static_pointer_cast<OHOS::Media::VideoProcessingEngine::Extension::ExtensionBase>(extension));
        return extensions;
    }

    void RegisterImageMetadataGeneratorExtensions(uintptr_t extensionListAddr)
    {
        OHOS::Media::VideoProcessingEngine::Extension::DoRegisterExtensions(extensionListAddr, RegisterExtensions);
    }
    ```
2. Implement algorithm
    ```cpp
    ImageMetadataGen::Process(const sptr<SurfaceBuffer> &input)
    ```
3. Add registered plugin callback function.
  Add at staticExtensionsRegisterMap
    ```cpp
    const std::unordered_map<std::string, RegisterExtensionFunc> staticExtensionsRegisterMap = {
        {"ImageMetadataGeneratorExtensions", RegisterImageMetadataGeneratorExtensions}
    };
    ```
## Repositories Involved

- [graphic_graphic_2d](https://gitee.com/openharmony/graphic_graphic_2d)
- [graphic_graphic_surface](https://gitee.com/openharmony/graphic_graphic_surface)
- [multimedia_image_framework](https://gitee.com/openharmony/multimedia_image_framework)
- [multimedia_media_foundation](https://gitee.com/openharmony/multimedia_media_foundation)
- [third_party_egl](https://gitee.com/openharmony/third_party_egl)
- [third_party_opengles](https://gitee.com/openharmony/third_party_opengles)
- [third_party_opencl-headers](https://gitee.com/openharmony/third_party_opencl-headers)
- [third_party_skia](https://gitee.com/openharmony/third_party_skia)
