#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
"""


import os

from test262.config import ARGS_PREFIX, RK3568_PRODUCT_NAME, ARK_DIR_SUFFIX

FUZZY_DIR_NAME = 'fuzzilli'
FUZZILLI_DIR_NAME = 'fuzzilli'
FUZZILLI_PATCH_DIR_NAME = 'patchs'
FUZZILLI_OUTPUT_DIR_NAME = 'output'
FUZZY_DIR = os.path.join(FUZZY_DIR_NAME, FUZZILLI_DIR_NAME)

FUZZY_GIT_HASH = "79c777839d23114e6f85322eefa6d44af53814ab"

FUZZY_GIT_URL = "https://gitee.com/mirrors_googleprojectzero/fuzzilli.git"
SWIFT_DOWNLOAD_URL = 'https://download.swift.org/swift-5.9.2-release/ubuntu1804/'\
                      'swift-5.9.2-RELEASE/swift-5.9.2-RELEASE-ubuntu18.04.tar.gz'

FUZZY_DEFAULT_ARK_FRONTEND_BINARY = os.path.join(ARGS_PREFIX, RK3568_PRODUCT_NAME, ARK_DIR_SUFFIX, 'es2abc')

DEFAULT_OPEN_FUZZILLI_MODE = False
