/*
 * Copyright (c) 2021 - 2023 Huawei Device Co., Ltd.
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

#include "phase.h"
#include "checker/checker.h"
#include "compiler/core/ASTVerifier.h"
#include "compiler/core/compilerContext.h"
#include "compiler/lowering/ets/objectIndexAccess.h"
#include "lexer/token/sourceLocation.h"
#include "compiler/lowering/checkerPhase.h"
#include "compiler/lowering/plugin_phase.h"
#include "compiler/lowering/scopesInit/scopesInitPhase.h"
#include "compiler/lowering/ets/expandBrackets.h"
#include "compiler/lowering/ets/generateDeclarations.h"
#include "compiler/lowering/ets/lambdaLowering.h"
#include "compiler/lowering/ets/interfacePropertyDeclarations.h"
#include "compiler/lowering/ets/opAssignment.h"
#include "compiler/lowering/ets/tupleLowering.h"
#include "compiler/lowering/ets/unionLowering.h"
#include "compiler/lowering/ets/structLowering.h"
#include "public/es2panda_lib.h"
#include "compiler/lowering/ets/promiseVoid.h"
#include "utils/json_builder.h"

namespace panda::es2panda::compiler {

static CheckerPhase CHECKER_PHASE;

std::vector<Phase *> GetTrivialPhaseList()
{
    return std::vector<Phase *> {
        &CHECKER_PHASE,
    };
}

static InterfacePropertyDeclarationsPhase INTERFACE_PROP_DECL_PHASE;
static GenerateTsDeclarationsPhase GENERATE_TS_DECLARATIONS_PHASE;
static LambdaConstructionPhase LAMBDA_CONSTRUCTION_PHASE;
static OpAssignmentLowering OP_ASSIGNMENT_LOWERING;
static ObjectIndexLowering OBJECT_INDEX_LOWERING;
static TupleLowering TUPLE_LOWERING;  // Can be only applied after checking phase, and OP_ASSIGNMENT_LOWERING phase
static UnionLowering UNION_LOWERING;
static ExpandBracketsPhase EXPAND_BRACKETS_PHASE;
static PromiseVoidInferencePhase PROMISE_VOID_INFERENCE_PHASE;
static StructLowering STRUCT_LOWERING;
static PluginPhase PLUGINS_AFTER_PARSE {"plugins-after-parse", ES2PANDA_STATE_PARSED, &util::Plugin::AfterParse};
static PluginPhase PLUGINS_AFTER_CHECK {"plugins-after-check", ES2PANDA_STATE_CHECKED, &util::Plugin::AfterCheck};
static PluginPhase PLUGINS_AFTER_LOWERINGS {"plugins-after-lowering", ES2PANDA_STATE_LOWERED,
                                            &util::Plugin::AfterLowerings};
// NOLINTBEGIN(fuchsia-statically-constructed-objects)
static InitScopesPhaseETS INIT_SCOPES_PHASE_ETS;
static InitScopesPhaseAS INIT_SCOPES_PHASE_AS;
static InitScopesPhaseTs INIT_SCOPES_PHASE_TS;
static InitScopesPhaseJs INIT_SCOPES_PHASE_JS;
// NOLINTEND(fuchsia-statically-constructed-objects)

std::vector<Phase *> GetETSPhaseList()
{
    return {
        &PLUGINS_AFTER_PARSE,    &INIT_SCOPES_PHASE_ETS,     &PROMISE_VOID_INFERENCE_PHASE,
        &STRUCT_LOWERING,        &LAMBDA_CONSTRUCTION_PHASE, &INTERFACE_PROP_DECL_PHASE,
        &CHECKER_PHASE,          &PLUGINS_AFTER_CHECK,       &GENERATE_TS_DECLARATIONS_PHASE,
        &OP_ASSIGNMENT_LOWERING, &OBJECT_INDEX_LOWERING,     &TUPLE_LOWERING,
        &UNION_LOWERING,         &EXPAND_BRACKETS_PHASE,     &PLUGINS_AFTER_LOWERINGS,
    };
}

std::vector<Phase *> GetASPhaseList()
{
    return {
        &INIT_SCOPES_PHASE_AS,
        &CHECKER_PHASE,
    };
}

std::vector<Phase *> GetTSPhaseList()
{
    return {
        &INIT_SCOPES_PHASE_TS,
        &CHECKER_PHASE,
    };
}

std::vector<Phase *> GetJSPhaseList()
{
    return {
        &INIT_SCOPES_PHASE_JS,
        &CHECKER_PHASE,
    };
}

std::vector<Phase *> GetPhaseList(ScriptExtension ext)
{
    switch (ext) {
        case ScriptExtension::ETS:
            return GetETSPhaseList();
        case ScriptExtension::AS:
            return GetASPhaseList();
        case ScriptExtension::TS:
            return GetTSPhaseList();
        case ScriptExtension::JS:
            return GetJSPhaseList();
        default:
            UNREACHABLE();
    }
}

bool Phase::Apply(public_lib::Context *ctx, parser::Program *program)
{
    const auto *options = ctx->compiler_context->Options();
    const auto name = std::string {Name()};
    if (options->skip_phases.count(name) > 0) {
        return true;
    }

    CheckOptionsBeforePhase(options, program, name);

#ifndef NDEBUG
    if (!Precondition(ctx, program)) {
        ctx->checker->ThrowTypeError({"Precondition check failed for ", util::StringView {Name()}},
                                     lexer::SourcePosition {});
    }
#endif

    if (!Perform(ctx, program)) {
        return false;
    }

    CheckOptionsAfterPhase(options, program, name);

#ifndef NDEBUG
    if (!Postcondition(ctx, program)) {
        ctx->checker->ThrowTypeError({"Postcondition check failed for ", util::StringView {Name()}},
                                     lexer::SourcePosition {});
    }
#endif

    return true;
}

void Phase::CheckOptionsBeforePhase(const CompilerOptions *options, const parser::Program *program,
                                    const std::string &name) const
{
    if (options->dump_after_phases.count(name) > 0) {
        std::cout << "After phase " << name << ":" << std::endl;
        std::cout << program->Dump() << std::endl;
    }

    if (options->dump_ets_src_after_phases.count(name) > 0) {
        std::cout << "After phase " << name << " ets source"
                  << ":" << std::endl;
        std::cout << program->Ast()->DumpEtsSrc() << std::endl;
    }
}

void Phase::CheckOptionsAfterPhase(const CompilerOptions *options, const parser::Program *program,
                                   const std::string &name) const
{
    if (options->dump_after_phases.count(name) > 0) {
        std::cout << "After phase " << name << ":" << std::endl;
        std::cout << program->Dump() << std::endl;
    }

    if (options->dump_ets_src_after_phases.count(name) > 0) {
        std::cout << "After phase " << name << " ets source"
                  << ":" << std::endl;
        std::cout << program->Ast()->DumpEtsSrc() << std::endl;
    }
}

}  // namespace panda::es2panda::compiler
