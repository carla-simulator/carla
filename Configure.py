from concurrent.futures import ProcessPoolExecutor, as_completed
from argparse import ArgumentParser
from pathlib import Path
import subprocess, psutil, shutil, sys, os

# Basic paths
workspace_path = Path(__file__).parent.resolve()
libcarla_path = workspace_path / 'LibCarla'
python_api_path = workspace_path / 'PythonAPI'
examples_folder = workspace_path / 'Examples'
build_folder = workspace_path / 'Build'
dist_folder = workspace_path / 'Dist'
util_folder = workspace_path / 'Util'
docker_utils_folder = util_folder / 'DockerUtils'
build_tools_path = util_folder / 'BuildTools'
# UE plugin
carla_ue_path = workspace_path / 'Unreal' / 'CarlaUE4'
carla_ue_plugin_root_path = carla_ue_path / 'Plugins'
carla_ue_plugins_path = carla_ue_plugin_root_path / 'Carla'
carla_ue_plugin_deps_path = carla_ue_plugin_root_path / 'CarlaDependencies'
# PythonAPI
python_version_major = sys.version_info.major
python_version_minor = sys.version_info.minor
python_api_source_path = python_api_path / 'carla'
# LibCarla
libcarla_build_path_server = build_folder / 'libcarla-server-build'
libcarla_build_path_client = build_folder / 'libcarla-client-build'
libcarla_build_path_pytorch = build_folder / 'libcarla-pytorch-build'
libcarla_install_path_server = carla_ue_plugin_deps_path
libcarla_install_path_client = python_api_source_path / 'dependencies'
libcarla_test_content_path = build_folder / 'test-content'
# OSM2ODR
osm2odr_build_path = build_folder / 'libosm2dr-build'
osm2odr_build_path_server = build_folder / 'libosm2dr-build-server'
osm2odr_source_path = build_folder / 'libosm2dr-source'
# Misc
test_results_path = build_folder / 'test-results'
libstdcpp_toolchain_path = build_folder / 'LibStdCppToolChain.cmake'
libcpp_toolchain_path = build_folder / 'LibCppToolChain.cmake'
cmake_config_file_path = build_folder / 'CMakeLists.txt.in'
# Unreal Engine
ue_workspace_path = os.getenv('UE4_ROOT')
if ue_workspace_path is None:
    print('Could not find Unreal Engine workspace. Please set the environment variable UE4_ROOT as specified in the docs.')
ue_workspace_path = Path(ue_workspace_path)
# Boost
# ZLib
# LibPNG
# RPCLib
# Google Test
# Recast & Detour
# Xercesc
# Sqlite3
# PROJ
# Eigen
# Chrono
# Houdini
houdini_url = 'https://github.com/sideeffects/HoudiniEngineForUnreal.git'
houdini_plugin_path = carla_ue_plugin_root_path / 'HoudiniEngine'
houdini_commit_hash = '55b6a16cdf274389687fce3019b33e3b6e92a914'
houdini_patch_path = util_folder / 'Patches' / 'houdini_patch.txt'
# Omniverse
omniverse_plugin_path = ue_workspace_path / 'Engine' / 'Plugins' / 'Marketplace' / 'NVIDIA' / 'Omniverse'
omniverse_patch_path = util_folder / 'Patches' / 'omniverse_4.26'
# Script settings
parallelism = psutil.cpu_count(logical = True)
sequential = False
cmake_generator = 'Ninja'



def UpdateGitDependency(name : str, path : Path, url : str):
    if path.exists():
        subprocess.run([
            'git',
            '-C', str(path),
            'pull'
        ]).check_returncode()
    else:
        subprocess.run([
            'git',
            '-C', str(path.parent),
            'clone',
            '-depth', '1', '-single-branch',
            url,
            name
        ]).check_returncode()



def UpdateHoudini():
    UpdateGitDependency(
        'HoudiniEngine',
        carla_ue_plugin_root_path,
        houdini_url)
    # subprocess.run([
    #     'git', '-C', str(houdini_plugin_path), 'checkout', houdini_commit_hash
    # ]).check_returncode()
    # subprocess.run([
    #     'git', '-C', str(houdini_plugin_path), 'apply', str(houdini_patch_path)
    # ]).check_returncode()



def InstallNVIDIAOmniverse():
    filename = 'USDCarlaInterface'
    header = f'{filename}.h'
    source = f'{filename}.cpp'
    omniverse_usd_path = omniverse_plugin_path / 'Source' / 'OmniverseUSD'
    files = [
        [ omniverse_usd_path / 'Public' / header, omniverse_patch_path / header ],
        [ omniverse_usd_path / 'Private' / source, omniverse_patch_path / source ],
    ]
    for src, dst in files:
        shutil.copyfile(src, dst)



def AwaitTasks(futures : list):
    for future in as_completed(futures):
        try:
            future.result()
        except Exception as e:
            print(f'Failed to run task: {e}')
            exit(-1)
    futures = []



def BuildCarlaUnrealEditor():
    if os.name == 'nt':
        subprocess.run([
            ue_workspace_path / 'Engine' / 'Build' / 'BatchFiles' / 'Build.bat',
            'CarlaUE4', 'Win64', 'Development', '-WaitMutex', '-FromMsBuild',
            carla_ue_path / 'CarlaUE4.uproject'
        ]).check_returncode()



def BuildCarlaUE(pool : ProcessPoolExecutor, futures : list):
    futures.append(pool.submit(UpdateHoudini))
    AwaitTasks(futures)
    futures.append(pool.submit(BuildCarlaUnrealEditor))



def ConfigureLibCarla(configuration : str):
    return subprocess.run([
        'cmake',
        '-G', cmake_generator,
        f'-DCMAKE_BUILD_TYPE={configuration}',
        '-DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"',
        f'-DCMAKE_INSTALL_PREFIX={libcarla_install_path_client}',
        workspace_path
    ])



def BuildLibCarlaCore(configuration : str):
    return subprocess.run([
        'cmake', '-build', '.', '-config', 'Release', '-target', 'install'
    ])



def BuildLibCarlaClient():
    print('Building LibCarla Client')
    ConfigureLibCarla('Client').check_returncode()
    BuildLibCarlaCore('Client').check_returncode()



def BuildLibCarlaServer():
    print('Building LibCarla Server')
    ConfigureLibCarla('Server').check_returncode()
    BuildLibCarlaCore('Server').check_returncode()



def BuildLibCarlaMain(pool : ProcessPoolExecutor, futures : list, arg):
    if sequential:
        if not arg.skip_libcarla_server:
            BuildLibCarlaServer()
        if not arg.skip_libcarla_client:
            BuildLibCarlaClient()
    else:
        if not arg.skip_libcarla_server:
            futures.append(pool.submit(BuildLibCarlaServer))
        if not arg.skip_libcarla_client:
            futures.append(pool.submit(BuildLibCarlaClient))



def BuildCarlaUEMain(pool : ProcessPoolExecutor, futures : list, arg):
    print('Building Carla UE Editor')
    optional_modules = {}
    optional_modules['CarSim'] = arg.use_unity
    optional_modules['Chrono'] = arg.use_unity
    optional_modules['Unity'] = arg.use_unity
    optional_modules['Omniverse'] = arg.use_omniverse
    if optional_modules['Omniverse']:
        futures.append(pool.submit(InstallNVIDIAOmniverse))
    BuildCarlaUE(pool, futures)
    AwaitTasks(futures)



def BuildPythonAPIMain(pool : ProcessPoolExecutor, futures : list, arg):
    print('Building Python API')
    pass



def Main():
    print('Starting.')
    arg_parser = ArgumentParser(description = __doc__)
    arg_parser.add_argument(
        '-build-libcarla',
        action='store_true',
        help = 'Build LibCarla.')
    arg_parser.add_argument(
        '-build-python-api',
        action='store_true',
        help = 'Build the CARLA Python API.')
    arg_parser.add_argument(
        '-build-carla-ue',
        action='store_true',
        help = 'Build Carla Unreal.')
    arg_parser.add_argument(
        '-build-osm2odr',
        action='store_true',
        help = 'Build OSM2ODR.')
    arg_parser.add_argument(
        '-package',
        action='store_true',
        help = 'Build OSM2ODR.')
    arg_parser.add_argument(
        '-clean',
        action='store_true',
        help = 'Clean build files.')
    # LibCarla
    arg_parser.add_argument(
        '-skip-libcarla-client',
        action='store_true',
        help = 'Whether to skip the libcarla client.')
    arg_parser.add_argument(
        '-skip-libcarla-server',
        action='store_true',
        help = 'Whether to skip the libcarla server.')
    # Carla UE:
    arg_parser.add_argument(
        '-use-carsim',
        action='store_true',
        help = 'Whether to enable plugin "CarSim".')
    arg_parser.add_argument(
        '-use-chrono',
        action='store_true',
        help = 'Whether to enable plugin "Chrono".')
    arg_parser.add_argument(
        '-use-unity',
        action='store_true',
        help = 'Whether to enable plugin "Unity".')
    arg_parser.add_argument(
        '-use-omniverse',
        action='store_true',
        help = 'Whether to enable plugin "NVIDIA Omniverse".')
    arg = arg_parser.parse_args()

    with ProcessPoolExecutor(parallelism) as pool:
        futures = []
        if arg.build_libcarla:
            BuildLibCarlaMain(pool, futures, arg)
            AwaitTasks()
        if arg.build_python_api:
            BuildPythonAPIMain(pool, futures, arg)
            AwaitTasks()
        if arg.build_carla_ue:
            BuildCarlaUEMain(pool, futures, arg)
            AwaitTasks()



if __name__ == '__main__':
    try:
        Main()
    finally:
        print('Done.')