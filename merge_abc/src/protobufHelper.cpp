/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <iostream>

namespace panda::proto {
std::string GetFilePath(const std::string &path)
{
#ifdef PANDA_TARGET_WINDOWS
    if (path.length() >= _MAX_PATH) {
        return path;
    } else {
        return panda::os::file::File::GetExtendedLengthStylePath(path);
    }
#else
    return path;
#endif  // PANDA_TARGET_WINDOWS
}
} // panda::proto