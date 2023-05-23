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
  createSourceFile,
  forEachChild,
  isBinaryExpression,
  isClassDeclaration,
  isEnumDeclaration,
  isEnumMember,
  isExportAssignment,
  isExportDeclaration,
  isExportSpecifier,
  isFunctionDeclaration,
  isInterfaceDeclaration,
  isMethodDeclaration,
  isMethodSignature, isModuleDeclaration,
  isPropertyDeclaration,
  isPropertySignature, isTypeAliasDeclaration,
  isVariableDeclaration, isVariableStatement,
  Node,
  ScriptKind,
  ScriptTarget,
  SyntaxKind
} from 'typescript';

import type {
  ModifiersArray,
  SourceFile
} from 'typescript';

import fs from 'fs';
import path from 'path';
import * as FileUtil from './FileUtil';

namespace api_extractor {
  interface KeywordInfo {
    hasExport: boolean,
    hasDeclare: boolean
  }

  enum ApiType {
    API = 1,
    COMPONENT = 2,
    SDK = 3,
    PROJECT = 4
  }

  let mExportNameList: string[] = [];
  let mCurrentExportNameList: string[] = [];
  let mPropertyList: string[] = [];

  /**
   * 用于筛选带export、default的类或接口等
   */
  const getKeyword = function (modifiers: ModifiersArray): KeywordInfo {
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
      if (!mCurrentExportNameList.includes(astNode.expression.getText())) {
        mCurrentExportNameList.push(astNode.expression.getText());
      }

      return;
    }

    if (astNode.modifiers === undefined) {
      return;
    }

    let {hasExport, hasDeclare} = getKeyword(astNode.modifiers);
    if (!hasExport) {
      return;
    }

    if (astNode.name) {
      if (!mCurrentExportNameList.includes(astNode.name.getText())) {
        mCurrentExportNameList.push(astNode.name.getText());
      }

      return;
    }

    if (hasDeclare && astNode.declarationList &&
      !mCurrentExportNameList.includes(astNode.declarationList.declarations[0].name.getText())) {
      mCurrentExportNameList.push(astNode.declarationList.declarations[0].name.getText());
    }
  };

  const checkPropertyNeedVisit = function (astNode): boolean {
    if (astNode.name && !mCurrentExportNameList.includes(astNode.name.getText())) {
      return false;
    }

    if (astNode.name === undefined) {
      if (astNode.modifiers === undefined) {
        return false;
      }
      let {hasDeclare} = getKeyword(astNode.modifiers);
      if (hasDeclare && astNode.declarationList &&
        !mCurrentExportNameList.includes(astNode.declarationList.declarations[0].name.getText())) {
        return false;
      }
    }

    return true;
  };

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
      if (astNode.name !== undefined && !mPropertyList.includes(astNode.name.getText())) {
        mPropertyList.push(astNode.name.getText());
      }
    }

    astNode.forEachChild((childNode) => {
      visitChildNode(childNode);
    });
  };

  /**
   * visit ast of a file and collect api list
   * @param astNode node of ast
   */
  const visitProperty = function (astNode): void {
    if (!checkPropertyNeedVisit(astNode)) {
      return;
    }

    visitChildNode(astNode);
  };

  const visitProjectNode = function (astNode): void {
    if (astNode.modifiers) {
      let {hasExport} = getKeyword(astNode.modifiers);
      if (!hasExport) {
        return;
      }

      if (astNode.name !== undefined) {
        if (!mPropertyList.includes(astNode.name.getText())) {
          mPropertyList.push(astNode.name.getText());
        }

        if (isModuleDeclaration(astNode)) {
          astNode.forEachChild((childNode) => {
            visitProjectNode(childNode);
          });
        }

        return;
      }

      if (isVariableStatement(astNode)) {
        astNode.declarationList.forEachChild((child) => {
          if (isVariableDeclaration(child) && !mPropertyList.includes(child.name.getText())) {
            mPropertyList.push(child.name.getText());
          }
        });
      }

      return;
    }

    if (isExportAssignment(astNode)) {
      if (isBinaryExpression(astNode.expression)) {
        if (!mPropertyList.includes(astNode.expression.left.getText())) {
          mPropertyList.push(astNode.expression.left.getText());
        }
      }

      return;
    }

    if (isExportDeclaration(astNode)) {
      if (astNode.exportClause && astNode.exportClause.kind === SyntaxKind.NamedExports) {
        astNode.exportClause.forEachChild((child) => {
          if (!isExportSpecifier(child)) {
            return;
          }

          if (!mPropertyList.includes(child.name.getText())) {
            mPropertyList.push(child.name.getText());
          }
        });
      }

      return;
    }

    astNode.forEachChild((childNode) => {
      visitProjectNode(childNode);
    });
  };

  const visitProjectProperty = function (astNode): void {
    visitProjectNode(astNode);
  };

  /**
   * parse file to api list and save to json object
   * @param fileName file name of api file
   * @param apiType
   * @private
   */
  const parseFile = function (fileName: string, apiType: ApiType): void {
    const scriptKind: ScriptKind = fileName.endsWith('.ts') ? ScriptKind.TS : ScriptKind.JS;
    const sourceFile: SourceFile = createSourceFile(fileName, fs.readFileSync(fileName).toString(),
      ScriptTarget.ES2015, true, scriptKind);

    // get export name list
    switch (apiType) {
      case ApiType.SDK:
      case ApiType.COMPONENT:
        forEachChild(sourceFile, visitChildNode);
        break;
      case ApiType.API:
        mCurrentExportNameList.length = 0;
        forEachChild(sourceFile, visitExport);

        mCurrentExportNameList.forEach((value) => {
          if (!mExportNameList.includes(value)) {
            mExportNameList.push(value);
          }
        });

        forEachChild(sourceFile, visitProperty);
        mCurrentExportNameList.length = 0;
        break;
      case ApiType.PROJECT:
        if (fileName.endsWith('.d.ts')) {
          forEachChild(sourceFile, visitChildNode);
          break;
        }

        mCurrentExportNameList.length = 0;
        forEachChild(sourceFile, visitProjectProperty);
        mCurrentExportNameList.length = 0;
        break;
      default:
        break;
    }
  };

  /**
   * traverse files of openHarmony sdk api directory
   * @param apiPath openHarmony sdk api directory path
   * @param apiType
   * @private
   */
  const traverseApiFiles = function (apiPath: string, apiType: ApiType): void {
    const fileNames: string[] = fs.readdirSync(apiPath);

    for (let fileName of fileNames) {
      let filePath: string = path.join(apiPath, fileName);
      if (fs.lstatSync(filePath).isDirectory()) {
        if (fileName === 'node_modules') {
          continue;
        }

        traverseApiFiles(filePath, apiType);
        continue;
      }

      if (fs.lstatSync(filePath).isSymbolicLink()) {
        filePath = fs.readlinkSync(filePath);
        if (fs.lstatSync(filePath).isDirectory()) {
          traverseApiFiles(filePath, apiType);
          continue;
        }
      }

      if ((apiType !== ApiType.PROJECT) && !filePath.endsWith('.d.ts')) {
        continue;
      }

      if (apiType === ApiType.PROJECT && !filePath.endsWith('.ts') && !filePath.endsWith('.js')) {
        continue;
      }

      parseFile(filePath, apiType);
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
    mExportNameList.length = 0;
    mPropertyList.length = 0;

    // visit api directory
    const apiPath: string = path.join(sdkPath, (isEts ? 'ets' : 'js'), version, 'api');
    traverseApiFiles(apiPath, ApiType.API);

    // visit component directory if ets
    if (isEts) {
      const componentPath: string = path.join(sdkPath, 'ets', version, 'component');
      traverseApiFiles(componentPath, ApiType.COMPONENT);
    }

    const reservedProperties: string[] = [...mExportNameList, ...mPropertyList];
    mExportNameList.length = 0;
    mPropertyList.length = 0;

    FileUtil.writeToFile(reservedProperties, version, isEts, outputDir);
  }

  /**
   * parse common project or file to extract exported api list
   * @return reserved api names
   */
  export function parseCommonProject(projectPath): string[] {
    mPropertyList.length = 0;

    if (fs.lstatSync(projectPath).isFile()) {
      if (projectPath.endsWith('.ts') || projectPath.endsWith('.js')) {
        parseFile(projectPath, ApiType.PROJECT);
      }
    } else {
      traverseApiFiles(projectPath, ApiType.PROJECT);
    }

    const reservedProperties: string[] = [...mPropertyList];
    mPropertyList.length = 0;

    return reservedProperties;
  }

  /**
   * parse api of third party libs like libs in node_modules
   * @param libPath
   */
  export function parseThirdPartyLibs(libPath): string[] {
    mPropertyList.length = 0;

    if (fs.lstatSync(libPath).isFile()) {
      if (libPath.endsWith('.ts') || libPath.endsWith('.js')) {
        parseFile(libPath, ApiType.SDK);
      }
    } else {
      traverseApiFiles(libPath, ApiType.SDK);
    }

    const reservedProperties: string[] = [...mPropertyList];
    mPropertyList.length = 0;

    return reservedProperties;
  }
}

export = api_extractor;
