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

#include "assignmentExpression.h"

#include "compiler/base/lreference.h"
#include "compiler/core/pandagen.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/regScope.h"
#include "ir/astDump.h"
#include "ir/base/scriptFunction.h"
#include "ir/base/spreadElement.h"
#include "ir/expressions/identifier.h"
#include "ir/expressions/arrayExpression.h"
#include "ir/expressions/binaryExpression.h"
#include "ir/expressions/objectExpression.h"
#include "ir/expressions/memberExpression.h"

#include "checker/TSchecker.h"
#include "checker/ts/destructuringContext.h"
#include "checker/ets/typeRelationContext.h"

namespace panda::es2panda::ir {
bool AssignmentExpression::ConvertibleToAssignmentPattern(bool must_be_pattern)
{
    bool conv_result = true;

    switch (left_->Type()) {
        case AstNodeType::ARRAY_EXPRESSION: {
            conv_result = left_->AsArrayExpression()->ConvertibleToArrayPattern();
            break;
        }
        case AstNodeType::SPREAD_ELEMENT: {
            conv_result = must_be_pattern && left_->AsSpreadElement()->ConvertibleToRest(false);
            break;
        }
        case AstNodeType::OBJECT_EXPRESSION: {
            conv_result = left_->AsObjectExpression()->ConvertibleToObjectPattern();
            break;
        }
        case AstNodeType::ASSIGNMENT_EXPRESSION: {
            conv_result = left_->AsAssignmentExpression()->ConvertibleToAssignmentPattern(must_be_pattern);
            break;
        }
        case AstNodeType::META_PROPERTY_EXPRESSION:
        case AstNodeType::CHAIN_EXPRESSION: {
            conv_result = false;
            break;
        }
        default: {
            break;
        }
    }

    if (must_be_pattern) {
        SetType(AstNodeType::ASSIGNMENT_PATTERN);
    }

    if (!right_->IsAssignmentExpression()) {
        return conv_result;
    }

    switch (right_->Type()) {
        case AstNodeType::ARRAY_EXPRESSION: {
            conv_result = right_->AsArrayExpression()->ConvertibleToArrayPattern();
            break;
        }
        case AstNodeType::CHAIN_EXPRESSION:
        case AstNodeType::SPREAD_ELEMENT: {
            conv_result = false;
            break;
        }
        case AstNodeType::OBJECT_EXPRESSION: {
            conv_result = right_->AsObjectExpression()->ConvertibleToObjectPattern();
            break;
        }
        case AstNodeType::ASSIGNMENT_EXPRESSION: {
            conv_result = right_->AsAssignmentExpression()->ConvertibleToAssignmentPattern(false);
            break;
        }
        default: {
            break;
        }
    }

    return conv_result;
}

void AssignmentExpression::TransformChildren(const NodeTransformer &cb)
{
    left_ = cb(left_)->AsExpression();
    right_ = cb(right_)->AsExpression();
}

void AssignmentExpression::Iterate(const NodeTraverser &cb) const
{
    cb(left_);
    cb(right_);
}

void AssignmentExpression::Dump(ir::AstDumper *dumper) const
{
    if (type_ == AstNodeType::ASSIGNMENT_EXPRESSION) {
        dumper->Add({{"type", "AssignmentExpression"}, {"operator", operator_}, {"left", left_}, {"right", right_}});
    } else {
        dumper->Add({{"type", "AssignmentPattern"}, {"left", left_}, {"right", right_}});
    }
}

void AssignmentExpression::Compile(compiler::PandaGen *pg) const
{
    compiler::RegScope rs(pg);
    auto lref = compiler::JSLReference::Create(pg, left_, false);

    if (operator_ == lexer::TokenType::PUNCTUATOR_LOGICAL_AND_EQUAL ||
        operator_ == lexer::TokenType::PUNCTUATOR_LOGICAL_OR_EQUAL) {
        compiler::PandaGen::Unimplemented();
    }

    if (operator_ == lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
        right_->Compile(pg);
        lref.SetValue();
        return;
    }

    compiler::VReg lhs_reg = pg->AllocReg();

    lref.GetValue();
    pg->StoreAccumulator(left_, lhs_reg);
    right_->Compile(pg);
    pg->Binary(this, operator_, lhs_reg);

    lref.SetValue();
}

void AssignmentExpression::Compile(compiler::ETSGen *etsg) const
{
    // All other operations are handled in OpAssignmentLowering
    ASSERT(operator_ == lexer::TokenType::PUNCTUATOR_SUBSTITUTION);
    compiler::RegScope rs(etsg);
    auto lref = compiler::ETSLReference::Create(etsg, left_, false);
    auto ttctx = compiler::TargetTypeContext(etsg, TsType());

    if (right_->IsNullLiteral()) {
        etsg->LoadAccumulatorNull(this, left_->TsType());
    } else {
        right_->Compile(etsg);
        etsg->ApplyConversion(right_, TsType());
    }
    lref.SetValue();
}

void AssignmentExpression::CompilePattern(compiler::PandaGen *pg) const
{
    compiler::RegScope rs(pg);
    auto lref = compiler::JSLReference::Create(pg, left_, false);
    right_->Compile(pg);
    lref.SetValue();
}

checker::Type *AssignmentExpression::Check(checker::TSChecker *checker)
{
    if (left_->IsArrayPattern()) {
        auto saved_context = checker::SavedCheckerContext(checker, checker::CheckerStatus::FORCE_TUPLE);
        auto destructuring_context = checker::ArrayDestructuringContext(checker, left_, true, true, nullptr, right_);
        destructuring_context.Start();
        return destructuring_context.InferredType();
    }

    if (left_->IsObjectPattern()) {
        auto saved_context = checker::SavedCheckerContext(checker, checker::CheckerStatus::FORCE_TUPLE);
        auto destructuring_context = checker::ObjectDestructuringContext(checker, left_, true, true, nullptr, right_);
        destructuring_context.Start();
        return destructuring_context.InferredType();
    }

    if (left_->IsIdentifier() && left_->AsIdentifier()->Variable() != nullptr &&
        left_->AsIdentifier()->Variable()->Declaration()->IsConstDecl()) {
        checker->ThrowTypeError({"Cannot assign to ", left_->AsIdentifier()->Name(), " because it is a constant."},
                                left_->Start());
    }

    auto *left_type = left_->Check(checker);

    if (left_type->HasTypeFlag(checker::TypeFlag::READONLY)) {
        checker->ThrowTypeError("Cannot assign to this property because it is readonly.", left_->Start());
    }

    if (operator_ == lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
        checker->ElaborateElementwise(left_type, right_, left_->Start());
        return checker->CheckTypeCached(right_);
    }

    auto *right_type = right_->Check(checker);

    switch (operator_) {
        case lexer::TokenType::PUNCTUATOR_MULTIPLY_EQUAL:
        case lexer::TokenType::PUNCTUATOR_EXPONENTIATION_EQUAL:
        case lexer::TokenType::PUNCTUATOR_DIVIDE_EQUAL:
        case lexer::TokenType::PUNCTUATOR_MOD_EQUAL:
        case lexer::TokenType::PUNCTUATOR_MINUS_EQUAL:
        case lexer::TokenType::PUNCTUATOR_LEFT_SHIFT_EQUAL:
        case lexer::TokenType::PUNCTUATOR_RIGHT_SHIFT_EQUAL:
        case lexer::TokenType::PUNCTUATOR_UNSIGNED_RIGHT_SHIFT_EQUAL:
        case lexer::TokenType::PUNCTUATOR_BITWISE_AND_EQUAL:
        case lexer::TokenType::PUNCTUATOR_BITWISE_XOR_EQUAL:
        case lexer::TokenType::PUNCTUATOR_BITWISE_OR_EQUAL: {
            return checker->CheckBinaryOperator(left_type, right_type, left_, right_, this, operator_);
        }
        case lexer::TokenType::PUNCTUATOR_PLUS_EQUAL: {
            return checker->CheckPlusOperator(left_type, right_type, left_, right_, this, operator_);
        }
        case lexer::TokenType::PUNCTUATOR_SUBSTITUTION: {
            checker->CheckAssignmentOperator(operator_, left_, left_type, right_type);
            return right_type;
        }
        default: {
            UNREACHABLE();
            break;
        }
    }

    return nullptr;
}

checker::Type *AssignmentExpression::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    if (TsType() != nullptr) {
        return TsType();
    }

    auto *left_type = left_->Check(checker);
    if (left_->IsMemberExpression() && left_->AsMemberExpression()->Object()->TsType()->IsETSArrayType() &&
        left_->AsMemberExpression()->Property()->IsIdentifier() &&
        left_->AsMemberExpression()->Property()->AsIdentifier()->Name().Is("length")) {
        checker->ThrowTypeError("Setting the length of an array is not permitted", left_->Start());
    }

    if (left_->IsIdentifier()) {
        target_ = left_->AsIdentifier()->Variable();
    } else {
        target_ = left_->AsMemberExpression()->PropVar();
    }

    if (target_ != nullptr) {
        checker->ValidateUnaryOperatorOperand(target_);
    }

    checker::Type *source_type {};
    ir::Expression *relation_node = right_;
    switch (operator_) {
        case lexer::TokenType::PUNCTUATOR_MULTIPLY_EQUAL:
        case lexer::TokenType::PUNCTUATOR_EXPONENTIATION_EQUAL:
        case lexer::TokenType::PUNCTUATOR_DIVIDE_EQUAL:
        case lexer::TokenType::PUNCTUATOR_MOD_EQUAL:
        case lexer::TokenType::PUNCTUATOR_MINUS_EQUAL:
        case lexer::TokenType::PUNCTUATOR_LEFT_SHIFT_EQUAL:
        case lexer::TokenType::PUNCTUATOR_RIGHT_SHIFT_EQUAL:
        case lexer::TokenType::PUNCTUATOR_UNSIGNED_RIGHT_SHIFT_EQUAL:
        case lexer::TokenType::PUNCTUATOR_BITWISE_AND_EQUAL:
        case lexer::TokenType::PUNCTUATOR_BITWISE_XOR_EQUAL:
        case lexer::TokenType::PUNCTUATOR_BITWISE_OR_EQUAL:
        case lexer::TokenType::PUNCTUATOR_PLUS_EQUAL: {
            std::tie(std::ignore, operation_type_) =
                checker->CheckBinaryOperator(left_, right_, this, operator_, Start(), true);

            auto unboxed_left = checker->ETSBuiltinTypeAsPrimitiveType(left_type);
            source_type = unboxed_left == nullptr ? left_type : unboxed_left;

            relation_node = this;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_SUBSTITUTION: {
            if (left_type->IsETSArrayType() && right_->IsArrayExpression()) {
                right_->AsArrayExpression()->SetPreferredType(left_type->AsETSArrayType()->ElementType());
            }
            if (right_->IsObjectExpression()) {
                right_->AsObjectExpression()->SetPreferredType(left_type);
            }

            source_type = right_->Check(checker);
            break;
        }
        default: {
            UNREACHABLE();
            break;
        }
    }

    checker::AssignmentContext(checker->Relation(), relation_node, source_type, left_type, right_->Start(),
                               {"Initializers type is not assignable to the target type"});

    SetTsType(left_->TsType());
    return TsType();
}

AssignmentExpression::AssignmentExpression([[maybe_unused]] Tag const tag, AssignmentExpression const &other,
                                           Expression *const left, Expression *const right)
    : AssignmentExpression(other)
{
    left_ = left;
    if (left_ != nullptr) {
        left_->SetParent(this);
    }

    right_ = right;
    if (right_ != nullptr) {
        right_->SetParent(this);
    }
}

// NOLINTNEXTLINE(google-default-arguments)
AssignmentExpression *AssignmentExpression::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    auto *const left = left_ != nullptr ? left_->Clone(allocator)->AsExpression() : nullptr;
    auto *const right = right_ != nullptr ? right_->Clone(allocator)->AsExpression() : nullptr;

    if (auto *const clone = allocator->New<AssignmentExpression>(Tag {}, *this, left, right); clone != nullptr) {
        if (parent != nullptr) {
            clone->SetParent(parent);
        }
        return clone;
    }

    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}
}  // namespace panda::es2panda::ir
