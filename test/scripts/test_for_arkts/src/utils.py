import json
import logging
import logging.handlers
import re
import shutil
import subprocess
import tarfile
import tempfile
import zipfile
from datetime import datetime, timedelta
from pathlib import Path
from queue import Queue

import requests

logger = logging.getLogger("test_logger")
log_queue = Queue()
queue_listener = None
config = {}


def pack_result(root_dir: Path) -> Path | None:
    result_dir = root_dir / "result"
    log_dir = root_dir / "logs"
    today = datetime.now().strftime("%Y_%m_%d")
    log_file_name = f"{today}_test.log"
    log_file = log_dir / log_file_name

    if not result_dir.exists():
        logger.error(f"Directory {result_dir} does not exist, skip packing.")
        return None

    if log_file.exists() and log_file.is_file():
        shutil.copy2(log_file, result_dir / log_file_name)

    archive_name = f"{today}_ArkTS_{result_dir.name}.zip"
    archive_path = result_dir.parent / archive_name

    with zipfile.ZipFile(archive_path, "w", zipfile.ZIP_DEFLATED) as zipf:
        for file in result_dir.glob("**/*"):
            zipf.write(file, file.relative_to(result_dir.parent))

    logger.info(f"Directory {result_dir} has been packed into {archive_path}")
    return archive_path


def clean(root_dir: Path, archive_path: Path | None) -> None:
    result_path = root_dir / "result"
    extern_path = root_dir / "extern"
    if result_path.exists() and result_path.is_dir():
        shutil.rmtree(result_path)
        logger.info(f"Deleted directory: {result_path}")

    if extern_path.exists() and extern_path.is_dir():
        shutil.rmtree(extern_path)
        logger.info(f"Deleted directory: {extern_path}")

    if archive_path is not None and archive_path.exists() and archive_path.is_file():
        archive_path.unlink()
        logger.info(f"Deleted file: {archive_path}")


def check_environment() -> tuple[bool, str | None]:
    # Check if running in WSL
    try:
        with open("/proc/version") as f:
            version_info = f.read()
        if "microsoft" not in version_info:
            logger.error("This script is not running in WSL.")
            return False, None
    except Exception as e:
        logger.error(f"Failed to check WSL environment: {e}")
        return False, None

    # Check if powershell.exe is available
    try:
        powershell_exe = subprocess.run(
            ["which", "powershell.exe"], capture_output=True, text=True, check=True
        ).stdout.strip()
        if not powershell_exe:
            logger.error("powershell.exe not found.")
            return False, None
    except subprocess.CalledProcessError as e:
        logger.error(f"Failed to find powershell.exe: {e}")
        return False, None

    return True, powershell_exe


def process_config(root_dir: Path) -> None:
    config_path = root_dir / "config.json"
    if config_path.exists() and config_path.is_file():
        with open(config_path) as f:
            global config
            config = json.load(f)


def setup_logger(root_dir: Path) -> None:
    global queue_listener
    log_dir = root_dir / "logs"
    log_dir.mkdir(parents=True, exist_ok=True)
    today = datetime.now().strftime("%Y_%m_%d")
    log_file = log_dir / f"{today}_test.log"

    file_handler = logging.FileHandler(log_file)
    file_handler.setLevel(logging.DEBUG)
    file_formatter = logging.Formatter(
        "[%(asctime)s] [%(levelname)s] %(funcName)s: %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
    )
    file_handler.setFormatter(file_formatter)

    console_handler = logging.StreamHandler()
    console_handler.setLevel(logging.DEBUG)
    console_formatter = logging.Formatter(
        "[%(asctime)s] [%(levelname)s] %(funcName)s: %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
    )
    console_handler.setFormatter(console_formatter)

    queue_handler = logging.handlers.QueueHandler(log_queue)

    logger.setLevel(logging.DEBUG)
    # logger.addHandler(file_handler)
    # logger.addHandler(console_handler)
    logger.addHandler(queue_handler)
    queue_listener = logging.handlers.QueueListener(
        log_queue, file_handler, console_handler
    )
    queue_listener.start()


def stop_logger() -> None:
    global queue_listener
    if queue_listener is not None:
        queue_listener.stop()
        queue_listener = None


def ci_build_request(days: int = 0) -> dict | None:
    url = "https://ci.openharmony.cn/api/daily_build/build"
    headers = {
        "Accept": "application/json, text/plain, */*",
        "Accept-Encoding": "gzip, deflate, br, zstd",
        "Accept-Language": "zh-CN,zh;q=0.8",
        "Access-Control-Allow-Credentials": "true",
        "Access-Control-Allow-Methods": "POST, GET, PUT, OPTIONS, DELETE, PATCH",
        "Access-Control-Allow-Origin": "*",
        "Origin": "https://ci.openharmony.cn",
        "Referer": "https://ci.openharmony.cn/workbench/cicd/dailybuild/dailylist",
        "Content-Type": "application/json",
    }

    now = datetime.now()
    if days == 0:
        start_time = now.strftime("%Y%m%d000000")
        end_time = now.strftime("%Y%m%d%H%M%S")
    else:
        target_date = now - timedelta(days=days)
        start_time = target_date.strftime("%Y%m%d000000")
        end_time = target_date.strftime("%Y%m%d235959")
    body = {
        "projectName": "openharmony",
        "branch": "master",
        "pageNum": 1,
        "pageSize": 10,
        "deviceLevel": "",
        "components": [],
        "type": "0",
        "startTime": f"{start_time}",
        "endTime": f"{end_time}",
        "sortType": "",
        "sortField": "",
    }

    session = requests.Session()
    session.headers.update(headers)

    response = session.post(url, json=body)

    if response.status_code == 200:
        logger.info("request success")
        return response.json()
    logger.error(f"request failed, status code: {response.status_code}")
    return None


def pr_ci_request(ci_id: str) -> dict | None:
    url = f"https://ci.openharmony.cn/api/codecheckAccess/ci-portal/v1/event/{ci_id}"
    headers = {
        "Accept": "application/json, text/plain, */*",
        "Accept-Encoding": "gzip, deflate, br, zstd",
        "Accept-Language": "zh-CN,zh;q=0.8",
        "Access-Control-Allow-Credentials": "true",
        "Access-Control-Allow-Methods": "POST, GET, PUT, OPTIONS, DELETE, PATCH",
        "Access-Control-Allow-Origin": "*",
        "Referer": f"https://ci.openharmony.cn/workbench/cicd/detail/{ci_id}/runlist",
        "Content-Type": "application/json",
    }

    session = requests.Session()
    session.headers.update(headers)

    response = session.get(url)

    if response.status_code == 200:
        logger.info("request success")
        return response.json()
    logger.error(f"request failed, status code: {response.status_code}")
    return None


def find_sdk_by_ci_id(ci_id: str) -> tuple[str | None, datetime | None]:
    res = pr_ci_request(ci_id)
    if res is not None:
        build_list = res["data"]["builds"]
        for build in build_list:
            if build["buildTarget"] != "ohos-sdk":
                continue
            if build["debug"]["result"] != "success":
                return None
            artifacts = build["debug"]["Artifacts"]
            match = re.match("([^/]+)/([^/]+)/([^/]+)", artifacts)
            part1, part2, part3 = match.groups()
            download_url = f"https://cidownload.openharmony.cn/{part1}/{part2}/{part3}/version/{part1}-{part2}-{part3}-version-{part2}.zip"
            with tempfile.NamedTemporaryFile(
                delete=False, suffix=".tar.gz"
            ) as tmp_file:
                download_success = download_file(download_url, tmp_file.name)
                if download_success:
                    print(f"File {download_url} has been downloaded to {tmp_file.name}")
                    return tmp_file.name
            break

    return None

def download_file(url: str, dest_path: str) -> bool:
    logger.info(f"Downloading file {url} to {dest_path}")
    response = requests.get(url, stream=True)
    if response.status_code == 200:
        with open(dest_path, "wb") as file:
            for chunk in response.iter_content(chunk_size=8192):
                if chunk:
                    file.write(chunk)
        logger.info(f"File {url} has been downloaded to {dest_path}")
        return True
    logger.error(f"Failed to download file {url}, status code: {response.status_code}")
    return False


def extract_sdk(tar_path: Path, dest_dir: Path) -> None:
    bak_path = dest_dir.parent / (dest_dir.name + "_bak")
    dest_dir.rename(bak_path)
    logger.info(f"Directory {dest_dir} has been renamed to {bak_path}")
    dest_dir.mkdir(parents=True, exist_ok=True)

    with tarfile.open(tar_path, "r:gz") as tar:
        tar.extractall(path=dest_dir)
    logger.info(f"SDK has been extracted to {dest_dir}")
    windows_path = dest_dir / "windows"
    if windows_path.exists() and windows_path.is_dir():
        shutil.rmtree(windows_path)
    manifest_xml = dest_dir / "manifest_tag.xml"
    if manifest_xml.exists() and manifest_xml.is_file():
        manifest_xml.unlink()
    linux_path = dest_dir / "linux"
    for file in linux_path.glob("*.zip"):
        shutil.move(file, dest_dir)
        logger.info(f"File {file} has been moved to {dest_dir}")
    shutil.rmtree(linux_path)
    for file in dest_dir.iterdir():
        if file.name.endswith(".zip"):
            logger.info(file.absolute())
            subprocess.run(["unzip", "-q", f"{file.name}"], cwd=dest_dir, check=True)
            logger.info(f"File {file} has been unzipped to {dest_dir}")
            file.unlink()
            logger.info(f"File {file} has been deleted")


# 默认找当天的构建，可以传递参数 days 找过去的构建
def find_sdk(days: int = 0) -> tuple[str | None, datetime | None]:
    for i in range(0, days + 1):
        res = ci_build_request(i)
        generated_time = None
        if res is not None:
            data_list = res["data"]["builds"]["dataList"]
            for data in data_list:
                if data["component"] == "ohos-sdk-full" and data["obsPath"]:
                    latest_time = datetime.strptime(data["latestTime"], "%Y%m%d%H%M%S")
                    generated_time = latest_time + timedelta(seconds=int(data["duration"]))
                    download_url = f"https://cidownload.openharmony.cn/{data['obsPath']}"
                    with tempfile.NamedTemporaryFile(
                        delete=False, suffix=".tar.gz"
                    ) as tmp_file:
                        download_success = download_file(download_url, tmp_file.name)
                        if download_success:
                            logger.info("Full SDK found")
                            return tmp_file.name, generated_time
                    break
    logger.error("Full SDK not found")
    return None, None


def change_api_version(sdk_path: Path) -> None:
    logger.info("Changing API version")
    for component_dir in sdk_path.iterdir():
        package_json = component_dir / "oh-uni-package.json"
        if package_json.exists() and package_json.is_file():
            with open(package_json) as f:
                package_data = json.load(f)
            package_data["apiVersion"] = 12
            with open(package_json, "w") as f:
                json.dump(package_data, f, indent=2)
            logger.info(f"API version of {component_dir} has been changed to 12")


def add_executable_permission(file_path: Path) -> None:
    current_mode = file_path.stat().st_mode
    new_mode = current_mode | 0o111
    file_path.chmod(new_mode)


def change_permission(sdk_path: Path) -> None:
    logger.info("Adding executable permission")
    toolchains_path = sdk_path / "toolchains"
    restool = toolchains_path / "restool"
    add_executable_permission(restool)


def replace_sdk(sdk_file: Path, dest_dir: Path) -> None:
    logger.info("Replacing SDK")
    if not dest_dir.exists():
        logger.error(f"Directory {dest_dir} does not exist")
        return
    extract_sdk(sdk_file, dest_dir)
    change_api_version(dest_dir)


def restore_sdk(sdk_path: Path) -> None:
    logger.info("Restoring SDK")
    sdk_bak_path = sdk_path.parent / (sdk_path.name + "_bak")
    shutil.rmtree(sdk_path)
    logger.info(f"Directory {sdk_path} has been deleted")
    sdk_bak_path.rename(sdk_path)
    logger.info(f"Directory {sdk_bak_path} has been renamed to {sdk_path}")


def prepare(root_dir: Path) -> bool:
    process_config(root_dir)
    setup_logger(root_dir)
    return True
