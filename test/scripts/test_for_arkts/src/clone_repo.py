import shutil
import logging
import subprocess
from pathlib import Path

test_logger = logging.getLogger("test_logger")


def create_extern_dir(extern_dir_path: Path):
    extern_dir_path.mkdir(parents=True, exist_ok=True)
    gitignore_path = extern_dir_path / ".gitignore"
    if not gitignore_path.exists():
        gitignore_path.write_text("*\n")


def clone(url, branch_or_tag, directory: Path, patches, submodules):
    if directory.exists() and directory.is_dir():
        shutil.rmtree(directory)
    Path(directory).mkdir(parents=True, exist_ok=True)
    max_attempts = 5
    for attempt in range(1, max_attempts + 1):
        try:
            clone_command = ["git", "clone", "--depth", "1", url, directory]
            if branch_or_tag.startswith("tag"):
                clone_command.extend(["--branch", branch_or_tag.split(",")[1]])
            elif branch_or_tag.startswith("branch"):
                clone_command.extend(["--branch", branch_or_tag.split(",")[1]])
            subprocess.run(clone_command, check=True)
            test_logger.info(f"Successfully cloned {url} on attempt {attempt}.")
            break
        except subprocess.CalledProcessError as e:
            test_logger.error(f"Attempt {attempt} to clone {url} failed. Error: {e}")
            if attempt == max_attempts:
                test_logger.error(
                    f"Failed to clone {url} after {max_attempts} attempts. Moving on to next repository."
                )
            else:
                test_logger.info("Retrying...")

    if patches != "no_patches":
        pass
    if submodules != "no_submodules":
        subprocess.run(
            ["git", "submodule", "update", "--init", "--recursive"],
            cwd=directory,
            check=True,
        )


def clone_repos(repo_file_path: Path, extern_dir_path: Path):
    create_extern_dir(extern_dir_path)
    with open(repo_file_path) as file:
        for line in file:
            name, url, branch_or_tag, version, patches, submodules = line.strip().split(
                ","
            )
            test_logger.info(f"Cloning {name} from {url}...")
            clone(
                url,
                branch_or_tag + "," + version,
                extern_dir_path / name,
                patches,
                submodules,
            )
            test_logger.info(f"Finished cloning {name}.")
        test_logger.info("=========== All repositories cloned ============")
