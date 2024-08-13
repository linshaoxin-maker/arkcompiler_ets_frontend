"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.Autofixer = void 0;
var ts = _interopRequireWildcard(require("typescript"));
var _TsUtils = require("../utils/TsUtils");
var _ContainsThis = require("../utils/functions/ContainsThis");
var _ForEachNodeInSubtree = require("../utils/functions/ForEachNodeInSubtree");
var _NameGenerator = require("../utils/functions/NameGenerator");
var _isAssignmentOperator = require("../utils/functions/isAssignmentOperator");
var _SymbolCache = require("./SymbolCache");
function _getRequireWildcardCache(e) { if ("function" != typeof WeakMap) return null; var r = new WeakMap(), t = new WeakMap(); return (_getRequireWildcardCache = function (e) { return e ? t : r; })(e); }
function _interopRequireWildcard(e, r) { if (!r && e && e.__esModule) return e; if (null === e || "object" != typeof e && "function" != typeof e) return { default: e }; var t = _getRequireWildcardCache(r); if (t && t.has(e)) return t.get(e); var n = { __proto__: null }, a = Object.defineProperty && Object.getOwnPropertyDescriptor; for (var u in e) if ("default" !== u && {}.hasOwnProperty.call(e, u)) { var i = a ? Object.getOwnPropertyDescriptor(e, u) : null; i && (i.get || i.set) ? Object.defineProperty(n, u, i) : n[u] = e[u]; } return n.default = e, t && t.set(e, n), n; }
function _newArrowCheck(n, r) { if (n !== r) throw new TypeError("Cannot instantiate an arrow function"); }
function _defineProperty(e, r, t) { return (r = _toPropertyKey(r)) in e ? Object.defineProperty(e, r, { value: t, enumerable: !0, configurable: !0, writable: !0 }) : e[r] = t, e; }
function _toPropertyKey(t) { var i = _toPrimitive(t, "string"); return "symbol" == typeof i ? i : i + ""; }
function _toPrimitive(t, r) { if ("object" != typeof t || !t) return t; var e = t[Symbol.toPrimitive]; if (void 0 !== e) { var i = e.call(t, r || "default"); if ("object" != typeof i) return i; throw new TypeError("@@toPrimitive must return a primitive value."); } return ("string" === r ? String : Number)(t); } /*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
const GENERATED_OBJECT_LITERAL_INTERFACE_NAME = 'GeneratedObjectLiteralInterface_';
const GENERATED_OBJECT_LITERAL_INTERFACE_TRESHOLD = 1000;
const GENERATED_TYPE_LITERAL_INTERFACE_NAME = 'GeneratedTypeLiteralInterface_';
const GENERATED_TYPE_LITERAL_INTERFACE_TRESHOLD = 1000;
class Autofixer {
  constructor(typeChecker, utils, sourceFile, cancellationToken) {
    this.typeChecker = typeChecker;
    this.utils = utils;
    this.sourceFile = sourceFile;
    this.cancellationToken = cancellationToken;
    _defineProperty(this, "renameSymbolAsIdentifierCache", new Map());
    _defineProperty(this, "enumMergingCache", new Map());
    _defineProperty(this, "printer", ts.createPrinter({
      omitTrailingSemicolon: false,
      removeComments: false,
      newLine: ts.NewLineKind.LineFeed
    }));
    _defineProperty(this, "nonCommentPrinter", ts.createPrinter({
      omitTrailingSemicolon: false,
      removeComments: true,
      newLine: ts.NewLineKind.LineFeed
    }));
    _defineProperty(this, "privateIdentifierCache", new Map());
    _defineProperty(this, "objectLiteralInterfaceNameGenerator", new _NameGenerator.NameGenerator(GENERATED_OBJECT_LITERAL_INTERFACE_NAME, GENERATED_OBJECT_LITERAL_INTERFACE_TRESHOLD));
    _defineProperty(this, "typeLiteralInterfaceNameGenerator", new _NameGenerator.NameGenerator(GENERATED_TYPE_LITERAL_INTERFACE_NAME, GENERATED_TYPE_LITERAL_INTERFACE_TRESHOLD));
    _defineProperty(this, "symbolCache", void 0);
    this.symbolCache = new _SymbolCache.SymbolCache(this.typeChecker, this.utils, sourceFile, cancellationToken);
  }
  fixLiteralAsPropertyNamePropertyAssignment(node) {
    const contextualType = this.typeChecker.getContextualType(node.parent);
    if (contextualType === undefined) {
      return undefined;
    }
    const symbol = this.utils.getPropertySymbol(contextualType, node);
    if (symbol === undefined) {
      return undefined;
    }
    return this.renameSymbolAsIdentifier(symbol);
  }
  fixLiteralAsPropertyNamePropertyName(node) {
    const symbol = this.typeChecker.getSymbolAtLocation(node);
    if (symbol === undefined) {
      return undefined;
    }
    return this.renameSymbolAsIdentifier(symbol);
  }
  fixPropertyAccessByIndex(node) {
    const symbol = this.typeChecker.getSymbolAtLocation(node.argumentExpression);
    if (symbol === undefined) {
      return undefined;
    }
    return this.renameSymbolAsIdentifier(symbol);
  }
  renameSymbolAsIdentifier(symbol) {
    var _this = this;
    if (this.renameSymbolAsIdentifierCache.has(symbol)) {
      return this.renameSymbolAsIdentifierCache.get(symbol);
    }
    if (!_TsUtils.TsUtils.isPropertyOfInternalClassOrInterface(symbol)) {
      this.renameSymbolAsIdentifierCache.set(symbol, undefined);
      return undefined;
    }
    const newName = this.utils.findIdentifierNameForSymbol(symbol);
    if (newName === undefined) {
      this.renameSymbolAsIdentifierCache.set(symbol, undefined);
      return undefined;
    }
    let result = [];
    this.symbolCache.getReferences(symbol).forEach(function (node) {
      _newArrowCheck(this, _this);
      if (result === undefined) {
        return;
      }
      let autofix;
      if (ts.isPropertyDeclaration(node) || ts.isPropertyAssignment(node) || ts.isPropertySignature(node)) {
        autofix = Autofixer.renamePropertyName(node.name, newName);
      } else if (ts.isElementAccessExpression(node)) {
        autofix = Autofixer.renameElementAccessExpression(node, newName);
      }
      if (autofix === undefined) {
        result = undefined;
        return;
      }
      result.push(...autofix);
    }.bind(this));
    if (!result?.length) {
      result = undefined;
    }
    this.renameSymbolAsIdentifierCache.set(symbol, result);
    return result;
  }
  static renamePropertyName(node, newName) {
    if (ts.isComputedPropertyName(node)) {
      return undefined;
    }
    if (ts.isMemberName(node)) {
      if (ts.idText(node) !== newName) {
        return undefined;
      }
      return [];
    }
    return [{
      replacementText: newName,
      start: node.getStart(),
      end: node.getEnd()
    }];
  }
  static renameElementAccessExpression(node, newName) {
    const argExprKind = node.argumentExpression.kind;
    if (argExprKind !== ts.SyntaxKind.NumericLiteral && argExprKind !== ts.SyntaxKind.StringLiteral) {
      return undefined;
    }
    return [{
      replacementText: node.expression.getText() + '.' + newName,
      start: node.getStart(),
      end: node.getEnd()
    }];
  }
  fixFunctionExpression(funcExpr, retType = funcExpr.type, modifiers, isGenerator, hasUnfixableReturnType) {
    const hasThisKeyword = (0, _ContainsThis.scopeContainsThis)(funcExpr.body);
    const isCalledRecursively = this.utils.isFunctionCalledRecursively(funcExpr);
    if (isGenerator || hasThisKeyword || isCalledRecursively || hasUnfixableReturnType) {
      return undefined;
    }
    let arrowFunc = ts.factory.createArrowFunction(modifiers, funcExpr.typeParameters, funcExpr.parameters, retType, ts.factory.createToken(ts.SyntaxKind.EqualsGreaterThanToken), funcExpr.body);
    if (Autofixer.needsParentheses(funcExpr)) {
      arrowFunc = ts.factory.createParenthesizedExpression(arrowFunc);
    }
    const text = this.printer.printNode(ts.EmitHint.Unspecified, arrowFunc, funcExpr.getSourceFile());
    return [{
      start: funcExpr.getStart(),
      end: funcExpr.getEnd(),
      replacementText: text
    }];
  }
  static isNodeInWhileOrIf(node) {
    return node.kind === ts.SyntaxKind.WhileStatement || node.kind === ts.SyntaxKind.DoStatement || node.kind === ts.SyntaxKind.IfStatement;
  }
  static isNodeInForLoop(node) {
    return node.kind === ts.SyntaxKind.ForInStatement || node.kind === ts.SyntaxKind.ForOfStatement || node.kind === ts.SyntaxKind.ForStatement;
  }
  static parentInFor(node) {
    let parentNode = node.parent;
    while (parentNode) {
      if (Autofixer.isNodeInForLoop(parentNode)) {
        return parentNode;
      }
      parentNode = parentNode.parent;
    }
    return undefined;
  }
  static parentInCaseOrWhile(varDeclList) {
    let parentNode = varDeclList.parent;
    while (parentNode) {
      if (parentNode.kind === ts.SyntaxKind.CaseClause || Autofixer.isNodeInWhileOrIf(parentNode)) {
        return false;
      }
      parentNode = parentNode.parent;
    }
    return true;
  }
  static isFunctionLikeDeclarationKind(node) {
    switch (node.kind) {
      case ts.SyntaxKind.FunctionDeclaration:
      case ts.SyntaxKind.MethodDeclaration:
      case ts.SyntaxKind.Constructor:
      case ts.SyntaxKind.GetAccessor:
      case ts.SyntaxKind.SetAccessor:
      case ts.SyntaxKind.FunctionExpression:
      case ts.SyntaxKind.ArrowFunction:
        return true;
      default:
        return false;
    }
  }
  static findVarScope(node) {
    while (node !== undefined) {
      if (node.kind === ts.SyntaxKind.Block || node.kind === ts.SyntaxKind.SourceFile) {
        break;
      }
      node = node.parent;
    }
    return node;
  }
  static varHasScope(node, scope) {
    while (node !== undefined) {
      if (node === scope) {
        return true;
      }
      node = node.parent;
    }
    return false;
  }
  static varInFunctionForScope(node, scope) {
    while (node !== undefined) {
      if (Autofixer.isFunctionLikeDeclarationKind(node)) {
        break;
      }
      node = node.parent;
    }
    // node now Function like declaration

    // node need to check that function like declaration is in scope
    if (Autofixer.varHasScope(node, scope)) {
      // var use is in function scope, which is in for scope
      return true;
    }
    return false;
  }
  static selfDeclared(decl, ident) {
    // Do not check the same node
    if (ident === decl) {
      return false;
    }
    while (ident !== undefined) {
      if (ident.kind === ts.SyntaxKind.VariableDeclaration) {
        const declName = ident.name;
        if (declName === decl) {
          return true;
        }
      }
      ident = ident.parent;
    }
    return false;
  }
  static analizeTDZ(decl, identifiers) {
    for (const ident of identifiers) {
      if (Autofixer.selfDeclared(decl.name, ident)) {
        return false;
      }
      if (ident.pos < decl.pos) {
        return false;
      }
    }
    return true;
  }
  static analizeScope(decl, identifiers) {
    const scope = Autofixer.findVarScope(decl);
    if (scope === undefined) {
      return false;
    } else if (scope.kind === ts.SyntaxKind.Block) {
      for (const ident of identifiers) {
        if (!Autofixer.varHasScope(ident, scope)) {
          return false;
        }
      }
    } else if (scope.kind === ts.SyntaxKind.SourceFile) {
      // Do nothing
    } else {
      // Unreachable, but check it
      return false;
    }
    return true;
  }
  static analizeFor(decl, identifiers) {
    const forNode = Autofixer.parentInFor(decl);
    if (forNode) {
      // analize that var is initialized
      if (forNode.kind === ts.SyntaxKind.ForInStatement || forNode.kind === ts.SyntaxKind.ForOfStatement) {
        const typedForNode = forNode;
        const forVarDeclarations = typedForNode.initializer.declarations;
        if (forVarDeclarations.length !== 1) {
          return false;
        }
        const forVarDecl = forVarDeclarations[0];

        // our goal to skip declarations in for of/in initializer
        if (forVarDecl !== decl && decl.initializer === undefined) {
          return false;
        }
      } else if (decl.initializer === undefined) {
        return false;
      }

      // analize that var uses are only in function block
      for (const ident of identifiers) {
        if (ident !== decl && !Autofixer.varHasScope(ident, forNode)) {
          return false;
        }
      }

      // analize that var is not in function
      for (const ident of identifiers) {
        if (ident !== decl && Autofixer.varInFunctionForScope(ident, forNode)) {
          return false;
        }
      }
    }
    return true;
  }
  checkVarDeclarations(varDeclList) {
    for (const decl of varDeclList.declarations) {
      const symbol = this.typeChecker.getSymbolAtLocation(decl.name);
      if (!symbol) {
        return false;
      }
      const identifiers = this.symbolCache.getReferences(symbol);
      const declLength = symbol.declarations?.length;
      if (!declLength || declLength >= 2) {
        return false;
      }

      // Check for var use in tdz oe self declaration
      if (!Autofixer.analizeTDZ(decl, identifiers)) {
        return false;
      }

      // Has use outside scope of declaration?
      if (!Autofixer.analizeScope(decl, identifiers)) {
        return false;
      }

      // For analisys
      if (!Autofixer.analizeFor(decl, identifiers)) {
        return false;
      }
      if (symbol.getName() === 'let') {
        return false;
      }
    }
    return true;
  }
  canAutofixNoVar(varDeclList) {
    if (!Autofixer.parentInCaseOrWhile(varDeclList)) {
      return false;
    }
    if (!this.checkVarDeclarations(varDeclList)) {
      return false;
    }
    return true;
  }
  fixVarDeclaration(node) {
    const newNode = ts.factory.createVariableDeclarationList(node.declarations, ts.NodeFlags.Let);
    const text = this.printer.printNode(ts.EmitHint.Unspecified, newNode, node.getSourceFile());
    return this.canAutofixNoVar(node) ? [{
      start: node.getStart(),
      end: node.getEnd(),
      replacementText: text
    }] : undefined;
  }
  getFixReturnTypeArrowFunction(funcLikeDecl, typeNode) {
    if (!funcLikeDecl.body) {
      return '';
    }
    const node = ts.factory.createArrowFunction(undefined, funcLikeDecl.typeParameters, funcLikeDecl.parameters, typeNode, ts.factory.createToken(ts.SyntaxKind.EqualsGreaterThanToken), funcLikeDecl.body);
    return this.printer.printNode(ts.EmitHint.Unspecified, node, funcLikeDecl.getSourceFile());
  }
  fixMissingReturnType(funcLikeDecl, typeNode) {
    if (ts.isArrowFunction(funcLikeDecl)) {
      const text = this.getFixReturnTypeArrowFunction(funcLikeDecl, typeNode);
      const startPos = funcLikeDecl.getStart();
      const endPos = funcLikeDecl.getEnd();
      return [{
        start: startPos,
        end: endPos,
        replacementText: text
      }];
    }
    const text = ': ' + this.printer.printNode(ts.EmitHint.Unspecified, typeNode, funcLikeDecl.getSourceFile());
    const pos = Autofixer.getReturnTypePosition(funcLikeDecl);
    return [{
      start: pos,
      end: pos,
      replacementText: text
    }];
  }
  dropTypeOnVarDecl(varDecl) {
    const newVarDecl = ts.factory.createVariableDeclaration(varDecl.name, undefined, undefined, undefined);
    const text = this.printer.printNode(ts.EmitHint.Unspecified, newVarDecl, varDecl.getSourceFile());
    return [{
      start: varDecl.getStart(),
      end: varDecl.getEnd(),
      replacementText: text
    }];
  }
  fixTypeAssertion(typeAssertion) {
    const asExpr = ts.factory.createAsExpression(typeAssertion.expression, typeAssertion.type);
    const text = this.nonCommentPrinter.printNode(ts.EmitHint.Unspecified, asExpr, typeAssertion.getSourceFile());
    return [{
      start: typeAssertion.getStart(),
      end: typeAssertion.getEnd(),
      replacementText: text
    }];
  }
  fixCommaOperator(tsNode) {
    const tsExprNode = tsNode;
    const text = this.recursiveCommaOperator(tsExprNode);
    return [{
      start: tsExprNode.parent.getFullStart(),
      end: tsExprNode.parent.getEnd(),
      replacementText: text
    }];
  }
  recursiveCommaOperator(tsExprNode) {
    let text = '';
    if (tsExprNode.operatorToken.kind !== ts.SyntaxKind.CommaToken) {
      return tsExprNode.getFullText() + ';';
    }
    if (tsExprNode.left.kind === ts.SyntaxKind.BinaryExpression) {
      text += this.recursiveCommaOperator(tsExprNode.left);
      text += '\n' + tsExprNode.right.getFullText() + ';';
    } else {
      const leftText = tsExprNode.left.getFullText();
      const rightText = tsExprNode.right.getFullText();
      text = leftText + ';\n' + rightText + ';';
    }
    return text;
  }
  fixEnumMerging(enumSymbol, enumDeclsInFile) {
    var _this2 = this;
    if (this.enumMergingCache.has(enumSymbol)) {
      return this.enumMergingCache.get(enumSymbol);
    }
    if (enumDeclsInFile.length <= 1) {
      this.enumMergingCache.set(enumSymbol, undefined);
      return undefined;
    }
    let result = [];
    this.symbolCache.getReferences(enumSymbol).forEach(function (node) {
      _newArrowCheck(this, _this2);
      if (result === undefined || !ts.isEnumDeclaration(node)) {
        return;
      }
      if (result.length) {
        result.push({
          start: node.getStart(),
          end: node.getEnd(),
          replacementText: ''
        });
        return;
      }
      const members = [];
      for (const decl of enumDeclsInFile) {
        for (const member of decl.members) {
          if (member.initializer && member.initializer.kind !== ts.SyntaxKind.NumericLiteral && member.initializer.kind !== ts.SyntaxKind.StringLiteral) {
            result = undefined;
            return;
          }
        }
        members.push(...decl.members);
      }
      const fullEnum = ts.factory.createEnumDeclaration(node.modifiers, node.name, members);
      const fullText = this.printer.printNode(ts.EmitHint.Unspecified, fullEnum, node.getSourceFile());
      result.push({
        start: node.getStart(),
        end: node.getEnd(),
        replacementText: fullText
      });
    }.bind(this));
    if (!result?.length) {
      result = undefined;
    }
    this.enumMergingCache.set(enumSymbol, result);
    return result;
  }
  static getReturnTypePosition(funcLikeDecl) {
    if (funcLikeDecl.body) {
      /*
       * Find position of the first node or token that follows parameters.
       * After that, iterate over child nodes in reverse order, until found
       * first closing parenthesis.
       */
      const postParametersPosition = ts.isArrowFunction(funcLikeDecl) ? funcLikeDecl.equalsGreaterThanToken.getStart() : funcLikeDecl.body.getStart();
      const children = funcLikeDecl.getChildren();
      for (let i = children.length - 1; i >= 0; i--) {
        const child = children[i];
        if (child.kind === ts.SyntaxKind.CloseParenToken && child.getEnd() <= postParametersPosition) {
          return child.getEnd();
        }
      }
    }

    // Shouldn't get here.
    return -1;
  }
  static needsParentheses(node) {
    const parent = node.parent;
    return ts.isPrefixUnaryExpression(parent) || ts.isPostfixUnaryExpression(parent) || ts.isPropertyAccessExpression(parent) || ts.isElementAccessExpression(parent) || ts.isTypeOfExpression(parent) || ts.isVoidExpression(parent) || ts.isAwaitExpression(parent) || ts.isCallExpression(parent) && node === parent.expression || ts.isBinaryExpression(parent) && !(0, _isAssignmentOperator.isAssignmentOperator)(parent.operatorToken);
  }
  fixCtorParameterProperties(ctorDecl, paramTypes) {
    if (paramTypes === undefined) {
      return undefined;
    }
    const fieldInitStmts = [];
    const newFieldPos = ctorDecl.getStart();
    const autofixes = [{
      start: newFieldPos,
      end: newFieldPos,
      replacementText: ''
    }];
    for (let i = 0; i < ctorDecl.parameters.length; i++) {
      this.fixCtorParameterPropertiesProcessParam(ctorDecl.parameters[i], paramTypes[i], ctorDecl.getSourceFile(), fieldInitStmts, autofixes);
    }

    // Note: Bodyless ctors can't have parameter properties.
    if (ctorDecl.body) {
      const newBody = ts.factory.createBlock(fieldInitStmts.concat(ctorDecl.body.statements), true);
      const newBodyText = this.printer.printNode(ts.EmitHint.Unspecified, newBody, ctorDecl.getSourceFile());
      autofixes.push({
        start: ctorDecl.body.getStart(),
        end: ctorDecl.body.getEnd(),
        replacementText: newBodyText
      });
    }
    return autofixes;
  }
  fixCtorParameterPropertiesProcessParam(param, paramType, sourceFile, fieldInitStmts, autofixes) {
    // Parameter property can not be a destructuring parameter.
    if (!ts.isIdentifier(param.name)) {
      return;
    }
    if (this.utils.hasAccessModifier(param)) {
      const propIdent = ts.factory.createIdentifier(param.name.text);
      const newFieldNode = ts.factory.createPropertyDeclaration(ts.getModifiers(param), propIdent, undefined, paramType, undefined);
      const newFieldText = this.printer.printNode(ts.EmitHint.Unspecified, newFieldNode, sourceFile) + '\n';
      autofixes[0].replacementText += newFieldText;
      const newParamDecl = ts.factory.createParameterDeclaration(undefined, undefined, param.name, param.questionToken, param.type, param.initializer);
      const newParamText = this.printer.printNode(ts.EmitHint.Unspecified, newParamDecl, sourceFile);
      autofixes.push({
        start: param.getStart(),
        end: param.getEnd(),
        replacementText: newParamText
      });
      fieldInitStmts.push(ts.factory.createExpressionStatement(ts.factory.createAssignment(ts.factory.createPropertyAccessExpression(ts.factory.createThis(), propIdent), propIdent)));
    }
  }
  fixPrivateIdentifier(node) {
    var _this3 = this;
    const classMember = this.typeChecker.getSymbolAtLocation(node);
    if (!classMember || (classMember.getFlags() & ts.SymbolFlags.ClassMember) === 0 || !classMember.valueDeclaration) {
      return undefined;
    }
    if (this.privateIdentifierCache.has(classMember)) {
      return this.privateIdentifierCache.get(classMember);
    }
    const memberDecl = classMember.valueDeclaration;
    const parentDecl = memberDecl.parent;
    if (!ts.isClassLike(parentDecl) || this.utils.classMemberHasDuplicateName(memberDecl, parentDecl, true)) {
      this.privateIdentifierCache.set(classMember, undefined);
      return undefined;
    }
    let result = [];
    this.symbolCache.getReferences(classMember).forEach(function (ident) {
      _newArrowCheck(this, _this3);
      if (ts.isPrivateIdentifier(ident)) {
        result.push(this.fixSinglePrivateIdentifier(ident));
      }
    }.bind(this));
    if (!result.length) {
      result = undefined;
    }
    this.privateIdentifierCache.set(classMember, result);
    return result;
  }
  isFunctionDeclarationFirst(tsFunctionDeclaration) {
    var _this4 = this;
    if (tsFunctionDeclaration.name === undefined) {
      return false;
    }
    const symbol = this.typeChecker.getSymbolAtLocation(tsFunctionDeclaration.name);
    if (symbol === undefined) {
      return false;
    }
    let minPos = tsFunctionDeclaration.pos;
    this.symbolCache.getReferences(symbol).forEach(function (ident) {
      _newArrowCheck(this, _this4);
      if (ident.pos < minPos) {
        minPos = ident.pos;
      }
    }.bind(this));
    return minPos >= tsFunctionDeclaration.pos;
  }
  fixNestedFunction(tsFunctionDeclaration) {
    const isGenerator = tsFunctionDeclaration.asteriskToken !== undefined;
    const hasThisKeyword = tsFunctionDeclaration.body === undefined ? false : (0, _ContainsThis.scopeContainsThis)(tsFunctionDeclaration.body);
    const canBeFixed = !isGenerator && !hasThisKeyword;
    if (!canBeFixed) {
      return undefined;
    }
    const name = tsFunctionDeclaration.name?.escapedText;
    const type = tsFunctionDeclaration.type;
    const body = tsFunctionDeclaration.body;
    if (!name || !type || !body) {
      return undefined;
    }

    // Check only illegal decorators, cause all decorators for function declaration are illegal
    if (ts.getIllegalDecorators(tsFunctionDeclaration)) {
      return undefined;
    }
    if (!this.isFunctionDeclarationFirst(tsFunctionDeclaration)) {
      return undefined;
    }
    const typeParameters = tsFunctionDeclaration.typeParameters;
    const parameters = tsFunctionDeclaration.parameters;
    const modifiers = ts.getModifiers(tsFunctionDeclaration);
    const token = ts.factory.createToken(ts.SyntaxKind.EqualsGreaterThanToken);
    const typeDecl = ts.factory.createFunctionTypeNode(typeParameters, parameters, type);
    const arrowFunc = ts.factory.createArrowFunction(modifiers, typeParameters, parameters, type, token, body);
    const declaration = ts.factory.createVariableDeclaration(name, undefined, typeDecl, arrowFunc);
    const list = ts.factory.createVariableDeclarationList([declaration], ts.NodeFlags.Let);
    const statement = ts.factory.createVariableStatement(modifiers, list);
    const text = this.printer.printNode(ts.EmitHint.Unspecified, statement, tsFunctionDeclaration.getSourceFile());
    return [{
      start: tsFunctionDeclaration.getStart(),
      end: tsFunctionDeclaration.getEnd(),
      replacementText: text
    }];
  }
  fixMultipleStaticBlocks(nodes) {
    const autofix = [];
    let body = nodes[0].body;
    let bodyStatements = [];
    bodyStatements = bodyStatements.concat(body.statements);
    for (let i = 1; i < nodes.length; i++) {
      bodyStatements = bodyStatements.concat(nodes[i].body.statements);
      autofix[i] = {
        start: nodes[i].getFullStart(),
        end: nodes[i].getEnd(),
        replacementText: ''
      };
    }
    body = ts.factory.createBlock(bodyStatements, true);
    // static blocks shouldn't have modifiers
    const statickBlock = ts.factory.createClassStaticBlockDeclaration(body);
    const text = this.printer.printNode(ts.EmitHint.Unspecified, statickBlock, nodes[0].getSourceFile());
    autofix[0] = {
      start: nodes[0].getStart(),
      end: nodes[0].getEnd(),
      replacementText: text
    };
    return autofix;
  }
  fixSinglePrivateIdentifier(ident) {
    if (ts.isPropertyDeclaration(ident.parent) || ts.isMethodDeclaration(ident.parent) || ts.isGetAccessorDeclaration(ident.parent) || ts.isSetAccessorDeclaration(ident.parent)) {
      // Note: 'private' modifier should always be first.
      const mods = ts.getModifiers(ident.parent);
      const newMods = [ts.factory.createModifier(ts.SyntaxKind.PrivateKeyword)];
      if (mods) {
        for (const mod of mods) {
          newMods.push(ts.factory.createModifier(mod.kind));
        }
      }
      const newName = ident.text.slice(1, ident.text.length);
      const newDecl = Autofixer.replacePrivateIdentInDeclarationName(newMods, newName, ident.parent);
      const text = this.printer.printNode(ts.EmitHint.Unspecified, newDecl, ident.getSourceFile());
      return {
        start: ident.parent.getStart(),
        end: ident.parent.getEnd(),
        replacementText: text
      };
    }
    return {
      start: ident.getStart(),
      end: ident.getEnd(),
      replacementText: ident.text.slice(1, ident.text.length)
    };
  }
  static replacePrivateIdentInDeclarationName(mods, name, oldDecl) {
    if (ts.isPropertyDeclaration(oldDecl)) {
      return ts.factory.createPropertyDeclaration(mods, ts.factory.createIdentifier(name), oldDecl.questionToken ?? oldDecl.exclamationToken, oldDecl.type, oldDecl.initializer);
    } else if (ts.isMethodDeclaration(oldDecl)) {
      return ts.factory.createMethodDeclaration(mods, oldDecl.asteriskToken, ts.factory.createIdentifier(name), oldDecl.questionToken, oldDecl.typeParameters, oldDecl.parameters, oldDecl.type, oldDecl.body);
    } else if (ts.isGetAccessorDeclaration(oldDecl)) {
      return ts.factory.createGetAccessorDeclaration(mods, ts.factory.createIdentifier(name), oldDecl.parameters, oldDecl.type, oldDecl.body);
    }
    return ts.factory.createSetAccessorDeclaration(mods, ts.factory.createIdentifier(name), oldDecl.parameters, oldDecl.body);
  }
  fixUntypedObjectLiteral(objectLiteralExpr, objectLiteralType) {
    // Can't fix if object literal already has contextual type.
    if (objectLiteralType) {
      return undefined;
    }
    const enclosingStmt = _TsUtils.TsUtils.getEnclosingTopLevelStatement(objectLiteralExpr);
    if (!enclosingStmt) {
      return undefined;
    }
    const newInterfaceProps = this.getInterfacePropertiesFromObjectLiteral(objectLiteralExpr, enclosingStmt);
    if (!newInterfaceProps) {
      return undefined;
    }
    const srcFile = objectLiteralExpr.getSourceFile();
    const newInterfaceName = _TsUtils.TsUtils.generateUniqueName(this.objectLiteralInterfaceNameGenerator, srcFile);
    if (!newInterfaceName) {
      return undefined;
    }
    return [this.createNewInterface(srcFile, newInterfaceName, newInterfaceProps, enclosingStmt.getStart()), this.fixObjectLiteralExpression(srcFile, newInterfaceName, objectLiteralExpr)];
  }
  getInterfacePropertiesFromObjectLiteral(objectLiteralExpr, enclosingStmt) {
    const interfaceProps = [];
    for (const prop of objectLiteralExpr.properties) {
      const interfaceProp = this.getInterfacePropertyFromObjectLiteralElement(prop, enclosingStmt);
      if (!interfaceProp) {
        return undefined;
      }
      interfaceProps.push(interfaceProp);
    }
    return interfaceProps;
  }
  getInterfacePropertyFromObjectLiteralElement(prop, enclosingStmt) {
    // Can't fix if property is not a key-value pair, or the property name is a computed value.
    if (!ts.isPropertyAssignment(prop) || ts.isComputedPropertyName(prop.name)) {
      return undefined;
    }
    const propType = this.typeChecker.getTypeAtLocation(prop);

    // Can't capture generic type parameters of enclosing declarations.
    if (this.utils.hasGenericTypeParameter(propType)) {
      return undefined;
    }
    if (Autofixer.propertyTypeIsCapturedFromEnclosingLocalScope(propType, enclosingStmt)) {
      return undefined;
    }
    const propTypeNode = this.typeChecker.typeToTypeNode(propType, undefined, ts.NodeBuilderFlags.None);
    if (!propTypeNode || !this.utils.isSupportedType(propTypeNode)) {
      return undefined;
    }
    const newProp = ts.factory.createPropertySignature(undefined, prop.name, undefined, propTypeNode);
    return newProp;
  }
  static propertyTypeIsCapturedFromEnclosingLocalScope(type, enclosingStmt) {
    const sym = type.getSymbol();
    let symNode = _TsUtils.TsUtils.getDeclaration(sym);
    while (symNode) {
      if (symNode === enclosingStmt) {
        return true;
      }
      symNode = symNode.parent;
    }
    return false;
  }
  createNewInterface(srcFile, interfaceName, members, pos) {
    const newInterfaceDecl = ts.factory.createInterfaceDeclaration(undefined, interfaceName, undefined, undefined, members);
    const text = this.printer.printNode(ts.EmitHint.Unspecified, newInterfaceDecl, srcFile) + '\n';
    return {
      start: pos,
      end: pos,
      replacementText: text
    };
  }
  fixObjectLiteralExpression(srcFile, newInterfaceName, objectLiteralExpr) {
    /*
     * If object literal is initializing a variable or property,
     * then simply add new 'contextual' type to the declaration.
     * Otherwise, cast object literal to newly created interface type.
     */
    if ((ts.isVariableDeclaration(objectLiteralExpr.parent) || ts.isPropertyDeclaration(objectLiteralExpr.parent) || ts.isParameter(objectLiteralExpr.parent)) && !objectLiteralExpr.parent.type) {
      const text = ': ' + newInterfaceName;
      const pos = Autofixer.getDeclarationTypePositionForObjectLiteral(objectLiteralExpr.parent);
      return {
        start: pos,
        end: pos,
        replacementText: text
      };
    }
    const newTypeRef = ts.factory.createTypeReferenceNode(newInterfaceName);
    let newExpr = ts.factory.createAsExpression(ts.factory.createObjectLiteralExpression(objectLiteralExpr.properties), newTypeRef);
    if (!ts.isParenthesizedExpression(objectLiteralExpr.parent)) {
      newExpr = ts.factory.createParenthesizedExpression(newExpr);
    }
    const text = this.printer.printNode(ts.EmitHint.Unspecified, newExpr, srcFile);
    return {
      start: objectLiteralExpr.getStart(),
      end: objectLiteralExpr.getEnd(),
      replacementText: text
    };
  }
  static getDeclarationTypePositionForObjectLiteral(decl) {
    if (ts.isPropertyDeclaration(decl)) {
      return (decl.questionToken || decl.exclamationToken || decl.name).getEnd();
    } else if (ts.isParameter(decl)) {
      return (decl.questionToken || decl.name).getEnd();
    }
    return (decl.exclamationToken || decl.name).getEnd();
  }
  /*
   * In case of type alias initialized with type literal, replace
   * entire type alias with identical interface declaration.
   */
  proceedTypeAliasDeclaration(typeLiteral) {
    if (ts.isTypeAliasDeclaration(typeLiteral.parent)) {
      const typeAlias = typeLiteral.parent;
      const newInterfaceDecl = ts.factory.createInterfaceDeclaration(typeAlias.modifiers, typeAlias.name, typeAlias.typeParameters, undefined, typeLiteral.members);
      const text = this.printer.printNode(ts.EmitHint.Unspecified, newInterfaceDecl, typeLiteral.getSourceFile());
      return [{
        start: typeAlias.getStart(),
        end: typeAlias.getEnd(),
        replacementText: text
      }];
    }
    return undefined;
  }
  fixTypeliteral(typeLiteral) {
    const typeAliasAutofix = this.proceedTypeAliasDeclaration(typeLiteral);
    if (typeAliasAutofix) {
      return typeAliasAutofix;
    }

    /*
     * Create new interface declaration with members of type literal
     * and put the interface name in place of the type literal.
     */
    const srcFile = typeLiteral.getSourceFile();
    const enclosingStmt = _TsUtils.TsUtils.getEnclosingTopLevelStatement(typeLiteral);
    if (!enclosingStmt) {
      return undefined;
    }
    if (this.typeLiteralCapturesTypeFromEnclosingLocalScope(typeLiteral, enclosingStmt)) {
      return undefined;
    }
    const newInterfaceName = _TsUtils.TsUtils.generateUniqueName(this.typeLiteralInterfaceNameGenerator, srcFile);
    if (!newInterfaceName) {
      return undefined;
    }
    const newInterfacePos = enclosingStmt.getStart();
    const newInterfaceDecl = ts.factory.createInterfaceDeclaration(undefined, newInterfaceName, undefined, undefined, typeLiteral.members);
    const interfaceText = this.printer.printNode(ts.EmitHint.Unspecified, newInterfaceDecl, srcFile) + '\n';
    return [{
      start: newInterfacePos,
      end: newInterfacePos,
      replacementText: interfaceText
    }, {
      start: typeLiteral.getStart(),
      end: typeLiteral.getEnd(),
      replacementText: newInterfaceName
    }];
  }
  typeLiteralCapturesTypeFromEnclosingLocalScope(typeLiteral, enclosingStmt) {
    var _this5 = this;
    let found = false;
    const callback = function callback(node) {
      _newArrowCheck(this, _this5);
      if (ts.isIdentifier(node)) {
        const sym = this.typeChecker.getSymbolAtLocation(node);
        let symNode = _TsUtils.TsUtils.getDeclaration(sym);
        while (symNode) {
          if (symNode === typeLiteral) {
            return;
          }
          if (symNode === enclosingStmt) {
            found = true;
            return;
          }
          symNode = symNode.parent;
        }
      }
    }.bind(this);
    const stopCondition = function stopCondition(node) {
      _newArrowCheck(this, _this5);
      void node;
      return found;
    }.bind(this);
    (0, _ForEachNodeInSubtree.forEachNodeInSubtree)(typeLiteral, callback, stopCondition);
    return found;
  }
}
exports.Autofixer = Autofixer;
//# sourceMappingURL=Autofixer.js.map