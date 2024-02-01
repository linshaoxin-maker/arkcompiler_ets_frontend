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
"""

import argparse
import asyncio
import json
import os

import utils


def before_update_sdk(file_path):
    pass


def after_update_sdk(file_path, output_path):
    # modify sdk version while api is different from your download
    modify_package_json(file_path, output_path)


def get_output_path_api_version(output_path):
    last_parma = os.path.basename(output_path)
    return last_parma


def modify_package_json(file_path, output_path):
    output_path_version = get_output_path_api_version(output_path)
    api_version = utils.get_api_version(os.path.join(
        *[file_path, 'ets', 'oh-uni-package.json']))
    if output_path_version != api_version:
        for file_name in os.listdir(output_path):
            package_json_path = os.path.join(output_path, rf'{file_name}\oh-uni-package.json')
            with open(package_json_path, 'r') as json_file:
                data = json.load(json_file)
            data['apiVersion'] = get_output_path_api_version(output_path)
            with open(package_json_path, 'w') as file:
                json.dump(data, file, indent=2)


async def update_sdk_to_output_path(file_path, output_path):
    before_update_sdk(file_path)
    await utils.copy_image_async(file_path, output_path)
    after_update_sdk(file_path, output_path)


async def update_dayu_to_output_path(file_path, output_path):
    await utils.copy_image_async(file_path, output_path)


async def update_image_async(file_path, output_path, semaphore):
    await semaphore.acquire()
    try:
        update_task_mapping = {
            "sdk": update_sdk_to_output_path,
            "dayu": update_dayu_to_output_path
        }
        for task in update_task_mapping:
            if task in file_path:
                print('sdk')
                await update_task_mapping[task](file_path, output_path)
                await asyncio.sleep(1)
                print(f'File copied to {output_path} successfully')
                break
            print("Invalid file path")
    finally:
        semaphore.release()


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--sdkFilePath', type=str, dest='sdk_file_path', default=None,
                        help='specify which sdk image you want to copy')
    parser.add_argument('--sdkOutputPath', type=str, dest='sdk_output_path', default=None,
                        nargs='+',
                        help='specify where you want to store the file')
    parser.add_argument('--dayuFilePath', type=str, dest='dayu_file_path', default=None,
                        help='specify which dayu image you want to copy')
    parser.add_argument('--dayuOutputPath', type=str, dest='dayu_output_path', default=None,
                        nargs='+',
                        help='specify where you want to store the file')

    return parser.parse_args()


async def start_update_image_task(file_path, output_path_list, max_async_tasks=3):
    if output_path_list is None:
        file_name = utils.parse_file_name(file_path)
        output_path_list = utils.get_output_path_list(file_name)
    semaphore = asyncio.Semaphore(max_async_tasks)
    tasks = [asyncio.create_task(update_image_async(file_path, output_path, semaphore))
             for output_path in output_path_list]

    await asyncio.gather(*tasks)


async def run():
    arguments = parse_args()

    if arguments.sdk_file_path is not None:
        await start_update_image_task(arguments.sdk_file_path, arguments.sdk_output_path)
    if arguments.dayu_file_path is not None:
        await start_update_image_task(arguments.dayu_file_path, arguments.dayu_output_path)
    if arguments.sdk_file_path is None and arguments.dayu_file_path is None:
        print('please input which image you want to copy')
        return


if __name__ == '__main__':
    asyncio.run(run())
