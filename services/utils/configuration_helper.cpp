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

#include "configuration_helper.h"

#include <functional>

#include <securec.h>
#include <unistd.h>

#include "vpe_log.h"

using namespace OHOS::Media::VideoProcessingEngine;

namespace {
inline const std::string GetElementInfo(const xmlNode& parent, const std::string& tag,
    std::function<const std::string(const xmlNode&)>&& getter)
{
    const xmlNode* node = parent.children;
    CHECK_AND_RETURN_RET_LOG(node != nullptr, "", "No children when finding <%{public}s>!", tag.c_str());
    CHECK_AND_RETURN_RET_LOG(xmlStrcmp(node->name, reinterpret_cast<const xmlChar*>(tag.c_str())) == 0, "",
        "Element <%{public}s> is not found!", tag.c_str());
    return getter(*node);
}

inline const xmlNode* GetElement(const xmlNode& parent, const std::string& tag,
    std::function<bool(const xmlNode&)>&& checker, const std::string& notFoundLog)
{
    for (const xmlNode* node = parent.children; node != nullptr; node = node->next) {
        if (xmlStrcmp(node->name, reinterpret_cast<const xmlChar*>(tag.c_str())) != 0) {
            continue;
        }
        if (checker(*node)) {
            return node;
        }
    }
    VPE_LOGW("%{public}s is NOT found!", notFoundLog.c_str());
    return nullptr;
}
}

bool ConfigurationHelper::LoadConfigurationFromXml(const std::string& xmlFilePath)
{
    if (access(xmlFilePath.c_str(), R_OK) != 0) [[unlikely]] {
        VPE_LOGW("Invalid input: %{public}s is NOT exist!", xmlFilePath.c_str());
        return false;
    }

    xmlDocPtr doc = xmlParseFile(xmlFilePath.c_str());
    if (doc == nullptr) [[unlikely]] {
        VPE_LOGW("Failed to parse %{public}s!", xmlFilePath.c_str());
        return false;
    }

    xmlNode* root = xmlDocGetRootElement(doc);
    if (root == nullptr) [[unlikely]] {
        VPE_LOGW("Failed to parse %{public}s: there is no root element!", xmlFilePath.c_str());
        xmlFreeDoc(doc);
        return false;
    }

    auto ret = ParseXml(*root);
    if (!ret) {
        VPE_LOGW("Failed to parse %{public}s: Content is invalid!", xmlFilePath.c_str());
    }
    xmlFreeDoc(doc);
    return ret;
}

bool ConfigurationHelper::ParseXml([[maybe_unused]] const xmlNode& root)
{
    return true;
}

const xmlNode* ConfigurationHelper::GetElement(const xmlNode& parent, const std::string& tag) const
{
    return ::GetElement(parent, tag, [](const xmlNode&) { return true; }, "<" + tag + ">");
}

const xmlNode* ConfigurationHelper::GetElementByName(const xmlNode& parent,
    const std::string& tag, const std::string& name) const
{
    return ::GetElement(parent, tag, [&tag, &name](const xmlNode& node) {
        const xmlChar* nameText = xmlGetProp(&node, reinterpret_cast<const xmlChar*>("name"));
        if (nameText == nullptr) {
            VPE_LOGD("name of <%{public}s> is null!", tag.c_str());
            return false;
        }
        return name.compare(reinterpret_cast<const char*>(nameText)) == 0;
    }, "<" + tag + " name=\"" + name + "\">");
}

const std::string ConfigurationHelper::GetElementName(const xmlNode& parent, const std::string& tag) const
{
    return GetElementInfo(parent, tag, [this](const xmlNode& node) { return GetElementName(node); });
}

const std::string ConfigurationHelper::GetElementName(const xmlNode& myself) const
{
    const char* nameText = reinterpret_cast<char*>(xmlGetProp(&myself, reinterpret_cast<const xmlChar*>("name")));
    CHECK_AND_RETURN_RET_LOG(nameText != nullptr, "", "name of <%{public}s> is null!", myself.name);
    return std::string(nameText);
}

const std::string ConfigurationHelper::GetElementText(const xmlNode& parent, const std::string& tag) const
{
    return GetElementInfo(parent, tag, [this](const xmlNode& node) { return GetElementText(node); });
}

const std::string ConfigurationHelper::GetElementText(const xmlNode& myself) const
{
    const char* text = reinterpret_cast<char*>(xmlNodeGetContent(&myself));
    CHECK_AND_RETURN_RET_LOG(text != nullptr, "", "Element <%{public}s> is empty!", myself.name);
    return std::string(text);
}

bool ConfigurationHelper::GetElementValue(const xmlNode& parent, const std::string& tag, uint32_t& value) const
{
    value = 0;
    const std::string& text = GetElementText(parent, tag);
    if (text.empty()) {
        return false;
    }
    return sscanf_s(text.c_str(), "%u", &value) == 1;
}

bool ConfigurationHelper::GetElementValue(const xmlNode& parent, const std::string& tag, int& value) const
{
    value = 0;
    const std::string& text = GetElementText(parent, tag);
    if (text.empty()) {
        return false;
    }
    return sscanf_s(text.c_str(), "%d", &value) == 1;
}

bool ConfigurationHelper::GetElementValue(const xmlNode& parent, const std::string& tag, uint64_t& value) const
{
    value = 0;
    const std::string& text = GetElementText(parent, tag);
    if (text.empty()) {
        return false;
    }
    return sscanf_s(text.c_str(), "%lu", &value) == 1;
}

bool ConfigurationHelper::GetElementValue(const xmlNode& parent, const std::string& tag, bool& value) const
{
    const std::string& text = GetElementText(parent, tag);
    if (text.empty()) {
        value = false;
        return false;
    }
    value = (text.compare("true") == 0);
    return true;
}

bool ConfigurationHelper::GetElementValue(const xmlNode& parent, const std::string& tag, float& value) const
{
    value = 0;
    const std::string& text = GetElementText(parent, tag);
    if (text.empty()) {
        return false;
    }
    return sscanf_s(text.c_str(), "%f", &value) == 1;
}

bool ConfigurationHelper::GetElementValue(const xmlNode& parent, const std::string& tag, std::string& value) const
{
    value = GetElementText(parent, tag);
    return !value.empty();
}

bool ConfigurationHelper::GetElementValue(const xmlNode& myself, std::string& value) const
{
    value = GetElementText(myself);
    return !value.empty();
}
