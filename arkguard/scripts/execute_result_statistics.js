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

const fs = require('fs');
const path = require('path');
const diff = require('diff');
const { execSync } = require('child_process');
import { Extension } from '../src/common/type';
import { FileUtils } from '../src/utils/FileUtils';

const testDirectory = path.resolve('./test/local');
const NonExecutableFile = ['name_as_export_api_1.ts', 'name_as_import_api_1.ts', 'ohmurl_test.ts', 'ohmurl_test_new.ts'];

function runTest(filePath) {
  try {
    const command = `node ./node_modules/ts-node/dist/bin.js ${filePath}`;
    execSync(command);
  } catch (error) {
    console.error(`Test case ${filePath} failed:`, error);
    return false;
  }
  return true;
}
let successCount = 0;
let failureCount = 0;
let contentcomparationSuccessCount = 0;
let contentcomparationFailureCount = 0;
const failedFiles = [];
const contentComparisionFailureFiles = [];

function runTestsInDirectory(directoryPath) {
  const files = fs.readdirSync(directoryPath);

  for (const file of files) {
    const filePath = path.join(directoryPath, file);

    if (fs.statSync(filePath).isDirectory()) {
      runTestsInDirectory(filePath);
    } else if (filePath.includes('assert-expectation.ts')) {
      const isSuccess = runTest(filePath);
      if (isSuccess) {
        successCount++;
      } else {
        failureCount++;
        failedFiles.push(filePath);
      }
    } else if ((filePath.endsWith(Extension.TS) || filePath.endsWith(Extension.JS)) && !(filePath.endsWith(Extension.DETS) ||
      filePath.endsWith(Extension.DTS))) {
      executeRunTest(file, filePath);
      compareContent(filePath);
    } else if (filePath.endsWith(Extension.DETS) || filePath.endsWith(Extension.DTS)) {
      compareContent(filePath);
    }
  }
}

function executeRunTest(fileName, filePath) {
  if (!NonExecutableFile.includes(fileName)) {
    const isSuccess = runTest(filePath);
    if (isSuccess) {
      successCount++;
    } else {
      failureCount++;
      failedFiles.push(filePath);
    }
  }
}

function compareContent(filePath) {
  const sourcePath = filePath.replace('/test/local/', '/test/grammar/');
  const sourcePathAndExtension = FileUtils.getFileSuffix(sourcePath);
  const expectationPath = sourcePathAndExtension.path + '_expected.txt';
  const resultPathAndExtension = FileUtils.getFileSuffix(filePath);
  const resultCachePath = resultPathAndExtension.path + '.ts.cache.json';
  const expectationCachePath = sourcePathAndExtension.path + '_expected_cache.txt';
  const hasExpectationFile = fs.existsSync(expectationPath);
  const hasExpectationCache = fs.existsSync(expectationCachePath);
  const hasResultCache = fs.existsSync(resultCachePath);
  if (hasExpectationFile || (hasExpectationCache && hasResultCache)) {
    let actual;
    let expectation;
    if (hasExpectationFile) {
      actual = fs.readFileSync(filePath).toString();
      expectation = fs.readFileSync(expectationPath).toString();
    } else {
      actual = fs.readFileSync(resultCachePath).toString();
      expectation = fs.readFileSync(expectationCachePath).toString();
    }
    if (actual === expectation) {
      contentcomparationSuccessCount++;
    } else {
      contentcomparationFailureCount++;
      contentComparisionFailureFiles.push(filePath);
      const differences = diff.diffLines(actual, expectation);
      differences.forEach(part => {
        const color = part.added ? '\x1b[32m' : part.removed ? '\x1b[31m' : '\x1b[0m';
        console.log(color + part.value + '\x1b[0m');
      });
    }
  }
}

runTestsInDirectory(testDirectory);

console.log('--- Grammar Test Results ---');
console.log(`Success count: ${successCount}`);
console.log(`Failure count: ${failureCount}`);
if (failureCount > 0) {
  console.log('Execution failed files:');
  for (const failedFile of failedFiles) {
    console.log(failedFile);
  }
}

console.log(`Content comparison Success count: ${contentcomparationSuccessCount}`);
console.log(`Content comparison Failure count: ${contentcomparationFailureCount}`);
if (contentcomparationFailureCount > 0) {
  console.log('Content comparision failed files:');
  for (const failedFile of contentComparisionFailureFiles) {
    console.log(failedFile);
  }
}