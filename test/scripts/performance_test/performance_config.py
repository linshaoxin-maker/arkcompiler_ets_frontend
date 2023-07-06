#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2023 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os


class IdeType():
    AS = 1
    DevEco = 2    


class Config():
    log_direct = "buildTestData"
    log_direct_data_format = "%Y-%m-%d-%H-%M-%S"
    temp_filename = "temp.txt"
    send_mail = True

    def __init__(self):
        # Default config settings for all projects, if it's not what you need, config them in application_configs
        self.cmd_prefix = r"hvigorw.bat"
        
        self.output_split = "_"
        self.ide_filename = ["AS", "DevEco"]
        self.debug_or_release = ["Debug", "Release"]
        self.build_type_of_log = ["full_then_incremental", "add_code_and_ui"]
        self.log_filename = ["sizeAll.csv", "sizeAvg.csv",
                             "timeAll.csv", "timeAvg.csv"]
        self.ide = IdeType.DevEco
        self.incremental_code_str = "let index = 5 + 6\n"
        self.incremental_code_start_pos = "let index = 5 + 6\n"
        self.incremental_code_end_pos = 'this.num = num'
        self.cmd_debug_suffix = r' --mode module -p product=default assembleHap --no-daemon'
        self.cmd_release_suffix = r' --mode project -p product=default assembleApp --no-daemon'
        self.debug_package_path = r'entry/build/default/outputs/default/entry-default-signed.hap'
        self.release_package_path = r'entry/build/default/outputs/default/app/entry-default.hap'
        self.incremental_code_path = r'entry/src/main/ets/pages/Index.ets'

        # build serveral times then calculate the average value
        self.build_times = 3
        # Do not build the project，use the test data if you need to debug the scripts
        self.developing_test_mode = False
        # set your node_js path, it should be the same to the setting in your IDE
        self.node_js_path = r"%s/nodejs" % os.environ['USERPROFILE']
        # Must set according environment
        self.jbr_path = r'xxx/DevEco Studio/jbr'

    # If Default config is not what you need, you can set here!
    application_configs = dict(
        [
            (    
                "FTB", dict
                    (
                        project_path=r"D:/FTB",
                    )
            ),
            (    
                "FDY", dict
                    (
                        project_path=r"D:/FDY",
                    )
            ),
            (    
                "FWX", dict
                    (
                        project_path=r"D:/FWX",
                    )
            ),
            (
                "HelloWorld", dict
                    (
                        # The following params must be set according you environment
                        project_path=r"D:/HelloWorld",
                        
                        # The following params is not neccessary to be modified
                        debug_package_path=r'entry/build/default/outputs/default/entry-default-unsigned.hap',
                        release_package_path=r'entry/build/default/outputs/default/app/entry-default.hap',
                        incremental_code_path=r'entry/src/main/ets/pages/Index.ets',
                        incremental_code_end_pos='build() {',
                        incremental_code_str="a: number=5 + 6\n",
                        incremental_code_start_pos="a: number=5 + 6\n",
                    )
            )
        ]
    )


def get_config(index):
    config = Config()
    res = config.application_configs.get(index)
    if not res:
        print("No key in config, please check: " + index)
        return res
    for k in res:
        setattr(config, k, res[k])
    return config


def get_html_prefix():
    return '<html><body><table width="100%" border=1 cellspacing=0 cellpadding=0 align="center">' + \
           '<tr><th bgcolor="SlateBlue"><font size="5">Daily Performance Test</font></th></tr></table>'


def get_html_suffix():
    return '<br><img src="cid:performance00"></br><br><img src="cid:performance01"></br>' + \
           '<br><img src="cid:performance10"></br><br><img src="cid:performance11"></br></body></html>'


class BuildMode():
    DEBUG = 0
    RELEASE = 1


class BuildType():
    FULL = 0
    INCREMENTAL = 1


class MailPicConfig():
    mail_data_path = os.path.join(
        os.path.dirname(os.path.abspath(__file__)),
        'mail_data',
    )

    html_file_path = os.path.join(
        mail_data_path,
        "email_msg.html"
    )

    attach_path = os.path.join(
        mail_data_path,
        "performance_logs.zip"
    )

    # Count of days which will be add into the email picture
    mail_pic_table_name = {
        BuildMode.DEBUG: {
            BuildType.FULL: os.path.join(mail_data_path, 'debug_full_time.csv'),
            BuildType.INCREMENTAL: os.path.join(mail_data_path, 'debug_incremental_time.csv')
        },
        BuildMode.RELEASE:{
            BuildType.FULL: os.path.join(mail_data_path, 'release_full_time.csv'),
            BuildType.INCREMENTAL: os.path.join(mail_data_path, 'release_incremental_time.csv')
        }
    }
    
    mail_pic_name = {
        BuildMode.DEBUG: {
            BuildType.FULL:os.path.join(mail_data_path, 'debug_full_time.jpg'),
            BuildType.INCREMENTAL: os.path.join(mail_data_path, 'debug_incremental_time.jpg')
        },
        BuildMode.RELEASE:{
            BuildType.FULL: os.path.join(mail_data_path, 'release_full_time.jpg'),
            BuildType.INCREMENTAL: os.path.join(mail_data_path, 'release_incremental_time.jpg')
        }
    }
    
    mail_pic_table_lable = {
        BuildMode.DEBUG: {
            BuildType.FULL: 'Debug Full Build',
            BuildType.INCREMENTAL: 'Debug Incremental Build'
        },
        BuildMode.RELEASE:{
            BuildType.FULL: 'Release Full Build',
            BuildType.INCREMENTAL: 'Release Incremental Time'
        }
    }