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

generate abc with javascript
"""


import argparse
import collections
import os
import sys
import subprocess
import datetime

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from test262.utils import report_command
from test262.config import DEFAULT_TIMEOUT, DEFAULT_THREADS, ARK_FRONTEND_LIST, DEFAULT_ARK_ARCH, ARK_ARCH_LIST, \
    DEFAULT_OPT_LEVEL, DEFAULT_ES2ABC_THREAD_COUNT, DEFAULT_PRODUCT_NAME, DEFAULT_HOST_PATH, DEFAULT_HOST_TYPE, \
    DEFAULT_ARK_TOOL, DEFAULT_ARK_AOT_TOOL, DEFAULT_LIBS_DIR, DEFAULT_MERGE_ABC_BINARY, DEFAULT_MERGE_ABC_MODE, \
    RK3568_PRODUCT_NAME, ARGS_PREFIX, ARK_DIR_SUFFIX, ICUI_DIR_SUFFIX, ARK_JS_RUNTIME_DIR_SUFFIX, ZLIB_DIR_SUFFIX, \
    LLVM_DIR
from config import FUZZY_DEFAULT_ARK_FRONTEND_BINARY, DEFAULT_OPEN_FUZZILLI_MODE


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
                             'all: Contains all use cases for es5_tests and es2015_tests and '
                             'es2021_tests and intl_tests' +
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


def run_check(runnable, env=None):
    report_command('Test command:', runnable, env=env)

    if env is not None:
        full_env = dict(os.environ)
        full_env.update(env)
        env = full_env

    proc = subprocess.Popen(runnable, env=env)
    stdout, stderr = proc.communicate()
    if stdout:
        print(f'Out message:{stdout.decode()}')
    if stderr:
        print(f'Error message:{stderr.decode()}')
    return proc.returncode


def get_host_path_type(args):
    host_path = DEFAULT_HOST_PATH
    host_type = DEFAULT_HOST_TYPE
    if args.engine:
        host_path = args.engine
        host_type = os.path.split(args.engine.strip())[1]
    return host_path, host_type


def get_host_args(args, host_type):
    host_args = ""
    ark_tool = DEFAULT_ARK_TOOL
    ark_aot_tool = DEFAULT_ARK_AOT_TOOL
    libs_dir = DEFAULT_LIBS_DIR
    ark_frontend = ARK_FRONTEND_LIST[1]
    ark_frontend_binary = FUZZY_DEFAULT_ARK_FRONTEND_BINARY
    ark_arch = DEFAULT_ARK_ARCH
    opt_level = DEFAULT_OPT_LEVEL
    es2abc_thread_count = DEFAULT_ES2ABC_THREAD_COUNT
    merge_abc_binary = DEFAULT_MERGE_ABC_BINARY
    merge_abc_mode = DEFAULT_MERGE_ABC_MODE
    product_name = RK3568_PRODUCT_NAME

    if args.product_name:
        product_name = args.product_name
        ark_dir = f"{ARGS_PREFIX}{product_name}{ARK_DIR_SUFFIX}"
        icui_dir = f"{ARGS_PREFIX}{product_name}{ICUI_DIR_SUFFIX}"
        ark_js_runtime_dir = f"{ARGS_PREFIX}{product_name}{ARK_JS_RUNTIME_DIR_SUFFIX}"
        zlib_dir = f"{ARGS_PREFIX}{product_name}{ZLIB_DIR_SUFFIX}"

        ark_tool = os.path.join(ark_js_runtime_dir, "ark_js_vm")
        libs_dir = f"{icui_dir}:{LLVM_DIR}:{ark_js_runtime_dir}:{zlib_dir}"
        ark_aot_tool = os.path.join(ark_js_runtime_dir, "ark_aot_compiler")
        merge_abc_binary = os.path.join(ark_dir, "merge_abc")

    if args.hostArgs:
        host_args = args.hostArgs

    if args.ark_tool:
        ark_tool = args.ark_tool

    if args.ark_aot_tool:
        ark_aot_tool = args.ark_aot_tool

    if args.libs_dir:
        libs_dir = args.libs_dir

    if args.ark_frontend:
        ark_frontend = args.ark_frontend

    if args.ark_frontend_binary:
        ark_frontend_binary = args.ark_frontend_binary

    if args.opt_level:
        opt_level = args.opt_level

    if args.es2abc_thread_count:
        es2abc_thread_count = args.es2abc_thread_count

    if args.merge_abc_binary:
        merge_abc_binary = args.merge_abc_binary

    if args.merge_abc_mode:
        merge_abc_mode = args.merge_abc_mode

    if host_type == DEFAULT_HOST_TYPE:
        host_args = f"-B test262/run_sunspider.py "
        host_args += f"--ark-tool={ark_tool} "
        if args.ark_aot:
            host_args += f"--ark-aot "
        if args.run_pgo:
            host_args += f"--run-pgo "
        host_args += f"--ark-aot-tool={ark_aot_tool} "
        host_args += f"--libs-dir={libs_dir} "
        host_args += f"--ark-frontend={ark_frontend} "
        host_args += f"--ark-frontend-binary={ark_frontend_binary} "
        host_args += f"--opt-level={opt_level} "
        host_args += f"--es2abc-thread-count={es2abc_thread_count} "
        host_args += f"--merge-abc-binary={merge_abc_binary} "
        host_args += f"--merge-abc-mode={merge_abc_mode} "
        host_args += f"--product-name={product_name} "

    if args.ark_arch != ark_arch:
        host_args += f"--ark-arch={args.ark_arch} "
        host_args += f"--ark-arch-root={args.ark_arch_root} "

    return host_args


def run_fuzzy_test(args):
    host_path, host_type = get_host_path_type(args)
    host_args = get_host_args(args, host_type)
    fuzzy_js_file = args.file
    host_args = host_args.replace(fuzzy_js_file, '')
    test_cmd = ['python3'] + host_args.strip().split(' ')
    test_cmd.append(f"--js-file={fuzzy_js_file}")
    return run_check(test_cmd)


Check = collections.namedtuple('Check', ['enabled', 'runner', 'arg'])


def main(args):
    if args.open_fuzzy_mode:
        print("\nWait a moment..........\n")
        start_time = datetime.datetime.now()
        check = Check(True, run_fuzzy_test, args)
        ret = check.runner(check.arg)
        if ret:
            sys.exit(ret)
        end_time = datetime.datetime.now()
        print(f"used time is: {str(end_time - start_time)}")
    else:
        print('You should execute the script with open fuzzilli mode by `--open-fuzzy-mode`!')
        sys.exit(1)


if __name__ == "__main__":
    sys.exit(main(parse_args()))
