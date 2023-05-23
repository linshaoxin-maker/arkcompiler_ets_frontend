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

import {
  createPrinter,
  createProgram,
  createSourceFile, createTextWriter,
  forEachChild,
  isIdentifier,
  isTypePredicateNode,
  ScriptTarget, 
  transform,
  createObfTextSingleLineWriter
} from 'typescript';

import type {
  CompilerOptions,
  EmitTextWriter,
  JSDocReturnTag,
  JSDocSignature,
  Node,
  Printer,
  PrinterOptions,
  Program,
  Signature,
  SignatureDeclaration,
  SourceFile,
  SourceMapGenerator,
  TransformationResult,
  TransformerFactory,
  TypeChecker,
  TypeNode
} from 'typescript';

import * as fs from 'fs';
import path from 'path';

import type {IOptions} from './configs/IOptions';
import {FileUtils} from './utils/FileUtils';
import {TransformerManager} from './transformers/TransformerManager';
import {getSourceMapGenerator} from './utils/SourceMapUtil';

import {
  addIdentifierNameCache,
  addPropertyNameCache, getMapFromJson,
  readNameCache,
  writeNameCache
} from './utils/NameCacheUtil';
import type {
  NameCacheInfo
} from './utils/NameCacheUtil';

import {ListUtil} from './utils/ListUtil';
import {needReadApiInfo, readOhReservedProperties, readProjectProperties} from './common/ApiReader';

const renameIdentifierModule = require('./transformers/rename/RenameIdentifierTransformer');
const renamePropertyModule = require('./transformers/rename/RenamePropertiesTransformer');

export class ArkObfuscator {
  /**
   * A text writer of Printer
   */
  private mTextWriter: EmitTextWriter;

  /**
   * A Printer to output obfuscated codes.
   */
  private mPrinter: Printer;

  /**
   * A list of source file path
   */
  private readonly mSourceFiles: string[];

  /**
   * Path of obfuscation configuration file.
   */
  private readonly mConfigPath: string;

  /**
   * Compiler Options for typescript,use to parse ast
   */
  private readonly mCompilerOptions: CompilerOptions;

  /**
   * User custom obfuscation profiles.
   * @private
   */
  private mCustomProfiles: IOptions;

  private mTransformers: TransformerFactory<Node>[];

  private mNeedCollectNarrowFunction: boolean;

  public constructor(sourceFiles: string[], configPath: string) {
    this.mSourceFiles = sourceFiles;
    this.mConfigPath = configPath;
    this.mCompilerOptions = {};
    this.mTransformers = [];
  }

  /**
   * init ArkObfuscator according to user config
   * should be called after constructor
   */
  public init(config?: any): boolean {
    if (!this.mConfigPath &&  config === undefined) {
      return false;
    }

    if (this.mConfigPath) {
      config = FileUtils.readFileAsJson(this.mConfigPath);
    }

    this.mCustomProfiles = config as IOptions;

    // set print options
    let printerOptions: PrinterOptions = {};
    if (this.mCustomProfiles.mRemoveComments) {
      printerOptions.removeComments = true;
    }

    if (this.mCustomProfiles.mCompact) {
      this.mTextWriter = createObfTextSingleLineWriter();
    } else {
      this.mTextWriter = createTextWriter('\n');
    }

    if (this.mCustomProfiles.mEnableSourceMap) {
      this.mCompilerOptions.sourceMap = true;
    }

    this.mPrinter = createPrinter(printerOptions);

    // check need collect narrow function names
    this.mNeedCollectNarrowFunction = this.checkNeedCollectNarrowFunction();

    if (!needReadApiInfo(this.mCustomProfiles)) {
      return true;
    }

    var ohProperties: string[];
    if(this.mCustomProfiles.mNameObfuscation.mRenameProperties && this.mCustomProfiles.apiSavedDir) {
      ohProperties = readOhReservedProperties(this.mCustomProfiles.apiSavedDir);
    }
    this.mCustomProfiles.mNameObfuscation.mReservedProperties = ListUtil.uniqueMergeList(ohProperties,
      this.mCustomProfiles.mNameObfuscation.mReservedProperties,
      this.mCustomProfiles.mNameObfuscation.mReservedNames);
    
    return true;
  }

  /**
   * Obfuscate all the source files.
   */
  public obfuscateFiles(): boolean {
    if (this.mCustomProfiles.mOutputDir && !fs.existsSync(this.mCustomProfiles.mOutputDir)) {
      fs.mkdirSync(this.mCustomProfiles.mOutputDir);
    }

    // load transformers
    this.mTransformers = TransformerManager.getInstance().loadTransformers(this.mCustomProfiles);
    readProjectProperties(this.mSourceFiles, this.mCustomProfiles);

    // support directory and file obfuscate
    for (const sourcePath of this.mSourceFiles) {
      if (!fs.existsSync(sourcePath)) {
        console.error(`File ${FileUtils.getFileName(sourcePath)} is not found.`);
        return;
      }

      // 1. file
      if (fs.lstatSync(sourcePath).isFile()) {
        this.obfuscateFile(sourcePath, this.mCustomProfiles.mOutputDir);
        continue;
      }

      // 2. directory
      const dirPrefix: string = FileUtils.getPrefix(sourcePath);
      this.obfuscateDir(sourcePath, dirPrefix);
    }
  }

  /**
   * obfuscate directory
   * @private
   */
  private obfuscateDir(dirName: string, dirPrefix: string): void {
    const currentDir: string = FileUtils.getPathWithoutPrefix(dirName, dirPrefix);
    const newDir: string = path.join(this.mCustomProfiles.mOutputDir, currentDir);
    if (!fs.existsSync(newDir)) {
      fs.mkdirSync(newDir);
    }

    const fileNames: string[] = fs.readdirSync(dirName);
    for (let fileName of fileNames) {
      const filePath: string = path.join(dirName, fileName);
      if (fs.lstatSync(filePath).isFile()) {
        this.obfuscateFile(filePath, newDir);
        continue;
      }

      // directory
      if (fileName === 'node_modules' || 'oh_modules') {
        continue;
      }

      this.obfuscateDir(filePath, dirPrefix);
    }
  }

  private checkNeedCollectNarrowFunction(): boolean {
    return this.mCustomProfiles.mControlFlowFlattening &&
      this.mCustomProfiles.mControlFlowFlattening.mEnable &&
      this.mCustomProfiles.mInstructionObfuscation &&
      this.mCustomProfiles.mInstructionObfuscation.mEnable;
  }

  private collectNarrowFunctions(file: string): void {
    if (!this.mNeedCollectNarrowFunction) {
      return;
    }

    let results: Set<string> = new Set<string>();

    let program: Program = createProgram([file], this.mCompilerOptions);
    let checker: TypeChecker = program.getTypeChecker();
    let visit = (node: Node): void => {
      if (!node) {
        return;
      }

      if (isIdentifier(node)) {
        let type: Signature = checker.getTypeAtLocation(node).getCallSignatures()[0];
        let declaration: SignatureDeclaration | JSDocSignature = type?.declaration;
        let retType: TypeNode | JSDocReturnTag = declaration?.type;
        if (retType && isTypePredicateNode(retType)) {
          results.add(node.text);
        }
      }

      forEachChild(node, visit);
    };

    let ast: SourceFile = program.getSourceFile(file);
    visit(ast);

    this.mCustomProfiles.mNarrowFunctionNames = [...results];
  }

  private consumeNameCache(sourceFile: string, outputDir: string): void {
    if (!this.mCustomProfiles.mNameObfuscation.mEnable || !this.mCustomProfiles.mEnableNameCache) {
      return;
    }

    const nameCachePath: string = path.join(outputDir, FileUtils.getFileName(sourceFile) + '.cache');
    const nameCache: NameCacheInfo = readNameCache(nameCachePath);
    if (!nameCache) {
      return;
    }

    renameIdentifierModule.nameCache = getMapFromJson(nameCache.identifierNameCache);
    if (this.mCustomProfiles.mNameObfuscation.mRenameProperties) {
      renamePropertyModule.mangledTable = getMapFromJson(nameCache.propertyNameCache);
    }
  }

  private produceNameCache(outputDir: string, sourceFile: string): void {
    if (!this.mCustomProfiles.mNameObfuscation.mEnable || !this.mCustomProfiles.mEnableNameCache) {
      renameIdentifierModule.nameCache = undefined;
      renamePropertyModule.mangledTable = undefined;
      return;
    }

    const nameCache: NameCacheInfo = {};
    addIdentifierNameCache(nameCache, renameIdentifierModule.nameCache);

    if (this.mCustomProfiles.mNameObfuscation.mRenameProperties) {
      addPropertyNameCache(nameCache, renamePropertyModule.mangledTable);
    }

    const nameCachePath: string = path.join(outputDir, FileUtils.getFileName(sourceFile) + '.cache');
    writeNameCache(nameCache, nameCachePath);

    renameIdentifierModule.nameCache = undefined;
    renamePropertyModule.mangledTable = undefined;
  }

  /**
   * Obfuscate single source file
   *
   * @param sourceFile single source file path
   * @param outputDir
   */
  public obfuscateFile(sourceFilePath: string, outputDir: string): string {
    const fileName: string = FileUtils.getFileName(sourceFilePath);
    console.debug(`Now we will process ${fileName}`);
    let suffix: string = FileUtils.getFileExtension(sourceFilePath);

    if ((suffix !== 'js' && suffix !== 'ts') || fileName.endsWith('.d.ts')) {  // || fileName === 'hvigorfile.js'
      fs.copyFileSync(sourceFilePath, path.join(outputDir, fileName));
      return;
    }

    // Advanced confusion requires calling this function
    // this.collectNarrowFunctions(sourceFile);

    // 1.generate ast
    let content: string = FileUtils.readFile(sourceFilePath);
    let ast: SourceFile = createSourceFile(sourceFilePath, content, ScriptTarget.ES2015, true);

    // 2. obfuscate ast
    const mixedInfo: {content: string, sourceMap: string} = this.obfuscate(ast, sourceFilePath, outputDir);

    // 3. write to output directory
    if (outputDir) {
      fs.writeFileSync(path.join(outputDir, FileUtils.getFileName(sourceFilePath)), mixedInfo.content);
      if (this.mCustomProfiles.mEnableSourceMap && mixedInfo.sourceMap) {
        fs.writeFileSync(path.join(outputDir, FileUtils.getFileName(sourceFilePath) + '.map'), mixedInfo.sourceMap);
      }
      this.produceNameCache(outputDir, sourceFilePath);
    }
    // 4. clear cache of text writer
    this.mTextWriter.clear();
  }

  /**
   * Obfuscate ast of a file.
   * @param ast ast of a source file
   */
  private obfuscate(ast: SourceFile, sourceFilePath: string, outputDir: string): any {
    if (ast.statements.length === 0) {
      return ast;
    }

    this.consumeNameCache(sourceFilePath, outputDir);

    let transformedResult: TransformationResult<Node> = transform(ast, this.mTransformers, this.mCompilerOptions);
    ast =  transformedResult.transformed[0] as SourceFile;

    // convert ast to output source file and generate sourcemap if needed.
    let sourceMapGenerator: SourceMapGenerator = undefined;
    if (this.mCustomProfiles.mEnableSourceMap) {
      sourceMapGenerator = getSourceMapGenerator(sourceFilePath);
    }

    this.mPrinter.writeFile(ast, this.mTextWriter, sourceMapGenerator);

    const result = {
      content: this.mTextWriter.getText()
    }

    if (this.mCustomProfiles.mEnableSourceMap) {
       result['sourceMap'] =sourceMapGenerator.toString()
    }
    
    return result;
  }
}
