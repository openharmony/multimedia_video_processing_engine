/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "detail_enhancer_image_fwk.h"

#include <dlfcn.h>

#include "detail_enhancer_common.h"
#include "extension_manager.h"
#include "native_buffer.h"
#include "surface_buffer.h"
#include "video_processing_client.h"
#include "vpe_log.h"
#include "vpe_trace.h"

namespace {
enum RectLevelItem {
    RECT_LEVEL_INVALID = -1,
    RECT_MIN_WIDTH,
    RECT_MAX_WIDTH,
    RECT_MIN_HEIGHT,
    RECT_MAX_HEIGHT,
    RECT_LEVEL_NUM,
};

constexpr float EPSILON = 1e-6; // extremely small value
const int MAX_URL_LENGTH = 100;
const int SUPPORTED_MIN_WIDTH = 32;
const int SUPPORTED_MIN_HEIGHT = 32;
const int SUPPORTED_MAX_WIDTH = 8192;
const int SUPPORTED_MAX_HEIGHT = 8192;
const int TIMEOUT_THRESHOLD = 10; // 10 millisecond
const std::unordered_set<int32_t> SUPPORTED_FORMATS = {
    OHOS::GRAPHIC_PIXEL_FMT_BGRA_8888, // BGRA
    OHOS::GRAPHIC_PIXEL_FMT_RGBA_8888, // RGBA
    OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_SP, // NV12
    OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_SP, // NV21
    OHOS::GRAPHIC_PIXEL_FMT_YCBCR_420_P, // YU12
    OHOS::GRAPHIC_PIXEL_FMT_YCRCB_420_P, // YV12
    OHOS::GRAPHIC_PIXEL_FMT_RGBA_1010102, // RGBA_1010102
};
const std::vector<std::array<int, RECT_LEVEL_NUM>> SUPER_LEVEL_TARGET_RECT = {
    {1, 1104, 1, 848},
    {1, 1104, 1, 1488},
    {1, 1488, 1, 1104},
    {1, 1872, 1, 1360},
};

inline bool IsValidSurfaceBuffer(const OHOS::sptr<OHOS::SurfaceBuffer>& buffer)
{
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, false, "buffer is nullptr!!");
    return SUPPORTED_FORMATS.find(buffer->GetFormat()) != SUPPORTED_FORMATS.end() &&
        buffer->GetWidth() > SUPPORTED_MIN_WIDTH && buffer->GetHeight() > SUPPORTED_MIN_HEIGHT &&
        buffer->GetWidth() <= SUPPORTED_MAX_WIDTH && buffer->GetHeight() <= SUPPORTED_MAX_HEIGHT;
}

std::atomic<int32_t> g_instanceId = -1;
std::timed_mutex g_externLock{};
}

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
DetailEnhancerImageFwk::DetailEnhancerImageFwk(int type)
{
    type_ = (type >= IMAGE && type <= VIDEO) ? type : IMAGE;
    Extension::ExtensionManager::GetInstance().IncreaseInstance();
}

DetailEnhancerImageFwk::~DetailEnhancerImageFwk()
{
    algorithms_.clear();
    Extension::ExtensionManager::GetInstance().DecreaseInstance();
}

std::shared_ptr<DetailEnhancerImage> DetailEnhancerImage::Create(int type)
{
    CHECK_AND_RETURN_RET_LOG(type <= VIDEO && type >= IMAGE, nullptr, "type is invalid!!");
    std::shared_ptr<DetailEnhancerImage> impl = std::make_shared<DetailEnhancerImageFwk>(type);
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr, "failed to init DetailEnhancerImage");
    return impl;
}

std::shared_ptr<DetailEnhancerBase> DetailEnhancerImageFwk::GetAlgorithm(int level)
{
    if (level < DETAIL_ENH_LEVEL_NONE || level > DETAIL_ENH_LEVEL_VIDEO) {
        VPE_LOGE("Invalid level:%{public}d", level);
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(lock_);
    auto createdImpl = algorithms_.find(level);
    if (createdImpl != algorithms_.end()) [[likely]] {
        return createdImpl->second;
    }
    algorithms_[level] = CreateAlgorithm(level);
    return algorithms_[level];
}

std::shared_ptr<DetailEnhancerBase> DetailEnhancerImageFwk::CreateAlgorithm(int level)
{
    auto& manager = Extension::ExtensionManager::GetInstance();
    VPE_SYNC_TRACE;
    std::shared_ptr<DetailEnhancerBase> algoImpl = manager.CreateDetailEnhancer(level);
    if (algoImpl == nullptr) {
        VPE_LOGE("Extension create failed, get a empty impl, level: %{public}d", level);
        return nullptr;
    }
    if (algoImpl->Init() != VPE_ALGO_ERR_OK) {
        VPE_LOGE("Init failed, extension level: %{public}d", level);
        return nullptr;
    }
    return algoImpl;
}

VPEAlgoErrCode DetailEnhancerImageFwk::SetParameter(const DetailEnhancerParameters& parameter)
{
    CHECK_AND_RETURN_RET_LOG(parameter.level >= DETAIL_ENH_LEVEL_NONE && parameter.level <= DETAIL_ENH_LEVEL_HIGH &&
        parameter.uri.length() < MAX_URL_LENGTH, VPE_ALGO_ERR_INVALID_VAL, "Invalid parameter");
    std::lock_guard<std::mutex> lock(lock_);
    parameter_ = parameter;
    parameterUpdated = true;
    VPE_LOGI("DetailEnhancerImageFwk SetParameter Succeed");
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode DetailEnhancerImageFwk::GetParameter(DetailEnhancerParameters& parameter) const
{
    std::lock_guard<std::mutex> lock(lock_);
    parameter = parameter_;
    VPE_LOGI("DetailEnhancerImageFwk SetParameter Succeed");
    return VPE_ALGO_ERR_OK;
}

bool DetailEnhancerImageFwk::IsValidProcessedObject(const sptr<SurfaceBuffer>& input,
    const sptr<SurfaceBuffer>& output)
{
    CHECK_AND_RETURN_RET_LOG((input != nullptr) && (output != nullptr),
        false, "Input or output is nullptr");
    CHECK_AND_RETURN_RET_LOG(input->GetFormat() == output->GetFormat(), false,
        "The input format and output format need to be consistent");
    CHECK_AND_RETURN_RET_LOG(IsValidSurfaceBuffer(input) && IsValidSurfaceBuffer(output), false, "Invalid buffer");
    return true;
}

int DetailEnhancerImageFwk::EvaluateTargetLevel(const sptr<SurfaceBuffer>& input,
    const sptr<SurfaceBuffer>& output, float widthRatio, float heightRatio) const
{
    CHECK_AND_RETURN_RET_LOG((input != nullptr) && (output != nullptr), false, "Input or output is nullptr");
    if (parameter_.level == DETAIL_ENH_LEVEL_HIGH) {
        int inputW = input->GetWidth();
        int inputH = input->GetHeight();
        if (widthRatio < 1.0 && heightRatio < 1.0 && // 1.0 means zoom out
            // 0.5 means rounding, 2 means two pixels
            std::abs(static_cast<int>(widthRatio * inputW + 0.5) - static_cast<int>(heightRatio * inputW + 0.5)) <= 2 &&
            // 0.5 means rounding, 2 means two pixels
            std::abs(static_cast<int>(widthRatio * inputH + 0.5) - static_cast<int>(heightRatio * inputH + 0.5)) <= 2) {
            VPE_LOGI("Prioritize using extream vision algo when scaling down scenes");
            return DETAIL_ENH_LEVEL_HIGH;
        }
        return DETAIL_ENH_LEVEL_HIGH_AISR;
    }
    return parameter_.level;
}

VPEAlgoErrCode DetailEnhancerImageFwk::ProcessVideo(const sptr<SurfaceBuffer>& input,
    const sptr<SurfaceBuffer>& output, bool flag)
{
    auto algoImpl = GetAlgorithm(DETAIL_ENH_LEVEL_VIDEO);
    if (algoImpl == nullptr) {
        VPE_LOGE("Get Algorithm impl for video failed!");
        return VPE_ALGO_ERR_UNKNOWN;
    }
    if (parameterUpdated.load() && (algoImpl->SetParameter(parameter_, type_, flag) !=  VPE_ALGO_ERR_OK)) {
        VPE_LOGE("set parameter failed!");
        return VPE_ALGO_ERR_UNKNOWN;
    } else {
        parameterUpdated = false;
    }
    if (algoImpl->Process(input, output) != VPE_ALGO_ERR_OK) {
        VPE_LOGE("process video failed");
        return VPE_ALGO_ERR_UNKNOWN;
    }
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode DetailEnhancerImageFwk::Process(const sptr<SurfaceBuffer>& input, const sptr<SurfaceBuffer>& output,
    bool flag)
{
    CHECK_AND_RETURN_RET_LOG(IsValidProcessedObject(input, output), VPE_ALGO_ERR_INVALID_VAL,
        "Invalid processd object");
    VPE_SYNC_TRACE;
    if (parameter_.forceEve) {
        auto algoImpl = GetAlgorithm(DETAIL_ENH_LEVEL_HIGH);
        CHECK_AND_RETURN_RET_LOG(algoImpl != nullptr && algoImpl->SetParameter(parameter_, type_, flag) ==
            VPE_ALGO_ERR_OK, VPE_ALGO_ERR_UNKNOWN, "set parameter failed!");
        return algoImpl->Process(input, output);
    }
    if (type_ == VIDEO) {
        return ProcessVideo(input, output, flag);
    }
    float widthRatio = static_cast<float>(output->GetWidth()) / static_cast<float>(input->GetWidth());
    float heightRatio = static_cast<float>(output->GetHeight()) / static_cast<float>(input->GetHeight());
    int targetLevel = EvaluateTargetLevel(input, output, widthRatio, heightRatio);
    if (targetLevel < DETAIL_ENH_LEVEL_HIGH_AISR &&
        std::fabs(widthRatio - 1.0f) < EPSILON && std::fabs(heightRatio - 1.0f) < EPSILON) {
        VPE_LOGI("The current scaling ratio is 1.0, and the algorithm is not AISR, so copy it directly.");
        return (memcpy_s(output->GetVirAddr(), output->GetSize(), input->GetVirAddr(), input->GetSize()) == EOK) ?
            VPE_ALGO_ERR_OK : VPE_ALGO_ERR_UNKNOWN;
    }
    bool processSuccessfully = false;
    for (int level = targetLevel; level >= DETAIL_ENH_LEVEL_NONE; level--) {
        auto algoImpl = GetAlgorithm(level);
        if (algoImpl == nullptr) {
            VPE_LOGE("Get Algorithm impl for %{public}d failed!", level);
            continue;
        }
        parameter_.level = static_cast<DetailEnhancerLevel>((level == DETAIL_ENH_LEVEL_HIGH_AISR) ?
            DETAIL_ENH_LEVEL_HIGH : level); // map level
        if (algoImpl->SetParameter(parameter_, type_, flag) !=  VPE_ALGO_ERR_OK) {
            VPE_LOGE("set parameter failed!");
            return VPE_ALGO_ERR_UNKNOWN;
        }
        if (algoImpl->Process(input, output) == VPE_ALGO_ERR_OK) {
            processSuccessfully = true;
            break;
        } else if (level == DETAIL_ENH_LEVEL_HIGH_AISR) {
            VPE_LOGD("AISR processed failed, try to process by EVE");
        } else if (level > DETAIL_ENH_LEVEL_NONE) {
            VPE_LOGW("Failed to process with level %{public}d", level);
        } else {
            VPE_LOGE("Failed to process with detail enhancer");
            return VPE_ALGO_ERR_UNKNOWN;
        }
    }
    return processSuccessfully ? VPE_ALGO_ERR_OK : VPE_ALGO_ERR_INVALID_VAL;
}

int32_t DetailEnhancerCreate(int32_t* instance)
{
    CHECK_AND_RETURN_RET_LOG(g_externLock.try_lock_for(std::chrono::milliseconds(TIMEOUT_THRESHOLD)),
        VPE_ALGO_ERR_INVALID_VAL, "get lock timeout");
    if (instance == nullptr) {
        VPE_LOGE("invalid instance");
        g_externLock.unlock();
        return VPE_ALGO_ERR_INVALID_VAL;
    }
    if (g_instanceId != -1) {
        // if there is an instance, return it
        *instance = g_instanceId;
        g_externLock.unlock();
        return VPE_ALGO_ERR_OK;
    }
    auto detailEnh = DetailEnhancerImage::Create();
    if (detailEnh == nullptr) {
        VPE_LOGE("cannot create instance");
        g_externLock.unlock();
        return VPE_ALGO_ERR_INVALID_VAL;
    }
    Extension::ExtensionManager::InstanceVariableType instanceVar { detailEnh };
    int32_t newId = Extension::ExtensionManager::GetInstance().NewInstanceId(instanceVar);
    if (newId == -1) {
        VPE_LOGE("cannot create more instance");
        g_externLock.unlock();
        return VPE_ALGO_ERR_NO_MEMORY;
    }
    *instance = newId;
    g_instanceId = newId;
    g_externLock.unlock();
    return VPE_ALGO_ERR_OK;
}

sptr<SurfaceBuffer> CreateSurfaceBufFromNativeWindow(OHNativeWindowBuffer* image)
{
    OH_NativeBuffer* imageNativeBuffer = nullptr;
    CHECK_AND_RETURN_RET_LOG(OH_NativeBuffer_FromNativeWindowBuffer(image, &imageNativeBuffer) == GSERROR_OK,
        nullptr, "invalid input or output image");
    sptr<SurfaceBuffer> imageSurfaceBuffer(SurfaceBuffer::NativeBufferToSurfaceBuffer(imageNativeBuffer));
    return imageSurfaceBuffer;
}

int32_t DetailEnhancerProcessImage(int32_t instance, OHNativeWindowBuffer* inputImage,
    OHNativeWindowBuffer* outputImage, int32_t level)
{
    CHECK_AND_RETURN_RET_LOG(g_externLock.try_lock_for(std::chrono::milliseconds(TIMEOUT_THRESHOLD)),
        VPE_ALGO_ERR_INVALID_VAL, "get lock timeout");
    if (inputImage == nullptr || outputImage == nullptr) {
        VPE_LOGE("invalid parameters");
        g_externLock.unlock();
        return VPE_ALGO_ERR_INVALID_VAL;
    }
    auto someInstance = Extension::ExtensionManager::GetInstance().GetInstance(instance);
    if (someInstance == std::nullopt) {
        VPE_LOGE("invalid instance");
        g_externLock.unlock();
        return VPE_ALGO_ERR_INVALID_VAL;
    }
    VPEAlgoErrCode ret = VPE_ALGO_ERR_INVALID_VAL;
    auto visitFunc = [inputImage, outputImage, &ret, &level](auto&& var) {
        using VarType = std::decay_t<decltype(var)>;
        if constexpr (std::is_same_v<VarType, std::shared_ptr<DetailEnhancerImage>>) {
            sptr<SurfaceBuffer> inputImageSurfaceBuffer = CreateSurfaceBufFromNativeWindow(inputImage);
            sptr<SurfaceBuffer> outputImageSurfaceBuffer = CreateSurfaceBufFromNativeWindow(outputImage);
            DetailEnhancerParameters param {
                .uri = "",
                .level = static_cast<DetailEnhancerLevel>(level),
            };
            var->SetParameter(param);
            ret = var->Process(inputImageSurfaceBuffer, outputImageSurfaceBuffer);
        } else {
            VPE_LOGE("instance may be miss used");
        }
    };
    std::visit(visitFunc, *someInstance);
    g_externLock.unlock();
    return ret;
}

int32_t DetailEnhancerDestroy(int32_t* instance)
{
    CHECK_AND_RETURN_RET_LOG(instance != nullptr, VPE_ALGO_ERR_INVALID_VAL, "instance is null");
    int ret = Extension::ExtensionManager::GetInstance().RemoveInstanceReference(*instance);
    if (ret == VPE_ALGO_ERR_OK) {
        g_instanceId = -1;
    }
    return ret;
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
