"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.TsUtils = exports.SYMBOL_CONSTRUCTOR = exports.SYMBOL = void 0;
var path = _interopRequireWildcard(require("node:path"));
var ts = _interopRequireWildcard(require("typescript"));
var _Problems = require("../Problems");
var _ArktsIgnorePaths = require("./consts/ArktsIgnorePaths");
var _ESObject = require("./consts/ESObject");
var _SendableAPI = require("./consts/SendableAPI");
var _SharedModuleAPI = require("./consts/SharedModuleAPI");
var _StandardLibraries = require("./consts/StandardLibraries");
var _SupportedDetsIndexableTypes = require("./consts/SupportedDetsIndexableTypes");
var _TypedArrays = require("./consts/TypedArrays");
var _ForEachNodeInSubtree = require("./functions/ForEachNodeInSubtree");
var _GetScriptKind = require("./functions/GetScriptKind");
var _IsStdLibrary = require("./functions/IsStdLibrary");
var _IsStruct = require("./functions/IsStruct");
var _PathHelper = require("./functions/PathHelper");
var _isAssignmentOperator = require("./functions/isAssignmentOperator");
var _isIntrinsicObjectType = require("./functions/isIntrinsicObjectType");
var _TsUtils;
function _getRequireWildcardCache(e) { if ("function" != typeof WeakMap) return null; var r = new WeakMap(), t = new WeakMap(); return (_getRequireWildcardCache = function (e) { return e ? t : r; })(e); }
function _interopRequireWildcard(e, r) { if (!r && e && e.__esModule) return e; if (null === e || "object" != typeof e && "function" != typeof e) return { default: e }; var t = _getRequireWildcardCache(r); if (t && t.has(e)) return t.get(e); var n = { __proto__: null }, a = Object.defineProperty && Object.getOwnPropertyDescriptor; for (var u in e) if ("default" !== u && {}.hasOwnProperty.call(e, u)) { var i = a ? Object.getOwnPropertyDescriptor(e, u) : null; i && (i.get || i.set) ? Object.defineProperty(n, u, i) : n[u] = e[u]; } return n.default = e, t && t.set(e, n), n; }
function _newArrowCheck(n, r) { if (n !== r) throw new TypeError("Cannot instantiate an arrow function"); }
function _defineProperty(e, r, t) { return (r = _toPropertyKey(r)) in e ? Object.defineProperty(e, r, { value: t, enumerable: !0, configurable: !0, writable: !0 }) : e[r] = t, e; }
function _toPropertyKey(t) { var i = _toPrimitive(t, "string"); return "symbol" == typeof i ? i : i + ""; }
function _toPrimitive(t, r) { if ("object" != typeof t || !t) return t; var e = t[Symbol.toPrimitive]; if (void 0 !== e) { var i = e.call(t, r || "default"); if ("object" != typeof i) return i; throw new TypeError("@@toPrimitive must return a primitive value."); } return ("string" === r ? String : Number)(t); } /*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
const SYMBOL = exports.SYMBOL = 'Symbol';
const SYMBOL_CONSTRUCTOR = exports.SYMBOL_CONSTRUCTOR = 'SymbolConstructor';
const ITERATOR = 'iterator';
class TsUtils {
  constructor(tsTypeChecker, testMode, advancedClassChecks, useRtLogic) {
    this.tsTypeChecker = tsTypeChecker;
    this.testMode = testMode;
    this.advancedClassChecks = advancedClassChecks;
    this.useRtLogic = useRtLogic;
    _defineProperty(this, "trueSymbolAtLocationCache", new Map());
  }
  entityNameToString(name) {
    if (ts.isIdentifier(name)) {
      return name.escapedText.toString();
    }
    return this.entityNameToString(name.left) + this.entityNameToString(name.right);
  }
  isNumberLikeType(tsType) {
    if (this.useRtLogic && tsType.isUnion()) {
      for (const tsCompType of tsType.types) {
        if ((tsCompType.flags & ts.TypeFlags.NumberLike) === 0) {
          return false;
        }
      }
      return true;
    }
    return (tsType.getFlags() & ts.TypeFlags.NumberLike) !== 0;
  }
  static isBooleanLikeType(tsType) {
    return (tsType.getFlags() & ts.TypeFlags.BooleanLike) !== 0;
  }
  static isDestructuringAssignmentLHS(tsExpr) {
    /*
     * Check whether given expression is the LHS part of the destructuring
     * assignment (or is a nested element of destructuring pattern).
     */
    let tsParent = tsExpr.parent;
    let tsCurrentExpr = tsExpr;
    while (tsParent) {
      if (ts.isBinaryExpression(tsParent) && (0, _isAssignmentOperator.isAssignmentOperator)(tsParent.operatorToken) && tsParent.left === tsCurrentExpr) {
        return true;
      }
      if ((ts.isForStatement(tsParent) || ts.isForInStatement(tsParent) || ts.isForOfStatement(tsParent)) && tsParent.initializer && tsParent.initializer === tsCurrentExpr) {
        return true;
      }
      tsCurrentExpr = tsParent;
      tsParent = tsParent.parent;
    }
    return false;
  }
  static isEnumType(tsType) {
    // when type equals `typeof <Enum>`, only symbol contains information about it's type.
    const isEnumSymbol = tsType.symbol && this.isEnum(tsType.symbol);
    // otherwise, we should analyze flags of the type itself
    const isEnumType = !!(tsType.flags & ts.TypeFlags.Enum) || !!(tsType.flags & ts.TypeFlags.EnumLiteral);
    return isEnumSymbol || isEnumType;
  }
  static isEnum(tsSymbol) {
    return !!(tsSymbol.flags & ts.SymbolFlags.Enum);
  }
  static hasModifier(tsModifiers, tsModifierKind) {
    if (!tsModifiers) {
      return false;
    }
    for (const tsModifier of tsModifiers) {
      if (tsModifier.kind === tsModifierKind) {
        return true;
      }
    }
    return false;
  }
  static unwrapParenthesized(tsExpr) {
    let unwrappedExpr = tsExpr;
    while (ts.isParenthesizedExpression(unwrappedExpr)) {
      unwrappedExpr = unwrappedExpr.expression;
    }
    return unwrappedExpr;
  }
  followIfAliased(sym) {
    if ((sym.getFlags() & ts.SymbolFlags.Alias) !== 0) {
      return this.tsTypeChecker.getAliasedSymbol(sym);
    }
    return sym;
  }
  trueSymbolAtLocation(node) {
    const cache = this.trueSymbolAtLocationCache;
    const val = cache.get(node);
    if (val !== undefined) {
      return val !== null ? val : undefined;
    }
    let sym = this.tsTypeChecker.getSymbolAtLocation(node);
    if (sym === undefined) {
      cache.set(node, null);
      return undefined;
    }
    sym = this.followIfAliased(sym);
    cache.set(node, sym);
    return sym;
  }
  static isTypeDeclSyntaxKind(kind) {
    return (0, _IsStruct.isStructDeclarationKind)(kind) || kind === ts.SyntaxKind.EnumDeclaration || kind === ts.SyntaxKind.ClassDeclaration || kind === ts.SyntaxKind.InterfaceDeclaration || kind === ts.SyntaxKind.TypeAliasDeclaration;
  }
  static symbolHasDuplicateName(symbol, tsDeclKind) {
    /*
     * Type Checker merges all declarations with the same name in one scope into one symbol.
     * Thus, check whether the symbol of certain declaration has any declaration with
     * different syntax kind.
     */
    const symbolDecls = symbol?.getDeclarations();
    if (symbolDecls) {
      for (const symDecl of symbolDecls) {
        const declKind = symDecl.kind;
        // we relax arkts-unique-names for namespace collision with class/interface/enum/type/struct
        const isNamespaceTypeCollision = TsUtils.isTypeDeclSyntaxKind(declKind) && tsDeclKind === ts.SyntaxKind.ModuleDeclaration || TsUtils.isTypeDeclSyntaxKind(tsDeclKind) && declKind === ts.SyntaxKind.ModuleDeclaration;

        /*
         * Don't count declarations with 'Identifier' syntax kind as those
         * usually depict declaring an object's property through assignment.
         */
        if (declKind !== ts.SyntaxKind.Identifier && declKind !== tsDeclKind && !isNamespaceTypeCollision) {
          return true;
        }
      }
    }
    return false;
  }
  static isPrimitiveType(type) {
    const f = type.getFlags();
    return (f & ts.TypeFlags.Boolean) !== 0 || (f & ts.TypeFlags.BooleanLiteral) !== 0 || (f & ts.TypeFlags.Number) !== 0 || (f & ts.TypeFlags.NumberLiteral) !== 0

    /*
     *  In ArkTS 'string' is not a primitive type. So for the common subset 'string'
     *  should be considered as a reference type. That is why next line is commented out.
     * (f & ts.TypeFlags.String) != 0 || (f & ts.TypeFlags.StringLiteral) != 0
     */;
  }
  static isTypeSymbol(symbol) {
    return !!symbol && !!symbol.flags && ((symbol.flags & ts.SymbolFlags.Class) !== 0 || (symbol.flags & ts.SymbolFlags.Interface) !== 0);
  }

  // Check whether type is generic 'Array<T>' type defined in TypeScript standard library.
  static isGenericArrayType(tsType) {
    return TsUtils.isTypeReference(tsType) && tsType.typeArguments?.length === 1 && tsType.target.typeParameters?.length === 1 && tsType.getSymbol()?.getName() === 'Array';
  }
  static isReadonlyArrayType(tsType) {
    return TsUtils.isTypeReference(tsType) && tsType.typeArguments?.length === 1 && tsType.target.typeParameters?.length === 1 && tsType.getSymbol()?.getName() === 'ReadonlyArray';
  }
  isTypedArray(tsType) {
    const symbol = tsType.symbol;
    if (!symbol) {
      return false;
    }
    const name = this.tsTypeChecker.getFullyQualifiedName(symbol);
    return this.isGlobalSymbol(symbol) && _TypedArrays.TYPED_ARRAYS.includes(name);
  }
  isArray(tsType) {
    return TsUtils.isGenericArrayType(tsType) || TsUtils.isReadonlyArrayType(tsType) || this.isTypedArray(tsType);
  }
  static isTuple(tsType) {
    return TsUtils.isTypeReference(tsType) && !!(tsType.objectFlags & ts.ObjectFlags.Tuple);
  }

  // does something similar to relatedByInheritanceOrIdentical function
  isOrDerivedFrom(tsType, checkType, checkedBaseTypes) {
    tsType = TsUtils.reduceReference(tsType);
    if (checkType.call(this, tsType)) {
      return true;
    }
    if (!tsType.symbol?.declarations) {
      return false;
    }

    // Avoid type recursion in heritage by caching checked types.
    (checkedBaseTypes = checkedBaseTypes || new Set()).add(tsType);
    for (const tsTypeDecl of tsType.symbol.declarations) {
      const isClassOrInterfaceDecl = ts.isClassDeclaration(tsTypeDecl) || ts.isInterfaceDeclaration(tsTypeDecl);
      const isDerived = isClassOrInterfaceDecl && !!tsTypeDecl.heritageClauses;
      if (!isDerived) {
        continue;
      }
      for (const heritageClause of tsTypeDecl.heritageClauses) {
        if (this.processParentTypesCheck(heritageClause.types, checkType, checkedBaseTypes)) {
          return true;
        }
      }
    }
    return false;
  }
  static isTypeReference(tsType) {
    return (tsType.getFlags() & ts.TypeFlags.Object) !== 0 && (tsType.objectFlags & ts.ObjectFlags.Reference) !== 0;
  }
  static isPrototypeSymbol(symbol) {
    return !!symbol && !!symbol.flags && (symbol.flags & ts.SymbolFlags.Prototype) !== 0;
  }
  static isFunctionSymbol(symbol) {
    return !!symbol && !!symbol.flags && (symbol.flags & ts.SymbolFlags.Function) !== 0;
  }
  static isInterfaceType(tsType) {
    return !!tsType && !!tsType.symbol && !!tsType.symbol.flags && (tsType.symbol.flags & ts.SymbolFlags.Interface) !== 0;
  }
  static isAnyType(tsType) {
    return (tsType.getFlags() & ts.TypeFlags.Any) !== 0;
  }
  static isUnknownType(tsType) {
    return (tsType.getFlags() & ts.TypeFlags.Unknown) !== 0;
  }
  static isUnsupportedType(tsType) {
    return !!tsType.flags && ((tsType.flags & ts.TypeFlags.Any) !== 0 || (tsType.flags & ts.TypeFlags.Unknown) !== 0 || (tsType.flags & ts.TypeFlags.Intersection) !== 0);
  }
  static isNullableUnionType(type) {
    if (type.isUnion()) {
      for (const t of type.types) {
        if (!!(t.flags & ts.TypeFlags.Undefined) || !!(t.flags & ts.TypeFlags.Null)) {
          return true;
        }
      }
    }
    return false;
  }
  static isMethodAssignment(tsSymbol) {
    return !!tsSymbol && (tsSymbol.flags & ts.SymbolFlags.Method) !== 0 && (tsSymbol.flags & ts.SymbolFlags.Assignment) !== 0;
  }
  static getDeclaration(tsSymbol) {
    if (tsSymbol?.declarations && tsSymbol.declarations.length > 0) {
      return tsSymbol.declarations[0];
    }
    return undefined;
  }
  static isVarDeclaration(tsDecl) {
    return ts.isVariableDeclaration(tsDecl) && ts.isVariableDeclarationList(tsDecl.parent);
  }
  isValidEnumMemberInit(tsExpr) {
    if (this.isNumberConstantValue(tsExpr.parent)) {
      return true;
    }
    if (this.isStringConstantValue(tsExpr.parent)) {
      return true;
    }
    return this.isCompileTimeExpression(tsExpr);
  }
  isCompileTimeExpressionHandlePropertyAccess(tsExpr) {
    if (!ts.isPropertyAccessExpression(tsExpr)) {
      return false;
    }

    /*
     * if enum member is in current enum declaration try to get value
     * if it comes from another enum consider as constant
     */
    const propertyAccess = tsExpr;
    if (this.isNumberConstantValue(propertyAccess)) {
      return true;
    }
    const leftHandSymbol = this.trueSymbolAtLocation(propertyAccess.expression);
    if (!leftHandSymbol) {
      return false;
    }
    const decls = leftHandSymbol.getDeclarations();
    if (!decls || decls.length !== 1) {
      return false;
    }
    return ts.isEnumDeclaration(decls[0]);
  }
  isCompileTimeExpression(tsExpr) {
    if (ts.isParenthesizedExpression(tsExpr) || ts.isAsExpression(tsExpr) && tsExpr.type.kind === ts.SyntaxKind.NumberKeyword) {
      return this.isCompileTimeExpression(tsExpr.expression);
    }
    switch (tsExpr.kind) {
      case ts.SyntaxKind.PrefixUnaryExpression:
        return this.isPrefixUnaryExprValidEnumMemberInit(tsExpr);
      case ts.SyntaxKind.ParenthesizedExpression:
      case ts.SyntaxKind.BinaryExpression:
        return this.isBinaryExprValidEnumMemberInit(tsExpr);
      case ts.SyntaxKind.ConditionalExpression:
        return this.isConditionalExprValidEnumMemberInit(tsExpr);
      case ts.SyntaxKind.Identifier:
        return this.isIdentifierValidEnumMemberInit(tsExpr);
      case ts.SyntaxKind.NumericLiteral:
        return true;
      case ts.SyntaxKind.StringLiteral:
        return true;
      case ts.SyntaxKind.PropertyAccessExpression:
        return this.isCompileTimeExpressionHandlePropertyAccess(tsExpr);
      default:
        return false;
    }
  }
  isPrefixUnaryExprValidEnumMemberInit(tsExpr) {
    return TsUtils.isUnaryOpAllowedForEnumMemberInit(tsExpr.operator) && this.isCompileTimeExpression(tsExpr.operand);
  }
  isBinaryExprValidEnumMemberInit(tsExpr) {
    return TsUtils.isBinaryOpAllowedForEnumMemberInit(tsExpr.operatorToken) && this.isCompileTimeExpression(tsExpr.left) && this.isCompileTimeExpression(tsExpr.right);
  }
  isConditionalExprValidEnumMemberInit(tsExpr) {
    return this.isCompileTimeExpression(tsExpr.whenTrue) && this.isCompileTimeExpression(tsExpr.whenFalse);
  }
  isIdentifierValidEnumMemberInit(tsExpr) {
    const tsSymbol = this.trueSymbolAtLocation(tsExpr);
    const tsDecl = TsUtils.getDeclaration(tsSymbol);
    return !!tsDecl && (TsUtils.isVarDeclaration(tsDecl) && TsUtils.isConst(tsDecl.parent) || tsDecl.kind === ts.SyntaxKind.EnumMember);
  }
  static isUnaryOpAllowedForEnumMemberInit(tsPrefixUnaryOp) {
    return tsPrefixUnaryOp === ts.SyntaxKind.PlusToken || tsPrefixUnaryOp === ts.SyntaxKind.MinusToken || tsPrefixUnaryOp === ts.SyntaxKind.TildeToken;
  }
  static isBinaryOpAllowedForEnumMemberInit(tsBinaryOp) {
    return tsBinaryOp.kind === ts.SyntaxKind.AsteriskToken || tsBinaryOp.kind === ts.SyntaxKind.SlashToken || tsBinaryOp.kind === ts.SyntaxKind.PercentToken || tsBinaryOp.kind === ts.SyntaxKind.MinusToken || tsBinaryOp.kind === ts.SyntaxKind.PlusToken || tsBinaryOp.kind === ts.SyntaxKind.LessThanLessThanToken || tsBinaryOp.kind === ts.SyntaxKind.GreaterThanGreaterThanToken || tsBinaryOp.kind === ts.SyntaxKind.BarBarToken || tsBinaryOp.kind === ts.SyntaxKind.GreaterThanGreaterThanGreaterThanToken || tsBinaryOp.kind === ts.SyntaxKind.AmpersandToken || tsBinaryOp.kind === ts.SyntaxKind.CaretToken || tsBinaryOp.kind === ts.SyntaxKind.BarToken || tsBinaryOp.kind === ts.SyntaxKind.AmpersandAmpersandToken;
  }
  static isConst(tsNode) {
    return !!(ts.getCombinedNodeFlags(tsNode) & ts.NodeFlags.Const);
  }
  isNumberConstantValue(tsExpr) {
    const tsConstValue = tsExpr.kind === ts.SyntaxKind.NumericLiteral ? Number(tsExpr.getText()) : this.tsTypeChecker.getConstantValue(tsExpr);
    return tsConstValue !== undefined && typeof tsConstValue === 'number';
  }
  isIntegerConstantValue(tsExpr) {
    const tsConstValue = tsExpr.kind === ts.SyntaxKind.NumericLiteral ? Number(tsExpr.getText()) : this.tsTypeChecker.getConstantValue(tsExpr);
    return tsConstValue !== undefined && typeof tsConstValue === 'number' && tsConstValue.toFixed(0) === tsConstValue.toString();
  }
  isStringConstantValue(tsExpr) {
    const tsConstValue = this.tsTypeChecker.getConstantValue(tsExpr);
    return tsConstValue !== undefined && typeof tsConstValue === 'string';
  }

  // Returns true if typeA is a subtype of typeB
  relatedByInheritanceOrIdentical(typeA, typeB) {
    typeA = TsUtils.reduceReference(typeA);
    typeB = TsUtils.reduceReference(typeB);
    if (typeA === typeB || this.isObject(typeB)) {
      return true;
    }
    if (!typeA.symbol?.declarations) {
      return false;
    }
    const isBISendable = TsUtils.isISendableInterface(typeB);
    for (const typeADecl of typeA.symbol.declarations) {
      if (isBISendable && ts.isClassDeclaration(typeADecl) && TsUtils.hasSendableDecorator(typeADecl)) {
        return true;
      }
      if (!ts.isClassDeclaration(typeADecl) && !ts.isInterfaceDeclaration(typeADecl) || !typeADecl.heritageClauses) {
        continue;
      }
      for (const heritageClause of typeADecl.heritageClauses) {
        const processInterfaces = typeA.isClass() ? heritageClause.token !== ts.SyntaxKind.ExtendsKeyword : true;
        if (this.processParentTypes(heritageClause.types, typeB, processInterfaces)) {
          return true;
        }
      }
    }
    return false;
  }
  static reduceReference(t) {
    return TsUtils.isTypeReference(t) && t.target !== t ? t.target : t;
  }
  needToDeduceStructuralIdentityHandleUnions(lhsType, rhsType, rhsExpr) {
    if (rhsType.isUnion()) {
      // Each Class/Interface of the RHS union type must be compatible with LHS type.
      for (const compType of rhsType.types) {
        if (this.needToDeduceStructuralIdentity(lhsType, compType, rhsExpr)) {
          return true;
        }
      }
      return false;
    }
    if (lhsType.isUnion()) {
      // RHS type needs to be compatible with at least one type of the LHS union.
      for (const compType of lhsType.types) {
        if (!this.needToDeduceStructuralIdentity(compType, rhsType, rhsExpr)) {
          return false;
        }
      }
      return true;
    }
    // should be unreachable
    return false;
  }

  // return true if two class types are not related by inheritance and structural identity check is needed
  needToDeduceStructuralIdentity(lhsType, rhsType, rhsExpr) {
    lhsType = this.getNonNullableType(lhsType);
    rhsType = this.getNonNullableType(rhsType);
    if (this.isLibraryType(lhsType)) {
      return false;
    }
    if (this.isDynamicObjectAssignedToStdType(lhsType, rhsExpr)) {
      return false;
    }
    // #14569: Check for Function type.
    if (this.areCompatibleFunctionals(lhsType, rhsType)) {
      return false;
    }
    if (rhsType.isUnion() || lhsType.isUnion()) {
      return this.needToDeduceStructuralIdentityHandleUnions(lhsType, rhsType, rhsExpr);
    }
    if (this.advancedClassChecks && TsUtils.isClassValueType(rhsType) && lhsType !== rhsType && !TsUtils.isObjectType(lhsType)) {
      // missing exact rule
      return true;
    }
    return lhsType.isClassOrInterface() && rhsType.isClassOrInterface() && !this.relatedByInheritanceOrIdentical(rhsType, lhsType);
  }
  processParentTypes(parentTypes, typeB, processInterfaces) {
    for (const baseTypeExpr of parentTypes) {
      const baseType = TsUtils.reduceReference(this.tsTypeChecker.getTypeAtLocation(baseTypeExpr));
      if (baseType && baseType.isClass() !== processInterfaces && this.relatedByInheritanceOrIdentical(baseType, typeB)) {
        return true;
      }
    }
    return false;
  }
  processParentTypesCheck(parentTypes, checkType, checkedBaseTypes) {
    for (const baseTypeExpr of parentTypes) {
      const baseType = TsUtils.reduceReference(this.tsTypeChecker.getTypeAtLocation(baseTypeExpr));
      if (baseType && !checkedBaseTypes.has(baseType) && this.isOrDerivedFrom(baseType, checkType, checkedBaseTypes)) {
        return true;
      }
    }
    return false;
  }
  isObject(tsType) {
    if (!tsType) {
      return false;
    }
    if (tsType.symbol && tsType.isClassOrInterface() && tsType.symbol.name === 'Object') {
      return true;
    }
    const node = this.tsTypeChecker.typeToTypeNode(tsType, undefined, undefined);
    return node !== undefined && node.kind === ts.SyntaxKind.ObjectKeyword;
  }
  isCallToFunctionWithOmittedReturnType(tsExpr) {
    if (ts.isCallExpression(tsExpr)) {
      const tsCallSignature = this.tsTypeChecker.getResolvedSignature(tsExpr);
      if (tsCallSignature) {
        const tsSignDecl = tsCallSignature.getDeclaration();
        // `tsSignDecl` is undefined when `getResolvedSignature` returns `unknownSignature`
        if (!tsSignDecl?.type) {
          return true;
        }
      }
    }
    return false;
  }
  static hasReadonlyFields(type) {
    var _this = this;
    // No members -> no readonly fields
    if (type.symbol.members === undefined) {
      return false;
    }
    let result = false;
    type.symbol.members.forEach(function (value) {
      _newArrowCheck(this, _this);
      if (value.declarations !== undefined && value.declarations.length > 0 && ts.isPropertyDeclaration(value.declarations[0])) {
        const propmMods = ts.getModifiers(value.declarations[0]);
        if (TsUtils.hasModifier(propmMods, ts.SyntaxKind.ReadonlyKeyword)) {
          result = true;
        }
      }
    }.bind(this));
    return result;
  }
  static hasDefaultCtor(type) {
    var _this2 = this;
    // No members -> no explicit constructors -> there is default ctor
    if (type.symbol.members === undefined) {
      return true;
    }

    // has any constructor
    let hasCtor = false;
    // has default constructor
    let hasDefaultCtor = false;
    type.symbol.members.forEach(function (value) {
      _newArrowCheck(this, _this2);
      if ((value.flags & ts.SymbolFlags.Constructor) !== 0) {
        hasCtor = true;
        if (value.declarations !== undefined && value.declarations.length > 0) {
          const declCtor = value.declarations[0];
          if (declCtor.parameters.length === 0) {
            hasDefaultCtor = true;
          }
        }
      }
    }.bind(this));

    // Has no any explicit constructor -> has implicit default constructor.
    return !hasCtor || hasDefaultCtor;
  }
  static isAbstractClass(type) {
    if (type.isClass() && type.symbol.declarations && type.symbol.declarations.length > 0) {
      const declClass = type.symbol.declarations[0];
      const classMods = ts.getModifiers(declClass);
      if (TsUtils.hasModifier(classMods, ts.SyntaxKind.AbstractKeyword)) {
        return true;
      }
    }
    return false;
  }
  static validateObjectLiteralType(type) {
    if (!type) {
      return false;
    }
    type = TsUtils.reduceReference(type);
    return type.isClassOrInterface() && TsUtils.hasDefaultCtor(type) && !TsUtils.hasReadonlyFields(type) && !TsUtils.isAbstractClass(type);
  }
  hasMethods(type) {
    const properties = this.tsTypeChecker.getPropertiesOfType(type);
    if (properties?.length) {
      for (const prop of properties) {
        if (prop.getFlags() & ts.SymbolFlags.Method) {
          return true;
        }
      }
    }
    return false;
  }
  findProperty(type, name) {
    const properties = this.tsTypeChecker.getPropertiesOfType(type);
    if (properties?.length) {
      for (const prop of properties) {
        if (prop.name === name) {
          return prop;
        }
      }
    }
    return undefined;
  }
  checkTypeSet(typeSet, predicate) {
    if (!typeSet.isUnionOrIntersection()) {
      return predicate.call(this, typeSet);
    }
    for (const elemType of typeSet.types) {
      if (this.checkTypeSet(elemType, predicate)) {
        return true;
      }
    }
    return false;
  }
  getNonNullableType(t) {
    const isNullableUnionType = this.useRtLogic ? TsUtils.isNullableUnionType(t) : t.isUnion();
    if (isNullableUnionType) {
      return t.getNonNullableType();
    }
    return t;
  }
  isObjectLiteralAssignableToUnion(lhsType, rhsExpr) {
    for (const compType of lhsType.types) {
      if (this.isObjectLiteralAssignable(compType, rhsExpr)) {
        return true;
      }
    }
    return false;
  }
  isObjectLiteralAssignable(lhsType, rhsExpr) {
    if (lhsType === undefined) {
      return false;
    }
    // Always check with the non-nullable variant of lhs type.
    lhsType = this.getNonNullableType(lhsType);
    if (lhsType.isUnion() && this.isObjectLiteralAssignableToUnion(lhsType, rhsExpr)) {
      return true;
    }

    /*
     * Allow initializing with anything when the type
     * originates from the library.
     */
    if (TsUtils.isAnyType(lhsType) || this.isLibraryType(lhsType)) {
      return true;
    }

    /*
     * issue 13412:
     * Allow initializing with a dynamic object when the LHS type
     * is primitive or defined in standard library.
     */
    if (this.isDynamicObjectAssignedToStdType(lhsType, rhsExpr)) {
      return true;
    }
    // For Partial<T>, Required<T>, Readonly<T> types, validate their argument type.
    if (this.isStdPartialType(lhsType) || this.isStdRequiredType(lhsType) || this.isStdReadonlyType(lhsType)) {
      if (lhsType.aliasTypeArguments && lhsType.aliasTypeArguments.length === 1) {
        lhsType = lhsType.aliasTypeArguments[0];
      } else {
        return false;
      }
    }

    /*
     * Allow initializing Record objects with object initializer.
     * Record supports any type for a its value, but the key value
     * must be either a string or number literal.
     */
    if (this.isStdRecordType(lhsType)) {
      return this.validateRecordObjectKeys(rhsExpr);
    }
    return TsUtils.validateObjectLiteralType(lhsType) && !this.hasMethods(lhsType) && this.validateFields(lhsType, rhsExpr);
  }
  isDynamicObjectAssignedToStdType(lhsType, rhsExpr) {
    if ((0, _IsStdLibrary.isStdLibraryType)(lhsType) || TsUtils.isPrimitiveType(lhsType)) {
      const rhsSym = ts.isCallExpression(rhsExpr) ? this.getSymbolOfCallExpression(rhsExpr) : this.useRtLogic ? this.trueSymbolAtLocation(rhsExpr) : this.tsTypeChecker.getSymbolAtLocation(rhsExpr);
      if (rhsSym && this.isLibrarySymbol(rhsSym)) {
        return true;
      }
    }
    return false;
  }
  validateFields(objectType, objectLiteral) {
    for (const prop of objectLiteral.properties) {
      if (ts.isPropertyAssignment(prop)) {
        if (!this.validateField(objectType, prop)) {
          return false;
        }
      }
    }
    return true;
  }
  getPropertySymbol(type, prop) {
    const propNameSymbol = this.tsTypeChecker.getSymbolAtLocation(prop.name);
    const propName = propNameSymbol ? ts.symbolName(propNameSymbol) : ts.isMemberName(prop.name) ? ts.idText(prop.name) : prop.name.getText();
    const propSym = this.findProperty(type, propName);
    return propSym;
  }
  validateField(type, prop) {
    // Issue 15497: Use unescaped property name to find correpsponding property.
    const propSym = this.getPropertySymbol(type, prop);
    if (!propSym?.declarations?.length) {
      return false;
    }
    const propType = this.tsTypeChecker.getTypeOfSymbolAtLocation(propSym, propSym.declarations[0]);
    const initExpr = TsUtils.unwrapParenthesized(prop.initializer);
    if (ts.isObjectLiteralExpression(initExpr)) {
      if (!this.isObjectLiteralAssignable(propType, initExpr)) {
        return false;
      }
    } else if (this.needToDeduceStructuralIdentity(propType, this.tsTypeChecker.getTypeAtLocation(initExpr), initExpr)) {
      // Only check for structural sub-typing.
      return false;
    }
    return true;
  }
  validateRecordObjectKeys(objectLiteral) {
    for (const prop of objectLiteral.properties) {
      if (!prop.name) {
        return false;
      }
      const isValidComputedProperty = ts.isComputedPropertyName(prop.name) && this.isValidComputedPropertyName(prop.name, true);
      if (!ts.isStringLiteral(prop.name) && !ts.isNumericLiteral(prop.name) && !isValidComputedProperty) {
        return false;
      }
    }
    return true;
  }
  static isSupportedTypeNodeKind(kind) {
    return kind !== ts.SyntaxKind.AnyKeyword && kind !== ts.SyntaxKind.UnknownKeyword && kind !== ts.SyntaxKind.SymbolKeyword && kind !== ts.SyntaxKind.IndexedAccessType && kind !== ts.SyntaxKind.ConditionalType && kind !== ts.SyntaxKind.MappedType && kind !== ts.SyntaxKind.InferType;
  }
  isSupportedTypeHandleUnionTypeNode(typeNode) {
    for (const unionTypeElem of typeNode.types) {
      if (!this.isSupportedType(unionTypeElem)) {
        return false;
      }
    }
    return true;
  }
  isSupportedTypeHandleTupleTypeNode(typeNode) {
    for (const elem of typeNode.elements) {
      if (ts.isTypeNode(elem) && !this.isSupportedType(elem)) {
        return false;
      }
      if (ts.isNamedTupleMember(elem) && !this.isSupportedType(elem.type)) {
        return false;
      }
    }
    return true;
  }
  isSupportedType(typeNode) {
    if (ts.isParenthesizedTypeNode(typeNode)) {
      return this.isSupportedType(typeNode.type);
    }
    if (ts.isArrayTypeNode(typeNode)) {
      return this.isSupportedType(typeNode.elementType);
    }
    if (ts.isTypeReferenceNode(typeNode) && typeNode.typeArguments) {
      for (const typeArg of typeNode.typeArguments) {
        if (!this.isSupportedType(typeArg)) {
          return false;
        }
      }
      return true;
    }
    if (ts.isUnionTypeNode(typeNode)) {
      return this.isSupportedTypeHandleUnionTypeNode(typeNode);
    }
    if (ts.isTupleTypeNode(typeNode)) {
      return this.isSupportedTypeHandleTupleTypeNode(typeNode);
    }
    return !ts.isTypeLiteralNode(typeNode) && (this.advancedClassChecks || !ts.isTypeQueryNode(typeNode)) && !ts.isIntersectionTypeNode(typeNode) && TsUtils.isSupportedTypeNodeKind(typeNode.kind);
  }
  isStructObjectInitializer(objectLiteral) {
    if (ts.isCallLikeExpression(objectLiteral.parent)) {
      const signature = this.tsTypeChecker.getResolvedSignature(objectLiteral.parent);
      const signDecl = signature?.declaration;
      return !!signDecl && ts.isConstructorDeclaration(signDecl) && (0, _IsStruct.isStructDeclaration)(signDecl.parent);
    }
    return false;
  }
  getParentSymbolName(symbol) {
    const name = this.tsTypeChecker.getFullyQualifiedName(symbol);
    const dotPosition = name.lastIndexOf('.');
    return dotPosition === -1 ? undefined : name.substring(0, dotPosition);
  }
  isGlobalSymbol(symbol) {
    const parentName = this.getParentSymbolName(symbol);
    return !parentName || parentName === 'global';
  }
  isSymbolAPI(symbol) {
    const parentName = this.getParentSymbolName(symbol);
    if (!this.useRtLogic) {
      const name = parentName ? parentName : symbol.escapedName;
      return name === SYMBOL || name === SYMBOL_CONSTRUCTOR;
    }
    return !!parentName && (parentName === SYMBOL || parentName === SYMBOL_CONSTRUCTOR);
  }
  isSymbolIterator(symbol) {
    if (!this.useRtLogic) {
      const name = symbol.name;
      const parName = this.getParentSymbolName(symbol);
      return (parName === SYMBOL || parName === SYMBOL_CONSTRUCTOR) && name === ITERATOR;
    }
    return this.isSymbolAPI(symbol) && symbol.name === ITERATOR;
  }
  static isDefaultImport(importSpec) {
    return importSpec?.propertyName?.text === 'default';
  }
  static getStartPos(nodeOrComment) {
    return nodeOrComment.kind === ts.SyntaxKind.SingleLineCommentTrivia || nodeOrComment.kind === ts.SyntaxKind.MultiLineCommentTrivia ? nodeOrComment.pos : nodeOrComment.getStart();
  }
  static getEndPos(nodeOrComment) {
    return nodeOrComment.kind === ts.SyntaxKind.SingleLineCommentTrivia || nodeOrComment.kind === ts.SyntaxKind.MultiLineCommentTrivia ? nodeOrComment.end : nodeOrComment.getEnd();
  }
  static getHighlightRange(nodeOrComment, faultId) {
    return this.highlightRangeHandlers.get(faultId)?.call(this, nodeOrComment) ?? [this.getStartPos(nodeOrComment), this.getEndPos(nodeOrComment)];
  }
  static getKeywordHighlightRange(nodeOrComment, keyword) {
    const start = this.getStartPos(nodeOrComment);
    return [start, start + keyword.length];
  }
  static getVarDeclarationHighlightRange(nodeOrComment) {
    return this.getKeywordHighlightRange(nodeOrComment, 'var');
  }
  static getCatchWithUnsupportedTypeHighlightRange(nodeOrComment) {
    const catchClauseNode = nodeOrComment.variableDeclaration;
    if (catchClauseNode !== undefined) {
      return [catchClauseNode.getStart(), catchClauseNode.getEnd()];
    }
    return undefined;
  }
  static getForInStatementHighlightRange(nodeOrComment) {
    return [this.getEndPos(nodeOrComment.initializer) + 1, this.getStartPos(nodeOrComment.expression) - 1];
  }
  static getWithStatementHighlightRange(nodeOrComment) {
    return [this.getStartPos(nodeOrComment), nodeOrComment.statement.getStart() - 1];
  }
  static getDeleteOperatorHighlightRange(nodeOrComment) {
    return this.getKeywordHighlightRange(nodeOrComment, 'delete');
  }
  static getTypeQueryHighlightRange(nodeOrComment) {
    return this.getKeywordHighlightRange(nodeOrComment, 'typeof');
  }
  static getInstanceofUnsupportedHighlightRange(nodeOrComment) {
    return this.getKeywordHighlightRange(nodeOrComment.operatorToken, 'instanceof');
  }
  static getConstAssertionHighlightRange(nodeOrComment) {
    if (nodeOrComment.kind === ts.SyntaxKind.AsExpression) {
      return [nodeOrComment.expression.getEnd() + 1, nodeOrComment.type.getStart() - 1];
    }
    return [nodeOrComment.expression.getEnd() + 1, nodeOrComment.type.getEnd() + 1];
  }
  static getLimitedReturnTypeInferenceHighlightRange(nodeOrComment) {
    let node;
    if (nodeOrComment.kind === ts.SyntaxKind.FunctionExpression) {
      // we got error about return type so it should be present
      node = nodeOrComment.type;
    } else if (nodeOrComment.kind === ts.SyntaxKind.FunctionDeclaration) {
      node = nodeOrComment.name;
    } else if (nodeOrComment.kind === ts.SyntaxKind.MethodDeclaration) {
      node = nodeOrComment.name;
    }
    if (node !== undefined) {
      return [node.getStart(), node.getEnd()];
    }
    return undefined;
  }
  static getLocalFunctionHighlightRange(nodeOrComment) {
    return this.getKeywordHighlightRange(nodeOrComment, 'function');
  }
  static getFunctionApplyCallHighlightRange(nodeOrComment) {
    const pointPos = nodeOrComment.getText().lastIndexOf('.');
    return [this.getStartPos(nodeOrComment) + pointPos + 1, this.getEndPos(nodeOrComment)];
  }
  static getDeclWithDuplicateNameHighlightRange(nodeOrComment) {
    // in case of private identifier no range update is needed
    const nameNode = nodeOrComment.name;
    if (nameNode !== undefined) {
      return [nameNode.getStart(), nameNode.getEnd()];
    }
    return undefined;
  }
  static getObjectLiteralNoContextTypeHighlightRange(nodeOrComment) {
    return this.getKeywordHighlightRange(nodeOrComment, '{');
  }
  static getClassExpressionHighlightRange(nodeOrComment) {
    return this.getKeywordHighlightRange(nodeOrComment, 'class');
  }
  static getMultipleStaticBlocksHighlightRange(nodeOrComment) {
    return this.getKeywordHighlightRange(nodeOrComment, 'static');
  }
  static getParameterPropertiesHighlightRange(nodeOrComment) {
    const params = nodeOrComment.parameters;
    if (params.length) {
      return [params[0].getStart(), params[params.length - 1].getEnd()];
    }
    return undefined;
  }
  static getObjectTypeLiteralHighlightRange(nodeOrComment) {
    return this.getKeywordHighlightRange(nodeOrComment, '{');
  }

  // highlight ranges for Sendable rules

  static getSendableDefiniteAssignmentHighlightRange(nodeOrComment) {
    const name = nodeOrComment.name;
    const exclamationToken = nodeOrComment.exclamationToken;
    return [name.getStart(), exclamationToken ? exclamationToken.getEnd() : name.getEnd()];
  }
  isStdRecordType(type) {
    /*
     * In TypeScript, 'Record<K, T>' is defined as type alias to a mapped type.
     * Thus, it should have 'aliasSymbol' and 'target' properties. The 'target'
     * in this case will resolve to origin 'Record' symbol.
     */
    if (type.aliasSymbol) {
      const target = type.target;
      if (target) {
        const sym = target.aliasSymbol;
        return !!sym && sym.getName() === 'Record' && this.isGlobalSymbol(sym);
      }
    }
    return false;
  }
  isStdErrorType(type) {
    const symbol = type.symbol;
    if (!symbol) {
      return false;
    }
    const name = this.tsTypeChecker.getFullyQualifiedName(symbol);
    return name === 'Error' && this.isGlobalSymbol(symbol);
  }
  isStdPartialType(type) {
    const sym = type.aliasSymbol;
    return !!sym && sym.getName() === 'Partial' && this.isGlobalSymbol(sym);
  }
  isStdRequiredType(type) {
    const sym = type.aliasSymbol;
    return !!sym && sym.getName() === 'Required' && this.isGlobalSymbol(sym);
  }
  isStdReadonlyType(type) {
    const sym = type.aliasSymbol;
    return !!sym && sym.getName() === 'Readonly' && this.isGlobalSymbol(sym);
  }
  isLibraryType(type) {
    const nonNullableType = type.getNonNullableType();
    if (nonNullableType.isUnion()) {
      for (const componentType of nonNullableType.types) {
        if (!this.isLibraryType(componentType)) {
          return false;
        }
      }
      return true;
    }
    return this.isLibrarySymbol(nonNullableType.aliasSymbol ?? nonNullableType.getSymbol());
  }
  hasLibraryType(node) {
    return this.isLibraryType(this.tsTypeChecker.getTypeAtLocation(node));
  }
  isLibrarySymbol(sym) {
    var _this3 = this;
    if (sym?.declarations && sym.declarations.length > 0) {
      const srcFile = sym.declarations[0].getSourceFile();
      if (!srcFile) {
        return false;
      }
      const fileName = srcFile.fileName;

      /*
       * Symbols from both *.ts and *.d.ts files should obey interop rules.
       * We disable such behavior for *.ts files in the test mode due to lack of 'ets'
       * extension support.
       */
      const ext = path.extname(fileName).toLowerCase();
      const isThirdPartyCode = _ArktsIgnorePaths.ARKTS_IGNORE_DIRS.some(function (ignore) {
        _newArrowCheck(this, _this3);
        return (0, _PathHelper.pathContainsDirectory)(path.normalize(fileName), ignore);
      }.bind(this)) || _ArktsIgnorePaths.ARKTS_IGNORE_FILES.some(function (ignore) {
        _newArrowCheck(this, _this3);
        return path.basename(fileName) === ignore;
      }.bind(this));
      const isEts = ext === '.ets';
      const isTs = ext === '.ts' && !srcFile.isDeclarationFile;
      const isStatic = (isEts || isTs && this.testMode) && !isThirdPartyCode;
      const isStdLib = _StandardLibraries.STANDARD_LIBRARIES.includes(path.basename(fileName).toLowerCase());

      /*
       * We still need to confirm support for certain API from the
       * TypeScript standard library in ArkTS. Thus, for now do not
       * count standard library modules as dynamic.
       */
      return !isStatic && !isStdLib;
    }
    return false;
  }
  isDynamicType(type) {
    if (type === undefined) {
      return false;
    }

    /*
     * Return 'true' if it is an object of library type initialization, otherwise
     * return 'false' if it is not an object of standard library type one.
     * In the case of standard library type we need to determine context.
     */

    /*
     * Check the non-nullable version of type to eliminate 'undefined' type
     * from the union type elements.
     */
    type = type.getNonNullableType();
    if (type.isUnion()) {
      for (const compType of type.types) {
        const isDynamic = this.isDynamicType(compType);
        if (isDynamic || isDynamic === undefined) {
          return isDynamic;
        }
      }
      return false;
    }
    if (this.isLibraryType(type)) {
      return true;
    }
    if (!(0, _IsStdLibrary.isStdLibraryType)(type) && !(0, _isIntrinsicObjectType.isIntrinsicObjectType)(type) && !TsUtils.isAnyType(type)) {
      return false;
    }
    return undefined;
  }
  static isObjectType(type) {
    return !!(type.flags & ts.TypeFlags.Object);
  }
  static isAnonymous(type) {
    if (TsUtils.isObjectType(type)) {
      return !!(type.objectFlags & ts.ObjectFlags.Anonymous);
    }
    return false;
  }
  isDynamicLiteralInitializerHandleCallExpression(callExpr) {
    const type = this.tsTypeChecker.getTypeAtLocation(callExpr.expression);
    if (TsUtils.isAnyType(type)) {
      return true;
    }
    let sym = type.symbol;
    if (this.isLibrarySymbol(sym)) {
      return true;
    }

    /*
     * #13483:
     * x.foo({ ... }), where 'x' is exported from some library:
     */
    if (ts.isPropertyAccessExpression(callExpr.expression)) {
      sym = this.trueSymbolAtLocation(callExpr.expression.expression);
      if (sym && this.isLibrarySymbol(sym)) {
        return true;
      }
    }
    return false;
  }
  isDynamicLiteralInitializer(expr) {
    if (!ts.isObjectLiteralExpression(expr) && !ts.isArrayLiteralExpression(expr)) {
      return false;
    }

    /*
     * Handle nested literals:
     * { f: { ... } }
     */
    let curNode = expr;
    while (ts.isObjectLiteralExpression(curNode) || ts.isArrayLiteralExpression(curNode)) {
      const exprType = this.tsTypeChecker.getContextualType(curNode);
      if (exprType !== undefined && !TsUtils.isAnonymous(exprType)) {
        const res = this.isDynamicType(exprType);
        if (res !== undefined) {
          return res;
        }
      }
      curNode = curNode.parent;
      if (ts.isPropertyAssignment(curNode)) {
        curNode = curNode.parent;
      }
    }

    /*
     * Handle calls with literals:
     * foo({ ... })
     */
    if (ts.isCallExpression(curNode) && this.isDynamicLiteralInitializerHandleCallExpression(curNode)) {
      return true;
    }

    /*
     * Handle property assignments with literals:
     * obj.f = { ... }
     */
    if (ts.isBinaryExpression(curNode)) {
      const binExpr = curNode;
      if (ts.isPropertyAccessExpression(binExpr.left)) {
        const propAccessExpr = binExpr.left;
        const type = this.tsTypeChecker.getTypeAtLocation(propAccessExpr.expression);
        return this.isLibrarySymbol(type.symbol);
      }
    }
    return false;
  }
  static isEsObjectType(typeNode) {
    return !!typeNode && ts.isTypeReferenceNode(typeNode) && ts.isIdentifier(typeNode.typeName) && typeNode.typeName.text === _ESObject.ES_OBJECT;
  }
  static isInsideBlock(node) {
    let par = node.parent;
    while (par) {
      if (ts.isBlock(par)) {
        return true;
      }
      par = par.parent;
    }
    return false;
  }
  static isEsObjectPossiblyAllowed(typeRef) {
    return ts.isVariableDeclaration(typeRef.parent);
  }
  isValueAssignableToESObject(node) {
    if (ts.isArrayLiteralExpression(node) || ts.isObjectLiteralExpression(node)) {
      return false;
    }
    const valueType = this.tsTypeChecker.getTypeAtLocation(node);
    return TsUtils.isUnsupportedType(valueType) || TsUtils.isAnonymousType(valueType);
  }
  getVariableDeclarationTypeNode(node) {
    const sym = this.trueSymbolAtLocation(node);
    if (sym === undefined) {
      return undefined;
    }
    return TsUtils.getSymbolDeclarationTypeNode(sym);
  }
  static getSymbolDeclarationTypeNode(sym) {
    const decl = TsUtils.getDeclaration(sym);
    if (!!decl && ts.isVariableDeclaration(decl)) {
      return decl.type;
    }
    return undefined;
  }
  hasEsObjectType(node) {
    const typeNode = this.getVariableDeclarationTypeNode(node);
    return typeNode !== undefined && TsUtils.isEsObjectType(typeNode);
  }
  static symbolHasEsObjectType(sym) {
    const typeNode = TsUtils.getSymbolDeclarationTypeNode(sym);
    return typeNode !== undefined && TsUtils.isEsObjectType(typeNode);
  }
  static isEsObjectSymbol(sym) {
    const decl = TsUtils.getDeclaration(sym);
    return !!decl && ts.isTypeAliasDeclaration(decl) && decl.name.escapedText === _ESObject.ES_OBJECT && decl.type.kind === ts.SyntaxKind.AnyKeyword;
  }
  static isAnonymousType(type) {
    if (type.isUnionOrIntersection()) {
      for (const compType of type.types) {
        if (TsUtils.isAnonymousType(compType)) {
          return true;
        }
      }
      return false;
    }
    return (type.flags & ts.TypeFlags.Object) !== 0 && (type.objectFlags & ts.ObjectFlags.Anonymous) !== 0;
  }
  getSymbolOfCallExpression(callExpr) {
    const signature = this.tsTypeChecker.getResolvedSignature(callExpr);
    const signDecl = signature?.getDeclaration();
    if (signDecl?.name) {
      return this.trueSymbolAtLocation(signDecl.name);
    }
    return undefined;
  }
  static isClassValueType(type) {
    if ((type.flags & ts.TypeFlags.Object) === 0 || (type.objectFlags & ts.ObjectFlags.Anonymous) === 0) {
      return false;
    }
    return type.symbol && (type.symbol.flags & ts.SymbolFlags.Class) !== 0;
  }
  isClassObjectExpression(expr) {
    if (!TsUtils.isClassValueType(this.tsTypeChecker.getTypeAtLocation(expr))) {
      return false;
    }
    const symbol = this.trueSymbolAtLocation(expr);
    return !symbol || (symbol.flags & ts.SymbolFlags.Class) === 0;
  }
  isClassTypeExrepssion(expr) {
    const sym = this.trueSymbolAtLocation(expr);
    return sym !== undefined && (sym.flags & ts.SymbolFlags.Class) !== 0;
  }
  isFunctionCalledRecursively(funcExpr) {
    var _this4 = this;
    if (!funcExpr.name) {
      return false;
    }
    const sym = this.tsTypeChecker.getSymbolAtLocation(funcExpr.name);
    if (!sym) {
      return false;
    }
    let found = false;
    const callback = function callback(node) {
      _newArrowCheck(this, _this4);
      if (ts.isCallExpression(node) && ts.isIdentifier(node.expression)) {
        const callSym = this.tsTypeChecker.getSymbolAtLocation(node.expression);
        if (callSym && callSym === sym) {
          found = true;
        }
      }
    }.bind(this);
    const stopCondition = function stopCondition(node) {
      _newArrowCheck(this, _this4);
      void node;
      return found;
    }.bind(this);
    (0, _ForEachNodeInSubtree.forEachNodeInSubtree)(funcExpr, callback, stopCondition);
    return found;
  }
  getTypeOrTypeConstraintAtLocation(expr) {
    const type = this.tsTypeChecker.getTypeAtLocation(expr);
    if (type.isTypeParameter()) {
      const constraint = type.getConstraint();
      if (constraint) {
        return constraint;
      }
    }
    return type;
  }
  areCompatibleFunctionals(lhsType, rhsType) {
    return (this.isStdFunctionType(lhsType) || TsUtils.isFunctionalType(lhsType)) && (this.isStdFunctionType(rhsType) || TsUtils.isFunctionalType(rhsType));
  }
  static isFunctionalType(type) {
    const callSigns = type.getCallSignatures();
    return callSigns && callSigns.length > 0;
  }
  isStdFunctionType(type) {
    const sym = type.getSymbol();
    return !!sym && sym.getName() === 'Function' && this.isGlobalSymbol(sym);
  }
  isStdBigIntType(type) {
    const sym = type.symbol;
    return !!sym && sym.getName() === 'BigInt' && this.isGlobalSymbol(sym);
  }
  isStdNumberType(type) {
    const sym = type.symbol;
    return !!sym && sym.getName() === 'Number' && this.isGlobalSymbol(sym);
  }
  isStdBooleanType(type) {
    const sym = type.symbol;
    return !!sym && sym.getName() === 'Boolean' && this.isGlobalSymbol(sym);
  }
  isEnumStringLiteral(expr) {
    const symbol = this.trueSymbolAtLocation(expr);
    const isEnumMember = !!symbol && !!(symbol.flags & ts.SymbolFlags.EnumMember);
    const type = this.tsTypeChecker.getTypeAtLocation(expr);
    const isStringEnumLiteral = TsUtils.isEnumType(type) && !!(type.flags & ts.TypeFlags.StringLiteral);
    return isEnumMember && isStringEnumLiteral;
  }
  isValidComputedPropertyName(computedProperty, isRecordObjectInitializer = false) {
    const expr = computedProperty.expression;
    if (!isRecordObjectInitializer) {
      const symbol = this.trueSymbolAtLocation(expr);
      if (!!symbol && this.isSymbolIterator(symbol)) {
        return true;
      }
    }
    // We allow computed property names if expression is string literal or string Enum member
    return ts.isStringLiteralLike(expr) || this.isEnumStringLiteral(computedProperty.expression);
  }
  skipPropertyInferredTypeCheck(decl, sourceFile, isEtsFileCb) {
    var _this5 = this;
    if (!sourceFile) {
      return false;
    }
    const isEts = this.useRtLogic ? !!isEtsFileCb && isEtsFileCb(sourceFile) : (0, _GetScriptKind.getScriptKind)(sourceFile) === ts.ScriptKind.ETS;
    return isEts && sourceFile.isDeclarationFile && !!decl.modifiers?.some(function (m) {
      _newArrowCheck(this, _this5);
      return m.kind === ts.SyntaxKind.PrivateKeyword;
    }.bind(this));
  }
  hasAccessModifier(decl) {
    const modifiers = ts.getModifiers(decl);
    return !!modifiers && (!this.useRtLogic && TsUtils.hasModifier(modifiers, ts.SyntaxKind.ReadonlyKeyword) || TsUtils.hasModifier(modifiers, ts.SyntaxKind.PublicKeyword) || TsUtils.hasModifier(modifiers, ts.SyntaxKind.ProtectedKeyword) || TsUtils.hasModifier(modifiers, ts.SyntaxKind.PrivateKeyword));
  }
  static getModifier(modifiers, modifierKind) {
    var _this6 = this;
    if (!modifiers) {
      return undefined;
    }
    return modifiers.find(function (x) {
      _newArrowCheck(this, _this6);
      return x.kind === modifierKind;
    }.bind(this));
  }
  static getAccessModifier(modifiers) {
    return TsUtils.getModifier(modifiers, ts.SyntaxKind.PublicKeyword) ?? TsUtils.getModifier(modifiers, ts.SyntaxKind.ProtectedKeyword) ?? TsUtils.getModifier(modifiers, ts.SyntaxKind.PrivateKeyword);
  }
  static getBaseClassType(type) {
    const baseTypes = type.getBaseTypes();
    if (baseTypes) {
      for (const baseType of baseTypes) {
        if (baseType.isClass()) {
          return baseType;
        }
      }
    }
    return undefined;
  }
  static destructuringAssignmentHasSpreadOperator(node) {
    var _this7 = this;
    if (ts.isArrayLiteralExpression(node)) {
      return node.elements.some(function (x) {
        _newArrowCheck(this, _this7);
        if (ts.isSpreadElement(x)) {
          return true;
        }
        if (ts.isObjectLiteralExpression(x) || ts.isArrayLiteralExpression(x)) {
          return TsUtils.destructuringAssignmentHasSpreadOperator(x);
        }
        return false;
      }.bind(this));
    }
    return node.properties.some(function (x) {
      _newArrowCheck(this, _this7);
      if (ts.isSpreadAssignment(x)) {
        return true;
      }
      if (ts.isPropertyAssignment(x) && (ts.isObjectLiteralExpression(x.initializer) || ts.isArrayLiteralExpression(x.initializer))) {
        return TsUtils.destructuringAssignmentHasSpreadOperator(x.initializer);
      }
      return false;
    }.bind(this));
  }
  static destructuringDeclarationHasSpreadOperator(node) {
    var _this8 = this;
    return node.elements.some(function (x) {
      _newArrowCheck(this, _this8);
      if (ts.isBindingElement(x)) {
        if (x.dotDotDotToken) {
          return true;
        }
        if (ts.isArrayBindingPattern(x.name) || ts.isObjectBindingPattern(x.name)) {
          return TsUtils.destructuringDeclarationHasSpreadOperator(x.name);
        }
      }
      return false;
    }.bind(this));
  }
  static hasNestedObjectDestructuring(node) {
    var _this9 = this;
    if (ts.isArrayLiteralExpression(node)) {
      return node.elements.some(function (x) {
        _newArrowCheck(this, _this9);
        const elem = ts.isSpreadElement(x) ? x.expression : x;
        if (ts.isArrayLiteralExpression(elem)) {
          return TsUtils.hasNestedObjectDestructuring(elem);
        }
        return ts.isObjectLiteralExpression(elem);
      }.bind(this));
    }
    return node.elements.some(function (x) {
      _newArrowCheck(this, _this9);
      if (ts.isBindingElement(x)) {
        if (ts.isArrayBindingPattern(x.name)) {
          return TsUtils.hasNestedObjectDestructuring(x.name);
        }
        return ts.isObjectBindingPattern(x.name);
      }
      return false;
    }.bind(this));
  }
  static getDecoratorName(decorator) {
    let decoratorName = '';
    if (ts.isIdentifier(decorator.expression)) {
      decoratorName = decorator.expression.text;
    } else if (ts.isCallExpression(decorator.expression) && ts.isIdentifier(decorator.expression.expression)) {
      decoratorName = decorator.expression.expression.text;
    }
    return decoratorName;
  }
  static unwrapParenthesizedTypeNode(typeNode) {
    let unwrappedTypeNode = typeNode;
    while (ts.isParenthesizedTypeNode(unwrappedTypeNode)) {
      unwrappedTypeNode = unwrappedTypeNode.type;
    }
    return unwrappedTypeNode;
  }
  isSendableTypeNode(typeNode) {
    var _this10 = this;
    /*
     * In order to correctly identify the usage of the enum member or
     * const enum in type annotation, we need to handle union type and
     * type alias cases by processing the type node and checking the
     * symbol in case of type reference node.
     */

    typeNode = TsUtils.unwrapParenthesizedTypeNode(typeNode);

    // Only a sendable union type is supported
    if (ts.isUnionTypeNode(typeNode)) {
      return typeNode.types.every(function (elemType) {
        _newArrowCheck(this, _this10);
        return this.isSendableTypeNode(elemType);
      }.bind(this));
    }
    const sym = ts.isTypeReferenceNode(typeNode) ? this.trueSymbolAtLocation(typeNode.typeName) : undefined;
    if (sym && sym.getFlags() & ts.SymbolFlags.TypeAlias) {
      const typeDecl = TsUtils.getDeclaration(sym);
      if (typeDecl && ts.isTypeAliasDeclaration(typeDecl)) {
        return this.isSendableTypeNode(typeDecl.type);
      }
    }

    // Const enum type is supported
    if (TsUtils.isConstEnum(sym)) {
      return true;
    }
    return this.isSendableType(this.tsTypeChecker.getTypeFromTypeNode(typeNode));
  }
  isSendableType(type) {
    if ((type.flags & (ts.TypeFlags.Boolean | ts.TypeFlags.Number | ts.TypeFlags.String | ts.TypeFlags.BigInt | ts.TypeFlags.Null | ts.TypeFlags.Undefined | ts.TypeFlags.TypeParameter)) !== 0) {
      return true;
    }
    return this.isSendableClassOrInterface(type);
  }
  isShareableType(tsType) {
    var _this11 = this;
    const sym = tsType.getSymbol();
    if (TsUtils.isConstEnum(sym)) {
      return true;
    }
    if (tsType.isUnion()) {
      return tsType.types.every(function (elemType) {
        _newArrowCheck(this, _this11);
        return this.isShareableType(elemType);
      }.bind(this));
    }
    return this.isSendableType(tsType);
  }
  isSendableClassOrInterface(type) {
    const sym = type.getSymbol();
    if (!sym) {
      return false;
    }
    const targetType = TsUtils.reduceReference(type);

    // class with @Sendable decorator
    if (targetType.isClass()) {
      if (sym.declarations?.length) {
        const decl = sym.declarations[0];
        if (ts.isClassDeclaration(decl)) {
          return TsUtils.hasSendableDecorator(decl);
        }
      }
    }
    // ISendable interface, or a class/interface that implements/extends ISendable interface
    return this.isOrDerivedFrom(type, TsUtils.isISendableInterface);
  }
  typeContainsSendableClassOrInterface(type) {
    var _this12 = this;
    // Only check type contains sendable class / interface
    if ((type.flags & ts.TypeFlags.Union) !== 0) {
      return !!type?.types?.some(function (type) {
        _newArrowCheck(this, _this12);
        return this.typeContainsSendableClassOrInterface(type);
      }.bind(this));
    }
    return this.isSendableClassOrInterface(type);
  }
  static isConstEnum(sym) {
    return !!sym && sym.flags === ts.SymbolFlags.ConstEnum;
  }
  isSendableUnionType(type) {
    var _this13 = this;
    const types = type?.types;
    if (!types) {
      return false;
    }
    return types.every(function (type) {
      _newArrowCheck(this, _this13);
      return this.isSendableType(type);
    }.bind(this));
  }
  static hasSendableDecorator(decl) {
    var _this14 = this;
    const decorators = ts.getDecorators(decl);
    return !!decorators?.some(function (x) {
      _newArrowCheck(this, _this14);
      return TsUtils.getDecoratorName(x) === _SendableAPI.SENDABLE_DECORATOR;
    }.bind(this));
  }
  static getNonSendableDecorators(decl) {
    var _this15 = this;
    const decorators = ts.getDecorators(decl);
    return decorators?.filter(function (x) {
      _newArrowCheck(this, _this15);
      return TsUtils.getDecoratorName(x) !== _SendableAPI.SENDABLE_DECORATOR;
    }.bind(this));
  }
  static getDecoratorsIfInSendableClass(declaration) {
    const classNode = TsUtils.getClassNodeFromDeclaration(declaration);
    if (classNode === undefined || !TsUtils.hasSendableDecorator(classNode)) {
      return undefined;
    }
    return ts.getDecorators(declaration);
  }
  static getClassNodeFromDeclaration(declaration) {
    if (declaration.kind === ts.SyntaxKind.Parameter) {
      return ts.isClassDeclaration(declaration.parent.parent) ? declaration.parent.parent : undefined;
    }
    return ts.isClassDeclaration(declaration.parent) ? declaration.parent : undefined;
  }
  static isISendableInterface(type) {
    const symbol = type.aliasSymbol ?? type.getSymbol();
    if (symbol?.declarations === undefined || symbol.declarations.length < 1) {
      return false;
    }
    return TsUtils.isArkTSISendableDeclaration(symbol.declarations[0]);
  }
  static isArkTSISendableDeclaration(decl) {
    if (!ts.isInterfaceDeclaration(decl) || !decl.name || decl.name.text !== _SupportedDetsIndexableTypes.ISENDABLE_TYPE) {
      return false;
    }
    if (!ts.isModuleBlock(decl.parent) || decl.parent.parent.name.text !== _SupportedDetsIndexableTypes.LANG_NAMESPACE) {
      return false;
    }
    if (path.basename(decl.getSourceFile().fileName).toLowerCase() !== _SupportedDetsIndexableTypes.ARKTS_LANG_D_ETS) {
      return false;
    }
    return true;
  }
  isAllowedIndexSignature(node) {
    /*
     * For now, relax index signature only for specific array-like types
     * with the following signature: 'collections.Array<T>.[_: number]: T'.
     */

    if (node.parameters.length !== 1) {
      return false;
    }
    const paramType = this.tsTypeChecker.getTypeAtLocation(node.parameters[0]);
    if ((paramType.flags & ts.TypeFlags.Number) === 0) {
      return false;
    }
    return this.isArkTSCollectionsArrayLikeDeclaration(node.parent);
  }
  isArkTSCollectionsArrayLikeType(type) {
    const symbol = type.aliasSymbol ?? type.getSymbol();
    if (symbol?.declarations === undefined || symbol.declarations.length < 1) {
      return false;
    }
    return this.isArkTSCollectionsArrayLikeDeclaration(symbol.declarations[0]);
  }
  isArkTSCollectionsArrayLikeDeclaration(decl) {
    if (!ts.isClassDeclaration(decl) && !ts.isInterfaceDeclaration(decl) || !decl.name) {
      return false;
    }
    if (!this.tsTypeChecker.getTypeAtLocation(decl).getNumberIndexType()) {
      return false;
    }
    if (!ts.isModuleBlock(decl.parent) || decl.parent.parent.name.text !== _SupportedDetsIndexableTypes.COLLECTIONS_NAMESPACE) {
      return false;
    }
    if (path.basename(decl.getSourceFile().fileName).toLowerCase() !== _SupportedDetsIndexableTypes.ARKTS_COLLECTIONS_D_ETS) {
      return false;
    }
    return true;
  }
  proceedConstructorDeclaration(isFromPrivateIdentifierOrSdk, targetMember, classMember, isFromPrivateIdentifier) {
    var _this16 = this;
    if (isFromPrivateIdentifierOrSdk && ts.isConstructorDeclaration(classMember) && classMember.parameters.some(function (x) {
      _newArrowCheck(this, _this16);
      return ts.isIdentifier(x.name) && this.hasAccessModifier(x) && this.isPrivateIdentifierDuplicateOfIdentifier(targetMember.name, x.name, isFromPrivateIdentifier);
    }.bind(this))) {
      return true;
    }
    return undefined;
  }
  proceedClassType(targetMember, classType, isFromPrivateIdentifier) {
    if (classType) {
      const baseType = TsUtils.getBaseClassType(classType);
      if (baseType) {
        const baseDecl = baseType.getSymbol()?.valueDeclaration;
        if (baseDecl) {
          return this.classMemberHasDuplicateName(targetMember, baseDecl, isFromPrivateIdentifier);
        }
      }
    }
    return undefined;
  }
  classMemberHasDuplicateName(targetMember, tsClassLikeDecl, isFromPrivateIdentifier, classType) {
    /*
     * If two class members have the same name where one is a private identifer,
     * then such members are considered to have duplicate names.
     */
    if (!TsUtils.isIdentifierOrPrivateIdentifier(targetMember.name)) {
      return false;
    }
    const isFromPrivateIdentifierOrSdk = this.isFromPrivateIdentifierOrSdk(isFromPrivateIdentifier);
    for (const classMember of tsClassLikeDecl.members) {
      if (targetMember === classMember) {
        continue;
      }

      // Check constructor parameter properties.
      const constructorDeclarationProceedResult = this.proceedConstructorDeclaration(isFromPrivateIdentifierOrSdk, targetMember, classMember, isFromPrivateIdentifier);
      if (constructorDeclarationProceedResult) {
        return constructorDeclarationProceedResult;
      }
      if (!TsUtils.isIdentifierOrPrivateIdentifier(classMember.name)) {
        continue;
      }
      if (this.isPrivateIdentifierDuplicateOfIdentifier(targetMember.name, classMember.name, isFromPrivateIdentifier)) {
        return true;
      }
    }
    if (isFromPrivateIdentifierOrSdk) {
      classType ??= this.tsTypeChecker.getTypeAtLocation(tsClassLikeDecl);
      const proceedClassTypeResult = this.proceedClassType(targetMember, classType, isFromPrivateIdentifier);
      if (proceedClassTypeResult) {
        return proceedClassTypeResult;
      }
    }
    return false;
  }
  isFromPrivateIdentifierOrSdk(isFromPrivateIdentifier) {
    return this.useRtLogic || isFromPrivateIdentifier;
  }
  static isIdentifierOrPrivateIdentifier(node) {
    if (!node) {
      return false;
    }
    return ts.isIdentifier(node) || ts.isPrivateIdentifier(node);
  }
  isPrivateIdentifierDuplicateOfIdentifier(ident1, ident2, isFromPrivateIdentifier) {
    if (ts.isIdentifier(ident1) && ts.isPrivateIdentifier(ident2)) {
      return ident1.text === ident2.text.substring(1);
    }
    if (ts.isIdentifier(ident2) && ts.isPrivateIdentifier(ident1)) {
      return ident2.text === ident1.text.substring(1);
    }
    if (this.isFromPrivateIdentifierOrSdk(isFromPrivateIdentifier) && ts.isPrivateIdentifier(ident1) && ts.isPrivateIdentifier(ident2)) {
      return ident1.text.substring(1) === ident2.text.substring(1);
    }
    return false;
  }
  findIdentifierNameForSymbol(symbol) {
    let name = TsUtils.getIdentifierNameFromString(symbol.name);
    if (name === undefined || name === symbol.name) {
      return name;
    }
    const parentType = this.getTypeByProperty(symbol);
    if (parentType === undefined) {
      return undefined;
    }
    while (this.findProperty(parentType, name) !== undefined) {
      name = '_' + name;
    }
    return name;
  }
  static getIdentifierNameFromString(str) {
    let result = '';
    let offset = 0;
    while (offset < str.length) {
      const codePoint = str.codePointAt(offset);
      if (!codePoint) {
        return undefined;
      }
      const charSize = TsUtils.charSize(codePoint);
      if (offset === 0 && !ts.isIdentifierStart(codePoint, undefined)) {
        result = '__';
      }
      if (!ts.isIdentifierPart(codePoint, undefined)) {
        if (codePoint === 0x20) {
          result += '_';
        } else {
          result += 'x' + codePoint.toString(16);
        }
      } else {
        for (let i = 0; i < charSize; i++) {
          result += str.charAt(offset + i);
        }
      }
      offset += charSize;
    }
    return result;
  }
  static charSize(codePoint) {
    return codePoint >= 0x10000 ? 2 : 1;
  }
  getTypeByProperty(symbol) {
    if (symbol.declarations === undefined) {
      return undefined;
    }
    for (const propDecl of symbol.declarations) {
      if (!ts.isPropertyDeclaration(propDecl) && !ts.isPropertyAssignment(propDecl) && !ts.isPropertySignature(propDecl)) {
        return undefined;
      }
      const type = this.tsTypeChecker.getTypeAtLocation(propDecl.parent);
      if (type !== undefined) {
        return type;
      }
    }
    return undefined;
  }
  static isPropertyOfInternalClassOrInterface(symbol) {
    if (symbol.declarations === undefined) {
      return false;
    }
    for (const propDecl of symbol.declarations) {
      if (!ts.isPropertyDeclaration(propDecl) && !ts.isPropertySignature(propDecl)) {
        return false;
      }
      if (!ts.isClassDeclaration(propDecl.parent) && !ts.isInterfaceDeclaration(propDecl.parent)) {
        return false;
      }
      if (TsUtils.hasModifier(ts.getModifiers(propDecl.parent), ts.SyntaxKind.ExportKeyword)) {
        return false;
      }
    }
    return true;
  }
  static isIntrinsicObjectType(type) {
    return !!(type.flags & ts.TypeFlags.NonPrimitive);
  }
  isStringType(tsType) {
    if ((tsType.getFlags() & ts.TypeFlags.String) !== 0) {
      return true;
    }
    if (!TsUtils.isTypeReference(tsType)) {
      return false;
    }
    const symbol = tsType.symbol;
    const name = this.tsTypeChecker.getFullyQualifiedName(symbol);
    return name === 'String' && this.isGlobalSymbol(symbol);
  }
  isStdMapType(type) {
    const sym = type.symbol;
    return !!sym && sym.getName() === 'Map' && this.isGlobalSymbol(sym);
  }
  hasGenericTypeParameter(type) {
    var _this17 = this;
    if (type.isUnionOrIntersection()) {
      return type.types.some(function (x) {
        _newArrowCheck(this, _this17);
        return this.hasGenericTypeParameter(x);
      }.bind(this));
    }
    if (TsUtils.isTypeReference(type)) {
      const typeArgs = this.tsTypeChecker.getTypeArguments(type);
      return typeArgs.some(function (x) {
        _newArrowCheck(this, _this17);
        return this.hasGenericTypeParameter(x);
      }.bind(this));
    }
    return type.isTypeParameter();
  }
  static getEnclosingTopLevelStatement(node) {
    var _this18 = this;
    return ts.findAncestor(node, function (ancestor) {
      _newArrowCheck(this, _this18);
      return ts.isSourceFile(ancestor.parent);
    }.bind(this));
  }
  static isDeclarationStatement(node) {
    const kind = node.kind;
    return kind === ts.SyntaxKind.FunctionDeclaration || kind === ts.SyntaxKind.ModuleDeclaration || kind === ts.SyntaxKind.ClassDeclaration || kind === ts.SyntaxKind.StructDeclaration || kind === ts.SyntaxKind.TypeAliasDeclaration || kind === ts.SyntaxKind.InterfaceDeclaration || kind === ts.SyntaxKind.EnumDeclaration || kind === ts.SyntaxKind.MissingDeclaration || kind === ts.SyntaxKind.ImportEqualsDeclaration || kind === ts.SyntaxKind.ImportDeclaration || kind === ts.SyntaxKind.NamespaceExportDeclaration;
  }
  static declarationNameExists(srcFile, name) {
    var _this19 = this;
    return srcFile.statements.some(function (stmt) {
      var _this20 = this;
      _newArrowCheck(this, _this19);
      if (ts.isImportDeclaration(stmt)) {
        if (!stmt.importClause) {
          return false;
        }
        if (stmt.importClause.namedBindings) {
          if (ts.isNamespaceImport(stmt.importClause.namedBindings)) {
            return stmt.importClause.namedBindings.name.text === name;
          }
          return stmt.importClause.namedBindings.elements.some(function (x) {
            _newArrowCheck(this, _this20);
            return x.name.text === name;
          }.bind(this));
        }
        return stmt.importClause.name?.text === name;
      }
      return TsUtils.isDeclarationStatement(stmt) && stmt.name !== undefined && ts.isIdentifier(stmt.name) && stmt.name.text === name;
    }.bind(this));
  }
  static generateUniqueName(nameGenerator, srcFile) {
    let newName;
    do {
      newName = nameGenerator.getName();
      if (newName !== undefined && TsUtils.declarationNameExists(srcFile, newName)) {
        continue;
      }
      break;
    } while (newName !== undefined);
    return newName;
  }
  static isSharedModule(sourceFile) {
    const statements = sourceFile.statements;
    for (const statement of statements) {
      if (ts.isImportDeclaration(statement)) {
        continue;
      }
      return ts.isExpressionStatement(statement) && ts.isStringLiteral(statement.expression) && statement.expression.text === _SharedModuleAPI.USE_SHARED;
    }
    return false;
  }
  getDeclarationNode(node) {
    const sym = this.trueSymbolAtLocation(node);
    return TsUtils.getDeclaration(sym);
  }
  static isFunctionLikeDeclaration(node) {
    return ts.isFunctionDeclaration(node) || ts.isMethodDeclaration(node) || ts.isGetAccessorDeclaration(node) || ts.isSetAccessorDeclaration(node) || ts.isConstructorDeclaration(node) || ts.isFunctionExpression(node) || ts.isArrowFunction(node);
  }
  isShareableEntity(node) {
    const decl = this.getDeclarationNode(node);
    const typeNode = decl?.type;
    return typeNode && !TsUtils.isFunctionLikeDeclaration(decl) ? this.isSendableTypeNode(typeNode) : this.isShareableType(this.tsTypeChecker.getTypeAtLocation(decl ? decl : node));
  }
  isSendableClassOrInterfaceEntity(node) {
    const decl = this.getDeclarationNode(node);
    if (!decl) {
      return false;
    }
    if (ts.isClassDeclaration(decl)) {
      return TsUtils.hasSendableDecorator(decl);
    }
    if (ts.isInterfaceDeclaration(decl)) {
      return this.isOrDerivedFrom(this.tsTypeChecker.getTypeAtLocation(decl), TsUtils.isISendableInterface);
    }
    return false;
  }
  static isInImportWhiteList(resolvedModule) {
    if (!resolvedModule.resolvedFileName || path.basename(resolvedModule.resolvedFileName).toLowerCase() !== _SupportedDetsIndexableTypes.ARKTS_LANG_D_ETS && path.basename(resolvedModule.resolvedFileName).toLowerCase() !== _SupportedDetsIndexableTypes.ARKTS_COLLECTIONS_D_ETS) {
      return false;
    }
    return true;
  }
  isClassNodeReference(node) {
    var _this21 = this;
    // handle union type node
    if (ts.isUnionTypeNode(node)) {
      return node.types.every(function (elemType) {
        _newArrowCheck(this, _this21);
        return this.isClassNode(elemType);
      }.bind(this));
    }
    return this.isClassNode(node);
  }
  isClassNode(node) {
    const decl = this.getDeclarationNode(node);
    return decl ? ts.isClassDeclaration(decl) : false;
  }
}
exports.TsUtils = TsUtils;
_TsUtils = TsUtils;
_defineProperty(TsUtils, "highlightRangeHandlers", new Map([[_Problems.FaultID.VarDeclaration, _TsUtils.getVarDeclarationHighlightRange], [_Problems.FaultID.CatchWithUnsupportedType, _TsUtils.getCatchWithUnsupportedTypeHighlightRange], [_Problems.FaultID.ForInStatement, _TsUtils.getForInStatementHighlightRange], [_Problems.FaultID.WithStatement, _TsUtils.getWithStatementHighlightRange], [_Problems.FaultID.DeleteOperator, _TsUtils.getDeleteOperatorHighlightRange], [_Problems.FaultID.TypeQuery, _TsUtils.getTypeQueryHighlightRange], [_Problems.FaultID.InstanceofUnsupported, _TsUtils.getInstanceofUnsupportedHighlightRange], [_Problems.FaultID.ConstAssertion, _TsUtils.getConstAssertionHighlightRange], [_Problems.FaultID.LimitedReturnTypeInference, _TsUtils.getLimitedReturnTypeInferenceHighlightRange], [_Problems.FaultID.LocalFunction, _TsUtils.getLocalFunctionHighlightRange], [_Problems.FaultID.FunctionBind, _TsUtils.getFunctionApplyCallHighlightRange], [_Problems.FaultID.FunctionApplyCall, _TsUtils.getFunctionApplyCallHighlightRange], [_Problems.FaultID.DeclWithDuplicateName, _TsUtils.getDeclWithDuplicateNameHighlightRange], [_Problems.FaultID.ObjectLiteralNoContextType, _TsUtils.getObjectLiteralNoContextTypeHighlightRange], [_Problems.FaultID.ClassExpression, _TsUtils.getClassExpressionHighlightRange], [_Problems.FaultID.MultipleStaticBlocks, _TsUtils.getMultipleStaticBlocksHighlightRange], [_Problems.FaultID.ParameterProperties, _TsUtils.getParameterPropertiesHighlightRange], [_Problems.FaultID.SendableDefiniteAssignment, _TsUtils.getSendableDefiniteAssignmentHighlightRange], [_Problems.FaultID.ObjectTypeLiteral, _TsUtils.getObjectTypeLiteralHighlightRange]]));
//# sourceMappingURL=TsUtils.js.map