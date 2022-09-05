const ts = require("typescript");
const fs = require("fs");
var exec = require("child_process").execSync;

const tscKind2es2pandaAST = {
  SourceFile: "Program",
  VariableDeclarationList: "VariableDeclaration",
  VariableDeclaration: "VariableDeclarator",
  TrueKeyword: true,
  Identifier: "Identifier",
  Block: "BlockStatement",
  InterfaceDeclaration: "TSInterfaceDeclaration",
  PropertySignature: "TSPropertySignature",
  NumberKeyword: "TSNumberKeyword",
  TypeReference: "TSTypeReference",
  SuperKeyword: "Super",
  PropertyAccessExpression: "MemberExpression",
  PropertyDeclaration: "ClassProperty",
  BooleanKeyword: "TSBooleanKeyword",
  ArrayType: "TSArrayType",
  MethodDeclaration: "MethodDefinition",
  ThisKeyword: "ThisExpression",
  NumericLiteral: "NumberLiteral",
  ArrayLiteralExpression: "ArrayExpression",
  ObjectLiteralExpression: "ObjectExpression",
  TypeQuery: "TSTypeQuery",
  TypeLiteral: "TSTypeLiteral",
  ConstructSignature: "TSConstructSignatureDeclaration",
  VoidKeyword: "TSVoidKeyword",
  TypeParameter: "TSTypeParameter",
  StringKeyword: "TSStringKeyword",
  IndexSignature: "TSIndexSignature",
  ModuleBlock: "TSModuleBlock",
  ModuleDeclaration: "TSModuleDeclaration",
  MethodSignature: "TSMethodSignature",
  FunctionType: "TSFunctionType",
  AnyKeyword: "TSAnyKeyword",
  ElementAccessExpression: "MemberExpression",
  PrefixUnaryExpression: "UnaryExpression",
  ArrowFunction: "ArrowFunctionExpression",
  CaseClause: "SwitchCase",
  DefaultClause: "SwitchCase",
  ForStatement: "ForUpdateStatement",
  DoStatement: "DoWhileStatement",
  NullKeyword: "NullLiteral",
  TrueKeyword: "BooleanLiteral",
  PropertyAssignment: "Property",
  LabeledStatement: "LabelledStatement",
  PostfixUnaryExpression: "UpdateExpression",
  TypeAssertionExpression: "TSTypeAssertion",
  CallSignature: "TSCallSignatureDeclaration",
  UnionType: "TSUnionType",
  TypeAliasDeclaration: "TSTypeAliasDeclaration",
  ObjectBindingPattern: "ObjectPattern",
  BindingElement: "Property",
  AsExpression: "TSAsExpression",
  ArrayBindingPattern: "ArrayPattern",
  EnumDeclaration: "TSEnumDeclaration",
  EnumMember: "TSEnumMember",
  VoidExpression: "UnaryExpression",
  TypeOfExpression: "UnaryExpression",
  UndefinedKeyword: "TSUndefinedKeyword",
  ParenthesizedType: "TSParenthesizedType",
  NonNullExpression: "TSNonNullExpression",
  TypeOperator: "TSTypeOperator",
  LiteralType: "TSLiteralType",
  UnknownKeyword: "TSUnknownKeyword",
  NeverKeyword: "TSNeverKeyword",
  TupleType: "TSTupleType",
  IndexedAccessType: "TSIndexedAccessType",
  PrivateIdentifier: "TSPrivateIdentifier",
  TypePredicate: "TSTypePredicate",
  FirstTypeNode: "TSTypePredicate",
  TemplateExpression: "TemplateLiteral",
  TemplateHead: "TemplateElement",
  ShorthandPropertyAssignment: "Property",
  LastStatement: "DebuggerStatement",
  DebuggerStatement: "DebuggerStatement",
  ImportEqualsDeclaration: "TSImportEqualsDeclaration",
};

function serializeChildObjects(node, indentLevel, sourceFile) {
  let arr = [];
  node.forEachChild((child) => {
    const val = printRecursiveFrom(child, indentLevel + 1, sourceFile, node);
    if (ts.isVariableStatement(node)) { //237
      arr = val;
    } else if (val === undefined) {
    } else {
      arr.push(val);
    }
  });
  return arr;
}

function serializeThisNodeObjects(
  node,
  indentLevel,
  sourceFile,
  parent = null,
  extendNum = 0
) {
  return printRecursiveFrom(
    node,
    indentLevel + 1,
    sourceFile,
    parent,
    extendNum
  );
}

function printRecursiveFrom(
  node,
  indentLevel,
  sourceFile,
  parent = null,
  extendNum = 0
) {
  if (node === null) return null;
  const identation = "-".repeat(indentLevel);
  const syntaxKind = ts.SyntaxKind[node.kind];
  if (node.kind !== 121) {
    var start = node.getStart(sourceFile);
    var end = node.getEnd();
    if (printPath !== "") {
      console.log(
        identation +
          syntaxKind +
          ":" +
          node.kind +
          "   ==> start: " +
          start +
          "   end: " +
          end +
          "  start: " +
          JSON.stringify(getRowandColumn(start, code)) +
          "end: " +
          JSON.stringify(getRowandColumn(end, code))
      );
    }
  }
  if (ts.isSourceFile(node)) {
    //305  SourceFile
    content = {
      type: tscKind2es2pandaAST[syntaxKind],
      statements: serializeChildObjects(node, indentLevel, sourceFile),
    };
  } else if (node.kind === 1) {
    //1 EndOfFileToken
    return;
  } else if (ts.isNumericLiteral(node)) {
    //8 NumericLiteral -->  NumberLiteral
    if (node.text == "Infinity") {
      content = { type: "NumberLiteral", value: "Infinity" };
    } else {
      content = {
        type: "NumberLiteral",
        value: KeepNDecimal(Number(node.text), 6),
      };
      function KeepNDecimal(num, N) {
        var result = parseFloat(num);
        if (isNaN(result)) {
          alert("参数错误");
        }
        result = Math.round(num * Math.pow(10, N)) / Math.pow(10, N);
        return result;
      }
    }
  } else if (ts.isStringLiteral(node)) {
    //10   StringLiteral-->StringLiteral
    const value = node.text;
    content = { type: syntaxKind, value: value };
  } else if (node.kind === 15) {
    // 15 TemplateHead--> TemplateElement
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.value = { raw: "", cooked: "" };
  } else if (node.kind === 16) {
    //  16 TemplateMiddle
  } else if (node.kind === 17) {
    //  17 TemplateTail
  } else if (node.kind === 25) {
    //  25 DotDotDotToken-->
  } else if (node.kind === 27) {
    //CommaToken
    return ",";
  } else if (node.kind === 28) {
    //QuestionDotToken
  } else if (node.kind === 29) {
    // LessThanToken
    return "<";
  } else if (node.kind === 31) {
    // 31 GreaterThanToken
    return ">";
  } else if (node.kind === 34) {
    //EqualsEqualsToken
    return "==";
  } else if (node.kind === 36) {
    //EqualsEqualsEqualsToken
    return "===";
  } else if (node.kind === 37) {
    //ExclamationEqualsEqualsToken
    return "!==";
  } else if (node.kind === 38) {
    // 38 EqualsGreaterThanToken
    return true;
  } else if (node.kind === 39) {
    // PlusToken
    return "+";
  } else if (node.kind === 40) {
    //MinusToken
    return "-";
  } else if (ts.isAsteriskToken(node)) {
    // 41 asteriskToken  ==> judge the generator
    if (ts.isBinaryExpression(parent)) {
      return "*";
    }
    return true;
  } else if (node.kind === 42) {
    //42  AsteriskAsteriskToken
    return "**";
  } else if (node.kind === 43) {
    //  43  SlashToken
    return "/";
  } else if (node.kind === 44) {
    // 44  PercentToken
    return "%";
  } else if (node.kind === 45) {
    //45 PlusPlusToken
    return "++";
  } else if (node.kind === 46) {
    //46 MinusMinusToken
    return "--";
  } else if (node.kind === 53) {
    // 53 Exclamation
    return "!";
  } else if (node.kind === 54) {
    //54 TildeToken
    return "~";
  } else if (node.kind === 55) {
    // 55 AmpersandAmpersandToken
    return "&&";
  } else if (node.kind === 56) {
    // 56 BarBarToken
    return "||";
  } else if (node.kind === 57) {
    //57 QuestionToken
    return true;
  } else if (node.kind === 60) {
    // 60 QuestionQuestionToken
    return "??";
  } else if (node.kind === 63) {
    //63 EqualsToken
    return "=";
  } else if (node.kind === 75) {
    //75 BarBarEqualsToken
    return "||=";
  } else if (node.kind === 76) {
    //76 AmpersandAmpersandEqualsToken
    return "&&=";
  } else if (node.kind === 77) {
    //77 QuestionQuestionEqualsToken
    return "??=";
  } else if (ts.isIdentifier(node)) {
    //79  Identifier-->Identifier
    const name_kind = syntaxKind;
    const name_escapedText = node.escapedText;
    content = { type: name_kind, name: name_escapedText, decorators: [] };
  } else if (ts.isPrivateIdentifier(node)) {
    // 80 PrivateIdentifier-->TSPrivateIdentifier
    content = { type: "Identifier" };
    content.name = node.escapedText.substring(1);
    content.decorators = [];
  } else if (node.kind === 93) {
    //93 ExportKeyword = 93,
    return true;
  } else if (node.kind === 94) {
    //  94 ExtendsKeyword
    //None
  } else if (node.kind === 95) {
    //  95 FalseKeyword
    content = { type: "BooleanLiteral" };
    content.value = false;
  } else if (node.kind === 102) {
    //102   InstanceOfKeyword
    return "instanceof";
  } else if (node.kind === 103) {
    //103   NewKeyword
  } else if (node.kind === 104) {
    //104 NullKeyword --> NullLiteral
    if (parent && parent.kind === 196) {
      content = { type: "TSNullKeyword" };
    } else {
      content = { type: tscKind2es2pandaAST[syntaxKind] };
      content.value = null;
    }
  } else if (node.kind === 106) {
    // 106 SuperKeyword --> Super
    content = { type: tscKind2es2pandaAST[syntaxKind] };
  } else if (node.kind === 108) {
    //108  ThisKeyword --> ThisExpression
    content = { type: tscKind2es2pandaAST[syntaxKind] };
  } else if (node.kind === 110) {
    //110  TrueKeyword --> BooleanLiteral
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.value = true;
  } else if (node.kind === 114) {
    // 114 VoidKeyword  --> TSVoidKeyword
    content = { type: tscKind2es2pandaAST[syntaxKind] };
  } else if (node.kind === 117) {
    /// 117 implements
    //None
  } else if (node.kind === 121) {
    //121  PrivateKeyword-->
    return "private";
  } else if (node.kind === 122) {
    //  122   ProtectedKeyword
    return "protected";
  } else if (node.kind === 123) {
    //123 PublicKeyword
    content = { type: "TSParameterProperty", accessibility: "public" };
    content.readonly = false;
    content.static = false;
    content.export = false;
    return content;
  } else if (node.kind === 128) {
    //  128   AssertsKeyword
    return true;
  } else if (node.kind === 124) {
    //  124   StaticKeyword
    return "static";
  } else if (node.kind === 130) {
    // 130 AnyKeyword -->TSAnyKeyword
    content = { type: tscKind2es2pandaAST[syntaxKind] };
  } else if (node.kind === 131) {
    // 131  AsyncKeyword ==> To judge async
    return true;
  } else if (node.kind === 133) {
    //133 BooleanKeyword  ->TSBooleanKeyword
    content = { type: tscKind2es2pandaAST[syntaxKind] };
  } else if (node.kind === 135) {
    //135  DeclareKeyword -->
    return true;
  } else if (node.kind === 140) {
    // 140
    return "keyof";
  } else if (node.kind === 143) {
    // 143  NeverKeyword-->TSNeverKeyword
    content = { type: tscKind2es2pandaAST[syntaxKind] };
  } else if (node.kind === 147) {
    //147  NumberKeyword  --> TSNumberKeyword
    content = { type: tscKind2es2pandaAST[syntaxKind] };
  } else if (node.kind === 150) {
    //  150  StringKeyword --> TSStringKeyword

    content = { type: tscKind2es2pandaAST[syntaxKind] };
  } else if (node.kind === 151) {
    //  151  SymbolKeyword
    content = { type: "TSTypeReference" };
    content.typeName = { type: "Identifier", name: "symbol" };
    content.typeName.decorators = [];
    content.typeName.loc = {
      start: getRowandColumn(node.getStart(sourceFile), code),
      end: getRowandColumn(node.end, code),
    };
  } else if (node.kind === 153) {
    // 153 UndefinedKeyword-->TSUndefinedKeyword
    content = { type: tscKind2es2pandaAST[syntaxKind] };
  } else if (node.kind === 155) {
    // 155  UnknownKeyword-->TSUnknownKeyword
    content = { type: tscKind2es2pandaAST[syntaxKind] };
  } else if (ts.isComputedPropertyName(node)) {
    //  162  ComputedPropertyName--> []  computed: true
    // if( node.expression.questionDotToken.kind === 28 || node.expression.questionToken.kind === 28 ){
    //     content= {"type":"ChainExpression"}
    //     content.expression = serializeThisNodeObjects(node.expression,indentLevel,sourceFile);
    // }else{
    return serializeThisNodeObjects(node.expression, indentLevel, sourceFile);
    //}
  } else if (ts.isTypeParameterDeclaration(node)) {
    //  163  TypeParameter --> TSTypeParameter
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.name = serializeThisNodeObjects(node.name, indentLevel, sourceFile);
    if (node.constraint) {
      content.constraint = serializeThisNodeObjects(
        node.constraint,
        indentLevel,
        sourceFile
      );
    }
  } else if (ts.isParameter(node)) {
    //164 Parameter(only)
    if (node.modifiers) {
      content = serializeThisNodeObjects(
        node.modifiers[0],
        indentLevel,
        sourceFile
      );
      content.parameter = serializeThisNodeObjects(
        node.name,
        indentLevel,
        sourceFile
      );
      content.parameter.typeAnnotation = serializeThisNodeObjects(
        node.type,
        indentLevel,
        sourceFile
      );
      content.loc = {
        start: getRowandColumn(node.getStart(sourceFile), code),
        end: getRowandColumn(node.name.end, code),
      };
    } else if (node.initializer) {
      content = { type: "AssignmentPattern" };
      content.left = serializeThisNodeObjects(
        node.name,
        indentLevel,
        sourceFile
      );
      content.right = serializeThisNodeObjects(
        node.initializer,
        indentLevel,
        sourceFile
      );
      content.loc = {
        start: getRowandColumn(node.getStart(sourceFile), code),
        end: getRowandColumn(node.name.end, code),
      };
    } else if (node.dotDotDotToken) {
      content = { type: "RestElement" };
      content.argument = serializeThisNodeObjects(
        node.name,
        indentLevel,
        sourceFile
      );
      if (node.type) {
        content.argument.typeAnnotation = serializeThisNodeObjects(
          node.type,
          indentLevel,
          sourceFile
        );
      }
      content.loc = {
        start: getRowandColumn(node.getStart(sourceFile), code),
        end: getRowandColumn(node.name.end, code),
      };
    } else {
      content = serializeThisNodeObjects(node.name, indentLevel, sourceFile);
      if (node.type) {
        content.typeAnnotation = serializeThisNodeObjects(
          node.type,
          indentLevel,
          sourceFile
        );
      }
      if (node.questionToken) {
        if (node.questionToken.kind === 57) {
          content.optional = true;
        }
      }
    }
    return content;
  } else if (ts.isDecorator(node)) {
    //  165 Decorator-->Decorator
    content = { type: syntaxKind };
    content.expression = serializeThisNodeObjects(
      node.expression,
      indentLevel,
      sourceFile
    );
  } else if (ts.isPropertySignature(node)) {
    //166 PropertySignature  -->  TSPropertySignature
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.computed = false;
    content.optional = false;
    content.readonly = false;
    content.key = serializeThisNodeObjects(node.name, indentLevel, sourceFile);
    if (node.type) {
      content.typeAnnotation = serializeThisNodeObjects(
        node.type,
        indentLevel,
        sourceFile
      );
    }
    if (node.questionToken) {
      content.optional = true;
    }
  } else if (ts.isPropertyDeclaration(node)) {
    // 167 PropertyDeclaration --> ClassProperty
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    if (ts.isPrivateIdentifier(node.name)) {
      // special condition: PrivateIdentifier
      content.key = { type: "TSPrivateIdentifier" };
      content.key.key = serializeThisNodeObjects(
        node.name,
        indentLevel,
        sourceFile
      );
      if (node.initializer) {
        content.key.value = serializeThisNodeObjects(
          node.initializer,
          indentLevel,
          sourceFile
        );
      }
      if (node.type) {
        content.key.typeAnnotation = serializeThisNodeObjects(
          node.type,
          indentLevel,
          sourceFile
        );
      }
      content.key.loc = {
        start: getRowandColumn(node.name.getStart(sourceFile), code),
        end: getRowandColumn(node.name.end, code),
      };
    } else {
      content.key = serializeThisNodeObjects(
        node.name,
        indentLevel,
        sourceFile
      );
      content.static = false;
      content.readonly = false;
      content.declare = false;
      content.optional = false;
      content.computed = false;
      if (node.modifiers) {
        for (var i = 0; i < node.modifiers.length; i++) {
          switch (node.modifiers[i].kind) {
            case 121:
              content.accessibility = "private";
              break;
            case 122:
              content.accessibility = "protected";
              break;
            case 123:
              content.accessibility = "public";
              break;
            case 124:
              content.static = true;
              break;
          }
        }
      }
      if (node.initializer) {
        content.value = serializeThisNodeObjects(
          node.initializer,
          indentLevel,
          sourceFile
        );
      }
      if (node.type) {
        content.typeAnnotation = serializeThisNodeObjects(
          node.type,
          indentLevel,
          sourceFile
        );
      }
      if (node.questionToken) {
        content.optional = true;
      }
      content.decorators = [];
      if (node.decorators) {
        for (var i = 0; i < node.decorators.length; i++) {
          content.decorators[i] = serializeThisNodeObjects(
            node.decorators[i],
            indentLevel,
            sourceFile
          );
        }
      }
    }
  } else if (ts.isMethodSignature(node)) {
    // 168 MethodSignature  --> TSMethodSignature
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.computed = false;
    content.optional = false;
    content.key = serializeThisNodeObjects(node.name, indentLevel, sourceFile);
    content.params = [];
    for (var i = 0; i < node.parameters.length; i++) {
      content.params[i] = serializeThisNodeObjects(
        node.parameters[i],
        indentLevel,
        sourceFile
      );
    }
    if (node.type) {
      content.typeAnnotation = serializeThisNodeObjects(
        node.type,
        indentLevel,
        sourceFile
      );
    }
    if (node.typeParameters) {
      content.typeParameters = { type: "TSTypeParameterDeclaration" };
      content.typeParameters.params = [];
      for (var i = 0; i < node.typeParameters.length; i++) {
        content.typeParameters.params[i] = serializeThisNodeObjects(
          node.typeParameters[i],
          indentLevel,
          sourceFile,
          node
        );
      }
    }
    if (node.questionToken) {
      content.optional = true;
    }
  } else if (ts.isMethodDeclaration(node)) {
    //169  MethodDeclaration -->  MethodDefinition
    if (parent && ts.isObjectLiteralExpression(parent)) {
      // 205 Attention！！！！！！！！！！
      content = { type: "Property" };
      content.method = true;
      content.shorthand = false;
      content.computed = false;
      content.value = {
        type: "FunctionExpression",
        function: {
          type: "ScriptFunction",
          id: null,
          generator: false,
          async: false,
          expression: false,
        },
      };
      content.kind = "init";
    } else {
      content = { type: tscKind2es2pandaAST[syntaxKind] };
      content.kind = "method";
      content.static = false;
      if (node.modifiers) {
        for (var i = 0; i < node.modifiers.length; i++) {
          switch (node.modifiers[i].kind) {
            case 121:
              content.accessibility = "private";
              break;
            case 122:
              content.accessibility = "protected";
              break;
            case 123:
              content.accessibility = "public";
              break;
            case 124:
              content.static = true;
              break;
          }
        }
      }
      content.optional = node.questionToken ? true : false;
      content.computed = false;
      content.overloads = [];
      content.decorators = [];
      if (node.decorators) {
        for (var i = 0; i < node.decorators.length; i++) {
          content.decorators[i] = serializeThisNodeObjects(
            node.decorators[i],
            indentLevel,
            sourceFile
          );
        }
      }
      content.value = {
        type: "FunctionExpression",
        function: {
          type: "ScriptFunction",
          id: null,
          generator: false,
          async: false,
          expression: false,
        },
      };
    }
    // common feature
    content.key = serializeThisNodeObjects(node.name, indentLevel, sourceFile);
    if (ts.isComputedPropertyName(node.name)) {
      content.computed = true;
    }
    content.value.function.params = [];
    for (var i = 0; i < node.parameters.length; i++) {
      content.value.function.params[i] = serializeThisNodeObjects(
        node.parameters[i],
        indentLevel,
        sourceFile
      );
    }
    if (node.typeParameters) {
      content.value.function.typeParameters = {
        type: "TSTypeParameterDeclaration",
      };
      content.value.function.typeParameters.params = [];
      for (var i = 0; i < node.typeParameters.length; i++) {
        content.value.function.typeParameters.params[i] =
          serializeThisNodeObjects(
            node.typeParameters[i],
            indentLevel,
            sourceFile
          );
      }
    }
    if (node.type) {
      content.value.function.returnType = serializeThisNodeObjects(
        node.type,
        indentLevel,
        sourceFile
      );
    }
    if (node.body) {
      content.value.function.body = serializeThisNodeObjects(
        node.body,
        indentLevel,
        sourceFile
      );
    }
    content.value.function.generator =
      node.asteriskToken && node.asteriskToken.kind === 41 ? true : false;
    if (node.modifiers) {
      for (var i = 0; i < node.modifiers.length; i++) {
        switch (node.modifiers[i].kind) {
          case 131:
            content.value.function.async = true;
        }
      }
    }
  } else if (ts.isConstructorDeclaration(node)) {
    //171  ConstructorDeclaration-->
    content = {
      type: "MethodDefinition",
      key: { type: "Identifier", name: "constructor", decorators: [] },
    };
    //content.definition.constructor.key.loc=
    content.kind = "constructor";
    content.static = false;
    content.optional = false;
    content.computed = false;
    content.value = {
      type: "FunctionExpression",
      function: {
        type: "ScriptFunction",
        id: null,
        generator: false,
        async: false,
        expression: false,
      },
    };
    content.value.function.params = [];
    for (var i = 0; i < node.parameters.length; i++) {
      content.value.function.params[i] = serializeThisNodeObjects(
        node.parameters[i],
        indentLevel,
        sourceFile
      );
    }
    content.value.function.body = serializeThisNodeObjects(
      node.body,
      indentLevel,
      sourceFile
    );
    content.overloads = [];
    content.decorators = [];
    if (node.decorators) {
      for (var i = 0; i < node.decorators.length; i++) {
        content.decorators[i] = serializeThisNodeObjects(
          node.decorators[i],
          indentLevel,
          sourceFile
        );
      }
    }
  } else if (ts.isGetAccessor(node)) {
    //172 GetAccessor-->
    if (parent && parent.kind === 205) {
      //ObjectExpression
      content = { type: "Property" };
      content.method = false;
      content.shorthand = false;
      content.computed = false;
    } else {
      content = { type: "MethodDefinition" };
      content.static = false;
      content.optional = false;
      content.computed = false;
    }
    content.key = serializeThisNodeObjects(node.name, indentLevel, sourceFile);
    content.kind = "get";
    content.value = {
      type: "FunctionExpression",
      function: {
        type: "ScriptFunction",
        id: null,
        generator: false,
        async: false,
        expression: false,
        params: [],
      },
    };
    for (var i = 0; i < node.parameters.length; i++) {
      content.value.function.params[i] = serializeThisNodeObjects(
        node.parameters[i],
        indentLevel,
        sourceFile
      );
    }
    if (node.body) {
      content.value.function.body = serializeThisNodeObjects(
        node.body,
        indentLevel,
        sourceFile
      );
    }
    if (node.modifiers) {
      for (var i = 0; i < node.modifiers.length; i++) {
        switch (node.modifiers[i].kind) {
          case 121:
            content.accessibility = "private";
            break;
          case 122:
            content.accessibility = "protected";
            break;
          case 123:
            content.accessibility = "public";
            break;
          case 124:
            content.static = true;
            break;
        }
      }
    }
    if (node.name.kind === 162) {
      content.computed = true;
    } //  162  ComputedPropertyName--> []  computed: true
    content.overloads = [];
    content.decorators = [];
    if (node.decorators) {
      for (var i = 0; i < node.decorators.length; i++) {
        content.decorators[i] = serializeThisNodeObjects(
          node.decorators[i],
          indentLevel,
          sourceFile
        );
      }
    }
  } else if (ts.isSetAccessor(node)) {
    //173 SetAccessor-->
    if (parent && parent.kind === 205) {
      //ObjectExpression
      content = { type: "Property" };
      content.method = false;
      content.shorthand = false;
      content.computed = false;
    } else {
      content = { type: "MethodDefinition" };
      content.static = false;
      content.optional = false;
      content.computed = false;
    }
    if (node.name.kind === 162) {
      content.computed = true;
    } //  162  ComputedPropertyName--> []  computed: true
    content.key = serializeThisNodeObjects(node.name, indentLevel, sourceFile);
    content.kind = "set";
    content.value = {
      type: "FunctionExpression",
      function: {
        type: "ScriptFunction",
        id: null,
        generator: false,
        async: false,
        expression: false,
        params: [],
      },
    };
    for (var i = 0; i < node.parameters.length; i++) {
      content.value.function.params[i] = serializeThisNodeObjects(
        node.parameters[i],
        indentLevel,
        sourceFile
      );
    }
    if (node.body) {
      content.value.function.body = serializeThisNodeObjects(
        node.body,
        indentLevel,
        sourceFile
      );
    }
    if (node.modifiers) {
      for (var i = 0; i < node.modifiers.length; i++) {
        switch (node.modifiers[i].kind) {
          case 121:
            content.accessibility = "private";
            break;
          case 122:
            content.accessibility = "protected";
            break;
          case 123:
            content.accessibility = "public";
            break;
          case 124:
            content.static = true;
            break;
        }
      }
    }
    content.overloads = [];
    content.decorators = [];
    if (node.decorators) {
      for (var i = 0; i < node.decorators.length; i++) {
        content.decorators[i] = serializeThisNodeObjects(
          node.decorators[i],
          indentLevel,
          sourceFile
        );
      }
    }
  } else if (ts.isCallSignatureDeclaration(node)) {
    //  174  CallSignature-->TSCallSignatureDeclaration
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.params = [];
    for (var i = 0; i < node.parameters.length; i++) {
      content.params[i] = serializeThisNodeObjects(
        node.parameters[i],
        indentLevel,
        sourceFile
      );
    }
    if (node.type) {
      content.returnType = serializeThisNodeObjects(
        node.type,
        indentLevel,
        sourceFile
      );
    }
    if (node.typeParameters) {
      content.typeParameters = { type: "TSTypeParameterDeclaration" };
      content.typeParameters.params = [];
      for (var i = 0; i < node.typeParameters.length; i++) {
        content.typeParameters.params[i] = serializeThisNodeObjects(
          node.typeParameters[i],
          indentLevel,
          sourceFile
        );
      }
    }
  } else if (ts.isConstructSignatureDeclaration(node)) {
    // 175  ConstructSignature --> TSConstructSignatureDeclaration
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.params = [];
    for (var i = 0; i < node.parameters.length; i++) {
      content.params[i] = serializeThisNodeObjects(
        node.parameters[i],
        indentLevel,
        sourceFile
      );
    }
    if (node.type) {
      content.returnType = serializeThisNodeObjects(
        node.type,
        indentLevel,
        sourceFile
      );
    }
    if (node.typeParameters) {
      content.typeParameters = { type: "TSTypeParameterDeclaration" };
      content.typeParameters.params = [];
      for (var i = 0; i < node.typeParameters.length; i++) {
        content.typeParameters.params[i] = serializeThisNodeObjects(
          node.typeParameters[i],
          indentLevel,
          sourceFile
        );
      }
    }
  } else if (ts.isIndexSignatureDeclaration(node)) {
    //176  IndexSignature  -->  TSIndexSignature
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.parameters = serializeThisNodeObjects(
      node.parameters[0].name,
      indentLevel,
      sourceFile
    );
    content.parameters.typeAnnotation = serializeThisNodeObjects(
      node.parameters[0].type,
      indentLevel,
      sourceFile
    );
    content.typeAnnotation = serializeThisNodeObjects(
      node.type,
      indentLevel,
      sourceFile
    );
    content.readonly = false;
  } else if (ts.isTypePredicateNode(node)) {
    // 177 FirstTypeNode / TypePredicate-->TSTypePredicate
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.parameterName = serializeThisNodeObjects(
      node.parameterName,
      indentLevel,
      sourceFile
    );
    if (node.type) {
      content.typeAnnotation = serializeThisNodeObjects(
        node.type,
        indentLevel,
        sourceFile
      );
    }
    if (node.assertsModifier) {
      content.asserts = node.assertsModifier.kind === 128 ? true : false;
    }
  } else if (ts.isTypeReferenceNode(node)) {
    //178 TypeReference-->  TSTypeReference
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.typeName = serializeThisNodeObjects(
      node.typeName,
      indentLevel,
      sourceFile
    );
    if (node.typeArguments) {
      content.typeParameters = { type: "TSTypeParameterInstantiation" };
      content.typeParameters.params = [];
      for (var i = 0; i < node.typeArguments.length; i++) {
        content.typeParameters.params[i] = serializeThisNodeObjects(
          node.typeArguments[i],
          indentLevel,
          sourceFile,
          node
        );
      }
      // content.typeParameters.loc=   Error,Waiting for dealing！！！！！！！！！！！！！！！
    }
  } else if (ts.isFunctionTypeNode(node)) {
    // 179  FunctionType --> TSFunctionType
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.params = [];
    for (var i = 0; i < node.parameters.length; i++) {
      content.params[i] = serializeThisNodeObjects(
        node.parameters[i],
        indentLevel,
        sourceFile
      );
    }
    if (node.typeParameters) {
      content.typeParameters = { type: "TSTypeParameterDeclaration" };
      content.typeParameters.params = [];
      for (var i = 0; i < node.typeParameters.length; i++) {
        content.typeParameters.params[i] = serializeThisNodeObjects(
          node.typeParameters[i],
          indentLevel,
          sourceFile
        );
      }
    }
    content.returnType = serializeThisNodeObjects(
      node.type,
      indentLevel,
      sourceFile
    );
  } else if (ts.isTypeQueryNode(node)) {
    //   181  TypeQuery --> TSTypeQuery
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.exprName = serializeThisNodeObjects(
      node.exprName,
      indentLevel,
      sourceFile
    );
  } else if (ts.isTypeLiteralNode(node)) {
    // 182 TypeLiteral  -->  TSTypeLiteral
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.members = [];
    for (var i = 0; i < node.members.length; i++) {
      content.members[i] = serializeThisNodeObjects(
        node.members[i],
        indentLevel,
        sourceFile
      );
    }
  } else if (ts.isArrayTypeNode(node)) {
    //   183 ArrayType -->  TSArrayType
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.elementType = serializeThisNodeObjects(
      node.elementType,
      indentLevel,
      sourceFile
    );
  } else if (ts.isTupleTypeNode(node)) {
    //   184 TupleType -->  TSTupleType
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.elementTypes = [];
    for (var i = 0; i < node.elements.length; i++) {
      content.elementTypes[i] = serializeThisNodeObjects(
        node.elements[i],
        indentLevel,
        sourceFile
      );
    }
  } else if (ts.isUnionTypeNode(node)) {
    //187 UnionType-->TSUnionType
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.types = [];
    for (var i = 0; i < node.types.length; i++) {
      content.types[i] = serializeThisNodeObjects(
        node.types[i],
        indentLevel,
        sourceFile
      );
    }
  } else if (ts.isParenthesizedTypeNode(node)) {
    // 191 ParenthesizedType-->TSParenthesizedType
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.typeAnnotation = serializeThisNodeObjects(
      node.type,
      indentLevel,
      sourceFile
    );
  } else if (ts.isTypeOperatorNode(node)) {
    // 193 TypeOperator-->TSTypeOperator
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    switch (node.operator) {
      case 140:
        content.operator = "keyof";
        break;
    }
    content.typeAnnotation = serializeThisNodeObjects(
      node.type,
      indentLevel,
      sourceFile
    );
  } else if (ts.isIndexedAccessTypeNode(node)) {
    // 194 IndexedAccessType -->TSIndexedAccessType
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.objectType = serializeThisNodeObjects(
      node.objectType,
      indentLevel,
      sourceFile
    );
    content.indexType = serializeThisNodeObjects(
      node.indexType,
      indentLevel,
      sourceFile
    );
  } else if (ts.isLiteralTypeNode(node)) {
    // 196   LiteralType-->TSLiteralType
    if (node.literal.kind === 104 || node.literal.kind === 106) {
      return serializeThisNodeObjects(
        node.literal,
        indentLevel,
        sourceFile,
        node
      );
    } else {
      content = { type: tscKind2es2pandaAST[syntaxKind] };
      content.literal = serializeThisNodeObjects(
        node.literal,
        indentLevel,
        sourceFile
      );
    }
  } else if (ts.isObjectBindingPattern(node)) {
    //201  ObjectBindingPattern-->ObjectPattern
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.properties = [];
    if (node.elements.length > 0) {
      for (var i = 0; i < node.elements.length; i++) {
        content.properties[i] = serializeThisNodeObjects(
          node.elements[i],
          indentLevel,
          sourceFile
        );
      }
    }
  } else if (ts.isArrayBindingPattern(node)) {
    //202 ArrayBindingPattern-->ArrayPattern
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.elements = [];
    for (var i = 0; i < node.elements.length; i++) {
      content.elements[i] = serializeThisNodeObjects(
        node.elements[i],
        indentLevel,
        sourceFile,
        node
      );
    }
  } else if (ts.isBindingElement(node)) {
    //203 BindingElement-->Property
    if (node.dotDotDotToken && node.dotDotDotToken.kind === 25) {
      // DotDotDotToken
      content = { type: "RestElement" };
      content.argument = serializeThisNodeObjects(
        node.name,
        indentLevel,
        sourceFile
      );
    } else if (parent && ts.isArrayBindingPattern(parent) && node.initializer) {
      //202
      content = { type: "AssignmentPattern" };
      content.left = serializeThisNodeObjects(
        node.name,
        indentLevel,
        sourceFile
      );
      content.right = serializeThisNodeObjects(
        node.initializer,
        indentLevel,
        sourceFile
      );
    } else if (parent && ts.isArrayBindingPattern(parent)) {
      //202
      return serializeThisNodeObjects(node.name, indentLevel, sourceFile);
    } else {
      content = { type: tscKind2es2pandaAST[syntaxKind] };
      content.method = false;
      content.shorthand = false;
      content.computed = false;
      if (node.propertyName && node.initializer) {
        content.key = serializeThisNodeObjects(
          node.propertyName,
          indentLevel,
          sourceFile
        );
        content.value = { type: "AssignmentPattern" };
        content.value.left = serializeThisNodeObjects(
          node.name,
          indentLevel,
          sourceFile
        );
        content.value.right = serializeThisNodeObjects(
          node.initializer,
          indentLevel,
          sourceFile
        );
        if (node.propertyName.kind === 162) {
          content.computed = true;
        } //  162  ComputedPropertyName--> []  computed: true
      } else if (node.propertyName) {
        content.key = serializeThisNodeObjects(
          node.propertyName,
          indentLevel,
          sourceFile
        );
        content.value = serializeThisNodeObjects(
          node.name,
          indentLevel,
          sourceFile
        );
        if (node.propertyName.kind === 162) {
          content.computed = true;
        } //  162  ComputedPropertyName--> []  computed: true
      } else if (node.initializer) {
        content.key = serializeThisNodeObjects(
          node.name,
          indentLevel,
          sourceFile
        );
        content.value = serializeThisNodeObjects(
          node.initializer,
          indentLevel,
          sourceFile
        );
      } else {
        content.key = serializeThisNodeObjects(
          node.name,
          indentLevel,
          sourceFile
        );
      }
      content.kind = "init";
    }
  } else if (ts.isArrayLiteralExpression(node)) {
    //   204 ArrayLiteralExpression -->  ArrayExpression
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.elements = [];
    if (node.elements.length > 0) {
      for (var i = 0; i < node.elements.length; i++) {
        content.elements[i] = serializeThisNodeObjects(
          node.elements[i],
          indentLevel,
          sourceFile
        );
      }
    }
  } else if (ts.isObjectLiteralExpression(node)) {
    // 205  ObjectLiteralExpression  --> ObjectExpression
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.properties = [];
    for (var i = 0; i < node.properties.length; i++) {
      content.properties[i] = serializeThisNodeObjects(
        node.properties[i],
        indentLevel,
        sourceFile,
        node
      );
    }
  } else if (ts.isPropertyAccessExpression(node)) {
    //  206  PropertyAccessExpression  --> MemberExpression
    // if( (node.expression.kind === 230 || node.expression.kind === 207 || node.expression.kind === 208)  || !parent
    //       && parent.kind !==206 && parent.kind !==207 && parent.kind !==230 ){
    //     content= {"type":"ChainExpression"}
    //     content.expression = {"type":tscKind2es2pandaAST[syntaxKind]}
    //     content.expression.object = serializeThisNodeObjects(node.expression,indentLevel,sourceFile,node)
    //     content.expression.property = serializeThisNodeObjects(node.name,indentLevel,sourceFile);
    //     content.expression.computed = false;
    //     content.expression.optional = false;
    //     if(node.questionDotToken) {content.expression.optional = true;}
    // }else{
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.object = serializeThisNodeObjects(
      node.expression,
      indentLevel,
      sourceFile
    );
    content.property = serializeThisNodeObjects(
      node.name,
      indentLevel,
      sourceFile
    );
    content.computed = false;
    content.optional = false;
    if (node.questionDotToken) {
      content.optional = true;
    }
    //}
  } else if (ts.isElementAccessExpression(node)) {
    //207 ElementAccessExpression --> MemberExpression
    // if(((node.expression.kind === 230||node.expression.kind === 207) && parent.kind !==206 && parent.kind !==207 && parent.kind !==230) || (node.expression.kind === 206 && parent && parent.kind !==206 && parent.kind !==207 && parent.kind !==230)){
    //     content= {"type":"ChainExpression"}
    //     content.expression = {"type":tscKind2es2pandaAST[syntaxKind]}
    //     content.expression.object = serializeThisNodeObjects(node.expression,indentLevel,sourceFile,node)
    //     content.expression.property = serializeThisNodeObjects(node.argumentExpression,indentLevel,sourceFile);
    //     content.expression.computed = false;
    //     content.expression.optional = false;
    //     if(node.questionDotToken) {content.expression.optional = true;}
    // }else if(node.questionDotToken && node.questionDotToken.kind === 28  && (parent &&  parent.kind !== 206 && parent.kind !== 207 && parent.kind !== 230)){
    //     content= {"type":"ChainExpression"}
    //     content.expression = {"type":tscKind2es2pandaAST[syntaxKind]}
    //     content.expression.object = serializeThisNodeObjects(node.expression,indentLevel,sourceFile,node)
    //     content.expression.property = serializeThisNodeObjects(node.argumentExpression,indentLevel,sourceFile)
    //     content.expression.computed = true;
    //     content.expression.optional = false;if(node.questionDotToken) {content.expression.optional = true;}
    // }else{
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.object = serializeThisNodeObjects(
      node.expression,
      indentLevel,
      sourceFile,
      node
    );
    content.property = serializeThisNodeObjects(
      node.argumentExpression,
      indentLevel,
      sourceFile
    );
    content.computed = true;
    content.optional = false;
    if (node.questionDotToken) {
      content.optional = true;
    }
    //}
  } else if (ts.isCallExpression(node)) {
    //208 CallExpression -->  CallExpression
    content = { type: syntaxKind };
    content.callee = serializeThisNodeObjects(
      node.expression,
      indentLevel,
      sourceFile
    );
    content.arguments = [];
    if (node.arguments.length > 0) {
      for (var i = 0; i < node.arguments.length; i++) {
        content.arguments[i] = serializeThisNodeObjects(
          node.arguments[i],
          indentLevel,
          sourceFile
        );
      }
    }
    if (node.typeArguments) {
      content.typeParameters = { type: "TSTypeParameterInstantiation" };
      content.typeParameters.params = [];
      for (var i = 0; i < node.typeArguments.length; i++) {
        content.typeParameters.params[i] = serializeThisNodeObjects(
          node.typeArguments[i],
          indentLevel,
          sourceFile
        );
      }
    }
    content.optional = false;
    if (node.questionDotToken) {
      content.optional = true;
    }
    // if( ( RecursiveFindQuestionDot(node) || node.expression.kind===206 || node.expression.kind===207|| node.expression.kind===208 || node.expression.kind===230)
    //   || !parent  &&  parent.kind !== 206  && parent.kind !== 207 && parent.kind !== 208 && parent.kind !== 230){
    //     tmpcontent = {"type":"ChainExpression"}
    //     content.loc= {"start":getRowandColumn(node.getStart(sourceFile),code),"end":getRowandColumn(node.end,code)};
    //     tmpcontent.expression = content
    //     tmpcontent.loc =  {"start":getRowandColumn(node.getStart(sourceFile),code),"end":getRowandColumn(node.end,code)};
    //     return tmpcontent;
    // }
  } else if (ts.isNewExpression(node)) {
    // 209 NewExpression-->NewExpression
    content = { type: syntaxKind };
    content.callee = serializeThisNodeObjects(
      node.expression,
      indentLevel,
      sourceFile
    );
    if (node.arguments) {
      content.arguments = [];
      for (var i = 0; i < node.arguments.length; i++) {
        content.arguments[i] = serializeThisNodeObjects(
          node.arguments[i],
          indentLevel,
          sourceFile
        );
      }
    }
  } else if (ts.isTaggedTemplateExpression(node)) {
    // 210 TaggedTemplateExpression-->TaggedTemplateExpression
    content = { type: syntaxKind };
    content.tag = serializeThisNodeObjects(node.tag, indentLevel, sourceFile);
    content.quasi = serializeThisNodeObjects(
      node.template,
      indentLevel,
      sourceFile
    );
  } else if (ts.isTypeAssertionExpression(node)) {
    //211 TypeAssertionExpression-->TSTypeAssertion
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.typeAnnotation = serializeThisNodeObjects(
      node.type,
      indentLevel,
      sourceFile
    );
    content.expression = serializeThisNodeObjects(
      node.expression,
      indentLevel,
      sourceFile
    );
  } else if (ts.isParenthesizedExpression(node)) {
    // 212  ParenthesizedExpression -->
    return serializeThisNodeObjects(
      node.expression,
      indentLevel,
      sourceFile,
      node
    );
  } else if (ts.isFunctionExpression(node)) {
    //213  FunctionExpression --> FunctionExpression
    content = {
      type: "FunctionExpression",
      function: {
        type: "ScriptFunction",
        id: null,
        generator: false,
        async: false,
        expression: false,
      },
    };
    if (node.modifiers && node.modifiers.length > 0) {
      for (var i = 0; i < node.modifiers.length; i++) {
        switch (node.modifiers[i].kind) {
          case 131:
            content.function.async = true;
          case 41:
            content.function.generator = true;
        }
      }
    }
    content.function.generator =
      node.asteriskToken && node.asteriskToken.kind === 41 ? true : false;
    content.function.params = [];
    for (var i = 0; i < node.parameters.length; i++) {
      content.function.params[i] = serializeThisNodeObjects(
        node.parameters[i],
        indentLevel,
        sourceFile
      );
    }
    content.function.body = serializeThisNodeObjects(
      node.body,
      indentLevel,
      sourceFile
    );
    content.function.loc = {
      start: getRowandColumn(node.getStart(sourceFile), code),
      end: getRowandColumn(node.end, code),
    };
    content.overloads = [];
    content.decorators = [];
    if (node.decorators) {
      for (var i = 0; i < node.decorators.length; i++) {
        content.decorators[i] = serializeThisNodeObjects(
          node.decorators[i],
          indentLevel,
          sourceFile
        );
      }
    }
    if (parent && ts.isParenthesizedExpression(parent)) {
      var start = parent.getStart(sourceFile);
      var end = parent.getEnd();
    }
    if (node.name) {
      content.function.id = serializeThisNodeObjects(
        node.name,
        indentLevel,
        sourceFile
      );
    }
    if (node.typeParameters) {
      content.function.typeParameters = { type: "TSTypeParameterDeclaration" };
      content.function.typeParameters.params = [];
      for (var i = 0; i < node.typeParameters.length; i++) {
        content.function.typeParameters.params[i] = serializeThisNodeObjects(
          node.typeParameters[i],
          indentLevel,
          sourceFile
        );
      }
    }
  } else if (ts.isArrowFunction(node)) {
    //214  ArrowFunction-->ArrowFunctionExpression
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.function = {
      type: "ScriptFunction",
      id: null,
      generator: false,
      async: false,
      expression: false,
    };
    if (node.asteriskToken) {
      if (node.asteriskToken.kind === 41) {
        content.function.generator = true;
      }
    }
    if (node.modifiers) {
      for (var i = 0; i < node.modifiers.length; i++) {
        if (node.modifiers[i].kind === 131) {
          content.function.async = true;
        }
      }
    }
    content.function.params = [];
    for (var i = 0; i < node.parameters.length; i++) {
      content.function.params[i] = serializeThisNodeObjects(
        node.parameters[i],
        indentLevel,
        sourceFile
      );
    }
    content.function.body = serializeThisNodeObjects(
      node.body,
      indentLevel,
      sourceFile
    );
    content.function.loc = {
      start: getRowandColumn(node.getStart(sourceFile), code),
      end: getRowandColumn(node.end, code),
    };
    if (node.typeParameters) {
      content.function.typeParameters = { type: "TSTypeParameterDeclaration" };
      content.function.typeParameters.params = [];
      for (var i = 0; i < node.typeParameters.length; i++) {
        content.function.typeParameters.params[i] = serializeThisNodeObjects(
          node.typeParameters[i],
          indentLevel,
          sourceFile
        );
      }
    }
    if (node.body.kind != 235) {
      content.function.expression = true;
    } // if it is { } block
    if (node.type)
      content.function.returnType = serializeThisNodeObjects(
        node.type,
        indentLevel,
        sourceFile
      );
    if (parent && ts.isParenthesizedExpression(parent)) {
      var start = parent.getStart(sourceFile);
      var end = parent.getEnd();
    }
  } else if (ts.isTypeOfExpression(node)) {
    // 216 TypeOfExpression-->UnaryExpression
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.operator = "typeof";
    content.prefix = true;
    content.argument = serializeThisNodeObjects(
      node.expression,
      indentLevel,
      sourceFile
    );
  } else if (ts.isVoidExpression(node)) {
    //217 VoidExpression --> UnaryExpression
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.operator = "void";
    content.prefix = true;
    content.argument = serializeThisNodeObjects(
      node.expression,
      indentLevel,
      sourceFile
    );
  } else if (ts.isAwaitExpression(node)) {
    // 218 AwaitExpression-->AwaitExpression
    content = { type: syntaxKind };
    content.argument = serializeThisNodeObjects(
      node.expression,
      indentLevel,
      sourceFile
    );
  } else if (ts.isPrefixUnaryExpression(node)) {
    //219 PrefixUnaryExpression --> UnaryExpression
    tmpOperator = node.operator;
    if (tmpOperator === 45 || tmpOperator === 46) {
      content = { type: "UpdateExpression" };
      switch (tmpOperator) {
        case 45:
          content.operator = "++";
          break;
        case 46:
          content.operator = "--";
          break;
      }
    } else {
      content = { type: tscKind2es2pandaAST[syntaxKind] };
      switch (node.operator) {
        case 39:
          content.operator = "+";
          break;
        case 40:
          content.operator = "-";
          break;
        case 53:
          content.operator = "!";
          break;
        case 54:
          content.operator = "~";
          break;
      }
    }
    if (parent && ts.isParenthesizedExpression(parent)) {
      start = parent.getStart(sourceFile);
      end = parent.getEnd();
    }
    content.prefix = true;
    content.argument = serializeThisNodeObjects(
      node.operand,
      indentLevel,
      sourceFile
    );
  } else if (ts.isPostfixUnaryExpression(node)) {
    // 220 PostfixUnaryExpression--> UpdateExpression
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    switch (node.operator) {
      case 45:
        content.operator = "++";
        break;
      case 46:
        content.operator = "--";
        break;
    }
    content.prefix = false;
    content.argument = serializeThisNodeObjects(
      node.operand,
      indentLevel,
      sourceFile
    );
  } else if (ts.isBinaryExpression(node)) {
    // 221 BinaryExpression --> BinaryExpression
    operator = serializeThisNodeObjects(
      node.operatorToken,
      indentLevel,
      sourceFile,
      node
    );
    if (operator == "||" || operator == "&&" || operator == "??") {
      content = { type: "LogicalExpression" };
    } else if (
      operator == "=" ||
      operator == "??=" ||
      operator == "||=" ||
      operator == "&&="
    ) {
      content = { type: "AssignmentExpression" };
    } else if (operator == ",") {
      content = { type: "SequenceExpression" };
      var tempexpressions = [];
      var num = 0;
      GetSequenceExoression(node);
      function GetSequenceExoression(tmpnode) {
        if (tmpnode.operatorToken) {
          if (tmpnode.operatorToken.kind === 27) {
            GetSequenceExoression(tmpnode.left);
          } else {
            tempexpressions[num++] = serializeThisNodeObjects(
              tmpnode,
              indentLevel,
              sourceFile
            );
            return;
          }
        } else {
          tempexpressions[num++] = serializeThisNodeObjects(
            tmpnode,
            indentLevel,
            sourceFile
          );
        }
        if (tmpnode.right) {
          tempexpressions[num++] = serializeThisNodeObjects(
            tmpnode.right,
            indentLevel,
            sourceFile
          );
        }
      }
      content.expressions = tempexpressions;
      content.loc = {
        start: getRowandColumn(node.getStart(sourceFile), code),
        end: getRowandColumn(node.end, code),
      };
      return content;
    } else {
      content = { type: syntaxKind };
    }
    content.operator = serializeThisNodeObjects(
      node.operatorToken,
      indentLevel,
      sourceFile,
      node
    );
    content.left = serializeThisNodeObjects(node.left, indentLevel, sourceFile);
    content.right = serializeThisNodeObjects(
      node.right,
      indentLevel,
      sourceFile
    );
  } else if (ts.isConditionalExpression(node)) {
    //222 ConditionalExpression-->ConditionalExpression
    content = { type: syntaxKind };
    content.test = serializeThisNodeObjects(
      node.condition,
      indentLevel,
      sourceFile
    );
    content.consequent = serializeThisNodeObjects(
      node.whenTrue,
      indentLevel,
      sourceFile
    );
    content.alternate = serializeThisNodeObjects(
      node.whenFalse,
      indentLevel,
      sourceFile
    );
  } else if (ts.isTemplateExpression(node)) {
    // 223 TemplateExpression-->TemplateLiteral  // The structure  difference between TSC is large, and it's temporarily ignored.
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.expressions = [];
    for (var i = 0; i < node.templateSpans.length; i++) {
      content.expressions[i] = serializeThisNodeObjects(
        node.templateSpans[i],
        indentLevel,
        sourceFile
      );
    }
    content.quasis = serializeThisNodeObjects(
      node.head,
      indentLevel,
      sourceFile
    );
  } else if (ts.isYieldExpression(node)) {
    //   224 YieldExpression  --> YieldExpression
    content = { type: syntaxKind };
    content.delegate =
      node.asteriskToken && node.asteriskToken.kind === 41 ? true : false;
    content.argument = node.expression
      ? serializeThisNodeObjects(node.expression, indentLevel, sourceFile)
      : null;
  } else if (ts.isSpreadElement(node)) {
    //  225 SpreadElement-->SpreadElement
    content = { type: syntaxKind };
    content.argument = serializeThisNodeObjects(
      node.expression,
      indentLevel,
      sourceFile
    );
  } else if (ts.isClassExpression(node)) {
    // 226 ClassExpression --> ClassExpression
    content = { type: syntaxKind };
    content.definition = {
      id: node.name
        ? serializeThisNodeObjects(node.name, indentLevel, sourceFile)
        : null,
    };
    content.definition.superClass = node.heritageClauses
      ? serializeThisNodeObjects(
          node.heritageClauses[0].types[0].expression,
          indentLevel,
          sourceFile
        )
      : null;
    if (
      node.heritageClauses &&
      node.heritageClauses[0].types[0].typeArguments
    ) {
      content.definition.superTypeParameters = {
        type: "TSTypeParameterInstantiation",
      };
      content.definition.superTypeParameters.params = [];
      for (
        var i = 0;
        i < node.heritageClauses[0].types[0].typeArguments.length;
        i++
      ) {
        content.definition.superTypeParameters.params[i] = node.heritageClauses
          ? serializeThisNodeObjects(
              node.heritageClauses[0].types[0].typeArguments[i],
              indentLevel,
              sourceFile
            )
          : null;
      }
    }
    content.definition.implements = [];
    content.definition.constructor = [];
    content.definition.body = [];
    for (var i = 0; i < node.members.length; i++) {
      content.definition.body[i] = serializeThisNodeObjects(
        node.members[i],
        indentLevel,
        sourceFile
      );
    }
    content.definition.indexSignatures = [];
    if (node.typeParameters) {
      content.definition.typeParameters = {
        type: "TSTypeParameterDeclaration",
      };
      content.definition.typeParameters.params = [];
      for (var i = 0; i < node.typeParameters.length; i++) {
        content.definition.typeParameters.params[i] = serializeThisNodeObjects(
          node.typeParameters[i],
          indentLevel,
          sourceFile
        );
      }
    }
  } else if (ts.isOmittedExpression(node)) {
    // 227 OmittedExpression --> OmittedExpression
    content = { type: syntaxKind };
  } else if (ts.isExpressionWithTypeArguments(node)) {
    //228 ExpressionWithTypeArguments-->
    return serializeThisNodeObjects(node.expression, indentLevel, sourceFile);
  } else if (ts.isAsExpression(node)) {
    //229 AsExpression-->TSAsExpression
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.expression = serializeThisNodeObjects(
      node.expression,
      indentLevel,
      sourceFile
    );
    content.typeAnnotation = serializeThisNodeObjects(
      node.type,
      indentLevel,
      sourceFile
    );
  } else if (ts.isNonNullExpression(node)) {
    // 230 NonNullExpression-->TSNonNullExpression
    if (
      (node.expression.kind === 206 || node.expression.kind === 207) &&
      parent &&
      parent.kind !== 206 &&
      parent.kind !== 207 &&
      parent.kind !== 230 &&
      parent.kind !== 230
    ) {
      content = { type: "ChainExpression" };
      content.expression = { type: tscKind2es2pandaAST[syntaxKind] };
      content.expression.expression = serializeThisNodeObjects(
        node.expression,
        indentLevel,
        sourceFile,
        node
      );
    } else {
      content = { type: tscKind2es2pandaAST[syntaxKind] };
      content.expression = serializeThisNodeObjects(
        node.expression,
        indentLevel,
        sourceFile,
        node
      );
    }
  } else if (ts.isMetaProperty(node)) {
    // 231 MetaProperty-->MetaProperty
    content = { type: syntaxKind };
    content.kind = "new.Target";
  } else if (ts.isTemplateSpan(node)) {
    // 233 TemplateSpan-->
    return serializeThisNodeObjects(node.expression, indentLevel, sourceFile);
  } else if (ts.isBlock(node)) {
    //235 Block  -->  BlockStatement
    const body_kind = tscKind2es2pandaAST[syntaxKind];
    content = {
      type: body_kind,
      statements: serializeChildObjects(node, indentLevel, sourceFile),
    };
  } else if (ts.isEmptyStatement(node)) {
    // 236 EmptyStatement-->EmptyStatement
    content = { type: syntaxKind };
  } else if (ts.isVariableStatement(node)) {
    //237
    return serializeChildObjects(node, indentLevel, sourceFile);
  } else if (ts.isExpressionStatement(node)) {
    //238  ExpressionStatement-->ExpressionStatement
    content = { type: syntaxKind };
    content.expression = serializeChildObjects(
      node,
      indentLevel,
      sourceFile
    )[0];
  } else if (ts.isIfStatement(node)) {
    // 239 IfStatement --> IfStatement
    content = { type: syntaxKind };
    content.test = serializeThisNodeObjects(
      node.expression,
      indentLevel,
      sourceFile
    );
    content.consequent = serializeThisNodeObjects(
      node.thenStatement,
      indentLevel,
      sourceFile
    );
    content.alternate = node.elseStatement
      ? serializeThisNodeObjects(node.elseStatement, indentLevel, sourceFile)
      : null;
  } else if (ts.isDoStatement(node)) {
    //240 DoStatement -->DoWhileStatement
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.body = serializeThisNodeObjects(
      node.statement,
      indentLevel,
      sourceFile
    );
    content.test = serializeThisNodeObjects(
      node.expression,
      indentLevel,
      sourceFile
    );
  } else if (ts.isWhileStatement(node)) {
    //241 WhileStatement -->WhileStatement
    content = { type: syntaxKind };
    content.test = serializeThisNodeObjects(
      node.expression,
      indentLevel,
      sourceFile
    );
    content.body = serializeThisNodeObjects(
      node.statement,
      indentLevel,
      sourceFile
    );
  } else if (ts.isForStatement(node)) {
    //242  ForStatement --> ForUpdateStatement
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.init = node.initializer
      ? serializeThisNodeObjects(node.initializer, indentLevel, sourceFile)
      : null;
    content.test = node.condition
      ? serializeThisNodeObjects(node.condition, indentLevel, sourceFile)
      : null;
    content.update = node.incrementor
      ? serializeThisNodeObjects(node.incrementor, indentLevel, sourceFile)
      : null;
    content.body = serializeThisNodeObjects(
      node.statement,
      indentLevel,
      sourceFile
    );
  } else if (ts.isForInStatement(node)) {
    //243  ForInStatement -->  ForInStatement
    content = { type: syntaxKind };
    content.left = serializeThisNodeObjects(
      node.initializer,
      indentLevel,
      sourceFile
    );
    content.right = serializeThisNodeObjects(
      node.expression,
      indentLevel,
      sourceFile
    );
    content.body = serializeThisNodeObjects(
      node.statement,
      indentLevel,
      sourceFile
    );
  } else if (ts.isForOfStatement(node)) {
    //244  ForOfStatement -->  ForOfStatement
    content = { type: syntaxKind };
    content.await = false;
    content.left = serializeThisNodeObjects(
      node.initializer,
      indentLevel,
      sourceFile
    );
    content.right = serializeThisNodeObjects(
      node.expression,
      indentLevel,
      sourceFile
    );
    content.body = serializeThisNodeObjects(
      node.statement,
      indentLevel,
      sourceFile
    );
  } else if (ts.isContinueStatement(node)) {
    //245 ContinueStatement-->ContinueStatement
    content = { type: syntaxKind };
    content.label = node.label
      ? serializeThisNodeObjects(node.label, indentLevel, sourceFile)
      : null;
  } else if (ts.isBreakStatement(node)) {
    //246 BreakStatement-->BreakStatement
    content = { type: syntaxKind };
    content.label = node.label
      ? serializeThisNodeObjects(node.label, indentLevel, sourceFile)
      : null;
  } else if (ts.isReturnStatement(node)) {
    //247 ReturnStatement --> ReturnStatement
    content = { type: syntaxKind };
    content.argument = node.expression
      ? serializeThisNodeObjects(node.expression, indentLevel, sourceFile)
      : null;
  } else if (ts.isSwitchStatement(node)) {
    //249 SwitchStatement-->SwitchStatement
    content = { type: syntaxKind };
    content.discriminant = serializeThisNodeObjects(
      node.expression,
      indentLevel,
      sourceFile
    );
    content.cases = [];
    for (var i = 0; i < node.caseBlock.clauses.length; i++) {
      content.cases[i] = serializeThisNodeObjects(
        node.caseBlock.clauses[i],
        indentLevel,
        sourceFile
      );
    }
  } else if (ts.isLabeledStatement(node)) {
    // 250  LabeledStatement-->LabelledStatement
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.label = serializeThisNodeObjects(
      node.label,
      indentLevel,
      sourceFile
    );
    content.body = serializeThisNodeObjects(
      node.statement,
      indentLevel,
      sourceFile
    );
  } else if (ts.isThrowStatement(node)) {
    //251 ThrowStatement --> ThrowStatement
    content = { type: syntaxKind };
    content.argument = serializeThisNodeObjects(
      node.expression,
      indentLevel,
      sourceFile
    );
  } else if (ts.isTryStatement(node)) {
    //252 TryStatement --> TryStatement
    content = { type: syntaxKind };
    content.block = serializeThisNodeObjects(
      node.tryBlock,
      indentLevel,
      sourceFile
    );
    if (node.catchClause) {
      content.handler = serializeThisNodeObjects(
        node.catchClause,
        indentLevel,
        sourceFile
      );
    } else {
      content.handler = null;
    }
    content.finalizer = node.finallyBlock
      ? serializeThisNodeObjects(node.finallyBlock, indentLevel, sourceFile)
      : null;
  } else if (ts.isDebuggerStatement(node)) {
    //253  LastStatement/DebuggerStatement-->DebuggerStatement
    content = { type: tscKind2es2pandaAST[syntaxKind] };
  } else if (ts.isVariableDeclaration(node)) {
    //254  VariableDeclaration --> VariableDeclarator
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.id = serializeThisNodeObjects(node.name, indentLevel, sourceFile);
    content.init = node.initializer
      ? serializeThisNodeObjects(node.initializer, indentLevel, sourceFile)
      : null;
    // const init = node.initializer;
    // const value = tscKind2es2pandaAST[ts.SyntaxKind[init.kind]];
    // content.init = {"type":dataType[value],"value":value};
    // content.init.loc={"start":getRowandColumn(init.getStart(sourceFile),code),"end":getRowandColumn(init.end,code)};
    if (node.type) {
      content.id.typeAnnotation = serializeThisNodeObjects(
        node.type,
        indentLevel,
        sourceFile
      );
    }
  } else if (ts.isVariableDeclarationList(node)) {
    //255  VariableDeclarationList  -->  VariableDeclaration
    content = {
      type: tscKind2es2pandaAST[syntaxKind],
      declarations: serializeChildObjects(node, indentLevel, sourceFile),
    };
    if (
      parent &&
      parent.hasOwnProperty("modifiers") &&
      parent["modifiers"] != undefined
    ) {
      for (var i = 0; i < parent.modifiers.length; i++) {
        switch (parent.modifiers[i].kind) {
          case 135:
            content.declare = true;
        }
      }
    }
    //need to get kind property
    content.kind = getVariableDeclarationKind(start, code);
    // adjust the location of start and end;   Unable to judge the attribute of node.parent?
    //  eg:const x = yield;   [for symbol ';' ,tsc will ingnore it. es2panda is the opposite]
    if (
      parent &&
      ts.isVariableStatement(parent) &&
      variableStatementHasSemicolon(end, code) > -1
    ) {
      end = parent.end; //！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
    } //！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
  } else if (ts.isFunctionDeclaration(node)) {
    //256   FunctionDeclaration-->FunctionDeclaration
    content = { type: syntaxKind };
    content.function = { type: "ScriptFunction" };
    content.function.id = serializeThisNodeObjects(
      node.name,
      indentLevel,
      sourceFile
    );
    content.function.generator = false;
    content.function.expression = false;
    content.function.async = false;
    if (node.asteriskToken) {
      if (node.asteriskToken.kind === 41) {
        content.function.generator = true;
      }
    }
    if (node.modifiers) {
      for (var i = 0; i < node.modifiers.length; i++) {
        if (node.modifiers[i].kind === 131) {
          content.function.async = true;
        }
      }
    }
    content.function.params = [];
    for (var i = 0; i < node.parameters.length; i++) {
      content.function.params[i] = serializeThisNodeObjects(
        node.parameters[i],
        indentLevel,
        sourceFile
      );
    }
    content.function.body = serializeThisNodeObjects(
      node.body,
      indentLevel,
      sourceFile
    );
    content.function.loc = {
      start: getRowandColumn(node.getStart(sourceFile), code),
      end: getRowandColumn(node.end, code),
    };
    if (node.typeParameters) {
      content.function.typeParameters = { type: "TSTypeParameterDeclaration" };
      content.function.typeParameters.params = [];
      for (var i = 0; i < node.typeParameters.length; i++) {
        content.function.typeParameters.params[i] = serializeThisNodeObjects(
          node.typeParameters[i],
          indentLevel,
          sourceFile
        );
      }
    }
    if (node.type) {
      content.function.returnType = serializeThisNodeObjects(
        node.type,
        indentLevel,
        sourceFile
      );
    }
  } else if (ts.isClassDeclaration(node)) {
    //257  ClassDeclaration --> ClassDeclaration
    content = {
      type: syntaxKind,
      definition: {
        id: serializeThisNodeObjects(node.name, indentLevel, sourceFile),
      },
    };
    content.definition.implements = [];
    content.definition.superClass = null;
    if (node.heritageClauses) {
      // superClass is not null
      for (var i = 0; i < node.heritageClauses.length; i++) {
        if (node.heritageClauses[i].token === 94) {
          //  94 ExtendsKeyword
          content.definition.superClass = serializeThisNodeObjects(
            node.heritageClauses[i],
            indentLevel,
            sourceFile
          )[0];
        } else if (node.heritageClauses[i].token === 117) {
          ///implements
          content.definition.implements[i] = serializeThisNodeObjects(
            node.heritageClauses[i],
            indentLevel,
            sourceFile
          );
        }
      }
    }
    content.definition.indexSignatures = [];
    // variables and constructer are in the members.  ！！！！！！！Attention！！！！！！！！！
    if (node.members.length > 0) {
      // constructor exists
      content.definition.body = [];
      for (var i = 0, j = 0, k = 0; i < node.members.length; i++) {
        var memberKind = node.members[i].kind;
        if (memberKind === 171) {
          content.definition.constructor = serializeThisNodeObjects(
            node.members[i],
            indentLevel,
            sourceFile
          );
        } else if (memberKind === 176) {
          content.definition.indexSignatures[k] = serializeThisNodeObjects(
            node.members[i],
            indentLevel,
            sourceFile
          );
          k++;
        } else {
          content.definition.body[j] = serializeThisNodeObjects(
            node.members[i],
            indentLevel,
            sourceFile
          );
          j++;
        }
      }
    }
    if (!content.definition.hasOwnProperty("constructor")) {
      content.definition.constructor = {}; // To ignore it ?
    }
    if (!content.definition.hasOwnProperty("body")) {
      content.definition.body = [];
    }
    if (node.typeParameters) {
      content.definition.typeParameters = {
        type: "TSTypeParameterDeclaration",
      };
      content.definition.typeParameters.params = [];
      for (var i = 0; i < node.typeParameters.length; i++) {
        content.definition.typeParameters.params[i] = serializeThisNodeObjects(
          node.typeParameters[i],
          indentLevel,
          sourceFile
        );
      }
    }
    content.decorators = [];
    if (node.decorators) {
      for (var i = 0; i < node.decorators.length; i++) {
        content.decorators[i] = serializeThisNodeObjects(
          node.decorators[i],
          indentLevel,
          sourceFile
        );
      }
    }
    if (
      node.heritageClauses &&
      node.heritageClauses[0].types[0].typeArguments
    ) {
      content.definition.superTypeParameters = {
        type: "TSTypeParameterInstantiation",
      };
      content.definition.superTypeParameters.params = [];
      for (
        var i = 0;
        i < node.heritageClauses[0].types[0].typeArguments.length;
        i++
      ) {
        content.definition.superTypeParameters.params[i] = node.heritageClauses
          ? serializeThisNodeObjects(
              node.heritageClauses[0].types[0].typeArguments[i],
              indentLevel,
              sourceFile
            )
          : null;
      }
    }
  } else if (ts.isInterfaceDeclaration(node)) {
    //258  InterfaceDeclaration --> TSInterfaceDeclaration
    content = {
      type: tscKind2es2pandaAST[syntaxKind],
      body: { type: "TSInterfaceBody" },
    };
    content.body.body = [];
    for (var i = 0; i < node.members.length; i++) {
      content.body.body[i] = serializeThisNodeObjects(
        node.members[i],
        indentLevel,
        sourceFile
      );
    }
    content.id = serializeThisNodeObjects(node.name, indentLevel, sourceFile);
    content.extends = [];
    if (node.heritageClauses) {
      for (var i = 0; i < node.heritageClauses[0].types.length; i++) {
        content.extends[i] = serializeThisNodeObjects(
          node.heritageClauses[0],
          indentLevel,
          sourceFile,
          node,
          i
        ); //extendNum
      }
    }
    if (node.typeParameters) {
      content.typeParameters = { type: "TSTypeParameterDeclaration" };
      content.typeParameters.params = [];
      for (var i = 0; i < node.typeParameters.length; i++) {
        content.typeParameters.params[i] = serializeThisNodeObjects(
          node.typeParameters[i],
          indentLevel,
          sourceFile
        );
      }
    }
  } else if (ts.isTypeAliasDeclaration(node)) {
    // 259  TypeAliasDeclaration-->TSTypeAliasDeclaration
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.id = serializeThisNodeObjects(node.name, indentLevel, sourceFile);
    content.typeAnnotation = serializeThisNodeObjects(
      node.type,
      indentLevel,
      sourceFile
    );
    if (node.typeParameters) {
      content.typeParameters = { type: "TSTypeParameterDeclaration" };
      content.typeParameters.params = [];
      for (var i = 0; i < node.typeParameters.length; i++) {
        content.typeParameters.params[i] = serializeThisNodeObjects(
          node.typeParameters[i],
          indentLevel,
          sourceFile
        );
      }
    }
  } else if (ts.isEnumDeclaration(node)) {
    // 260 EnumDeclaration-->TSEnumDeclaration
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.id = serializeThisNodeObjects(node.name, indentLevel, sourceFile);
    content.members = [];
    for (var i = 0; i < node.members.length; i++) {
      content.members[i] = serializeThisNodeObjects(
        node.members[i],
        indentLevel,
        sourceFile
      );
    }
    content.const = false;
  } else if (ts.isModuleDeclaration(node)) {
    //   261 ModuleDeclaration -->  TSModuleDeclaration
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.id = serializeThisNodeObjects(node.name, indentLevel, sourceFile);
    if (node.body) {
      content.body = serializeThisNodeObjects(
        node.body,
        indentLevel,
        sourceFile
      );
    }
    content.declare = false;
    if (node.modifiers) {
      for (var i = 0; i < node.modifiers.length; i++) {
        if (node.modifiers[i].kind === 135) {
          content.declare = true;
        }
      }
    }
    content.global = false;
  } else if (ts.isModuleBlock(node)) {
    //262 ModuleBlock  --> TSModuleBlock
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.body = [];
    for (var i = 0; i < node.statements.length; i++) {
      content.body[i] = serializeThisNodeObjects(
        node.statements[i],
        indentLevel,
        sourceFile
      );
    }
  } else if (ts.isCaseBlock(node)) {
    //263 CaseBlock  -->
  } else if (ts.isImportEqualsDeclaration(node)) {
    //265  ImportEqualsDeclaration --> TSImportEqualsDeclaration
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.id = serializeThisNodeObjects(node.name, indentLevel, sourceFile);
    if (node.modifiers) {
      for (var i = 0; i < node.modifiers.length; i++) {
        switch (node.modifiers[i].kind) {
          case 93:
            content.isExport = true;
            break;
        }
      }
    }
    content.moduleReference = serializeThisNodeObjects(
      node.moduleReference,
      indentLevel,
      sourceFile
    );
  } else if (ts.isCaseClause(node)) {
    //  289  CaseClause  --> SwitchCase
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.test = serializeThisNodeObjects(
      node.expression,
      indentLevel,
      sourceFile
    );
    content.consequent = [];
    for (var i = 0; i < node.statements.length; i++) {
      content.consequent[i] = serializeThisNodeObjects(
        node.statements[i],
        indentLevel,
        sourceFile
      );
    }
  } else if (ts.isDefaultClause(node)) {
    //  290  DefaultClause-->SwitchCase
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.test = null;
    content.consequent = [];
    for (var i = 0; i < node.statements.length; i++) {
      content.consequent[i] = serializeThisNodeObjects(
        node.statements[i],
        indentLevel,
        sourceFile
      );
    }
  } else if (ts.isHeritageClause(node)) {
    //  291  HeritageClause-->TSInterfaceHeritage
    if (node.token === 117) {
      // 117 ImplementsKeyword
      content = { type: "TSClassImplements" };
      content.expression = serializeThisNodeObjects(
        node.types[extendNum],
        indentLevel,
        sourceFile
      );
      if (node.types[extendNum].typeArguments) {
        content.typeParameters = { type: "TSTypeParameterInstantiation" };
        content.typeParameters.params = [];
        for (var i = 0; i < node.types[extendNum].typeArguments.length; i++) {
          content.typeParameters.params[i] = serializeThisNodeObjects(
            node.types[extendNum].typeArguments[i],
            indentLevel,
            sourceFile,
            node
          );
        }
      }
    } else if (
      node.token === 94 &&
      parent !== null &&
      ts.isInterfaceDeclaration(parent)
    ) {
      //ExtendsKeyword 94   258   ==>according to 'arrayLiterals2ES6.ts'
      content = { type: "TSInterfaceHeritage" };
      content.expression = { type: "TSTypeReference" };
      content.expression.typeName = serializeThisNodeObjects(
        node.types[extendNum].expression,
        indentLevel,
        sourceFile
      );
      if (node.types[extendNum].typeArguments) {
        content.expression.typeParameters = {
          type: "TSTypeParameterInstantiation",
        };
        content.expression.typeParameters.params = [];
        for (var i = 0; i < node.types[extendNum].typeArguments.length; i++) {
          content.expression.typeParameters.params[i] =
            serializeThisNodeObjects(
              node.types[extendNum].typeArguments[i],
              indentLevel,
              sourceFile,
              node
            );
        }
      }
    } else {
      serializeThisNodeObjects(node.types[0], indentLevel, sourceFile);
    }
  } else if (ts.isCatchClause(node)) {
    //292 CatchClause -> CatchClause
    content = { type: syntaxKind };
    content.body = serializeThisNodeObjects(
      node.block,
      indentLevel,
      sourceFile
    );
    content.param = null;
    if (node.variableDeclaration) {
      const child_node = node.variableDeclaration.name;
      content.param = serializeThisNodeObjects(
        child_node,
        indentLevel,
        sourceFile
      );
    }
  } else if (ts.isPropertyAssignment(node)) {
    // 296 PropertyAssignment--> Property
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.method = false;
    content.shorthand = false;
    content.computed = false;
    content.key = serializeThisNodeObjects(node.name, indentLevel, sourceFile);
    content.value = serializeThisNodeObjects(
      node.initializer,
      indentLevel,
      sourceFile
    );
    content.kind = "init";
    if (node.name.kind === 162) {
      content.computed = true;
    }
  } else if (ts.isShorthandPropertyAssignment(node)) {
    // 297 ShorthandPropertyAssignment--> Property
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.method = false;
    content.shorthand = true;
    content.computed = false;
    content.key = serializeThisNodeObjects(node.name, indentLevel, sourceFile);
    content.value = serializeThisNodeObjects(
      node.name,
      indentLevel,
      sourceFile
    );
    content.kind = "init";
  } else if (ts.isEnumMember(node)) {
    //299  EnumMember-->TSEnumMember
    content = { type: tscKind2es2pandaAST[syntaxKind] };
    content.id = serializeThisNodeObjects(node.name, indentLevel, sourceFile);
    if (node.initializer) {
      content.initializer = serializeThisNodeObjects(
        node.initializer,
        indentLevel,
        sourceFile
      );
    }
  }

  var start_rowAndcol = getRowandColumn(start, code);
  if (ts.isSourceFile(node)) {
    start_rowAndcol = getRowandColumn(0, code);
  }
  var end_rowAndcol = getRowandColumn(end, code);
  //console.log("start:"+JSON.stringify(start_rowAndcol)+"end:"+JSON.stringify(end_rowAndcol)+"<><><><><><><><><><><>");
  if (content) {
    content.loc = { start: start_rowAndcol, end: end_rowAndcol };
  } else {
    var content = serializeChildObjects(node, indentLevel, sourceFile);
  }
  return content;
}

function RecursiveFindQuestionDot(node, flag = false) {
  if (node.questionDotToken) {
    flag = true;
    return flag;
  }
  node.forEachChild((child) => {
    flag = flag || RecursiveFindQuestionDot(child);
  });
  return flag;
}

function getRowandColumn(pos, code) {
  var lines = code.split("\n");
  var line = 1;
  var column = 1;

  for (var str of lines) {
    let len = str.length;
    //console.log("ThisLineString: "+str+" len: "+len+" pos: "+pos+" line:"+line+" column: "+column);
    if (pos > len) {
      pos -= len;
      line++;
      pos--; //  dut to : \n
    } else {
      column = column + pos;
      break;
    }
  }
  if (column === 0) column = 1;
  //console.log(" pos: "+pos+" line:"+line+" column: "+column);
  return { line: line, column: column };
}
function variableStatementHasSemicolon(pos, code) {
  var lines = code.split("\n");
  var column = 1;
  var str = "";
  for (str of lines) {
    let len = str.length;
    if (pos > len) {
      pos -= len;
      pos--; //  dut to : \n
    } else {
      column = column + pos;
      break;
    }
  }
  return str.indexOf(";");
}

function getVariableDeclarationKind(pos, code, content) {
  var lines = code.split("\n");
  var line = 1;
  var column = 1;
  var linestr = "";
  for (var i in lines) {
    len = lines[i].length;
    linestr = lines[i].slice(pos);
    //console.log("line: "+lines[i]+" len: "+len+" pos: "+pos+" line:"+line);
    if (pos > len) {
      pos -= len;
      line++;
      pos--; //  dut to : \n
    } else {
      column = column + pos;
      break;
    }
  }
  return linestr.trim().split(" ")[0];
}

function getKeys(left, right) {
  if (right === null || left == null) {
    console.log("***");
  }
  // if left or right is a string, convert to empty object
  left = typeof left !== "string" ? left : {};
  right = typeof right !== "string" ? right : {};

  // declare vars
  const keys = [...Object.keys(left)];
  const rightKeys = Object.keys(right);
  const leftKeyMap = {};

  // map out which keys in left object exist
  for (let i = 0, key; i < keys.length; i++) {
    key = keys[i];
    if (!leftKeyMap[key]) {
      leftKeyMap[key] = true;
    }
  }

  // filter out keys which are unique to right objects
  for (let j = 0, key; j < rightKeys.length; j++) {
    key = rightKeys[j];
    if (!leftKeyMap[key]) {
      keys.push(key);
    }
  }

  return keys;
}

/***** helper function: determine if an argument 'obj' is an object *****/
function isObject(obj) {
  return obj && typeof obj === "object"; // note: 'typeof obj' returns true if obj === null
}

/***** module to be exported ***/
function getDiffScope() {
  const cachedArgs = {}; // since 'getDiff' is a recursive function that calls certain left/right argument pairs multiple times, let's cache those argument pairs
  const diffs = []; // array containing all of the 'diff' entries

  /* recursive closure function */
  const getDiff = function (left = {}, right = {}, parentKeys = []) {
    // stringify JSON objects so they can be stored as keys in 'cachedArgs'
    const l_str = JSON.stringify(left);
    const r_str = JSON.stringify(right);

    // execute only if "cachedArgs[l_str][r_str]" does not already exist
    if (!cachedArgs[l_str] || !cachedArgs[l_str][r_str]) {
      var allKeys = [];
      /* get all keys in left and right objects */
      //if( right !=null){
      allKeys = getKeys(left, right);
      //}
      const parentKeyString = parentKeys.length
        ? parentKeys.join(".") + "."
        : "";

      /* loop through all left and right keys */
      for (let i = 0; i < allKeys.length; i++) {
        const key = allKeys[i];

        // when left key and right key are different, start comparison
        if (left[key] !== right[key]) {
          var lk_str = JSON.stringify(left[key]);
          var rk_str = JSON.stringify(right[key]);
          //console.log(left);
          // console.log("-------------------")
          // console.log(lk_str);
          // type: MethodDefinition  -->  name: constructor
          if (
            left.type == "ChainExpression" ||
            right.type == "ChainExpression" ||
            left.kind == "constructor" ||
            right.type == "EmptyStatement" ||
            right.type == "EmptyStatement"
          ) {
            continue;
          }
          if (
            left.type == "TSInterfaceBody" ||
            left.name == "constructor" ||
            left.type == "FunctionExpression" ||
            left.type == "ClassProperty" ||
            left.type == "ScriptFunction" ||
            left.type == "ClassDeclaration" ||
            left.type == "TSTypeParameterDeclaration"
          ) {
            if (left.type == "ClassDeclaration") {
              delete left.definition.loc;
              if (
                right.hasOwnProperty("definition") &&
                right.definition.hasOwnProperty("constructor") &&
                JSON.stringify(right.definition.constructor) === "{}"
              ) {
                delete left.definition.constructor;
              }
            } else if (left.type == "FunctionExpression") {
              //有两个FunctionExpression，一个是Node.kind匹配的，一个是自己添加在某个Node的。下面删除需要在自己添加的删除掉
              if (!right.loc) {
                //如果是我添加的，则位置信息不需要比较
                delete left.loc; // the location of {}. need to delete
                delete left.function.loc;
              }
            } else if (left.type == "ScriptFunction") {
              //如果funtion存在修饰符时，比如async，那么ScriptFunction的start位置会报错
              delete left.loc;
              delete right.loc;
            } else if (left.type == "TSTypeParameterDeclaration") {
              //es2panda的loc是自己添加的。 如 <T> ,而 tsc只考虑 T
              delete left.loc;
            } else if (left.type == "ClassProperty") {
              //它的位置只包含了name的loc
              delete left.loc.end;
              if (right.hasOwnProperty("loc")) delete right.loc.end;
            } else {
              delete left.loc;
            }
            // console.log("*******************")
            // console.log(left);
            lk_str = JSON.stringify(left[key]);
            //console.log(left);
            // console.log("######################");
            // console.log(lk_str);
          }

          // output subtraction from left key
          if (left.hasOwnProperty(key)) {
            if (isObject(left[key])) {
              // invoke "getDiff" recursively
              getDiff(left[key], right[key], [...parentKeys, key]);
              // cache left/right arg pairs
              cachedArgs[lk_str] = cachedArgs[lk_str] || {};
              cachedArgs[lk_str][rk_str] = true;
            } else {
              const leftKey =
                typeof left[key] === "string" ? `'${left[key]}'` : left[key];
              diffs.push(`-${parentKeyString}${key}:${leftKey}`);
            }
          }
          // output addition to right key
          if (right.hasOwnProperty(key)) {
            if (isObject(right[key])) {
              // invoke "getDiff" recursively
              getDiff(left[key], right[key], [...parentKeys, key]);
              // cache left/right arg pairs
              cachedArgs[lk_str] = cachedArgs[lk_str] || {};
              cachedArgs[lk_str][rk_str] = true;
            } else {
              const rightKey =
                typeof right[key] === "string" ? `'${right[key]}'` : right[key];
              diffs.push(`+${parentKeyString}${key}:${rightKey}`);
            }
          }
        }
        // end of block dealing with left and right key difference
      }
      // end of primary code (which executes only if left/right argument pair is not cached)
    }
    // recursive function returns "Set" object containing all unique diffs
    return diffs;
  };
  /* return recursive closure function */
  return getDiff;
}

function RunTask(third_path, es2abcPath, AlloutDirpath) {
  if (printPath !== "" && third_path !== printPath) {
    return;
  }

  //tscTestsRootPath = " /mnt/disk4/zhangchen/TypeScript/chen/tsc_cases/"
  // var cmdStr = "cp "+third_path +tscCasesPath
  // exec(cmdStr,function(err,stdout,stderr){
  // if(err){
  //     console.log("error: "+stderr);
  // }else{
  //     //console.log(stdout)
  // }});

  const targetname = third_path
    .split("/")
    [third_path.split("/").length - 1].replace(".ts", ""); //"numericIndexerConstraint3";
  console.log("targetname: " + targetname);

  const path = AlloutDirpath + "/" + targetname + ".json";

  //cmdStr = "/mnt/disk4/zhangchen/ohos/out/rk3568/clang_x64/exe.unstripped/clang_x64/ark/ark/es2abc --extension ts --dump-ast --parse-only "+third_path  +" > "+path
  cmdStr =
    es2abcPath +
    " --extension ts --dump-ast --parse-only " +
    third_path +
    " > " +
    path;
  exec(cmdStr, function (err, stdout, stderr) {
    if (err) {
      console.log("error: " + stderr);
    } else {
      //console.log(stdout)
    }
  });

  code = fs.readFileSync(third_path, "utf8");
  const sourceFile = ts.createSourceFile(
    third_path,
    code,
    ts.ScriptTarget.Latest
  );
  if (printPath !== "" && third_path === printPath) {
    console.log(JSON.stringify(sourceFile));
  }

  var tsc_convert_ast = printRecursiveFrom(sourceFile, 0, sourceFile, null, 0);
  if (printPath !== "" && third_path === printPath) {
    // delete the file
    var tempoutpath = AlloutDirpath + "/tmp_convert_ast.json";
    try {
      if (fs.existsSync(tempoutpath)) {
        fs.unlinkSync(tempoutpath);
      }
    } catch (err) {
      console.log(err);
    }
    fs.writeFileSync(tempoutpath, JSON.stringify(tsc_convert_ast));
  }

  const Alloutpath = AlloutDirpath + "/AlloutPath.txt";
  fs.appendFileSync(
    Alloutpath,
    "**************************************************************************************\n"
  );
  fs.appendFileSync(Alloutpath, third_path + "\n\n");
  fs.appendFileSync(Alloutpath, JSON.stringify(sourceFile) + "\n");

  //get the difference

  var es2panda_ast_json = JSON.parse(fs.readFileSync(path, "UTF-8"));
  const getDiffFunction1 = getDiffScope();
  let diffs = getDiffFunction1(es2panda_ast_json, tsc_convert_ast);

  //print the result
  if (printPath !== "" && third_path === printPath) {
    if (diffs.length == 0) {
      console.log("The two jsons are same");
    } else {
      diffs.forEach((obj) => console.log(obj));
      //for(var i=0 ; i< 100 ; i++){     console.log(diffs[i]) }
    }
  }
  //save the result
  var result = diffs.length == 0 ? ["The two jsons are same"] : diffs;
  fs.appendFileSync(Alloutpath, result.join("\n"));
  fs.appendFileSync(Alloutpath, "\n");
}

function run(fileListPath, es2abcPath, AlloutDirpath) {
  const Alloutpath = AlloutDirpath + "/AlloutPath.txt";
  try {
    if (fs.existsSync(Alloutpath)) {
      fs.unlinkSync(Alloutpath);
    }
  } catch (err) {
    console.log(err);
  }
  const fileList = fs.readFileSync(fileListPath, "UTF-8");
  for (filepath of fileList.trim().split("\n")) {
    RunTask(filepath, es2abcPath, AlloutDirpath);
  }
}

var code = "";
var printPath = ""; //"/mnt/disk4/zhangchen/ohos/third_party/typescript/tests/cases/compiler/collisionSuperAndLocalVarInMethod.ts"
const es2abcPath =
  "/mnt/disk4/zhangchen/ohos/out/rk3568/clang_x64/exe.unstripped/clang_x64/ark/ark/es2abc";
const fileListPath =
  "/mnt/disk4/zhangchen/TypeScript/chen/es2panda_ast/saveModuleDir/es2panSuccessFileList3.txt";
const AlloutDirpath =
  "/mnt/disk4/zhangchen/ohos/arkcompiler/ets_frontend/es2panda/test/parser_verification/astOutDir";
run(fileListPath, es2abcPath, AlloutDirpath);
