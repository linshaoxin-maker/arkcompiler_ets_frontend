/* eslint-disable @stylistic/max-len */
/* eslint-disable max-lines-per-function */
/* eslint-disable multiline-comment-style */
import * as ts from 'typescript';
import type { CallExpression } from 'typescript';
import { TsUtils } from '../utils/TsUtils';


export type GenericDeclaration = ts.DeclarationWithTypeParameterChildren;

export default class SendableGeneric {
  constructor(
    private readonly tsTypeChecker: ts.TypeChecker,
    private readonly tsUtils: TsUtils
  ) {}

  checkCallExpression(
    callExpr: ts.CallExpression,
    checkHandle:(decl:GenericDeclaration, typeArg:ts.Type)=>boolean = (decl:GenericDeclaration, typeArg:ts.Type) => {
      if (!ts.isClassDeclaration(decl) || !TsUtils.hasSendableDecorator(decl)) {
        return true;
      }
      return this.tsUtils.isSendableTypeWithUnion(typeArg);
    }
  ):boolean {
    // 判断是不是一个带泛型模版的函数
    const genericDecl = this.getGenericDeclByCallOrNewExpression(callExpr);
    if (!genericDecl) {
      return true;
    }
    // 开始检查关联性
    this.beginCreateContact(genericDecl);
    // 检查这次调用传入的实参
    const typeArgumentsTypes = this.getCallTypeArguments(callExpr);
    if (!typeArgumentsTypes?.length) {
      return true;
    }
    for (let i = 0; i < typeArgumentsTypes?.length; i++) {
      const argType = typeArgumentsTypes[i];
      const param = genericDecl.typeParameters?.[i];
      if (SendableGeneric.getTypeParamsByType(argType)?.length === (argType.isUnion() ? argType.types.length : 1) || !param) {
        // 实参全部都是泛型引用时，不做检查
        continue;
      }
      //
      const list = [param];
      while (list.length) {
        const typeParam = list.shift()!;
        if (!checkHandle(typeParam.parent as ts.DeclarationWithTypeParameterChildren, argType)) {
          return false;
        }
        list.push(...this.paramMap.get(typeParam) || []);
      }
    }
    return true;
  }

  // -------------------- contact -------------------- //
  private readonly declMap: Map<GenericDeclaration, boolean> = new Map();
  private readonly paramMap: Map<ts.TypeParameterDeclaration, ts.TypeParameterDeclaration[]> = new Map();

  private createQueue: GenericDeclaration[] = [];

  private beginCreateContact(decl: GenericDeclaration): void {
    this.createQueue = [decl];
    while (this.createQueue.length) {
      const item = this.createQueue.shift()!;
      // 检查过了
      if (this.declMap.has(item)) {
        continue;
      }
      this.declMap.set(decl, true);
      this.createContactMap(item);
    }
  }

  private createContactMap(decl: GenericDeclaration):void {
    const appendQueue = (decl: GenericDeclaration):void => {
      if (this.declMap.has(decl)) {
        return;
      }
      this.createQueue.push(decl);
    };
    const concat = (typeArgumentsTypes:readonly ts.Type[], curDecl: GenericDeclaration):void => {
      let need = false;
      typeArgumentsTypes.forEach((argType, index) => {
        // 实参关联到的 泛型形参
        const typeParam = curDecl.typeParameters?.[index];
        const refTypeParams = SendableGeneric.getTypeParamsByType(argType);
        if (typeParam && refTypeParams.length) {
          refTypeParams.forEach((refTypeParam) => {
            if (!this.paramMap.has(refTypeParam)) {
              this.paramMap.set(refTypeParam, []);
            }
            this.paramMap.get(refTypeParam)?.push(typeParam);
            need = true;
          });
        }
      });
      need && appendQueue(curDecl);
    };


    const recursion = (node: ts.Node):void => {

      /*
       * -------------------- T 的应用 -------------------- //
       * 处理函数调用/new调用
       */
      if (ts.isCallExpression(node) || ts.isNewExpression(node)) {
        const decl = this.getGenericDeclByCallOrNewExpression(node);
        if (decl) {
          const typeArguments = this.getCallTypeArguments(node);
          typeArguments?.length && concat(typeArguments, decl);
        }
      }

      // 处理类型使用
      if (ts.isTypeReferenceNode(node)) {
        const decl = this.getGenericDeclByTypeReference(node);
        if (decl) {
          const typeArguments = node.typeArguments;
          typeArguments?.length && concat(typeArguments.map((arg) => {
            return this.tsTypeChecker.getTypeAtLocation(arg);
          }), decl);
        }
      }

      // -------------------- 包裹了其他声明 -------------------- //
      if (SendableGeneric.isGenericDeclaration(node)) {
        // 子声明可能因为被调用先检查过了，此时不需要再遍历子声明
        appendQueue(node);
        return;
      }
      ts.forEachChild(node, (child) => {
        recursion(child);
      });
    };
    ts.forEachChild(decl, (child) => {
      recursion(child);
    });
  }
  // -------------------- Utils -------------------- //

  // 带泛型模版的声明
  static isGenericDeclaration(node: ts.Node): node is GenericDeclaration {
    return (
      (TsUtils.isFunctionLikeDeclaration(node) ||
        ts.isClassDeclaration(node) ||
        ts.isInterfaceDeclaration(node) ||
        ts.isTypeAliasDeclaration(node)) &&
      !!node.typeParameters?.length
    );
  }

  // 通过 callExpr 得到一个带泛型模版的声明
  private getGenericDeclByCallOrNewExpression(callOrNew: ts.CallExpression | ts.NewExpression): GenericDeclaration | undefined {
    const decl = this.tsUtils.getDeclarationNode(callOrNew.expression);
    if (
      !decl ||
      !TsUtils.isFunctionLikeDeclaration(decl) && !ts.isClassDeclaration(decl) ||
      !decl.typeParameters?.length
    ) {
      return undefined;
    }
    return decl;
  }

  // 通过 typeReferenceNode 得到一个带泛型模版的声明
  private getGenericDeclByTypeReference(node: ts.TypeReferenceNode): GenericDeclaration | undefined {
    if (!node.typeArguments?.length) {
      return undefined;
    }
    const decl = this.tsUtils.getDeclarationNode(node.typeName);
    if (!decl || !SendableGeneric.isGenericDeclaration(decl)) {
      return undefined;
    }
    return decl;
  }

  // 获取函数调用的泛型实参
  private getCallTypeArguments(callOrNew: ts.CallExpression | ts.NewExpression): readonly ts.Type[] | undefined {
    const callSignature = this.tsTypeChecker.getResolvedSignature(callOrNew);
    if (!callSignature?.mapper) {
      return undefined;
    }
    const mapper = callSignature.mapper;
    if (mapper.kind === ts.TypeMapKind.Simple) {
      return [mapper.target];

    } else if (mapper.kind === ts.TypeMapKind.Array) {
      return mapper.targets;
    }
    return undefined;
  }


  getTypeParamByRef(node: ts.Node) : ts.TypeParameterDeclaration | undefined {
    if (!ts.isTypeReferenceNode(node)) {
      return undefined;
    }
    const decl = this.tsUtils.getDeclarationNode(node.typeName);

    return decl && ts.isTypeParameterDeclaration(decl) ? decl : undefined;
  }

  static getTypeParamsByType(argType: ts.Type, result:ts.TypeParameterDeclaration[] = []) :ts.TypeParameterDeclaration[] {
    if (argType.isUnion()) {
      argType.types.forEach((compType) => {
        SendableGeneric.getTypeParamsByType(compType, result);
      });
    }
    if (argType.isTypeParameter()) {
      const decl = TsUtils.getDeclaration(argType.symbol);
      if (decl && ts.isTypeParameterDeclaration(decl)) {
        result.push(decl);
      }
    }
    return result;
  }
}
