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

import type {Expression, Node, ObjectBindingPattern, SourceFile} from 'typescript';
import {
  SyntaxKind,
  getModifiers,
  isBinaryExpression,
  isBindingElement,
  isCallExpression,
  isClassDeclaration,
  isClassExpression,
  isComputedPropertyName,
  isConstructorDeclaration,
  isElementAccessExpression,
  isEnumMember,
  isGetAccessor,
  isIdentifier,
  isMethodDeclaration,
  isMethodSignature,
  isParameter,
  isPrivateIdentifier,
  isPropertyAccessExpression,
  isPropertyAssignment,
  isPropertyDeclaration,
  isPropertySignature,
  isQualifiedName,
  isSetAccessor,
  isVariableDeclaration
} from 'typescript';
import { isParameterPropertyModifier } from './OhsUtil';

export class NodeUtils {
  public static isPropertyDeclarationNode(node: Node): boolean {
    let parent: Node | undefined = node.parent;
    if (!parent) {
      return false;
    }

    /** eg: { 'name'' : 'akira' }, pass */
    if (isPropertyAssignment(parent)) {
      return parent.name === node;
    }

    if (isComputedPropertyName(parent) && parent.expression === node) {
      return true;
    }

    /** object binding pattern */
    if (isBindingElement(parent) && parent.propertyName === node) {
      return true;
    }

    /** eg: interface/type inf { 'name' : string}, pass */
    if (isPropertySignature(parent) && parent.name === node) {
      return true;
    }

    /** eg: interface/type T1 { func(arg: string): number;} */
    if (isMethodSignature(parent) && parent.name === node) {
      return true;
    }

    /** eg: enum { xxx = 1}; */
    if (isEnumMember(parent) && parent.name === node) {
      return true;
    }

    /** class { private name= 1}; */
    if (isPropertyDeclaration(parent) && parent.name === node) {
      return true;
    }

    /** class {'getName': function() {}} let _ = { getName() [}} */
    if (isMethodDeclaration(parent) && parent.name === node) {
      return true;
    }

    if (isSetAccessor(parent) && parent.name === node) {
      return true;
    }

    return isGetAccessor(parent) && parent.name === node;
  }

  public static isPropertyOrElementAccessNode(node: Node): boolean {
    return this.isPropertyAccessNode(node) || this.isElementAccessNode(node) || false;
  }

  public static isPropertyAccessNode(node: Node): boolean {
    let parent: Node | undefined = node.parent;
    if (!parent) {
      return false;
    }

    /** eg: a.b = 1 */
    if (isPropertyAccessExpression(parent) && parent.name === node) {
      return true;
    }
    if (isPrivateIdentifier(node) && NodeUtils.isInClassDeclaration(parent)) {
      return NodeUtils.isInExpression(parent);
    }
    return isQualifiedName(parent) && parent.right === node;
  }

  private static isInClassDeclaration(node: Node | undefined): boolean {
    if (!node) {
      return false;
    }

    if (isClassDeclaration(node) || isClassExpression(node)) {
      return true;
    }

    return NodeUtils.isInClassDeclaration(node.parent);
  }

  private static isInExpression(node: Node | undefined): boolean {
    return !!node && NodeUtils.isInOperator(node);
  }

  private static isInOperator(node: Node): boolean {
    return isBinaryExpression(node) && node.operatorToken.kind === SyntaxKind.InKeyword;
  }

  public static isElementAccessNode(node: Node): boolean {
    let parent: Node | undefined = node.parent;
    if (!parent) {
      return false;
    }

    return isElementAccessExpression(parent) && parent.argumentExpression === node;
  }

  public static isClassPropertyInConstructorParams(node: Node): boolean {
    if (!isIdentifier(node)) {
      return false;
    }

    if (!node.parent || !isParameter(node.parent)) {
      return false;
    }

    const modifiers = getModifiers(node.parent);
    if (!modifiers || modifiers.length === 0 || !modifiers.find(modifier => isParameterPropertyModifier(modifier))) {
      return false;
    }

    return node.parent.parent && isConstructorDeclaration(node.parent.parent);
  }

  public static isClassPropertyInConstructorBody(node: Node, constructorParams: Set<string>): boolean {
    if (!isIdentifier(node)) {
      return false;
    }

    const id: string = node.escapedText.toString();
    let curNode: Node = node.parent;
    while (curNode) {
      if (isConstructorDeclaration(curNode) && constructorParams.has(id)) {
        return true;
      }

      curNode = curNode.parent;
    }

    return false;
  }

  public static isPropertyNode(node: Node): boolean {
    if (this.isPropertyOrElementAccessNode(node)) {
      return true;
    }

    return this.isPropertyDeclarationNode(node);
  }

  public static isObjectBindingPatternAssignment(node: ObjectBindingPattern): boolean {
    if (!node || !node.parent || !isVariableDeclaration(node.parent)) {
      return false;
    }

    const initializer: Expression = node.parent.initializer;
    return initializer && isCallExpression(initializer);
  }

  public static isDeclarationFile(node: SourceFile): boolean {
    return node.isDeclarationFile;
  }
}
