import ts from "typescript";
import { ClassType } from "./base/typeSystem";

export class TypeChecker {
    private static instance: TypeChecker;
    private compiledTypeChecker: any = null;
    private constructor() {}

    public static getInstance(): TypeChecker {
        if (!TypeChecker.instance) {
            TypeChecker.instance = new TypeChecker();
        }
        return TypeChecker.instance;
    }

    public setTypeChecker(typeChecker: ts.TypeChecker) {
        this.compiledTypeChecker = typeChecker;
    }

    public getTypeChecker() : ts.TypeChecker {
        return this.compiledTypeChecker;
    }

    public formatNodeType(node: ts.Node) {
        if (this.compiledTypeChecker === null) {
            return ;
        }
        if (node.kind === ts.SyntaxKind.VariableStatement) {
            const variableStatementNode = <ts.VariableStatement>node;
            const decList = variableStatementNode.declarationList;
            decList.declarations.forEach(declaration => {
                const nameNode = declaration.name;
                let type: ts.Type = this.compiledTypeChecker.getTypeAtLocation(nameNode);
                let targetNode = type.getSymbol()?.valueDeclaration;
                let testClassType = new ClassType(<ts.ClassDeclaration>targetNode);
                // console.log(type.getSymbol()?.valueDeclaration); 
            })
        }
    }
}
