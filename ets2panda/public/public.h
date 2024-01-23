/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef ES2PANDA_PUBLIC_PUBLIC_H
#define ES2PANDA_PUBLIC_PUBLIC_H

#include "public/es2panda_lib.h"

#include "assembler/assembly-program.h"
#include "libpandabase/mem/arena_allocator.h"

#include "es2panda.h"
#include "compiler/core/compileQueue.h"
#include "parser/ETSparser.h"
#include "checker/checker.h"
#include "compiler/core/emitter.h"
#include "util/options.h"

namespace panda::es2panda::compiler {
class Phase;
}  // namespace panda::es2panda::compiler

namespace panda::es2panda::public_lib {
struct ConfigImpl {
    util::Options *options;
};

struct Context {
    ConfigImpl *config = nullptr;
    std::string sourceFileName;
    std::string input;
    SourceFile const *sourceFile = nullptr;
    ArenaAllocator *allocator = nullptr;
    compiler::CompileQueue *queue = nullptr;
    std::vector<util::Plugin> const *plugins = nullptr;
    std::vector<compiler::Phase *> phases;
    size_t currentPhase = 0;

    parser::Program *parserProgram = nullptr;
    parser::ParserImpl *parser = nullptr;
    checker::Checker *checker = nullptr;
    checker::SemanticAnalyzer *analyzer = nullptr;
    compiler::CompilerContext *compilerContext = nullptr;
    compiler::Emitter *emitter = nullptr;
    pandasm::Program *program = nullptr;

    es2panda_ContextState state = ES2PANDA_STATE_NEW;
    std::string errorMessage;
    lexer::SourcePosition errorPos;
};
}  // namespace panda::es2panda::public_lib

#endif
