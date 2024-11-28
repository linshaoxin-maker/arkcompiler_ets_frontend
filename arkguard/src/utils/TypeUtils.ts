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
  ScriptTarget,
  createCompilerHost,
  createProgram,
  createSourceFile,
  SymbolFlags
} from 'typescript';

import type {
  CompilerHost,
  CompilerOptions,
  Program,
  SourceFile,
  TypeChecker,
  Symbol
} from 'typescript';
import { Extension, PathAndExtension } from '../common/type';
import { FileUtils } from './FileUtils';
import { EventList, performancePrinter } from '../ArkObfuscator';
import { endSingleFileEvent, startSingleFileEvent } from './PrinterUtils';

export class TypeUtils {
  /**
   * Create .d.ets, .d.ts, .ts ast from .d.ets, .d.ts, .ts content.
   * Create .ts ast from .ets, .js content
   * @param {string} sourceFilePath 
   * @param {string} content - The content in sourceFilePath
   */
  public static createObfSourceFile(sourceFilePath: string, content: string): SourceFile {
    const pathOrExtension: PathAndExtension = FileUtils.getFileSuffix(sourceFilePath);
    const fileSuffix = pathOrExtension.ext;

    if (fileSuffix === Extension.JS) {
      sourceFilePath = pathOrExtension.path + Extension.TS;
    }

    return createSourceFile(sourceFilePath, content, ScriptTarget.ES2015, true);
  }

  public static tsToJs(ast: SourceFile): void {
    const pathOrExtension: PathAndExtension = FileUtils.getFileSuffix(ast.fileName);
    const fileSuffix = Extension.JS;
    const targetName: string = pathOrExtension.path + fileSuffix;
    ast.fileName = targetName;
  }

  public static createChecker(ast: SourceFile): TypeChecker {
    const host: CompilerHost = createCompilerHost({});

    const customHost: CompilerHost = {
      getSourceFile(name, languageVersion): SourceFile | undefined {
        if (name === ast.fileName) {
          return ast;
        } else {
          return host.getSourceFile(name, languageVersion);
        }
      },
      // optional
      getDefaultLibLocation: () => '',
      getDefaultLibFileName: () => '',
      writeFile: (filename, data) => {
      },
      getCurrentDirectory: () => '',
      useCaseSensitiveFileNames: host.useCaseSensitiveFileNames,
      getCanonicalFileName: host.getCanonicalFileName,
      getNewLine: host.getNewLine,
      fileExists: () => true,
      readFile: (name): string => {
        return name === ast.fileName ? ast.text : host.readFile(name);
      },
      // must, read program.ts => createCompilerHost
      directoryExists: undefined,
      getEnvironmentVariable: undefined,
      getDirectories: undefined,
    };

    let option: CompilerOptions = {};
    if (ast.fileName.endsWith('.js')) {
      option.allowJs = true;
    }

    startSingleFileEvent(EventList.CREATE_PROGRAM, performancePrinter.timeSumPrinter);
    let program: Program = createProgram([ast.fileName], option, customHost);
    endSingleFileEvent(EventList.CREATE_PROGRAM, performancePrinter.timeSumPrinter);

    startSingleFileEvent(EventList.GET_CHECKER, performancePrinter.timeSumPrinter);
    let typeChecker: TypeChecker = program.getTypeChecker();
    endSingleFileEvent(EventList.GET_CHECKER, performancePrinter.timeSumPrinter);
    return typeChecker;
  }

  /**
   * Retrieves the symbol associated with the declaration site of the given symbol.
   *
   * This method resolves the symbol to its declaration site to ensure consistency
   * in obfuscated naming. Obfuscation names are bound to the symbol at the declaration
   * site. If the symbol at the declaration site differs from the symbol at the usage
   * site, discrepancies in obfuscated names may occur. By using this method, the
   * obfuscation name can be consistently retrieved from the declaration site symbol,
   * ensuring uniformity between the declaration and usage sites.
   */
  public static getOriginalSymbol(symbol: Symbol, checker: TypeChecker): Symbol {
    if (symbol.getFlags() & SymbolFlags.Alias) {
      let originalSymbol: Symbol = checker.getAliasedSymbol(symbol);
      if (originalSymbol && originalSymbol.name === symbol.name) {
        // Only replace `symbol` with `originalSymbol` if their associated nodes have the same name.
        // When the names differ, separate obfuscated names need to be generated for each symbol 
        symbol = originalSymbol;
      }
    }
    return symbol;
  }
}
