/* eslint-disable @stylistic/max-len */
/* eslint-disable max-lines-per-function */
/* eslint-disable multiline-comment-style */
import * as ts from 'typescript';

import { TsUtils } from '../utils/TsUtils';

// 带有泛型模版的声明
export type GenericDeclaration = ts.DeclarationWithTypeParameterChildren & {
  typeParameters: ts.NodeArray<ts.TypeParameterDeclaration>;
};

/**
 * 思路：
 * 1. 在函数调用( foo<T>(); )，new表达式( new SomeClass<T>() )，类型声明( const a:SomeClass<T>;const b:SomeType<T> )... 等处触发检查
 * 2. 找到对应的声明(函数声明,类声明,别名声明...)，并且找到该声明的顶级父泛型声明(存在声明嵌套的情况 function foo<T>() { class SomeClass<T>{} })，为避免重复查找，所以将顶级父声明作为查找目标
 * 3. 遍历当前声明，找到当前声明中所有使用到当前声明的泛型形参（T）的节点，绑定T和该节点对应声明的泛型形参（T2），并将该节点的声明加入搜索列表重复2-3
 * 4. 通过2-3的不断遍历，获取到相关泛型形参的所有能关联到的其他泛型形参。
 * 5. 检测所有查找到的泛型形参，是否可以关联到SendableClass的泛型形参，并缓存
 * 6. 获取函数调用的实参，使用步骤5的缓存，如果实参对应的形参可以关联到SendableClass，则判断该实参是否为SendableType
 * 7. 返回结果
 *
 * 副作用：
 * 内存消耗，每个带泛型的声明，其中的每个泛型形参，都会生成一个 {key: TypeParameterDeclaration, value: boolean} 的缓存数据
 * 性能消耗，每个带泛型的声明，都会完整的遍历一次
 */
export default class SendableGenericChecker {
  constructor(
    private readonly tsTypeChecker: ts.TypeChecker,
    private readonly tsUtils: TsUtils
  ) {}

  isWrongCallOrNewExpression(callOrNew: ts.CallExpression | ts.NewExpression): boolean {
    const decl = this.getGenericDeclByCallOrNewExpression(callOrNew);
    if (!decl) {
      return false;
    }
    const typeArgumentsTypes = this.getTypeArgsTypesByCallOrNew(callOrNew);
    if (!typeArgumentsTypes?.length) {
      return false;
    }
    return this.isWrongGeneric(decl, typeArgumentsTypes);
  }

  isWrongTypeReference(typeRef: ts.TypeReferenceNode): boolean {
    const decl = this.getGenericDeclByTypeReference(typeRef);
    if (!decl) {
      return false;
    }
    const typeArgumentsTypes = this.getTypeArgsTypesByTypeReference(decl, typeRef);
    if (!typeArgumentsTypes?.length) {
      return false;
    }
    return this.isWrongGeneric(decl, typeArgumentsTypes);
  }

  private isWrongGeneric(
    genericDecl: GenericDeclaration,
    typeArgumentsTypes: readonly ts.Type[]
  ): boolean {
    // 为泛型引用创建映射
    this.createContact(genericDecl);
    // 检查这次调用传入的实参是否会导致错误的Sendable泛型
    for (let i = 0; i < Math.min(typeArgumentsTypes.length, genericDecl.typeParameters.length); i++) {
      if (
        !!this.paramContactSendable.get(genericDecl.typeParameters[i]) &&
         this.isWrongSendableType(typeArgumentsTypes[i])
      ) {
        return true;
      }
    }
    return false;
  }

  //
  private readonly searchedDecl: Set<GenericDeclaration> = new Set();
  private readonly paramContactSendable: Map<ts.TypeParameterDeclaration, boolean> = new Map();
  private paramContactTemp: Map<ts.TypeParameterDeclaration, Set<ts.TypeParameterDeclaration>> = new Map();
  private searchQueueTemp: GenericDeclaration[] = [];

  // 从指定泛型声明开始,创建泛型类型参数的引用关系图
  private createContact(decl: GenericDeclaration): void {
    this.paramContactTemp = new Map();
    this.searchQueueTemp = [SendableGenericChecker.getTopGenericDeclaration(decl)];
    while (this.searchQueueTemp.length) {
      this.searchContact(this.searchQueueTemp.shift()!);
    }

    const tempList:ts.TypeParameterDeclaration[] = [];
    for (const param of this.paramContactTemp.keys()) {
      this.checkParamCantactToSendable(param, tempList);
    }

    this.paramContactTemp.clear();
    this.searchQueueTemp.length = 0;
  }

  // 检查泛型形参是否关联到SendableClass
  private checkParamCantactToSendable(param: ts.TypeParameterDeclaration, parents: ts.TypeParameterDeclaration[] = []):boolean {
    if (this.paramContactSendable.has(param)) {
      return !!this.paramContactSendable.get(param);
    }
    let result = false;
    if (parents.includes(param)) {
      // 出现了循环引用
      result = false;
    } else {
      const sets = this.paramContactTemp.get(param);
      if (!sets) {
        result = false;
      } else {
        parents.push(param);
        for (const child of sets) {
          if (this.checkParamCantactToSendable(child, parents)) {
            result = true;
          }
        }
        parents.pop();
      }
    }
    this.paramContactSendable.set(param, result);
    return result;
  }

  // 搜索泛型间的关联
  private searchContact(topDecl: GenericDeclaration): void {
    if (this.searchedDecl.has(topDecl)) {
      return;
    }
    this.searchedDecl.add(topDecl);

    const appendSearchQueue = (decl: GenericDeclaration): void => {
      const topDecl = SendableGenericChecker.getTopGenericDeclaration(decl);
      if (this.searchedDecl.has(topDecl)) {
        return;
      }
      this.searchQueueTemp.push(topDecl);
    };

    // decl被使用时传入的实参typeArgumentsTypes,如果typeArgumentsTypes中存在泛型引用,则创建关联
    const createContact = (typeArgumentsTypes: readonly ts.Type[], decl: GenericDeclaration): void => {
      let needSearch = false;
      typeArgumentsTypes.forEach((argType, index) => {
        // 实参关联到的 泛型形参
        const typeParam = decl.typeParameters?.[index];
        const refTypeParams = SendableGenericChecker.getTypeParamsByType(argType);
        if (typeParam && refTypeParams.length) {
          refTypeParams.forEach((param) => {
            if (!this.paramContactTemp.has(param)) {
              this.paramContactTemp.set(param, new Set());
            }
            this.paramContactTemp.get(param)?.add(typeParam);
            needSearch = true;
          });
        }
      });
      needSearch && appendSearchQueue(decl);
    };

    const searchNode = (node: ts.Node, isInSendableClass:boolean = false): void => {


      if (isInSendableClass) {
        if (ts.isPropertyDeclaration(node) && node.type) {
          // 对SendableClass的属性中引用到的泛型形参，进行标记
          const type = this.tsTypeChecker.getTypeAtLocation(node.type);
          const params = SendableGenericChecker.getTypeParamsByType(type);
          params.forEach((param) => {
            this.paramContactSendable.set(param, true);
          });
        }
      }

      /*
       * 处理函数调用/new调用, foo<T>(); new Cls<T>();
       */
      if (ts.isCallExpression(node) || ts.isNewExpression(node)) {
        const decl = this.getGenericDeclByCallOrNewExpression(node);
        if (decl) {
          const types = this.getTypeArgsTypesByCallOrNew(node);
          types?.length && createContact(types, decl);
        }
        return;
      }

      // 处理类型引用, const a:Class<T>; const a:Interface<T>; const a:Type<T>;
      if (ts.isTypeReferenceNode(node)) {
        const decl = this.getGenericDeclByTypeReference(node);
        if (decl) {
          const types = this.getTypeArgsTypesByTypeReference(decl, node);
          types?.length && createContact(types, decl);
        }
        return;
      }

      // Sendable类声明中的Non-Sendable类声明,不算inSendableClass
      const childInSendableClass = ts.isClassDeclaration(node) ? TsUtils.hasSendableDecorator(node) : isInSendableClass;

      ts.forEachChild(node, (child) => {
        searchNode(child, childInSendableClass);
      });
    };
    searchNode(topDecl);
  }
  // -------------------- Utils -------------------- //

  // 获取一个泛型声明的
  static getTopGenericDeclaration(decl: GenericDeclaration): GenericDeclaration {
    let parent: ts.Node = decl.parent;
    let target = decl;
    while (parent) {
      if (SendableGenericChecker.isGenericDeclaration(parent)) {
        target = parent;
      }
      parent = parent.parent;
    }
    return target;
  }

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
  private getGenericDeclByCallOrNewExpression(
    callOrNew: ts.CallExpression | ts.NewExpression
  ): GenericDeclaration | undefined {
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
    const decl = this.tsUtils.getDeclarationNode(node.typeName);
    if (!decl || !SendableGenericChecker.isGenericDeclaration(decl)) {
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

  private getTypeArgsTypesByTypeReference(decl: GenericDeclaration, node: ts.TypeReferenceNode): readonly ts.Type[] | undefined {
    const result: ts.Type[] = [];
    decl.typeParameters.forEach((param, index) => {
      const arg = node.typeArguments?.[index];
      if (arg) {
        result.push(this.tsTypeChecker.getTypeAtLocation(arg));
      }
      if (param.default) {
        result.push(this.tsTypeChecker.getTypeAtLocation(param.default));
      }
    });
    return result;
  }

  // 返回类型中包含的TypeParameterDeclaration-
  static getTypeParamsByType(
    type: ts.Type,
    result: ts.TypeParameterDeclaration[] = []
  ): ts.TypeParameterDeclaration[] {
    if (type.isUnion()) {
      type.types.forEach((compType) => {
        SendableGenericChecker.getTypeParamsByType(compType, result);
      });
    }
    if (type.isTypeParameter()) {
      const decl = TsUtils.getDeclaration(type.symbol);
      if (decl && ts.isTypeParameterDeclaration(decl)) {
        result.push(decl);
      }
    }
    return result;
  }

  // -------------------- check --------------------//

  isWrongSendableType(type: ts.Type): boolean {
    if (type.isUnion()) {
      return type.types.some((compType) => {
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
// 考虑 interface 是否要检测
