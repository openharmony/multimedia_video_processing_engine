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

#include "extension_manager.h"
#include <cstdint>
#include <dlfcn.h>
#include <functional>
#include <string>
#include <unordered_map>
#include "static_extension_list.h"
#include "vpe_log.h"
#include "extension_base.h"
namespace {
    using LibFunctionGetRegisters = std::unordered_map<std::string,
        OHOS::Media::VideoProcessingEngine::Extension::RegisterExtensionFunc>* (*)();
    LibFunctionGetRegisters GetRegisterExtensionFuncs{nullptr};
    LibFunctionGetRegisters GetRegisterMetdataGenExtensionFuncs{nullptr};
    LibFunctionGetRegisters GetRegisterComposeExtensionFuncs{nullptr};
    LibFunctionGetRegisters GetRegisterDecomposeExtensionFuncs{nullptr};
    using LibFunctionGetRegisterFunction = OHOS::Media::VideoProcessingEngine::Extension::RegisterExtensionFunc (*)();
    LibFunctionGetRegisterFunction GetRegisterVRRExtensionFuncs{nullptr};
    void* g_algoHandle{nullptr};
} // namespace

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
namespace Extension {

ExtensionManager& ExtensionManager::GetInstance()
{
    static ExtensionManager instance;
    return instance;
}

ExtensionManager::ExtensionManager()
{
    (void)Init();
    g_algoHandle = dlopen("libvideoprocessingengine_ext.z.so", RTLD_NOW);
}

ExtensionManager::~ExtensionManager()
{
    if (g_algoHandle != nullptr) {
        dlclose(g_algoHandle);
        g_algoHandle = nullptr;
    }
}

VPEAlgoErrCode ExtensionManager::Init()
{
    if (initialized_) {
        return VPE_ALGO_ERR_OK;
    }
    initialized_ = true;
    return VPE_ALGO_ERR_OK;
}

void ExtensionManager::IncreaseInstance()
{
    std::lock_guard<std::mutex> lock(instanceCountMtx_);
    if (usedInstance_ == 0 && g_algoHandle == nullptr) {
        g_algoHandle = dlopen("libvideoprocessingengine_ext.z.so", RTLD_NOW);
    }
    usedInstance_++;
}

void ExtensionManager::DecreaseInstance()
{
    std::lock_guard<std::mutex> lock(instanceCountMtx_);
    usedInstance_--;
    if ((usedInstance_ == 0) && (g_algoHandle != nullptr)) {
        dlclose(g_algoHandle);
        g_algoHandle = nullptr;
    }
}

bool ExtensionManager::IsColorSpaceConversionSupported(const FrameInfo &inputInfo, const FrameInfo &outputInfo) const
{
    if (!initialized_) {
        return false;
    }
    return FindColorSpaceConverterExtension(inputInfo, outputInfo) == nullptr;
}

std::shared_ptr<ColorSpaceConverterBase> ExtensionManager::CreateColorSpaceConverter(const FrameInfo &inputInfo,
    const FrameInfo &outputInfo, Extension::ExtensionInfo &extensionInfo) const
{
    CHECK_AND_RETURN_RET_LOG(initialized_ == true, nullptr, "Not initialized");
    auto extension = FindColorSpaceConverterExtension(inputInfo, outputInfo);
    CHECK_AND_RETURN_RET_LOG(extension != nullptr, nullptr, "Create extension failed, get an empty extension");
    auto impl = extension->creator();
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr,
        "Call extension creator failed, return a empty impl, extension: %{public}s", extension->info.name.c_str());
    extensionInfo = extension->info;
    return impl;
}

std::shared_ptr<VideoRefreshRatePredictionBase> ExtensionManager::CreateVideoRefreshRatePredictor() const
{
    std::shared_ptr<ExtensionBase> extension;
    CHECK_AND_RETURN_RET_LOG(g_algoHandle != nullptr, {}, "dlopen ext fail!");
    GetRegisterVRRExtensionFuncs = reinterpret_cast<LibFunctionGetRegisterFunction>
        (dlsym(g_algoHandle, "GetRegisterVRRExtensionFuncs"));
    CHECK_AND_RETURN_RET_LOG(GetRegisterVRRExtensionFuncs != nullptr, {}, "dlsym Get VRR Extension fail!");
    
    auto registerFunctionPtr = GetRegisterVRRExtensionFuncs();
    CHECK_AND_RETURN_RET_LOG(registerFunctionPtr != nullptr, {}, "get GetRegisterVRRExtensionFuncs fail!!");
    registerFunctionPtr(reinterpret_cast<uintptr_t>(&extension));
    std::shared_ptr<VideoRefreshratePredictionExtension> vrrExtension =
            std::static_pointer_cast<VideoRefreshratePredictionExtension>(extension);
    auto impl = vrrExtension->creator();
    return impl;
}

ColorSpaceConverterDisplaySet ExtensionManager::CreateColorSpaceConverterDisplay() const
{
    CHECK_AND_RETURN_RET_LOG(initialized_ == true, {}, "Not initialized");

    ColorSpaceConverterDisplaySet impl;
    for (const auto &extension : FindColorSpaceConverterDisplayExtension()) {
        auto temp = extension->creator();
        CHECK_AND_RETURN_RET_LOG(temp != nullptr, {}, "Create failed, extension: %{public}s",
            extension->info.name.c_str());
        impl.emplace(std::move(temp));
    }
    return impl;
}

std::shared_ptr<MetadataGeneratorBase> ExtensionManager::CreateMetadataGenerator(const FrameInfo &inputInfo,
    Extension::ExtensionInfo &extensionInfo, MetadataGeneratorAlgoType algoType) const
{
    CHECK_AND_RETURN_RET_LOG(initialized_ == true, nullptr, "Not initialized");
    auto extension = FindMetadataGeneratorExtension(inputInfo, algoType);
    CHECK_AND_RETURN_RET_LOG(extension != nullptr, nullptr, "Create failed, get an empty extension");
    auto impl = extension->creator();
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr,
        "Call extension creator failed, return a empty impl, extension: %{public}s", extension->info.name.c_str());
    extensionInfo = extension->info;
    return impl;
}

std::shared_ptr<AihdrEnhancerBase> ExtensionManager::CreateAihdrEnhancer(const FrameInfo &inputInfo,
    Extension::ExtensionInfo &extensionInfo) const
{
    CHECK_AND_RETURN_RET_LOG(initialized_ == true, nullptr, "Not initialized");
    auto extension = FindAihdrEnhancerExtension(inputInfo);
    CHECK_AND_RETURN_RET_LOG(extension != nullptr, nullptr, "Create failed, get an empty extension");
    auto impl = extension->creator();
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr,
        "Call extension creator failed, return a empty impl, extension: %{public}s", extension->info.name.c_str());
    extensionInfo = extension->info;
    return impl;
}

int32_t ExtensionManager::NewInstanceId(const ExtensionManager::InstanceVariableType& instance)
{
    std::lock_guard<std::mutex> lock(instanceManagementMtx_);
    int32_t newId = currentId_;
    do {
        currentId_ = (currentId_ == MAX_INSTANCE_NUM) ? 0 : currentId_;
        if (instanceList_[currentId_] == std::nullopt) {
            instanceList_[currentId_] = instance;
            newId = currentId_;
            ++currentId_;
            return newId;
        }
        ++currentId_;
    } while (currentId_ != newId);

    return -1;
}

int32_t ExtensionManager::RemoveInstanceReference(int32_t& id)
{
    std::lock_guard<std::mutex> lock(instanceManagementMtx_);
    CHECK_AND_RETURN_RET_LOG(id >= 0 && id < MAX_INSTANCE_NUM, VPE_ALGO_ERR_INVALID_VAL, "invalid instance id");
    instanceList_[id].reset() ;
    id = -1;

    return VPE_ALGO_ERR_OK;
}

std::optional<ExtensionManager::InstanceVariableType> ExtensionManager::GetInstance(int32_t id)
{
    std::lock_guard<std::mutex> lock(instanceManagementMtx_);
    CHECK_AND_RETURN_RET_LOG(id >= 0 && id < MAX_INSTANCE_NUM, std::nullopt, "invalid instance id");
    return instanceList_[id];
}

std::shared_ptr<DetailEnhancerBase> ExtensionManager::CreateDetailEnhancer(uint32_t level) const
{
    CHECK_AND_RETURN_RET_LOG(initialized_ == true, nullptr, "Not initialized");
    auto extension = FindDetailEnhancerExtension(level);
    CHECK_AND_RETURN_RET_LOG(extension != nullptr, nullptr,
        "Create failed, get an empty extension. level: %{public}d", level);
    auto impl = extension->creator();
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr,
        "Call extension creator failed, return a empty impl, extension: %{public}s, level: %{public}d",
        extension->info.name.c_str(), level);
    return impl;
}

ExtensionList ExtensionManager::LoadExtensions() const
{
    ExtensionList extensionList {};
    LoadDynamicExtensions(extensionList);
    VPEAlgoErrCode ret = LoadStaticExtensions(extensionList);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, {}, "Load extension failed");
    return extensionList;
}

ExtensionList ExtensionManager::LoadStaticImageExtensions(
    const std::unordered_map<std::string, RegisterExtensionFunc> staticListRegisterMap) const
{
    ExtensionList extensionList {};
    for (auto &reg : staticListRegisterMap) {
        CHECK_AND_RETURN_RET_LOG(reg.second != nullptr, {},
            "Get an empty register, extension: %{public}s", reg.first.c_str());
        VPE_LOGD("Load extension set: %{public}s", reg.first.c_str());
        reg.second(reinterpret_cast<uintptr_t>(&extensionList));
    }
    return extensionList;
}

bool ExtensionManager::FindImageConverterExtension(
    const FrameInfo &inputInfo, const FrameInfo &outputInfo) const
{
    auto extensionList = LoadExtensions();
    CHECK_AND_RETURN_RET_LOG(!extensionList.empty(), false, "No extension found");
    auto colorSpaceConverterCapabilityMap = BuildCaps<ColorSpaceConverterCapabilityMap>(extensionList);
    CHECK_AND_RETURN_RET_LOG(!colorSpaceConverterCapabilityMap.empty(), false, "No extension available");
    auto key =
        std::make_tuple(inputInfo.colorSpace, inputInfo.pixelFormat, outputInfo.colorSpace, outputInfo.pixelFormat);
    VPE_LOG_PRINT_COLOR_SPACE_CAPBILITY(inputInfo.colorSpace.colorSpaceInfo, inputInfo.pixelFormat);
    VPE_LOG_PRINT_COLOR_SPACE_CAPBILITY(outputInfo.colorSpace.colorSpaceInfo, outputInfo.pixelFormat);
    const auto iter = colorSpaceConverterCapabilityMap.find(key);
    CHECK_AND_RETURN_RET_LOG(iter != colorSpaceConverterCapabilityMap.cend() && !iter->second.empty(), false,
        "CSC Extension is not found");
    return true;
}

bool ExtensionManager::FindImageComposeExtension(
    const FrameInfo &inputInfo, const FrameInfo &outputInfo) const
{
    auto extensionList = LoadDynamicComposeExtensions();
    CHECK_AND_RETURN_RET_LOG(!extensionList.empty(), false, "No extension found");
    auto colorSpaceConverterCapabilityMap = BuildCaps<ColorSpaceConverterCapabilityMap>(extensionList);
    CHECK_AND_RETURN_RET_LOG(!colorSpaceConverterCapabilityMap.empty(), false, "No extension available");
    auto key =
        std::make_tuple(inputInfo.colorSpace, inputInfo.pixelFormat, outputInfo.colorSpace, outputInfo.pixelFormat);
    VPE_LOG_PRINT_COLOR_SPACE_CAPBILITY(inputInfo.colorSpace.colorSpaceInfo, inputInfo.pixelFormat);
    VPE_LOG_PRINT_COLOR_SPACE_CAPBILITY(outputInfo.colorSpace.colorSpaceInfo, outputInfo.pixelFormat);
    const auto iter = colorSpaceConverterCapabilityMap.find(key);
    CHECK_AND_RETURN_RET_LOG(iter != colorSpaceConverterCapabilityMap.cend() && !iter->second.empty(), false,
        "CSC Extension is not found");
    return true;
}

bool ExtensionManager::FindImageDecomposeExtension(
    const FrameInfo &inputInfo, const FrameInfo &outputInfo) const
{
    auto extensionList = LoadDynamicDecomposeExtensions();
    CHECK_AND_RETURN_RET_LOG(!extensionList.empty(), false, "No extension found");
    auto colorSpaceConverterCapabilityMap = BuildCaps<ColorSpaceConverterCapabilityMap>(extensionList);
    CHECK_AND_RETURN_RET_LOG(!colorSpaceConverterCapabilityMap.empty(), false, "No extension available");
    auto key =
        std::make_tuple(inputInfo.colorSpace, inputInfo.pixelFormat, outputInfo.colorSpace, outputInfo.pixelFormat);
    VPE_LOG_PRINT_COLOR_SPACE_CAPBILITY(inputInfo.colorSpace.colorSpaceInfo, inputInfo.pixelFormat);
    VPE_LOG_PRINT_COLOR_SPACE_CAPBILITY(outputInfo.colorSpace.colorSpaceInfo, outputInfo.pixelFormat);
    const auto iter = colorSpaceConverterCapabilityMap.find(key);
    CHECK_AND_RETURN_RET_LOG(iter != colorSpaceConverterCapabilityMap.cend() && !iter->second.empty(), false,
        "CSC Extension is not found");
    return true;
}

bool ExtensionManager::FindImageMetadataGenExtension(const FrameInfo &inputInfo) const
{
    auto extensionList = LoadDynamicMetadataGenExtensions();
    CHECK_AND_RETURN_RET_LOG(!extensionList.empty(), false, "No extension found");
    auto metadataGeneratorCapabilityMap = BuildCaps<MetadataGeneratorCapabilityMap>(extensionList);
    CHECK_AND_RETURN_RET_LOG(!metadataGeneratorCapabilityMap.empty(), false, "No extension available");
    auto key = std::make_tuple(inputInfo.colorSpace, inputInfo.pixelFormat,
        MetadataGeneratorAlgoType::META_GEN_ALGO_TYPE_IMAGE);
    VPE_LOG_PRINT_METADATA_GEN_CAPBILITY(inputInfo.colorSpace.colorSpaceInfo, inputInfo.pixelFormat,
        MetadataGeneratorAlgoType::META_GEN_ALGO_TYPE_IMAGE);
    const auto iter = metadataGeneratorCapabilityMap.find(key);
    CHECK_AND_RETURN_RET_LOG(iter != metadataGeneratorCapabilityMap.cend() && !iter->second.empty(), false,
        "CSC metadata generator extension is not found");
    return true;
}

bool ImageProcessing_IsColorSpaceConversionSupported(const ColorSpaceInfo inputInfo, const ColorSpaceInfo outputInfo)
{
    FrameInfo inputFrameInfo;
    inputFrameInfo.colorSpace = {GetColorSpaceInfo(inputInfo.colorSpace), inputInfo.metadataType};
    inputFrameInfo.pixelFormat = inputInfo.pixelFormat;

    FrameInfo outputFrameInfo;
    outputFrameInfo.colorSpace = {GetColorSpaceInfo(outputInfo.colorSpace), outputInfo.metadataType};
    outputFrameInfo.pixelFormat = outputInfo.pixelFormat;
    ExtensionManager::GetInstance().IncreaseInstance();
    auto flag = ExtensionManager::GetInstance().FindImageConverterExtension(inputFrameInfo, outputFrameInfo);
    ExtensionManager::GetInstance().DecreaseInstance();
    return flag;
}

bool ImageProcessing_IsCompositionSupported(const ColorSpaceInfo inputInfo, const ColorSpaceInfo outputInfo)
{
    FrameInfo inputFrameInfo;
    inputFrameInfo.colorSpace = {GetColorSpaceInfo(inputInfo.colorSpace), inputInfo.metadataType};
    inputFrameInfo.pixelFormat = inputInfo.pixelFormat;

    FrameInfo outputFrameInfo;
    outputFrameInfo.colorSpace = {GetColorSpaceInfo(outputInfo.colorSpace), outputInfo.metadataType};
    outputFrameInfo.pixelFormat = outputInfo.pixelFormat;
    ExtensionManager::GetInstance().IncreaseInstance();
    auto flag = ExtensionManager::GetInstance().FindImageComposeExtension(inputFrameInfo, outputFrameInfo);
    ExtensionManager::GetInstance().DecreaseInstance();
    return flag;
}

bool ImageProcessing_IsDecompositionSupported(const ColorSpaceInfo inputInfo, const ColorSpaceInfo outputInfo)
{
    FrameInfo inputFrameInfo;
    inputFrameInfo.colorSpace = {GetColorSpaceInfo(inputInfo.colorSpace), inputInfo.metadataType};
    inputFrameInfo.pixelFormat = inputInfo.pixelFormat;

    FrameInfo outputFrameInfo;
    outputFrameInfo.colorSpace = {GetColorSpaceInfo(outputInfo.colorSpace), outputInfo.metadataType};
    outputFrameInfo.pixelFormat = outputInfo.pixelFormat;
    ExtensionManager::GetInstance().IncreaseInstance();
    auto flag = ExtensionManager::GetInstance().FindImageDecomposeExtension(inputFrameInfo, outputFrameInfo);
    ExtensionManager::GetInstance().DecreaseInstance();
    return flag;
}

bool ImageProcessing_IsMetadataGenerationSupported(const ColorSpaceInfo inputInfo)
{
    FrameInfo inputFrameInfo;
    inputFrameInfo.colorSpace = {GetColorSpaceInfo(inputInfo.colorSpace), inputInfo.metadataType};
    inputFrameInfo.pixelFormat = inputInfo.pixelFormat;
    ExtensionManager::GetInstance().IncreaseInstance();
    auto flag = ExtensionManager::GetInstance().FindImageMetadataGenExtension(inputFrameInfo);
    ExtensionManager::GetInstance().DecreaseInstance();
    return flag;
}

VPEAlgoErrCode ExtensionManager::LoadStaticExtensions(ExtensionList& extensionList) const
{
    for (auto &reg : staticExtensionsRegisterMap) {
        CHECK_AND_RETURN_RET_LOG(reg.second != nullptr, VPE_ALGO_ERR_UNKNOWN,
            "Get an empty register, extension: %{public}s", reg.first.c_str());
        VPE_LOGD("Load extension set: %{public}s", reg.first.c_str());
        reg.second(reinterpret_cast<uintptr_t>(&extensionList));
    }
    return VPE_ALGO_ERR_OK;
}

void ExtensionManager::LoadDynamicExtensions(ExtensionList& extensionList) const
{
    CHECK_AND_RETURN_LOG(g_algoHandle != nullptr, "dlopen ext fail!");
    GetRegisterExtensionFuncs = reinterpret_cast<LibFunctionGetRegisters>
        (dlsym(g_algoHandle, "GetRegisterExtensionFuncs"));
    CHECK_AND_RETURN_LOG(GetRegisterExtensionFuncs != nullptr, "dlsym GetRegisterExtensionFuncs fail!");
    auto dynamicExtensionsRegisterMapPtr = GetRegisterExtensionFuncs();
    CHECK_AND_RETURN_LOG(dynamicExtensionsRegisterMapPtr != nullptr, "get dynamicExtensionsRegisterMap fail!!");
    auto dynamicExtensionsRegisterMap = *dynamicExtensionsRegisterMapPtr;
    for (auto &reg : dynamicExtensionsRegisterMap) {
        if (reg.second != nullptr) {
            VPE_LOGD("Load extension set: %{public}s", reg.first.c_str());
            reg.second(reinterpret_cast<uintptr_t>(&extensionList));
        }
    }
}

ExtensionList ExtensionManager::LoadDynamicMetadataGenExtensions() const
{
    ExtensionList extensionList {};
    CHECK_AND_RETURN_RET_LOG(g_algoHandle != nullptr, {}, "dlopen ext fail!");

    GetRegisterMetdataGenExtensionFuncs = reinterpret_cast<LibFunctionGetRegisters>
        (dlsym(g_algoHandle, "GetRegisterMetdataGenExtensionFuncs"));
    CHECK_AND_RETURN_RET_LOG(GetRegisterMetdataGenExtensionFuncs != nullptr, {},
        "dlsym GetRegisterMetdataGenExtensionFuncs fail!");

    auto dynamicExtensionsRegisterMapPtr = GetRegisterMetdataGenExtensionFuncs();
    CHECK_AND_RETURN_RET_LOG(dynamicExtensionsRegisterMapPtr != nullptr, {},
        "get dynamicMetadataGenExtensions fail!!");
    auto dynamicExtensionsRegisterMap = *dynamicExtensionsRegisterMapPtr;
    for (auto &reg : dynamicExtensionsRegisterMap) {
        if (reg.second != nullptr) {
            VPE_LOGD("Load extension set: %{public}s", reg.first.c_str());
            reg.second(reinterpret_cast<uintptr_t>(&extensionList));
        }
    }
    return extensionList;
}

ExtensionList ExtensionManager::LoadDynamicComposeExtensions() const
{
    ExtensionList extensionList {};
    CHECK_AND_RETURN_RET_LOG(g_algoHandle != nullptr, {}, "dlopen ext fail!");

    GetRegisterComposeExtensionFuncs = reinterpret_cast<LibFunctionGetRegisters>
        (dlsym(g_algoHandle, "GetRegisterComposeExtensionFuncs"));
    CHECK_AND_RETURN_RET_LOG(GetRegisterComposeExtensionFuncs != nullptr, {},
        "dlsym GetRegisterComposeExtensionFuncs fail!");

    auto dynamicExtensionsRegisterMapPtr = GetRegisterComposeExtensionFuncs();
    CHECK_AND_RETURN_RET_LOG(dynamicExtensionsRegisterMapPtr != nullptr, {},
        "get dynamicMetadataGenExtensions fail!!");
    auto dynamicExtensionsRegisterMap = *dynamicExtensionsRegisterMapPtr;
    for (auto &reg : dynamicExtensionsRegisterMap) {
        if (reg.second != nullptr) {
            VPE_LOGD("Load extension set: %{public}s", reg.first.c_str());
            reg.second(reinterpret_cast<uintptr_t>(&extensionList));
        }
    }
    return extensionList;
}

ExtensionList ExtensionManager::LoadDynamicDecomposeExtensions() const
{
    ExtensionList extensionList {};
    CHECK_AND_RETURN_RET_LOG(g_algoHandle != nullptr, {}, "dlopen ext fail, g_algoHandle null!");

    GetRegisterDecomposeExtensionFuncs = reinterpret_cast<LibFunctionGetRegisters>
        (dlsym(g_algoHandle, "GetRegisterDecomposeExtensionFuncs"));
    CHECK_AND_RETURN_RET_LOG(GetRegisterDecomposeExtensionFuncs != nullptr, {},
        "dlsym GetRegisterDecomposeExtensionFuncs fail!");

    auto dynamicExtensionsRegisterMapPtr = GetRegisterDecomposeExtensionFuncs();
    CHECK_AND_RETURN_RET_LOG(dynamicExtensionsRegisterMapPtr != nullptr, {},
        "get dynamicMetadataGenExtensions fail!!");
    auto dynamicExtensionsRegisterMap = *dynamicExtensionsRegisterMapPtr;
    for (auto &reg : dynamicExtensionsRegisterMap) {
        if (reg.second != nullptr) {
            VPE_LOGD("Load extension set: %{public}s", reg.first.c_str());
            reg.second(reinterpret_cast<uintptr_t>(&extensionList));
        }
    }
    return extensionList;
}

template<typename T>
T ExtensionManager::BuildCaps(const ExtensionList& extensionList) const
{
    VPEAlgoErrCode err = VPE_ALGO_ERR_OK;
    T capMap {};
    for (size_t idx = 0; idx < extensionList.size(); ++idx) {
        auto extension = extensionList[idx];
        CHECK_AND_RETURN_RET_LOG(extension != nullptr, {}, "Extension is nullptr");
        if constexpr (std::is_same_v<T, ColorSpaceConverterCapabilityMap>) {
            if (extension->info.type == ExtensionType::COLORSPACE_CONVERTER) {
                err = BuildColorSpaceConverterCaps(extension, idx, capMap);
                CHECK_AND_LOG(err == VPE_ALGO_ERR_OK, "Build caps failed, extension: %{public}s",
                    extension->info.name.c_str());
            }
        } else if constexpr (std::is_same_v<T, ColorSpaceConverterDisplayCapabilityMap>) {
            if (extension->info.type == ExtensionType::COLORSPACE_CONVERTER_DISPLAY) {
                err = BuildColorSpaceConverterDisplayCaps(extension, idx, capMap);
                CHECK_AND_LOG(err == VPE_ALGO_ERR_OK, "Build caps failed, extension: %{public}s",
                    extension->info.name.c_str());
            }
        } else if constexpr (std::is_same_v<T, MetadataGeneratorCapabilityMap>) {
            if (extension->info.type == ExtensionType::METADATA_GENERATOR) {
                err = BuildMetadataGeneratorCaps(extension, idx, capMap);
                CHECK_AND_LOG(err == VPE_ALGO_ERR_OK, "Build caps failed, extension: %{public}s",
                    extension->info.name.c_str());
            }
        } else if constexpr (std::is_same_v<T, DetailEnhancerCapabilityMap>) {
            if (extension->info.type == ExtensionType::DETAIL_ENHANCER) {
                err = BuildDetailEnhancerCaps(extension, idx, capMap);
                CHECK_AND_LOG(err == VPE_ALGO_ERR_OK, "Build caps failed, extension: %{public}s",
                    extension->info.name.c_str());
            }
        } else if constexpr (std::is_same_v<T, AihdrEnhancerCapabilityMap>) {
            if (extension->info.type == ExtensionType::AIHDR_ENHANCER) {
                err = BuildAihdrEnhancerCaps(extension, idx, capMap);
                CHECK_AND_LOG(err == VPE_ALGO_ERR_OK, "Build caps failed, extension: %{public}s",
                    extension->info.name.c_str());
            }
        } else {
            VPE_LOGE("Unknown extension type");
            return {};
        }
    }

    return capMap;
}

std::shared_ptr<ColorSpaceConverterExtension> ExtensionManager::FindColorSpaceConverterExtension(
    const FrameInfo &inputInfo, const FrameInfo &outputInfo) const
{
    VPE_LOG_PRINT_COLOR_SPACE_CAPBILITY(inputInfo.colorSpace.colorSpaceInfo, inputInfo.pixelFormat);
    VPE_LOG_PRINT_COLOR_SPACE_CAPBILITY(outputInfo.colorSpace.colorSpaceInfo, outputInfo.pixelFormat);
    auto extensionList = LoadExtensions();
    CHECK_AND_RETURN_RET_LOG(!extensionList.empty(), nullptr, "No extension found");
    auto colorSpaceConverterCapabilityMap = BuildCaps<ColorSpaceConverterCapabilityMap>(extensionList);
    CHECK_AND_RETURN_RET_LOG(!colorSpaceConverterCapabilityMap.empty(), nullptr, "No extension available");
    auto key =
        std::make_tuple(inputInfo.colorSpace, inputInfo.pixelFormat, outputInfo.colorSpace, outputInfo.pixelFormat);
    const auto iter = colorSpaceConverterCapabilityMap.find(key);
    if (iter == colorSpaceConverterCapabilityMap.cend() || iter->second.empty()) {
        VPE_LOGE("CSC Extension is not found");
        CM_ColorSpaceInfo CSDesc = inputInfo.colorSpace.colorSpaceInfo;
        VPE_LOGE("input: (primary=%{public}3d,trans=%{public}3d,mat=%{public}3d,range=%{public}3d,"\
            "pixel=%{public}3d,metadata=%{public}3d", (CSDesc).primaries, (CSDesc).transfunc, (CSDesc).matrix,
            (CSDesc).range, inputInfo.pixelFormat, inputInfo.colorSpace.metadataType);
        CSDesc = outputInfo.colorSpace.colorSpaceInfo;
        VPE_LOGE("output: (primary=%{public}3d,trans=%{public}3d,mat=%{public}3d,range=%{public}3d,"\
            "pixel=%{public}3d,metadata=%{public}3d", (CSDesc).primaries, (CSDesc).transfunc, (CSDesc).matrix,
            (CSDesc).range, outputInfo.pixelFormat, outputInfo.colorSpace.metadataType);
        return nullptr;
    }
    size_t idx = std::get<2>(*(iter->second.cbegin()));
    for (const auto &cap : iter->second) {
        if (std::get<0>(cap) == Rank::RANK_HIGH) {
            idx = std::get<2>(cap);
            break;
        }
    }
    return std::static_pointer_cast<ColorSpaceConverterExtension>(extensionList[idx]);
}

ColorSpaceConverterDisplayExtensionSet ExtensionManager::FindColorSpaceConverterDisplayExtension() const
{
    auto extensionList = LoadExtensions();
    CHECK_AND_RETURN_RET_LOG(!extensionList.empty(), {}, "No extension found");
    ColorSpaceConverterDisplayExtensionSet extensions {};
    for (const auto &extension : extensionList) {
        CHECK_AND_CONTINUE_LOG(extension != nullptr, "Get an empty extension");
        if (extension->info.type != ExtensionType::COLORSPACE_CONVERTER_DISPLAY) {
            continue;
        }
        extensions.emplace(std::static_pointer_cast<ColorSpaceConverterDisplayExtension>(extension));
    }
    return extensions;
}

std::shared_ptr<MetadataGeneratorExtension> ExtensionManager::FindMetadataGeneratorExtension(const FrameInfo &inputInfo,
    MetadataGeneratorAlgoType algoType) const
{
    auto extensionList = LoadExtensions();
    CHECK_AND_RETURN_RET_LOG(!extensionList.empty(), nullptr, "No extension found");
    auto metadataGeneratorCapabilityMap = BuildCaps<MetadataGeneratorCapabilityMap>(extensionList);
    CHECK_AND_RETURN_RET_LOG(!metadataGeneratorCapabilityMap.empty(), nullptr, "No extension available");
    auto key = std::make_tuple(inputInfo.colorSpace, inputInfo.pixelFormat, algoType);
    VPE_LOG_PRINT_METADATA_GEN_CAPBILITY(inputInfo.colorSpace.colorSpaceInfo, inputInfo.pixelFormat, algoType);
    const auto iter = metadataGeneratorCapabilityMap.find(key);
    CHECK_AND_RETURN_RET_LOG(iter != metadataGeneratorCapabilityMap.cend() && !iter->second.empty(), nullptr,
        "CSC metadata generator extension is not found");
    size_t idx = std::get<2>(*(iter->second.cbegin()));
    for (const auto &cap : iter->second) {
        if (std::get<0>(cap) == Rank::RANK_HIGH) {
            idx = std::get<2>(cap);
            break;
        }
    }
    return std::static_pointer_cast<MetadataGeneratorExtension>(extensionList[idx]);
}

VPEAlgoErrCode ExtensionManager::BuildColorSpaceConverterCaps(const std::shared_ptr<ExtensionBase> &ext, size_t idx,
    ColorSpaceConverterCapabilityMap& colorSpaceConverterCapabilityMap) const
{
    VPEAlgoErrCode err = VPE_ALGO_ERR_OK;
    auto realExtension = std::static_pointer_cast<ColorSpaceConverterExtension>(ext);
    auto capabilities = realExtension->capabilitiesBuilder();
    for (const auto &cap : capabilities) {
        err = ExtractColorSpaceConverterCap(cap, idx, colorSpaceConverterCapabilityMap);
    }
    return err;
}

std::shared_ptr<DetailEnhancerExtension> ExtensionManager::FindDetailEnhancerExtension(uint32_t level) const
{
    auto extensionList = LoadExtensions();
    CHECK_AND_RETURN_RET_LOG(!extensionList.empty(), nullptr, "No extension found");
    auto detailEnhancerCapabilityMap = BuildCaps<DetailEnhancerCapabilityMap>(extensionList);
    CHECK_AND_RETURN_RET_LOG(!detailEnhancerCapabilityMap.empty(), nullptr, "No extension available");
    const auto iter = detailEnhancerCapabilityMap.find(level);
    CHECK_AND_RETURN_RET_LOG(iter != detailEnhancerCapabilityMap.cend(), nullptr,
        "Detail enhancer Extension is not found");
    size_t idx = iter->second;
    return std::static_pointer_cast<DetailEnhancerExtension>(extensionList[idx]);
}

std::shared_ptr<AihdrEnhancerExtension> ExtensionManager::FindAihdrEnhancerExtension(const FrameInfo &inputInfo) const
{
    auto extensionList = LoadExtensions();
    CHECK_AND_RETURN_RET_LOG(!extensionList.empty(), nullptr, "No extension found");
    auto aihdrEnhancerCapabilityMap = BuildCaps<AihdrEnhancerCapabilityMap>(extensionList);
    CHECK_AND_RETURN_RET_LOG(!aihdrEnhancerCapabilityMap.empty(), nullptr, "No extension available");
    auto key = std::make_tuple(inputInfo.colorSpace, inputInfo.pixelFormat);
    const auto iter = aihdrEnhancerCapabilityMap.find(key);
    CHECK_AND_RETURN_RET_LOG(iter != aihdrEnhancerCapabilityMap.cend() && !iter->second.empty(), nullptr,
        "Aihdr enhancer extension is not found");
    size_t idx = std::get<2>(*(iter->second.cbegin()));
    for (const auto &cap : iter->second) {
        if (std::get<0>(cap) == Rank::RANK_HIGH) {
            idx = std::get<2>(cap);
            break;
        }
    }
    return std::static_pointer_cast<AihdrEnhancerExtension>(extensionList[idx]);
}

VPEAlgoErrCode ExtensionManager::ExtractColorSpaceConverterCap(const ColorSpaceConverterCapability& cap, size_t idx,
    ColorSpaceConverterCapabilityMap& colorSpaceConverterCapabilityMap) const
{
    auto inputColorSpaceDesc = cap.inputColorSpaceDesc;
    auto outputColorSpaceDesc = cap.outputColorSpaceDesc;
    Rank rank = cap.rank;
    int32_t version = cap.version;
    for (const auto &[inputPixelFormat, outputPixelFormats] : cap.pixelFormatMap) {
        for (const auto &outputPixelFormat : outputPixelFormats) {
            auto key = std::make_tuple(inputColorSpaceDesc, inputPixelFormat, outputColorSpaceDesc, outputPixelFormat);
            VPE_LOG_PRINT_COLOR_SPACE_CAPBILITY(inputColorSpaceDesc.colorSpaceInfo, inputPixelFormat);
            VPE_LOG_PRINT_COLOR_SPACE_CAPBILITY(outputColorSpaceDesc.colorSpaceInfo, outputPixelFormat);
            auto value = std::make_tuple(rank, version, idx);
            colorSpaceConverterCapabilityMap[key].push_back(value);
        }
    }
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode ExtensionManager::BuildColorSpaceConverterDisplayCaps(const std::shared_ptr<ExtensionBase> &ext,
    size_t idx, ColorSpaceConverterDisplayCapabilityMap& colorSpaceConverterDisplayCapabilityMap) const
{
    (void)ext;
    (void)idx;
    (void)colorSpaceConverterDisplayCapabilityMap;
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode ExtensionManager::ExtractMetadataGeneratorCap(const MetadataGeneratorCapability &cap, size_t idx,
    MetadataGeneratorAlgoType algoType, MetadataGeneratorCapabilityMap& metadataGeneratorCapabilityMap) const
{
    auto colorSpaceDesc = cap.colorspaceDesc;
    Rank rank = cap.rank;
    int32_t version = cap.version;
    for (const auto &pixelFormat : cap.pixelFormats) {
        auto key = std::make_tuple(colorSpaceDesc, pixelFormat, algoType);
        VPE_LOG_PRINT_METADATA_GEN_CAPBILITY(colorSpaceDesc.colorSpaceInfo, pixelFormat, algoType);
        auto value = std::make_tuple(rank, version, idx);
        metadataGeneratorCapabilityMap[key].push_back(value);
    }
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode ExtensionManager::BuildMetadataGeneratorCaps(const std::shared_ptr<ExtensionBase> &ext, size_t idx,
    MetadataGeneratorCapabilityMap& metadataGeneratorCapabilityMap) const
{
    VPEAlgoErrCode err = VPE_ALGO_ERR_OK;
    auto realExtension = std::static_pointer_cast<MetadataGeneratorExtension>(ext);
    auto capabilities = realExtension->capabilitiesBuilder();
    MetadataGeneratorAlgoType algoType = MetadataGeneratorAlgoType::META_GEN_ALGO_TYPE_IMAGE;
    if (ext->info.name == "VideoMetadataGen") {
        algoType = MetadataGeneratorAlgoType::META_GEN_ALGO_TYPE_VIDEO;
    }
    for (const auto &cap : capabilities) {
        err = ExtractMetadataGeneratorCap(cap, idx, algoType, metadataGeneratorCapabilityMap);
    }
    return err;
}

VPEAlgoErrCode ExtensionManager::BuildDetailEnhancerCaps(const std::shared_ptr<ExtensionBase>& ext, size_t idx,
    DetailEnhancerCapabilityMap& detailEnhancerCapabilityMap) const
{
    auto realExtension = std::static_pointer_cast<DetailEnhancerExtension>(ext);
    auto capabilities = realExtension->capabilitiesBuilder();
    for (const auto &level : capabilities.levels) {
        auto itr = detailEnhancerCapabilityMap.find(level);
        if (itr == detailEnhancerCapabilityMap.end()) {
            detailEnhancerCapabilityMap.emplace(level, idx);
        } else {
            if (capabilities.rank == Rank::RANK_HIGH) {
                itr->second = idx;
            }
        }
    }
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode ExtensionManager::ExtractAihdrEnhancerCap(const AihdrEnhancerCapability &cap, size_t idx,
    AihdrEnhancerCapabilityMap& aihdrEnhancerCapabilityMap) const
{
    auto colorSpaceDesc = cap.colorspaceDesc;
    Rank rank = cap.rank;
    int32_t version = cap.version;
    for (const auto &pixelFormat : cap.pixelFormats) {
        auto key = std::make_tuple(colorSpaceDesc, pixelFormat);
        auto value = std::make_tuple(rank, version, idx);
        aihdrEnhancerCapabilityMap[key].push_back(value);
    }
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode ExtensionManager::BuildAihdrEnhancerCaps(const std::shared_ptr<ExtensionBase> &ext, size_t idx,
    AihdrEnhancerCapabilityMap& aihdrEnhancerCapabilityMap) const
{
    VPEAlgoErrCode err = VPE_ALGO_ERR_OK;
    auto realExtension = std::static_pointer_cast<AihdrEnhancerExtension>(ext);
    auto capabilities = realExtension->capabilitiesBuilder();
    for (const auto &cap : capabilities) {
        err = ExtractAihdrEnhancerCap(cap, idx, aihdrEnhancerCapabilityMap);
    }
    return err;
}
} // namespace Extension
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
