import logging
import os

from pathlib import Path

import yaml

test_logger = logging.getLogger("test_logger")
configs = {}


class HapModule:
    def __init__(self, name, inc_modify_file: list) -> None:
        self.name = name
        self.inc_modify_file = inc_modify_file


class PerformanceTestTask:
    def __init__(self, name, path, modules: list[HapModule]) -> None:
        self.name = name
        self.path = path
        self.modules = modules


def parse_configs(root_path: Path):
    config_yaml = root_path / "resources" / "config.yaml"
    with open(config_yaml, "r") as f:
        global configs
        configs = yaml.safe_load(f)


def create_performance_tasks():
    task_list = []
    projects = configs["projects"]

    for project in projects:
        if not os.path.exists(project["path"]):
            test_logger.error(f"{project['path']} not exists")
            continue
        hap_modules: dict = project["hap_modules"]
        modules = [
            HapModule(name, inc_modify_file["inc_modify_file"])
            for name, inc_modify_file in hap_modules.items()
        ]
        task = PerformanceTestTask(project["name"], project["path"], modules)
        task_list.append(task)
    return task_list


def process_options(root_path: Path):
    parse_configs(root_path)
    return create_performance_tasks()
