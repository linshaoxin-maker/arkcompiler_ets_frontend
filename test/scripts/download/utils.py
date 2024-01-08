import gzip
import os
import sys
import logging
import datetime
import zipfile
from urllib.parse import urlparse, unquote
import time
import json
import stat

import httpx
import requests
import tqdm
from pywinauto.application import Application


def is_windows():
    return sys.platform == 'win32' or sys.platform == 'cygwin'


def is_mac():
    return sys.platform == 'darwin'


def is_linux():
    return sys.platform == 'linux'


def get_time_string():
    return time.strftime('%Y%m%d-%H%M%S')


def get_encoding():
    if is_windows():
        return 'utf-8'
    else:
        return sys.getfilesystemencoding()


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


def get_download_url(task_name):
    now_time = datetime.datetime.now().strftime('%Y%m%d%H%M%S')
    last_hour = (datetime.datetime.now() +
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
    downnload_job['endTime'] = str(now_time)
    post_result = requests.post(url, json=downnload_job)
    post_data = json.loads(post_result.text)
    sdk_url_suffix = ''
    for ohos_sdk_list in post_data['data']['dailyBuildVos']:
        try:
            if get_remote_download_name(task_name) in ohos_sdk_list['obsPath']:
                sdk_url_suffix = ohos_sdk_list['obsPath']
                break
        except BaseException as err:
            logging.error(err)
    download_url = 'http://download.ci.openharmony.cn/' + sdk_url_suffix
    return download_url


def download(download_url, temp_file, temp_file_name):
    with httpx.stream('GET', download_url) as response:
        with open(temp_file, "wb") as temp:
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
                        if str(percentage).startswith('100'):
                            logging.info(f'{temp_file_name} Download Complete {percentage: .1f}%')
                            break
                        else:
                            logging.info(f'{temp_file_name} Downloading... {percentage: .1f}%')
                        count += 1
                    pbar.update(len(chunk))


def end_burn(dlg):
    timeout = 300
    while True:
        if timeout < 0:
            return
        mode = dlg.window(control_type="Tab").window_text()
        if mode == 'Found One MASKROM Device':
            dlg.Button16.click()
            print("image burnning finished")
            return
        else:
            print("please wait for a while...")
            time.sleep(5)
            timeout -= 5


def auto_burn():
    app = Application(backend='uia').start('RKDevTool.exe')
    dlg = app.top_window()

    while True:
        mode = dlg.window(control_type="Tab").window_text()
        if mode == 'Found One LOADER Device':
            print('start burning')
            dlg.window(title="Run").click()
            time.sleep(100)
            end_burn(dlg)
            return
        else:
            time.sleep(1)


def check_gzip_file(file_path):
    try:
        with gzip.open(file_path, 'rb') as gzfile:
            gzfile.read(1)
    except Exception as e:
        logging.exception(e)
        return False
    return True


def get_remote_download_name(task_name):
    if is_windows():
        if task_name == 'sdk':
            return 'ohos-sdk-full.tar.gz'
        if task_name == 'dayu200':
            return 'dayu200.tar.gz'
    elif is_mac():
        if task_name == 'sdk':
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
    return file_name


def close_arkts_code_linter():
    ets_checker_path = 'D:\\enviorment\\SDK\\openHarmony_SDK\\11\\ets\\build-tools\\ets-loader\\lib\\ets_checker.js'
    with open(ets_checker_path, 'r+') as modified_file:
        content = modified_file.read()
        content = content.replace(
            'return _main.partialUpdateConfig.executeArkTSLinter?_main.partialUpdateConfig.standardArkTSLinter&&isStandardMode()?_do_arkTS_linter.ArkTSLinterMode.STANDARD_MODE:_do_arkTS_linter.ArkTSLinterMode.COMPATIBLE_MODE:_do_arkTS_linter.ArkTSLinterMode.NOT_USE',
            'return _do_arkTS_linter.ArkTSLinterMode.COMPATIBLE_MODE')
        modified_file.seek(0)
        modified_file.write(content)
        modified_file.truncate()
