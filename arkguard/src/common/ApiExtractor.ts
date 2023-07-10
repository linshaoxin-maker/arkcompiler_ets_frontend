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

import {isStructDeclaration, ModifiersArray, SourceFile} from 'typescript';
import {
  createSourceFile,
  forEachChild,
  isBinaryExpression,
  isClassDeclaration, isClassExpression,
  isEnumDeclaration,
  isEnumMember,
  isExportAssignment,
  isExportDeclaration,
  isExportSpecifier,
  isFunctionDeclaration,
  isIdentifier,
  isInterfaceDeclaration,
  isMethodDeclaration,
  isMethodSignature,
  isObjectLiteralExpression,
  isPropertyDeclaration,
  isPropertySignature,
  isTypeAliasDeclaration,
  isVariableDeclaration,
  isVariableStatement,
  ScriptKind,
  ScriptTarget,
  SyntaxKind
} from 'typescript';

import fs from 'fs';
import path from 'path';
import {getClassProperties, getEnumProperties, getObjectProperties} from '../utils/OhsUtil';

export namespace ApiExtractor {
  interface KeywordInfo {
    hasExport: boolean,
    hasDeclare: boolean
  }

  export enum ApiType {
    API = 1,
    COMPONENT = 2,
    PROJECT_DEPENDS = 3,
    PROJECT = 4
  }

  let mCurrentExportNameSet: Set<string> = new Set<string>();
  export let mPropertySet: Set<string> = new Set<string>();

  /**
   * filter classes or interfaces with export, default, etc
   */
  const getKeyword = function (modifiers: ModifiersArray): KeywordInfo {
    if (modifiers === undefined) {
      return {hasExport: false, hasDeclare: false};
    }

    let hasExport: boolean = false;
    let hasDeclare: boolean = false;

    for (const modifier of modifiers) {
      if (modifier.kind === SyntaxKind.ExportKeyword) {
        hasExport = true;
      }

      if (modifier.kind === SyntaxKind.DeclareKeyword) {
        hasDeclare = true;
      }
    }

    return {hasExport: hasExport, hasDeclare: hasDeclare};
  };

  /**
   * get export name list
   * @param astNode
   */
  const visitExport = function (astNode): void {
    if (isExportAssignment(astNode)) {
      if (!mCurrentExportNameSet.has(astNode.expression.getText())) {
        mCurrentExportNameSet.add(astNode.expression.getText());
        mPropertySet.add(astNode.expression.getText());
      }

      return;
    }

    let {hasExport, hasDeclare} = getKeyword(astNode.modifiers);
    if (!hasExport) {
      return;
    }

    if (astNode.name) {
      if (!mCurrentExportNameSet.has(astNode.name.getText())) {
        mCurrentExportNameSet.add(astNode.name.getText());
        mPropertySet.add(astNode.name.getText());
      }

      return;
    }

    if (hasDeclare && astNode.declarationList &&
      !mCurrentExportNameSet.has(astNode.declarationList.declarations[0].name.getText())) {
      mCurrentExportNameSet.add(astNode.declarationList.declarations[0].name.getText());
      mPropertySet.add(astNode.declarationList.declarations[0].name.getText());
    }
  };

  const checkPropertyNeedVisit = function (astNode): boolean {
    if (astNode.name && !mCurrentExportNameSet.has(astNode.name.getText())) {
      return false;
    }

    if (astNode.name === undefined) {
      let {hasDeclare} = getKeyword(astNode.modifiers);
      if (hasDeclare && astNode.declarationList &&
        !mCurrentExportNameSet.has(astNode.declarationList.declarations[0].name.getText())) {
        return false;
      }
    }

    return true;
  };

  /**
   * used only in oh sdk api extract or api of xxx.d.ts declaration file
   * @param astNode
   */
  const visitChildNode = function (astNode): void {
    if (isClassDeclaration(astNode) ||
      isInterfaceDeclaration(astNode) ||
      isEnumDeclaration(astNode) ||
      isTypeAliasDeclaration(astNode) ||
      isPropertySignature(astNode) ||
      isMethodSignature(astNode) ||
      isFunctionDeclaration(astNode) ||
      isMethodDeclaration(astNode) ||
      isPropertyDeclaration(astNode) ||
      isEnumMember(astNode) ||
      isExportSpecifier(astNode) ||
      isVariableDeclaration(astNode)) {
      if (astNode.name !== undefined && !mPropertySet.has(astNode.name.getText())) {
        mPropertySet.add(astNode.name.getText());
      }
    }

    astNode.forEachChild((childNode) => {
      visitChildNode(childNode);
    });
  };

  /**
   * visit ast of a file and collect api list
   * used only in oh sdk api extract
   * @param astNode node of ast
   */
  const visitPropertyAndName = function (astNode): void {
    if (!checkPropertyNeedVisit(astNode)) {
      return;
    }

    visitChildNode(astNode);
  };

  /**
   * extract project export name
   * - export {xxx, xxx};
   * - export {xxx as xx, xxx as xx};
   * - export default function/class/...{};
   * - export class xxx{}
   * - ...
   * @param astNode
   */
  const visitProjectExport = function (astNode): void {
    if (isExportAssignment(astNode)) {
      // let xxx; export default xxx = a;
      if (isBinaryExpression(astNode.expression)) {
        if (isObjectLiteralExpression(astNode.expression.right)) {
          getObjectProperties(astNode.expression.right, mPropertySet);
          return;
        }

        if (isClassExpression(astNode.expression.right)) {
          getClassProperties(astNode.expression.right, mPropertySet);
        }

        return;
      }

      // export = xxx; The xxx here can't be obfuscated
      // export default yyy; The yyy here can be obfuscated
      if (isIdentifier(astNode.expression)) {
        if (!mCurrentExportNameSet.has(astNode.expression.getText())) {
          mCurrentExportNameSet.add(astNode.expression.getText());
          mPropertySet.add(astNode.expression.getText());
        }
        return;
      }

      if (isObjectLiteralExpression(astNode.expression)) {
        getObjectProperties(astNode.expression, mPropertySet);
      }

      return;
    }

    if (isExportDeclaration(astNode)) {
      if (astNode.exportClause) {
        if (astNode.exportClause.kind === SyntaxKind.NamedExports) {
          astNode.exportClause.forEachChild((child) => {
            if (!isExportSpecifier(child)) {
              return;
            }

            if (child.propertyName) {
              mCurrentExportNameSet.add(child.propertyName.getText());
            }

            let exportName = child.name.getText();
            mPropertySet.add(exportName);
            mCurrentExportNameSet.add(exportName);
          });
        }

        if (astNode.exportClause.kind === SyntaxKind.NamespaceExport) {
          mPropertySet.add(astNode.exportClause.name.getText());
          return;
        }
      }
      return;
    }

    let {hasExport} = getKeyword(astNode.modifiers);
    if (!hasExport) {
      forEachChild(astNode, visitProjectExport);
      return;
    }

    if (astNode.name) {
      if (!mCurrentExportNameSet.has(astNode.name.getText())) {
        mCurrentExportNameSet.add(astNode.name.getText());
        mPropertySet.add(astNode.name.getText());
      }

      forEachChild(astNode, visitProjectExport);
      return;
    }

    if (isClassDeclaration(astNode)) {
      getClassProperties(astNode, mPropertySet);
      return;
    }

    if (isVariableStatement(astNode)) {
      astNode.declarationList.forEachChild((child) => {
        if (isVariableDeclaration(child) && !mCurrentExportNameSet.has(child.name.getText())) {
          mCurrentExportNameSet.add(child.name.getText());
          mPropertySet.add(child.name.getText());
        }
      });

      return;
    }

    forEachChild(astNode, visitProjectExport);
  };

  /**
   * extract the class, enum, and object properties of the export in the project before obfuscation
   * class A{};
   * export = A; need to be considered
   * export = namespace;
   * This statement also needs to determine whether there is an export in the namespace, and namespaces are also allowed in the namespace
   * @param astNode
   */
  const visitProjectNode = function (astNode): void {
    if ((isClassDeclaration(astNode) || isStructDeclaration(astNode)) && astNode.name && mCurrentExportNameSet.has(astNode.name.text)) {
      getClassProperties(astNode, mPropertySet);
      return;
    }

    // collect export enum structure properties
    if (isEnumDeclaration(astNode) && mCurrentExportNameSet.has(astNode.name.getText())) {
      getEnumProperties(astNode, mPropertySet);
      return;
    }

    if (isVariableDeclaration(astNode) && mCurrentExportNameSet.has(astNode.name.getText())) {
      if (astNode.initializer && isObjectLiteralExpression(astNode.initializer)) {
        getObjectProperties(astNode.initializer, mPropertySet);
        return;
      }

      if (astNode.initializer && isClassExpression(astNode.initializer)) {
        getClassProperties(astNode.initializer, mPropertySet);
      }

      return;
    }

    forEachChild(astNode, visitProjectNode);
  };

  /**
   * parse file to api list and save to json object
   * @param fileName file name of api file
   * @param apiType
   * @private
   */
  const parseFile = function (fileName: string, apiType: ApiType): void {
    const sourceFile: SourceFile = createSourceFile(fileName, fs.readFileSync(fileName).toString(), ScriptTarget.ES2015, true);

    // get export name list
    switch (apiType) {
      case ApiType.COMPONENT:
        forEachChild(sourceFile, visitChildNode);
        break;
      case ApiType.API:
        mCurrentExportNameSet.clear();
        forEachChild(sourceFile, visitExport);

        forEachChild(sourceFile, visitPropertyAndName);
        mCurrentExportNameSet.clear();
        break;
      case ApiType.PROJECT_DEPENDS:
      case ApiType.PROJECT:
        if (fileName.endsWith('.d.ts') || fileName.endsWith('.d.ets')) {
          forEachChild(sourceFile, visitChildNode);
          break;
        }

        mCurrentExportNameSet.clear();
        forEachChild(sourceFile, visitProjectExport);
        forEachChild(sourceFile, visitProjectNode);
        mCurrentExportNameSet.clear();
        break;
      default:
        break;
    }
  };

  const projectExtensions: string[] = ['.ets', '.ts', '.js'];
  const projectDependencyExtensions: string[] = ['.d.ets', '.d.ts', '.ets', '.ts', '.js'];
  /**
   * traverse files of  api directory
   * @param apiPath api directory path
   * @param apiType
   * @private
   */
  export const traverseApiFiles = function (apiPath: string, apiType: ApiType): void {
    let fileNames: string[] = [];
    if (fs.statSync(apiPath).isDirectory()) {
      fileNames = fs.readdirSync(apiPath);
      for (let fileName of fileNames) {
        let filePath: string = path.join(apiPath, fileName);
        if (fs.statSync(filePath).isDirectory()) {
          traverseApiFiles(filePath, apiType);
          continue;
        }
        const suffix: string = path.extname(filePath);
        if ((apiType !== ApiType.PROJECT) && !projectDependencyExtensions.includes(suffix)) {
          continue;
        }

        if (apiType === ApiType.PROJECT && !projectExtensions.includes(suffix)) {
          continue;
        }
        parseFile(filePath, apiType);
      }
    } else {
      parseFile(apiPath, apiType);
    }
  };

  /**
   * desc: parse openHarmony sdk to get api list
   * @param version version of api, e.g. version 5.0.1.0 for api 9
   * @param sdkPath sdk real path of openHarmony
   * @param isEts true for ets, false for js
   * @param outputDir: sdk api output directory
   */
  export function parseOhSdk(sdkPath: string, version: string, isEts: boolean, outputDir: string): void {
    mPropertySet.clear();

    // visit api directory
    const apiPath: string = path.join(sdkPath, (isEts ? 'ets' : 'js'), version, 'api');
    traverseApiFiles(apiPath, ApiType.API);

    // visit component directory if ets
    if (isEts) {
      const componentPath: string = path.join(sdkPath, 'ets', version, 'component');
      traverseApiFiles(componentPath, ApiType.COMPONENT);
    }

    // visit the UI conversion API
    const uiConversionPath: string = path.join(sdkPath, (isEts ? 'ets' : 'js'), version,
      'build-tools', 'ets-loader', 'lib', 'pre_define.js');
    extractStringsFromFile(uiConversionPath);

    const reservedProperties: string[] = [...mPropertySet.values()];
    mPropertySet.clear();

    writeToFile(reservedProperties, path.join(outputDir, 'propertiesReserved.json'));
  }

  export function extractStringsFromFile(filePath: string): void {
    let collections: string[] = [];
    const fileContent = fs.readFileSync(filePath, 'utf-8');
    const regex = /"([^"]*)"/g;
    const matches = fileContent.match(regex);

    if (matches) {
      collections = matches.map(match => match.slice(1, -1));
    }

    collections.forEach(name => mPropertySet.add(name));
  }

  /**
   * parse common project or file to extract exported api list
   * @return reserved api names
   */
  export function parseCommonProject(projectPath): string[] {
    mPropertySet.clear();

    if (fs.lstatSync(projectPath).isFile()) {
      if (projectPath.endsWith('.ets') || projectPath.endsWith('.ts') || projectPath.endsWith('.js')) {
        parseFile(projectPath, ApiType.PROJECT);
      }
    } else {
      traverseApiFiles(projectPath, ApiType.PROJECT);
    }

    const reservedProperties: string[] = [...mPropertySet];
    mPropertySet.clear();

    return reservedProperties;
  }

  /**
   * parse api of third party libs like libs in node_modules
   * @param libPath
   */
  export function parseThirdPartyLibs(libPath): string[] {
    mPropertySet.clear();

    if (fs.lstatSync(libPath).isFile()) {
      if (libPath.endsWith('.ets') || libPath.endsWith('.ts') || libPath.endsWith('.js')) {
        parseFile(libPath, ApiType.PROJECT_DEPENDS);
      }
    } else {
      const filesAndfolders = fs.readdirSync(libPath);
      for (let subPath of filesAndfolders) {
        traverseApiFiles(path.join(libPath, subPath), ApiType.PROJECT_DEPENDS);
      }
    }

    const reservedProperties: string[] = [...mPropertySet];
    mPropertySet.clear();

    return reservedProperties;
  }

  /**
   * save api json object to file
   * @private
   */
  export function writeToFile(reservedProperties: string[], outputPath: string): void {
    let str: string = JSON.stringify(reservedProperties, null, '\t');
    fs.writeFileSync(outputPath, str);
  }
}

