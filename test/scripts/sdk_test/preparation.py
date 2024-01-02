#!/usr/bin/env python3
# coding: utf-8

"""
Copyright (c) 2023 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Description: prepare environment for test
"""


import os
import shutil

import options
from utils import is_mac


def setup_env():
    old_env = os.environ.copy()
    old_env_path = old_env['PATH']

    java_home = os.path.join(options.configs.get('deveco_path'), 'jbr')
    node_js_path = options.configs.get('node_js_path')
    if is_mac():
        node_js_path = os.path.join(node_js_path, 'bin')
    java_path = os.path.join(java_home, 'bin')

    os.environ['PATH'] = os.pathsep.join(
        [java_path, node_js_path]) + os.pathsep + old_env_path
    os.environ['JAVA_HOME'] = java_home


def clean_log():
    output_log_file = options.configs.get('log_file')
    daily_report_file = options.configs.get('output_html_file')
    picture_dic = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'pictures')
    if os.path.exists(output_log_file):
        os.remove(output_log_file)
    if os.path.exists(daily_report_file):
        os.remove(daily_report_file)
    if os.path.exists(picture_dic):
        shutil.rmtree(picture_dic)


def prepare_test_env():
    clean_log()
    setup_env()



