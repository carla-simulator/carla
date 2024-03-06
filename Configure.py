from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path
from collections import deque
import subprocess, tarfile, zipfile, argparse, requests, psutil, shutil, glob, json, sys, os, stat
from distutils.spawn import find_executable

# Constants:
FALLBACK_CARLA_VERSION_STRING = '0.9.15'
EXE_EXT = '.exe' if os.name == 'nt' else ''
LIB_PREFIX = '' if os.name == 'nt' else 'lib'
LIB_EXT = '.lib' if os.name == 'nt' else '.a'
OBJ_EXT = '.obj' if os.name == 'nt' else '.o'
SHELL_EXT = '.bat' if os.name == 'nt' else '.sh'
WORKSPACE_PATH = Path(__file__).parent.resolve()
LIBCARLA_PATH = WORKSPACE_PATH / 'LibCarla'
LIBCARLA_SOURCE_PATH = LIBCARLA_PATH / 'source'
PYTHON_API_PATH = WORKSPACE_PATH / 'PythonAPI' / 'carla'
EXAMPLES_PATH = WORKSPACE_PATH / 'Examples'
UTIL_PATH = WORKSPACE_PATH / 'Util'
DOCKER_UTILS_PATH = UTIL_PATH / 'DockerUtils'
PATCHES_PATH = UTIL_PATH / 'Patches'
CARLA_UE_PATH = WORKSPACE_PATH / 'Unreal' / 'CarlaUnreal'
CARLA_UE_PLUGIN_ROOT_PATH = CARLA_UE_PATH / 'Plugins'
CARLA_UE_PLUGIN_PATH = CARLA_UE_PLUGIN_ROOT_PATH / 'Carla'
CARLA_UE_PLUGIN_DEPENDENCIES_PATH = CARLA_UE_PLUGIN_ROOT_PATH / 'CarlaDependencies'
CARLA_UE_CONTENT_PATH = CARLA_UE_PATH / 'Content'
CARLA_UE_CONTENT_CARLA_PATH = CARLA_UE_CONTENT_PATH / 'Carla'
CARLA_UE_CONTENT_VERSIONS_FILE_PATH = WORKSPACE_PATH / 'Util' / 'ContentVersions.json'
LOGICAL_PROCESSOR_COUNT = psutil.cpu_count(logical = True)
DEFAULT_PARALLELISM = LOGICAL_PROCESSOR_COUNT + (2 if LOGICAL_PROCESSOR_COUNT >= 8 else 0)
READTHEDOCS_URL_SUFFIX = 'how_to_build_on_windows/\n' if os.name == "nt" else 'build_linux/\n'
DEFAULT_BOOST_TOOLSET = 'msvc-14.3' if os.name == 'nt' else 'clang'
DEFAULT_ERROR_MESSAGE = (
  '\n'
  'Ok, an error ocurred, don\'t panic!\n'
  'We have different platforms where you can find some help:\n'
  '\n'
  '- Make sure you have read the documentation:\n'
  f'  https://carla.readthedocs.io/en/latest/{READTHEDOCS_URL_SUFFIX}'
  '\n'
  '- If the problem persists, submit an issue on our GitHub page:\n'
  '  https://github.com/carla-simulator/carla/issues\n'
  '\n'
  '- Or just use our Discord server!\n'
  '  We\'ll be glad to help you there:\n'
  '  https://discord.gg/42KJdRj\n'
)



def FindExecutable(candidates : list):
  for e in candidates:
    executable_path = find_executable(e)
    if executable_path:
      return e
  return None

DEFAULT_C_COMPILER = FindExecutable([
  'cl',
  'clang-cl',
] if os.name == 'nt' else [
  'clang-16',
])

DEFAULT_CPP_COMPILER = FindExecutable([
  'cl',
  'clang-cl',
] if os.name == 'nt' else [
  'clang++-16',
])

DEFAULT_LINKER = FindExecutable([
  'link',
  'llvm-link',
] if os.name == 'nt' else [
  'lld-16',
])

DEFAULT_LIB = FindExecutable([
  'lib',
  'llvm-lib',
  'llvm-ar'
] if os.name == 'nt' else [
  'llvm-ar-16',
])

DEFAULT_C_STANDARD = 11
DEFAULT_CPP_STANDARD = 20

argp = argparse.ArgumentParser(description = __doc__)

def AddCLIFlag(name : str, help : str):
  argp.add_argument(f'--{name}', action = 'store_true', help = help)

def AddCLIStringOption(name : str, default : str, help : str):
  argp.add_argument(f'--{name}', type = str, default = str(default), help = f'{help} (default = "{default}").')

def ADDCLIIntOption(name : str, default : int, help : str):
  argp.add_argument(f'--{name}', type = int, default = int(default), help = f'{help} (default = {default}).')

AddCLIFlag(
  'all',
  'Build all targets.')
AddCLIFlag(
  'import',
  f'Import maps and assets from "{WORKSPACE_PATH / "Import"}" into Unreal.')
AddCLIFlag(
  'package',
  'Build a packaged version of CARLA ready for distribution.')
AddCLIFlag(
  'docs',
  'Build the CARLA documentation, through Doxygen.')
AddCLIFlag(
  'clean',
  'Delete all build files.')
AddCLIFlag(
  'rebuild',
  'Delete all build files and recompiles.')
AddCLIFlag(
  'check-libcarla',
  'Run unit the test suites for LibCarla')
AddCLIFlag(
  'check-python-api',
  'Run unit the test suites for PythonAPI')
AddCLIFlag(
  'check',
  'Run unit the test suites for LibCarla and PythonAPI')
AddCLIFlag(
  'carla-ue',
  'Build the CARLA Unreal Engine plugin and project.')
AddCLIFlag(
  'update-ue-assets',
  'Download the CARLA Unreal Engine assets.')
AddCLIFlag(
  'python-api',
  'Build the CARLA Python API.')
AddCLIFlag(
  'libcarla-client',
  'Build the LibCarla Client module.')
AddCLIFlag(
  'libcarla-server',
  'Build the LibCarla Server module.')
AddCLIFlag(
  'update-deps',
  'Download all project dependencies.')
AddCLIFlag(
  'build-deps',
  'Build all project dependencies.')
AddCLIFlag(
  'configure-sequential',
  'Whether to disable parallelism in the configuration script.')
AddCLIFlag(
  'no-log',
  'Whether to disable saving logs.')
AddCLIFlag(
  'pytorch',
  'Whether to enable PyTorch.')
AddCLIFlag(
  'chrono',
  'Whether to enable Chrono.')
AddCLIFlag(
  'gtest',
  'Whether to enable gtest.')
AddCLIFlag(
  'carsim',
  'Whether to enable CarSim')
AddCLIFlag(
  'ros2',
  'Whether to enable ROS2')
AddCLIFlag(
  'unity-build',
  'Whether to enable Unity Build for Unreal Engine projects.')
AddCLIFlag(
  'osm2odr',
  'Whether to enable OSM2ODR.')
AddCLIFlag(
  'osm-world-renderer',
  'Whether to enable OSM World Renderer.')
AddCLIFlag(
  'nv-omniverse',
  'Whether to enable the NVIDIA Omniverse Plugin.')
AddCLIFlag(
  'march-native',
  'Whether to add "-march=native" to C/C++ compile flags.')
AddCLIFlag(
  'rss',
  'Whether to enable RSS.')
ADDCLIIntOption(
  'parallelism',
  DEFAULT_PARALLELISM,
  'Set the configure/build parallelism.')
ADDCLIIntOption(
  'c-standard',
  DEFAULT_C_STANDARD,
  'Set the target C standard.')
ADDCLIIntOption(
  'cpp-standard',
  DEFAULT_CPP_STANDARD,
  'Set the target C++ standard.')
AddCLIStringOption(
  'c-compiler',
  DEFAULT_C_COMPILER,
  'Set the target C compiler.')
AddCLIStringOption(
  'cpp-compiler',
  DEFAULT_CPP_COMPILER,
  'Set the target C++ compiler.')
AddCLIStringOption(
  'linker',
  DEFAULT_LINKER,
  'Set the target linker.')
AddCLIStringOption(
  'ar',
  DEFAULT_LIB,
  'Set the target ar/lib tool.')
AddCLIStringOption(
  'version',
  FALLBACK_CARLA_VERSION_STRING,
  'Override the CARLA version.')
AddCLIStringOption(
  'generator',
  'Ninja',
  'Set the CMake generator.')
AddCLIStringOption(
  'build-path',
  WORKSPACE_PATH / 'Build',
  'Set the CARLA build path.')
AddCLIStringOption(
  'boost-toolset',
  DEFAULT_BOOST_TOOLSET,
  'Set the target boost toolset.')
AddCLIStringOption(
  'ue-path',
  os.getenv(
    'UNREAL_ENGINE_PATH',
    '<Could not automatically infer Unreal Engine source path using the "UNREAL_ENGINE_PATH" environment variable>'),
  'Set the path of Unreal Engine.')

ARGS_SYNC_PATH = WORKSPACE_PATH / 'ArgsSync.json'


def SyncArgs():
  argv = argparse.Namespace()
  if __name__ == '__main__':
    argv = argp.parse_args()
    with open(ARGS_SYNC_PATH, 'w') as file:
      json.dump(argv.__dict__, file)
  else:
    with open(ARGS_SYNC_PATH, 'r') as file:
      argv.__dict__ = json.load(file)
  return argv

ARGV = SyncArgs()

SEQUENTIAL = ARGV.configure_sequential
ENABLE_OSM2ODR = ARGV.osm2odr or ARGV.all
ENABLE_GTEST = ARGV.gtest
ENABLE_OSM_WORLD_RENDERER = ARGV.osm_world_renderer or ARGV.all
ENABLE_CARLA_UE = ARGV.carla_ue or ARGV.all
ENABLE_PYTHON_API = ARGV.python_api or ARGV.all
ENABLE_LIBCARLA_CLIENT = ARGV.libcarla_client or ARGV.all
ENABLE_LIBCARLA_SERVER = ARGV.libcarla_server or ARGV.all
ENABLE_LIBCARLA = any([
  ENABLE_CARLA_UE,
  ENABLE_PYTHON_API,
  ENABLE_LIBCARLA_CLIENT,
  ENABLE_LIBCARLA_SERVER,
])

ENABLE_CARSIM = ARGV.carsim
ENABLE_CHRONO = ARGV.chrono
ENABLE_ROS2 = ARGV.ros2
ENABLE_UNITY_BUILD = ARGV.unity_build
ENABLE_NVIDIA_OMNIVERSE = ARGV.nv_omniverse
ENABLE_RSS = ARGV.rss

UPDATE_DEPENDENCIES = ARGV.update_deps or ARGV.all
BUILD_DEPENDENCIES = ARGV.build_deps or ARGV.all
UPDATE_CARLA_UE_ASSETS = ARGV.update_ue_assets or ARGV.all
PARALLELISM = ARGV.parallelism
# Root paths:
CARLA_VERSION_STRING = ARGV.version
BUILD_PATH = Path(ARGV.build_path)
BUILD_TEMP_PATH = BUILD_PATH / 'Temp'
LOG_PATH = BUILD_PATH / 'BuildLogs'
DEPENDENCIES_PATH = BUILD_PATH / 'Dependencies'
LIBCARLA_BUILD_PATH = BUILD_PATH / 'LibCarla'
LIBCARLA_INSTALL_PATH = WORKSPACE_PATH / 'Install' / 'LibCarla'
# Language options
C_COMPILER = FindExecutable([ARGV.c_compiler])
if not C_COMPILER:
  sys.exit(f"Error C Compiler not found: {ARGV.c_compiler}")
CPP_COMPILER = FindExecutable([ARGV.cpp_compiler])
if not CPP_COMPILER:
  sys.exit(f"Error CPP Compiler not found: {ARGV.cpp_compiler}")
LINKER = FindExecutable([ARGV.linker])
if not LINKER:
  sys.exit(f"Error LINKER not found: {ARGV.linker}")
LIB = FindExecutable([ARGV.ar])
if not LIB:
  sys.exit(f"Error AR not found {ARGV.ar}")
C_STANDARD = ARGV.c_standard
CPP_STANDARD = ARGV.cpp_standard
C_COMPILER_CLI_TYPE = 'msvc' if ('cl' in C_COMPILER and os.name == 'nt') else 'gnu'
CPP_COMPILER_CLI_TYPE = 'msvc' if ('cl' in CPP_COMPILER and os.name == 'nt') else 'gnu'
C_COMPILER_IS_CLANG = 'clang' in C_COMPILER
CPP_COMPILER_IS_CLANG = 'clang' in CPP_COMPILER
C_ENABLE_MARCH_NATIVE = ARGV.march_native and C_COMPILER_CLI_TYPE == 'gnu'
CPP_ENABLE_MARCH_NATIVE = ARGV.march_native and CPP_COMPILER_CLI_TYPE == 'gnu'
LIB_IS_AR = 'ar' in LIB
# Unreal Engine
UNREAL_ENGINE_PATH = Path(ARGV.ue_path)
# Dependencies:
# Boost
BOOST_USE_SUPERPROJECT = True
BOOST_VERSION = (1, 84, 0)
BOOST_VERSION_MAJOR, BOOST_VERSION_MINOR, BOOST_VERSION_PATCH = BOOST_VERSION
BOOST_VERSION_STRING = f'{BOOST_VERSION_MAJOR}.{BOOST_VERSION_MINOR}.{BOOST_VERSION_PATCH}'
BOOST_TOOLSET = ARGV.boost_toolset
BOOST_SOURCE_PATH = DEPENDENCIES_PATH / 'boost-source'
BOOST_BUILD_PATH = DEPENDENCIES_PATH / 'boost-build'
BOOST_INSTALL_PATH = DEPENDENCIES_PATH / 'boost-install'
BOOST_INCLUDE_PATH = BOOST_INSTALL_PATH / 'include'
BOOST_LIBRARY_PATH = BOOST_INSTALL_PATH / 'lib'
BOOST_B2_PATH = BOOST_SOURCE_PATH / f'b2{EXE_EXT}'

BOOST_ALGORITHM_SOURCE_PATH = DEPENDENCIES_PATH / 'boost-algorithm-source'
BOOST_ALGORITHM_BUILD_PATH = DEPENDENCIES_PATH / 'boost-algorithm-build'
BOOST_ALGORITHM_INSTALL_PATH = DEPENDENCIES_PATH / 'boost-algorithm-install'
BOOST_ALGORITHM_INCLUDE_PATH = BOOST_ALGORITHM_INSTALL_PATH / 'include'
BOOST_ALGORITHM_LIB_PATH = BOOST_ALGORITHM_INSTALL_PATH / 'lib'

BOOST_ASIO_SOURCE_PATH = DEPENDENCIES_PATH / 'boost-asio-source'
BOOST_ASIO_BUILD_PATH = DEPENDENCIES_PATH / 'boost-asio-build'
BOOST_ASIO_INSTALL_PATH = DEPENDENCIES_PATH / 'boost-asio-install'
BOOST_ASIO_INCLUDE_PATH = BOOST_ASIO_INSTALL_PATH / 'include'
BOOST_ASIO_LIB_PATH = BOOST_ASIO_INSTALL_PATH / 'lib'

BOOST_DATE_SOURCE_PATH = DEPENDENCIES_PATH / 'boost-date-time-source'
BOOST_DATE_BUILD_PATH = DEPENDENCIES_PATH / 'boost-date-time-build'
BOOST_DATE_INSTALL_PATH = DEPENDENCIES_PATH / 'boost-date-install'
BOOST_DATE_INCLUDE_PATH = BOOST_DATE_INSTALL_PATH / 'include'
BOOST_DATE_LIB_PATH = BOOST_DATE_INSTALL_PATH / 'lib'

BOOST_GEOMETRY_SOURCE_PATH = DEPENDENCIES_PATH / 'boost-geometry-source'
BOOST_GEOMETRY_BUILD_PATH = DEPENDENCIES_PATH / 'boost-geometry-build'
BOOST_GEOMETRY_INSTALL_PATH = DEPENDENCIES_PATH / 'boost-geometry-install'
BOOST_GEOMETRY_INCLUDE_PATH = BOOST_GEOMETRY_INSTALL_PATH / 'include'
BOOST_GEOMETRY_LIB_PATH = BOOST_GEOMETRY_INSTALL_PATH / 'lib'

BOOST_GIL_SOURCE_PATH = DEPENDENCIES_PATH / 'boost-gil-source'
BOOST_GIL_BUILD_PATH = DEPENDENCIES_PATH / 'boost-gil-build'
BOOST_GIL_INSTALL_PATH = DEPENDENCIES_PATH / 'boost-gil-install'
BOOST_GIL_INCLUDE_PATH = BOOST_GIL_INSTALL_PATH / 'include'
BOOST_GIL_LIB_PATH = BOOST_GIL_INSTALL_PATH / 'lib'

BOOST_ITERATOR_SOURCE_PATH = DEPENDENCIES_PATH / 'boost-iterator-source'
BOOST_ITERATOR_BUILD_PATH = DEPENDENCIES_PATH / 'boost-iterator-build'
BOOST_ITERATOR_INSTALL_PATH = DEPENDENCIES_PATH / 'boost-iterator-install'
BOOST_ITERATOR_INCLUDE_PATH = BOOST_ITERATOR_INSTALL_PATH / 'include'
BOOST_ITERATOR_LIB_PATH = BOOST_ITERATOR_INSTALL_PATH / 'lib'

BOOST_PYTHON_SOURCE_PATH = DEPENDENCIES_PATH / 'boost-python-source'
BOOST_PYTHON_BUILD_PATH = DEPENDENCIES_PATH / 'boost-python-build'
BOOST_PYTHON_INSTALL_PATH = DEPENDENCIES_PATH / 'boost-python-install'
BOOST_PYTHON_INCLUDE_PATH = BOOST_PYTHON_INSTALL_PATH / 'include'
BOOST_PYTHON_LIB_PATH = BOOST_PYTHON_INSTALL_PATH / 'lib'
# Eigen
EIGEN_SOURCE_PATH = DEPENDENCIES_PATH / 'eigen-source'
EIGEN_BUILD_PATH = DEPENDENCIES_PATH / 'eigen-build'
EIGEN_INSTALL_PATH = DEPENDENCIES_PATH / 'eigen-install'
EIGEN_INCLUDE_PATH = EIGEN_INSTALL_PATH / 'include'
# Chrono
CHRONO_SOURCE_PATH = DEPENDENCIES_PATH / 'chrono-source'
CHRONO_BUILD_PATH = DEPENDENCIES_PATH / 'chrono-build'
CHRONO_INSTALL_PATH = DEPENDENCIES_PATH / 'chrono-install'
CHRONO_INCLUDE_PATH = CHRONO_INSTALL_PATH / 'include'
CHRONO_LIBRARY_PATH = CHRONO_INSTALL_PATH / 'lib'
# Google Test
GTEST_SOURCE_PATH = DEPENDENCIES_PATH / 'gtest-source'
GTEST_BUILD_PATH = DEPENDENCIES_PATH / 'gtest-build'
GTEST_INSTALL_PATH = DEPENDENCIES_PATH / 'gtest-install'
GTEST_INCLUDE_PATH = GTEST_INSTALL_PATH / 'include'
GTEST_LIBRARY_PATH = GTEST_INSTALL_PATH / 'lib'
# ZLib
ZLIB_SOURCE_PATH = DEPENDENCIES_PATH / 'zlib-source'
ZLIB_BUILD_PATH = DEPENDENCIES_PATH / 'zlib-build'
ZLIB_INSTALL_PATH = DEPENDENCIES_PATH / 'zlib-install'
ZLIB_INCLUDE_PATH = ZLIB_INSTALL_PATH / 'include'
ZLIB_LIBRARY_PATH = ZLIB_INSTALL_PATH / 'lib'
ZLIB_LIB_PATH = ZLIB_LIBRARY_PATH / (f'zlibstatic{LIB_EXT}' if os.name == 'nt' else f'libz{LIB_EXT}')
# LibPNG
LIBPNG_SOURCE_PATH = DEPENDENCIES_PATH / 'libpng-source'
LIBPNG_BUILD_PATH = DEPENDENCIES_PATH / 'libpng-build'
LIBPNG_INSTALL_PATH = DEPENDENCIES_PATH / 'libpng-install'
LIBPNG_INCLUDE_PATH = LIBPNG_INSTALL_PATH / 'include'
LIBPNG_LIBRARY_PATH = LIBPNG_INSTALL_PATH / 'lib'
# SQLite
SQLITE_SOURCE_PATH = DEPENDENCIES_PATH / 'sqlite-source'
SQLITE_BUILD_PATH = DEPENDENCIES_PATH / 'sqlite-build'
SQLITE_LIBRARY_PATH = SQLITE_BUILD_PATH
SQLITE_INCLUDE_PATH = SQLITE_SOURCE_PATH
SQLITE_LIB_PATH = SQLITE_BUILD_PATH / f'{LIB_PREFIX}sqlite{LIB_EXT}'
SQLITE_EXE_PATH = SQLITE_BUILD_PATH / f'sqlite{EXE_EXT}'
# Proj
PROJ_SOURCE_PATH = DEPENDENCIES_PATH / 'proj-source'
PROJ_BUILD_PATH = DEPENDENCIES_PATH / 'proj-build'
PROJ_INSTALL_PATH = DEPENDENCIES_PATH / 'proj-install'
PROJ_INCLUDE_PATH = PROJ_INSTALL_PATH / 'include'
PROJ_LIBRARY_PATH = PROJ_INSTALL_PATH / 'lib'
# Recast & Detour
RECAST_SOURCE_PATH = DEPENDENCIES_PATH / 'recast-source'
RECAST_BUILD_PATH = DEPENDENCIES_PATH / 'recast-build'
RECAST_INSTALL_PATH = DEPENDENCIES_PATH / 'recast-install'
RECAST_INCLUDE_PATH = RECAST_INSTALL_PATH / 'include'
RECAST_LIBRARY_PATH = RECAST_INSTALL_PATH / 'lib'
# RPCLib
RPCLIB_SOURCE_PATH = DEPENDENCIES_PATH / 'rpclib-source'
RPCLIB_BUILD_PATH = DEPENDENCIES_PATH / 'rpclib-build'
RPCLIB_INSTALL_PATH = DEPENDENCIES_PATH / 'rpclib-install'
RPCLIB_INCLUDE_PATH = RPCLIB_INSTALL_PATH / 'include'
RPCLIB_LIBRARY_PATH = RPCLIB_INSTALL_PATH / 'lib'
# Xerces-C
XERCESC_SOURCE_PATH = DEPENDENCIES_PATH / 'xercesc-source'
XERCESC_BUILD_PATH = DEPENDENCIES_PATH / 'xercesc-build'
XERCESC_INSTALL_PATH = DEPENDENCIES_PATH / 'xercesc-install'
XERCESC_LIBRARY_PATH = XERCESC_INSTALL_PATH / 'lib'
XERCESC_INCLUDE_PATH = XERCESC_INSTALL_PATH / 'include'
# LibOSMScout
LIBOSMSCOUT_SOURCE_PATH = DEPENDENCIES_PATH / 'libosmscout-source'
LIBOSMSCOUT_BUILD_PATH = DEPENDENCIES_PATH / 'libosmscout-build'
LIBOSMSCOUT_INSTALL_PATH = DEPENDENCIES_PATH / 'libosmscout-install'
LIBOSMSCOUT_INCLUDE_PATH = LIBOSMSCOUT_INSTALL_PATH / 'include'
LIBOSMSCOUT_LIBRARY_PATH = LIBOSMSCOUT_INSTALL_PATH / 'lib'
# LunaSVG
LUNASVG_SOURCE_PATH = DEPENDENCIES_PATH / 'lunasvg-source'
LUNASVG_BUILD_PATH = DEPENDENCIES_PATH / 'lunasvg-build'
LUNASVG_INSTALL_PATH = DEPENDENCIES_PATH / 'lunasvg-install'
LUNASVG_INCLUDE_PATH = LUNASVG_INSTALL_PATH / 'include'
LUNASVG_LIBRARY_PATH = LUNASVG_INSTALL_PATH / 'lib'
# SUMO
SUMO_SOURCE_PATH = DEPENDENCIES_PATH / 'sumo-source'
SUMO_BUILD_PATH = DEPENDENCIES_PATH / 'sumo-build'
SUMO_INSTALL_PATH = DEPENDENCIES_PATH / 'sumo-install'
SUMO_INCLUDE_PATH = SUMO_INSTALL_PATH / 'include'
SUMO_LIBRARY_PATH = SUMO_INSTALL_PATH / 'lib'
# Nvidia Omniverse
NV_OMNIVERSE_PLUGIN_PATH = UNREAL_ENGINE_PATH / 'Engine' / 'Plugins' / 'Marketplace' / 'NVIDIA' / 'Omniverse'
NV_OMNIVERSE_PATCH_PATH = PATCHES_PATH / 'omniverse_4.26'

# Basic IO functions:



def Log(message):
  message = str(message)
  message += '\n'
  print(message, end='')



def LaunchSubprocess(
    cmd : list,
    working_directory : Path = None,
    log = None):
  return subprocess.run(
    cmd,
    stdout = log,
    stderr = log,
    cwd = working_directory)



def LaunchSubprocessImmediate(
    cmd : list,
    working_directory : Path = None,
    log_name : str = None):
  sp = None
  if not ARGV.no_log and log_name != None:
    with open(LOG_PATH / f'{log_name}.log', 'w') as file:
      sp = subprocess.run(
        cmd,
        cwd = working_directory,
        stdout = file,
        stderr = file)
  else:
    sp = subprocess.run(
      cmd,
      cwd = working_directory,
      stdout = subprocess.PIPE,
      stderr = subprocess.PIPE)
  sp.check_returncode()



# Convenience classes for listing dependencies:

class Download:
  def __init__(self, url : str):
    self.url = url

class GitRepository:
  def __init__(self, url : str, tag_or_branch : str = None, commit : str = None):
    self.url = url
    self.tag_or_branch = tag_or_branch
    self.commit = commit

class Dependency:
  def __init__(self, name : str, *sources):
    self.name = name
    self.sources = [ *sources ]
    assert all(
      type(e) is Download or
      type(e) is GitRepository
      for e in self.sources)

class DependencyUEPlugin(Dependency):
  def __init__(self, name: str, *sources):
    super().__init__(name, *sources)



DEFAULT_DEPENDENCIES = [
  Dependency(
    'eigen',
    GitRepository('https://gitlab.com/libeigen/eigen.git', tag_or_branch = '3.4.0')),
  Dependency(
    'libpng',
    GitRepository('https://github.com/glennrp/libpng.git', tag_or_branch = 'v1.6.40')),
  
  Dependency(
    'zlib',
    GitRepository('https://github.com/madler/zlib.git'),),
  Dependency(
    'sqlite',
    Download('https://www.sqlite.org/2021/sqlite-amalgamation-3340100.zip')),
  Dependency(
    'rpclib',
    GitRepository('https://github.com/carla-simulator/rpclib.git', tag_or_branch = 'carla')),
  Dependency(
    'recast',
    GitRepository('https://github.com/carla-simulator/recastnavigation.git', tag_or_branch = 'carla')),
] + [
  Dependency(
    'boost',
    Download(f'https://github.com/boostorg/boost/releases/download/boost-{BOOST_VERSION_STRING}/boost-{BOOST_VERSION_STRING}.zip')),
] if BOOST_USE_SUPERPROJECT else [
  Dependency(
    'boost-algorithm',
    GitRepository('https://github.com/boostorg/algorithm.git')),
  Dependency(
    'boost-asio',
    GitRepository('https://github.com/boostorg/asio.git')),
  Dependency(
    'boost-iterator',
    GitRepository('https://github.com/boostorg/iterator.git')),
  Dependency(
    'boost-python',
    GitRepository('https://github.com/boostorg/python.git')),
  Dependency(
    'boost-geometry',
    GitRepository('https://github.com/boostorg/geometry.git')),
  Dependency(
    'boost-date-time',
    GitRepository('https://github.com/boostorg/date_time.git')),
  Dependency(
    'boost-gil',
    GitRepository('https://github.com/boostorg/gil.git')),
]

if ENABLE_GTEST:
  DEFAULT_DEPENDENCIES.append(Dependency(
      'gtest',
      GitRepository('https://github.com/google/googletest.git', tag_or_branch = 'v1.14.0')))

CHRONO_DEPENDENCIES = [
  Dependency(
    'chrono',
    GitRepository('https://github.com/projectchrono/chrono.git', tag_or_branch = '8.0.0')),
]

OSM_WORLD_RENDERER_DEPENDENCIES = [
  Dependency(
    'libosmscout',
    GitRepository('https://github.com/Framstag/libosmscout.git')),
  Dependency(
    'lunasvg',
    GitRepository('https://github.com/sammycage/lunasvg.git')),
]

OSM2ODR_DEPENDENCIES = [
  Dependency(
    'proj',
    GitRepository('https://github.com/OSGeo/PROJ.git', tag_or_branch = '7.2.1')),
  Dependency(
    'xercesc',
    GitRepository('https://github.com/apache/xerces-c.git', tag_or_branch = 'v3.2.4')),
  Dependency(
    'sumo',
    GitRepository('https://github.com/carla-simulator/sumo.git', tag_or_branch = 'carla_osm2odr')),
]

CARLA_UE_DEPENDENCIES = [
  DependencyUEPlugin(
    'StreetMap',
    GitRepository(
      'https://github.com/carla-simulator/StreetMap.git',
      tag_or_branch = 'UE5Native')),
]

CARLA_UE_ASSETS_DEPENDENCIES = [
  Dependency(
    'Carla',
    GitRepository(
      'https://bitbucket.org/carla-simulator/carla-content.git',
      tag_or_branch = 'marcel/5.3' # @CARLAUE5 This branch should be removed once merged.
    ))
]



class Task:
  def __init__(self, name : str, in_edges : list, body, *args):
    assert all([ type(e) == Task for e in in_edges ])
    self.name = name
    self.body = body
    self.args = args
    self.in_edge_done_count = 0
    self.in_edges = in_edges
    self.out_edges = [] # Filled right before task graph execution.
    self.done = False

  def CreateSubprocessTask(name : str, in_edges : list, command : list):
    return Task(name, in_edges, LaunchSubprocessImmediate, command, None, name)
  
  def CreateCMakeConfigureDefaultCommandLine(source_path : Path, build_path : Path) -> list:
    cpp_flags_release = ''
    if os.name == 'nt':
      cpp_flags_release = '/MD'
    if CPP_ENABLE_MARCH_NATIVE:
      cpp_flags_release += ' -march=native'
    cmd = [
      'cmake',
      '-G', ARGV.generator,
      '-S', source_path,
      '-B', build_path,
      '-DCMAKE_C_COMPILER=' + C_COMPILER,
      '-DCMAKE_CXX_COMPILER=' + CPP_COMPILER,
      '-DCMAKE_BUILD_TYPE=Release',
      f'-DCMAKE_CXX_FLAGS_RELEASE={cpp_flags_release}',
    ]
    if os.name != 'nt':
      cmd.append('-DCMAKE_POSITION_INDEPENDENT_CODE=ON')
      CPP_ABI_VERSIONS = f'-fuse-ld={LINKER} -nostdinc++ -nostdlib++'
      CPP_LIB= f'-isystem {UNREAL_ENGINE_PATH}/Engine/Source/ThirdParty/Unix/LibCxx/include/c++/v1 -L {UNREAL_ENGINE_PATH}/Engine/Source/ThirdParty/Unix/LibCxx/lib/Unix/x86_64-unknown-linux-gnu/ -lc++'
      cmd.append(f'-DCMAKE_CXX_FLAGS={CPP_ABI_VERSIONS} {CPP_LIB}')
    return cmd

  def CreateCMakeConfigureDefault(name : str, in_edges : list, source_path : Path, build_path : Path, *args, install_path : Path = None):
    cmd = Task.CreateCMakeConfigureDefaultCommandLine(source_path, build_path)
    if install_path != None:
      cmd.append('-DCMAKE_INSTALL_PREFIX=' + str(install_path))
    cmd.extend([ *args, source_path ])
    return Task.CreateSubprocessTask(name, in_edges, cmd)

  def CreateCMakeBuildDefault(name : str, in_edges : list, build_path : Path, *args):
    cmd = [ 'cmake', '--build', build_path ]
    cmd.extend([ *args ])
    return Task.CreateSubprocessTask(name, in_edges, cmd)
  
  def CreateCMakeInstallDefault(name : str, in_edges : list, build_path : Path, install_path : Path, *args):
    cmd = [ 'cmake', '--install', build_path, '--prefix', install_path ]
    cmd.extend([ *args ])
    return Task.CreateSubprocessTask(name, in_edges, cmd)
  
  def Run(self):
    self.body(*self.args)
    
  def ToString(self):
    return f'{[ e.name for e in self.in_edges ]} -> {self.name}'



class TaskGraph:

  def __init__(self, parallelism : int = PARALLELISM):
    self.sequential = SEQUENTIAL
    self.parallelism = parallelism
    self.tasks = []
    self.sources = []
    self.task_map = {}
  
  def Reset(self):
    self.tasks = []
    self.sources = []
    self.task_map = {}

  def Add(self, task : Task):
    self.tasks.append(task)
    if len(task.in_edges) == 0:
      self.sources.append(task)
    self.task_map[task.name] = self.tasks[-1]
    return task
  
  def Validate(self):
    return True

  def ToString(self):
    return '\n'.join([ e.ToString() for e in self.tasks ])
  
  def Print(self):
    Log(self.ToString())

  def Execute(self, sequential : bool = False):
    if len(self.tasks) == 0:
      return
    Log('-- Running task graph --')
    self.Print()
    assert self.Validate()
    prior_sequential = self.sequential
    self.sequential = sequential
    try:
      for task in self.tasks:
        for in_edge in task.in_edges:
          assert in_edge != None
          in_edge.out_edges.append(task)
      task_queue = deque()
      active_count = 0
      done_count = 0
      def UpdateOutEdges(task):
        nonlocal task_queue
        if len(task.out_edges) == 0:
          return
        for out in task.out_edges:
          assert out.in_edge_done_count < len(out.in_edges)
          out.in_edge_done_count += 1
          if out.in_edge_done_count == len(out.in_edges):
            task_queue.append(out)
      def Flush():
        nonlocal futures
        nonlocal future_map
        nonlocal done_count
        nonlocal active_count
        if active_count != 0:
          done = [ e for e in as_completed(futures) ]
          done_tasks = [ future_map[e] for e in done ]
          for e in done_tasks:
            e.done = True
            Log(f'> {task.name} - DONE')
            UpdateOutEdges(e)
          assert active_count == len(done_tasks)
          done_count += len(done_tasks)
          active_count = 0
          future_map = {}
          futures = []
      assert len(set(self.sources)) == len(self.sources)
      task_queue.extend(self.sources)
      with ProcessPoolExecutor(self.parallelism) as pool:
        futures = []
        future_map = {}
        while len(task_queue) != 0:
          while len(task_queue) != 0 and active_count < self.parallelism:
            task = task_queue.popleft()
            Log(f'> {task.name} - STARTED')
            if not self.sequential:
              active_count += 1
              future = pool.submit(task.Run)
              future_map[future] = task
              futures.append(future)
            else:
              task.Run()
              Log(f'> {task.name} - DONE')
              task.done = True
              done_count += 1
              UpdateOutEdges(task)
          Flush()
      if done_count != len(self.tasks):
        pending_tasks = []
        for e in self.tasks:
          if not e.done:
            pending_tasks.append(e)
        Log(f'> {len(self.tasks) - done_count} did not complete: {pending_tasks}.')
        assert False
    finally:
      Log('-- Done --')
      self.sequential = prior_sequential
      self.Reset()



def UpdateGitRepository(path : Path, url : str, branch : str = None, commit : str = None):
  if path.exists():
    LaunchSubprocessImmediate([ 'git', '-C', str(path), 'pull' ])
  else:
    cmd = [ 'git', '-C', str(path.parent), 'clone', '--depth', '1', '--single-branch' ]
    if branch != None:
      cmd.extend([ '-b', branch ])
    cmd.extend([ url, path.stem ])
    LaunchSubprocessImmediate(cmd)
  if commit != None:
    LaunchSubprocessImmediate([ 'git', '-C', str(path), 'fetch' ])
    LaunchSubprocessImmediate([ 'git', '-C', str(path), 'checkout', commit ])



def DownloadDependency(name : str, path : Path, url : str):
  # Download:
  try:
    temp_path = Path(str(path) + '.tmp')
    with requests.Session() as session:
      with session.get(url, stream = True) as result:
        result.raise_for_status()
        with open(temp_path, 'wb') as file:
          shutil.copyfileobj(result.raw, file)
  except Exception as err:
    Log(f'Failed to download dependency "{name}": {err}')
  # Extract:
  try:
    extract_path = path.with_name(path.name + '-temp')
    if url.endswith('.tar.gz'):
      archive_path = temp_path.with_suffix('.tar.gz')
      temp_path.rename(archive_path)
      with tarfile.open(archive_path) as file:
        file.extractall(extract_path)
    elif url.endswith('.zip'):
      archive_path = temp_path.with_suffix('.zip')
      temp_path.rename(archive_path)
      zipfile.ZipFile(archive_path).extractall(extract_path)
    # Peel unnecessary outer directory:
    entries = [ file for file in extract_path.iterdir() ]
    if len(entries) == 1 and entries[0].is_dir():
      Path(entries[0]).rename(path)
      if extract_path.exists():
        extract_path.rmdir()
    else:
      extract_path.rename(path)
  except Exception as err:
    Log(f'Failed to extract dependency "{name}": {err}')



def UpdateDependency(
    dep : Dependency,
    download_path : Path = None):
  name = dep.name
  if download_path == None:
    download_path = DEPENDENCIES_PATH / f'{name}-source'
  if type(dep) is DependencyUEPlugin: # Override download path if we're dealing with an Unreal Engine Plugin.
    download_path = CARLA_UE_PLUGIN_ROOT_PATH / name
  for source in dep.sources:
    try:
      if type(source) is GitRepository:
        UpdateGitRepository(download_path, source.url, source.tag_or_branch, source.commit)
      elif type(source) is Download:
        if download_path.exists():
          Log(f'Dependency "{name}" already present. Delete "{download_path}" if you wish for it to be downloaded again.')
        else:
          DownloadDependency(name, download_path, source.url)
      return
    finally:
      pass
  Log(f'Failed to update dependency "{name}".')
  assert False



def UpdateDependencies(task_graph : TaskGraph):
  DEPENDENCIES_PATH.mkdir(exist_ok = True)
  unique_deps = set(DEFAULT_DEPENDENCIES)
  if ENABLE_CARLA_UE:
    unique_deps.update(CARLA_UE_DEPENDENCIES)
  if ENABLE_OSM_WORLD_RENDERER:
    unique_deps.update(OSM_WORLD_RENDERER_DEPENDENCIES)
  if ENABLE_OSM2ODR:
    unique_deps.update(OSM2ODR_DEPENDENCIES)
  if ENABLE_CHRONO:
    unique_deps.update(CHRONO_DEPENDENCIES)
  return [
    task_graph.Add(Task(f'{dep.name}-update', [], UpdateDependency, dep)) for dep in unique_deps
  ]



def CleanDownloadsMain():
  for ext in [ '*.tmp', '*.zip', '*.tar.gz' ]:
    for e in DEPENDENCIES_PATH.glob(ext):
      e.unlink(missing_ok = True)



def CleanDownloads(task_graph : TaskGraph):
  return task_graph.Add(Task('clean-downloads', [], CleanDownloadsMain))



def ConfigureBoost():
  if BOOST_B2_PATH.exists():
    return
  bootstrap_file_path = BOOST_SOURCE_PATH / f'bootstrap{SHELL_EXT}'
  log_name = 'boost-configure'
  if os.name != 'nt':
    try:
      os.chmod(bootstrap_file_path, stat.S_IEXEC | stat.S_IREAD)
      os.chmod(BOOST_SOURCE_PATH / 'tools/build/src/engine/build.sh', stat.S_IEXEC | stat.S_IREAD)
    except Exception as e:
      with open(LOG_PATH / f'{log_name}.log', 'w') as log_file:
        log_file.write(f"Cannot give permisions: {str(e)}")
  
  LaunchSubprocessImmediate(
    [ bootstrap_file_path ],
    working_directory = BOOST_SOURCE_PATH,
    log_name = log_name)



def BuildAndInstallBoost():

  LaunchSubprocessImmediate([
    BOOST_B2_PATH,
    f'-j{PARALLELISM}',
    'architecture=x86',
    'address-model=64',
    f'toolset={BOOST_TOOLSET}',
    'variant=release',
    'link=static',
    'runtime-link=shared',
    'threading=multi',
    '--layout=versioned',
    '--with-system',
    '--with-python',
    '--with-date_time',
    f'--build-dir={BOOST_BUILD_PATH}',
    f'--prefix={BOOST_INSTALL_PATH}',
    f'--libdir={BOOST_LIBRARY_PATH}',
    f'--includedir={BOOST_INCLUDE_PATH}',
    'install'
  ],
  working_directory = BOOST_SOURCE_PATH,
  log_name = 'boost-build')
  if (BOOST_INCLUDE_PATH / 'boost').exists():
    return
  candidates = glob.glob(f'{BOOST_INCLUDE_PATH}/**/boost', recursive = True)
  candidates.sort()
  boost_path = Path(candidates[0])
  shutil.move(boost_path, BOOST_INCLUDE_PATH / 'boost')
  boost_path.parent.rmdir()



def BuildSQLite():
  SQLITE_BUILD_PATH.mkdir(exist_ok = True)
  sqlite_sources = glob.glob(f'{SQLITE_SOURCE_PATH}/**/*.c', recursive = True)
  sqlite_sources = [ Path(e) for e in sqlite_sources ]
  if not SQLITE_EXE_PATH.exists():
    cmd = [ C_COMPILER ]
    cmd.extend([
      f'-std=c89',
      f'-fuse-ld={LINKER}',
      '-O2',
      '-Wno-error=int-conversion',
    ] if C_COMPILER_CLI_TYPE == 'gnu' else [
      f'/std:c{C_STANDARD}',
      '/O2',
      '/Qvec',
      '/MD',
      '/EHsc',
    ])
    if C_ENABLE_MARCH_NATIVE:
      cmd.append('-march=native')
    cmd.extend(sqlite_sources)
    if C_COMPILER_CLI_TYPE == 'msvc':
      cmd.append(f'/Fe{SQLITE_EXE_PATH}')
    else:
      cmd.extend([ '-o', SQLITE_EXE_PATH ])
    LaunchSubprocessImmediate(cmd, log_name = 'sqlite-exe-build')
  if not SQLITE_LIB_PATH.exists():
    objs = []
    BUILD_TEMP_PATH.mkdir(exist_ok = True)
    for e in sqlite_sources:
      cmd = [
        C_COMPILER,
        '-c' if C_COMPILER_CLI_TYPE == 'gnu' else '/c',
      ]
      cmd.extend([
        f'-std=c89',
        '-march=native',
        '-O2',
        '-Wno-error=int-conversion',
        '-fPIC',
        '-nostdinc++',
        '-nostdlib++',
        '-isystem',
        f'{UNREAL_ENGINE_PATH}/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v22_clang-16.0.6-centos7/x86_64-unknown-linux-gnu/usr/include/',
        '-L', 
        f'{UNREAL_ENGINE_PATH}/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v22_clang-16.0.6-centos7/x86_64-unknown-linux-gnu/usr/lib64',
        '-lc',
      ] if C_COMPILER_CLI_TYPE == 'gnu' else [
        f'/std:c{C_STANDARD}',
        '/O2',
        '/Qvec',
        '/MD',
        '/EHsc',
      ])
      obj_path = BUILD_TEMP_PATH / f'{e.name}{OBJ_EXT}'
      if C_COMPILER_CLI_TYPE != 'gnu':
        cmd.extend([ e, f'/Fo{obj_path}' ])
      else:
        cmd.extend([ e, '-o', obj_path ])
      LaunchSubprocessImmediate(cmd, log_name = f'sqlite-{e.stem}-build')
      objs.append(obj_path)
    cmd = [
      LIB
    ]
    if os.name == 'nt':
      cmd.append(f'/OUT:{SQLITE_LIB_PATH}')
    else:
      cmd.extend(['rsc', SQLITE_LIB_PATH])
    cmd.extend(objs)
    LaunchSubprocessImmediate(cmd, log_name = 'sqlite-lib-build')



def ConfigureSUMO():
  xercesc_path_candidates = glob.glob(f'{XERCESC_INSTALL_PATH}/**/{LIB_PREFIX}xerces-c*{LIB_EXT}', recursive=True)
  if len(xercesc_path_candidates) == 0:
    raise Exception('Could not configure SUMO since xerces-c could not be found.')
  if len(xercesc_path_candidates) > 1:
    Log('Warning: multiple xerces-c libraries found.')
  xercesc_path_candidates.sort()
  XERCESC_LIB_PATH = xercesc_path_candidates[0]
  cmd = Task.CreateCMakeConfigureDefaultCommandLine(
    SUMO_SOURCE_PATH,
    SUMO_BUILD_PATH)
  proj_candidates = glob.glob(str(PROJ_INSTALL_PATH / 'lib' / '**' / f'*{LIB_PREFIX}proj{LIB_EXT}'), recursive = True)
  if len(proj_candidates) == 0:
    raise Exception('Could not configure SUMO since PROJ could not be found.')
  PROJ_LIB_PATH = proj_candidates[0]
  cmd.extend([
    f'-DZLIB_INCLUDE_DIR={ZLIB_INCLUDE_PATH}',
    f'-DZLIB_LIBRARY={ZLIB_LIB_PATH}',
    f'-DPROJ_INCLUDE_DIR={PROJ_INSTALL_PATH}/include',
    f'-DPROJ_LIBRARY={PROJ_LIB_PATH}',
    f'-DXercesC_INCLUDE_DIR={XERCESC_INSTALL_PATH}/include',
    f'-DXercesC_LIBRARY={XERCESC_LIB_PATH}',
    # '-DSUMO_LIBRARIES=OFF',
    # '-DPROFILING=OFF',
    # '-DPPROF=OFF',
    # '-DCOVERAGE=OFF',
    # '-DSUMO_UTILS=OFF',
    # '-DFMI=OFF',
    # '-DNETEDIT=OFF',
    # '-DENABLE_JAVA_BINDINGS=OFF',
    # '-DENABLE_CS_BINDINGS=OFF',
    # '-DCCACHE_SUPPORT=OFF',
  ])
  LaunchSubprocessImmediate(cmd, log_name='sumo-configure')



def BuildDependencies(task_graph : TaskGraph):
  
  # There are some dependencies that need sqlite to be built before configuring.
  build_sqlite = task_graph.Add(Task('sqlite-build', [], BuildSQLite))

  configure_zlib = task_graph.Add(Task.CreateCMakeConfigureDefault(
    'zlib-configure',
    [],
    ZLIB_SOURCE_PATH,
    ZLIB_BUILD_PATH,
    install_path = ZLIB_INSTALL_PATH))
  
  build_zlib = task_graph.Add(Task.CreateCMakeBuildDefault(
    'zlib-build',
    [ configure_zlib ],
    ZLIB_BUILD_PATH))

  # Configure step:

  if BOOST_USE_SUPERPROJECT:
    task_graph.Add(Task('boost-configure', [], ConfigureBoost))
  else:
    task_graph.Add(Task.CreateCMakeConfigureDefault(
      'boost-algorithm-configure',
      [],
      BOOST_ALGORITHM_SOURCE_PATH,
      BOOST_ALGORITHM_BUILD_PATH,
      install_path = BOOST_ALGORITHM_INSTALL_PATH))

    task_graph.Add(Task.CreateCMakeConfigureDefault(
      'boost-asio-configure',
      [],
      BOOST_ASIO_SOURCE_PATH,
      BOOST_ASIO_BUILD_PATH,
      install_path = BOOST_ASIO_INSTALL_PATH))

    task_graph.Add(Task.CreateCMakeConfigureDefault(
      'boost-date-time-configure',
      [],
      BOOST_DATE_SOURCE_PATH,
      BOOST_DATE_BUILD_PATH,
      install_path = BOOST_DATE_INSTALL_PATH))

    task_graph.Add(Task.CreateCMakeConfigureDefault(
      'boost-geometry-configure',
      [],
      BOOST_GEOMETRY_SOURCE_PATH,
      BOOST_GEOMETRY_BUILD_PATH,
      install_path = BOOST_GEOMETRY_INSTALL_PATH))

    task_graph.Add(Task.CreateCMakeConfigureDefault(
      'boost-gil-configure',
      [],
      BOOST_GIL_SOURCE_PATH,
      BOOST_GIL_BUILD_PATH,
      install_path = BOOST_GIL_INSTALL_PATH))

    task_graph.Add(Task.CreateCMakeConfigureDefault(
      'boost-iterator-configure',
      [],
      BOOST_ITERATOR_SOURCE_PATH,
      BOOST_ITERATOR_BUILD_PATH,
      install_path = BOOST_ITERATOR_INSTALL_PATH))

    task_graph.Add(Task.CreateCMakeConfigureDefault(
      'boost-python-configure',
      [],
      BOOST_PYTHON_SOURCE_PATH,
      BOOST_PYTHON_BUILD_PATH,
      install_path = BOOST_PYTHON_INSTALL_PATH))

  task_graph.Add(Task.CreateCMakeInstallDefault(
    'zlib-install',
    [ build_zlib ],
    ZLIB_BUILD_PATH,
    ZLIB_INSTALL_PATH))

  if ENABLE_GTEST:  
    task_graph.Add(Task.CreateCMakeConfigureDefault(
      'gtest-configure',
      [],
      GTEST_SOURCE_PATH,
      GTEST_BUILD_PATH))
  
  task_graph.Add(Task.CreateCMakeConfigureDefault(
    'libpng-configure',
    [],
    LIBPNG_SOURCE_PATH,
    LIBPNG_BUILD_PATH,
    '-DPNG_TESTS=OFF',
    '-DPNG_SHARED=OFF',
    '-DPNG_TOOLS=OFF',
    '-DPNG_BUILD_ZLIB=ON',
    f'-DZLIB_INCLUDE_DIRS={ZLIB_INCLUDE_PATH}',
    f'-DZLIB_LIBRARIES={ZLIB_LIB_PATH}'))
  
  task_graph.Add(Task.CreateCMakeConfigureDefault(
    'recast-configure',
    [],
    RECAST_SOURCE_PATH,
    RECAST_BUILD_PATH,
    '-DRECASTNAVIGATION_DEMO=OFF',
    '-DRECASTNAVIGATION_TESTS=OFF',
    '-DRECASTNAVIGATION_EXAMPLES=OFF'))
  
  task_graph.Add(Task.CreateCMakeConfigureDefault(
    'rpclib-configure',
    [],
    RPCLIB_SOURCE_PATH,
    RPCLIB_BUILD_PATH,
    '-DRPCLIB_BUILD_TESTS=OFF',
    '-DRPCLIB_GENERATE_COMPDB=OFF',
    '-DRPCLIB_BUILD_EXAMPLES=OFF',
    '-DRPCLIB_ENABLE_LOGGING=OFF',
    '-DRPCLIB_ENABLE_COVERAGE=OFF',
    '-DRPCLIB_MSVC_STATIC_RUNTIME=OFF',
    '-DCMAKE_POSITION_INDEPENDENT_CODE=ON'))
  
  if ENABLE_OSM_WORLD_RENDERER:
    task_graph.Add(Task.CreateCMakeConfigureDefault(
      'libosmscout-configure',
      [],
      LIBOSMSCOUT_SOURCE_PATH,
      LIBOSMSCOUT_BUILD_PATH,
      '-DOSMSCOUT_BUILD_TOOL_STYLEEDITOR=OFF',
      '-DOSMSCOUT_BUILD_TOOL_OSMSCOUT2=OFF',
      '-DOSMSCOUT_BUILD_TESTS=OFF',
      '-DOSMSCOUT_BUILD_CLIENT_QT=OFF',
      '-DOSMSCOUT_BUILD_DEMOS=OFF'))
    
    task_graph.Add(Task.CreateCMakeConfigureDefault(
      'lunasvg-configure',
      [],
      LUNASVG_SOURCE_PATH,
      LUNASVG_BUILD_PATH))
    
  if ENABLE_CHRONO:
    task_graph.Add(Task.CreateCMakeConfigureDefault(
      'chrono-configure',
      [],
      CHRONO_SOURCE_PATH,
      CHRONO_BUILD_PATH,
      f'-DEIGEN3_INCLUDE_DIR={EIGEN_SOURCE_PATH}',
      '-DENABLE_MODULE_VEHICLE=ON'))
  
  if ENABLE_OSM2ODR:
    task_graph.Add(Task.CreateCMakeConfigureDefault(
      'proj-configure',
      [ build_sqlite ],
      PROJ_SOURCE_PATH,
      PROJ_BUILD_PATH,
      f'-DSQLITE3_INCLUDE_DIR={SQLITE_SOURCE_PATH}',
      f'-DSQLITE3_LIBRARY={SQLITE_LIB_PATH}',
      f'-DEXE_SQLITE3={SQLITE_EXE_PATH}',
      '-DWIN32_LEAN_AND_MEAN=1',
      '-DVC_EXTRALEAN=1',
      '-DNOMINMAX=1',
      '-DENABLE_TIFF=OFF',
      '-DENABLE_CURL=OFF',
      '-DBUILD_SHARED_LIBS=OFF',
      '-DBUILD_PROJSYNC=OFF',
      '-DBUILD_PROJINFO=OFF',
      '-DBUILD_CCT=OFF',
      '-DBUILD_CS2CS=OFF',
      '-DBUILD_GEOD=OFF',
      '-DBUILD_GIE=OFF',
      '-DBUILD_PROJ=OFF',
      '-DBUILD_TESTING=OFF',
      install_path = PROJ_INSTALL_PATH))
    
    configure_xercesc = task_graph.Add(Task.CreateCMakeConfigureDefault(
    'xercesc-configure',
    [],
    XERCESC_SOURCE_PATH,
    XERCESC_BUILD_PATH,
    '-DBUILD_SHARED_LIBS=OFF'))
    
    # SUMO requires that Proj and Xerces be built and installed before its configure step:
    build_xercesc = task_graph.Add(Task.CreateCMakeBuildDefault(
      'xercesc-build',
      [ configure_xercesc ],
      XERCESC_BUILD_PATH))
    
    task_graph.Add(Task.CreateCMakeInstallDefault(
      'xercesc-install',
      [ build_xercesc ],
      XERCESC_BUILD_PATH,
      XERCESC_INSTALL_PATH))

  # We wait for all pending tasks to finish here, then we'll switch to sequential task execution for the build step.
  task_graph.Execute()

  # Build:
  
  if BOOST_USE_SUPERPROJECT:
    task_graph.Add(Task('boost-build', [], BuildAndInstallBoost))
  else:
    task_graph.Add(Task.CreateCMakeBuildDefault('boost-algorithm-build', [], BOOST_ALGORITHM_BUILD_PATH))
    task_graph.Add(Task.CreateCMakeBuildDefault('boost-asio-build', [], BOOST_ASIO_BUILD_PATH))
    task_graph.Add(Task.CreateCMakeBuildDefault('boost-date-build', [], BOOST_DATE_BUILD_PATH))
    task_graph.Add(Task.CreateCMakeBuildDefault('boost-geometry-build', [], BOOST_GEOMETRY_BUILD_PATH))
    task_graph.Add(Task.CreateCMakeBuildDefault('boost-gil-build', [], BOOST_GIL_BUILD_PATH))
    task_graph.Add(Task.CreateCMakeBuildDefault('boost-iterator-build', [], BOOST_ITERATOR_BUILD_PATH))
    task_graph.Add(Task.CreateCMakeBuildDefault('boost-python-build', [], BOOST_PYTHON_BUILD_PATH))

  if ENABLE_GTEST:
    task_graph.Add(Task.CreateCMakeBuildDefault('gtest-build', [], GTEST_BUILD_PATH))
  task_graph.Add(Task.CreateCMakeBuildDefault('libpng-build', [], LIBPNG_BUILD_PATH))
  task_graph.Add(Task.CreateCMakeBuildDefault('recast-build', [], RECAST_BUILD_PATH))
  task_graph.Add(Task.CreateCMakeBuildDefault('rpclib-build', [], RPCLIB_BUILD_PATH))

  if ENABLE_OSM2ODR:
    build_proj = task_graph.Add(Task.CreateCMakeBuildDefault('proj-build', [], PROJ_BUILD_PATH))
    install_proj = task_graph.Add(Task.CreateCMakeInstallDefault('proj-install', [ build_proj ], PROJ_BUILD_PATH, PROJ_INSTALL_PATH))
    configure_sumo = task_graph.Add(Task('sumo-configure', [ install_proj ], ConfigureSUMO))
    task_graph.Add(Task.CreateCMakeBuildDefault('sumo-build', [ configure_sumo ], SUMO_BUILD_PATH))
    task_graph.Add(Task.CreateCMakeBuildDefault('xercesc-build', [], XERCESC_BUILD_PATH))

  if ENABLE_OSM_WORLD_RENDERER:
    task_graph.Add(Task.CreateCMakeBuildDefault('lunasvg-build', [], LUNASVG_BUILD_PATH))
    task_graph.Add(Task.CreateCMakeBuildDefault('libosmscout-build', [], LIBOSMSCOUT_BUILD_PATH))

  if ENABLE_CHRONO:
    task_graph.Add(Task.CreateCMakeBuildDefault('chrono-build', [], CHRONO_BUILD_PATH))

  task_graph.Execute(sequential = True) # The underlying build system should already parallelize.

  # Install:
  if not BOOST_USE_SUPERPROJECT:
    task_graph.Add(Task.CreateCMakeBuildDefault('boost-algorithm-install', [], BOOST_ALGORITHM_BUILD_PATH))
    task_graph.Add(Task.CreateCMakeBuildDefault('boost-asio-install', [], BOOST_ASIO_BUILD_PATH))
    task_graph.Add(Task.CreateCMakeBuildDefault('boost-date-install', [], BOOST_DATE_BUILD_PATH))
    task_graph.Add(Task.CreateCMakeBuildDefault('boost-geometry-install', [], BOOST_GEOMETRY_BUILD_PATH))
    task_graph.Add(Task.CreateCMakeBuildDefault('boost-gil-install', [], BOOST_GIL_BUILD_PATH))
    task_graph.Add(Task.CreateCMakeBuildDefault('boost-iterator-install', [], BOOST_ITERATOR_BUILD_PATH))
    task_graph.Add(Task.CreateCMakeBuildDefault('boost-python-install', [], BOOST_PYTHON_BUILD_PATH))
  if ENABLE_GTEST:
    task_graph.Add(Task.CreateCMakeInstallDefault('gtest-install', [], GTEST_BUILD_PATH, GTEST_INSTALL_PATH))
  task_graph.Add(Task.CreateCMakeInstallDefault('libpng-install', [], LIBPNG_BUILD_PATH, LIBPNG_INSTALL_PATH))
  task_graph.Add(Task.CreateCMakeInstallDefault('recast-install', [], RECAST_BUILD_PATH, RECAST_INSTALL_PATH))
  task_graph.Add(Task.CreateCMakeInstallDefault('rpclib-install', [], RPCLIB_BUILD_PATH, RPCLIB_INSTALL_PATH))
  if ENABLE_OSM_WORLD_RENDERER:
    task_graph.Add(Task.CreateCMakeInstallDefault('lunasvg-install', [], LUNASVG_BUILD_PATH, LUNASVG_INSTALL_PATH))
    task_graph.Add(Task.CreateCMakeInstallDefault('libosmscout-install', [], LIBOSMSCOUT_BUILD_PATH, LIBOSMSCOUT_INSTALL_PATH))
  if ENABLE_OSM2ODR:
    task_graph.Add(Task.CreateCMakeInstallDefault('sumo-install', [], SUMO_BUILD_PATH, SUMO_INSTALL_PATH))
    task_graph.Add(Task.CreateCMakeInstallDefault('proj-install', [], PROJ_BUILD_PATH, PROJ_INSTALL_PATH))
    task_graph.Add(Task.CreateCMakeInstallDefault('xercesc-install', [], XERCESC_BUILD_PATH, XERCESC_INSTALL_PATH))
  if ENABLE_CHRONO:
    task_graph.Add(Task.CreateCMakeInstallDefault('chrono-install', [], CHRONO_BUILD_PATH, CHRONO_INSTALL_PATH))
  task_graph.Execute()



def BuildLibCarlaMain(task_graph : TaskGraph):
  configure_libcarla = task_graph.Add(Task.CreateCMakeConfigureDefault(
    'libcarla-configure',
    [],
    WORKSPACE_PATH,
    LIBCARLA_BUILD_PATH,
    f'-DCARLA_DEPENDENCIES_PATH={DEPENDENCIES_PATH}',
    f'-DBUILD_LIBCARLA_SERVER={"ON" if ARGV.libcarla_server else "OFF"}',
    f'-DBUILD_LIBCARLA_CLIENT={"ON" if ARGV.libcarla_client else "OFF"}',
    f'-DBUILD_OSM_WORLD_RENDERER={"ON" if ENABLE_OSM_WORLD_RENDERER else "OFF"}',
    f'-DLIBCARLA_PYTORCH={"ON" if ARGV.pytorch else "OFF"}'))
  build_libcarla = task_graph.Add(Task.CreateCMakeBuildDefault(
    'libcarla-build',
    [ configure_libcarla ],
    LIBCARLA_BUILD_PATH))
  return task_graph.Add(Task.CreateCMakeInstallDefault(
    'libcarla-install',
    [ build_libcarla ],
    LIBCARLA_BUILD_PATH,
    LIBCARLA_INSTALL_PATH))



def BuildPythonAPIMain():
  content = ''
  with open(PYTHON_API_PATH / 'setup.py.in', 'r') as file:
    content = file.read()
  content = content.format_map(globals())
  if os.name == 'nt':
    content = content.replace(os.sep, '\\\\')
  with open(PYTHON_API_PATH / 'setup.py', 'w') as file:
    file.write(content)
  LaunchSubprocessImmediate(
    [ sys.executable, 'setup.py', 'bdist_egg', 'bdist_wheel' ],
    working_directory = PYTHON_API_PATH,
    log_name = 'python-api-build')



def BuildPythonAPI(task_graph : TaskGraph):
  install_libcarla = task_graph.task_map.get('libcarla-install')
  task_graph.Add(Task('python-api-build', [ install_libcarla ], BuildPythonAPIMain))



def SetupUnrealEngine(task_graph : TaskGraph):
  pass



def UpdateCarlaUEAssets(task_graph : TaskGraph):
  CARLA_UE_CONTENT_PATH.mkdir(parents = True, exist_ok = True)
  Log('Cloning CARLA UE content...')
  for e in CARLA_UE_ASSETS_DEPENDENCIES:
    UpdateDependency(e, CARLA_UE_CONTENT_CARLA_PATH)
    


def BuildCarlaUEMain():
  assert UNREAL_ENGINE_PATH.exists()
  unreal_build_tool_args = []
  if ENABLE_CARSIM:
    unreal_build_tool_args.append('-carsim')
  if ENABLE_CHRONO:
    unreal_build_tool_args.append('-chrono')
  if ENABLE_ROS2:
    unreal_build_tool_args.append('-ros2')
  if ENABLE_UNITY_BUILD:
    unreal_build_tool_args.append('-unity-build')
  if ENABLE_NVIDIA_OMNIVERSE:
    unreal_build_tool_args.append('-nv-omniverse')
  if os.name == 'nt':
    LaunchSubprocessImmediate([
      UNREAL_ENGINE_PATH / 'Engine' / 'Build' / 'BatchFiles' / 'Build.bat',
      'CarlaUnrealEditor',
      'Win64',
      'Development',
      '-WaitMutex',
      '-FromMsBuild',
      CARLA_UE_PATH / 'CarlaUnreal.uproject',
    ], log_name = 'carla-ue-editor-build')
  else:
    pass



def BuildCarlaUE(task_graph : TaskGraph):
  if ENABLE_NVIDIA_OMNIVERSE:
    task_graph.Add(Task('nv-omniverse-install', [], InstallNVIDIAOmniverse))
  dependencies = []
  if ENABLE_LIBCARLA:
    dependencies.append(task_graph.task_map.get('libcarla-install'))
  if ENABLE_PYTHON_API:
    dependencies.append(task_graph.task_map.get('python-api-build'))
  task_graph.Add(Task('carla-ue-build', dependencies, BuildCarlaUEMain))



def InstallNVIDIAOmniverse():
  filename = 'USDCarlaInterface'
  header = f'{filename}.h'
  source = f'{filename}.cpp'
  omniverse_usd_path = NV_OMNIVERSE_PLUGIN_PATH / 'Source' / 'OmniverseUSD'
  files = [
    [ omniverse_usd_path / 'Public' / header, NV_OMNIVERSE_PATCH_PATH / header ],
    [ omniverse_usd_path / 'Private' / source, NV_OMNIVERSE_PATCH_PATH / source ],
  ]
  for src, dst in files:
    shutil.copyfile(src, dst)



def Clean():
  if not BUILD_PATH.exists():
    return
  try:
    shutil.rmtree(BUILD_PATH)
  finally:
    Log(f'Failed to remove {BUILD_PATH}.')
    exit(-1)



if __name__ == '__main__':
  try:
    task_graph = TaskGraph(PARALLELISM)
    if ARGV.clean or ARGV.rebuild:
      Clean()
    BUILD_PATH.mkdir(exist_ok = True)
    if not ARGV.no_log:
      LOG_PATH.mkdir(exist_ok = True)
    if UPDATE_DEPENDENCIES:
      UpdateDependencies(task_graph)
    CleanDownloads(task_graph)
    task_graph.Execute()
    if BUILD_DEPENDENCIES:
      BuildDependencies(task_graph)
    if ENABLE_LIBCARLA:
      BuildLibCarlaMain(task_graph)
    if ENABLE_PYTHON_API:
      BuildPythonAPI(task_graph)
    if UPDATE_CARLA_UE_ASSETS:
      UpdateCarlaUEAssets(task_graph)
    if ENABLE_CARLA_UE:
      BuildCarlaUE(task_graph)
    task_graph.Execute()
  except Exception as err:
    Log(err)
    Log(DEFAULT_ERROR_MESSAGE)
    exit(-1)
  finally:
    try:
      ARGS_SYNC_PATH.unlink(missing_ok = True)
    finally:
      pass
  exit(0)
