/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_PATH_GETTER
#define PANDA_PATH_GETTER

#include "macros.h"
#include <string>

namespace test::utils {

class PandaPathGetter {
public:
    PandaPathGetter() = default;

    std::string BinPathGet() const
    {
#ifdef BUILD_FOLDER
        return BUILD_FOLDER + std::string("/bin-gtests/es2panda_depanalyz_tests");
#else
        ASSERT_PRINT(false, "BUILD FOLDER not set");
        return std::string {};
#endif
    }

    std::string TestPathGet(std::string test_path) const
    {
#ifdef TESTS_FOLDER
        return TESTS_FOLDER + std::string("/") + test_path;
#else
        ASSERT_PRINT(false, "TESTS FOLDER not set");
        return std::string {};
#endif
    }
};

}  // namespace test::utils

#endif
