/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "event_handler_factory.h"
#include "vpe_log.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
const int HANDLER_NAME_MAX_LANEGTH = 4096;

EventHandlerFactory& EventHandlerFactory::GetInstance()
{
    static EventHandlerFactory instance;
    return instance;
}

int32_t EventHandlerFactory::Init()
{
    VPE_LOGI("EventHandlerFactory Init!");
    auto unloadRunner = AppExecFwk::EventRunner::Create("unload_vpe_sa_handler");
    std::lock_guard<std::mutex> lock(eventHandlerMutex_);
    eventHandlerMap_["unload_vpe_sa_handler"] = std::make_shared<AppExecFwk::EventHandler>(unloadRunner);
    return ERR_OK;
}

int32_t EventHandlerFactory::UnInit()
{
    VPE_LOGI("EventHandlerFactory UnInit");
    std::lock_guard<std::mutex> lock(eventHandlerMutex_);
    eventHandlerMap_.clear();
    return ERR_OK;
}

std::shared_ptr<AppExecFwk::EventHandler> EventHandlerFactory::CreateEventHandler(const std::string& handlerName)
{
    if (handlerName.empty() || handlerName.size() > HANDLER_NAME_MAX_LANEGTH) {
        VPE_LOGE("handlerName is invalid!");
        return nullptr;
    }
    VPE_LOGI("createEventHandler, handlerName: %{public}s", handlerName.c_str());
    std::lock_guard<std::mutex> lock(eventHandlerMutex_);
    return eventHandlerMap_[handlerName];
}
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS