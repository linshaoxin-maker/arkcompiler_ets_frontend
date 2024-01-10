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


arguments = {}


def download_is_successful():
    if not os.path.exists('download/download_url.txt'):
        return False
    with open('download/download_url.txt', 'r') as file:
        content = file.read()
    if 'successfully' not in content:
        return False

    return True


def start_download_task():
    download_command = ['python', './download/download.py']
    if arguments.download_url is not None:
        download_command.extend(['--downloadUrl', arguments.download_url])
    if arguments.output_path is not None:
        download_command.extend(['--outputPath', arguments.output_path])
    job(download_command)


def start_crawl_task():
    crawl_command = ['python', './get_commit_log/get_commit_log.py']
    if arguments.start_time is not None:
        crawl_command.extend(['--startTime', arguments.start_time])
    if arguments.repo_name is not None:
        crawl_command.extend(['--repoName'])
        for repo_name in arguments.repo_name:
            crawl_command.extend([repo_name])
    job(crawl_command)


def job(cmd):
    subprocess.run(cmd, shell=False)


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--testModel', type=str, dest='test_model', default=all,
                        help='specify which model you want to test')
    parser.add_argument('--runTime', type=str, dest='run_time', default=None,
                        help='specify when to start the test')
    parser.add_argument('--skipDownload', type=str, dest='skip_download', default=False,
                        help='specify whether to skip the download or not')
    parser.add_argument('--downloadUrl', type=str, dest='download_url', default=None,
                        help='specify what you want to download')
    parser.add_argument('--outputPath', type=str, dest='output_path', default=None,
                        nargs='+',
                        help='specify where you want to store the file')
    parser.add_argument('--startTime', type=str, dest='start_time', default=None,
                        help='specify crawl start time')
    parser.add_argument('--repoName', type=str, dest='repo_name', default=None,
                        nargs='+',
                        help='specify which repo you want to crawl')

    global arguments
    arguments = parser.parse_args()


def run():
    if not arguments.skip_download:
        start_download_task()
        if not download_is_successful():
            return

    start_crawl_task()

    os.chdir(os.path.dirname(os.path.realpath(__file__)))
    job(os.path.join(".", "auto_xts_test", "run.bat"))

    job(f'python {os.path.join(".", "sdk_test", "entry.py")}')

    job(f'python {os.path.join(".", "performance_test", "performance_entry.py")}')

    job(f'python {os.path.join(".", "send_email.py")}')


if __name__ == '__main__':
    parse_args()
    if arguments.run_time is not None:
        run_time = arguments.run_time
        os.chdir(os.path.dirname(os.path.realpath(__file__)))
        schedule.every().day.at(run_time).do(run)
        while True:
            schedule.run_pending()
            time.sleep(1)
    else:
        run()


