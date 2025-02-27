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

#ifndef VPE_VIDEO_PROCESSING_SERVER_H
#define VPE_VIDEO_PROCESSING_SERVER_H

#include <string>

#include <refbase.h>
#include <system_ability.h>

#include "video_processing_service_manager_stub.h"
#include "ipc_types.h"
#include "event_handler.h"
#include "event_runner.h"
#include "vpe_log.h"
#include "vpe_model_path.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class VideoProcessingServer : public SystemAbility, public VideoProcessingServiceManagerStub {
    DECLARE_SYSTEM_ABILITY(VideoProcessingServer);

public:
    VideoProcessingServer(int32_t saId, bool runOnCreate);
    ~VideoProcessingServer();

    int32_t Init();
    int32_t UnInit();
    ErrCode LoadInfo(int32_t key, SurfaceBufferInfo& bufferInfo) override;
    bool IsInited();

protected:
    void OnStart() override;
    void OnStop() override;

private:
    int32_t CreateUnloadHandler();
    int32_t DestroyUnloadHandler();
    void DelayUnloadTask();
    void UnloadVideoProcessingSA();

private:
    std::shared_ptr<AppExecFwk::EventHandler> unloadHandler_;
    std::mutex unloadMutex_;
    std::atomic<bool> isInited_{false};
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
#endif // VPE_VIDEO_PROCESSING_SERVER_H