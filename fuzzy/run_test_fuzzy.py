#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Copyright (c) 2021 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Description: Use ark to execute fuzzilli test suite
"""

import argparse
import datetime
import collections
import json
import os
import shutil
import sys
import subprocess
from multiprocessing import Pool
import platform
import tarfile

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from test262.utils import *
from test262.config import *

SWIFT_DOWNLOAD_URL = 'https://download.swift.org/swift-5.9.2-release/ubuntu1804/swift-5.9.2-RELEASE/swift-5.9.2-RELEASE-ubuntu18.04.tar.gz'


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--dir', metavar='DIR',
                        help='Directory to test ')
    parser.add_argument('--file', metavar='FILE',
                        required=True,
                        help='File to test')
    parser.add_argument('--mode',
                        nargs='?', choices=[1, 2, 3], type=int,
                        help='selection information as: ' +
                             '1: only default \n ' +
                             '2: only strict mode \n' +
                             '3: both default and strict mode\n')
    parser.add_argument('--es51', action='store_true',
                        help='Run test262 ES5.1 version')
    parser.add_argument('--es2021', default=False, const='all',
                        nargs='?', choices=['all', 'only'],
                        help='Run test262 - ES2021. ' +
                             'all: Contains all use cases for es5_tests and es2015_tests and es2021_tests and intl_tests' +
                             'only: Only include use cases for ES2021')
    parser.add_argument('--es2022', default=False, const='all',
                        nargs='?', choices=['all', 'only'],
                        help='Run test262 - ES2022. ' +
                             'all: Contains all use cases for es5_tests and es2015_tests and es2021_tests' +
                             'and es2022_tests and intl_tests' +
                             'only: Only include use cases for ES2022')
    parser.add_argument('--es2023', default=False, const='all',
                        nargs='?', choices=['all', 'only'],
                        help='Run test262 - ES2023. ' +
                             'all: Contains all use cases for es5_tests and es2015_tests and es2021_tests' +
                             'and es2022_tests and es2023_tests and intl_tests' +
                             'only: Only include use cases for ES2023')
    parser.add_argument('--intl', default=False, const='intl',
                        nargs='?', choices=['intl'],
                        help='Run test262 - Intltest. ' +
                             'intl: Only include use cases for intlcsae')
    parser.add_argument('--es2015', default=False, const='es2015',
                        nargs='?', choices=['es2015'],
                        help='Run test262 - es2015. ' +
                             'es2015: Only include use cases for es2015')
    parser.add_argument('--ci-build', action='store_true',
                        help='Run test262 ES2015 filter cases for build version')
    parser.add_argument('--esnext', action='store_true',
                        help='Run test262 - ES.next.')
    parser.add_argument('--engine', metavar='FILE',
                        help='Other engine binarys to run tests(as:d8,qjs...)')
    parser.add_argument('--babel', action='store_true',
                        help='Whether to use Babel conversion')
    parser.add_argument('--timeout', default=DEFAULT_TIMEOUT, type=int,
                        help='Set a custom test timeout in milliseconds !!!\n')
    parser.add_argument('--threads', default=DEFAULT_THREADS, type=int,
                        help="Run this many tests in parallel.")
    parser.add_argument('--hostArgs',
                        help="command-line arguments to pass to eshost host\n")
    parser.add_argument('--ark-tool',
                        help="ark's binary tool")
    parser.add_argument('--ark-aot', action='store_true',
                        help="Run test262 with aot")
    parser.add_argument('--ark-aot-tool',
                        help="ark's aot tool")
    parser.add_argument("--libs-dir",
                        help="The path collection of dependent so has been divided by':'")
    parser.add_argument('--ark-frontend',
                        nargs='?', choices=ARK_FRONTEND_LIST, type=str,
                        help="Choose one of them")
    parser.add_argument('--ark-frontend-binary',
                        help="ark frontend conversion binary tool")
    parser.add_argument('--ark-arch',
                        default=DEFAULT_ARK_ARCH,
                        nargs='?', choices=ARK_ARCH_LIST, type=str,
                        help="Choose one of them")
    parser.add_argument('--ark-arch-root',
                        default=DEFAULT_ARK_ARCH,
                        help="the root path for qemu-aarch64 or qemu-arm")
    parser.add_argument('--opt-level',
                        default=DEFAULT_OPT_LEVEL,
                        help="the opt level for es2abc")
    parser.add_argument('--es2abc-thread-count',
                        default=DEFAULT_ES2ABC_THREAD_COUNT,
                        help="the thread count for es2abc")
    parser.add_argument('--merge-abc-binary',
                        help="frontend merge abc binary tool")
    parser.add_argument('--merge-abc-mode',
                        help="run test for merge abc mode")
    parser.add_argument('--product-name',
                        default=DEFAULT_PRODUCT_NAME,
                        help="ark's product name")
    parser.add_argument('--run-pgo', action='store_true',
                        help="Run test262 with aot pgo")
    parser.add_argument('--open-fuzzy-mode', action='store_true', default=DEFAULT_OPEN_FUZZILLI_MODE,
                        help='run test with open fuzzilli mode')
    return parser.parse_args()


def exec_command_communicate(cmd, shell_flag=False, timeout=600):
    """
    Execute the cmd command to return the terminal output value
    """
    proc = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        encoding="utf-8",
        universal_newlines=True,
        shell=shell_flag
    )
    out, error = proc.communicate(timeout=timeout)
    ret_code = proc.returncode
    return ret_code, out, error


class TestPrepare:

    def prepare_fuzzili_code(self):
        """
        1. Download the fuzzilli code
        2. Switch branches and specify commit
        3. Patch code
        """
        if not os.path.isdir(os.path.join(FUZZY_DIR, '.git')):
            git_clone(FUZZY_GIT_URL, FUZZY_DIR)
            git_checkout(FUZZY_GIT_HASH, FUZZY_DIR)
            # git_apply('../fuzzy.patch', FUZZY_DIR)  # TODO: 需要根据实际修改

    def get_sysctl_config(self):
        cmd = ['sysctl', '-a', '|grep', 'kernel.core_pattern']
        ret, output, error = exec_command_communicate(cmd)
        if not ret:
            lines = output.split('\n')
            config = {}
            for line in lines:
                if line.strip() != '':
                    key, value = line.split(' = ')
                    config[key.strip()] = value.strip()
            return config

    def check_bin_false(self):
        config = self.get_sysctl_config()
        if config:
            kcp_val = config.get('kernel.core_pattern')
            return kcp_val == '|/bin/false'

    def kernel_core_pattern(self):
        """
        Set temporary variables
        """
        is_bin_false = self.check_bin_false()
        if not is_bin_false:
            cmd = ['sysctl', '-w', 'kernel.core_pattern=|/bin/false']
            ret, _, error = exec_command_communicate(cmd)
            if ret:
                raise SystemExit(f'Set kernel.core_pattern Error: {error}')
            return 0

    def do_fuzzy_compiler(self, profile='es2abc', storage_path=FUZZILLI_OUTPUT_DIR_NAME):
        """
        Run the compilation command to generate the compiled result
        """
        # mkdir for output
        _output_dir = os.path.join(FUZZY_DIR, storage_path)
        if not os.path.exists(_output_dir):
            mkdir(_output_dir)
        try:
            swift_tool = check_swift()
        except SystemExit:
            swift_tool = prepare_swift()

        # change to child dir
        os.chdir(FUZZY_DIR)
        # where es2abc path
        ejs_shell = ARK_FRONTEND_BINARY_LIST[1]
        cmd = [swift_tool, 'run', '-c', 'release', '-Xlinker="-lrt"', 'FuzzilliCli', f'--profile={profile}',
               f'--storagePath={_output_dir}', ejs_shell]
        ret, _, error = exec_command_communicate(cmd)
        if ret:
            raise SystemExit(f'fuzzy compiler error: {error}')
        return 0

    def fuzzy_es2abc_action(self):
        """
        1. download fuzzilli code and apply patch
        2. check or set env
        3. compiler and output
        """
        self.prepare_fuzzili_code()
        self.kernel_core_pattern()
        self.do_fuzzy_compiler()

    def run(self):
        self.fuzzy_es2abc_action()


def prepare_swift():
    print('Start downloading swift……')
    _cmd = ['wget', SWIFT_DOWNLOAD_URL]
    ret, output, error = exec_command_communicate(_cmd)
    if ret:
        raise SystemExit(f'Download swift error: {error}')
    print('Downloading swift finished.')
    # 打开tar包
    with tarfile.open('swift-5.9.2-RELEASE-ubuntu18.04.tar.gz', 'r:gz') as tar:
        # 解压所有文件
        tar.extractall()

    current_dir = os.getcwd()
    swift_path = os.path.join(current_dir, 'swift-5.9.2-RELEASE-ubuntu18.04', 'usr', 'bin', 'swift')
    try:
        check_swift(swift_path)
    except SystemExit as e:
        raise SystemExit(str(e))
    return swift_path


def check_swift(swift_path='swift'):
    version_cmd = [swift_path, '--version']
    ret, output, error = exec_command_communicate(version_cmd)
    if ret:
        raise SystemExit('Swift not found.')
    print(output)
    return swift_path


def main():
    test_prepare = TestPrepare()
    test_prepare.run()


if __name__ == "__main__":
    sys.exit(main())
