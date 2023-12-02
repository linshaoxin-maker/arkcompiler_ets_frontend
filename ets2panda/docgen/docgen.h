/**
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef ES2PANDA_DOCKGEN_DOCKGEN_H
#define ES2PANDA_DOCKGEN_DOCKGEN_H

#include "parser/program/program.h"

namespace panda::es2panda::docgen {
class Docgen {
public:
    Docgen() = default;
    ~Docgen() = default;
    NO_COPY_SEMANTIC(Docgen);
    NO_MOVE_SEMANTIC(Docgen);

    [[nodiscard]] virtual std::string Generate(parser::Program *prog) = 0;
};
}  // namespace panda::es2panda::docgen

#endif
