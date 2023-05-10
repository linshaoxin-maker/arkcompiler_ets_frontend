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
    if not os.path.exists(tsc_package_path):
        cmd = " ".join(["cp", "-f", tsc_path, deps_path])
        run_cmd(cmd)
    
    if not os.path.exists(os.path.join(args[1], "node_modules", "typescript")):
        cmd = " ".join(["npm", "install", tsc_path, "--legacy-peer-deps"])
        run_cmd(cmd, args[1])

    if not os.path.exists(os.path.join(cur_path, "node_modules", "typescript")):
        tsc_cur_path = os.path.join(deps_path, "typescript-4.2.3-r2.tgz")
        cmd = " ".join(["npm", "install", tsc_cur_path, "--legacy-peer-deps"])
        run_cmd(cmd, cur_path)

if __name__ == "__main__":
    run(sys.argv[1:])