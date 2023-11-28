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

#include "compilerImpl.h"

#include "compiler/core/compilerContext.h"
#include "compiler/core/compileQueue.h"
#include "compiler/core/compilerImpl.h"
#include "compiler/core/pandagen.h"
#include "compiler/core/ETSCompiler.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/JSCompiler.h"
#include "compiler/core/JSemitter.h"
#include "compiler/core/ETSemitter.h"
#include "compiler/lowering/phase.h"
#include "parser/parserImpl.h"
#include "parser/JSparser.h"
#include "parser/ASparser.h"
#include "parser/TSparser.h"
#include "parser/ETSparser.h"
#include "parser/program/program.h"
#include "varbinder/JSBinder.h"
#include "varbinder/ASBinder.h"
#include "varbinder/TSBinder.h"
#include "varbinder/ETSBinder.h"
#include "checker/TSAnalyzer.h"
#include "checker/TSchecker.h"
#include "checker/ETSAnalyzer.h"
#include "checker/ETSchecker.h"
#include "checker/ASchecker.h"
#include "checker/JSchecker.h"
#include "es2panda.h"
#include "util/declgenEts2Ts.h"

namespace panda::es2panda::compiler {

void CompilerImpl::HandleContextLiterals(CompilerContext *context)
{
    auto *emitter = context->GetEmitter();

    uint32_t index = 0;
    for (const auto &buff : context->ContextLiterals()) {
        emitter->AddLiteralBuffer(buff, index++);
    }

    emitter->LiteralBufferIndex() += context->ContextLiterals().size();
}

panda::pandasm::Program *CompilerImpl::Emit(CompilerContext *context)
{
    HandleContextLiterals(context);

    queue_.Schedule(context);

    /* Main thread can also be used instead of idling */
    queue_.Consume();
    auto *emitter = context->GetEmitter();
    queue_.Wait([emitter](CompileJob *job) { emitter->AddProgramElement(job->GetProgramElement()); });

    return emitter->Finalize(context->DumpDebugInfo(), Signatures::ETS_GLOBAL);
}

template <typename CodeGen, typename RegSpiller, typename FunctionEmitter, typename Emitter, typename AstCompiler>
static CompilerContext::CodeGenCb MakeCompileJob()
{
    return [](CompilerContext *context, varbinder::FunctionScope *scope,
              compiler::ProgramElement *program_element) -> void {
        RegSpiller reg_spiller;
        ArenaAllocator allocator(SpaceType::SPACE_TYPE_COMPILER, nullptr, true);
        AstCompiler astcompiler;
        CodeGen cg(&allocator, &reg_spiller, context, scope, program_element, &astcompiler);
        FunctionEmitter func_emitter(&cg, program_element);
        func_emitter.Generate();
    };
}

using EmitCb = std::function<pandasm::Program *(compiler::CompilerContext *)>;
using PhaseListGetter = std::function<std::vector<compiler::Phase *>()>;

template <typename Parser, typename VarBinder, typename Checker, typename Analyzer, typename AstCompiler,
          typename CodeGen, typename RegSpiller, typename FunctionEmitter, typename Emitter>
static pandasm::Program *CreateCompiler(const CompilationUnit &unit, const PhaseListGetter &get_phases,
                                        const EmitCb &emit_cb)
{
    ArenaAllocator allocator(SpaceType::SPACE_TYPE_COMPILER, nullptr, true);
    auto program = parser::Program::NewProgram<VarBinder>(&allocator);
    program.MarkEntry();
    auto parser = Parser(&program, unit.options, static_cast<parser::ParserStatus>(unit.raw_parser_status));
    auto checker = Checker();
    auto analyzer = Analyzer(&checker);
    checker.SetAnalyzer(&analyzer);

    auto *varbinder = program.VarBinder();
    varbinder->SetProgram(&program);

    CompilerContext context(varbinder, &checker, unit.options,
                            MakeCompileJob<CodeGen, RegSpiller, FunctionEmitter, Emitter, AstCompiler>());
    varbinder->SetCompilerContext(&context);

    auto emitter = Emitter(&context);
    context.SetEmitter(&emitter);

    context.SetParser(&parser);

    parser.ParseScript(unit.input, unit.options.compilation_mode == CompilationMode::GEN_STD_LIB);
    if constexpr (std::is_same_v<Parser, parser::ETSParser> && std::is_same_v<VarBinder, varbinder::ETSBinder>) {
        reinterpret_cast<varbinder::ETSBinder *>(varbinder)->FillResolvedImportPathes(parser.ResolvedParsedSourcesMap(),
                                                                                      &allocator);
    }
    for (auto *phase : get_phases()) {
        if (!phase->Apply(&context, &program)) {
            return nullptr;
        }
    }

    emitter.GenAnnotation();

    return emit_cb(&context);
}

pandasm::Program *CompilerImpl::Compile(const CompilationUnit &unit)
{
    auto emit_cb = [this](CompilerContext *context) -> pandasm::Program * { return Emit(context); };

    switch (unit.ext) {
        case ScriptExtension::TS: {
            return CreateCompiler<parser::TSParser, varbinder::TSBinder, checker::TSChecker, checker::TSAnalyzer,
                                  compiler::JSCompiler, compiler::PandaGen, compiler::DynamicRegSpiller,
                                  compiler::JSFunctionEmitter, compiler::JSEmitter>(unit, compiler::GetTrivialPhaseList,
                                                                                    emit_cb);
        }
        case ScriptExtension::AS: {
            return CreateCompiler<parser::ASParser, varbinder::ASBinder, checker::ASChecker, checker::TSAnalyzer,
                                  compiler::JSCompiler, compiler::PandaGen, compiler::DynamicRegSpiller,
                                  compiler::JSFunctionEmitter, compiler::JSEmitter>(unit, compiler::GetTrivialPhaseList,
                                                                                    emit_cb);
        }
        case ScriptExtension::ETS: {
            return CreateCompiler<parser::ETSParser, varbinder::ETSBinder, checker::ETSChecker, checker::ETSAnalyzer,
                                  compiler::ETSCompiler, compiler::ETSGen, compiler::StaticRegSpiller,
                                  compiler::ETSFunctionEmitter, compiler::ETSEmitter>(unit, compiler::GetETSPhaseList,
                                                                                      emit_cb);
        }
        case ScriptExtension::JS: {
            return CreateCompiler<parser::JSParser, varbinder::JSBinder, checker::JSChecker, checker::TSAnalyzer,
                                  compiler::JSCompiler, compiler::PandaGen, compiler::DynamicRegSpiller,
                                  compiler::JSFunctionEmitter, compiler::JSEmitter>(unit, compiler::GetTrivialPhaseList,
                                                                                    emit_cb);
        }
        default: {
            UNREACHABLE();
            return nullptr;
        }
    }
}

void CompilerImpl::DumpAsm(const panda::pandasm::Program *prog)
{
    Emitter::DumpAsm(prog);
}
}  // namespace panda::es2panda::compiler
