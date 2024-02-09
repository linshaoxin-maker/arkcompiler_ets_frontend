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

#ifndef ES2PANDA_CHECKER_TS_DESTRUCTURING_CONTEXT_H
#define ES2PANDA_CHECKER_TS_DESTRUCTURING_CONTEXT_H

#include "checker/TSchecker.h"
#include "ir/expression.h"

#include <macros.h>

namespace ark::es2panda::ir {
class Expression;
class SpreadElement;
}  // namespace ark::es2panda::ir

namespace ark::es2panda::checker {
class Type;

class DestructuringContext {
public:
    DestructuringContext(TSChecker *checker, ir::Expression *id, bool inAssignment, bool convertTupleToArray,
                         ir::TypeNode *typeAnnotation, ir::Expression *initializer)
        : checker_(checker), id_(id), inAssignment_(inAssignment), convertTupleToArray_(convertTupleToArray)
    {
        Prepare(typeAnnotation, initializer, id->Start());
    }

    void SetInferredType(Type *type)
    {
        inferredType_ = type;
    }

    void SetSignatureInfo(SignatureInfo *info)
    {
        signatureInfo_ = info;
    }

    Type *InferredType()
    {
        return inferredType_;
    }

    void ValidateObjectLiteralType(ObjectType *objType, ir::ObjectExpression *objPattern);
    void HandleDestructuringAssignment(ir::Identifier *ident, Type *inferredType, Type *defaultType);
    void HandleAssignmentPattern(ir::AssignmentExpression *assignmentPattern, Type *inferredType, bool validateDefault);
    void SetInferredTypeForVariable(varbinder::Variable *var, Type *inferredType, const lexer::SourcePosition &loc);
    void Prepare(ir::TypeNode *typeAnnotation, ir::Expression *initializer, const lexer::SourcePosition &loc);

    DEFAULT_COPY_SEMANTIC(DestructuringContext);
    DEFAULT_MOVE_SEMANTIC(DestructuringContext);
    ~DestructuringContext() = default;

    virtual void Start() = 0;
    virtual void ValidateInferredType() = 0;
    virtual Type *NextInferredType([[maybe_unused]] const util::StringView &searchName, bool throwError) = 0;
    virtual void HandleRest(ir::SpreadElement *rest) = 0;
    virtual Type *GetRestType([[maybe_unused]] const lexer::SourcePosition &loc) = 0;
    virtual Type *ConvertTupleTypeToArrayTypeIfNecessary(ir::AstNode *node, Type *type) = 0;

protected:
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    TSChecker *checker_;
    ir::Expression *id_;
    bool inAssignment_;
    bool convertTupleToArray_;
    Type *inferredType_ {};
    SignatureInfo *signatureInfo_ {};
    bool validateObjectPatternInitializer_ {true};
    bool validateTypeAnnotation_ {};
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

class ArrayDestructuringContext : public DestructuringContext {
public:
    ArrayDestructuringContext(TSChecker *checker, ir::Expression *id, bool inAssignment, bool convertTupleToArray,
                              ir::TypeNode *typeAnnotation, ir::Expression *initializer)
        : DestructuringContext(checker, id, inAssignment, convertTupleToArray, typeAnnotation, initializer)
    {
    }

    Type *GetTypeFromTupleByIndex(TupleType *tuple);
    Type *CreateArrayTypeForRest(UnionType *inferredType);
    Type *CreateTupleTypeForRest(TupleType *tuple);
    void SetRemainingParameterTypes();

    void Start() override;
    void ValidateInferredType() override;
    Type *NextInferredType([[maybe_unused]] const util::StringView &searchName, bool throwError) override;
    void HandleRest(ir::SpreadElement *rest) override;
    Type *GetRestType([[maybe_unused]] const lexer::SourcePosition &loc) override;
    Type *ConvertTupleTypeToArrayTypeIfNecessary(ir::AstNode *node, Type *type) override;

private:
    uint32_t index_ {0};
};

class ObjectDestructuringContext : public DestructuringContext {
public:
    ObjectDestructuringContext(TSChecker *checker, ir::Expression *id, bool inAssignment, bool convertTupleToArray,
                               ir::TypeNode *typeAnnotation, ir::Expression *initializer)
        : DestructuringContext(checker, id, inAssignment, convertTupleToArray, typeAnnotation, initializer)
    {
    }

    Type *CreateObjectTypeForRest(ObjectType *objType);

    void Start() override;
    void ValidateInferredType() override;
    Type *NextInferredType([[maybe_unused]] const util::StringView &searchName, bool throwError) override;
    void HandleRest(ir::SpreadElement *rest) override;
    Type *GetRestType([[maybe_unused]] const lexer::SourcePosition &loc) override;
    Type *ConvertTupleTypeToArrayTypeIfNecessary(ir::AstNode *node, Type *type) override;
};
}  // namespace ark::es2panda::checker

#endif
