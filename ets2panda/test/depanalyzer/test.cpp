/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include <vector>

#include "dependency_analyzer/dep_analyzer.h"
#include "panda_path_getter.h"
#include "os/filesystem.h"

TEST(Test, Subtestv1)
{
    DepAnalyzer da;
    std::string binPath = test::utils::PandaPathGetter {}.BinPathGet();
    std::string testPath = test::utils::PandaPathGetter {}.TestPathGet("test1/file.sts");

    const char **options = new const char *[2] { binPath.c_str(), testPath.c_str() };

    da.AnalyzeDeps(2, options);

    std::vector<std::string> answer;
    std::string ansPath = test::utils::PandaPathGetter {}.TestPathGet("test1/file.sts");
    answer.push_back(ark::os::GetAbsolutePath(ansPath));
    ASSERT(da.GetSourcesPath() == answer);

    delete[] options;
}

TEST(Test, Subtestv2)
{
    DepAnalyzer da;
    std::string binPath = test::utils::PandaPathGetter {}.BinPathGet();
    std::string testPath = test::utils::PandaPathGetter {}.TestPathGet("test2/file1.sts");

    const char **options = new const char *[2] { binPath.c_str(), testPath.c_str() };

    da.AnalyzeDeps(2, options);

    std::vector<std::string> answer;
    for (size_t i = 1; i < 5; ++i) {
        std::string ansPath = test::utils::PandaPathGetter {}.TestPathGet("test2/file" + std::to_string(i) + ".sts");
        answer.push_back(ark::os::GetAbsolutePath(ansPath));
    }
    ASSERT(da.GetSourcesPath() == answer);

    delete[] options;
}

TEST(Test, Subtestv3)
{
    DepAnalyzer da;
    std::string binPath = test::utils::PandaPathGetter {}.BinPathGet();
    std::string testPath = test::utils::PandaPathGetter {}.TestPathGet("test3/filea.sts");

    const char **options = new const char *[2] { binPath.c_str(), testPath.c_str() };

    da.AnalyzeDeps(2, options);

    std::vector<std::string> answer;
    std::string ansPath = test::utils::PandaPathGetter {}.TestPathGet("test3/filea.sts");
    answer.push_back(ark::os::GetAbsolutePath(ansPath));
    ansPath = test::utils::PandaPathGetter {}.TestPathGet("test3/filee.sts");
    answer.push_back(ark::os::GetAbsolutePath(ansPath));
    ansPath = test::utils::PandaPathGetter {}.TestPathGet("test3/fileb.sts");
    answer.push_back(ark::os::GetAbsolutePath(ansPath));
    ASSERT(da.GetSourcesPath() == answer);

    delete[] options;
}

TEST(Test, Subtestv4)
{
    DepAnalyzer da;
    std::string binPath = test::utils::PandaPathGetter {}.BinPathGet();
    std::string testPath = test::utils::PandaPathGetter {}.TestPathGet("test4/file.sts");

    const char **options = new const char *[2] { binPath.c_str(), testPath.c_str() };

    da.AnalyzeDeps(2, options);

    std::vector<std::string> answer;
    std::string ansPath = test::utils::PandaPathGetter {}.TestPathGet("test4/file.sts");
    answer.push_back(ark::os::GetAbsolutePath(ansPath));
    ASSERT(da.GetSourcesPath() == answer);

    delete[] options;
}
