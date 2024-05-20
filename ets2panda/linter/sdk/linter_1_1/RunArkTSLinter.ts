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
import { getScriptKind } from '../GetScriptKind';
import { SdkTSCCompiledProgram } from './SdkTSCCompiledProgram';

function makeDiag(category: ts.DiagnosticCategory, code: number, file: ts.SourceFile,
  start: number, length: number, messageText: string): ts.Diagnostic {
  return { category, code, file, start, length, messageText };
}

export function translateDiag(srcFile: ts.SourceFile, problemInfo: ProblemInfo): ts.Diagnostic {
  const LINTER_MSG_CODE_START = -1;
  const severity = problemInfo.severity === ProblemSeverity.ERROR ?
    ts.DiagnosticCategory.Error :
    ts.DiagnosticCategory.Warning;
  return makeDiag(severity,
    LINTER_MSG_CODE_START /* + problemInfo.ruleTag */,
    srcFile,
    problemInfo.start,
    problemInfo.end - problemInfo.start + 1,
    problemInfo.rule);
}

export function runArkTSLinter(tsBuilderProgram: ts.BuilderProgram, srcFile?: ts.SourceFile,
  buildInfoWriteFile?: ts.WriteFileCallback, arkTSVersion?: string): ts.Diagnostic[] {
  const diagnostics: ts.Diagnostic[] = [];

  /*
   * Initialize incremental linter state. Since this call collects changed files
   * from the old program state, it needs to be done before re-evaluating program
   * diagnostics through the call 'tscDiagnosticsLinter.updateCompilationDiagnostics()'
   * below, as it will update program state, clearing the changedFiles list in program state.
   */
  const tscDiagnosticsLinter = new SdkTSCCompiledProgram(tsBuilderProgram);
  const program = tscDiagnosticsLinter.getProgram();
  const incrementalLinterState = new ts.incrementalLinter.IncrementalLinterState(tsBuilderProgram, arkTSVersion);
  incrementalLinterState.updateProgramStateArkTSVersion(arkTSVersion);
  const timePrinterInstance = ArkTSLinterTimePrinter.getInstance();
  timePrinterInstance.appendTime(TimePhase.INIT);

  tscDiagnosticsLinter.updateCompilationDiagnostics();
  const srcFiles: ts.SourceFile[] = getSrcFiles(program, srcFile);
  const tscStrictDiagnostics = getTscDiagnostics(tscDiagnosticsLinter, srcFiles.filter((file) => {
    return incrementalLinterState.isFileChanged(file);
  }));
  timePrinterInstance.appendTime(TimePhase.GET_TSC_DIAGNOSTICS);

  const linter = createTypeScriptLinter(program, tscStrictDiagnostics);

  for (const fileToLint of srcFiles) {
    if (getScriptKind(fileToLint) !== ts.ScriptKind.ETS) {
      continue;
    }

    const currentDiagnostics = getDiagnostic(incrementalLinterState, linter, fileToLint, tscStrictDiagnostics);
    diagnostics.push(...currentDiagnostics);

    // Add linter diagnostics to new cache.
    incrementalLinterState.updateDiagnostics(fileToLint, currentDiagnostics);
  }

  timePrinterInstance.appendTime(TimePhase.LINT);

  // Write tsbuildinfo file only after we cached the linter diagnostics.
  if (buildInfoWriteFile) {
    incrementalLinterState.emitBuildInfo(buildInfoWriteFile, tscDiagnosticsLinter.getBuilderProgram());
    timePrinterInstance.appendTime(TimePhase.EMIT_BUILD_INFO);
  }

  releaseResources();
  return diagnostics;
}

function getDiagnostic(incrementalLinterState: ts.incrementalLinter.IncrementalLinterState,
  linter: TypeScriptLinter, fileToLint: ts.SourceFile,
  tscStrictDiagnostics: Map<string, ts.Diagnostic[]>): ts.Diagnostic[] {
  let currentDiagnostics: ts.Diagnostic[];
  if (incrementalLinterState.isFileChanged(fileToLint)) {
    linter.lint(fileToLint);

    // Get list of bad nodes from the current run.
    currentDiagnostics = tscStrictDiagnostics.get(path.normalize(fileToLint.fileName)) ?? [];
    linter.problemsInfos.forEach((x) => {
      return currentDiagnostics.push(translateDiag(fileToLint, x));
    });
    linter.problemsInfos.length = 0;
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

function createTypeScriptLinter(program: ts.Program,
  tscStrictDiagnostics: Map<string, ts.Diagnostic[]>): TypeScriptLinter {
  TypeScriptLinter.initGlobals();
  TypeScriptLinter.ideMode = true;

  return new TypeScriptLinter(
    program.getLinterTypeChecker(),
    false,
    undefined,
    undefined,
    tscStrictDiagnostics
  );
}

// Reclaim memory for Hvigor with "no-parallel" and "daemon".
function releaseResources(): void {
}
