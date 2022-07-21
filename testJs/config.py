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

LD_LIBRARY_PATH_LIST = [
    'out/hispark_taurus/clang_x64/ark/ark',
    'out/hispark_taurus/clang_x64/ark/ark_js_runtime',
    'out/hispark_taurus/clang_x64/thirdparty/icu',
    'prebuilts/clang/ohos/linux-x86_64/llvm/lib'
]

OUT_DIR = os.path.join("out")
OUT_TEST_DIR = os.path.join("out", "testJs")
OUT_RESULT_FILE = os.path.join("out", "testJs", "result.txt")
TEST_DIR = os.path.join("testJs")
TS_CASES_DIR = os.path.join(".", "testJs", "test")
CUR_FILE_DIR = os.path.dirname(__file__)
CODE_ROOT = os.path.abspath(os.path.join(CUR_FILE_DIR, "../../.."))
ARK_TS2ABC_PATH = 'arkcompiler/ets_frontend/'
ARK_JS_VM = './out/hispark_taurus/clang_x64/ark/ark_js_runtime/ark_js_vm'
ARK_DIR = f"{CODE_ROOT}/out/hispark_taurus/clang_x64/ark/ark"
WORK_PATH = f'{CODE_ROOT}/{ARK_TS2ABC_PATH}'
EXPECT_PATH = 'expect_output.txt'
DEFAULT_ARK_FRONTEND_TOOL = os.path.join(ARK_DIR, "build", "src", "index.js")

TEST_PATH = os.sep.join([".", "testJs", "test"])
OUT_PATH = os.sep.join([".", "out", "testJs"])
JS_EXT = ".js"
TXT_EXT = ".txt"
ABC_EXT = ".abc"
