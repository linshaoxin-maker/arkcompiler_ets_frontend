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

import logging
import os
import shutil

import options
from utils import is_mac, is_linux


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


def check_deveco_env():
    if is_linux():
        return False

    java_path = os.path.join(options.configs.get('deveco_path'), 'jbr')
    if not os.path.exists(java_path):
        logging.error("Java not found!")
        return False

    if not os.path.exists(options.configs.get('node_js_path')):
        logging.error("Node js not found!")
        return False

    return True


def prepare_image():
    if options.arguments.run_haps:
        return True

    # TODO: get pictures reference

    return True


def remove_file_or_dir(path):
    try:
        if os.path.isfile(path) or os.path.islink(path):
            os.remove(path)
        elif os.path.isdir(path):
            shutil.rmtree(path)
    except OSError as e:
        logging.error(f"Error: {e.filename} - {e.strerror}.")


def clean_log():
    output_file_paths = options.configs['output_file_paths']
    for key, path in output_file_paths.items():
        remove_file_or_dir(path)


def prepare_test_env():
    clean_log()
    prepared = check_deveco_env()
    setup_env()
    prepared = prepared and prepare_image()
    return prepared
