/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import {describe, it} from 'mocha';
import {expect} from 'chai';
import {NodeUtils} from '../../../src/utils/NodeUtils'
import * as ts from 'typescript'
import sinon from 'sinon';

type Mutable<T extends object> = { -readonly [K in keyof T]: T[K] }

function getNodeOfKind(node: ts.Node, kind: ts.SyntaxKind): ts.Node | undefined{
    if (node.kind == kind) {
        return node;
    }
    node.forEachChild(child =>getNodeOfKind(child,kind));
}

describe('test for NodeUtils', function () {
    describe('test for isPropertyDeclarationNode', function () {
        it('should return false if node has no parent', function () {
            const node = ts.factory.createIdentifier('name');
            expect(NodeUtils.isPropertyDeclarationNode(node)).to.be.false;
        })
        it('should return ture if isPropertyAssignment', function () {
            const node = ts.factory.createIdentifier('name');
            const parent = ts.factory.createPropertyAssignment(node, ts.factory.createNumericLiteral('1'));
            (node as Mutable<ts.Node>).parent = parent;
            expect(NodeUtils.isPropertyDeclarationNode(node)).to.be.true;
        })
        it('should return ture for ComputedPropertyName', function () {
            const node = ts.factory.createIdentifier('name');
            const parent = ts.factory.createComputedPropertyName(node);
            (node as Mutable<ts.Node>).parent = parent;
            expect(NodeUtils.isPropertyDeclarationNode(node)).to.be.true;
        })
        it('should return ture for BindingElement', function () {
            const node = ts.factory.createIdentifier('name');
            const parent = ts.factory.createBindingElement(undefined,node,'bindingElement');
            (node as Mutable<ts.Node>).parent = parent;
            expect(NodeUtils.isPropertyDeclarationNode(node)).to.be.true;
        })
        it('should return ture for PropertySignature', function () {
            const node = ts.factory.createIdentifier('name');
            const parent = ts.factory.createPropertySignature(undefined,node,undefined,ts.factory.createKeywordTypeNode(ts.SyntaxKind.StringKeyword));
            (node as Mutable<ts.Node>).parent = parent;
            expect(NodeUtils.isPropertyDeclarationNode(node)).to.be.true;
        })
        it('should return ture for MethodSignature', function () {
            const node = ts.factory.createIdentifier('name');
            const parent = ts.factory.createMethodSignature(undefined,node,undefined,undefined,[],undefined);
            (node as Mutable<ts.Node>).parent = parent;
            expect(NodeUtils.isPropertyDeclarationNode(node)).to.be.true;
        })
        it('should return ture for EnumMember', function () {
            const node = ts.factory.createIdentifier('name');
            const parent = ts.factory.createEnumMember(node);
            (node as Mutable<ts.Node>).parent = parent;
            expect(NodeUtils.isPropertyDeclarationNode(node)).to.be.true;
        })
        it('should return ture for PropertyDeclaration', function () {
            const node = ts.factory.createIdentifier('name');
            const parent = ts.factory.createPropertyDeclaration(undefined,undefined,node,undefined,undefined,undefined);
            (node as Mutable<ts.Node>).parent = parent;
            expect(NodeUtils.isPropertyDeclarationNode(node)).to.be.true;
        })
        it('should return ture for MethodDeclaration', function () {
            const node = ts.factory.createIdentifier('name');
            const parent = ts.factory.createMethodDeclaration(undefined,undefined,undefined,node,undefined,undefined,[],undefined,undefined);
            (node as Mutable<ts.Node>).parent = parent;
            expect(NodeUtils.isPropertyDeclarationNode(node)).to.be.true;
        })
        it('should return ture for SetAccessorDeclaration', function () {
            const node = ts.factory.createIdentifier('name');
            const parent = ts.factory.createSetAccessorDeclaration(undefined,undefined,node,[],undefined);
            (node as Mutable<ts.Node>).parent = parent;
            expect(NodeUtils.isPropertyDeclarationNode(node)).to.be.true;
        })
        it('should return ture for GetAccessorDeclaration', function () {
            const node = ts.factory.createIdentifier('name');
            const parent = ts.factory.createGetAccessorDeclaration(undefined,undefined,node,[],undefined,undefined);
            (node as Mutable<ts.Node>).parent = parent;
            expect(NodeUtils.isPropertyDeclarationNode(node)).to.be.true;
        })
    })
    describe('test for isPropertyOrElementAccessNode', function () {
        let isPropertyAccessNodeStub;
        let isElementAccessNodeStub;
      
        beforeEach(function() {
          isPropertyAccessNodeStub = sinon.stub(NodeUtils, 'isPropertyAccessNode').returns(false);
          isElementAccessNodeStub = sinon.stub(NodeUtils, 'isElementAccessNode').returns(false);
        });
      
        afterEach(function() {
          isPropertyAccessNodeStub.restore();
          isElementAccessNodeStub.restore();
        });

        it('should return true when node is a PropertyAccessNode', function () {
            const node = ts.factory.createIdentifier('name');
            isPropertyAccessNodeStub.returns(true);
            expect(NodeUtils.isPropertyOrElementAccessNode(node)).to.be.true;
        })
        it('should return true when node is a isElementAccessNode', function () {
            const node = ts.factory.createIdentifier('name');
            isElementAccessNodeStub.returns(true);
            expect(NodeUtils.isPropertyOrElementAccessNode(node)).to.be.true;
        })
        it('should return false when both isPropertyAccessNode and isElementAccessNode return false', function () {
            const node = ts.factory.createIdentifier('name');
            expect(NodeUtils.isPropertyOrElementAccessNode(node)).to.be.false;
        })
    })
    describe('test for isPropertyAccessNode', function () {
        let isInClassDeclarationStub;
        let isInExpressionStub;
      
        beforeEach(function() {
          isInClassDeclarationStub = sinon.stub(NodeUtils, 'isInClassDeclaration').returns(false);
          isInExpressionStub = sinon.stub(NodeUtils, 'isInExpression').returns(false);
        });
      
        afterEach(function() {
          isInClassDeclarationStub.restore();
          isInExpressionStub.restore();
        });

        it('should return false if node has no parent', function () {
            const node = ts.factory.createIdentifier('name');
            expect(NodeUtils.isPropertyAccessNode(node)).to.be.false;
        })
        it('should return true if isPropertyAccessExpression and parent.name equals to node', function () {
            const node = ts.factory.createIdentifier('name');
            const parent = ts.factory.createPropertyAccessExpression(node,node);
            (node as Mutable<ts.Node>).parent = parent;
            expect(NodeUtils.isPropertyAccessNode(node)).to.be.true;
        })
        it('should return isInExpression(parent) if isPrivateIdentifier and isInClassDeclaration', function () {
            const node = ts.factory.createPrivateIdentifier("#name");
            const parent = ts.factory.createIdentifier('parent');
            (node as Mutable<ts.Node>).parent = parent;
            isInClassDeclarationStub.returns(true);
            isInExpressionStub.returns(true)
            expect(NodeUtils.isPropertyAccessNode(node)).to.be.true;
            isInExpressionStub.returns(false)
            expect(NodeUtils.isPropertyAccessNode(node)).to.be.false;
        })
        it('should return true if isQualifiedName and parent.right equals to node', function () {
            const node = ts.factory.createIdentifier('name');
            const parent = ts.factory.createQualifiedName(node,node);
            (node as Mutable<ts.Node>).parent = parent;
            expect(NodeUtils.isPropertyAccessNode(node)).to.be.true;
        })
    })
    describe('test for isInClassDeclaration', function () {
        it('should return flase when node is undefined', function () {
            expect(NodeUtils.isInClassDeclarationForTest(undefined)).to.be.false;
        })
        it('should return true when node is ClassDeclaration', function () {
            const node = ts.factory.createClassDeclaration(undefined,undefined,undefined,undefined,undefined,[]);
            expect(NodeUtils.isInClassDeclarationForTest(node)).to.be.true;
        })
        it('should return true when node is ClassExpression', function () {
            const node = ts.factory.createClassExpression(undefined,undefined,undefined,undefined,undefined,[]);
            expect(NodeUtils.isInClassDeclarationForTest(node)).to.be.true;
        })
        it('should return true when node.parent is isInClassDeclaration', function () {
            const node = ts.factory.createIdentifier('name');
            const parent = ts.factory.createClassExpression(undefined,undefined,undefined,undefined,undefined,[]);
            (node as Mutable<ts.Node>).parent = parent;
            expect(NodeUtils.isInClassDeclarationForTest(node)).to.be.true;
        })
    })
    describe('test for isInClassDeclaration', function () {
        it('should return flase when node is undefined', function () {
            expect(NodeUtils.isInClassDeclarationForTest(undefined)).to.be.false;
        })
        it('should return true when node is ClassDeclaration', function () {
            const node = ts.factory.createClassDeclaration(undefined,undefined,undefined,undefined,undefined,[]);
            expect(NodeUtils.isInClassDeclarationForTest(node)).to.be.true;
        })
        it('should return true when node is ClassExpression', function () {
            const node = ts.factory.createClassExpression(undefined,undefined,undefined,undefined,undefined,[]);
            expect(NodeUtils.isInClassDeclarationForTest(node)).to.be.true;
        })
        it('should return true when node.parent is isInClassDeclaration', function () {
            const node = ts.factory.createIdentifier('name');
            const parent = ts.factory.createClassExpression(undefined,undefined,undefined,undefined,undefined,[]);
            (node as Mutable<ts.Node>).parent = parent;
            expect(NodeUtils.isInClassDeclarationForTest(node)).to.be.true;
        })
    })
    describe('test for isInExpression', function () {
        it('should return flase when node is undefined', function () {
            expect(NodeUtils.isInExpressionForTest(undefined)).to.be.false;
        })
        it('should return isInOperator(node) when node is not undefined', function () {
            let isInOperatorStub = sinon.stub(NodeUtils, 'isInOperator').returns(false);
            const node = ts.factory.createIdentifier('name');
            expect(NodeUtils.isInExpressionForTest(node)).to.be.false;
            isInOperatorStub.returns(true);
            expect(NodeUtils.isInExpressionForTest(node)).to.be.true;
            isInOperatorStub.restore();
        })
    })
    describe('test for isInOperator', function () {
        it('should return true when node is binary expression and operator is InKeyword', function () {
            const name = ts.factory.createIdentifier('name');
            const node = ts.factory.createBinaryExpression(name,ts.SyntaxKind.InKeyword,name);
            expect(NodeUtils.isInOperatorTest(node)).to.be.true;
        })
        it('should return false when node is not binary expression', function () {
            const node = ts.factory.createIdentifier('name');
            expect(NodeUtils.isInOperatorTest(node)).to.be.false;
        })
        it('should return false when operator is not Inkeyword', function () {
            const name = ts.factory.createIdentifier('name');
            const node = ts.factory.createBinaryExpression(name,ts.SyntaxKind.PlusEqualsToken,name);
            expect(NodeUtils.isInOperatorTest(node)).to.be.false;
        })
    })
})