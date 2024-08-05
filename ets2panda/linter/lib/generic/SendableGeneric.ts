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
    const genericDecl = this.getGenericDeclByCallExpression(callExpr);
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
      if (SendableGeneric.getTypeParamByType(argType) || !param) {
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

  // checkNewExpression(tsNewExpr: ts.NewExpression):void {}

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
      this.createContactMap(SendableGeneric.getContactStart(item));
    }
  }

  private createContactMap(decl: GenericDeclaration):void {
    const concat = (typeArgumentsTypes:readonly ts.Type[], decl: GenericDeclaration):void => {
      typeArgumentsTypes.forEach((argType, index) => {
        // 实参关联到的 泛型形参
        const typeParam = decl.typeParameters?.[index];
        const refTypeParam = SendableGeneric.getTypeParamByType(argType);
        if (typeParam && refTypeParam) {
          if (!this.paramMap.has(refTypeParam)) {
            this.paramMap.set(refTypeParam, []);
          }
          this.paramMap.get(refTypeParam)?.push(typeParam);
        }
      });
    };
    const recursion = (node: ts.Node, parents: ts.Node[]):void => {

      /*
       * -------------------- T 的应用 -------------------- //
       * 处理函数调用
       */
      if (ts.isCallExpression(node)) {
        const decl = this.getGenericDeclByCallExpression(node);
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
        this.declMap.set(node, true);
      }
      ts.forEachChild(node, (child) => {
        recursion(child, parents);
      });
    };
    recursion(decl, []);
  }

  // 存在作用域包裹的情况(函数包函数,类包函数)，需要找到顶级作用域的声明开始搜索
  static getContactStart(decl: GenericDeclaration): GenericDeclaration {
    let parent: ts.Node = decl.parent;
    let target = decl;
    while (parent) {
      if (SendableGeneric.isGenericDeclaration(parent)) {
        target = parent;
      }
      parent = parent.parent;
    }
    return target;
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
  private getGenericDeclByCallExpression(callExpr: ts.CallExpression): GenericDeclaration | undefined {
    const decl = this.tsUtils.getDeclarationNode(callExpr.expression);
    if (!decl || !TsUtils.isFunctionLikeDeclaration(decl) || !decl.typeParameters?.length) {
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
  private getCallTypeArguments(tsCallExpr: CallExpression): readonly ts.Type[] | undefined {
    const callSignature = this.tsTypeChecker.getResolvedSignature(tsCallExpr);
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

  static getTypeParamByType(argType: ts.Type) : ts.TypeParameterDeclaration | undefined {
    if (argType.isTypeParameter()) {
      const decl = TsUtils.getDeclaration(argType.symbol);
      return decl && ts.isTypeParameterDeclaration(decl) ? decl : undefined;
    }
    return undefined;
  }
}
