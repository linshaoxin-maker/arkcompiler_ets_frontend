/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

"use strict";
const path = require("path");
const fs = require("fs");
const spawn = require('child_process').spawn;

let isWin = !1;
let isMac = !1;

const arkDir = path.resolve(__dirname);

if (fs.existsSync(path.join(arkDir, 'build-win'))) {
    isWin = !0;
} else if (fs.existsSync(path.join(arkDir, 'build-mac'))) {
    isMac = !0;
} else if (!fs.existsSync(path.join(arkDir, 'build'))) {
    throw Error('find build fail').message;
}

let frontendCompiler;
if (isWin) {
    frontendCompiler = path.join(arkDir, 'build-win', 'bin', 'es2abc.exe');
} else if (isMac) {
    frontendCompiler = path.join(arkDir, 'build-mac', 'bin', 'es2abc');
} else {
    frontendCompiler = path.join(arkDir, 'build', 'bin', 'es2abc');
}

function callEs2abc(args) {
    let proc = spawn(`${frontendCompiler}`, args);

    proc.stderr.on('data', (data) => {
        throw Error(`${data}`).message;
    });

    proc.stdout.on('data', (data) => {
        process.stdout.write(`${data}`);
    });
}

let args = process.argv.splice(2);
if (args.length == 1 && args[0] == "--bc-version") { // keep bc-version to be compatible with old IDE versions
    callEs2abc(args);
    return;
}

if (args[0] == "--target-api-version") {
    args.push("--target-ark-version");
    callEs2abc(args);
}
