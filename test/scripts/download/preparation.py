import os
import shutil

from utils import get_tool, is_linux
from utils import parse_configs

configs = {}
arguments = {}


def clean_log():
    download_url_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'result')
    if os.path.exists(download_url_path):
        shutil.rmtree(download_url_path)


def check_deveco_dev():
    if is_linux():
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
        get_tool('RKDevTool.zip', url, output_path)

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
        get_tool('pictures_references.zip', url, output_path)
        if not os.path.exists(pictures_reference_path):
            return False

    return True


def prepare_test_dev():
    global configs
    configs = parse_configs()
    clean_log()
    sdk_prepared = check_deveco_dev() and check_pictures_reference
    xts_prepared = check_rkDevTool()
    prepared = sdk_prepared and xts_prepared
    return prepared


if __name__ == '__main__':
    prepare_test_dev()

