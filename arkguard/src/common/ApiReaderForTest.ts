import { ArkObfuscator } from '../ArkObfuscator';
import { IOptions } from '../configs/IOptions';
import { readProjectPropertiesByCollectedPaths } from "./ApiReader";
import fs from 'fs';
import path from 'path'

/**
 * 递归获取文件夹下的所有子文件路径
 * @param dir - 需要遍历的文件夹路径
 * @returns 返回所有子文件路径的数组
 */
function getAllFiles(inputPaths: string[]): Set<string> {
  let results: Set<string> = new Set();
  for (let inputPath of inputPaths) {
    const stat = fs.statSync(inputPath);
    if (stat.isDirectory()) {
      recursiveRead(inputPath);
    } else if (stat.isFile()) {
      results.add(inputPath);
    }
  }
  function recursiveRead(dir: string): void {
    const list = fs.readdirSync(dir);
    list.forEach((file) => {
      const filePath = path.join(dir, file);
      const stat = fs.statSync(filePath);

      if (stat && stat.isDirectory()) {
        recursiveRead(filePath);
      } else {
        results.add(filePath);
      }
    });
  }

  return results;
}

/**
 * read project reserved properties for UT
 * @param projectPaths can be dir or file
 * @param customProfiles
 */
export function readProjectProperties(projectPaths: string[], customProfiles: IOptions, arkObfuscator: ArkObfuscator): void {
  const allPaths: Set<string> = getAllFiles(projectPaths);
  let projectAndLibs: { projectAndLibsReservedProperties: string[]; libExportNames: string[] } =
    readProjectPropertiesByCollectedPaths(allPaths, customProfiles, false);

  if (customProfiles.mNameObfuscation.mReservedProperties && projectAndLibs.projectAndLibsReservedProperties) {
    arkObfuscator.addReservedProperties(projectAndLibs.projectAndLibsReservedProperties);
  }
  if (customProfiles.mExportObfuscation && projectAndLibs.libExportNames) {
    arkObfuscator.addReservedNames(projectAndLibs.libExportNames);
    arkObfuscator.addReservedToplevelNames(projectAndLibs.libExportNames);
  }
}