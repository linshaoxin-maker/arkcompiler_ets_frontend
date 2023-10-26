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

#include "ETSCompiler.h"

#include "checker/types/ets/etsDynamicFunctionType.h"
#include "compiler/base/condition.h"
#include "compiler/base/lreference.h"
#include "compiler/core/ETSGen.h"
#include "compiler/function/functionBuilder.h"

namespace panda::es2panda::compiler {

ETSGen *ETSCompiler::GetETSGen() const
{
    return static_cast<ETSGen *>(GetCodeGen());
}

// from as folder
void ETSCompiler::Compile([[maybe_unused]] const ir::NamedType *node) const
{
    UNREACHABLE();
}

void ETSCompiler::Compile([[maybe_unused]] const ir::PrefixAssertionExpression *expr) const
{
    UNREACHABLE();
}
// from base folder
void ETSCompiler::Compile(const ir::CatchClause *st) const
{
    ETSGen *etsg = GetETSGen();
    compiler::LocalRegScope lrs(etsg, st->Scope()->ParamScope());
    etsg->SetAccumulatorType(etsg->Checker()->GlobalETSObjectType());
    auto lref = compiler::ETSLReference::Create(etsg, st->Param(), true);
    lref.SetValue();
    st->Body()->Compile(etsg);
}

void ETSCompiler::Compile([[maybe_unused]] const ir::ClassDefinition *node) const
{
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ClassProperty *st) const
{
    ETSGen *etsg = GetETSGen();
    if (st->Value() == nullptr || (st->IsStatic() && st->TsType()->HasTypeFlag(checker::TypeFlag::CONSTANT))) {
        return;
    }

    auto ttctx = compiler::TargetTypeContext(etsg, st->TsType());
    compiler::RegScope rs(etsg);

    if (!etsg->TryLoadConstantExpression(st->Value())) {
        st->Value()->Compile(etsg);
        etsg->ApplyConversion(st->Value(), nullptr);
    }

    if (st->IsStatic()) {
        etsg->StoreStaticOwnProperty(st, st->TsType(), st->Key()->AsIdentifier()->Name());
    } else {
        etsg->StoreProperty(st, st->TsType(), etsg->GetThisReg(), st->Key()->AsIdentifier()->Name());
    }
}

void ETSCompiler::Compile([[maybe_unused]] const ir::ClassStaticBlock *st) const
{
    UNREACHABLE();
}

void ETSCompiler::Compile([[maybe_unused]] const ir::Decorator *st) const
{
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::MetaProperty *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::MethodDefinition *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::Property *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ScriptFunction *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::SpreadElement *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TemplateElement *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSIndexSignature *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSMethodSignature *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSPropertySignature *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile([[maybe_unused]] const ir::TSSignatureDeclaration *node) const
{
    UNREACHABLE();
}
// from ets folder
void ETSCompiler::Compile(const ir::ETSClassLiteral *expr) const
{
    ETSGen *etsg = GetETSGen();
    if (expr->expr_->TsType()->HasTypeFlag(checker::TypeFlag::ETS_ARRAY_OR_OBJECT)) {
        expr->expr_->Compile(etsg);
        etsg->GetType(expr, false);
    } else {
        ASSERT(expr->expr_->TsType()->HasTypeFlag(checker::TypeFlag::ETS_PRIMITIVE));
        etsg->SetAccumulatorType(expr->expr_->TsType());
        etsg->GetType(expr, true);
    }
}

void ETSCompiler::Compile([[maybe_unused]] const ir::ETSFunctionType *node) const
{
    UNREACHABLE();
}

void ETSCompiler::Compile([[maybe_unused]] const ir::ETSImportDeclaration *node) const
{
    UNREACHABLE();
}

void ETSCompiler::Compile([[maybe_unused]] const ir::ETSLaunchExpression *expr) const
{
#ifdef PANDA_WITH_ETS
    ETSGen *etsg = GetETSGen();
    compiler::RegScope rs(etsg);
    compiler::VReg callee_reg = etsg->AllocReg();
    checker::Signature *signature = expr->expr_->Signature();
    bool is_static = signature->HasSignatureFlag(checker::SignatureFlags::STATIC);
    bool is_reference = signature->HasSignatureFlag(checker::SignatureFlags::TYPE);

    if (!is_reference && expr->expr_->Callee()->IsIdentifier()) {
        if (!is_static) {
            etsg->LoadThis(expr->expr_);
            etsg->StoreAccumulator(expr, callee_reg);
        }
    } else if (!is_reference && expr->expr_->Callee()->IsMemberExpression()) {
        if (!is_static) {
            expr->expr_->Callee()->AsMemberExpression()->Object()->Compile(etsg);
            etsg->StoreAccumulator(expr, callee_reg);
        }
    } else {
        expr->expr_->Callee()->Compile(etsg);
        etsg->StoreAccumulator(expr, callee_reg);
    }

    if (is_static) {
        etsg->LaunchStatic(expr, signature, expr->expr_->Arguments());
    } else if (signature->HasSignatureFlag(checker::SignatureFlags::PRIVATE)) {
        etsg->LaunchThisStatic(expr, callee_reg, signature, expr->expr_->Arguments());
    } else {
        etsg->LaunchThisVirtual(expr, callee_reg, signature, expr->expr_->Arguments());
    }

    etsg->SetAccumulatorType(expr->TsType());
#endif  // PANDA_WITH_ETS
}

void ETSCompiler::Compile(const ir::ETSNewArrayInstanceExpression *expr) const
{
    ETSGen *etsg = GetETSGen();
    compiler::RegScope rs(etsg);
    compiler::TargetTypeContext ttctx(etsg, etsg->Checker()->GlobalIntType());

    expr->dimension_->Compile(etsg);

    compiler::VReg arr = etsg->AllocReg();
    compiler::VReg dim = etsg->AllocReg();
    etsg->ApplyConversionAndStoreAccumulator(expr, dim, expr->dimension_->TsType());
    etsg->NewArray(expr, arr, dim, expr->TsType());
    etsg->SetVRegType(arr, expr->TsType());
    etsg->LoadAccumulator(expr, arr);
}

static void CreateDynamicObject(const ir::AstNode *node, compiler::ETSGen *etsg, compiler::VReg &obj_reg,
                                ir::Expression *name, checker::Signature *signature,
                                const ArenaVector<ir::Expression *> &arguments)
{
    auto qname_reg = etsg->AllocReg();

    std::vector<util::StringView> parts;

    while (name->IsTSQualifiedName()) {
        auto *qname = name->AsTSQualifiedName();
        name = qname->Left();
        parts.push_back(qname->Right()->AsIdentifier()->Name());
    }

    auto *var = name->AsIdentifier()->Variable();
    auto *data = etsg->VarBinder()->DynamicImportDataForVar(var);
    if (data != nullptr) {
        auto *import = data->import;
        auto *specifier = data->specifier;
        ASSERT(import->Language().IsDynamic());
        etsg->LoadAccumulatorDynamicModule(node, import);
        if (specifier->IsImportSpecifier()) {
            parts.push_back(specifier->AsImportSpecifier()->Imported()->Name());
        }
    } else {
        name->Compile(etsg);
    }

    etsg->StoreAccumulator(node, obj_reg);

    std::stringstream ss;
    std::for_each(parts.rbegin(), parts.rend(), [&ss](util::StringView sv) { ss << "." << sv; });

    etsg->LoadAccumulatorString(node, util::UString(ss.str(), etsg->Allocator()).View());
    etsg->StoreAccumulator(node, qname_reg);

    etsg->CallDynamic(node, obj_reg, qname_reg, signature, arguments);
}

void ETSCompiler::Compile(const ir::ETSNewClassInstanceExpression *expr) const
{
    ETSGen *etsg = GetETSGen();
    if (expr->TsType()->IsETSDynamicType()) {
        auto obj_reg = etsg->AllocReg();
        auto *name = expr->GetTypeRef()->AsETSTypeReference()->Part()->Name();
        CreateDynamicObject(expr, etsg, obj_reg, name, expr->signature_, expr->GetArguments());
    } else {
        etsg->InitObject(expr, expr->signature_, expr->GetArguments());
    }

    if (expr->GetBoxingUnboxingFlags() == ir::BoxingUnboxingFlags::NONE) {
        etsg->SetAccumulatorType(expr->TsType());
    }
}

void ETSCompiler::Compile(const ir::ETSNewMultiDimArrayInstanceExpression *expr) const
{
    ETSGen *etsg = GetETSGen();
    etsg->InitObject(expr, expr->signature_, expr->dimensions_);
    etsg->SetAccumulatorType(expr->TsType());
}

void ETSCompiler::Compile([[maybe_unused]] const ir::ETSPackageDeclaration *st) const
{
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ETSParameterExpression *expr) const
{
    ETSGen *etsg = GetETSGen();
    expr->Ident()->Identifier::Compile(etsg);
}

void ETSCompiler::Compile([[maybe_unused]] const ir::ETSPrimitiveType *node) const
{
    UNREACHABLE();
}

void ETSCompiler::Compile([[maybe_unused]] const ir::ETSStructDeclaration *node) const
{
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ETSTypeReference *node) const
{
    ETSGen *etsg = GetETSGen();
    node->Part()->Compile(etsg);
}

void ETSCompiler::Compile(const ir::ETSTypeReferencePart *node) const
{
    ETSGen *etsg = GetETSGen();
    node->Name()->Compile(etsg);
}

void ETSCompiler::Compile(const ir::ETSUnionType *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile([[maybe_unused]] const ir::ETSWildcardType *node) const
{
    ETSGen *etsg = GetETSGen();
    etsg->Unimplemented();
}
// compile methods for EXPRESSIONS in alphabetical order
void ETSCompiler::Compile(const ir::ArrayExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ArrowFunctionExpression *expr) const
{
    ETSGen *etsg = GetETSGen();
    ASSERT(expr->TsType()->AsETSObjectType()->HasObjectFlag(checker::ETSObjectFlags::FUNCTIONAL_INTERFACE));
    ASSERT(expr->ResolvedLambda() != nullptr);
    auto *ctor = expr->ResolvedLambda()->TsType()->AsETSObjectType()->ConstructSignatures()[0];
    std::vector<compiler::VReg> arguments;

    for (auto *it : expr->CapturedVars()) {
        if (it->HasFlag(varbinder::VariableFlags::LOCAL)) {
            arguments.push_back(it->AsLocalVariable()->Vreg());
        }
    }

    if (expr->propagate_this_) {
        arguments.push_back(etsg->GetThisReg());
    }

    etsg->InitLambdaObject(expr, ctor, arguments);
    etsg->SetAccumulatorType(expr->TsType());
}

void ETSCompiler::Compile(const ir::AssignmentExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::AwaitExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::BinaryExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::CallExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ChainExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ClassExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ConditionalExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::DirectEvalExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::FunctionExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::Identifier *expr) const
{
    ETSGen *etsg = GetETSGen();
    auto lambda = etsg->VarBinder()->LambdaObjects().find(expr);
    if (lambda != etsg->VarBinder()->LambdaObjects().end()) {
        etsg->CreateLambdaObjectFromIdentReference(expr, lambda->second.first);
        return;
    }

    auto ttctx = compiler::TargetTypeContext(etsg, expr->TsType());

    ASSERT(expr->Variable() != nullptr);
    if (!expr->Variable()->HasFlag(varbinder::VariableFlags::TYPE_ALIAS)) {
        etsg->LoadVar(expr, expr->Variable());
    } else {
        etsg->LoadVar(expr, expr->TsType()->Variable());
    }
}

void ETSCompiler::Compile([[maybe_unused]] const ir::ImportExpression *expr) const
{
    UNREACHABLE();
}

static bool CompileComputed(compiler::ETSGen *etsg, const ir::MemberExpression *expr)
{
    if (expr->IsComputed()) {
        auto *const object_type = etsg->Checker()->GetNonNullishType(expr->Object()->TsType());

        auto ottctx = compiler::TargetTypeContext(etsg, expr->Object()->TsType());
        etsg->CompileAndCheck(expr->Object());

        auto const load_element = [expr, etsg, object_type]() {
            compiler::VReg obj_reg = etsg->AllocReg();
            etsg->StoreAccumulator(expr, obj_reg);

            etsg->CompileAndCheck(expr->Property());
            etsg->ApplyConversion(expr->Property(), expr->Property()->TsType());

            auto ttctx = compiler::TargetTypeContext(etsg, expr->OptionalType());

            if (object_type->IsETSDynamicType()) {
                auto lang = object_type->AsETSDynamicType()->Language();
                etsg->LoadElementDynamic(expr, obj_reg, lang);
            } else {
                etsg->LoadArrayElement(expr, obj_reg);
            }
        };

        etsg->EmitMaybeOptional(expr, load_element, expr->IsOptional());
        return true;
    }
    return false;
}

void ETSCompiler::Compile(const ir::MemberExpression *expr) const
{
    ETSGen *etsg = GetETSGen();
    auto lambda = etsg->VarBinder()->LambdaObjects().find(expr);
    if (lambda != etsg->VarBinder()->LambdaObjects().end()) {
        etsg->CreateLambdaObjectFromMemberReference(expr, expr->object_, lambda->second.first);
        etsg->SetAccumulatorType(expr->TsType());
        return;
    }

    compiler::RegScope rs(etsg);

    auto *const object_type = etsg->Checker()->GetNonNullishType(expr->Object()->TsType());

    if (CompileComputed(etsg, expr)) {
        return;
    }

    auto &prop_name = expr->Property()->AsIdentifier()->Name();

    if (object_type->IsETSArrayType() && prop_name.Is("length")) {
        auto ottctx = compiler::TargetTypeContext(etsg, object_type);
        etsg->CompileAndCheck(expr->Object());

        auto const load_length = [expr, etsg]() {
            compiler::VReg obj_reg = etsg->AllocReg();
            etsg->StoreAccumulator(expr, obj_reg);

            auto ttctx = compiler::TargetTypeContext(etsg, expr->OptionalType());
            etsg->LoadArrayLength(expr, obj_reg);
            etsg->ApplyConversion(expr, expr->TsType());
        };

        etsg->EmitMaybeOptional(expr, load_length, expr->IsOptional());
        return;
    }

    if (object_type->IsETSEnumType() || object_type->IsETSStringEnumType()) {
        auto const *const enum_interface = [object_type, expr]() -> checker::ETSEnumInterface const * {
            if (object_type->IsETSEnumType()) {
                return expr->OptionalType()->AsETSEnumType();
            }
            return expr->OptionalType()->AsETSStringEnumType();
        }();

        auto ottctx = compiler::TargetTypeContext(etsg, object_type);
        auto ttctx = compiler::TargetTypeContext(etsg, expr->OptionalType());
        etsg->LoadAccumulatorInt(expr, enum_interface->GetOrdinal());
        return;
    }

    if (etsg->Checker()->IsVariableStatic(expr->PropVar())) {
        auto ttctx = compiler::TargetTypeContext(etsg, expr->OptionalType());

        if (expr->PropVar()->TsType()->HasTypeFlag(checker::TypeFlag::GETTER_SETTER)) {
            checker::Signature *sig = expr->PropVar()->TsType()->AsETSFunctionType()->FindGetter();
            etsg->CallStatic0(expr, sig->InternalName());
            etsg->SetAccumulatorType(expr->TsType());
            return;
        }

        util::StringView full_name =
            etsg->FormClassPropReference(expr->Object()->TsType()->AsETSObjectType(), prop_name);
        etsg->LoadStaticProperty(expr, expr->OptionalType(), full_name);
        return;
    }

    auto ottctx = compiler::TargetTypeContext(etsg, expr->Object()->TsType());
    etsg->CompileAndCheck(expr->Object());

    auto const load_property = [expr, etsg, prop_name, object_type]() {
        etsg->ApplyConversion(expr->Object());
        compiler::VReg obj_reg = etsg->AllocReg();
        etsg->StoreAccumulator(expr, obj_reg);

        auto ttctx = compiler::TargetTypeContext(etsg, expr->OptionalType());

        if (expr->PropVar()->TsType()->HasTypeFlag(checker::TypeFlag::GETTER_SETTER)) {
            checker::Signature *sig = expr->PropVar()->TsType()->AsETSFunctionType()->FindGetter();
            etsg->CallThisVirtual0(expr, obj_reg, sig->InternalName());
            etsg->SetAccumulatorType(expr->TsType());
        } else if (object_type->IsETSDynamicType()) {
            auto lang = object_type->AsETSDynamicType()->Language();
            etsg->LoadPropertyDynamic(expr, expr->OptionalType(), obj_reg, prop_name, lang);
        } else if (object_type->IsETSUnionType()) {
            etsg->LoadUnionProperty(expr, expr->OptionalType(), obj_reg, prop_name);
        } else {
            const auto full_name = etsg->FormClassPropReference(object_type->AsETSObjectType(), prop_name);
            etsg->LoadProperty(expr, expr->OptionalType(), obj_reg, full_name);
        }
    };

    etsg->EmitMaybeOptional(expr, load_property, expr->IsOptional());
}

void ETSCompiler::Compile([[maybe_unused]] const ir::NewExpression *expr) const
{
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ObjectExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile([[maybe_unused]] const ir::OmittedExpression *expr) const
{
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::OpaqueTypeNode *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::SequenceExpression *expr) const
{
    ETSGen *etsg = GetETSGen();
    for (const auto *it : expr->Sequence()) {
        it->Compile(etsg);
    }
}

void ETSCompiler::Compile(const ir::SuperExpression *expr) const
{
    ETSGen *etsg = GetETSGen();
    etsg->LoadThis(expr);
    etsg->SetAccumulatorType(etsg->GetAccumulatorType()->AsETSObjectType()->SuperType());
}

void ETSCompiler::Compile(const ir::TaggedTemplateExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TemplateLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ThisExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::UnaryExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::UpdateExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::YieldExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}
// compile methods for LITERAL EXPRESSIONS in alphabetical order
void ETSCompiler::Compile([[maybe_unused]] const ir::BigIntLiteral *expr) const
{
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::BooleanLiteral *expr) const
{
    ETSGen *etsg = GetETSGen();
    etsg->LoadAccumulatorBoolean(expr, expr->Value());
}

void ETSCompiler::Compile(const ir::CharLiteral *expr) const
{
    ETSGen *etsg = GetETSGen();
    etsg->LoadAccumulatorChar(expr, expr->Char());
}

void ETSCompiler::Compile(const ir::NullLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::NumberLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::RegExpLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::StringLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::UndefinedLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

// compile methods for MODULE-related nodes in alphabetical order
void ETSCompiler::Compile(const ir::ExportAllDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ExportDefaultDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ExportNamedDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ExportSpecifier *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ImportDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ImportDefaultSpecifier *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ImportNamespaceSpecifier *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ImportSpecifier *st) const
{
    (void)st;
    UNREACHABLE();
}

static void ThrowError(compiler::ETSGen *const etsg, const ir::AssertStatement *st)
{
    const compiler::RegScope rs(etsg);

    if (st->Second() != nullptr) {
        st->Second()->Compile(etsg);
    } else {
        etsg->LoadAccumulatorString(st, "Assertion failed.");
    }

    const auto message = etsg->AllocReg();
    etsg->StoreAccumulator(st, message);

    const auto assertion_error = etsg->AllocReg();
    etsg->NewObject(st, assertion_error, compiler::Signatures::BUILTIN_ASSERTION_ERROR);
    etsg->CallThisStatic1(st, assertion_error, compiler::Signatures::BUILTIN_ASSERTION_ERROR_CTOR, message);
    etsg->EmitThrow(st, assertion_error);
}
// compile methods for STATEMENTS in alphabetical order
void ETSCompiler::Compile(const ir::AssertStatement *st) const
{
    ETSGen *etsg = GetETSGen();
    auto res = compiler::Condition::CheckConstantExpr(etsg, st->Test());
    if (res == compiler::Condition::Result::CONST_TRUE) {
        return;
    }

    if (res == compiler::Condition::Result::CONST_FALSE) {
        ThrowError(etsg, st);
        return;
    }

    compiler::Label *true_label = etsg->AllocLabel();
    compiler::Label *false_label = etsg->AllocLabel();

    compiler::Condition::Compile(etsg, st->Test(), false_label);
    etsg->JumpTo(st, true_label);

    etsg->SetLabel(st, false_label);
    ThrowError(etsg, st);

    etsg->SetLabel(st, true_label);
}

void ETSCompiler::Compile(const ir::BlockStatement *st) const
{
    ETSGen *etsg = GetETSGen();
    compiler::LocalRegScope lrs(etsg, st->Scope());

    etsg->CompileStatements(st->Statements());
}

template <typename CodeGen>
static void CompileImpl(const ir::BreakStatement *self, [[maybe_unused]] CodeGen *cg)
{
    compiler::Label *target = cg->ControlFlowChangeBreak(self->Ident());
    cg->Branch(self, target);
}

void ETSCompiler::Compile(const ir::BreakStatement *st) const
{
    ETSGen *etsg = GetETSGen();
    if (etsg->ExtendWithFinalizer(st->parent_, st)) {
        return;
    }
    CompileImpl(st, etsg);
}

void ETSCompiler::Compile(const ir::ClassDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ContinueStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::DebuggerStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::DoWhileStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::EmptyStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ExpressionStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ForInStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ForOfStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ForUpdateStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::FunctionDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::IfStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::LabelledStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ReturnStatement *st) const
{
    ETSGen *etsg = GetETSGen();
    if (st->Argument() == nullptr) {
        if (st->ReturnType() == nullptr || st->ReturnType()->IsETSVoidType()) {
            if (etsg->ExtendWithFinalizer(st->parent_, st)) {
                return;
            }

            if (etsg->CheckControlFlowChange()) {
                etsg->ControlFlowChangeBreak();
            }
            etsg->EmitReturnVoid(st);
            return;
        }

        etsg->LoadBuiltinVoid(st);
    } else {
        auto ttctx = compiler::TargetTypeContext(etsg, etsg->ReturnType());

        if (!etsg->TryLoadConstantExpression(st->Argument())) {
            st->Argument()->Compile(etsg);
        }
        etsg->ApplyConversion(st->Argument(), nullptr);
        etsg->ApplyConversion(st->Argument(), st->ReturnType());
    }

    if (etsg->ExtendWithFinalizer(st->parent_, st)) {
        return;
    }

    if (etsg->CheckControlFlowChange()) {
        compiler::RegScope rs(etsg);
        compiler::VReg res = etsg->AllocReg();

        etsg->StoreAccumulator(st, res);
        etsg->ControlFlowChangeBreak();
        etsg->LoadAccumulator(st, res);
    }

    etsg->ReturnAcc(st);
}

void ETSCompiler::Compile(const ir::SwitchCaseStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::SwitchStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ThrowStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TryStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::VariableDeclarator *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::VariableDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::WhileStatement *st) const
{
    (void)st;
    UNREACHABLE();
}
// from ts folder
void ETSCompiler::Compile(const ir::TSAnyKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSArrayType *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSAsExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSBigintKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSBooleanKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSClassImplements *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSConditionalType *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSConstructorType *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSEnumDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSEnumMember *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSExternalModuleReference *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSFunctionType *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSImportEqualsDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSImportType *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSIndexedAccessType *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSInferType *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSInterfaceBody *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSInterfaceDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSInterfaceHeritage *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSIntersectionType *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSLiteralType *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSMappedType *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSModuleBlock *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSModuleDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSNamedTupleMember *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSNeverKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSNonNullExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSNullKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSNumberKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSObjectKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSParameterProperty *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSParenthesizedType *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSQualifiedName *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSStringKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSThisType *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSTupleType *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSTypeAliasDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSTypeAssertion *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSTypeLiteral *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSTypeOperator *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSTypeParameter *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSTypeParameterDeclaration *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSTypeParameterInstantiation *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSTypePredicate *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSTypeQuery *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSTypeReference *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSUndefinedKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSUnionType *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSUnknownKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::TSVoidKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

}  // namespace panda::es2panda::compiler