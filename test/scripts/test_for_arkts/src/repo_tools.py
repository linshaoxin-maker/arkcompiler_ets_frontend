import logging
import re
import shutil
import subprocess
from pathlib import Path

import requests
from git import Repo

logger = logging.getLogger("test_logger")


def create_extern_dir(extern_dir_path: Path) -> None:
    extern_dir_path.mkdir(parents=True, exist_ok=True)
    gitignore_path = extern_dir_path / ".gitignore"
    if not gitignore_path.exists():
        gitignore_path.write_text("*\n")


def clone(
    url: str, branch_or_tag: str, directory: Path, patches: str, submodules: str
) -> None:
    logger.info(f"Cloning {url} to {directory}...")
    if directory.exists() and directory.is_dir():
        shutil.rmtree(directory)
    Path(directory).mkdir(parents=True, exist_ok=True)
    max_attempts = 5
    for attempt in range(1, max_attempts + 1):
        try:
            clone_command = ["git", "clone", "--depth", "1", url, directory]
            if branch_or_tag.startswith("tag") or branch_or_tag.startswith("branch"):
                clone_command.extend(["--branch", branch_or_tag.split(",")[1]])
            subprocess.run(clone_command, check=True)
            logger.info(f"Successfully cloned {url} on attempt {attempt}.")
            break
        except subprocess.CalledProcessError as e:
            logger.error(f"Attempt {attempt} to clone {url} failed. Error: {e}")
            if attempt == max_attempts:
                logger.error(
                    f"Failed to clone {url} after {max_attempts} attempts. Moving on to next repository."
                )
            else:
                logger.info("Retrying...")

    if patches != "no_patches":
        pass
    if submodules != "no_submodules":
        subprocess.run(
            ["git", "submodule", "update", "--init", "--recursive"],
            cwd=directory,
            check=True,
        )


def clone_repos(repo_file_path: Path, extern_dir_path: Path) -> None:
    create_extern_dir(extern_dir_path)
    with open(repo_file_path) as file:
        for line in file:
            name, url, branch_or_tag, version, patches, submodules = line.strip().split(
                ","
            )
            clone(
                url,
                branch_or_tag + "," + version,
                extern_dir_path / name,
                patches,
                submodules,
            )


def parse_pr_link(pr_link: str) -> tuple:
    match = re.match(r"https://gitee.com/([^/]+)/([^/]+)/pulls/(\d+)", pr_link)
    if match:
        owner, repo, pr_number = match.groups()
        return owner, repo, pr_number
    msg = f"Invalid PR link: {pr_link}"
    raise ValueError(msg)


def get_pr_info(owner: str, repo: str, pr_number: str) -> dict:
    url = f"https://gitee.com/api/v5/repos/{owner}/{repo}/pulls/{pr_number}"
    response = requests.get(url)
    if response.status_code != 200:
        response.raise_for_status()
    return response.json()


def clone_and_update_repo(pr_info: dict, repo_dir: Path) -> None:
    base_repo_url = pr_info["base"]["repo"]["html_url"]
    base_branch = pr_info["base"]["ref"]

    head_repo_url = pr_info["head"]["repo"]["html_url"]
    head_branch = pr_info["head"]["ref"]

    # Clone the base repository
    if not repo_dir.exists():
        repo = Repo.clone_from(base_repo_url, repo_dir)
    else:
        repo = Repo(repo_dir)

    # Checkout the base branch
    repo.git.checkout(base_branch)
    repo.git.pull("origin", base_branch)

    # Add the head repo as a remote if it's different from the base repo
    if head_repo_url != base_repo_url:
        remote_name = "fork"
        if remote_name not in [remote.name for remote in repo.remotes]:
            repo.create_remote(remote_name, head_repo_url)
        repo.git.fetch(remote_name, head_branch)
        head_ref = f"{remote_name}/{head_branch}"
    else:
        repo.git.fetch("origin", head_branch)
        head_ref = f"origin/{head_branch}"

    # Merge the head branch into the base branch
    repo.git.merge(head_ref)

    print(
        f'Successfully merged PR #{pr_info["number"]} from {head_repo_url} into {base_branch}'
    )
