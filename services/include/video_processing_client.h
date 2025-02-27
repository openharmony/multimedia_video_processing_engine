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

#ifndef VPE_VIDEO_PROCESSING_CLENT_H
#define VPE_VIDEO_PROCESSING_CLENT_H

#include "surface_buffer_info.h"
#include "video_processing_service_manager_proxy.h"
#include "ipc_types.h"
#include "refbase.h"
#include "iremote_object.h"
#include "vpe_model_path.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class VideoProcessingManager {
public:
    VideoProcessingManager(const VideoProcessingManager&) = delete;
    VideoProcessingManager& operator=(const VideoProcessingManager&) = delete;
    VideoProcessingManager(VideoProcessingManager&&) = delete;
    VideoProcessingManager& operator=(VideoProcessingManager&&) = delete;

    static VideoProcessingManager& GetInstance();

    /*
     * @brief Initialize the client environment.
     *
     */
    void Connect();

    /*
     * @brief Clear the client environment.
     *
     */
    void Disconnect();

    /*
     * @brief Read file from system to pass surface buffer to VPE module.
     * @param key
     */
    ErrCode LoadInfo(int32_t key, SurfaceBufferInfo& bufferInfo);

    void LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject);
    void LoadSystemAbilityFail();

private:
    VideoProcessingManager() = default;
    virtual ~VideoProcessingManager() = default;
    sptr<IVideoProcessingServiceManager> g_proxy = nullptr;
    std::condition_variable g_proxyConVar;
};
}
}
}
#endif // VPE_VIDEO_PROCESSING_CLENT_H