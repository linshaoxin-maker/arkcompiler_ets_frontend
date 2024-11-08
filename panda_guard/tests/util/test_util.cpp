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

#include "test_util.h"

#include <filesystem>
#include <regex>

#include "gtest/gtest.h"

#include "util/file_util.h"

void panda::guard::TestUtil::RemoveFile(const std::string &filePath)
{
    std::filesystem::path file_name{filePath};
    std::filesystem::remove(file_name);
}

void panda::guard::TestUtil::ExcludeSpecialCharacters(std::string &data)
{
    data = std::regex_replace(data, std::regex("# source binary:[^\n]+\n"), "");
    data = std::regex_replace(data, std::regex("[\r\n\t' ']"), "");
}

void panda::guard::TestUtil::ValidateData(const std::string &oriPath, const std::string &expectPath)
{
    std::string oriData = FileUtil::GetFileContent(oriPath);
    EXPECT_FALSE(oriData.empty());
    ExcludeSpecialCharacters(oriData);

    std::string expectData = FileUtil::GetFileContent(expectPath);
    EXPECT_FALSE(expectData.empty());
    ExcludeSpecialCharacters(expectData);

    EXPECT_EQ(oriData, expectData);
}