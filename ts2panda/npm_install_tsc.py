#!/usr/bin/env python

import sys
import subprocess

def run_cmd(cmd, execution_path = None):
    res = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                           stdin=subprocess.PIPE,
                           stderr=subprocess.PIPE,
                           cwd=execution_path,
                           shell=True)
    sout, serr = res.communicate()
    res.wait()
    return res.pid, res.returncode, sout, serr

def run(args):
    tsc_path =args[0]
    cmd = " ".join(["npm", "install", tsc_path, "--legacy-peer-deps"])
    run_cmd(cmd, args[1])
    run_cmd(cmd, args[2])

if __name__ == "__main__":
    run(sys.argv[1:])