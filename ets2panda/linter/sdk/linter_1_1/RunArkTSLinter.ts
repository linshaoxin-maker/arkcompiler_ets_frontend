/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, softwareP
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import * as ts from 'typescript';
import * as path from 'node:path';
import type { ProblemInfo } from '../../lib/ProblemInfo';
import { ProblemSeverity } from '../../lib/ProblemSeverity';
import { TypeScriptLinter } from '../../lib/TypeScriptLinter';
import { getTscDiagnostics } from '../../lib/ts-diagnostics/GetTscDiagnostics';
import { ArkTSLinterTimePrinter, TimePhase } from '../ArkTSTimePrinter';
import { getScriptKind } from '../../lib/utils/functions/GetScriptKind';
import { SdkTSCCompiledProgram } from './SdkTSCCompiledProgram';
import { IncrementalLinterState } from './incrementalLinter';
import { InteropTypescriptLinter } from '../../lib/InteropTypescriptLinter';

function makeDiag(
  category: ts.DiagnosticCategory,
  code: number,
  file: ts.SourceFile,
  start: number,
  length: number,
  messageText: string
): ts.Diagnostic {
  return { category, code, file, start, length, messageText };
}

export function translateDiag(srcFile: ts.SourceFile, problemInfo: ProblemInfo): ts.Diagnostic {
  const LINTER_MSG_CODE_START = -1;
  const severity =
    problemInfo.severity === ProblemSeverity.ERROR ? ts.DiagnosticCategory.Error : ts.DiagnosticCategory.Warning;
  return makeDiag(
    severity,
    LINTER_MSG_CODE_START /* + problemInfo.ruleTag */,
    srcFile,
    problemInfo.start,
    problemInfo.end - problemInfo.start + 1,
    problemInfo.rule
  );
}

export function runArkTSLinter(
  tsBuilderProgram: ts.BuilderProgram,
  srcFile?: ts.SourceFile,
  buildInfoWriteFile?: ts.WriteFileCallback,
  arkTSVersion?: string,
  needAutoFix?: boolean,
  isUseRtLogic?: boolean
): ts.Diagnostic[] {
  const diagnostics: ts.Diagnostic[] = [];
  const tscDiagnosticsLinter = new SdkTSCCompiledProgram(tsBuilderProgram);
  const program = tscDiagnosticsLinter.getProgram();
  const incrementalLinterState = new IncrementalLinterState(tsBuilderProgram, arkTSVersion);
  incrementalLinterState.updateProgramStateArkTSVersion(arkTSVersion);
  const timePrinterInstance = ArkTSLinterTimePrinter.getInstance();
  timePrinterInstance.appendTime(TimePhase.INIT);
  tscDiagnosticsLinter.updateCompilationDiagnostics();
  const srcFiles: ts.SourceFile[] = getSrcFiles(program, srcFile);
  const tscStrictDiagnostics = getTscDiagnostics(
    tscDiagnosticsLinter,
    srcFiles.filter((file) => {
      return incrementalLinterState.isFileChanged(file);
    })
  );
  timePrinterInstance.appendTime(TimePhase.GET_TSC_DIAGNOSTICS);
  const etsLoaderPath = program.getCompilerOptions().etsLoaderPath;
  const tsImportSendableEnable = program.getCompilerOptions().tsImportSendableEnable;
  const linter = createTypeScriptLinter(program, tscStrictDiagnostics, needAutoFix, isUseRtLogic);
  const interopTypescriptLinter = createInteropTypescriptLinter(
    program,
    tsBuilderProgram.getCompilerOptions(),
    isUseRtLogic
  );
  processFiles(
    incrementalLinterState,
    linter,
    interopTypescriptLinter,
    srcFiles,
    tscStrictDiagnostics,
    diagnostics,
    etsLoaderPath,
    tsImportSendableEnable
  );
  timePrinterInstance.appendTime(TimePhase.LINT);
  if (buildInfoWriteFile) {
    IncrementalLinterState.emitBuildInfo(buildInfoWriteFile, tscDiagnosticsLinter.getBuilderProgram());
    timePrinterInstance.appendTime(TimePhase.EMIT_BUILD_INFO);
  }
  releaseResources();
  return diagnostics;
}

function processFiles(
  incrementalLinterState: IncrementalLinterState,
  linter: TypeScriptLinter,
  interopTypescriptLinter: InteropTypescriptLinter,
  srcFiles: ts.SourceFile[],
  tscStrictDiagnostics: Map<string, ts.Diagnostic[]>,
  diagnostics: ts.Diagnostic[],
  etsLoaderPath?: string,
  tsImportSendableEnable?: boolean
): void {
  for (const fileToLint of srcFiles) {
    const scriptKind = getScriptKind(fileToLint);
    if (scriptKind !== ts.ScriptKind.ETS && scriptKind !== ts.ScriptKind.TS) {
      return;
    }
    
    const currentDiagnostics = getDiagnostic(
      incrementalLinterState,
      linter,
      interopTypescriptLinter,
      fileToLint,
      tscStrictDiagnostics,
      scriptKind,
      etsLoaderPath,
      tsImportSendableEnable
    );
    diagnostics.push(...currentDiagnostics);
    incrementalLinterState.updateDiagnostics(fileToLint, currentDiagnostics);
  }
}

function getDiagnostic(
  incrementalLinterState: IncrementalLinterState,
  linter: TypeScriptLinter,
  interopTypescriptLinter: InteropTypescriptLinter,
  fileToLint: ts.SourceFile,
  tscStrictDiagnostics: Map<string, ts.Diagnostic[]>,
  scriptKind: ts.ScriptKind,
  etsLoaderPath?: string,
  tsImportSendableEnable?: boolean
): ts.Diagnostic[] {
  let currentDiagnostics: ts.Diagnostic[] = [];
  if (incrementalLinterState.isFileChanged(fileToLint)) {
    if (scriptKind === ts.ScriptKind.ETS) {
      linter.lint(fileToLint);

      // Get list of bad nodes from the current run.
      currentDiagnostics = tscStrictDiagnostics.get(path.normalize(fileToLint.fileName)) ?? [];
      linter.problemsInfos.forEach((x) => {
        return currentDiagnostics.push(translateDiag(fileToLint, x));
      });
    } else {
      const isKit = path.basename(fileToLint.fileName).toLowerCase().
        indexOf('@kit.') === 0;
      const isInSdk = etsLoaderPath ?
        path.normalize(fileToLint.fileName).indexOf(path.resolve(etsLoaderPath, '../..')) === 0 :
        false;
      const isInOhModules = ts.isOHModules(fileToLint.fileName);
      if (isKit || isInOhModules || !tsImportSendableEnable && !isInSdk) {
        return currentDiagnostics;
      }

      interopTypescriptLinter.lint(fileToLint);
      interopTypescriptLinter.problemsInfos.forEach((x) => {
        return currentDiagnostics.push(translateDiag(fileToLint, x));
      });
    }
  } else {
    // Get diagnostics from old run.
    currentDiagnostics = incrementalLinterState.getOldDiagnostics(fileToLint);
  }
  return currentDiagnostics;
}

function getSrcFiles(program: ts.Program, srcFile?: ts.SourceFile): ts.SourceFile[] {
  let srcFiles: ts.SourceFile[] = [];
  if (srcFile) {
    srcFiles.push(srcFile);
  } else {
    srcFiles = program.getSourceFiles() as ts.SourceFile[];
  }
  return srcFiles;
}

function createTypeScriptLinter(
  program: ts.Program,
  tscStrictDiagnostics: Map<string, ts.Diagnostic[]>,
  needAutoFix?: boolean,
  isUseRtLogic?: boolean
): TypeScriptLinter {
  TypeScriptLinter.initGlobals();
  TypeScriptLinter.ideMode = true;

  return new TypeScriptLinter(
    program.getLinterTypeChecker(),
    !!needAutoFix,
    !!isUseRtLogic,
    undefined,
    undefined,
    tscStrictDiagnostics
  );
}

function createInteropTypescriptLinter(
  program: ts.Program,
  compileOptions: ts.CompilerOptions,
  isUseRtLogic?: boolean
): InteropTypescriptLinter {
  return new InteropTypescriptLinter(program.getLinterTypeChecker(), compileOptions, undefined, !!isUseRtLogic);
}

// Reclaim memory for Hvigor with "no-parallel" and "daemon".
function releaseResources(): void {}
