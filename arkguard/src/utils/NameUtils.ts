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
import { needToBeReserved } from './TransformUtil';

export function getNewNameForProerty(original: string, generator: INameGenerator, exportObfuscation: boolean, propertyObfuscation: boolean,
  globalMangledTable: Map<string, string>, historyMangledTable, reservedProperties) {
  let mangledName = undefined;
  while (!mangledName) {
    let tmpName = generator.getName();
    if (tmpName === original) {
      continue;
    }

    if ((exportObfuscation || propertyObfuscation) && isInExsitngPropertyNames(tmpName, globalMangledTable, historyMangledTable, reservedProperties)) {
       continue;
    }
    mangledName = tmpName;
    globalMangledTable.set(original, mangledName);
  }
  return mangledName;
}

function isInExsitngPropertyNames(tmpName: string, globalMangledTable: Map<string, string>, historyMangledTable: Map<string, string>,
  reservedProperties: Set<string>): boolean {
  return reservedProperties.has(tmpName) || 
    isValueInMap(globalMangledTable, tmpName) || 
    (historyMangledTable && isValueInMap(historyMangledTable, tmpName));
}

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


export function getPropertyName(original: string, generator, reservedProperties, universalReservedProperties, historyMangledTable, globalMangledTable): string {
  const historyName: string = historyMangledTable?.get(original);
  let mangledName: string = historyName ? historyName : globalMangledTable.get(original);

  while (!mangledName) {
    let tmpName = generator.getName();
    if (needToBeReserved(reservedProperties, universalReservedProperties, tmpName) ||
      tmpName === original) {
      continue;
    }

    let isInGlobalMangledTable = false;
    for (const value of globalMangledTable.values()) {
      if (value === tmpName) {
        isInGlobalMangledTable = true;
        break;
      }
    }

    if (isInGlobalMangledTable) {
      continue;
    }

    let isInHistoryMangledTable = false;
    if (historyMangledTable) {
      for (const value of historyMangledTable.values()) {
        if (value === tmpName) {
          isInHistoryMangledTable = true;
          break;
        }
      }
    }

    if (!isInHistoryMangledTable) {
      mangledName = tmpName;
      break;
    }
  }
  globalMangledTable.set(original, mangledName);
  return mangledName;
}