/**
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

#ifndef ES2PANDA_COMPILER_CORE_JSCOMPILER_H
#define ES2PANDA_COMPILER_CORE_JSCOMPILER_H

#include "compiler/core/ASTCompiler.h"

namespace panda::es2panda::ir {
class AstNode;
}  // namespace panda::es2panda::ir
namespace panda::es2panda::compiler {

class JSCompiler final : public AstCompiler {
public:
    JSCompiler() = default;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_JSCOMPILER_COMPILE_METHOD(_, nodeType) void Compile(const ir::nodeType *node) const override;
    AST_NODE_MAPPING(DECLARE_JSCOMPILER_COMPILE_METHOD)
#undef DECLARE_JSCOMPILER_COMPILE_METHOD

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_JSCOMPILER_COMPILE_METHOD(_, __, nodeType, ___) void Compile(const ir::nodeType *node) const override;
    AST_NODE_REINTERPRET_MAPPING(DECLARE_JSCOMPILER_COMPILE_METHOD)
#undef DECLARE_JSCOMPILER_COMPILE_METHOD

private:
    PandaGen *GetPandaGen() const;
};

}  // namespace panda::es2panda::compiler

#endif  // ES2PANDA_COMPILER_CORE_JSCOMPILER_H
