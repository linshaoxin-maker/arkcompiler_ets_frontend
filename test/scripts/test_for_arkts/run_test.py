import logging
from pathlib import Path
from datetime import datetime
from src.tools import clean
from src.tools import pack_result
from src.clone_repo import clone_repos
from src.test_task import test_rt
from src.test_task import test_tsc
from src.test_task import test_arktstest
from src.send_email import send_email
from src.send_email import add_manifest_to_html
from src.send_email import construct_rt_html_body
from src.send_email import construct_tsc_html_body
from src.send_email import construct_arktstest_html_body

script_dir = Path(__file__).parent

repo_file_path = script_dir / "resources" / "repo"
extern_dir_path = script_dir / "extern"

test_logger = logging.getLogger("test_logger")
test_logger.setLevel(logging.DEBUG)


def setup_logger(dir: Path):
    log_dir = dir / "logs"
    if not log_dir.exists():
        log_dir.mkdir(parents=True, exist_ok=True)
    today = datetime.now().strftime("%Y_%m_%d")
    log_file = log_dir / f"{today}_test.log"

    file_handler = logging.FileHandler(log_file)
    file_handler.setLevel(logging.DEBUG)
    file_formatter = logging.Formatter(
        "[%(asctime)s] [%(levelname)s] %(message)s", datefmt="%Y-%m-%d %H:%M:%S"
    )
    file_handler.setFormatter(file_formatter)

    console_handler = logging.StreamHandler()
    console_handler.setLevel(logging.DEBUG)
    console_formatter = logging.Formatter(
        "[%(asctime)s] [%(levelname)s] %(message)s", datefmt="%Y-%m-%d %H:%M:%S"
    )
    console_handler.setFormatter(console_formatter)

    test_logger.addHandler(file_handler)
    test_logger.addHandler(console_handler)


def main():
    setup_logger(script_dir)
    # clone repositories
    clone_repos(repo_file_path, extern_dir_path)

    template_html = script_dir / "resources" / "email_template.html"

    with open(template_html, "r", encoding="utf-8") as f:
        template = f.read()
    html_body = template

    html_body = add_manifest_to_html(html_body)

    rt_result_file = test_rt(extern_dir_path)
    if rt_result_file:
        html_body = construct_rt_html_body(rt_result_file, html_body)
        test_logger.info("================ RT Test Done ==================")
    else:
        test_logger.error("=============== RT Test Failed =================")

    tsc_result_file = test_tsc(extern_dir_path)
    if tsc_result_file:
        html_body = construct_tsc_html_body(tsc_result_file, html_body)
        test_logger.info("================ TSC Test Done =================")
    else:
        test_logger.error("============== TSC Test Failed =================")

    arktstest_result_file = test_arktstest(extern_dir_path)
    if arktstest_result_file:
        html_body = construct_arktstest_html_body(arktstest_result_file, html_body)
        test_logger.info("================ arkTSTest Done ================")
    else:
        test_logger.error("============== arkTSTest Failed ================")

    archive_path = pack_result(script_dir)

    send_email(html_body, archive_path)

    clean(script_dir, archive_path)


if __name__ == "__main__":
    main()
