import argparse
import os
import shutil
import subprocess
import tarfile
import zipfile

import utils
from preparation import prepare_test_dev


configs = {}
arguments = {}


class downloadTask:
    def __init__(self):
        self.name = ''
        self.path = ''
        self.output_path = ''


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


def get_download_image_simple(download_url):
    download_name = utils.parse_file_name(download_url)
    download_save_path = configs.get('download_save_path')
    task_name = 'dayu200'
    if 'sdk' in download_name:
        task_name = 'sdk'
    download_zip_file(task_name, download_url, download_save_path, download_name)

    file_path = download_save_path + '_temp'
    zip_file_path = os.path.join(file_path, download_name)
    # todo
    utils.before_update_sdk()

    output_path_list = []
    if arguments.output_path is not None:
        output_path_list = arguments.output_path
    else:
        for item in configs['output_path_list']:
            if item.get('name') == task_name:
                output_path_list = item.get('output_path', [])
                break

    for output_path in output_path_list:
        if task_name == 'sdk':
            sdk_temp_file = os.path.join(file_path, 'SDK_TEMP')
            update_sdk_to_output_path(sdk_temp_file, output_path)
            # todo
            utils.after_update_sdk(output_path)
        else:
            if os.path.exists(output_path):
                shutil.rmtree(output_path)
            if os.path.exists(zip_file_path):
                os.remove(zip_file_path)
            if os.path.exists(file_path):
                shutil.copytree(file_path, output_path)
    shutil.rmtree(file_path)


def update_sdk_to_output_path(file_path, output_path):
    api_version = utils.get_api_version(os.path.join(
        *[file_path, 'ets', 'oh-uni-package.json']))
    output_path = os.path.join(output_path, api_version)
    if os.path.exists(output_path):
        shutil.rmtree(output_path)
    shutil.copytree(file_path, output_path)


def download_zip_file(task_name, download_url, path, download_name=''):
    temp_floder = path + '_temp'
    if download_name == '':
        download_name = utils.get_remote_download_name(task_name)
    download_temp_file = os.path.join(temp_floder, download_name)
    if os.path.exists(temp_floder):
        shutil.rmtree(temp_floder)
    os.mkdir(temp_floder)
    print(f'download {task_name} from {download_url}, please wait!!!')
    success = utils.download(download_url, download_temp_file, download_name)
    if not success:
        return False
    if not utils.check_gzip_file(download_temp_file):
        print('The downloaded file is not a valid gzip file.')
        return False

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
    return True


def update_to_output_path(task_name, path, output_path, temp_floder=''):
    if task_name == 'sdk':
        if temp_floder == '':
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


def update_sdk_to_deveco(sdk_path, api_version, deveco_sdk_path=''):
    if deveco_sdk_path == '':
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
                                                           'ets-loader', 'bin', 'ark', 'build-mac', 'bin', 'es2abc'))
                    utils.add_executable_permission(os.path.join(sdk_path, item, 'build-tools',
                                                           'ets-loader', 'bin', 'ark', 'build-mac', 'legacy_api8',
                                                           'bin', 'js2abc'))
                elif item == 'js':
                    utils.add_executable_permission(os.path.join(sdk_path, item, 'build-tools',
                                                           'ace-loader', 'bin', 'ark', 'build-mac', 'bin', 'es2abc'))
                    utils.add_executable_permission(os.path.join(sdk_path, item, 'build-tools',
                                                           'ace-loader', 'bin', 'ark', 'build-mac', 'legacy_api8',
                                                           'bin', 'js2abc'))
            shutil.move(os.path.join(sdk_path, item),
                        os.path.join(deveco_sdk_version_path, item))
    # todo
    utils.after_update_sdk_to_deveco()


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
                        help='specify what you want to download')
    parser.add_argument('--outputPath', type=str, dest='output_path', default=None,
                        nargs='+',
                        help='specify where you want to store the file')
    return parser.parse_args()


def get_the_latest_image():
    download_url_txt = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                    'download_url.txt')
    if os.path.exists(download_url_txt):
        os.remove(download_url_txt)
    download_task_list = create_download_task()
    with open(download_url_txt, 'a') as file:
        for task in download_task_list:
            download_url = utils.get_download_url(task.name)
            success = download_zip_file(task.name, download_url, task.path)
            if not success:
                return
            temp_file = task.path + '_temp'
            update_to_output_path(task.name, temp_file, task.output_path)
            file.write(f'{task.name}, {download_url}\n')
        file.write('all tasks download successfully!!!')
    burn_system_image()
    print('complete all tasks successfully')


def main():
    if not prepare_test_dev():
        return

    if arguments.download_url is not None:
        get_download_image_simple(arguments.download_url)
    else:
        get_the_latest_image()


if __name__ == '__main__':
    configs = utils.parse_configs()
    arguments = parse_args()
    main()
