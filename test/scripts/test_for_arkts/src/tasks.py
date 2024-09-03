import concurrent.futures
import fileinput
import json
import logging
import re
import subprocess
from pathlib import Path

from git import Repo

from src import email, repo_tools, utils

logger = logging.getLogger("test_logger")


def parse_test_rt(log_dir: Path, test_name: str) -> tuple:
    test_failures = []
    current_test = None
    collecting_failure = False
    test_overall_result = "Unknown"

    with open(log_dir / f"{test_name}.log") as f:
        lines = f.readlines()

    if lines[-1].strip() == "TEST SUCCESSFUL":
        test_overall_result = "Passed"
    elif lines[-1].strip() == "TEST FAILED":
        test_overall_result = "Failed"

    if test_overall_result == "Passed":
        return test_overall_result, []

    for line in lines:
        if line.startswith("Running"):
            if collecting_failure and current_test is not None:
                test_failures.append(current_test.split("Running test ", 1)[1])
                collecting_failure = False

            current_test = line.strip()
        elif line.startswith("Expected"):
            collecting_failure = True

    # Check if the last test case was failed
    if collecting_failure and current_test is not None:
        test_failures.append(current_test.split("Running test ", 1)[1])

    return test_overall_result, test_failures


def save_rt_summary(log_dir: Path, test_names: list) -> Path:
    summary = {}
    for test_name in test_names:
        overall_result, failures = parse_test_rt(log_dir, test_name)
        summary[test_name] = {"overall_result": overall_result, "failures": failures}

    summary_file = log_dir / "Summary.json"
    with open(summary_file, "w") as f:
        json.dump(summary, f, indent=4)

    return summary_file


def run_command(command: str, cwd: Path) -> subprocess.CompletedProcess:
    logger.info(f"Running {command} (in {cwd.absolute()})")
    return subprocess.run(command.split(), cwd=cwd, capture_output=True, text=True)


def run_commands(commands: list, cwd: Path, log_dir: Path) -> bool:
    for command in commands:
        result = run_command(command, cwd)
        with open(log_dir / "build.log", "a") as f:
            f.write(
                f"Running {command}\n{result.stdout}\n{result.stderr}\n==============================\n"
            )
        if result.returncode != 0:
            logger.error(f"{command} failed with error code {result.returncode}")
            return False

    return True


def test_rt() -> Path | None:
    logger.info("Testing RT")
    repo_dir = extern_dir / "arkcompiler_ets_frontend" / "ets2panda" / "linter"
    if not repo_dir.exists():
        logger.error(f"{repo_dir} does not exist. Skip RT tests")
        return None

    log_dir = root_dir / "result" / "rt_logs"
    log_dir.mkdir(parents=True, exist_ok=True)

    pre_commands = ["npm install"]
    if not run_commands(pre_commands, repo_dir, log_dir):
        logger.error("Failed to run pre_commands. Skip RT tests")
        return None

    tasks = ["test_main", "test_rules", "test_regression"]

    for test_name in tasks:
        logger.info(f"Running npm run {test_name}")
        test_result = run_command(f"npm run {test_name}", repo_dir)
        with open(log_dir / f"{test_name}.log", "w") as f:
            f.write(test_result.stdout)

    summary_file = save_rt_summary(log_dir, tasks)
    if summary_file.exists():
        return summary_file

    logger.error("Failed to generate RT Summary")
    return None


def parse_test_tsc(log_dir: Path, test_name: str) -> tuple:
    test_failures = []
    test_overall_result = "Unknown"
    passing_pattern = re.compile(r"^\s\s\d+ passing \(.*\)$")
    failure_pattern = re.compile(r"^\s*\d+\)")
    collect_failures = False

    with open(log_dir / f"tsc_{test_name}.log") as f:
        lines = f.readlines()

    for i, line in enumerate(lines):
        if not collect_failures and passing_pattern.match(line):
            collect_failures = True
            continue

        if collect_failures and failure_pattern.match(line) and i + 4 < len(lines):
            failure_info = {
                "task": lines[i + 2].strip(),
                "case": lines[i + 3].strip(),
                "error": lines[i + 4].strip(),
            }
            test_failures.append(failure_info)

    test_overall_result = "Failed" if test_failures else "Passed"
    return test_overall_result, test_failures


def save_tsc_summary(log_dir: Path, test_names: list) -> Path:
    summary = {}
    for test_name in test_names:
        overall_result, failures = parse_test_tsc(log_dir, test_name)
        summary[test_name] = {"overall_result": overall_result, "failures": failures}

    summary_file = log_dir / "Summary.json"
    with open(summary_file, "w") as f:
        json.dump(summary, f, indent=4)

    return summary_file


def update_tsc_interface(repo_path: Path) -> None:
    logger.info("Updating interface")
    config_file = repo_path / "tests/dets/tsconfig.json"

    with fileinput.FileInput(config_file, inplace=True) as f:
        for line in f:
            if "../interface/sdk-js" in line:
                line = line.replace("../interface/sdk-js", "interface_sdk-js")
            print(line, end="")


def test_tsc() -> Path | None:
    logger.info("Testing TSC")
    repo_dir = extern_dir / "third_party_typescript"
    if not repo_dir.exists():
        logger.error(f"{repo_dir} does not exist. Skip tsc tests")
        return None

    update_tsc_interface(repo_dir)

    log_dir = root_dir / "result" / "tsc_logs"
    log_dir.mkdir(parents=True, exist_ok=True)

    pre_commands = ["npm install", "npm run build", "npm run release"]
    if not run_commands(pre_commands, repo_dir, log_dir):
        logger.error("Failed to run pre_commands. Skip tsc tests")
        return None

    tasks = ["test"]

    for test_name in tasks:
        logger.info(f"Running npm run {test_name}")
        test_result = run_command(f"npm run {test_name}", repo_dir)
        with open(log_dir / f"tsc_{test_name}.log", "w") as f:
            f.write(test_result.stdout)

    summary_file = save_tsc_summary(log_dir, tasks)
    if summary_file.exists():
        return summary_file

    logger.error("Failed to generate TSC Summary")
    return None


def parse_test_arktstest(log_dir: Path, test_name: str) -> tuple:
    test_failures = []
    test_overall_result = "Unknown"
    collect_failures = False

    with open(log_dir / f"arktstest_{test_name}.log") as f:
        lines = f.readlines()

    for line in lines:
        if not collect_failures and line.startswith("Failed test cases:"):
            collect_failures = True
            continue

        if collect_failures:
            if not line.startswith("testcase"):
                break
            test_failures.append(line.strip())

    test_overall_result = "Failed" if test_failures else "Passed"
    return test_overall_result, test_failures


def save_arktstest_summary(log_dir: Path, test_names: list) -> Path:
    summary = {}
    for test_name in test_names:
        overall_result, failures = parse_test_arktstest(log_dir, test_name)
        summary[f"arktstest_{test_name}"] = {
            "overall_result": overall_result,
            "failures": failures,
        }

    summary_file = log_dir / "Summary.json"
    with open(summary_file, "w") as f:
        json.dump(summary, f, indent=4)

    return summary_file


def test_arktstest() -> Path | None:
    logger.info("Testing arkTSTest")
    repo_dir = extern_dir / "third_party_typescript"
    if not repo_dir.exists():
        logger.error(f"{repo_dir} does not exist. Skip arkTSTest tests")
        return None

    log_dir = root_dir / "result" / "arktstest_logs"
    log_dir.mkdir(parents=True, exist_ok=True)

    src_dst_pairs = [
        (
            repo_dir / "./lib/typescript.d.ts",
            repo_dir / "./tests/baselines/reference/api/typescript.d.ts",
        ),
        (
            repo_dir / "./lib/tsserverlibrary.d.ts",
            repo_dir / "./tests/baselines/reference/api/tsserverlibrary.d.ts",
        ),
    ]
    for src, dst in src_dst_pairs:
        if not src.exists():
            logger.error(f"{src} does not exist. Skip arkTSTest tests")
            return None
        logger.info(f"Copying {src} to {dst}")
        dst.write_bytes(src.read_bytes())

    pack_command = ["npm pack"]
    if not run_commands(pack_command, repo_dir, log_dir):
        logger.error("Failed to run pack_command. Skip arkTSTest tests")
        return None

    arktstest_dir = repo_dir / "tests" / "arkTSTest"

    install_commands = ["npm install"]
    if not run_commands(install_commands, arktstest_dir, log_dir):
        logger.error("Failed to run install_commands. Skip arkTSTest tests")
        return None

    tasks = ["v1.0", "v1.1"]

    for test_name in tasks:
        logger.info(f"Running node run.js -D -{test_name}")
        test_result = run_command(f"node run.js -D -{test_name}", arktstest_dir)
        with open(log_dir / f"arktstest_{test_name}.log", "w") as f:
            f.write(test_result.stdout)

    summary_file = save_arktstest_summary(log_dir, tasks)
    if summary_file.exists():
        return summary_file

    logger.error("Failed to generate arkTSTest Summary")
    return None


def parse_cookbook_log(log_file: Path) -> tuple:
    logger.info("Parsing Cookbook log")

    with open(log_file) as f:
        lines = f.readlines()

    collect_flag = False
    passed_list = []
    failed_list = []

    for line in lines:
        line = line.strip()

        if line.startswith("=========="):
            collect_flag = True
            continue

        if collect_flag:
            match = re.match(r"^(.*\s(True|False))$", line)
            if match:
                test_case, result = match.groups()
                if result == "True":
                    passed_list.append(test_case.replace(" True", ""))
                elif result == "False":
                    failed_list.append(test_case.replace(" False", ""))

    if len(failed_list) == 0:
        return "Passed", failed_list

    return "Failed", failed_list


def save_cookbook_result(log_dir: Path) -> Path:
    logger.info("Saving Cookbook result")
    overall_res, failed_list = parse_cookbook_log(log_dir / "cookbook.log")
    summary = {}
    summary["cookbook"] = {"overall_result": overall_res, "failures": failed_list}

    summary_file = log_dir / "Summary.json"
    with open(summary_file, "w") as f:
        json.dump(summary, f, indent=4)

    return summary_file


def test_cookbook(sdk_file: str) -> Path | None:
    logger.info("Testing Cookbook")
    utils.replace_sdk(Path(sdk_file), Path(utils.config["ohos_sdk"]))
    cookbook_path = utils.config["cookbook_path"]
    cookbook_script = f"{cookbook_path}/cookbook.sh"
    cookbook_cmd = ["/usr/bin/bash", cookbook_script]
    logger.info(cookbook_cmd)

    try:
        result = subprocess.run(
            cookbook_cmd,
            cwd=cookbook_path,
            shell=False,
            capture_output=True,
            text=True,
        )
    except subprocess.CalledProcessError as e:
        logger.error(f"Failed to run script: {e}")
        return None

    log_dir = root_dir / "result" / "cookbook"
    log_dir.mkdir(parents=True, exist_ok=True)
    with open(log_dir / "cookbook.log", "w") as f:
        f.write(result.stdout)
    with open(log_dir / "cookbook.err", "w") as f:
        f.write(result.stderr)

    summary_file = save_cookbook_result(log_dir)

    utils.restore_sdk(Path(utils.config["ohos_sdk"]))
    if summary_file.exists():
        logger.info("Cookbook test finished")
        return summary_file

    logger.error("Cookbook test failed")
    return None


def parse_verify_log(log_file: Path) -> tuple:
    logger.info("Parsing 3rd libs verification log")
    with open(log_file) as f:
        lines = f.readlines()

    skiped_collect = False
    skiped_list = []
    failed_collect = False
    failed_list = []
    for line in lines:
        line = line.strip()
        if line.startswith("Skiped projects:"):
            skiped_collect = True
            failed_collect = False
            continue
        if line.startswith("Failed projects:"):
            failed_collect = True
            skiped_collect = False
            continue
        if skiped_collect:
            if line.startswith("========"):
                skiped_collect = False
            else:
                skiped_list.append(line)
        if failed_collect:
            if line.startswith("========"):
                failed_collect = False
            else:
                failed_list.append(line)

    status = "Failed" if failed_list else "Passed"
    return status, failed_list, skiped_list


def save_verity_result(log_dir: Path) -> Path:
    logger.info("Saving 3rd libs verification result")
    overall_res, failed_list, skiped_list = parse_verify_log(log_dir / "verify.log")
    summary = {}
    summary["libs_verification"] = {
        "overall_result": overall_res,
        "failures": failed_list,
        "skiped_projects": skiped_list,
    }

    summary_file = log_dir / "Summary.json"
    with open(summary_file, "w") as f:
        json.dump(summary, f, indent=4)

    return summary_file


def test_verify_3rd_libs(sdk_file: str) -> Path | None:
    logger.info("Testing 3rd party libraries")
    utils.replace_sdk(Path(sdk_file), Path(utils.config["oh_sdk"]))
    verify_path = utils.config["verify_path"]
    libs_verification_script = f"{verify_path}/verify.sh"
    verification_cmd = ["/usr/bin/bash", libs_verification_script]
    logger.info(verification_cmd)

    try:
        result = subprocess.run(
            verification_cmd,
            cwd=verify_path,
            shell=False,
            capture_output=True,
            text=True,
        )
    except subprocess.CalledProcessError as e:
        logger.error(f"Failed to run script: {e}")
        return None

    log_dir = root_dir / "result" / "verify"
    log_dir.mkdir(parents=True, exist_ok=True)
    with open(log_dir / "verify.log", "w") as f:
        f.write(result.stdout)
    with open(log_dir / "verify.err", "w") as f:
        f.write(result.stderr)

    summary_file = save_verity_result(log_dir)

    utils.restore_sdk(Path(utils.config["oh_sdk"]))
    if summary_file.exists():
        logger.info("3rd libs verification finished")
        return summary_file

    logger.error("3rd libs verification failed")
    return None


def sdk_test() -> tuple:
    with concurrent.futures.ThreadPoolExecutor() as executor:
        future_find_sdk = executor.submit(utils.find_sdk)
        sdk_file, sdk_generated_time = future_find_sdk.result()
        if sdk_file:
            future_cookbook = executor.submit(test_cookbook, sdk_file)
            future_libs_verification = executor.submit(test_verify_3rd_libs, sdk_file)
            cookbook_summary = future_cookbook.result()
            libs_verification_summary = future_libs_verification.result()
            tar_file = Path(sdk_file)
            if tar_file.exists():
                tar_file.unlink()
        else:
            cookbook_summary = None
            libs_verification_summary = None

        return sdk_generated_time, cookbook_summary, libs_verification_summary


def run_tests(root: Path) -> None:
    global root_dir
    global extern_dir
    root_dir = root
    repo_file = root_dir / "resources" / "repo"
    extern_dir = root_dir / "extern"
    try:
        # clone repositories
        repo_tools.clone_repos(repo_file, extern_dir)

        with concurrent.futures.ThreadPoolExecutor() as executor:
            future_rt = executor.submit(test_rt)
            future_tsc = executor.submit(test_tsc)
            future_sdk_test = executor.submit(sdk_test)
            tsc_summary = future_tsc.result()
            future_arktstest = executor.submit(test_arktstest)

            rt_summary = future_rt.result()
            arktstest_summary = future_arktstest.result()
            sdk_generated_time, cookbook_summary, libs_verification_summary = (
                future_sdk_test.result()
            )

        email_template = root_dir / "resources" / "email_template.html"
        with open(email_template, encoding="utf-8") as f:
            html_body = f.read()

        html_body = email.add_manifest(html_body)
        html_body = email.add_sdk_generated_time(sdk_generated_time, html_body)
        html_body = email.add_rt_result(rt_summary, html_body)
        html_body = email.add_tsc_result(tsc_summary, html_body)
        html_body = email.add_arktstest_result(arktstest_summary, html_body)
        html_body = email.add_cookbook_result(cookbook_summary, html_body)
        html_body = email.add_libs_verification_result(
            libs_verification_summary, html_body
        )

        if utils.config["send_email"]:
            archive_path = utils.pack_result(root_dir)
            email.send_email(html_body, archive_path)
            utils.clean(root_dir, archive_path)
    except Exception as e:
        logger.error(f"Failed to run tests: {e}")


def test_tsc_by_pr(pr_url: str, ci_id: str, extern_dir: Path) -> None:
    logger.info("Testing TSC by PR")
    extern_dir.mkdir(parents=True, exist_ok=True)
    # ci_sdk_file = utils.find_sdk(days=30)
    owner, repo, pr_number = repo_tools.parse_pr_link(pr_url)
    Repo.clone_from(f"https://gitee.com/{owner}/{repo}.git", extern_dir / repo)
    pr_info = repo_tools.get_pr_info(owner, repo, pr_number)
    repo_dir = extern_dir / repo
    repo_tools.clone_and_update_repo(pr_info, repo_dir)
    if repo == "third_party_typescript":
        Repo.clone_from(
            "https://gitee.com/openharmony/interface_sdk-js.git",
            extern_dir / "interface_sdk-js",
        )
        # tsc_summary = test_tsc(extern_dir)
        # arkts_summary = test_arktstest(extern_dir)
        with concurrent.futures.ThreadPoolExecutor() as executor:
            future_daily_sdk = executor.submit(utils.find_sdk(days=30))
            future_pr_sdk = executor.submit(utils.find_sdk_by_ci_id(ci_id))
            daily_sdk = future_daily_sdk.result()
            pr_sdk_file = future_pr_sdk.result()
        if daily_sdk:
            daily_libs_verification_summary = test_verify_3rd_libs(daily_sdk)
            print(daily_libs_verification_summary)
            tar_file = Path(daily_sdk)
            if tar_file.exists():
                tar_file.unlink()
        if pr_sdk_file:
            with concurrent.futures.ThreadPoolExecutor() as executor:
                future_cookbook = executor.submit(test_cookbook, pr_sdk_file)
                future_pr_libs_verification = executor.submit(test_verify_3rd_libs, pr_sdk_file)
                cookbook_summary = future_cookbook.result()
                libs_verification_summary = future_pr_libs_verification.result()
                print(cookbook_summary)
                print(libs_verification_summary)
                tar_file = Path(pr_sdk_file)
                if tar_file.exists():
                    tar_file.unlink()
        # deveco_test(extern_dir.parent)
    # elif repo == "arkcompiler_ets_frontend":
    #     # summary_file = test_rt(extern_dir)
    #     print("...")
