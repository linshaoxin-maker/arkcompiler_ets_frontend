/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

import { readFileSync, writeFileSync } from 'fs';
import { join } from 'path';

const COPYRIGHT_HEADER =
  '/* \n\
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd. \n\
 * Licensed under the Apache License, Version 2.0 (the "License"); \n\
 * you may not use this file except in compliance with the License. \n\
 * You may obtain a copy of the License at \n\
 * \n\
 * http://www.apache.org/licenses/LICENSE-2.0 \n\
 * \n\
 * Unless required by applicable law or agreed to in writing, software \n\
 * distributed under the License is distributed on an "AS IS" BASIS, \n\
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. \n\
 * See the License for the specific language governing permissions and \n\
 * limitations under the License. \n\
 */ \n\
';

const CODE_PROLOGUE =
  'export const cookBookMsg: string[] = [];\n\
export const cookBookTag: string[] = [];\n\
\n\
for( let i = 0; i <= 150; i++) {\n\
  cookBookMsg[ i ] = \'\';\n\
}\n\
';

// HTML tags
const T_BR = '<br>';
const T_BOLD = '<b>';
const T_END_BOLD = '</b>';
const T_CODE = '<code>';
const T_END_CODE = '</code>';
const T_NBSP = '&nbsp;';
const T_HR = '<hr style="height:3px;">';

// RST substititions
const CB_PREFIX = '|CB_PREFIX';
const CB_R = '|CB_R|';
const CB_RULE = '|CB_RULE|';
const CB_BAD = '|CB_BAD|';
const CB_OK = '|CB_OK|';
// replace:: **Severity: error**
const CB_ERROR = '|CB_ERROR|';
// replace:: **Severity: warning**
const CB_WARNING = '|CB_WARNING|';
const CB_SEE = '|CB_SEE|';
const CB_REF = ':ref:';
const CB_META = '.. meta';
const CB_FIX = ':fix:';

const NEW_REC_HEADER = /.. _R\d\d\d:/;
// should be ".. code-block" but in some places there is error in doc file
const CODE_BLOCK = '.. code';

let docLines: string[];
let curLine: number;
let recNum: number;

const tegs: string[] = [];
const ruleNames: string[] = [];
let mdText: string[] = [];
const fixTitles: Map<number, string> = new Map();

// continue line
const CL = ' \\';
const STR_DLMTR = '\'';

function syncReadFile(filename: string): string[] {
  const contents = readFileSync(filename, 'utf-8');

  docLines = contents.split(/\r?\n/);

  // make table of rule names
  curLine = 0;
  let ruleNum = -1;
  while (curLine < docLines.length) {
    const line = docLines[curLine];
    if (NEW_REC_HEADER.test(line)) {
      ruleNum = Number(line.replace(/\D/g, ''));
      console.log('>>>>>>> START RULE ' + ruleNum + ':');
      console.log('                    NUMBER: ' + ruleNum);
    }
    if (docLines[curLine].startsWith(CB_R)) {
      let line = docLines[curLine].split(CB_R)[1];
      ruleNames[ruleNum] = line;
      curLine++;
      needHeader();
      if (docLines[curLine].startsWith(CB_RULE)) {
        line = docLines[curLine].trim().replace(CB_RULE, '').
          trim();
        ruleNames[ruleNum] = ruleNames[ruleNum] + ' (' + line + ')';
      }
    }
    curLine++;
  }

  // scan text
  curLine = 0;
  while (curLine < docLines.length) {
    skipEmptyLines();
    const line = docLines[curLine];
    if (NEW_REC_HEADER.test(line)) {
      makeRecipe();
    } else {
      curLine++;
    }
  }

  return docLines;
}

/*
 *
 * utility functions
 *
 */

function replaceAll(s: string, from: string, to: string): string {
  const ss = s.split(from);
  let outStr = '';
  ss.forEach((line) => {
    outStr += to + line;
  });

  // remove 1st 'to' substring
  return outStr.replace(to, '');
}

function translateLine(s: string): string {
  let line = s;
  line = line.replace(CB_BAD, 'TypeScript');
  line = line.replace(CB_OK, 'ArkTS');

  line = line.replace(CB_ERROR, '**Severity: error**');
  line = line.replace(CB_WARNING, '**Severity: warning**');
  line = line.replace(CB_SEE, '## See also');

  line = replaceAll(line, '|JS|', 'JavaScript');
  line = replaceAll(line, '|LANG|', 'ArkTS');
  line = replaceAll(line, '|TS|', 'TypeScript');

  return line;
}

function translateTeg(s: string): string {
  return replaceAll(s, '``', '"').trim();
}

function highlightCode(s: string): string {
  const ss = s.split('``');
  let line = ss[0];
  for (let i = 1; i < ss.length; i++) {
    if (i % 2 === 0) {
      line += T_END_CODE;
    } else {
      line += T_CODE;
    }
    line += ss[i];
  }
  return line;
}

function escapeSym(s: string): string {
  const ss = replaceAll(s, '\'', '\\\'');
  return replaceAll(ss, '"', '\\"');
}

function setNBSP(s: string): string {
  let ss = '';
  let flag = true;
  for (const ch of s) {
    if (ch !== ' ' && ch !== '\t') {
      flag = false;
    }
    if (flag && ch === ' ') {
      ss += T_NBSP;
    } else if (flag && ch === '\t') {
      ss += T_NBSP + T_NBSP + T_NBSP + T_NBSP + T_NBSP + T_NBSP + T_NBSP + T_NBSP;
    } else {
      ss += ch;
    }
  }
  return ss;
}

function skipEmptyLines(): void {
  while (curLine < docLines.length) {
    let s = docLines[curLine];
    s = s.trim();
    if (s !== '') {
      break;
    }
    curLine++;
  }
}

function isHeader(): boolean {
  return docLines[curLine].startsWith(CB_PREFIX) || docLines[curLine].startsWith('..');
}

function needHeader(): void {
  while (curLine < docLines.length && !isHeader()) {
    curLine++;
  }
}

function isFixTitle(): boolean {
  return docLines[curLine].trimStart().startsWith(CB_FIX);
}

/*
 *
 * parsing functions
 *
 */

function makeFixTitle(): void {
  while (curLine < docLines.length && !isHeader() && !isFixTitle()) {
    curLine++;
  }

  if (isFixTitle()) {
    const title = docLines[curLine].split(CB_FIX)[1].trim();
    fixTitles.set(recNum, escapeSym(title));
  }
}

function makeRecipe(): void {
  const line = docLines[curLine];
  recNum = Number(line.replace(/\D/g, ''));
  console.log('cookBookMsg[ ' + recNum + ' ] = ' + STR_DLMTR + CL);
  curLine++;
  mdText = [];
  makeTag();
  makeBody();
  makeBad();
  makeOk();
  makeSee();

  // emit .md file
  const mdFileName = join('./md', 'recipe' + recNum + '.md');
  writeFileSync(mdFileName, '', { flag: 'w' });
  mdText.forEach((mdLine) => {
    console.error('MD> ' + mdLine);
    writeFileSync(mdFileName, mdLine + '\n', { flag: 'a+' });
  });

  console.log(STR_DLMTR + ';');
  console.log('');
}

function makeTag(): void {
  needHeader();
  console.error('>>>TEG>>>: ' + curLine + ' -> ' + docLines[curLine]);
  if (!docLines[curLine].startsWith(CB_R)) {
    return;
  }
  let line = docLines[curLine].split(CB_R)[1];

  mdText.push('# ' + translateLine(line));
  mdText.push('');

  line = escapeSym(translateLine(line));
  const teg = translateTeg(line);
  const hdr = highlightCode(line);
  console.log(hdr + T_BR + CL);
  tegs[recNum] = teg;
  curLine++;
}

function makeBody(): string {
  let body = '';
  needHeader();
  console.error('>>>BODY HDR>>>: ' + +curLine + ' -> ' + docLines[curLine]);
  if (!docLines[curLine].startsWith(CB_RULE)) {
    return '';
  }

  let line = docLines[curLine].trim();
  const mdLine = line;
  line = line.replace(CB_RULE, '');
  line = escapeSym(translateLine(line));
  tegs[recNum] = tegs[recNum].trim() + ' (' + replaceAll(translateTeg(line), '"', '') + ')';
  curLine++;

  // skip underline
  curLine++;
  console.log(T_HR + T_BOLD + 'Rule' + T_END_BOLD + T_BR + CL);

  // ("## Rule");
  mdText.push(mdLine.replace(CB_RULE, 'Rule'));
  mdText.push('');
  needHeader();
  console.error('>>>BODY 2 HDR>>>: ' + +curLine + ' -> ' + docLines[curLine]);

  if (docLines[curLine].startsWith(CB_META)) {
    curLine++;
    makeFixTitle();
    needHeader();
    console.error('>>>BODY 3 HDR>>>: ' + +curLine + ' -> ' + docLines[curLine]);
  }

  while (!isHeader() || docLines[curLine].startsWith(CB_ERROR) || docLines[curLine].startsWith(CB_WARNING)) {
    let s = translateLine(docLines[curLine]);

    mdText.push(s);
    s = highlightCode(s);
    s = escapeSym(s);
    console.log(s + CL);
    body += s;
    curLine++;
  }

  console.log(T_BR + CL);
  mdText.push('');

  return body;
}

function makeBad(): string {
  let badCode = '';

  needHeader();
  console.error('>>>makeBAD HDR>>>: ' + docLines[curLine]);
  if (!docLines[curLine].startsWith(CB_BAD)) {
    return '';
  }
  curLine++;
  // skip underline
  curLine++;

  console.log(T_HR + T_BOLD + 'TypeScript' + T_END_BOLD + T_BR + CL);

  mdText.push('## TypeScript');
  mdText.push('');

  while (curLine < docLines.length && !isHeader()) {
    let s = translateLine(docLines[curLine]);
    mdText.push(s);

    s = highlightCode(s);
    console.log(s + CL);

    badCode += s;
    curLine++;
  }

  skipEmptyLines();
  if (docLines[curLine++].startsWith(CODE_BLOCK)) {
    mdText.push('```');
    console.log(T_CODE + CL);
    while (curLine < docLines.length && !isHeader()) {
      mdText.push(docLines[curLine]);
      console.log(setNBSP(escapeSym(docLines[curLine])) + T_BR + CL);
      curLine++;
    }
    console.log(T_END_CODE + T_BR + CL);

    mdText.push('```');
  }
  mdText.push('');

  return badCode;
}

function makeOk(): string {
  let goodCode = '';

  needHeader();
  console.error('>>>makeOK HDR>>>: ' + docLines[curLine]);
  if (curLine >= docLines.length || !docLines[curLine].startsWith(CB_OK)) {
    return '';
  }
  curLine++;
  // skip underline
  curLine++;
  console.log(T_HR + T_BOLD + 'ArkTS' + T_END_BOLD + T_BR + CL);

  mdText.push('## ArkTS');
  mdText.push('');

  while (curLine < docLines.length && !isHeader()) {
    let s = translateLine(docLines[curLine]);

    mdText.push(s);

    s = highlightCode(s);
    console.log(s + CL);

    goodCode += s;
    curLine++;
  }

  skipEmptyLines();
  if (docLines[curLine++].startsWith(CODE_BLOCK)) {
    console.log(T_CODE + CL);

    mdText.push('```');

    while (curLine < docLines.length && !isHeader()) {
      mdText.push(docLines[curLine]);
      console.log(setNBSP(escapeSym(docLines[curLine])) + T_BR + CL);
      curLine++;
    }
    console.log(T_END_CODE + T_BR + CL);

    mdText.push('```');
  }

  mdText.push('');

  return goodCode;
}

function makeSee(): string {
  const RECIPE = 'Recipe ';
  console.error('>>> #' + recNum + ' PASSED: ' + docLines[curLine]);
  while (curLine < docLines.length && !docLines[curLine].startsWith('..')) {
    let s = translateLine(docLines[curLine]);

    if (s.split(CB_REF)[1]) {
      s = s.replace('*', '-');
      s = s.replace(CB_REF, RECIPE);
      s = s.replace('`R', '');
      const ruleNum = Number(s.replace('`', '').split(RECIPE)[1]);
      console.error('>>>RULE in SEE ' + ruleNum + ' ' + s.replace('`', '') + ' -> ' + ruleNames[ruleNum]);
      s = s.replace('`', ':');
      s += ' ' + ruleNames[ruleNum];
    }

    mdText.push(s);

    if (docLines[curLine].startsWith(CB_SEE)) {
      curLine++;
    }
    curLine++;
  }

  mdText.push('');

  return '';
}

/*
 *
 * Main routine
 *
 */
let commandLineArgs = process.argv.slice(2);
if (commandLineArgs.length === 0) {
  console.error('>>> Command line error: no arguments');
  process.exit(-1);
}
if (commandLineArgs[0] === '-md') {
  commandLineArgs = process.argv.slice(3);
}
const inFileName = commandLineArgs[0];
console.log(COPYRIGHT_HEADER);
console.log(CODE_PROLOGUE);
syncReadFile(inFileName);

for (recNum = 1; recNum < tegs.length; recNum++) {
  console.log('cookBookTag[ ' + recNum + ' ] = ' + STR_DLMTR + (tegs[recNum] ? tegs[recNum] : '') + STR_DLMTR + ';');
}

console.log('\nexport const cookBookRefToFixTitle: Map<number, string> = new Map([');
for (const num of fixTitles.keys()) {
  console.log(`  [${num}, '${fixTitles.get(num)}'],`);
}
console.log(']);');
