/* eslint-disable @stylistic/max-len */
/* eslint-disable max-lines-per-function */
/* eslint-disable multiline-comment-style */
import * as ts from 'typescript';

import { TsUtils } from '../utils/TsUtils';

// 带有泛型模版的声明
export type GenericDeclaration = ts.DeclarationWithTypeParameterChildren & {
  typeParameters: ts.NodeArray<ts.TypeParameterDeclaration>
};

export default class SendableGeneric {
  constructor(
    private readonly tsTypeChecker: ts.TypeChecker,
    private readonly tsUtils: TsUtils
  ) {}

  isWrongCallOrNewExpression(
    callOrNew: ts.CallExpression | ts.NewExpression
  ):boolean {
    const genericDecl = this.getGenericDeclByCallOrNewExpression(callOrNew);
    if (!genericDecl) {
      return false;
    }
    // 开始检查关联性
    this.createContactMap(genericDecl);
    // 检查这次调用传入的实参
    const typeArgumentsTypes = this.getTypeArgsTypesByCallOrNew(callOrNew);
    if (!typeArgumentsTypes?.length) {
      return false;
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
        if (this.isWrongSendable(typeParam.parent, argType)) {
          return true;
        }
        list.push(...this.paramContact.get(typeParam) || []);
      }
    }
    return false;
  }


  private readonly declSearched: Set<GenericDeclaration> = new Set();
  private readonly paramContact: Map<ts.TypeParameterDeclaration, ts.TypeParameterDeclaration[]> = new Map();
  private readonly paramValidity: Map<ts.TypeParameterDeclaration, boolean> = new Map();
  private readonly searchQueue: GenericDeclaration[] = [];

  // 从指定泛型声明开始,创建泛型类型参数的引用关系图
  private createContactMap(decl: GenericDeclaration): void {
    this.searchQueue.push(decl);
    while (this.searchQueue.length) {
      const decl = this.searchQueue.shift()!;
      if (this.declSearched.has(decl)) {
        continue;
      }
      this.searchContact(decl);
    }
  }

  private searchContact(searchDecl: GenericDeclaration):void {
    const appendSearchQueue = (decl: GenericDeclaration):void => {
      if (this.declSearched.has(decl)) {
        return;
      }
      this.searchQueue.push(decl);
    };

    // decl被使用时传入的实参typeArgumentsTypes,如果typeArgumentsTypes中存在泛型引用,则创建关联
    const createContact = (typeArgumentsTypes:readonly ts.Type[], decl: GenericDeclaration):void => {
      let needSearch = false;
      typeArgumentsTypes.forEach((argType, index) => {
        // 实参关联到的 泛型形参
        const typeParam = decl.typeParameters?.[index];
        const refTypeParams = SendableGeneric.getTypeParamsByType(argType);
        if (typeParam && refTypeParams.length) {
          refTypeParams.forEach((param) => {
            if (!this.paramContact.has(param)) {
              this.paramContact.set(param, []);
            }
            this.paramContact.get(param)?.push(typeParam);
            needSearch = true;
          });
        }
      });
      needSearch && appendSearchQueue(decl);
    };

    const searchNode = (node: ts.Node):void => {

      /*
       * -------------------- T 的应用 -------------------- //
       * 处理函数调用/new调用, foo<T>(); new Cls<T>();
       */
      if (ts.isCallExpression(node) || ts.isNewExpression(node)) {
        const decl = this.getGenericDeclByCallOrNewExpression(node);
        if (decl) {
          const types = this.getTypeArgsTypesByCallOrNew(node);
          types?.length && createContact(types, decl);
        }
      }

      // 处理类型引用, const a:Class<T>; const a:Interface<T>; const a:Type<T>;
      if (ts.isTypeReferenceNode(node)) {
        const decl = this.getGenericDeclByTypeReference(node);
        if (decl) {
          const typeArguments = node.typeArguments;
          typeArguments?.length && createContact(typeArguments.map((arg) => {
            return this.tsTypeChecker.getTypeAtLocation(arg);
          }), decl);
        }
      }

      // 声明嵌套 function foo<T>(){class Cls<T>{}};
      if (SendableGeneric.isGenericDeclaration(node)) {
        appendSearchQueue(node);
        return;
      }
      ts.forEachChild(node, (child) => {
        searchNode(child);
      });
    };
    if (this.declSearched.has(searchDecl)) {
      return;
    }
    this.declSearched.add(searchDecl);
    if (ts.isClassDeclaration(searchDecl) && TsUtils.hasSendableDecorator(searchDecl)) {
      searchDecl.typeParameters.forEach((param) => {
        this.paramValidity.set(param, true);
      });
    }
    ts.forEachChild(searchDecl, (child) => {
      searchNode(child);
    });
  }
  // -------------------- Utils -------------------- //

  // 是否为带泛型模版的声明
  static isGenericDeclaration(node: ts.Node): node is GenericDeclaration {
    return (
      (TsUtils.isFunctionLikeDeclaration(node) ||
        ts.isClassDeclaration(node) ||
        ts.isInterfaceDeclaration(node) ||
        ts.isTypeAliasDeclaration(node)) &&
      !!node.typeParameters?.length
    );
  }

  // 通过 callExpr/newExpr 得到对应的 GenericDeclaration
  private getGenericDeclByCallOrNewExpression(callOrNew: ts.CallExpression | ts.NewExpression): GenericDeclaration | undefined {
    const decl = this.tsUtils.getDeclarationNode(callOrNew.expression);
    if (
      !decl ||
      !TsUtils.isFunctionLikeDeclaration(decl) && !ts.isClassDeclaration(decl) ||
      !decl.typeParameters?.length
    ) {
      return undefined;
    }
    return decl as GenericDeclaration;
  }

  // 通过 typeReferenceNode 得到对应的 GenericDeclaration
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
  private getTypeArgsTypesByCallOrNew(callOrNew: ts.CallExpression | ts.NewExpression): readonly ts.Type[] | undefined {
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

  // 如果type是泛型模版类型，返回相应的TypeParameterDeclaration
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

  // -------------------- check --------------------//

  static isValidTypeParam(param: ts.TypeParameterDeclaration):boolean {
    const decl = param.parent;
    return ts.isClassDeclaration(decl) && TsUtils.hasSendableDecorator(decl);
  }

  isWrongSendable:(decl:ts.Node, typeArg:ts.Type)=>boolean = (decl:ts.Node, typeArg:ts.Type) => {
    if (!ts.isClassDeclaration(decl) || !TsUtils.hasSendableDecorator(decl)) {
      // 只对@Sendable class做检查
      return false;
    }
    return !this.isWrongSendableType(typeArg);
  };

  isWrongSendableType(type: ts.Type): boolean {
    if (type.isUnion()) {
      return type.types.every((compType) => {
        return this.isWrongSendableType(compType);
      });
    }
    if (type.isTypeParameter()) {
      // 忽略泛型引用
      return false;
    }
    return !this.tsUtils.isSendableType(type);
  }
}


// DeclarationWithTypeParameterChildren::ClassLikeDeclaration::ClassExpression  const cls = class {}; 已经被linter限制了，无需考虑
// DeclarationWithTypeParameterChildren::ClassLikeDeclaration::StructDeclaration  stuct test<T>{}; 已经被限制了，无需考虑
// 需要考虑 interface 的各种情况

