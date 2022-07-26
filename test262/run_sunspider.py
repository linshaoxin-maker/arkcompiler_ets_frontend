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

Description: Use ark to execute js files
"""

import argparse
import os
import platform
import sys
import signal
import re
import fileinput
import subprocess
from utils import *
from config import *


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--ark-tool',
                        default=DEFAULT_ARK_TOOL,
                        required=False,
                        help="ark's binary tool")
    parser.add_argument('--ark-aot', action='store_true',
                        required=False,
                        help="Run test262 with aot")
    parser.add_argument('--ark-aot-tool',
                        default=DEFAULT_ARK_AOT_TOOL,
                        required=False,
                        help="ark's aot tool")
    parser.add_argument("--libs-dir",
                        default=DEFAULT_LIBS_DIR,
                        required=False,
                        help="The path collection of dependent so has been divided by':'")
    parser.add_argument("--js-file",
                        required=True,
                        help="js file")
    parser.add_argument('--ark-frontend',
                        default=DEFAULT_ARK_FRONTEND,
                        required=False,
                        nargs='?', choices=ARK_FRONTEND_LIST, type=str,
                        help="Choose one of them")
    parser.add_argument('--ark-frontend-binary',
                        default=DEFAULT_ARK_FRONTEND_BINARY,
                        required=False,
                        help="ark frontend conversion binary tool")
    parser.add_argument('--module-list',
                        required=True,
                        help="module file list")
    parser.add_argument('--ark-arch',
                        default=DEFAULT_ARK_ARCH,
                        required=False,
                        nargs='?', choices=ARK_ARCH_LIST, type=str,
                        help="Choose one of them")
    parser.add_argument('--ark-arch-root',
                        default=DEFAULT_ARK_ARCH,
                        required=False,
                        help="the root path for qemu-aarch64 or qemu-arm")
    parser.add_argument('--opt-level',
                        default=DEFAULT_OPT_LEVEL,
                        required=False,
                        help="the opt level for es2abc")
    parser.add_argument('--es2abc-thread-count',
                        default=DEFAULT_ES2ABC_THREAD_COUNT,
                        required=False,
                        help="the thread count for es2abc")
    arguments = parser.parse_args()
    return arguments


ICU_PATH = f"--icu-data-path={CODE_ROOT}/third_party/icu/ohos_icu4j/data"
if platform.system() == "Windows" :
    ICU_PATH = ICU_PATH.replace("/","\\")
ARK_TOOL = DEFAULT_ARK_TOOL
LIBS_DIR = DEFAULT_LIBS_DIR
ARK_AOT_TOOL = DEFAULT_ARK_AOT_TOOL
ARK_FRONTEND = DEFAULT_ARK_FRONTEND
ARK_FRONTEND_BINARY = DEFAULT_ARK_FRONTEND_BINARY
ARK_ARCH = DEFAULT_ARK_ARCH


def output(retcode, msg):
    if retcode == 0:
        if msg != '':
            print(str(msg))
    elif retcode == -6:
        sys.stderr.write("Aborted (core dumped)")
    elif retcode == -4:
        sys.stderr.write("Aborted (core dumped)")
    elif retcode == -11:
        sys.stderr.write("Segmentation fault (core dumped)")
    elif msg != '':
        sys.stderr.write(str(msg))
    else:
        sys.stderr.write("Unknown Error: " + str(retcode))


def exec_command(cmd_args, timeout=DEFAULT_TIMEOUT):
    proc = subprocess.Popen(cmd_args,
                            stderr=subprocess.PIPE,
                            stdout=subprocess.PIPE,
                            close_fds=True,
                            start_new_session=True)
    cmd_string = " ".join(cmd_args)
    code_format = 'utf-8'
    if platform.system() == "Windows":
        code_format = 'gbk'

    try:
        (msg, errs) = proc.communicate(timeout=timeout)
        ret_code = proc.poll()

        if errs.decode(code_format, 'ignore') != '':
            output(1, errs.decode(code_format, 'ignore'))
            return 1

        if ret_code and ret_code != 1:
            code = ret_code
            msg = f"Command {cmd_string}: \n"
            msg += f"error: {str(errs.decode(code_format,'ignore'))}"
        else:
            code = 0
            msg = str(msg.decode(code_format, 'ignore'))

    except subprocess.TimeoutExpired:
        proc.kill()
        proc.terminate()
        os.kill(proc.pid, signal.SIGTERM)
        code = 1
        msg = f"Timeout:'{cmd_string}' timed out after' {str(timeout)} seconds"
    except Exception as err:
        code = 1
        msg = f"{cmd_string}: unknown error: {str(err)}"
    output(code, msg)
    return code

def print_command(cmd_args):
    sys.stderr.write("\n")
    for arg in cmd_args:
        sys.stderr.write("%s%s"%(arg, " "))
    sys.stderr.write("\n")

def run_command(cmd_args):
    timeout = DEFAULT_TIMEOUT / 1000
    cmd = f"timeout {timeout} "
    for arg in cmd_args:
        cmd += f"{arg} "
    return subprocess.run(cmd)

class ArkProgram():
    def __init__(self, args):
        self.args = args
        self.ark_tool = ARK_TOOL
        self.ark_aot = False
        self.ark_aot_tool = ARK_AOT_TOOL
        self.libs_dir = LIBS_DIR
        self.ark_frontend = ARK_FRONTEND
        self.ark_frontend_binary = ARK_FRONTEND_BINARY
        self.module_list = []
        self.js_file = ""
        self.module = False
        self.abc_file = ""
        self.arch = ARK_ARCH
        self.arch_root = ""
        self.opt_level = DEFAULT_OPT_LEVEL
        self.es2abc_thread_count = DEFAULT_ES2ABC_THREAD_COUNT

    def proce_parameters(self):
        if self.args.ark_tool:
            self.ark_tool = self.args.ark_tool

        if self.args.ark_aot:
            self.ark_aot = self.args.ark_aot

        if self.args.ark_aot_tool:
            self.ark_aot_tool = self.args.ark_aot_tool

        if self.args.ark_frontend_binary:
            self.ark_frontend_binary = self.args.ark_frontend_binary

        if self.args.libs_dir:
            self.libs_dir = self.args.libs_dir

        if self.args.ark_frontend:
            self.ark_frontend = self.args.ark_frontend

        if self.args.opt_level:
            self.opt_level = self.args.opt_level

        if self.args.es2abc_thread_count:
            self.es2abc_thread_count = self.args.es2abc_thread_count

        self.module_list = self.args.module_list.splitlines()

        self.js_file = self.args.js_file

        self.arch = self.args.ark_arch

        self.arch_root = self.args.ark_arch_root

    def gen_abc(self):
        js_file = self.js_file
        file_name_pre = os.path.splitext(js_file)[0]
        file_name = os.path.basename(js_file)
        out_file = f"{file_name_pre}.abc"
        self.abc_file = out_file
        mod_opt_index = 0
        cmd_args = []
        frontend_tool = self.ark_frontend_binary
        if self.ark_frontend == ARK_FRONTEND_LIST[0]:
            mod_opt_index = 3
            cmd_args = ['node', '--expose-gc', frontend_tool,
                        js_file, '-o', out_file]
            if file_name in self.module_list:
                cmd_args.insert(mod_opt_index, "-m")
                self.module = True
        elif self.ark_frontend == ARK_FRONTEND_LIST[1]:
            mod_opt_index = 1
            cmd_args = [frontend_tool, '--opt-level=' + str(self.opt_level),
                        '--thread=' + str(self.es2abc_thread_count), '--output', out_file, js_file]
            if file_name in self.module_list:
                cmd_args.insert(mod_opt_index, "--module")
                self.module = True
        if self.ark_aot:
            subprocess.run(f'''sed -i 's/;$262.destroy();/\/\/;$262.destroy();/g' {js_file}''')
            if self.module:
                js_dir = os.path.dirname(js_file)
                for line in fileinput.input(js_file):
                    import_line = re.findall(r"^(?:ex|im)port.*from.*\.js", line)
                    if len(import_line):
                        import_file = re.findall(r"['\"].*\.js", import_line[0])
                        if len(import_file):
                            abc_file = import_file[0][1:].replace(".js", ".abc")
                            if self.abc_file.find(abc_file) < 0:
                                self.abc_file += f':{js_dir}/{abc_file}'
        retcode = exec_command(cmd_args)
        return retcode

    def compile_aot(self):
        os.environ["LD_LIBRARY_PATH"] = self.libs_dir
        file_name_pre = os.path.splitext(self.js_file)[0]
        abc_file = f'{file_name_pre}.abc'
        cmd_args = []
        if self.arch == ARK_ARCH_LIST[1]:
            cmd_args = [self.ark_aot_tool, ICU_PATH,
                        f'--target-triple=aarch64-unknown-linux-gnu',
                        f'--aot-file={file_name_pre}.m',
                        f'--snapshot-output-file={file_name_pre}.snapshot',
                        self.abc_file,
                        f'> {file_name_pre}.log 2>&1']
        elif self.arch == ARK_ARCH_LIST[2]:
            cmd_args = [self.ark_aot_tool, ICU_PATH,
                        f'--target-triple=arm-unknown-linux-gnu',
                        f'--aot-file={file_name_pre}.m',
                        f'--snapshot-output-file={file_name_pre}.snapshot',
                        self.abc_file,
                        f'> {file_name_pre}.log 2>&1']
        elif self.arch == ARK_ARCH_LIST[0]:
            cmd_args = [self.ark_aot_tool, ICU_PATH,
                        f'--aot-file={file_name_pre}.m',
                        f'--snapshot-output-file={file_name_pre}.snapshot',
                        self.abc_file,
                        f'> {file_name_pre}.log 2>&1']
        retcode = run_command(cmd_args)
        if retcode:
            print_command(cmd_args)

    def execute_aot(self):
        os.environ["LD_LIBRARY_PATH"] = self.libs_dir
        file_name_pre = os.path.splitext(self.js_file)[0]
        cmd_args = []
        if self.arch == ARK_ARCH_LIST[1]:
            qemu_tool = "qemu-aarch64"
            qemu_arg1 = "-L"
            qemu_arg2 = self.arch_root
            cmd_args = [qemu_tool, qemu_arg1, qemu_arg2, self.ark_tool,
                        ICU_PATH,
                        '--asm-interpreter=1',
                        f'--aot-file={file_name_pre}.m',
                        f'--snapshot-output-file={file_name_pre}.snapshot',
                        f'{file_name_pre}.abc']
        elif self.arch == ARK_ARCH_LIST[2]:
            qemu_tool = "qemu-arm"
            qemu_arg1 = "-L"
            qemu_arg2 =  self.arch_root
            cmd_args = [qemu_tool, qemu_arg1, qemu_arg2, self.ark_tool,
                        ICU_PATH,
                        '--asm-interpreter=1',
                        f'--aot-file={file_name_pre}.m',
                        f'--snapshot-output-file={file_name_pre}.snapshot',
                        f'{file_name_pre}.abc']
        elif self.arch == ARK_ARCH_LIST[0]:
            cmd_args = [self.ark_tool, ICU_PATH,
                        '--asm-interpreter=1',
                        f'--aot-file={file_name_pre}.m',
                        f'--snapshot-output-file={file_name_pre}.snapshot',
                        f'{file_name_pre}.abc']

        retcode = run_command(cmd_args)
        if retcode:
            subprocess.run(f'cp {self.js_file} {BASE_OUT_DIR}/../')
            print_command(cmd_args)
        return retcode

    def execute(self):
        if platform.system() == "Windows" :
            #add env path for cmd/powershell execute
            libs_dir = self.libs_dir.replace(":", ";")
            libs_dir = libs_dir.replace("/", "\\")
            os.environ["PATH"] = libs_dir + ";" + os.environ["PATH"]
        elif platform.system() == "Linux" :
            os.environ["LD_LIBRARY_PATH"] = self.libs_dir
        else :
            sys.exit(f" test262 on {platform.system()} not supported");
        file_name_pre = os.path.splitext(self.js_file)[0]
        cmd_args = []
        if self.arch == ARK_ARCH_LIST[1]:
            qemu_tool = "qemu-aarch64"
            qemu_arg1 = "-L"
            qemu_arg2 = self.arch_root
            cmd_args = [qemu_tool, qemu_arg1, qemu_arg2, self.ark_tool,
                        ICU_PATH,
                        f'{file_name_pre}.abc']
        elif self.arch == ARK_ARCH_LIST[2]:
            qemu_tool = "qemu-arm"
            qemu_arg1 = "-L"
            qemu_arg2 =  self.arch_root
            cmd_args = [qemu_tool, qemu_arg1, qemu_arg2, self.ark_tool,
                        ICU_PATH,
                        f'{file_name_pre}.abc']
        elif self.arch == ARK_ARCH_LIST[0]:
            cmd_args = [self.ark_tool, ICU_PATH,
                        f'{file_name_pre}.abc']

        retcode = exec_command(cmd_args)
        if retcode:
            print_command(cmd_args)
        return retcode

    def is_legal_frontend(self):
        if self.ark_frontend not in ARK_FRONTEND_LIST:
            sys.stderr.write("Wrong ark front-end option")
            return False
        return True

    def execute_ark(self):
        self.proce_parameters()
        if not self.is_legal_frontend():
            return
        if self.gen_abc():
            return
        if self.ark_aot:
            self.compile_aot()
            self.execute_aot()
        else:
            self.execute()

def main():
    args = parse_args()

    ark = ArkProgram(args)
    ark.execute_ark()


if __name__ == "__main__":
    sys.exit(main())
