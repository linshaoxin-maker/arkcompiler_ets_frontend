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

#ifndef ES2PANDA_IR_STATEMENT_DECLARE_ETS_TOPLEVEL_H
#define ES2PANDA_IR_STATEMENT_DECLARE_ETS_TOPLEVEL_H

#include "blockStatement.h"

namespace ark::es2panda::ir {

class ETSTopLevel : public BlockStatement {
public:
    explicit ETSTopLevel(ArenaAllocator *allocator, ArenaVector<Statement *> &&statementList)
        : BlockStatement(allocator, std::move(statementList))
    {
        type_ = AstNodeType::ETS_TOPLEVEL;
    }
};
}  // namespace ark::es2panda::ir

#endif
