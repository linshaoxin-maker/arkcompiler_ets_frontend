#!/usr/bin/env python

import sys
import subprocess
import os

def run_cmd(cmd, execution_path = None):
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                           stdin=subprocess.PIPE,
                           stderr=subprocess.PIPE,
                           cwd=execution_path,
                           shell=True)
    stdout, stderr = proc.communicate()
    if proc.returncode != 0:
        print(stdout.decode(), stderr.decode())
        raise Exception(stderr.decode())

def run(args):
    tsc_path = args[0]
    cur_path = args[2]
    deps_path = os.path.join(cur_path, "deps")
    if not os.path.exists(deps_path):
        os.makedirs(deps_path, exist_ok= True)

    tsc_package_path = os.path.join(deps_path, "typescript-4.2.3-r2.tgz")
    cmd = " ".join(["cp", "-f", tsc_path, deps_path])
    run_cmd(cmd)
    cmd = " ".join(["npm", "install", tsc_path, "--legacy-peer-deps"])
    run_cmd(cmd, args[1])
    cmd = " ".join(["npm", "install", tsc_package_path, "--legacy-peer-deps"])
    run_cmd(cmd, cur_path)

    if os.path.exists(args[3]):
        cmd =" ".join(["rm", "-rf", args[3]])
        run_cmd(cmd)

if __name__ == "__main__":
    run(sys.argv[1:])