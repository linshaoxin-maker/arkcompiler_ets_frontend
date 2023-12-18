/*
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

#ifndef ES2PANDA_COMPILER_CORE_ASTVERIFIER_H
#define ES2PANDA_COMPILER_CORE_ASTVERIFIER_H

#include <regex>
#include "ir/astNode.h"
#include "lexer/token/sourceLocation.h"
#include "parser/program/program.h"
#include "util/ustring.h"
#include "utils/arena_containers.h"
#include "varbinder/variable.h"
#include "utils/json_builder.h"
#include "ir/statements/blockStatement.h"
#include "compiler/lowering/phase.h"

namespace panda::es2panda::compiler {

/*
 * ASTVerifier used for checking various invariants that should hold during AST transformation in lowerings
 * For all available checks lookup the constructor
 */
class ASTVerifier final {
public:
    struct InvariantError {
        std::string cause;
        std::string message;
        size_t line;
    };
    struct CheckError {
        util::StringView invariant_name;
        InvariantError error;

        std::function<void(JsonObjectBuilder &)> DumpJSON() const
        {
            return [&](JsonObjectBuilder &body) {
                body.AddProperty("invariant", invariant_name.Utf8());
                body.AddProperty("cause", error.cause);
                body.AddProperty("message", error.message);
                body.AddProperty("line", error.line + 1);
            };
        }
    };
    using Errors = std::vector<CheckError>;

    enum class CheckResult { Failed, Success, SkipSubtree };
    struct ErrorContext;
    using InvariantCheck = std::function<CheckResult(ErrorContext &ctx, const ir::AstNode *)>;
    struct Invariant {
        util::StringView invariant_name;
        InvariantCheck invariant;
    };
    using Invariants = std::vector<Invariant>;

    NO_COPY_SEMANTIC(ASTVerifier);
    NO_MOVE_SEMANTIC(ASTVerifier);

    explicit ASTVerifier(ArenaAllocator *allocator);
    ~ASTVerifier() = default;

    using InvariantSet = std::set<std::string>;

    /**
     * @brief Run all existing invariants on some ast node (and consequently it's children)
     * @param ast AstNode which will be analyzed
     * @return Errors report of analysis
     */
    Errors VerifyFull(const ir::AstNode *ast);

    /**
     * @brief Run some particular invariants on some ast node
     * @note invariants must be supplied as strings to invariant_set, additionally invariant
     * name can be suffixed by `ForAll` string to include recursive analysis of provided node
     * I.e. 'HasParent' invariant can be named 'HasParentRecursive' to traverse all child nodes as well
     * @param ast AstNode which will be analyzed
     * @param invariant_set Set of strings which will be used as invariant names
     * @return Errors report of analysis
     */
    Errors Verify(const ir::AstNode *ast, const InvariantSet &invariant_set);

private:
    Invariants invariants_checks_;
    InvariantSet invariants_names_;
};

class ASTVerifierContext final {
public:
    ASTVerifierContext(ASTVerifier &verifier) : verifier_ {verifier} {}

    void IntroduceNewInvariants(util::StringView phase_name)
    {
        auto invariant_set = [phase_name]() -> std::optional<ASTVerifier::InvariantSet> {
            (void)phase_name;
            if (phase_name == "ScopesInitPhase") {
                return {{
                    "NodeHasParentForAll",
                    "IdentifierHasVariableForAll",
                    "ModifierAccessValidForAll",
                    "ImportExportAccessValid",
                }};
            } else if (phase_name == "PromiseVoidInferencePhase") {
                return {{}};
            } else if (phase_name == "StructLowering") {
                return {{}};
            } else if (phase_name == "CheckerPhase") {
                return {{
                    "NodeHasTypeForAll",
                    "ArithmeticOperationValidForAll",
                    "SequenceExpressionHasLastTypeForAll",
                    "EveryChildHasValidParentForAll",
                    "ForLoopCorrectlyInitializedForAll",
                    "VariableHasScopeForAll",
                    "VariableHasEnclosingScopeForAll",
                }};
            } else if (phase_name == "GenerateTsDeclarationsPhase") {
                return {{}};
            } else if (phase_name == "InterfacePropertyDeclarationsPhase") {
                return {{}};
            } else if (phase_name == "LambdaConstructionPhase") {
                return {{}};
            } else if (phase_name == "ObjectIndexLowering") {
                return {{}};
            } else if (phase_name == "OpAssignmentLowering") {
                return {{}};
            } else if (phase_name == "PromiseVoidInferencePhase") {
                return {{}};
            } else if (phase_name == "TupleLowering") {
                return {{}};
            } else if (phase_name == "UnionLowering") {
                return {{}};
            } else if (phase_name == "ExpandBracketsPhase") {
                return {{}};
            } else if (phase_name.Utf8().find("plugins") != std::string_view::npos) {
                return {{}};
            }
            return std::nullopt;
        }();

        ASSERT_PRINT(invariant_set.has_value(),
                     std::string {"Invariant set does not contain value for "} + phase_name.Mutf8());
        const auto &s = *invariant_set;
        accumulated_checks_.insert(s.begin(), s.end());
    }

    bool Verify(const ir::AstNode *ast, util::StringView phase_name, util::StringView source_name)
    {
        errors_ = verifier_.Verify(ast, accumulated_checks_);
        for (const auto &e : errors_) {
            error_array_.Add([e, source_name, phase_name](JsonObjectBuilder &err) {
                err.AddProperty("from", source_name.Utf8());
                err.AddProperty("phase", phase_name.Utf8());
                err.AddProperty("error", e.DumpJSON());
            });
        }
        auto result = errors_.empty();
        errors_.clear();
        return result;
    }

    std::string DumpErrorsJSON()
    {
        return std::move(error_array_).Build();
    }

private:
    ASTVerifier &verifier_;
    ASTVerifier::Errors errors_;
    JsonArrayBuilder error_array_;
    ASTVerifier::InvariantSet accumulated_checks_ {};
};

}  // namespace panda::es2panda::compiler

#endif  // ES2PANDA_COMPILER_CORE_ASTVERIFIER_H
