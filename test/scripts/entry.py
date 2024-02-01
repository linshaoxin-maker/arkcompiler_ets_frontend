#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2023 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import os
import subprocess
import time

import schedule

from utils.preparation import prepare_test


def job(cmd):
    subprocess.run(cmd, shell=False)


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--runTime', type=str, dest='run_time', default=None,
                        help='specify when to start the test')
    parser.add_argument('--skipDownloadSdk', dest='skip_download_sdk', action='store_true', default=False,
                        help='specify whether to skip the download sdk or not')
    parser.add_argument('--skipDownloadDayu', dest='skip_download_dayu', action='store_true', default=False,
                        help='specify whether to skip the download dayu or not')

    return parser.parse_args()


def run(arguments):
    prepare_test(arguments)

    job(os.path.join(".", "auto_xts_test", "run.bat"))

    job(f'python {os.path.join(".", "sdk_test", "entry.py")}')

    job(f'python {os.path.join(".", "performance_test", "performance_entry.py")}')

    job(f'python {os.path.join(".", "utils/send_email", "send_email.py")}')


if __name__ == '__main__':
    arguments_info = parse_args()
    if arguments_info.run_time is not None:
        run_time = arguments_info.run_time
        os.chdir(os.path.dirname(os.path.realpath(__file__)))
        schedule.every().day.at(run_time).do(run(arguments_info))
        while True:
            schedule.run_pending()
            time.sleep(1)
    else:
        run(arguments_info)


