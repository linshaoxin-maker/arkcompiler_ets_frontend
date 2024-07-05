import os
import re
import json
import time
import shutil
import logging
import subprocess
import statistics

from pathlib import Path
from matplotlib import pyplot as plt
from matplotlib.axes import Axes
from matplotlib.figure import Figure
from jinja2 import Environment, FileSystemLoader

from src.performance_test import options

test_logger = logging.getLogger("test_logger")
MAX_LOOP: int = 25


def plot_data(
    ax: Axes,
    date_list: list[str],
    data: list[float],
    label: str,
    baseline: float | None,
    color,
):
    line = ax.plot(date_list, data, color=color, label=label)
    ax.tick_params(axis="x", rotation=45)
    if baseline is not None:
        ax.axhline(y=baseline, linestyle="--", color="gray")
        ax.text(-0.7, baseline, f"{baseline}", ha="right", va="center")
        for i, j in enumerate(data):
            diff = j - baseline
            diff_str = f"+{round(diff, 1)}" if diff > 0 else f"{round(diff, 1)}"
            color = "red" if diff > 0 else "green"
            va = "bottom" if diff > 0 else "top"
            ax.text(i, j, diff_str, ha="center", va=va, color=color)
    return line


def generate_line_chart(results_dir: Path, days: int, picture_path: Path):
    if days < 1:
        test_logger.error("Select data of at least one day for display")
        return
    old_result_dirs = sorted(
        [d for d in results_dir.iterdir() if d.name.startswith("result_")]
    )
    last_several_results = (
        old_result_dirs[-days:] if len(old_result_dirs) > days else old_result_dirs
    )

    rebuild_total = []
    rebuild_arkts = []
    product_full_total = []
    product_full_arkts = []
    product_full_ets_checker = []
    date_list = []

    for res in last_several_results:
        summary_path = res / "summary.json"
        data = json.loads(summary_path.read_text() if summary_path.exists() else "{}")
        rebuild_data = data["project"]["rebuild"]
        rebuild_total.append(rebuild_data["total_compile"]["average"])
        rebuild_arkts.append(rebuild_data["arkts_compile"]["average"])
        product_full_data = data["project"]["product_full"]
        product_full_total.append(product_full_data["total_compile"]["average"])
        product_full_arkts.append(product_full_data["arkts_compile"]["average"])
        product_full_ets_checker.append(product_full_data["ets_checker"]["average"])
        date_list.append(res.name.split("result_")[1])

    plt.style.use("fast")
    fig: Figure
    ax: Axes
    fig, ax = plt.subplots(figsize=(10, 6))

    baseline_list = [500, 1000, 320, 290, 180]
    plot_data(
        ax, date_list, rebuild_total, "rebuild Total", baseline_list[0], "#1f77b4"
    )
    plot_data(
        ax,
        date_list,
        rebuild_arkts,
        "compile arkts in rebuild project",
        baseline_list[1],
        "#ff7f0e",
    )
    plot_data(
        ax,
        date_list,
        product_full_arkts,
        "compile arkts in compile product",
        baseline_list[3],
        "#d62728",
    )
    plot_data(
        ax,
        date_list,
        product_full_ets_checker,
        "ets_checker in compile product",
        baseline_list[4],
        "#9467bd",
    )

    ax.set_title("Compile Performance")
    ax.grid(True)
    ax.legend(loc="center left", bbox_to_anchor=(1, 0.5))
    plt.tight_layout()
    plt.savefig(picture_path / "line_chart.png", bbox_inches="tight")


def is_compile_success(compile_stdout):
    pattern = r"BUILD SUCCESSFUL in (\d+ min)?(\d+ s)?(\d+ ms)?"
    match_result = re.search(pattern, compile_stdout)
    if not match_result:
        return [False, ""]

    return [True, match_result.group(0)]


def get_compile_command(module: None | options.HapModule):
    cmd = [
        os.environ.get("NODE_JS_EXE"),
        os.environ.get("HVIGORW_JS"),
        "--mode",
        "module",
        "-p",
        "product=default",
        "-p",
        f"module={module.name}@default",
        "assembleHap",
        "--analyze=normal",
        "--parallel",
        "--incremental",
        "--daemon",
    ]
    return cmd


def clean_compile(task: options.PerformanceTestTask):
    cmd = [
        os.environ.get("NODE_JS_EXE"),
        os.environ.get("HVIGORW_JS"),
        "-p",
        "product=default",
        "clean",
        "--analyze=normal",
        "--parallel",
        "--incremental",
        "--daemon",
    ]
    test_logger.debug(f"cmd: {cmd}")
    test_logger.debug(f"cmd execution path: {task.path}")
    process = subprocess.Popen(
        cmd, shell=False, cwd=task.path, stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    out, err = process.communicate(timeout=800)


def compile_project(
    task: options.PerformanceTestTask, module: None | options.HapModule, cmd=None
):
    if cmd is None:
        cmd = get_compile_command(module=module)

    test_logger.debug(f"cmd: {cmd}")
    test_logger.debug(f"cmd execution path: {task.path}")
    process = subprocess.Popen(
        cmd, shell=False, cwd=task.path, stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )

    stdout, stderr = process.communicate(timeout=800)
    stdout_utf8 = stdout.decode("utf-8", errors="ignore")
    stderr_utf8 = stderr.decode("utf-8", errors="ignore")

    return [stdout_utf8, stderr_utf8]


def export_latest_report(task: options.PerformanceTestTask, log_dir: Path):
    report_dir = Path(os.path.join(task.path, ".hvigor", "report"))
    dst_dir = log_dir / "reports"
    dst_dir.mkdir(parents=True, exist_ok=True)

    latest_file = max(
        (file for file in report_dir.iterdir() if file.is_file()),
        key=os.path.getctime,
        default=None,
    )

    if latest_file:
        shutil.copy(str(latest_file), str(dst_dir / latest_file.name))
        test_logger.info("report copied successfully")
    else:
        test_logger.error("report copied failed")


def compile_incremental_add_line(
    task: options.PerformanceTestTask, module: options.HapModule, res_dir: Path
):
    test_name = "add_oneline"

    test_logger.info(f"=========> Running {test_name} for task: {task.name}")
    modify_file_item = module.inc_modify_file
    modify_file = os.path.join(task.path, *modify_file_item)
    modify_file_backup = modify_file + ".bak"
    shutil.copyfile(modify_file, modify_file_backup)
    log_dir = res_dir / f"{task.name}" / f"{module.name}_incremental"
    log_dir.mkdir(parents=True, exist_ok=True)

    for i in range(MAX_LOOP):
        with open(modify_file, "a", encoding="utf-8") as file:
            file.write("\nconsole.log('This is a new line');")

        [stdout, stderr] = compile_project(task, module)
        [is_success, time_str] = is_compile_success(stdout)
        if is_success:
            export_latest_report(task, log_dir)
            with open(log_dir / f"out_{i}.log", "w") as f:
                f.write(stdout)
            with open(log_dir / f"err_{i}.log", "w") as f:
                f.write(stderr)
        time.sleep(10)

    shutil.move(modify_file_backup, modify_file)


def performance_rebuild_compile(task: options.PerformanceTestTask):
    test_logger.info(f"=========> Running task: {task.name} in rebuild compilation")
    cmd = [
        os.environ.get("NODE_JS_EXE"),
        os.environ.get("HVIGORW_JS"),
        "clean",
        "--mode",
        "module",
        "-p",
        "product=default",
        "assembleHap",
        "--analyze=normal",
        "--parallel",
        "--incremental",
        "--daemon",
    ]
    compile_token = "rebuild"
    log_dir = res_dir / f"{task.name}" / compile_token
    log_dir.mkdir(parents=True, exist_ok=True)
    for i in range(MAX_LOOP):
        clean_compile(task)
        [stdout, stderr] = compile_project(task, module=None, cmd=cmd)
        [is_success, time_str] = is_compile_success(stdout)
        if is_success:
            with open(log_dir / f"out_{i}.log", "w") as f:
                f.write(stdout)
            with open(log_dir / f"err_{i}.log", "w") as f:
                f.write(stderr)
    clean_compile(task)


def performance_module_incremental_compile(
    task: options.PerformanceTestTask, module: options.HapModule
):
    test_logger.info(f"=========> Running task: {task.name} in incremental compilation")
    clean_compile(task)
    [stdout, stderr] = compile_project(task, module=module)
    [is_success, time_str] = is_compile_success(stdout)
    if not is_success:
        test_logger.error("Incremental compile failed due to the first compile failed!")
        return
    compile_incremental_add_line(task, module, res_dir)
    clean_compile(task)


def performance_module_compile(
    task: options.PerformanceTestTask, module: options.HapModule
):
    test_logger.info(f"=========> Running task: {task.name} in module compilation")
    clean_compile(task)
    [stdout, stderr] = compile_project(task, module=module)
    [is_success, time_str] = is_compile_success(stdout)
    if not is_success:
        return False
    log_dir = res_dir / f"{task.name}" / f"{module.name}_full"
    log_dir.mkdir(parents=True, exist_ok=True)
    for i in range(MAX_LOOP):
        clean_compile(task)
        [stdout, stderr] = compile_project(task, module=module)
        [is_success, time_str] = is_compile_success(stdout)
        if is_success:
            export_latest_report(task, log_dir)
            with open(log_dir / f"out_{i}.log", "w") as f:
                f.write(stdout)
            with open(log_dir / f"err_{i}.log", "w") as f:
                f.write(stderr)
        clean_compile(task)
    return True


def extract_total_time(log_path: Path):
    pattern = r"BUILD SUCCESSFUL in (\d+ min )?(\d+ s )?(\d+ ms )?"
    match_result = re.search(pattern, log_path.read_text())

    total_compile_time = 0

    def time_to_ms(time_str: str):
        if " min " in time_str:
            return int(time_str.replace(" min ", "")) * 60 * 1000
        elif " s " in time_str:
            return int(time_str.replace(" s ", "")) * 1000
        elif " ms " in time_str:
            return int(time_str.replace(" ms ", ""))
        else:
            return 0

    for group in match_result.groups():
        if group is not None:
            total_compile_time += time_to_ms(group)
    return total_compile_time


def milliseconds_to_readable(milliseconds: int) -> str:
    seconds = milliseconds // 1000
    minutes = seconds // 60
    remaining_seconds = seconds % 60
    remaining_milliseconds = milliseconds % 1000
    readable_time = ""
    if minutes:
        readable_time += f"{minutes} min "
    if remaining_seconds:
        readable_time += f"{remaining_seconds} s "
    if remaining_milliseconds:
        readable_time += f"{remaining_milliseconds} ms"
    return readable_time


def milliseconds_to_seconds(milliseconds: int) -> float:
    seconds = milliseconds // 1000
    remaining_milliseconds = milliseconds % 1000
    seconds_str = str(seconds) + "." + str(remaining_milliseconds).zfill(3)
    return float(seconds_str)


def dump_to_json(log_path: Path, summary_path: Path, compile_token: str, out_flag: str):
    if not summary_path.exists():
        return

    data = json.loads(summary_path.read_text())
    if f"{compile_token}_out" in data:
        out = data[f"{compile_token}_out"]
    else:
        out = {}
    out[f"{out_flag}_res"] = {}

    with open(log_path, "r") as f:
        lines = f.readlines()

    time_regex = r"(\d+)(\.\d+)?"
    phase_regex = r"([a-zA-Z]+)(\(.*\))?"
    collect_flag = False
    for line in lines:
        if not collect_flag and line.startswith("Phase"):
            collect_flag = True
            continue
        if collect_flag and not line.startswith("---"):
            phase_match = re.findall(phase_regex, line)
            phase_name = ""
            for match in phase_match:
                for group in match:
                    phase_name += group

            time_match = re.findall(time_regex, line)
            time_cost = ""
            for match in time_match:
                for group in match:
                    time_cost += group
            time_cost = float(time_cost)
            if phase_name in out[f"{out_flag}_res"]:
                time_sum = out[f"{out_flag}_res"][phase_name] + time_cost
                out[f"{out_flag}_res"][phase_name] = float(f"{time_sum:.3f}")
            else:
                out[f"{out_flag}_res"][phase_name] = time_cost
        if collect_flag and line.startswith("total"):
            collect_flag = False

    total_compile_time = extract_total_time(log_path)

    out[f"{out_flag}_res"]["total_compile_time"] = milliseconds_to_seconds(
        total_compile_time
    )
    data[f"{compile_token}_out"] = out
    summary_path.write_text(json.dumps(data, indent=4))


def get_duration(file_path: Path, module_name: str):
    if os.path.exists(file_path):
        with open(file_path, "r") as f:
            data = json.load(f)
    events = data["events"]
    for e in events:
        head = e["head"]
        if (
            head["name"] == f"{module_name}:default@CompileArkTS"
            and head["type"] == "duration"
        ):
            body = e["body"]
            start_time = body["startTime"]
            end_time = body["endTime"]
            duration = end_time - start_time
            return duration


def calc_report(
    task: options.PerformanceTestTask,
    module_name: str,
    compile_token: str,
    report_dir: Path,
):
    durations = []
    for file in report_dir.iterdir():
        duration = get_duration(file, module_name)
        if duration is not None:
            duration_in_seconds = milliseconds_to_seconds(round(duration // 1000000))
            durations.append(duration_in_seconds)
    arkts_average_duration = round(sum(durations) / len(durations), 3)
    readable_time = milliseconds_to_readable(int(arkts_average_duration * 1000))
    arkts_average_stdev = round(statistics.stdev(durations), 3)
    return arkts_average_duration, readable_time, arkts_average_stdev


def calc_phase(summary: Path, compile_token: str):
    if not summary.exists():
        return
    data = json.loads(summary.read_text())
    out: dict[str, dict] = data[f"{compile_token}_out"]
    phase_times: dict[str, list[float]] = {
        phase: []
        for phase in [
            "phase1",
            "phase2",
            "phase3",
        ]
    }
    total_compile_time_list = []
    for res in out:
        for s, t in out[res].items():
            if s in phase_times:
                phase_times[s].append(t)
            elif s == "total_compile_time":
                total_compile_time_list.append(t)

    ets_checker_list = []

    phase1_times = phase_times.get("phase1")
    phase3_times = phase_times.get("phase3")
    if phase1_times is not None and phase3_times is not None:
        for i, v in enumerate(phase1_times):
            ets_checker_list.append(v + phase3_times[i])

    average_phase_times = {
        phase: {
            "average": round(sum(times) / len(times), 3),
            "stdev": round(statistics.stdev(times), 3),
            "stdev_proportion_to_avg": (
                round(
                    round(statistics.stdev(times), 3)
                    / round(sum(times) / len(times), 3)
                    * 100,
                    2,
                )
                if round(sum(times) / len(times), 3) != 0
                else 0
            ),
        }
        for phase, times in phase_times.items()
    }

    total_average_time = round(
        sum(total_compile_time_list) / len(total_compile_time_list), 3
    )
    total_average_stdev = round(statistics.stdev(total_compile_time_list), 3)
    data[f"{compile_token}"]["total_compile"] = {
        "average": total_average_time,
        "stdev": total_average_stdev,
        "readable_time": milliseconds_to_readable(int(total_average_time * 1000)),
        "stdev_proportion_to_avg": round(
            total_average_stdev / total_average_time * 100, 2
        ),
    }

    ets_checker_avg = round(sum(ets_checker_list) / len(ets_checker_list), 3)
    ets_checker_stdev = round(statistics.stdev(ets_checker_list), 3)
    data[f"{compile_token}"]["ets_checker"] = {
        "average": ets_checker_avg,
        "stdev": ets_checker_stdev,
        "readable_time": milliseconds_to_readable(int(ets_checker_avg * 1000)),
        "stdev_proportion_to_avg": round(ets_checker_stdev / ets_checker_avg * 100, 2),
    }

    data[f"{compile_token}"]["phase_avg"] = average_phase_times
    summary.write_text(json.dumps(data, indent=4))


def performance_rebuild_parse(task: options.PerformanceTestTask):
    log_dir = "rebuild"
    output_dir = res_dir / f"{task.name}"
    summary_path = output_dir / "summary.json"
    if not output_dir.exists():
        test_logger.error(f"======> There is no result for: {task.name}")
        return

    data = json.loads(summary_path.read_text() if summary_path.exists() else "{}")
    rebuild = {"module": {}}
    line_pattern = re.compile(r"^> hvigor")

    def process_line(line):
        name_pattern = r":([a-zA-Z]+):default@"
        name_match = re.search(name_pattern, line)
        pattern = r"after (\d+) min (\d+) s (\d+) ms"
        match = re.search(pattern, line)
        if match:
            minutes, seconds, milliseconds = map(int, match.groups())
            single_time = minutes * 60 * 1000 + seconds * 1000 + milliseconds
        else:
            pattern = r"after (\d+) s (\d+) ms"
            match = re.search(pattern, line)
            if match:
                seconds, milliseconds = map(int, match.groups())
                single_time = seconds * 1000 + milliseconds
            else:
                pattern = r"after (\d+) ms"
                match = re.search(pattern, line)
                if match:
                    milliseconds = int(match.group(1))
                    single_time = milliseconds
        return name_match.group(1), single_time

    arkts_time_list = []
    total_time_list = []

    log_dir = output_dir / "rebuild"
    for log_file in log_dir.iterdir():
        if log_file.is_file() and log_file.name.startswith("out_"):
            with open(log_file, "r") as f:
                lines = f.readlines()
            time_sum = 0
            for line in lines:
                if line_pattern.match(line):
                    module, single_time = process_line(line)
                    time_sum += milliseconds_to_seconds(single_time)
                    rebuild["module"][module] = rebuild["module"].get(module, []) + [
                        milliseconds_to_seconds(single_time)
                    ]
            arkts_time_list.append(time_sum)
            total_compile_time = extract_total_time(log_file)
            total_time_list.append(milliseconds_to_seconds(total_compile_time))

    total_average_time = round(sum(total_time_list) / len(total_time_list), 3)
    total_average_stdev = round(statistics.stdev(total_time_list), 3)
    rebuild["total_compile"] = {
        "average": total_average_time,
        "stdev": total_average_stdev,
        "readable_time": milliseconds_to_readable(int(total_average_time * 1000)),
        "stdev_proportion_to_avg": round(
            total_average_stdev / total_average_time * 100, 2
        ),
    }

    arkts_average_time = round(sum(arkts_time_list) / len(arkts_time_list), 3)
    arkts_average_stdev = round(statistics.stdev(arkts_time_list), 3)
    rebuild["arkts_compile"] = {
        "average": arkts_average_time,
        "stdev": arkts_average_stdev,
        "readable_time": milliseconds_to_readable(int(arkts_average_time * 1000)),
        "stdev_proportion_to_avg": round(
            arkts_average_stdev / arkts_average_time * 100, 2
        ),
    }

    for module, times in rebuild["module"].items():
        rebuild["module"][module] = {
            "average": round(sum(times) / len(times), 3),
            "stdev": round(statistics.stdev(times), 3),
            "stdev_proportion_to_avg": (
                round(
                    round(statistics.stdev(times), 3)
                    / round(sum(times) / len(times), 3)
                    * 100,
                    2,
                )
            ),
        }

    data["rebuild"] = rebuild
    summary_path.write_text(json.dumps(data, indent=4))


def performance_result_parse(
    task: options.PerformanceTestTask, module: options.HapModule, compile_type: str
):
    output_dir = res_dir / f"{task.name}"
    summary_path = output_dir / "summary.json"
    compile_token = f"{module.name}_{compile_type}"
    if not output_dir.exists():
        test_logger.error(f"======> There is no result for: {task.name}")
        return

    data = json.loads(summary_path.read_text() if summary_path.exists() else "{}")
    arkts_average_duration, readable_time, arkts_average_stdev = calc_report(
        task, module.name, compile_token, output_dir / compile_token / "reports"
    )

    if f"{compile_token}" not in data:
        data[f"{compile_token}"] = {}

    data[f"{compile_token}"]["arkts_compile"] = {
        "average": arkts_average_duration,
        "stdev": arkts_average_stdev,
        "readable_time": readable_time,
        "stdev_proportion_to_avg": round(
            arkts_average_stdev / arkts_average_duration * 100, 2
        ),
    }

    summary_path.write_text(json.dumps(data, indent=4))

    log_dir = output_dir / compile_token
    for f in log_dir.iterdir():
        if f.is_file() and f.name.startswith("out_"):
            dump_to_json(
                f,
                summary_path,
                compile_token=compile_token,
                out_flag=f.name.split(".")[0],
            )

    calc_phase(summary_path, compile_token)


def generate_output(root_path: Path, task: options.PerformanceTestTask):
    with open(res_dir / f"{task.name}" / "summary.json", "r") as f:
        data = json.load(f)
    env = Environment(loader=FileSystemLoader(root_path / "resources"))
    template = env.get_template("performance_template.html")
    html_content = template.render(data=data)
    html_content = html_content.encode("utf-8")
    with open(res_dir / f"{task.name}" / "output.html", "wb") as f:
        f.write(html_content)


def generete_output_by_project(
    root_path: Path,
    test_tasks: list[options.PerformanceTestTask],
    sdk_generate_time,
):
    summary_path = res_dir / "summary.json"
    data = json.loads(summary_path.read_text() if summary_path.exists() else "{}")
    if sdk_generate_time is not None:
        data["sdk_generate_time"] = time.strftime(
            "%Y-%m-%d %H:%M:%S", sdk_generate_time
        )
    for task in test_tasks:
        project_summary = res_dir / f"{task.name}" / "summary.json"
        project_data = json.loads(
            project_summary.read_text() if project_summary.exists() else "{}"
        )
        data[task.name] = project_data

    summary_path.write_text(json.dumps(data, indent=4))
    env = Environment(loader=FileSystemLoader(root_path / "resources"))
    template = env.get_template("performance_template.html")
    html_content = template.render(data=data)
    html_content = html_content.encode("utf-8")
    with open(res_dir / "output.html", "wb") as f:
        f.write(html_content)

    day_str = time.strftime("%Y%m%d", sdk_generate_time)
    shutil.copytree(res_dir, res_dir.parent / f"result_{day_str}", dirs_exist_ok=True)

    generate_line_chart(res_dir.parent, 15, res_dir)
    shutil.copytree(res_dir, res_dir.parent / f"result_{day_str}", dirs_exist_ok=True)


def performance_test(root_path: Path, test_tasks: list[options.PerformanceTestTask]):
    global res_dir
    res_dir = root_path / "results" / "result"
    sdk_generate_time = time.localtime()
    for task in test_tasks:
        try:
            test_logger.info(f"======> Start to test: {task.name}")
            for module in task.modules:
                is_success = performance_module_compile(task, module)
                if not is_success:
                    test_logger.error(f"======> Failed to test: {task.name}")
                    break

                performance_module_incremental_compile(task, module)
                performance_result_parse(task, module, "full")
                performance_result_parse(task, module, "incremental")
            if is_success:
                performance_rebuild_compile(task)
                performance_rebuild_parse(task)
            test_logger.info(f"======> Finish to test: {task.name}")
        except Exception as e:
            test_logger.error(f"======> Failed to test: {task.name}")
            test_logger.error(f"======> Error: {e}")
    generete_output_by_project(root_path, test_tasks, sdk_generate_time)
