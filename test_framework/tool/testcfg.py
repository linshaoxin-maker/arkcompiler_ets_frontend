import os
import re
import yaml
import platform
import subprocess
from tool.test_helper import read_declaration

STRICT_OFF = ['--strict', 'false']
STRICT_ON = ['--strict', 'true']
HARNESS_PATH = os.path.abspath('./harness/')
MODULE = ['--module']

def get_error_message(str, filename):
    line_number = re.findall(filename + r':(\d+)', str)[-1]
    err_message = str #re.findall(r'\[AssertionError\]: ([\S ]+?)\\', str)[0]
    return err_message, line_number
    

class TestCase():
    
    temp_path = ""
    ld_library_path = ""
    js_runtime_path = ""
    
    def __init__(self, path):
        self.path = path
        self.target_js_path =""
        try: 
            data = yaml.load(read_declaration(path), yaml.SafeLoader)
        except:
            data = {}
        self.declaration = data
        self.includes = ['assertionError.ts', 'assert.ts']
        self.fail = False
        self.is_test_case = False if data is None else True
        self.detail_result = ""
        self.err_line = 0
        self.abc_file_path = ""
        self.files_info_path = os.path.join(os.path.splitext(self.path)[0], '_filesInfo.txt')
        
        
    def execute(self, arkruntime = False):
        if not self.is_test_case:
            return
        if arkruntime:
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
    '''检查测试用例声明是否合法'''
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
        
    def __get_includes(self):
        includes = ['assertionError.ts', 'assert.ts']
        if "includes" in self.declaration:
            includes.extend(self.declaration['includes'])
        for index, value in enumerate(includes):
            includes[index] = os.path.join(HARNESS_PATH, value)
        return includes
    
    def __get_tsc_cmd(self):
        if platform.system().lower() == 'windows':
            cmd = ['cmd', '/c', 'tsc', '--target', 'es2020']
        else:
            cmd = ['tsc', '--target', 'es2020']
        if self.__is_strict():
            cmd.extend(STRICT_ON)
        else:
            cmd.extend(STRICT_OFF)
        if self.is_set_module():
            cmd.extend(MODULE)
            cmd.append('ESNext')
        if self.is_current():
            cmd.append(self.path)
            cmd.append('--outDir')
            cmd.append(TestCase.temp_path)
            self.target_js_path = TestCase.temp_path + self.__get_js_basename()
        else:
            cmd.append('--outFile')
            cmd.append(TestCase.temp_path + self.__get_js_basename())    
            cmd.extend(self.__get_includes())
            cmd.append(self.path)
            cmd.append(HARNESS_PATH + "/console.ts")
        print(' '.join(cmd))
        return cmd
    
    def __get_node_cmd(self):
        cmd = ['node']
        if self.is_current():
            cmd.append(self.target_js_path)
        else:
            cmd.append(TestCase.temp_path + self.__get_js_basename())
        return cmd
    
    '''
    生成fileinfo文件，供--merge-abc使用
    '''
    def __gen_files_info(self):
        fd = os.open(self.files_info_path, os.O_RDWR | os.O_CREAT | os.O_TRUNC)
        f = os.fdopen(fd, "w")
        files = self.__get_includes()
        files.append(self.path)
        files.append(HARNESS_PATH + "print.ts")
        for test_path in files:
            record_name = os.path.relpath(test_path, os.path.dirname(self.files_info_path)).split('.')[0]
            module_kind = "esm"
            file_info = ('%s;%s;%s;%s;%s' % (test_path, record_name, module_kind, test_path, record_name))
            f.writelines(file_info + '\n')
        f.close()
    
    '''
    获取es2abc --merge-abc命令
    '''
    def __get_es2abc_cmd(self):
        self.abc_file_path = ("%s.abc" % (os.path.splitext(self.path)[0]))
        cmd = ['es2abc', '--merge-abc']
        cmd.extend(['--output', self.abc_file_path, self.files_info_path])
        return cmd

    '''
    获取执行abc文件的命令
    '''
    def _get_ark_js_cmd(self):
        os.environ.setdefault("LD_LIBRARY_PATH", TestCase.ld_library_path)
        run_abc_cmd = [os.path.join(TestCase.js_runtime_path, 'ark_js_vm')]
        run_abc_cmd.append(self.abc_file_path)
        pass
        
    
    def __get_js_basename(self):
        return os.path.basename(self.path).replace('.ts', '.js')
     
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
        #positive case
        if returncode != 0 :
            self.detail_result = out.decode("utf-8", errors="ignore") + err.decode("utf-8", errors="ignore")
            self.fail = True
            return
        if self.is_current():
            with open(self.target_js_path, 'a') as fileAdded:
              fileAdded.write('console.log("TESTCASE SUCCESS");')
        #run node command
        process = subprocess.Popen(self.__get_node_cmd(), stdin = subprocess.PIPE, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
        out, err = process.communicate()
        returncode = process.returncode
        if self.is_current():
            if os.path.exists(self.target_js_path):
                os.remove(self.target_js_path)
        if returncode != 0:
            err_msg, line = get_error_message(out.decode("utf-8", errors="ignore") + err.decode("utf-8", errors="ignore"), self.__get_js_basename())
            self.detail_result = err_msg
            print(err_msg)
            self.err_line = line
            self.fail = True
            return
        # check std out
        if "TESTCASE SUCCESS" not in out.decode("utf-8", errors="ignore"):
            self.detail_result = "check stdout failed!"
            self.fail = True
            return
        
    def __test_es2abc(self):
        self.__gen_files_info()
        # compiler to abc 
        process = subprocess.Popen(self.__get_es2abc_cmd(), stdin = subprocess.PIPE, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
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
        #positive case
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