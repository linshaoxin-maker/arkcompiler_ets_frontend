import os
import sys
import shutil
import argparse
import subprocess as sp

libs_dir_list = [
    '../../out/hispark_taurus/clang_x64/ark/ark',
    '../../out/hispark_taurus/clang_x64/ark/ark_js_runtime',
    '../../out/hispark_taurus/clang_x64/thirdparty/icu',
    '../../prebuilts/clang/ohos/linux-x86_64/llvm/lib'
]

ark_tool_list = [
    '--ark-tool',
    '../../out/hispark_taurus/clang_x64/ark/ark_js_runtime/ark_js_vm'
]

ark_frontend_tool_list = [
    '--ark-frontend-tool',
    '../../out/hispark_taurus/clang_x64/ark/ark/build/src/index.js'
]

ark_frontend_binary_list = [
    '--ark-frontend-binary',
    '../../out/hispark_taurus/clang_x64/ark/ark/es2abc'
]

ark_frontend_list = [
    '--ark-frontend',
    'es2panda'
]

build_target_list = [
    'ark_js_host_linux_tools_packages',
    'ark_ts2abc_build',
    'ets_frontend_build',
    'ts2abc_type_adapter_unit_tests'
]

libs_dir_str = ':'.join(libs_dir_list)
ark_tool_str = '='.join(ark_tool_list)
ark_frontend_tool_str = '='.join(ark_frontend_tool_list)
ark_frontend_binary_str = '='.join(ark_frontend_binary_list)
ark_frontend_str = '='.join(ark_frontend_list)
build_target_str = ''
for build_target in build_target_list:
    build_target_str += ' --build-target' + ' ' + build_target

build_cmd = [
    './build.sh',
    '--product-name',
    'hispark_taurus_standard',
    build_target_str
]

test262_ts2abc_cmd = [
    'python3',
    'test262/run_test262.py',
    '--es2021',
    'all',
    '--libs-dir',
    libs_dir_str,
    ark_tool_str,
    ark_frontend_tool_str
]

test262_es2abc_cmd = [
    'python3',
    'test262/run_test262.py',
    '--es2021',
    'all',
    '--libs-dir',
    libs_dir_str,
    ark_tool_str,
    ark_frontend_binary_str,
    ark_frontend_str
]

ut_cmd = [
    'python3',
    'ts2panda/scripts/run_tests.py'
]

testJs_cmd = [
    'python3',
    'testJs/run_testJs.py'
]

testTs_cmd = [
    'python3',
    'testTs/run_testTs.py'
]

ts_instruction_typemap_test_cmd = [
    '../../out/hispark_taurus/clang_x64/ark/ark/ts2abc_type_adapter_unit_tests'
]

tests_list = [
    test262_es2abc_cmd,
    ut_cmd,
    testTs_cmd,
    testJs_cmd,
    ts_instruction_typemap_test_cmd
]

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--test_type', default=False, 
                        choices=['test262', 'ut', 'testTs', 'ts_instruction_typemap'], help="Choose a different test")
    parser.add_argument('--test262_tool', default=False, 
                        choices=['es2abc', 'ts2abc'], help="test262 different tools")
    arguments = parser.parse_args()
    return arguments  


def main(args):
    try:
        os.chdir('../../')
        if os.path.exists('./out'):
            shutil.rmtree('./out')
        sp.run(build_cmd)
        os.chdir('arkcompiler/ets_frontend/')
        if not args.test_type:
            for test in tests_list:
                sp.run(test)
        elif args.test_type == 'test262':
            if not args.test262_tool:
                sp.run(test262_es2abc_cmd)
            elif args.test262_tool == 'es2abc':
                sp.run(test262_es2abc_cmd)
            elif args.test262_tool == 'ts2abc':
                sp.run(test262_ts2abc_cmd)
        elif args.test_type == 'ut':
            sp.run(ut_cmd)
        elif args.test_type == 'testTs':
            sp.run(testTs_cmd)
        elif args.test_type == 'ts_instruction_typemap':
            sp.run(ts_instruction_typemap_test_cmd)
    except BaseException:
        print("Run Python Script Fail")


if __name__ == "__main__":
    sys.exit(main(parse_args()))
