import re
import json
import fileinput
import logging
import subprocess
from pathlib import Path

test_logger = logging.getLogger("test_logger")


def parse_test_rt(log_dir: Path, test_name: str):
    test_failures = []
    current_test = None
    collecting_failure = False
    test_overall_result = "Unknown"

    with open(log_dir / f"{test_name}.log", "r") as f:
        lines = f.readlines()

    if lines[-1].strip() == "TEST SUCCESSFUL":
        test_overall_result = "Passed"
    elif lines[-1].strip() == "TEST FAILED":
        test_overall_result = "Failed"

    if test_overall_result == "Passed":
        return test_overall_result, []

    for line in lines:
        if line.startswith("Running"):
            if collecting_failure:
                test_failures.append(current_test.split("Running test ", 1)[1])
                collecting_failure = False

            current_test = line.strip()
        elif line.startswith("Expected"):
            collecting_failure = True

    # Check if the last test case was failed
    if collecting_failure:
        test_failures.append(current_test.split("Running test ", 1)[1])

    return test_overall_result, test_failures


def save_rt_summary_as_json(log_dir: Path, test_names: list):
    summary = {}
    for test_name in test_names:
        overall_result, failures = parse_test_rt(log_dir, test_name)
        summary[test_name] = {"overall_result": overall_result, "failures": failures}

    summary_file = log_dir / "Summary.json"
    with open(summary_file, "w") as f:
        json.dump(summary, f, indent=4)

    return summary_file


def run_commands(commands: list, cwd: Path, log_dir: Path = None):
    for command in commands:
        test_logger.info(f"Running {command} (in {cwd.name})")
        result = subprocess.run(
            command.split(), cwd=cwd, capture_output=True, text=True
        )
        with open(log_dir / "build.log", "a") as f:
            f.write(f"Running {command}\n")
            f.write(result.stdout)
            f.write(result.stderr)
            f.write("\n==============================\n")
        if result.returncode != 0:
            test_logger.error(f"{command} failed with error code {result.returncode}")
            return False

    return True


def test_rt(extern_dir_path: Path):
    test_logger.info("Testing RT")
    dir = extern_dir_path / "arkcompiler_ets_frontend" / "ets2panda" / "linter"
    if not dir.exists():
        test_logger.error(f"{dir} does not exist. Skip RT tests")
        return None

    log_dir = extern_dir_path / ".." / "result" / "rt_logs"
    if not log_dir.exists():
        log_dir.mkdir(parents=True, exist_ok=True)

    pre_commands = ["npm install"]
    if not run_commands(pre_commands, dir, log_dir):
        test_logger.error("Failed to run pre_commands. Skip RT tests")
        return None

    tasks = ["test_main", "test_rules", "test_regression"]

    for test_name in tasks:
        test_logger.info(f"Running npm run {test_name} (in {dir.name})")
        test_result = subprocess.run(
            ["npm", "run", test_name], cwd=dir, capture_output=True, text=True
        )
        log_file = log_dir / f"{test_name}.log"
        with open(log_file, "w") as f:
            f.write(test_result.stdout)

    summary_file = save_rt_summary_as_json(log_dir, tasks)

    if summary_file.exists():
        return summary_file
    else:
        test_logger.error("Failed to generate RT Summary")
        return None


def parse_test_tsc(log_dir: Path, test_name: str):
    test_failures = []
    test_overall_result = "Unknown"
    passing_pattern = re.compile(r"^\s\s\d+ passing \(.*\)$")
    failure_pattern = re.compile(r"^\s*\d+\)")
    collect_failures = False

    with open(log_dir / f"tsc_{test_name}.log", "r") as f:
        lines = f.readlines()

    for i, line in enumerate(lines):
        if not collect_failures and passing_pattern.match(line):
            collect_failures = True
            continue

        if collect_failures:
            if failure_pattern.match(line) and i + 4 < len(lines):
                failure_info = {
                    "task": lines[i + 2].strip(),
                    "case": lines[i + 3].strip(),
                    "error": lines[i + 4].strip(),
                }
                test_failures.append(failure_info)

    if test_failures:
        test_overall_result = "Failed"
    else:
        test_overall_result = "Passed"

    return test_overall_result, test_failures


def save_tsc_summary_as_json(log_dir: Path, test_names: list):
    summary = {}
    for test_name in test_names:
        overall_result, failures = parse_test_tsc(log_dir, test_name)
        summary[test_name] = {"overall_result": overall_result, "failures": failures}

    summary_file = log_dir / "Summary.json"
    with open(summary_file, "w") as f:
        json.dump(summary, f, indent=4)

    return summary_file


def update_interface(repo_path: Path):
    test_logger.info("Updating interface")
    config_file = repo_path / "tests/dets/tsconfig.json"

    with fileinput.FileInput(config_file, inplace=True) as f:
        for line in f:
            if "../interface/sdk-js" in line:
                line = line.replace("../interface/sdk-js", "interface_sdk-js")
            print(line, end="")


def test_tsc(extern_dir_path: Path):
    test_logger.info("Testing TSC")
    dir = extern_dir_path / "third_party_typescript"
    if not dir.exists():
        test_logger.error(f"{dir} does not exist. Skip tsc tests")
        return None

    update_interface(dir)

    log_dir = extern_dir_path / ".." / "result" / "tsc_logs"
    if not log_dir.exists():
        log_dir.mkdir(parents=True, exist_ok=True)

    pre_commands = ["npm install", "npm run build", "npm run release"]
    if not run_commands(pre_commands, dir, log_dir):
        test_logger.error("Failed to run pre_commands. Skip tsc tests")
        return None

    tasks = ["test"]

    for test_name in tasks:
        test_logger.info(f"Running npm run {test_name} (in {dir.name})")
        test_result = subprocess.run(
            ["npm", "run", test_name], cwd=dir, capture_output=True, text=True
        )
        log_file = log_dir / f"tsc_{test_name}.log"
        with open(log_file, "w") as f:
            f.write(test_result.stdout)

    summary_file = save_tsc_summary_as_json(log_dir, tasks)

    if summary_file.exists():
        return summary_file
    else:
        test_logger.error("Failed to generate TSC Summary")
        return None


def parse_test_arktstest(log_dir: Path, test_name: str):
    test_failures = []
    test_overall_result = "Unknown"
    collect_failures = False

    with open(log_dir / f"arktstest_{test_name}.log", "r") as f:
        lines = f.readlines()

    for line in lines:
        if not collect_failures and line.startswith("Failed test cases:"):
            collect_failures = True
            continue

        if collect_failures:
            if not line.startswith("testcase"):
                break
            test_failures.append(line.strip())

    if test_failures:
        test_overall_result = "Failed"
    else:
        test_overall_result = "Passed"

    return test_overall_result, test_failures


def save_arktstest_summary_as_json(log_dir: Path, test_names: list):
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


def test_arktstest(extern_dir_path: Path):
    test_logger.info("Testing arkTSTest")
    dir = extern_dir_path / "third_party_typescript"
    if not dir.exists():
        test_logger.error(f"{dir} does not exist. Skip arkTSTest tests")
        return None

    log_dir = extern_dir_path / ".." / "result" / "arktstest_logs"
    if not log_dir.exists():
        log_dir.mkdir(parents=True, exist_ok=True)

    src_dst_pairs = [
        (
            dir / "./lib/typescript.d.ts",
            dir / "./tests/baselines/reference/api/typescript.d.ts",
        ),
        (
            dir / "./lib/tsserverlibrary.d.ts",
            dir / "./tests/baselines/reference/api/tsserverlibrary.d.ts",
        ),
    ]
    for src, dst in src_dst_pairs:
        if not src.exists():
            test_logger.error(f"{src} does not exist. Skip arkTSTest tests")
            return None
        test_logger.info(f"Copying {src} to {dst}")
        dst.write_bytes(src.read_bytes())

    pack_command = ["npm pack"]
    if not run_commands(pack_command, dir, log_dir):
        test_logger.error("Failed to run pack_command. Skip arkTSTest tests")
        return None

    arktstest_dir = dir / "tests" / "arkTSTest"

    install_commands = ["npm install"]
    if not run_commands(install_commands, arktstest_dir, log_dir):
        test_logger.error("Failed to run install_commands. Skip arkTSTest tests")
        return None

    tasks = ["v1.0", "v1.1"]

    for test_name in tasks:
        test_logger.info(
            f"Running node run.js -D -{test_name} (in {arktstest_dir.name})"
        )
        test_result = subprocess.run(
            ["node", "run.js", "-D", f"-{test_name}"],
            cwd=arktstest_dir,
            capture_output=True,
            text=True,
        )
        log_file = log_dir / f"arktstest_{test_name}.log"
        with open(log_file, "w") as f:
            f.write(test_result.stdout)

    summary_file = save_arktstest_summary_as_json(log_dir, tasks)

    if summary_file.exists():
        return summary_file
    else:
        test_logger.error("Failed to generate arkTSTest Summary")
        return None
