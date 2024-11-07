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

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include "public/es2panda_lib.h"
#include "os/library_loader.h"

// NOLINTBEGIN

static const char *LIBNAME = "es2panda-public";
static const int MIN_ARGC = 3;
static const int NULLPTR_IMPL_ERROR_CODE = 2;

static es2panda_Impl *impl = nullptr;

es2panda_Impl *GetImpl()
{
    if (impl != nullptr) {
        return impl;
    }

    std::string soName = ark::os::library_loader::DYNAMIC_LIBRARY_PREFIX + std::string(LIBNAME) +
                         ark::os::library_loader::DYNAMIC_LIBRARY_SUFFIX;
    auto libraryRes = ark::os::library_loader::Load(soName);
    if (!libraryRes.HasValue()) {
        std::cout << "Error in load lib" << std::endl;
        return nullptr;
    }

    auto library = std::move(libraryRes.Value());
    auto getImpl = ark::os::library_loader::ResolveSymbol(library, "es2panda_GetImpl");
    if (!getImpl.HasValue()) {
        std::cout << "Error in load func get impl" << std::endl;
        return nullptr;
    }

    auto getImplFunc = reinterpret_cast<const es2panda_Impl *(*)(int)>(getImpl.Value());
    if (getImplFunc != nullptr) {
        return const_cast<es2panda_Impl *>(getImplFunc(ES2PANDA_LIB_VERSION));
    }
    return nullptr;
}

static auto source = std::string("function main() { \nlet a = 5;\n assert(a == 5);\n  }");

es2panda_AstNode *parNode;
es2panda_Context *newCtx;

static void changeParent(es2panda_AstNode *child)
{
    impl->AstNodeSetParent(newCtx, child, parNode);
}

static void SetRightParent(es2panda_AstNode *node, void *arg)
{
    es2panda_Context *ctx = static_cast<es2panda_Context *>(arg);
    newCtx = ctx;
    parNode = node;
    impl->AstNodeIterateConst(ctx, node, changeParent);
}

void CheckForErrors(std::string StateName, es2panda_Context *context)
{
    if (impl->ContextState(context) == ES2PANDA_STATE_ERROR) {
        std::cout << "PROCEED TO " << StateName << " ERROR" << std::endl;
        std::cout << impl->ContextErrorMessage << std::endl;
    } else {
        std::cout << "PROCEED TO " << StateName << " SUCCESS" << std::endl;
    }
}

void ChangeAst(es2panda_Context *context)
{
    auto Ast = impl->ProgramAst(impl->ContextProgram(context));
    std::cout << impl->AstNodeDumpJSONConst(context, Ast) << std::endl;
    size_t n = 0;
    auto statements = impl->BlockStatementStatements(context, Ast, &n);
    auto classDef = impl->ClassDeclarationDefinition(context, statements[0]);
    auto mainDecl = impl->ClassDefinitionBody(context, classDef, &n)[1];
    auto mainFunc = impl->MethodDefinitionFunction(context, mainDecl);
    auto mainFuncBody = impl->ScriptFunctionBody(context, mainFunc);
    std::cout << impl->AstNodeDumpJSONConst(context, mainFunc) << std::endl;
    auto mainStatements = impl->BlockStatementStatements(context, mainFuncBody, &n);
    auto letStatement = mainStatements[0];
    auto assertStatementTest = impl->AssertStatementTest(context, mainStatements[1]);
    std::cout << impl->AstNodeDumpJSONConst(context, letStatement) << std::endl;
    std::cout << impl->AstNodeDumpJSONConst(context, assertStatementTest) << std::endl;

    std::string className = std::string("b");
    auto *memForName = static_cast<char *>(impl->AllocMemory(context, className.size() + 1, 1));
    std::copy_n(className.c_str(), className.size() + 1, memForName);

    auto varIdent = impl->CreateIdentifier1(context, memForName);
    auto assertIdent = impl->CreateIdentifier1(context, memForName);
    auto declarator = impl->CreateVariableDeclarator1(
        context, Es2pandaVariableDeclaratorFlag::VARIABLE_DECLARATOR_FLAG_LET, varIdent,
        impl->VariableDeclaratorInit(context, impl->VariableDeclarationDeclaratorsConst(context, letStatement, &n)[0]));
    auto declaration = impl->CreateVariableDeclaration(
        context, Es2pandaVariableDeclarationKind::VARIABLE_DECLARATION_KIND_LET, &declarator, 1);

    impl->BinaryExpressionSetLeft(context, assertStatementTest, assertIdent);
    auto assertStatement = impl->CreateAssertStatement(context, assertStatementTest, nullptr);

    es2panda_AstNode *newMainStatements[2] = {declaration, assertStatement};
    impl->BlockStatementSetStatements(context, mainFuncBody, newMainStatements, 2U);
    impl->AstNodeForEach(Ast, SetRightParent, context);
    std::cout << impl->AstNodeDumpJSONConst(context, Ast) << std::endl;

    impl->AstNodeRecheck(context, declaration);
    impl->AstNodeRecheck(context, assertStatement);

    std::cout << impl->AstNodeDumpEtsSrcConst(context, Ast) << std::endl;
}

int main(int argc, char **argv)
{
    if (argc < MIN_ARGC) {
        return 1;
    }

    if (GetImpl() == nullptr) {
        return NULLPTR_IMPL_ERROR_CODE;
    }
    impl = GetImpl();
    std::cout << "LOAD SUCCESS" << std::endl;
    const char **args = const_cast<const char **>(&(argv[1]));
    auto config = impl->CreateConfig(argc - 1, args);
    auto context = impl->CreateContextFromString(config, source.data(), argv[argc - 1]);
    if (context != nullptr) {
        std::cout << "CREATE CONTEXT SUCCESS" << std::endl;
    }

    impl->ProceedToState(context, ES2PANDA_STATE_PARSED);
    CheckForErrors("PARSE", context);

    impl->ProceedToState(context, ES2PANDA_STATE_SCOPE_INITED);
    CheckForErrors("SCOPE INITED", context);

    impl->ProceedToState(context, ES2PANDA_STATE_CHECKED);
    CheckForErrors("CHECKED", context);

    impl->ProceedToState(context, ES2PANDA_STATE_LOWERED);
    CheckForErrors("LOWERED", context);

    ChangeAst(context);

    impl->ProceedToState(context, ES2PANDA_STATE_ASM_GENERATED);
    CheckForErrors("ASM", context);

    impl->ProceedToState(context, ES2PANDA_STATE_BIN_GENERATED);
    CheckForErrors("BIN", context);
    impl->DestroyConfig(config);

    return 0;
}

// NOLINTEND
