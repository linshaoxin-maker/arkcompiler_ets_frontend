#!/usr/bin/env python3
# coding: utf-8

"""
Copyright (c) 2021 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Description: Generate project byte code using es2abc
"""

import argparse
import json
import os
import subprocess

accept_file_extensions = ['js', 'ts', 'ets']


def gen_files_info_for_dir(fp, prefix, directory):
    for file in os.listdir(directory):
        path = os.path.join(directory, file)
        if os.path.isdir(path):
            gen_files_info_for_dir(fp, os.path.join(prefix, file), path)
        else:
            record_name = os.path.join(prefix, file)
            record_name = record_name.replace('\\', '/')
            (name, extension) = record_name.rsplit(".", 1)
            if not (extension in accept_file_extensions):
                continue
            print(record_name)
            fp.writelines("{};{};esm;{};entry;\n".format(path, name, record_name))


class Project:
    def __init__(self, name, root_dir, out_dir):
        self.name = name
        self.root_dir = root_dir
        self.out_dir = out_dir
        self.obf_dir = os.path.join(out_dir, 'obf')
        self.files_info_path = os.path.join(out_dir, 'filesInfo.txt')
        self.modules_abc_path = os.path.join(out_dir, 'modules.abc')
        self.obf_modules_abc_path = os.path.join(self.obf_dir, 'modules.abc')

    def init(self):
        if not os.path.exists(self.out_dir):
            os.makedirs(self.out_dir)
        if not os.path.exists(self.obf_dir):
            os.makedirs(self.obf_dir)
        if os.path.exists(self.files_info_path):
            os.remove(self.files_info_path)

    def gen_config_file(self):
        configFiles = []
        for file in os.listdir(self.root_dir):
            if file.endswith(".json") and '.cache' not in file:
                configFiles.append(os.path.join(self.root_dir, file))
        for configFile in configFiles:
            with open(configFile) as in_file:
                fileName = os.path.split(configFile)[-1]
                name = fileName.split('.')[0]
                data = json.load(in_file)
                data['abcFilePath'] = self.modules_abc_path
                data['obfAbcFilePath'] = os.path.join(self.out_dir, 'obf', name + '.abc')
                data['obfPaFilePath'] = os.path.join(self.out_dir, 'obf', name + '.pa')
                data['defaultNameCachePath'] = os.path.join(self.out_dir, 'obf', name + '.cache.json')
                outConfigFile = os.path.join(self.out_dir, fileName)
                with open(outConfigFile, 'w') as out_file:
                    out_file.write(json.dumps(data, indent=2))

    def gen_files_info(self):
        with open(self.files_info_path, 'w') as fp:
            gen_files_info_for_dir(fp, '', self.root_dir)

    def gen_abc(self):
        self.init()
        self.gen_config_file()
        self.gen_files_info()


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--projects-dir",
                        help="project root directory")
    parser.add_argument("--out-dir",
                        help="output directory")
    parser.add_argument("--frontend-tool-path",
                        help="path of the frontend conversion tool")
    return parser.parse_args()


def run_command(cmd, execution_path):
    print(" ".join(cmd) + " | execution_path: " + execution_path)
    proc = subprocess.Popen(cmd, cwd=execution_path)
    proc.wait()


def gen_projects_abc(args):
    (path, name) = os.path.split(args.frontend_tool_path)

    for file in os.listdir(args.projects_dir):
        project_path = os.path.join(args.projects_dir, file)
        if os.path.isdir(project_path):
            print('found project:' + file)
            project = Project(file, project_path, os.path.join(args.out_dir, file))
            project.gen_abc()
            cmd = [os.path.join("./", name, "es2abc"),
                   "@{}".format(project.files_info_path),
                   "--output", project.modules_abc_path,
                   "--target-api-sub-version=beta3",
                   "--merge-abc"]
            run_command(cmd, path)


if __name__ == '__main__':
    print('=============================================')
    gen_projects_abc(parse_args())
    print('=============================================')
