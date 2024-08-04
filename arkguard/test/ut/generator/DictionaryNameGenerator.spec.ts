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

import fs from 'fs';
import mocha from 'mocha';
import { assert } from 'chai';
import { DictionaryNameGenerator } from '../../../src/generator/DictionaryNameGenerator';

describe('Tester Cases for <DictionaryNameGenerator>.', function () {
	describe('Tester Cases for <constructor>.', function () {
		it('should initialize with default dictionary list if no options provided', function() {
      const generator = new DictionaryNameGenerator();
      assert.deepEqual(generator.getDictionaryList(), ['hello', 'world', 'dictionary', 'light', 'thunder', 'storm']);
			assert.strictEqual(generator.getDictIndex(), 0);
			assert.strictEqual(generator.getTransformNumber(), 0);
    });

		it('should initialize with provided dictionary list if options are provided', function() {
      const customDictionaryList = ['custom1', 'custom2'];
      const generator = new DictionaryNameGenerator({ dictionaryList: customDictionaryList });
      assert.deepEqual(generator.getDictionaryList(), customDictionaryList);
			assert.strictEqual(generator.getDictIndex(), 0);
			assert.strictEqual(generator.getTransformNumber(), 0);
    });
	});

	describe('Tester Cases for <getName>.', function () {
		it('should return null when mDictIndex is out of range', function() {
			const customDictionaryList = [];
      const generator = new DictionaryNameGenerator({ dictionaryList: customDictionaryList });
      assert.isNull(generator.getName());
    });

		it('should return a string based on binary transformation', function() {
      const customDictionaryList = ['hello'];
			const generator = new DictionaryNameGenerator({ dictionaryList: customDictionaryList });
      assert.equal(generator.getName(), 'hello');
    });

		it('should return a string with uppercase characters based on binary transformation', function() {
      const customDictionaryList = ['hello', 'world'];
			const generator = new DictionaryNameGenerator({ dictionaryList: customDictionaryList });
			generator.getName();
      assert.equal(generator.getName(), 'Hello');
			assert.strictEqual(generator.getDictIndex(), 0);
			assert.strictEqual(generator.getTransformNumber(), 2);
    });

		it('should set mDictIndex and mTransformNumber', function() {
      const customDictionaryList = ['h'];
			const generator = new DictionaryNameGenerator({ dictionaryList: customDictionaryList });
			assert.equal(generator.getName(), 'h');
			assert.equal(generator.getName(), 'H');
      assert.strictEqual(generator.getDictIndex(), 1);
			assert.strictEqual(generator.getTransformNumber(), 0);
    });

		it('should skip reserved names and return the next valid name', function() {
      const customDictionaryList = ['hello'];
			const customNames =  new Set(['hello']);
			const generator = new DictionaryNameGenerator({ dictionaryList: customDictionaryList, reservedNames: customNames});
      assert.equal(generator.getName(), 'Hello');
			assert.strictEqual(generator.getDictIndex(), 0);
			assert.strictEqual(generator.getTransformNumber(), 2);
    });
	});

	describe('Tester Cases for <reset>.', function () {
		it('should reset mDictIndex and mTransformNumber to 0', function() {
			const customDictionaryList = ['h'];
      const generator = new DictionaryNameGenerator({ dictionaryList: customDictionaryList });
			generator.getName();
			assert.strictEqual(generator.getTransformNumber(), 1);
			generator.reset();
			assert.strictEqual(generator.getTransformNumber(), 0);
			generator.getName();
			generator.getName();
      assert.strictEqual(generator.getDictIndex(), 1);
			generator.reset();
			assert.strictEqual(generator.getDictIndex(), 0);
    });

	});
});