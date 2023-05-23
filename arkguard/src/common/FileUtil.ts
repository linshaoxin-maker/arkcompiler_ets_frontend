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

import * as fs from 'fs';
import * as path from 'path';

/**
 * create file name of api list json object save file
 * @param version version
 * @param isEts
 * @private
 */
export function createFileName(version: string, isEts: boolean): string {
  return (isEts ? 'ets' : 'js') + version + '.json';
}

/**
 * save api json object to file
 * @private
 */
export function writeToFile(reservedProperties: string[], version: string, isEts: boolean, outputDir: string): void {
  if (!fs.existsSync(outputDir)) {
    fs.mkdirSync(outputDir);
  }

  let fileName: string = createFileName(version, isEts);
  fileName = path.join(outputDir, fileName);

  let str: string = JSON.stringify(reservedProperties, null, '\t');
  fs.writeFileSync(fileName, str);
}
