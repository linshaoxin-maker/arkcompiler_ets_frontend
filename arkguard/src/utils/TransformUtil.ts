/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import {
  forEachChild,
  getLeadingCommentRangesOfNode,
  isBinaryExpression,
  isCallExpression,
  isExpressionStatement,
  isIdentifier,
  isPropertyAccessExpression,
  isStringLiteral,
  isVariableStatement,
  SyntaxKind,
  visitEachChild
} from 'typescript';

import type {
  CommentRange,
  Expression,
  Identifier,
  Node,
  SourceFile,
  Statement,
  TransformationContext 
} from 'typescript';
/**
 * collect exist identifier names in current source file
 * @param sourceFile
 */
export function collectExistNames(sourceFile: SourceFile): Set<string> {
  const identifiers: Set<string> = new Set<string>();

  let visit = (node: Node): void => {
    if (isIdentifier(node)) {
      identifiers.add(node.text);
    }

    forEachChild(node, visit);
  };

  forEachChild(sourceFile, visit);
  return identifiers;
}

/**
 * collect exist identifiers in current source file
 * @param sourceFile
 * @param context
 */
export function collectIdentifiers(sourceFile: SourceFile, context: TransformationContext): Identifier[] {
  const identifiers: Identifier[] = [];

  let visit = (node: Node): Node => {
    if (!isIdentifier(node) || !node.parent) {
      return visitEachChild(node, visit, context);
    }

    identifiers.push(node);
    return node;
  };

  visit(sourceFile);
  return identifiers;
}

/**
 * is current node contain ignore obfuscation comment
 * comment: // @skipObfuscate
 */
export function isObfsIgnoreNode(node: Node, sourceFile: SourceFile): boolean {
  const ranges: CommentRange[] = getLeadingCommentRangesOfNode(node, sourceFile);
  if (!ranges) {
    return false;
  }

  const ignoreComment: string = '//@skipObfuscate';
  for (const range of ranges) {
    const comment: string = sourceFile.text.slice(range.pos, range.end).replace(' ', '').replace('\t', '');
    if (comment === ignoreComment) {
      return true;
    }
  }

  return false;
}

export enum OhPackType {
  NONE,
  JS_BUNDLE,
  ES_MODULE
}

/**
 * find openHarmony module import statement
 * example:
 *  jsbundle - var _ohos = _interopRequireDefault(requireModule('@ohos.hilog'));
 *  esmodule - var hilog = globalThis.requireNapi('hilog') || ...
 *
 * @param node
 * @param moduleName full name of imported module, must check format before called, example:
 *  - '@ohos.hilog'
 *  - '@ohos.application.Ability'
 */
export function findOhImportStatement(node: Statement, moduleName: string): OhPackType {
  if (!isVariableStatement(node) || node.declarationList.declarations.length !== 1) {
    return OhPackType.NONE;
  }

  const initializer: Expression = node.declarationList.declarations[0].initializer;
  if (initializer === undefined) {
    return OhPackType.NONE;
  }

  /** esmodule */
  if (isBinaryExpression(initializer)) {
    if (initializer.operatorToken.kind !== SyntaxKind.BarBarToken) {
      return OhPackType.NONE;
    }

    if (!isCallExpression(initializer.left)) {
      return OhPackType.NONE;
    }

    if (!isPropertyAccessExpression(initializer.left.expression)) {
      return OhPackType.NONE;
    }

    if (!isIdentifier(initializer.left.expression.expression) ||
      initializer.left.expression.expression.text !== 'globalThis') {
      return OhPackType.NONE;
    }

    if (!isIdentifier(initializer.left.expression.name) ||
      initializer.left.expression.name.text !== 'requireNapi') {
      return OhPackType.NONE;
    }

    if (initializer.left.arguments.length !== 1) {
      return OhPackType.NONE;
    }

    const arg: Expression = initializer.left.arguments[0];
    if (isStringLiteral(arg) && arg.text === moduleName.substring('@ohos.'.length)) {
      return OhPackType.ES_MODULE;
    }
  }

  /** jsbundle */
  if (isCallExpression(initializer)) {
    if (initializer.arguments.length !== 1) {
      return OhPackType.NONE;
    }

    if (!isIdentifier(initializer.expression) ||
      initializer.expression.text !== '_interopRequireDefault') {
      return OhPackType.NONE;
    }

    const arg: Expression = initializer.arguments[0];
    if (!isCallExpression(arg)) {
      return OhPackType.NONE;
    }

    if (!isIdentifier(arg.expression) || arg.expression.text !== 'requireModule') {
      return OhPackType.NONE;
    }

    const innerArg: Expression = arg.arguments[0];
    if (!isStringLiteral(innerArg) || innerArg.text !== moduleName) {
      return OhPackType.NONE;
    }

    return OhPackType.JS_BUNDLE;
  }

  return OhPackType.NONE;
}

export function isCommentedNode(node: Node, sourceFile: SourceFile): boolean {
  const ranges: CommentRange[] = getLeadingCommentRangesOfNode(node, sourceFile);
  return ranges !== undefined;
}

export function isSuperCallStatement(node: Node): boolean {
  return isExpressionStatement(node) &&
    isCallExpression(node.expression) &&
    node.expression.expression.kind === SyntaxKind.SuperKeyword;
}
