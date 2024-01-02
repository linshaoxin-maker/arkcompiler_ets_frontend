import argparse
import os
import shutil
import sys
import tarfile
import zipfile
import subprocess

import yaml


import utils


configs = {}
arguments = {}


class downloadTask:
    def __init__(self):
        self.name = ''
        self.path = ''
        self.output_path = ''


def parse_configs():
    config_file_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'config.yaml')
    with open(config_file_path, 'r', encoding='utf-8') as config_file:
        global configs
        configs = yaml.safe_load(config_file)


def clean_log():
    output_log_dic = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'result')
    if os.path.exists(output_log_dic):
        shutil.rmtree(output_log_dic)


def create_download_task():
    task_list = []
    download_list = configs.get('download_list')
    for download_task in download_list:
        task = downloadTask()
        task.name = download_task['name']
        task.path = download_task['path']
        task.output_path = download_task['output_path']
        task_list.append(task)

    return task_list


def check_deveco_dev():
    if utils.is_linux():
        return False

    java_path = os.path.join(configs.get('deveco_path'), 'jbr')
    if not os.path.exists(java_path):
        print("Java not found!")
        return False

    if not os.path.exists(configs.get('node_js_path')):
        print("Node js not found!")
        return False

    return True


def check_rkDevTool():
    rKDevTool_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), '../auto_xts_test/RKDevTool')
    if not os.path.exists(rKDevTool_path):
        url = configs.get('RKdevtool_download_path')
        output_path = configs.get('RKdevtool_output_path')
        utils.get_tool('RKDevTool.zip', url, output_path)

    rKDevTool_exe_path = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                      '../auto_xts_test/RKDevTool/RKDevTool.exe')
    if not rKDevTool_exe_path:
        return False

    return True


def check_pictures_reference():
    pictures_reference_path = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                           '../sdk_test/pictures_reference')
    if not os.path.exists(pictures_reference_path):
        url = configs.get('pictures_download_path')
        output_path = configs.get('pictures_output_path')
        utils.get_tool('pictures_references.zip', url, output_path)
        if not os.path.exists(pictures_reference_path):
            return False

    return True


def prepare_test_dev():
    clean_log()
    prepared = check_deveco_dev()
    prepared = prepared and check_rkDevTool() and check_pictures_reference()
    return prepared


def download_simple(download_url):
    download_name = utils.parse_file_name(download_url)
    download_path = configs.get('download_path')
    download_temp_file = os.path.join(download_path, download_name)
    if not os.path.exists(download_path):
        os.mkdir(download_path)
    utils.download(download_url, download_temp_file, download_name)

    # 判断文件名后缀， 如果是gz则对文件进行解压
    file_extension = os.path.splitext(download_name)[-1]
    if file_extension == '.gz':
        with tarfile.open(download_temp_file, 'r:gz') as tar:
            print(f'Unpacking {download_temp_file}')
            tar.extractall(download_path)
            print(f'Decompression {download_temp_file} completed')


def download_zip_file(task_name, download_url, path):
    # 读取文件路径
    print(path)
    temp_floder = path + '_temp'
    download_name = utils.get_remote_download_name(task_name)

    download_temp_file = os.path.join(temp_floder, download_name)
    if os.path.exists(temp_floder):
        shutil.rmtree(temp_floder)
    os.mkdir(temp_floder)
    # 开始下载任务
    print(f'download {task_name} from {download_url}, please wait!!!')
    utils.download(download_url, download_temp_file, download_name)

    # 校验是否正确获取到文件
    if not utils.check_gzip_file(download_temp_file):
        print('The downloaded file is not a valid gzip file.')
        return '', ''

    # 解压文件
    with tarfile.open(download_temp_file, 'r:gz') as tar:
        print(f'Unpacking {download_temp_file}')
        tar.extractall(temp_floder)
        print(f'Decompression {download_temp_file} completed')

    if task_name == 'sdk':
        sdk_zip_path_list = [temp_floder, 'ohos-sdk', 'windows']
        if utils.is_mac():
            sdk_zip_path_list = [temp_floder, 'sdk',
                                 'packages', 'ohos-sdk', 'darwin']
        sdk_floder = os.path.join(temp_floder, 'SDK_TEMP')
        sdk_zip_path = os.path.join(*sdk_zip_path_list)
        for item in os.listdir(sdk_zip_path):
            if item != '.DS_Store':
                print(f'Unpacking {item}')
                with zipfile.ZipFile(os.path.join(sdk_zip_path, item)) as zip_file:
                    zip_file.extractall(os.path.join(sdk_floder))
                print(f'Decompression {item} completed')


def updata_to_output_path(task_name, path, output_path):
    if task_name == 'sdk':
        deveco_sdk_path = configs.get('deveco_sdk_path')
        temp_floder = deveco_sdk_path + '_temp'
        sdk_floder = os.path.join(temp_floder, 'SDK_TEMP')
        api_version = utils.get_api_version(os.path.join(
            *[sdk_floder, 'ets', 'oh-uni-package.json']))
        update_sdk_to_deveco(sdk_floder, api_version)
    else:
        if os.path.exists(output_path):
            shutil.rmtree(output_path)
        if os.path.exists(path):
            shutil.move(path, output_path)


def update_sdk_to_deveco(sdk_path, api_version):
    deveco_sdk_path = configs.get('deveco_sdk_path')
    deveco_sdk_version_path = os.path.join(deveco_sdk_path, api_version)
    for sdk_item in os.listdir(deveco_sdk_path):
        if sdk_item.startswith(f'{api_version}-'):
            shutil.rmtree(os.path.join(deveco_sdk_path, sdk_item))
    if os.path.exists(deveco_sdk_version_path):
        shutil.move(deveco_sdk_version_path,
                    deveco_sdk_version_path + '-' + utils.get_time_string())
    for item in os.listdir(sdk_path):
        if item != '.DS_Store':
            if utils.is_mac():
                if item == 'toolchains':
                    utils.add_executable_permission(
                        os.path.join(sdk_path, item, 'restool'))
                    utils.add_executable_permission(
                        os.path.join(sdk_path, item, 'ark_disasm'))
                elif item == 'ets':
                    utils.add_executable_permission(os.path.join(sdk_path, item, 'build-tools',
                                                                 'ets-loader', 'bin', 'ark', 'build-mac', 'bin',
                                                                 'es2abc'))
                    utils.add_executable_permission(os.path.join(sdk_path, item, 'build-tools',
                                                                 'ets-loader', 'bin', 'ark', 'build-mac', 'legacy_api8',
                                                                 'bin', 'js2abc'))
                elif item == 'js':
                    utils.add_executable_permission(os.path.join(sdk_path, item, 'build-tools',
                                                                 'ace-loader', 'bin', 'ark', 'build-mac', 'bin',
                                                                 'es2abc'))
                    utils.add_executable_permission(os.path.join(sdk_path, item, 'build-tools',
                                                                 'ace-loader', 'bin', 'ark', 'build-mac', 'legacy_api8',
                                                                 'bin', 'js2abc'))
            shutil.move(os.path.join(sdk_path, item),
                        os.path.join(deveco_sdk_version_path, item))
    # 关闭arkts语法规则校验
    utils.close_arkts_code_linter()


def burn_system_image():
    RKDevTool_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), '../auto_xts_test/RKDevTool')
    os.chdir(RKDevTool_path)
    cmd = 'hdc shell reboot bootloader'
    subprocess.run(cmd, shell=False)
    utils.auto_burn()
    print('burn_system_image_success')


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--downloadUrl', type=str, dest='download_url', default=None,
                        nargs='+',
                        help='specify what you want to download')
    return parser.parse_args()


if __name__ == '__main__':
    # 1. 读取配置 && 参数解析
    parse_configs()
    parse_args()
    arguments = parse_args()
    # 如果输入了参数（下载url)就会自动去下载文件到配置中指定的文件夹，并退出。
    if arguments.download_url is not None:
        for download_url in arguments.download_url:
            print(f'download url: {download_url}')
            download_simple(download_url)
        # 下载并解压完后退出程序
        sys.exit(1)

    # 2. 检查环境(deveco && 烧录工具)
    if not prepare_test_dev():
        # logging.error('The test environment is incomplete, please check')
        print("The test environment is incomplete, please check")
        sys.exit(1)

    # 3. 下载镜像
    if os.path.exists('download_url.txt'):
        os.remove('download_url.txt')
    download_task_list = create_download_task()
    with open('download_url.txt', 'a') as file:
        for task in download_task_list:
            print(task.name)
            # a. 获取下载的url地址
            download_url = utils.get_download_url(task.name)
            # b. 下载压缩包到指定目录 && 解压
            download_zip_file(task.name, download_url, task.path)
            # c. 覆盖到对应文件夹
            temp_file = task.path + '_temp'
            updata_to_output_path(task.name, temp_file, task.output_path)
            file.write(f'{task.name}, {download_url}\n')
        file.write('download all tasks successfully!!!')
    # 4. 烧录镜像到板子中
    burn_system_image()
    print('complete all tasks successfully')
