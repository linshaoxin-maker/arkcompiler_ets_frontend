/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

import fs from 'fs'

const combinationConfigPath = './combination_config.json'

const defaultConfig = {
  "mCompact": false,
  "mRemoveComments": false,
  "mOutputDir": "../local",
  "mDisableHilog": false,
  "mDisableConsole": false,
  "mSimplify": false,
  "mNameObfuscation": {
      "mEnable": true,
      "mNameGeneratorType": 1,
      "mDictionaryList": [],
      "mRenameProperties": false,
      "mKeepStringProperty": false,
      "mTopLevel": false
  },
  "mExportObfuscation": false,
  "mEnableSourceMap": false,
  "mEnableNameCache": false
}

function run(cmd) {
  
}

function combineOptions(enbleOptions, result) {
  const result = [];

  function combine(currentCombination, start) {
    result.push([...currentCombination]);

    for (let i = start; i < array.length; i++) {
      currentCombination.push(enbleOptions[i]);
      combine(currentCombination, i + 1);
      currentCombination.pop();
    }
  }

  combine([], 0);
  return result;
}

function readConfig() {
  const configs = JSON.parse(fs.readFileSync(combinationConfigPath, 'utf-8'));
  for (let key in configContent) {
    const enbleOptions = configs.enbleOptions;
    const testsFolder = configs.testsFolder;
    const outputRootDir = configs.outputRootDir;
    const arkguardConfig = combineOptions(enbleOptions, outputRootDir);
    const cmd = ``
  }
}
function main() {
  
}