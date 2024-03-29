#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024 Huawei Device Co., Ltd.
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


import json
import threading
import time

import psutil


class MemoryMonitor:
    def __init__(self, process_name, interval=1.0):
        self.process_name = process_name
        self.report_path = ''
        self.interval = interval
        self.monitoring_active = False
        self.current_memory_usage = 0
        self.max_memory_usage = 0
        self.lock = threading.Lock()
        self.monitoring_thread = threading.Thread(target=self.monitor_memory, daemon=True)

    @staticmethod
    def write_data_to_report_json(memory_usage_data, report_path):
        with open(report_path, 'r+') as file:
            data = json.load(file)
            if 'memory_usage' in data:
                data['memory_usage'].update(memory_usage_data)
            else:
                data['memory_usage'] = memory_usage_data
            file.seek(0)
            json.dump(data, file, indent=4)
            file.truncate()

    def kill_process_before_monitor(self):
        pass

    def monitor_memory(self):
        while self.monitoring_active:
            try:
                self.get_current_memory_usage()
                with self.lock:
                    self.max_memory_usage = max(self.current_memory_usage, self.max_memory_usage)
            except Exception:
                print('Error monitoring memory usage')
            time.sleep(self.interval)

    def get_current_memory_usage(self):
        try:
            # There are multiple processes with the same name; take the one with the largest value.
            max_usage = 0
            for proc in psutil.process_iter(['name', 'memory_info']):
                if proc.info['name'] == self.process_name:
                    max_usage = max(max_usage, proc.info['memory_info'].rss)
            self.current_memory_usage = max_usage / 1024
        except Exception:
            print('Error retrieving memory usage')
            self.current_memory_usage = 0

    def collect_memory_usage(self, wait_time=120):
        # Wait two minutes and then go to obtain the static memory usage of node.exe
        time.sleep(wait_time)
        self.get_current_memory_usage()

        memory_usage_data = {
            f'{self.process_name}_max_memory_usage': self.max_memory_usage,
            f'{self.process_name}_static_memory_usage': self.current_memory_usage
        }
        self.write_data_to_report_json(memory_usage_data, self.report_path)

    def start(self):
        self.monitoring_active = True
        self.monitoring_thread.start()

    def stop(self, report_path):
        self.report_path = report_path
        self.collect_memory_usage()
        self.monitoring_active = False
        self.monitoring_thread.join()
