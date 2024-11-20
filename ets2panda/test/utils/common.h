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

#ifndef ES2PANDA_TEST_UTILS_COMMON_H
#define ES2PANDA_TEST_UTILS_COMMON_H

#include "ast_verifier/ASTVerifier.h"
#include "assembler/assembly-program.h"
#include "util/options.h"
#include "compiler/core/compilerImpl.h"
#include "compiler/lowering/phase.h"
#include "checker/ETSchecker.h"
#include "panda_executable_path_getter.h"
#include "compiler/core/regSpiller.h"
#include "compiler/core/ETSemitter.h"
#include "checker/ETSAnalyzer.h"

#include <gtest/gtest.h>

namespace ir_alias = ark::es2panda::ir;
namespace checker_alias = ark::es2panda::checker;
namespace varbinder_alias = ark::es2panda::varbinder;
namespace util_alias = ark::es2panda::util;
namespace verifier_alias = ark::es2panda::compiler::ast_verifier;
namespace plib_alias = ark::es2panda::public_lib;
namespace parser_alias = ark::es2panda::parser;
namespace compiler_alias = ark::es2panda::compiler;

using AnnotationMap = std::map<std::string, std::vector<std::pair<std::string, std::string>>>;
using AnnotationValueType = std::variant<bool, uint8_t, uint16_t, uint32_t, uint64_t, float, double, std::string>;

namespace test::utils {

class ScopeInitTest : public testing::Test {
public:
    /*
     * Shortcut to convert single elemnt block expression body to it's name
     * Example: { let x; } => x
     */
    ir_alias::Identifier *BodyToFirstName(ir_alias::Statement *body);
};

class CheckerTest : public ScopeInitTest {
public:
    checker_alias::Type *FindClassType(varbinder_alias::ETSBinder *varbinder, std::string_view className);

    checker_alias::Type *FindTypeAlias(checker_alias::ETSChecker *checker, std::string_view aliasName);

    es2panda_Context *CreateContextAndProceedToState(const es2panda_Impl *impl, es2panda_Config *config,
                                                     char const *source, char const *fileName,
                                                     es2panda_ContextState state);

    verifier_alias::Messages VerifyCheck(verifier_alias::ASTVerifier &verifier, const ir_alias::AstNode *ast,
                                         const std::string &check, verifier_alias::InvariantNameSet &checks);

    verifier_alias::Messages VerifyCheck(verifier_alias::ASTVerifier &verifier, const ir_alias::AstNode *ast,
                                         const std::string &check);

    template <typename Ast>
    Ast *GetAstFromContext(const es2panda_Impl *impl, es2panda_Context *ctx)
    {
        auto ast = reinterpret_cast<Ast *>(impl->ProgramAst(impl->ContextProgram(ctx)));
        return ast;
    }
};

class CheckerInitTest : public CheckerTest {
public:
    CheckerInitTest()
        : allocator_(std::make_unique<ark::ArenaAllocator>(ark::SpaceType::SPACE_TYPE_COMPILER)),
          publicContext_ {std::make_unique<plib_alias::Context>()},
          program_ {parser_alias::Program::NewProgram<varbinder_alias::ETSBinder>(allocator_.get())},
          es2pandaPath_ {PandaExecutablePathGetter {}.Get()}
    {
    }

    checker_alias::ETSChecker *Checker()
    {
        return &checker_;
    }

    ark::ArenaAllocator *Allocator()
    {
        return allocator_.get();
    }

    parser_alias::Program *Program()
    {
        return &program_;
    }

    void InitializeChecker(std::string_view fileName, std::string_view src)
    {
        auto es2pandaPathPtr = es2pandaPath_.c_str();
        ASSERT(es2pandaPathPtr);

        InitializeChecker<parser_alias::ETSParser, varbinder_alias::ETSBinder, checker_alias::ETSChecker,
                          checker_alias::ETSAnalyzer, compiler_alias::ETSCompiler, compiler_alias::ETSGen,
                          compiler_alias::StaticRegSpiller, compiler_alias::ETSFunctionEmitter,
                          compiler_alias::ETSEmitter>(&es2pandaPathPtr, fileName, src, &checker_, &program_);
    }

    template <typename Parser, typename VarBinder, typename Checker, typename Analyzer, typename AstCompiler,
              typename CodeGen, typename RegSpiller, typename FunctionEmitter, typename Emitter>
    void InitializeChecker(const char **argv, std::string_view fileName, std::string_view src,
                           checker_alias::ETSChecker *checker, parser_alias::Program *program)
    {
        auto options = std::make_unique<ark::es2panda::util::Options>();
        if (!options->Parse(1, argv)) {
            std::cerr << options->ErrorMsg() << std::endl;
            return;
        }

        ark::Logger::ComponentMask mask {};
        mask.set(ark::Logger::Component::ES2PANDA);
        ark::Logger::InitializeStdLogging(ark::Logger::LevelFromString(options->LogLevel()), mask);

        ark::es2panda::Compiler compiler(options->Extension(), options->ThreadCount());
        ark::es2panda::SourceFile input(fileName, src, options->ParseModule());
        compiler_alias::CompilationUnit unit {input, *options, 0, options->Extension()};
        auto getPhases = compiler_alias::GetPhaseList(ark::es2panda::ScriptExtension::ETS);

        program->MarkEntry();
        auto parser = Parser(program, unit.options.CompilerOptions(),
                             static_cast<parser_alias::ParserStatus>(unit.rawParserStatus));
        auto analyzer = Analyzer(checker);
        checker->SetAnalyzer(&analyzer);

        auto *varbinder = program->VarBinder();
        varbinder->SetProgram(program);

        varbinder->SetContext(publicContext_.get());

        auto emitter = Emitter(publicContext_.get());

        auto config = plib_alias::ConfigImpl {};
        publicContext_->config = &config;
        publicContext_->config->options = &unit.options;
        publicContext_->sourceFile = &unit.input;
        publicContext_->allocator = allocator_.get();
        publicContext_->parser = &parser;
        publicContext_->checker = checker;
        publicContext_->analyzer = publicContext_->checker->GetAnalyzer();
        publicContext_->emitter = &emitter;
        publicContext_->parserProgram = program;

        parser.ParseScript(unit.input, unit.options.CompilerOptions().compilationMode ==
                                           ark::es2panda::CompilationMode::GEN_STD_LIB);
        for (auto *phase : getPhases) {
            if (!phase->Apply(publicContext_.get(), program)) {
                return;
            }
        }
    }

private:
    std::unique_ptr<ark::ArenaAllocator> allocator_;
    std::unique_ptr<plib_alias::Context> publicContext_;
    parser_alias::Program program_;
    std::string es2pandaPath_;
    checker_alias::ETSChecker checker_;
};

class AsmTest : public CheckerTest {
public:
    static std::unique_ptr<ark::pandasm::Program> GetProgram(int argc, const char **argv, std::string_view fileName,
                                                             std::string_view src)
    {
        auto options = std::make_unique<util_alias::Options>();
        if (!options->Parse(argc, argv)) {
            std::cerr << options->ErrorMsg() << std::endl;
            return nullptr;
        }

        ark::Logger::ComponentMask mask {};
        mask.set(ark::Logger::Component::ES2PANDA);
        ark::Logger::InitializeStdLogging(ark::Logger::LevelFromString(options->LogLevel()), mask);

        ark::es2panda::Compiler compiler(options->Extension(), options->ThreadCount());
        ark::es2panda::SourceFile input(fileName, src, options->ParseModule());

        return std::unique_ptr<ark::pandasm::Program>(compiler.Compile(input, *options));
    }

    ark::pandasm::Function *GetFunction(std::string_view functionName,
                                        const std::unique_ptr<ark::pandasm::Program> &program);

    void CompareActualWithExpected(const std::string &expectedValue, ark::pandasm::ScalarValue *scalarValue,
                                   const std::string &field);

    void CheckAnnoDecl(ark::pandasm::Program *program, const std::string &annoName,
                       const std::vector<std::pair<std::string, std::string>> &expectedAnnotations);

    void CheckLiteralArrayTable(
        ark::pandasm::Program *program,
        const std::vector<std::pair<std::string, std::vector<AnnotationValueType>>> &expectedLiteralArrayTable);

    void CheckAnnotation(const std::vector<std::pair<std::string, std::string>> &expectedValues,
                         const ark::pandasm::AnnotationData &annotation);

    void CheckClassAnnotations(ark::pandasm::Program *program, const std::string &className,
                               const AnnotationMap &expectedAnnotations);

    void CheckFunctionAnnotations(ark::pandasm::Program *program, const std::string &functionName,
                                  const AnnotationMap &expectedAnnotations);
};

}  // namespace test::utils

#endif  // ES2PANDA_TEST_UTILS_COMMON_H
