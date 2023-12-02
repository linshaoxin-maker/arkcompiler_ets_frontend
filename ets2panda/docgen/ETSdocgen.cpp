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

#include "ETSdocgen.h"
#include "json.h"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <regex>

#include "generated/signatures.h"

#include "parser/program/program.h"

#include "varbinder/varbinder.h"
#include "varbinder/variable.h"
#include "varbinder/variableFlags.h"

#include "ir/astNode.h"
#include "ir/base/classProperty.h"
#include "ir/base/methodDefinition.h"
#include "ir/statements/blockStatement.h"
#include "ir/statements/functionDeclaration.h"
#include "ir/statements/classDeclaration.h"
#include "ir/ets/etsPackageDeclaration.h"
#include "ir/ts/tsQualifiedName.h"
#include "ir/base/scriptFunction.h"
#include "ir/expressions/identifier.h"
#include "ir/base/classDefinition.h"
#include "ir/ets/etsTypeReference.h"
#include "ir/ets/etsTypeReferencePart.h"
#include "ir/ets/etsParameterExpression.h"
#include "ir/hasDoc.h"
#include "ir/ts/tsTypeParameterInstantiation.h"
#include "ir/ts/tsClassImplements.h"
#include "ir/ts/tsInterfaceBody.h"
#include "ir/ts/tsInterfaceDeclaration.h"
#include "ir/ts/tsInterfaceHeritage.h"
#include "ir/ts/tsTypeParameter.h"
#include "ir/ets/etsScript.h"
#include "ir/ts/tsTypeAliasDeclaration.h"
#include "ir/ts/tsEnumDeclaration.h"
#include "ir/expressions/literals/numberLiteral.h"
#include "ir/expressions/literals/stringLiteral.h"
#include "ir/ts/tsEnumMember.h"
#include "ir/base/spreadElement.h"

#include "checker/types/type.h"
#include "checker/types/ets/etsFunctionType.h"
#include "checker/types/ets/etsObjectType.h"
#include "checker/types/ets/etsArrayType.h"
#include "checker/checker.h"
#include "checker/ETSchecker.h"

#include "compiler/core/compilerContext.h"
#include "es2panda.h"

namespace panda::es2panda::docgen {

namespace {

template <typename T>
T &ConstructDefaultGet(JObject &obj, std::string_view name)
{
    return std::get<T>(obj.emplace(name, T {}).first->second);
}

template <typename T>
T &ConstructDefaultBackGet(JArray &obj)
{
    return std::get<T>(obj.emplace_back(T {}));
}

template <typename T>
class ScopedChanger {
public:
    ScopedChanger(T &to, T val) : storage_(to), old_value_(std::move(to))
    {
        to = std::move(val);
    }

    NO_COPY_SEMANTIC(ScopedChanger);
    NO_MOVE_SEMANTIC(ScopedChanger);

    ~ScopedChanger()
    {
        storage_ = std::move(old_value_);
    }

private:
    T &storage_;
    T old_value_;
};

template <typename T, typename D>
ScopedChanger(T &, D) -> ScopedChanger<T>;

class ErrorHandler {
public:
    explicit ErrorHandler(JArray *errors) : errors_(errors) {}

    void PushTrace(JValue v)
    {
        trace_.emplace_back(std::move(v));
    }

    void PopTrace()
    {
        trace_.pop_back();
    }

    void Error(JValue text)
    {
        errors_->emplace_back(JObject {{"text", JValue {std::move(text)}}, {"trace", JValue {trace_}}});
    }

    template <typename F>
    void WithTrace(JValue trace, F action)
    {
        PushTrace(std::move(trace));
        auto fn = [](ErrorHandler *c) { c->PopTrace(); };
        auto clean = std::unique_ptr<ErrorHandler, decltype(fn)>(this, fn);
        action();
    }

private:
    JArray trace_;
    JArray *errors_;
};

bool IsSpace(char a)
{
    return std::isspace(a) != 0;
}

template <typename F>
std::string_view SpanWhileTrue(std::string_view &str, F pred)
{
    size_t idx = 0;
    while (idx < str.size() && pred(str[idx])) {
        idx++;
    }
    auto ret = str.substr(0, idx);
    str = str.substr(idx);
    return ret;
}

std::string_view SpanSpaces(std::string_view &str)
{
    return SpanWhileTrue(str, [](auto c) { return IsSpace(c); });
}

enum class DocMode { TEXT, CODE };

class ETSDocgenImpl;

class DocParser {
protected:
    template <typename T>
    T *InitializeField(const std::string &name)
    {
        return &std::get<T>(result_.insert({name, JValue {T {}}}).first->second);
    }

private:
    JArray *InOther(const std::string_view &name)
    {
        auto &obj = ConstructDefaultBackGet<JObject>(*other_);
        obj.emplace("kind", "other");
        obj.emplace("title", name);
        return &std::get<JArray>(obj.emplace("data", JArray {}).first->second);
    }

public:
    using Text = JArray;

    DocParser(std::string_view &rest, ETSDocgenImpl *parent) : rest_(rest), parent_(parent)
    {
        brief_ = InitializeField<Text>("brief");
        description_ = InitializeField<Text>("description");
        deprecated_ = InitializeField<Text>("deprecated");
        other_ = InitializeField<JArray>("other");
        tags_ = InitializeField<JArray>("tags");
    }

    std::pair<DocMode, JArray *> Start()
    {
        return {DocMode::TEXT, brief_};
    }

    virtual std::pair<DocMode, JArray *> HandleNextTextCommand(std::string_view command, JArray *previous)
    {
        if (command == "brief") {
            return {DocMode::TEXT, brief_};
        }
        if (command == "description") {
            return {DocMode::TEXT, description_};
        }
        if (command == "deprecated") {
            return {DocMode::TEXT, deprecated_};
        }
        if (command == "example") {
            return {DocMode::CODE, InOther(command)};
        }
        if (command == "tparam") {
            return HandleTParam();
        }
        if (command == "link") {
            return HandleTextLink(previous);
        }
        if (command == "tag") {
            auto name = FetchId();
            tags_->push_back(JValue {JString {name}});
            return {DocMode::TEXT, previous};
        }
        static const std::set<std::string_view> DEFAULT_TEXT_COMMANDS = {"note", "remark", "info", "see"};
        if (DEFAULT_TEXT_COMMANDS.count(command) == 0) {
            std::string txt = "unknown tag `";
            txt += command;
            txt += "`";
            Error(JValue {txt});
        }
        std::ignore = previous;
        return {DocMode::TEXT, InOther(command)};
    }

    std::string_view FetchId()
    {
        SpanSpaces(rest_);
        return SpanWhileTrue(rest_, [](auto c) { return std::isalnum(c) != 0 || c == '_'; });
    }

    JValue FetchType();

    void Error(JValue val);

    JObject Finish()
    {
        ASSERT(rest_.empty());
        if (tags_->empty()) {
            result_.erase("tags");
        }
        return result_;
    }

private:
    std::string_view &rest_;
    ETSDocgenImpl *parent_;

    JObject result_ {};
    Text *brief_ {};
    Text *description_ {};
    Text *deprecated_ {};
    JArray *other_ {};
    JArray *tags_ {};

    std::pair<DocMode, JArray *> HandleTParam()
    {
        auto id = FetchId();
        auto &targs = ConstructDefaultGet<JArray>(result_, "targs");
        auto &obj = ConstructDefaultBackGet<JObject>(targs);
        obj.emplace("name", id);
        return {DocMode::TEXT, &std::get<JArray>(obj.emplace("data", JArray {}).first->second)};
    }

    std::pair<DocMode, JArray *> HandleTextLink(JArray *previous)
    {
        if (!previous->empty() && std::holds_alternative<std::string>(previous->back())) {
            auto &str = std::get<std::string>(previous->back());
            if (!str.empty() && str.back() == '{') {
                str.pop_back();
            }
        }
        if (!rest_.empty() && rest_.front() == '{') {
            rest_ = rest_.substr(1);
        }
        JObject link;
        link.emplace("kind", "link");
        SpanSpaces(rest_);
        link.emplace("link", SpanWhileTrue(rest_, [](auto c) { return !IsSpace(c) && c != '}' && c != '|'; }));
        SpanSpaces(rest_);
        if (!rest_.empty() && rest_.front() == '|') {
            rest_ = rest_.substr(1);
            SpanSpaces(rest_);
            link.emplace("text", SpanWhileTrue(rest_, [](auto c) { return c != '}'; }));
        }
        if (!rest_.empty() && rest_.front() == '}') {
            rest_ = rest_.substr(1);
        }
        previous->emplace_back(std::move(link));
        return {DocMode::TEXT, previous};
    }
};

class MethodDocParser : public DocParser {
public:
    using DocParser::DocParser;

    std::pair<DocMode, JArray *> HandleNextTextCommand(std::string_view command, JArray *previous) override
    {
        if (command == "returns") {
            return {DocMode::TEXT, returns_};
        }
        if (command == "throws") {
            auto type = FetchType();
            auto &obj = ConstructDefaultBackGet<JObject>(*throws_);
            obj.emplace("type", std::move(type));
            return {DocMode::TEXT, &std::get<JArray>(obj.emplace("data", JArray {}).first->second)};
        }
        if (command == "param") {
            auto id = FetchId();
            auto &obj = ConstructDefaultBackGet<JObject>(*args_);
            obj.emplace("name", id);
            return {DocMode::TEXT, &std::get<JArray>(obj.emplace("data", JArray {}).first->second)};
        }
        return DocParser::HandleNextTextCommand(command, previous);
    }

private:
    JArray *returns_ = InitializeField<JArray>("returns");
    JArray *args_ = InitializeField<JArray>("params");
    JArray *throws_ = InitializeField<JArray>("throws");
};

class ETSDocgenImpl {
public:
    std::string Str();

    ETSDocgenImpl &Run(parser::Program *prog);

    [[nodiscard]] JValue DumpType(ir::ETSTypeReference *ref)
    {
        auto type = DumpType(ref->TsType());
        if (std::holds_alternative<JObject>(type)) {
            AddGenericInfo(std::get<JObject>(type), ref->Part()->TypeParams());
        }
        return type;
    }

    [[nodiscard]] JValue DumpObjectType(checker::ETSObjectType *obj_type)
    {
        auto r = obj_type->HasObjectFlag(checker::ETSObjectFlags::FUNCTIONAL)       ? DumpFunctionalType(obj_type)
                 : obj_type->HasObjectFlag(checker::ETSObjectFlags::TYPE_PARAMETER) ? DumpVarType(obj_type)
                                                                                    : DumpClassType(obj_type);
        JArray alts = {std::move(r)};

        auto nullable = false;
        auto undefinable = false;
        if (obj_type->HasObjectFlag(checker::ETSObjectFlags::TYPE_PARAMETER)) {
            if (obj_type->GetBaseType() != nullptr) {
                nullable = obj_type->ContainsNull();
                undefinable = obj_type->ContainsUndefined();
            }
        } else {
            nullable = obj_type->ContainsNull();
            undefinable = obj_type->ContainsUndefined();
        }
        if (nullable) {
            alts.emplace_back("null");
        }
        if (undefinable) {
            alts.emplace_back("undefined");
        }

        if (alts.size() == 1) {
            auto ret = std::move(alts[0]);
            return ret;
        }
        JObject union_type = {{"kind", JValue {"union"}}};
        union_type.emplace("alts", std::move(alts));
        return JValue {union_type};
    }

    [[nodiscard]] JValue DumpType(checker::Type *type)
    {
        if (type == nullptr) {
            return JNULL;
        }

        if (type->IsETSArrayType()) {
            JObject r;
            r.emplace("kind", "arr");
            r.emplace("elem", DumpType(type->AsETSArrayType()->ElementType()));
            return JValue {r};
        }
        if (type->IsETSObjectType()) {
            return DumpObjectType(type->AsETSObjectType());
        }
        ASSERT(!type->IsNonPrimitiveType());
        std::stringstream out;
        type->ToString(out);
        return JValue {out.str()};
    }

    ErrorHandler &Errors()
    {
        return errors_;
    }

    compiler::CompilerContext *Context() const
    {
        return ctx_;
    }

    varbinder::Scope *Scope() const
    {
        return scope_;
    }

private:
    JValue result_ {JObject {}};
    JValue *insert_point_ {&result_};
    std::vector<JValue> trace_;
    ErrorHandler errors_ {&std::get<JArray>(std::get<JObject>(result_).emplace("errors", JArray {}).first->second)};
    compiler::CompilerContext *ctx_ {};
    varbinder::Scope *scope_ {};

    template <typename T>
    class CommentParser {
    public:
        static_assert(std::is_base_of_v<DocParser, T>);

        CommentParser(std::string_view &comment, ETSDocgenImpl *parent) : comment_(comment), parser_(comment, parent) {}

        JValue Run()
        {
            while (!comment_.empty()) {
                if (state_.first == DocMode::CODE) {
                    HandleCode();
                    continue;
                }
                SkipIntro(true);
                if (comment_.empty()) {
                    break;
                }
                // empty line
                if (comment_.front() == '\n') {
                    if (!text_.empty() && text_.back() != '\n') {
                        text_ += '\n';
                    }
                    continue;
                }

                HandleTextLine();
            }
            FlushText();
            return JValue {parser_.Finish()};
        }

    private:
        std::string_view &comment_;
        T parser_;
        std::pair<DocMode, JArray *> state_ = parser_.Start();
        std::string code_;
        std::string text_;

        void SkipIntro(bool all)
        {
            while (!comment_.empty() && IsSpace(comment_.front())) {
                comment_ = comment_.substr(1);
            }
            while (!comment_.empty() && comment_.front() == '*') {
                comment_ = comment_.substr(1);
            }
            while (all && !comment_.empty() && IsSpace(comment_.front()) && comment_.front() != '\n') {
                comment_ = comment_.substr(1);
            }
        }

        void HandleCode()
        {
            auto lines = ReadCode();
            JObject res {{"kind", JValue {"block-code"}}, {"data", JValue {PrepareCode(lines)}}};
            state_.second->emplace_back(std::move(res));
            state_.first = DocMode::TEXT;
        }

        void HandleTextLine()
        {
            while (!comment_.empty()) {
                auto fr = comment_.front();
                comment_ = comment_.substr(1);
                if (fr == '\n') {
                    text_ += ' ';
                    break;
                }
                if (fr != '@') {
                    text_ += fr;
                    continue;
                }
                FlushText();
                auto command = parser_.FetchId();
                state_ = parser_.HandleNextTextCommand(command, state_.second);
                break;
            }
        }

        void FlushText()
        {
            if (!text_.empty()) {
                state_.second->emplace_back(std::move(text_));
                text_.clear();
            }
        }

        std::vector<std::string_view> ReadCode()
        {
            std::vector<std::string_view> lines {};
            do {
                auto pos = comment_.find('\n');
                if (pos == 0 && !lines.empty()) {
                    break;
                }
                lines.emplace_back(comment_.substr(0, pos));
                if (pos == std::string::npos) {
                    comment_ = "";
                } else {
                    comment_ = comment_.substr(pos + 1);
                }
                SkipIntro(false);
            } while (!comment_.empty());
            return lines;
        }

        std::string PrepareCode(std::vector<std::string_view> &lines)
        {
            size_t max_common_space = lines.empty() ? 0 : std::numeric_limits<size_t>::max();
            for (auto &l : lines) {
                while (!l.empty() && IsSpace(l.back())) {
                    l = l.substr(0, l.size() - 1);
                }
                auto pos = l.find_first_not_of(" \t");
                if (pos != std::string::npos) {
                    max_common_space = std::min(max_common_space, pos);
                }
            }
            std::string result;
            for (auto &l : lines) {
                if (max_common_space < l.size()) {
                    result += l.substr(max_common_space);
                }
                result += "\n";
            }
            return result;
        }
    };

    template <typename T>
    JValue ParseComment(std::string_view comment)
    {
        return CommentParser<T> {comment, this}.Run();
    }

    void AppendProgram(util::StringView package_name, parser::Program *prog);

    void InsertStatement(ir::Statement *stmt);

    struct NullInserter {
        JValue operator()() const
        {
            return JNULL;
        }
    };

    struct ObjectInserter {
    public:
        JValue operator()() const
        {
            return JValue {JObject()};
        }
    };

    struct ArrayInserter {
    public:
        JValue operator()() const
        {
            return JValue {JArray {}};
        }
    };

    // This function is used for a tree zipper
    template <bool ALLOW_DUPLICATE, typename Inserter = ObjectInserter, typename F>
    void WithInserted(util::StringView name, F &&action)
    {
        ASSERT(std::holds_alternative<JObject>(*insert_point_));
        auto *cur_obj = &std::get<JObject>(*insert_point_);
        auto [iter, inserted] = cur_obj->emplace(name, JNULL);
        if constexpr (!ALLOW_DUPLICATE) {
            ASSERT(inserted);
        }
        if (inserted) {
            iter->second = Inserter {}();
        }
        auto old_insert = insert_point_;
        insert_point_ = &iter->second;
        action();
        insert_point_ = old_insert;
    }

    JValue DumpFunctionalType(checker::ETSObjectType *type)
    {
        auto fn = type->GetFunctionalInterfaceInvokeType();
        ASSERT(fn->CallSignatures().size() == 1);
        auto *sig = fn->CallSignatures().front();

        JObject ret;
        ret.emplace("kind", "fn");
        ret.emplace("ret", DumpType(sig->ReturnType()));
        auto &args = ConstructDefaultGet<JArray>(ret, "args");
        for (auto &par : sig->Params()) {
            JObject arg;
            arg.emplace("name", par->Name());
            arg.emplace("type", DumpType(par->TsType()));
            args.emplace_back(std::move(arg));
        }
        return JValue {ret};
    }

    JValue DumpVarType(checker::ETSObjectType *type)
    {
        JObject ret;
        ret.emplace("kind", "var");
        ret.emplace("name", type->Name());
        return JValue {ret};
    }

    JValue DumpClassType(checker::ETSObjectType *type)
    {
        if (type->IsETSVoidType() || type == this->ctx_->Checker()->AsETSChecker()->GlobalBuiltinVoidType()) {
            return JValue {std::string_view {"void"}};
        }
        JObject ret = {{"kind", JValue {"named"}}};
        auto decl = type->GetDeclNode();
        while (decl != nullptr && !decl->IsBlockStatement()) {
            decl = decl->Parent();
        }
        if (decl != nullptr) {
            auto program = static_cast<ir::ETSScript *>(decl)->Program();
            ret.emplace("pack", program->GetPackageName());
        }
        ret.emplace("name", type->Name());

        if (!type->TypeArguments().empty()) {
            JArray targs;
            std::transform(type->TypeArguments().begin(), type->TypeArguments().end(), std::back_inserter(targs),
                           [this](checker::Type *typ) -> JValue { return DumpType(typ); });
            ret.emplace("targs", std::move(targs));
        }
        return JValue {ret};
    }

    void EmplaceToObject(std::string_view name, JValue v)
    {
        std::get<JObject>(*insert_point_).emplace(name, std::move(v));
    }

    template <typename T>
    void EmplaceToArray(T &&v)
    {
        std::get<JArray>(*insert_point_).emplace_back(std::forward<T>(v));
    }

    void FillInVisibility(JObject &to, ir::AstNode *def, bool is_global)
    {
        to.emplace("exported", def->IsExported());
        if (is_global) {
            return;
        }
        auto selected = def->IsPrivate()     ? "private"
                        : def->IsProtected() ? "protected"
                        : def->IsInternal()  ? "internal"
                        : def->IsPublic()    ? "public"
                                             : nullptr;
        if (selected != nullptr) {
            to.emplace("visibility", selected);
        }
    }

    void AddClassProperty(ir::ClassProperty *def, bool is_global)
    {
        JObject prop;
        prop.emplace("name", def->Id()->Name());
        prop.emplace("static", def->IsStatic());
        prop.emplace("readonly", def->IsReadonly());
        prop.emplace("const", def->IsConst());
        FillInVisibility(prop, def, is_global);
        prop.emplace("type", DumpType(def->TsType()));
        AddDocComment<DocParser>(prop, def->Documentation());

        EmplaceToArray(std::move(prop));
    }

    template <typename T>
    void AddDocComment(JObject &obj, ir::HasDoc &doc)
    {
        obj.emplace("comment", doc.GetDocComment().has_value() ? ParseComment<T>(doc.GetDocComment().value()) : JNULL);
        std::ignore = obj;
        std::ignore = doc;
    }

    void AddMethod(ir::MethodDefinition *def, bool is_global)
    {
        if (def->Function()->IsHidden()) {
            return;
        }
        JObject meth = {
            {"name", JValue {def->Id()->Name()}},
            {"final", JValue {def->IsFinal()}},
            {"static", JValue {def->IsStatic()}},
            {"override", JValue {def->IsOverride()}},
            {"throws", JValue {def->Function()->IsThrowing()}},
        };
        if (def->IsSetter()) {
            meth.emplace("property", "set");
        }
        if (def->Function() != nullptr && def->Function()->IsSetter()) {
            meth.emplace("property", "set");
        }
        if (def->Function() != nullptr && def->Function()->IsGetter()) {
            meth.emplace("property", "get");
        }
        errors_.WithTrace(JValue {def->Id()->Name()}, [this, &def, &meth, is_global]() {
            AddGenericInfo(meth, def->Function()->TypeParams());
            FillInVisibility(meth, def, is_global);
            AddDocComment<MethodDocParser>(meth, def->Documentation());
            meth.emplace("ret", DumpType(def->Function()->Signature()->ReturnType()));
            auto &jparams = ConstructDefaultGet<JArray>(meth, "args");
            for (const auto &param : def->Function()->Params()) {
                auto &jparam = ConstructDefaultBackGet<JObject>(jparams);
                ASSERT(param->IsETSParameterExpression());
                auto param_expr = param->AsETSParameterExpression();
                if (param_expr->IsRestParameter()) {
                    jparam.emplace("rest", true);
                }
                auto id = param_expr->Ident();
                jparam.emplace("name", id->Name());
                jparam.emplace("type", DumpType(id->Variable()->TsType()));
            }
        });

        EmplaceToArray(std::move(meth));
        for (auto &over : def->Overloads()) {
            AddMethod(over, is_global);
        }
    }

    void PopulateGlobal(ir::ClassDeclaration *global)
    {
        for (auto &el : global->Definition()->Body()) {
            if (el->IsAutoGenerated()) {
                continue;
            }
            switch (el->Type()) {
                case ir::AstNodeType::METHOD_DEFINITION: {
                    WithInserted<true, ArrayInserter>("methods",
                                                      [this, &el]() { AddMethod(el->AsMethodDefinition(), true); });
                    break;
                }
                case ir::AstNodeType::CLASS_STATIC_BLOCK: {
                    break;
                }
                case ir::AstNodeType::CLASS_PROPERTY: {
                    WithInserted<true, ArrayInserter>("props",
                                                      [this, &el]() { AddClassProperty(el->AsClassProperty(), true); });
                    break;
                }
                default:
                    std::cerr << "unknown element type " << (int)el->Type() << std::endl;
                    UNREACHABLE();
            }
        }
    }

    void AddGenericInfo(JObject &type, const ir::TSTypeParameterInstantiation *targs)
    {
        if (targs == nullptr || targs->Params().empty()) {
            return;
        }
        JArray res;
        std::transform(
            targs->Params().begin(), targs->Params().end(), std::back_inserter(res),
            [this](ir::TypeNode *typ) { return DumpType(typ->GetType(static_cast<checker::ETSChecker *>(nullptr))); });
        type.emplace("targs", std::move(res));
    }

    void AddGenericInfo(JObject &type, const ir::TSTypeParameterDeclaration *targs)
    {
        if (targs == nullptr || targs->Params().empty()) {
            return;
        }
        JArray res;
        std::transform(targs->Params().begin(), targs->Params().end(), std::back_inserter(res),
                       [](ir::TSTypeParameter *typ) {
                           // NOTE(kprokopenko): dump typ->Constraint()
                           return JValue {typ->Name()->Name()};
                       });
        [[maybe_unused]] auto inserted = type.emplace("targs", std::move(res));
        ASSERT(inserted.second);
    }

    void PopulateInterface(ir::TSInterfaceDeclaration *cls)
    {
        FillInVisibility(std::get<JObject>(*insert_point_), cls, true);
        AddDocComment<DocParser>(std::get<JObject>(*insert_point_), cls->Documentation());
        WithInserted<true, ArrayInserter>("extends", [cls, this]() {
            for (auto &impl : cls->Extends()) {
                EmplaceToArray(DumpType(impl->Expr()->AsETSTypeReference()));
            }
        });
        AddGenericInfo(std::get<JObject>(*insert_point_), cls->TypeParams());
        AddDocComment<DocParser>(std::get<JObject>(*insert_point_), cls->Documentation());
        [[maybe_unused]] auto scope_changer = ScopedChanger(scope_, cls->Scope());
        for (auto &el : cls->Body()->Body()) {
            switch (el->Type()) {
                case ir::AstNodeType::METHOD_DEFINITION: {
                    WithInserted<true, ArrayInserter>("methods",
                                                      [this, el]() { AddMethod(el->AsMethodDefinition(), false); });
                    break;
                }
                case ir::AstNodeType::CLASS_STATIC_BLOCK: {
                    break;
                }
                case ir::AstNodeType::CLASS_PROPERTY: {
                    WithInserted<true, ArrayInserter>("variables",
                                                      [this, el]() { AddClassProperty(el->AsClassProperty(), false); });
                    break;
                }
                default:
                    std::cerr << "unknown element type " << (int)el->Type() << std::endl;
                    UNREACHABLE();
            }
        }
    }

    void AddTypeAlias(ir::TSTypeAliasDeclaration *alias)
    {
        JObject data;
        AddGenericInfo(data, alias->TypeParams());
        data.emplace("type", DumpType(alias->TypeAnnotation()->GetType(ctx_->Checker()->AsETSChecker())));
        EmplaceToObject(alias->Id()->Name().Utf8(), JValue {std::move(data)});
    }

    JValue DumpLiteral(const ir::Expression *expr)
    {
        using AstNodeType = ir::AstNodeType;
        switch (expr->Type()) {
            case AstNodeType::NUMBER_LITERAL: {
                auto num = expr->AsNumberLiteral()->Number();
                if (num.IsLong()) {
                    // num.Str() may be empty
                    return JValue {
                        JObject {{"kind", JValue {"num"}}, {"value", JValue {std::to_string(num.GetLong())}}}};
                }
                return JValue {num.GetDouble()};
            }
            case AstNodeType::STRING_LITERAL:
                return JValue {expr->AsStringLiteral()->Str()};
            default:
                UNREACHABLE();
        };
    }

    void AddEnum(ir::TSEnumDeclaration *enu)
    {
        JObject e;
        AddDocComment<DocParser>(e, enu->Documentation());
        FillInVisibility(e, enu, true);
        auto &members = ConstructDefaultGet<JArray>(e, "members");
        for (auto &mem : enu->Members()) {
            auto e_mem = mem->AsTSEnumMember();
            JValue value = DumpLiteral(e_mem->Init());
            auto obj = JObject {
                {"name", JValue {e_mem->Key()->AsIdentifier()->Name()}},
                {"value", std::move(value)},
            };
            AddDocComment<DocParser>(obj, e_mem->Documentation());
            members.emplace_back(std::move(obj));
        }
        EmplaceToObject(enu->Key()->Name().Utf8(), JValue {std::move(e)});
    }

    void PopulateClass(ir::ClassDeclaration *cls)
    {
        errors_.WithTrace(JValue {cls->Definition()->Ident()->Name()}, [this, cls]() {
            auto &klass = std::get<JObject>(*insert_point_);
            FillInVisibility(std::get<JObject>(*insert_point_), cls, true);
            klass.emplace("final", cls->IsFinal());
            WithInserted<true, ArrayInserter>("implements", [this, cls]() {
                for (auto &impl : cls->Definition()->Implements()) {
                    EmplaceToArray(DumpType(impl->Expr()->AsETSTypeReference()));
                }
            });
            if (cls->Definition()->Super() != nullptr) {
                EmplaceToObject("extends", DumpType(cls->Definition()->Super()->AsETSTypeReference()));
            }

            AddGenericInfo(std::get<JObject>(*insert_point_), cls->Definition()->TypeParams());
            AddDocComment<DocParser>(std::get<JObject>(*insert_point_), cls->Documentation());
            [[maybe_unused]] auto scope_changer = ScopedChanger(scope_, cls->Definition()->Scope());
            for (auto &el : cls->Definition()->Body()) {
                if (el->IsAutoGenerated()) {
                    continue;
                }
                switch (el->Type()) {
                    case ir::AstNodeType::METHOD_DEFINITION: {
                        WithInserted<true, ArrayInserter>("methods",
                                                          [this, el]() { AddMethod(el->AsMethodDefinition(), false); });
                        break;
                    }
                    case ir::AstNodeType::CLASS_STATIC_BLOCK: {
                        break;
                    }
                    case ir::AstNodeType::CLASS_PROPERTY: {
                        WithInserted<true, ArrayInserter>(
                            "variables", [this, el]() { AddClassProperty(el->AsClassProperty(), false); });
                        break;
                    }
                    default:
                        std::cerr << "unknown element type " << (int)el->Type() << std::endl;
                        UNREACHABLE();
                }
            }
        });
    }
};

JValue DocParser::FetchType()
{
    // NOTE(kprokopenko) need to start parser&lexer to be able to parse all types
    //  or integrate comments parsing into lexer itself
    auto type = FetchId();
    auto errored_return = [this, &type]() {
        parent_->Errors().Error(JValue {"can't resolve type " + JString {type}});
        return JValue {JString {type}};
    };
    auto scope = parent_->Scope();
    if (scope == nullptr) {
        return errored_return();
    }
    auto ctx = parent_->Context();
    auto checker = static_cast<checker::ETSChecker *>(ctx->Checker());
    auto allocator = checker->Allocator();
    auto id = allocator->New<ir::Identifier>(type, allocator);
    id->SetReference();
    auto res = scope->Find(type, varbinder::ResolveBindingOptions::TYPE_ALIASES |
                                     varbinder::ResolveBindingOptions::DECLARATION);
    if (res.variable == nullptr) {
        return errored_return();
    }
    id->SetVariable(res.variable);
    auto part = allocator->New<ir::ETSTypeReferencePart>(id, nullptr, nullptr);
    auto ref = allocator->New<ir::ETSTypeReference>(part);
    return parent_->DumpType(ref->Check(checker));
}

void DocParser::Error(JValue val)
{
    parent_->Errors().Error(std::move(val));
}

}  // namespace

std::string ETSDocgenImpl::Str()
{
    return JValue {std::move(result_)}.ToString();
}

void ETSDocgenImpl::InsertStatement(ir::Statement *stmt)
{
    using AstNodeType = ir::AstNodeType;
    switch (stmt->Type()) {
        case AstNodeType::ETS_IMPORT_DECLARATION:
        case AstNodeType::ETS_PACKAGE_DECLARATION: {
            break;
        }
        case AstNodeType::TS_INTERFACE_DECLARATION: {
            auto inter = stmt->AsTSInterfaceDeclaration();
            WithInserted<true>("interfaces", [this, inter]() {
                WithInserted<false>(inter->Id()->Name(), [this, inter]() { PopulateInterface(inter); });
            });
            break;
        }
        case AstNodeType::TS_TYPE_ALIAS_DECLARATION: {
            WithInserted<true>("aliases", [this, stmt]() { AddTypeAlias(stmt->AsTSTypeAliasDeclaration()); });
            break;
        }
        case AstNodeType::CLASS_DECLARATION: {
            auto cls = stmt->AsClassDeclaration();
            if (cls->Definition()->Ident()->Name() == compiler::Signatures::ETS_GLOBAL) {
                PopulateGlobal(cls);
            } else {
                WithInserted<true>("classes", [this, cls]() {
                    WithInserted<false>(cls->Definition()->Ident()->Name(), [this, cls]() { PopulateClass(cls); });
                });
            }
            break;
        }
        case AstNodeType::TS_ENUM_DECLARATION: {
            auto enu = stmt->AsTSEnumDeclaration();
            WithInserted<true>("enums", [this, enu]() { AddEnum(enu); });
            break;
        }
        default: {
            UNREACHABLE();
        }
    }
}

void ETSDocgenImpl::AppendProgram(util::StringView package_name, parser::Program *prog)
{
    [[maybe_unused]] std::string package_name_dots {package_name};
    std::replace(package_name_dots.begin(), package_name_dots.end(), '/', '.');
    ASSERT(prog->GetPackageName() == util::StringView(package_name_dots));
    auto trace_value = JValue {package_name};
    if (!prog->VarBinder()->GetCompilerContext()->GetMakeDoc()->secure) {
        trace_value = JValue {JArray {JValue {prog->SourceFilePath()}, JValue {prog->ResolvedFilePath()}, trace_value}};
    }
    errors_.WithTrace(std::move(trace_value), [this, prog]() {
        [[maybe_unused]] auto scope_changer = ScopedChanger(scope_, prog->Ast()->Scope());
        for (const auto &stmt : prog->Ast()->Statements()) {
            if (stmt->IsAutoGenerated()) {
                continue;
            }
            InsertStatement(stmt);
        }
    });
}

ETSDocgenImpl &ETSDocgenImpl::Run(parser::Program *prog)
{
    ctx_ = prog->VarBinder()->GetCompilerContext();
    auto doc = prog->VarBinder()->GetCompilerContext()->GetMakeDoc();
    std::regex allowed {doc ? doc.value().filter : ".*"};
    WithInserted<false>("packages", [&, this, prog]() {
        if (std::regex_match(std::string {prog->GetPackageName()}, allowed)) {
            JObject *ins {};
            WithInserted<true>(prog->GetPackageName(), [this, prog, &ins]() {
                AppendProgram(prog->GetPackageName(), prog);
                ins = &std::get<JObject>(*insert_point_);
            });
            bool non_empty = false;
            for (auto [k, v] : *ins) {
                std::ignore = k;
                if (std::holds_alternative<JObject>(v) && !std::get<JObject>(v).empty()) {
                    non_empty = true;
                    break;
                }
                if (std::holds_alternative<JArray>(v) && !std::get<JArray>(v).empty()) {
                    non_empty = true;
                    break;
                }
            }
            if (!non_empty) {
                std::get<JObject>(*insert_point_).erase(JString {prog->GetPackageName()});
            }
        }
        for (const auto &name_progs : prog->ExternalSources()) {
            if (!std::regex_match(std::string {name_progs.first.Utf8()}, allowed)) {
                continue;
            }
            WithInserted<true>(name_progs.first, [this, &name_progs]() {
                for (auto &sub_prog : name_progs.second) {
                    AppendProgram(name_progs.first, sub_prog);
                }
            });
        }
    });
    return *this;
}

std::string ETSDocgen::Generate(parser::Program *prog)
{
    return ETSDocgenImpl {}.Run(prog).Str();
}

}  // namespace panda::es2panda::docgen
