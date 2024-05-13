import {
  factory,
  isSourceFile,
  setParentRecursive,
  isStringLiteral,
  visitEachChild,
  Expression,
  SyntaxKind,
  StringLiteral
} from 'typescript';

import type {
  Node,
  TransformationContext,
  Transformer,
  TransformerFactory
} from 'typescript';

import type {IOptions} from '../../configs/IOptions';
import type {TransformPlugin} from '../TransformPlugin';
import {TransformerOrder} from '../TransformPlugin';
import { NodeUtils } from '../../utils/NodeUtils';

namespace secharmony {
  const createEncryptStringFactory = function(option: IOptions): TransformerFactory<Node> {
      // if (!option.mEncryptString) {
      //   return null;
      // }
      console.info('createEncryptStringFactory begin.');

  
      return encryptStringFactory;
  
      function encryptStringFactory(context: TransformationContext): Transformer<Node> {
        return transformer;
  
        function transformer(node: Node): Node {
          if (!isSourceFile(node) || NodeUtils.isDeclarationFile(node)) {
            return node;
          }
  
          let resultAst: Node = visitAst(node);
          return setParentRecursive(resultAst, true);
        }
  
        function visitAst(node: Node): Node {
          if (isStringLiteral(node) && node.text.length != 0) {
            const encryptedText = encrypt(node.text);
            //const decryptCall = factory.createCallExpression(factory.createIdentifier("decryptString"), undefined, [factory.createStringLiteral(encryptedText)]);
            //const decryptCall =  createBufferToStringCall(createBufferFromCall(encryptedText));
            return stringTranslateToCharArray(node.text);
          }else if (node.kind === SyntaxKind.ImportDeclaration) {
            // If it's an import declaration, simply return it without encryption
            return node;
          }
  
          return visitEachChild(node, visitAst, context);
        }
      }

      function stringTranslateToCharArray(text : string) : StringLiteral{
        const splitText = text.split('').join('"]["') + '"]';
        return factory.createStringLiteral(`["${splitText}`);
      }

      function decryptString(text: string): string {
        // Add your decryption logic here
        // For example, if you used Base64 encoding for encryption, you can use atob() function for decryption in JavaScript
        return atob(text);
      }

      function createBufferFromCall(base64String: string): Expression {
        return factory.createCallExpression(
            factory.createPropertyAccessExpression(
                factory.createIdentifier("Buffer"),
                factory.createIdentifier("from")
            ),
            undefined,
            [factory.createStringLiteral(base64String), factory.createStringLiteral("base64")]
        );
      }

      function createBufferToStringCall(buffer: Expression): Node {
        return factory.createCallExpression(
            factory.createPropertyAccessExpression(
                buffer,
                factory.createIdentifier("toString")
            ),
            undefined,
            [factory.createStringLiteral("utf-8")]
        );
      }

      function encrypt(text: string): string {
        // Implement encryption logic here
        return Buffer.from(text).toString('base64');
      }
    };

    export let transformerPlugin: TransformPlugin = {
      'name': 'encryptStringPlugin',
      'order': (1 << TransformerOrder.ENCRYPT_STRING_TRANSFORMER),
      'createTransformerFactory': createEncryptStringFactory
    };
  }

  export = secharmony;