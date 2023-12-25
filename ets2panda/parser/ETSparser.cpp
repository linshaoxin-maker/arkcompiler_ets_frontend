/**
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

#include "ETSparser.h"
#include <utility>

#include "macros.h"
#include "parser/parserFlags.h"
#include "util/arktsconfig.h"
#include "util/helpers.h"
#include "util/language.h"
#include "varbinder/varbinder.h"
#include "varbinder/scope.h"
#include "varbinder/ETSBinder.h"
#include "lexer/lexer.h"
#include "lexer/ETSLexer.h"
#include "checker/types/ets/etsEnumType.h"
#include "ir/astNode.h"
#include "ir/base/classDefinition.h"
#include "ir/base/decorator.h"
#include "ir/base/catchClause.h"
#include "ir/base/classProperty.h"
#include "ir/base/scriptFunction.h"
#include "ir/base/methodDefinition.h"
#include "ir/base/classStaticBlock.h"
#include "ir/base/spreadElement.h"
#include "ir/expressions/identifier.h"
#include "ir/expressions/functionExpression.h"
#include "ir/statements/functionDeclaration.h"
#include "ir/statements/expressionStatement.h"
#include "ir/statements/classDeclaration.h"
#include "ir/statements/variableDeclarator.h"
#include "ir/statements/variableDeclaration.h"
#include "ir/expressions/arrayExpression.h"
#include "ir/expressions/assignmentExpression.h"
#include "ir/expressions/sequenceExpression.h"
#include "ir/expressions/callExpression.h"
#include "ir/expressions/blockExpression.h"
#include "ir/expressions/thisExpression.h"
#include "ir/expressions/superExpression.h"
#include "ir/expressions/memberExpression.h"
#include "ir/expressions/updateExpression.h"
#include "ir/expressions/arrowFunctionExpression.h"
#include "ir/expressions/unaryExpression.h"
#include "ir/expressions/yieldExpression.h"
#include "ir/expressions/awaitExpression.h"
#include "ir/expressions/literals/booleanLiteral.h"
#include "ir/expressions/literals/charLiteral.h"
#include "ir/expressions/literals/nullLiteral.h"
#include "ir/expressions/literals/numberLiteral.h"
#include "ir/expressions/literals/stringLiteral.h"
#include "ir/expressions/literals/undefinedLiteral.h"
#include "ir/expressions/templateLiteral.h"
#include "ir/expressions/objectExpression.h"
#include "ir/module/importDeclaration.h"
#include "ir/module/importDefaultSpecifier.h"
#include "ir/module/importSpecifier.h"
#include "ir/statements/assertStatement.h"
#include "ir/statements/blockStatement.h"
#include "ir/statements/emptyStatement.h"
#include "ir/statements/ifStatement.h"
#include "ir/statements/labelledStatement.h"
#include "ir/statements/switchStatement.h"
#include "ir/statements/throwStatement.h"
#include "ir/statements/tryStatement.h"
#include "ir/statements/whileStatement.h"
#include "ir/statements/doWhileStatement.h"
#include "ir/statements/breakStatement.h"
#include "ir/statements/continueStatement.h"
#include "ir/statements/debuggerStatement.h"
#include "ir/ets/etsLaunchExpression.h"
#include "ir/ets/etsClassLiteral.h"
#include "ir/ets/etsPrimitiveType.h"
#include "ir/ets/etsPackageDeclaration.h"
#include "ir/ets/etsReExportDeclaration.h"
#include "ir/ets/etsWildcardType.h"
#include "ir/ets/etsNewArrayInstanceExpression.h"
#include "ir/ets/etsTuple.h"
#include "ir/ets/etsFunctionType.h"
#include "ir/ets/etsNewClassInstanceExpression.h"
#include "ir/ets/etsNewMultiDimArrayInstanceExpression.h"
#include "ir/ets/etsScript.h"
#include "ir/ets/etsTypeReference.h"
#include "ir/ets/etsTypeReferencePart.h"
#include "ir/ets/etsUnionType.h"
#include "ir/ets/etsImportSource.h"
#include "ir/ets/etsImportDeclaration.h"
#include "ir/ets/etsStructDeclaration.h"
#include "ir/ets/etsParameterExpression.h"
#include "ir/module/importNamespaceSpecifier.h"
#include "ir/ts/tsAsExpression.h"
#include "ir/ts/tsInterfaceDeclaration.h"
#include "ir/ts/tsEnumDeclaration.h"
#include "ir/ts/tsTypeParameterInstantiation.h"
#include "ir/ts/tsInterfaceBody.h"
#include "ir/ts/tsImportEqualsDeclaration.h"
#include "ir/ts/tsArrayType.h"
#include "ir/ts/tsQualifiedName.h"
#include "ir/ts/tsTypeReference.h"
#include "ir/ts/tsTypeParameter.h"
#include "ir/ts/tsIntersectionType.h"
#include "ir/ts/tsInterfaceHeritage.h"
#include "ir/ts/tsFunctionType.h"
#include "ir/ts/tsClassImplements.h"
#include "ir/ts/tsEnumMember.h"
#include "ir/ts/tsTypeAliasDeclaration.h"
#include "ir/ts/tsTypeParameterDeclaration.h"
#include "ir/ts/tsNonNullExpression.h"
#include "ir/ts/tsThisType.h"
#include "libpandabase/os/file.h"
#include "generated/signatures.h"

#if defined PANDA_TARGET_MOBILE
#define USE_UNIX_SYSCALL
#endif

#ifdef USE_UNIX_SYSCALL
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#else
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
#endif

namespace panda::es2panda::parser {
using namespace std::literals::string_literals;

std::unique_ptr<lexer::Lexer> ETSParser::InitLexer(const SourceFile &source_file)
{
    GetProgram()->SetSource(source_file);
    auto lexer = std::make_unique<lexer::ETSLexer>(&GetContext());
    SetLexer(lexer.get());
    return lexer;
}

void ETSParser::ParseProgram(ScriptKind kind)
{
    lexer::SourcePosition start_loc = Lexer()->GetToken().Start();
    Lexer()->NextToken();
    GetProgram()->SetKind(kind);

    if (GetProgram()->SourceFilePath().Utf8()[0] == '@') {
        // NOTE(user): handle multiple sourceFiles
    }

    auto statements = PrepareGlobalClass();
    ParseDefaultSources();

    ParseETSGlobalScript(start_loc, statements);
}

void ETSParser::ParseETSGlobalScript(lexer::SourcePosition start_loc, ArenaVector<ir::Statement *> &statements)
{
    auto paths = ParseImportDeclarations(statements);

    auto remove_parsed_sources = [this](std::vector<std::string> &items) {
        items.erase(remove_if(begin(items), end(items),
                              [this](auto x) {
                                  auto resolved = ResolveImportPath(x);
                                  auto path_iter =
                                      std::find_if(resolved_parsed_sources_.begin(), resolved_parsed_sources_.end(),
                                                   [resolved](const auto &p) { return p.second == resolved; });
                                  auto found = path_iter != resolved_parsed_sources_.end();
                                  if (found) {
                                      resolved_parsed_sources_.emplace(x, resolved);
                                  }
                                  return found;
                              }),
                    end(items));

        for (const auto &item : items) {
            parsed_sources_.push_back(ResolveImportPath(item));
        }
    };

    remove_parsed_sources(paths);

    ParseSources(paths, false);

    if (!GetProgram()->VarBinder()->AsETSBinder()->ReExportImports().empty()) {
        std::vector<std::string> re_export_paths;

        for (auto re_export : GetProgram()->VarBinder()->AsETSBinder()->ReExportImports()) {
            if (std::find(paths.begin(), paths.end(), re_export->GetProgramPath().Mutf8()) != paths.end()) {
                auto path = re_export->GetProgramPath().Mutf8().substr(
                    0, re_export->GetProgramPath().Mutf8().find_last_of('/'));
                for (auto item : re_export->GetUserPaths()) {
                    re_export_paths.push_back(
                        path + "/" + item.Mutf8().substr(item.Mutf8().find_first_of('/') + 1, item.Mutf8().length()));
                }
            }
        }

        remove_parsed_sources(re_export_paths);

        ParseSources(re_export_paths, false);
    }

    ParseTopLevelDeclaration(statements);

    auto *ets_script = AllocNode<ir::ETSScript>(Allocator(), std::move(statements), GetProgram());
    ets_script->SetRange({start_loc, Lexer()->GetToken().End()});
    GetProgram()->SetAst(ets_script);
}

void ETSParser::CreateGlobalClass()
{
    auto *ident = AllocNode<ir::Identifier>(compiler::Signatures::ETS_GLOBAL, Allocator());

    auto *class_def = AllocNode<ir::ClassDefinition>(Allocator(), ident, ir::ClassDefinitionModifiers::GLOBAL,
                                                     ir::ModifierFlags::ABSTRACT, Language(Language::Id::ETS));
    GetProgram()->SetGlobalClass(class_def);

    [[maybe_unused]] auto *class_decl = AllocNode<ir::ClassDeclaration>(class_def, Allocator());
}

ArenaVector<ir::Statement *> ETSParser::PrepareGlobalClass()
{
    ArenaVector<ir::Statement *> statements(Allocator()->Adapter());
    ParsePackageDeclaration(statements);
    CreateGlobalClass();

    return statements;
}

ArenaVector<ir::Statement *> ETSParser::PrepareExternalGlobalClass([[maybe_unused]] const SourceFile &source_file)
{
    ArenaVector<ir::Statement *> statements(Allocator()->Adapter());
    ParsePackageDeclaration(statements);

    if (statements.empty()) {
        GetProgram()->SetGlobalClass(global_program_->GlobalClass());
    }

    auto &ext_sources = global_program_->ExternalSources();
    const util::StringView name = GetProgram()->SourceFileFolder();

    auto res = ext_sources.end();
    if (!statements.empty()) {
        res = ext_sources.find(name);
    } else {
        auto path = GetProgram()->SourceFileFolder().Mutf8() + panda::os::file::File::GetPathDelim().at(0) +
                    GetProgram()->GetPackageName().Mutf8();
        auto resolved = ResolveImportPath(path);
        resolved_parsed_sources_.emplace(path, resolved);
        GetProgram()->SetSource(GetProgram()->SourceCode(), GetProgram()->SourceFilePath(),
                                util::UString(resolved, Allocator()).View());
    }

    if (res == ext_sources.end()) {
        CreateGlobalClass();
        auto ins_res = ext_sources.emplace(GetProgram()->SourceFileFolder(), Allocator()->Adapter());
        ins_res.first->second.push_back(GetProgram());
    } else {
        res->second.push_back(GetProgram());
        auto *ext_prog = res->second.front();
        GetProgram()->SetGlobalClass(ext_prog->GlobalClass());
        // NOTE(user): check nullptr cases and handle recursive imports
    }

    return statements;
}

static bool IsCompitableExtension(const std::string &extension)
{
    return extension == ".ets" || extension == ".ts";
}

void ETSParser::CollectDefaultSources()
{
    std::vector<std::string> paths;
    std::vector<std::string> stdlib = {"std/core", "std/math",       "std/containers",
                                       "std/time", "std/interop/js", "escompat"};

#ifdef USE_UNIX_SYSCALL
    for (auto const &path : stdlib) {
        auto resolved_path = ResolveImportPath(path);
        DIR *dir = opendir(resolved_path.c_str());

        if (dir == nullptr) {
            ThrowSyntaxError({"Cannot open folder: ", resolved_path});
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type != DT_REG) {
                continue;
            }

            std::string file_name = entry->d_name;
            std::string::size_type pos = file_name.find_last_of('.');

            if (pos == std::string::npos || !IsCompitableExtension(file_name.substr(pos))) {
                continue;
            }

            std::string file_path = path + "/" + entry->d_name;

            if (file_name == "Object.ets") {
                parsed_sources_.emplace(parsed_sources_.begin(), file_path);
            } else {
                parsed_sources_.emplace_back(file_path);
            }
        }

        closedir(dir);
    }
#else
    for (auto const &path : stdlib) {
        for (auto const &entry : fs::directory_iterator(ResolveImportPath(path))) {
            if (!fs::is_regular_file(entry) || !IsCompitableExtension(entry.path().extension().string())) {
                continue;
            }

            std::string base_name = path;
            std::size_t pos = entry.path().string().find_last_of(panda::os::file::File::GetPathDelim());

            base_name.append(entry.path().string().substr(pos, entry.path().string().size()));

            if (entry.path().filename().string() == "Object.ets") {
                parsed_sources_.emplace(parsed_sources_.begin(), base_name);
            } else {
                parsed_sources_.emplace_back(base_name);
            }
        }
    }
#endif
}

ETSParser::ImportData ETSParser::GetImportData(const std::string &path)
{
    auto &dynamic_paths = ArkTSConfig()->DynamicPaths();
    auto key = panda::os::NormalizePath(path);

    auto it = dynamic_paths.find(key);
    if (it == dynamic_paths.cend()) {
        key = panda::os::RemoveExtension(key);
    }

    while (it == dynamic_paths.cend() && !key.empty()) {
        it = dynamic_paths.find(key);
        if (it != dynamic_paths.cend()) {
            break;
        }
        key = panda::os::GetParentDir(key);
    }

    if (it != dynamic_paths.cend()) {
        return {it->second.GetLanguage(), key, it->second.HasDecl()};
    }
    return {ToLanguage(Extension()), path, true};
}

std::string ETSParser::ResolveFullPathFromRelative(const std::string &path)
{
    char path_delimiter = panda::os::file::File::GetPathDelim().at(0);
    auto resolved_fp = GetProgram()->ResolvedFilePath().Mutf8();
    auto source_fp = GetProgram()->SourceFileFolder().Mutf8();
    if (resolved_fp.empty()) {
        auto fp = source_fp + path_delimiter + path;
        return util::Helpers::IsRealPath(fp) ? fp : path;
    }
    auto fp = resolved_fp + path_delimiter + path;
    if (util::Helpers::IsRealPath(fp)) {
        return fp;
    }
    if (path.find(source_fp) == 0) {
        return resolved_fp + path_delimiter + path.substr(source_fp.size());
    }
    return path;
}

std::string ETSParser::ResolveImportPath(const std::string &path)
{
    char path_delimiter = panda::os::file::File::GetPathDelim().at(0);
    if (util::Helpers::IsRelativePath(path)) {
        return util::Helpers::GetAbsPath(ResolveFullPathFromRelative(path));
    }

    std::string base_url;
    // Resolve delimeter character to basePath.
    if (path.find('/') == 0) {
        base_url = ArkTSConfig()->BaseUrl();

        base_url.append(path, 0, path.length());
        return base_url;
    }

    auto &dynamic_paths = ArkTSConfig()->DynamicPaths();
    auto it = dynamic_paths.find(path);
    if (it != dynamic_paths.cend() && !it->second.HasDecl()) {
        return path;
    }

    // Resolve the root part of the path.
    // E.g. root part of std/math is std.
    std::string::size_type pos = path.find('/');
    bool contains_delim = (pos != std::string::npos);
    std::string root_part = contains_delim ? path.substr(0, pos) : path;

    if (root_part == "std" && !GetOptions().std_lib.empty()) {  // Get std path from CLI if provided
        base_url = GetOptions().std_lib + "/std";
    } else if (root_part == "escompat" && !GetOptions().std_lib.empty()) {  // Get escompat path from CLI if provided
        base_url = GetOptions().std_lib + "/escompat";
    } else {
        auto resolved_path = ArkTSConfig()->ResolvePath(path);
        if (resolved_path.empty()) {
            ThrowSyntaxError({"Can't find prefix for '", path, "' in ", ArkTSConfig()->ConfigPath()});
        }
        return resolved_path;
    }

    if (contains_delim) {
        base_url.append(1, path_delimiter);
        base_url.append(path, root_part.length() + 1, path.length());
    }

    return base_url;
}

std::tuple<std::string, bool> ETSParser::GetSourceRegularPath(const std::string &path, const std::string &resolved_path)
{
    if (!panda::os::file::File::IsRegularFile(resolved_path)) {
        std::string import_extension = ".ets";

        if (!panda::os::file::File::IsRegularFile(resolved_path + import_extension)) {
            import_extension = ".ts";

            if (!panda::os::file::File::IsRegularFile(resolved_path + import_extension)) {
                ThrowSyntaxError("Incorrect path: " + resolved_path);
            }
        }
        return {path + import_extension, true};
    }
    return {path, false};
}

std::tuple<std::vector<std::string>, bool> ETSParser::CollectUserSources(const std::string &path)
{
    std::vector<std::string> user_paths;

    const std::string resolved_path = ResolveImportPath(path);
    resolved_parsed_sources_.emplace(path, resolved_path);
    const auto data = GetImportData(resolved_path);

    if (!data.has_decl) {
        return {user_paths, false};
    }

    if (!panda::os::file::File::IsDirectory(resolved_path)) {
        std::string regular_path;
        bool is_module = false;
        std::tie(regular_path, is_module) = GetSourceRegularPath(path, resolved_path);
        user_paths.emplace_back(regular_path);
        return {user_paths, is_module};
    }

#ifdef USE_UNIX_SYSCALL
    DIR *dir = opendir(resolved_path.c_str());
    bool is_index = false;
    std::vector<std::string> tmp_paths;

    if (dir == nullptr) {
        ThrowSyntaxError({"Cannot open folder: ", resolved_path});
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type != DT_REG) {
            continue;
        }

        std::string file_name = entry->d_name;
        std::string::size_type pos = file_name.find_last_of('.');

        if (pos == std::string::npos || !IsCompitableExtension(file_name.substr(pos))) {
            continue;
        }

        std::string file_path = path + "/" + entry->d_name;

        if (file_name == "index.ets" || file_name == "index.ts") {
            user_paths.emplace_back(file_path);
            is_index = true;
            break;
        } else if (file_name == "Object.ets") {
            tmp_paths.emplace(user_paths.begin(), file_path);
        } else {
            tmp_paths.emplace_back(file_path);
        }
    }

    closedir(dir);

    if (is_index) {
        return {user_paths, false};
    }

    user_paths.insert(user_paths.end(), tmp_paths.begin(), tmp_paths.end());
#else
    if (fs::exists(resolved_path + "/index.ets")) {
        user_paths.emplace_back(path + "/index.ets");
    } else if (fs::exists(resolved_path + "/index.ts")) {
        user_paths.emplace_back(path + "/index.ts");
    } else {
        for (auto const &entry : fs::directory_iterator(resolved_path)) {
            if (!fs::is_regular_file(entry) || !IsCompitableExtension(entry.path().extension().string())) {
                continue;
            }

            std::string base_name = path;
            std::size_t pos = entry.path().string().find_last_of(panda::os::file::File::GetPathDelim());

            base_name.append(entry.path().string().substr(pos, entry.path().string().size()));
            user_paths.emplace_back(base_name);
        }
    }
#endif
    return {user_paths, false};
}

void ETSParser::ParseSources(const std::vector<std::string> &paths, bool is_external)
{
    GetContext().Status() |= is_external ? ParserStatus::IN_EXTERNAL : ParserStatus::IN_IMPORT;

    const std::size_t path_count = paths.size();
    for (std::size_t idx = 0; idx < path_count; idx++) {
        std::string resolved_path = ResolveImportPath(paths[idx]);
        resolved_parsed_sources_.emplace(paths[idx], resolved_path);

        const auto data = GetImportData(resolved_path);

        if (!data.has_decl) {
            continue;
        }

        std::ifstream input_stream(resolved_path.c_str());

        if (GetProgram()->SourceFilePath().Is(resolved_path)) {
            break;
        }

        if (input_stream.fail()) {
            ThrowSyntaxError({"Failed to open file: ", resolved_path.c_str()});
        }

        std::stringstream ss;
        ss << input_stream.rdbuf();
        auto external_source = ss.str();

        auto current_lang = GetContext().SetLanguage(data.lang);
        ParseSource({paths[idx].c_str(), external_source.c_str(), resolved_path.c_str(), false});
        GetContext().SetLanguage(current_lang);
    }

    GetContext().Status() &= is_external ? ~ParserStatus::IN_EXTERNAL : ~ParserStatus::IN_IMPORT;
}

void ETSParser::ParseDefaultSources()
{
    auto isp = InnerSourceParser(this);
    SourceFile source(varbinder::ETSBinder::DEFAULT_IMPORT_SOURCE_FILE, varbinder::ETSBinder::DEFAULT_IMPORT_SOURCE);
    auto lexer = InitLexer(source);
    ArenaVector<ir::Statement *> statements(Allocator()->Adapter());

    Lexer()->NextToken();

    GetContext().Status() |= ParserStatus::IN_DEFAULT_IMPORTS;

    ParseImportDeclarations(statements);
    GetContext().Status() &= ~ParserStatus::IN_DEFAULT_IMPORTS;

    CollectDefaultSources();

    ParseSources(parsed_sources_, true);
}

void ETSParser::ParseSource(const SourceFile &source_file)
{
    auto *program = Allocator()->New<parser::Program>(Allocator(), GetProgram()->VarBinder());
    auto esp = ExternalSourceParser(this, program);
    auto lexer = InitLexer(source_file);

    lexer::SourcePosition start_loc = Lexer()->GetToken().Start();
    Lexer()->NextToken();

    auto statements = PrepareExternalGlobalClass(source_file);
    ParseETSGlobalScript(start_loc, statements);
}

ir::ScriptFunction *ETSParser::AddInitMethod(ArenaVector<ir::AstNode *> &global_properties)
{
    if (GetProgram()->Kind() == ScriptKind::STDLIB) {
        return nullptr;
    }

    // Lambda to create empty function node with signature: func(): void
    // NOTE: replace with normal createFunction call
    auto const create_function =
        [this](std::string_view const function_name, ir::ScriptFunctionFlags function_flags,
               ir::ModifierFlags const function_modifiers) -> std::pair<ir::ScriptFunction *, ir::MethodDefinition *> {
        auto *init_ident = AllocNode<ir::Identifier>(function_name, Allocator());
        ir::ScriptFunction *init_func;

        {
            ArenaVector<ir::Expression *> params(Allocator()->Adapter());

            ArenaVector<ir::Statement *> statements(Allocator()->Adapter());
            auto *init_body = AllocNode<ir::BlockStatement>(Allocator(), std::move(statements));

            init_func = AllocNode<ir::ScriptFunction>(ir::FunctionSignature(nullptr, std::move(params), nullptr),
                                                      init_body, function_flags, false, GetContext().GetLanguge());
        }

        init_func->SetIdent(init_ident);
        init_func->AddModifier(function_modifiers);

        auto *func_expr = AllocNode<ir::FunctionExpression>(init_func);

        auto *init_method = AllocNode<ir::MethodDefinition>(ir::MethodDefinitionKind::METHOD, init_ident, func_expr,
                                                            function_modifiers, Allocator(), false);

        return std::make_pair(init_func, init_method);
    };

    // Create public method for module re-initialization. The assignments and statements are sequentially called inside.
    auto [init_func, init_method] = create_function(compiler::Signatures::INIT_METHOD, ir::ScriptFunctionFlags::NONE,
                                                    ir::ModifierFlags::STATIC | ir::ModifierFlags::PUBLIC);
    global_properties.emplace_back(init_method);

    return init_func;
}

void ETSParser::MarkNodeAsExported(ir::AstNode *node, lexer::SourcePosition start_pos, bool default_export,
                                   std::size_t num_of_elements)
{
    ir::ModifierFlags flag = default_export ? ir::ModifierFlags::DEFAULT_EXPORT : ir::ModifierFlags::EXPORT;

    if (UNLIKELY(flag == ir::ModifierFlags::DEFAULT_EXPORT)) {
        if (num_of_elements > 1) {
            ThrowSyntaxError("Only one default export is allowed in a module", start_pos);
        }
    }

    node->AddModifier(flag);
}

ArenaVector<ir::AstNode *> ETSParser::ParseTopLevelStatements(ArenaVector<ir::Statement *> &statements)
{
    ArenaVector<ir::AstNode *> global_properties(Allocator()->Adapter());
    field_map_.clear();
    export_name_map_.clear();
    bool default_export = false;

    using ParserFunctionPtr = std::function<ir::Statement *(ETSParser *)>;
    auto const parse_type = [this, &statements, &default_export](std::size_t const current_pos,
                                                                 ParserFunctionPtr const &parser_function) -> void {
        ir::Statement *node = nullptr;

        node = parser_function(this);
        if (node != nullptr) {
            if (current_pos != std::numeric_limits<std::size_t>::max()) {
                MarkNodeAsExported(node, node->Start(), default_export);
                default_export = false;
            }
            statements.push_back(node);
        }
    };

    // Add special '_$init$_' method that will hold all the top-level variable initializations (as assignments) and
    // statements. By default it will be called in the global class static constructor but also it can be called
    // directly from outside using public '_$init$_' method call in global scope.
    // NOTE: now only a single-file modules are supported. Such a technique can be implemented in packages directly.
    ir::ScriptFunction *init_function = nullptr;
    if (GetProgram()->GetPackageName().Empty()) {
        init_function = AddInitMethod(global_properties);
    }

    while (Lexer()->GetToken().Type() != lexer::TokenType::EOS) {
        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SEMI_COLON) {
            Lexer()->NextToken();
            continue;
        }

        auto current_pos = std::numeric_limits<size_t>::max();
        if (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_EXPORT) {
            Lexer()->NextToken();
            current_pos = global_properties.size();

            if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_MULTIPLY ||
                Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
                ParseExport(Lexer()->GetToken().Start());
                continue;
            }

            if (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_DEFAULT) {
                default_export = true;
                Lexer()->NextToken();
            }
        }

        lexer::SourcePosition start_loc = Lexer()->GetToken().Start();

        auto member_modifiers = ir::ModifierFlags::STATIC | ir::ModifierFlags::PUBLIC;

        if (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_DECLARE) {
            CheckDeclare();
            member_modifiers |= ir::ModifierFlags::DECLARE;
        }

        switch (auto const token_type = Lexer()->GetToken().Type(); token_type) {
            case lexer::TokenType::KEYW_CONST: {
                member_modifiers |= ir::ModifierFlags::CONST;
                [[fallthrough]];
            }
            case lexer::TokenType::KEYW_LET: {
                Lexer()->NextToken();
                auto *member_name = ExpectIdentifier();
                ParseClassFieldDefiniton(member_name, member_modifiers, &global_properties, init_function, &start_loc);
                break;
            }
            case lexer::TokenType::KEYW_ASYNC:
            case lexer::TokenType::KEYW_NATIVE: {
                bool is_async = token_type == lexer::TokenType::KEYW_ASYNC;

                if (is_async) {
                    member_modifiers |= ir::ModifierFlags::ASYNC;
                } else {
                    member_modifiers |= ir::ModifierFlags::NATIVE;
                }

                Lexer()->NextToken();

                if (Lexer()->GetToken().Type() != lexer::TokenType::KEYW_FUNCTION) {
                    ThrowSyntaxError(
                        {is_async ? "'async'" : "'native'", " flags must be used for functions at top-level."});
                }
                [[fallthrough]];
            }
            case lexer::TokenType::KEYW_FUNCTION: {
                Lexer()->NextToken();
                // check whether it is an extension function
                ir::Identifier *class_name = nullptr;
                if (Lexer()->Lookahead() == lexer::LEX_CHAR_DOT) {
                    class_name = ExpectIdentifier();
                    Lexer()->NextToken();
                }

                auto *member_name = ExpectIdentifier();
                auto *class_method = ParseClassMethodDefinition(member_name, member_modifiers, class_name);
                class_method->SetStart(start_loc);
                if (!class_method->Function()->IsOverload()) {
                    global_properties.push_back(class_method);
                }
                break;
            }
            case lexer::TokenType::KEYW_STATIC:
                [[fallthrough]];
            case lexer::TokenType::KEYW_ABSTRACT:
                [[fallthrough]];
            case lexer::TokenType::KEYW_FINAL:
                [[fallthrough]];
            case lexer::TokenType::KEYW_ENUM:
                [[fallthrough]];
            case lexer::TokenType::KEYW_INTERFACE:
                [[fallthrough]];
            case lexer::TokenType::KEYW_CLASS: {
                // NOLINTNEXTLINE(modernize-avoid-bind)
                parse_type(current_pos, std::bind(&ETSParser::ParseTypeDeclaration, std::placeholders::_1, false));
                break;
            }
            case lexer::TokenType::KEYW_TYPE: {
                parse_type(current_pos, &ETSParser::ParseTypeAliasDeclaration);
                break;
            }
            default: {
                // If struct is a soft keyword, handle it here, otherwise it's an identifier.
                if (IsStructKeyword()) {
                    parse_type(current_pos, [](ETSParser *obj) { return obj->ParseTypeDeclaration(false); });
                    break;
                }

                if (init_function != nullptr) {
                    if (auto *const statement = ParseTopLevelStatement(); statement != nullptr) {
                        statement->SetParent(init_function->Body());
                        init_function->Body()->AsBlockStatement()->Statements().emplace_back(statement);
                    }
                    break;
                }

                ThrowUnexpectedToken(token_type);
            }
        }

        GetContext().Status() &= ~ParserStatus::IN_AMBIENT_CONTEXT;

        while (current_pos < global_properties.size()) {
            MarkNodeAsExported(global_properties[current_pos], start_loc, default_export,
                               global_properties.size() - current_pos);
            default_export = false;
            current_pos++;
        }
    }
    // Add export modifier flag to nodes exported in previous statements.
    for (auto &iter : export_name_map_) {
        util::StringView export_name = iter.first;
        lexer::SourcePosition start_loc = export_name_map_[export_name];
        if (field_map_.count(export_name) == 0) {
            ThrowSyntaxError("Cannot find name '" + std::string {export_name.Utf8()} + "' to export.", start_loc);
        }
        auto field = field_map_[export_name];
        // selective export does not support default
        MarkNodeAsExported(field, start_loc, false);
    }
    return global_properties;
}

// NOLINTNEXTLINE(google-default-arguments)
ir::Statement *ETSParser::ParseTopLevelStatement(StatementParsingFlags flags)
{
    switch (auto const token_type = Lexer()->GetToken().Type(); token_type) {
        case lexer::TokenType::PUNCTUATOR_LEFT_BRACE: {
            return ParseBlockStatement();
        }
        case lexer::TokenType::PUNCTUATOR_SEMI_COLON: {
            return ParseEmptyStatement();
        }
        case lexer::TokenType::KEYW_ASSERT: {
            return ParseAssertStatement();
        }
        case lexer::TokenType::KEYW_IF: {
            return ParseIfStatement();
        }
        case lexer::TokenType::KEYW_DO: {
            return ParseDoWhileStatement();
        }
        case lexer::TokenType::KEYW_FOR: {
            return ParseForStatement();
        }
        case lexer::TokenType::KEYW_TRY: {
            return ParseTryStatement();
        }
        case lexer::TokenType::KEYW_WHILE: {
            return ParseWhileStatement();
        }
        case lexer::TokenType::KEYW_BREAK: {
            return ParseBreakStatement();
        }
        case lexer::TokenType::KEYW_CONTINUE: {
            return ParseContinueStatement();
        }
        case lexer::TokenType::KEYW_THROW: {
            return ParseThrowStatement();
        }
        case lexer::TokenType::KEYW_SWITCH: {
            return ParseSwitchStatement();
        }
        case lexer::TokenType::KEYW_DEBUGGER: {
            return ParseDebuggerStatement();
        }
        case lexer::TokenType::LITERAL_IDENT: {
            if (Lexer()->Lookahead() == lexer::LEX_CHAR_COLON) {
                const auto pos = Lexer()->Save();
                Lexer()->NextToken();
                return ParseLabelledStatement(pos);
            }

            return ParseExpressionStatement(flags);
        }
        // These cases never can occur here!
        case lexer::TokenType::KEYW_EXPORT:
            [[fallthrough]];
        case lexer::TokenType::KEYW_IMPORT:
            [[fallthrough]];
        case lexer::TokenType::KEYW_RETURN: {
            ThrowUnexpectedToken(token_type);
        }
        // Note: let's leave the default processing case separately, because it can be changed in the future.
        default: {
            ThrowUnexpectedToken(token_type);
            // return ParseExpressionStatement(flags);
        }
    }
}

void ETSParser::ParseTopLevelDeclaration(ArenaVector<ir::Statement *> &statements)
{
    lexer::SourcePosition class_body_start_loc = Lexer()->GetToken().Start();
    auto global_properties = ParseTopLevelStatements(statements);

    auto *class_def = GetProgram()->GlobalClass();

    if (class_def->IsGlobalInitialized()) {
        class_def->AddProperties(std::move(global_properties));
        Lexer()->NextToken();
        return;
    }

    CreateCCtor(global_properties, class_body_start_loc, GetProgram()->Kind() != ScriptKind::STDLIB);
    class_def->AddProperties(std::move(global_properties));
    auto *class_decl = class_def->Parent()->AsClassDeclaration();
    class_def->SetGlobalInitialized();
    class_def->SetRange(class_def->Range());

    statements.push_back(class_decl);
    Lexer()->NextToken();
}

// NOLINTNEXTLINE(google-default-arguments)
void ETSParser::CreateCCtor(ArenaVector<ir::AstNode *> &properties, const lexer::SourcePosition &loc,
                            const bool in_global_class)
{
    bool has_static_field = false;
    for (const auto *prop : properties) {
        if (prop->IsClassStaticBlock()) {
            return;
        }

        if (!prop->IsClassProperty()) {
            continue;
        }

        const auto *field = prop->AsClassProperty();

        if (field->IsStatic()) {
            has_static_field = true;
        }
    }

    if (!has_static_field && !in_global_class) {
        return;
    }

    ArenaVector<ir::Expression *> params(Allocator()->Adapter());

    auto *id = AllocNode<ir::Identifier>(compiler::Signatures::CCTOR, Allocator());

    ArenaVector<ir::Statement *> statements(Allocator()->Adapter());

    // Add the call to special '_$init$_' method containing all the top-level variable initializations (as assignments)
    // and statements to the end of static constructor of the global class.
    if (in_global_class) {
        if (auto const it = std::find_if(properties.begin(), properties.end(),
                                         [](ir::AstNode const *const item) {
                                             return item->IsMethodDefinition() &&
                                                    item->AsMethodDefinition()->Id()->Name() ==
                                                        compiler::Signatures::INIT_METHOD;
                                         });
            it != properties.end()) {
            if (!(*it)->AsMethodDefinition()->Function()->Body()->AsBlockStatement()->Statements().empty()) {
                auto *const callee = AllocNode<ir::Identifier>(compiler::Signatures::INIT_METHOD, Allocator());
                callee->SetReference();

                auto *const call_expr = AllocNode<ir::CallExpression>(
                    callee, ArenaVector<ir::Expression *>(Allocator()->Adapter()), nullptr, false, false);

                statements.emplace_back(AllocNode<ir::ExpressionStatement>(call_expr));
            }
        }
    }

    auto *body = AllocNode<ir::BlockStatement>(Allocator(), std::move(statements));
    auto *func = AllocNode<ir::ScriptFunction>(ir::FunctionSignature(nullptr, std::move(params), nullptr), body,
                                               ir::ScriptFunctionFlags::STATIC_BLOCK | ir::ScriptFunctionFlags::HIDDEN,
                                               ir::ModifierFlags::STATIC, false, GetContext().GetLanguge());
    func->SetIdent(id);

    auto *func_expr = AllocNode<ir::FunctionExpression>(func);
    auto *static_block = AllocNode<ir::ClassStaticBlock>(func_expr, Allocator());
    static_block->AddModifier(ir::ModifierFlags::STATIC);
    static_block->SetRange({loc, loc});
    properties.push_back(static_block);
}

static bool IsClassModifier(lexer::TokenType type)
{
    return type == lexer::TokenType::KEYW_STATIC || type == lexer::TokenType::KEYW_ABSTRACT ||
           type == lexer::TokenType::KEYW_FINAL;
}

ir::ModifierFlags ETSParser::ParseClassModifiers()
{
    ir::ModifierFlags flags = ir::ModifierFlags::NONE;

    while (IsClassModifier(Lexer()->GetToken().KeywordType())) {
        ir::ModifierFlags current_flag = ir::ModifierFlags::NONE;

        lexer::TokenFlags token_flags = Lexer()->GetToken().Flags();
        if ((token_flags & lexer::TokenFlags::HAS_ESCAPE) != 0) {
            ThrowSyntaxError("Keyword must not contain escaped characters");
        }

        switch (Lexer()->GetToken().KeywordType()) {
            case lexer::TokenType::KEYW_STATIC: {
                current_flag = ir::ModifierFlags::STATIC;
                break;
            }
            case lexer::TokenType::KEYW_FINAL: {
                current_flag = ir::ModifierFlags::FINAL;
                break;
            }
            case lexer::TokenType::KEYW_ABSTRACT: {
                current_flag = ir::ModifierFlags::ABSTRACT;
                break;
            }
            default: {
                UNREACHABLE();
            }
        }

        if ((flags & current_flag) != 0) {
            ThrowSyntaxError("Duplicated modifier is not allowed");
        }

        Lexer()->NextToken();
        flags |= current_flag;
    }

    return flags;
}

std::tuple<ir::Expression *, ir::TSTypeParameterInstantiation *> ETSParser::ParseClassImplementsElement()
{
    TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR |
                                           TypeAnnotationParsingOptions::IGNORE_FUNCTION_TYPE |
                                           TypeAnnotationParsingOptions::ALLOW_WILDCARD;
    return {ParseTypeReference(&options), nullptr};
}

ir::Expression *ETSParser::ParseSuperClassReference()
{
    if (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_EXTENDS) {
        Lexer()->NextToken();

        TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR |
                                               TypeAnnotationParsingOptions::IGNORE_FUNCTION_TYPE |
                                               TypeAnnotationParsingOptions::ALLOW_WILDCARD;
        return ParseTypeReference(&options);
    }

    return nullptr;
}

ir::TypeNode *ETSParser::ParseInterfaceExtendsElement()
{
    TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR |
                                           TypeAnnotationParsingOptions::IGNORE_FUNCTION_TYPE |
                                           TypeAnnotationParsingOptions::ALLOW_WILDCARD;
    return ParseTypeReference(&options);
}

static bool IsClassMemberAccessModifier(lexer::TokenType type)
{
    return type == lexer::TokenType::KEYW_PUBLIC || type == lexer::TokenType::KEYW_PRIVATE ||
           type == lexer::TokenType::KEYW_PROTECTED || type == lexer::TokenType::KEYW_INTERNAL;
}

std::tuple<ir::ModifierFlags, bool> ETSParser::ParseClassMemberAccessModifiers()
{
    if (IsClassMemberAccessModifier(Lexer()->GetToken().Type())) {
        char32_t next_cp = Lexer()->Lookahead();
        if (!(next_cp != lexer::LEX_CHAR_EQUALS && next_cp != lexer::LEX_CHAR_COLON &&
              next_cp != lexer::LEX_CHAR_LEFT_PAREN)) {
            return {ir::ModifierFlags::NONE, false};
        }

        lexer::TokenFlags token_flags = Lexer()->GetToken().Flags();
        if ((token_flags & lexer::TokenFlags::HAS_ESCAPE) != 0) {
            ThrowSyntaxError("Keyword must not contain escaped characters");
        }

        ir::ModifierFlags access_flag = ir::ModifierFlags::NONE;

        switch (Lexer()->GetToken().KeywordType()) {
            case lexer::TokenType::KEYW_PUBLIC: {
                access_flag = ir::ModifierFlags::PUBLIC;
                break;
            }
            case lexer::TokenType::KEYW_PRIVATE: {
                access_flag = ir::ModifierFlags::PRIVATE;
                break;
            }
            case lexer::TokenType::KEYW_PROTECTED: {
                access_flag = ir::ModifierFlags::PROTECTED;
                break;
            }
            case lexer::TokenType::KEYW_INTERNAL: {
                Lexer()->NextToken(lexer::NextTokenFlags::KEYWORD_TO_IDENT);
                if (Lexer()->GetToken().KeywordType() != lexer::TokenType::KEYW_PROTECTED) {
                    access_flag = ir::ModifierFlags::INTERNAL;
                    return {access_flag, true};
                }
                access_flag = ir::ModifierFlags::INTERNAL_PROTECTED;
                break;
            }
            default: {
                UNREACHABLE();
            }
        }

        Lexer()->NextToken(lexer::NextTokenFlags::KEYWORD_TO_IDENT);
        return {access_flag, true};
    }

    return {ir::ModifierFlags::PUBLIC, false};
}

static bool IsClassFieldModifier(lexer::TokenType type)
{
    return type == lexer::TokenType::KEYW_STATIC || type == lexer::TokenType::KEYW_READONLY;
}

ir::ModifierFlags ETSParser::ParseClassFieldModifiers(bool seen_static)
{
    ir::ModifierFlags flags = seen_static ? ir::ModifierFlags::STATIC : ir::ModifierFlags::NONE;

    while (IsClassFieldModifier(Lexer()->GetToken().KeywordType())) {
        char32_t next_cp = Lexer()->Lookahead();
        if (!(next_cp != lexer::LEX_CHAR_EQUALS && next_cp != lexer::LEX_CHAR_COLON)) {
            return flags;
        }

        ir::ModifierFlags current_flag = ir::ModifierFlags::NONE;

        lexer::TokenFlags token_flags = Lexer()->GetToken().Flags();
        if ((token_flags & lexer::TokenFlags::HAS_ESCAPE) != 0) {
            ThrowSyntaxError("Keyword must not contain escaped characters");
        }

        switch (Lexer()->GetToken().KeywordType()) {
            case lexer::TokenType::KEYW_STATIC: {
                current_flag = ir::ModifierFlags::STATIC;
                break;
            }
            case lexer::TokenType::KEYW_READONLY: {
                // NOTE(OCs): Use ir::ModifierFlags::READONLY once compiler is ready for it.
                current_flag = ir::ModifierFlags::CONST;
                break;
            }
            default: {
                UNREACHABLE();
            }
        }

        if ((flags & current_flag) != 0) {
            ThrowSyntaxError("Duplicated modifier is not allowed");
        }

        Lexer()->NextToken(lexer::NextTokenFlags::KEYWORD_TO_IDENT);
        flags |= current_flag;
    }

    return flags;
}

static bool IsClassMethodModifier(lexer::TokenType type)
{
    switch (type) {
        case lexer::TokenType::KEYW_STATIC:
        case lexer::TokenType::KEYW_FINAL:
        case lexer::TokenType::KEYW_NATIVE:
        case lexer::TokenType::KEYW_ASYNC:
        case lexer::TokenType::KEYW_OVERRIDE:
        case lexer::TokenType::KEYW_ABSTRACT: {
            return true;
        }
        default: {
            break;
        }
    }

    return false;
}

ir::ModifierFlags ETSParser::ParseClassMethodModifiers(bool seen_static)
{
    ir::ModifierFlags flags = seen_static ? ir::ModifierFlags::STATIC : ir::ModifierFlags::NONE;

    while (IsClassMethodModifier(Lexer()->GetToken().KeywordType())) {
        char32_t next_cp = Lexer()->Lookahead();
        if (!(next_cp != lexer::LEX_CHAR_LEFT_PAREN)) {
            return flags;
        }

        ir::ModifierFlags current_flag = ir::ModifierFlags::NONE;

        lexer::TokenFlags token_flags = Lexer()->GetToken().Flags();
        if ((token_flags & lexer::TokenFlags::HAS_ESCAPE) != 0) {
            ThrowSyntaxError("Keyword must not contain escaped characters");
        }

        switch (Lexer()->GetToken().KeywordType()) {
            case lexer::TokenType::KEYW_STATIC: {
                current_flag = ir::ModifierFlags::STATIC;
                break;
            }
            case lexer::TokenType::KEYW_FINAL: {
                current_flag = ir::ModifierFlags::FINAL;
                break;
            }
            case lexer::TokenType::KEYW_NATIVE: {
                current_flag = ir::ModifierFlags::NATIVE;
                break;
            }
            case lexer::TokenType::KEYW_ASYNC: {
                current_flag = ir::ModifierFlags::ASYNC;
                break;
            }
            case lexer::TokenType::KEYW_OVERRIDE: {
                current_flag = ir::ModifierFlags::OVERRIDE;
                break;
            }
            case lexer::TokenType::KEYW_ABSTRACT: {
                current_flag = ir::ModifierFlags::ABSTRACT;
                break;
            }
            case lexer::TokenType::KEYW_DECLARE: {
                current_flag = ir::ModifierFlags::DECLARE;
                break;
            }
            default: {
                UNREACHABLE();
            }
        }

        if ((flags & current_flag) != 0) {
            ThrowSyntaxError("Duplicated modifier is not allowed");
        }

        Lexer()->NextToken(lexer::NextTokenFlags::KEYWORD_TO_IDENT);
        flags |= current_flag;
        if ((flags & ir::ModifierFlags::ASYNC) != 0 && (flags & ir::ModifierFlags::NATIVE) != 0) {
            ThrowSyntaxError("Native method cannot be async");
        }
    }

    return flags;
}

// NOLINTNEXTLINE(google-default-arguments)
void ETSParser::ParseClassFieldDefiniton(ir::Identifier *field_name, ir::ModifierFlags modifiers,
                                         ArenaVector<ir::AstNode *> *declarations, ir::ScriptFunction *init_function,
                                         lexer::SourcePosition *let_loc)
{
    lexer::SourcePosition start_loc = let_loc != nullptr ? *let_loc : Lexer()->GetToken().Start();
    lexer::SourcePosition end_loc = start_loc;
    ir::TypeNode *type_annotation = nullptr;
    TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR;

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COLON) {
        Lexer()->NextToken();  // eat ':'
        type_annotation = ParseTypeAnnotation(&options);
    }

    ir::Expression *initializer = nullptr;
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
        Lexer()->NextToken();  // eat '='
        initializer = ParseInitializer();
    } else if (type_annotation == nullptr) {
        ThrowSyntaxError("Field type annotation expected");
    }

    // Add initialization of top-level (global) variables to a special '_$init$_' function so that it could be
    // performed multiple times.
    if (init_function != nullptr && (modifiers & ir::ModifierFlags::CONST) == 0U && initializer != nullptr &&
        !initializer->IsArrowFunctionExpression()) {
        if (auto *const func_body = init_function->Body(); func_body != nullptr && func_body->IsBlockStatement()) {
            auto *ident = AllocNode<ir::Identifier>(field_name->Name(), Allocator());
            ident->SetReference();
            ident->SetRange(field_name->Range());

            auto *assignment_expression =
                AllocNode<ir::AssignmentExpression>(ident, initializer, lexer::TokenType::PUNCTUATOR_SUBSTITUTION);
            end_loc = initializer->End();
            assignment_expression->SetRange({field_name->Start(), end_loc});
            assignment_expression->SetParent(func_body);

            auto expression_statement = AllocNode<ir::ExpressionStatement>(assignment_expression);
            if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SEMI_COLON) {
                end_loc = Lexer()->GetToken().End();
            }
            expression_statement->SetRange({start_loc, end_loc});
            func_body->AsBlockStatement()->Statements().emplace_back(expression_statement);

            if (type_annotation != nullptr && !type_annotation->IsETSFunctionType()) {
                initializer = nullptr;
            }
        }
    }

    bool is_declare = (modifiers & ir::ModifierFlags::DECLARE) != 0;

    if (is_declare && initializer != nullptr) {
        ThrowSyntaxError("Initializers are not allowed in ambient contexts.");
    }
    auto *field = AllocNode<ir::ClassProperty>(field_name, initializer, type_annotation, modifiers, Allocator(), false);
    start_loc = field_name->Start();
    if (initializer != nullptr) {
        end_loc = initializer->End();
    } else {
        end_loc = type_annotation != nullptr ? type_annotation->End() : field_name->End();
    }
    field->SetRange({start_loc, end_loc});

    field_map_.insert({field_name->Name(), field});
    declarations->push_back(field);

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COMMA) {
        Lexer()->NextToken();
        ir::Identifier *next_name = ExpectIdentifier(false, true);
        ParseClassFieldDefiniton(next_name, modifiers, declarations);
    }
}

ir::MethodDefinition *ETSParser::ParseClassMethodDefinition(ir::Identifier *method_name, ir::ModifierFlags modifiers,
                                                            ir::Identifier *class_name, ir::Identifier *ident_node)
{
    auto new_status = ParserStatus::NEED_RETURN_TYPE | ParserStatus::ALLOW_SUPER;
    auto method_kind = ir::MethodDefinitionKind::METHOD;

    if (class_name != nullptr) {
        method_kind = ir::MethodDefinitionKind::EXTENSION_METHOD;
        new_status |= ParserStatus::IN_EXTENSION_FUNCTION;
    }

    if ((modifiers & ir::ModifierFlags::CONSTRUCTOR) != 0) {
        new_status = ParserStatus::CONSTRUCTOR_FUNCTION | ParserStatus::ALLOW_SUPER | ParserStatus::ALLOW_SUPER_CALL;
        method_kind = ir::MethodDefinitionKind::CONSTRUCTOR;
    }

    if ((modifiers & ir::ModifierFlags::ASYNC) != 0) {
        new_status |= ParserStatus::ASYNC_FUNCTION;
    }

    if ((modifiers & ir::ModifierFlags::STATIC) == 0) {
        new_status |= ParserStatus::ALLOW_THIS_TYPE;
    }

    ir::ScriptFunction *func = ParseFunction(new_status, class_name);
    func->SetIdent(method_name);
    auto *func_expr = AllocNode<ir::FunctionExpression>(func);
    func_expr->SetRange(func->Range());
    func->AddModifier(modifiers);

    if (class_name != nullptr) {
        func->AddFlag(ir::ScriptFunctionFlags::INSTANCE_EXTENSION_METHOD);
    }
    auto *method = AllocNode<ir::MethodDefinition>(method_kind, method_name, func_expr, modifiers, Allocator(), false);
    method->SetRange(func_expr->Range());

    field_map_.insert({method_name->Name(), method});
    AddProxyOverloadToMethodWithDefaultParams(method, ident_node);

    return method;
}

ir::ScriptFunction *ETSParser::ParseFunction(ParserStatus new_status, ir::Identifier *class_name)
{
    FunctionContext function_context(this, new_status | ParserStatus::FUNCTION);
    lexer::SourcePosition start_loc = Lexer()->GetToken().Start();
    auto [signature, throw_marker] = ParseFunctionSignature(new_status, class_name);

    ir::AstNode *body = nullptr;
    lexer::SourcePosition end_loc = start_loc;
    bool is_overload = false;
    bool is_arrow = (new_status & ParserStatus::ARROW_FUNCTION) != 0;

    if ((new_status & ParserStatus::ASYNC_FUNCTION) != 0) {
        function_context.AddFlag(ir::ScriptFunctionFlags::ASYNC);
    }

    if (is_arrow) {
        if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_ARROW) {
            ThrowSyntaxError("'=>' expected");
        }

        function_context.AddFlag(ir::ScriptFunctionFlags::ARROW);
        Lexer()->NextToken();
    }

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
        std::tie(std::ignore, body, end_loc, is_overload) =
            ParseFunctionBody(signature.Params(), new_status, GetContext().Status());
    } else if (is_arrow) {
        body = ParseExpression();
        end_loc = body->AsExpression()->End();
        function_context.AddFlag(ir::ScriptFunctionFlags::EXPRESSION);
    }

    if ((GetContext().Status() & ParserStatus::FUNCTION_HAS_RETURN_STATEMENT) != 0) {
        function_context.AddFlag(ir::ScriptFunctionFlags::HAS_RETURN);
        GetContext().Status() ^= ParserStatus::FUNCTION_HAS_RETURN_STATEMENT;
    }
    function_context.AddFlag(throw_marker);

    auto *func_node = AllocNode<ir::ScriptFunction>(std::move(signature), body, function_context.Flags(), false,
                                                    GetContext().GetLanguge());
    func_node->SetRange({start_loc, end_loc});

    return func_node;
}

ir::MethodDefinition *ETSParser::ParseClassMethod(ClassElementDescriptor *desc,
                                                  const ArenaVector<ir::AstNode *> &properties,
                                                  ir::Expression *prop_name, lexer::SourcePosition *prop_end)
{
    if (desc->method_kind != ir::MethodDefinitionKind::SET &&
        (desc->new_status & ParserStatus::CONSTRUCTOR_FUNCTION) == 0) {
        desc->new_status |= ParserStatus::NEED_RETURN_TYPE;
    }

    ir::ScriptFunction *func = ParseFunction(desc->new_status);

    auto *func_expr = AllocNode<ir::FunctionExpression>(func);
    func_expr->SetRange(func->Range());

    if (desc->method_kind == ir::MethodDefinitionKind::SET) {
        ValidateClassSetter(desc, properties, prop_name, func);
    } else if (desc->method_kind == ir::MethodDefinitionKind::GET) {
        ValidateClassGetter(desc, properties, prop_name, func);
    }

    *prop_end = func->End();
    func->AddFlag(ir::ScriptFunctionFlags::METHOD);
    auto *method = AllocNode<ir::MethodDefinition>(desc->method_kind, prop_name, func_expr, desc->modifiers,
                                                   Allocator(), desc->is_computed);
    method->SetRange(func_expr->Range());

    return method;
}

std::tuple<bool, ir::BlockStatement *, lexer::SourcePosition, bool> ETSParser::ParseFunctionBody(
    [[maybe_unused]] const ArenaVector<ir::Expression *> &params, [[maybe_unused]] ParserStatus new_status,
    [[maybe_unused]] ParserStatus context_status)
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE);

    ir::BlockStatement *body = ParseBlockStatement();

    return {true, body, body->End(), false};
}

ir::TypeNode *ETSParser::ParseFunctionReturnType([[maybe_unused]] ParserStatus status)
{
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COLON) {
        if ((status & ParserStatus::CONSTRUCTOR_FUNCTION) != 0U) {
            ThrowSyntaxError("Type annotation isn't allowed for constructor.");
        }
        Lexer()->NextToken();  // eat ':'
        TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR |
                                               TypeAnnotationParsingOptions::CAN_BE_TS_TYPE_PREDICATE |
                                               TypeAnnotationParsingOptions::RETURN_TYPE;
        return ParseTypeAnnotation(&options);
    }

    return nullptr;
}

ir::ScriptFunctionFlags ETSParser::ParseFunctionThrowMarker(bool is_rethrows_allowed)
{
    ir::ScriptFunctionFlags throw_marker = ir::ScriptFunctionFlags::NONE;

    if (Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_IDENT) {
        if (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_THROWS) {
            Lexer()->NextToken();  // eat 'throws'
            throw_marker = ir::ScriptFunctionFlags::THROWS;
        } else if (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_RETHROWS) {
            if (is_rethrows_allowed) {
                Lexer()->NextToken();  // eat 'rethrows'
                throw_marker = ir::ScriptFunctionFlags::RETHROWS;
            } else {
                ThrowSyntaxError("Only 'throws' can be used with function types");
            }
        }
    }

    return throw_marker;
}

void ETSParser::ValidateLabeledStatement(lexer::TokenType type)
{
    if (type != lexer::TokenType::KEYW_DO && type != lexer::TokenType::KEYW_WHILE &&
        type != lexer::TokenType::KEYW_FOR && type != lexer::TokenType::KEYW_SWITCH) {
        ThrowSyntaxError("Label must be followed by a loop statement", Lexer()->GetToken().Start());
    }
}

// NOLINTNEXTLINE(google-default-arguments)
ir::AstNode *ETSParser::ParseClassElement([[maybe_unused]] const ArenaVector<ir::AstNode *> &properties,
                                          [[maybe_unused]] ir::ClassDefinitionModifiers modifiers,
                                          [[maybe_unused]] ir::ModifierFlags flags,
                                          [[maybe_unused]] ir::Identifier *ident_node)
{
    auto start_loc = Lexer()->GetToken().Start();
    auto saved_pos = Lexer()->Save();  // NOLINT(clang-analyzer-deadcode.DeadStores)

    if (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_STATIC &&
        Lexer()->Lookahead() == lexer::LEX_CHAR_LEFT_BRACE) {
        return ParseClassStaticBlock();
    }

    auto [memberModifiers, stepToken] = ParseClassMemberAccessModifiers();

    if (InAmbientContext()) {
        memberModifiers |= ir::ModifierFlags::DECLARE;
    }

    bool seen_static = false;
    char32_t next_cp = Lexer()->Lookahead();

    if (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_STATIC && next_cp != lexer::LEX_CHAR_EQUALS &&
        next_cp != lexer::LEX_CHAR_COLON && next_cp != lexer::LEX_CHAR_LEFT_PAREN &&
        next_cp != lexer::LEX_CHAR_LESS_THAN) {
        Lexer()->NextToken();
        memberModifiers |= ir::ModifierFlags::STATIC;
        seen_static = true;
    }

    if (IsClassFieldModifier(Lexer()->GetToken().KeywordType())) {
        memberModifiers |= ParseClassFieldModifiers(seen_static);
    } else if (IsClassMethodModifier(Lexer()->GetToken().Type())) {
        memberModifiers |= ParseClassMethodModifiers(seen_static);
    }

    switch (Lexer()->GetToken().Type()) {
        case lexer::TokenType::KEYW_INTERFACE:
        case lexer::TokenType::KEYW_CLASS:
        case lexer::TokenType::KEYW_ENUM: {
            ThrowSyntaxError(
                "Local type declaration (class, struct, interface and enum) support is not yet implemented.");
            // remove saved_pos nolint

            Lexer()->Rewind(saved_pos);
            if (stepToken) {
                Lexer()->NextToken();
            }

            Lexer()->GetToken().SetTokenType(Lexer()->GetToken().KeywordType());
            ir::AstNode *type_decl = ParseTypeDeclaration(true);
            memberModifiers &= (ir::ModifierFlags::PUBLIC | ir::ModifierFlags::PROTECTED | ir::ModifierFlags::PRIVATE |
                                ir::ModifierFlags::INTERNAL);
            type_decl->AddModifier(memberModifiers);

            if (!seen_static) {
                if (type_decl->IsClassDeclaration()) {
                    type_decl->AsClassDeclaration()->Definition()->AsClassDefinition()->SetInnerModifier();
                } else if (type_decl->IsETSStructDeclaration()) {
                    type_decl->AsETSStructDeclaration()->Definition()->AsClassDefinition()->SetInnerModifier();
                }
            }

            return type_decl;
        }
        case lexer::TokenType::KEYW_CONSTRUCTOR: {
            if ((memberModifiers & ir::ModifierFlags::ASYNC) != 0) {
                ThrowSyntaxError({"Constructor should not be async."});
            }
            auto *member_name = AllocNode<ir::Identifier>(Lexer()->GetToken().Ident(), Allocator());
            memberModifiers |= ir::ModifierFlags::CONSTRUCTOR;
            Lexer()->NextToken();
            auto *class_method = ParseClassMethodDefinition(member_name, memberModifiers);
            class_method->SetStart(start_loc);

            return class_method;
        }
        default: {
            break;
        }
    }

    switch (Lexer()->GetToken().Type()) {
        case lexer::TokenType::KEYW_PUBLIC:
        case lexer::TokenType::KEYW_PRIVATE:
        case lexer::TokenType::KEYW_PROTECTED: {
            ThrowSyntaxError("Access modifier must precede field and method modifiers.");
            break;
        }
        default:
            break;
    }

    if (Lexer()->Lookahead() != lexer::LEX_CHAR_LEFT_PAREN && Lexer()->Lookahead() != lexer::LEX_CHAR_LESS_THAN &&
        (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_GET ||
         Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_SET)) {
        return ParseClassGetterSetterMethod(properties, modifiers, memberModifiers);
    }

    auto *member_name = ExpectIdentifier(false, true);

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS ||
        Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LESS_THAN) {
        auto *class_method = ParseClassMethodDefinition(member_name, memberModifiers, nullptr, ident_node);
        class_method->SetStart(start_loc);
        return class_method;
    }

    ArenaVector<ir::AstNode *> field_declarations(Allocator()->Adapter());
    auto *placeholder = AllocNode<ir::TSInterfaceBody>(std::move(field_declarations));
    ParseClassFieldDefiniton(member_name, memberModifiers, placeholder->BodyPtr());
    return placeholder;
}

ir::MethodDefinition *ETSParser::ParseClassGetterSetterMethod(const ArenaVector<ir::AstNode *> &properties,
                                                              const ir::ClassDefinitionModifiers modifiers,
                                                              const ir::ModifierFlags member_modifiers)
{
    ClassElementDescriptor desc(Allocator());
    desc.method_kind = Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_GET ? ir::MethodDefinitionKind::GET
                                                                                       : ir::MethodDefinitionKind::SET;
    Lexer()->NextToken();  // eat get/set
    auto *method_name = AllocNode<ir::Identifier>(Lexer()->GetToken().Ident(), Allocator());
    if (desc.method_kind == ir::MethodDefinitionKind::GET) {
        method_name->SetAccessor();
    } else {
        method_name->SetMutator();
    }

    Lexer()->NextToken(lexer::NextTokenFlags::KEYWORD_TO_IDENT);

    desc.new_status = ParserStatus::ALLOW_SUPER;
    desc.has_super_class = (modifiers & ir::ClassDefinitionModifiers::HAS_SUPER) != 0U;
    desc.prop_start = Lexer()->GetToken().Start();
    desc.modifiers = member_modifiers;

    lexer::SourcePosition prop_end = method_name->End();
    ir::MethodDefinition *method = ParseClassMethod(&desc, properties, method_name, &prop_end);
    method->Function()->SetIdent(method_name);
    method->Function()->AddModifier(desc.modifiers);
    method->SetRange({desc.prop_start, prop_end});
    if (desc.method_kind == ir::MethodDefinitionKind::GET) {
        method->Function()->AddFlag(ir::ScriptFunctionFlags::GETTER);
    } else {
        method->Function()->AddFlag(ir::ScriptFunctionFlags::SETTER);
    }

    return method;
}

ir::MethodDefinition *ETSParser::ParseInterfaceGetterSetterMethod(const ir::ModifierFlags modifiers)
{
    auto method_kind = Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_GET ? ir::MethodDefinitionKind::GET
                                                                                       : ir::MethodDefinitionKind::SET;
    Lexer()->NextToken();  // eat get/set
    ir::MethodDefinition *method = ParseInterfaceMethod(modifiers, method_kind);
    method->SetRange({Lexer()->GetToken().Start(), method->Id()->End()});
    if (method_kind == ir::MethodDefinitionKind::GET) {
        method->Id()->SetAccessor();
        method->Function()->AddFlag(ir::ScriptFunctionFlags::GETTER);
    } else {
        method->Id()->SetMutator();
        method->Function()->AddFlag(ir::ScriptFunctionFlags::SETTER);
    }

    method->Function()->SetIdent(method->Id());
    method->Function()->AddModifier(method->Modifiers());

    return method;
}

ir::Statement *ETSParser::ParseTypeDeclaration(bool allow_static)
{
    auto saved_pos = Lexer()->Save();

    auto modifiers = ir::ClassDefinitionModifiers::ID_REQUIRED | ir::ClassDefinitionModifiers::CLASS_DECL;

    auto token_type = Lexer()->GetToken().Type();
    switch (token_type) {
        case lexer::TokenType::KEYW_STATIC: {
            if (!allow_static) {
                ThrowUnexpectedToken(Lexer()->GetToken().Type());
            }

            Lexer()->NextToken();

            if (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_INTERFACE) {
                return ParseInterfaceDeclaration(true);
            }

            Lexer()->Rewind(saved_pos);
            [[fallthrough]];
        }
        case lexer::TokenType::KEYW_ABSTRACT:
        case lexer::TokenType::KEYW_FINAL: {
            auto flags = ParseClassModifiers();
            if (allow_static && (flags & ir::ModifierFlags::STATIC) == 0U) {
                modifiers |= ir::ClassDefinitionModifiers::INNER;
            }

            if (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_CLASS) {
                return ParseClassDeclaration(modifiers, flags);
            }

            if (IsStructKeyword()) {
                return ParseStructDeclaration(modifiers, flags);
            }

            ThrowUnexpectedToken(Lexer()->GetToken().Type());
        }
        case lexer::TokenType::KEYW_ENUM: {
            return ParseEnumDeclaration(false);
        }
        case lexer::TokenType::KEYW_INTERFACE: {
            return ParseInterfaceDeclaration(false);
        }
        case lexer::TokenType::KEYW_CLASS: {
            return ParseClassDeclaration(modifiers);
        }
        case lexer::TokenType::KEYW_TYPE: {
            return ParseTypeAliasDeclaration();
        }
        case lexer::TokenType::LITERAL_IDENT: {
            if (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_STRUCT) {
                return ParseStructDeclaration(modifiers);
            }
            [[fallthrough]];
        }
        case lexer::TokenType::LITERAL_NUMBER:
        case lexer::TokenType::LITERAL_NULL:
        case lexer::TokenType::KEYW_UNDEFINED:
        case lexer::TokenType::LITERAL_STRING:
        case lexer::TokenType::LITERAL_FALSE:
        case lexer::TokenType::LITERAL_TRUE:
        case lexer::TokenType::LITERAL_CHAR: {
            std::string err_msg("Cannot used in global scope '");

            std::string text = token_type == lexer::TokenType::LITERAL_CHAR
                                   ? util::Helpers::UTF16toUTF8(Lexer()->GetToken().Utf16())
                                   : Lexer()->GetToken().Ident().Mutf8();

            if ((Lexer()->GetToken().Flags() & lexer::TokenFlags::HAS_ESCAPE) == 0) {
                err_msg.append(text);
            } else {
                err_msg.append(util::Helpers::CreateEscapedString(text));
            }

            err_msg.append("'");
            ThrowSyntaxError(err_msg.c_str());
        }
        default: {
            ThrowUnexpectedToken(Lexer()->GetToken().Type());
        }
    }
}

ir::TSTypeAliasDeclaration *ETSParser::ParseTypeAliasDeclaration()
{
    ASSERT(Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_TYPE);

    if ((GetContext().Status() & parser::ParserStatus::FUNCTION) != 0U) {
        ThrowSyntaxError("Type alias is allowed only as top-level declaration");
    }

    lexer::SourcePosition type_start = Lexer()->GetToken().Start();
    Lexer()->NextToken();  // eat type keyword

    if (Lexer()->GetToken().Type() != lexer::TokenType::LITERAL_IDENT) {
        ThrowSyntaxError("Identifier expected");
    }

    if (Lexer()->GetToken().IsReservedTypeName()) {
        std::string err_msg("Type alias name cannot be '");
        err_msg.append(TokenToString(Lexer()->GetToken().KeywordType()));
        err_msg.append("'");
        ThrowSyntaxError(err_msg.c_str());
    }

    const util::StringView ident = Lexer()->GetToken().Ident();
    auto *id = AllocNode<ir::Identifier>(ident, Allocator());
    id->SetRange(Lexer()->GetToken().Loc());

    auto *type_alias_decl = AllocNode<ir::TSTypeAliasDeclaration>(Allocator(), id);

    Lexer()->NextToken();  // eat alias name

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LESS_THAN) {
        auto options =
            TypeAnnotationParsingOptions::THROW_ERROR | TypeAnnotationParsingOptions::ALLOW_DECLARATION_SITE_VARIANCE;
        ir::TSTypeParameterDeclaration *params = ParseTypeParameterDeclaration(&options);
        type_alias_decl->AddTypeParameters(params);
        params->SetParent(type_alias_decl);
    }

    if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
        ThrowSyntaxError("'=' expected");
    }

    Lexer()->NextToken();  // eat '='

    TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR;
    ir::TypeNode *type_annotation = ParseTypeAnnotation(&options);
    type_alias_decl->SetTsTypeAnnotation(type_annotation);
    type_alias_decl->SetRange({type_start, Lexer()->GetToken().End()});
    type_annotation->SetParent(type_alias_decl);

    return type_alias_decl;
}

ir::TSInterfaceDeclaration *ETSParser::ParseInterfaceBody(ir::Identifier *name, bool is_static)
{
    GetContext().Status() |= ParserStatus::ALLOW_THIS_TYPE;

    ir::TSTypeParameterDeclaration *type_param_decl = nullptr;
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LESS_THAN) {
        auto options =
            TypeAnnotationParsingOptions::THROW_ERROR | TypeAnnotationParsingOptions::ALLOW_DECLARATION_SITE_VARIANCE;
        type_param_decl = ParseTypeParameterDeclaration(&options);
    }

    ArenaVector<ir::TSInterfaceHeritage *> extends(Allocator()->Adapter());
    if (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_EXTENDS) {
        extends = ParseInterfaceExtendsClause();
    }

    lexer::SourcePosition body_start = Lexer()->GetToken().Start();
    auto members = ParseTypeLiteralOrInterface();

    for (auto &member : members) {
        if (member->Type() == ir::AstNodeType::CLASS_DECLARATION ||
            member->Type() == ir::AstNodeType::STRUCT_DECLARATION ||
            member->Type() == ir::AstNodeType::TS_ENUM_DECLARATION ||
            member->Type() == ir::AstNodeType::TS_INTERFACE_DECLARATION) {
            ThrowSyntaxError(
                "Local type declaration (class, struct, interface and enum) support is not yet implemented.");
        }
    }

    auto *body = AllocNode<ir::TSInterfaceBody>(std::move(members));
    body->SetRange({body_start, Lexer()->GetToken().End()});

    const auto is_external = (GetContext().Status() & ParserStatus::IN_EXTERNAL);
    auto *interface_decl =
        AllocNode<ir::TSInterfaceDeclaration>(Allocator(), name, type_param_decl, body, std::move(extends), is_static,
                                              is_external, GetContext().GetLanguge());

    Lexer()->NextToken();
    GetContext().Status() &= ~ParserStatus::ALLOW_THIS_TYPE;

    return interface_decl;
}

ir::Statement *ETSParser::ParseInterfaceDeclaration(bool is_static)
{
    if ((GetContext().Status() & parser::ParserStatus::FUNCTION) != 0U) {
        ThrowSyntaxError("Local interface declaration support is not yet implemented.");
    }

    lexer::SourcePosition interface_start = Lexer()->GetToken().Start();
    Lexer()->NextToken();  // eat interface keyword

    auto *id = ExpectIdentifier(false, true);

    auto *decl_node = ParseInterfaceBody(id, is_static);

    decl_node->SetRange({interface_start, Lexer()->GetToken().End()});
    return decl_node;
}

// NOLINTNEXTLINE(google-default-arguments)
ir::Statement *ETSParser::ParseEnumDeclaration(bool is_const, bool is_static)
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::KEYW_ENUM);

    if ((GetContext().Status() & parser::ParserStatus::FUNCTION) != 0U) {
        ThrowSyntaxError("Local enum declaration support is not yet implemented.");
    }

    lexer::SourcePosition enum_start = Lexer()->GetToken().Start();
    Lexer()->NextToken();  // eat enum keyword

    auto *key = ExpectIdentifier(false, true);

    auto *decl_node = ParseEnumMembers(key, enum_start, is_const, is_static);

    return decl_node;
}

ir::Expression *ETSParser::ParseLaunchExpression(ExpressionParseFlags flags)
{
    lexer::SourcePosition start = Lexer()->GetToken().Start();
    Lexer()->NextToken();  // eat launch

    ir::Expression *expr = ParseLeftHandSideExpression(flags);
    if (!expr->IsCallExpression()) {
        ThrowSyntaxError("Only call expressions are allowed after 'launch'", expr->Start());
    }
    auto call = expr->AsCallExpression();
    auto *launch_expression = AllocNode<ir::ETSLaunchExpression>(call);
    launch_expression->SetRange({start, call->End()});

    return launch_expression;
}

// NOLINTNEXTLINE(google-default-arguments)
ir::ClassDefinition *ETSParser::ParseClassDefinition(ir::ClassDefinitionModifiers modifiers, ir::ModifierFlags flags)
{
    Lexer()->NextToken();

    ir::Identifier *ident_node = ParseClassIdent(modifiers);

    ir::TSTypeParameterDeclaration *type_param_decl = nullptr;
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LESS_THAN) {
        auto options =
            TypeAnnotationParsingOptions::THROW_ERROR | TypeAnnotationParsingOptions::ALLOW_DECLARATION_SITE_VARIANCE;
        type_param_decl = ParseTypeParameterDeclaration(&options);
    }

    // Parse SuperClass
    auto [superClass, superTypeParams] = ParseSuperClass();

    if (superClass != nullptr) {
        modifiers |= ir::ClassDefinitionModifiers::HAS_SUPER;
        GetContext().Status() |= ParserStatus::ALLOW_SUPER;
    }

    if (InAmbientContext()) {
        flags |= ir::ModifierFlags::DECLARE;
    }

    // Parse implements clause
    ArenaVector<ir::TSClassImplements *> implements(Allocator()->Adapter());
    if (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_IMPLEMENTS) {
        Lexer()->NextToken();
        implements = ParseClassImplementClause();
    }

    ExpectToken(lexer::TokenType::PUNCTUATOR_LEFT_BRACE, false);

    // Parse ClassBody
    auto [ctor, properties, bodyRange] = ParseClassBody(modifiers, flags, ident_node);
    CreateCCtor(properties, bodyRange.start);

    auto *class_definition = AllocNode<ir::ClassDefinition>(
        util::StringView(), ident_node, type_param_decl, superTypeParams, std::move(implements), ctor, superClass,
        std::move(properties), modifiers, flags, GetContext().GetLanguge());

    class_definition->SetRange(bodyRange);

    GetContext().Status() &= ~ParserStatus::ALLOW_SUPER;

    return class_definition;
}

static bool IsInterfaceMethodModifier(lexer::TokenType type)
{
    return type == lexer::TokenType::KEYW_STATIC || type == lexer::TokenType::KEYW_PRIVATE;
}

ir::ModifierFlags ETSParser::ParseInterfaceMethodModifiers()
{
    ir::ModifierFlags flags = ir::ModifierFlags::NONE;

    while (IsInterfaceMethodModifier(Lexer()->GetToken().Type())) {
        ir::ModifierFlags current_flag = ir::ModifierFlags::NONE;

        switch (Lexer()->GetToken().Type()) {
            case lexer::TokenType::KEYW_STATIC: {
                current_flag = ir::ModifierFlags::STATIC;
                break;
            }
            case lexer::TokenType::KEYW_PRIVATE: {
                current_flag = ir::ModifierFlags::PRIVATE;
                break;
            }
            default: {
                UNREACHABLE();
            }
        }

        char32_t next_cp = Lexer()->Lookahead();
        if (next_cp == lexer::LEX_CHAR_COLON || next_cp == lexer::LEX_CHAR_LEFT_PAREN ||
            next_cp == lexer::LEX_CHAR_EQUALS) {
            break;
        }

        if ((flags & current_flag) != 0) {
            ThrowSyntaxError("Duplicated modifier is not allowed");
        }

        Lexer()->NextToken();
        flags |= current_flag;
    }

    return flags;
}

ir::ClassProperty *ETSParser::ParseInterfaceField()
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_IDENT);
    auto *name = AllocNode<ir::Identifier>(Lexer()->GetToken().Ident(), Allocator());
    name->SetRange(Lexer()->GetToken().Loc());
    Lexer()->NextToken();

    ir::TypeNode *type_annotation = nullptr;
    if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_COLON) {
        ThrowSyntaxError("Interface fields must have typeannotation.");
    }
    Lexer()->NextToken();  // eat ':'
    TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR;
    type_annotation = ParseTypeAnnotation(&options);

    name->SetTsTypeAnnotation(type_annotation);

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_EQUAL) {
        ThrowSyntaxError("Initializers are not allowed on interface propertys.");
    }

    ir::ModifierFlags field_modifiers = ir::ModifierFlags::PUBLIC;

    if (InAmbientContext()) {
        field_modifiers |= ir::ModifierFlags::DECLARE;
    }

    auto *field = AllocNode<ir::ClassProperty>(name, nullptr, type_annotation, field_modifiers, Allocator(), false);
    field->SetEnd(Lexer()->GetToken().End());

    return field;
}

ir::MethodDefinition *ETSParser::ParseInterfaceMethod(ir::ModifierFlags flags, ir::MethodDefinitionKind method_kind)
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_IDENT);
    auto *name = AllocNode<ir::Identifier>(Lexer()->GetToken().Ident(), Allocator());
    name->SetRange(Lexer()->GetToken().Loc());
    Lexer()->NextToken();

    FunctionContext function_context(this, ParserStatus::FUNCTION);

    lexer::SourcePosition start_loc = Lexer()->GetToken().Start();

    auto [signature, throw_marker] = ParseFunctionSignature(ParserStatus::NEED_RETURN_TYPE);

    ir::BlockStatement *body = nullptr;

    bool is_declare = InAmbientContext();
    if (is_declare) {
        flags |= ir::ModifierFlags::DECLARE;
    }

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
        if (method_kind == ir::MethodDefinitionKind::SET || method_kind == ir::MethodDefinitionKind::GET) {
            ThrowSyntaxError("Getter and setter methods must be abstracts in the interface body", start_loc);
        }
        body = ParseBlockStatement();
    } else if ((flags & (ir::ModifierFlags::PRIVATE | ir::ModifierFlags::STATIC)) != 0 && !is_declare) {
        ThrowSyntaxError("Private or static interface methods must have body", start_loc);
    }

    function_context.AddFlag(throw_marker);

    auto *func = AllocNode<ir::ScriptFunction>(std::move(signature), body, function_context.Flags(), flags, true,
                                               GetContext().GetLanguge());

    if ((flags & ir::ModifierFlags::STATIC) == 0 && body == nullptr) {
        func->AddModifier(ir::ModifierFlags::ABSTRACT);
    }
    func->SetRange({start_loc, body != nullptr                           ? body->End()
                               : func->ReturnTypeAnnotation() != nullptr ? func->ReturnTypeAnnotation()->End()
                                                                         : (*func->Params().end())->End()});

    auto *func_expr = AllocNode<ir::FunctionExpression>(func);
    func_expr->SetRange(func->Range());
    func->AddFlag(ir::ScriptFunctionFlags::METHOD);

    func->SetIdent(name);
    auto *method =
        AllocNode<ir::MethodDefinition>(ir::MethodDefinitionKind::METHOD, name, func_expr, flags, Allocator(), false);
    method->SetRange(func_expr->Range());

    ConsumeSemicolon(method);

    return method;
}

std::pair<bool, std::size_t> ETSParser::CheckDefaultParameters(const ir::ScriptFunction *const function) const
{
    bool has_default_parameter = false;
    bool has_rest_parameter = false;
    std::size_t required_parameters_number = 0U;

    for (auto *const it : function->Params()) {
        auto const *const param = it->AsETSParameterExpression();

        if (param->IsRestParameter()) {
            has_rest_parameter = true;
            continue;
        }

        if (has_rest_parameter) {
            ThrowSyntaxError("Rest parameter should be the last one.", param->Start());
        }

        if (param->IsDefault()) {
            has_default_parameter = true;
            continue;
        }

        if (has_default_parameter) {
            ThrowSyntaxError("Required parameter follows default parameter(s).", param->Start());
        }

        ++required_parameters_number;
    }

    if (has_default_parameter && has_rest_parameter) {
        ThrowSyntaxError("Both optional and rest parameters are not allowed in function's parameter list.",
                         function->Start());
    }

    return std::make_pair(has_default_parameter, required_parameters_number);
}

ir::MethodDefinition *ETSParser::CreateProxyConstructorDefinition(ir::MethodDefinition const *const method)
{
    ASSERT(method->IsConstructor());

    const auto *const function = method->Function();
    std::string proxy_method = function->Id()->Name().Mutf8() + '(';

    for (const auto *const it : function->Params()) {
        auto const *const param = it->AsETSParameterExpression();
        proxy_method += param->Ident()->Name().Mutf8() + ": " + GetNameForTypeNode(param->TypeAnnotation()) + ", ";
    }

    proxy_method += ir::PROXY_PARAMETER_NAME;
    proxy_method += ": int) { this(";

    auto const parameters_number = function->Params().size();
    for (size_t i = 0U; i < parameters_number; ++i) {
        if (auto const *const param = function->Params()[i]->AsETSParameterExpression(); param->IsDefault()) {
            std::string proxy_if = "(((" + std::string {ir::PROXY_PARAMETER_NAME} + " >> " + std::to_string(i) +
                                   ") & 0x1) == 0) ? " + param->Ident()->Name().Mutf8() + " : (" +
                                   param->LexerSaved().Mutf8() + "), ";
            proxy_method += proxy_if;
        } else {
            proxy_method += function->Params()[i]->AsETSParameterExpression()->Ident()->Name().Mutf8() + ", ";
        }
    }

    proxy_method.pop_back();  // Note: at least one parameter always should present!
    proxy_method.pop_back();
    proxy_method += ") }";

    return CreateConstructorDefinition(method->Modifiers(), proxy_method, DEFAULT_PROXY_FILE);
}

ir::MethodDefinition *ETSParser::CreateProxyMethodDefinition(ir::MethodDefinition const *const method,
                                                             ir::Identifier const *const ident_node)
{
    ASSERT(!method->IsConstructor());

    const auto *const function = method->Function();
    std::string proxy_method = function->Id()->Name().Mutf8() + "_proxy(";

    for (const auto *const it : function->Params()) {
        auto const *const param = it->AsETSParameterExpression();
        proxy_method += param->Ident()->Name().Mutf8() + ": " + GetNameForTypeNode(param->TypeAnnotation()) + ", ";
    }

    const bool has_function_return_type = function->ReturnTypeAnnotation() != nullptr;
    const std::string return_type =
        has_function_return_type ? GetNameForTypeNode(function->ReturnTypeAnnotation()) : "";

    proxy_method += ir::PROXY_PARAMETER_NAME;
    proxy_method += ": int)";
    if (has_function_return_type) {
        proxy_method += ": " + return_type;
    }
    proxy_method += " { ";

    auto const parameters_number = function->Params().size();
    for (size_t i = 0U; i < parameters_number; ++i) {
        if (auto const *const param = function->Params()[i]->AsETSParameterExpression(); param->IsDefault()) {
            std::string proxy_if = "if (((" + std::string {ir::PROXY_PARAMETER_NAME} + " >> " + std::to_string(i) +
                                   ") & 0x1) == 1) { " + param->Ident()->Name().Mutf8() + " = " +
                                   param->LexerSaved().Mutf8() + " } ";
            proxy_method += proxy_if;
        }
    }

    proxy_method += ' ';
    if (return_type != "void") {
        proxy_method += "return ";
    }

    if (ident_node != nullptr) {
        if (method->IsStatic()) {
            ASSERT(ident_node != nullptr);
            proxy_method += ident_node->Name().Mutf8() + ".";
        } else {
            proxy_method += "this.";
        }
    }

    proxy_method += function->Id()->Name().Mutf8();
    proxy_method += '(';

    for (const auto *const it : function->Params()) {
        proxy_method += it->AsETSParameterExpression()->Ident()->Name().Mutf8() + ", ";
    }
    proxy_method.pop_back();
    proxy_method.pop_back();
    proxy_method += ") }";

    return CreateMethodDefinition(method->Modifiers(), proxy_method, DEFAULT_PROXY_FILE);
}

void ETSParser::AddProxyOverloadToMethodWithDefaultParams(ir::MethodDefinition *method, ir::Identifier *ident_node)
{
    if (auto const [has_default_parameters, required_parameters] = CheckDefaultParameters(method->Function());
        has_default_parameters) {
        if (ir::MethodDefinition *proxy_method_def = !method->IsConstructor()
                                                         ? CreateProxyMethodDefinition(method, ident_node)
                                                         : CreateProxyConstructorDefinition(method);
            proxy_method_def != nullptr) {
            auto *const proxy_param = proxy_method_def->Function()->Params().back()->AsETSParameterExpression();
            proxy_param->SetRequiredParams(required_parameters);

            proxy_method_def->Function()->SetDefaultParamProxy();
            proxy_method_def->Function()->AddFlag(ir::ScriptFunctionFlags::OVERLOAD);
            method->AddOverload(proxy_method_def);
            proxy_method_def->SetParent(method);
        }
    }
}

std::string ETSParser::PrimitiveTypeToName(ir::PrimitiveType type)
{
    switch (type) {
        case ir::PrimitiveType::BYTE:
            return "byte";
        case ir::PrimitiveType::INT:
            return "int";
        case ir::PrimitiveType::LONG:
            return "long";
        case ir::PrimitiveType::SHORT:
            return "short";
        case ir::PrimitiveType::FLOAT:
            return "float";
        case ir::PrimitiveType::DOUBLE:
            return "double";
        case ir::PrimitiveType::BOOLEAN:
            return "boolean";
        case ir::PrimitiveType::CHAR:
            return "char";
        case ir::PrimitiveType::VOID:
            return "void";
        default:
            UNREACHABLE();
    }
}
std::string ETSParser::GetNameForETSUnionType(const ir::TypeNode *type_annotation) const
{
    ASSERT(type_annotation->IsETSUnionType());
    std::string newstr;
    for (size_t i = 0; i < type_annotation->AsETSUnionType()->Types().size(); i++) {
        auto type = type_annotation->AsETSUnionType()->Types()[i];
        if (type->IsNullAssignable() || type->IsUndefinedAssignable()) {
            continue;
        }
        std::string str = GetNameForTypeNode(type, false);
        newstr += str;
        if (i != type_annotation->AsETSUnionType()->Types().size() - 1) {
            newstr += "|";
        }
    }
    if (type_annotation->IsNullAssignable()) {
        newstr += "|null";
    }
    if (type_annotation->IsUndefinedAssignable()) {
        newstr += "|undefined";
    }
    return newstr;
}

std::string ETSParser::GetNameForTypeNode(const ir::TypeNode *type_annotation, bool adjust) const
{
    if (type_annotation->IsETSUnionType()) {
        return GetNameForETSUnionType(type_annotation);
    }

    const auto adjust_nullish = [type_annotation, adjust](std::string const &s) {
        std::string newstr = s;
        if (type_annotation->IsNullAssignable() && adjust) {
            newstr += "|null";
        }
        if (type_annotation->IsUndefinedAssignable() && adjust) {
            newstr += "|undefined";
        }
        return newstr;
    };

    if (type_annotation->IsETSPrimitiveType()) {
        return adjust_nullish(PrimitiveTypeToName(type_annotation->AsETSPrimitiveType()->GetPrimitiveType()));
    }

    if (type_annotation->IsETSTypeReference()) {
        return adjust_nullish(type_annotation->AsETSTypeReference()->Part()->Name()->AsIdentifier()->Name().Mutf8());
    }

    if (type_annotation->IsETSFunctionType()) {
        std::string lambda_params = " ";

        for (const auto *const param : type_annotation->AsETSFunctionType()->Params()) {
            lambda_params += param->AsETSParameterExpression()->Ident()->Name().Mutf8();
            lambda_params += ":";
            lambda_params += GetNameForTypeNode(param->AsETSParameterExpression()->Ident()->TypeAnnotation());
            lambda_params += ",";
        }

        lambda_params.pop_back();
        const std::string return_type_name = GetNameForTypeNode(type_annotation->AsETSFunctionType()->ReturnType());

        return adjust_nullish("((" + lambda_params + ") => " + return_type_name + ")");
    }

    if (type_annotation->IsTSArrayType()) {
        // Note! array is required for the rest parameter.
        return GetNameForTypeNode(type_annotation->AsTSArrayType()->ElementType()) + "[]";
    }

    UNREACHABLE();
}

void ETSParser::ValidateRestParameter(ir::Expression *param)
{
    if (param->IsETSParameterExpression()) {
        if (param->AsETSParameterExpression()->IsRestParameter()) {
            GetContext().Status() |= ParserStatus::HAS_COMPLEX_PARAM;

            if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS) {
                ThrowSyntaxError("Rest parameter must be the last formal parameter.");
            }
        }
    }
}

ir::AstNode *ETSParser::ParseTypeLiteralOrInterfaceMember()
{
    auto start_loc = Lexer()->GetToken().Start();
    ir::ModifierFlags method_flags = ParseInterfaceMethodModifiers();

    if (method_flags != ir::ModifierFlags::NONE) {
        if ((method_flags & ir::ModifierFlags::PRIVATE) == 0) {
            method_flags |= ir::ModifierFlags::PUBLIC;
        }

        auto *method = ParseInterfaceMethod(method_flags, ir::MethodDefinitionKind::METHOD);
        method->SetStart(start_loc);
        return method;
    }

    if (Lexer()->Lookahead() != lexer::LEX_CHAR_LEFT_PAREN && Lexer()->Lookahead() != lexer::LEX_CHAR_LESS_THAN &&
        (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_GET ||
         Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_SET)) {
        return ParseInterfaceGetterSetterMethod(method_flags);
    }

    if (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_READONLY) {
        Lexer()->NextToken();  // eat 'readonly' keyword
        auto *field = ParseInterfaceField();
        field->SetStart(start_loc);
        field->AddModifier(ir::ModifierFlags::READONLY);
        return field;
    }

    if (Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_IDENT) {
        char32_t next_cp = Lexer()->Lookahead();

        if (next_cp == lexer::LEX_CHAR_LEFT_PAREN || next_cp == lexer::LEX_CHAR_LESS_THAN) {
            auto *method = ParseInterfaceMethod(ir::ModifierFlags::PUBLIC, ir::MethodDefinitionKind::METHOD);
            method->SetStart(start_loc);
            return method;
        }

        auto *field = ParseInterfaceField();
        field->SetStart(start_loc);
        return field;
    }

    return ParseTypeDeclaration(true);
}

std::tuple<ir::Expression *, ir::TSTypeParameterInstantiation *> ETSParser::ParseTypeReferencePart(
    TypeAnnotationParsingOptions *options)
{
    ExpressionParseFlags flags = ExpressionParseFlags::NO_OPTS;

    if (((*options) & TypeAnnotationParsingOptions::POTENTIAL_CLASS_LITERAL) != 0) {
        flags |= ExpressionParseFlags::POTENTIAL_CLASS_LITERAL;
    }

    auto *type_name = ParseQualifiedName(flags);
    if (type_name == nullptr) {
        return {nullptr, nullptr};
    }

    if (((*options) & TypeAnnotationParsingOptions::POTENTIAL_CLASS_LITERAL) != 0 &&
        (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_CLASS || IsStructKeyword())) {
        return {type_name, nullptr};
    }

    ir::TSTypeParameterInstantiation *type_param_inst = nullptr;
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_SHIFT ||
        Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LESS_THAN) {
        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_SHIFT) {
            Lexer()->BackwardToken(lexer::TokenType::PUNCTUATOR_LESS_THAN, 1);
        }
        *options |= TypeAnnotationParsingOptions::ALLOW_WILDCARD;
        type_param_inst = ParseTypeParameterInstantiation(options);
        *options &= ~TypeAnnotationParsingOptions::ALLOW_WILDCARD;
    }

    return {type_name, type_param_inst};
}

ir::TypeNode *ETSParser::ParseTypeReference(TypeAnnotationParsingOptions *options)
{
    auto start_pos = Lexer()->GetToken().Start();
    ir::ETSTypeReferencePart *type_ref_part = nullptr;

    while (true) {
        auto part_pos = Lexer()->GetToken().Start();
        auto [typeName, typeParams] = ParseTypeReferencePart(options);
        if (typeName == nullptr) {
            return nullptr;
        }

        type_ref_part = AllocNode<ir::ETSTypeReferencePart>(typeName, typeParams, type_ref_part);
        type_ref_part->SetRange({part_pos, Lexer()->GetToken().End()});

        if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_PERIOD) {
            break;
        }

        Lexer()->NextToken();

        if (((*options) & TypeAnnotationParsingOptions::POTENTIAL_CLASS_LITERAL) != 0 &&
            (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_CLASS || IsStructKeyword())) {
            break;
        }
    }

    auto *type_reference = AllocNode<ir::ETSTypeReference>(type_ref_part);
    type_reference->SetRange({start_pos, Lexer()->GetToken().End()});
    return type_reference;
}

ir::TypeNode *ETSParser::ParseBaseTypeReference(TypeAnnotationParsingOptions *options)
{
    ir::TypeNode *type_annotation = nullptr;

    switch (Lexer()->GetToken().KeywordType()) {
        case lexer::TokenType::KEYW_BOOLEAN: {
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::BOOLEAN);
            break;
        }
        case lexer::TokenType::KEYW_BYTE: {
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::BYTE);
            break;
        }
        case lexer::TokenType::KEYW_CHAR: {
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::CHAR);
            break;
        }
        case lexer::TokenType::KEYW_DOUBLE: {
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::DOUBLE);
            break;
        }
        case lexer::TokenType::KEYW_FLOAT: {
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::FLOAT);
            break;
        }
        case lexer::TokenType::KEYW_INT: {
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::INT);
            break;
        }
        case lexer::TokenType::KEYW_LONG: {
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::LONG);
            break;
        }
        case lexer::TokenType::KEYW_SHORT: {
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::SHORT);
            break;
        }

        default: {
            break;
        }
    }

    return type_annotation;
}

ir::TypeNode *ETSParser::ParsePrimitiveType(TypeAnnotationParsingOptions *options, ir::PrimitiveType type)
{
    if (((*options) & TypeAnnotationParsingOptions::DISALLOW_PRIMARY_TYPE) != 0) {
        ThrowSyntaxError("Primitive type is not allowed here.");
    }

    auto *type_annotation = AllocNode<ir::ETSPrimitiveType>(type);
    type_annotation->SetRange(Lexer()->GetToken().Loc());
    Lexer()->NextToken();
    return type_annotation;
}

ir::TypeNode *ETSParser::ParseUnionType(ir::TypeNode *const first_type)
{
    ArenaVector<ir::TypeNode *> types(Allocator()->Adapter());
    types.push_back(first_type->AsTypeNode());

    ir::ModifierFlags nullish_modifiers {};

    while (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_BITWISE_OR) {
        Lexer()->NextToken();  // eat '|'

        if (Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_NULL) {
            nullish_modifiers |= ir::ModifierFlags::NULL_ASSIGNABLE;
            Lexer()->NextToken();
        } else if (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_UNDEFINED) {
            nullish_modifiers |= ir::ModifierFlags::UNDEFINED_ASSIGNABLE;
            Lexer()->NextToken();
        } else {
            auto options = TypeAnnotationParsingOptions::THROW_ERROR | TypeAnnotationParsingOptions::DISALLOW_UNION;
            types.push_back(ParseTypeAnnotation(&options));
        }
    }

    lexer::SourcePosition const end_loc = types.back()->End();

    if (types.size() == 1) {  // Workaround until nullability is a typeflag
        first_type->AddModifier(nullish_modifiers);
        first_type->SetRange({first_type->Start(), end_loc});
        return first_type;
    }

    auto *const union_type = AllocNode<ir::ETSUnionType>(std::move(types));
    union_type->AddModifier(nullish_modifiers);
    union_type->SetRange({first_type->Start(), end_loc});
    return union_type;
}

ir::TSIntersectionType *ETSParser::ParseIntersectionType(ir::Expression *type)
{
    auto start_loc = type->Start();
    ArenaVector<ir::Expression *> types(Allocator()->Adapter());
    types.push_back(type);
    TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR;

    while (true) {
        if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_BITWISE_AND) {
            break;
        }

        Lexer()->NextToken();  // eat '&'
        types.push_back(ParseTypeReference(&options));
    }

    lexer::SourcePosition end_loc = types.back()->End();
    auto *intersection_type = AllocNode<ir::TSIntersectionType>(std::move(types));
    intersection_type->SetRange({start_loc, end_loc});
    return intersection_type;
}

ir::TypeNode *ETSParser::GetTypeAnnotationOfPrimitiveType([[maybe_unused]] lexer::TokenType token_type,
                                                          TypeAnnotationParsingOptions *options)
{
    ir::TypeNode *type_annotation = nullptr;
    switch (token_type) {
        case lexer::TokenType::KEYW_BOOLEAN:
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::BOOLEAN);
            break;
        case lexer::TokenType::KEYW_DOUBLE:
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::DOUBLE);
            break;
        case lexer::TokenType::KEYW_BYTE:
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::BYTE);
            break;
        case lexer::TokenType::KEYW_FLOAT:
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::FLOAT);
            break;
        case lexer::TokenType::KEYW_SHORT:
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::SHORT);
            break;
        case lexer::TokenType::KEYW_INT:
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::INT);
            break;
        case lexer::TokenType::KEYW_CHAR:
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::CHAR);
            break;
        case lexer::TokenType::KEYW_LONG:
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::LONG);
            break;
        default:
            type_annotation = ParseTypeReference(options);
            break;
    }
    return type_annotation;
}

ir::TypeNode *ETSParser::ParseWildcardType(TypeAnnotationParsingOptions *options)
{
    const auto variance_start_loc = Lexer()->GetToken().Start();
    const auto variance_end_loc = Lexer()->GetToken().End();
    const auto variance_modifier = ParseTypeVarianceModifier(options);

    auto *type_reference = [this, &variance_modifier, options]() -> ir::ETSTypeReference * {
        if (variance_modifier == ir::ModifierFlags::OUT &&
            (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_GREATER_THAN ||
             Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COMMA)) {
            // unbounded 'out'
            return nullptr;
        }
        return ParseTypeReference(options)->AsETSTypeReference();
    }();

    auto *wildcard_type = AllocNode<ir::ETSWildcardType>(type_reference, variance_modifier);
    wildcard_type->SetRange({variance_start_loc, type_reference == nullptr ? variance_end_loc : type_reference->End()});

    return wildcard_type;
}

ir::TypeNode *ETSParser::ParseFunctionType()
{
    auto start_loc = Lexer()->GetToken().Start();
    auto params = ParseFunctionParams();

    auto *const return_type_annotation = [this]() -> ir::TypeNode * {
        ExpectToken(lexer::TokenType::PUNCTUATOR_ARROW);
        TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR;
        return ParseTypeAnnotation(&options);
    }();

    ir::ScriptFunctionFlags throw_marker = ParseFunctionThrowMarker(false);

    auto *func_type = AllocNode<ir::ETSFunctionType>(
        ir::FunctionSignature(nullptr, std::move(params), return_type_annotation), throw_marker);
    const auto end_loc = return_type_annotation->End();
    func_type->SetRange({start_loc, end_loc});

    return func_type;
}

ir::TypeNode *ETSParser::ParseETSTupleType(TypeAnnotationParsingOptions *const options)
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET);

    const auto start_loc = Lexer()->GetToken().Start();
    Lexer()->NextToken();  // eat '['

    ArenaVector<ir::TypeNode *> tuple_type_list(Allocator()->Adapter());
    auto *const tuple_type = AllocNode<ir::ETSTuple>(Allocator());

    bool spread_type_present = false;

    while (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_SQUARE_BRACKET) {
        // Parse named parameter if name presents
        if ((Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_IDENT) &&
            (Lexer()->Lookahead() == lexer::LEX_CHAR_COLON)) {
            ExpectIdentifier();
            Lexer()->NextToken();  // eat ':'
        }

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_PERIOD_PERIOD_PERIOD) {
            if (spread_type_present) {
                ThrowSyntaxError("Only one spread type declaration allowed, at the last index");
            }

            spread_type_present = true;
            Lexer()->NextToken();  // eat '...'
        } else if (spread_type_present) {
            // This can't be implemented to any index, with type consistency. If a spread type is in the middle of the
            // tuple, then bounds check can't be made for element access, so the type of elements after the spread can't
            // be determined in compile time.
            ThrowSyntaxError("Spread type must be at the last index in the tuple type");
        }

        auto *const current_type_annotation = ParseTypeAnnotation(options);
        current_type_annotation->SetParent(tuple_type);

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_QUESTION_MARK) {
            // NOTE(mmartin): implement optional types for tuples
            ThrowSyntaxError("Optional types in tuples are not yet implemented.");
        }

        if (spread_type_present) {
            if (!current_type_annotation->IsTSArrayType()) {
                ThrowSyntaxError("Spread type must be an array type");
            }

            tuple_type->SetSpreadType(current_type_annotation);
        } else {
            tuple_type_list.push_back(current_type_annotation);
        }

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COMMA) {
            Lexer()->NextToken();  // eat comma
            continue;
        }

        if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_SQUARE_BRACKET) {
            ThrowSyntaxError("Comma is mandatory between elements in a tuple type declaration");
        }
    }

    Lexer()->NextToken();  // eat ']'

    tuple_type->SetTypeAnnotationsList(tuple_type_list);
    const auto end_loc = Lexer()->GetToken().End();
    tuple_type->SetRange({start_loc, end_loc});

    return tuple_type;
}

// Just to reduce the size of ParseTypeAnnotation(...) method
std::pair<ir::TypeNode *, bool> ETSParser::GetTypeAnnotationFromToken(TypeAnnotationParsingOptions *options)
{
    ir::TypeNode *type_annotation = nullptr;

    switch (Lexer()->GetToken().Type()) {
        case lexer::TokenType::LITERAL_IDENT: {
            if (const auto keyword = Lexer()->GetToken().KeywordType();
                keyword == lexer::TokenType::KEYW_IN || keyword == lexer::TokenType::KEYW_OUT) {
                type_annotation = ParseWildcardType(options);
            } else {
                if (Lexer()->GetToken().IsDefinableTypeName()) {
                    type_annotation = GetTypeAnnotationOfPrimitiveType(Lexer()->GetToken().KeywordType(), options);
                } else {
                    type_annotation = ParseTypeReference(options);
                }
            }

            if (((*options) & TypeAnnotationParsingOptions::POTENTIAL_CLASS_LITERAL) != 0 &&
                (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_CLASS || IsStructKeyword())) {
                return std::make_pair(type_annotation, false);
            }
            break;
        }
        case lexer::TokenType::KEYW_BOOLEAN: {
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::BOOLEAN);
            break;
        }
        case lexer::TokenType::KEYW_BYTE: {
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::BYTE);
            break;
        }
        case lexer::TokenType::KEYW_CHAR: {
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::CHAR);
            break;
        }
        case lexer::TokenType::KEYW_DOUBLE: {
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::DOUBLE);
            break;
        }
        case lexer::TokenType::KEYW_FLOAT: {
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::FLOAT);
            break;
        }
        case lexer::TokenType::KEYW_INT: {
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::INT);
            break;
        }
        case lexer::TokenType::KEYW_LONG: {
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::LONG);
            break;
        }
        case lexer::TokenType::KEYW_SHORT: {
            type_annotation = ParsePrimitiveType(options, ir::PrimitiveType::SHORT);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS: {
            auto start_loc = Lexer()->GetToken().Start();
            lexer::LexerPosition saved_pos = Lexer()->Save();
            Lexer()->NextToken();  // eat '('

            if (((*options) & TypeAnnotationParsingOptions::IGNORE_FUNCTION_TYPE) == 0 &&
                (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS ||
                 Lexer()->Lookahead() == lexer::LEX_CHAR_COLON)) {
                type_annotation = ParseFunctionType();
                type_annotation->SetStart(start_loc);
                return std::make_pair(type_annotation, false);
            }

            type_annotation = ParseTypeAnnotation(options);
            type_annotation->SetStart(start_loc);

            if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_BITWISE_OR) {
                type_annotation = ParseUnionType(type_annotation);
            }

            if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS) {
                if (((*options) & TypeAnnotationParsingOptions::THROW_ERROR) != 0) {
                    ThrowExpectedToken(lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS);
                }

                Lexer()->Rewind(saved_pos);
                type_annotation = nullptr;
            } else {
                Lexer()->NextToken();  // eat ')'
            }

            break;
        }
        case lexer::TokenType::PUNCTUATOR_FORMAT: {
            type_annotation = ParseTypeFormatPlaceholder();
            break;
        }
        case lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET: {
            type_annotation = ParseETSTupleType(options);
            break;
        }
        case lexer::TokenType::KEYW_THIS: {
            type_annotation = ParseThisType(options);
            break;
        }
        default: {
            break;
        }
    }

    return std::make_pair(type_annotation, true);
}

ir::TypeNode *ETSParser::ParseThisType(TypeAnnotationParsingOptions *options)
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::KEYW_THIS);

    // A syntax error should be thrown if
    // - the usage of 'this' as a type is not allowed in the current context, or
    // - 'this' is not used as a return type, or
    // - the current context is an arrow function (might be inside a method of a class where 'this' is allowed).
    if (((*options & TypeAnnotationParsingOptions::THROW_ERROR) != 0) &&
        (((GetContext().Status() & ParserStatus::ALLOW_THIS_TYPE) == 0) ||
         ((*options & TypeAnnotationParsingOptions::RETURN_TYPE) == 0) ||
         ((GetContext().Status() & ParserStatus::ARROW_FUNCTION) != 0))) {
        ThrowSyntaxError("A 'this' type is available only as return type in a non-static method of a class or struct.");
    }

    auto *this_type = AllocNode<ir::TSThisType>();
    this_type->SetRange(Lexer()->GetToken().Loc());

    Lexer()->NextToken();  // eat 'this'

    return this_type;
}

ir::TypeNode *ETSParser::ParseTypeAnnotation(TypeAnnotationParsingOptions *options)
{
    bool const throw_error = ((*options) & TypeAnnotationParsingOptions::THROW_ERROR) != 0;

    auto [type_annotation, need_further_processing] = GetTypeAnnotationFromToken(options);

    if (type_annotation == nullptr) {
        if (throw_error) {
            ThrowSyntaxError("Invalid Type");
        }
        return nullptr;
    }

    if (!need_further_processing) {
        return type_annotation;
    }

    const lexer::SourcePosition &start_pos = Lexer()->GetToken().Start();

    if (((*options) & TypeAnnotationParsingOptions::ALLOW_INTERSECTION) != 0 &&
        Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_BITWISE_AND) {
        if (type_annotation->IsETSPrimitiveType()) {
            if (throw_error) {
                ThrowSyntaxError("Invalid intersection type.");
            }
            return nullptr;
        }

        return ParseIntersectionType(type_annotation);
    }

    while (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET) {
        Lexer()->NextToken();  // eat '['

        if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_SQUARE_BRACKET) {
            if (throw_error) {
                ThrowExpectedToken(lexer::TokenType::PUNCTUATOR_RIGHT_SQUARE_BRACKET);
            }
            return nullptr;
        }

        Lexer()->NextToken();  // eat ']'
        type_annotation = AllocNode<ir::TSArrayType>(type_annotation);
        type_annotation->SetRange({start_pos, Lexer()->GetToken().End()});
    }

    if (((*options) & TypeAnnotationParsingOptions::DISALLOW_UNION) == 0 &&
        Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_BITWISE_OR) {
        return ParseUnionType(type_annotation);
    }

    return type_annotation;
}

void ETSParser::ThrowIfVarDeclaration(VariableParsingFlags flags)
{
    if ((flags & VariableParsingFlags::VAR) != 0) {
        ThrowUnexpectedToken(lexer::TokenType::KEYW_VAR);
    }
}

void ETSParser::ValidateForInStatement()
{
    ThrowUnexpectedToken(lexer::TokenType::KEYW_IN);
}

ir::DebuggerStatement *ETSParser::ParseDebuggerStatement()
{
    ThrowUnexpectedToken(lexer::TokenType::KEYW_DEBUGGER);
}

void ETSParser::ParseExport(lexer::SourcePosition start_loc)
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_MULTIPLY ||
           Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE);
    ArenaVector<ir::AstNode *> specifiers(Allocator()->Adapter());

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_MULTIPLY) {
        ParseNameSpaceSpecifier(&specifiers, true);
    } else {
        ParseNamedSpecifiers(&specifiers, true);

        if (Lexer()->GetToken().KeywordType() != lexer::TokenType::KEYW_FROM) {
            // selective export directive
            return;
        }
    }

    // re-export directive
    ir::ImportSource *re_export_source = nullptr;
    std::vector<std::string> user_paths;

    std::tie(re_export_source, user_paths) = ParseFromClause(true);

    lexer::SourcePosition end_loc = re_export_source->Source()->End();
    auto *re_export_declaration = AllocNode<ir::ETSImportDeclaration>(re_export_source, specifiers);
    re_export_declaration->SetRange({start_loc, end_loc});

    auto varbinder = GetProgram()->VarBinder()->AsETSBinder();
    if (re_export_declaration->Language().IsDynamic()) {
        varbinder->AddDynamicImport(re_export_declaration);
    }

    ConsumeSemicolon(re_export_declaration);

    auto *re_export = Allocator()->New<ir::ETSReExportDeclaration>(re_export_declaration, user_paths,
                                                                   GetProgram()->SourceFilePath(), Allocator());
    varbinder->AddReExportImport(re_export);
}

ir::Statement *ETSParser::ParseFunctionStatement([[maybe_unused]] const StatementParsingFlags flags)
{
    ASSERT((flags & StatementParsingFlags::GLOBAL) == 0);
    ThrowSyntaxError("Nested functions are not allowed");
}

void ETSParser::ParsePackageDeclaration(ArenaVector<ir::Statement *> &statements)
{
    auto start_loc = Lexer()->GetToken().Start();

    if (Lexer()->GetToken().Type() != lexer::TokenType::KEYW_PACKAGE) {
        if (!IsETSModule() && GetProgram()->IsEntryPoint()) {
            return;
        }

        auto base_name = GetProgram()->SourceFilePath().Utf8();
        base_name = base_name.substr(base_name.find_last_of(panda::os::file::File::GetPathDelim()) + 1);
        const size_t idx = base_name.find_last_of('.');
        if (idx != std::string::npos) {
            base_name = base_name.substr(0, idx);
        }

        GetProgram()->SetPackageName(base_name);

        return;
    }

    Lexer()->NextToken();

    ir::Expression *name = ParseQualifiedName();

    auto *package_declaration = AllocNode<ir::ETSPackageDeclaration>(name);
    package_declaration->SetRange({start_loc, Lexer()->GetToken().End()});

    ConsumeSemicolon(package_declaration);
    statements.push_back(package_declaration);

    if (name->IsIdentifier()) {
        GetProgram()->SetPackageName(name->AsIdentifier()->Name());
    } else {
        GetProgram()->SetPackageName(name->AsTSQualifiedName()->ToString(Allocator()));
    }
}

std::tuple<ir::ImportSource *, std::vector<std::string>> ETSParser::ParseFromClause(bool require_from)
{
    if (Lexer()->GetToken().KeywordType() != lexer::TokenType::KEYW_FROM) {
        if (require_from) {
            ThrowSyntaxError("Unexpected token.");
        }
    } else {
        Lexer()->NextToken();  // eat `from`
    }

    if (Lexer()->GetToken().Type() != lexer::TokenType::LITERAL_STRING) {
        ThrowSyntaxError("Unexpected token.");
    }

    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_STRING);
    std::vector<std::string> user_paths;
    bool is_module = false;
    auto import_path = Lexer()->GetToken().Ident();
    auto resolved_import_path = ResolveImportPath(import_path.Mutf8());
    resolved_parsed_sources_.emplace(import_path.Mutf8(), resolved_import_path);

    ir::StringLiteral *resolved_source;
    if (*import_path.Bytes() == '/') {
        resolved_source = AllocNode<ir::StringLiteral>(util::UString(resolved_import_path, Allocator()).View());
    } else {
        resolved_source = AllocNode<ir::StringLiteral>(import_path);
    }

    auto import_data = GetImportData(resolved_import_path);

    if ((GetContext().Status() & ParserStatus::IN_DEFAULT_IMPORTS) == 0) {
        std::tie(user_paths, is_module) = CollectUserSources(import_path.Mutf8());
    }

    ir::StringLiteral *module = nullptr;
    if (is_module) {
        auto pos = import_path.Mutf8().find_last_of(panda::os::file::File::GetPathDelim());

        util::UString base_name(import_path.Mutf8().substr(0, pos), Allocator());
        if (base_name.View().Is(".") || base_name.View().Is("..")) {
            base_name.Append(panda::os::file::File::GetPathDelim());
        }

        module = AllocNode<ir::StringLiteral>(util::UString(import_path.Mutf8().substr(pos + 1), Allocator()).View());
        import_path = base_name.View();
    }

    auto *source = AllocNode<ir::StringLiteral>(import_path);
    source->SetRange(Lexer()->GetToken().Loc());

    Lexer()->NextToken();

    auto *import_source =
        Allocator()->New<ir::ImportSource>(source, resolved_source, import_data.lang, import_data.has_decl, module);
    return {import_source, user_paths};
}

std::vector<std::string> ETSParser::ParseImportDeclarations(ArenaVector<ir::Statement *> &statements)
{
    std::vector<std::string> all_user_paths;
    std::vector<std::string> user_paths;
    ArenaVector<ir::ETSImportDeclaration *> imports(Allocator()->Adapter());

    while (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_IMPORT) {
        auto start_loc = Lexer()->GetToken().Start();
        Lexer()->NextToken();  // eat import

        ArenaVector<ir::AstNode *> specifiers(Allocator()->Adapter());
        ir::ImportSource *import_source = nullptr;

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_MULTIPLY) {
            ParseNameSpaceSpecifier(&specifiers);
        } else if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
            ParseNamedSpecifiers(&specifiers);
        } else {
            ParseImportDefaultSpecifier(&specifiers);
        }

        std::tie(import_source, user_paths) = ParseFromClause(true);

        all_user_paths.insert(all_user_paths.end(), user_paths.begin(), user_paths.end());
        lexer::SourcePosition end_loc = import_source->Source()->End();
        auto *import_declaration = AllocNode<ir::ETSImportDeclaration>(import_source, std::move(specifiers));
        import_declaration->SetRange({start_loc, end_loc});

        ConsumeSemicolon(import_declaration);

        statements.push_back(import_declaration);
        imports.push_back(import_declaration);
    }

    sort(statements.begin(), statements.end(), [](ir::Statement const *s1, ir::Statement const *s2) -> bool {
        return s1->IsETSImportDeclaration() && s2->IsETSImportDeclaration() &&
               s1->AsETSImportDeclaration()->Specifiers()[0]->IsImportNamespaceSpecifier() &&
               !s2->AsETSImportDeclaration()->Specifiers()[0]->IsImportNamespaceSpecifier();
    });

    if ((GetContext().Status() & ParserStatus::IN_DEFAULT_IMPORTS) != 0) {
        static_cast<varbinder::ETSBinder *>(GetProgram()->VarBinder())
            ->SetDefaultImports(std::move(imports));  // get rid of it
    }

    sort(all_user_paths.begin(), all_user_paths.end());
    all_user_paths.erase(unique(all_user_paths.begin(), all_user_paths.end()), all_user_paths.end());

    return all_user_paths;
}

void ETSParser::ParseNamedSpecifiers(ArenaVector<ir::AstNode *> *specifiers, bool is_export)
{
    lexer::SourcePosition start_loc = Lexer()->GetToken().Start();
    // NOTE(user): handle qualifiedName in file bindings: qualifiedName '.' '*'
    if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
        ThrowExpectedToken(lexer::TokenType::PUNCTUATOR_LEFT_BRACE);
    }
    Lexer()->NextToken();  // eat '{'

    auto file_name = GetProgram()->SourceFilePath().Mutf8();
    std::vector<util::StringView> exported_idents;

    while (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_BRACE) {
        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_MULTIPLY) {
            ThrowSyntaxError("The '*' token is not allowed as a selective binding (between braces)");
        }

        if (Lexer()->GetToken().Type() != lexer::TokenType::LITERAL_IDENT) {
            ThrowSyntaxError("Unexpected token");
        }

        lexer::Token imported_token = Lexer()->GetToken();
        auto *imported = AllocNode<ir::Identifier>(imported_token.Ident(), Allocator());
        ir::Identifier *local = nullptr;
        imported->SetRange(Lexer()->GetToken().Loc());

        Lexer()->NextToken();  // eat import/export name

        if (CheckModuleAsModifier() && Lexer()->GetToken().Type() == lexer::TokenType::KEYW_AS) {
            Lexer()->NextToken();  // eat `as` literal
            local = ParseNamedImport(Lexer()->GetToken());
            Lexer()->NextToken();  // eat local name
        } else {
            local = ParseNamedImport(imported_token);
        }

        auto *specifier = AllocNode<ir::ImportSpecifier>(imported, local);
        specifier->SetRange({imported->Start(), local->End()});

        util::Helpers::CheckImportedName(specifiers, specifier, file_name);

        if (is_export) {
            util::StringView member_name = local->Name();
            exported_idents.push_back(member_name);
        }
        specifiers->push_back(specifier);

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COMMA) {
            Lexer()->NextToken(lexer::NextTokenFlags::KEYWORD_TO_IDENT);  // eat comma
        }
    }

    Lexer()->NextToken();  // eat '}'

    if (is_export && Lexer()->GetToken().KeywordType() != lexer::TokenType::KEYW_FROM) {
        // update exported idents to export name map when it is not the case of re-export
        for (auto member_name : exported_idents) {
            export_name_map_.insert({member_name, start_loc});
        }
    }
}

void ETSParser::ParseNameSpaceSpecifier(ArenaVector<ir::AstNode *> *specifiers, bool is_re_export)
{
    lexer::SourcePosition namespace_start = Lexer()->GetToken().Start();
    Lexer()->NextToken();  // eat `*` character

    if (!CheckModuleAsModifier()) {
        ThrowSyntaxError("Unexpected token.");
    }

    auto *local = AllocNode<ir::Identifier>(util::StringView(""), Allocator());
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COMMA ||
        Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_FROM || is_re_export) {
        auto *specifier = AllocNode<ir::ImportNamespaceSpecifier>(local);
        specifier->SetRange({namespace_start, Lexer()->GetToken().End()});
        specifiers->push_back(specifier);
        return;
    }

    Lexer()->NextToken();  // eat `as` literal
    local = ParseNamedImport(Lexer()->GetToken());

    auto *specifier = AllocNode<ir::ImportNamespaceSpecifier>(local);
    specifier->SetRange({namespace_start, Lexer()->GetToken().End()});
    specifiers->push_back(specifier);

    Lexer()->NextToken();  // eat local name
}

ir::AstNode *ETSParser::ParseImportDefaultSpecifier(ArenaVector<ir::AstNode *> *specifiers)
{
    if (Lexer()->GetToken().Type() != lexer::TokenType::LITERAL_IDENT) {
        ThrowSyntaxError("Unexpected token, expected an identifier");
    }

    auto *imported = AllocNode<ir::Identifier>(Lexer()->GetToken().Ident(), Allocator());
    imported->SetRange(Lexer()->GetToken().Loc());
    Lexer()->NextToken();  // Eat import specifier.

    if (Lexer()->GetToken().KeywordType() != lexer::TokenType::KEYW_FROM) {
        ThrowSyntaxError("Unexpected token, expected 'from'");
    }

    auto *specifier = AllocNode<ir::ImportDefaultSpecifier>(imported);
    specifier->SetRange({imported->Start(), imported->End()});
    specifiers->push_back(specifier);

    return nullptr;
}

bool ETSParser::CheckModuleAsModifier()
{
    if ((Lexer()->GetToken().Flags() & lexer::TokenFlags::HAS_ESCAPE) != 0U) {
        ThrowSyntaxError("Escape sequences are not allowed in 'as' keyword");
    }

    return true;
}

ir::AnnotatedExpression *ETSParser::GetAnnotatedExpressionFromParam()
{
    ir::AnnotatedExpression *parameter;

    switch (Lexer()->GetToken().Type()) {
        case lexer::TokenType::LITERAL_IDENT: {
            parameter = AllocNode<ir::Identifier>(Lexer()->GetToken().Ident(), Allocator());
            if (parameter->AsIdentifier()->Decorators().empty()) {
                parameter->SetRange(Lexer()->GetToken().Loc());
            } else {
                parameter->SetRange(
                    {parameter->AsIdentifier()->Decorators().front()->Start(), Lexer()->GetToken().End()});
            }
            break;
        }

        case lexer::TokenType::PUNCTUATOR_PERIOD_PERIOD_PERIOD: {
            const auto start_loc = Lexer()->GetToken().Start();
            Lexer()->NextToken();

            if (Lexer()->GetToken().Type() != lexer::TokenType::LITERAL_IDENT) {
                ThrowSyntaxError("Unexpected token, expected an identifier.");
            }

            auto *const rest_ident = AllocNode<ir::Identifier>(Lexer()->GetToken().Ident(), Allocator());
            rest_ident->SetRange(Lexer()->GetToken().Loc());

            parameter = AllocNode<ir::SpreadElement>(ir::AstNodeType::REST_ELEMENT, Allocator(), rest_ident);
            parameter->SetRange({start_loc, Lexer()->GetToken().End()});
            break;
        }

        default: {
            ThrowSyntaxError("Unexpected token, expected an identifier.");
        }
    }

    Lexer()->NextToken();
    return parameter;
}

// NOLINTBEGIN(modernize-avoid-c-arrays)
static constexpr char const NO_DEFAULT_FOR_REST[] = "Rest parameter cannot have the default value.";
static constexpr char const ONLY_ARRAY_FOR_REST[] = "Rest parameter should be of an array type.";
static constexpr char const EXPLICIT_PARAM_TYPE[] = "Parameter declaration should have an explicit type annotation.";
// NOLINTEND(modernize-avoid-c-arrays)

ir::Expression *ETSParser::ParseFunctionParameter()
{
    ir::ETSParameterExpression *param_expression;
    auto *const param_ident = GetAnnotatedExpressionFromParam();

    bool default_undefined = false;
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_QUESTION_MARK) {
        if (param_ident->IsRestElement()) {
            ThrowSyntaxError(NO_DEFAULT_FOR_REST);
        }
        default_undefined = true;
        Lexer()->NextToken();  // eat '?'
    }

    const bool is_arrow = (GetContext().Status() & ParserStatus::ARROW_FUNCTION) != 0;

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COLON) {
        Lexer()->NextToken();  // eat ':'

        TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR;
        ir::TypeNode *type_annotation = ParseTypeAnnotation(&options);

        if (param_ident->IsRestElement() && !type_annotation->IsTSArrayType()) {
            ThrowSyntaxError(ONLY_ARRAY_FOR_REST);
        }

        type_annotation->SetParent(param_ident);
        param_ident->SetTsTypeAnnotation(type_annotation);
        param_ident->SetEnd(type_annotation->End());

    } else if (!is_arrow && !default_undefined) {
        ThrowSyntaxError(EXPLICIT_PARAM_TYPE);
    }

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
        if (param_ident->IsRestElement()) {
            ThrowSyntaxError(NO_DEFAULT_FOR_REST);
        }

        auto const lexer_pos = Lexer()->Save().Iterator();
        Lexer()->NextToken();  // eat '='

        if (default_undefined) {
            ThrowSyntaxError("Not enable default value with default undefined");
        }

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS ||
            Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COMMA) {
            ThrowSyntaxError("You didn't set the value.");
        }

        param_expression = AllocNode<ir::ETSParameterExpression>(param_ident->AsIdentifier(), ParseExpression());

        std::string value = Lexer()->SourceView(lexer_pos.Index(), Lexer()->Save().Iterator().Index()).Mutf8();
        while (value.back() == ' ') {
            value.pop_back();
        }
        if (value.back() == ')' || value.back() == ',') {
            value.pop_back();
        }
        param_expression->SetLexerSaved(util::UString(value, Allocator()).View());

        param_expression->SetRange({param_ident->Start(), param_expression->Initializer()->End()});
    } else if (param_ident->IsIdentifier()) {
        auto *const type_annotation = param_ident->AsIdentifier()->TypeAnnotation();

        const auto type_annotation_value = [this, type_annotation]() -> std::pair<ir::Expression *, std::string> {
            if (type_annotation == nullptr) {
                return std::make_pair(nullptr, "");
            }
            if (!type_annotation->IsETSPrimitiveType()) {
                return std::make_pair(AllocNode<ir::UndefinedLiteral>(), "undefined");
            }
            // NOTE(ttamas) : after nullable fix, fix this scope
            switch (type_annotation->AsETSPrimitiveType()->GetPrimitiveType()) {
                case ir::PrimitiveType::BYTE:
                case ir::PrimitiveType::INT:
                case ir::PrimitiveType::LONG:
                case ir::PrimitiveType::SHORT:
                case ir::PrimitiveType::FLOAT:
                case ir::PrimitiveType::DOUBLE:
                    return std::make_pair(AllocNode<ir::NumberLiteral>(lexer::Number(0)), "0");
                case ir::PrimitiveType::BOOLEAN:
                    return std::make_pair(AllocNode<ir::BooleanLiteral>(false), "false");
                case ir::PrimitiveType::CHAR:
                    return std::make_pair(AllocNode<ir::CharLiteral>(), "c'\\u0000'");
                default: {
                    UNREACHABLE();
                }
            }
        }();

        if (default_undefined && !type_annotation->IsETSPrimitiveType()) {
            type_annotation->AddModifier(ir::ModifierFlags::UNDEFINED_ASSIGNABLE);
        }

        param_expression = AllocNode<ir::ETSParameterExpression>(
            param_ident->AsIdentifier(), default_undefined ? std::get<0>(type_annotation_value) : nullptr);

        if (default_undefined) {
            param_expression->SetLexerSaved(util::UString(std::get<1>(type_annotation_value), Allocator()).View());
        }

        param_expression->SetRange({param_ident->Start(), param_ident->End()});
    } else {
        param_expression = AllocNode<ir::ETSParameterExpression>(param_ident->AsRestElement(), nullptr);
        param_expression->SetRange({param_ident->Start(), param_ident->End()});
    }

    return param_expression;
}

ir::Expression *ETSParser::CreateParameterThis(const util::StringView class_name)
{
    auto *param_ident = AllocNode<ir::Identifier>(varbinder::TypedBinder::MANDATORY_PARAM_THIS, Allocator());
    param_ident->SetRange(Lexer()->GetToken().Loc());

    ir::Expression *class_type_name = AllocNode<ir::Identifier>(class_name, Allocator());
    class_type_name->AsIdentifier()->SetReference();
    class_type_name->SetRange(Lexer()->GetToken().Loc());

    auto type_ref_part = AllocNode<ir::ETSTypeReferencePart>(class_type_name, nullptr, nullptr);
    ir::TypeNode *type_annotation = AllocNode<ir::ETSTypeReference>(type_ref_part);

    type_annotation->SetParent(param_ident);
    param_ident->SetTsTypeAnnotation(type_annotation);

    auto *param_expression = AllocNode<ir::ETSParameterExpression>(param_ident, nullptr);
    param_expression->SetRange({param_ident->Start(), param_ident->End()});

    return param_expression;
}

ir::AnnotatedExpression *ETSParser::ParseVariableDeclaratorKey([[maybe_unused]] VariableParsingFlags flags)
{
    ir::Identifier *init = ExpectIdentifier();
    ir::TypeNode *type_annotation = nullptr;

    if (auto const token_type = Lexer()->GetToken().Type(); token_type == lexer::TokenType::PUNCTUATOR_COLON) {
        Lexer()->NextToken();  // eat ':'
        TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR;
        type_annotation = ParseTypeAnnotation(&options);
    } else if (token_type != lexer::TokenType::PUNCTUATOR_SUBSTITUTION &&
               (flags & VariableParsingFlags::FOR_OF) == 0U) {
        ThrowSyntaxError("Variable must be initialized or it's type must be declared");
    }

    if (type_annotation != nullptr) {
        init->SetTsTypeAnnotation(type_annotation);
        type_annotation->SetParent(init);
    }

    return init;
}

ir::Expression *ETSParser::ParseInitializer()
{
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET) {
        return ParseArrayLiteral();
    }

    return ParseExpression();
}

ir::ArrayExpression *ETSParser::ParseArrayLiteral()
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET);

    lexer::SourcePosition start_loc = Lexer()->GetToken().Start();

    ArenaVector<ir::Expression *> elements(Allocator()->Adapter());

    Lexer()->NextToken();

    while (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_SQUARE_BRACKET) {
        elements.push_back(ParseInitializer());

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COMMA) {
            Lexer()->NextToken();  // eat comma
            continue;
        }

        if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_SQUARE_BRACKET) {
            ThrowSyntaxError("Comma is mandatory between elements in an array literal");
        }
    }

    auto *array_literal = AllocNode<ir::ArrayExpression>(std::move(elements), Allocator());
    array_literal->SetRange({start_loc, Lexer()->GetToken().End()});
    Lexer()->NextToken();

    return array_literal;
}

ir::VariableDeclarator *ETSParser::ParseVariableDeclaratorInitializer(ir::Expression *init, VariableParsingFlags flags,
                                                                      const lexer::SourcePosition &start_loc)
{
    if ((flags & VariableParsingFlags::DISALLOW_INIT) != 0) {
        ThrowSyntaxError("for-await-of loop variable declaration may not have an initializer");
    }

    Lexer()->NextToken();

    ir::Expression *initializer = ParseInitializer();

    lexer::SourcePosition end_loc = initializer->End();

    auto *declarator = AllocNode<ir::VariableDeclarator>(GetFlag(flags), init, initializer);
    declarator->SetRange({start_loc, end_loc});

    return declarator;
}

ir::VariableDeclarator *ETSParser::ParseVariableDeclarator(ir::Expression *init, lexer::SourcePosition start_loc,
                                                           VariableParsingFlags flags)
{
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
        return ParseVariableDeclaratorInitializer(init, flags, start_loc);
    }

    if ((flags & VariableParsingFlags::CONST) != 0 &&
        static_cast<uint32_t>(flags & VariableParsingFlags::ACCEPT_CONST_NO_INIT) == 0U) {
        ThrowSyntaxError("Missing initializer in const declaration");
    }

    if (init->AsIdentifier()->TypeAnnotation() == nullptr && (flags & VariableParsingFlags::FOR_OF) == 0U) {
        ThrowSyntaxError("Variable must be initialized or it's type must be declared");
    }

    lexer::SourcePosition end_loc = init->End();
    auto declarator = AllocNode<ir::VariableDeclarator>(GetFlag(flags), init);
    declarator->SetRange({start_loc, end_loc});

    return declarator;
}

ir::Statement *ETSParser::ParseAssertStatement()
{
    lexer::SourcePosition start_loc = Lexer()->GetToken().Start();
    Lexer()->NextToken();

    ir::Expression *test = ParseExpression();
    lexer::SourcePosition end_loc = test->End();
    ir::Expression *second = nullptr;

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COLON) {
        Lexer()->NextToken();  // eat ':'
        second = ParseExpression();
        end_loc = second->End();
    }

    auto *as_statement = AllocNode<ir::AssertStatement>(test, second);
    as_statement->SetRange({start_loc, end_loc});
    ConsumeSemicolon(as_statement);

    return as_statement;
}

ir::Expression *ETSParser::ParseCatchParam()
{
    if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS) {
        ThrowSyntaxError("Unexpected token, expected '('");
    }

    ir::AnnotatedExpression *param = nullptr;

    Lexer()->NextToken();  // eat left paren

    if (Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_IDENT) {
        CheckRestrictedBinding();
        param = ExpectIdentifier();
    } else {
        ThrowSyntaxError("Unexpected token in catch parameter, expected an identifier");
    }

    ParseCatchParamTypeAnnotation(param);

    if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS) {
        ThrowSyntaxError("Unexpected token, expected ')'");
    }

    Lexer()->NextToken();  // eat right paren

    return param;
}

void ETSParser::ParseCatchParamTypeAnnotation([[maybe_unused]] ir::AnnotatedExpression *param)
{
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COLON) {
        Lexer()->NextToken();  // eat ':'

        TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR;
        param->SetTsTypeAnnotation(ParseTypeAnnotation(&options));
    }

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
        ThrowSyntaxError("Catch clause variable cannot have an initializer");
    }
}

ir::Statement *ETSParser::ParseTryStatement()
{
    lexer::SourcePosition start_loc = Lexer()->GetToken().Start();
    Lexer()->NextToken();  // eat the 'try' keyword

    if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
        ThrowSyntaxError("Unexpected token, expected '{'");
    }

    ir::BlockStatement *body = ParseBlockStatement();

    ArenaVector<ir::CatchClause *> catch_clauses(Allocator()->Adapter());

    while (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_CATCH) {
        ir::CatchClause *clause {};

        clause = ParseCatchClause();

        catch_clauses.push_back(clause);
    }

    ir::BlockStatement *finalizer = nullptr;
    if (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_FINALLY) {
        Lexer()->NextToken();  // eat 'finally' keyword

        finalizer = ParseBlockStatement();
    }

    if (catch_clauses.empty() && finalizer == nullptr) {
        ThrowSyntaxError("A try statement should contain either finally clause or at least one catch clause.",
                         start_loc);
    }

    lexer::SourcePosition end_loc = finalizer != nullptr ? finalizer->End() : catch_clauses.back()->End();

    ArenaVector<std::pair<compiler::LabelPair, const ir::Statement *>> finalizer_insertions(Allocator()->Adapter());

    auto *try_statement = AllocNode<ir::TryStatement>(body, std::move(catch_clauses), finalizer, finalizer_insertions);
    try_statement->SetRange({start_loc, end_loc});
    ConsumeSemicolon(try_statement);

    return try_statement;
}

ir::Statement *ETSParser::ParseImportDeclaration([[maybe_unused]] StatementParsingFlags flags)
{
    char32_t next_char = Lexer()->Lookahead();
    if (next_char == lexer::LEX_CHAR_LEFT_PAREN || next_char == lexer::LEX_CHAR_DOT) {
        return ParseExpressionStatement();
    }

    lexer::SourcePosition start_loc = Lexer()->GetToken().Start();
    Lexer()->NextToken();  // eat import

    ArenaVector<ir::AstNode *> specifiers(Allocator()->Adapter());

    ir::ImportSource *import_source = nullptr;
    std::vector<std::string> user_paths;

    if (Lexer()->GetToken().Type() != lexer::TokenType::LITERAL_STRING) {
        ir::AstNode *ast_node = ParseImportSpecifiers(&specifiers);
        if (ast_node != nullptr) {
            ASSERT(ast_node->IsTSImportEqualsDeclaration());
            ast_node->SetRange({start_loc, Lexer()->GetToken().End()});
            ConsumeSemicolon(ast_node->AsTSImportEqualsDeclaration());
            return ast_node->AsTSImportEqualsDeclaration();
        }
        std::tie(import_source, user_paths) = ParseFromClause(true);
    } else {
        std::tie(import_source, user_paths) = ParseFromClause(false);
    }

    lexer::SourcePosition end_loc = import_source->Source()->End();
    auto *import_declaration = AllocNode<ir::ETSImportDeclaration>(import_source, std::move(specifiers));
    import_declaration->SetRange({start_loc, end_loc});

    ConsumeSemicolon(import_declaration);

    return import_declaration;
}

ir::Statement *ETSParser::ParseExportDeclaration([[maybe_unused]] StatementParsingFlags flags)
{
    ThrowUnexpectedToken(lexer::TokenType::KEYW_EXPORT);
}

// NOLINTNEXTLINE(google-default-arguments)
ir::Expression *ETSParser::ParseUnaryOrPrefixUpdateExpression(ExpressionParseFlags flags)
{
    switch (Lexer()->GetToken().Type()) {
        case lexer::TokenType::PUNCTUATOR_PLUS_PLUS:
        case lexer::TokenType::PUNCTUATOR_MINUS_MINUS:
        case lexer::TokenType::PUNCTUATOR_PLUS:
        case lexer::TokenType::PUNCTUATOR_MINUS:
        case lexer::TokenType::PUNCTUATOR_TILDE:
        case lexer::TokenType::PUNCTUATOR_DOLLAR_DOLLAR:
        case lexer::TokenType::PUNCTUATOR_EXCLAMATION_MARK: {
            break;
        }
        case lexer::TokenType::KEYW_LAUNCH: {
            return ParseLaunchExpression(flags);
        }
        default: {
            return ParseLeftHandSideExpression(flags);
        }
    }

    lexer::TokenType operator_type = Lexer()->GetToken().Type();
    auto start = Lexer()->GetToken().Start();
    Lexer()->NextToken();

    ir::Expression *argument = nullptr;
    switch (Lexer()->GetToken().Type()) {
        case lexer::TokenType::PUNCTUATOR_PLUS:
        case lexer::TokenType::PUNCTUATOR_MINUS:
        case lexer::TokenType::PUNCTUATOR_TILDE:
        case lexer::TokenType::PUNCTUATOR_EXCLAMATION_MARK:
        case lexer::TokenType::PUNCTUATOR_DOLLAR_DOLLAR:
        case lexer::TokenType::PUNCTUATOR_PLUS_PLUS:
        case lexer::TokenType::PUNCTUATOR_MINUS_MINUS: {
            argument = ParseUnaryOrPrefixUpdateExpression();
            break;
        }
        default: {
            argument = ParseLeftHandSideExpression(flags);
            break;
        }
    }

    if (lexer::Token::IsUpdateToken(operator_type)) {
        if (!argument->IsIdentifier() && !argument->IsMemberExpression()) {
            ThrowSyntaxError("Invalid left-hand side in prefix operation");
        }
    }

    lexer::SourcePosition end = argument->End();

    ir::Expression *return_expr = nullptr;
    if (lexer::Token::IsUpdateToken(operator_type)) {
        return_expr = AllocNode<ir::UpdateExpression>(argument, operator_type, true);
    } else {
        return_expr = AllocNode<ir::UnaryExpression>(argument, operator_type);
    }

    return_expr->SetRange({start, end});

    return return_expr;
}

// NOLINTNEXTLINE(google-default-arguments)
ir::Expression *ETSParser::ParseDefaultPrimaryExpression(ExpressionParseFlags flags)
{
    auto start_loc = Lexer()->GetToken().Start();
    auto saved_pos = Lexer()->Save();
    TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::POTENTIAL_CLASS_LITERAL |
                                           TypeAnnotationParsingOptions::IGNORE_FUNCTION_TYPE |
                                           TypeAnnotationParsingOptions::DISALLOW_UNION;
    ir::TypeNode *potential_type = ParseTypeAnnotation(&options);

    if (potential_type != nullptr) {
        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_PERIOD) {
            Lexer()->NextToken();  // eat '.'
        }

        if (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_CLASS || IsStructKeyword()) {
            Lexer()->NextToken();  // eat 'class' and 'struct'
            auto *class_literal = AllocNode<ir::ETSClassLiteral>(potential_type);
            class_literal->SetRange({start_loc, Lexer()->GetToken().End()});
            return class_literal;
        }
    }

    Lexer()->Rewind(saved_pos);

    Lexer()->NextToken();
    bool pretend_arrow = Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_ARROW;
    Lexer()->Rewind(saved_pos);

    if (Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_IDENT && !pretend_arrow) {
        return ParsePrimaryExpressionIdent(flags);
    }

    ThrowSyntaxError({"Unexpected token '", lexer::TokenToString(Lexer()->GetToken().Type()), "'."});
    return nullptr;
}

// NOLINTNEXTLINE(google-default-arguments)
ir::Expression *ETSParser::ParsePrimaryExpression(ExpressionParseFlags flags)
{
    switch (Lexer()->GetToken().Type()) {
        case lexer::TokenType::LITERAL_TRUE:
        case lexer::TokenType::LITERAL_FALSE: {
            return ParseBooleanLiteral();
        }
        case lexer::TokenType::LITERAL_NULL: {
            return ParseNullLiteral();
        }
        case lexer::TokenType::KEYW_UNDEFINED: {
            return ParseUndefinedLiteral();
        }
        case lexer::TokenType::LITERAL_NUMBER: {
            return ParseCoercedNumberLiteral();
        }
        case lexer::TokenType::LITERAL_STRING: {
            return ParseStringLiteral();
        }
        case lexer::TokenType::LITERAL_CHAR: {
            return ParseCharLiteral();
        }
        case lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS: {
            return ParseCoverParenthesizedExpressionAndArrowParameterList();
        }
        case lexer::TokenType::KEYW_THIS: {
            return ParseThisExpression();
        }
        case lexer::TokenType::KEYW_SUPER: {
            return ParseSuperExpression();
        }
        case lexer::TokenType::KEYW_NEW: {
            return ParseNewExpression();
        }
        case lexer::TokenType::KEYW_ASYNC: {
            return ParseAsyncExpression();
        }
        case lexer::TokenType::KEYW_AWAIT: {
            return ParseAwaitExpression();
        }
        case lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET: {
            return ParseArrayExpression(CarryPatternFlags(flags));
        }
        case lexer::TokenType::PUNCTUATOR_LEFT_BRACE: {
            return ParseObjectExpression(CarryPatternFlags(flags));
        }
        case lexer::TokenType::PUNCTUATOR_BACK_TICK: {
            return ParseTemplateLiteral();
        }
        case lexer::TokenType::KEYW_TYPE: {
            ThrowSyntaxError("Type alias is allowed only as top-level declaration");
        }
        case lexer::TokenType::PUNCTUATOR_FORMAT: {
            return ParseExpressionFormatPlaceholder();
        }
        default: {
            return ParseDefaultPrimaryExpression(flags);
        }
    }
}

bool ETSParser::IsArrowFunctionExpressionStart()
{
    const auto saved_pos = Lexer()->Save();
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS);
    Lexer()->NextToken();
    auto token_type = Lexer()->GetToken().Type();

    size_t open_brackets = 1;
    bool expect_identifier = true;
    while (token_type != lexer::TokenType::EOS && open_brackets > 0) {
        switch (token_type) {
            case lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS:
                --open_brackets;
                break;
            case lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS:
                ++open_brackets;
                break;
            case lexer::TokenType::PUNCTUATOR_COMMA:
                expect_identifier = true;
                break;
            case lexer::TokenType::PUNCTUATOR_SEMI_COLON:
                Lexer()->Rewind(saved_pos);
                return false;
            default:
                if (!expect_identifier) {
                    break;
                }
                if (token_type != lexer::TokenType::LITERAL_IDENT &&
                    token_type != lexer::TokenType::PUNCTUATOR_PERIOD_PERIOD_PERIOD) {
                    Lexer()->Rewind(saved_pos);
                    return false;
                }
                expect_identifier = false;
        }
        Lexer()->NextToken();
        token_type = Lexer()->GetToken().Type();
    }

    while (token_type != lexer::TokenType::EOS && token_type != lexer::TokenType::PUNCTUATOR_ARROW) {
        if (lexer::Token::IsPunctuatorToken(token_type) && token_type != lexer::TokenType::PUNCTUATOR_COLON &&
            token_type != lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET &&
            token_type != lexer::TokenType::PUNCTUATOR_RIGHT_SQUARE_BRACKET &&
            token_type != lexer::TokenType::PUNCTUATOR_LESS_THAN &&
            token_type != lexer::TokenType::PUNCTUATOR_GREATER_THAN &&
            token_type != lexer::TokenType::PUNCTUATOR_BITWISE_OR) {
            break;
        }
        Lexer()->NextToken();
        token_type = Lexer()->GetToken().Type();
    }
    Lexer()->Rewind(saved_pos);
    return token_type == lexer::TokenType::PUNCTUATOR_ARROW;
}

ir::ArrowFunctionExpression *ETSParser::ParseArrowFunctionExpression()
{
    auto new_status = ParserStatus::ARROW_FUNCTION;
    auto *func = ParseFunction(new_status);
    auto *arrow_func_node = AllocNode<ir::ArrowFunctionExpression>(Allocator(), func);
    arrow_func_node->SetRange(func->Range());
    return arrow_func_node;
}

ir::Expression *ETSParser::ParseCoverParenthesizedExpressionAndArrowParameterList()
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS);
    if (IsArrowFunctionExpressionStart()) {
        return ParseArrowFunctionExpression();
    }

    lexer::SourcePosition start = Lexer()->GetToken().Start();
    Lexer()->NextToken();

    ir::Expression *expr = ParseExpression(ExpressionParseFlags::ACCEPT_COMMA);

    if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS) {
        ThrowSyntaxError("Unexpected token, expected ')'");
    }

    expr->SetGrouped();
    expr->SetRange({start, Lexer()->GetToken().End()});
    Lexer()->NextToken();

    return expr;
}

bool ETSParser::ParsePotentialGenericFunctionCall(ir::Expression *primary_expr, ir::Expression **return_expression,
                                                  [[maybe_unused]] const lexer::SourcePosition &start_loc,
                                                  bool ignore_call_expression)
{
    if (Lexer()->Lookahead() == lexer::LEX_CHAR_LESS_THAN ||
        (!primary_expr->IsIdentifier() && !primary_expr->IsMemberExpression())) {
        return true;
    }

    const auto saved_pos = Lexer()->Save();

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_SHIFT) {
        Lexer()->BackwardToken(lexer::TokenType::PUNCTUATOR_LESS_THAN, 1);
    }

    TypeAnnotationParsingOptions options =
        TypeAnnotationParsingOptions::ALLOW_WILDCARD | TypeAnnotationParsingOptions::IGNORE_FUNCTION_TYPE;
    ir::TSTypeParameterInstantiation *type_params = ParseTypeParameterInstantiation(&options);

    if (type_params == nullptr) {
        Lexer()->Rewind(saved_pos);
        return true;
    }

    if (Lexer()->GetToken().Type() == lexer::TokenType::EOS) {
        ThrowSyntaxError("'(' expected");
    }

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS) {
        if (!ignore_call_expression) {
            *return_expression = ParseCallExpression(*return_expression, false, false);
            (*return_expression)->AsCallExpression()->SetTypeParams(type_params);
            return false;
        }

        return true;
    }

    Lexer()->Rewind(saved_pos);
    return true;
}

ir::Expression *ETSParser::ParsePostPrimaryExpression(ir::Expression *primary_expr, lexer::SourcePosition start_loc,
                                                      bool ignore_call_expression,
                                                      [[maybe_unused]] bool *is_chain_expression)
{
    ir::Expression *return_expression = primary_expr;

    while (true) {
        switch (Lexer()->GetToken().Type()) {
            case lexer::TokenType::PUNCTUATOR_QUESTION_DOT: {
                Lexer()->NextToken();  // eat ?.

                if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET) {
                    return_expression = ParseElementAccess(return_expression, true);
                    continue;
                }

                if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS) {
                    return_expression = ParseCallExpression(return_expression, true, false);
                    continue;
                }

                return_expression = ParsePropertyAccess(return_expression, true);
                continue;
            }
            case lexer::TokenType::PUNCTUATOR_PERIOD: {
                Lexer()->NextToken();  // eat period

                return_expression = ParsePropertyAccess(return_expression);
                continue;
            }
            case lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET: {
                return_expression = ParseElementAccess(return_expression);
                continue;
            }
            case lexer::TokenType::PUNCTUATOR_LEFT_SHIFT:
            case lexer::TokenType::PUNCTUATOR_LESS_THAN: {
                if (ParsePotentialGenericFunctionCall(return_expression, &return_expression, start_loc,
                                                      ignore_call_expression)) {
                    break;
                }

                continue;
            }
            case lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS: {
                if (ignore_call_expression) {
                    break;
                }

                return_expression = ParseCallExpression(return_expression, false, false);
                continue;
            }
            case lexer::TokenType::PUNCTUATOR_EXCLAMATION_MARK: {
                const bool should_break = ParsePotentialNonNullExpression(&return_expression, start_loc);

                if (should_break) {
                    break;
                }

                continue;
            }
            case lexer::TokenType::PUNCTUATOR_FORMAT: {
                ThrowUnexpectedToken(lexer::TokenType::PUNCTUATOR_FORMAT);
            }
            default: {
                break;
            }
        }

        break;
    }

    return return_expression;
}

ir::Expression *ETSParser::ParsePotentialAsExpression(ir::Expression *primary_expr)
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::KEYW_AS);
    Lexer()->NextToken();

    TypeAnnotationParsingOptions options =
        TypeAnnotationParsingOptions::THROW_ERROR | TypeAnnotationParsingOptions::ALLOW_INTERSECTION;
    ir::TypeNode *type = ParseTypeAnnotation(&options);

    auto *as_expression = AllocNode<ir::TSAsExpression>(primary_expr, type, false);
    as_expression->SetRange(primary_expr->Range());
    return as_expression;
}

ir::Expression *ETSParser::ParseNewExpression()
{
    lexer::SourcePosition start = Lexer()->GetToken().Start();

    Lexer()->NextToken();  // eat new

    TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR;
    ir::TypeNode *base_type_reference = ParseBaseTypeReference(&options);
    ir::TypeNode *type_reference = base_type_reference;
    if (type_reference == nullptr) {
        options |= TypeAnnotationParsingOptions::IGNORE_FUNCTION_TYPE | TypeAnnotationParsingOptions::ALLOW_WILDCARD;
        type_reference = ParseTypeReference(&options);
    } else if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
        ThrowSyntaxError("Invalid { after base types.");
    }

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET) {
        Lexer()->NextToken();
        ir::Expression *dimension = ParseExpression();

        auto end_loc = Lexer()->GetToken().End();
        ExpectToken(lexer::TokenType::PUNCTUATOR_RIGHT_SQUARE_BRACKET);

        if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET) {
            auto *arr_instance = AllocNode<ir::ETSNewArrayInstanceExpression>(Allocator(), type_reference, dimension);
            arr_instance->SetRange({start, end_loc});
            return arr_instance;
        }

        ArenaVector<ir::Expression *> dimensions(Allocator()->Adapter());
        dimensions.push_back(dimension);

        do {
            Lexer()->NextToken();
            dimensions.push_back(ParseExpression());

            end_loc = Lexer()->GetToken().End();
            ExpectToken(lexer::TokenType::PUNCTUATOR_RIGHT_SQUARE_BRACKET);
        } while (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET);

        auto *multi_array = AllocNode<ir::ETSNewMultiDimArrayInstanceExpression>(type_reference, std::move(dimensions));
        multi_array->SetRange({start, end_loc});
        return multi_array;
    }

    ArenaVector<ir::Expression *> arguments(Allocator()->Adapter());
    lexer::SourcePosition end_loc = type_reference->End();

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS) {
        if (base_type_reference != nullptr) {
            ThrowSyntaxError("Can not use 'new' on primitive types.", base_type_reference->Start());
        }

        Lexer()->NextToken();

        while (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS) {
            ir::Expression *argument = ParseExpression();
            arguments.push_back(argument);

            if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COMMA) {
                Lexer()->NextToken();
                continue;
            }
        }

        end_loc = Lexer()->GetToken().End();
        Lexer()->NextToken();
    }

    ir::ClassDefinition *class_definition {};

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
        ArenaVector<ir::TSClassImplements *> implements(Allocator()->Adapter());
        auto modifiers = ir::ClassDefinitionModifiers::ANONYMOUS | ir::ClassDefinitionModifiers::HAS_SUPER;
        auto [ctor, properties, bodyRange] = ParseClassBody(modifiers);

        auto new_ident = AllocNode<ir::Identifier>("#0", Allocator());
        class_definition = AllocNode<ir::ClassDefinition>(
            "#0", new_ident, nullptr, nullptr, std::move(implements), ctor,  // remove name
            type_reference, std::move(properties), modifiers, ir::ModifierFlags::NONE, Language(Language::Id::ETS));

        class_definition->SetRange(bodyRange);
    }

    auto *new_expr_node =
        AllocNode<ir::ETSNewClassInstanceExpression>(type_reference, std::move(arguments), class_definition);
    new_expr_node->SetRange({start, Lexer()->GetToken().End()});

    return new_expr_node;
}

ir::Expression *ETSParser::ParseAsyncExpression()
{
    Lexer()->NextToken();  // eat 'async'
    if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS ||
        !IsArrowFunctionExpressionStart()) {
        ThrowSyntaxError("Unexpected token. expected '('");
    }

    auto new_status = ParserStatus::NEED_RETURN_TYPE | ParserStatus::ARROW_FUNCTION | ParserStatus::ASYNC_FUNCTION;
    auto *func = ParseFunction(new_status);
    auto *arrow_func_node = AllocNode<ir::ArrowFunctionExpression>(Allocator(), func);
    arrow_func_node->SetRange(func->Range());
    return arrow_func_node;
}

ir::Expression *ETSParser::ParseAwaitExpression()
{
    lexer::SourcePosition start = Lexer()->GetToken().Start();
    Lexer()->NextToken();
    ir::Expression *argument = ParseExpression();
    auto *await_expression = AllocNode<ir::AwaitExpression>(argument);
    await_expression->SetRange({start, Lexer()->GetToken().End()});
    return await_expression;
}

ir::ModifierFlags ETSParser::ParseTypeVarianceModifier(TypeAnnotationParsingOptions *const options)
{
    if ((*options & TypeAnnotationParsingOptions::ALLOW_WILDCARD) == 0 &&
        (*options & TypeAnnotationParsingOptions::ALLOW_DECLARATION_SITE_VARIANCE) == 0) {
        ThrowSyntaxError("Variance modifier is not allowed here.");
    }

    switch (Lexer()->GetToken().KeywordType()) {
        case lexer::TokenType::KEYW_IN: {
            Lexer()->NextToken();
            return ir::ModifierFlags::IN;
        }
        case lexer::TokenType::KEYW_OUT: {
            Lexer()->NextToken();
            return ir::ModifierFlags::OUT;
        }
        default: {
            return ir::ModifierFlags::NONE;
        }
    }
}

ir::TSTypeParameter *ETSParser::ParseTypeParameter([[maybe_unused]] TypeAnnotationParsingOptions *options)
{
    lexer::SourcePosition start_loc = Lexer()->GetToken().Start();

    const auto variance_modifier = [this, options] {
        switch (Lexer()->GetToken().KeywordType()) {
            case lexer::TokenType::KEYW_IN:
            case lexer::TokenType::KEYW_OUT:
                return ParseTypeVarianceModifier(options);
            default:
                return ir::ModifierFlags::NONE;
        }
    }();

    auto *param_ident = ExpectIdentifier();

    ir::TypeNode *constraint = nullptr;
    if (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_EXTENDS) {
        Lexer()->NextToken();
        TypeAnnotationParsingOptions new_options = TypeAnnotationParsingOptions::THROW_ERROR |
                                                   TypeAnnotationParsingOptions::ALLOW_INTERSECTION |
                                                   TypeAnnotationParsingOptions::IGNORE_FUNCTION_TYPE;
        constraint = ParseTypeAnnotation(&new_options);
    }

    auto *type_param = AllocNode<ir::TSTypeParameter>(param_ident, constraint, nullptr, variance_modifier);
    type_param->SetRange({start_loc, Lexer()->GetToken().End()});
    return type_param;
}

// NOLINTBEGIN(cert-err58-cpp)
static std::string const DUPLICATE_ENUM_VALUE = "Duplicate enum initialization value "s;
static std::string const INVALID_ENUM_TYPE = "Invalid enum initialization type"s;
static std::string const INVALID_ENUM_VALUE = "Invalid enum initialization value"s;
static std::string const MISSING_COMMA_IN_ENUM = "Missing comma between enum constants"s;
static std::string const TRAILING_COMMA_IN_ENUM = "Trailing comma is not allowed in enum constant list"s;
// NOLINTEND(cert-err58-cpp)

ir::TSEnumDeclaration *ETSParser::ParseEnumMembers(ir::Identifier *const key, const lexer::SourcePosition &enum_start,
                                                   const bool is_const, const bool is_static)
{
    if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
        ThrowSyntaxError("'{' expected");
    }

    Lexer()->NextToken(lexer::NextTokenFlags::KEYWORD_TO_IDENT);  // eat '{'

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_RIGHT_BRACE) {
        ThrowSyntaxError("An enum must have at least one enum constant");
    }

    // Lambda to check if enum underlying type is string:
    auto const is_string_enum = [this]() -> bool {
        Lexer()->NextToken();
        auto token_type = Lexer()->GetToken().Type();
        while (token_type != lexer::TokenType::PUNCTUATOR_RIGHT_BRACE &&
               token_type != lexer::TokenType::PUNCTUATOR_COMMA) {
            if (token_type == lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
                Lexer()->NextToken();
                if (Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_STRING) {
                    return true;
                }
            }
            Lexer()->NextToken();
            token_type = Lexer()->GetToken().Type();
        }
        return false;
    };

    // Get the underlying type of enum (number or string). It is defined from the first element ONLY!
    auto const pos = Lexer()->Save();
    auto const string_type_enum = is_string_enum();
    Lexer()->Rewind(pos);

    ArenaVector<ir::AstNode *> members(Allocator()->Adapter());

    if (string_type_enum) {
        ParseStringEnum(members);
    } else {
        ParseNumberEnum(members);
    }

    auto *const enum_declaration =
        AllocNode<ir::TSEnumDeclaration>(Allocator(), key, std::move(members), is_const, is_static);
    enum_declaration->SetRange({enum_start, Lexer()->GetToken().End()});

    Lexer()->NextToken();  // eat '}'

    return enum_declaration;
}

void ETSParser::ParseNumberEnum(ArenaVector<ir::AstNode *> &members)
{
    checker::ETSEnumType::ValueType current_value {};

    // Lambda to parse enum member (maybe with initializer)
    auto const parse_member = [this, &members, &current_value]() {
        auto *const ident = ExpectIdentifier(false, true);

        ir::NumberLiteral *ordinal;
        lexer::SourcePosition end_loc;

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
            // Case when user explicitly set the value for enumeration constant

            bool minus_sign = false;

            Lexer()->NextToken();
            if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_PLUS) {
                Lexer()->NextToken();
            } else if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_MINUS) {
                minus_sign = true;
                Lexer()->NextToken();
            }

            if (Lexer()->GetToken().Type() != lexer::TokenType::LITERAL_NUMBER) {
                ThrowSyntaxError(INVALID_ENUM_TYPE);
            }

            ordinal = ParseNumberLiteral()->AsNumberLiteral();
            if (!ordinal->Number().CanGetValue<checker::ETSEnumType::ValueType>()) {
                ThrowSyntaxError(INVALID_ENUM_VALUE);
            } else if (minus_sign) {
                ordinal->Number().Negate();
            }

            current_value = ordinal->Number().GetValue<checker::ETSEnumType::ValueType>();

            end_loc = ordinal->End();
        } else {
            // Default enumeration constant value. Equal to 0 for the first item and = previous_value + 1 for all
            // the others.

            ordinal = AllocNode<ir::NumberLiteral>(lexer::Number(current_value));

            end_loc = ident->End();
        }

        auto *const member = AllocNode<ir::TSEnumMember>(ident, ordinal);
        member->SetRange({ident->Start(), end_loc});
        members.emplace_back(member);

        ++current_value;
    };

    parse_member();

    while (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_BRACE) {
        if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_COMMA) {
            ThrowSyntaxError(MISSING_COMMA_IN_ENUM);
        }

        Lexer()->NextToken(lexer::NextTokenFlags::KEYWORD_TO_IDENT);  // eat ','

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_RIGHT_BRACE) {
            break;
        }

        parse_member();
    }
}

void ETSParser::ParseStringEnum(ArenaVector<ir::AstNode *> &members)
{
    // Lambda to parse enum member (maybe with initializer)
    auto const parse_member = [this, &members]() {
        auto *const ident = ExpectIdentifier();

        ir::StringLiteral *item_value;

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
            // Case when user explicitly set the value for enumeration constant

            Lexer()->NextToken();
            if (Lexer()->GetToken().Type() != lexer::TokenType::LITERAL_STRING) {
                ThrowSyntaxError(INVALID_ENUM_TYPE);
            }

            item_value = ParseStringLiteral();
        } else {
            // Default item value is not allowed for string type enumerations!
            ThrowSyntaxError("All items of string-type enumeration should be explicitly initialized.");
        }

        auto *const member = AllocNode<ir::TSEnumMember>(ident, item_value);
        member->SetRange({ident->Start(), item_value->End()});
        members.emplace_back(member);
    };

    parse_member();

    while (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_BRACE) {
        if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_COMMA) {
            ThrowSyntaxError(MISSING_COMMA_IN_ENUM);
        }

        Lexer()->NextToken(lexer::NextTokenFlags::KEYWORD_TO_IDENT);  // eat ','

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_RIGHT_BRACE) {
            ThrowSyntaxError(TRAILING_COMMA_IN_ENUM);
        }

        parse_member();
    }
}

ir::ThisExpression *ETSParser::ParseThisExpression()
{
    auto *this_expression = TypedParser::ParseThisExpression();

    if (Lexer()->GetToken().NewLine()) {
        return this_expression;
    }

    switch (Lexer()->GetToken().Type()) {
        case lexer::TokenType::PUNCTUATOR_PERIOD:
        case lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS:
        case lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS:
        case lexer::TokenType::PUNCTUATOR_SEMI_COLON:
        case lexer::TokenType::PUNCTUATOR_COLON:
        case lexer::TokenType::PUNCTUATOR_EQUAL:
        case lexer::TokenType::PUNCTUATOR_NOT_EQUAL:
        case lexer::TokenType::PUNCTUATOR_STRICT_EQUAL:
        case lexer::TokenType::PUNCTUATOR_NOT_STRICT_EQUAL:
        case lexer::TokenType::PUNCTUATOR_COMMA:
        case lexer::TokenType::PUNCTUATOR_QUESTION_MARK:
        case lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET:
        case lexer::TokenType::KEYW_AS: {
            break;
        }
        default: {
            ThrowUnexpectedToken(Lexer()->GetToken().Type());
            break;
        }
    }

    return this_expression;
}

ir::Identifier *ETSParser::ParseClassIdent([[maybe_unused]] ir::ClassDefinitionModifiers modifiers)
{
    return ExpectIdentifier(false, true);
}

// NOLINTNEXTLINE(google-default-arguments)
ir::ClassDeclaration *ETSParser::ParseClassStatement([[maybe_unused]] StatementParsingFlags flags,
                                                     [[maybe_unused]] ir::ClassDefinitionModifiers modifiers,
                                                     [[maybe_unused]] ir::ModifierFlags mod_flags)
{
    ThrowSyntaxError("Illegal start of expression", Lexer()->GetToken().Start());
}

// NOLINTNEXTLINE(google-default-arguments)
ir::ETSStructDeclaration *ETSParser::ParseStructStatement([[maybe_unused]] StatementParsingFlags flags,
                                                          [[maybe_unused]] ir::ClassDefinitionModifiers modifiers,
                                                          [[maybe_unused]] ir::ModifierFlags mod_flags)
{
    ThrowSyntaxError("Illegal start of expression", Lexer()->GetToken().Start());
}

bool ETSParser::CheckClassElement(ir::AstNode *property, [[maybe_unused]] ir::MethodDefinition *&ctor,
                                  [[maybe_unused]] ArenaVector<ir::AstNode *> &properties)
{
    if (property->IsClassStaticBlock()) {
        if (std::any_of(properties.cbegin(), properties.cend(),
                        [](const auto *prop) { return prop->IsClassStaticBlock(); })) {
            ThrowSyntaxError("Only one static block is allowed", property->Start());
        }

        auto *id = AllocNode<ir::Identifier>(compiler::Signatures::CCTOR, Allocator());
        property->AsClassStaticBlock()->Function()->SetIdent(id);
    }

    if (property->IsTSInterfaceBody()) {
        return CheckClassElementInterfaceBody(property, properties);
    }

    if (!property->IsMethodDefinition()) {
        return false;
    }

    auto const *const method = property->AsMethodDefinition();
    auto const *const function = method->Function();

    //  Check the special '$_get' and '$_set' methods using for object's index access
    if (method->Kind() == ir::MethodDefinitionKind::METHOD) {
        CheckIndexAccessMethod(function, property->Start());
    }

    return false;  // resolve overloads later on scopes stage
}

void ETSParser::CheckIndexAccessMethod(ir::ScriptFunction const *function, const lexer::SourcePosition &position) const
{
    auto const name = function->Id()->Name();

    if (name.Is(compiler::Signatures::GET_INDEX_METHOD)) {
        if (function->IsAsyncFunc()) {
            ThrowSyntaxError(std::string {ir::INDEX_ACCESS_ERROR_1} + std::string {name.Utf8()} +
                                 std::string {ir::INDEX_ACCESS_ERROR_2},
                             position);
        }

        bool is_valid = function->Params().size() == 1U;
        if (is_valid) {
            auto const *const param = function->Params()[0]->AsETSParameterExpression();
            is_valid = !param->IsDefault() && !param->IsRestParameter();
        }

        if (!is_valid) {
            ThrowSyntaxError(std::string {ir::INDEX_ACCESS_ERROR_1} + std::string {name.Utf8()} +
                                 std::string {"' should have exactly one required parameter."},
                             position);
        }
    } else if (name.Is(compiler::Signatures::SET_INDEX_METHOD)) {
        if (function->IsAsyncFunc()) {
            ThrowSyntaxError(std::string {ir::INDEX_ACCESS_ERROR_1} + std::string {name.Utf8()} +
                                 std::string {ir::INDEX_ACCESS_ERROR_2},
                             position);
        }

        bool is_valid = function->Params().size() == 2U;
        if (is_valid) {
            auto const *const param1 = function->Params()[0]->AsETSParameterExpression();
            auto const *const param2 = function->Params()[1]->AsETSParameterExpression();
            is_valid = !param1->IsDefault() && !param1->IsRestParameter() && !param2->IsDefault() &&
                       !param2->IsRestParameter();
        }

        if (!is_valid) {
            ThrowSyntaxError(std::string {ir::INDEX_ACCESS_ERROR_1} + std::string {name.Utf8()} +
                                 std::string {"' should have exactly two required parameters."},
                             position);
        }
    }
}

void ETSParser::CreateImplicitConstructor([[maybe_unused]] ir::MethodDefinition *&ctor,
                                          ArenaVector<ir::AstNode *> &properties,
                                          [[maybe_unused]] ir::ClassDefinitionModifiers modifiers,
                                          const lexer::SourcePosition &start_loc)
{
    if (std::any_of(properties.cbegin(), properties.cend(), [](ir::AstNode *prop) {
            return prop->IsMethodDefinition() && prop->AsMethodDefinition()->IsConstructor();
        })) {
        return;
    }

    if ((modifiers & ir::ClassDefinitionModifiers::ANONYMOUS) != 0) {
        return;
    }

    auto *method_def = BuildImplicitConstructor(ir::ClassDefinitionModifiers::SET_CTOR_ID, start_loc);
    properties.push_back(method_def);
}

ir::Expression *ETSParser::ParsePotentialExpressionSequence(ir::Expression *expr, ExpressionParseFlags flags)
{
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COMMA &&
        (flags & ExpressionParseFlags::ACCEPT_COMMA) != 0 && (flags & ExpressionParseFlags::IN_FOR) != 0U) {
        return ParseSequenceExpression(expr, (flags & ExpressionParseFlags::ACCEPT_REST) != 0);
    }

    return expr;
}

bool ETSParser::ParsePotentialNonNullExpression(ir::Expression **expression, const lexer::SourcePosition start_loc)
{
    if (expression == nullptr || Lexer()->GetToken().NewLine()) {
        return true;
    }

    const auto non_null_expr = AllocNode<ir::TSNonNullExpression>(*expression);
    non_null_expr->SetRange({start_loc, Lexer()->GetToken().End()});
    non_null_expr->SetParent(*expression);

    *expression = non_null_expr;

    Lexer()->NextToken();

    return false;
}

bool ETSParser::IsStructKeyword() const
{
    return (Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_IDENT &&
            Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_STRUCT);
}

// NOLINTNEXTLINE(google-default-arguments)
ir::Expression *ETSParser::ParseExpression(ExpressionParseFlags flags)
{
    if (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_YIELD &&
        (flags & ExpressionParseFlags::DISALLOW_YIELD) == 0U) {
        ir::YieldExpression *yield_expr = ParseYieldExpression();

        return ParsePotentialExpressionSequence(yield_expr, flags);
    }

    ir::Expression *unary_expression_node = ParseUnaryOrPrefixUpdateExpression(flags);
    ir::Expression *assignment_expression = ParseAssignmentExpression(unary_expression_node, flags);

    if (Lexer()->GetToken().NewLine()) {
        return assignment_expression;
    }

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COMMA &&
        (flags & ExpressionParseFlags::ACCEPT_COMMA) != 0U && (flags & ExpressionParseFlags::IN_FOR) != 0U) {
        return ParseSequenceExpression(assignment_expression, (flags & ExpressionParseFlags::ACCEPT_REST) != 0U);
    }

    return assignment_expression;
}

void ETSParser::ParseTrailingBlock(ir::CallExpression *call_expr)
{
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
        call_expr->SetIsTrailingBlockInNewLine(Lexer()->GetToken().NewLine());
        call_expr->SetTrailingBlock(ParseBlockStatement());
    }
}

ir::Expression *ETSParser::ParseCoercedNumberLiteral()
{
    if ((Lexer()->GetToken().Flags() & lexer::TokenFlags::NUMBER_FLOAT) != 0U) {
        auto *number = AllocNode<ir::NumberLiteral>(Lexer()->GetToken().GetNumber());
        number->SetRange(Lexer()->GetToken().Loc());
        auto *float_type = AllocNode<ir::ETSPrimitiveType>(ir::PrimitiveType::FLOAT);
        float_type->SetRange(Lexer()->GetToken().Loc());
        auto *as_expression = AllocNode<ir::TSAsExpression>(number, float_type, true);
        as_expression->SetRange(Lexer()->GetToken().Loc());

        Lexer()->NextToken();
        return as_expression;
    }
    return ParseNumberLiteral();
}

void ETSParser::CheckDeclare()
{
    ASSERT(Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_DECLARE);

    if (InAmbientContext()) {
        ThrowSyntaxError("A 'declare' modifier cannot be used in an already ambient context.");
    }

    GetContext().Status() |= ParserStatus::IN_AMBIENT_CONTEXT;

    Lexer()->NextToken();  // eat 'declare'

    switch (Lexer()->GetToken().KeywordType()) {
        case lexer::TokenType::KEYW_LET:
        case lexer::TokenType::KEYW_CONST:
        case lexer::TokenType::KEYW_FUNCTION:
        case lexer::TokenType::KEYW_CLASS:
        case lexer::TokenType::KEYW_NAMESPACE:
        case lexer::TokenType::KEYW_ENUM:
        case lexer::TokenType::KEYW_ABSTRACT:
        case lexer::TokenType::KEYW_INTERFACE: {
            return;
        }
        default: {
            ThrowSyntaxError("Unexpected token.");
        }
    }
}

//================================================================================================//
//  Methods to create AST node(s) from the specified string (part of valid ETS-code!)
//================================================================================================//

// NOLINTBEGIN(modernize-avoid-c-arrays)
static constexpr char const INVALID_NUMBER_NODE[] = "Invalid node number in format expression.";
static constexpr char const INVALID_FORMAT_NODE[] = "Invalid node type in format expression.";
static constexpr char const INSERT_NODE_ABSENT[] = "There is no any node to insert at the placeholder position.";
static constexpr char const INVALID_INSERT_NODE[] =
    "Inserting node type differs from that required by format specification.";
// NOLINTEND(modernize-avoid-c-arrays)

ParserImpl::NodeFormatType ETSParser::GetFormatPlaceholderIdent() const
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_FORMAT);
    Lexer()->NextToken();
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_IDENT);

    char const *const ident_data = Lexer()->GetToken().Ident().Bytes();

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic, cert-err34-c)
    auto ident_number = std::atoi(ident_data + 1U);
    if (ident_number <= 0) {
        ThrowSyntaxError(INVALID_NUMBER_NODE, Lexer()->GetToken().Start());
    }

    return {*ident_data, static_cast<decltype(std::declval<ParserImpl::NodeFormatType>().second)>(ident_number - 1)};
}

ir::AstNode *ETSParser::ParseFormatPlaceholder()
{
    if (inserting_nodes_.empty()) {
        ThrowSyntaxError(INSERT_NODE_ABSENT, Lexer()->GetToken().Start());
    }

    if (auto node_format = GetFormatPlaceholderIdent(); node_format.first == EXPRESSION_FORMAT_NODE) {
        return ParseExpressionFormatPlaceholder(std::make_optional(node_format));
    } else if (node_format.first == IDENTIFIER_FORMAT_NODE) {  // NOLINT(readability-else-after-return)
        return ParseIdentifierFormatPlaceholder(std::make_optional(node_format));
    } else if (node_format.first == TYPE_FORMAT_NODE) {  // NOLINT(readability-else-after-return)
        return ParseTypeFormatPlaceholder(std::make_optional(node_format));
    } else if (node_format.first == STATEMENT_FORMAT_NODE) {  // NOLINT(readability-else-after-return)
        return ParseStatementFormatPlaceholder(std::make_optional(node_format));
    }

    ThrowSyntaxError(INVALID_FORMAT_NODE, Lexer()->GetToken().Start());
}

ir::Expression *ETSParser::ParseExpressionFormatPlaceholder(std::optional<ParserImpl::NodeFormatType> node_format)
{
    if (!node_format.has_value()) {
        if (inserting_nodes_.empty()) {
            ThrowSyntaxError(INSERT_NODE_ABSENT, Lexer()->GetToken().Start());
        }

        if (node_format = GetFormatPlaceholderIdent(); node_format->first == TYPE_FORMAT_NODE) {
            return ParseTypeFormatPlaceholder(std::move(node_format));
        } else if (node_format->first == IDENTIFIER_FORMAT_NODE) {  // NOLINT(readability-else-after-return)
            return ParseIdentifierFormatPlaceholder(std::move(node_format));
        } else if (node_format->first != EXPRESSION_FORMAT_NODE) {  // NOLINT(readability-else-after-return)
            ThrowSyntaxError(INVALID_FORMAT_NODE, Lexer()->GetToken().Start());
        }
    }

    auto *const inserting_node =
        node_format->second < inserting_nodes_.size() ? inserting_nodes_[node_format->second] : nullptr;
    if (inserting_node == nullptr || !inserting_node->IsExpression()) {
        ThrowSyntaxError(INVALID_INSERT_NODE, Lexer()->GetToken().Start());
    }

    auto *const insert_expression = inserting_node->AsExpression();
    Lexer()->NextToken();
    return insert_expression;
}

ir::TypeNode *ETSParser::ParseTypeFormatPlaceholder(std::optional<ParserImpl::NodeFormatType> node_format)
{
    if (!node_format.has_value()) {
        if (inserting_nodes_.empty()) {
            ThrowSyntaxError(INSERT_NODE_ABSENT, Lexer()->GetToken().Start());
        }

        if (node_format = GetFormatPlaceholderIdent(); node_format->first != TYPE_FORMAT_NODE) {
            ThrowSyntaxError(INVALID_FORMAT_NODE, Lexer()->GetToken().Start());
        }
    }

    auto *const inserting_node =
        node_format->second < inserting_nodes_.size() ? inserting_nodes_[node_format->second] : nullptr;
    if (inserting_node == nullptr || !inserting_node->IsExpression() || !inserting_node->AsExpression()->IsTypeNode()) {
        ThrowSyntaxError(INVALID_INSERT_NODE, Lexer()->GetToken().Start());
    }

    auto *const insert_type = inserting_node->AsExpression()->AsTypeNode();
    Lexer()->NextToken();
    return insert_type;
}

// NOLINTNEXTLINE(google-default-arguments)
ir::Identifier *ETSParser::ParseIdentifierFormatPlaceholder(std::optional<ParserImpl::NodeFormatType> node_format)
{
    if (!node_format.has_value()) {
        if (inserting_nodes_.empty()) {
            ThrowSyntaxError(INSERT_NODE_ABSENT, Lexer()->GetToken().Start());
        }

        if (node_format = GetFormatPlaceholderIdent(); node_format->first != IDENTIFIER_FORMAT_NODE) {
            ThrowSyntaxError(INVALID_FORMAT_NODE, Lexer()->GetToken().Start());
        }
    }

    auto *const inserting_node =
        node_format->second < inserting_nodes_.size() ? inserting_nodes_[node_format->second] : nullptr;
    if (inserting_node == nullptr || !inserting_node->IsExpression() ||
        !inserting_node->AsExpression()->IsIdentifier()) {
        ThrowSyntaxError(INVALID_INSERT_NODE, Lexer()->GetToken().Start());
    }

    auto *const insert_identifier = inserting_node->AsExpression()->AsIdentifier();
    Lexer()->NextToken();
    return insert_identifier;
}

ir::Statement *ETSParser::ParseStatementFormatPlaceholder(std::optional<ParserImpl::NodeFormatType> node_format)
{
    if (!node_format.has_value()) {
        if (inserting_nodes_.empty()) {
            ThrowSyntaxError(INSERT_NODE_ABSENT, Lexer()->GetToken().Start());
        }

        if (node_format = GetFormatPlaceholderIdent(); node_format->first != STATEMENT_FORMAT_NODE) {
            ThrowSyntaxError(INVALID_FORMAT_NODE, Lexer()->GetToken().Start());
        }
    }

    auto *const inserting_node =
        node_format->second < inserting_nodes_.size() ? inserting_nodes_[node_format->second] : nullptr;
    if (inserting_node == nullptr || !inserting_node->IsStatement()) {
        ThrowSyntaxError(INVALID_INSERT_NODE, Lexer()->GetToken().Start());
    }

    auto *const insert_statement = inserting_node->AsStatement();
    Lexer()->NextToken();
    return insert_statement;
}

ir::Statement *ETSParser::CreateStatement(std::string_view const source_code, std::string_view const file_name)
{
    util::UString source {source_code, Allocator()};
    auto const isp = InnerSourceParser(this);
    auto const lexer = InitLexer({file_name, source.View().Utf8()});

    lexer::SourcePosition const start_loc = lexer->GetToken().Start();
    lexer->NextToken();

    auto statements = ParseStatementList(StatementParsingFlags::STMT_GLOBAL_LEXICAL);
    auto const statement_number = statements.size();

    if (statement_number == 0U) {
        return nullptr;
    }

    if (statement_number == 1U) {
        return statements[0U];
    }

    auto *const block_stmt = AllocNode<ir::BlockStatement>(Allocator(), std::move(statements));
    block_stmt->SetRange({start_loc, lexer->GetToken().End()});

    return block_stmt;
}

ir::Statement *ETSParser::CreateFormattedStatement(std::string_view const source_code,
                                                   std::vector<ir::AstNode *> &inserting_nodes,
                                                   std::string_view const file_name)
{
    inserting_nodes_.swap(inserting_nodes);
    auto const statement = CreateStatement(source_code, file_name);
    inserting_nodes_.swap(inserting_nodes);
    return statement;
}

ArenaVector<ir::Statement *> ETSParser::CreateStatements(std::string_view const source_code,
                                                         std::string_view const file_name)
{
    util::UString source {source_code, Allocator()};
    auto const isp = InnerSourceParser(this);
    auto const lexer = InitLexer({file_name, source.View().Utf8()});

    lexer->NextToken();
    return ParseStatementList(StatementParsingFlags::STMT_GLOBAL_LEXICAL);
}

ArenaVector<ir::Statement *> ETSParser::CreateFormattedStatements(std::string_view const source_code,
                                                                  std::vector<ir::AstNode *> &inserting_nodes,
                                                                  std::string_view const file_name)
{
    inserting_nodes_.swap(inserting_nodes);
    auto statements = CreateStatements(source_code, file_name);
    inserting_nodes_.swap(inserting_nodes);
    return statements;
}

ir::MethodDefinition *ETSParser::CreateMethodDefinition(ir::ModifierFlags modifiers, std::string_view const source_code,
                                                        std::string_view const file_name)
{
    util::UString source {source_code, Allocator()};
    auto const isp = InnerSourceParser(this);
    auto const lexer = InitLexer({file_name, source.View().Utf8()});

    auto const start_loc = Lexer()->GetToken().Start();
    Lexer()->NextToken();

    if (IsClassMethodModifier(Lexer()->GetToken().Type())) {
        modifiers |= ParseClassMethodModifiers(false);
    }

    ir::MethodDefinition *method_definition = nullptr;
    auto *method_name = ExpectIdentifier();

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS ||
        Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LESS_THAN) {
        method_definition = ParseClassMethodDefinition(method_name, modifiers);
        method_definition->SetStart(start_loc);
    }

    return method_definition;
}

ir::MethodDefinition *ETSParser::CreateConstructorDefinition(ir::ModifierFlags modifiers,
                                                             std::string_view const source_code,
                                                             std::string_view const file_name)
{
    util::UString source {source_code, Allocator()};
    auto const isp = InnerSourceParser(this);
    auto const lexer = InitLexer({file_name, source.View().Utf8()});

    auto const start_loc = Lexer()->GetToken().Start();
    Lexer()->NextToken();

    if (IsClassMethodModifier(Lexer()->GetToken().Type())) {
        modifiers |= ParseClassMethodModifiers(false);
    }

    if (Lexer()->GetToken().Type() != lexer::TokenType::KEYW_CONSTRUCTOR) {
        ThrowSyntaxError({"Unexpected token. 'Constructor' keyword is expected."});
    }

    if ((modifiers & ir::ModifierFlags::ASYNC) != 0) {
        ThrowSyntaxError({"Constructor should not be async."});
    }

    auto *member_name = AllocNode<ir::Identifier>(Lexer()->GetToken().Ident(), Allocator());
    modifiers |= ir::ModifierFlags::CONSTRUCTOR;
    Lexer()->NextToken();

    auto *const method_definition = ParseClassMethodDefinition(member_name, modifiers);
    method_definition->SetStart(start_loc);

    return method_definition;
}

ir::Expression *ETSParser::CreateExpression(std::string_view const source_code, ExpressionParseFlags const flags,
                                            std::string_view const file_name)
{
    util::UString source {source_code, Allocator()};
    auto const isp = InnerSourceParser(this);
    auto const lexer = InitLexer({file_name, source.View().Utf8()});

    lexer::SourcePosition const start_loc = lexer->GetToken().Start();
    lexer->NextToken();

    ir::Expression *return_expression = ParseExpression(flags);
    return_expression->SetRange({start_loc, lexer->GetToken().End()});

    return return_expression;
}

ir::Expression *ETSParser::CreateFormattedExpression(std::string_view const source_code,
                                                     std::vector<ir::AstNode *> &inserting_nodes,
                                                     std::string_view const file_name)
{
    ir::Expression *return_expression;
    inserting_nodes_.swap(inserting_nodes);

    if (auto statements = CreateStatements(source_code, file_name);
        statements.size() == 1U && statements.back()->IsExpressionStatement()) {
        return_expression = statements.back()->AsExpressionStatement()->GetExpression();
    } else {
        return_expression = AllocNode<ir::BlockExpression>(std::move(statements));
    }

    inserting_nodes_.swap(inserting_nodes);
    return return_expression;
}

ir::TypeNode *ETSParser::CreateTypeAnnotation(TypeAnnotationParsingOptions *options, std::string_view const source_code,
                                              std::string_view const file_name)
{
    util::UString source {source_code, Allocator()};
    auto const isp = InnerSourceParser(this);
    auto const lexer = InitLexer({file_name, source.View().Utf8()});

    lexer->NextToken();
    return ParseTypeAnnotation(options);
}

//================================================================================================//
//  ExternalSourceParser class
//================================================================================================//

ExternalSourceParser::ExternalSourceParser(ETSParser *parser, Program *new_program)
    : parser_(parser),
      saved_program_(parser_->GetProgram()),
      saved_lexer_(parser_->Lexer()),
      saved_top_scope_(parser_->GetProgram()->VarBinder()->TopScope())
{
    parser_->SetProgram(new_program);
    parser_->GetContext().SetProgram(new_program);
}

ExternalSourceParser::~ExternalSourceParser()
{
    parser_->SetLexer(saved_lexer_);
    parser_->SetProgram(saved_program_);
    parser_->GetContext().SetProgram(saved_program_);
    parser_->GetProgram()->VarBinder()->ResetTopScope(saved_top_scope_);
}

//================================================================================================//
//  InnerSourceParser class
//================================================================================================//

InnerSourceParser::InnerSourceParser(ETSParser *parser)
    : parser_(parser),
      saved_lexer_(parser_->Lexer()),
      saved_source_code_(parser_->GetProgram()->SourceCode()),
      saved_source_file_(parser_->GetProgram()->SourceFilePath()),
      saved_source_file_path_(parser_->GetProgram()->SourceFileFolder())
{
}

InnerSourceParser::~InnerSourceParser()
{
    parser_->SetLexer(saved_lexer_);
    parser_->GetProgram()->SetSource(saved_source_code_, saved_source_file_, saved_source_file_path_);
}
}  // namespace panda::es2panda::parser
#undef USE_UNIX_SYSCALL
