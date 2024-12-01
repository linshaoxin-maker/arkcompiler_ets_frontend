import logging
from pathlib import Path

from src import tasks, utils

root_dir = Path(__file__).parent
logger = logging.getLogger("test_logger")


def main() -> None:
    env_check_result = utils.prepare(root_dir)
    if env_check_result:
        tasks.run_tests(root_dir)
    else:
        logger.error("This script should be run in WSL environment.")
    utils.stop_logger()


if __name__ == "__main__":
    main()
