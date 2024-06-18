/*
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

import * as npath from 'node:path';
import * as path from 'node:path';
import * as ts from 'typescript';
import { LinterConfig } from '../TypeScriptLinterConfig';
import { STANDARD_LIBRARIES } from './Consts';
import type { TsUtils } from './TsUtils';

function scopeContainsThisVisitor(tsNode: ts.Node): boolean {
  if (tsNode.kind === ts.SyntaxKind.ThisKeyword) {
    return true;
  }

  /*
   * Visit children nodes. Skip any local declaration that defines
   * its own scope as it needs to be checked separately.
   */
  const isClassLike = ts.isClassDeclaration(tsNode) || ts.isClassExpression(tsNode);
  const isFunctionLike = ts.isFunctionDeclaration(tsNode) || ts.isFunctionExpression(tsNode);
  const isModuleDecl = ts.isModuleDeclaration(tsNode);
  if (isClassLike || isFunctionLike || isModuleDecl) {
    return false;
  }

  for (const child of tsNode.getChildren()) {
    if (scopeContainsThisVisitor(child)) {
      return true;
    }
  }

  return false;
}

export function scopeContainsThis(tsNode: ts.Expression | ts.Block): boolean {
  return scopeContainsThisVisitor(tsNode);
}

export interface DiagnosticChecker {
  checkDiagnosticMessage: (msgText: string | ts.DiagnosticMessageChain) => boolean;
}

export function forEachNodeInSubtree(
  node: ts.Node,
  cb: (n: ts.Node) => void,
  stopCond?: (n: ts.Node) => boolean
): void {
  cb(node);
  if (stopCond?.(node)) {
    return;
  }

  ts.forEachChild(node, (child) => {
    forEachNodeInSubtree(child, cb, stopCond);
  });
}

export function hasPredecessor(node: ts.Node, predicate: (node: ts.Node) => boolean): boolean {
  let parent = node.parent;
  while (parent !== undefined) {
    if (predicate(parent)) {
      return true;
    }
    parent = parent.parent;
  }
  return false;
}

function isInstanceofContext(tsIdentStart: ts.Node): boolean {
  return (
    ts.isBinaryExpression(tsIdentStart.parent) &&
    tsIdentStart.parent.operatorToken.kind === ts.SyntaxKind.InstanceOfKeyword
  );
}

function isNewExpressionContext(tsIdentStart: ts.Node): boolean {
  return ts.isNewExpression(tsIdentStart.parent) && tsIdentStart === tsIdentStart.parent.expression;
}

/*
 * If identifier is the right-most name of Property Access chain or Qualified name,
 * or it's a separate identifier expression, then identifier is being referenced as an value.
 */
function isQualifiedNameContext(tsIdentStart: ts.Node, tsIdentifier: ts.Identifier): boolean {
  // rightmost in AST is rightmost in qualified name chain
  return ts.isQualifiedName(tsIdentStart) && tsIdentifier !== tsIdentStart.right;
}

function isPropertyAccessContext(tsIdentStart: ts.Node, tsIdentifier: ts.Identifier): boolean {
  // rightmost in AST is rightmost in qualified name chain
  return ts.isPropertyAccessExpression(tsIdentStart) && tsIdentifier !== tsIdentStart.name;
}

function getQualifiedStart(ident: ts.Node): ts.Node {
  let qualifiedStart: ts.Node = ident;
  while (ts.isPropertyAccessExpression(qualifiedStart.parent) || ts.isQualifiedName(qualifiedStart.parent)) {
    qualifiedStart = qualifiedStart.parent;
  }
  return qualifiedStart;
}

function isEnumPropAccess(ident: ts.Identifier, tsSym: ts.Symbol, context: ts.Node): boolean {
  return (
    ts.isElementAccessExpression(context) &&
    !!(tsSym.flags & ts.SymbolFlags.Enum) &&
    (context.expression === ident ||
      ts.isPropertyAccessExpression(context.expression) && context.expression.name === ident)
  );
}

function isValidParent(parent: ts.Node): boolean {
  // treat TypeQuery as valid because it's already forbidden (FaultID.TypeQuery)
  return (
    ts.isTypeNode(parent) && !ts.isTypeOfExpression(parent) ||
    ts.isExpressionWithTypeArguments(parent) ||
    ts.isExportAssignment(parent) ||
    ts.isExportSpecifier(parent) ||
    ts.isMetaProperty(parent) ||
    ts.isImportClause(parent) ||
    ts.isClassLike(parent) ||
    ts.isInterfaceDeclaration(parent) ||
    ts.isModuleDeclaration(parent) ||
    ts.isEnumDeclaration(parent) ||
    ts.isNamespaceImport(parent) ||
    ts.isImportSpecifier(parent) ||
    ts.isImportEqualsDeclaration(parent)
  );
}

export function identiferUseInValueContext(ident: ts.Identifier, tsSym: ts.Symbol): boolean {
  const qualifiedStart = getQualifiedStart(ident);
  const parent = qualifiedStart.parent;
  const isValidUse =
    isValidParent(parent) ||
    isEnumPropAccess(ident, tsSym, parent) ||
    isQualifiedNameContext(qualifiedStart, ident) ||
    isPropertyAccessContext(qualifiedStart, ident) ||
    isNewExpressionContext(qualifiedStart) ||
    isInstanceofContext(qualifiedStart);
  return !isValidUse;
}

export function isAssignmentOperator(tsBinOp: ts.BinaryOperatorToken): boolean {
  return tsBinOp.kind >= ts.SyntaxKind.FirstAssignment && tsBinOp.kind <= ts.SyntaxKind.LastAssignment;
}

export function isIntrinsicObjectType(type: ts.Type): boolean {
  return !!(type.flags & ts.TypeFlags.NonPrimitive);
}

export function isStdLibraryType(type: ts.Type): boolean {
  return isStdLibrarySymbol(type.aliasSymbol ?? type.getSymbol());
}

export function isStdLibrarySymbol(sym: ts.Symbol | undefined): boolean {
  if (sym?.declarations && sym.declarations.length > 0) {
    const srcFile = sym.declarations[0].getSourceFile();
    return srcFile && STANDARD_LIBRARIES.includes(path.basename(srcFile.fileName).toLowerCase());
  }
  return false;
}

export function isStruct(symbol: ts.Symbol): boolean {
  if (!symbol.declarations) {
    return false;
  }
  for (const decl of symbol.declarations) {
    if (isStructDeclaration(decl)) {
      return true;
    }
  }
  return false;
}

export function isStructDeclarationKind(kind: ts.SyntaxKind): boolean {
  return LinterConfig.tsSyntaxKindNames[kind] === 'StructDeclaration';
}

export function isStructDeclaration(node: ts.Node): boolean {
  return isStructDeclarationKind(node.kind);
}

/*
 * Current approach relates on error code and error message matching and it is quite fragile,
 * so this place should be checked thoroughly in the case of typescript upgrade
 */
export const TYPE_0_IS_NOT_ASSIGNABLE_TO_TYPE_1_ERROR_CODE = 2322;
export const TYPE_UNKNOWN_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE =
  /^Type '(.*)\bunknown\b(.*)' is not assignable to type '.*'\.$/;
export const TYPE_NULL_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE = /^Type '(.*)\bnull\b(.*)' is not assignable to type '.*'\.$/;
export const TYPE_UNDEFINED_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE =
  /^Type '(.*)\bundefined\b(.*)' is not assignable to type '.*'\.$/;

export const ARGUMENT_OF_TYPE_0_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_ERROR_CODE = 2345;
export const OBJECT_IS_POSSIBLY_UNDEFINED_ERROR_CODE = 2532;
export const NO_OVERLOAD_MATCHES_THIS_CALL_ERROR_CODE = 2769;
export const ARGUMENT_OF_TYPE_NULL_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_RE =
  /^Argument of type '(.*)\bnull\b(.*)' is not assignable to parameter of type '.*'\.$/;
export const ARGUMENT_OF_TYPE_UNDEFINED_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_RE =
  /^Argument of type '(.*)\bundefined\b(.*)' is not assignable to parameter of type '.*'\.$/;

export class LibraryTypeCallDiagnosticChecker implements DiagnosticChecker {
  inLibCall: boolean = false;
  diagnosticMessages: Array<ts.DiagnosticMessageChain> | undefined;
  filteredDiagnosticMessages: Set<ts.DiagnosticMessageChain>;

  constructor(filteredDiagnosticMessages: Set<ts.DiagnosticMessageChain>) {
    this.filteredDiagnosticMessages = filteredDiagnosticMessages;
  }

  configure(inLibCall: boolean, diagnosticMessages: Array<ts.DiagnosticMessageChain>): void {
    this.inLibCall = inLibCall;
    this.diagnosticMessages = diagnosticMessages;
  }

  checkMessageText(msg: string): boolean {
    if (this.inLibCall) {
      const match =
        msg.match(ARGUMENT_OF_TYPE_NULL_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_RE) ||
        msg.match(ARGUMENT_OF_TYPE_UNDEFINED_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_RE) ||
        msg.match(TYPE_UNDEFINED_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE) ||
        msg.match(TYPE_NULL_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE);
      return !match;
    }
    return true;
  }

  checkMessageChain(chain: ts.DiagnosticMessageChain): boolean {
    if (chain.code === TYPE_0_IS_NOT_ASSIGNABLE_TO_TYPE_1_ERROR_CODE) {
      if (chain.messageText.match(TYPE_UNKNOWN_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE)) {
        return false;
      }
      if (this.inLibCall && chain.messageText.match(TYPE_UNDEFINED_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE)) {
        return false;
      }
      if (this.inLibCall && chain.messageText.match(TYPE_NULL_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE)) {
        return false;
      }
    }
    if (chain.code === ARGUMENT_OF_TYPE_0_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_ERROR_CODE) {
      if (
        this.inLibCall &&
        chain.messageText.match(ARGUMENT_OF_TYPE_UNDEFINED_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_RE)
      ) {
        return false;
      }
      if (
        this.inLibCall &&
        chain.messageText.match(ARGUMENT_OF_TYPE_NULL_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_RE)
      ) {
        return false;
      }
    }
    return chain.next === undefined ? true : this.checkMessageChain(chain.next[0]);
  }

  checkFilteredDiagnosticMessages(msgText: ts.DiagnosticMessageChain | string): boolean {
    if (this.filteredDiagnosticMessages.size === 0) {
      return true;
    }

    if (typeof msgText !== 'string' && this.filteredDiagnosticMessages.has(msgText)) {
      return false;
    }

    for (const msgChain of this.filteredDiagnosticMessages) {
      if (typeof msgText == 'string') {
        if (msgText === msgChain.messageText) {
          return false;
        }
        continue;
      }

      let curMsg: ts.DiagnosticMessageChain | undefined = msgText;
      let curFilteredMsg: ts.DiagnosticMessageChain | undefined = msgChain;
      while (curMsg) {
        if (!curFilteredMsg) {
          return true;
        }

        if (curMsg.code !== curFilteredMsg.code) {
          return true;
        }

        if (curMsg.messageText !== curFilteredMsg.messageText) {
          return true;
        }

        curMsg = curMsg.next ? curMsg.next[0] : undefined;
        curFilteredMsg = curFilteredMsg.next ? curFilteredMsg.next[0] : undefined;
      }

      return false;
    }
    return true;
  }

  checkDiagnosticMessage(msgText: string | ts.DiagnosticMessageChain): boolean {
    if (!this.diagnosticMessages) {
      return false;
    }

    if (this.inLibCall && !this.checkFilteredDiagnosticMessages(msgText)) {
      return false;
    }

    if (typeof msgText == 'string') {
      return this.checkMessageText(msgText);
    }

    if (!this.checkMessageChain(msgText)) {
      this.diagnosticMessages.push(msgText);
      return false;
    }
    return true;
  }
}

export function logTscDiagnostic(diagnostics: readonly ts.Diagnostic[], log: (message: string) => void): void {
  diagnostics.forEach((diagnostic) => {
    let message = ts.flattenDiagnosticMessageText(diagnostic.messageText, '\n');

    if (diagnostic.file && diagnostic.start) {
      const { line, character } = ts.getLineAndCharacterOfPosition(diagnostic.file, diagnostic.start);
      message = `${diagnostic.file.fileName} (${line + 1}, ${character + 1}): ${message}`;
    }

    log(message);
  });
}

export function mergeArrayMaps<K, V>(lhs: Map<K, V[]>, rhs: Map<K, V[]>): Map<K, V[]> {
  if (lhs.size === 0) {
    return rhs;
  }
  if (rhs.size === 0) {
    return lhs;
  }

  rhs.forEach((values, key) => {
    if (values.length !== 0) {
      if (lhs.has(key)) {
        lhs.get(key)!.push(...values);
      } else {
        lhs.set(key, values);
      }
    }
  });
  return lhs;
}

export class NameGenerator {
  private counter = 0;

  constructor(private readonly prefix: string, private readonly treshold: number) { }

  getName(): string | undefined {
    if (this.counter >= this.treshold) {
      return undefined;
    }
    return this.prefix + ++this.counter;
  }
}

export function pathContainsDirectory(path: string, dir: string): boolean {
  for (const pathDir of npath.dirname(path).split(npath.sep)) {
    if (pathDir === dir) {
      return true;
    }
  }
  return false;
}

type CheckStdCallApi = (callExpr: ts.CallExpression) => boolean;
type StdCallApiEntry = Map<string, CheckStdCallApi>;

export class SupportedStdCallApiChecker {
  tsUtils: TsUtils;
  typeChecker: ts.TypeChecker;

  constructor(tsUtils: TsUtils, typeChecker: ts.TypeChecker) {
    this.tsUtils = tsUtils;
    this.typeChecker = typeChecker;
  }

  stdObjectEntry = new Map<string, CheckStdCallApi>([
    ['assign', this.checkObjectAssignCall]
  ]);

  StdCallApi = new Map<string | undefined, StdCallApiEntry>([
    ['Object', this.stdObjectEntry],
    ['ObjectConstructor', this.stdObjectEntry]
  ]);

  isSupportedStdCallAPI(callExpr: ts.CallExpression, parentSymName: string | undefined, symName: string): boolean {
    const entry = this.StdCallApi.get(parentSymName);
    if (entry) {
      const stdCallApiCheckCb = entry.get(symName);
      return !!stdCallApiCheckCb && stdCallApiCheckCb.call(this, callExpr);
    }

    return false;
  }

  private checkObjectAssignCall(callExpr: ts.CallExpression): boolean {

    /*
     * 'Object.assign' is allowed only with signature like following:
     *    assign(target: Record<string, V>, ...source: Object[]>): Record<String, V>
     *
     * Note: For 'return' type, check the contextual type of call expression, as the
     * return type of actual call signature will be deduced as an intersection of all
     * types of the 'target' and 'source' arguments.
     */

    if (callExpr.typeArguments || callExpr.arguments.length === 0) {
      return false;
    }
    const targetArgType = this.typeChecker.getTypeAtLocation(callExpr.arguments[0]);
    if (!this.isValidObjectAssignRecordType(targetArgType)) {
      return false;
    }
    const contextualType = this.typeChecker.getContextualType(callExpr);
    if (!contextualType || !this.isValidObjectAssignRecordType(contextualType)) {
      return false;
    }

    return true;
  }

  private isValidObjectAssignRecordType(type: ts.Type): boolean {
    if (this.tsUtils.isStdRecordType(type) && type.aliasTypeArguments?.length) {
      const typeArg = type.aliasTypeArguments[0];
      if (typeArg.getFlags() & (ts.TypeFlags.String | ts.TypeFlags.StringLiteral)) {
        return true;
      }
    }
    return false;
  }
}
