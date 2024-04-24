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

import { ApiExtractor } from '../ArkObfuscator';
import type { ToplevelObf } from '../common/ToplevelObf';
import type { INameGenerator } from '../generator/INameGenerator';
import type { Scope } from './ScopeAnalyzer';

export function getNewNameForToplevel(original: string, generator: INameGenerator, topvelObfIns: ToplevelObf, exportObfuscation: boolean, propertyObfuscation: boolean,
  globalMangledTable, historyMangledTable, reservedProperties, scope: Scope, currFileConstructorParams: Set<string>) {
    let mangledName = undefined;
    while (!mangledName) {
      let tmpName = generator.getName();
      if (tmpName === original) {
        continue;
      }
      const isExistedToplevelName: boolean = isInExsitngToplevelNames(tmpName, topvelObfIns);
      if (isExistedToplevelName) {
        continue;
      }
      if (exportObfuscation && propertyObfuscation && isInExsitngPropertyNames(tmpName, globalMangledTable, historyMangledTable, reservedProperties)) {
        continue;
      }
      if (scope && scope.exportNames && scope.exportNames.has(tmpName)) {
        continue;
      }
      if (searchMangledInParent(scope, tmpName)) {
        continue;
      }
      if ((propertyObfuscation && currFileConstructorParams.has(tmpName)) || ApiExtractor.mConstructorPropertySet?.has(tmpName)) {
        continue;
      }
      mangledName = tmpName;
    }
    return mangledName;
}

export function getNewNameForProerty(original: string, generator: INameGenerator, topvelObfIns: ToplevelObf, exportObfuscation: boolean, propertyObfuscation: boolean,
  globalMangledTable, historyMangledTable, reservedProperties) {
  let mangledName = undefined;
  while (!mangledName) {
    let tmpName = generator.getName();
    if (tmpName === original) {
      continue;
    }
    const isExistedToplevelName: boolean = isInExsitngToplevelNames(tmpName, topvelObfIns);
    if (isExistedToplevelName) {
      continue;
    }
    if (exportObfuscation && propertyObfuscation && isInExsitngPropertyNames(tmpName, globalMangledTable, historyMangledTable, reservedProperties)) {
       continue;
    }
    mangledName = tmpName;
  }
  return mangledName;
}

function isInExsitngToplevelNames(tmpName: string, topvelObfIns: ToplevelObf | undefined): boolean {
  if (topvelObfIns) {
    return topvelObfIns.reservedToplevelNames.has(tmpName) ||
    isValueInMap(topvelObfIns.toplevelNameMangledTable, tmpName) ||
    (topvelObfIns.historyToplevelMangledTable && isValueInMap(topvelObfIns.historyToplevelMangledTable, tmpName));
  }
  return false;
}

function isInExsitngPropertyNames(tmpName: string, globalMangledTable: Map<string, string>, historyMangledTable: Map<string, string>,
  reservedProperties: Set<string>): boolean {
  return reservedProperties.has(tmpName) || 
    isValueInMap(globalMangledTable, tmpName) || 
    (historyMangledTable && isValueInMap(historyMangledTable, tmpName));
}

function isValueInMap(map: Map<string, string>, targetValue: string): boolean {
  let isInMap: boolean = false;
  for (const value of map.values()) {
    if (value === targetValue) {
      isInMap = true;
      break;
    }
  }
  return isInMap;
}

export function searchMangledInParent(scope: Scope | undefined, name: string): boolean {
  let found: boolean = false;
  let parentScope = scope;
  while (parentScope) {
    if (parentScope.mangledNames.has(name)) {
      found = true;
      break;
    }

    parentScope = parentScope.parent;
  }

  return found;
}