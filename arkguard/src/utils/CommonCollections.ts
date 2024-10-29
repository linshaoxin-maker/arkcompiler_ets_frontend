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

import {FileUtils} from './FileUtils';
import { MergedConfig } from '../initialization/ConfigResolver';
import { ApiExtractor } from '../common/ApiExtractor'
import * as crypto from 'crypto';
import * as ts from 'typescript';
import fs from 'fs';
import path from 'path';

// This records the collections related to property obfuscation.
export namespace PropCollections {
  // whether enable property obfuscation
  export let enablePropertyObfuscation: boolean = false;
  // global mangled properties table used by all files in a project
  export let globalMangledTable: Map<string, string> = new Map();
  // used for property cache
  export let historyMangledTable: Map<string, string> = undefined;
  // the white list of property
  export let reservedProperties: Set<string> = new Set();
  export let universalReservedProperties: RegExp[] = [];
  // saved generated property name
  export let newlyOccupiedMangledProps: Set<string> = new Set();
  export let mangledPropsInNameCache: Set<string> = new Set();

  // When the module is compiled, call this function to clear the collections associated with property obfuscation.
  export function clearPropsCollections(): void {
    globalMangledTable.clear();
    historyMangledTable?.clear();
    reservedProperties.clear();
    universalReservedProperties = [];
    newlyOccupiedMangledProps.clear();
    mangledPropsInNameCache.clear();
  }
}

// This records the collections related to whitelists
export namespace UnobfuscationCollections {
  // printKeptName: by user configuration, it decides whether to print unobfuscation names and whitelists.
  export let printKeptName: boolean = false;
  // whitelist
  export let reservedSdkApiForProp: Set<string> = new Set();
  export let reservedSdkApiForGlobal: Set<string> = new Set();
  export let reservedSdkApiForLocal: Set<string> = new Set();
  export let reservedStruct: Set<string> = new Set();
  export let reservedLangForProperty: Set<string> = new Set();
  // declare global {}
  // In the above syntax, 'global' is named '__global' in tsc.
  export let reservedLangForTopLevel: Set<string> = new Set(['__global', 'default']); // Will not add new elements anymore
  export let reservedExportName: Set<string> = new Set();
  export let reservedExportNameAndProp: Set<string> = new Set();
  export let reservedStrProp: Set<string> = new Set();
  export let reservedEnum: Set<string> = new Set();
  
  // The mapping between the unobfuscated names and their reasons.
  export let unobfuscatedPropMap: Map<string, Set<string>> = new Map();
  export let unobfuscatedNamesMap: Map<string, Set<string>> = new Map();

  // The mapping between wildcards and regular expressions
  export let reservedWildcardMap: Map<RegExp, string> = new Map();

  export function clear(): void {
    printKeptName = false;
    reservedSdkApiForProp.clear();
    reservedSdkApiForGlobal.clear();
    reservedSdkApiForLocal.clear();
    reservedStruct.clear();
    reservedLangForProperty.clear();
    reservedExportName.clear();
    reservedExportNameAndProp.clear();
    reservedStrProp.clear();
    reservedEnum.clear();
    unobfuscatedPropMap.clear();
    unobfuscatedNamesMap.clear();
  }
}

export namespace LocalVariableCollections {
  export let reservedStruct: Set<string> = new Set(); 
  export let reservedConfig: Set<string> = new Set(); // Obtain the name from the user-configured .d.ts file
  export let reservedLangForLocal: Set<string> = new Set(['this', '__global']); // Will not add new elements anymore

  export function clear(): void {
    reservedStruct.clear();
    reservedConfig.clear();
  }
}

export function addToSet<T>(targetSet: Set<T>, sourceSet: Set<T>): void {
  sourceSet.forEach(element => targetSet.add(element));
}

export function arrayToSet<T>(array: T[]): Set<T> {
  return new Set(array);
}

export function setToArray<T>(set: Set<T>): T[] {
  return Array.from(set);
}

export function areSetsEqual<T>(set1: Set<T>, set2: Set<T>): boolean {
  if (set1.size !== set2.size) {
    return false;
  }
  for (const item of set1) {
    if (!set2.has(item)) {
      return false;
    }
  }
  return true;
}

export namespace ProjectCollections {
  export const SOURCE_FILE_PATHS: string = 'sourceFilePaths.cache';
  export const TRANSFORMED_PATH: string = 'transformed';
  export const FILE_NAMES_MAP: string = 'transformedFileNamesMap.json';
  export const FILE_WHITE_LISTS: string = 'fileWhiteLists.json';
  export const PROJECT_WHITE_LIST: string = 'projectWhiteList.json';

  /**
   * Informantion of build files
   */
  export interface ModuleInfo {
    content: string,
    /**
     * the path in build cache dir
     */
    buildFilePath: string,
    /**
     * the `originSourceFilePath` relative to project root dir.
     */
    relativeSourceFilePath: string,
    /**
     * the origin source file path will be set with rollup moduleId when obfuscate intermediate js source code,
     * whereas be set with tsc node.fileName when obfuscate intermediate ts source code.
     */
    originSourceFilePath?: string,
    rollupModuleId?: string
  }

  /**
   * We have these structure to stroe project white list and file white lists,
   * project white list is merged by each file white list.
   * 
   * We have keepinfo and reservedinfo in white lists:
   *  KeepInfo: names that we cannot obfuscate.
   *  ReservedInfo: names that we cannot obufscate as.
   * 
   * ProjectWhiteList
   * ├── projectKeepInfo: ProjectKeepInfo
   * │   ├── propertyNames: Set<string>
   * │   └── globalNames: Set<string>
   * └── projectReservedInfo: ProjectReservedInfo
   *     ├── enumProperties: Set<string>
   *     └── propertyParams: Set<string>
 
   * FileWhiteList
   * ├── fileKeepInfo: FileKeepInfo
   * │   ├── keepSymbol?: KeepInfo (optional)
   * │   │   ├── propertyNames: Set<string>
   * │   │   └── globalNames: Set<string>
   * │   ├── keepAsConsumer?: KeepInfo (optional)
   * │   │   ├── propertyNames: Set<string>
   * │   │   └── globalNames: Set<string>
   * │   ├── structProperties: Set<string>
   * │   ├── exportedNames: KeepInfo
   * │   │   ├── propertyNames: Set<string>
   * │   │   └── globalNames: Set<string>
   * │   ├── enumProperties: Set<string>
   * │   └── stringProperties: Set<string>
   * └── fileReservedInfo: FileReservedInfo
   *     ├── enumProperties: Set<string>
   *     └── propertyParams: Set<string>
   */
  export interface FileContent {
    moduleInfo: ModuleInfo;
    previousStageSourceMap?: ts.RawSourceMap;
  }

  export interface KeepInfo {
    propertyNames: Set<string>;
    globalNames: Set<string>;
  }

  export interface FileKeepInfo {
    keepSymbol?: KeepInfo;
    keepAsConsumer?: KeepInfo;
    structProperties: Set<string>;
    exportedNames: KeepInfo;
    enumProperties: Set<string>;
    stringProperties: Set<string>;
  }

  export interface FileReservedInfo {
    enumProperties: Set<string>;
    propertyParams: Set<string>;
  }

  export interface FileWhiteList {
    fileKeepInfo: FileKeepInfo;
    fileReservedInfo: FileReservedInfo;
  }

  export interface ProjectKeepInfo {
    propertyNames: Set<string>;
    globalNames: Set<string>;
  }

  export interface ProjectReservedInfo {
    enumProperties: Set<string>;
    propertyParams: Set<string>;
  }

  export interface ProjectWhiteList {
    projectKeepInfo: ProjectKeepInfo;
    projectReservedInfo: ProjectReservedInfo;
  }

  // initialize temp collections, will be cleared after collecting each file
  export let keepSymbolTemp: KeepInfo = {
    propertyNames: new Set<string>,
    globalNames:new Set<string>
  }
  export let keepAsConsumerTemp: KeepInfo = {
    propertyNames: new Set<string>,
    globalNames:new Set<string>
  }
  export let structPropertiesTemp: Set<string> = new Set();
  export let exportedNamesTemp: KeepInfo = {
    propertyNames: new Set<string>,
    globalNames: new Set<string>
  }
  export let enumPropertiesTemp: Set<string> = new Set();
  export let stringPropertiesTemp: Set<string> = new Set();
  export let reservedInfoTemp: FileReservedInfo = {
    enumProperties: new Set<string>,
    propertyParams: new Set<string>
  }

  // clear names we collect from transformed source file
  export function clearCollectionsOftransformed(): void {
    structPropertiesTemp.clear();
    enumPropertiesTemp.clear();
  }

  // clear names we collect from origin source file
  export function clearAllTempCollections(): void {
    keepSymbolTemp.propertyNames.clear();
    keepSymbolTemp.globalNames.clear();
    keepAsConsumerTemp.propertyNames.clear();
    keepAsConsumerTemp.globalNames.clear();
    structPropertiesTemp.clear();
    exportedNamesTemp.propertyNames.clear();
    exportedNamesTemp.globalNames.clear();
    enumPropertiesTemp.clear();
    stringPropertiesTemp.clear();
    reservedInfoTemp.enumProperties.clear();
    reservedInfoTemp.propertyParams.clear();
  }

  /**
   * This class is used to manage project white lists.
   * Used to collect white list of each file and merge them into project white lists.
   */
  export class ProjectWhiteListManager {
    // projectWhiteListManager to manage project white lists
    private static projectWhiteListManager: ProjectWhiteListManager;
  
    // if atKeep is enabled
    private enableAtKeep: boolean;

    // if we need to emit consumer, used only when atKeep is enabled
    private shouldEmitConsumer: boolean;

    // obfuscation cache
    private cachePath: string;

    // cache path for file white lists
    private fileWhiteListsCachePath: string;

    // cache path for project white lists
    private projectWhiteListCachePath: string;

    // if it is incremental compliation
    private isIncremental: boolean = false;

    // if we should reObfuscate all files
    shouldReObfuscate: boolean = false;

    // white list of each file
    private fileWhiteLists: Map<string, FileWhiteList>; 

    private constructor(obfuscationConfig: MergedConfig, shouldEmitConsumer: boolean) {
      this.enableAtKeep = obfuscationConfig.options.enableAtKeep;
      this.shouldEmitConsumer = shouldEmitConsumer;

      // initialize fileWhiteLists
      this.fileWhiteLists = new Map();
    }

    // initialize FileWhiteList, should be called after initialize obfuscation config
    public static initFileWhiteList(obfuscationConfig?: MergedConfig, shouldEmitConsumer?: boolean) {
      if (!ProjectWhiteListManager.projectWhiteListManager) {
        ProjectWhiteListManager.projectWhiteListManager = new ProjectWhiteListManager(obfuscationConfig, shouldEmitConsumer);
      }
      return ProjectWhiteListManager.projectWhiteListManager
    }

    // enable cache, should be called when incremental cache is enabled
    public static initCachePath(cachePath: string, isIncreMental: boolean) {
      ProjectWhiteListManager.projectWhiteListManager.cachePath = cachePath;
      ProjectWhiteListManager.projectWhiteListManager.fileWhiteListsCachePath = path.join(cachePath, FILE_WHITE_LISTS);
      ProjectWhiteListManager.projectWhiteListManager.projectWhiteListCachePath = path.join(cachePath, PROJECT_WHITE_LIST);
      ProjectWhiteListManager.projectWhiteListManager.isIncremental = isIncreMental;
    }

    public static getProjectWhiteListManager(): ProjectWhiteListManager {
      return ProjectWhiteListManager.projectWhiteListManager;
    }

    // create one fileWhilteList obj, with keepSymbol & keepAsConsumer if atKeep is enabled
    public createFileWhiteList(): FileWhiteList {
      if (ProjectWhiteListManager.projectWhiteListManager.enableAtKeep) {
        return {
          fileKeepInfo: {
            keepSymbol: {
              propertyNames: new Set<string>(),
              globalNames: new Set<string>()
            },
            keepAsConsumer: {
              propertyNames: new Set<string>(),
              globalNames: new Set<string>()
            },
            structProperties: new Set<string>,
            exportedNames: {
              propertyNames: new Set<string>(),
              globalNames: new Set<string>()
            },
            enumProperties: new Set<string>,
            stringProperties: new Set<string>
          },
          fileReservedInfo: {
            enumProperties: new Set<string>,
            propertyParams: new Set<string>
          }
        }
      } else {
        return {
          fileKeepInfo: {
            structProperties: new Set<string>,
            exportedNames: {
              propertyNames: new Set<string>(),
              globalNames: new Set<string>()
            },
            enumProperties: new Set<string>,
            stringProperties: new Set<string>
          },
          fileReservedInfo: {
            enumProperties: new Set<string>,
            propertyParams: new Set<string>
          }
        }
      }
    }

    // collect transformed file white lists
    public collectTransformedFileWhiteLists(path: string) {
      const originPath = FileUtils.toUnixPath(path);
      const fileWhiteList: ProjectCollections.FileWhiteList = this.createFileWhiteList();
      addToSet(fileWhiteList.fileKeepInfo.structProperties, structPropertiesTemp);
      addToSet(fileWhiteList.fileKeepInfo.enumProperties, enumPropertiesTemp);
      ProjectWhiteListManager.projectWhiteListManager.fileWhiteLists.set(originPath, fileWhiteList);
      ProjectCollections.clearCollectionsOftransformed();
    }

    // collect origin file white lists
    public collectOriginFileWhiteLists(path: string) {
      let fileWhiteList: ProjectCollections.FileWhiteList | undefined =
        ProjectWhiteListManager.projectWhiteListManager.fileWhiteLists.get(path);
      if (!fileWhiteList) {
        fileWhiteList = this.createFileWhiteList();
        ProjectWhiteListManager.projectWhiteListManager.fileWhiteLists.set(path, fileWhiteList);
      }
      // file keep info
      if (ProjectWhiteListManager.projectWhiteListManager.enableAtKeep) {
        addToSet(fileWhiteList.fileKeepInfo.keepSymbol.propertyNames, keepSymbolTemp.propertyNames);
        addToSet(fileWhiteList.fileKeepInfo.keepSymbol.globalNames, keepSymbolTemp.globalNames);
        addToSet(fileWhiteList.fileKeepInfo.keepAsConsumer.propertyNames, keepAsConsumerTemp.propertyNames);
        addToSet(fileWhiteList.fileKeepInfo.keepAsConsumer.globalNames, keepAsConsumerTemp.globalNames);
      }
      addToSet(fileWhiteList.fileKeepInfo.structProperties, stringPropertiesTemp);
      addToSet(fileWhiteList.fileKeepInfo.exportedNames.propertyNames, exportedNamesTemp.propertyNames);
      addToSet(fileWhiteList.fileKeepInfo.exportedNames.globalNames, exportedNamesTemp.globalNames);
      addToSet(fileWhiteList.fileKeepInfo.stringProperties, stringPropertiesTemp);

      // file reserved info
      addToSet(fileWhiteList.fileReservedInfo.enumProperties, reservedInfoTemp.enumProperties);
      addToSet(fileWhiteList.fileReservedInfo.propertyParams, reservedInfoTemp.propertyParams);

      ProjectCollections.clearAllTempCollections();
    }

    private readFileWhiteLists(filePath: string): Map<string, FileWhiteList> {
      const fileContent = fs.readFileSync(filePath, 'utf8');
      const parsed: any = JSON.parse(fileContent);
     
      const map = new Map<string, FileWhiteList>();
      for (const key in parsed) {
        if (parsed.hasOwnProperty(key)) {
          const fileKeepInfo: FileKeepInfo = {
            keepSymbol: parsed[key].fileKeepInfo.keepSymbol
              ? {
                  propertyNames: arrayToSet(parsed[key].fileKeepInfo.keepSymbol.propertyNames),
                  globalNames: arrayToSet(parsed[key].fileKeepInfo.keepSymbol.globalNames),
                }
              : undefined,
            keepAsConsumer: parsed[key].fileKeepInfo.keepAsConsumer
              ? {
                  propertyNames: arrayToSet(parsed[key].fileKeepInfo.keepAsConsumer.propertyNames),
                  globalNames: arrayToSet(parsed[key].fileKeepInfo.keepAsConsumer.globalNames),
                }
              : undefined,
            structProperties: arrayToSet(parsed[key].fileKeepInfo.structProperties),
            exportedNames: {
              propertyNames: arrayToSet(parsed[key].fileKeepInfo.exportedNames.propertyNames),
              globalNames: arrayToSet(parsed[key].fileKeepInfo.exportedNames.globalNames),
            },
            enumProperties: arrayToSet(parsed[key].fileKeepInfo.enumProperties),
            stringProperties: arrayToSet(parsed[key].fileKeepInfo.stringProperties),
          };
     
          const fileReservedInfo: FileReservedInfo = {
            enumProperties: arrayToSet(parsed[key].fileReservedInfo.enumProperties),
            propertyParams: arrayToSet(parsed[key].fileReservedInfo.propertyParams),
          };
     
          map.set(key, { fileKeepInfo, fileReservedInfo });
        }
      }

      return map;
    }

    private writeFileWhiteLists(filePath: string, fileWhiteLists: Map<string, FileWhiteList>): void {
      const jsonData: any = {};
      for (const [key, value] of fileWhiteLists) {
        jsonData[key] = {
          fileKeepInfo: {
            keepSymbol: value.fileKeepInfo.keepSymbol
              ? {
                  propertyNames: setToArray(value.fileKeepInfo.keepSymbol?.propertyNames),
                  globalNames: setToArray(value.fileKeepInfo.keepSymbol?.globalNames),
                }
              : undefined,
            keepAsConsumer: value.fileKeepInfo.keepAsConsumer
              ? {
                  propertyNames: setToArray(value.fileKeepInfo.keepAsConsumer?.propertyNames),
                  globalNames: setToArray(value.fileKeepInfo.keepAsConsumer?.globalNames),
                }
              : undefined,
            structProperties: setToArray(value.fileKeepInfo.structProperties),
            exportedNames: {
              propertyNames: setToArray(value.fileKeepInfo.exportedNames.propertyNames),
              globalNames: setToArray(value.fileKeepInfo.exportedNames.globalNames),
            },
            enumProperties: setToArray(value.fileKeepInfo.enumProperties),
            stringProperties: setToArray(value.fileKeepInfo.stringProperties),
          },
          fileReservedInfo: {
            enumProperties: setToArray(value.fileReservedInfo.enumProperties),
            propertyParams: setToArray(value.fileReservedInfo.propertyParams),
          },
        };
      }
     
      const jsonString = JSON.stringify(jsonData, null, 2); // 格式化 JSON 字符串，缩进 2 个空格
      fs.writeFileSync(filePath, jsonString, 'utf8');
    }

    private readProjectWhiteList(filePath: string): ProjectWhiteList {
      const fileContent = fs.readFileSync(filePath, 'utf8');
      const parsed: any = JSON.parse(fileContent);
     
      const projectKeepInfo: ProjectKeepInfo = {
        propertyNames: new Set(parsed.projectKeepInfo.propertyNames),
        globalNames: new Set(parsed.projectKeepInfo.globalNames),
      };
     
      const projectReservedInfo: ProjectReservedInfo = {
        enumProperties: new Set(parsed.projectReservedInfo.enumProperties),
        propertyParams: new Set(parsed.projectReservedInfo.propertyParams),
      };
     
      return {
        projectKeepInfo,
        projectReservedInfo,
      };
    }

    private writeProjectWhiteList(filePath: string, projectWhiteList: ProjectWhiteList): void {
      const jsonData: any = {
        projectKeepInfo: {
          propertyNames: Array.from(projectWhiteList.projectKeepInfo.propertyNames),
          globalNames: Array.from(projectWhiteList.projectKeepInfo.globalNames),
        },
        projectReservedInfo: {
          enumProperties: Array.from(projectWhiteList.projectReservedInfo.enumProperties),
          propertyParams: Array.from(projectWhiteList.projectReservedInfo.propertyParams),
        },
      };
     
      const jsonString = JSON.stringify(jsonData, null, 2);
      fs.writeFileSync(filePath, jsonString, 'utf8');
    }

    public updateFileWhiteLists(deletedFilePath: Set<string>) {
      const lastFileWhiteLists: Map<string, FileWhiteList> = this.readFileWhiteLists(ProjectWhiteListManager.projectWhiteListManager.fileWhiteListsCachePath);

      deletedFilePath.forEach(path => {
        lastFileWhiteLists.delete(path);
      });
      ProjectWhiteListManager.projectWhiteListManager.fileWhiteLists.forEach((value, key) => {
        lastFileWhiteLists.set(key, value);
      })
      ProjectWhiteListManager.projectWhiteListManager.writeFileWhiteLists(ProjectWhiteListManager.projectWhiteListManager.fileWhiteListsCachePath, lastFileWhiteLists);
      ProjectWhiteListManager.projectWhiteListManager.fileWhiteLists = lastFileWhiteLists;
    }

    public createProjectWhiteList(fileWhiteLists: Map<string, FileWhiteList>): ProjectWhiteList {
      const projectKeepInfo: ProjectKeepInfo = {
        propertyNames: new Set(),
        globalNames: new Set()
      };

      const projectReservedInfo: ProjectReservedInfo = {
        enumProperties: new Set(),
        propertyParams: new Set()
      };

      const projectWhiteList = {
        projectKeepInfo: projectKeepInfo,
        projectReservedInfo: projectReservedInfo
      };

      fileWhiteLists.forEach((fileWhiteList) => {
       // 1. collect fileKeepInfo
       // collect keepSymbol
       fileWhiteList.fileKeepInfo.keepSymbol?.globalNames.forEach((globalName) => {
        projectWhiteList.projectKeepInfo.globalNames.add(globalName);
       });
       fileWhiteList.fileKeepInfo.keepSymbol?.propertyNames.forEach((propertyName) => {
        projectWhiteList.projectKeepInfo.propertyNames.add(propertyName);
       });

       // collect keepAsConsumer
       fileWhiteList.fileKeepInfo.keepAsConsumer?.globalNames.forEach((globalName) => {
        projectWhiteList.projectKeepInfo.globalNames.add(globalName);
       });
       fileWhiteList.fileKeepInfo.keepSymbol?.propertyNames.forEach((propertyName) => {
        projectWhiteList.projectKeepInfo.propertyNames.add(propertyName);
       });

       // collect structProperties
       fileWhiteList.fileKeepInfo.structProperties.forEach((propertyName) => {
        projectWhiteList.projectKeepInfo.propertyNames.add(propertyName);
       })

       // collect exoportedNames
       fileWhiteList.fileKeepInfo.exportedNames.globalNames.forEach((globalName) => {
        projectWhiteList.projectKeepInfo.globalNames.add(globalName);
       })
       fileWhiteList.fileKeepInfo.exportedNames.propertyNames.forEach((propertyName) => {
        projectWhiteList.projectKeepInfo.propertyNames.add(propertyName);
       })

       // collect enumProperties
       fileWhiteList.fileKeepInfo.enumProperties.forEach((propertyName) => {
        projectWhiteList.projectKeepInfo.propertyNames.add(propertyName);
       })

       // collect stringProperties
       fileWhiteList.fileKeepInfo.stringProperties.forEach((propertyName) => {
        projectWhiteList.projectKeepInfo.propertyNames.add(propertyName);
       })

       // 2. collect fileReservedInfo
       // collect enumProperties
       fileWhiteList.fileReservedInfo.enumProperties.forEach((enumPropertyName) => {
        projectWhiteList.projectReservedInfo.enumProperties.add(enumPropertyName);
       })

       // collect propertyParams
       fileWhiteList.fileReservedInfo.propertyParams.forEach((propertyParam) => {
        projectWhiteList.projectReservedInfo.propertyParams.add(propertyParam);
       })
      })

      return projectWhiteList;
    }

    // judge whether project white lists changed
    private areProjectWhiteListsEqual(whiteList1: ProjectWhiteList, whiteList2: ProjectWhiteList): boolean {
      const projectKeepInfoEqual = areSetsEqual(
        whiteList1.projectKeepInfo.propertyNames,
        whiteList2.projectKeepInfo.propertyNames
      ) && areSetsEqual(
        whiteList1.projectKeepInfo.globalNames,
        whiteList2.projectKeepInfo.globalNames
      );

      const projectReservedInfoEqual = areSetsEqual(
        whiteList1.projectReservedInfo.enumProperties,
        whiteList2.projectReservedInfo.enumProperties
      ) && areSetsEqual(
        whiteList1.projectReservedInfo.propertyParams,
        whiteList2.projectReservedInfo.propertyParams
      );

      return projectKeepInfoEqual && projectReservedInfoEqual;
    }

    // We only scan updated files or newly created files in incremental compliation,
    // so we need to update the UnobfuscationCollections and reserved names use all file white lists.
    // This should be called in incremental compliation after we updated file white list.
    private updateUnobfuscationCollections(): void {
      this.fileWhiteLists.forEach((fileWhiteList) => {
        fileWhiteList.fileKeepInfo.structProperties.forEach((structProperty) => {
          UnobfuscationCollections.reservedStruct.add(structProperty);
        });
        fileWhiteList.fileKeepInfo.enumProperties.forEach((enumProperty) => {
          UnobfuscationCollections.reservedEnum.add(enumProperty);
        });
        fileWhiteList.fileKeepInfo.exportedNames.globalNames.forEach((globalName) => {
          UnobfuscationCollections.reservedExportName.add(globalName);
        });
        fileWhiteList.fileKeepInfo.exportedNames.propertyNames.forEach((propertyName) => {
          UnobfuscationCollections.reservedExportNameAndProp.add(propertyName);
        });
        fileWhiteList.fileKeepInfo.stringProperties.forEach((stringProperty) => {
          UnobfuscationCollections.reservedStrProp.add(stringProperty);
        });
        fileWhiteList.fileReservedInfo.propertyParams.forEach((propertyParam) => {
          ApiExtractor.mConstructorPropertySet.add(propertyParam);
        })
      })
    }

    private updateProjectWhiteList(): void {
      const lastProjectWhiteList: ProjectWhiteList = this.readProjectWhiteList(this.projectWhiteListCachePath);
      const newestProjectWhiteList: ProjectWhiteList = this.createProjectWhiteList(this.fileWhiteLists);
      if (this.areProjectWhiteListsEqual(lastProjectWhiteList, newestProjectWhiteList)) {
        return;
      } else {
        this.writeProjectWhiteList(this.projectWhiteListCachePath ,newestProjectWhiteList);
        this.shouldReObfuscate = true;
      }
    }

    public createOrUpdateWhiteListCaches(deletedFilePath?: Set<string>): void {
      if (!this.isIncremental) {
        this.writeFileWhiteLists(this.fileWhiteListsCachePath, this.fileWhiteLists);
        const projectWhiteList = this.createProjectWhiteList(this.fileWhiteLists);
        this.writeProjectWhiteList(this.projectWhiteListCachePath, projectWhiteList);
      } else {
        this.updateFileWhiteLists(deletedFilePath);
        this.updateProjectWhiteList();
        this.updateUnobfuscationCollections();
      }
      this.fileWhiteLists.clear();
    }
  }

  /**
   * This class is used to manage sourceFilePath.json file.
   * Will be created when initialize arkObfuscator.
   */
  export class FilePathManager {
    // stores all source files paths
    private sourceFilePaths: Set<string>;

    // files deleted in incremental build
    private deletedSourceFilePaths: Set<string>;

    // cache path of sourceFilePaths.json file
    private cachePath: string;

    constructor(cachePath: string) {
      this.cachePath = cachePath;
      this.sourceFilePaths = new Set();
      this.deletedSourceFilePaths = new Set();
    }

    private setSourceFilePaths(sourceFilePaths: Set<string>) {
      sourceFilePaths.forEach((filePath) => {
        if (!FileUtils.isReadableFile(filePath) || !ApiExtractor.isParsableFile(filePath)) {
          return;
        }
        this.sourceFilePaths.add(filePath);
      })
    }

    private writeSourceFilePaths() {
      const content = Array.from(this.sourceFilePaths).join('\n');
      FileUtils.writeFile(this.cachePath, content);
    }

    // Update sourceFilePaths.json and get deleted file list
    private updateSourceFilePaths() {
      const cacheContent = FileUtils.readFile(this.cachePath).split('\n');
      const cacheSet = new Set(cacheContent);

      for (const path of cacheSet) {
        if (!this.sourceFilePaths.has(path)) {
          this.deletedSourceFilePaths.add(path);
        }
      }

      this.writeSourceFilePaths();
    }

    // Create or update sourceFilePaths.json
    public createOrUpdateSourceFilePaths(sourceFilePaths: Set<string>) {
      this.setSourceFilePaths(sourceFilePaths);
      if (this.isIncreMental()) {
        this.updateSourceFilePaths();
      } else {
        this.writeSourceFilePaths();
      }
    }

    public getDeletedSourceFilePaths(): Set<string> {
      return this.deletedSourceFilePaths;
    }

    public isIncreMental(): boolean {
      return fs.existsSync(this.cachePath);
    }
  }

  /**
   * This class is used to manage transfromed file content.
   * Will be created when initialize arkObfuscator.
   */
  export class FileContentManager {
    // path that stores all transfromed file content
    private cachePath: string;

    // path of fileNamesMap
    private fileNamesMapPath: string;

    // stores [originpath -> transformed cache path]
    private fileNamesMap: Map<string, string>;

    // if it is incremental compilation
    private isIncremental: boolean;

    constructor(cachePath: string, isIncremental: boolean) {
      this.cachePath = cachePath;
      this.fileNamesMapPath = path.join(this.cachePath, FILE_NAMES_MAP);
      this.fileNamesMap = new Map();
      this.isIncremental = isIncremental;
      FileUtils.createDirectory(this.cachePath);
    }

    //generate hash from filePah
    private generatePathHash(filePath: string): string {
      const hash = crypto.createHash('sha256');
      hash.update(filePath);
      return hash.digest('hex');
    }

    // generate new file name for transfromed sourcefiles
    // generated from origin filePath's hash & time stamp
    private generateFileName(filePath: string): string {
      const hash = this.generatePathHash(filePath);
      const baseName = hash.slice(0, 16);
      const timestamp = Date.now().toString();
      return `${baseName}_${timestamp}`;
    }

    // delete a set of files, used in incremental compliation if we have deleted some files
    public deleteFileContent(deletedSourceFilePaths: Set<string>): void {
      deletedSourceFilePaths.forEach((filePath) => {
        this.deleteSingleFile(filePath);
      })
    }

    // delete one single file
    private deleteSingleFile(deletedSourceFilePath: string): void {
      if (this.fileNamesMap.has(deletedSourceFilePath)) {
        const transformedFilePath: string = this.fileNamesMap.get(deletedSourceFilePath);
        this.fileNamesMap.delete(deletedSourceFilePath);
        FileUtils.deleteFile(transformedFilePath);
      }
    }

    public readFileNamesMap() {
      const jsonObject = FileUtils.readFileAsJson(this.fileNamesMapPath);
      this.fileNamesMap = new Map<string, string>(Object.entries(jsonObject));
    }

    public writeFileNamesMap(): void {
      const jsonObject = Object.fromEntries(this.fileNamesMap.entries());
      const jsonString = JSON.stringify(jsonObject, null, 2);
      FileUtils.writeFile(this.fileNamesMapPath, jsonString);
      this.fileNamesMap.clear();
    }

    public readFileContent(transformedFilePath: string): FileContent {
      const fileContentJson = FileUtils.readFile(transformedFilePath);
      const fileConstent: FileContent = JSON.parse(fileContentJson);
      return fileConstent;
    }

    private writeFileContent(filePath: string, fileContent: FileContent) {
      const jsonString = JSON.stringify(fileContent, null, 2);
      FileUtils.writeFile(filePath, jsonString);
    }

    // updte file content for newly created files or updated files
    public updateFileContent(fileContent: FileContent) {
      const originPath = fileContent.moduleInfo.originSourceFilePath;
      if (this.isIncremental) {
        this.deleteSingleFile(originPath);
      }
      const fileName = this.generateFileName(originPath);
      const transformedFilePath = path.join(this.cachePath, fileName);
      this.fileNamesMap.set(originPath, transformedFilePath);
      this.writeFileContent(transformedFilePath, fileContent);
    }
  }
}