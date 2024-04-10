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

Description: Generate javascript byte code using es2abc
"""
import os
import tempfile

replace_config1 = [
    {"id": "REPLACE_FUNC_FOO1", "start": 0, "end": 31608},
    {"id": "REPLACE_FUNC_FOO2", "start": 31609, "end": 65535},
]
replace_config2 = [
    {"id": "REPLACE_FUNC_FOO1", "start": 0, "end": 65488},
    {"id": "REPLACE_FUNC_FOO2", "start": 65489, "end": 65535},
]
replace_config3 = [
    {"id": "REPLACE_FUNC_FOO1", "start": 0, "end": 65488},
    {"id": "REPLACE_FUNC_FOO2", "start": 65489, "end": 65536},
]

file_list = [
    {
        "file_name": "ets_runtime/test/quickfix/multi_funcconstpool/base_modify.js",
        "replace_config": replace_config1,
    },
    {
        "file_name": "ets_runtime/test/quickfix/multi_funcconstpool/base.js",
        "replace_config": replace_config1,
    },
    {
        "file_name": "ets_runtime/test/quickfix/multi_funccallconstpool/base_modify.js",
        "replace_config": replace_config1,
    },
    {
        "file_name": "ets_runtime/test/quickfix/multi_funccallconstpool/base.js",
        "replace_config": replace_config1,
    },
    {
        "file_name": "ets_runtime/test/quickfix/multi_constructorconstpool/base_modify.js",
        "replace_config": replace_config1,
    },
    {
        "file_name": "ets_runtime/test/quickfix/multi_constructorconstpool/base.js",
        "replace_config": replace_config1,
    },
    {
        "file_name": "ets_runtime/test/quickfix/multi_closureconstpool/base_modify.js",
        "replace_config": replace_config3,
    },
    {
        "file_name": "ets_runtime/test/quickfix/multi_closureconstpool/base.js",
        "replace_config": replace_config3,
    },    
    {
        "file_name": "ets_runtime/test/quickfix/multi_classconstpool/base_modify.js",
        "replace_config": replace_config2,      
    },
    {
        "file_name": "ets_runtime/test/quickfix/multi_classconstpool/base.js",
        "replace_config": replace_config2,
    },
    {
        "file_name": "ets_runtime/test/moduletest/multiconstpoolobj/multiconstpoolobj.js",
        "replace_config": replace_config2,
    },    
    {
        "file_name": "ets_runtime/test/moduletest/multiconstpoolfunc/multiconstpoolfunc.js",
        "replace_config": replace_config2,   
    },
    {
        "file_name": "ets_runtime/test/moduletest/multiconstpoolconstructor/multiconstpoolconstructor.js",
        "replace_config": replace_config2,
    },  
    {
        "file_name": "ets_runtime/test/moduletest/multiconstpoolclass/multiconstpoolclass.js",
        "replace_config": replace_config2,
    },    
    {
        "file_name": "ets_runtime/test/moduletest/multiconstpoolarray/multiconstpoolarray.js",
        "replace_config": replace_config2,
    },
]


def generate_var(var_begin, var_end):
    sVar = ""
    for i in range(var_begin, var_end + 1):
        sVar += 'var a%d = "%d";' % (i, i)
        if (i + 1) % 6 == 0:
            sVar += "\n"
    return sVar


def get_tmp_file_name(srcfile):
    temp_dir = tempfile.gettempdir()
    pth = srcfile.split("/")
    dist_pth = os.path.join(temp_dir, pth[-3], pth[-2])
    if not os.path.exists(dist_pth):
        try:
            os.makedirs(dist_pth)
        except:
            pass
    return os.path.join(dist_pth, pth[-1])


def write_to_tmp(src_js, data):
    dist_js = get_tmp_file_name(src_js)
    with open(dist_js, "w") as fp:
        fp.write(data)
    return dist_js


def replace_js(src_js):
    for file in file_list:
        if src_js.endswith(file["file_name"]):
            with open(src_js, "r") as fp:
                data = fp.read()

            for cfg in file["replace_config"]:
                if cfg["id"] in data:
                    sVar = generate_var(cfg["start"], cfg["end"])
                    data = data.replace(cfg["id"], sVar)

            return write_to_tmp(src_js, data)
    return None


def replace_var(src_js, merge_abc, generate_patch):
    if (merge_abc or generate_patch) and src_js.endswith(".txt"):
        with open(src_js[1:], "r") as fp:  # open test.txt
            replaced = False
            ls = fp.readlines()
            for i, l in enumerate(ls):
                ll = l.split(";")  # ll[0] is js file
                js_fn = replace_js(ll[0])
                if js_fn:
                    ll[0] = js_fn
                    ls[i] = ";".join(ll)  # reset js file name
                    replaced = True
            if replaced:
                dist_txt = get_tmp_file_name(src_js)
                with open(dist_txt, "w") as fp:
                    fp.writelines(ls)
                return "@" + dist_txt
    else:
        return replace_js(src_js)
    return src_js
