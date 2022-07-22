import os
import subprocess
import argparse
import sys
from utils import *
from config import *


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--dir', metavar='DIR', help="Directory to test")
    parser.add_argument('--file', metavar='FILE', help="File to test")
    parser.add_argument(
        '--ark_frontend_tool',
        help="ark frontend conversion tool")
    arguments = parser.parse_args()
    return arguments


def run_js_test(filepath):
    output_path = filepath.replace(JS_EXT, TXT_EXT)
    command_os(['node', '--expose-gc', DEFAULT_ARK_FRONTEND_TOOL, filepath])
    abc_filepath = ARK_TS2ABC_PATH + filepath.replace(JS_EXT, ABC_EXT)
    os.chdir('../../')
    run_abc_cmd = [ARK_JS_VM, abc_filepath]
    run_abc_result = subprocess.Popen(run_abc_cmd, stdout=subprocess.PIPE)
    js_result_list = []
    for line in run_abc_result.stdout.readlines():
        js_result = str(line).lstrip("b'").rstrip("\\n'")+'\n'
        js_result_list.append(js_result)
    remove_file(abc_filepath)
    os.chdir(ARK_TS2ABC_PATH)
    with open(output_path,'w') as output:
        output.writelines(js_result_list)
    


def compare(filepath):
    outputfilepath = filepath.replace(JS_EXT, TXT_EXT)
    outcont = read_file(outputfilepath)
    ll = filepath.split(os.sep)
    del ll[-1]
    ll.append(EXPECT_PATH)
    expectfilepath = (os.sep).join(ll)
    expectcont = read_file(expectfilepath)
    if outcont == expectcont:
        result = f'PASS {filepath}\n'
    else:
        result = f'FAIL {filepath}\n'
    remove_file(outputfilepath)
    print(result)
    return result


def run_test_machine(args):
    result_path = []
    if args.file:
        run_js_test(args.file)
        result = compare(args.file)
        result_path.append(result)
    elif args.dir:
        for root, dirs, files in os.walk(args.dir):
            for file in files:
                filepath = f'{root}/{file}'
                if filepath.endswith(JS_EXT):
                    run_js_test(filepath)
                    result = compare(filepath)
                    result_path.append(result)
    elif args.file is None and args.dir is None:
        for root, dirs, files in os.walk(TS_CASES_DIR):
            for file in files:
                filepath = f'{root}/{file}'
                if filepath.endswith(JS_EXT):
                    run_js_test(filepath)
                    result = compare(filepath)
                    result_path.append(result)
    with open(OUT_RESULT_FILE, 'w') as read_out_result:
        read_out_result.writelines(result_path)


def init_path():
    remove_dir(OUT_TEST_DIR)
    mk_dir(OUT_TEST_DIR)


def summary():
    if not os.path.exists(OUT_RESULT_FILE):
        return
    count = -1
    fail_count = 0
    with open(OUT_RESULT_FILE, 'r') as read_outfile:
        for count, line in enumerate(read_outfile):
            if line.startswith("FAIL"):
                fail_count += 1
            pass
    count += 1

    print("\n      Regression summary")
    print("===============================")
    print("     Total         %5d         " % (count))
    print("-------------------------------")
    print("     Passed tests: %5d         " % (count - fail_count))
    print("     Failed tests: %5d         " % (fail_count))
    print("===============================")


def main(args):
    try:
        init_path()
        excuting_npm_install(args)
        export_path()
        run_test_machine(args)
        summary()
    except BaseException:
        print("Run Python Script Fail")


if __name__ == "__main__":
    sys.exit(main(parse_args()))

