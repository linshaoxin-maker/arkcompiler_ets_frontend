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

#ifndef ES2PANDA_COMPILER_LOWERING_PHASE_H
#define ES2PANDA_COMPILER_LOWERING_PHASE_H

#include "parser/program/program.h"
#include "public/public.h"

namespace panda::es2panda::compiler {

class Phase {
public:
    /* If Apply returns false, processing is stopped. */
    bool Apply(public_lib::Context *ctx, parser::Program *program);

    virtual std::string_view Name() = 0;

    virtual bool Precondition([[maybe_unused]] public_lib::Context *ctx,
                              [[maybe_unused]] const parser::Program *program)
    {
        return true;
    }
    virtual bool Perform(public_lib::Context *ctx, parser::Program *program) = 0;
    virtual bool Postcondition([[maybe_unused]] public_lib::Context *ctx,
                               [[maybe_unused]] const parser::Program *program)
    {
        return true;
    }

private:
    void CheckOptionsBeforePhase(const CompilerOptions *options, const parser::Program *program,
                                 const std::string &name) const;
    void CheckOptionsAfterPhase(const CompilerOptions *options, const parser::Program *program,
                                const std::string &name) const;
};

std::vector<Phase *> GetPhaseList(ScriptExtension ext);

}  // namespace panda::es2panda::compiler

#endif
