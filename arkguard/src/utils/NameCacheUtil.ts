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

import {FileUtils} from './FileUtils';

export interface NameCacheInfo {
  identifierNameCache?: Object,
  propertyNameCache?: Object
}

export function addIdentifierNameCache(nameCacheInfo: NameCacheInfo, identifierNameCache: Map<string, string>): void {
  nameCacheInfo.identifierNameCache = Object.fromEntries(identifierNameCache);
}

export function addPropertyNameCache(nameCacheInfo: NameCacheInfo, propertyNameCache: Map<string, string>): void {
  nameCacheInfo.propertyNameCache = Object.fromEntries(propertyNameCache);
}

export function writeNameCache(nameCache: NameCacheInfo, destFileName: string): void {
  // convert map to json string
  const nameCacheString: string = JSON.stringify(nameCache);

  // write to file
  FileUtils.writeFile(destFileName, nameCacheString);
}

export function readNameCache(filePath: string): NameCacheInfo | undefined {
  // read json string from file
  const nameCacheString: string = FileUtils.readFile(filePath);
  if (nameCacheString === undefined) {
    return undefined;
  }

  // get map from json string
  return JSON.parse(nameCacheString);
}

export function getMapFromJson(jsonObj: Object): Map<string, string> {
  if (jsonObj === undefined) {
    return new Map<string, string>();
  }

  return new Map<string, string>(Object.entries(jsonObj));
}
