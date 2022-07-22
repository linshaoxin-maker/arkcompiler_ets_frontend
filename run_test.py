import os
import shutil
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

build_target_list = [
    'ark_js_host_linux_tools_packages',
    'ark_ts2abc_build',
    'ts2abc_type_adapter_unit_tests'
]

libs_dir_str = ':'.join(libs_dir_list)
ark_tool_str = '='.join(ark_tool_list)
ark_frontend_tool_str = '='.join(ark_frontend_tool_list)
ld_library_path_str = ':'.join([i.replace('../../', '') for i in libs_dir_list])
build_target_str = ''
for build_target in build_target_list:
    build_target_str += ' --build-target' + ' ' + build_target


build_cmd = [
    './build.sh',
    '--product-name',
    'hispark_taurus_standard',
    build_target_str
]

test262_cmd = [
    'python3',
    'test262/run_test262.py',
    '--es2021',
    'all',
    '--libs-dir',
    libs_dir_str,
    ark_tool_str,
    ark_frontend_tool_str
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
    test262_cmd,
    ut_cmd,
    testTs_cmd,
    testJs_cmd,
    ts_instruction_typemap_test_cmd
]  

os.chdir('../../')
if os.path.exists('./out'):
    shutil.rmtree('./out')
sp.run(build_cmd)     
os.environ['LD_LIBRARY_PATH'] = ld_library_path_str
os.chdir('arkcompiler/ets_frontend/')
for test in tests_list:
    sp.run(test)
