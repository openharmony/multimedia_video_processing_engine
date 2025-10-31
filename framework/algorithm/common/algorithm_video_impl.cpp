/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "algorithm_video_impl.h"

#include <chrono>
#include <limits>
#include <list>
#include <sstream>

#include "algorithm_utils.h"
#include "vpe_trace.h"

using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;
using namespace std::chrono_literals;

namespace {
constexpr uint32_t WAIT_FOR_EVER = std::numeric_limits<uint32_t>::max();
constexpr uint32_t BUFFER_QUEUE_SIZE = 5;
} // namespace

std::string VpeVideoImpl::ToString(const sptr<SurfaceBuffer>& buffer, const std::string& tag) const
{
    if (buffer == nullptr) {
        return "null";
    }
    std::stringstream stream;
    if (!tag.empty()) {
        stream << tag << " ";
    }
    stream << "id:" << buffer->GetSeqNum() << " fd:" << buffer->GetFileDescriptor() <<
        " " << buffer->GetWidth() << "x" << buffer->GetHeight() <<
        " format:" << buffer->GetFormat() << " usage:0x" << std::hex << buffer->GetUsage();
    return stream.str();
}

std::string VpeVideoImpl::ToString(const BufferRequestConfig& requestCfg) const
{
    std::stringstream stream;
    stream << requestCfg.width << "x" << requestCfg.height << " format:" << requestCfg.format <<
        " usage:0x" << std::hex << requestCfg.usage;
    return stream.str();
}

VpeVideoImpl::~VpeVideoImpl()
{
    VPE_LOGD("Step in");
    Deinitialize();
}

VPEAlgoErrCode VpeVideoImpl::RegisterCallback(const std::shared_ptr<VpeVideoCallback>& callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, VPE_ALGO_ERR_INVALID_VAL, "Invalid input: callback is null!");

    return ExecuteWhenIdle(
        [this, callback]() {
            cb_ = callback;
            return VPE_ALGO_ERR_OK;
        }, "Registration of callbacks during running is not allowed!", VPE_LOG_INFO);
}

VPEAlgoErrCode VpeVideoImpl::SetOutputSurface(const sptr<Surface>& surface)
{
    CHECK_AND_RETURN_RET_LOG(surface != nullptr, VPE_ALGO_ERR_INVALID_VAL, "Invalid input: surface is null!");
    CHECK_AND_RETURN_RET_LOG(!surface->IsConsumer(), VPE_ALGO_ERR_INVALID_VAL,
        "Invalid input: surface is NOT producer!");
    CHECK_AND_RETURN_RET_LOG(IsProducerSurfaceValid(surface), VPE_ALGO_ERR_INVALID_VAL,
        "Invalid input: surface is invalid!");

    std::lock_guard<std::mutex> lock(lock_);
    std::lock_guard<std::mutex> producerLock(producerLock_);
    if (producer_ != nullptr) {
        if (producer_->GetUniqueId() == surface->GetUniqueId()) {
            VPE_LOGD("Oops! The same surface(%" PRIu64 ")", surface->GetUniqueId());
            return VPE_ALGO_ERR_OK;
        }
        producer_->UnRegisterReleaseListener();
        producer_->CleanCache(true);
    }
    surface->UnRegisterReleaseListener();
    GSError err = surface->RegisterReleaseListener([this](sptr<SurfaceBuffer>&) { return OnProducerBufferReleased(); });
    CHECK_AND_RETURN_RET_LOG(err == GSERROR_OK, VPE_ALGO_ERR_UNKNOWN, "RegisterReleaseListener failed!");
    VPE_LOGI("Set output(%" PRIu64 ") buffer queue size to %{public}u", surface->GetUniqueId(), BUFFER_QUEUE_SIZE);
    surface->SetQueueSize(BUFFER_QUEUE_SIZE);
    surface->Connect();
    surface->CleanCache();
    CHECK_AND_RETURN_RET_LOG(AttachAndRefreshProducerBuffers(surface), VPE_ALGO_ERR_UNKNOWN,
        "Failed to attach buffers to new output surface(%" PRIu64 ")!", surface->GetUniqueId());;
    if (state_.load() != VPEState::IDLE) {
        cvTrigger_.notify_one();
    }
    producer_ = surface;

    VPEAlgoErrCode ret = VPE_ALGO_ERR_OK;
    if (isEnable_.load()) {
        ret = UpdateRequestCfg(surface, requestCfg_);
    }
    VPE_LOGD("requestCfg_({ %{public}s }) ret:%{public}d", VpeVideoImpl::ToString(requestCfg_).c_str(), ret);

    isForceUpdateProducer_ = true;
    if (ret == VPE_ALGO_ERR_OK) {
        if (consumer_ != nullptr) {
            return UpdateProducerLocked();
        }
    }
    return ret;
}

sptr<Surface> VpeVideoImpl::GetInputSurface()
{
    sptr<Surface> producerSurface;
    if (ExecuteWhenIdle([this, &producerSurface]() {
        producerSurface = CreateConsumerSurfaceLocked();
        return VPE_ALGO_ERR_OK;
    }, "Getting a surface during running is not allowed!", VPE_LOG_INFO) != VPE_ALGO_ERR_OK) {
        return nullptr;
    }
    return producerSurface;
}

VPEAlgoErrCode VpeVideoImpl::Start()
{
    VPE_LOGD("step in");
    return ExecuteWhenIdle(
        [this]() {
            CHECK_AND_RETURN_RET_LOG(isInitialized_.load(), VPE_ALGO_ERR_INVALID_OPERATION, "NOT initialized!");
            CHECK_AND_RETURN_RET_LOG(consumer_ != nullptr && producer_ != nullptr && cb_ != nullptr,
                VPE_ALGO_ERR_INVALID_OPERATION, "The input surface, output surface or callback is NOT ready!");
            state_ = VPEState::RUNNING;
            OnStateLocked(VPEAlgoState::RUNNING);
            return VPE_ALGO_ERR_OK;
        }, "Already start!", VPE_LOG_INFO);
}

VPEAlgoErrCode VpeVideoImpl::Stop()
{
    VPE_LOGD("step in");
    auto err = ExecuteWhenRunning([this]() {
        state_ = VPEState::STOPPING;
        cvTrigger_.notify_one();
        return VPE_ALGO_ERR_OK;
    }, "Already stop!", VPE_LOG_INFO);
    return err;
}

VPEAlgoErrCode VpeVideoImpl::Release()
{
    return Deinitialize();
}

VPEAlgoErrCode VpeVideoImpl::Flush()
{
    VPE_LOGD("step in");
    CHECK_AND_RETURN_RET_LOG(isRunning_.load(), VPE_ALGO_ERR_INVALID_OPERATION, "Flush must be called during running!");
    std::lock_guard<std::mutex> lock(lock_);
    {
        std::unique_lock<std::mutex> lockTask(taskLock_);
        cvDone_.wait(lockTask, [this]() { return !isProcessing_.load(); });
    }

    if (isEnable_.load()) {
        std::queue<SurfaceBufferInfo> tempQueue1;
        std::queue<SurfaceBufferInfo> tempQueue2;
        {
            std::lock_guard<std::mutex> consumerBufferLock(consumerBufferLock_);
            std::lock_guard<std::mutex> bufferLock(bufferLock_);
            consumerBufferQueue_.swap(tempQueue1);
            for (auto& [index, bufferInfo] : renderBufferQueue_) {
                producerBufferQueue_.push(bufferInfo);
            }
            renderBufferQueue_.clear();
            attachBufferQueue_.swap(tempQueue2);
            attachBufferIDs_.clear();
        }
        ClearConsumerLocked(tempQueue1);
        ClearConsumerLocked(tempQueue2);
    } else {
        CHECK_AND_RETURN_RET_LOG(producer_ != nullptr, VPE_ALGO_ERR_INVALID_OPERATION, "output surface is null!");
        producer_->CleanCache(false);
        ClearBufferQueues();
    }
    VPE_LOGD("step out");
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode VpeVideoImpl::Enable()
{
    CHECK_AND_RETURN_RET_LOG(!isEnable_.load(), VPE_ALGO_ERR_INVALID_OPERATION, "Already enabled!");

    std::lock_guard<std::mutex> lock(lock_);
    isEnable_ = true;
    isEnableChange_ = true;
    if (producer_ == nullptr) {
        return VPE_ALGO_ERR_OK;
    }

    auto ret = UpdateRequestCfg(producer_, requestCfg_);
    VPE_LOGD("requestCfg_({ %{public}s })", VpeVideoImpl::ToString(requestCfg_).c_str());
    if (requestCfg_.usage == orgRequestCfg_.usage &&
        requestCfg_.format == orgRequestCfg_.format &&
        requestCfg_.width == orgRequestCfg_.width &&
        requestCfg_.height == orgRequestCfg_.height) {
        producer_->CleanCache(false);
        ClearBufferQueues();
    }
    return ret;
}

VPEAlgoErrCode VpeVideoImpl::Disable()
{
    CHECK_AND_RETURN_RET_LOG(isEnable_.load(), VPE_ALGO_ERR_INVALID_OPERATION, "Already disabled!");

    std::lock_guard<std::mutex> lock(lock_);
    return DisableLocked();
}

VPEAlgoErrCode VpeVideoImpl::NotifyEos()
{
    VPE_LOGD("step in.");
    return ExecuteWhenNotIdle(
        [this]() {
            VPE_LOGD("Try to NotifyEos...");
            {
                std::lock_guard<std::mutex> consumerBufferLock(consumerBufferLock_);
                if (!isBufferQueueReady_.load()) {
                    isBufferQueueReady_ = true;
                    VPE_LOGD("Use requestCfg_({ %{public}s }) to prepare buffers.",
                        VpeVideoImpl::ToString(requestCfg_).c_str());
                    needPrepareBuffers_ = true;
                }
                SurfaceBufferInfo bufferInfo{};
                bufferInfo.bufferFlag = VPE_BUFFER_FLAG_EOS;
                consumerBufferQueue_.push(bufferInfo);
            }
            cvTrigger_.notify_one();
            VPE_LOGD("NotifyEos done.");
            return VPE_ALGO_ERR_OK;
        }, "Notify EOS must be called during running!", VPE_LOG_INFO);
}

VPEAlgoErrCode VpeVideoImpl::ReleaseOutputBuffer(uint32_t index, bool render)
{
    CHECK_AND_RETURN_RET_LOG(state_.load() != VPEState::IDLE, VPE_ALGO_ERR_INVALID_OPERATION,
        "Release output buffer must be called during running!");
    return RenderOutputBuffer(index, -1, render);
}

VPEAlgoErrCode VpeVideoImpl::RenderOutputBufferAtTime(uint32_t index, int64_t renderTimestamp)
{
    CHECK_AND_RETURN_RET_LOG(state_.load() != VPEState::IDLE, VPE_ALGO_ERR_INVALID_OPERATION,
        "Render output buffer must be called during running!");
    return RenderOutputBuffer(index, renderTimestamp, true);
}

bool VpeVideoImpl::IsInitialized() const
{
    return isInitialized_.load();
}

VPEAlgoErrCode VpeVideoImpl::Initialize()
{
    VPE_LOGD("Start to initializing... this:%p", this);
    std::lock_guard<std::mutex> lock(lock_);
    if (isInitialized_.load()) {
        VPE_LOGD("Already initialize!");
        return VPE_ALGO_ERR_OK;
    }
    isRunning_ = true;
    worker_ = std::thread([this]() {
        while (isRunning_.load()) {
            if (!WaitTrigger()) {
                continue;
            }
            VPE_LOGD("ProcessBuffers");
            ProcessBuffers();
            CheckStopping();
        };
    });
    auto errorCode = OnInitialize();
    isInitialized_ = true;
    VPE_LOGD("OnInitialize() return %{public}d. this:%p", errorCode, this);
    return errorCode;
}

VPEAlgoErrCode VpeVideoImpl::Deinitialize()
{
    VPE_LOGD("Start to deinitializing... this:%p", this);
    std::unique_lock<std::mutex> lock(lock_);
    if (!isInitialized_.load()) {
        VPE_LOGD("Already deinitialize!");
        return VPE_ALGO_ERR_OK;
    }
    isRunning_.store(false);
    isInitialized_.store(false);
    if (state_.load() == VPEState::RUNNING) {
        state_ = VPEState::STOPPING;
    }
    lock.unlock();
    cvTrigger_.notify_one();
    if (worker_.joinable()) {
        VPE_LOGD("Wait for thread ending... this:%p", this);
        worker_.join();
    }
    lock.lock();
    CheckStoppingLocked();
    VPEAlgoErrCode errorCode = OnDeinitialize();
    cb_ = nullptr;
    std::lock_guard<std::mutex> producerLock(producerLock_);
    ClearBufferQueues();
    if (consumer_ != nullptr) {
        consumer_->UnregisterConsumerListener();
        consumer_ = nullptr;
    }
    if (producer_ != nullptr) {
        producer_->UnRegisterReleaseListener();
        producer_->CleanCache(true);
        producer_ = nullptr;
    }
    VPE_LOGD("OnDeinitialize() return %{public}d. this:%p", errorCode, this);
    return errorCode;
}

void VpeVideoImpl::RefreshBuffers()
{
    if (!isRunning_.load()) {
        VPE_LOGI("Skip when died.");
        return;
    }
    std::lock_guard<std::mutex> lock(lock_);
    if (state_.load() != VPEState::RUNNING) {
        VPE_LOGD("Skip refreshing during Non-Running.");
        return;
    }
    if (!isEnable_.load()) {
        VPE_LOGD("Skip refreshing when it is disabled.");
        return;
    }

    if (!isBufferQueueReady_.load()) {
        VPE_LOGD("Skip refreshing when buffer queue is not ready.");
        return;
    }

    VPE_LOGD("Clear all buffer queues.");
    ClearBufferQueues();
}

void VpeVideoImpl::OnOutputFormatChanged(const Format& format)
{
    if (!isRunning_.load()) {
        VPE_LOGI("Skip when died.");
        return;
    }
    std::lock_guard<std::mutex> lock(lock_);
    if (cb_ != nullptr) {
        VPE_LOGD("OnOutputFormatChanged()");
        cb_->OnOutputFormatChanged(format);
    }
}

VPEAlgoErrCode VpeVideoImpl::OnInitialize()
{
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode VpeVideoImpl::OnDeinitialize()
{
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode VpeVideoImpl::Process([[maybe_unused]] const sptr<SurfaceBuffer>& sourceImage,
    [[maybe_unused]] sptr<SurfaceBuffer>& destinationImage)
{
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode VpeVideoImpl::ResetAfterDisable()
{
    return VPE_ALGO_ERR_OK;
}

bool VpeVideoImpl::IsDisableAfterProcessFail()
{
    return true;
}

bool VpeVideoImpl::IsProducerSurfaceValid([[maybe_unused]] const sptr<Surface>& surface)
{
    return true;
}

bool VpeVideoImpl::IsConsumerBufferValid([[maybe_unused]] const sptr<SurfaceBuffer>& buffer)
{
    return true;
}

VPEAlgoErrCode VpeVideoImpl::UpdateRequestCfg([[maybe_unused]] const sptr<Surface>& surface,
    [[maybe_unused]] BufferRequestConfig& requestCfg)
{
    return VPE_ALGO_ERR_OK;
}

void VpeVideoImpl::UpdateRequestCfg([[maybe_unused]] const sptr<SurfaceBuffer>& consumerBuffer,
    [[maybe_unused]] BufferRequestConfig& requestCfg)
{
}

void VpeVideoImpl::OnError(VPEAlgoErrCode errorCode)
{
    if (cb_ != nullptr) {
        VPE_LOGD("OnError(%{public}s)", AlgorithmUtils::ToString(errorCode).c_str());
        cb_->OnError(errorCode);
    }
}

void VpeVideoImpl::OnStateLocked(VPEAlgoState state)
{
    if (cb_ != nullptr) {
        VPE_LOGD("OnState(%{public}s)", AlgorithmUtils::ToString(state).c_str());
        cb_->OnState(state);
    }
}

void VpeVideoImpl::OnEffectChange(uint32_t type)
{
    if (cb_ != nullptr) {
        VPE_LOGD("OnEffectChange(0x%{public}x)", type);
        cb_->OnEffectChange(type);
    }
}

void VpeVideoImpl::OnOutputBufferAvailable(uint32_t index, const VpeBufferInfo& info)
{
    if (cb_ != nullptr) {
        VPE_LOGD("OnOutputBufferAvailable(%{public}u, flag:%{public}d)", index, info.flag);
        cb_->OnOutputBufferAvailable(index, info);
    }
}

GSError VpeVideoImpl::OnConsumerBufferAvailable()
{
    if (!isRunning_.load()) {
        VPE_LOGI("Skip when died.");
        return GSERROR_OK;
    }
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(consumer_ != nullptr, GSERROR_OK, "Input surface is null!");
    if (state_.load() != VPEState::RUNNING) {
        VPE_LOGD("NOT running now!");
        return GSERROR_OK;
    }

    SurfaceBufferInfo bufferInfo{};
    OHOS::Rect damage;
    GSError err = consumer_->AcquireBuffer(bufferInfo.buffer, bufferInfo.fence, bufferInfo.timestamp, damage);
    if (err != GSERROR_OK || bufferInfo.buffer == nullptr) {
        VPE_LOGE("Failed to acquire buffer, ret:%{public}s!", AlgorithmUtils::ToString(err).c_str());
        return err;
    }
    if (bufferInfo.fence != nullptr) [[likely]] {
        bufferInfo.fence->Wait(WAIT_FOR_EVER);
    } else [[unlikely]] {
        VPE_LOGW("Fence is null for { %{public}s }", VpeVideoImpl::ToString(bufferInfo.buffer).c_str());
    }
    VPE_LOGD("consumer_->AcquireBuffer({ %{public}s })", VpeVideoImpl::ToString(bufferInfo.buffer).c_str());
    if (!IsConsumerBufferValid(bufferInfo.buffer)) {
        VPE_LOGD("Invalid buffer:{ %{public}s }", VpeVideoImpl::ToString(bufferInfo.buffer).c_str());
        DisableLocked();
    }
    lastConsumerBufferId_ = bufferInfo.buffer->GetSeqNum();

    {
        std::lock_guard<std::mutex> consumerBufferLock(consumerBufferLock_);
        if (!isBufferQueueReady_.load()) {
            isBufferQueueReady_ = true;
            requestCfg_.usage = bufferInfo.buffer->GetUsage();
            requestCfg_.timeout = 0;
            requestCfg_.strideAlignment = 32; // 32 bits align
            UpdateRequestCfg(bufferInfo.buffer, requestCfg_);
            VPE_LOGD("Use requestCfg_({ %{public}s }) to prepare buffers.",
                VpeVideoImpl::ToString(requestCfg_).c_str());
            needPrepareBuffers_ = true;
        }
        consumerBufferQueue_.push(bufferInfo);
    }
    cvTrigger_.notify_one();
    return GSERROR_OK;
}

GSError VpeVideoImpl::OnProducerBufferReleased()
{
    VPE_LOGD("step in");
    if (!isRunning_.load()) {
        VPE_LOGI("Skip when died.");
        return GSERROR_OK;
    }
    std::lock_guard<std::mutex> producerLock(producerLock_);
    CHECK_AND_RETURN_RET_LOG(producer_ != nullptr, GSERROR_OK, "Output surface is null!");

    {
        std::lock_guard<std::mutex> bufferLock(bufferLock_);
        GSError err = GSERROR_OK;
        SurfaceBufferInfo bufferInfo{};
        CHECK_AND_RETURN_RET_LOG(RequestBuffer(bufferInfo, err, VPE_LOG_INFO_EX), err, "Failed to request buffer!");
        PopBuffer(flushBufferQueue_, bufferInfo.buffer->GetSeqNum(), bufferInfo,
            [this](sptr<SurfaceBuffer>& buffer) {
                VPE_LOGD("OnProducerBufferReleased: Pop flush buffer { %{public}s } flushBQ=%{public}zu",
                    VpeVideoImpl::ToString(buffer).c_str(), flushBufferQueue_.size());
            }, VPE_LOG_INFO_EX);
    }
    if (state_.load() != VPEState::IDLE) {
        cvTrigger_.notify_one();
    }
    VPE_LOGD("step out");
    return GSERROR_OK;
}

bool VpeVideoImpl::CheckEnableNotifyStatus(bool isEnable, const std::string& status)
{
    if (!isRunning_.load()) {
        VPE_LOGI("Skip when died.");
        return false;
    }
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(isEnableChange_.load(), false, "Oops! Keep enable:%{public}d", isEnable_.load());
    if (isEnable_.load() != isEnable) {
        VPE_LOGD("Try to notify '%{public}s' but enable:%{public}d", status.c_str(), isEnable_.load());
        return false;
    }
    isEnableChange_ = false;
    return true;
}

VPEAlgoErrCode VpeVideoImpl::DisableLocked()
{
    CHECK_AND_RETURN_RET_LOG(isEnable_.load(), VPE_ALGO_ERR_INVALID_OPERATION, "Already disabled!");

    isEnable_ = false;
    isEnableChange_ = true;
    VPE_LOGD("requestCfg_({ %{public}s })", VpeVideoImpl::ToString(requestCfg_).c_str());
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode VpeVideoImpl::UpdateProducerLocked()
{
    auto transform = consumer_->GetTransform();
    GSError err;
    if (isForceUpdateProducer_ || lastTransform_ != transform) {
        err = producer_->SetTransform(transform);
        CHECK_AND_RETURN_RET_LOG(err == GSERROR_OK, VPE_ALGO_ERR_UNKNOWN,
            "Failed to SetTransform(%{public}d), ret:%{public}s!", transform, AlgorithmUtils::ToString(err).c_str());
        lastTransform_ = transform;
        VPE_LOGD("producer_->SetTransform(%{public}d) success.", transform);
    }
    if (lastConsumerBufferId_ == 0) {
        return VPE_ALGO_ERR_OK;
    }
    ScalingMode scaleMode;
    err = consumer_->GetScalingMode(lastConsumerBufferId_, scaleMode);
    CHECK_AND_RETURN_RET_LOG(err == GSERROR_OK, VPE_ALGO_ERR_UNKNOWN,
        "Failed to GetScalingMode(%{public}u), ret:%{public}s!",
        lastConsumerBufferId_, AlgorithmUtils::ToString(err).c_str());
    if (isForceUpdateProducer_ || lastScalingMode_ != scaleMode) {
        err = producer_->SetScalingMode(scaleMode);
        CHECK_AND_RETURN_RET_LOG(err == GSERROR_OK, VPE_ALGO_ERR_UNKNOWN,
            "Failed to SetScalingMode(%{public}d), ret:%{public}s!", scaleMode, AlgorithmUtils::ToString(err).c_str());
        lastScalingMode_ = scaleMode;
        VPE_LOGD("producer_->SetScalingMode(%{public}d) success.", scaleMode);
    }
    isForceUpdateProducer_ = false;
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode VpeVideoImpl::RenderOutputBuffer(uint32_t index, int64_t renderTimestamp, bool render)
{
    if (!isRunning_.load()) {
        VPE_LOGI("Skip when died.");
        return VPE_ALGO_ERR_INVALID_OPERATION;
    }
    std::unique_lock<std::mutex> bufferLock(bufferLock_);
    auto it = renderBufferQueue_.find(index);
    CHECK_AND_RETURN_RET_LOG(it != renderBufferQueue_.end(), VPE_ALGO_ERR_INVALID_PARAM,
        "Invalid input: index=%{public}u!", index);
    SurfaceBufferInfo bufferInfo = it->second;
    renderBufferQueue_.erase(it);
    bufferLock.unlock();

    if (render) {
        BufferFlushConfig flushcfg{};
        flushcfg.damage.w = bufferInfo.buffer->GetWidth();
        flushcfg.damage.h = bufferInfo.buffer->GetHeight();
        flushcfg.timestamp = (renderTimestamp == -1) ? bufferInfo.timestamp : renderTimestamp;
        {
            std::lock_guard<std::mutex> producerLock(producerLock_);
            CHECK_AND_RETURN_RET_LOG(producer_ != nullptr, VPE_ALGO_ERR_INVALID_OPERATION, "Output surface is null!");
            auto ret = producer_->FlushBuffer(bufferInfo.buffer, -1, flushcfg);
            VPE_LOGD("producer_(%" PRIu64 ")->FlushBuffer({ %{public}s })=%{public}s flushBQ=%{public}zu",
                producer_->GetUniqueId(), VpeVideoImpl::ToString(bufferInfo.buffer).c_str(),
                AlgorithmUtils::ToString(ret).c_str(), flushBufferQueue_.size() + 1);
            if (ret != GSERROR_OK) {
                return VPE_ALGO_ERR_UNKNOWN;
            }
        }
        bufferLock.lock();
        flushBufferQueue_.push(bufferInfo);
        auto itCache = producerBufferCache_.find(bufferInfo.buffer->GetSeqNum());
        if (itCache != producerBufferCache_.end()) {
            itCache->second.isFlushed = true;
            itCache->second.flushedTime = std::chrono::steady_clock::now();
        }
    } else {
        VPE_LOGD("reback { %{public}s } producerBQ=%{public}zu",
            VpeVideoImpl::ToString(bufferInfo.buffer).c_str(), producerBufferQueue_.size() + 1);
        bufferLock.lock();
        producerBufferQueue_.push(bufferInfo);
        bufferLock.unlock();
        if (state_.load() != VPEState::IDLE) {
            cvTrigger_.notify_one();
        }
    }
    return VPE_ALGO_ERR_OK;
}

sptr<Surface> VpeVideoImpl::CreateConsumerSurfaceLocked()
{
    CHECK_AND_RETURN_RET_LOG(consumer_ == nullptr, nullptr, "input surface already exists!");

    consumer_ = Surface::CreateSurfaceAsConsumer("VideoProcessingSurface");
    CHECK_AND_RETURN_RET_LOG(consumer_ != nullptr, nullptr, "Failed to create consumer surface!");
    sptr<IBufferConsumerListener> listener = new(std::nothrow) ConsumerListener(shared_from_this());
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, nullptr, "Failed to create consumer surface listener!");
    CHECK_AND_RETURN_RET_LOG(consumer_->RegisterConsumerListener(listener) == GSERROR_OK, nullptr,
        "Failed to register consumer surface listener!");

    sptr<IBufferProducer> producer = consumer_->GetProducer();
    sptr<Surface> producerSurface = Surface::CreateSurfaceAsProducer(producer);
    CHECK_AND_RETURN_RET_LOG(producerSurface != nullptr, nullptr, "Failed to create producer surface!");
    producerSurface->SetDefaultUsage(BUFFER_USAGE_CPU_READ | BUFFER_USAGE_CPU_WRITE |
        BUFFER_USAGE_MEM_DMA | BUFFER_USAGE_MEM_MMZ_CACHE | BUFFER_USAGE_HW_COMPOSER); // [TEST]
    consumer_->SetQueueSize(BUFFER_QUEUE_SIZE);
    VPE_LOGI("Set input buffer queue size to %{public}u", BUFFER_QUEUE_SIZE);

    hasConsumer_ = true;
    return producerSurface;
}

bool VpeVideoImpl::RequestBuffer(SurfaceBufferInfo& bufferInfo, GSError& errorCode,
    const LogInfoEx& logInfos)
{
    CHECK_AND_RETURN_RET_LOG(hasConsumer_.load(), false, "Input surface is null!");
    errorCode = producer_->RequestBuffer(bufferInfo.buffer, bufferInfo.fence, requestCfg_);
    if (errorCode != GSERROR_OK || bufferInfo.buffer == nullptr) {
        VPE_EX_LOGW(logInfos, "Failed to producer_(%" PRIu64 ")->RequestBuffer(requestCfg={ %{public}s }), "
            "ret:%{public}s", producer_->GetUniqueId(),
            VpeVideoImpl::ToString(requestCfg_).c_str(), AlgorithmUtils::ToString(errorCode).c_str());
        return false;
    }
    producerBufferQueue_.push(bufferInfo);
    AddBufferToCache(bufferInfo);
    if (!isEnable_.load()) {
        VPE_EX_LOGD(logInfos, "producer_(%" PRIu64 ")->RequestBuffer({ %{public}s }) and try to release.",
            producer_->GetUniqueId(), VpeVideoImpl::ToString(bufferInfo.buffer).c_str());
        auto it = attachBufferIDs_.find(bufferInfo.buffer->GetSeqNum());
        if (it != attachBufferIDs_.end()) {
            PopBuffer(attachBufferQueue_, bufferInfo.buffer->GetSeqNum(), bufferInfo,
                [this, &logInfos](sptr<SurfaceBuffer>& buffer) {
                    CHECK_AND_RETURN_LOG(buffer != nullptr, "Attach buffer is null!");
                    attachBufferIDs_.erase(buffer->GetSeqNum());
                    auto ret = consumer_->ReleaseBuffer(buffer, -1);
                    VPE_EX_LOGD(logInfos, "consumer_->ReleaseBuffer({ %{public}s })=%{public}s cache=%{public}zu",
                        VpeVideoImpl::ToString(buffer).c_str(), AlgorithmUtils::ToString(ret).c_str(),
                        attachBufferQueue_.size());
                }, ADD_VPE_LOG_INFO(logInfos));
        }
    } else {
        VPE_EX_LOGD(logInfos, "producer_(%" PRIu64 ")->RequestBuffer({ %{public}s })", producer_->GetUniqueId(),
            VpeVideoImpl::ToString(bufferInfo.buffer).c_str());
        if (attachBufferQueue_.empty()) {
            return true;
        }
        bufferInfo = attachBufferQueue_.front();
        attachBufferQueue_.pop();
        if (bufferInfo.buffer != nullptr) {
            attachBufferIDs_.erase(bufferInfo.buffer->GetSeqNum());
            auto ret = consumer_->ReleaseBuffer(bufferInfo.buffer, -1);
            VPE_EX_LOGD(logInfos, "consumer_->ReleaseBuffer({ %{public}s })=%{public}s cache->%{public}zu",
                VpeVideoImpl::ToString(bufferInfo.buffer).c_str(), AlgorithmUtils::ToString(ret).c_str(),
                attachBufferQueue_.size());
        }
    }
    return true;
}

void VpeVideoImpl::PrepareBuffers()
{
    if (!isRunning_.load()) {
        VPE_LOGI("Skip when died.");
        return;
    }
    CHECK_AND_RETURN_LOG(producer_ != nullptr, "producer is null!");
    uint32_t cacheSize = producerBufferCache_.size();
    VPE_LOGD("producerBufferCache_.size=%{public}u", cacheSize);
    for (uint32_t i = cacheSize; i < producer_->GetQueueSize(); i++) {
        GSError errorCode;
        SurfaceBufferInfo bufferInfo{};
        RequestBuffer(bufferInfo, errorCode, VPE_LOG_INFO_EX);
        VPE_LOGD("<%{public}u> RequestBuffer({ %{public}s })=%{public}s", i,
            VpeVideoImpl::ToString(requestCfg_).c_str(), AlgorithmUtils::ToString(errorCode).c_str());
    }
}

bool VpeVideoImpl::AttachAndRefreshProducerBuffers(const sptr<Surface>& producer)
{
    std::lock_guard<std::mutex> bufferLock(bufferLock_);
    needPrepareBuffersForNewProducer_ = true;
    CheckAndUpdateProducerCache();
    for (auto& [index, bufferInfo] : producerBufferCache_) {
        auto errorCode = producer->AttachBufferToQueue(bufferInfo.buffer);
        CHECK_AND_RETURN_RET_LOG(errorCode == GSERROR_OK, false,
            "Failed to producer(%" PRIu64 ")->AttachBufferToQueue({ %{public}s })=%{public}s", producer->GetUniqueId(),
            VpeVideoImpl::ToString(bufferInfo.buffer).c_str(), AlgorithmUtils::ToString(errorCode).c_str());
        VPE_LOGD("producer(%" PRIu64 ")->AttachBufferToQueue({ %{public}s })", producer->GetUniqueId(),
            VpeVideoImpl::ToString(bufferInfo.buffer).c_str());
    }
    std::set<uint32_t> producerBufferIDs{};
    std::queue<SurfaceBufferInfo> tempQueue;
    producerBufferQueue_.swap(tempQueue);
    AddProducerBuffers(tempQueue, producerBufferIDs);
    AddProducerBuffers(flushBufferQueue_, producerBufferIDs);
    if (producerBufferQueue_.size() + renderBufferQueue_.size() >= producerBufferCache_.size()) {
        VPE_LOGD("producerBQ:%{public}zu renderBQ:%{public}zu flushBQ:%{public}zu attachBQ:%{public}zu "
            "producerBufferCache_:%{public}zu",
            producerBufferQueue_.size(), renderBufferQueue_.size(), flushBufferQueue_.size(),
            attachBufferQueue_.size(), producerBufferCache_.size());
        return true;
    }
    VPE_LOGD("Add cache buffers to producerBQ");
    for (auto& [index, bufferInfo] : producerBufferCache_) {
        if (producerBufferIDs.count(index) > 0) {
            continue;
        }
        VPE_LOGD("Add { %{public}s } to producerBQ", VpeVideoImpl::ToString(bufferInfo.buffer).c_str());
        producerBufferQueue_.push(bufferInfo);
    }
    return true;
}

void VpeVideoImpl::AddProducerBuffers(std::queue<SurfaceBufferInfo>& bufferQueue, std::set<uint32_t>& producerBufferIDs)
{
    for (; !bufferQueue.empty(); bufferQueue.pop()) {
        auto bufferInfo = bufferQueue.front();
        if (bufferInfo.buffer == nullptr) {
            VPE_LOGW("buffer is null!");
            continue;
        }
        producerBufferIDs.insert(bufferInfo.buffer->GetSeqNum());
        producerBufferQueue_.push(bufferInfo);
    }
}

void VpeVideoImpl::AddBufferToCache(const SurfaceBufferInfo& bufferInfo)
{
    CHECK_AND_RETURN_LOG(bufferInfo.buffer != nullptr, "buffer is null!");
    auto it = producerBufferCache_.find(bufferInfo.buffer->GetSeqNum());
    if (it != producerBufferCache_.end()) {
        it->second.isFlushed = false;
        VPE_LOGD("Refresh { %{public}s } of producerBufferCache_.size:%{public}zu",
            VpeVideoImpl::ToString(bufferInfo.buffer).c_str(), producerBufferCache_.size());
        return;
    }
    uint32_t size = producerBufferCache_.size();
    producerBufferCache_[bufferInfo.buffer->GetSeqNum()] = bufferInfo;
    VPE_LOGD("Add { %{public}s } to producerBufferCache_.size:%{public}u->%{public}zu",
        VpeVideoImpl::ToString(bufferInfo.buffer).c_str(), size, producerBufferCache_.size());
    CheckAndUpdateProducerCache();
}

void VpeVideoImpl::DelBufferFromCache(const SurfaceBufferInfo& bufferInfo)
{
    CHECK_AND_RETURN_LOG(bufferInfo.buffer != nullptr, "buffer is null!");
    uint32_t size = producerBufferCache_.size();
    producerBufferCache_.erase(bufferInfo.buffer->GetSeqNum());
    VPE_LOGD("Del { %{public}s } from producerBufferCache_.size:%{public}u->%{public}zu",
        VpeVideoImpl::ToString(bufferInfo.buffer).c_str(), size, producerBufferCache_.size());
}

void VpeVideoImpl::CheckAndUpdateProducerCache()
{
    uint32_t size = producerBufferCache_.size();
    if (size <= BUFFER_QUEUE_SIZE) {
        return;
    }
    std::list<SurfaceBufferInfo> toBeDel;
    for (auto& [index, info] : producerBufferCache_) {
        if (!info.isFlushed) {
            continue;
        }
        for (auto it = toBeDel.begin(); it != toBeDel.end(); it++) {
            if (info.flushedTime < it->flushedTime) {
                toBeDel.insert(it, info);
                continue;
            }
        }
        toBeDel.push_back(info);
    }
    for (auto it = toBeDel.begin(); it != toBeDel.end(); it++) {
        size = producerBufferCache_.size();
        producerBufferCache_.erase(it->buffer->GetSeqNum());
        VPE_LOGD("Del { %{public}s } from producerBufferCache_.size:%{public}u->%{public}zu",
            VpeVideoImpl::ToString(it->buffer).c_str(), size, producerBufferCache_.size());
        if (producerBufferCache_.size() <= BUFFER_QUEUE_SIZE) {
            break;
        }
    }
}

void VpeVideoImpl::ProcessBuffers()
{
    if (!isRunning_.load()) {
        VPE_LOGI("Skip when died.");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(lock_);
        CHECK_AND_RETURN_LOG(consumer_ != nullptr && producer_ != nullptr, "consumer or producer is null!");
        UpdateProducerLocked();
    }
    while (isRunning_.load()) {
        isProcessing_ = true;

        SurfaceBufferInfo srcBufferInfo;
        SurfaceBufferInfo dstBufferInfo;
        if (!GetConsumerAndProducerBuffer(srcBufferInfo, dstBufferInfo)) {
            break;
        }
        if (srcBufferInfo.bufferFlag == VPE_BUFFER_FLAG_EOS) {
            VPE_LOGD("EOS frame.");
            OutputBuffer(srcBufferInfo, dstBufferInfo, [] {}, VPE_LOG_INFO);
            break;
        }
        if (isEnable_.load()) {
            if (!ProcessBuffer(srcBufferInfo, dstBufferInfo) && IsDisableAfterProcessFail()) {
                VPE_LOGD("Dsiable because failed to process !");
                Disable();
                continue;
            }
        } else {
            BypassBuffer(srcBufferInfo, dstBufferInfo);
        }
    }
    PrintBufferSize();

    isProcessing_ = false;
    cvDone_.notify_all();
}

bool VpeVideoImpl::GetConsumerAndProducerBuffer(SurfaceBufferInfo& srcBufferInfo, SurfaceBufferInfo& dstBufferInfo)
{
    std::lock_guard<std::mutex> consumerBufferLock(consumerBufferLock_);
    std::lock_guard<std::mutex> bufferLock(bufferLock_);
    if (producerBufferQueue_.empty() || consumerBufferQueue_.empty()) {
        return false;
    }
    srcBufferInfo = consumerBufferQueue_.front();
    if (srcBufferInfo.buffer == nullptr && srcBufferInfo.bufferFlag != VPE_BUFFER_FLAG_EOS) {
        consumerBufferQueue_.pop();
        VPE_LOGW("input buffer is null!");
        return false;
    }
    dstBufferInfo = producerBufferQueue_.front();
    if (dstBufferInfo.buffer == nullptr) {
        producerBufferQueue_.pop();
        VPE_LOGW("output buffer is null!");
        return false;
    }
    consumerBufferQueue_.pop();
    producerBufferQueue_.pop();
    return true;
}

bool VpeVideoImpl::ProcessBuffer(SurfaceBufferInfo& srcBufferInfo, SurfaceBufferInfo& dstBufferInfo)
{
    dstBufferInfo.timestamp = srcBufferInfo.timestamp;
    auto errorCode = Process(srcBufferInfo.buffer, dstBufferInfo.buffer);
    auto ret = consumer_->ReleaseBuffer(srcBufferInfo.buffer, -1);
    VPE_LOGD("consumer_->ReleaseBuffer({ %{public}s })=%{public}s",
        VpeVideoImpl::ToString(srcBufferInfo.buffer).c_str(), AlgorithmUtils::ToString(ret).c_str());
    if (errorCode != VPE_ALGO_ERR_OK) {
        OnError(errorCode);
        std::lock_guard<std::mutex> bufferLock(bufferLock_);
        producerBufferQueue_.push(dstBufferInfo);
        VPE_LOGW("Failed to process({ %{public}s },{ %{public}s })=%{public}d",
            VpeVideoImpl::ToString(srcBufferInfo.buffer).c_str(),
            VpeVideoImpl::ToString(dstBufferInfo.buffer).c_str(), errorCode);
        return false;
    }
    OutputBuffer(srcBufferInfo, dstBufferInfo, [] {}, VPE_LOG_INFO);
    NotifyEnableStatus(type_, "enable");
    return true;
}

void VpeVideoImpl::BypassBuffer(SurfaceBufferInfo& srcBufferInfo, SurfaceBufferInfo& dstBufferInfo)
{
    GSError ret1;
    GSError ret2;
    {
        if (!isRunning_.load()) {
            VPE_LOGI("Skip when died.");
            return;
        }
        std::lock_guard<std::mutex> lock(lock_);
        ret1 = producer_->DetachBufferFromQueue(dstBufferInfo.buffer);
        ret2 = producer_->AttachBufferToQueue(srcBufferInfo.buffer);
        SetRequestCfgLocked(srcBufferInfo.buffer);
        VPE_LOGD("producer_(%" PRIu64 ")->DetachBufferFromQueue({ %{public}s })=%{public}s, "
            "AttachBufferToQueue({ %{public}s })=%{public}s requestCfg:{ %{public}s } ", producer_->GetUniqueId(),
            VpeVideoImpl::ToString(dstBufferInfo.buffer).c_str(), AlgorithmUtils::ToString(ret1).c_str(),
            VpeVideoImpl::ToString(srcBufferInfo.buffer).c_str(), AlgorithmUtils::ToString(ret2).c_str(),
            VpeVideoImpl::ToString(requestCfg_).c_str());
    }
    OutputBuffer(srcBufferInfo, srcBufferInfo, [this, &srcBufferInfo, &dstBufferInfo, &ret1, &ret2] {
        attachBufferIDs_.insert(srcBufferInfo.buffer->GetSeqNum());
        attachBufferQueue_.push(srcBufferInfo);
        if (ret1 == GSERROR_OK) {
            DelBufferFromCache(dstBufferInfo);
        }
        if (ret2 == GSERROR_OK) {
            AddBufferToCache(srcBufferInfo);
        }
    }, VPE_LOG_INFO);
    VPE_LOGD("cache(%{public}s)->%{public}zu/%{public}zu", VpeVideoImpl::ToString(srcBufferInfo.buffer).c_str(),
        attachBufferIDs_.size(), attachBufferIDs_.size());
    NotifyEnableStatus(0, "disable");
}

void VpeVideoImpl::OutputBuffer(const SurfaceBufferInfo& bufferInfo, const SurfaceBufferInfo& bufferImage,
    std::function<void(void)>&& getReadyToRender, const LogInfo& logInfo)
{
    {
        std::lock_guard<std::mutex> bufferLock(bufferLock_);
        renderBufferQueue_[bufferImage.buffer->GetSeqNum()] = bufferImage;
        VPE_ORG_LOGD(logInfo, "renderBufferQueue_.push({ %{public}s }) size:%{public}zu",
            VpeVideoImpl::ToString(bufferImage.buffer).c_str(), renderBufferQueue_.size());
        getReadyToRender();
    }
    VpeBufferInfo info {
        .flag = bufferInfo.bufferFlag,
        .presentationTimestamp = bufferInfo.timestamp,
    };
    OnOutputBufferAvailable(bufferImage.buffer->GetSeqNum(), info);
}

void VpeVideoImpl::NotifyEnableStatus(uint32_t type, const std::string& status)
{
    if (!isEnableChange_.load()) {
        VPE_LOGD("Keep %{public}s.", status.c_str());
        return;
    }
    if (!CheckEnableNotifyStatus(type > 0, status)) {
        return;
    }
    OnEffectChange(type);
    if (type == 0) {
        VPE_LOGD("Call ResetAfterDisable.");
        ResetAfterDisable();
    }
}

bool VpeVideoImpl::PopBuffer(std::queue<SurfaceBufferInfo>& bufferQueue, uint32_t index, SurfaceBufferInfo& bufferInfo,
    std::function<void(sptr<SurfaceBuffer>&)>&& func, const LogInfoEx& logInfos)
{
    bool isFound = false;
    while (!bufferQueue.empty()) {
        bufferInfo = bufferQueue.front();
        bufferQueue.pop();
        func(bufferInfo.buffer);
        VPE_EX_LOGD(logInfos, "index:%{public}u buffer:{ %{public}s }", index,
            VpeVideoImpl::ToString(bufferInfo.buffer).c_str());
        if (bufferInfo.buffer != nullptr && bufferInfo.buffer->GetSeqNum() == index) {
            isFound = true;
            break;
        }
    }
    return isFound;
}

void VpeVideoImpl::PrintBufferSize() const
{
    std::lock_guard<std::mutex> consumerBufferLock(consumerBufferLock_);
    std::lock_guard<std::mutex> bufferLock(bufferLock_);
    VPE_LOGD("consumerBQ:%{public}zu producerBQ:%{public}zu renderBQ:%{public}zu flushBQ:%{public}zu "
        "attachBQ:%{public}zu", consumerBufferQueue_.size(), producerBufferQueue_.size(), renderBufferQueue_.size(),
        flushBufferQueue_.size(), attachBufferQueue_.size());
}

void VpeVideoImpl::SetRequestCfgLocked(const sptr<SurfaceBuffer>& buffer)
{
    requestCfg_.usage = buffer->GetUsage();
    requestCfg_.format = buffer->GetFormat();
    requestCfg_.width = buffer->GetWidth();
    requestCfg_.height = buffer->GetHeight();
    orgRequestCfg_ = requestCfg_;
}

bool VpeVideoImpl::WaitTrigger()
{
    uint32_t consumerSize = 0;
    uint32_t producerSize = 0;
    std::unique_lock<std::mutex> waitLock(waitLock_);
    if (!cvTrigger_.wait_for(waitLock, 200s, [this, &consumerSize, &producerSize] {
            if (!isRunning_.load()) {
                VPE_LOGI("Skip WaitTrigger when died.");
                return true;
            }
            if (needPrepareBuffers_.load() || needPrepareBuffersForNewProducer_.load()) {
                std::lock_guard<std::mutex> lock(lock_);
                std::lock_guard<std::mutex> consumerBufferLock(consumerBufferLock_);
                std::lock_guard<std::mutex> bufferLock(bufferLock_);
                PrepareBuffers();
                needPrepareBuffers_ = false;
                needPrepareBuffersForNewProducer_ = false;
            }
            std::lock_guard<std::mutex> consumerBufferLock(consumerBufferLock_);
            std::lock_guard<std::mutex> bufferLock(bufferLock_);
            consumerSize = consumerBufferQueue_.size();
            producerSize = producerBufferQueue_.size();
            VPE_LOGD("run:%{public}d stopping:%{public}d consumerBQ:%{public}u producerBQ:%{public}u "
                "renderBQ:%{public}zu flushBQ:%{public}zu attachBQ:%{public}zu",
                isRunning_.load(), state_.load() == VPEState::STOPPING, consumerSize, producerSize,
                renderBufferQueue_.size(), flushBufferQueue_.size(), attachBufferQueue_.size());
            return !isRunning_.load() || state_.load() == VPEState::STOPPING || (consumerSize > 0 && producerSize > 0);
        })) {
        VPE_LOGI("Video processing timeout.");
        return false;
    }

    if (!isRunning_.load()) {
        VPE_LOGI("die.");
        return false;
    }
    if (consumerSize == 0 || producerSize == 0) {
        CheckSpuriousWakeup();
        VPE_LOGD("Empty! consumerBQ:%{public}u producerBQ:%{public}u", consumerSize, producerSize);
        return false;
    }
    return true;
}

void VpeVideoImpl::CheckSpuriousWakeup()
{
    if (!CheckStopping()) {
        VPE_LOGD("Video processing spurious wakeup.");
    }
}

bool VpeVideoImpl::CheckStopping()
{
    if (!isRunning_.load()) {
        VPE_LOGI("Skip when died.");
        return true;
    }
    std::lock_guard<std::mutex> lock(lock_);
    return CheckStoppingLocked();
}

bool VpeVideoImpl::CheckStoppingLocked()
{
    if (state_.load() == VPEState::STOPPING) {
        state_ = VPEState::IDLE;
        OnStateLocked(VPEAlgoState::STOPPED);
        return true;
    }
    return false;
}

void VpeVideoImpl::ClearQueue(std::queue<SurfaceBufferInfo>& bufferQueue)
{
    if (bufferQueue.empty()) {
        return;
    }
    bufferQueue = std::queue<SurfaceBufferInfo>();
}

void VpeVideoImpl::ClearConsumerLocked(std::queue<SurfaceBufferInfo>& bufferQueue)
{
    CHECK_AND_RETURN_LOG(consumer_ != nullptr, "input surface is null!");
    while (!bufferQueue.empty()) {
        auto bufferInfo = bufferQueue.front();
        auto err = consumer_->ReleaseBuffer(bufferInfo.buffer, -1);
        if (err != GSERROR_OK) {
            VPE_LOGW("Failed to ReleaseBuffer({ %{public}s }):%{public}s",
                VpeVideoImpl::ToString(bufferInfo.buffer).c_str(), AlgorithmUtils::ToString(err).c_str());
        }
        bufferQueue.pop();
    }
}

void VpeVideoImpl::ClearBufferQueues()
{
    std::queue<SurfaceBufferInfo> tempQueue1;
    std::queue<SurfaceBufferInfo> tempQueue2;
    {
        std::lock_guard<std::mutex> consumerBufferLock(consumerBufferLock_);
        std::lock_guard<std::mutex> bufferLock(bufferLock_);
        isBufferQueueReady_ = false;
        consumerBufferQueue_.swap(tempQueue1);
        ClearQueue(producerBufferQueue_);
        renderBufferQueue_.clear();
        ClearQueue(flushBufferQueue_);
        producerBufferCache_.clear();
        attachBufferQueue_.swap(tempQueue2);
        attachBufferIDs_.clear();
    }
    ClearConsumerLocked(tempQueue1);
    ClearConsumerLocked(tempQueue2);
}

VPEAlgoErrCode VpeVideoImpl::ExecuteWhenIdle(std::function<VPEAlgoErrCode(void)>&& operation,
    const std::string& errorMessage, const LogInfo& logInfo)
{
    return ExecuteWithCheck([this] { return state_.load() == VPEState::IDLE; }, std::move(operation), errorMessage,
        logInfo);
}

VPEAlgoErrCode VpeVideoImpl::ExecuteWhenNotIdle(std::function<VPEAlgoErrCode(void)>&& operation,
    const std::string& errorMessage, const LogInfo& logInfo)
{
    return ExecuteWithCheck([this] { return state_.load() != VPEState::IDLE; }, std::move(operation), errorMessage,
        logInfo);
}

VPEAlgoErrCode VpeVideoImpl::ExecuteWhenRunning(std::function<VPEAlgoErrCode(void)>&& operation,
    const std::string& errorMessage, const LogInfo& logInfo)
{
    return ExecuteWithCheck([this] { return state_.load() == VPEState::RUNNING; }, std::move(operation), errorMessage,
        logInfo);
}

VPEAlgoErrCode VpeVideoImpl::ExecuteWithCheck(std::function<bool(void)>&& checker,
    std::function<VPEAlgoErrCode(void)>&& operation, const std::string& errorMessage, const LogInfo& logInfo)
{
    std::lock_guard<std::mutex> lock(lock_);
    if (checker()) {
        return operation();
    }
    VPE_ORG_LOGW(logInfo, "%{public}s", errorMessage.c_str());
    return VPE_ALGO_ERR_INVALID_OPERATION;
}

void VpeVideoImpl::ConsumerListener::OnBufferAvailable()
{
    std::shared_ptr<VpeVideoImpl> owner = owner_.lock();
    if (owner == nullptr) {
        VPE_LOGE("Video processing is null!");
        return;
    }
    owner->OnConsumerBufferAvailable();
}
