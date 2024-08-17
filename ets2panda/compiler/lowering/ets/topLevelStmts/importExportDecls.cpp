/*
 * Copyright (c) 2023 - 2024 Huawei Device Co., Ltd.
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

#include "compiler/lowering/ets/topLevelStmts/importExportDecls.h"
#include "ir/ets/etsReExportDeclaration.h"
#include "parser/program/program.h"
#include "util/importPathManager.h"

namespace ark::es2panda::compiler {

static bool ProgramFileNameLessThan(const parser::Program *a, const parser::Program *b)
{
    return a->FileName().Mutf8() < b->FileName().Mutf8();
}

void ImportExportDecls::ParseDefaultSources()
{
    auto imports = parser_->ParseDefaultSources(DEFAULT_IMPORT_SOURCE_FILE, defaultImportSource_);
    varbinder_->SetDefaultImports(std::move(imports));
}

/**
 * @brief checks if `prog` matches with the path in `stmt` (an ImportDeclaration statement)

 * If match is found, `prog` is inserted into a container (`triggeringCCtorMethodsAndPrograms`) that will be used when
 * calling the  triggeringCCtorMethod inside the entrypoint's `_$init$_`.
 *
 * The structure of `triggeringCCtorMethodsAndPrograms` is delineated in the `CollectImportedProgramsFromStmts` method.

 * @return true if `prog` is part of a package
 * @return false otherwise
 */
bool ImportExportDecls::MatchImportDeclarationPathWithProgram(
    ark::es2panda::ir::ETSImportDeclaration *stmt, parser::Program *prog, std::string_view resolvedImportPath,
    const util::StringView moduleName, const GlobalClassHandler *globalClass,
    GlobalClassHandler::TriggeringCCtorMethodsAndPrograms *triggeringCCtorMethodsAndPrograms)
{
    ASSERT(stmt->IsETSImportDeclaration());

    parser::Program *importedProgram = nullptr;
    if (!prog->IsStdLib()) {
        if ((prog->IsPackageModule() && (prog->SourceFileFolder().Is(resolvedImportPath))) ||
            (prog->AbsoluteName().Is(resolvedImportPath))) {
            importedProgram = prog;
        }
        if (importedProgram != nullptr && !importedProgram->IsDeclarationModule()) {
            if (triggeringCCtorMethodsAndPrograms->count(moduleName) == 0) {
                triggeringCCtorMethodsAndPrograms->emplace(moduleName, prog->Allocator()->Adapter());
            }
            auto triggeringCCtorMethodName =
                globalClass->FormTriggeringCCtorMethodName(stmt->AsETSImportDeclaration()->Source()->Str());
            triggeringCCtorMethodsAndPrograms->at(moduleName)
                .emplace_back(std::pair(triggeringCCtorMethodName, importedProgram));
            AddImportSpecifierForTriggeringCCtorMethod(stmt, globalClass, triggeringCCtorMethodName);
        }
        if (prog->IsPackageModule()) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Collects imported programs and local names for the functions that are to trigger the owner classes of the
 * imported programs
 *
 * The function to trigger the CCtor in the imported program is named `_$trigger_cctor$_`. A local
 * alias is assigned to this function. The `_$init$_` of the entrypoint program's owner class will use this alias
 * when calling the special function.
 *
 * The alias for the special function and the program is stored in std::pairs. The pairs that belong to the same
 * package form a vector. The vectors are stored in an unordered map, the key being the name of the package name.
 */
void ImportExportDecls::CollectImportedProgramsFromStmts(
    ark::es2panda::ir::ETSImportDeclaration *stmt, parser::Program *program, const GlobalClassHandler *globalClass,
    GlobalClassHandler::TriggeringCCtorMethodsAndPrograms *triggeringCCtorMethodsAndPrograms)
{
    auto resolvedImportPath = stmt->AsETSImportDeclaration()->ResolvedSource()->Str().Utf8();

    for (auto const &directExtSource : program->DirectExternalSources()) {
        for (auto prog : directExtSource.second) {
            bool importedWithinAPackage = MatchImportDeclarationPathWithProgram(
                stmt, prog, resolvedImportPath, directExtSource.first, globalClass, triggeringCCtorMethodsAndPrograms);
            if (importedWithinAPackage) {
                break;
            }
        }
    }
}

/**
 * @brief  Adds specifier of the TriggeringCCtorMethod to the import declaration `stmt`.
 *
 * Adds specifier of the TriggeringCCtorMethod to the import declaration (`stmt`). The specifier have a different
 * local name, to avoid collision with the `_$trigger_cctor$_` in different modules.
 */
void ImportExportDecls::AddImportSpecifierForTriggeringCCtorMethod(ark::es2panda::ir::ETSImportDeclaration *stmt,
                                                                   const GlobalClassHandler *globalClass,
                                                                   util::StringView triggeringCCtorMethodName)
{
    ir::ImportSpecifier *specifier =
        parser_->AsETSParser()->GetTriggeringCCTORSpecifier(triggeringCCtorMethodName, globalClass->TRIGGER_CCTOR);
    stmt->Specifiers().push_back(specifier);
    specifier->SetParent(stmt);
}

GlobalClassHandler::TriggeringCCtorMethodsAndPrograms ImportExportDecls::HandleGlobalStmts(
    ArenaVector<parser::Program *> &programs, GlobalClassHandler *globalClass)
{
    VerifySingleExportDefault(programs);
    VerifyTypeExports(programs);
    GlobalClassHandler::TriggeringCCtorMethodsAndPrograms triggeringCCtorMethodsAndPrograms {
        programs.front()->Allocator()->Adapter()};
    if (!programs.empty()) {
        std::sort(programs.begin(), programs.end(), ProgramFileNameLessThan);
    }
    for (const auto &program : programs) {
        fieldMap_.clear();
        exportNameMap_.clear();
        exportedTypes_.clear();
        for (auto stmt : program->Ast()->Statements()) {
            // note (hurton): Current implementation of triggering the imported programs top level statements does
            // not support type imports and re-exports.
            if (stmt->IsETSImportDeclaration() && !stmt->AsETSImportDeclaration()->IsTypeKind() &&
                !program->IsStdLib() && !program->IsDeclarationModule()) {
                CollectImportedProgramsFromStmts(stmt->AsETSImportDeclaration(), program, globalClass,
                                                 &triggeringCCtorMethodsAndPrograms);
            }
            stmt->Accept(this);
        }
        for (auto const &[exportName, startLoc] : exportNameMap_) {
            const bool isType = exportedTypes_.find(exportName) != exportedTypes_.end();
            if ((fieldMap_.count(exportName) == 0 && !isType)) {
                util::ErrorHandler::ThrowSyntaxError(
                    program, "Cannot find name '" + exportName.Mutf8() + "' to export.", startLoc);
            }
            if (!isType) {
                auto field = fieldMap_[exportName];
                field->AddModifier(ir::ModifierFlags::EXPORT);
            }
        }
    }
    return triggeringCCtorMethodsAndPrograms;
}

void ImportExportDecls::VisitFunctionDeclaration(ir::FunctionDeclaration *funcDecl)
{
    auto id = funcDecl->Function()->Id();
    fieldMap_.emplace(id->Name(), funcDecl->Function());
}

void ImportExportDecls::VisitVariableDeclaration(ir::VariableDeclaration *varDecl)
{
    for (const auto &decl : varDecl->Declarators()) {
        auto id = decl->Id()->AsIdentifier();
        fieldMap_.emplace(id->Name(), varDecl);
    }
}

void ImportExportDecls::VisitExportNamedDeclaration(ir::ExportNamedDeclaration *exportDecl)
{
    for (auto spec : exportDecl->Specifiers()) {
        auto local = spec->Local();
        if (exportDecl->IsExportedType()) {
            exportedTypes_.insert(local->Name());
        }
        if (!exportNameMap_.emplace(local->Name(), local->Start()).second) {
            util::ErrorHandler::ThrowSyntaxError(varbinder_->Program(),
                                                 "Cannot redeclare exported variable '" + local->Name().Mutf8() + "'",
                                                 local->Start());
        }
    }
}

void ImportExportDecls::HandleSimpleType(std::set<util::StringView> &exportedTypes,
                                         std::set<util::StringView> &exportedStatements, ir::Statement *stmt,
                                         util::StringView name, parser::Program *program, lexer::SourcePosition pos)
{
    if (stmt->IsExported()) {
        exportedStatements.insert(name);
    }

    if (!stmt->IsExportedType()) {
        return;
    }

    if (exportedStatements.find(name) != exportedStatements.end()) {
        util::ErrorHandler::ThrowSyntaxError(
            program, "Name '" + name.Mutf8() + "' cannot be exported and type exported at the same time.", pos);
    }

    if (exportedTypes.find(name) != exportedTypes.end()) {
        util::ErrorHandler::ThrowSyntaxError(program, "Cannot export the same '" + name.Mutf8() + "' type twice.", pos);
    } else {
        exportedTypes.insert(name);
    }
}

void ImportExportDecls::VerifyTypeExports(const ArenaVector<parser::Program *> &programs)
{
    std::set<util::StringView> exportedTypes;
    std::set<util::StringView> exportedStatements;
    std::map<util::StringView, ir::AstNode *> typesMap;

    for (const auto &program : programs) {
        for (auto stmt : program->Ast()->Statements()) {
            VerifyType(stmt, program, exportedTypes, exportedStatements, typesMap);
        }
    }
}

void ImportExportDecls::VerifyType(ir::Statement *stmt, parser::Program *program,
                                   std::set<util::StringView> &exportedTypes,
                                   std::set<util::StringView> &exportedStatements,
                                   std::map<util::StringView, ir::AstNode *> &typesMap)
{
    if (stmt->IsClassDeclaration()) {
        typesMap.insert({stmt->AsClassDeclaration()->Definition()->Ident()->Name(), stmt});
        return HandleSimpleType(exportedTypes, exportedStatements, stmt,
                                stmt->AsClassDeclaration()->Definition()->Ident()->Name(), program, stmt->Start());
    }

    if (stmt->IsTSInterfaceDeclaration()) {
        typesMap.insert({stmt->AsTSInterfaceDeclaration()->Id()->Name(), stmt});
        return HandleSimpleType(exportedTypes, exportedStatements, stmt, stmt->AsTSInterfaceDeclaration()->Id()->Name(),
                                program, stmt->Start());
    }

    if (stmt->IsTSTypeAliasDeclaration()) {
        typesMap.insert({stmt->AsTSTypeAliasDeclaration()->Id()->Name(), stmt});
        return HandleSimpleType(exportedTypes, exportedStatements, stmt, stmt->AsTSTypeAliasDeclaration()->Id()->Name(),
                                program, stmt->Start());
    }

    if (!stmt->IsExportedType()) {
        return;
    }

    if (!stmt->IsExportNamedDeclaration()) {
        util::ErrorHandler::ThrowSyntaxError(program, "Can only type export class or interface!", stmt->Start());
    }

    for (auto spec : stmt->AsExportNamedDeclaration()->Specifiers()) {
        util::StringView name = spec->Local()->Name();
        util::StringView nameFind = spec->Exported()->Name();

        auto element = typesMap.find(nameFind);
        if (element == typesMap.end()) {
            util::ErrorHandler::ThrowSyntaxError(program, "Can only type export class or interface!",
                                                 spec->Local()->Start());
        }
        if (!element->second->IsExportedType()) {
            element->second->AddModifier(ir::ModifierFlags::EXPORT_TYPE);
        }
        HandleSimpleType(exportedTypes, exportedStatements, stmt, name, program, spec->Local()->Start());
        if (!name.Is(nameFind.Mutf8())) {
            element->second->AddAstNodeFlags(ir::AstNodeFlags::HAS_EXPORT_ALIAS);
            varbinder_->AddExportSelectiveAlias(program->SourceFilePath(), name, nameFind);
            HandleSimpleType(exportedTypes, exportedStatements, stmt, nameFind, program, spec->Local()->Start());
        }
    }
}

void ImportExportDecls::VerifySingleExportDefault(const ArenaVector<parser::Program *> &programs)
{
    bool metDefaultExport = false;
    auto verifyDefault = [&metDefaultExport](ir::Statement *stmt, parser::Program *program) {
        if ((stmt->Modifiers() & ir::ModifierFlags::DEFAULT_EXPORT) == 0) {
            return;
        }
        if (metDefaultExport) {
            util::ErrorHandler::ThrowSyntaxError(program, "Only one default export is allowed in a module",
                                                 stmt->Start());
        }
        metDefaultExport = true;
    };
    for (const auto &program : programs) {
        for (auto stmt : program->Ast()->Statements()) {
            verifyDefault(stmt, program);
        }
        metDefaultExport = false;
    }
}

}  // namespace ark::es2panda::compiler
