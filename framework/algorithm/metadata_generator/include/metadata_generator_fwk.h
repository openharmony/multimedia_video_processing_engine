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

#ifndef FRAMEWORK_ALGORITHM_METADATA_GENERATOR_METADATA_GENERATOR_FWK_H
#define FRAMEWORK_ALGORITHM_METADATA_GENERATOR_METADATA_GENERATOR_FWK_H

#include <atomic>
#include <memory>
#include <optional>
#include "metadata_generator.h"
#include "metadata_generator_base.h"
#include "extension_base.h"

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class MetadataGeneratorFwk : public MetadataGenerator {
public:
    MetadataGeneratorFwk();
    MetadataGeneratorFwk(std::shared_ptr<OpenGLContext> openGlContext);
    ~MetadataGeneratorFwk();
    VPEAlgoErrCode SetParameter(const MetadataGeneratorParameter &parameter) override;
    VPEAlgoErrCode GetParameter(MetadataGeneratorParameter &parameter) const override;
    VPEAlgoErrCode Process(const sptr<SurfaceBuffer> &input) override;

private:
    VPEAlgoErrCode Init(const sptr<SurfaceBuffer> &input);
    void OpenGLInit();
    
    std::shared_ptr<MetadataGeneratorBase> impl_ { nullptr };
    MetadataGeneratorParameter parameter_;
    std::atomic<bool> initialized_ { false };
    Extension::ExtensionInfo extensionInfo_;
    VPEContext context;
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif // FRAMEWORK_ALGORITHM_METADATA_GENERATOR_METADATA_GENERATOR_FWK_H
