#include "ast_verifier_test.h"
#include "macros.h"
#include "util/ast-builders/classDefinitionBuilder.h"
#include "util/ast-builders/binaryExpressionBuilder.h"
#include "util/ast-builders/classPropertyBuilder.h"
#include "util/ast-builders/identifierBuilder.h"
#include "util/ast-builders/numberLiteralBuilder.h"

#include <gtest/gtest.h>

using ark::es2panda::compiler::ast_verifier::InvariantNameSet;
using ark::es2panda::ir::BinaryExpressionBuilder;
using ark::es2panda::ir::ClassDefinitionBuilder;
using ark::es2panda::ir::ClassPropertyBuilder;
using ark::es2panda::ir::IdentifierBuilder;
using ark::es2panda::ir::NumberLiteralBuilder;

TEST_F(ASTVerifierTest, AstBuilder)
{
    auto id = IdentifierBuilder(Allocator()).SetName("a").Build();
    auto number = NumberLiteralBuilder(Allocator()).SetValue("10").Build();
    auto classProperty = ClassPropertyBuilder(Allocator())
                             .SetKey(id)
                             .SetValue(number)
                             .AddModifier(ark::es2panda::ir::ModifierFlags::PRIVATE)
                             .Build();

    auto classDef = ClassDefinitionBuilder(Allocator())
                        .SetIdentifier("A")
                        .AddProperty(classProperty)
                        .Build();

    std::cout << "Class Definition:" << classDef->DumpJSON() << std::endl;

    // Fail on purpose to display output
    ASSERT_FALSE(true);
}
