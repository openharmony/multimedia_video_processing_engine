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

#ifndef CSCV_DESCRIPTION_H
#define CSCV_DESCRIPTION_H

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class __attribute__((visibility("default"))) CscVDescriptionKey {
public:
    /*
     * Key for colorspace type, value type is int32_t, see {link @CM_ColorPrimaries}
     */
    static constexpr std::string_view CSCV_KEY_COLORSPACE_PRIMARIES = "colorspace_primaries";

    /*
     * Key for colorspace type, value type is int32_t, see {link @CM_TransFunc}
     */
    static constexpr std::string_view CSCV_KEY_COLORSPACE_TRANS_FUNC = "colorspace_trans_func";

    /*
     * Key for colorspace type, value type is int32_t, see {link @CM_Matrix}
     */
    static constexpr std::string_view CSCV_KEY_COLORSPACE_MATRIX = "colorspace_matrix";

    /*
     * Key for colorspace type, value type is int32_t, see {link @CM_Range}
     */
    static constexpr std::string_view CSCV_KEY_COLORSPACE_RANGE = "colorspace_range";

    /*
     * Key for hdr metedata type, value type is int32_t, see {link @CM_HDRMetaDataType}
     */
    static constexpr std::string_view CSCV_KEY_HDR_METADATA_TYPE = "hdr_metadata_type";

    /*
     * Key for render intent, value type is int32_t, see {link @RenderIntent}
     */
    static constexpr std::string_view CSCV_KEY_RENDER_INTENT = "render_intent";

    /*
     * Key for brightness ratio for sdrui, value type is float
     */
    static constexpr std::string_view CSCV_KEY_SDRUI_BRIGHTNESS_RATIO = "sdrui_brightness_ratio";

    /*
     * Key for pixel format, value type is int, see {link @GraphicPixelFormat}
     */
    static constexpr std::string_view CSCV_KEY_PIXEL_FORMAT = "pixel_format";
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS
#endif // CSCV_DESCRIPTION_H
