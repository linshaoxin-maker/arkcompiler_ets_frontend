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
#include <algorithm>

#include "checker/ETSAnalyzer.h"
#include "checker/ETSchecker.h"
#include "compiler/core/compilerImpl.h"
#include "compiler/core/ETSCompiler.h"
#include "compiler/core/ETSemitter.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/regSpiller.h"
#include "compiler/lowering/phase.h"
#include "es2panda.h"
#include "mem/arena_allocator.h"
#include "mem/pool_manager.h"
#include "public/public.h"
#include "util/arktsconfig.h"
#include "util/generateBin.h"
#include "varbinder/ETSBinder.h"
#include "test/utils/panda_executable_path_getter.h"
#include "checker/types/globalTypesHolder.h"

namespace ark::es2panda {

class UnionNormalizationTest : public testing::Test {
public:
    UnionNormalizationTest()
        : allocator_(std::make_unique<ArenaAllocator>(SpaceType::SPACE_TYPE_COMPILER)),
          publicContext_ {std::make_unique<public_lib::Context>()},
          program_ {parser::Program::NewProgram<varbinder::ETSBinder>(Allocator())},
          es2pandaPath_ {test::utils::PandaExecutablePathGetter {}.Get()}
    {
    }

    ~UnionNormalizationTest() override = default;

    static void SetUpTestCase()
    {
        constexpr auto COMPILER_SIZE = operator""_MB(256ULL);
        mem::MemConfig::Initialize(0, 0, COMPILER_SIZE, 0, 0, 0);
        PoolManager::Initialize();
    }

    ArenaAllocator *Allocator()
    {
        return allocator_.get();
    }

    parser::Program *Program()
    {
        return &program_;
    }

    checker::ETSChecker *Checker()
    {
        return &checker_;
    }

    void InitializeChecker(std::string_view fileName, std::string_view src)
    {
        auto es2pandaPathPtr = es2pandaPath_.c_str();
        ASSERT(es2pandaPathPtr);

        InitializeChecker<parser::ETSParser, varbinder::ETSBinder, checker::ETSChecker, checker::ETSAnalyzer,
                          compiler::ETSCompiler, compiler::ETSGen, compiler::StaticRegSpiller,
                          compiler::ETSFunctionEmitter, compiler::ETSEmitter>(&es2pandaPathPtr, fileName, src,
                                                                              &checker_, &program_);
    }

    template <typename CodeGen, typename RegSpiller, typename FunctionEmitter, typename Emitter, typename AstCompiler>
    public_lib::Context::CodeGenCb MakeCompileJob()
    {
        return [this](public_lib::Context *context, varbinder::FunctionScope *scope,
                      compiler::ProgramElement *programElement) -> void {
            RegSpiller regSpiller;
            AstCompiler astcompiler;
            CodeGen cg(allocator_.get(), &regSpiller, context, scope, programElement, &astcompiler);
            FunctionEmitter funcEmitter(&cg, programElement);
            funcEmitter.Generate();
        };
    }

    template <typename Parser, typename VarBinder, typename Checker, typename Analyzer, typename AstCompiler,
              typename CodeGen, typename RegSpiller, typename FunctionEmitter, typename Emitter>
    void InitializeChecker(const char **argv, std::string_view fileName, std::string_view src,
                           checker::ETSChecker *checker, parser::Program *program)
    {
        auto options = std::make_unique<ark::es2panda::util::Options>();
        if (!options->Parse(1, argv)) {
            std::cerr << options->ErrorMsg() << std::endl;
            return;
        }

        ark::Logger::ComponentMask mask {};
        mask.set(ark::Logger::Component::ES2PANDA);
        ark::Logger::InitializeStdLogging(ark::Logger::LevelFromString(options->LogLevel()), mask);

        Compiler compiler(options->Extension(), options->ThreadCount());
        SourceFile input(fileName, src, options->ParseModule());
        compiler::CompilationUnit unit {input, *options, 0, options->Extension()};
        auto getPhases = compiler::GetPhaseList(ScriptExtension::ETS);

        program->MarkEntry();
        auto parser =
            Parser(program, unit.options.CompilerOptions(), static_cast<parser::ParserStatus>(unit.rawParserStatus));
        auto analyzer = Analyzer(checker);
        checker->SetAnalyzer(&analyzer);

        auto *varbinder = program->VarBinder();
        varbinder->SetProgram(program);

        varbinder->SetContext(publicContext_.get());

        auto emitter = Emitter(publicContext_.get());

        auto config = public_lib::ConfigImpl {};
        publicContext_->config = &config;
        publicContext_->config->options = &unit.options;
        publicContext_->sourceFile = &unit.input;
        publicContext_->allocator = allocator_.get();
        publicContext_->parser = &parser;
        publicContext_->checker = checker;
        publicContext_->analyzer = publicContext_->checker->GetAnalyzer();
        publicContext_->emitter = &emitter;
        publicContext_->parserProgram = program;

        parser.ParseScript(unit.input, unit.options.CompilerOptions().compilationMode == CompilationMode::GEN_STD_LIB);
        for (auto *phase : getPhases) {
            if (!phase->Apply(publicContext_.get(), program)) {
                return;
            }
        }
    }

    static checker::Type *FindClassType(varbinder::ETSBinder *varbinder, std::string_view className)
    {
        auto classDefs = varbinder->AsETSBinder()->GetRecordTable()->ClassDefinitions();
        auto baseClass = std::find_if(classDefs.begin(), classDefs.end(), [className](ir::ClassDefinition *cdef) {
            return cdef->Ident()->Name().Is(className);
        });
        if (baseClass == classDefs.end()) {
            return nullptr;
        }
        return (*baseClass)->TsType();
    }

    static checker::Type *FindTypeAlias(checker::ETSChecker *checker, std::string_view aliasName)
    {
        auto *foundVar =
            checker->Scope()->FindLocal(aliasName, varbinder::ResolveBindingOptions::ALL)->AsLocalVariable();
        if (foundVar == nullptr) {
            return nullptr;
        }
        return foundVar->Declaration()->Node()->AsTSTypeAliasDeclaration()->TypeAnnotation()->TsType();
    }

    NO_COPY_SEMANTIC(UnionNormalizationTest);
    NO_MOVE_SEMANTIC(UnionNormalizationTest);

protected:
    static constexpr uint8_t SIZE2 = 2;
    static constexpr uint8_t SIZE3 = 3;
    static constexpr uint8_t SIZE4 = 4;
    static constexpr uint8_t SIZE5 = 5;
    static constexpr uint8_t IDX0 = 0;
    static constexpr uint8_t IDX1 = 1;
    static constexpr uint8_t IDX2 = 2;
    static constexpr uint8_t IDX3 = 3;
    static constexpr uint8_t IDX4 = 4;

private:
    std::unique_ptr<ArenaAllocator> allocator_;
    std::unique_ptr<public_lib::Context> publicContext_;
    parser::Program program_;
    std::string es2pandaPath_;
    checker::ETSChecker checker_;
};

TEST_F(UnionNormalizationTest, UnionWithObject)
{
    InitializeChecker("_.ets", "");

    auto checker = Checker();
    ASSERT(checker);

    auto *syntheticNode = checker->AllocNode<ir::Identifier>("synth", checker->Allocator());
    checker->Relation()->SetNode(syntheticNode);

    // Test normalization: int | Object | string ==> Object
    ArenaVector<checker::Type *> unionConstituents(checker->Allocator()->Adapter());
    unionConstituents.emplace_back(checker->GlobalIntType());
    unionConstituents.emplace_back(checker->GetGlobalTypesHolder()->GlobalETSObjectType());
    unionConstituents.emplace_back(checker->GetGlobalTypesHolder()->GlobalETSStringBuiltinType());

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType = checker->CreateETSUnionType(std::move(unionConstituents));
    ASSERT_NE(normalizedType, nullptr);
    ASSERT_TRUE(normalizedType->IsETSObjectType());
    ASSERT_EQ(normalizedType, checker->GlobalETSObjectType());

    // Test normalization: 1 | Object ==> Object
    ArenaVector<checker::Type *> unionConstituents2(checker->Allocator()->Adapter());
    unionConstituents2.emplace_back(checker->CreateIntType(1));
    unionConstituents2.emplace_back(checker->GetGlobalTypesHolder()->GlobalETSObjectType());

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType2 = checker->CreateETSUnionType(std::move(unionConstituents2));
    ASSERT_NE(normalizedType2, nullptr);
    ASSERT_TRUE(normalizedType2->IsETSObjectType());
    ASSERT_EQ(normalizedType2, checker->GlobalETSObjectType());
}

TEST_F(UnionNormalizationTest, UnionWithIdenticalTypes1)
{
    // Test normalization: number | Base | string | number ==> number | Base | string
    InitializeChecker("_.ets", "class Base {}");

    auto program = Program();
    ASSERT(program);

    auto *const baseType = FindClassType(program->VarBinder()->AsETSBinder(), "Base");
    ASSERT_NE(baseType, nullptr);

    auto checker = Checker();
    ASSERT(checker);

    ArenaVector<checker::Type *> unionConstituents(checker->Allocator()->Adapter());
    unionConstituents.emplace_back(checker->GlobalDoubleType());
    unionConstituents.emplace_back(baseType);
    unionConstituents.emplace_back(checker->GlobalBuiltinETSStringType());
    unionConstituents.emplace_back(checker->GlobalDoubleType());

    auto *syntheticNode = checker->AllocNode<ir::Identifier>("synth", checker->Allocator());
    checker->Relation()->SetNode(syntheticNode);

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType = checker->CreateETSUnionType(std::move(unionConstituents));
    ASSERT_NE(normalizedType, nullptr);
    ASSERT_TRUE(normalizedType->IsETSUnionType());
    auto *const unionType = normalizedType->AsETSUnionType();
    ASSERT_EQ(unionType->ConstituentTypes().size(), SIZE3);
    ASSERT_EQ(unionType->ConstituentTypes().at(IDX0), checker->GetGlobalTypesHolder()->GlobalDoubleBuiltinType());
    ASSERT_EQ(unionType->ConstituentTypes().at(IDX1), baseType);
    ASSERT_EQ(unionType->ConstituentTypes().at(IDX2), checker->GlobalBuiltinETSStringType());
}

TEST_F(UnionNormalizationTest, UnionWithIdenticalTypes2)
{
    // Test normalization: Base | int | Base | double | short | number ==> Base | Int | Double | Short
    InitializeChecker("_.ets", "class Base {}");

    auto program = Program();
    ASSERT(program);

    auto *const baseType = FindClassType(program->VarBinder()->AsETSBinder(), "Base");
    ASSERT_NE(baseType, nullptr);

    auto checker = Checker();
    ASSERT(checker);

    ArenaVector<checker::Type *> unionConstituents(checker->Allocator()->Adapter());
    unionConstituents.emplace_back(baseType);
    unionConstituents.emplace_back(checker->GlobalIntType());
    unionConstituents.emplace_back(baseType);
    unionConstituents.emplace_back(checker->GlobalDoubleType());
    unionConstituents.emplace_back(checker->GlobalShortType());
    unionConstituents.emplace_back(checker->GlobalDoubleType());

    auto *syntheticNode = checker->AllocNode<ir::Identifier>("synth", checker->Allocator());
    checker->Relation()->SetNode(syntheticNode);

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType = checker->CreateETSUnionType(std::move(unionConstituents));
    ASSERT_NE(normalizedType, nullptr);
    ASSERT_TRUE(normalizedType->IsETSUnionType());
    auto *const unionType = normalizedType->AsETSUnionType();
    ASSERT_EQ(unionType->ConstituentTypes().size(), SIZE4);
    ASSERT_EQ(unionType->ConstituentTypes().at(IDX0), baseType);
    ASSERT_EQ(unionType->ConstituentTypes().at(IDX1), checker->GetGlobalTypesHolder()->GlobalIntegerBuiltinType());
    ASSERT_EQ(unionType->ConstituentTypes().at(IDX2), checker->GetGlobalTypesHolder()->GlobalDoubleBuiltinType());
    ASSERT_EQ(unionType->ConstituentTypes().at(IDX3), checker->GetGlobalTypesHolder()->GlobalShortBuiltinType());
}

TEST_F(UnionNormalizationTest, UnionWithNumeric1)
{
    // Test normalization: boolean | int | double | short ==> Boolean | Int | Double | Short
    InitializeChecker("_.ets", "");

    auto checker = Checker();
    ASSERT(checker);

    ArenaVector<checker::Type *> unionConstituents(checker->Allocator()->Adapter());
    unionConstituents.emplace_back(checker->GlobalETSBooleanType());
    unionConstituents.emplace_back(checker->GlobalIntType());
    unionConstituents.emplace_back(checker->GlobalDoubleType());
    unionConstituents.emplace_back(checker->GlobalShortType());

    auto *syntheticNode = checker->AllocNode<ir::Identifier>("synth", checker->Allocator());
    checker->Relation()->SetNode(syntheticNode);

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType = checker->CreateETSUnionType(std::move(unionConstituents));
    ASSERT_NE(normalizedType, nullptr);
    ASSERT_TRUE(normalizedType->IsETSUnionType());
    auto *const unionType = normalizedType->AsETSUnionType();
    ASSERT_EQ(unionType->ConstituentTypes().size(), SIZE4);
    ASSERT_EQ(unionType->ConstituentTypes().at(IDX0), checker->GetGlobalTypesHolder()->GlobalETSBooleanBuiltinType());
    ASSERT_EQ(unionType->ConstituentTypes().at(IDX1), checker->GetGlobalTypesHolder()->GlobalIntegerBuiltinType());
    ASSERT_EQ(unionType->ConstituentTypes().at(IDX2), checker->GetGlobalTypesHolder()->GlobalDoubleBuiltinType());
    ASSERT_EQ(unionType->ConstituentTypes().at(IDX3), checker->GetGlobalTypesHolder()->GlobalShortBuiltinType());
}

TEST_F(UnionNormalizationTest, UnionWithNumeric2)
{
    // Test normalization: string | int | Base | double | short ==> string | Int | Base | Double | Short
    InitializeChecker("_.ets", "class Base {}");

    auto program = Program();
    ASSERT(program);

    auto *const baseType = FindClassType(program->VarBinder()->AsETSBinder(), "Base");
    ASSERT_NE(baseType, nullptr);

    auto checker = Checker();
    ASSERT(checker);

    ArenaVector<checker::Type *> unionConstituents(checker->Allocator()->Adapter());
    unionConstituents.emplace_back(checker->GlobalBuiltinETSStringType());
    unionConstituents.emplace_back(checker->GlobalIntType());
    unionConstituents.emplace_back(baseType);
    unionConstituents.emplace_back(checker->GlobalDoubleType());
    unionConstituents.emplace_back(checker->GlobalShortType());

    auto *syntheticNode = checker->AllocNode<ir::Identifier>("synth", checker->Allocator());
    checker->Relation()->SetNode(syntheticNode);

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType = checker->CreateETSUnionType(std::move(unionConstituents));
    ASSERT_NE(normalizedType, nullptr);
    ASSERT_TRUE(normalizedType->IsETSUnionType());
    auto *const unionType = normalizedType->AsETSUnionType();
    ASSERT_EQ(unionType->ConstituentTypes().size(), SIZE5);
    ASSERT_EQ(unionType->ConstituentTypes().at(IDX0), checker->GlobalBuiltinETSStringType());
    ASSERT_EQ(unionType->ConstituentTypes().at(IDX1), checker->GetGlobalTypesHolder()->GlobalIntegerBuiltinType());
    ASSERT_EQ(unionType->ConstituentTypes().at(IDX2), baseType);
    ASSERT_EQ(unionType->ConstituentTypes().at(IDX3), checker->GetGlobalTypesHolder()->GlobalDoubleBuiltinType());
    ASSERT_EQ(unionType->ConstituentTypes().at(IDX4), checker->GetGlobalTypesHolder()->GlobalShortBuiltinType());
}

TEST_F(UnionNormalizationTest, UnionWithSubTypes)
{
    // Test 4 cases of normalization
    static constexpr std::string_view SRC =
        "\
        class Base {}\
        class Derived1 extends Base {}\
        class Derived2 extends Base {}\
        ";
    InitializeChecker("_.ets", SRC);

    auto program = Program();
    ASSERT(program);

    auto *const baseType = FindClassType(program->VarBinder()->AsETSBinder(), "Base");
    ASSERT_NE(baseType, nullptr);
    auto *const derived1Type = FindClassType(program->VarBinder()->AsETSBinder(), "Derived1");
    ASSERT_NE(derived1Type, nullptr);
    auto *const derived2Type = FindClassType(program->VarBinder()->AsETSBinder(), "Derived2");
    ASSERT_NE(derived2Type, nullptr);

    auto checker = Checker();
    ASSERT(checker);

    // Test normalization: Derived1 | Base ==> Base
    ArenaVector<checker::Type *> unionConstituents1(checker->Allocator()->Adapter());
    unionConstituents1.emplace_back(derived1Type);
    unionConstituents1.emplace_back(baseType);

    auto *syntheticNode = checker->AllocNode<ir::Identifier>("synth", checker->Allocator());
    checker->Relation()->SetNode(syntheticNode);

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType1 = checker->CreateETSUnionType(std::move(unionConstituents1));
    ASSERT_NE(normalizedType1, nullptr);
    ASSERT_TRUE(normalizedType1->IsETSObjectType());
    ASSERT_EQ(normalizedType1, baseType);

    // Test normalization: Base | Derived2 ==> Base
    ArenaVector<checker::Type *> unionConstituents2(checker->Allocator()->Adapter());
    unionConstituents2.emplace_back(baseType);
    unionConstituents2.emplace_back(derived2Type);

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType2 = checker->CreateETSUnionType(std::move(unionConstituents2));
    ASSERT_NE(normalizedType2, nullptr);
    ASSERT_TRUE(normalizedType2->IsETSObjectType());
    ASSERT_EQ(normalizedType2, baseType);

    // Test normalization: Derived1 | Derived2 ==> Derived1 | Derived2
    ArenaVector<checker::Type *> unionConstituents3(checker->Allocator()->Adapter());
    unionConstituents3.emplace_back(derived1Type);
    unionConstituents3.emplace_back(derived2Type);

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType3 = checker->CreateETSUnionType(std::move(unionConstituents3));
    ASSERT_NE(normalizedType3, nullptr);
    auto *const unionType = normalizedType3->AsETSUnionType();
    ASSERT_EQ(unionType->ConstituentTypes().size(), SIZE2);
    ASSERT_EQ(unionType->ConstituentTypes().at(IDX0), derived1Type);
    ASSERT_EQ(unionType->ConstituentTypes().at(IDX1), derived2Type);

    // Test normalization: Derived2 | Base | Derived1 ==> Base
    ArenaVector<checker::Type *> unionConstituents4(checker->Allocator()->Adapter());
    unionConstituents4.emplace_back(derived1Type);
    unionConstituents4.emplace_back(baseType);
    unionConstituents4.emplace_back(derived2Type);

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType4 = checker->CreateETSUnionType(std::move(unionConstituents4));
    ASSERT_NE(normalizedType4, nullptr);
    ASSERT_TRUE(normalizedType4->IsETSObjectType());
    ASSERT_EQ(normalizedType4, baseType);
}

TEST_F(UnionNormalizationTest, UnionLinearization)
{
    // Test 3 cases of normalization
    static constexpr std::string_view SRC =
        "\
        class Base {}\
        class Derived1 extends Base {}\
        class Derived2 extends Base {}\
        type UT = int | string\
        \
        type UT1 = int | (int | string) | number\
        type UT2 = int | UT | number\
        type UT3 = int | (Derived2 | Base) | Derived1 | (string | number | short) | (int | string)\
        ";
    InitializeChecker("_.ets", SRC);

    auto program = Program();
    ASSERT(program);

    auto *varbinder = program->VarBinder()->AsETSBinder();
    auto *const baseType = FindClassType(varbinder, "Base");
    ASSERT_NE(baseType, nullptr);
    auto *const derived1Type = FindClassType(program->VarBinder()->AsETSBinder(), "Derived1");
    ASSERT_NE(derived1Type, nullptr);
    auto *const derived2Type = FindClassType(program->VarBinder()->AsETSBinder(), "Derived2");
    ASSERT_NE(derived2Type, nullptr);

    auto checker = Checker();
    ASSERT(checker);

    auto *syntheticNode = checker->AllocNode<ir::Identifier>("synth", checker->Allocator());
    checker->Relation()->SetNode(syntheticNode);

    // Test normalization: int | (int | string) | number ==> Int | string | Number
    auto *const ut1Type = FindTypeAlias(checker, "UT1");
    ASSERT_NE(ut1Type, nullptr);
    ASSERT_TRUE(ut1Type->IsETSUnionType());
    auto *ut1 = ut1Type->AsETSUnionType();
    ASSERT_EQ(ut1->ConstituentTypes().size(), SIZE3);
    ASSERT_EQ(ut1->ConstituentTypes().at(IDX0), checker->GetGlobalTypesHolder()->GlobalIntegerBuiltinType());
    ASSERT_EQ(ut1->ConstituentTypes().at(IDX1), checker->GetGlobalTypesHolder()->GlobalDoubleBuiltinType());
    ASSERT_EQ(ut1->ConstituentTypes().at(IDX2), checker->GlobalBuiltinETSStringType());

    // Test normalization: int | UT | number ==> Int | string | Number
    auto *const ut2Type = FindTypeAlias(checker, "UT2");
    ASSERT_NE(ut2Type, nullptr);
    ASSERT_TRUE(ut2Type->IsETSUnionType());
    auto *ut2 = ut2Type->AsETSUnionType();
    ASSERT_EQ(ut2->ConstituentTypes().size(), SIZE3);
    ASSERT_EQ(ut2->ConstituentTypes().at(IDX0), checker->GetGlobalTypesHolder()->GlobalIntegerBuiltinType());
    ASSERT_EQ(ut2->ConstituentTypes().at(IDX1), checker->GetGlobalTypesHolder()->GlobalDoubleBuiltinType());
    ASSERT_EQ(ut2->ConstituentTypes().at(IDX2), checker->GlobalBuiltinETSStringType());

    // Test normalization:
    // int | (Derived2 | Base) | Derived1 | (string | number | short) | (int | string) ==>
    // Int | Base | string | Number | Short
    auto *const ut3Type = FindTypeAlias(checker, "UT3");
    ASSERT_NE(ut3Type, nullptr);
    ASSERT_TRUE(ut3Type->IsETSUnionType());
    auto *ut3 = ut3Type->AsETSUnionType();
    ASSERT_EQ(ut3->ConstituentTypes().size(), SIZE5);
    ASSERT_EQ(ut3->ConstituentTypes().at(IDX0), checker->GetGlobalTypesHolder()->GlobalIntegerBuiltinType());
    ASSERT_EQ(ut3->ConstituentTypes().at(IDX1), baseType);
    ASSERT_EQ(ut3->ConstituentTypes().at(IDX2), checker->GlobalBuiltinETSStringType());
    ASSERT_EQ(ut3->ConstituentTypes().at(IDX3), checker->GetGlobalTypesHolder()->GlobalDoubleBuiltinType());
    ASSERT_EQ(ut3->ConstituentTypes().at(IDX4), checker->GetGlobalTypesHolder()->GlobalShortBuiltinType());
}

TEST_F(UnionNormalizationTest, UnionStringLiterals)
{
    InitializeChecker("_.ets", "");

    auto checker = Checker();
    ASSERT(checker);

    // Test normalization: string | "abc" ==> string
    ArenaVector<checker::Type *> unionConstituents1(checker->Allocator()->Adapter());
    unionConstituents1.emplace_back(checker->GlobalBuiltinETSStringType());
    unionConstituents1.emplace_back(checker->CreateETSStringLiteralType("abc"));

    auto *syntheticNode = checker->AllocNode<ir::Identifier>("synth", checker->Allocator());
    checker->Relation()->SetNode(syntheticNode);

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType1 = checker->CreateETSUnionType(std::move(unionConstituents1));
    ASSERT_NE(normalizedType1, nullptr);
    ASSERT_TRUE(normalizedType1->IsETSObjectType());
    ASSERT_EQ(normalizedType1, checker->GlobalBuiltinETSStringType());

    // Test normalization: "abc" | string | string ==> string
    ArenaVector<checker::Type *> unionConstituents2(checker->Allocator()->Adapter());
    unionConstituents2.emplace_back(checker->CreateETSStringLiteralType("abc"));
    unionConstituents2.emplace_back(checker->GlobalBuiltinETSStringType());
    unionConstituents2.emplace_back(checker->GlobalBuiltinETSStringType());

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType2 = checker->CreateETSUnionType(std::move(unionConstituents2));
    ASSERT_NE(normalizedType2, nullptr);
    ASSERT_TRUE(normalizedType2->IsETSObjectType());
    ASSERT_EQ(normalizedType2, checker->GlobalBuiltinETSStringType());

    // Test normalization: number | "abc" | string | "xy" ==> number | string
    ArenaVector<checker::Type *> unionConstituents3(checker->Allocator()->Adapter());
    unionConstituents3.emplace_back(checker->GlobalDoubleType());
    unionConstituents3.emplace_back(checker->CreateETSStringLiteralType("abc"));
    unionConstituents3.emplace_back(checker->GlobalBuiltinETSStringType());
    unionConstituents3.emplace_back(checker->CreateETSStringLiteralType("xy"));

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType3 = checker->CreateETSUnionType(std::move(unionConstituents3));
    ASSERT_NE(normalizedType3, nullptr);
    ASSERT_TRUE(normalizedType3->IsETSUnionType());
    auto *const unionType1 = normalizedType3->AsETSUnionType();
    ASSERT_EQ(unionType1->ConstituentTypes().size(), SIZE2);
    ASSERT_EQ(unionType1->ConstituentTypes().at(IDX0), checker->GetGlobalTypesHolder()->GlobalDoubleBuiltinType());
    ASSERT_EQ(unionType1->ConstituentTypes().at(IDX1), checker->GlobalBuiltinETSStringType());

    // Test normalization: "hello" | "goodbye" ==> "hello" | "goodbye"
    ArenaVector<checker::Type *> unionConstituents4(checker->Allocator()->Adapter());
    unionConstituents4.emplace_back(checker->CreateETSStringLiteralType("hello"));
    unionConstituents4.emplace_back(checker->CreateETSStringLiteralType("goodbye"));

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType4 = checker->CreateETSUnionType(std::move(unionConstituents4));
    ASSERT_NE(normalizedType4, nullptr);
    ASSERT_TRUE(normalizedType4->IsETSUnionType());
    auto *const unionType2 = normalizedType4->AsETSUnionType();
    ASSERT_TRUE(unionType2->ConstituentTypes().at(IDX0)->IsETSStringType());
    ASSERT_EQ(unionType2->ConstituentTypes().at(IDX0)->AsETSStringType()->GetValue(), "hello");
    ASSERT_TRUE(unionType2->ConstituentTypes().at(IDX1)->IsETSStringType());
    ASSERT_EQ(unionType2->ConstituentTypes().at(IDX1)->AsETSStringType()->GetValue(), "goodbye");
}

TEST_F(UnionNormalizationTest, UnionWithPrimitives)
{
    InitializeChecker("_.ets", "");

    auto checker = Checker();
    ASSERT(checker);

    // Test normalization: int | short | number ==> number
    ArenaVector<checker::Type *> unionConstituents1(checker->Allocator()->Adapter());
    unionConstituents1.emplace_back(checker->GlobalIntType());
    unionConstituents1.emplace_back(checker->GlobalShortType());
    unionConstituents1.emplace_back(checker->GlobalDoubleType());

    auto *syntheticNode = checker->AllocNode<ir::Identifier>("synth", checker->Allocator());
    checker->Relation()->SetNode(syntheticNode);

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType1 = checker->CreateETSUnionType(std::move(unionConstituents1));
    ASSERT_NE(normalizedType1, nullptr);
    ASSERT_TRUE(normalizedType1->IsDoubleType() && !normalizedType1->IsConstantType());

    // Test normalization: byte | short ==> short
    ArenaVector<checker::Type *> unionConstituents2(checker->Allocator()->Adapter());
    unionConstituents2.emplace_back(checker->GlobalByteType());
    unionConstituents2.emplace_back(checker->GlobalShortType());

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType2 = checker->CreateETSUnionType(std::move(unionConstituents2));
    ASSERT_NE(normalizedType2, nullptr);
    ASSERT_TRUE(normalizedType2->IsShortType() && !normalizedType2->IsConstantType());

    // Test normalization: int | number | float ==> number
    ArenaVector<checker::Type *> unionConstituents3(checker->Allocator()->Adapter());
    unionConstituents3.emplace_back(checker->GlobalIntType());
    unionConstituents3.emplace_back(checker->GlobalDoubleType());
    unionConstituents3.emplace_back(checker->GlobalFloatType());

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType3 = checker->CreateETSUnionType(std::move(unionConstituents3));
    ASSERT_NE(normalizedType3, nullptr);
    ASSERT_TRUE(normalizedType3->IsDoubleType() && !normalizedType3->IsConstantType());

    // Test normalization: int | float | byte ==> float
    ArenaVector<checker::Type *> unionConstituents4(checker->Allocator()->Adapter());
    unionConstituents4.emplace_back(checker->GlobalIntType());
    unionConstituents4.emplace_back(checker->GlobalFloatType());
    unionConstituents4.emplace_back(checker->GlobalByteType());

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType4 = checker->CreateETSUnionType(std::move(unionConstituents4));
    ASSERT_NE(normalizedType4, nullptr);
    ASSERT_TRUE(normalizedType4->IsFloatType() && !normalizedType4->IsConstantType());

    // Test normalization: int | Number ==> Int | Number
    ArenaVector<checker::Type *> unionConstituents5(checker->Allocator()->Adapter());
    unionConstituents5.emplace_back(checker->GlobalIntType());
    unionConstituents5.emplace_back(checker->GetGlobalTypesHolder()->GlobalDoubleBuiltinType());

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType5 = checker->CreateETSUnionType(std::move(unionConstituents5));
    ASSERT_NE(normalizedType5, nullptr);
    ASSERT_TRUE(normalizedType5->IsETSUnionType());
    auto *const unionType5 = normalizedType5->AsETSUnionType();
    ASSERT_EQ(unionType5->ConstituentTypes().size(), SIZE2);
    ASSERT_EQ(unionType5->ConstituentTypes().at(IDX0), checker->GetGlobalTypesHolder()->GlobalIntegerBuiltinType());
    ASSERT_EQ(unionType5->ConstituentTypes().at(IDX1), checker->GetGlobalTypesHolder()->GlobalDoubleBuiltinType());

    // Test normalization: Byte | Int | Long ==> Byte | Int | Long
    ArenaVector<checker::Type *> unionConstituents6(checker->Allocator()->Adapter());
    unionConstituents6.emplace_back(checker->GetGlobalTypesHolder()->GlobalByteBuiltinType());
    unionConstituents6.emplace_back(checker->GetGlobalTypesHolder()->GlobalIntegerBuiltinType());
    unionConstituents6.emplace_back(checker->GetGlobalTypesHolder()->GlobalLongBuiltinType());

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType6 = checker->CreateETSUnionType(std::move(unionConstituents6));
    ASSERT_NE(normalizedType6, nullptr);
    ASSERT_TRUE(normalizedType6->IsETSUnionType());
    auto *const unionType6 = normalizedType6->AsETSUnionType();
    ASSERT_EQ(unionType6->ConstituentTypes().size(), SIZE3);
    ASSERT_EQ(unionType6->ConstituentTypes().at(IDX0), checker->GetGlobalTypesHolder()->GlobalByteBuiltinType());
    ASSERT_EQ(unionType6->ConstituentTypes().at(IDX1), checker->GetGlobalTypesHolder()->GlobalIntegerBuiltinType());
    ASSERT_EQ(unionType6->ConstituentTypes().at(IDX2), checker->GetGlobalTypesHolder()->GlobalLongBuiltinType());
}

TEST_F(UnionNormalizationTest, UnionWithNever)
{
    // Test normalization: int | never | number ==> number
    InitializeChecker("_.ets", "");

    auto checker = Checker();
    ASSERT(checker);

    ArenaVector<checker::Type *> unionConstituents(checker->Allocator()->Adapter());
    unionConstituents.emplace_back(checker->GlobalIntType());
    unionConstituents.emplace_back(checker->GetGlobalTypesHolder()->GlobalBuiltinNeverType());
    unionConstituents.emplace_back(checker->GlobalDoubleType());

    auto *syntheticNode = checker->AllocNode<ir::Identifier>("synth", checker->Allocator());
    checker->Relation()->SetNode(syntheticNode);

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType = checker->CreateETSUnionType(std::move(unionConstituents));
    ASSERT_NE(normalizedType, nullptr);
    ASSERT_TRUE(normalizedType->IsDoubleType() && !normalizedType->IsConstantType());
}

TEST_F(UnionNormalizationTest, UnionWithLiterals1)
{
    // Test normalization: 'x' | string | 'x' | 'y' ==> string
    InitializeChecker("_.ets", "");

    auto checker = Checker();
    ASSERT(checker);

    auto *syntheticNode = checker->AllocNode<ir::Identifier>("synth", checker->Allocator());
    checker->Relation()->SetNode(syntheticNode);

    ArenaVector<checker::Type *> unionConstituents1(checker->Allocator()->Adapter());
    unionConstituents1.emplace_back(checker->CreateETSStringLiteralType("x"));
    unionConstituents1.emplace_back(checker->GlobalBuiltinETSStringType());
    unionConstituents1.emplace_back(checker->CreateETSStringLiteralType("x"));
    unionConstituents1.emplace_back(checker->CreateETSStringLiteralType("y"));

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType1 = checker->CreateETSUnionType(std::move(unionConstituents1));
    ASSERT_NE(normalizedType1, nullptr);
    ASSERT_TRUE(normalizedType1->IsETSObjectType());
    ASSERT_EQ(normalizedType1, checker->GlobalBuiltinETSStringType());

    // Test normalization: string | 11 | true | boolean | 33  ==> string | 11 | boolean | 33
    ArenaVector<checker::Type *> unionConstituents2(checker->Allocator()->Adapter());
    unionConstituents2.emplace_back(checker->GlobalBuiltinETSStringType());
    unionConstituents2.emplace_back(checker->CreateIntType(11));
    unionConstituents2.emplace_back(checker->CreateETSBooleanType(true));
    unionConstituents2.emplace_back(checker->GlobalETSBooleanType());
    unionConstituents2.emplace_back(checker->CreateIntType(33));

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType2 = checker->CreateETSUnionType(std::move(unionConstituents2));
    ASSERT_NE(normalizedType2, nullptr);
    ASSERT_TRUE(normalizedType2->IsETSUnionType());
    auto *const unionType2 = normalizedType2->AsETSUnionType();
    ASSERT_EQ(unionType2->ConstituentTypes().size(), SIZE4);
    ASSERT_EQ(unionType2->ConstituentTypes().at(IDX0), checker->GlobalBuiltinETSStringType());
    ASSERT_TRUE(unionType2->ConstituentTypes().at(IDX1)->IsIntType());
    ASSERT_EQ(unionType2->ConstituentTypes().at(IDX1)->AsIntType()->GetValue(), 11);
    ASSERT_EQ(unionType2->ConstituentTypes().at(IDX2), checker->GetGlobalTypesHolder()->GlobalETSBooleanBuiltinType());
    ASSERT_TRUE(unionType2->ConstituentTypes().at(IDX3)->IsIntType());
    ASSERT_EQ(unionType2->ConstituentTypes().at(IDX3)->AsIntType()->GetValue(), 33);

    // Test normalization: c'X' | c'Y' | char | string  ==> char | string
    ArenaVector<checker::Type *> unionConstituents3(checker->Allocator()->Adapter());
    unionConstituents3.emplace_back(checker->CreateCharType('X'));
    unionConstituents3.emplace_back(checker->CreateCharType('Y'));
    unionConstituents3.emplace_back(checker->GlobalCharType());
    unionConstituents3.emplace_back(checker->GlobalBuiltinETSStringType());

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType3 = checker->CreateETSUnionType(std::move(unionConstituents3));
    ASSERT_NE(normalizedType3, nullptr);
    ASSERT_TRUE(normalizedType3->IsETSUnionType());
    auto *const unionType3 = normalizedType3->AsETSUnionType();
    ASSERT_EQ(unionType3->ConstituentTypes().size(), SIZE2);
    ASSERT_EQ(unionType3->ConstituentTypes().at(IDX0), checker->GetGlobalTypesHolder()->GlobalCharBuiltinType());
    ASSERT_EQ(unionType3->ConstituentTypes().at(IDX1), checker->GlobalBuiltinETSStringType());

    // Test normalization: 325 | 525 | 725  ==> 325 | 525 | 725
    ArenaVector<checker::Type *> unionConstituents4(checker->Allocator()->Adapter());
    unionConstituents4.emplace_back(checker->CreateIntType(325));
    unionConstituents4.emplace_back(checker->CreateIntType(525));
    unionConstituents4.emplace_back(checker->CreateIntType(725));

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType4 = checker->CreateETSUnionType(std::move(unionConstituents4));
    ASSERT_NE(normalizedType4, nullptr);
    ASSERT_TRUE(normalizedType4->IsETSUnionType());
    auto *const unionType4 = normalizedType4->AsETSUnionType();
    ASSERT_TRUE(unionType4->IsNumericUnion());
    ASSERT_EQ(unionType4->ConstituentTypes().size(), SIZE3);
    ASSERT_TRUE(unionType4->ConstituentTypes().at(IDX0)->IsIntType());
    ASSERT_EQ(unionType4->ConstituentTypes().at(IDX0)->AsIntType()->GetValue(), 325);
    ASSERT_TRUE(unionType4->ConstituentTypes().at(IDX1)->IsIntType());
    ASSERT_EQ(unionType4->ConstituentTypes().at(IDX1)->AsIntType()->GetValue(), 525);
    ASSERT_TRUE(unionType4->ConstituentTypes().at(IDX2)->IsIntType());
    ASSERT_EQ(unionType4->ConstituentTypes().at(IDX2)->AsIntType()->GetValue(), 725);

    // Test normalization: number | 0 ==> number
    ArenaVector<checker::Type *> unionConstituents5(checker->Allocator()->Adapter());
    unionConstituents5.emplace_back(checker->GlobalDoubleType());
    unionConstituents5.emplace_back(checker->CreateIntType(0));

    auto *const normalizedType5 = checker->CreateETSUnionType(std::move(unionConstituents5));
    ASSERT_NE(normalizedType5, nullptr);
    ASSERT_TRUE(normalizedType5->IsDoubleType() && !normalizedType5->IsConstantType());

    // Test normalization: short | 2 ==> short
    ArenaVector<checker::Type *> unionConstituents6(checker->Allocator()->Adapter());
    unionConstituents6.emplace_back(checker->GlobalShortType());
    unionConstituents6.emplace_back(checker->CreateIntType(2));

    auto *const normalizedType6 = checker->CreateETSUnionType(std::move(unionConstituents6));
    ASSERT_NE(normalizedType6, nullptr);
    ASSERT_TRUE(normalizedType6->IsShortType() && !normalizedType6->IsConstantType());
}

TEST_F(UnionNormalizationTest, UnionSpecSamplesWithPrimitives)
{
    InitializeChecker("_.ets", "");

    auto checker = Checker();
    ASSERT(checker);

    auto *syntheticNode = checker->AllocNode<ir::Identifier>("synth", checker->Allocator());
    checker->Relation()->SetNode(syntheticNode);

    // Test normalization: 1 | 1 | 1  ==> 1
    ArenaVector<checker::Type *> unionConstituents1(checker->Allocator()->Adapter());
    unionConstituents1.emplace_back(checker->CreateIntType(1));
    unionConstituents1.emplace_back(checker->CreateIntType(1));
    unionConstituents1.emplace_back(checker->CreateIntType(1));

    auto *const normalizedType1 = checker->CreateETSUnionType(std::move(unionConstituents1));
    ASSERT_NE(normalizedType1, nullptr);
    ASSERT_TRUE(normalizedType1->IsIntType() && normalizedType1->IsConstantType());
    ASSERT_EQ(normalizedType1->AsIntType()->GetValue(), 1);

    // Test normalization: number | Number ==> Number
    ArenaVector<checker::Type *> unionConstituents2(checker->Allocator()->Adapter());
    unionConstituents2.emplace_back(checker->GlobalDoubleType());
    unionConstituents2.emplace_back(checker->GetGlobalTypesHolder()->GlobalDoubleBuiltinType());

    auto *const normalizedType2 = checker->CreateETSUnionType(std::move(unionConstituents2));
    ASSERT_NE(normalizedType2, nullptr);
    ASSERT_TRUE(normalizedType2->IsETSObjectType());
    ASSERT_EQ(normalizedType2->AsETSObjectType(), checker->GetGlobalTypesHolder()->GlobalDoubleBuiltinType());

    // Test normalization: Int | float ==> Int | Float
    ArenaVector<checker::Type *> unionConstituents3(checker->Allocator()->Adapter());
    unionConstituents3.emplace_back(checker->GetGlobalTypesHolder()->GlobalIntegerBuiltinType());
    unionConstituents3.emplace_back(checker->GlobalFloatType());

    auto *const normalizedType3 = checker->CreateETSUnionType(std::move(unionConstituents3));
    ASSERT_NE(normalizedType3, nullptr);
    ASSERT_TRUE(normalizedType3->IsETSUnionType());
    auto *const unionType3 = normalizedType3->AsETSUnionType();
    ASSERT_EQ(unionType3->ConstituentTypes().size(), SIZE2);
    ASSERT_EQ(unionType3->ConstituentTypes().at(IDX0), checker->GetGlobalTypesHolder()->GlobalIntegerBuiltinType());
    ASSERT_EQ(unionType3->ConstituentTypes().at(IDX1), checker->GetGlobalTypesHolder()->GlobalFloatBuiltinType());

    // Test normalization: Int | 3.14 ==> Int | 3.14
    ArenaVector<checker::Type *> unionConstituents4(checker->Allocator()->Adapter());
    unionConstituents4.emplace_back(checker->GetGlobalTypesHolder()->GlobalIntegerBuiltinType());
    unionConstituents4.emplace_back(checker->CreateDoubleType(3.14));

    auto *const normalizedType4 = checker->CreateETSUnionType(std::move(unionConstituents4));
    ASSERT_NE(normalizedType4, nullptr);
    ASSERT_TRUE(normalizedType4->IsETSUnionType());
    auto *const unionType4 = normalizedType4->AsETSUnionType();
    ASSERT_EQ(unionType4->ConstituentTypes().size(), SIZE2);
    ASSERT_EQ(unionType4->ConstituentTypes().at(IDX0), checker->GetGlobalTypesHolder()->GlobalIntegerBuiltinType());
    ASSERT_TRUE(unionType4->ConstituentTypes().at(IDX1)->IsDoubleType());
    ASSERT_EQ(unionType4->ConstituentTypes().at(IDX1)->AsDoubleType()->GetValue(), 3.14);

    // Test normalization: int | short | float | 2 ==> float
    ArenaVector<checker::Type *> unionConstituents5(checker->Allocator()->Adapter());
    unionConstituents5.emplace_back(checker->GlobalIntType());
    unionConstituents5.emplace_back(checker->GlobalShortType());
    unionConstituents5.emplace_back(checker->GlobalFloatType());
    unionConstituents5.emplace_back(checker->CreateIntType(2));

    auto *const normalizedType5 = checker->CreateETSUnionType(std::move(unionConstituents5));
    ASSERT_NE(normalizedType5, nullptr);
    ASSERT_TRUE(normalizedType5->IsFloatType() && !normalizedType5->IsConstantType());

    // Test normalization: int | long | 2.71828 ==> long | 2.71828
    ArenaVector<checker::Type *> unionConstituents6(checker->Allocator()->Adapter());
    unionConstituents6.emplace_back(checker->GlobalIntType());
    unionConstituents6.emplace_back(checker->GlobalLongType());
    unionConstituents6.emplace_back(checker->CreateDoubleType(2.71828));

    auto *const normalizedType6 = checker->CreateETSUnionType(std::move(unionConstituents6));
    ASSERT_NE(normalizedType6, nullptr);
    ASSERT_TRUE(normalizedType6->IsETSUnionType());
    auto *const unionType6 = normalizedType6->AsETSUnionType();
    ASSERT_EQ(unionType6->ConstituentTypes().size(), SIZE2);
    ASSERT_TRUE(unionType6->ConstituentTypes().at(IDX0)->IsDoubleType());
    ASSERT_EQ(unionType6->ConstituentTypes().at(IDX0)->AsDoubleType()->GetValue(), 2.71828);
    ASSERT_EQ(unionType6->ConstituentTypes().at(IDX1), checker->GlobalLongType());

    // Test normalization: 1 | number | number ==> number
    ArenaVector<checker::Type *> unionConstituents7(checker->Allocator()->Adapter());
    unionConstituents7.emplace_back(checker->CreateIntType(1));
    unionConstituents7.emplace_back(checker->GlobalDoubleType());
    unionConstituents7.emplace_back(checker->GlobalDoubleType());

    auto *const normalizedType7 = checker->CreateETSUnionType(std::move(unionConstituents7));
    ASSERT_NE(normalizedType7, nullptr);
    ASSERT_TRUE(normalizedType7->IsDoubleType() && !normalizedType7->IsConstantType());

    // Test normalization: Int | 3.14f | Float ==> Int | Float
    ArenaVector<checker::Type *> unionConstituents8(checker->Allocator()->Adapter());
    unionConstituents8.emplace_back(checker->GetGlobalTypesHolder()->GlobalIntegerBuiltinType());
    unionConstituents8.emplace_back(checker->CreateFloatType(3.14));
    unionConstituents8.emplace_back(checker->GetGlobalTypesHolder()->GlobalFloatBuiltinType());

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType8 = checker->CreateETSUnionType(std::move(unionConstituents8));
    ASSERT_NE(normalizedType8, nullptr);
    ASSERT_TRUE(normalizedType8->IsETSUnionType());
    auto *const unionType8 = normalizedType8->AsETSUnionType();
    ASSERT_EQ(unionType8->ConstituentTypes().size(), SIZE2);
    ASSERT_EQ(unionType8->ConstituentTypes().at(IDX0), checker->GetGlobalTypesHolder()->GlobalIntegerBuiltinType());
    ASSERT_EQ(unionType8->ConstituentTypes().at(IDX1), checker->GetGlobalTypesHolder()->GlobalFloatBuiltinType());

    // Test normalization: 1 | string | number ==> string | Number
    ArenaVector<checker::Type *> unionConstituents9(checker->Allocator()->Adapter());
    unionConstituents9.emplace_back(checker->CreateIntType(1));
    unionConstituents9.emplace_back(checker->GetGlobalTypesHolder()->GlobalETSStringBuiltinType());
    unionConstituents9.emplace_back(checker->GetGlobalTypesHolder()->GlobalDoubleBuiltinType());

    // Create union type, which will be normalized inside creation function
    auto *const normalizedType9 = checker->CreateETSUnionType(std::move(unionConstituents9));
    ASSERT_NE(normalizedType9, nullptr);
    ASSERT_TRUE(normalizedType9->IsETSUnionType());
    auto *const unionType9 = normalizedType9->AsETSUnionType();
    ASSERT_EQ(unionType9->ConstituentTypes().size(), SIZE2);
    ASSERT_EQ(unionType9->ConstituentTypes().at(IDX0), checker->GetGlobalTypesHolder()->GlobalETSStringBuiltinType());
    ASSERT_EQ(unionType9->ConstituentTypes().at(IDX1), checker->GetGlobalTypesHolder()->GlobalDoubleBuiltinType());
}

}  // namespace ark::es2panda
