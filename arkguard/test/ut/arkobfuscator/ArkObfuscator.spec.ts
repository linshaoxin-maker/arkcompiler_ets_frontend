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

import { describe, it } from 'mocha';
import { assert, expect } from 'chai';
import { 
  ApiExtractor,
  ArkObfuscator,
  clearGlobalCaches,
  collectResevedFileNameInIDEConfig,
  containWildcards,
  deleteLineInfoForNameString,
  enableObfuscatedFilePathConfig,
  enableObfuscateFileName,
  FileUtils,
  generateConsumerObConfigFile,
  getMapFromJson,
  getRelativeSourcePath,
  handleObfuscatedFilePath,
  handleUniversalPathInObf,
  initObfuscationConfig,
  mangleFilePath,
  MemoryUtils,
  MergedConfig,
  nameCacheMap,
  ObConfigResolver,
  ObfuscationResultType,
  orignalFilePathForSearching,
  PropCollections,
  readNameCache,
  readProjectPropertiesByCollectedPaths,
  renameFileNameModule,
  ReseverdSetForArkguard,
  separateUniversalReservedItem,
  UnobfuscationCollections,
  wildcardTransformer,
  writeObfuscationNameCache,
  writeUnobfuscationContent,
  unobfuscationNamesObj
} from '../../../src/ArkObfuscator';

import { ArkObfuscatorForTest } from '../../../src/ArkObfuscatorForTest'
import path from 'path';
import { 
  SourceFile,
  createSourceFile,
  ScriptTarget,
  createTextWriter,
  RawSourceMap,
  createObfTextSingleLineWriter
} from 'typescript';

import { IOptions } from '../../../src/configs/IOptions';
import { getSourceMapGenerator } from '../../../src/utils/SourceMapUtil';
import { 
  globalFileNameMangledTable,
  historyFileNameMangledTable
} from '../../../src/transformers/rename/RenameFileNameTransformer';
import { LocalVariableCollections } from '../../../src/utils/CommonCollections';
import { FilePathObj } from '../../../src/common/type';

describe('Tester Cases for <ArkObfuscatorForTest>.', function () {
  describe('Tester Cases for <ArkObfuscatorForTest>.', function () {
    let etsSourceFile: SourceFile;
    let dEtsSourceFile: SourceFile;
    let tsSourceFile: SourceFile;
    let etsSourceFilePath: string = 'demo.ets';
    let dEtsSourceFilePath: string = 'demo.d.ets';
    let tsSourceFilePath: string = 'demo.ts';

    before('init sourceFile', function () {
      const etsfileContent = `//This is a comment
//This is a comment
//This is a comment
//This is a comment
class Demo{
  constructor(public  title: string, public  content: string, public  mark: number) {
      this.title = title
      this.content = content
      this.mark = mark
  }
}
`;
      const dEtsFileContent = `
      /**
       * This is a comment
       */
      
      class Demo{
          constructor(public  title: string, public  content: string, public  mark: number) {
              this.title = title
              this.content = content
              this.mark = mark
          }
      }
      `;

      const tsFileContent = `
      //This is a comment
      class Demo{
          constructor(public  title: string, public  content: string, public  mark: number) {
              this.title = title
              this.content = content
              this.mark = mark
          }
      }
      `;

      etsSourceFile = createSourceFile('demo.ts', etsfileContent, ScriptTarget.ES2015, true);
      dEtsSourceFile = createSourceFile(dEtsSourceFilePath, dEtsFileContent, ScriptTarget.ES2015, true);
      tsSourceFile = createSourceFile(tsSourceFilePath, tsFileContent, ScriptTarget.ES2015, true);
    });

    it('Tester: test case for handleTsHarComments for ets file', function () {
      let mCustomProfiles: IOptions | undefined = FileUtils.readFileAsJson(path.join(__dirname, 'default_config.json'));
      let arkobfuscator = new ArkObfuscatorForTest();
      arkobfuscator.init(mCustomProfiles);
      let originalFilePath = 'demo.ets';
      ArkObfuscatorForTest.projectInfo = { packageDir: '', projectRootPath: '', localPackageSet: new Set<string>(), useNormalized: false, useTsHar: true };
      arkobfuscator.handleTsHarComments(etsSourceFile, originalFilePath);
      let sourceMapGenerator = getSourceMapGenerator(originalFilePath);
      const textWriter = createTextWriter('\n');
      arkobfuscator.createObfsPrinter(etsSourceFile.isDeclarationFile).writeFile(etsSourceFile, textWriter, sourceMapGenerator);
      const actualContent = textWriter.getText();
      const expectContent = `
      // @keepTs
      // @ts-nocheck
      class Demo {
          constructor(public title: string, public content: string, public mark: number) {
              this.title = title;
              this.content = content;
              this.mark = mark;
          }
      }`;
      
      let actualSourceMap: RawSourceMap = sourceMapGenerator.toJSON();
      actualSourceMap.sourceRoot = '';
      let expectSourceMap = {
          'version': 3,
          'file': 'demo.ets',
          'sourceRoot': '',
          'sources': [
            'demo.ts'
          ],
          'names': [],
          'mappings': ';;AAIA,MAAM,IAAI;IACR,YAAY,MAAM,CAAE,KAAK,EAAE,MAAM,EAAE,MAAM,CAAE,OAAO,EAAE,MAAM,EAAE,MAAM,CAAE,IAAI,EAAE,MAAM;QAC5E,IAAI,CAAC,KAAK,GAAG,KAAK,CAAA;QAClB,IAAI,CAAC,OAAO,GAAG,OAAO,CAAA;QACtB,IAAI,CAAC,IAAI,GAAG,IAAI,CAAA;IACpB,CAAC;CACF'
      };

      assert.strictEqual(compareStringsIgnoreNewlines(actualContent, expectContent), true);
      assert.strictEqual(compareStringsIgnoreNewlines(JSON.stringify(actualSourceMap, null, 2), JSON.stringify(expectSourceMap, null, 2)), true);
    });

    it('Tester: test case for handleTsHarComments for d.ets file', function () {
      let mCustomProfiles: IOptions | undefined = FileUtils.readFileAsJson(path.join(__dirname, 'default_config.json'));
      let arkobfuscator = new ArkObfuscatorForTest();
      arkobfuscator.init(mCustomProfiles);
      ArkObfuscatorForTest.projectInfo = { packageDir: '', projectRootPath: '', localPackageSet: new Set<string>(), useNormalized: false, useTsHar: true };
      arkobfuscator.handleTsHarComments(dEtsSourceFile, dEtsSourceFilePath);
      let sourceMapGenerator = getSourceMapGenerator(dEtsSourceFilePath);
      const textWriter = createTextWriter('\n');
      arkobfuscator.createObfsPrinter(dEtsSourceFile.isDeclarationFile).writeFile(dEtsSourceFile, textWriter, sourceMapGenerator);
      const actualContent = textWriter.getText();
      const expectContent = `
      /**
       * This is a comment
       */
      class Demo {
          constructor(public title: string, public content: string, public mark: number) {
              this.title = title;
              this.content = content;
              this.mark = mark;
          }
      }`;
      assert.strictEqual(compareStringsIgnoreNewlines(actualContent, expectContent), true);
    });

    it('Tester: test case for handleTsHarComments for ts file', function () {
      let mCustomProfiles: IOptions | undefined = FileUtils.readFileAsJson(path.join(__dirname, 'default_config.json'));
      let arkobfuscator = new ArkObfuscatorForTest();
      arkobfuscator.init(mCustomProfiles);
      ArkObfuscatorForTest.projectInfo = { packageDir: '', projectRootPath: '', localPackageSet: new Set<string>(), useNormalized: false, useTsHar: true };
      arkobfuscator.handleTsHarComments(tsSourceFile, tsSourceFilePath);
      let sourceMapGenerator = getSourceMapGenerator(tsSourceFilePath);
      const textWriter = createTextWriter('\n');
      arkobfuscator.createObfsPrinter(tsSourceFile.isDeclarationFile).writeFile(tsSourceFile, textWriter, sourceMapGenerator);
      const actualContent = textWriter.getText();
      const expectContent = `
        class Demo {
          constructor(public title: string, public content: string, public mark: number) {
            this.title = title;
            this.content = content;
            this.mark = mark;
          }
        }`;

      assert.strictEqual(compareStringsIgnoreNewlines(actualContent, expectContent), true);
      console.log(actualContent);
    });
  });

  describe('Tester Cases for <ArkObfuscatorForTest>.', function () {
    it('Tester: test case for ArkObfuscatorForTest.ini', function (){
      let configPath = 'test/ut/arkobfuscator/iniTestObfConfig.json'
      let obfuscator: ArkObfuscatorForTest = new ArkObfuscatorForTest();
      const originalConfig: IOptions | undefined = FileUtils.readFileAsJson(configPath);
      obfuscator.init(originalConfig);
      let config = obfuscator.customProfiles;
      let reservedTopelevelNames = config.mNameObfuscation?.mReservedToplevelNames;
      let reservedProperty = config.mNameObfuscation?.mReservedProperties;
      let universalReservedToplevelNames = config.mNameObfuscation?.mUniversalReservedToplevelNames as RegExp[];
      let universalReservedProperties = config.mNameObfuscation?.mUniversalReservedProperties as RegExp[];
      assert.isTrue(reservedTopelevelNames?.includes('func2'));
      assert.isTrue(reservedProperty?.includes('prop'));
      assert.equal(universalReservedToplevelNames[0].toString(), new RegExp(`^${wildcardTransformer('a*')}$`).toString());
      assert.equal(universalReservedToplevelNames[1].toString(), new RegExp(`^${wildcardTransformer('*shoul?keep*')}$`).toString());
      assert.equal(universalReservedProperties[0].toString(), new RegExp(`^${wildcardTransformer('prop?')}$`).toString());
      assert.equal(universalReservedProperties[2].toString(), new RegExp(`^${wildcardTransformer('*pro?')}$`).toString());
      assert.equal(universalReservedProperties[1].toString(), new RegExp(`^${wildcardTransformer('function*')}$`).toString());
    });
  });
});

describe('Tester Cases for <ArkObfuscator>', function () {
  let obfuscator: ArkObfuscator;
  let defaultConfig: IOptions;

  const sourceFilePathObj: FilePathObj = {
    buildFilePath: 'demo.ts',
    relativeFilePath: ''
  };
  let sourceFile: SourceFile;
  let sourceFileContent: string = `class Person {
  constructor(public name: string, public age: number) {
      this.name = name;
      this.age = age;
  }
}`;

  const jsSourceFilePathObj: FilePathObj = {
    buildFilePath: 'demo.js',
    relativeFilePath: ''
  };
  let jsSourceFile: SourceFile;
  let jsSourceFileContent: string = `//This is a comment
//This is a comment
function subtract(a, b) {
    return a - b;
}`;

  const declSourceFilePathObj: FilePathObj = {
    buildFilePath: 'demo.d.ts',
    relativeFilePath: ''
  };
  let declSourceFile: SourceFile;
  let declSourceFileContent: string = `//This is a comment
//This is a comment
export declare function add(num1: number, num2: number): number;
export declare function findElement<T>(arr: T[], callback: (item: T) => boolean): T | undefined;`;

  beforeEach(() => {
    obfuscator = new ArkObfuscator();

    // Clear the collection to ensure test isolation
    PropCollections.clearPropsCollections();
    UnobfuscationCollections.clear();
    LocalVariableCollections.clear();
  
    defaultConfig = {
      mRemoveComments: true,
      mNameObfuscation: {
        mEnable: true,
        mNameGeneratorType: 1,
        mReservedNames: [],
        mRenameProperties: true,
        mReservedProperties: [],
        mTopLevel: true,
        mReservedToplevelNames: [],
      },
      mEnableSourceMap: true,
      mEnableNameCache: true
    };

    sourceFile = createSourceFile(sourceFilePathObj.buildFilePath, sourceFileContent, ScriptTarget.ES2015, true);
    jsSourceFile = createSourceFile(jsSourceFilePathObj.buildFilePath, jsSourceFileContent, ScriptTarget.ES2015, true);
    declSourceFile = createSourceFile(declSourceFilePathObj.buildFilePath, declSourceFileContent, ScriptTarget.ES2015, true);
  });

  describe('test for ArkObfuscator.init', () => {
    it('should return false if config is undefined', () => {
      const result = obfuscator.init(undefined);
      expect(result).to.be.false;
    });

    it('should initialize with valid config', () => {
      let configPath = 'test/ut/arkobfuscator/iniTestObfConfig.json';
      let config = FileUtils.readFileAsJson(configPath) as IOptions;
      obfuscator.init(config);
      let reservedTopelevelNames = config.mNameObfuscation?.mReservedToplevelNames;
      let reservedProperty = config.mNameObfuscation?.mReservedProperties;
      let universalReservedToplevelNames = config.mNameObfuscation?.mUniversalReservedToplevelNames;
      let universalReservedProperties = config.mNameObfuscation?.mUniversalReservedProperties;
      assert.isTrue(reservedTopelevelNames?.includes('func2'));
      assert.isTrue(reservedTopelevelNames?.includes('a*'));
      assert.isTrue(reservedTopelevelNames?.includes('*shoul?keep*'));
      assert.isTrue(reservedProperty?.includes('prop'));
      assert.isTrue(reservedProperty?.includes('prop?'));
      assert.isTrue(reservedProperty?.includes('*pro?'));
      assert.isTrue(reservedProperty?.includes('function*'));
      assert.equal(universalReservedToplevelNames, undefined);
      assert.equal(universalReservedProperties, undefined);
    });

    it('source code should be compacted into one line', () => {
      const config: IOptions = {
        mCompact: true,
        mEnableSourceMap: true,
        mRemoveComments: true
      };
      obfuscator.init(config);

      let sourceMapGenerator = getSourceMapGenerator(jsSourceFilePathObj.buildFilePath);
      const textWriter = createObfTextSingleLineWriter();
      obfuscator.createObfsPrinter(jsSourceFile.isDeclarationFile).writeFile(jsSourceFile, textWriter, sourceMapGenerator);
      const actualContent = textWriter.getText();
      const expectContent = `function subtract(a, b) {return a - b;}`;
      expect(actualContent === expectContent).to.be.true;
    });
  });

  describe('test for ArkObfuscator.obfuscate', () => {
    it('should return empty result for ignored files', async () => {
      const sourceFilePathObj: FilePathObj = {
        buildFilePath: 'ignoredFile.cpp',
        relativeFilePath: ''
      };
      const result: ObfuscationResultType = await obfuscator.obfuscate(
        'hello world',
        sourceFilePathObj
      );
      expect(result).to.deep.equal({ content: undefined });
    });

    it('should return empty result for empty AST', async () => {
      const sourceFilePathObj: FilePathObj = {
        buildFilePath: 'emptyFile.js',
        relativeFilePath: ''
      };
      const result: ObfuscationResultType = await obfuscator.obfuscate('', sourceFilePathObj);
      expect(result).to.deep.equal({ content: undefined });
    });

    it('should process valid AST and return obfuscated content', async () => {
      obfuscator.init(defaultConfig);
      
      const result: ObfuscationResultType = await obfuscator.obfuscate(
        sourceFile,
        sourceFilePathObj
      );
      const expectResult = `class g {
    constructor(public name: string, public h: number) {
        this.name = name;
        this.h = h;
    }
}
`;
      expect(result.content === expectResult).to.be.true;
    });

    it('comments in declaration file should be removed', async () => {
      const config: IOptions = {
        mRemoveDeclarationComments: {
          mEnable: true,
          mReservedComments: [],
          mUniversalReservedComments: []
        },
      };

      obfuscator.init(config);
      const result: ObfuscationResultType = await obfuscator.obfuscate(declSourceFile, declSourceFilePathObj);
      const expectResult = `export declare function add(num1: number, num2: number): number;
export declare function findElement<T>(arr: T[], callback: (item: T) => boolean): T | undefined;
`;
      expect(result.content === expectResult).to.be.true;
    });

    it('comments in declaration file should not be removed', async () => {
      obfuscator.init(defaultConfig);
      const result: ObfuscationResultType = await obfuscator.obfuscate(declSourceFile, declSourceFilePathObj);
      const expectResult = `//This is a comment
//This is a comment
export declare function add(d: number, e: number): number;
export declare function findElement<a>(b: a[], c: (item: a) => boolean): a | undefined;
`;
      expect(result.content === expectResult).to.be.true;
    });

    it('when enable -print-kept-names, unobfuscationNameMap should not be undefined', async () => {
      const config = {
        mUnobfuscationOption: {
          mPrintKeptNames: true,
          mPrintPath: 'local.json'
        }
      };
      obfuscator.init(config);
      const result: ObfuscationResultType = await obfuscator.obfuscate(
        sourceFile,
        sourceFilePathObj
      );

      expect(result.unobfuscationNameMap !== undefined).to.be.true
    });

    it('when disable -print-kept-names, unobfuscationNameMap should be undefined', async () => {
      obfuscator.init(defaultConfig);
      const result: ObfuscationResultType = await obfuscator.obfuscate(
        sourceFile,
        sourceFilePathObj
      );

      expect(result.unobfuscationNameMap === undefined).to.be.true;
    });

    it('should use historyNameCache', async () => {
      obfuscator.init(defaultConfig);
      const historyNameCache = new Map([['#Person', 'm'], ['Person:2:5', 'm']]);
      const result: ObfuscationResultType = await obfuscator.obfuscate(
        sourceFile,
        sourceFilePathObj,
        undefined,
        historyNameCache
      );
      const expectResult = `class m {
    constructor(public name: string, public g: number) {
        this.name = name;
        this.g = g;
    }
}
`;
      expect(result.content === expectResult).to.be.true;
    });

    it('should obfuscate fileName', async () => {
      const config: IOptions = {
        mRenameFileName: {
          mEnable: true,
          mNameGeneratorType: 1,
          mReservedFileNames: [],
        }
      }
      obfuscator.init(config);
      const result: ObfuscationResultType = await obfuscator.obfuscate(
        sourceFile,
        sourceFilePathObj
      );

      expect(orignalFilePathForSearching === 'demo.ts').to.be.true;
      expect(result.filePath === 'a.ts').to.be.true;
    });

    it('shoule clear caches after obfuscate', async () => {
      PropCollections.globalMangledTable.set('test', 'obfuscated');
      PropCollections.newlyOccupiedMangledProps.add('newProp');
      const config: IOptions = {
        mRemoveComments: true,
        mNameObfuscation: {
          mEnable: true,
          mNameGeneratorType: 1,
          mReservedNames: [],
          mRenameProperties: false,
          mReservedProperties: [],
          mTopLevel: true,
          mReservedToplevelNames: [],
        },
        mExportObfuscation: false
      };
      obfuscator.init(config);
      await obfuscator.obfuscate(sourceFile, sourceFilePathObj);

      expect(PropCollections.globalMangledTable.size).to.equal(0);
      expect(PropCollections.newlyOccupiedMangledProps.size).to.equal(0);
    });

    it('test for use sourcemap mapping', async () => {
      obfuscator.init(defaultConfig);
      const previousStageSourceMap = {
        version: 3,
        file: 'demo.js',
        sourceRoot: '',
        sources: [ 'demo.js' ],
        names: [],
        mappings: 'AAEA,SAAS,QAAQ,CAAC,CAAC,EAAE,CAAC;IAClB,OAAO,CAAC,GAAG,CAAC,CAAC;AACjB,CAAC',
        sourcesContent: undefined
      } as RawSourceMap
      const result: ObfuscationResultType = await obfuscator.obfuscate(
        jsSourceFile,
        jsSourceFilePathObj,
        previousStageSourceMap
      );

      const actual = JSON.stringify(result.sourceMap)
      const expect = '{"version":3,"file":"demo.js","sources":["demo.js"],"names":[],"mappings":"AAIA,WAAC,CAAA,EAAA,CAAA;;","sourceRoot":""}';
      assert.isTrue(actual === expect);
    });

    it('test for not use sourcemap mapping', async () => {
      obfuscator.init(defaultConfig);
      const result: ObfuscationResultType = await obfuscator.obfuscate(
        jsSourceFile,
        jsSourceFilePathObj,
      );

      const actual = JSON.stringify(result.sourceMap)
      const expect = '{"version":3,"file":"demo.js","sourceRoot":"","sources":["demo.js"],"names":[],"mappings":"AAEA,WAAkB,CAAC,EAAE,CAAC;IAClB,OAAO,KAAK,CAAC;AACjB,CAAC"}';
      assert.isTrue(actual === expect);
    });
  });

  describe('test for ArkObfuscator.setWriteOriginalFile', () => {
    it('should set writeOriginalFile to true', () => {
      obfuscator.setWriteOriginalFile(true);
      expect(obfuscator.getWriteOriginalFileForTest()).to.be.true;
    });

    it('should set writeOriginalFile to false', () => {
      obfuscator.setWriteOriginalFile(false);
      expect(obfuscator.getWriteOriginalFileForTest()).to.be.false;
    });
  });

  describe('test for ArkObfuscator.addReservedSetForPropertyObf', () => {
    it('should add reserved sets correctly', () => {
      const properties: ReseverdSetForArkguard = {
        structPropertySet: new Set(['struct1']),
        stringPropertySet: new Set(['string1']),
        exportNameAndPropSet: new Set(['export1']),
        exportNameSet: undefined,
        enumPropertySet: new Set(['enum1']),
      };

      obfuscator.addReservedSetForPropertyObf(properties);

      expect(UnobfuscationCollections.reservedStruct.has('struct1')).to.be.true;
      expect(UnobfuscationCollections.reservedStrProp.has('string1')).to.be.true;
      expect(UnobfuscationCollections.reservedExportNameAndProp.has('export1')).to.be.true;
      expect(UnobfuscationCollections.reservedEnum.has('enum1')).to.be.true;
    });

    it('should not add empty sets', () => {
      const properties: ReseverdSetForArkguard = {
        structPropertySet: new Set(),
        stringPropertySet: new Set(),
        exportNameAndPropSet: new Set(),
        exportNameSet: undefined,
        enumPropertySet: new Set(),
      };

      obfuscator.addReservedSetForPropertyObf(properties);

      expect(UnobfuscationCollections.reservedStruct.size).to.equal(0);
      expect(UnobfuscationCollections.reservedStrProp.size).to.equal(0);
      expect(UnobfuscationCollections.reservedExportNameAndProp.size).to.equal(0);
      expect(UnobfuscationCollections.reservedEnum.size).to.equal(0);
    });
  });

  describe('test for ArkObfuscator.addReservedSetForDefaultObf', () => {
    it('should add reserved export name set correctly', () => {
      const properties: ReseverdSetForArkguard = {
        structPropertySet: undefined,
        stringPropertySet: undefined,
        exportNameAndPropSet: undefined,
        exportNameSet: new Set(['exportName1']),
        enumPropertySet: undefined,
      };

      obfuscator.addReservedSetForDefaultObf(properties);
      expect(UnobfuscationCollections.reservedExportName.has('exportName1')).to.be.true;
    });

    it('should not add empty export name set', () => { 
      const properties: ReseverdSetForArkguard = {
        structPropertySet: undefined,
        stringPropertySet: undefined,
        exportNameAndPropSet: undefined,
        exportNameSet: new Set(),
        enumPropertySet: undefined,
      };

      obfuscator.addReservedSetForDefaultObf(properties);
      expect(UnobfuscationCollections.reservedExportName.size).to.equal(0);
    });
  });

  describe('test for ArkObfuscator.addReservedSetForDefaultObf', () => {
    it('should add reserved export name set correctly', () => {
      const properties: ReseverdSetForArkguard = {
        structPropertySet: undefined,
        stringPropertySet: undefined,
        exportNameAndPropSet: undefined,
        exportNameSet: new Set(['exportName1']),
        enumPropertySet: undefined,
      };

      obfuscator.addReservedSetForDefaultObf(properties);
      expect(UnobfuscationCollections.reservedExportName.has('exportName1')).to.be.true;
    });

    it('should not add empty export name set', () => {
      const properties: ReseverdSetForArkguard = {
        structPropertySet: undefined,
        stringPropertySet: undefined,
        exportNameAndPropSet: undefined,
        exportNameSet: new Set(),
        enumPropertySet: undefined,
      };

      obfuscator.addReservedSetForDefaultObf(properties);
      expect(UnobfuscationCollections.reservedExportName.size).to.equal(0);
    });  
  });

  describe('test for ArkObfuscator.setKeepSourceOfPaths', () => {
    it('should set the keep source of paths correctly', () => {
      const config: IOptions = {
        mKeepFileSourceCode: {
          mKeepSourceOfPaths: new Set(),
          mkeepFilesAndDependencies: new Set()
        },
      };
      obfuscator.init(config);

      const paths = new Set(['path1', 'path2']);
      obfuscator.setKeepSourceOfPaths(paths);
      expect(obfuscator.customProfiles.mKeepFileSourceCode?.mKeepSourceOfPaths).to.equal(paths);
    });
  });

  describe('test for ArkObfuscator.handleTsHarComments', () => {
    it('should set writeTsHarComments to true when conditions are met', () => {
      ArkObfuscator.projectInfo = {
        packageDir: '',
        projectRootPath: '',
        localPackageSet: new Set<string>(),
        useNormalized: false,
        useTsHar: true,
      };
      obfuscator.init(defaultConfig);
      obfuscator.handleTsHarComments(sourceFile, 'test.ets');
      // @ts-ignore
      expect(sourceFile.writeTsHarComments).to.be.true;
    });

    it('should not set writeTsHarComments if conditions are not met', () => {
      ArkObfuscator.projectInfo = {
        packageDir: '',
        projectRootPath: '',
        localPackageSet: new Set<string>(),
        useNormalized: false,
        useTsHar: true
      };
      obfuscator.init(defaultConfig);
      obfuscator.handleTsHarComments(declSourceFile, undefined);
      // @ts-ignore
      expect(sourceFile.writeTsHarComments).to.be.undefined;
    });
  });

  describe('test for ArkObfuscator.isCurrentFileInKeepPaths', () => {
    it('should return false if mKeepSourceOfPaths is empty', () => {
      const customProfiles: IOptions = {
        mKeepFileSourceCode: {
          mKeepSourceOfPaths: new Set(),
          mkeepFilesAndDependencies: new Set()
        },
      };
      const result = obfuscator['isCurrentFileInKeepPaths'](customProfiles,'some/file/path.js');
      expect(result).to.be.false;
    });

    it('should return true if originalFilePath is in mKeepSourceOfPaths', () => {
      const keepPaths = new Set(['some/file/path.js']);
      const customProfiles: IOptions = {
        mKeepFileSourceCode: {
          mKeepSourceOfPaths: keepPaths,
          mkeepFilesAndDependencies: new Set()
        }
      };
      const result = obfuscator['isCurrentFileInKeepPaths'](customProfiles,'some/file/path.js');
      expect(result).to.be.true;
    });
  });
});

describe('test for clearGlobalCaches', () => {
  beforeEach(() => {
    PropCollections.globalMangledTable.set('test1', 'obfuscated1');
    PropCollections.historyMangledTable = new Map([['test2', 'obfuscated2']]);
    PropCollections.reservedProperties.add('reserved1');
    PropCollections.universalReservedProperties.push(/universal\d+/);
    PropCollections.newlyOccupiedMangledProps.add('newProp1');
    PropCollections.mangledPropsInNameCache.add('cachedProp1');
    globalFileNameMangledTable.set('key1', 'value1');
    renameFileNameModule.historyFileNameMangledTable = new Map([['keyA', 'valueA']]);
    UnobfuscationCollections.reservedSdkApiForProp.add('api1');  
    UnobfuscationCollections.reservedSdkApiForGlobal.add('globalApi1');  
    UnobfuscationCollections.reservedSdkApiForLocal.add('localApi1');  
    UnobfuscationCollections.reservedStruct.add('struct1');  
    UnobfuscationCollections.reservedLangForProperty.add('lang1');  
    UnobfuscationCollections.reservedExportName.add('exportName1');  
    UnobfuscationCollections.reservedExportNameAndProp.add('exportNameAndProp1');  
    UnobfuscationCollections.reservedStrProp.add('stringProp1');  
    UnobfuscationCollections.reservedEnum.add('enum1');  
    UnobfuscationCollections.unobfuscatedPropMap.set('age', new Set(['key', 'value']));
    UnobfuscationCollections.unobfuscatedNamesMap.set('name1', new Set(['key1', 'value2']));  
    LocalVariableCollections.reservedStruct.add('localStruct1');
    LocalVariableCollections.reservedConfig.add('localConfig1');
  });

  it('should clear all global caches', () => {
    clearGlobalCaches();

    expect(PropCollections.globalMangledTable.size).to.equal(0);
    expect(PropCollections.historyMangledTable.size).to.equal(0);
    expect(PropCollections.reservedProperties.size).to.equal(0);
    expect(PropCollections.universalReservedProperties.length).to.equal(0);
    expect(PropCollections.newlyOccupiedMangledProps.size).to.equal(0);
    expect(PropCollections.mangledPropsInNameCache.size).to.equal(0);
    expect(globalFileNameMangledTable.size).to.equal(0);
    expect(historyFileNameMangledTable.size).to.equal(0);
    expect(UnobfuscationCollections.reservedSdkApiForProp.size).to.equal(0);
    expect(UnobfuscationCollections.reservedStruct.size).to.equal(0);
    expect(UnobfuscationCollections.reservedExportName.size).to.equal(0);
    expect(UnobfuscationCollections.printKeptName).to.be.false;
    expect(LocalVariableCollections.reservedStruct.size).to.equal(0);  
    expect(LocalVariableCollections.reservedConfig.size).to.equal(0);
  });
});

describe('test whether the methods exported from the ArkObfuscator file exist', () => {
  it('should import collectResevedFileNameInIDEConfig', () => {
    expect(collectResevedFileNameInIDEConfig).to.exist;
  });

  it('should import enableObfuscatedFilePathConfig', () => {
    expect(enableObfuscatedFilePathConfig).to.exist;
  });

  it('should import enableObfuscateFileName', () => {
    expect(enableObfuscateFileName).to.exist;
  });

  it('should import generateConsumerObConfigFile', () => {
    expect(generateConsumerObConfigFile).to.exist;
  });

  it('should import getRelativeSourcePath', () => {
    expect(getRelativeSourcePath).to.exist;
  });

  it('should import handleObfuscatedFilePath', () => {
    expect(handleObfuscatedFilePath).to.exist;
  });

  it('should import handleUniversalPathInObf', () => {
    expect(handleUniversalPathInObf).to.exist;
  });

  it('should import mangleFilePath', () => {
    expect(mangleFilePath).to.exist;
  });

  it('should import MergedConfig', () => {
    expect(MergedConfig).to.exist;
  });

  it('should import ObConfigResolver', () => {
    expect(ObConfigResolver).to.exist;
  });

  it('should import readNameCache', () => {
    expect(readNameCache).to.exist;
  });

  it('should import writeObfuscationNameCache', () => {
    expect(writeObfuscationNameCache).to.exist;
  });

  it('should import MemoryUtils', () => {
    expect(MemoryUtils).to.exist;
  });

  it('should import initObfuscationConfig', () => {
    expect(initObfuscationConfig).to.exist;
  });

  it('should import nameCacheMap', () => {
    expect(nameCacheMap).to.exist;
  });

  it('should import separateUniversalReservedItem', () => {
    expect(separateUniversalReservedItem).to.exist;
  });

  it('should import containWildcards', () => {
    expect(containWildcards).to.exist;
  });

  it('should import getMapFromJson', () => {
    expect(getMapFromJson).to.exist;
  });

  it('should import readProjectPropertiesByCollectedPaths', () => {
    expect(readProjectPropertiesByCollectedPaths).to.exist;
  });

  it('should import deleteLineInfoForNameString', () => {
    expect(deleteLineInfoForNameString).to.exist;
  });

  it('should import ApiExtractor', () => {
    expect(ApiExtractor).to.exist;
  });

  it('should import PropCollections', () => {
    expect(PropCollections).to.exist;
  });

  it('should import writeUnobfuscationContent', () => {
    expect(writeUnobfuscationContent).to.exist;
  });

  it('should import unobfuscationNamesObj', () => {
    expect(unobfuscationNamesObj).to.exist;
  });
});

function compareStringsIgnoreNewlines(str1: string, str2: string): boolean {
  const normalize = (str: string) => str.replace(/[\n\r\s]+/g, '');
  return normalize(str1) === normalize(str2);
}