/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef ES2PANDA_COMPILER_CORE_ETSFUNCTION_H
#define ES2PANDA_COMPILER_CORE_ETSFUNCTION_H

#include "ir/irnode.h"

namespace panda::es2panda::ir {
class ScriptFunction;
class BlockStatement;
}  // namespace panda::es2panda::ir

namespace panda::es2panda::compiler {
class ETSGen;

class ETSFunction {
public:
    ETSFunction() = delete;

    static void Compile(ETSGen *etsg);

private:
    static void GenerateEnumMembers(ETSGen *etsg, const ir::AstNode *node, VReg array_obj,
                                    const ir::TSEnumMember *enum_member, int32_t index);
    static void CompileSourceBlock(ETSGen *etsg, const ir::BlockStatement *block);
    static void CompileFunction(ETSGen *etsg);
    static void CallImplicitCtor(ETSGen *etsg);
};
}  // namespace panda::es2panda::compiler

#endif
