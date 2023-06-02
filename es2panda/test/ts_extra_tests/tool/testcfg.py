#  Copyright (c) 2023 Huawei Device Co., Ltd.
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.


import os
import re
import yaml
import platform
import subprocess
from tool.test_helper import read_declaration

STRICT_OFF = ['--strict', 'false']
STRICT_ON = ['--strict', 'true']
MODULE = ['--module']

def get_error_message(str, filename):
    if len(re.findall(filename + r':(\d+)', str)) > 0:
        line_number = re.findall(filename + r':(\d+)', str)
    else:
        line_number = 0
    err_message = str
    return err_message, line_number


class TestCase():

    temp_path = ""
    ld_library_path = ""
    js_runtime_path = ""
    es2abc = ""
    tsc = ""

    def __init__(self, path):
        self.path = path
        self.target_js_path =""
        try:
            data = yaml.load(read_declaration(path), yaml.SafeLoader)
        except:
            data = {}
        self.declaration = data
        self.fail = False
        self.is_test_case = False if data is None else True
        self.detail_result = ""
        self.err_line = 0
        self.abc_file_path = ""
        self.abc_file_path_temp = ""


    def execute(self, arkruntime = False):
        if not self.is_test_case:
            return
        if arkruntime:
            with open(self.path, 'a') as fileAdded:
                fileAdded.write('print("TESTCASE SUCCESS");')
            self.__test_es2abc()
            if(os.path.exists(self.abc_file_path)):
                os.remove(self.abc_file_path)
        else:
            self.__tsc_test()

    def is_negative(self):
        if 'error' in self.declaration:
            return True
        return False
    def is_current(self):
        if 'isCurrent' in self.declaration:
            return True
        return False
    def is_set_module(self):
        if 'module' in self.declaration:
            return True
        return False
    # check if
    def check_declaration(self):
        if self.declaration == {}:
            self.detail_result = "parse test case declaration failed, maybe bad format."
            return False
        if 'error' in self.declaration:
            if self.declaration['error'] is None or 'code' not in self.declaration['error'] and 'type' not in self.declaration['error']:
                self.detail_result = "neither error code nor error type are defined in negative case."
                return False
        return True

    def __error_code(self):
        if 'code' in self.declaration['error']:
            return self.declaration['error']['code']
        return None

    def __error_type(self):
        if 'type' in self.declaration['error']:
            return self.declaration['error']['type']
        return None

    def __get_tsc_cmd(self):
        if platform.system().lower() == 'windows':
            cmd = ['cmd', '/c', 'tsc', '--target', 'es2020']
        else:
            cmd = [TestCase.tsc, '--target', 'es2020']
        if self.__is_strict():
            cmd.extend(STRICT_ON)
        else:
            cmd.extend(STRICT_OFF)
        if self.is_set_module():
            cmd.extend(MODULE)
            cmd.append('es2020')
        if self.is_current():
            cmd.append(self.path)
            cmd.append('--outDir')
            cmd.append(TestCase.temp_path)
            self.target_js_path = TestCase.temp_path + self.__get_js_basename()
        return cmd

    def __get_node_cmd(self):
        cmd = ['node']
        if self.is_current():
            cmd.append(self.target_js_path)
        else:
            cmd.append(TestCase.temp_path + self.__get_js_basename())
        return cmd

    # get es2abc --merge-abc
    def __get_es2abc_cmd(self, file_path):
        abc_file_path = ("%s.abc" % (os.path.splitext(file_path)[0]))
        self.abc_file_path_temp = abc_file_path
        cmd = [TestCase.es2abc + 'es2abc']
        cmd.extend(['--module', '--output', abc_file_path, file_path])
        return cmd

    # create abc files
    def create_abc(self, filename):
        process = subprocess.Popen(self.__get_es2abc_cmd(filename), stdin = subprocess.PIPE, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
        out, err = process.communicate()
        returncode = process.returncode
        if returncode != 0:
            err_msg, line = get_error_message(out.decode("utf-8", errors="ignore") + err.decode("utf-8", errors="ignore"), filename)
            self.detail_result = err_msg
            self.err_line = line
            self.fail = True
            return
        if "TESTCASE SUCCESS" not in out.decode("utf-8", errors="ignore"):
            self.detail_result = "check stdout failed!"
            self.fail = True
            return

    # get es2abc file commands
    def _get_ark_js_cmd(self):
        os.environ.setdefault("LD_LIBRARY_PATH", TestCase.ld_library_path)
        run_abc_cmd = [os.path.join(TestCase.js_runtime_path, 'ark_js_vm')]
        run_abc_cmd.append(self.abc_file_path_temp)
        return run_abc_cmd
        pass

    def __get_js_basename(self):
        sp = '/'
        return "test_ts_cases" + sp + self.path.split(sp+"test_ts_cases"+sp)[1].replace('.ts', '.js')

    def __is_strict(self):
        if 'strict' in self.declaration:
            return bool(self.declaration['strict'])
        return True

    def __tsc_test(self):
        process = subprocess.Popen(self.__get_tsc_cmd(), stdin = subprocess.PIPE, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
        out, err = process.communicate()
        returncode = process.returncode
        if self.is_negative():
            if returncode == 0:
                self.fail = True
                self.detail_result = "No error found in negative case."
                return
            if self.__error_code() in out.decode("utf-8", errors="ignore") + err.decode("utf-8", errors="ignore"):
                return
            self.fail = True
            self.detail_result = "Error code not as expected."
            return
        # positive case
        if returncode != 0 :
            self.detail_result = out.decode("utf-8", errors="ignore") + err.decode("utf-8", errors="ignore")
            self.fail = True
            return
        if self.is_current():
            with open(self.target_js_path, 'a') as fileAdded:
                fileAdded.write('console.log("TESTCASE SUCCESS");')
        # run node command
        process = subprocess.Popen(self.__get_node_cmd(), stdin = subprocess.PIPE, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
        out, err = process.communicate()
        returncode = process.returncode
        if self.is_current():
            if os.path.exists(self.target_js_path):
                os.remove(self.target_js_path)
        if returncode != 0:
            err_msg, line = get_error_message(out.decode("utf-8", errors="ignore") + err.decode("utf-8", errors="ignore"), self.__get_js_basename())
            self.detail_result = err_msg
            self.err_line = line
            self.fail = True
            return
        # check std out
        if "TESTCASE SUCCESS" not in out.decode("utf-8", errors="ignore"):
            self.detail_result = "check stdout failed!"
            self.fail = True
            return

    def __test_es2abc(self):
        # compiler to abc
        process = subprocess.Popen(self.__get_es2abc_cmd(self.path), stdin = subprocess.PIPE, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
        out, err = process.communicate()
        returncode = process.returncode
        if self.is_negative():
            if returncode == 0:
                self.fail = True
                return
            if self.__error_type() in out.decode("utf-8", errors="ignore") + err.decode("utf-8", errors="ignore"):
                return
            self.fail = True
            self.detail_result = "Error type not as expected."
            return
        # positive case
        if returncode != 0 :
            self.detail_result = out.decode("utf-8", errors="ignore") + err.decode("utf-8", errors="ignore")
            self.fail = True
            return
        # execute ark_js_vm
        process = subprocess.Popen(self._get_ark_js_cmd(), stdin = subprocess.PIPE, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
        out, err = process.communicate()
        returncode = process.returncode
        if returncode != 0:
            err_msg, line = get_error_message(out.decode("utf-8", errors="ignore") + err.decode("utf-8", errors="ignore"), os.path.basename(self.abc_file_path))
            self.detail_result = err_msg
            self.err_line = line
            self.fail = True
            return
        # check std out
        if "TESTCASE SUCCESS" not in out.decode("utf-8", errors="ignore"):
            self.detail_result = "check stdout failed!"
            self.fail = True
            return
