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

import mocha from 'mocha';
import { isDebug, isFileExist, sortAndDeduplicateStringArr, mergeSet, convertSetToArray } from '../../../src/initialization/utils';
import { assert, expect } from 'chai';
import { DEBUG } from "../../../src/initialization/CommonObject";
import * as fs from 'fs';

describe('Tester Cases for <utils>.', function () {
  describe('Tester Cases for <isDebug>.', function () {
    /** test for isDebug */
    it('Tester: <Debug Mode> case for isDebug', function () {
      const projectConfig: any = {
        buildMode: DEBUG
      };
      assert.strictEqual(isDebug(projectConfig), true);
    });
  });

  describe('Tester Cases for <isFileExist>.', function () {
    /** test for isFileExist */
    it('Tester: <file not exist> case for isFileExist', function () {
      const path: string = 'test/ut/initialization/testFileNotExiet.txt';
      assert.strictEqual(isFileExist(path), false);
    });

    it('Tester: <file exist> case for isFileExist', function () {
      const path: string = 'test/ut/initialization/demo.txt';
      fs.writeFileSync(path, 'test');
      assert.strictEqual(isFileExist(path), true);
      fs.unlinkSync(path);
    });
  });

  describe('Tester Cases for <sortAndDeduplicateStringArr>.', function () {
    /** test for sortAndDeduplicateStringArr */
    it('Tester: <the length of arr is 0> case for sortAndDeduplicateStringArr', function () {
      const arr: string[] = [];
      assert.strictEqual(sortAndDeduplicateStringArr(arr).length, 0);
    });

    it('Tester: <sort and deduplicate> case for sortAndDeduplicateStringArr', function () {
      const arr0: string[] = ['test2', 'test2', 'test1', 'test0'];
      const arr2: string[] = ['test0', 'test1', 'test2'];
      let arr1 = sortAndDeduplicateStringArr(arr0);
      expect(arr1).to.deep.equal(arr2);
    });
  });

  describe('Tester Case for <mergeSet>.', function () {
    /** test for mergeSet */
    it('Tester: <The length of set1 and the length of set2 are both greater than 0> case for mergeSet', function () {
      const set1: Set<string> = new Set(['a', 'b']);
      const set2: Set<string> = new Set(['b', 'c']);
      const set3: Set<string> = new Set(['a', 'b', 'c']);
      const result: Set<string> = mergeSet(set1, set2);
      expect(result).to.deep.equal(set3);
    });

    it('Tester: <The length of set1 is greater than 0> case for mergeSet', function () {
      const set1: Set<string> = new Set(['a', 'b']);
      const set2: Set<string> = new Set([]);
      const result1: Set<string> = mergeSet(set1, set2);
      expect(result1).to.deep.equal(set1);
    });

    it('Tester: <The length of set1 is greater than 0> case for mergeSet', function () {
      const set1: Set<string> = new Set([]);
      const set2: Set<string> = new Set(['a', 'b']);
      const result1: Set<string> = mergeSet(set1, set2);
      expect(result1).to.deep.equal(set2);
    });
  });

  describe('Tester Case for <convertSetToArray>.', function () {
    it('Tester: reservedSet is undefined', function () {
      const reservedSet: Set<string> | undefined = undefined;
      const expect1: string[] = [];
      expect(expect1).to.deep.equal(convertSetToArray(reservedSet))
    });

    it('Tester: reservedSet is not undefined', function () {
      const reservedSet: Set<string> = new Set(['a', 'b']);
      const expect1: string[] = ['a', 'b'];
      expect(expect1).to.deep.equal(convertSetToArray(reservedSet))
    });
  });
});
