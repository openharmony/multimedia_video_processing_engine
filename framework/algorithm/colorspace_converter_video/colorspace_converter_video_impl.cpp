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

#include "colorspace_converter_video_impl.h"
#include <cstring>
#include <limits>
#include <memory>
#include <algorithm>
#include "vpe_log.h"
#include "algorithm_errors.h"
#include "colorspace_converter_video_description.h"
#include "algorithm_common.h"
#include "v1_0/buffer_handle_meta_key_type.h"
#include "v1_0/cm_color_space.h"
#include "v1_0/hdr_static_metadata.h"
#include "vpe_trace.h"
#include "securec.h"
#include "extension_manager.h"
#include "meta/meta_key.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
std::shared_ptr<ColorSpaceConverterVideo> ColorSpaceConverterVideo::Create()
{
    std::shared_ptr<ColorSpaceConverterVideoImpl> impl = std::make_shared<ColorSpaceConverterVideoImpl>();
    int32_t ret = impl->Init();
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, nullptr, "failed to init ColorSpaceConverterVideoImpl");
    return impl;
}

std::shared_ptr<ColorSpaceConverterVideo> ColorSpaceConverterVideo::Create(std::shared_ptr<OpenGLContext> openglContext)
{
    std::shared_ptr<ColorSpaceConverterVideoImpl> impl = std::make_shared<ColorSpaceConverterVideoImpl>();
    int32_t ret = impl->Init(openglContext);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, nullptr, "failed to init ColorSpaceConverterVideoImpl");
    return impl;
}

ColorSpaceConverterVideoImpl::ColorSpaceConverterVideoImpl()
{
    requestCfg_.timeout = 0;
    requestCfg_.strideAlignment = 32;  // 32 byte alignment
    requestCfg_.usage =
        BUFFER_USAGE_CPU_READ | BUFFER_USAGE_CPU_WRITE | BUFFER_USAGE_HW_RENDER | BUFFER_USAGE_HW_TEXTURE;
    requestCfg_.format = 0;
    requestCfg_.width = 0;
    requestCfg_.height = 0;

    flushCfg_.timestamp = 0;
    flushCfg_.damage.x = 0;
    flushCfg_.damage.y = 0;
    flushCfg_.damage.w = 0;
    flushCfg_.damage.h = 0;
}

ColorSpaceConverterVideoImpl::~ColorSpaceConverterVideoImpl()
{
    Release();
}

int32_t ColorSpaceConverterVideoImpl::Init()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(
        state_ == VPEAlgoState::UNINITIALIZED, VPE_ALGO_ERR_INVALID_STATE, "Init failed: not in UNINITIALIZED state");

    csc_ = ColorSpaceConverter::Create();
    CHECK_AND_RETURN_RET_LOG(csc_ != nullptr, VPE_ALGO_ERR_UNKNOWN, "ColorSpaceConverter Create failed");

    isRunning_.store(true);
    taskThread_ = std::make_shared<std::thread>(&ColorSpaceConverterVideoImpl::OnTriggered, this);
    CHECK_AND_RETURN_RET_LOG(taskThread_ != nullptr, VPE_ALGO_ERR_UNKNOWN, "Fatal: No memory");

    state_ = VPEAlgoState::INITIALIZED;
    return VPE_ALGO_ERR_OK;
}

int32_t ColorSpaceConverterVideoImpl::Init(std::shared_ptr<OpenGLContext> openglContext)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(
        state_ == VPEAlgoState::UNINITIALIZED, VPE_ALGO_ERR_INVALID_STATE, "Init failed: not in UNINITIALIZED state");

    csc_ = ColorSpaceConverter::Create(openglContext);
    CHECK_AND_RETURN_RET_LOG(csc_ != nullptr, VPE_ALGO_ERR_UNKNOWN, "ColorSpaceConverter Create failed");

    isRunning_.store(true);
    taskThread_ = std::make_shared<std::thread>(&ColorSpaceConverterVideoImpl::OnTriggered, this);
    CHECK_AND_RETURN_RET_LOG(taskThread_ != nullptr, VPE_ALGO_ERR_UNKNOWN, "Fatal: No memory");

    state_ = VPEAlgoState::INITIALIZED;
    return VPE_ALGO_ERR_OK;
}

int32_t ColorSpaceConverterVideoImpl::SetCallback(const std::shared_ptr<ColorSpaceConverterVideoCallback> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, VPE_ALGO_ERR_INVALID_VAL, "Set callback failed: callback is NULL");
    CHECK_AND_RETURN_RET_LOG(state_ == VPEAlgoState::INITIALIZED || state_ == VPEAlgoState::CONFIGURING,
        VPE_ALGO_ERR_INVALID_STATE, "SetCallback failed: not in INITIALIZED or CONFIGURING state");
    cb_ = callback;
    state_ = VPEAlgoState::CONFIGURING;
    return VPE_ALGO_ERR_OK;
}

int32_t ColorSpaceConverterVideoImpl::AttachToNewSurface(sptr<Surface> newSurface)
{
    std::lock_guard<std::mutex> lockrender(renderQueMutex_);
     for (auto it = outputBufferAvilQueBak_.begin(); it != outputBufferAvilQueBak_.end(); ++it) {
        auto buffer = it->second;
        GSError err = newSurface->AttachBufferToQueue(buffer->memory);
        CHECK_AND_RETURN_RET_LOG(err == GSERROR_OK, VPE_ALGO_ERR_UNKNOWN, "outputbuffer AttachToNewSurface fail");
    }
    return VPE_ALGO_ERR_OK;
}

int32_t ColorSpaceConverterVideoImpl::GetReleaseOutBuffer()
{
    std::lock_guard<std::mutex> mapLock(renderQueMutex_);
    for (RenderBufferAvilMapType::iterator it = renderBufferMapBak_.begin(); it != renderBufferMapBak_.end(); ++it) {
        outputBufferAvilQue_.push(it->second);
    }
    renderBufferMapBak_.clear();
    return VPE_ALGO_ERR_OK;
}

int32_t ColorSpaceConverterVideoImpl::SetOutputSurfaceConfig(sptr<Surface> surface)
{
    GSError err = surface->RegisterReleaseListener([this](sptr<SurfaceBuffer> &buffer) {
        (void)buffer;
        return OnProducerBufferReleased();
    });
    CHECK_AND_RETURN_RET_LOG(err == GSERROR_OK, VPE_ALGO_ERR_UNKNOWN, "RegisterReleaseListener fail");
    surface->SetQueueSize(outBufferCnt_);
    outputSurface_ = surface;
    state_ = VPEAlgoState::CONFIGURING;
    return VPE_ALGO_ERR_OK;
}

int32_t ColorSpaceConverterVideoImpl::SetOutputSurfaceRunning(sptr<Surface> newSurface)
{
    std::lock_guard<std::mutex> lockSurface(surfaceChangeMutex_);
    std::lock_guard<std::mutex> lockSurface2(surfaceChangeMutex2_);
    uint64_t oldId = outputSurface_->GetUniqueId();
    uint64_t newId = newSurface->GetUniqueId();
    if (oldId == newId) {
        VPE_LOGD("SetOutputSurfaceRunning same surface");
        return VPE_ALGO_ERR_OK;
    }

    outputSurface_->UnRegisterReleaseListener();
    outputSurface_->CleanCache(true);
    GSError err = newSurface->RegisterReleaseListener([this](sptr<SurfaceBuffer> &buffer) {
        (void)buffer;
        return OnProducerBufferReleased();
    });
    CHECK_AND_RETURN_RET_LOG(err == GSERROR_OK, VPE_ALGO_ERR_UNKNOWN, "RegisterReleaseListener fail");
    newSurface->SetQueueSize(outBufferCnt_);
    newSurface->Connect();
    newSurface->CleanCache();
    GetReleaseOutBuffer();
    int32_t ret = AttachToNewSurface(newSurface);
    if (ret != VPE_ALGO_ERR_OK) {
        return ret;
    }

    GraphicTransformType inTransform;
    ScalingMode inScaleMode;
    inTransform = inputSurface_->GetTransform();
    outputSurface_ = newSurface;
    err = outputSurface_->SetTransform(inTransform);
    CHECK_AND_RETURN_RET_LOG(err == GSERROR_OK, VPE_ALGO_ERR_UNKNOWN, "SetTransform fail");
    if (lastSurfaceSequence_ != MAX_SURFACE_SEQUENCE) {
        err = inputSurface_->GetScalingMode(lastSurfaceSequence_, inScaleMode);
        CHECK_AND_RETURN_RET_LOG(err == GSERROR_OK, VPE_ALGO_ERR_UNKNOWN, "GetScalingMode fail");
        err = outputSurface_->SetScalingMode(inScaleMode);
        CHECK_AND_RETURN_RET_LOG(err == GSERROR_OK, VPE_ALGO_ERR_UNKNOWN, "SetScalingMode fail");
    }
    return VPE_ALGO_ERR_OK;
}

int32_t ColorSpaceConverterVideoImpl::SetOutputSurface(sptr<Surface> surface)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(surface != nullptr, VPE_ALGO_ERR_INVALID_VAL, "surface is nullptr");
    CHECK_AND_RETURN_RET_LOG(surface->IsConsumer() == false, VPE_ALGO_ERR_INVALID_VAL, "surface is not producer");
    if (state_ == VPEAlgoState::INITIALIZED || state_ == VPEAlgoState::CONFIGURING) {
        int32_t ret = SetOutputSurfaceConfig(surface);
        CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, VPE_ALGO_ERR_INVALID_STATE, "SetOutputSurface config fail");
    } else if (state_ == VPEAlgoState::RUNNING || state_ == VPEAlgoState::EOS || state_ == VPEAlgoState::FLUSHED) {
        int32_t ret = SetOutputSurfaceRunning(surface);
        CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, VPE_ALGO_ERR_INVALID_STATE, "SetOutputSurface Running fail");
    } else {
        CHECK_AND_RETURN_RET_LOG(false, VPE_ALGO_ERR_INVALID_STATE, "surface state not support SetOutputSurface");
    }

    return VPE_ALGO_ERR_OK;
}

sptr<Surface> ColorSpaceConverterVideoImpl::CreateInputSurface()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(state_ == VPEAlgoState::INITIALIZED || state_ == VPEAlgoState::CONFIGURING, nullptr,
        "CreateInputSurface failed: not in INITIALIZED or CONFIGURING state");
    CHECK_AND_RETURN_RET_LOG(inputSurface_ == nullptr, nullptr, "inputSurface already exists");

    inputSurface_ = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    CHECK_AND_RETURN_RET_LOG(inputSurface_ != nullptr, nullptr, "CreateSurfaceAsConsumer fail");
    sptr<IBufferConsumerListener> listener = new ImageProcessBufferConsumerListener(this);
    GSError err = inputSurface_->RegisterConsumerListener(listener);
    CHECK_AND_RETURN_RET_LOG(err == GSERROR_OK, nullptr, "RegisterConsumerListener fail");

    sptr<IBufferProducer> producer = inputSurface_->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    CHECK_AND_RETURN_RET_LOG(producerSurface != nullptr, nullptr, "CreateSurfaceAsProducer fail");
    producerSurface->SetDefaultUsage(BUFFER_USAGE_CPU_READ);
    inputSurface_->SetQueueSize(inBufferCnt_);
    state_ = VPEAlgoState::CONFIGURING;

    return producerSurface;
}

int32_t ColorSpaceConverterVideoImpl::ConfigureColorSpace(const Format &format)
{
    int32_t primaries;
    if (!format.GetIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_PRIMARIES, primaries)) {
        VPE_LOGE("format should contain colorspace_primaries");
        return VPE_ALGO_ERR_INVALID_VAL;
    }

    int32_t transfunc;
    if (!format.GetIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_TRANS_FUNC, transfunc)) {
        VPE_LOGE("format should contain colorspace_trans_func");
        return VPE_ALGO_ERR_INVALID_VAL;
    }

    int32_t matrix;
    if (!format.GetIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_MATRIX, matrix)) {
        VPE_LOGE("format should contain colorspace_matrix");
        return VPE_ALGO_ERR_INVALID_VAL;
    }

    int32_t range;
    if (!format.GetIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_RANGE, range)) {
        VPE_LOGE("format should contain colorspace_range");
        return VPE_ALGO_ERR_INVALID_VAL;
    }

    format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_PRIMARIES, primaries);
    format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_TRANS_FUNC, transfunc);
    format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_MATRIX, matrix);
    format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_COLORSPACE_RANGE, range);

    CM_ColorSpaceInfo outColorInfo{
	    .primaries = static_cast<CM_ColorPrimaries>(primaries),
        .transfunc = static_cast<CM_TransFunc>(transfunc),
        .matrix = static_cast<CM_Matrix>(matrix),
        .range = static_cast<CM_Range>(range)};

    colorSpaceVec_.resize(sizeof(outColorInfo));
    errno_t ret = memcpy_s(colorSpaceVec_.data(), colorSpaceVec_.size(), &outColorInfo, sizeof(outColorInfo));
    if (ret != EOK) {
        VPE_LOGE("memcpy_s failed, err = %d\n", ret);
        return VPE_ALGO_ERR_INVALID_VAL;
    }
    return VPE_ALGO_ERR_OK;
}

int32_t ColorSpaceConverterVideoImpl::Configure(const Format &format)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(state_ == VPEAlgoState::INITIALIZED || state_ == VPEAlgoState::CONFIGURING || state_ ==
        VPEAlgoState::STOPPED, VPE_ALGO_ERR_INVALID_STATE, "Configure failed: not in INITIALIZED or CONFIGURING state");
    GraphicPixelFormat surfacePixelFmt;
    if (!format.GetIntValue(CscVDescriptionKey::CSCV_KEY_PIXEL_FORMAT, *(int *)&surfacePixelFmt)) {
        VPE_LOGE("format should contain output pixel_format");
        return VPE_ALGO_ERR_INVALID_VAL;
    }
    int32_t outputColorSpace = 0;
    if (format.GetIntValue(Media::Tag::VIDEO_DECODER_OUTPUT_COLOR_SPACE, outputColorSpace)) {
        outputFormat_.PutIntValue(Media::Tag::VIDEO_DECODER_OUTPUT_COLOR_SPACE, outputColorSpace);
    }
    requestCfg_.format = surfacePixelFmt;
    format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_PIXEL_FORMAT, int(surfacePixelFmt));
    // 指定色彩空间
    if (format.GetIntValue(CscVDescriptionKey::CSCV_KEY_HDR_METADATA_TYPE, hdrType_)) {
        format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_HDR_METADATA_TYPE, hdrType_);
        hdrVec_.resize(sizeof(hdrType_));
        errno_t ret = memcpy_s(hdrVec_.data(), hdrVec_.size(), &hdrType_, sizeof(hdrType_));
        if (ret != EOK) {
            VPE_LOGE("memcpy_s failed, err = %d\n", ret);
            return VPE_ALGO_ERR_INVALID_VAL;
        }
    }

    if (ConfigureColorSpace(format) != VPE_ALGO_ERR_OK) {
        return VPE_ALGO_ERR_INVALID_VAL;
    }
    int32_t renderIntent;
    if (format.GetIntValue(CscVDescriptionKey::CSCV_KEY_RENDER_INTENT, renderIntent)) {
        format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_RENDER_INTENT, renderIntent);
    }
    double sdruiBrightnessRatio;
    if (format.GetDoubleValue(CscVDescriptionKey::CSCV_KEY_SDRUI_BRIGHTNESS_RATIO, sdruiBrightnessRatio)) {
        format_.PutDoubleValue(CscVDescriptionKey::CSCV_KEY_SDRUI_BRIGHTNESS_RATIO, sdruiBrightnessRatio);
    }

    ColorSpaceConverterParameter param = {RenderIntent(renderIntent), sdruiBrightnessRatio};
    int32_t ret = csc_->SetParameter(param);
    state_ = (ret == VPE_ALGO_ERR_OK ? VPEAlgoState::CONFIGURING : VPEAlgoState::ERROR);
    return ret;
}

int32_t ColorSpaceConverterVideoImpl::GetParameter(Format &parameter)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(state_ >= VPEAlgoState::CONFIGURED && state_ < VPEAlgoState::EOS,
        VPE_ALGO_ERR_INVALID_STATE, "GetParameter failed: not in right state");
    ColorSpaceConverterParameter param;
    csc_->GetParameter(param);
    if (param.sdrUIBrightnessRatio.has_value()) {
        format_.PutDoubleValue(CscVDescriptionKey::CSCV_KEY_SDRUI_BRIGHTNESS_RATIO, param.sdrUIBrightnessRatio.value());
    }
    format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_RENDER_INTENT, int(param.renderIntent));
    parameter = format_;
    return VPE_ALGO_ERR_OK;
}

int32_t ColorSpaceConverterVideoImpl::SetParameter(const Format &parameter)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(state_ >= VPEAlgoState::CONFIGURED && state_ <= VPEAlgoState::RUNNING,
        VPE_ALGO_ERR_INVALID_STATE, "SetParameter failed: not in right state");

    int32_t renderIntent;
    if (parameter.GetIntValue(CscVDescriptionKey::CSCV_KEY_RENDER_INTENT, renderIntent)) {
        format_.PutIntValue(CscVDescriptionKey::CSCV_KEY_RENDER_INTENT, renderIntent);
    }
    double sdruiBrightnessRatio;
    if (parameter.GetDoubleValue(CscVDescriptionKey::CSCV_KEY_SDRUI_BRIGHTNESS_RATIO, sdruiBrightnessRatio)) {
        format_.PutDoubleValue(CscVDescriptionKey::CSCV_KEY_SDRUI_BRIGHTNESS_RATIO, sdruiBrightnessRatio);
    }
    ColorSpaceConverterParameter param = {RenderIntent(renderIntent), sdruiBrightnessRatio};
    int32_t ret = csc_->SetParameter(param);
    if (ret != VPE_ALGO_ERR_OK) {
        state_ = VPEAlgoState::ERROR;
        VPE_LOGE("SetParameter failed");
        return ret;
    }
    return ret;
}

int32_t ColorSpaceConverterVideoImpl::Prepare()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (state_ == VPEAlgoState::STOPPED) {
        state_ = VPEAlgoState::CONFIGURED;
        return VPE_ALGO_ERR_OK;
    }
    CHECK_AND_RETURN_RET_LOG(
        state_ == VPEAlgoState::CONFIGURING, VPE_ALGO_ERR_INVALID_STATE, "Prepare failed: not in CONFIGURING state");
    CHECK_AND_RETURN_RET_LOG(
        cb_ != nullptr && inputSurface_ != nullptr && outputSurface_ != nullptr && format_.GetFormatMap().size() > 0,
        VPE_ALGO_ERR_INVALID_OPERATION, "Prepare faled: inputSurface or outputSurface or callback is null");

    state_ = VPEAlgoState::CONFIGURED;
    return VPE_ALGO_ERR_OK;
}

void GetFormatFromSurfaceBuffer(Format &outputFormat, sptr<SurfaceBuffer> &buffer)
{
    outputFormat.PutIntValue(Media::Tag::VIDEO_WIDTH, buffer->GetWidth());
    outputFormat.PutIntValue(Media::Tag::VIDEO_HEIGHT, buffer->GetHeight());
    outputFormat.PutIntValue(Media::Tag::VIDEO_PIC_WIDTH, buffer->GetWidth());
    outputFormat.PutIntValue(Media::Tag::VIDEO_PIC_HEIGHT, buffer->GetHeight());
    int32_t stride =  buffer->GetStride();
    outputFormat.PutIntValue(Media::Tag::VIDEO_STRIDE, stride);
    if (stride <= 0) {
        VPE_LOGW("invalid stride %d", stride);
        return;
    }
    OH_NativeBuffer_Planes *planes = nullptr;
    GSError err = buffer->GetPlanesInfo(reinterpret_cast<void **>(&planes));
    if (err != GSERROR_OK || planes == nullptr) {
        VPE_LOGW("get plane info failed, GSError=%{public}d", err);
        return;
    }
    for (uint32_t j = 0; j < planes->planeCount; j++) {
        VPE_LOGD("plane[%{public}u]: offset=%{public}" PRIu64 ", rowStride=%{public}u, columnStride=%{public}u",
            j,
            planes->planes[j].offset,
            planes->planes[j].rowStride,
            planes->planes[j].columnStride);
    }
    int32_t sliceHeight = buffer->GetHeight();
    if (planes->planeCount > 1) {
        sliceHeight = static_cast<int32_t>(static_cast<int64_t>(planes->planes[1].offset) / stride);
    }
    outputFormat.PutIntValue(Media::Tag::VIDEO_SLICE_HEIGHT, sliceHeight);
}

void ColorSpaceConverterVideoImpl::InitBuffers()
{
    flushCfg_.damage.x = 0;
    flushCfg_.damage.y = 0;
    flushCfg_.damage.w = requestCfg_.width;
    flushCfg_.damage.h = requestCfg_.height;
    bool firstBuffer = true;
    for (uint32_t i = 0; i < outBufferCnt_; ++i) {
        std::shared_ptr<SurfaceBufferWrapper> buffer = std::make_shared<SurfaceBufferWrapper>();
        GSError err = outputSurface_->RequestBuffer(buffer->memory, buffer->fence, requestCfg_);
        if (err != GSERROR_OK || buffer->memory == nullptr) {
            VPE_LOGW("RequestBuffer %{public}u failed, GSError=%{public}d", i, err);
            continue;
        }
        outputBufferAvilQue_.push(buffer);
        outputBufferAvilQueBak_.insert(std::make_pair(buffer->memory->GetSeqNum(), buffer));
        if (firstBuffer) {
            GetFormatFromSurfaceBuffer(outputFormat_, buffer->memory);
            firstBuffer = false;
        }
    }
}

int32_t ColorSpaceConverterVideoImpl::Start()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(
        (state_ == VPEAlgoState::CONFIGURED || state_ == VPEAlgoState::STOPPED || state_ == VPEAlgoState::FLUSHED),
        VPE_ALGO_ERR_INVALID_STATE,
        "Start failed: not in CONFIGURED or STOPPED state");
    if (isEos_.load()) {
        state_ = VPEAlgoState::EOS;
    } else {
        state_ = VPEAlgoState::RUNNING;
    }
    cvTaskStart_.notify_all();
    cb_->OnState(static_cast<int32_t>(state_.load()));
    return VPE_ALGO_ERR_OK;
}

int32_t ColorSpaceConverterVideoImpl::Stop()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(
        state_ == VPEAlgoState::RUNNING || state_ == VPEAlgoState::EOS || state_ == VPEAlgoState::FLUSHED,
        VPE_ALGO_ERR_INVALID_STATE,
        "Stop failed: not in RUNNING or EOS state");

    state_ = VPEAlgoState::STOPPED;
    if (!isProcessing_) {
        cb_->OnState(static_cast<int32_t>(state_.load()));
    }
    return VPE_ALGO_ERR_OK;
}

int32_t ColorSpaceConverterVideoImpl::Reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(
        state_ != VPEAlgoState::UNINITIALIZED, VPE_ALGO_ERR_INVALID_STATE, "Start failed: not in right state");
    std::unique_lock<std::mutex> lockTask(mtxTaskDone_);
    state_ = VPEAlgoState::INITIALIZED;
    cvTaskDone_.wait(lockTask, [this]() { return isProcessing_.load() == false; });

    csc_ = ColorSpaceConverter::Create();
    CHECK_AND_RETURN_RET_LOG(csc_ != nullptr, VPE_ALGO_ERR_UNKNOWN, "ColorSpaceConverter Create failed");
    format_ = Format();
    colorSpaceVec_.clear();
    hdrVec_.clear();
    isEos_.store(false);

    return VPE_ALGO_ERR_OK;
}

int32_t ColorSpaceConverterVideoImpl::Release()
{
    std::lock_guard<std::mutex> lock(mutex_);
    {
        std::unique_lock<std::mutex> lockTask(mtxTaskDone_);
        state_ = VPEAlgoState::UNINITIALIZED;
        cvTaskDone_.wait(lockTask, [this]() { return isProcessing_.load() == false; });

        inputSurface_ = nullptr;
        std::unique_lock<std::mutex> lockSurface(surfaceChangeMutex_);
        std::unique_lock<std::mutex> lockSurface2(surfaceChangeMutex2_);
        if (outputSurface_ != nullptr) {
            outputSurface_->UnRegisterReleaseListener();
            outputSurface_->CleanCache(true);
            outputSurface_ = nullptr;
        }
        lockSurface2.unlock();
        lockSurface.unlock();
        cb_ = nullptr;
        csc_ = nullptr;
        isRunning_.store(false);
    }
    if (taskThread_ != nullptr && taskThread_->joinable()) {
        cvTaskStart_.notify_all();
        taskThread_->join();
    }
    return VPE_ALGO_ERR_OK;
}

int32_t ColorSpaceConverterVideoImpl::Flush()
{
    std::lock_guard<std::mutex> lock(mutex_);
    {
        std::unique_lock<std::mutex> lockTask(mtxTaskDone_);
        cvTaskDone_.wait(lockTask, [this]() { return isProcessing_.load() == false; });
    }

    {
        std::unique_lock<std::mutex> lockInQue(onBqMutex_);
        std::queue<std::shared_ptr<SurfaceBufferWrapper>> tempQueue;
        inputBufferAvilQue_.swap(tempQueue);
        for (; tempQueue.size() != 0;) {
            auto buffer = tempQueue.front();
            tempQueue.pop();
            CHECK_AND_RETURN_RET_LOG(buffer && buffer->memory != nullptr, VPE_ALGO_ERR_UNKNOWN, "Invalid memory");
            GSError err = inputSurface_->ReleaseBuffer(buffer->memory, -1);
            CHECK_AND_RETURN_RET_LOG(err == GSERROR_OK, VPE_ALGO_ERR_UNKNOWN, "Release buffer failed");
        }
    }

    std::lock_guard<std::mutex> mapLock(renderQueMutex_);
    for (auto &[id, buffer] : renderBufferAvilMap_) {
        VPE_LOGD("Reclaim buffer %{public}" PRIu64, id);
        outputBufferAvilQue_.push(buffer);
    }
    renderBufferAvilMap_.clear();
    state_ = VPEAlgoState::FLUSHED;
    return VPE_ALGO_ERR_OK;
}

int32_t ColorSpaceConverterVideoImpl::GetOutputFormat(Format &format)
{
    int32_t width = 0;
    if (outputFormat_.GetIntValue(Media::Tag::VIDEO_WIDTH, width)) {
        format.PutIntValue(Media::Tag::VIDEO_WIDTH, width);
        format.PutIntValue(Media::Tag::VIDEO_PIC_WIDTH, width);
    }
    int32_t height = 0;
    if (outputFormat_.GetIntValue(Media::Tag::VIDEO_HEIGHT, height)) {
        format.PutIntValue(Media::Tag::VIDEO_HEIGHT, height);
        format.PutIntValue(Media::Tag::VIDEO_PIC_HEIGHT, height);
    }
    int32_t stride = 0;
    if (outputFormat_.GetIntValue(Media::Tag::VIDEO_STRIDE, stride)) {
        format.PutIntValue(Media::Tag::VIDEO_STRIDE, stride);
    }
    int32_t sliceHeight = 0;
    if (outputFormat_.GetIntValue(Media::Tag::VIDEO_SLICE_HEIGHT, sliceHeight)) {
        format.PutIntValue(Media::Tag::VIDEO_SLICE_HEIGHT, sliceHeight);
    }
    int32_t outputColorSpace = 0;
    if (outputFormat_.GetIntValue(Media::Tag::VIDEO_DECODER_OUTPUT_COLOR_SPACE, outputColorSpace)) {
        format.PutIntValue(Media::Tag::VIDEO_DECODER_OUTPUT_COLOR_SPACE, outputColorSpace);
    }
    return VPE_ALGO_ERR_OK;
}

void ColorSpaceConverterVideoImpl::Process(
    std::shared_ptr<SurfaceBufferWrapper> inputBuffer, std::shared_ptr<SurfaceBufferWrapper> outputBuffer)
{
    int32_t ret = 0;
    outputBuffer->timestamp = inputBuffer->timestamp;
    sptr<SurfaceBuffer> surfaceInputBuffer = inputBuffer->memory;
    sptr<SurfaceBuffer> surfaceOutputBuffer = outputBuffer->memory;
    int32_t currentWidth = surfaceInputBuffer->GetWidth();
    int32_t currentHeight = surfaceInputBuffer->GetHeight();
    if ((currentWidth != surfaceOutputBuffer->GetWidth()) || (currentHeight != surfaceOutputBuffer->GetHeight())) {
        requestCfg_.width = currentWidth;
        requestCfg_.height = currentHeight;
        surfaceOutputBuffer->EraseMetadataKey(ATTRKEY_COLORSPACE_INFO);
        surfaceOutputBuffer->EraseMetadataKey(ATTRKEY_HDR_METADATA_TYPE);
        surfaceOutputBuffer->Alloc(requestCfg_);
    }
    if (colorSpaceVec_.size() > 0) {
        surfaceOutputBuffer->SetMetadata(ATTRKEY_COLORSPACE_INFO, colorSpaceVec_);
    }
    if (hdrVec_.size() > 0) {
        surfaceOutputBuffer->SetMetadata(ATTRKEY_HDR_METADATA_TYPE, hdrVec_);
    }
    if ((currentWidth != lastOutputWidth_) || (currentHeight != lastOutputHeight_)) {
        Format outputFormat;
        GetFormatFromSurfaceBuffer(outputFormat_, surfaceOutputBuffer);
        GetOutputFormat(outputFormat);
        cb_->OnOutputFormatChanged(outputFormat);
        lastOutputWidth_ = currentWidth;
        lastOutputHeight_ = currentHeight;
    }
    {
        VPETrace cscTrace("ColorSpaceConverterVideoImpl::csc_->Process");
        ret = csc_->Process(surfaceInputBuffer, surfaceOutputBuffer);
        if (ret != 0 && cb_) {
            cb_->OnError(ret);
        }
    }
    inputSurface_->ReleaseBuffer(surfaceInputBuffer, -1);
    if (!ret) {
        std::unique_lock<std::mutex> lockOnBq(renderQueMutex_);
        renderBufferAvilMap_.emplace(outputBuffer->memory->GetSeqNum(), outputBuffer);
    } else {
        std::lock_guard<std::mutex> renderLock(renderQueMutex_);
        outputBufferAvilQue_.push(outputBuffer);
    }
    if (!ret && cb_) {
        cb_->OnOutputBufferAvailable(surfaceOutputBuffer->GetSeqNum(), outputBuffer->bufferFlag);
    }
}

bool ColorSpaceConverterVideoImpl::WaitProcessing()
{
    if (!isRunning_.load()) {
        return false;
    }

    {
        std::unique_lock<std::mutex> lock(mtxTaskStart_);
        cvTaskStart_.wait(lock, [this]() {
            std::lock_guard<std::mutex> inQueueLock(onBqMutex_);
            std::lock_guard<std::mutex> outQueueLock(renderQueMutex_);
            return ((inputBufferAvilQue_.size() > 0 && outputBufferAvilQue_.size() > 0) || !isRunning_.load());
        });
    }

    return true;
}

bool ColorSpaceConverterVideoImpl::AcquireInputOutputBuffers(
    std::shared_ptr<SurfaceBufferWrapper> &inputBuffer, std::shared_ptr<SurfaceBufferWrapper> &outputBuffer)
{
    std::lock_guard<std::mutex> lockOnBq(onBqMutex_);
    std::lock_guard<std::mutex> mapLock(renderQueMutex_);
    if (inputBufferAvilQue_.size() == 0 || outputBufferAvilQue_.size() == 0) {
        if (state_ == VPEAlgoState::STOPPED) {
            cb_->OnState(static_cast<int32_t>(state_.load()));
        }
        return false;
    }
    inputBuffer = inputBufferAvilQue_.front();
    outputBuffer = outputBufferAvilQue_.front();
    inputBufferAvilQue_.pop();
    outputBufferAvilQue_.pop();
    return inputBuffer && outputBuffer;
}

void ColorSpaceConverterVideoImpl::DoTask()
{
    std::shared_ptr<SurfaceBufferWrapper> inputBuffer = nullptr;
    std::shared_ptr<SurfaceBufferWrapper> outputBuffer = nullptr;
    while (true) {
        std::lock_guard<std::mutex> lockTask(mtxTaskDone_);
        if (!isRunning_.load()) {
            return;
        }
        isProcessing_.store(true);

        if (!AcquireInputOutputBuffers(inputBuffer, outputBuffer)) {
            break;
        }

        if (inputBuffer->bufferFlag == CSCV_BUFFER_FLAG_EOS) {
            {
                std::unique_lock<std::mutex> lockOnBq(renderQueMutex_);
                renderBufferAvilMap_.emplace(outputBuffer->memory->GetSeqNum(), outputBuffer);
            }

            if (cb_) {
                cb_->OnOutputBufferAvailable(outputBuffer->memory->GetSeqNum(), CSCV_BUFFER_FLAG_EOS);
            }
            break;
        }

        Process(inputBuffer, outputBuffer);
    }
    isProcessing_.store(false);
    cvTaskDone_.notify_all();
}

void ColorSpaceConverterVideoImpl::OnTriggered()
{
    while (true) {
        if (!WaitProcessing()) {
            break;
        }

        DoTask();
    }
}

int32_t ColorSpaceConverterVideoImpl::ReleaseOutputBuffer(uint32_t index, bool render)
{
    CHECK_AND_RETURN_RET_LOG(state_ == VPEAlgoState::RUNNING || state_ == VPEAlgoState::EOS,
        VPE_ALGO_ERR_INVALID_STATE, "ReleaseOutputBuffer failed: not in RUNNING or EOS state");

    std::unique_lock<std::mutex> lockRenderQue(renderQueMutex_);
    auto search = renderBufferAvilMap_.find(index);
    if (search == renderBufferAvilMap_.end()) {
        VPE_LOGE("ReleaseOutputBuffer invalid index %{public}d ", index);
        return VPE_ALGO_ERR_INVALID_PARAM;
    }
    auto buffer = search->second;
    renderBufferAvilMap_.erase(search);
    lockRenderQue.unlock();

    if (render) {
        flushCfg_.timestamp = buffer->timestamp;
        flushCfg_.damage.w = buffer->memory->GetWidth();
        flushCfg_.damage.h = buffer->memory->GetHeight();
        {
            std::lock_guard<std::mutex> lockSurface(surfaceChangeMutex_);
            CHECK_AND_RETURN_RET_LOG(outputSurface_ != nullptr, GSERROR_OK, "outputSurface_ is nullptr");
            auto ret = outputSurface_->FlushBuffer(buffer->memory, -1, flushCfg_);
            if (ret != 0) {
                VPE_LOGE("ReleaseOutputBuffer flushbuffer err %{public}d ", (int)ret);
                return VPE_ALGO_ERR_UNKNOWN;
            }
        }
        std::lock_guard<std::mutex> renderLock(renderQueMutex_);
        renderBufferMapBak_.emplace(buffer->memory->GetSeqNum(), buffer);
    } else {
        std::lock_guard<std::mutex> renderLock(renderQueMutex_);
        outputBufferAvilQue_.push(buffer);
    }

    return VPE_ALGO_ERR_OK;
}

int32_t ColorSpaceConverterVideoImpl::NotifyEos()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::lock_guard<std::mutex> lockOnBq(onBqMutex_);
    CHECK_AND_RETURN_RET_LOG(
        state_ == VPEAlgoState::RUNNING, VPE_ALGO_ERR_INVALID_STATE, "NotifyEos failed: not in RUNNING state");
    state_ = VPEAlgoState::EOS;
    isEos_.store(true);
    std::shared_ptr<SurfaceBufferWrapper> buf = std::make_shared<SurfaceBufferWrapper>();
    buf->bufferFlag = CSCV_BUFFER_FLAG_EOS;
    inputBufferAvilQue_.push(buf);

    cvTaskStart_.notify_all();

    return VPE_ALGO_ERR_OK;
}

GSError ColorSpaceConverterVideoImpl::OnProducerBufferReleased()
{
    {
        std::unique_lock<std::mutex> lockSurface(surfaceChangeMutex2_);
        std::lock_guard<std::mutex> outQueLock(renderQueMutex_);
        std::shared_ptr<SurfaceBufferWrapper> buf = std::make_shared<SurfaceBufferWrapper>();
        CHECK_AND_RETURN_RET_LOG(outputSurface_ != nullptr, GSERROR_OK, "outputSurface_ is nullptr");
        if (renderBufferMapBak_.empty()) {
            return GSERROR_OK;
        }
        GSError err = outputSurface_->RequestBuffer(buf->memory, buf->fence, requestCfg_);
        if (err != GSERROR_OK || buf->memory == nullptr) {
            VPE_LOGE("RequestBuffer failed, GSError=%{public}d", err);
            return err;
        }
        lockSurface.unlock();
        outputBufferAvilQue_.push(buf);
        auto bufSeqNum = buf->memory->GetSeqNum();
        lastSurfaceSequence_ = bufSeqNum;
        renderBufferMapBak_.erase(bufSeqNum);
        auto it = outputBufferAvilQueBak_.find(bufSeqNum);
        if (it == outputBufferAvilQueBak_.end()) {
            outputBufferAvilQueBak_.insert(std::make_pair(bufSeqNum, buf));
            auto firstSeqNum = renderBufferMapBak_.begin();
            if (firstSeqNum != renderBufferMapBak_.end()) {
                outputBufferAvilQueBak_.erase(firstSeqNum->first);
                renderBufferMapBak_.erase(firstSeqNum->first);
            }
        }
    }
    if (state_ == VPEAlgoState::RUNNING || state_ == VPEAlgoState::EOS) {
        cvTaskStart_.notify_all();
    }

    return GSERROR_OK;
}

GSError ColorSpaceConverterVideoImpl::OnConsumerBufferAvailable()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::lock_guard<std::mutex> lockInQue(onBqMutex_);
    CHECK_AND_RETURN_RET_LOG(inputSurface_ != nullptr, GSERROR_OK, "inputSurface is nullptr");
    CHECK_AND_RETURN_RET_LOG(state_ != VPEAlgoState::STOPPED, GSERROR_OK, "state change to stop");
    std::shared_ptr<SurfaceBufferWrapper> buffer = std::make_shared<SurfaceBufferWrapper>();
    OHOS::Rect damage;
    GSError err = inputSurface_->AcquireBuffer(buffer->memory, buffer->fence, buffer->timestamp, damage);
    if (err != GSERROR_OK || buffer->memory == nullptr) {
        VPE_LOGW("AcquireBuffer failed, GSError=%{public}d", err);
        return err;
    }
    inputBufferAvilQue_.push(buffer);

    if (!getUsage_) {
        requestCfg_.usage = (buffer->memory->GetUsage() | requestCfg_.usage);
        getUsage_ = true;
        requestCfg_.width = buffer->memory->GetWidth();
        requestCfg_.height = buffer->memory->GetHeight();
        InitBuffers();
    }

    if (state_ == VPEAlgoState::RUNNING) {
        cvTaskStart_.notify_all();
    }

    return GSERROR_OK;
}

void ImageProcessBufferConsumerListener::OnBufferAvailable()
{
    if (process_ != nullptr) {
        process_->OnConsumerBufferAvailable();
    }
}

ColorSpaceConverterVideoCallbackImpl::ColorSpaceConverterVideoCallbackImpl(Callback *callback, ArgumentType *userData)
    : userData_(userData)
{
    callback_ = std::make_unique<Callback>();
    if (callback_) {
        callback_->onError = nullptr;
        callback_->onOutputBufferAvailable = nullptr;
        callback_->OnOutputFormatChanged = nullptr;
        if (callback) {
            callback_->onError = callback->onError;
            callback_->onOutputBufferAvailable = callback->onOutputBufferAvailable;
            callback_->OnOutputFormatChanged = callback->OnOutputFormatChanged;
        }
    }
}

void ColorSpaceConverterVideoCallbackImpl::OnError(int32_t errorCode)
{
    if (callback_ && callback_->onError) {
        callback_->onError(errorCode, userData_);
    }
}

void ColorSpaceConverterVideoCallbackImpl::OnState(int32_t state)
{
    (void)state;
    return;
}

void ColorSpaceConverterVideoCallbackImpl::OnOutputBufferAvailable(uint32_t index, CscvBufferFlag flag)
{
    if (callback_ && callback_->onOutputBufferAvailable) {
        CscvBufferFlag flagTemp = static_cast<CscvBufferFlag>(flag);
        callback_->onOutputBufferAvailable(index, flagTemp, userData_);
    }
}

void ColorSpaceConverterVideoCallbackImpl::OnOutputFormatChanged(const Format& format)
{
    if (callback_ && callback_->OnOutputFormatChanged) {
        callback_->OnOutputFormatChanged(format, userData_);
    }
}

struct ColorSpaceConvertVideoHandleImpl {
    std::shared_ptr<ColorSpaceConverterVideoImpl> obj;
};

int32_t ColorSpaceConvertVideoIsColorSpaceConversionSupported(const ArgumentType *input, const ArgumentType *output)
{
    struct CapabilityInfo {
        int32_t colorSpaceType;
        int32_t metadataType;
        int32_t pixelFormat;
    };
    VPE_LOGD("Enter");
    CHECK_AND_RETURN_RET_LOG(input != nullptr && output != nullptr, -1, "Input or output is null");
    auto inputInfo = *static_cast<const CapabilityInfo *>(input);
    auto outputInfo = *static_cast<const CapabilityInfo *>(output);
    FrameInfo inputFrameInfo;
    inputFrameInfo.colorSpace.colorSpaceInfo = GetColorSpaceInfo(static_cast<uint32_t>(inputInfo.colorSpaceType));
    inputFrameInfo.colorSpace.metadataType =
        static_cast<OHOS::HDI::Display::Graphic::Common::V1_0::CM_HDR_Metadata_Type>(inputInfo.metadataType);
    inputFrameInfo.pixelFormat = static_cast<OHOS::GraphicPixelFormat>(inputInfo.pixelFormat);
    FrameInfo outputFrameInfo;
    outputFrameInfo.colorSpace.colorSpaceInfo = GetColorSpaceInfo(static_cast<uint32_t>(outputInfo.colorSpaceType));
    outputFrameInfo.colorSpace.metadataType =
        static_cast<OHOS::HDI::Display::Graphic::Common::V1_0::CM_HDR_Metadata_Type>(outputInfo.metadataType);
    outputFrameInfo.pixelFormat = static_cast<OHOS::GraphicPixelFormat>(outputInfo.pixelFormat);
    VPE_LOGD("Get extension manager");
    auto &manager = Extension::ExtensionManager::GetInstance();
    VPE_LOGD("Query capability");
    manager.IncreaseInstance();
    auto supported = manager.IsColorSpaceConversionSupported(inputFrameInfo, outputFrameInfo);
    manager.DecreaseInstance();
    VPE_LOGD("Exit");
    return supported ? 0 : -1;
}

ColorSpaceConvertVideoHandle *ColorSpaceConvertVideoCreate()
{
    std::shared_ptr<ColorSpaceConverterVideoImpl> impl = std::make_shared<ColorSpaceConverterVideoImpl>();
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr, "failed to init ColorSpaceConverterVideoImpl");

    int32_t ret = impl->Init();
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, nullptr, "failed to init ColorSpaceConverterVideoImpl");

    auto handle = new ColorSpaceConvertVideoHandleImpl;
    handle->obj = impl;
    return static_cast<ColorSpaceConvertVideoHandle *>(handle);
}

void ColorSpaceConvertVideoDestroy(ColorSpaceConvertVideoHandle *handle)
{
    if (handle != nullptr) {
        auto p = static_cast<ColorSpaceConvertVideoHandleImpl *>(handle);
        delete p;
    }
}

int32_t ColorSpaceConvertVideoSetCallback(
    ColorSpaceConvertVideoHandle *handle, ArgumentType *callback, ArgumentType *userData)
{
    int32_t ret = -1;
    if (handle != nullptr && callback != nullptr) {
        auto callbackPtr = static_cast<ColorSpaceConverterVideoCallbackImpl::Callback *>(callback);
        auto callbackImpl = std::make_shared<ColorSpaceConverterVideoCallbackImpl>(callbackPtr, userData);
        auto handlePtr = static_cast<ColorSpaceConvertVideoHandleImpl *>(handle);
        ret = handlePtr->obj->SetCallback(std::static_pointer_cast<ColorSpaceConverterVideoCallback>(callbackImpl));
    }
    return ret;
}

int32_t ColorSpaceConvertVideoSetOutputSurface(ColorSpaceConvertVideoHandle *handle, ArgumentType *surface)
{
    int32_t ret = -1;
    if (handle != nullptr && surface != nullptr) {
        auto sf = static_cast<sptr<Surface> *>(surface);
        auto handlePtr = static_cast<ColorSpaceConvertVideoHandleImpl *>(handle);
        ret = handlePtr->obj->SetOutputSurface(*sf);
    }
    return ret;
}

int32_t ColorSpaceConvertVideoCreateInputSurface(ColorSpaceConvertVideoHandle *handle, ArgumentType *surface)
{
    if (handle != nullptr) {
        auto sf = static_cast<sptr<Surface> *>(surface);
        auto handlePtr = static_cast<ColorSpaceConvertVideoHandleImpl *>(handle);
        *sf = handlePtr->obj->CreateInputSurface();
        return 0;
    }
    return -1;
}

int32_t ColorSpaceConvertVideoSetParameter(ColorSpaceConvertVideoHandle *handle, ArgumentType *parameter)
{
    int32_t ret = -1;
    if (handle != nullptr) {
        Format *p = static_cast<Format *>(parameter);
        auto handlePtr = static_cast<ColorSpaceConvertVideoHandleImpl *>(handle);
        ret = handlePtr->obj->SetParameter(*p);
    }
    return ret;
}

int32_t ColorSpaceConvertVideoGetParameter(ColorSpaceConvertVideoHandle *handle, ArgumentType *parameter)
{
    int32_t ret = -1;
    if (handle != nullptr) {
        Format *p = static_cast<Format *>(parameter);
        auto handlePtr = static_cast<ColorSpaceConvertVideoHandleImpl *>(handle);
        ret = handlePtr->obj->GetParameter(*p);
    }
    return ret;
}

int32_t ColorSpaceConvertVideoConfigure(ColorSpaceConvertVideoHandle *handle, ArgumentType *configuration)
{
    int32_t ret = -1;
    if (handle != nullptr && configuration != nullptr) {
        Format *format = static_cast<Format *>(configuration);
        auto handlePtr = static_cast<ColorSpaceConvertVideoHandleImpl *>(handle);
        ret = handlePtr->obj->Configure(*format);
    }
    return ret;
}

int32_t ColorSpaceConvertVideoPrepare(ColorSpaceConvertVideoHandle *handle)
{
    int32_t ret = -1;
    if (handle != nullptr) {
        auto handlePtr = static_cast<ColorSpaceConvertVideoHandleImpl *>(handle);
        ret = handlePtr->obj->Prepare();
    }
    return ret;
}

int32_t ColorSpaceConvertVideoStart(ColorSpaceConvertVideoHandle *handle)
{
    int32_t ret = -1;
    if (handle != nullptr) {
        auto handlePtr = static_cast<ColorSpaceConvertVideoHandleImpl *>(handle);
        ret = handlePtr->obj->Start();
    }
    return ret;
}

int32_t ColorSpaceConvertVideoStop(ColorSpaceConvertVideoHandle *handle)
{
    int32_t ret = -1;
    if (handle != nullptr) {
        auto handlePtr = static_cast<ColorSpaceConvertVideoHandleImpl *>(handle);
        ret = handlePtr->obj->Stop();
    }
    return ret;
}

int32_t ColorSpaceConvertVideoReset(ColorSpaceConvertVideoHandle *handle)
{
    int32_t ret = -1;
    if (handle != nullptr) {
        auto handlePtr = static_cast<ColorSpaceConvertVideoHandleImpl *>(handle);
        ret = handlePtr->obj->Reset();
    }
    return ret;
}

int32_t ColorSpaceConvertVideoRelease(ColorSpaceConvertVideoHandle *handle)
{
    int32_t ret = -1;
    if (handle != nullptr) {
        auto handlePtr = static_cast<ColorSpaceConvertVideoHandleImpl *>(handle);
        ret = handlePtr->obj->Release();
    }
    return ret;
}

int32_t ColorSpaceConvertVideoFlush(ColorSpaceConvertVideoHandle *handle)
{
    int32_t ret = -1;
    if (handle != nullptr) {
        auto handlePtr = static_cast<ColorSpaceConvertVideoHandleImpl *>(handle);
        ret = handlePtr->obj->Flush();
    }
    return ret;
}

int32_t ColorSpaceConvertVideoGetOutputFormat(ColorSpaceConvertVideoHandle *handle, ArgumentType* format)
{
    int32_t ret = -1;
    if (handle != nullptr && format != nullptr) {
        auto handlePtr = static_cast<ColorSpaceConvertVideoHandleImpl *>(handle);
        auto formatPtr = static_cast<Format *>(format);
        ret = handlePtr->obj->GetOutputFormat(*formatPtr);
    }
    return ret;
}

int32_t ColorSpaceConvertVideoNotifyEos(ColorSpaceConvertVideoHandle *handle)
{
    int32_t ret = -1;
    if (handle != nullptr) {
        auto handlePtr = static_cast<ColorSpaceConvertVideoHandleImpl *>(handle);
        ret = handlePtr->obj->NotifyEos();
    }
    return ret;
}

int32_t ColorSpaceConvertVideoReleaseOutputBuffer(ColorSpaceConvertVideoHandle *handle, uint32_t index, bool render)
{
    int32_t ret = -1;
    if (handle != nullptr) {
        auto handlePtr = static_cast<ColorSpaceConvertVideoHandleImpl *>(handle);
        ret = handlePtr->obj->ReleaseOutputBuffer(index, render);
    }
    return ret;
}

int32_t ColorSpaceConvertVideoOnProducerBufferReleased(ColorSpaceConvertVideoHandle *handle)
{
    int32_t ret = -1;
    if (handle != nullptr) {
        auto handlePtr = static_cast<ColorSpaceConvertVideoHandleImpl *>(handle);
        ret = handlePtr->obj->OnProducerBufferReleased();
    }
    return ret;
}

}  // namespace VideoProcessingEngine
}  // namespace Media
}  // namespace OHOS
