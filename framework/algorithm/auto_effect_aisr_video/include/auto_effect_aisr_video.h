/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
 
#ifndef AUTO_EFFECT_AISR_VIDEO_H
#define AUTO_EFFECT_AISR_VIDEO_H
 
#include <atomic>
#include <string>
 
#include "algorithm_video_impl.h"
#include "detail_enhancer_base.h"
#include "video_processing_callback_listener_base.h"
#include "video_processing_server_listener_base.h"
 
namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class AutoEffectAisrVideo : public VpeVideoImpl {
public:
    static std::shared_ptr<VpeVideoImpl> Create();
    static bool IsSupported(const OHOS::Media::Format& parameter);
    static bool IsProductSupported();
    static bool IsSurfaceSupported(const sptr<Surface>& surface);
    VPEAlgoErrCode GernerateAisrMetadata(sptr<SurfaceBuffer>& srcBuffer,
        sptr<SurfaceBuffer>& dstBuffer, sptr<SyncFence> infenceFd) override;
 
    // Static methods for CAPI delegation
    static VPEAlgoErrCode UseAutoEffect(uint32_t type, bool enable, const char* name);
    static VPEAlgoErrCode SetAutoEffectEnabled(bool enable, bool hasEnable,
        const char* name, float strength, bool hasStrength);
 
    explicit AutoEffectAisrVideo(uint32_t type, bool disable)
        : VpeVideoImpl(type, false, VIDEO_DEFAULT_USAGE, disable), isAutoDisable_(disable) {}
    ~AutoEffectAisrVideo() override;
    AutoEffectAisrVideo(const AutoEffectAisrVideo&) = delete;
    AutoEffectAisrVideo& operator=(const AutoEffectAisrVideo&) = delete;
    AutoEffectAisrVideo(AutoEffectAisrVideo&&) = delete;
    AutoEffectAisrVideo& operator=(AutoEffectAisrVideo&&) = delete;
 
    VPEAlgoErrCode SetParameter(const Format& parameter) final;
 
protected:
    VPEAlgoErrCode OnInitialize() override;
    VPEAlgoErrCode OnDeinitialize() override;
    VPEAlgoErrCode Process(const sptr<SurfaceBuffer>& sourceImage,
        sptr<SurfaceBuffer>& destinationImage, sptr<SyncFence> infenceFd = nullptr) override;
    VPEAlgoErrCode UpdateRequestCfg(const sptr<Surface>& surface, BufferRequestConfig& requestCfg) override;
    void UpdateRequestCfg(const sptr<SurfaceBuffer>& consumerBuffer, BufferRequestConfig& requestCfg) override;
 
private:
    class ServerListener : public VideoProcessingServerListenerBase {
    public:
        explicit ServerListener(AutoEffectAisrVideo& owner) : owner_(owner) {}
        ~ServerListener() override = default;
        ServerListener(const ServerListener&) = delete;
        ServerListener& operator=(const ServerListener&) = delete;
        ServerListener(ServerListener&&) = delete;
        ServerListener& operator=(ServerListener&&) = delete;
 
        void OnServerDied() final;
    private:
        AutoEffectAisrVideo& owner_;
    };
 
    class AlgorithmCallback : public VideoProcessingCallbackListenerBase {
    public:
        explicit AlgorithmCallback(AutoEffectAisrVideo& owner) : owner_(owner) {}
        ~AlgorithmCallback() override = default;
        AlgorithmCallback(const AlgorithmCallback&) = delete;
        AlgorithmCallback& operator=(const AlgorithmCallback&) = delete;
        AlgorithmCallback(AlgorithmCallback&&) = delete;
        AlgorithmCallback& operator=(AlgorithmCallback&&) = delete;
 
        void OnPolicyControl(bool isAlgorithmEnable) final;
    private:
        AutoEffectAisrVideo& owner_;
    };
 
    VPEAlgoErrCode SetAutoEffectEnabledInner(bool enable, bool hasEnable, float strength, bool hasStrength);
    bool RegisterAlgorithmCallback();
    bool RegisterAlgorithmCallbackLocked();
    bool UnRegisterAlgorithmCallback();
    bool UnRegisterAlgorithmCallbackLocked();
    bool RegisterServerListener();
    void UnregisterServerListener();
    void RegisterInVpeMap(const std::string& name);
 
    enum ParamError {
        PARAM_ERR_INVALID = -1,
        PARAM_ERR_NOT_FOUND = 0,
        PARAM_ERR_OK = 1,
    };
 
    ParamError SetAutoEffectStrength(const Format& parameter);
    ParamError SetAutoEffectName(const Format& parameter);
    ParamError SetNodeId(const Format& parameter);
 
    std::mutex lock_{};
    std::shared_ptr<AlgorithmCallback> listener_{};
    std::shared_ptr<ServerListener> serverListener_{};
    std::shared_ptr<DetailEnhancerBase> algo_{};
    std::string effectName_;
    std::atomic<float> strength_{-1.0f};
    bool isAutoDisable_{};
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
 
#endif // AUTO_EFFECT_AISR_VIDEO_H