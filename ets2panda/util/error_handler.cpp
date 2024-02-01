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

#include "util/error_handler.h"

namespace panda::es2panda::util {

void ErrorHandler::ThrowSyntaxError(std::string_view errorMessage, const lexer::SourcePosition &pos) const
{
    lexer::LineIndex index(program_->SourceCode());
    lexer::SourceLocation loc = index.GetLocation(pos);

    throw Error {ErrorType::SYNTAX, program_->SourceFilePath().Utf8(), errorMessage, loc.line, loc.col};
}

void ErrorHandler::ThrowSyntaxError(const parser::Program *program, std::string_view errorMessage,
                                    const lexer::SourcePosition &pos)
{
    ErrorHandler(program).ThrowSyntaxError(errorMessage, pos);
}

}  // namespace panda::es2panda::util
