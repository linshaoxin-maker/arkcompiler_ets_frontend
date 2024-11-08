/**
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

#include <gtest/gtest.h>

#include "guard_driver.h"

#include "util/test_util.h"

using namespace testing::ext;
using namespace panda::guard;

namespace {
    const std::string GRAMMAR_TEST_OUT_DIR = PANDA_GUARD_GRAMMAR_OUT_DIR;
    const std::string GRAMMAR_TEST_EXPECT_DIR = PANDA_GUARD_GRAMMAR_EXPECT_DIR;
    const std::string PA_FILE_SUFFIX = ".pa";
    const std::string JSON_FILE_SUFFIX = ".json";
    const std::string CACHE_FILE_SUFFIX = ".cache.json";
}

#define DECLARE_VALIDATE_GRAMMAR_TEST(projectName, configName)\
class GrammarTest_##projectName##_##configName : public testing::Test {\
public:\
    static void SetUpTestCase() {}\
    static void TearDownTestCase() {}\
    void SetUp() override {}\
    void TearDown() override {}\
    std::string GetProjectName()\
    {\
        return #projectName;\
    }\
    std::string GetConfigName()\
    {\
        return #configName;\
    }\
};\
\
HWTEST_F(GrammarTest_##projectName##_##configName, should_success_when_obf_##projectName##_with_##configName, TestSize.Level4)\
{\
    std::string configPath = GRAMMAR_TEST_OUT_DIR + this->GetProjectName() + "/" + this->GetConfigName() + JSON_FILE_SUFFIX;\
    int argc = 3;\
    char *argv[3];\
    argv[0] = const_cast<char *>("xxx");\
    argv[1] = const_cast<char *>("--debug");\
    argv[2] = const_cast<char *>(configPath.c_str());\
    GuardDriver driver;\
    driver.Run(argc, const_cast<const char **>(argv));\
\
    std::string nameCachePath = GRAMMAR_TEST_OUT_DIR + this->GetProjectName() + "/obf/" + this->GetConfigName() + CACHE_FILE_SUFFIX;\
    std::string expectNameCachePath = GRAMMAR_TEST_EXPECT_DIR + this->GetProjectName() + "/" + this->GetConfigName() + CACHE_FILE_SUFFIX;\
    TestUtil::ValidateData(nameCachePath, expectNameCachePath);\
\
    std::string paPath = GRAMMAR_TEST_OUT_DIR + this->GetProjectName() + "/obf/" + this->GetConfigName() + PA_FILE_SUFFIX;\
    std::string expectPaPath = GRAMMAR_TEST_EXPECT_DIR + this->GetProjectName() + "/" + this->GetConfigName() + PA_FILE_SUFFIX;\
    TestUtil::ValidateData(paPath, expectPaPath);\
}

DECLARE_VALIDATE_GRAMMAR_TEST(advanced_type, config)
DECLARE_VALIDATE_GRAMMAR_TEST(array_validation, config)
DECLARE_VALIDATE_GRAMMAR_TEST(circulations, config)
DECLARE_VALIDATE_GRAMMAR_TEST(class_validation, config)
DECLARE_VALIDATE_GRAMMAR_TEST(console, config)
DECLARE_VALIDATE_GRAMMAR_TEST(constructor_property_export, config)
DECLARE_VALIDATE_GRAMMAR_TEST(constructor_property_property, config)
DECLARE_VALIDATE_GRAMMAR_TEST(constructor_property_toplevel, config)
DECLARE_VALIDATE_GRAMMAR_TEST(control_statement, config)
DECLARE_VALIDATE_GRAMMAR_TEST(data_type, config)
DECLARE_VALIDATE_GRAMMAR_TEST(date_validation, config)
DECLARE_VALIDATE_GRAMMAR_TEST(declare_global, config)
DECLARE_VALIDATE_GRAMMAR_TEST(double_underscore_constructor_default, config)
DECLARE_VALIDATE_GRAMMAR_TEST(double_underscore_constructor_property, config)
DECLARE_VALIDATE_GRAMMAR_TEST(double_underscore_exportDefault, config)
DECLARE_VALIDATE_GRAMMAR_TEST(double_underscore_method, config)
DECLARE_VALIDATE_GRAMMAR_TEST(double_underscore_nosymbolidentifier, config)
DECLARE_VALIDATE_GRAMMAR_TEST(double_underscore_shorthand, config)
DECLARE_VALIDATE_GRAMMAR_TEST(enum, config)
DECLARE_VALIDATE_GRAMMAR_TEST(export, config)
DECLARE_VALIDATE_GRAMMAR_TEST(export_default, config)
DECLARE_VALIDATE_GRAMMAR_TEST(export_namecache, config)
DECLARE_VALIDATE_GRAMMAR_TEST(export_obfuscation, config)
DECLARE_VALIDATE_GRAMMAR_TEST(function_like_namecache_remove_console, config)
DECLARE_VALIDATE_GRAMMAR_TEST(function_usage, config)
DECLARE_VALIDATE_GRAMMAR_TEST(function_validation, config)
DECLARE_VALIDATE_GRAMMAR_TEST(generics_validation, config)
DECLARE_VALIDATE_GRAMMAR_TEST(getsetaccessor_defaultConfig, config)
//DECLARE_VALIDATE_GRAMMAR_TEST(getsetaccessor_propertyConfig, config)
DECLARE_VALIDATE_GRAMMAR_TEST(import_name, config)
DECLARE_VALIDATE_GRAMMAR_TEST(interface_validation, config)
DECLARE_VALIDATE_GRAMMAR_TEST(in_operator, config)
DECLARE_VALIDATE_GRAMMAR_TEST(jsfile, config)
DECLARE_VALIDATE_GRAMMAR_TEST(keepPaths_fileNameObfs, config)
DECLARE_VALIDATE_GRAMMAR_TEST(keepPaths_SourceCode, config)
DECLARE_VALIDATE_GRAMMAR_TEST(module_validation, config)
DECLARE_VALIDATE_GRAMMAR_TEST(namespace, config)
DECLARE_VALIDATE_GRAMMAR_TEST(number_validation, config)
//DECLARE_VALIDATE_GRAMMAR_TEST(obfuscation_validation, config)
DECLARE_VALIDATE_GRAMMAR_TEST(obj, config)
DECLARE_VALIDATE_GRAMMAR_TEST(print_unobfuscation, config)
DECLARE_VALIDATE_GRAMMAR_TEST(rename_file_name, config)
DECLARE_VALIDATE_GRAMMAR_TEST(rename_file_name_ohmurl_file, config)
DECLARE_VALIDATE_GRAMMAR_TEST(rename_file_name_ohmurl_file_normalized, config)
DECLARE_VALIDATE_GRAMMAR_TEST(samename, config)
DECLARE_VALIDATE_GRAMMAR_TEST(shorthand_defaultConfig, config)
DECLARE_VALIDATE_GRAMMAR_TEST(shorthand_initializer, config)
DECLARE_VALIDATE_GRAMMAR_TEST(shorthand_toplevelConfig, config)
DECLARE_VALIDATE_GRAMMAR_TEST(string_validation, config)
DECLARE_VALIDATE_GRAMMAR_TEST(target, config)
DECLARE_VALIDATE_GRAMMAR_TEST(toplevel, config)
DECLARE_VALIDATE_GRAMMAR_TEST(toplevel_obfuscation_close_export, config)
DECLARE_VALIDATE_GRAMMAR_TEST(toplevel_obfuscation_open_export, config)
DECLARE_VALIDATE_GRAMMAR_TEST(types_definition, config)
DECLARE_VALIDATE_GRAMMAR_TEST(variable_declaration, config)
DECLARE_VALIDATE_GRAMMAR_TEST(wildcard_filename_keep_filename1, config)
DECLARE_VALIDATE_GRAMMAR_TEST(wildcard_filename_keep_filename2, config)
DECLARE_VALIDATE_GRAMMAR_TEST(wildcard_filename_keep_filename3, config)
DECLARE_VALIDATE_GRAMMAR_TEST(wildcard_filename_keep_filename4, config)
DECLARE_VALIDATE_GRAMMAR_TEST(wildcard_filename_keep_filename5, config)
DECLARE_VALIDATE_GRAMMAR_TEST(wildcard_filename_keep_filename6, config)
DECLARE_VALIDATE_GRAMMAR_TEST(wildcard_global_keep_global1, config)
DECLARE_VALIDATE_GRAMMAR_TEST(wildcard_global_keep_global2, config)
DECLARE_VALIDATE_GRAMMAR_TEST(wildcard_global_keep_global3, config)
DECLARE_VALIDATE_GRAMMAR_TEST(wildcard_property_keep_property1, config)
DECLARE_VALIDATE_GRAMMAR_TEST(wildcard_property_keep_property2, config)
DECLARE_VALIDATE_GRAMMAR_TEST(wildcard_property_keep_property3, config)
DECLARE_VALIDATE_GRAMMAR_TEST(wildcard_property_keep_property4, config)
DECLARE_VALIDATE_GRAMMAR_TEST(wildcard_property_keep_property5, config)