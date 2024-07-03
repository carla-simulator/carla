# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
# 
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
"""

from pathlib import Path
import subprocess
import os

CARLA_ROOT_PATH = Path(__file__).parent
CARLA_TEMP_PATH = CARLA_ROOT_PATH / 'Temp'

CARLA_UNREAL_ENGINE_FORK_URL = 'https://github.com/CarlaUnreal/UnrealEngine.git'
CARLA_UNREAL_ROOT_PATH = CARLA_ROOT_PATH / 'Unreal' / 'CarlaUnreal'

CARLA_PLUGINS_ROOT_PATH = CARLA_UNREAL_ROOT_PATH / 'Plugins'
CARLA_PLUGIN_PATH = CARLA_PLUGINS_ROOT_PATH / 'Carla'
CARLA_TOOLS_PLUGIN_PATH = CARLA_PLUGINS_ROOT_PATH / 'CarlaTools'
CARLA_EXPORTER_PLUGIN_PATH = CARLA_PLUGINS_ROOT_PATH / 'CarlaExporter'

CARLA_CONTENT_PATH = CARLA_UNREAL_ROOT_PATH / 'Content' / 'Carla'
CARLA_CMAKE_BUILD_TYPE = 'Release'
CARLA_CMAKE_MAKE_TOOL = 'Ninja'
SHELL_EXT = '.bat' if os.name == 'nt' else '.sh'

VISUAL_STUDIO_COMPONENTS = [
  'Microsoft.VisualStudio.Workload.NativeDesktop',
  'Microsoft.VisualStudio.Workload.NativeGame',
  'Microsoft.VisualStudio.Workload.ManagedDesktop',
  'Microsoft.VisualStudio.Component.Windows10SDK.18362',
  'Microsoft.VisualStudio.Component.VC.CMake.Project',
  'Microsoft.Net.ComponentGroup.4.8.1.DeveloperTools',
  'Microsoft.VisualStudio.Component.VC.Llvm.Clang',
  'Microsoft.VisualStudio.Component.VC.Llvm.ClangToolset',
  'Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Llvm.Clang'
]

VISUAL_STUDIO_LANGUAGE = 'En-us'



def UpdateVisualStudio():
  if os.name != 'nt':
    return None
  print('Setting up Visual Studio...')
  subprocess.run([
    'curl',
    '-L',
    '-o',
    CARLA_TEMP_PATH / 'vs_community.exe',
    'https://aka.ms/vs/17/release/vs_community.exe'
  ]).check_returncode()
  cmd = [
    CARLA_TEMP_PATH / 'vs_community.exe',
    '--add'
  ]
  cmd.extend(VISUAL_STUDIO_COMPONENTS)
  cmd.extend([
    '--addProductLang',
    VISUAL_STUDIO_LANGUAGE,
    '--installWhileDownloading',
    '--passive',
    '--wait'
  ])
  subprocess.run(cmd).check_returncode()



def DownloadNinja(argv, pool):
  pass



def InstallPythonPackages():
  import sys
  subprocess.run([
    sys.executable,
    '-m',
    'pip',
    'install',
    '--upgrade',
    'pip'
  ]).check_returncode()
  subprocess.run([
    sys.executable,
    '-m',
    'pip',
    'install',
    '-r',
    CARLA_ROOT_PATH / 'requirements.txt'
  ]).check_returncode()



def DownloadUnrealEngine(
    unreal_engine_path : str,
    unreal_engine_branch : str,
    unreal_engine_full_clone : str):
  UE_PATH = Path(unreal_engine_path)
  cmd = None
  if UE_PATH.is_dir() and (UE_PATH / '.git').is_dir():
    cmd = [
      'git',
      '-C',
      UE_PATH,
      'pull',
      'origin'
    ]
  else:
    cmd = [
      'git',
      '-C',
      UE_PATH.parent,
      'clone',
      CARLA_UNREAL_ENGINE_FORK_URL,
    ]
    if unreal_engine_branch != None:
      cmd.extend([ '-b', unreal_engine_branch ])
    if not unreal_engine_full_clone:
      cmd.extend([ '--single-branch', '--depth', '1' ])
    cmd.extend([ UE_PATH.name ])
  subprocess.run(cmd).check_returncode()



def BuildUnrealEngine(unreal_engine_path):
  UE_PATH = Path(unreal_engine_path)
  subprocess.run(
    'Setup' + SHELL_EXT,
    cwd = UE_PATH).check_returncode()
  subprocess.run(
    'GenerateProjectFiles' + SHELL_EXT,
    cwd = UE_PATH).check_returncode()
  cmd = [
    'make'
  ] if os.name != 'nt' else [
    'msbuild',
    UE_PATH / 'Engine' / 'Intermediate' / 'ProjectFiles' / 'UE5.vcxproj',
    '/property:Configuration="Development_Editor"',
    '/property:Platform="x64"'
  ]
  subprocess.run(
    cmd,
    cwd = UE_PATH).check_returncode()



def ConfigureCARLA(build_path):
  cmd = [
    'cmake',
    '-S', CARLA_ROOT_PATH,
    '-B', build_path,
    '-G', CARLA_CMAKE_MAKE_TOOL,
    f'-DCMAKE_BUILD_TYPE={CARLA_CMAKE_BUILD_TYPE}',
  ]
  subprocess.run(cmd).check_returncode()



def BuildCARLA(build_path):
  subprocess.run([
    'cmake',
    '--build',
    build_path
  ]).check_returncode()



def BuildCARLAPythonAPI(build_path):
  subprocess.run([
    'cmake',
    '--build',
    build_path,
    '--target',
    'carla-python-api'
  ]).check_returncode()



def InstallCARLAPythonAPI(build_path):
  subprocess.run([
    'cmake',
    '--build',
    build_path,
    '--target',
    'carla-python-api-install'
  ]).check_returncode()



def BuildCARLAUnrealEditor(build_path):
  subprocess.run([
    'cmake',
    '--build',
    build_path,
    '--target',
    'carla-unreal-editor'
  ]).check_returncode()



def LaunchCARLA(build_path):
  subprocess.run([
    'cmake',
    '--build',
    build_path,
    '--target',
    'launch'
  ]).check_returncode()



def Main():
  import shutil
  from Update import UpdateContent
  from argparse import ArgumentParser

  argp = ArgumentParser(
    description = __doc__)
  argp.add_argument(
    '--skip-unreal-engine', '--skip-ue',
    help = 'Skip the Unreal Engine setup steps.',
    action = 'store_true'
  )
  argp.add_argument(
    '--skip-visual-studio', '--skip-vs',
    help = 'Skip the Visual Studio setup steps.',
    action = 'store_true'
  )
  argp.add_argument(
    '--skip-python-requirements', '--skip-python-reqs',
    help = 'Skip checking Python requirements.',
    action = 'store_true'
  )
  argp.add_argument(
    '--skip-content',
    help = 'Skip the download/clone Content step.',
    action = 'store_true'
  )
  argp.add_argument(
    '--build-path', '--b',
    help = 'Set the Unreal Engine install location.',
    type = str,
    default = str(CARLA_ROOT_PATH / 'Build')
  )
  argp.add_argument(
    '--unreal-engine-path', '--ue-path',
    help = 'Set the Unreal Engine install location.',
    type = str,
    default = str(CARLA_ROOT_PATH.parent / 'UnrealEngine5_carla')
  )
  argp.add_argument(
    '--unreal-engine-branch', '--ue-branch',
    help = 'Set the Unreal Engine branch to download.',
    type = str,
    default = 'ue5-dev-carla'
  )
  argp.add_argument(
    '--unreal-engine-full-clone', '--ue-full-clone',
    help = 'Disable shallow clone for Unreal Engine.',
    action = 'store_true'
  )
  argp.add_argument(
    '--build-python-api', '--python-api',
    help = 'Whether to build the CARLA Python API.',
    action = 'store_true'
  )
  argp.add_argument(
    '--install-python-api',
    help = 'Whether to build and install the CARLA Python API.',
    action = 'store_true'
  )
  argp.add_argument(
    '--build-carla-unreal-editor', '--build-carla-ue',
    help = 'Whether to launch the CARLA Unreal Editor once building finishes.',
    action = 'store_true'
  )
  argp.add_argument(
    '--launch-carla-unreal-editor', '--launch',
    help = 'Whether to launch the CARLA Unreal Editor once building finishes.',
    action = 'store_true'
  )
  argp.add_argument(
    '--content-version',
    help = 'Set the Content version to download (unused when cloning).',
    type = str,
    default = None
  )
  argp.add_argument(
    '--content-clone',
    help = 'Enable clone instead of direct download.',
    action = 'store_true'
  )
  argp.add_argument(
    '--content-tag',
    help = 'Set the Content repo branch or tag when cloning.',
    type = str,
    default = None
  )
  argp.add_argument(
    '--content-shallow-clone',
    help = 'Enable shadow cloning.',
    action = 'store_true'
  )
  argp.add_argument(
    '--yes', '-y',
    help = 'Default all yes/no prompts to "yes".',
    action = 'store_true'
  )
  argp.add_argument(
    '--no', '-n',
    help = 'Default all yes/no prompts to "no".',
    action = 'store_true'
  )
  argp.add_argument(
    '--all', '-a',
    help = 'Perform all setups steps. Overrides all other on/off options.',
    action = 'store_true'
  )
  ARGV = argp.parse_args()
  print(ARGV)
  CARLA_TEMP_PATH.mkdir(exist_ok = True)
  assert not(ARGV.yes and ARGV.no)

  SETUP_CONTENT = ARGV.all or (not ARGV.skip_content)
  SETUP_PYTHON_PACKAGES = ARGV.all or (True)
  SETUP_CARLA = ARGV.all or (True)
  SETUP_VS = ARGV.all or (os.name == 'nt' and not ARGV.skip_visual_studio)
  SETUP_UE = ARGV.all or (not ARGV.skip_unreal_engine)

  def SubmitIf(key : bool, *args):
    return pool.submit(*args) if key else None
  
  def WaitIf(key, ft):
    if key:
      return ft.result()
    else:
      assert ft == None

  from concurrent.futures import as_completed, ProcessPoolExecutor
  with ProcessPoolExecutor() as pool:
    update_content = SubmitIf(
      SETUP_CONTENT,
      UpdateContent,
      ARGV.content_version,
      ARGV.content_clone,
      ARGV.content_tag,
      ARGV.content_shallow_clone)
    update_vs = SubmitIf(SETUP_VS, UpdateVisualStudio)
    install_python_packages = SubmitIf(SETUP_PYTHON_PACKAGES, InstallPythonPackages)
    download_ue = SubmitIf(
      SETUP_UE,
      DownloadUnrealEngine,
      ARGV.unreal_engine_path,
      ARGV.unreal_engine_branch,
      ARGV.unreal_engine_full_clone) if SETUP_UE else None
    WaitIf(SETUP_VS, update_vs)
    WaitIf(download_ue, download_ue)
    build_ue = SubmitIf(
      SETUP_UE,
      BuildUnrealEngine,
      ARGV.unreal_engine_path)
    WaitIf(SETUP_PYTHON_PACKAGES, install_python_packages)
    configure_carla = SubmitIf(SETUP_CARLA, ConfigureCARLA, ARGV.build_path)
    WaitIf(SETUP_CARLA, configure_carla)
    build_carla = SubmitIf(SETUP_CARLA, BuildCARLA, ARGV.build_path)
    WaitIf(SETUP_CARLA, build_carla)
    if ARGV.build_python_api and not ARGV.install_python_api:
      BuildCARLAPythonAPI(ARGV)
    elif ARGV.install_python_api:
      InstallCARLAPythonAPI(ARGV)
    WaitIf(SETUP_UE, build_ue)
    WaitIf(SETUP_CONTENT, update_content)
    shutil.rmtree(CARLA_TEMP_PATH)
    if ARGV.launch_carla_unreal_editor:
      LaunchCARLA(ARGV.build_path)
    elif ARGV.build_carla_unreal_editor:
      BuildCARLAUnrealEditor(ARGV.build_path)
  


if __name__ == '__main__':
  Main()
