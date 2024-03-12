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

#ifndef ES2PANDA_COMPILER_DEBUGGER_EVALUATE_CLASS_DECLARATION_CREATOR_H
#define ES2PANDA_COMPILER_DEBUGGER_EVALUATE_CLASS_DECLARATION_CREATOR_H

#include "checker/types/ets/etsObjectType.h"

#include "ir/expressions/identifier.h"

#include "libpandafile/class_data_accessor-inl.h"
#include "libpandabase/mem/arena_allocator.h"

namespace ark::es2panda {

namespace checker {
class ETSChecker;
}

class DebugInfoLookup;

// Help to create class declaration using information obtained from given .abc file
class ClassDeclarationCreator {
public:
    DEFAULT_COPY_SEMANTIC(ClassDeclarationCreator);
    DEFAULT_MOVE_SEMANTIC(ClassDeclarationCreator);

    explicit ClassDeclarationCreator(checker::ETSChecker *checker, ArenaAllocator *allocator)
        : checker_(checker), allocator_(allocator)
    {
    }
    ~ClassDeclarationCreator() = default;

    void CreateClassDeclaration(const util::StringView &identName, panda_file::ClassDataAccessor *cda);

    checker::Type *ToCheckerType(panda_file::Type pandaFileType);

    void SetDebugInfoLookup(DebugInfoLookup *other)
    {
        debugInfoLookup_ = other;
    }

private:
    using MethodBuilder = std::function<void(varbinder::FunctionScope *scope, 
                                             ArenaVector<ir::Statement *> *stms,
                                             ArenaVector<ir::Expression *> *fparams, 
                                             checker::Type **rettype)>;
         
    std::vector<checker::Type *> GetFunctionParameters(panda_file::MethodDataAccessor &mda);

    checker::Type *ResolveReferenceType(const std::string &refName);

    void CreateClassBody(varbinder::ClassScope *scope, ArenaVector<ir::AstNode *> *classBody,
                         panda_file::ClassDataAccessor *cda);

    void CreateFieldsProperties(varbinder::ClassScope *scope, 
                                ArenaVector<ir::AstNode *> *classBody,
                                checker::ETSObjectType *classType, 
                                panda_file::ClassDataAccessor *cda);

    void CreateFunctionProperties(varbinder::ClassScope *scope,
                                  ArenaVector<ir::AstNode *> *classBody,
                                  checker::ETSObjectType *classType,
                                  panda_file::ClassDataAccessor *cda);
private:
    checker::ETSChecker *checker_ {nullptr};
    ArenaAllocator *allocator_ {nullptr};
    DebugInfoLookup *debugInfoLookup_ {nullptr};
};

}  // namespace ark::es2panda

#endif  // ES2PANDA_COMPILER_DEBUGGER_EVALUATE_CLASS_DECLARATION_CREATOR_H
