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
import { ArkTSProgram } from '../ArkTSProgram';
import { ArkTSLinterTimePrinter, TimePhase } from '../ArkTSTimePrinter';
import { TSCCompiledProgram } from './lib/ts-diagnostics/TSCCompiledProgram';
import { getStrictDiagnostics } from './lib/ts-diagnostics/TypeScriptDiagnosticsExtractor';

export class SdkTSCCompiledProgram implements TSCCompiledProgram {
  private wasStrict: boolean;
  private strictProgram: ts.BuilderProgram;
  private nonStrictProgram: ts.BuilderProgram;

  constructor(program: ArkTSProgram, reverseStrictBuilderProgram: ArkTSProgram) {
    this.strictProgram = program.wasStrict ? program.builderProgram : reverseStrictBuilderProgram.builderProgram;
    this.nonStrictProgram = program.wasStrict ? reverseStrictBuilderProgram.builderProgram : program.builderProgram;
    this.wasStrict = program.wasStrict;
  }

  public getProgram(): ts.Program {
    return this.getStrictProgram();
  }

  public getOriginalProgram(): ts.Program {
    return this.wasStrict
      ? this.strictProgram.getProgram()
      : this.nonStrictProgram.getProgram();
  }

  public getStrictProgram(): ts.Program {
    return this.strictProgram.getProgram();
  }

  public getNonStrictProgram(): ts.Program {
    return this.nonStrictProgram.getProgram();
  }

  public getStrictBuilderProgram(): ts.BuilderProgram {
    return this.strictProgram;
  }

  public getNonStrictBuilderProgram(): ts.BuilderProgram {
    return this.nonStrictProgram;
  }

  public getStrictDiagnostics(fileName: string): ts.Diagnostic[] {
    return getStrictDiagnostics(this.getStrictProgram(), this.getNonStrictProgram(), fileName);
  }

  /**
   * Updates all diagnostics in TSC compilation program after the incremental build.
   */
  public updateCompilationDiagnostics() {
    this.strictProgram.getSemanticDiagnostics();
    const timePrinterInstance = ArkTSLinterTimePrinter.getInstance();
    timePrinterInstance.appendTime(TimePhase.STRICT_PROGRAM_GET_SEMANTIC_DIAGNOSTICS);
    this.strictProgram.getSyntacticDiagnostics();
    timePrinterInstance.appendTime(TimePhase.STRICT_PROGRAM_GET_SYNTACTIC_DIAGNOSTICS);
    this.nonStrictProgram.getSemanticDiagnostics();
    timePrinterInstance.appendTime(TimePhase.NON_STRICT_PROGRAM_GET_SEMANTIC_DIAGNOSTICS);
    this.nonStrictProgram.getSyntacticDiagnostics();
    timePrinterInstance.appendTime(TimePhase.NON_STRICT_PROGRAM_GET_SYNTACTIC_DIAGNOSTICS);
  }
}
