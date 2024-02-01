#!/usr/bin/env python3
# coding: utf-8

"""
Copyright (c) 2024 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Description: utils for test suite
"""

import os
import subprocess

from autoburn import auto_burn


def job(cmd):
    subprocess.run(cmd, shell=False)


def start_download_sdk():
    sdk_download_cmd = ['python', './download.py']
    sdk_download_cmd.extend(['--sdkUrl', ''])
    sdk_download_cmd.extend(['--sdkPath', None])
    sdk_download_cmd.extend(['--sdkDate', None])

    job(sdk_download_cmd)


def start_download_dayu():
    dayu_download_cmd = ['python', './download.py']
    dayu_download_cmd.extend(['--dayuUrl', ''])
    dayu_download_cmd.extend(['--dayuDate', None])
    dayu_download_cmd.extend(['--dayuPath', None])

    job(dayu_download_cmd)


def get_commit_message():
    crawl_cmd = ['python', './get_commit_log.py']
    crawl_cmd.extend(['--repoName', None])
    crawl_cmd.extend(['--startTime', None])

    job(crawl_cmd)


def clean_download_log():
    download_url_txt = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                    'download_url.txt')
    if os.path.exists(download_url_txt):
        os.remove(download_url_txt)


def prepare_test(arguments):
    clean_download_log()
    if not arguments.skip_download_sdk:
        start_download_sdk()

    if not arguments.skip_download_dayu:
        start_download_dayu()
        auto_burn()
    get_commit_message()
