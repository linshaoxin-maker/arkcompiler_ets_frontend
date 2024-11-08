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

#ifndef ES2PANDA_UTIL_INCLUDE_CLASS_BUILDER
#define ES2PANDA_UTIL_INCLUDE_CLASS_BUILDER

#include "mem/arena_allocator.h"
#include "identifierBuilder.h"
#include "classDeclarationBuilder.h"
#include "classDefinitionBuilder.h"
#include "etsTypeReferenceBuilder.h"
#include "etsTypeReferencePartBuilder.h"

namespace ark::es2panda::ir {

class ClassBuilder {
public:
    explicit ClassBuilder(ark::ArenaAllocator *allocator) : allocator_(allocator), classDef_(allocator) {};

    ClassBuilder &SetId(ir::Identifier *id)
    {
        classDef_.SetIdentifier(id);
        return *this;
    }

    ClassBuilder &SetId(util::StringView id)
    {
        classDef_.SetIdentifier(IdentifierBuilder(allocator_).SetName(id).Build());
        return *this;
    }

    ClassBuilder &SetSuperClass(util::StringView name)
    {
        auto superClass = ETSTypeReferenceBuilder(allocator_)
                              .SetETSTypeReferencePart(ETSTypeReferencePartBuilder(allocator_)
                                                           .SetName(IdentifierBuilder(allocator_).SetName(name).Build())
                                                           .Build())
                              .Build();
        classDef_.SetSuperClass(superClass);
        return *this;
    }

    ClassBuilder &AddProperty(ir::AstNode *prop)
    {
        classDef_.AddProperty(prop);
        return *this;
    }

    ir::ClassDeclaration *Build()
    {
        auto classDecl = ClassDeclarationBuilder(allocator_).SetDefinition(classDef_.Build()).Build();
        for (auto it : classDecl->Definition()->AsClassDefinition()->Body()) {
            it->SetParent(classDecl->Definition()->AsClassDefinition());
        }
        classDecl->Definition()->AsClassDefinition()->SetParent(classDecl);
        return classDecl;
    }

private:
    ark::ArenaAllocator *allocator_;
    ClassDefinitionBuilder classDef_;
};

}  // namespace ark::es2panda::ir
#endif  // ES2PANDA_UTIL_INCLUDE_CLASS_BUILDER