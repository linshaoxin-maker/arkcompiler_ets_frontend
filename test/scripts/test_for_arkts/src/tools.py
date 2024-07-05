import logging
import zipfile
import shutil
from datetime import datetime
from pathlib import Path

test_logger = logging.getLogger("test_logger")


def pack_result(root: Path):
    result_dir = root / "result"
    log_dir = root / "logs"
    today = datetime.now().strftime("%Y_%m_%d")
    log_file_name = f"{today}_test.log"
    log_file = log_dir / log_file_name

    if not result_dir.exists():
        test_logger.warning(f"Directory {result_dir} does not exist, skip packing.")
        return None

    if log_file.exists() and log_file.is_file():
        shutil.copy2(log_file, result_dir / log_file_name)

    archive_name = f"{today}_ArkTS_{result_dir.name}.zip"
    archive_path = result_dir.parent / archive_name

    with zipfile.ZipFile(archive_path, "w", zipfile.ZIP_DEFLATED) as zipf:
        for file in result_dir.glob('**/*'):
            zipf.write(file, file.relative_to(result_dir.parent))

    test_logger.info(f"Directory {result_dir} has been packed into {archive_path}")
    return archive_path


def clean(root: Path, archive_path: Path):
    result_path = root / "result"
    extern_path = root / "extern"
    if result_path.exists() and result_path.is_dir():
        shutil.rmtree(result_path)
        test_logger.info(f"Deleted directory: {result_path}")

    if extern_path.exists() and extern_path.is_dir():
        shutil.rmtree(extern_path)
        test_logger.info(f"Deleted directory: {extern_path}")

    if archive_path.exists() and archive_path.is_file():
        archive_path.unlink()
        test_logger.info(f"Deleted file: {archive_path}")
