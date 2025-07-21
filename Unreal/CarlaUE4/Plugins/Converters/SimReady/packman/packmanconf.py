# Copyright 2021-2024 NVIDIA CORPORATION

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#     http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Use this file to bootstrap packman into your Python environment. Simply
# add the path by doing sys.insert to where packmanconf.py is located and then execute:
#
# >>> import packmanconf
# >>> packmanconf.init()
#
# It will use the configured remote(s) and the version of packman in the same folder,
# giving you full access to the packman API via the following module
#
# >> import packmanapi
# >> dir(packmanapi)

import os
import platform
import sys


MIN_PYTHON_VERSION = (3, 10, 0)
MAX_PYTHON_VERSION = (3, 11, 2)


def is_valid_python_version(version: tuple[int, int, int] = sys.version_info[:3]):
    return MIN_PYTHON_VERSION <= version <= MAX_PYTHON_VERSION


def validate_python_version(version: tuple[int, int, int] = sys.version_info[:3]):
    if not is_valid_python_version(version):

        def ver_str(pyver):
            return ".".join(str(x) for x in pyver)

        raise RuntimeError(
            f"This version of packman requires Python {ver_str(MIN_PYTHON_VERSION)} "
            f"up to {ver_str(MAX_PYTHON_VERSION)}, but {ver_str(version)} was provided"
        )


def init():
    """Call this function to initialize the packman configuration.

    Calls to the packman API will work after successfully calling this function.

    Note:
        This function only needs to be called once during the execution of your
        program. Calling it repeatedly is harmless but wasteful.
        Compatibility with your Python interpreter is checked and upon failure
        the function will report what is required.

    Example:
        >>> import packmanconf
        >>> packmanconf.init()
        >>> import packmanapi
        >>> packmanapi.set_verbosity_level(packmanapi.VERBOSITY_HIGH)
    """
    validate_python_version()
    conf_dir = os.path.dirname(os.path.abspath(__file__))
    os.environ["PM_INSTALL_PATH"] = conf_dir
    packages_root = get_packages_root(conf_dir)
    version = get_version(conf_dir)
    module_dir = get_module_dir(conf_dir, packages_root, version)
    sys.path.insert(1, module_dir)


def get_packages_root(conf_dir: str) -> str:
    root = os.getenv("PM_PACKAGES_ROOT")
    if not root:
        platform_name = platform.system()
        if platform_name == "Windows":
            drive, _ = os.path.splitdrive(conf_dir)
            root = os.path.join(drive, "packman-repo")
        elif platform_name == "Darwin":
            # macOS
            root = os.path.join(
                os.path.expanduser("~"), "Library/Application Support/packman-cache"
            )
        elif platform_name == "Linux":
            try:
                cache_root = os.environ["XDG_HOME_CACHE"]
            except KeyError:
                cache_root = os.path.join(os.path.expanduser("~"), ".cache")
            return os.path.join(cache_root, "packman")
        else:
            raise RuntimeError(f"Unsupported platform '{platform_name}'")
    # make sure the path exists:
    os.makedirs(root, exist_ok=True)
    return root


def get_module_dir(conf_dir, packages_root: str, version: str) -> str:
    module_dir = os.path.join(packages_root, "packman-common", version)
    if not os.path.exists(module_dir):
        import tempfile

        tf = tempfile.NamedTemporaryFile(delete=False)
        target_name = tf.name
        tf.close()
        # Forced to change to https because some customers will simply not allow http,
        # even when it's used in a safe way (with download checksum verification).
        # See issue #367 for more background.
        url = f"https://bootstrap.packman.nvidia.com/packman-common@{version}.zip"
        print(f"Downloading '{url}' ...")
        import urllib.request

        urllib.request.urlretrieve(url, target_name)
        from importlib.machinery import SourceFileLoader

        # import module from path provided
        script_path = os.path.join(conf_dir, "bootstrap", "install_package.py")
        ip = SourceFileLoader("install_package", script_path).load_module()
        print("Unpacking ...")
        ip.install_common_module(target_name, module_dir)
        os.unlink(tf.name)
    return module_dir


def get_version(conf_dir: str):
    path = os.path.join(conf_dir, "packman")
    if not os.path.exists(path):  # in dev repo fallback
        path += ".sh"
    with open(path, "rt", encoding="utf8") as launch_file:
        for line in launch_file.readlines():
            if "PM_PACKMAN_VERSION" in line:
                _, value = line.split("=")
                return value.strip()
    raise RuntimeError(f"Unable to find 'PM_PACKMAN_VERSION' in '{path}'")
