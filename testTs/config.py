#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Copyright (c) 2022 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Description: Use ark to execute test 262 test suite
"""

import os
import json
TS_GIT_PATH = 'https://gitee.com/zhangrengao1/TypeScript.git'
TS_TAG = "v4.3.5"

LD_LIBRARY_PATH_LIST = [
    'out/hispark_taurus/clang_x64/ark/ark',
    'out/hispark_taurus/clang_x64/ark/ark_js_runtime',
    'out/hispark_taurus/clang_x64/thirdparty/icu',
    'prebuilts/clang/ohos/linux-x86_64/llvm/lib'
]

EXPECT_DIR = os.path.join("testTs", "expect")
OUT_DIR = os.path.join("out")
OUT_TEST_DIR = os.path.join("out", "testTs")
OUT_RESULT_FILE = os.path.join("out", "testTs", "result.txt")
TEST_DIR = os.path.join("testTs")
CASES_DIR = os.path.join(".", "testTs", "test")
TS_CASES_DIR = os.path.join(".", "testTs", "test", "testts")
JS_CASES_DIR = os.path.join(".", "testTs", "test", "testjs")
SKIP_FILE_PATH = os.path.join("testTs", "skip_tests.json")
IMPORT_FILE_PATH = os.path.join("testTs", "import_tests.json")
CUR_FILE_DIR = os.path.dirname(__file__)
CODE_ROOT = os.path.abspath(os.path.join(CUR_FILE_DIR, "../../.."))
ARK_TS2ABC_PATH = 'arkcompiler/ets_frontend/'
ARK_JS_VM = './out/hispark_taurus/clang_x64/ark/ark_js_runtime/ark_js_vm'
ARK_DIR = f"{CODE_ROOT}/out/hispark_taurus/clang_x64/ark/ark"
WORK_PATH = f'{CODE_ROOT}/{ARK_TS2ABC_PATH}'

DEFAULT_ARK_FRONTEND_TOOL = os.path.join(ARK_DIR, "build", "src", "index.js")
ARK_FRONTEND_TOOL_ES2ABC = os.path.join(ARK_DIR, "es2abc")

TEST_PATH = os.sep.join([".", "testTs", "test", "testts"])
OUT_TS_PATH = os.sep.join([".", "out", "testTs", "testts"])
OUT_JS_PATH = os.sep.join([".", "out", "testTs", "testjs"])
EXPECT_PATH = os.sep.join([".", "testTs", "expect"])
TS_EXT = ".ts"
JS_EXT = ".js"
TXT_EXT = ".txt"
ABC_EXT = ".abc"
EXPECT_EXT = "_expect.txt"
IMPORT_TEST = ""
with open(IMPORT_FILE_PATH, 'r') as f:
    content = f.read()
    IMPORT_TEST = json.loads(content)
