import ts from "typescript";
import { ClassType } from "./base/typeSystem";

export class TypeChecker {
    private static instance: TypeChecker;
    private compiledTypeChecker: any = null;
    private constructor() { }

    public static getInstance(): TypeChecker {
        if (!TypeChecker.instance) {
            TypeChecker.instance = new TypeChecker();
        }
        return TypeChecker.instance;
    }

    public setTypeChecker(typeChecker: ts.TypeChecker) {
        this.compiledTypeChecker = typeChecker;
    }

    public getTypeChecker(): ts.TypeChecker {
        return this.compiledTypeChecker;
    }

    public getTypeDeclAtLocation(node: ts.Node): ts.Node {
        let identifierSymbol = this.compiledTypeChecker.getSymbolAtLocation(node);
        if (identifierSymbol && identifierSymbol.declarations) {
            return identifierSymbol!.declarations[0];
        }
        return node;
    }

    public getTypeFlagsAtLocation(node: ts.Node): string {
        let typeFlag = this.compiledTypeChecker.getTypeAtLocation(node).getFlags();
        return ts.TypeFlags[typeFlag].toUpperCase();
    }

    public formatNodeType(node: ts.Node) {
        if (this.compiledTypeChecker === null) {
            return;
        }
        if (node.kind === ts.SyntaxKind.VariableStatement) {
            const variableStatementNode = <ts.VariableStatement>node;
            const decList = variableStatementNode.declarationList;
            decList.declarations.forEach(declaration => {
                const nameNode = declaration.name;
                let newExpressionFlag = false;
                if (declaration.initializer && declaration.initializer.kind == ts.SyntaxKind.NewExpression) {
                    newExpressionFlag = true;
                }
                // let symbol: ts.Symbol = this.compiledTypeChecker.getSymbolAtLocation(nameNode);
                // let targetNode = symbol?.valueDeclaration;
                let type: ts.Type = this.compiledTypeChecker.getTypeAtLocation(nameNode);
                let targetNode = type.getSymbol()?.valueDeclaration;
                if (targetNode) {
                    if (ts.isClassDeclaration(targetNode!)) {
                        let testClassType = new ClassType(<ts.ClassDeclaration>targetNode, newExpressionFlag, nameNode);
                    }
                }
                // console.log(type.getSymbol()?.valueDeclaration);
            })
        }
    }
}
