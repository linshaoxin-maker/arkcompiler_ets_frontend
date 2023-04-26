# importing required modules
import argparse
import os
import shutil
import tempfile

from tool.test_helper import get_path_file, get_disable_list, is_disable_case
from tool.testcfg import TestCase


TEST_PATH = './'
TEST_TMP_PATH = '/testTmp/'
TEMP_PATH = os.getcwd() + TEST_TMP_PATH


if os.path.exists(TEMP_PATH):
    shutil.rmtree(TEMP_PATH)


if (os.path.exists(TEMP_PATH) == False):
    os.mkdir(TEMP_PATH)

total_case = 0
failed_case = 0
TestCase.temp_path = TEMP_PATH
def is_testcase_exist(parser, arg):
    if not os.path.isabs(arg):
        arg = TEST_PATH + arg
    if not os.path.exists(arg):
        parser.error("The directory or file '%s' does not exist" % arg)
    return os.path.abspath(arg)

def is_file(parser, arg):
    if not os.path.isfile(arg):
        parser.error("The file '%s' does not exist" % arg)

    return os.path.abspath(arg)

def is_directory(parser, arg):
    if not os.path.isdir(arg):
        parser.error("The directory '%s' does not exist" % arg)

    return os.path.abspath(arg)

def parse_and_execute(path, arkruntime = False, skip_negative = True):
    if path.endswith(".ts"):
        test_case = TestCase(path)
        if not test_case.is_test_case:
            return False, False
        # check test case declare
        if not test_case.check_declaration():
            print(test_case.path, test_case.detail_result, sep='\t')
            return True, True
        if skip_negative and test_case.is_negative():
            return False, False
        else:
            test_case.execute(arkruntime)
            if test_case.fail:
                print('TESTCASE Fail! Fail reason is coming:')
                print(test_case.path, test_case.detail_result, sep='\t')
                return True, True
            return True, False

# create a parser object
parser = argparse.ArgumentParser(description = "TypeScript Spec&Feature Test Tool")


# 指定测试范围,可以指定多个目录 or 文件，指定目录下的所有测试都会被执行
parser.add_argument("release", nargs = '*', metavar = "release", type = lambda arg: is_testcase_exist(parser, arg),
					help = "All test case in the release will be execute")

parser.add_argument("-a", "--arkruntime", action="store_true", default=False, help= "test on arkruntime")

parser.add_argument("-s", "--skip-abnormal-case", action="store_true", default=False, help= "skip abnormal test case")

# skip list 被指定的场合，优先级最高，例如一个case在ski-list的文件中被指定，同时也被指定执行，那么它会被忽略
parser.add_argument("-d", "--disable-list", type= lambda arg: is_file(parser, arg), default=None, 
                    help= "path to the file that contains test to skip")

parser.add_argument(
        '--js-runtime', dest='js_runtime_path', default=None, type=lambda arg: is_directory(parser, arg),
        help='the path of js vm runtime')
parser.add_argument(
        '--LD_LIBRARY_PATH', dest='ld_library_path', default=None, help='LD_LIBRARY_PATH')

# parse the arguments from standard input
args = parser.parse_args()
if args.js_runtime_path:
    TestCase.js_runtime_path = args.js_runtime_path
if args.ld_library_path:
    TestCase.ld_library_path = args.ld_library_path

disable_list = []
if args.disable_list:
    disable_list = get_disable_list(args.disable_list)

# 执行测试脚本、这里需要分情况 tsc + node / es2abc
print("TEST CASE", "FAIL REASON", "FAIL LINE", sep="\t")
for file_path in args.release:
    if is_disable_case(file_path, disable_list):
        continue
    if os.path.isfile(file_path):
        is_test_count , failed = parse_and_execute(file_path, args.arkruntime, args.skip_abnormal_case)
        if is_test_count:
            total_case += 1
            if failed:
                failed_case += 1
        continue
    for file_path in get_path_file(file_path):
        if False == file_path.endswith(".ts"):
            continue
        if is_disable_case(file_path, disable_list):
            continue
        is_test_count , failed = parse_and_execute(file_path, args.arkruntime, args.skip_abnormal_case)
        if is_test_count:
            total_case += 1
            if failed :
                failed_case += 1

print("TOTAL CASE COUNT:%d" % total_case)
print("FAILED CASE COUNT:%d" % failed_case)
# delete temp dir
if os.path.exists(TEMP_PATH):
    shutil.rmtree(TEMP_PATH)