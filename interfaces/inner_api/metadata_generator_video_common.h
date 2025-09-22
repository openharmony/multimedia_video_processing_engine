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
#ifndef METADATA_GENERATOR_VIDEO_COMMON_H
#define METADATA_GENERATOR_VIDEO_COMMON_H
namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
enum MdgBufferFlag : uint32_t {
    MDG_BUFFER_FLAG_NONE = 0,
    /* This signals the end of stream */
    MDG_BUFFER_FLAG_EOS = 1 << 0,
};

enum VIDEO_META_GEN_STYLE {
    META_GEN_BRIGHT_STYLE = 0,
    META_GEN_CONTRAST_STYLE = 1,
};

class __attribute__((visibility("default"))) MetadataGeneratorVideoCallback {
public:
    virtual ~MetadataGeneratorVideoCallback() = default;
    /* *
     * Called when an error occurred.
     *
     * @param errorCode Error code.
     * @since 5.0
     */
    virtual void OnError(int32_t errorCode) = 0;

    /* *
     * Called when an state changed.
     *
     * @param state current state.
     * @since 5.0
     */
    virtual void OnState(int32_t state) = 0;

    /* *
     * Called when an output buffer becomes available.
     *
     * @param index The index of the available output buffer.
     * @since 5.0
     */
    virtual void OnOutputBufferAvailable(uint32_t index, MdgBufferFlag flag) = 0;
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
#endif // METADATA_GENERATOR_VIDEO_COMMON_H
