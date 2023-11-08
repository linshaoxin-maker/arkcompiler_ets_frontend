/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

void ETSCompiler::Compile(const ir::TSSignatureDeclaration *node) const
{
    (void)node;
    UNREACHABLE();
}
// from ets folder
void ETSCompiler::Compile(const ir::ETSClassLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ETSFunctionType *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ETSImportDeclaration *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ETSLaunchExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ETSNewArrayInstanceExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ETSNewClassInstanceExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ETSNewMultiDimArrayInstanceExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ETSPackageDeclaration *st) const
{
    (void)st;
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
    ASSERT(expr->ResolvedLambda() != nullptr);
    auto *ctor = expr->ResolvedLambda()->TsType()->AsETSObjectType()->ConstructSignatures()[0];
    std::vector<compiler::VReg> arguments;

    for (auto *it : expr->CapturedVars()) {
        if (it->HasFlag(binder::VariableFlags::LOCAL)) {
            arguments.push_back(it->AsLocalVariable()->Vreg());
        }
    }

    if (expr->propagate_this_) {
        arguments.push_back(etsg->GetThisReg());
    }

    etsg->InitLambdaObject(expr, ctor, arguments);
    etsg->SetAccumulatorType(expr->resolved_lambda_->TsType());
}

void ETSCompiler::Compile(const ir::AssignmentExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::AwaitExpression *expr) const
{
    ETSGen *etsg = GetETSGen();
    static constexpr bool IS_UNCHECKED_CAST = false;
    compiler::RegScope rs(etsg);
    compiler::VReg argument_reg = etsg->AllocReg();
    expr->Argument()->Compile(etsg);
    etsg->StoreAccumulator(expr, argument_reg);
    etsg->CallThisVirtual0(expr->Argument(), argument_reg, compiler::Signatures::BUILTIN_PROMISE_AWAIT_RESOLUTION);
    etsg->CastToArrayOrObject(expr->Argument(), expr->TsType(), IS_UNCHECKED_CAST);
    etsg->SetAccumulatorType(expr->TsType());
}

static void CompileLogical(compiler::ETSGen *etsg, const ir::BinaryExpression *expr)
{
    auto *end_label = etsg->AllocLabel();

    if (expr->OperatorType() == lexer::TokenType::PUNCTUATOR_NULLISH_COALESCING) {
        expr->Left()->Compile(etsg);
        etsg->ApplyConversion(expr->Left(), expr->OperationType());
        etsg->BranchIfNotNull(expr, end_label);
        expr->Right()->Compile(etsg);
        etsg->ApplyConversion(expr->Right(), expr->OperationType());
        etsg->SetLabel(expr, end_label);
        return;
    }

    ASSERT(expr->IsLogicalExtended());
    auto ttctx = compiler::TargetTypeContext(etsg, expr->OperationType());
    compiler::RegScope rs(etsg);
    auto lhs = etsg->AllocReg();
    auto rhs = etsg->AllocReg();
    expr->Left()->Compile(etsg);
    etsg->ApplyConversionAndStoreAccumulator(expr->Left(), lhs, expr->OperationType());

    auto left_false_label = etsg->AllocLabel();
    if (expr->OperatorType() == lexer::TokenType::PUNCTUATOR_LOGICAL_AND) {
        etsg->ResolveConditionalResultIfFalse(expr->Left(), left_false_label);
        etsg->BranchIfFalse(expr, left_false_label);

        expr->Right()->Compile(etsg);
        etsg->ApplyConversionAndStoreAccumulator(expr->Right(), rhs, expr->OperationType());
        etsg->Branch(expr, end_label);

        etsg->SetLabel(expr, left_false_label);
        etsg->LoadAccumulator(expr, lhs);
    } else {
        etsg->ResolveConditionalResultIfFalse(expr->Left(), left_false_label);
        etsg->BranchIfFalse(expr, left_false_label);

        etsg->LoadAccumulator(expr, lhs);
        etsg->Branch(expr, end_label);

        etsg->SetLabel(expr, left_false_label);
        expr->Right()->Compile(etsg);
        etsg->ApplyConversionAndStoreAccumulator(expr->Right(), rhs, expr->OperationType());
    }

    etsg->SetLabel(expr, end_label);
}

void ETSCompiler::Compile(const ir::BinaryExpression *expr) const
{
    ETSGen *etsg = GetETSGen();
    if (etsg->TryLoadConstantExpression(expr)) {
        return;
    }

    auto ttctx = compiler::TargetTypeContext(etsg, expr->OperationType());

    if (expr->IsLogical()) {
        CompileLogical(etsg, expr);
        etsg->ApplyConversion(expr, expr->OperationType());
        return;
    }

    compiler::RegScope rs(etsg);
    compiler::VReg lhs = etsg->AllocReg();

    if (expr->OperatorType() == lexer::TokenType::PUNCTUATOR_PLUS &&
        (expr->Left()->TsType()->IsETSStringType() || expr->Right()->TsType()->IsETSStringType())) {
        etsg->BuildString(expr);
        return;
    }

    expr->Left()->Compile(etsg);
    etsg->ApplyConversionAndStoreAccumulator(expr->Left(), lhs, expr->OperationType());
    expr->Right()->Compile(etsg);
    etsg->ApplyConversion(expr->Right(), expr->OperationType());
    if (expr->OperatorType() >= lexer::TokenType::PUNCTUATOR_LEFT_SHIFT &&
        expr->OperatorType() <= lexer::TokenType::PUNCTUATOR_UNSIGNED_RIGHT_SHIFT) {
        etsg->ApplyCast(expr->Right(), expr->OperationType());
    }

    etsg->Binary(expr, expr->OperatorType(), lhs);
}

static void ConvertRestArguments(checker::ETSChecker *const checker, const ir::CallExpression *expr)
{
    if (expr->Signature()->RestVar() != nullptr) {
        std::size_t const argument_count = expr->Arguments().size();
        std::size_t const parameter_count = expr->Signature()->MinArgCount();
        ASSERT(argument_count >= parameter_count);

        auto &arguments = const_cast<ArenaVector<ir::Expression *> &>(expr->Arguments());
        std::size_t i = parameter_count;

        if (i < argument_count && expr->Arguments()[i]->IsSpreadElement()) {
            arguments[i] = expr->Arguments()[i]->AsSpreadElement()->Argument();
        } else {
            ArenaVector<ir::Expression *> elements(checker->Allocator()->Adapter());
            for (; i < argument_count; ++i) {
                elements.emplace_back(expr->Arguments()[i]);
            }
            auto *array_expression = checker->AllocNode<ir::ArrayExpression>(std::move(elements), checker->Allocator());
            array_expression->SetParent(const_cast<ir::CallExpression *>(expr));
            array_expression->SetTsType(expr->Signature()->RestVar()->TsType());
            arguments.erase(expr->Arguments().begin() + parameter_count, expr->Arguments().end());
            arguments.emplace_back(array_expression);
        }
    }
}

void ETSCompiler::Compile(const ir::CallExpression *expr) const
{
    ETSGen *etsg = GetETSGen();
    compiler::RegScope rs(etsg);
    compiler::VReg callee_reg = etsg->AllocReg();

    const auto is_proxy = expr->Signature()->HasSignatureFlag(checker::SignatureFlags::PROXY);

    if (is_proxy && expr->Callee()->IsMemberExpression()) {
        auto *const callee_object = expr->callee_->AsMemberExpression()->Object();

        auto const *const enum_interface = [callee_type =
                                                callee_object->TsType()]() -> checker::ETSEnumInterface const * {
            if (callee_type->IsETSEnumType()) {
                return callee_type->AsETSEnumType();
            }
            if (callee_type->IsETSStringEnumType()) {
                return callee_type->AsETSStringEnumType();
            }
            return nullptr;
        }();

        if (enum_interface != nullptr) {
            ArenaVector<ir::Expression *> arguments(etsg->Allocator()->Adapter());

            checker::Signature *const signature = [expr, callee_object, enum_interface, &arguments]() {
                const auto &member_proxy_method_name = expr->signature_->InternalName();

                if (member_proxy_method_name == checker::ETSEnumType::TO_STRING_METHOD_NAME) {
                    arguments.push_back(callee_object);
                    return enum_interface->ToStringMethod().global_signature;
                }
                if (member_proxy_method_name == checker::ETSEnumType::GET_VALUE_METHOD_NAME) {
                    arguments.push_back(callee_object);
                    return enum_interface->GetValueMethod().global_signature;
                }
                if (member_proxy_method_name == checker::ETSEnumType::GET_NAME_METHOD_NAME) {
                    arguments.push_back(callee_object);
                    return enum_interface->GetNameMethod().global_signature;
                }
                if (member_proxy_method_name == checker::ETSEnumType::VALUES_METHOD_NAME) {
                    return enum_interface->ValuesMethod().global_signature;
                }
                if (member_proxy_method_name == checker::ETSEnumType::VALUE_OF_METHOD_NAME) {
                    arguments.push_back(expr->Arguments().front());
                    return enum_interface->ValueOfMethod().global_signature;
                }
                UNREACHABLE();
            }();

            ASSERT(signature->ReturnType() == expr->Signature()->ReturnType());
            etsg->CallStatic(expr, signature, arguments);
            etsg->SetAccumulatorType(expr->TsType());
            return;
        }
    }

    bool is_static = expr->Signature()->HasSignatureFlag(checker::SignatureFlags::STATIC);
    bool is_reference = expr->Signature()->HasSignatureFlag(checker::SignatureFlags::TYPE);
    bool is_dynamic = expr->Callee()->TsType()->HasTypeFlag(checker::TypeFlag::ETS_DYNAMIC_FLAG);

    ConvertRestArguments(const_cast<checker::ETSChecker *>(etsg->Checker()->AsETSChecker()), expr);

    compiler::VReg dyn_param2;

    // Helper function to avoid branching in non optional cases
    auto emit_arguments = [expr, etsg, is_static, is_dynamic, &callee_reg, &dyn_param2]() {
        if (is_dynamic) {
            etsg->CallDynamic(expr, callee_reg, dyn_param2, expr->Signature(), expr->Arguments());
        } else if (is_static) {
            etsg->CallStatic(expr, expr->Signature(), expr->Arguments());
        } else if (expr->Signature()->HasSignatureFlag(checker::SignatureFlags::PRIVATE) ||
                   expr->IsETSConstructorCall() ||
                   (expr->Callee()->IsMemberExpression() &&
                    expr->Callee()->AsMemberExpression()->Object()->IsSuperExpression())) {
            etsg->CallThisStatic(expr, callee_reg, expr->Signature(), expr->Arguments());
        } else {
            etsg->CallThisVirtual(expr, callee_reg, expr->Signature(), expr->Arguments());
        }

        if (expr->GetBoxingUnboxingFlags() != ir::BoxingUnboxingFlags::NONE) {
            etsg->ApplyConversion(expr, nullptr);
        } else {
            etsg->SetAccumulatorType(expr->Signature()->ReturnType());
        }
    };

    if (is_dynamic) {
        dyn_param2 = etsg->AllocReg();

        ir::Expression *obj = expr->callee_;
        std::vector<util::StringView> parts;

        while (obj->IsMemberExpression() && obj->AsMemberExpression()->ObjType()->IsETSDynamicType()) {
            auto *mem_expr = obj->AsMemberExpression();
            obj = mem_expr->Object();
            parts.push_back(mem_expr->Property()->AsIdentifier()->Name());
        }

        if (!obj->IsMemberExpression() && obj->IsIdentifier()) {
            auto *var = obj->AsIdentifier()->Variable();
            auto *data = etsg->Binder()->DynamicImportDataForVar(var);
            if (data != nullptr) {
                auto *import = data->import;
                auto *specifier = data->specifier;
                ASSERT(import->Language().IsDynamic());
                etsg->LoadAccumulatorDynamicModule(expr, import);
                if (specifier->IsImportSpecifier()) {
                    parts.push_back(specifier->AsImportSpecifier()->Imported()->Name());
                }
            } else {
                obj->Compile(etsg);
            }
        } else {
            obj->Compile(etsg);
        }

        etsg->StoreAccumulator(expr, callee_reg);

        if (!parts.empty()) {
            std::stringstream ss;
            for_each(parts.rbegin(), parts.rend(), [&ss](util::StringView sv) { ss << "." << sv; });

            etsg->LoadAccumulatorString(expr, util::UString(ss.str(), etsg->Allocator()).View());
        } else {
            auto lang = expr->Callee()->TsType()->IsETSDynamicFunctionType()
                            ? expr->Callee()->TsType()->AsETSDynamicFunctionType()->Language()
                            : expr->Callee()->TsType()->AsETSDynamicType()->Language();

            etsg->LoadUndefinedDynamic(expr, lang);
        }

        etsg->StoreAccumulator(expr, dyn_param2);

        emit_arguments();

        if (expr->Signature()->ReturnType() != expr->TsType()) {
            etsg->ApplyConversion(expr, expr->TsType());
        }
    } else if (!is_reference && expr->Callee()->IsIdentifier()) {
        if (!is_static) {
            etsg->LoadThis(expr);
            etsg->StoreAccumulator(expr, callee_reg);
        }
        emit_arguments();
    } else if (!is_reference && expr->Callee()->IsMemberExpression()) {
        if (!is_static) {
            expr->Callee()->AsMemberExpression()->Object()->Compile(etsg);
            etsg->StoreAccumulator(expr, callee_reg);
        }
        emit_arguments();
    } else {
        expr->Callee()->Compile(etsg);
        etsg->StoreAccumulator(expr, callee_reg);
        if (expr->optional_) {
            compiler::Label *end_label = etsg->AllocLabel();
            etsg->BranchIfNull(expr, end_label);
            emit_arguments();
            etsg->SetLabel(expr, end_label);
        } else {
            emit_arguments();
        }
    }
}

void ETSCompiler::Compile([[maybe_unused]] const ir::ChainExpression *expr) const
{
    UNREACHABLE();
}

void ETSCompiler::Compile([[maybe_unused]] const ir::ClassExpression *expr) const
{
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ConditionalExpression *expr) const
{
    ETSGen *etsg = GetETSGen();
    auto *false_label = etsg->AllocLabel();
    auto *end_label = etsg->AllocLabel();

    compiler::Condition::Compile(etsg, expr->Test(), false_label);
    expr->Consequent()->Compile(etsg);
    etsg->ApplyConversion(expr->Consequent());
    etsg->Branch(expr, end_label);
    etsg->SetLabel(expr, false_label);
    expr->Alternate()->Compile(etsg);
    etsg->ApplyConversion(expr->Alternate());
    etsg->SetLabel(expr, end_label);
}

void ETSCompiler::Compile([[maybe_unused]] const ir::DirectEvalExpression *expr) const
{
    UNREACHABLE();
}

void ETSCompiler::Compile([[maybe_unused]] const ir::FunctionExpression *expr) const
{
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::Identifier *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ImportExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::MemberExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::NewExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::ObjectExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::OmittedExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::OpaqueTypeNode *node) const
{
    (void)node;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::SequenceExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::SuperExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
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
void ETSCompiler::Compile(const ir::BigIntLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::BooleanLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::CharLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
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
// compile methods for STATEMENTS in alphabetical order
void ETSCompiler::Compile(const ir::AssertStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::BlockStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

void ETSCompiler::Compile(const ir::BreakStatement *st) const
{
    (void)st;
    UNREACHABLE();
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

    // etsg->ApplyConversion(st, st->ReturnType());
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