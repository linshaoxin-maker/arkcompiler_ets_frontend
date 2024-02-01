#!/usr/bin/env python3
# coding: utf-8

"""
Copyright (c) 2023 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in awriting, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

import argparse
import shutil
import subprocess
import tarfile
import zipfile
import os

import utils


def copy_to_output_path(task_name, file_path, output_path_list):
    update_file_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'update.py')
    cmd = ['python', update_file_path]
    if 'sdk' in task_name:
        sdk_temp_file = os.path.join(file_path, 'sdk_temp')
        cmd.extend(['--sdkFilePath', sdk_temp_file])
        cmd.extend(['--sdkOutputPath'])
        for output_path in output_path_list:
            cmd.extend([output_path])

    if 'dayu' in task_name:
        dayu_temp_file = os.path.join(file_path, 'dayu200_xts')
        cmd.extend(['--dayuFilePath', dayu_temp_file])
        cmd.extend(['--dayuOutputPath'])
        for output_path in output_path_list:
            cmd.extend([output_path])
    print(cmd)
    subprocess.run(cmd, shell=False)


def change_file_name(temp_path, new_file_path):
    if os.path.exists(temp_path):
        if not os.path.exists(new_file_path):
            os.rename(temp_path, os.path.join(temp_path, new_file_path))


def delete_redundant_files(task_name, file_path, is_save):
    if is_save:
        if 'sdk' in task_name:
            api_version = utils.get_api_version(os.path.join(
                *[file_path, 'sdk_temp', 'ets', 'oh-uni-package.json']))
            sdk_file_name = f'{api_version}-{task_name}'
            sdk_temp_path = os.path.join(file_path, 'sdk_temp')
            change_file_name(sdk_temp_path, os.path.join(file_path, sdk_file_name))
        if 'dayu' in task_name:
            dayu_temp_path = os.path.join(file_path, 'dayu200_xts')
            change_file_name(dayu_temp_path, os.path.join(file_path, task_name))

    subdirs_and_files = [subdir_or_file for subdir_or_file in os.listdir(file_path)]
    for subdir_or_file in subdirs_and_files:
        if not subdir_or_file[0].isdigit():
            path = os.path.join(file_path, subdir_or_file)
            if os.path.isdir(path):
                shutil.rmtree(path)
            elif os.path.isfile(path):
                os.remove(path)


def get_the_unzip_file(task_name, download_url, path):
    download_name = utils.get_remote_download_name(task_name)
    download_temp_file = os.path.join(path, download_name)
    if not os.path.exists(path):
        os.mkdir(path)
    print(f'download {task_name} from {download_url}, please wait!!!')
    success = utils.retry_after_download_failed(download_url, download_temp_file, download_name)
    if not success:
        return False
    if utils.check_gzip_file(download_temp_file):
        with tarfile.open(download_temp_file, 'r:gz') as tar:
            print(f'Unpacking {download_temp_file}')
            if 'dayu' in task_name:
                path = os.path.join(path, 'dayu200_xts')
            tar.extractall(path)
            print(f'Decompression {download_temp_file} completed')
    elif utils.check_zip_file(download_temp_file):
        with zipfile.ZipFile(download_temp_file, 'r') as zip_file:
            print(f'Unpacking {download_temp_file}')
            zip_file.extractall(path)
            print(f'Decompression {download_temp_file} completed')
    else:
        print('The downloaded file is not a valid gzip or zip file.')

    if 'sdk' in task_name:
        unpack_sdk_file(path)

    return True


def unpack_sdk_file(path):
    sdk_zip_path_list = [path, 'ohos-sdk', 'windows']
    if utils.is_mac():
        sdk_zip_path_list = [path, 'sdk',
                             'packages', 'ohos-sdk', 'darwin']
    sdk_floder = os.path.join(path, 'sdk_temp')
    sdk_zip_path = os.path.join(*sdk_zip_path_list)
    for item in os.listdir(sdk_zip_path):
        if item != '.DS_Store':
            print(f'Unpacking {item}')
            with zipfile.ZipFile(os.path.join(sdk_zip_path, item)) as zip_file:
                zip_file.extractall(os.path.join(sdk_floder))
            print(f'Decompression {item} completed')


def get_output_path_list(task_name):
    configs = utils.parse_configs()
    download_list = configs['download_list']
    for item in download_list:
        if item['name'] in task_name:
            output_path_list = item['output_path_list']
    return output_path_list


def write_download_url_to_txt(task_name, download_url):
    download_url_txt = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                    'download_url.txt')
    with open(download_url_txt, 'a+') as file:
        file.write(f'{task_name}, {download_url}\n')


def get_the_image(task_name, download_url, image_date, download_save_path, output_path_list, is_save):
    if download_url == '':
        download_url = utils.get_download_url(task_name, image_date)
        # only save the download url you need for daily test
        write_download_url_to_txt(task_name, download_url)

    file_name = utils.parse_file_name(download_url)
    if output_path_list is None:
        output_path_list = get_output_path_list(file_name)
    print(output_path_list)
    success = get_the_unzip_file(file_name, download_url, download_save_path)
    if not success:
        print(f'get {task_name} unzip file failed')
        return

    copy_to_output_path(file_name, download_save_path, output_path_list)
    delete_redundant_files(file_name, download_save_path, is_save)


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--sdkUrl', type=str, dest='sdk_url', nargs='?', const='', default=None,
                        help='specify which sdk you want to download')
    parser.add_argument('--dayuUrl', type=str, dest='dayu_url', nargs='?', const='', default=None,
                        help='specify which dayu200 you want to download')
    parser.add_argument('--sdkDate', type=str, dest='sdk_date', default=None,
                        help='specify which day you want to download the sdk')
    parser.add_argument('--dayuDate', type=str, dest='dayu_date', default=None,
                        help='specify which day you want to download the dayu')
    parser.add_argument('--sdkPath', type=str, dest='sdk_path', default=None,
                        nargs='+',
                        help='specify where you want to store the sdk')
    parser.add_argument('--dayuPath', type=str, dest='dayu_path', default=None,
                        nargs='+',
                        help='specify where you want to store the dayu200')
    parser.add_argument('--continueTest', dest='continue_test', action='store_true', default=False,
                        help='input this parameter to continue test')
    return parser.parse_args()


def main():
    arguments = parse_args()
    configs = utils.parse_configs()
    is_save = configs.get('is_save')
    download_save_path = configs.get('download_save_path')

    if arguments.sdk_url is not None:
        get_the_image('sdk', arguments.sdk_url, arguments.sdk_date, download_save_path, arguments.sdk_path, is_save)

    if arguments.dayu_url is not None:
        get_the_image('dayu', arguments.dayu_url, arguments.dayu_date, download_save_path, arguments.dayu_path, is_save)

    if arguments.continue_test:
        cmd = ['python', '../entry', '--skipSdk', '--skipDayu']
        subprocess.run(cmd)


if __name__ == '__main__':
    main()
