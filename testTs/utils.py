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
import datetime
import shutil
import difflib
from config import *
import subprocess
import json


# Executing terminal Commands
def command_os(order):
    cmd = order
    subprocess.run(cmd, shell=True)


# Creating a folder
def mk_dir(path):
    if not os.path.exists(path):
        os.makedirs(path)


# Delete folders (empty folders vs. non-empty folders)
def remove_dir(path):
    if os.path.exists(path):
        shutil.rmtree(path)


# Delete the file
def remove_file(path):
    if os.path.exists(path):
        os.remove(path)


# Read file contents (all)
def read_file(path):
    content = []
    with open(path, 'r') as f:
        content = f.readlines()

    return content


# Appending files (path: file path, content: write content)
def write_append(path, content):
    with open(path, 'a+') as f:
        f.write(content)


def move_file(srcfile, dstfile):
    subprocess.getstatusoutput("mv %s %s" % (srcfile, dstfile))


def excuting_npm_install(args):
    ark_frontend_tool = os.path.join(DEFAULT_ARK_FRONTEND_TOOL)
    if args.ark_frontend_tool:
        ark_frontend_tool = os.path.join(args.ark_frontend_tool)

    ts2abc_build_dir = os.path.join(os.path.dirname(
        os.path.realpath(ark_frontend_tool)), "..")
    if os.path.exists(os.path.join(ts2abc_build_dir, "package.json")):
        npm_install(ts2abc_build_dir)
    elif os.path.exists(os.path.join(ts2abc_build_dir, "..", "package.json")):
        npm_install(os.path.join(ts2abc_build_dir, ".."))


def npm_install(cwd):
    try:
        os.chdir(cwd)
        command_os('npm install')
        os.chdir(WORK_PATH)
    except Exception as e:
        print(e)
