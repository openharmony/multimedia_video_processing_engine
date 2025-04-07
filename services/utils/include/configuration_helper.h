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

#ifndef CONFIGURATION_HELPER_H
#define CONFIGURATION_HELPER_H

#include <cinttypes>
#include <string>

#include <libxml/parser.h>

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {
class ConfigurationHelper {
protected:
    ConfigurationHelper() = default;
    virtual ~ConfigurationHelper() = default;
    ConfigurationHelper(const ConfigurationHelper&) = delete;
    ConfigurationHelper& operator=(const ConfigurationHelper&) = delete;
    ConfigurationHelper(ConfigurationHelper&&) = delete;
    ConfigurationHelper& operator=(ConfigurationHelper&&) = delete;

    bool LoadConfigurationFromXml(const std::string& xmlFilePath);

    virtual bool ParseXml(const xmlNode& root);

    const xmlNode* GetElement(const xmlNode& parent, const std::string& tag) const;
    const xmlNode* GetElementByName(const xmlNode& parent, const std::string& tag, const std::string& name) const;

    const std::string GetElementName(const xmlNode& parent, const std::string& tag) const;
    const std::string GetElementName(const xmlNode& myself) const;
    const std::string GetElementText(const xmlNode& parent, const std::string& tag) const;
    const std::string GetElementText(const xmlNode& myself) const;
    bool GetElementValue(const xmlNode& parent, const std::string& tag, uint32_t& value) const;
    bool GetElementValue(const xmlNode& parent, const std::string& tag, int& value) const;
    bool GetElementValue(const xmlNode& parent, const std::string& tag, uint64_t& value) const;
    bool GetElementValue(const xmlNode& parent, const std::string& tag, bool& value) const;
    bool GetElementValue(const xmlNode& parent, const std::string& tag, float& value) const;
    bool GetElementValue(const xmlNode& parent, const std::string& tag, std::string& value) const;
    bool GetElementValue(const xmlNode& myself, std::string& value) const;
};
} // namespace VideoProcessingEngine
} // namespace Media
} // namespace OHOS

#endif /* CONFIGURATION_HELPER_H */
