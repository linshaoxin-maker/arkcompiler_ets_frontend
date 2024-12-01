#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Copyright (c) 2023 Huawei Device Co., Ltd.
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
import os
import sys
import subprocess
import tarfile

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from test262.utils import git_clone, git_checkout, git_apply, mkdir
from test262.config import CODE_ROOT
from config import FUZZY_DIR, FUZZY_GIT_URL, FUZZY_GIT_HASH, FUZZY_DIR_NAME, FUZZILLI_PATCH_DIR_NAME, \
    FUZZILLI_OUTPUT_DIR_NAME, SWIFT_DOWNLOAD_URL


def run_command(cmd, shell_flag=False, timeout=60000):
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
    @staticmethod
    def prepare_fuzzilli_code():
        if not os.path.isdir(os.path.join(FUZZY_DIR, '.git')):
            git_clone(FUZZY_GIT_URL, FUZZY_DIR)
            git_checkout(FUZZY_GIT_HASH, FUZZY_DIR)
            current_dir = os.getcwd()
            fuzzilli_patch_path = os.path.join(current_dir, FUZZY_DIR_NAME, FUZZILLI_PATCH_DIR_NAME, 'fuzzilli.patch')
            build_gn_patch_path = os.path.join(current_dir, FUZZY_DIR_NAME, FUZZILLI_PATCH_DIR_NAME, 'build_gn.patch')
            if os.path.exists(fuzzilli_patch_path) and os.path.isfile(fuzzilli_patch_path):
                git_apply(fuzzilli_patch_path, FUZZY_DIR)

            if os.path.exists(build_gn_patch_path) and os.path.isfile(build_gn_patch_path):
                git_apply(build_gn_patch_path, current_dir)

    @staticmethod
    def get_sysctl_config():
        cmd = ['sysctl', '-a', '|grep', 'kernel.core_pattern']
        ret, output, error = run_command(cmd)
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

    def init_kernel_core_pattern(self):
        is_bin_false = self.check_bin_false()
        if not is_bin_false:
            cmd = ['sysctl', '-w', 'kernel.core_pattern=|/bin/false']
            ret, _, error = run_command(cmd)
            if ret:
                raise Exception(f'Set kernel.core_pattern Error: {error}')
            return 0

    @staticmethod
    def fuzzy_compiler(profile='es2abc', storage_path=FUZZILLI_OUTPUT_DIR_NAME):
        _output_dir = os.path.join(FUZZY_DIR, storage_path)
        if not os.path.exists(_output_dir):
            mkdir(_output_dir)
        try:
            swift_tool = check_swift()
        except Exception:
            swift_tool = prepare_swift()

        # change to child dir
        os.chdir(FUZZY_DIR)
        # where es2abc path
        _DEFAULT_ARK_DIR = f"{CODE_ROOT}/out/rk3568/clang_x64/arkcompiler/ets_frontend"
        ejs_shell = os.path.join(_DEFAULT_ARK_DIR, "arkfuzzer")
        cmd = [swift_tool, 'run', '-c', 'release', 'FuzzilliCli', f'--profile={profile}',
               f'--storagePath={_output_dir}', ejs_shell]
        ret, _, error = run_command(cmd)
        if ret:
            raise Exception(f'fuzzy compiler error: {error}')
        return 0

    @staticmethod
    def arkfuzzer_compiler():
        _compiler_dir = f"{CODE_ROOT}"
        os.chdir(_compiler_dir)
        cmd = ['./build.sh', '--product-name', 'rk3568', '--build-target', 'ark_js_host_linux_tools_packages',
               '--build-target', 'ets_frontend_build', f'--no-prebuilt-sdk']
        print('compilering arkfuzzer. Please be patient and wait')
        ret, _, error = run_command(cmd)
        if ret:
            raise Exception(f'Fuzzilli compiler failed:{error}')
        print('arkfuzzer compiler done!')
        # Return to the original directory
        _original_directory = f"{CODE_ROOT}/arkcompiler/ets_frontend"
        os.chdir(_original_directory)
        return 0

    def prepare_fuzzy_test(self):
        self.prepare_fuzzilli_code()
        self.init_kernel_core_pattern()
        # compile arkfuzzer
        self.arkfuzzer_compiler()
        self.fuzzy_compiler()

    def run(self):
        self.prepare_fuzzy_test()


def prepare_swift():
    print('Start downloading swift……')
    swift_path = os.path.join(FUZZY_DIR_NAME, 'swift.tar.gz')
    _cmd = ['wget', '-O', swift_path, SWIFT_DOWNLOAD_URL]
    ret, output, error = run_command(_cmd)
    if ret:
        raise Exception(f'Download swift error: {error}')
    print('Downloading swift finished.')
    with tarfile.open(swift_path, 'r:gz') as tar:
        tar.extractall(FUZZY_DIR_NAME)

    current_dir = os.getcwd()
    swift_path = os.path.join(current_dir, FUZZY_DIR_NAME, 'swift-5.9.2-RELEASE-ubuntu18.04', 'usr', 'bin', 'swift')
    try:
        check_swift(swift_path)
    except Exception as e:
        raise Exception(str(e))
    return swift_path


def check_swift(swift_path='swift'):
    version_cmd = [swift_path, '--version']
    try:
        ret, output, error = run_command(version_cmd)
    except FileNotFoundError:
        raise Exception('Swift not found.')
    if ret:
        raise Exception('Swift not found.')
    print(output)
    return swift_path


def main():
    test_prepare = TestPrepare()
    test_prepare.run()


if __name__ == "__main__":
    sys.exit(main())
