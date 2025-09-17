/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "metadata_generator_video_impl.h"
#include <cstring>
#include <limits>
#include <memory>
#include <algorithm>
#include "vpe_log.h"
#include "algorithm_errors.h"
#include "algorithm_common.h"
#include "vpe_trace.h"
#include "algorithm_utils.h"
#include "securec.h"
#include "extension_manager.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
std::shared_ptr<MetadataGeneratorVideo> MetadataGeneratorVideo::Create()
{
    std::shared_ptr<MetadataGeneratorVideoImpl> impl = std::make_shared<MetadataGeneratorVideoImpl>();
    int32_t ret = impl->Init();
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, nullptr, "failed to init MetadataGeneratorVideoImpl");
    return impl;
}

std::shared_ptr<MetadataGeneratorVideo> MetadataGeneratorVideo::Create(std::shared_ptr<OpenGLContext> openglContext)
{
    std::shared_ptr<MetadataGeneratorVideoImpl> impl = std::make_shared<MetadataGeneratorVideoImpl>();
    int32_t ret = impl->Init(openglContext);
    CHECK_AND_RETURN_RET_LOG(ret == VPE_ALGO_ERR_OK, nullptr, "failed to init MetadataGeneratorVideoImpl");
    return impl;
}

MetadataGeneratorVideoImpl::MetadataGeneratorVideoImpl()
{
    requestCfg_.timeout = 0;
    requestCfg_.strideAlignment = 32; // 32 内存对齐
    requestCfg_.usage = 0;
    requestCfg_.format = 0;
    requestCfg_.width = 0;
    requestCfg_.height = 0;
    flushCfg_.timestamp = 0;
    flushCfg_.damage.x = 0;
    flushCfg_.damage.y = 0;
    flushCfg_.damage.w = 0;
    flushCfg_.damage.h = 0;
}

MetadataGeneratorVideoImpl::~MetadataGeneratorVideoImpl()
{
    Release();
}

int32_t MetadataGeneratorVideoImpl::Init()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(state_ == VPEAlgoState::UNINITIALIZED, VPE_ALGO_ERR_INVALID_STATE,
        "Init failed: not in UNINITIALIZED state");
    csc_ = MetadataGenerator::Create();
    CHECK_AND_RETURN_RET_LOG(csc_ != nullptr, VPE_ALGO_ERR_UNKNOWN, "MetadataGenerator Create failed");

    isRunning_.store(true);
    taskThread_ = std::make_shared<std::thread>(&MetadataGeneratorVideoImpl::OnTriggered, this);
    CHECK_AND_RETURN_RET_LOG(taskThread_ != nullptr, VPE_ALGO_ERR_UNKNOWN, "Fatal: No memory");

    state_ = VPEAlgoState::INITIALIZED;
    return VPE_ALGO_ERR_OK;
}

int32_t MetadataGeneratorVideoImpl::Init(std::shared_ptr<OpenGLContext> openglContext)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(state_ == VPEAlgoState::UNINITIALIZED, VPE_ALGO_ERR_INVALID_STATE,
        "Init failed: not in UNINITIALIZED state");
    csc_ = MetadataGenerator::Create(openglContext);
    CHECK_AND_RETURN_RET_LOG(csc_ != nullptr, VPE_ALGO_ERR_UNKNOWN, "MetadataGenerator Create failed");

    isRunning_.store(true);
    taskThread_ = std::make_shared<std::thread>(&MetadataGeneratorVideoImpl::OnTriggered, this);
    CHECK_AND_RETURN_RET_LOG(taskThread_ != nullptr, VPE_ALGO_ERR_UNKNOWN, "Fatal: No memory");

    state_ = VPEAlgoState::INITIALIZED;
    return VPE_ALGO_ERR_OK;
}

int32_t MetadataGeneratorVideoImpl::SetCallback(const std::shared_ptr<MetadataGeneratorVideoCallback> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, VPE_ALGO_ERR_INVALID_VAL, "Set callback failed: callback is NULL");
    CHECK_AND_RETURN_RET_LOG(state_ == VPEAlgoState::INITIALIZED || state_ == VPEAlgoState::CONFIGURING,
        VPE_ALGO_ERR_INVALID_STATE, "SetCallback failed: not in INITIALIZED or CONFIGURING state");
    cb_ = callback;
    state_ = VPEAlgoState::CONFIGURING;
    return VPE_ALGO_ERR_OK;
}

int32_t MetadataGeneratorVideoImpl::AttachToNewSurface(sptr<Surface> newSurface)
{
    std::lock_guard<std::mutex> lockrender(renderQueMutex_);
    for (auto it = outputBufferAvilQueBak_.begin(); it != outputBufferAvilQueBak_.end(); ++it) {
        auto buffer = it->second;
        GSError err = newSurface->AttachBufferToQueue(buffer->memory);
        CHECK_AND_RETURN_RET_LOG(err == GSERROR_OK, VPE_ALGO_ERR_UNKNOWN, "outputbuffer AttachToNewSurface fail");
    }
    return VPE_ALGO_ERR_OK;
}

int32_t MetadataGeneratorVideoImpl::GetReleaseOutBuffer()
{
    std::lock_guard<std::mutex> mapLock(renderQueMutex_);
    renderBufferMapBak_.clear();
    return VPE_ALGO_ERR_OK;
}


int32_t MetadataGeneratorVideoImpl::SetOutputSurfaceConfig(sptr<Surface> surface)
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

int32_t MetadataGeneratorVideoImpl::SetOutputSurfaceRunning(sptr<Surface> newSurface)
{
    std::lock_guard<std::mutex> outputLock(outputQueMutex_);
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
    newSurface->SetDefaultUsage(outputSurface_->GetDefaultUsage());
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

int32_t MetadataGeneratorVideoImpl::SetOutputSurface(sptr<Surface> surface)
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

sptr<Surface> MetadataGeneratorVideoImpl::CreateInputSurface()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(state_ == VPEAlgoState::INITIALIZED || state_ == VPEAlgoState::CONFIGURING, nullptr,
        "CreateInputSurface failed: not in INITIALIZED or CONFIGURING state");
    CHECK_AND_RETURN_RET_LOG(inputSurface_ == nullptr, nullptr, "inputSurface already exists");

    inputSurface_ = Surface::CreateSurfaceAsConsumer("ConvertInputSurface");
    CHECK_AND_RETURN_RET_LOG(inputSurface_ != nullptr, nullptr, "CreateSurfaceAsConsumer fail");
    sptr<IBufferConsumerListener> listener = new MetadataGeneratorBufferConsumerListener(this);
    GSError err = inputSurface_->RegisterConsumerListener(listener);
    CHECK_AND_RETURN_RET_LOG(err == GSERROR_OK, nullptr, "RegisterConsumerListener fail");

    sptr<IBufferProducer> producer = inputSurface_->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    CHECK_AND_RETURN_RET_LOG(producerSurface != nullptr, nullptr, "CreateSurfaceAsProducer fail");
    inputSurface_->SetQueueSize(inBufferCnt_);
    state_ = VPEAlgoState::CONFIGURING;

    return producerSurface;
}

int32_t MetadataGeneratorVideoImpl::Configure()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(state_ == VPEAlgoState::INITIALIZED || state_ == VPEAlgoState::CONFIGURING || state_ ==
        VPEAlgoState::STOPPED, VPE_ALGO_ERR_INVALID_STATE, "Configure failed: not in INITIALIZED or CONFIGURING state");
    MetadataGeneratorParameter param;
    param.algoType = MetadataGeneratorAlgoType::META_GEN_ALGO_TYPE_VIDEO;
    int32_t ret = csc_->SetParameter(param);
    state_ = (ret == VPE_ALGO_ERR_OK ? VPEAlgoState::CONFIGURING : VPEAlgoState::ERROR);
    return ret;
}

int32_t MetadataGeneratorVideoImpl::Prepare()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (state_ == VPEAlgoState::STOPPED) {
        state_ = VPEAlgoState::CONFIGURED;
        return VPE_ALGO_ERR_OK;
    }
    CHECK_AND_RETURN_RET_LOG(state_ == VPEAlgoState::CONFIGURING, VPE_ALGO_ERR_INVALID_STATE,
        "Prepare failed: not in CONFIGURING state");
    CHECK_AND_RETURN_RET_LOG(cb_ != nullptr && inputSurface_ != nullptr && outputSurface_ != nullptr,
        VPE_ALGO_ERR_INVALID_OPERATION, "Prepare faled: inputSurface or outputSurface or callback is null");

    state_ = VPEAlgoState::CONFIGURED;
    return VPE_ALGO_ERR_OK;
}

void MetadataGeneratorVideoImpl::InitBuffers()
{
    if (!isRunning_.load()) {
        VPE_LOGD("Skip when died.");
        return;
    }
    CHECK_AND_RETURN_LOG(outputSurface_ != nullptr, "outputSurface_ is nullptr");
    flushCfg_.damage.x = 0;
    flushCfg_.damage.y = 0;
    flushCfg_.damage.w = requestCfg_.width;
    flushCfg_.damage.h = requestCfg_.height;
    outputSurface_->CleanCache(true);
}

int32_t MetadataGeneratorVideoImpl::Start()
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
    cb_->OnState(static_cast<int32_t>(state_.load()));
    cvTaskStart_.notify_all();
    return VPE_ALGO_ERR_OK;
}

int32_t MetadataGeneratorVideoImpl::Stop()
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

int32_t MetadataGeneratorVideoImpl::Reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(
        state_ != VPEAlgoState::UNINITIALIZED, VPE_ALGO_ERR_INVALID_STATE, "Start failed: not in right state");
    std::unique_lock<std::mutex> lockTask(mtxTaskDone_);
    state_ = VPEAlgoState::INITIALIZED;
    cvTaskDone_.wait(lockTask, [this]() { return isProcessing_.load() == false; });

    csc_ = MetadataGenerator::Create();
    CHECK_AND_RETURN_RET_LOG(csc_ != nullptr, VPE_ALGO_ERR_UNKNOWN, "ColorSpaceConverter Create failed");
    isEos_.store(false);

    return VPE_ALGO_ERR_OK;
}

int32_t MetadataGeneratorVideoImpl::Release()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
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

int32_t MetadataGeneratorVideoImpl::Flush()
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
    renderBufferAvilMap_.clear();
    state_ = VPEAlgoState::FLUSHED;
    return VPE_ALGO_ERR_OK;
}

void MetadataGeneratorVideoImpl::CheckRequestCfg(sptr<SurfaceBuffer> inputBuffer)
{
    if (requestCfg_.width != inputBuffer->GetWidth() || requestCfg_.height != inputBuffer->GetHeight() ||
        requestCfg_.format != inputBuffer->GetFormat() || requestCfg_.usage != inputBuffer->GetUsage()) {
        requestCfg_.width = inputBuffer->GetWidth();
        requestCfg_.height = inputBuffer->GetHeight();
        requestCfg_.format = inputBuffer->GetFormat();
        requestCfg_.usage = inputBuffer->GetUsage();
        outputSurface_->CleanCache(true);
        outputSurface_->SetDefaultUsage(requestCfg_.usage);
        std::lock_guard<std::mutex> lock(outputQueMutex_);
        outputBufferAvilQueBak_.clear();
    }
}

void MetadataGeneratorVideoImpl::Process(std::shared_ptr<SurfaceBufferWrapper> inputBuffer)
{
    VPETrace videoTrace("MetadataGeneratorVideoImpl::Process");
    int32_t ret = VPE_ALGO_ERR_EXTENSION_PROCESS_FAILED;
    sptr<SurfaceBuffer> surfaceInputBuffer = inputBuffer->memory;
    CheckRequestCfg(surfaceInputBuffer);
    std::unique_lock<std::mutex> outputLock(outputQueMutex_);
    auto it = outputBufferAvilQueBak_.find(surfaceInputBuffer->GetSeqNum());
    if (it == outputBufferAvilQueBak_.end()) {
        ret = outputSurface_->AttachBufferToQueue(surfaceInputBuffer);
        CHECK_AND_RETURN_LOG(ret == GSERROR_OK, "AttachBufferToQueue failed %{public}d", ret);
        outputBufferAvilQueBak_.emplace(surfaceInputBuffer->GetSeqNum(), inputBuffer);
    }
    outputLock.unlock();
    {
        VPETrace cscTrace("MetadataGeneratorVideoImpl::csc_->Process");
        ret = csc_->Process(surfaceInputBuffer);
    }
    if (ret != 0 && cb_) {
        cb_->OnError(ret);
        inputSurface_->ReleaseBuffer(surfaceInputBuffer, -1);
    }
    if (!ret) {
        std::unique_lock<std::mutex> lockOnBq(renderQueMutex_);
        renderBufferAvilMap_.emplace(surfaceInputBuffer->GetSeqNum(), inputBuffer);
        lockOnBq.unlock();
        if (cb_) {
            cb_->OnOutputBufferAvailable(surfaceInputBuffer->GetSeqNum(), inputBuffer->bufferFlag);
        }
    }
}

bool MetadataGeneratorVideoImpl::WaitProcessing()
{
    if (!isRunning_.load()) {
        return false;
    }
    {
        std::unique_lock<std::mutex> lock(mtxTaskStart_);
        cvTaskStart_.wait(lock, [this]() {
            std::lock_guard<std::mutex> lock(mutex_);
            std::lock_guard<std::mutex> inQueueLock(onBqMutex_);
            std::lock_guard<std::mutex> outQueueLock(renderQueMutex_);
            if (initBuffer_.load()) {
                InitBuffers();
                initBuffer_.store(false);
            }
            return ((inputBufferAvilQue_.size() > 0) || !isRunning_.load());
        });
    }

    return true;
}

bool MetadataGeneratorVideoImpl::AcquireInputBuffers(std::shared_ptr<SurfaceBufferWrapper>& inputBuffer)
{
    std::lock_guard<std::mutex> lockOnBq(onBqMutex_);
    if (inputBufferAvilQue_.size() == 0) {
        if (state_ == VPEAlgoState::STOPPED) {
            cb_->OnState(static_cast<int32_t>(state_.load()));
        }
        return false;
    }
    inputBuffer = inputBufferAvilQue_.front();
    inputBufferAvilQue_.pop();
    return inputBuffer != nullptr;
}

void MetadataGeneratorVideoImpl::DoTask()
{
    std::shared_ptr<SurfaceBufferWrapper> inputBuffer = nullptr;
    while (true) {
        std::lock_guard<std::mutex> lockTask(mtxTaskDone_);
        if (!isRunning_.load()) {
            return;
        }
        isProcessing_.store(true);

        if (!AcquireInputBuffers(inputBuffer)) {
            break;
        }
        if (inputBuffer->bufferFlag == MDG_BUFFER_FLAG_EOS) {
            std::unique_lock<std::mutex> outputLock(outputQueMutex_);
            auto it = outputBufferAvilQueBak_.find(inputBuffer->memory->GetSeqNum());
            if (it == outputBufferAvilQueBak_.end()) {
                int32_t ret = outputSurface_->AttachBufferToQueue(inputBuffer->memory);
                CHECK_AND_RETURN_LOG(ret == GSERROR_OK, "AttachBufferToQueue failed %{public}d", ret);
            }
            outputLock.unlock();
            {
                std::unique_lock<std::mutex> lockOnBq(renderQueMutex_);
                renderBufferAvilMap_.emplace(inputBuffer->memory->GetSeqNum(), inputBuffer);
            }
            if (cb_) {
                cb_->OnOutputBufferAvailable(inputBuffer->memory->GetSeqNum(), MDG_BUFFER_FLAG_EOS);
            }
            break;
        }
        Process(inputBuffer);
    }
    isProcessing_.store(false);
    cvTaskDone_.notify_all();
}

void MetadataGeneratorVideoImpl::OnTriggered()
{
    while (true) {
        if (!WaitProcessing()) {
            break;
        }

        DoTask();
    }
}

int32_t MetadataGeneratorVideoImpl::ReleaseOutputBuffer(uint32_t index, bool render)
{
    CHECK_AND_RETURN_RET_LOG(state_ == VPEAlgoState::RUNNING || state_ == VPEAlgoState::EOS, VPE_ALGO_ERR_INVALID_STATE,
        "ReleaseOutputBuffer failed: not in RUNNING or EOS state");
    
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
        inputSurface_->ReleaseBuffer(buffer->memory, -1);
    }
    return VPE_ALGO_ERR_OK;
}

int32_t MetadataGeneratorVideoImpl::NotifyEos()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::lock_guard<std::mutex> lockOnBq(onBqMutex_);
    CHECK_AND_RETURN_RET_LOG(state_ == VPEAlgoState::RUNNING, VPE_ALGO_ERR_INVALID_STATE,
        "NotifyEos failed: not in RUNNING state");
    state_ = VPEAlgoState::EOS;
    isEos_.store(true);
    std::shared_ptr<SurfaceBufferWrapper> buf = std::make_shared<SurfaceBufferWrapper>();
    buf->bufferFlag = MDG_BUFFER_FLAG_EOS;
    inputBufferAvilQue_.push(buf);

    cvTaskStart_.notify_all();

    return VPE_ALGO_ERR_OK;
}

GSError MetadataGeneratorVideoImpl::OnProducerBufferReleased()
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
        auto it = renderBufferMapBak_.find(bufSeqNum);
        if (it != renderBufferMapBak_.end()) {
            lastSurfaceSequence_ = bufSeqNum;
            renderBufferMapBak_.erase(bufSeqNum);
            inputSurface_->ReleaseBuffer(buf->memory, -1);
        } else {
            std::lock_guard<std::mutex> lock(outputQueMutex_);
            auto it2 = outputBufferAvilQueBak_.find(buf->memory->GetSeqNum());
            if (it2 == outputBufferAvilQueBak_.end()) {
                outputSurface_->DetachBufferFromQueue(buf->memory);
            }
        }
    }

    if (state_ == VPEAlgoState::RUNNING || state_ == VPEAlgoState::EOS) {
        cvTaskStart_.notify_all();
    }

    return GSERROR_OK;
}

GSError MetadataGeneratorVideoImpl::OnConsumerBufferAvailable()
{
    if (!isRunning_.load()) {
        VPE_LOGD("Skip when died.");
        return GSERROR_OK;
    }
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
    constexpr uint32_t waitForEver = -1; // wait fence -1
    if (buffer->fence != nullptr) {
        (void)buffer->fence->Wait(waitForEver);
        buffer->memory->InvalidateCache();
    }
    inputBufferAvilQue_.push(buffer);

    if (!getUsage_) {
        requestCfg_.usage = buffer->memory->GetUsage();
        getUsage_ = true;
        requestCfg_.width = buffer->memory->GetWidth();
        requestCfg_.height = buffer->memory->GetHeight();
        requestCfg_.format = buffer->memory->GetFormat();
        outputSurface_->SetDefaultUsage(requestCfg_.usage);
        initBuffer_.store(true);
    }

    if (state_ == VPEAlgoState::RUNNING) {
        cvTaskStart_.notify_all();
    }

    return GSERROR_OK;
}

void MetadataGeneratorBufferConsumerListener::OnBufferAvailable()
{
    if (process_ != nullptr) {
        process_->OnConsumerBufferAvailable();
    }
}

} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
