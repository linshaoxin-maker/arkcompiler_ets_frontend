/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import * as ts from 'typescript';
import type { CommandLineOptions } from '../lib/CommandLineOptions';
import { formTscOptions } from './ts-compiler/FormTscOptions';
import { logTscDiagnostic } from '../lib/utils/functions/LogTscDiagnostic';
import { consoleLog } from '../lib/TypeScriptLinter';
import type { LintOptions } from '../lib/LintOptions';
import { TSCCompiledProgramWithDiagnostics } from '../lib/ts-diagnostics/TSCCompiledProgram';

function compile(cmdOptions: CommandLineOptions, overrideCompilerOptions: ts.CompilerOptions): ts.Program {
  const createProgramOptions = formTscOptions(cmdOptions, overrideCompilerOptions);
  const program = ts.createProgram(createProgramOptions);
  // Log Tsc errors if needed
  if (cmdOptions.logTscErrors) {
    const diagnostics = ts.getPreEmitDiagnostics(program);
    logTscDiagnostic(diagnostics, consoleLog);
    diagnostics.forEach((diagnostic) => {
      if (diagnostic.file && diagnostic.start) {
        const { line, character } = ts.getLineAndCharacterOfPosition(diagnostic.file, diagnostic.start);
        const message = ts.flattenDiagnosticMessageText(diagnostic.messageText, '\n');
        consoleLog(`${diagnostic.file.fileName} (${line + 1}, ${character + 1}): ${message}`);
      } else {
        consoleLog(ts.flattenDiagnosticMessageText(diagnostic.messageText, '\n'));
      }
    });
  }
  return program;
}

export function compileLintOptions(cmdOptions: CommandLineOptions): LintOptions {
  const strict = compile(cmdOptions, TSCCompiledProgramWithDiagnostics.getOverrideCompilerOptions(true));
  const nonStrict = compile(cmdOptions, TSCCompiledProgramWithDiagnostics.getOverrideCompilerOptions(false));
  return {
    cmdOptions: cmdOptions,
    tscCompiledProgram: new TSCCompiledProgramWithDiagnostics(strict, nonStrict, cmdOptions.inputFiles)
  };
}
