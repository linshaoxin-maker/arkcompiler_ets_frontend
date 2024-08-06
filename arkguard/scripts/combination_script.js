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

const fs = require('fs')
const path = require('path')

const combinationConfigPath = path.join(__dirname, './combination_config.json');

const defaultConfig = {
  "mCompact": false,
  "mRemoveComments": false,
  "mOutputDir": "../local",
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

const configAlias = {
  "mCompact": "comp",
  "mRemoveComments": false,
  "mOutputDir": "../local",
  "mDisableConsole": "con",
  "mSimplify": false,
  "mNameObfuscation": {
      "mEnable": "localVar",
      "mNameGeneratorType": 1,
      "mDictionaryList": [],
      "mRenameProperties": "prop",
      "mKeepStringProperty": "strProp",
      "mTopLevel": "top"
  },
  "mExportObfuscation": "export",
  "mEnableSourceMap": false,
  "mEnableNameCache": false
}

function concatConfigsAlias(options, configAlias) {
  const keys = Object.keys(target);
  const result = ''
  for (const key of keys) {
    if (configAlias[key] == 'object') {
      result += configAlias[key];
    } else {
      return undefined; // Return undefined if the key doesn't exist
    }
  }
  return result;
}

function run(optionsCombinations, testsFolder) {
  const outpurDir = {"mOutputDir": testsFolder};
  for (let options of optionsCombinations) {
    const configAlias = concatConfigsAlias(options, '');
    const updatedConfig =  {...defaultConfig, ...options, ...outpurDir}
  }
}

function generateCombinations(obj) {
  const result = [{}]; // Initialize with the empty object
  const keys = Object.keys(obj);

  function combine(current, remainingKeys) {
    if (remainingKeys.length === 0) return;

    const key = remainingKeys[0];
    const value = obj[key];

    if (typeof value === 'object' && !Array.isArray(value)) {
      const subKeys = Object.keys(value);
      const subCombinations = [];

      subKeys.forEach(subKey => {
        const subValue = value[subKey];
        const currentSubCombinations = subCombinations.slice();

        currentSubCombinations.forEach(subComb => {
          const newComb = { ...subComb, [subKey]: subValue };
          subCombinations.push(newComb);
        });

        subCombinations.push({ [subKey]: subValue });
      });
      console.log('subCombinations---',subCombinations)
      subCombinations.forEach(subComb => {
        const newComb = { ...current, [key]: subComb };
        result.push(newComb);
        combine(newComb, remainingKeys.slice(1));
      });
    } else {
      const newComb = { ...current, [key]: value };
      result.push(newComb);
      combine(newComb, remainingKeys.slice(1));
    }

    combine(current, remainingKeys.slice(1));
  }

  combine({}, keys);
  return result;
}

function readConfig() {
  const configs = JSON.parse(fs.readFileSync(combinationConfigPath, 'utf-8'));
  console.log(configs)
  for (let key in configs) {
    const enableOptions = configs[key].enableOptions;
    const testsFolder = configs[key].testsFolder;
    const outputRootDir = configs[key].outputRootDir;
    const optionsCombinations = generateCombinations(enableOptions);
    console.log("混淆选项组合数量", optionsCombinations.length);
    run(optionsCombinations, testsFolder)
  }
}
function main() {
  readConfig();
}

main();