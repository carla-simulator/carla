# Copyright 2019 NVIDIA CORPORATION

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#     http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import logging
import zipfile
import tempfile
import sys
import os
import stat
import time
import hashlib
from typing import Any, Callable, Union


RENAME_RETRY_COUNT = 100
RENAME_RETRY_DELAY = 0.1

logging.basicConfig(level=logging.WARNING, format="%(message)s")
logger = logging.getLogger("install_package")


def remove_directory_item(path):
    if os.path.islink(path) or os.path.isfile(path):
        try:
            os.remove(path)
        except PermissionError:
            # make sure we have access and try again:
            os.chmod(path, stat.S_IRWXU)
            os.remove(path)
    else:
        # try first to delete the dir because this will work for folder junctions, otherwise we would follow the junctions and cause destruction!
        clean_out_folder = False
        try:
            # make sure we have access preemptively - this is necessary because recursing into a directory without permissions
            # will only lead to heart ache
            os.chmod(path, stat.S_IRWXU)
            os.rmdir(path)
        except OSError:
            clean_out_folder = True

        if clean_out_folder:
            # we should make sure the directory is empty
            names = os.listdir(path)
            for name in names:
                fullname = os.path.join(path, name)
                remove_directory_item(fullname)
            # now try to again get rid of the folder - and not catch if it raises:
            os.rmdir(path)


class StagingDirectory:
    def __init__(self, staging_path):
        self.staging_path = staging_path
        self.temp_folder_path = None
        os.makedirs(staging_path, exist_ok=True)

    def __enter__(self):
        self.temp_folder_path = tempfile.mkdtemp(prefix="ver-", dir=self.staging_path)
        return self

    def get_temp_folder_path(self):
        return self.temp_folder_path

    # this function renames the temp staging folder to folder_name, it is required that the parent path exists!
    def promote_and_rename(self, folder_name):
        abs_dst_folder_name = os.path.join(self.staging_path, folder_name)
        os.rename(self.temp_folder_path, abs_dst_folder_name)

    def __exit__(self, type, value, traceback):
        # Remove temp staging folder if it's still there (something went wrong):
        path = self.temp_folder_path
        if os.path.isdir(path):
            remove_directory_item(path)


def rename_folder(staging_dir: StagingDirectory, folder_name: str):
    try:
        staging_dir.promote_and_rename(folder_name)
    except OSError as exc:
        # if we failed to rename because the folder now exists we can assume that another packman process
        # has managed to update the package before us - in all other cases we re-raise the exception
        abs_dst_folder_name = os.path.join(staging_dir.staging_path, folder_name)
        if os.path.exists(abs_dst_folder_name):
            logger.warning(
                f"Directory {abs_dst_folder_name} already present, package installation already completed"
            )
        else:
            raise


def call_with_retry(
    op_name: str, func: Callable, retry_count: int = 3, retry_delay: float = 20
) -> Any:
    retries_left = retry_count
    while True:
        try:
            return func()
        except (OSError, IOError) as exc:
            logger.warning(f"Failure while executing {op_name} [{str(exc)}]")
            if retries_left:
                retry_str = "retry" if retries_left == 1 else "retries"
                logger.warning(
                    f"Retrying after {retry_delay} seconds"
                    f" ({retries_left} {retry_str} left) ..."
                )
                time.sleep(retry_delay)
            else:
                logger.error("Maximum retries exceeded, giving up")
                raise
            retries_left -= 1


def rename_folder_with_retry(staging_dir: StagingDirectory, folder_name):
    dst_path = os.path.join(staging_dir.staging_path, folder_name)
    call_with_retry(
        f"rename {staging_dir.get_temp_folder_path()} -> {dst_path}",
        lambda: rename_folder(staging_dir, folder_name),
        RENAME_RETRY_COUNT,
        RENAME_RETRY_DELAY,
    )


def generate_sha256_for_file(file_path: Union[str, os.PathLike]) -> str:
    """Returns the SHA-256 hex digest for the file at `file_path`"""
    hash = hashlib.sha256()
    # Read the file in binary mode and update the hash object with data
    with open(file_path, "rb") as file:
        for chunk in iter(lambda: file.read(4096), b""):
            hash.update(chunk)
    return hash.hexdigest()


def install_common_module(package_path, install_path):
    COMMON_SHA256 = "457678c37d81e2a87037cfa4c6e149ff48f37e55c38cf2c072300d9c6d7e3fda"
    package_sha256 = generate_sha256_for_file(package_path)
    if package_sha256 != COMMON_SHA256:
        raise RuntimeError(
            f"Package at '{package_path}' must have a sha256 of '{COMMON_SHA256}' "
            f"but was found to have '{package_sha256}'"
        )
    staging_path, version = os.path.split(install_path)
    with StagingDirectory(staging_path) as staging_dir:
        output_folder = staging_dir.get_temp_folder_path()
        with zipfile.ZipFile(package_path, allowZip64=True) as zip_file:
            zip_file.extractall(output_folder)

            # attempt the rename operation
            rename_folder_with_retry(staging_dir, version)

    print(f"Package successfully installed to {install_path}")


if __name__ == "__main__":
    executable_paths = os.getenv("PATH")
    paths_list = executable_paths.split(os.path.pathsep) if executable_paths else []
    target_path_np = os.path.normpath(sys.argv[2])
    target_path_np_nc = os.path.normcase(target_path_np)
    for exec_path in paths_list:
        if os.path.normcase(os.path.normpath(exec_path)) == target_path_np_nc:
            raise RuntimeError(f"packman will not install to executable path '{exec_path}'")
    install_common_module(sys.argv[1], target_path_np)
