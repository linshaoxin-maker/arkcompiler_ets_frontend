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

import { ApiExtractor } from "../ArkObfuscator";
import type { ToplevelObf } from "../common/ToplevelObf";
import type { INameGenerator } from "../generator/INameGenerator";
import { Scope } from "./ScopeAnalyzer";
//import { globalMangledTable, historyMangledTable, reservedProperties } from "../transformers/rename/RenamePropertiesTransformer";

export function isValueInMap(map: Map<string, string>, targetValue: string): boolean {
  let isInMap: boolean = false;
  for (const value of map.values()) {
    if (value === targetValue) {
      isInMap = true;
      break;
    }
  }
  return isInMap;
}

export function getNewNameForToplevel(original: string, generator: INameGenerator, topvelObfIns: ToplevelObf, exportObfuscation: boolean, propertyObfuscation: boolean,
  globalMangledTable, historyMangledTable, reservedProperties, scope, manager) {
    let mangledName = undefined;
    while (!mangledName) {
      let tmpName = generator.getName();
      const isExistedName: boolean = isInExsitngList(tmpName, original, topvelObfIns, exportObfuscation, propertyObfuscation, globalMangledTable, historyMangledTable, reservedProperties)
      if (isExistedName) {
        continue;
      }
      if (scope && scope.exportNames && scope.exportNames.has(tmpName)) {
        continue;
      }
      if (searchMangledInParent(scope, tmpName)) {
        continue;
      }
      if ((propertyObfuscation && manager.getRootScope().constructorReservedParams.has(tmpName)) ||
      ApiExtractor.mConstructorPropertySet?.has(tmpName)) {
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
    const isExistedName: boolean = isInExsitngList(tmpName, original, topvelObfIns, exportObfuscation, propertyObfuscation, globalMangledTable, historyMangledTable, reservedProperties)
    if (isExistedName) {
      continue;
    }
    mangledName = tmpName;
  }
  return mangledName;
}

function isInExsitngList(tmpName: string, original, topvelObfIns: ToplevelObf | undefined, exportObfuscation: boolean, propertyObfuscation: boolean,
  globalMangledTable, historyMangledTable, reservedProperties): boolean {
  if (tmpName === original) {
    return true;
  }
  if (topvelObfIns) {
    if (topvelObfIns.reservedToplevelNames.has(tmpName) || isValueInMap(topvelObfIns.toplevelNameMangledTable, tmpName)) {
      return true;
    }

    if (topvelObfIns.historyToplevelMangledTable && isValueInMap(topvelObfIns.historyToplevelMangledTable, tmpName)) {
      return true;
    }
  }

  if (exportObfuscation && propertyObfuscation) {
    if (reservedProperties.has(tmpName)) {
      return true;
    }
    if (isValueInMap(globalMangledTable, tmpName)) {
      return true;
    }

    if (historyMangledTable && isValueInMap(historyMangledTable, tmpName)) {
      return true;
    }
  }
  return false;
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