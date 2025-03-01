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
#include <sstream>

#include "vpe_log.h"
#include "vpe_trace.h"

using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;
using namespace std::chrono_literals;

namespace {
constexpr uint32_t BUFFER_QUEUE_SIZE = 5;

std::string ToString(const sptr<SurfaceBuffer>& buffer)
{
    if (buffer == nullptr) {
        return "null";
    }
    std::stringstream stream;
    stream << "id:" << buffer->GetSeqNum() << " " << buffer->GetWidth() << "x" << buffer->GetHeight() <<
        " format:" << buffer->GetFormat() << " usage:0x" << std::hex << buffer->GetUsage();
    return stream.str();
}

std::string ToString(const BufferRequestConfig& requestCfg)
{
    std::stringstream stream;
    stream << requestCfg.width << "x" << requestCfg.height << " format:" << requestCfg.format <<
        " usage:0x" << std::hex << requestCfg.usage;
    return stream.str();
}
} // namespace

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
        }, "Registration of callbacks during running is not allowed!");
}

VPEAlgoErrCode VpeVideoImpl::SetOutputSurface(const sptr<Surface>& surface)
{
    CHECK_AND_RETURN_RET_LOG(surface != nullptr, VPE_ALGO_ERR_INVALID_VAL, "Invalid input: surface is null!");
    CHECK_AND_RETURN_RET_LOG(!surface->IsConsumer(), VPE_ALGO_ERR_INVALID_VAL,
        "Invalid input: surface is NOT producer!");
    CHECK_AND_RETURN_RET_LOG(IsProducerSurfaceValid(surface), VPE_ALGO_ERR_INVALID_VAL,
        "Invalid input: surface is invalid!");

    std::lock_guard<std::mutex> lock(lock_);
    if (producer_ == surface) {
        VPE_LOGD("Oops! The same surface!");
        return VPE_ALGO_ERR_OK;
    }
    if (producer_ != nullptr) {
        producer_->UnRegisterReleaseListener();
        producer_->CleanCache(true);
    }
    surface->UnRegisterReleaseListener();
    surface->CleanCache(true);
    GSError err = surface->RegisterReleaseListener([this](sptr<SurfaceBuffer>&) { return OnProducerBufferReleased(); });
    CHECK_AND_RETURN_RET_LOG(err == GSERROR_OK, VPE_ALGO_ERR_UNKNOWN, "RegisterReleaseListener failed!");
    VPE_LOGI("Set output buffer queue size to %{public}u", BUFFER_QUEUE_SIZE);
    surface->SetQueueSize(BUFFER_QUEUE_SIZE);
    surface->Connect();
    AttachBuffers(surface);
    if (state_.load() != VPEState::IDLE) {
        cvTrigger_.notify_one();
    }
    producer_ = surface;
    VPEAlgoErrCode ret = VPE_ALGO_ERR_OK;
    if (isEnable_.load()) {
        ret = UpdateRequestCfg(surface, requestCfg_);
    }
    VPE_LOGD("requestCfg_({ %{public}s })", ToString(requestCfg_).c_str());
    return ret;
}

sptr<Surface> VpeVideoImpl::GetInputSurface()
{
    sptr<Surface> producerSurface;
    if (ExecuteWhenIdle([this, &producerSurface]() {
        producerSurface = CreateConsumerSurfaceLocked();
        return VPE_ALGO_ERR_OK;
    }, "Getting a surface during running is not allowed!") != VPE_ALGO_ERR_OK) {
        return nullptr;
    }
    return producerSurface;
}

VPEAlgoErrCode VpeVideoImpl::Start()
{
    return ExecuteWhenIdle(
        [this]() {
            if (consumer_ == nullptr || producer_ == nullptr || cb_ == nullptr) {
                VPE_LOGE("The input surface, output surface or callback is NOT ready!");
                return VPE_ALGO_ERR_INVALID_OPERATION;
            }
            state_ = VPEState::RUNNING;
            OnStateLocked(VPEAlgoState::RUNNING);
            return VPE_ALGO_ERR_OK;
        }, "Already start!");
}

VPEAlgoErrCode VpeVideoImpl::Stop()
{
    auto err = ExecuteWhenRunning([this]() {
        state_ = VPEState::STOPPING;
        cvTrigger_.notify_one();
        return VPE_ALGO_ERR_OK;
    }, "Already stop!");
    return err;
}

VPEAlgoErrCode VpeVideoImpl::Release()
{
    return Deinitialize();
}

VPEAlgoErrCode VpeVideoImpl::Flush()
{
    std::unique_lock<std::mutex> lock(lock_);
    cvDone_.wait(lock, [this]() { return !isProcessing_.load(); });
    std::queue<SurfaceBufferInfo> tempQueue1;
    std::queue<SurfaceBufferInfo> tempQueue2;
    {
        std::lock_guard<std::mutex> bufferLock(bufferLock_);
        consumerBufferQueue_.swap(tempQueue1);
        while (!renderBufferQueue_.empty()) {
            producerBufferQueue_.push(renderBufferQueue_.front());
            renderBufferQueue_.pop();
        }
        attachBufferQueue_.swap(tempQueue2);
        attachBufferIDs_.clear();
    }
    ClearConsumerLocked(tempQueue1);
    ClearConsumerLocked(tempQueue2);
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
    VPE_LOGD("requestCfg_({ %{public}s })", ToString(requestCfg_).c_str());
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
    isEnable_ = false;
    isEnableChange_ = true;
    VPE_LOGD("requestCfg_({ %{public}s })", ToString(requestCfg_).c_str());
    return VPE_ALGO_ERR_OK;
}

VPEAlgoErrCode VpeVideoImpl::NotifyEos()
{
    return ExecuteWhenNotIdle(
        [this]() {
            {
                std::lock_guard<std::mutex> buffferlock(bufferLock_);
                SurfaceBufferInfo bufferInfo{};
                bufferInfo.bufferFlag = VPE_BUFFER_FLAG_EOS;
                consumerBufferQueue_.push(bufferInfo);
            }
            cvTrigger_.notify_one();
            return VPE_ALGO_ERR_OK;
        }, "Notify EOS must be called during running!");
}

VPEAlgoErrCode VpeVideoImpl::ReleaseOutputBuffer(uint32_t index, bool render)
{
    return ExecuteWhenNotIdle(
        [this, index, render]() { return RenderOutputBufferLocked(index, -1, render); },
        "Release output buffer must be called during running!");
}

VPEAlgoErrCode VpeVideoImpl::RenderOutputBufferAtTime(uint32_t index, int64_t renderTimestamp)
{
    return ExecuteWhenNotIdle(
        [this, index, renderTimestamp]() { return RenderOutputBufferLocked(index, renderTimestamp, true); },
        "Render output buffer must be called during running!");
}

bool VpeVideoImpl::IsInitialized() const
{
    return isInitialized_.load();
}

VPEAlgoErrCode VpeVideoImpl::Initialize()
{
    std::lock_guard<std::mutex> lock(lock_);
    if (isInitialized_.load()) {
        VPE_LOGD("Already initialize!");
        return VPE_ALGO_ERR_OK;
    }
    VPE_LOGD("Start to initializing...");
    isRunning_ = true;
    worker_ = std::thread([this]() {
        while (isRunning_.load()) {
            {
                std::unique_lock<std::mutex> bufferLock(bufferLock_);
                if (!cvTrigger_.wait_for(bufferLock, 200s, [this] {
                        VPE_LOGD("run:%d stopping:%d consumerBQ:%{public}zu producerBQ:%{public}zu "
                            "renderBQ:%{public}zu flushBQ:%{public}zu attachBQ:%{public}zu",
                            isRunning_.load(), state_.load() == VPEState::STOPPING,
                            consumerBufferQueue_.size(), producerBufferQueue_.size(),
                            renderBufferQueue_.size(), flushBufferQueue_.size(), attachBufferQueue_.size());
                        return !isRunning_.load() || state_.load() == VPEState::STOPPING ||
                            (!producerBufferQueue_.empty() && !consumerBufferQueue_.empty());
                    })) {
                    VPE_LOGI("Video processing timeout.");
                    continue;
                }

                if (!isRunning_.load()) {
                    VPE_LOGI("Video processing clear.");
                    break;
                }
                if (producerBufferQueue_.empty() || consumerBufferQueue_.empty()) {
                    CheckSpuriousWakeup();
                    VPE_LOGD("Empty! consumerBQ:%{public}zu producerBQ:%{public}zu", consumerBufferQueue_.size(),
                        producerBufferQueue_.size());
                    continue;
                }
            }
            VPE_LOGD("ProcessBuffers");
            ProcessBuffers();
            CheckStopping();
        };
    });
    auto errorCode = OnInitialize();
    isInitialized_ = true;
    VPE_LOGD("OnInitialize() return %{public}d. this:%{public}p", errorCode, this);
    return errorCode;
}

VPEAlgoErrCode VpeVideoImpl::Deinitialize()
{
    std::unique_lock<std::mutex> lock(lock_);
    VPE_LOGD("Start to deinitializing... this:%{public}p", this);
    cvDone_.wait(lock, [this]() { return !isProcessing_.load(); });
    if (!isInitialized_.load()) {
        VPE_LOGD("Already deinitialize!");
        return VPE_ALGO_ERR_OK;
    }
    isInitialized_ = false;
    VPEAlgoErrCode errorCode = OnDeinitialize();
    if (state_.load() == VPEState::RUNNING) {
        state_ = VPEState::STOPPING;
    }
    isRunning_ = false;
    cvTrigger_.notify_one();
    if (worker_.joinable()) {
        worker_.join();
    }
    CheckStoppingLocked();
    cb_ = nullptr;
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
    VPE_LOGD("OnDeinitialize() return %{public}d. this:%{public}p", errorCode, this);
    return errorCode;
}

void VpeVideoImpl::RefreshBuffers()
{
    std::lock_guard<std::mutex> lock(lock_);
    if (state_.load() != VPEState::RUNNING) {
        VPE_LOGD("Skip refreshing during Non-Running.");
        return;
    }
    if (!isEnable_.load()) {
        VPE_LOGD("Skip refreshing when it is disabled.");
        return;
    }

    if (!isBufferQueueReady_) {
        VPE_LOGD("Skip refreshing when buffer queue is not ready.");
        return;
    }

    VPE_LOGD("Clear all buffer queues.");
    ClearBufferQueues();
}

void VpeVideoImpl::OnOutputFormatChanged(const Format& format)
{
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

bool VpeVideoImpl::IsProducerSurfaceValid([[maybe_unused]] const sptr<Surface>& surface)
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

void VpeVideoImpl::OnErrorLocked(VPEAlgoErrCode errorCode)
{
    if (cb_ != nullptr) {
        VPE_LOGD("OnError(%{public}d)", errorCode);
        cb_->OnError(errorCode);
    }
}

void VpeVideoImpl::OnStateLocked(VPEAlgoState state)
{
    if (cb_ != nullptr) {
        VPE_LOGD("OnState(%{public}d)", state);
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
        cb_->OnOutputBufferAvailable(index, info);
    }
}

GSError VpeVideoImpl::OnConsumerBufferAvailable()
{
    std::lock_guard<std::mutex> lock(lock_);
    if (state_.load() != VPEState::RUNNING) {
        VPE_LOGD("NOT running now!");
        return GSERROR_INVALID_OPERATING;
    }

    SurfaceBufferInfo bufferInfo{};
    int releaseFence = -1;
    OHOS::Rect damage;
    GSError err = consumer_->AcquireBuffer(bufferInfo.buffer, releaseFence, bufferInfo.timestamp, damage);
    if (err != GSERROR_OK || bufferInfo.buffer == nullptr) {
        VPE_LOGE("Failed to acquire buffer!");
        return err;
    }
    VPE_LOGD("consumer_->AcquireBuffer({ %{public}s })", ToString(bufferInfo.buffer).c_str());

    {
        std::lock_guard<std::mutex> buffferlock(bufferLock_);
        if (!isBufferQueueReady_) {
            isBufferQueueReady_ = true;
            requestCfg_.usage = bufferInfo.buffer->GetUsage();

            requestCfg_.timeout = 0;
            requestCfg_.strideAlignment = 32; // 32 bits align
            UpdateRequestCfg(bufferInfo.buffer, requestCfg_);
            VPE_LOGD("Use requestCfg_({ %{public}s }) to prepare buffers.", ToString(requestCfg_).c_str());
            PrepareBuffers();
        }
        consumerBufferQueue_.push(bufferInfo);
    }
    cvTrigger_.notify_one();
    return GSERROR_OK;
}

GSError VpeVideoImpl::OnProducerBufferReleased()
{
    VPE_LOGD("step in");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(consumer_ != nullptr || producer_ != nullptr, GSERROR_OK,
        "Input or output surface is null!");

    {
        std::lock_guard<std::mutex> bufferLock(bufferLock_);
        if (flushBufferQueue_.empty()) {
            VPE_LOGD("NO flush buffer!");
            return GSERROR_OK;
        }
        GSError err = GSERROR_OK;
        SurfaceBufferInfo bufferInfo{};
        CHECK_AND_RETURN_RET_LOG(RequestBuffer(bufferInfo, err), err, "Failed to request buffer!");
        PopBuffer(flushBufferQueue_, bufferInfo.buffer->GetSeqNum(), bufferInfo,
            [this](sptr<SurfaceBuffer>& buffer) {
                CHECK_AND_RETURN_LOG(buffer != nullptr, "OnProducerBufferReleased: Flush buffer is null!");
                VPE_LOGD("OnProducerBufferReleased: Pop flush buffer { %{public}s } flushBQ=%{public}zu",
                    ToString(buffer).c_str(), flushBufferQueue_.size());
            });
    }
    if (state_.load() != VPEState::IDLE) {
        cvTrigger_.notify_one();
    }
    VPE_LOGD("step out");
    return GSERROR_OK;
}

VPEAlgoErrCode VpeVideoImpl::RenderOutputBufferLocked(uint32_t index, int64_t renderTimestamp, bool render)
{
    std::unique_lock<std::mutex> bufferLock(bufferLock_);
    SurfaceBufferInfo bufferInfo{};
    bool isFound = PopBuffer(renderBufferQueue_, index, bufferInfo, [](sptr<SurfaceBuffer>&) {});
    bufferLock.unlock();

    CHECK_AND_RETURN_RET_LOG(isFound, VPE_ALGO_ERR_INVALID_PARAM, "Invalid input: index=%{public}u!", index);
    if (render) {
        BufferFlushConfig flushcfg{};
        flushcfg.damage.w = bufferInfo.buffer->GetWidth();
        flushcfg.damage.h = bufferInfo.buffer->GetHeight();
        flushcfg.timestamp = (renderTimestamp == -1) ? bufferInfo.timestamp : renderTimestamp;
        auto ret = producer_->FlushBuffer(bufferInfo.buffer, -1, flushcfg);
        VPE_LOGD("producer_->FlushBuffer({ %{public}s })=%{public}d flushBQ=%{public}zu",
            ToString(bufferInfo.buffer).c_str(), ret, flushBufferQueue_.size() + 1);
        bufferLock.lock();
        flushBufferQueue_.push(bufferInfo);
    } else {
        VPE_LOGD("reback { %{public}s } producerBQ=%{public}zu",
            ToString(bufferInfo.buffer).c_str(), producerBufferQueue_.size() + 1);
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
        BUFFER_USAGE_MEM_DMA | BUFFER_USAGE_MEM_MMZ_CACHE | BUFFER_USAGE_HW_COMPOSER);
    consumer_->SetQueueSize(BUFFER_QUEUE_SIZE);
    VPE_LOGI("Set input buffer queue size to %{public}u", BUFFER_QUEUE_SIZE);

    return producerSurface;
}

bool VpeVideoImpl::RequestBuffer(SurfaceBufferInfo& bufferInfo, GSError& errorCode)
{
    int releaseFence = -1;
    errorCode = producer_->RequestBuffer(bufferInfo.buffer, releaseFence, requestCfg_);
    if (errorCode != GSERROR_OK || bufferInfo.buffer == nullptr) {
        VPE_LOGW("Failed to producer_->RequestBuffer(requestCfg={ %{public}s })", ToString(requestCfg_).c_str());
        return false;
    }
    producerBufferQueue_.push(bufferInfo);
    if (!isEnable_.load()) {
        VPE_LOGD("producer_->RequestBuffer({ %{public}s }) and try to release.", ToString(bufferInfo.buffer).c_str());
        auto it = attachBufferIDs_.find(bufferInfo.buffer->GetSeqNum());
        if (it != attachBufferIDs_.end()) {
            PopBuffer(attachBufferQueue_, bufferInfo.buffer->GetSeqNum(), bufferInfo,
                [this](sptr<SurfaceBuffer>& buffer) {
                    CHECK_AND_RETURN_LOG(buffer != nullptr, "Attach buffer is null!");
                    attachBufferIDs_.erase(buffer->GetSeqNum());
                    auto ret = consumer_->ReleaseBuffer(buffer, -1);
                    VPE_LOGD("consumer_->ReleaseBuffer({ %{public}s })=%{public}d cache=%{public}zu",
                        ToString(buffer).c_str(), ret, attachBufferQueue_.size());
                });
        }
    } else {
        VPE_LOGD("producer_->RequestBuffer({ %{public}s })", ToString(bufferInfo.buffer).c_str());
        if (attachBufferQueue_.empty()) {
            return true;
        }
        bufferInfo = attachBufferQueue_.front();
        attachBufferQueue_.pop();
        if (bufferInfo.buffer != nullptr) {
            attachBufferIDs_.erase(bufferInfo.buffer->GetSeqNum());
            auto ret = consumer_->ReleaseBuffer(bufferInfo.buffer, -1);
            VPE_LOGD("consumer_->ReleaseBuffer({ %{public}s })=%{public}d cache->%{public}zu",
                ToString(bufferInfo.buffer).c_str(), ret, attachBufferQueue_.size());
        }
    }
    return true;
}

void VpeVideoImpl::PrepareBuffers()
{
    for (uint32_t i = 0; i < producer_->GetQueueSize(); i++) {
        GSError errorCode;
        SurfaceBufferInfo bufferInfo{};
        RequestBuffer(bufferInfo, errorCode);
        VPE_LOGD("<%{public}u> RequestBuffer({ %{public}s })=%{public}d", i, ToString(requestCfg_).c_str(), errorCode);
    }
}

void VpeVideoImpl::AttachBuffers(const sptr<Surface>& producer)
{
    std::lock_guard<std::mutex> bufferLock(bufferLock_);
    std::queue<SurfaceBufferInfo> tempQueue;
    producerBufferQueue_.swap(tempQueue);
    AttachBuffers(producer, tempQueue);
    AttachBuffers(producer, flushBufferQueue_);
}

void VpeVideoImpl::AttachBuffers(const sptr<Surface>& producer, std::queue<SurfaceBufferInfo>& bufferQueue)
{
    for (; !bufferQueue.empty(); bufferQueue.pop()) {
        auto bufferInfo = bufferQueue.front();
        if (bufferInfo.buffer == nullptr) {
            VPE_LOGW("buffer is null!");
            continue;
        }
        auto errorCode = producer->AttachBufferToQueue(bufferInfo.buffer);
        if (errorCode != GSERROR_OK) {
            VPE_LOGW("Failed to producer->AttachBufferToQueue({ %{public}s })=%{public}d",
                ToString(bufferInfo.buffer).c_str(), errorCode);
            continue;
        }
        producerBufferQueue_.push(bufferInfo);
    }
}

void VpeVideoImpl::ProcessBuffers()
{
    sptr<Surface> consumer;
    {
        std::lock_guard<std::mutex> lock(lock_);
        if (consumer_ == nullptr) {
            return;
        }
        consumer = consumer_;
        isProcessing_ = true;
    }
    while (isRunning_.load()) {
        SurfaceBufferInfo srcBufferInfo;
        SurfaceBufferInfo dstBufferInfo;
        {
            std::lock_guard<std::mutex> bufferLock(bufferLock_);
            if (producerBufferQueue_.empty() || consumerBufferQueue_.empty()) {
                break;
            }
            srcBufferInfo = consumerBufferQueue_.front();
            dstBufferInfo = producerBufferQueue_.front();
            if (srcBufferInfo.buffer == nullptr) {
                consumerBufferQueue_.pop();
                continue;
            }
            if (dstBufferInfo.buffer == nullptr) {
                producerBufferQueue_.pop();
                continue;
            }
            consumerBufferQueue_.pop();
            producerBufferQueue_.pop();
        }
        if (isEnable_.load()) {
            if (!ProcessBuffer(consumer, srcBufferInfo, dstBufferInfo)) {
                continue;
            }
        } else {
            BypassBuffer(srcBufferInfo, dstBufferInfo);
        }
    }
    VPE_LOGD("consumerBQ:%{public}zu producerBQ:%{public}zu renderBQ:%{public}zu flushBQ:%{public}zu "
        "attachBQ:%{public}zu", consumerBufferQueue_.size(), producerBufferQueue_.size(), renderBufferQueue_.size(),
        flushBufferQueue_.size(), attachBufferQueue_.size());
    {
        std::lock_guard<std::mutex> lock(lock_);
        isProcessing_ = false;
    }
    cvDone_.notify_all();
}

bool VpeVideoImpl::ProcessBuffer(sptr<Surface>& consumer, SurfaceBufferInfo& srcBufferInfo,
    SurfaceBufferInfo& dstBufferInfo)
{
    if (srcBufferInfo.bufferFlag != VPE_BUFFER_FLAG_EOS) {
        dstBufferInfo.timestamp = srcBufferInfo.timestamp;
        auto errorCode = Process(srcBufferInfo.buffer, dstBufferInfo.buffer);
        auto ret = consumer->ReleaseBuffer(srcBufferInfo.buffer, -1);
        VPE_LOGD("consumer_->ReleaseBuffer({ %{public}s })=%{public}d",
            ToString(srcBufferInfo.buffer).c_str(), ret);
        if (errorCode != VPE_ALGO_ERR_OK) {
            OnErrorLocked(errorCode);
            std::lock_guard<std::mutex> bufferLock(bufferLock_);
            producerBufferQueue_.push(dstBufferInfo);
            VPE_LOGW("Failed to process({ %{public}s },{ %{public}s })=%{public}d",
                ToString(srcBufferInfo.buffer).c_str(), ToString(dstBufferInfo.buffer).c_str(), errorCode);
            return false;
        }
    }
    OutputBuffer(srcBufferInfo, dstBufferInfo, true, [] {});
    return true;
}

void VpeVideoImpl::BypassBuffer(SurfaceBufferInfo& srcBufferInfo, SurfaceBufferInfo& dstBufferInfo)
{
    {
        std::lock_guard<std::mutex> lock(lock_);
        auto ret1 = producer_->DetachBufferFromQueue(dstBufferInfo.buffer);
        auto ret2 = producer_->AttachBufferToQueue(srcBufferInfo.buffer);
        SetRequestCfgLocked(srcBufferInfo.buffer);
        VPE_LOGD("producer_->DetachBufferFromQueue({ %{public}s })=%{public}d, "
            "AttachBufferToQueue({ %{public}s })=%{public}d requestCfg:{ %{public}s }",
            ToString(dstBufferInfo.buffer).c_str(), ret1,
            ToString(srcBufferInfo.buffer).c_str(), ret2, ToString(requestCfg_).c_str());
    }
    OutputBuffer(srcBufferInfo, srcBufferInfo, false, [this, &srcBufferInfo] {
        attachBufferIDs_.insert(srcBufferInfo.buffer->GetSeqNum());
        attachBufferQueue_.push(srcBufferInfo);
    });
    VPE_LOGD("cache(%{public}s)->%{public}zu/%{public}zu", ToString(srcBufferInfo.buffer).c_str(),
        attachBufferIDs_.size(), attachBufferIDs_.size());
}

void VpeVideoImpl::OutputBuffer(const SurfaceBufferInfo& bufferInfo, const SurfaceBufferInfo& bufferImage,
    bool isProcessed, std::function<void(void)>&& getReadyToRender)
{
    {
        std::lock_guard<std::mutex> bufferLock(bufferLock_);
        renderBufferQueue_.push(bufferImage);
        getReadyToRender();
    }
    VpeBufferInfo info {
        .flag = bufferInfo.bufferFlag,
        .presentationTimestamp = bufferInfo.timestamp,
    };
    OnOutputBufferAvailable(bufferImage.buffer->GetSeqNum(), info);
    if (!isEnableChange_.load()) {
        VPE_LOGD("no enable change");
        return;
    }
    std::lock_guard<std::mutex> lock(lock_);
    if (isEnable_.load() && !isProcessed) {
        VPE_LOGD("No frame is processed after enabling.");
        return;
    }
    OnEffectChange(isEnable_.load() ? type_ : 0);
    isEnableChange_ = false;
}

bool VpeVideoImpl::PopBuffer(std::queue<SurfaceBufferInfo>& bufferQueue, uint32_t index, SurfaceBufferInfo& bufferInfo,
    std::function<void(sptr<SurfaceBuffer>&)>&& func)
{
    bool isFound = false;
    while (!bufferQueue.empty()) {
        bufferInfo = bufferQueue.front();
        bufferQueue.pop();
        func(bufferInfo.buffer);
        if (bufferInfo.buffer != nullptr && bufferInfo.buffer->GetSeqNum() == index) {
            isFound = true;
            break;
        }
    }
    return isFound;
}

void VpeVideoImpl::SetRequestCfgLocked(const sptr<SurfaceBuffer>& buffer)
{
    requestCfg_.usage = buffer->GetUsage();
    requestCfg_.format = buffer->GetFormat();
    requestCfg_.width = buffer->GetWidth();
    requestCfg_.height = buffer->GetHeight();
    orgRequestCfg_ = requestCfg_;
}

void VpeVideoImpl::CheckSpuriousWakeup()
{
    if (!CheckStopping()) {
        VPE_LOGD("Video processing spurious wakeup.");
    }
}

bool VpeVideoImpl::CheckStopping()
{
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
    while (!bufferQueue.empty()) {
        auto bufferInfo = bufferQueue.front();
        consumer_->ReleaseBuffer(bufferInfo.buffer, -1);
        bufferQueue.pop();
    }
}

void VpeVideoImpl::ClearBufferQueues()
{
    std::queue<SurfaceBufferInfo> tempQueue1;
    std::queue<SurfaceBufferInfo> tempQueue2;
    {
        std::lock_guard<std::mutex> bufferLock(bufferLock_);
        isBufferQueueReady_ = false;
        consumerBufferQueue_.swap(tempQueue1);
        ClearQueue(producerBufferQueue_);
        ClearQueue(renderBufferQueue_);
        attachBufferQueue_.swap(tempQueue2);
        attachBufferIDs_.clear();
    }
    ClearConsumerLocked(tempQueue1);
    ClearConsumerLocked(tempQueue2);
}

VPEAlgoErrCode VpeVideoImpl::ExecuteWhenIdle(std::function<VPEAlgoErrCode(void)>&& operation,
    const std::string& errorMessage)
{
    return ExecuteWithCheck([this] { return state_.load() == VPEState::IDLE; }, std::move(operation), errorMessage);
}

VPEAlgoErrCode VpeVideoImpl::ExecuteWhenNotIdle(std::function<VPEAlgoErrCode(void)>&& operation,
    const std::string& errorMessage)
{
    return ExecuteWithCheck([this] { return state_.load() != VPEState::IDLE; }, std::move(operation), errorMessage);
}

VPEAlgoErrCode VpeVideoImpl::ExecuteWhenRunning(std::function<VPEAlgoErrCode(void)>&& operation,
    const std::string& errorMessage)
{
    return ExecuteWithCheck([this] { return state_.load() == VPEState::RUNNING; }, std::move(operation), errorMessage);
}

VPEAlgoErrCode VpeVideoImpl::ExecuteWithCheck(std::function<bool(void)>&& checker,
    std::function<VPEAlgoErrCode(void)>&& operation, const std::string& errorMessage)
{
    std::lock_guard<std::mutex> lock(lock_);
    if (checker()) {
        return operation();
    }
    VPE_LOGW("%{public}s", errorMessage.c_str());
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
