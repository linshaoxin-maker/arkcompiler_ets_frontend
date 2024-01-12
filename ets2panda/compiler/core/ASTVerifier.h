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

#include <algorithm>
#include <iterator>
#include <regex>
#include <string>
#include <unordered_set>

#include "ir/astNode.h"
#include "ir/statements/blockStatement.h"
#include "lexer/token/sourceLocation.h"
#include "parser/program/program.h"
#include "util/ustring.h"
#include "utils/arena_containers.h"
#include "utils/json_builder.h"
#include "varbinder/variable.h"

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
        explicit CheckError(std::string name, InvariantError error)
            : invariant_name_ {std::move(name)}, error_ {std::move(error)}
        {
        }
        std::function<void(JsonObjectBuilder &)> DumpJSON() const
        {
            return [&](JsonObjectBuilder &body) {
                body.AddProperty("invariant", invariant_name_);
                body.AddProperty("cause", error_.cause);
                body.AddProperty("message", error_.message);
                body.AddProperty("line", error_.line + 1);
            };
        }
        const std::string &GetName() const
        {
            return invariant_name_;
        }

    private:
        std::string invariant_name_;
        InvariantError error_;
    };
    using Errors = std::vector<CheckError>;

    enum class CheckResult { FAILED, SUCCESS, SKIP_SUBTREE };
    class ErrorContext {
    public:
        explicit ErrorContext() = default;

        void AddError(const std::string &message)
        {
            errors_.emplace_back(CheckError {"Unnamed", ASTVerifier::InvariantError {message, "", 0}});
        }

        virtual void AddInvariantError(const std::string &name, const std::string &cause, const ir::AstNode &node)
        {
            errors_.emplace_back(
                CheckError {name, ASTVerifier::InvariantError {cause, node.DumpJSON(), node.Start().line}});
        }

        ASTVerifier::Errors GetErrors()
        {
            return errors_;
        }

    private:
        Errors errors_;
    };

    class AssertsContext : public ErrorContext {
    public:
        void AddInvariantError(const std::string &name, const std::string &cause, const ir::AstNode &node) override
        {
            ASTVerifier::ErrorContext::AddInvariantError(name, cause, node);
            // NOTE(tatiana): add ASSERT here
        }
    };

    class NoneContext : public ErrorContext {
    public:
        void AddInvariantError([[maybe_unused]] const std::string &name, [[maybe_unused]] const std::string &cause,
                               [[maybe_unused]] const ir::AstNode &node) override
        {
        }
    };
    using InvariantCheck = std::function<CheckResult(ErrorContext &ctx, const ir::AstNode *)>;
    struct Invariant {
        util::StringView invariant_name;
        InvariantCheck invariant;
    };
    using Invariants = std::map<std::string, InvariantCheck>;

    NO_COPY_SEMANTIC(ASTVerifier);
    NO_MOVE_SEMANTIC(ASTVerifier);

    explicit ASTVerifier(ArenaAllocator *allocator);
    ~ASTVerifier() = default;

    using InvariantSet = std::unordered_set<std::string>;

    /**
     * @brief Run all existing invariants on some ast node (and consequently it's children)
     * @param ast AstNode which will be analyzed
     * @return Errors report of analysis
     */
    std::tuple<ASTVerifier::Errors, ASTVerifier::Errors> VerifyFull(const std::unordered_set<std::string> &warnings,
                                                                    const std::unordered_set<std::string> &asserts,
                                                                    const ir::AstNode *ast);

    /**
     * @brief Run some particular invariants on some ast node
     * @note invariants must be supplied as strings to invariant_set, additionally invariant
     * name can be suffixed by `ForAll` string to include recursive analysis of provided node
     * I.e. 'HasParent' invariant can be named 'HasParentRecursive' to traverse all child nodes as well
     * @param ast AstNode which will be analyzed
     * @param invariant_set Set of strings which will be used as invariant names
     * @return Errors report of analysis
     */
    std::tuple<ASTVerifier::Errors, ASTVerifier::Errors> Verify(const std::unordered_set<std::string> &warnings,
                                                                const std::unordered_set<std::string> &asserts,
                                                                const ir::AstNode *ast,
                                                                const InvariantSet &invariant_set);

private:
    void AddInvariant(const std::string &name, const InvariantCheck &invariant);

    Invariants invariants_checks_;
    InvariantSet invariants_names_;
};

class ASTVerifierContext final {
public:
    explicit ASTVerifierContext(ASTVerifier &verifier) : verifier_ {verifier} {}

    void IntroduceNewInvariants(util::StringView phase_name)
    {
        auto invariant_set = [phase_name]() -> std::optional<ASTVerifier::InvariantSet> {
            (void)phase_name;
            if (phase_name == "ScopesInitPhase") {
                return {{
                    "NodeHasParentForAll",
                    "EveryChildHasValidParentForAll",
                    "VariableHasScopeForAll",
                }};
            }
            if (phase_name == "CheckerPhase") {
                return {{
                    "NodeHasTypeForAll",
                    "IdentifierHasVariableForAll",
                    "ArithmeticOperationValidForAll",
                    "SequenceExpressionHasLastTypeForAll",
                    "ForLoopCorrectlyInitializedForAll",
                    "VariableHasEnclosingScopeForAll",
                    "ModifierAccessValidForAll",
                    "ImportExportAccessValid",
                }};
            }
            const std::set<std::string> without_additional_checks = {"PromiseVoidInferencePhase",
                                                                     "StructLowering",
                                                                     "GenerateTsDeclarationsPhase",
                                                                     "InterfacePropertyDeclarationsPhase",
                                                                     "LambdaConstructionPhase",
                                                                     "ObjectIndexLowering",
                                                                     "OpAssignmentLowering",
                                                                     "PromiseVoidInferencePhase",
                                                                     "TupleLowering",
                                                                     "UnionLowering",
                                                                     "ExpandBracketsPhase"};
            if (without_additional_checks.count(phase_name.Mutf8()) > 0) {
                return {{}};
            }
            if (phase_name.Utf8().find("plugins") != std::string_view::npos) {
                return {{}};
            }
            return std::nullopt;
        }();

        ASSERT_PRINT(invariant_set.has_value(),
                     std::string {"Invariant set does not contain value for "} + phase_name.Mutf8());
        const auto &s = *invariant_set;
        accumulated_checks_.insert(s.begin(), s.end());
    }

    bool Verify(const std::unordered_set<std::string> &warnings, const std::unordered_set<std::string> &errors,
                const ir::AstNode *ast, util::StringView phase_name, util::StringView source_name)
    {
        auto [warns, asserts] = verifier_.Verify(warnings, errors, ast, accumulated_checks_);
        std::for_each(warns.begin(), warns.end(), [this, &source_name, &phase_name](ASTVerifier::CheckError &e) {
            warnings_.Add([e, source_name, phase_name](JsonObjectBuilder &err) {
                err.AddProperty("from", source_name.Utf8());
                err.AddProperty("phase", phase_name.Utf8());
                err.AddProperty("error", e.DumpJSON());
            });
        });
        std::for_each(asserts.begin(), asserts.end(), [this, &source_name, &phase_name](ASTVerifier::CheckError &e) {
            asserts_.Add([e, source_name, phase_name](JsonObjectBuilder &err) {
                err.AddProperty("from", source_name.Utf8());
                err.AddProperty("phase", phase_name.Utf8());
                err.AddProperty("error", e.DumpJSON());
            });
        });
        return warns.empty() && asserts.empty();
    }

    std::string DumpWarningsJSON()
    {
        return std::move(warnings_).Build();
    }
    std::string DumpAssertsJSON()
    {
        return std::move(asserts_).Build();
    }

private:
    ASTVerifier &verifier_;
    JsonArrayBuilder warnings_;
    JsonArrayBuilder asserts_;
    ASTVerifier::InvariantSet accumulated_checks_ {};
};

}  // namespace panda::es2panda::compiler

#endif  // ES2PANDA_COMPILER_CORE_ASTVERIFIER_H
