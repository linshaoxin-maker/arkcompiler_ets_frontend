/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

export class IncrementalLinterState {
  private readonly changedFiles: ts.Set<ts.Path> = new ts.Set<ts.Path>();
  private readonly programState: ts.ReusableBuilderProgramState;
  private readonly oldDiagnostics:
    | ts.ESMap<ts.Path, readonly ts.ReusableDiagnostic[] | readonly ts.Diagnostic[]>
    | undefined;

  constructor(builderProgram: ts.BuilderProgram, arkTSVersion?: string) {
    this.programState = builderProgram.getState();
    this.oldDiagnostics = this.programState.arktsLinterDiagnosticsPerFile;
    this.programState.arktsLinterDiagnosticsPerFile = new Map();
    this.changedFiles = IncrementalLinterState.collectChangedFilesFromProgramState(this.programState, arkTSVersion);
  }

  isFileChanged(srcFile: ts.SourceFile): boolean {
    return this.changedFiles.has(srcFile.resolvedPath);
  }

  getOldDiagnostics(srcFile: ts.SourceFile): ts.Diagnostic[] {
    return (this.oldDiagnostics?.get(srcFile.resolvedPath) as ts.Diagnostic[]) ?? [];
  }

  updateDiagnostics(srcFile: ts.SourceFile, newDiagnostics: ts.Diagnostic[]): void {
    this.programState.arktsLinterDiagnosticsPerFile?.set(srcFile.resolvedPath, newDiagnostics);
  }

  updateProgramStateArkTSVersion(arkTSVersion?: string): void {
    this.programState.arkTSVersion = arkTSVersion;
  }

  static emitBuildInfo(buildInfoWriteFile: ts.WriteFileCallback, builderProgram: ts.BuilderProgram): void {
    builderProgram.emitBuildInfo(buildInfoWriteFile);
  }

  private static collectChangedFilesFromProgramState(
    state: ts.ReusableBuilderProgramState,
    arkTSVersion?: string
  ): ts.Set<ts.Path> {

    /*
     * If old arkTSVersion from last run is not same current arkTSVersion from ets_loader,
     * then process all files in project.
     */
    if (state.arkTSVersion !== arkTSVersion) {
      return new ts.Set<ts.Path>(IncrementalLinterState.arrayFrom(state.fileInfos.keys()));
    }

    const changedFiles = new ts.Set<ts.Path>(state.changedFilesSet);

    /*
     * If any source file that affects global scope has been changed,
     * then process all files in project.
     */
    for (const changedFile of IncrementalLinterState.arrayFrom(changedFiles.keys())) {
      const fileInfo = state.fileInfos.get(changedFile);
      if (fileInfo?.affectsGlobalScope) {
        return new ts.Set<ts.Path>(IncrementalLinterState.arrayFrom(state.fileInfos.keys()));
      }
    }

    if (!state.referencedMap) {
      return changedFiles;
    }

    const seenPaths = new ts.Set<ts.Path>();
    const queue = IncrementalLinterState.arrayFrom(changedFiles.keys());
    while (queue.length) {
      const path = queue.pop()!;
      if (!seenPaths.has(path)) {
        seenPaths.add(path);

        // Collect all files that import this file
        queue.push(...ts.BuilderState.getReferencedByPaths(state, path));
      }
    }
    return seenPaths;
  }

  private static arrayFrom(iterator: Iterator<ts.Path>): ts.Path[] {
    const result: ts.Path[] = [];
    for (let iterResult = iterator.next(); !iterResult.done; iterResult = iterator.next()) {
      result.push(iterResult.value);
    }
    return result;
  }
}
