#include "multiArrayLowering.h"
#include "generated/signatures.h"
#include "compiler/lowering/scopesInit/scopesInitPhase.h"

namespace ark::es2panda::compiler {

ir::Expression *MultiArrayLowering::ProcessMultiArray(parser::ETSParser *parser, checker::ETSChecker *checker,
                                varbinder::ETSBinder *varbinder,
                                ir::ETSNewMultiDimArrayInstanceExpression *multiArray) const
{
    std::string class_name = std::string{multiArray->TypeReference()->AsETSTypeReference()->Part()->Name()->AsIdentifier()->Name()};
    static std::string CALL_EXPRESSION = 
        std::string {compiler::Signatures::BUILTIN_MULTIARRAY_INIT} + std::string{"<"} + class_name;
    for (size_t i=0; i<multiArray->Dimensions().size(); ++i) {
        CALL_EXPRESSION += "[]";
    }
    CALL_EXPRESSION += std::string{">("} + std::string {multiArray->DumpEtsSrc()}+",";
    CALL_EXPRESSION += "():Object => {return new " + class_name + "()})";
    auto *const initExpr = parser->CreateExpression(CALL_EXPRESSION);
    initExpr->SetParent(multiArray->Parent());
    InitScopesPhaseETS::RunExternalNode(initExpr, varbinder);
    initExpr->Check(checker);
    return initExpr;
}

bool MultiArrayLowering::Perform(public_lib::Context *ctx, parser::Program *program)
{
    auto *const parser = ctx->parser->AsETSParser();
    ASSERT(parser != nullptr);
    auto *const checker = ctx->checker->AsETSChecker();
    ASSERT(checker != nullptr);
    auto *const varbinder = ctx->compilerContext->VarBinder()->AsETSBinder();
    ASSERT(varbinder != nullptr);

//    static std::string const CALL_EXPRESSION = 
//        compiler::Signatures::BUILTIN_MULTIARRAY_INIT;
    program->Ast()->TransformChildrenRecursively([this,parser,checker,varbinder](ir::AstNode *ast) -> ir::AstNode * {
        if (ast->IsETSNewMultiDimArrayInstanceExpression()) {
            return ProcessMultiArray(parser, checker,varbinder, ast->AsETSNewMultiDimArrayInstanceExpression());
        }
        return ast;
    }
    );
    return true;
}
}
