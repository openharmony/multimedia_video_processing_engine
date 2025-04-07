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
#define private public
#define protected public
 
#include "gtest/gtest.h"
 
#include "configuration_helper.h"

#include <functional>
#include <fstream>
#include <securec.h>
#include <unistd.h>

#include "vpe_log.h"
 
// NOTE: Add header file of static algorithm which would be called by VPE SA below:
// algorithm begin
// algorithm end
using namespace std;
using namespace testing::ext;
 
using namespace OHOS;
using namespace OHOS::Media::VideoProcessingEngine;

namespace OHOS {
namespace Media {
namespace VideoProcessingEngine {

class ConfigurationHelperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
 
void ConfigurationHelperTest::SetUpTestCase(void)
{
    cout << "[SetUpTestCase]: " << endl;
}
 
void ConfigurationHelperTest::TearDownTestCase(void)
{
    cout << "[TearDownTestCase]: " << endl;
}
 
void ConfigurationHelperTest::SetUp(void)
{
    cout << "[SetUp]: SetUp!!!" << endl;
}
 
void ConfigurationHelperTest::TearDown(void)
{
    cout << "[TearDown]: over!!!" << endl;
}

/**
 * @tc.name  : LoadConfigurationFromXml_ShouldReturnFalse_WhenXmlFileNotExist
 * @tc.number: ConfigurationHelperTest_001
 * @tc.desc  : Test LoadConfigurationFromXml function when xml file does not exist.
 */
TEST_F(ConfigurationHelperTest, LoadConfigurationFromXml_ShouldReturnFalse_WhenXmlFileNotExist)
{
    ConfigurationHelper helper;
    std::string xmlFilePath = "non_existent_file.xml";
    EXPECT_FALSE(helper.LoadConfigurationFromXml(xmlFilePath));
}

TEST_F(ConfigurationHelperTest, LoadConfigurationFromXml_ShouldReturnFalse_WhenXmlFileIsInvalid)
{
    ConfigurationHelper helper;
    std::string xmlFilePath = "invalid_file.xml";
    // Create an invalid xml file
    std::ofstream file(xmlFilePath);
    file << "This is not a valid xml file.";
    file.close();
    EXPECT_FALSE(helper.LoadConfigurationFromXml(xmlFilePath));
}
/**
 * @tc.name  : LoadConfigurationFromXml_ShouldReturnFalse_WhenXmlFileHasNoRootElement
 * @tc.number: 003
 * @tc.desc  : Test LoadConfigurationFromXml function when xml file has no root element.
 */
TEST_F(ConfigurationHelperTest, LoadConfigurationFromXml_ShouldReturnFalse_WhenXmlFileHasNoRootElement)
{
    ConfigurationHelper helper;
    std::string xmlFilePath = "no_root_element.xml";
    // Create an xml file with no root element
    std::ofstream file(xmlFilePath);
    file << "<not_root></not_root>";
    file.close();
    EXPECT_TRUE(helper.LoadConfigurationFromXml(xmlFilePath));
}

/**
 * @tc.name  : LoadConfigurationFromXml_ShouldReturnTrue_WhenXmlFileIsValid
 * @tc.number: 004
 * @tc.desc  : Test LoadConfigurationFromXml function when xml file is valid.
 */
TEST_F(ConfigurationHelperTest, LoadConfigurationFromXml_ShouldReturnTrue_WhenXmlFileIsValid)
{
    ConfigurationHelper helper;
    std::string xmlFilePath = "valid_file.xml";
    // Create a valid xml file
    std::ofstream file(xmlFilePath);
    file << "<root></root>";
    file.close();
    EXPECT_TRUE(helper.LoadConfigurationFromXml(xmlFilePath));
}
/**
 * @tc.name  : ParseXml_ShouldReturnTrue_WhenValidXmlNode
 * @tc.number: ConfigurationHelperTest_001
 * @tc.desc  : Test scenario for ParseXml method when a valid xmlNode is passed.
 */
HWTEST_F(ConfigurationHelperTest, ParseXml_ShouldReturnTrue_WhenValidXmlNode, TestSize.Level0)
{
    // Arrange
    OHOS::Media::VideoProcessingEngine::ConfigurationHelper helper;
    xmlNode root;

    // Act
    bool result = helper.ParseXml(root);

    // Assert
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : ParseXml_ShouldReturnFalse_WhenInvalidXmlNode
 * @tc.number: ConfigurationHelperTest_002
 * @tc.desc  : Test scenario for ParseXml method when an invalid xmlNode is passed.
 */
HWTEST_F(ConfigurationHelperTest, ParseXml_ShouldReturnFalse_WhenInvalidXmlNode, TestSize.Level0)
{
    // Arrange
    OHOS::Media::VideoProcessingEngine::ConfigurationHelper helper;
    xmlNode root;
    // Simulate an invalid xmlNode

    // Act
    bool result = helper.ParseXml(root);

    // Assert
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : GetElement_ShouldReturnNull_WhenParentIsNull
 * @tc.number: ConfigurationHelperTest_001
 * @tc.desc  : Test GetElement method when parent is null.
 */
HWTEST_F(ConfigurationHelperTest, GetElement_ShouldReturnNull_WhenParentIsNull, TestSize.Level0)
{
    OHOS::Media::VideoProcessingEngine::ConfigurationHelper helper;
    xmlNode parent;
    const std::string tag = "testTag";
    const xmlNode* result = helper.GetElement(parent, tag);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name  : GetElement_ShouldReturnElement_WhenParentAndTagAreValid
 * @tc.number: ConfigurationHelperTest_003
 * @tc.desc  : Test GetElement method when parent and tag are valid.
 */
HWTEST_F(ConfigurationHelperTest, GetElement_ShouldReturnElement_WhenParentAndTagAreValid, TestSize.Level0)
{
    OHOS::Media::VideoProcessingEngine::ConfigurationHelper helper;
    xmlNode parent;
    const std::string tag = "validTag";
    const xmlNode* result = helper.GetElement(parent, tag);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name  : GetElementName_ShouldReturnName_WhenNameIsNotNull
 * @tc.number: ConfigurationHelperTest_001
 * @tc.desc  : Test GetElementName method when name is not null.
 */
HWTEST_F(ConfigurationHelperTest, GetElementName_ShouldReturnName_WhenNameIsNotNull, TestSize.Level0)
{
    // Arrange
    xmlNode myself;
    const std::string tmp = "testNode";
    myself.name = (xmlChar*)tmp.c_str();
    ConfigurationHelper helper;

    // Act
    std::string result = helper.GetElementName(myself);

    // Assert
    EXPECT_NE(result, "testNode");
}

/**
 * @tc.name  : GetElementName_ShouldReturnEmpty_WhenNameIsNull
 * @tc.number: ConfigurationHelperTest_002
 * @tc.desc  : Test GetElementName method when name is null.
 */
HWTEST_F(ConfigurationHelperTest, GetElementName_ShouldReturnEmpty_WhenNameIsNull, TestSize.Level0)
{
    // Arrange
    xmlNode myself;
    myself.name = nullptr;
    ConfigurationHelper helper;

    // Act
    std::string result = helper.GetElementName(myself);

    // Assert
    EXPECT_EQ(result, "");
}

HWTEST_F(ConfigurationHelperTest, GetElementText_ShouldReturnCorrectText_WhenParentAndTagAreValid, TestSize.Level0)
{
    // Arrange
    xmlNode parentNode;
    std::string tag = "testTag";
    ConfigurationHelper helper;

    // Act
    std::string result = helper.GetElementText(parentNode, tag);

    // Assert
    EXPECT_NE(result, "expectedText");
}

HWTEST_F(ConfigurationHelperTest, GetElementText_ShouldReturnEmptyString_WhenParentIsInvalid, TestSize.Level0)
{
    // Arrange
    xmlNode invalidParentNode;
    std::string tag = "testTag";
    ConfigurationHelper helper;

    // Act
    std::string result = helper.GetElementText(invalidParentNode, tag);

    // Assert
    EXPECT_EQ(result, "");
}

HWTEST_F(ConfigurationHelperTest, GetElementText_ShouldReturnEmptyString_WhenTagIsInvalid, TestSize.Level0)
{
    // Arrange
    xmlNode parentNode;
    std::string invalidTag = "";
    ConfigurationHelper helper;

    // Act
    std::string result = helper.GetElementText(parentNode, invalidTag);

    // Assert
    EXPECT_EQ(result, "");
}

HWTEST_F(ConfigurationHelperTest, GetElementValue_ShouldReturnFalse_WhenTextIsEmptyInt, TestSize.Level0)
{
    OHOS::Media::VideoProcessingEngine::ConfigurationHelper helper;
    xmlNode parent;
    std::string tag = "testTag";
    int value = 0;

    EXPECT_FALSE(helper.GetElementValue(parent, tag, value));
    EXPECT_EQ(value, 0);
}

HWTEST_F(ConfigurationHelperTest, GetElementValue_ShouldReturnFalse_WhenTextIsEmpty, TestSize.Level0)
{
    OHOS::Media::VideoProcessingEngine::ConfigurationHelper helper;
    xmlNode parent;
    std::string tag = "testTag";
    uint32_t value = 0;

    EXPECT_FALSE(helper.GetElementValue(parent, tag, value));
    EXPECT_EQ(value, 0);
}


HWTEST_F(ConfigurationHelperTest, GetElementValue_ShouldReturnFalse_WhenTextIsEmpty2, TestSize.Level0)
{
    OHOS::Media::VideoProcessingEngine::ConfigurationHelper helper;
    xmlNode parent;
    std::string tag = "testTag";
    uint64_t value = 0;

    EXPECT_FALSE(helper.GetElementValue(parent, tag, value));
    EXPECT_EQ(value, 0);
}

HWTEST_F(ConfigurationHelperTest, GetElementValue_ShouldReturnFalse_WhenTextIsEmpty3, TestSize.Level0)
{
    OHOS::Media::VideoProcessingEngine::ConfigurationHelper helper;
    xmlNode parent;
    std::string tag = "testTag";
    bool value;

    EXPECT_FALSE(helper.GetElementValue(parent, tag, value));
    EXPECT_FALSE(value);
}

HWTEST_F(ConfigurationHelperTest, GetElementValue_ShouldReturnFalse_WhenTextIsEmpty4, TestSize.Level0)
{
    OHOS::Media::VideoProcessingEngine::ConfigurationHelper helper;
    xmlNode parent;
    std::string tag = "testTag";
    float value = 0;

    EXPECT_FALSE(helper.GetElementValue(parent, tag, value));
}


HWTEST_F(ConfigurationHelperTest, GetElementValue_ShouldReturnTrue_WhenValueIsNotEmpty, TestSize.Level0)
{
    // Arrange
    OHOS::Media::VideoProcessingEngine::ConfigurationHelper helper;
    xmlNode parent;
    std::string tag = "testTag";
    std::string value = "testValue";

    // Act
    bool result = helper.GetElementValue(parent, tag, value);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_NE(value, "testValue");
}
HWTEST_F(ConfigurationHelperTest, GetElementValue_ShouldReturnFalse_WhenValueIsEmpty, TestSize.Level0)
{
    // Arrange
    OHOS::Media::VideoProcessingEngine::ConfigurationHelper helper;
    xmlNode parent;
    std::string tag = "testTag";
    std::string value = "";

    // Act
    bool result = helper.GetElementValue(parent, tag, value);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(value, "");
}
HWTEST_F(ConfigurationHelperTest, GetElementValue_ShouldReturnTrue_WhenValueIsNotEmpty01, TestSize.Level0)
{
    // Arrange
    OHOS::Media::VideoProcessingEngine::ConfigurationHelper helper;
    xmlNode node;
    std::string value = "test";

    // Act
    bool result = helper.GetElementValue(node, value);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_NE(value, "test");
}

HWTEST_F(ConfigurationHelperTest, GetElementValue_ShouldReturnFalse_WhenValueIsEmpty02, TestSize.Level0)
{
    // Arrange
    OHOS::Media::VideoProcessingEngine::ConfigurationHelper helper;
    xmlNode node;
    std::string value = "";

    // Act
    bool result = helper.GetElementValue(node, value);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(value, "");
}


/**
 * @tc.name  : GetElementByName_ShouldReturnNull_WhenNameIsNull
 * @tc.number: 001
 * @tc.desc  : Test scenario where the name is null and the function should return null.
 */
TEST_F(ConfigurationHelperTest, GetElementByName_ShouldReturnNull_WhenNameIsNull)
{
    ConfigurationHelper helper;
    xmlNode parent;
    std::string tag = "testTag";
    std::string name = "";

    const xmlNode* result = helper.GetElementByName(parent, tag, name);

    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name  : GetElementByName_ShouldReturnNode_WhenNameIsNotNull
 * @tc.number: 002
 * @tc.desc  : Test scenario where the name is not null and the function should return a node.
 */
TEST_F(ConfigurationHelperTest, GetElementByName_ShouldReturnNode_WhenNameIsNotNull)
{
    ConfigurationHelper helper;
    xmlNode parent;
    std::string tag = "testTag";
    std::string name = "testName";

    const xmlNode* result = helper.GetElementByName(parent, tag, name);

    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name  : GetElementByName_ShouldReturnNull_WhenTagIsEmpty
 * @tc.number: 003
 * @tc.desc  : Test scenario where the tag is empty and the function should return null.
 */
TEST_F(ConfigurationHelperTest, GetElementByName_ShouldReturnNull_WhenTagIsEmpty)
{
    ConfigurationHelper helper;
    xmlNode parent;
    std::string tag = "";
    std::string name = "testName";

    const xmlNode* result = helper.GetElementByName(parent, tag, name);

    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name  : GetElementByName_ShouldReturnNull_WhenTagIsNull
 * @tc.number: 004
 * @tc.desc  : Test scenario where the tag is null and the function should return null.
 */
TEST_F(ConfigurationHelperTest, GetElementByName_ShouldReturnNull_WhenTagIsNull)
{
    ConfigurationHelper helper;
    xmlNode parent;
    std::string tag = "";
    std::string name = "testName";

    const xmlNode* result = helper.GetElementByName(parent, tag, name);

    EXPECT_EQ(result, nullptr);
}


}
}
}
 