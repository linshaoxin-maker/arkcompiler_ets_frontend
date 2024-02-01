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
import asyncio
import datetime
import gzip
import httpx
import json
import os
import re
import requests
import shutil
import stat
import sys
import time
import tqdm
import yaml
import zipfile

from urllib.parse import urlparse, unquote


def is_windows():
    return sys.platform == 'win32' or sys.platform == 'cygwin'


def is_mac():
    return sys.platform == 'darwin'


def is_linux():
    return sys.platform == 'linux'


def get_time_string():
    return time.strftime('%Y%m%d-%H%M%S')


def set_to_end_of_day(date_time):
    end_of_day = date_time.replace(hour=23, minute=59, second=59)
    return end_of_day


def get_encoding():
    if is_windows():
        return 'utf-8'
    else:
        return sys.getfilesystemencoding()


def parse_configs():
    config_file_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'config.yaml')
    with open(config_file_path, 'r', encoding='utf-8') as config_file:
        configs = yaml.safe_load(config_file)
    return configs


def get_output_path_list(task_name):
    configs = parse_configs()
    image_name = 'sdk' if 'sdk' in task_name else 'dayu200'
    output_path_list = configs[image_name][1]['output_path_list']

    return output_path_list


async def copy_image_async(file_path, output_path):
    if os.path.exists(output_path):
        loop = asyncio.get_event_loop()
        await loop.run_in_executor(None, shutil.rmtree, output_path)
    print(f'Copy from {file_path} to {output_path}, please wait!!!')
    loop = asyncio.get_event_loop()
    await loop.run_in_executor(None, shutil.copytree, file_path, output_path)


def get_tool(tar_name, url, output_path):
    print(f"Getting {tar_name} from {url}")
    r = requests.get(url, stream=True)
    total = int(r.headers.get('content-length'), 0)
    flags = os.O_WRONLY | os.O_CREAT
    modes = stat.S_IWUSR | stat.S_IRUSR
    with os.fdopen(os.open(f"{output_path}.zip", flags, modes), "wb") as f, tqdm(
            desc=f"{tar_name}",
            total=total,
            unit='iB',
            unit_scale=True,
            unit_divisor=1024,
    ) as bar:
        for byte in r.iter_content(chunk_size=1024):
            size = f.write(byte)
            bar.update(size)
    with zipfile.ZipFile(f"{output_path}.zip", 'r') as zfile:
        zfile.extractall(path=f"{output_path}")


def convert_to_datetime(date_str):
    date_format = '%Y%m%d%H%M%S'
    date_time = datetime.datetime.strptime(date_str, date_format)
    return date_time


def get_download_url(task_name, image_date):
    if image_date is None:
        image_date = datetime.datetime.now().strftime('%Y%m%d%H%M%S')
    else:
        image_date = image_date + '235959'
    last_hour = (convert_to_datetime(image_date) +
                 datetime.timedelta(hours=-24)).strftime('%Y%m%d%H%M%S')
    url = 'http://ci.openharmony.cn/api/daily_build/build/tasks'
    downnload_job = {
        'pageNum': 1,
        'pageSize': 1000,
        'startTime': '',
        'endTime': '',
        'projectName': 'openharmony',
        'branch': 'master',
        'component': '',
        'deviceLevel': '',
        'hardwareBoard': '',
        'buildStatus': '',
        'buildFailReason': '',
        'testResult': '',
    }
    downnload_job['startTime'] = str(last_hour)
    downnload_job['endTime'] = str(image_date)
    post_result = requests.post(url, json=downnload_job)
    post_data = json.loads(post_result.text)
    download_url_suffix = ''
    for ohos_sdk_list in post_data['data']['dailyBuildVos']:
        try:
            if get_remote_download_name(task_name) in ohos_sdk_list['obsPath']:
                download_url_suffix = ohos_sdk_list['obsPath']
                break
        except BaseException as err:
            print(err)
    if download_url_suffix == '':
        return None
    download_url = 'http://download.ci.openharmony.cn/' + download_url_suffix
    return download_url


def retry_after_download_failed(download_url, temp_file, temp_file_name, max_retries=3):
    for i in range(max_retries):
        try:
            download(download_url, temp_file, temp_file_name)
            return True
        except Exception as e:
            print(f"download failed! retrying... ({i + 1}/{max_retries})")
            time.sleep(2)
    return False


def download_progress_bar(response, temp, temp_file_name):
    total_length = int(response.headers.get("content-length"))
    with tqdm.tqdm(total=total_length, unit="B", unit_scale=True) as pbar:
        pbar.set_description(temp_file_name)
        chunk_sum = 0
        count = 0
        for chunk in response.iter_bytes():
            temp.write(chunk)
            chunk_sum += len(chunk)
            percentage = chunk_sum / total_length * 100
            while str(percentage).startswith(str(count)):
                count += 1
            pbar.update(len(chunk))


def download(download_url, temp_file, temp_file_name):
    with httpx.stream('GET', download_url) as response:
        with open(temp_file, "wb") as temp:
            download_progress_bar(response, temp, temp_file_name)
    return True


def check_gzip_file(file_path):
    try:
        with gzip.open(file_path, 'rb') as gzfile:
            gzfile.read(1)
    except Exception as e:
        print(e)
        return False
    return True


def check_zip_file(file_path):
    try:
        if zipfile.is_zipfile(file_path):
            with zipfile.ZipFile(file_path, 'r') as zip_file:
                pass
        else:
            return False
    except Exception as e:
        print(e)
        return False
    return True


def get_remote_download_name(task_name):
    if is_windows():
        if 'sdk' in task_name:
            return 'ohos-sdk-full.tar.gz'
        return 'dayu200.tar.gz'
    elif is_mac():
        if 'sdk' in task_name:
            return 'L2-MAC-SDK-FULL.tar.gz'
    else:
        print('Unsuport platform to get sdk from daily build')
        return ''


def get_api_version(json_path):
    with open(json_path, 'r') as uni:
        uni_cont = uni.read()
        uni_data = json.loads(uni_cont)
        api_version = uni_data['apiVersion']
    return api_version


def add_executable_permission(file_path):
    current_mode = os.stat(file_path).st_mode
    new_mode = current_mode | 0o111
    os.chmod(file_path, new_mode)


def parse_file_name(url):
    parsed_url = urlparse(url)
    path = unquote(parsed_url.path)
    file_name = os.path.basename(path)

    pattern = r'\d{8}'
    match = re.search(pattern, file_name)
    if match:
        datetime_info = match.group()
        if 'sdk' in file_name:
            return datetime_info + '-sdk'
        if 'dayu' in file_name:
            return datetime_info + '-dayu200'
    else:
        return file_name

