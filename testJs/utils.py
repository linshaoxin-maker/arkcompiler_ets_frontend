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
import shutil
from config import *
import subprocess


def command_os(order):
    subprocess.run(order)


def mk_dir(path):
    if not os.path.exists(path):
        os.makedirs(path)


def remove_dir(path):
    if os.path.exists(path):
        shutil.rmtree(path)


def remove_file(path):
    if os.path.exists(path):
        os.remove(path)


def read_file(path):
    util_read_content = []
    with open(path, "r") as utils_read:
        util_read_content = utils_read.readlines()

    return util_read_content


def excuting_npm_install(args):
    ark_frontend_tool = os.path.join(DEFAULT_ARK_FRONTEND_TOOL)
    if args.ark_frontend_tool:
        ark_frontend_tool = os.path.join(args.ark_frontend_tool)

    ts2abc_build_dir = os.path.join(os.path.dirname(os.path.realpath(ark_frontend_tool)), "..")
    if os.path.exists(os.path.join(ts2abc_build_dir, "package.json")):
        npm_install(ts2abc_build_dir)
    elif os.path.exists(os.path.join(ts2abc_build_dir, "..", "package.json")):
        npm_install(os.path.join(ts2abc_build_dir, ".."))


def npm_install(cwd):
    try:
        os.chdir(cwd)
        command_os(["npm", "install"])
        os.chdir(WORK_PATH)
    except BaseException as e:
        print(e)


def export_path():
    ld_library_path = ':'.join(LD_LIBRARY_PATH_LIST)
    try:
        os.chdir(CODE_ROOT)
        os.environ['LD_LIBRARY_PATH'] = ld_library_path
        os.chdir(WORK_PATH)
    except BaseException as e:
        print(e)
