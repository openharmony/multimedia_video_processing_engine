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

#ifndef FRAMEWORK_ALGORITHM_EXTENSION_MANAGER_EXTENSION_MANAGER_H
#define FRAMEWORK_ALGORITHM_EXTENSION_MANAGER_EXTENSION_MANAGER_H

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <tuple>
#include <unordered_set>
#include <variant>
#include <vector>

#include "aihdr_enhancer.h"
#include "aihdr_enhancer_extension.h"
#include "static_extension_list.h"
#include "extension_base.h"
#include "frame_info.h"
#include "colorspace_converter.h"
#include "metadata_generator.h"
#include "detail_enhancer_image.h"
#include "video_refreshrate_prediction.h"
#include "colorspace_converter_capability.h"
#include "colorspace_converter_display_capability.h"
#include "metadata_generator_capability.h"
#include "colorspace_converter_extension.h"
#include "colorspace_converter_display_extension.h"
#include "metadata_generator_extension.h"
#include "detail_enhancer_extension.h"
#include "contrast_enhancer_extension.h"
#include "video_refreshrate_prediction_extension.h"
#include "utils.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
namespace Extension {
namespace {
/*
{
    (inputColorSpaceDesc, inputPixelFormat, outputColorSpaceDesc, outputPixelFormat): [
        (rank, version, extensionListIndex),
        (rank, version, extensionListIndex),
        ......
    ]
}*/
using ColorSpaceConverterCapabilityMap =
    std::map<
        std::tuple<ColorSpaceDescription, GraphicPixelFormat, ColorSpaceDescription, GraphicPixelFormat>,
        std::vector<std::tuple<Rank, int32_t, size_t>>>;
using ColorSpaceConverterDisplayCapabilityMap = ColorSpaceConverterCapabilityMap;
/*
{
    (colorSpaceDesc, pixelFormat, metadataGeneratorAlgoType): [
        (rank, version, extensionListIndex),
        (rank, version, extensionListIndex),
        ......
    ]
}
*/
using MetadataGeneratorCapabilityMap =
    std::map<
        std::tuple<ColorSpaceDescription, GraphicPixelFormat, MetadataGeneratorAlgoType>,
        std::vector<std::tuple<Rank, int32_t, size_t>>>;
using DetailEnhancerCapabilityMap = std::map<uint32_t, size_t>;
using AihdrEnhancerCapabilityMap =
    std::map<
        std::tuple<ColorSpaceDescription, GraphicPixelFormat>,
        std::vector<std::tuple<Rank, int32_t, size_t>>>;
using ContrastEnhancerCapabilityMap = std::map<ContrastEnhancerType, size_t>;

using ColorSpaceConverterDisplaySet = std::set<std::shared_ptr<ColorSpaceConverterDisplayBase>>;
using ColorSpaceConverterDisplayExtensionSet = std::set<std::shared_ptr<ColorSpaceConverterDisplayExtension>>;
}

class ExtensionManager {
public:
    static ExtensionManager& GetInstance();

    bool IsColorSpaceConversionSupported(const FrameInfo &inputInfo, const FrameInfo &outputInfo) const;
    std::shared_ptr<ColorSpaceConverterBase> CreateColorSpaceConverter(const FrameInfo &inputInfo,
        const FrameInfo &outputInfo, Extension::ExtensionInfo &extensionInfo) const;
    ColorSpaceConverterDisplaySet CreateColorSpaceConverterDisplay() const;
    std::shared_ptr<MetadataGeneratorBase> CreateMetadataGenerator(const FrameInfo &inputInfo,
        Extension::ExtensionInfo &extensionInfo, MetadataGeneratorAlgoType algoType) const;
    std::shared_ptr<AihdrEnhancerBase> CreateAihdrEnhancer(const FrameInfo &inputInfo,
        Extension::ExtensionInfo &extensionInfo) const;
    std::shared_ptr<DetailEnhancerBase> CreateDetailEnhancer(uint32_t level) const;
    std::shared_ptr<VideoRefreshRatePredictionBase> CreateVideoRefreshRatePredictor() const;
    std::shared_ptr<ContrastEnhancerBase> CreateContrastEnhancer(ContrastEnhancerType type) const;

    using InstanceVariableType = std::variant<std::shared_ptr<ColorSpaceConverter>,
                                                std::shared_ptr<MetadataGenerator>,
                                                std::shared_ptr<DetailEnhancerImage>,
                                                std::shared_ptr<AihdrEnhancer>>;
    void IncreaseInstance();
    void DecreaseInstance();
    int32_t NewInstanceId(const InstanceVariableType& instance);
    int32_t RemoveInstanceReference(int32_t& id);
    std::optional<InstanceVariableType> GetInstance(int32_t id);
    bool FindImageConverterExtension(const FrameInfo &inputInfo, const FrameInfo &outputInfo) const;
    bool FindImageComposeExtension(const FrameInfo &inputInfo, const FrameInfo &outputInfo) const;
    bool FindImageDecomposeExtension(const FrameInfo &inputInfo, const FrameInfo &outputInfo) const;
    bool FindImageMetadataGenExtension(const FrameInfo &inputInfo) const;
private:
    VPEAlgoErrCode Init();
    ExtensionManager();
    ~ExtensionManager();
    ExtensionManager(const ExtensionManager&) = delete;
    ExtensionManager& operator=(const ExtensionManager&) = delete;

    std::shared_ptr<ColorSpaceConverterExtension> FindColorSpaceConverterExtension(const FrameInfo &inputInfo,
        const FrameInfo &outputInfo) const;
    ColorSpaceConverterDisplayExtensionSet FindColorSpaceConverterDisplayExtension() const;
    std::shared_ptr<MetadataGeneratorExtension> FindMetadataGeneratorExtension(const FrameInfo &inputInfo,
        MetadataGeneratorAlgoType algoType) const;
    std::shared_ptr<AihdrEnhancerExtension> FindAihdrEnhancerExtension(const FrameInfo &inputInfo) const;
    std::shared_ptr<DetailEnhancerExtension> FindDetailEnhancerExtension(uint32_t level) const;
    std::shared_ptr<ContrastEnhancerExtension> FindContrastEnhancerExtension(ContrastEnhancerType type) const;
    ExtensionList LoadExtensions() const;
    VPEAlgoErrCode LoadStaticExtensions(ExtensionList& extensionList) const;
    ExtensionList LoadStaticImageExtensions(
    const std::unordered_map<std::string, RegisterExtensionFunc> staticListRegisterMap) const;
    ExtensionList LoadDynamicMetadataGenExtensions() const;
    ExtensionList LoadDynamicComposeExtensions() const;
    ExtensionList LoadDynamicDecomposeExtensions() const;
    void LoadDynamicExtensions(ExtensionList& extensionList) const;
    template<typename T> T BuildCaps(const ExtensionList& extensionList) const;
    VPEAlgoErrCode BuildColorSpaceConverterCaps(const std::shared_ptr<ExtensionBase>& ext, size_t idx,
        ColorSpaceConverterCapabilityMap& colorSpaceConverterCapabilityMap) const;
    VPEAlgoErrCode BuildColorSpaceConverterDisplayCaps(const std::shared_ptr<ExtensionBase>& ext, size_t idx,
        ColorSpaceConverterDisplayCapabilityMap& colorSpaceConverterDisplayCapabilityMap) const;
    VPEAlgoErrCode BuildMetadataGeneratorCaps(const std::shared_ptr<ExtensionBase>& ext, size_t idx,
        MetadataGeneratorCapabilityMap& metadataGeneratorCapabilityMap) const;
    VPEAlgoErrCode BuildAihdrEnhancerCaps(const std::shared_ptr<ExtensionBase>& ext, size_t idx,
        AihdrEnhancerCapabilityMap& aihdrEnhancerCapabilityMap) const;
    VPEAlgoErrCode BuildDetailEnhancerCaps(const std::shared_ptr<ExtensionBase>& ext, size_t idx,
        DetailEnhancerCapabilityMap& detailEnhancerCapabilityMap) const;
    VPEAlgoErrCode BuildContrastEnhancerCaps(const std::shared_ptr<ExtensionBase>& ext, size_t idx,
    ContrastEnhancerCapabilityMap& contrastEnhancerCapabilityMap) const;
    VPEAlgoErrCode ExtractColorSpaceConverterCap(const ColorSpaceConverterCapability& cap, size_t idx,
        ColorSpaceConverterCapabilityMap& colorSpaceConverterCapabilityMap) const;
    VPEAlgoErrCode ExtractMetadataGeneratorCap(const MetadataGeneratorCapability &cap, size_t idx,
        MetadataGeneratorAlgoType algoType, MetadataGeneratorCapabilityMap& metadataGeneratorCapabilityMap) const;
    VPEAlgoErrCode ExtractAihdrEnhancerCap(const AihdrEnhancerCapability &cap, size_t idx,
        AihdrEnhancerCapabilityMap& aihdrEnhancerCapabilityMap) const;

    std::atomic<bool> initialized_ {false};

    static constexpr int32_t MAX_INSTANCE_NUM { 1024 };
    std::mutex instanceManagementMtx_;
    std::mutex instanceCountMtx_;
    int32_t currentId_ { 0 };
    int32_t usedInstance_ { 0 };
    std::array<std::optional<InstanceVariableType>, MAX_INSTANCE_NUM> instanceList_ { std::nullopt };
};
    extern "C" bool ImageProcessing_IsColorSpaceConversionSupported(const ColorSpaceInfo inputInfo,
        const ColorSpaceInfo outputInfo);
    extern "C" bool ImageProcessing_IsCompositionSupported(const ColorSpaceInfo inputInfo,
        const ColorSpaceInfo outputInfo);
    extern "C" bool ImageProcessing_IsDecompositionSupported(const ColorSpaceInfo inputInfo,
        const ColorSpaceInfo outputInfo);
    extern "C" bool ImageProcessing_IsMetadataGenerationSupported(const ColorSpaceInfo inputInfo);
} // namespace Extension
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // FRAMEWORK_ALGORITHM_EXTENSION_MANAGER_EXTENSION_MANAGER_H
