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

import fs from 'fs';
import path from 'path';

import { ArkObfuscator } from '../ArkObfuscator';
import { collectResevedFileNameInIDEConfig, MergedConfig, ObConfigResolver, readNameCache } from './ConfigResolver';
import { type IOptions } from '../configs/IOptions';

export const printerConfig = {
  // Print obfuscation time&memory usage of all files and obfuscation processes
  mFilesPrinter: false,
  // Print time&memory usage of a single file obfuscation in transform processes
  mSingleFilePrinter: false,
  // Print sum up time of transform processes during obfuscation
  mSumPrinter: false,
  // Output path of printer
  mOutputPath: '',
};

export function initObfuscationConfig(projectConfig: any, arkProjectConfig: any, logger: any): void {
  const obConfig: ObConfigResolver = new ObConfigResolver(projectConfig, logger, true);
  const mergedObConfig: MergedConfig = obConfig.resolveObfuscationConfigs();
  const isHarCompiled: boolean = projectConfig.compileHar;
  if (mergedObConfig.options.disableObfuscation) {
    return;
  }

  if (mergedObConfig.options.enableFileNameObfuscation) {
    const ohPackagePath = path.join(projectConfig.modulePath, 'oh-package.json5');
    const entryArray = arkProjectConfig.entryArrayForObf;
    const reservedFileNamesInIDEconfig = collectResevedFileNameInIDEConfig(
      ohPackagePath,
      projectConfig,
      arkProjectConfig.modulePathMap,
      entryArray,
    );
    mergedObConfig.reservedFileNames.push(...reservedFileNamesInIDEconfig);
  }
  arkProjectConfig.obfuscationMergedObConfig = mergedObConfig;

  arkProjectConfig.arkObfuscator = initArkGuardConfig(
    projectConfig.obfuscationOptions?.obfuscationCacheDir,
    logger,
    mergedObConfig,
    isHarCompiled,
  );
}

function initArkGuardConfig(
  obfuscationCacheDir: string | undefined,
  logger: any,
  mergedObConfig: MergedConfig,
  isHarCompiled: boolean,
): ArkObfuscator {
  const arkguardConfig: IOptions = {
    mCompact: mergedObConfig.options.compact,
    mDisableConsole: mergedObConfig.options.removeLog,
    mSimplify: false,
    mRemoveComments: true,
    mNameObfuscation: {
      mEnable: true,
      mNameGeneratorType: 1,
      mReservedNames: mergedObConfig.reservedNames,
      mRenameProperties: mergedObConfig.options.enablePropertyObfuscation,
      mReservedProperties: mergedObConfig.reservedPropertyNames,
      mKeepStringProperty: !mergedObConfig.options.enableStringPropertyObfuscation,
      mTopLevel: mergedObConfig.options.enableToplevelObfuscation,
      mReservedToplevelNames: mergedObConfig.reservedGlobalNames,
      mUniversalReservedProperties: mergedObConfig.universalReservedPropertyNames,
      mUniversalReservedToplevelNames: mergedObConfig.universalReservedGlobalNames,
    },
    mRemoveDeclarationComments: {
      mEnable: mergedObConfig.options.removeComments,
      mReservedComments: mergedObConfig.keepComments,
    },
    mEnableSourceMap: true,
    mEnableNameCache: true,
    mRenameFileName: {
      mEnable: mergedObConfig.options.enableFileNameObfuscation,
      mNameGeneratorType: 1,
      mReservedFileNames: mergedObConfig.reservedFileNames,
    },
    mExportObfuscation: mergedObConfig.options.enableExportObfuscation,
    mPerformancePrinter: printerConfig,
    mKeepFileSourceCode: {
      mKeepSourceOfPaths: new Set(),
      mkeepFilesAndDependencies: new Set(),
    },
  };

  const arkObfuscator: ArkObfuscator = new ArkObfuscator();
  arkObfuscator.init(arkguardConfig);
  if (mergedObConfig.options.applyNameCache && mergedObConfig.options.applyNameCache.length > 0) {
    readNameCache(mergedObConfig.options.applyNameCache, logger);
  } else {
    if (obfuscationCacheDir) {
      const defaultNameCachePath: string = path.join(obfuscationCacheDir, 'nameCache.json');
      if (fs.existsSync(defaultNameCachePath)) {
        readNameCache(defaultNameCachePath, logger);
      }
    }
  }
  return arkObfuscator;
}