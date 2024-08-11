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
const resultStatistics = require('./execute_result_statistics');
const { execSync } = require('child_process');
const { dir } = require('console');

const combinationConfigPath = path.join(__dirname, './combination_config.json');
const indentation = 2;
const defaultConfig = {
  "mCompact": false,
  "mRemoveComments": false,
  "mOutputDir": "",
  "mDisableConsole": false,
  "mSimplify": false,
  "mNameObfuscation": {
      "mEnable": true,
      "mNameGeneratorType": 1,
      "mDictionaryList": [],
      "mRenameProperties": false,
      "mKeepStringProperty": true,
      "mTopLevel": false
  },
  "mExportObfuscation": false,
  "mEnableSourceMap": false,
  "mEnableNameCache": false,
  "mKeepFileSourceCode": {
    "mKeepSourceOfPaths": [],
    "mkeepFilesAndDependencies": []
  },
  "mRenameFileName": {
      "mEnable": false,
      "mNameGeneratorType": 1,
      "mReservedFileNames": [],
      "mOhmUrlUseNormalized": false
  }
}

const configAlias = {
  "mCompact": "comp",
  "mRemoveComments": "rmComments",
  "mDisableConsole": "con",
  "mNameObfuscation": {
      "mEnable": "localVar",
      "mRenameProperties": "prop",
      "mKeepStringProperty": "strProp",
      "mTopLevel": "top"
  },
  "mExportObfuscation": "export",
  "mEnableSourceMap": "sourcemap",
  "mEnableNameCache": "namecache"
}

const getAliasValue = (config, target) => {
  for (const key in target) {
    if (target.hasOwnProperty(key) && config.hasOwnProperty(key)) {
      if (typeof target[key] === 'object' && !Array.isArray(target[key])) {
        return getAliasValue(config[key], target[key]);
      } else {
        return config[key];
      }
    }
  }
  return undefined;
};

function run(inputDirs, configPath, type) {
  for (let inputDir of inputDirs){
    const inputAbsDir = path.join(__dirname, inputDir)
    const dirs = fs.readdirSync(inputAbsDir);
    for (let oneCasePath of dirs) {
      const inputPath = path.join(inputAbsDir, oneCasePath)
      const command = `node --loader=ts-node/esm src/cli/SecHarmony.ts ${inputPath} --config-path ${configPath} --cases-flag ${type}`;
      try {
        execSync(command, { encoding: 'utf-8' });
      } catch (error) {
        console.error('Error:', error.message);
        console.error('Stdout:', error.stdout.toString());
        console.error('Stderr:', error.stderr.toString());
      }
    }
  }
}

const getAliasFromConfig = (config, input) => {
  let result = '';

  const recursiveSearch = (configPart, inputPart) => {
    for (const key in inputPart) {
      if (inputPart.hasOwnProperty(key) && configPart.hasOwnProperty(key)) {
        const inputVal = inputPart[key];
        const configVal = configPart[key];

        if (typeof inputVal === 'object' && !Array.isArray(inputVal)) {
          recursiveSearch(configVal, inputVal);
        } else {
          const connector = result == '' ? '' : '+'
          result += (connector + configVal)
        }
      }
    }
  };

  recursiveSearch(config, input);
  return result === '' ? 'default' : result;;
}

function mergeDeep(target, source) {
  for (const key in source) {
    if (source[key] instanceof Object && key in target) {
      Object.assign(source[key], mergeDeep(target[key], source[key]));
    }
  }

  Object.assign(target || {}, source);
  return target;
}

function generateObfConfigg(optionsCombinations, inputDirs, outputDir) {
  const outputAbsDir = path.join(__dirname, outputDir);
  for (let i = 0; i < optionsCombinations.length; i++) {
    const options = optionsCombinations[i];
    const aliasStr = getAliasFromConfig(configAlias, options);
    const outpurDirForCurrentOption = path.join(outputAbsDir, aliasStr);
    const tempConfigPath = path.join(__dirname, aliasStr + '_config.json');
    const mergeConfig = mergeDeep(structuredClone(defaultConfig), structuredClone(options))
    const updatedConfig =  Object.assign(mergeConfig, {mOutputDir: outpurDirForCurrentOption})
    fs.writeFileSync(tempConfigPath, JSON.stringify(updatedConfig, null, indentation));
    run(inputDirs, tempConfigPath, 'combinations');
    // delete temp config file
    fs.unlinkSync(tempConfigPath);
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
      // console.log('subCombinations---',subCombinations)
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

function parseConfigAndRun() {
  const configs = JSON.parse(fs.readFileSync(combinationConfigPath, 'utf-8'));
  for (let key in configs) {
    const enableOptions = configs[key].enableOptions;
    const inputDirs = configs[key].inputDirs;
    const outputDir = configs[key].outputDir;
    const optionsCombinations = generateCombinations(enableOptions);
    console.log("混淆选项组合数量", optionsCombinations.length);
    console.log("混淆选项组合", optionsCombinations);
    generateObfConfigg(optionsCombinations, inputDirs, outputDir)
  }
}

function countResult() {
  const testDirectory = path.resolve('./test/combinations_local');
  resultStatistics.runTestAndCount(testDirectory);
}
function main() {
  parseConfigAndRun();
  countResult();
}

main();