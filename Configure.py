from concurrent.futures import ProcessPoolExecutor, as_completed
from argparse import ArgumentParser
from pathlib import Path
import subprocess, tarfile, zipfile, requests, psutil, shutil, json, sys, os

# Basic paths
workspace_path = Path(__file__).parent.resolve()
libcarla_path = workspace_path / 'LibCarla'
python_api_path = workspace_path / 'PythonAPI'
examples_folder = workspace_path / 'Examples'
build_path = workspace_path / 'Build'
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
libcarla_build_path_server = build_path / 'libcarla-server-build'
libcarla_build_path_client = build_path / 'libcarla-client-build'
libcarla_build_path_pytorch = build_path / 'libcarla-pytorch-build'
libcarla_install_path_server = carla_ue_plugin_deps_path
libcarla_install_path_client = python_api_source_path / 'dependencies'
libcarla_test_content_path = build_path / 'test-content'
# OSM2ODR
osm2odr_build_path = build_path / 'libosm2dr-build'
osm2odr_build_path_server = build_path / 'libosm2dr-build-server'
osm2odr_source_path = build_path / 'libosm2dr-source'
# Misc
test_results_path = build_path / 'test-results'
libstdcpp_toolchain_path = build_path / 'LibStdCppToolChain.cmake'
libcpp_toolchain_path = build_path / 'LibCppToolChain.cmake'
cmake_config_file_path = build_path / 'CMakeLists.txt.in'
# Unreal Engine
ue_workspace_path = os.getenv('UE4_ROOT')
if ue_workspace_path is None:
    print('Could not find Unreal Engine workspace. Please set the environment variable UE4_ROOT as specified in the docs.')
ue_workspace_path = Path(ue_workspace_path)
# Dependencies
dependency_list_file_path = util_folder / 'Dependencies.json'
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
force_sequential = False
cmake_generator = 'Ninja'



class ConfigureContext:

    def __init__(self, arg):
        self.pool = ProcessPoolExecutor(parallelism)
        self.futures = []
        self.arg = arg
    
    def Dispatch(self, callable, arg):
        if force_sequential:
            callable(arg)
        else:
            self.futures.append(self.pool.submit(callable, arg))
    
    def Wait(self):
        if len(self.futures) == 0 or force_sequential:
            return
        for future in as_completed(self.futures):
            try:
                future.result()
            except Exception as e:
                print(f'Failed to run task: {e}')
            finally:
                pass
        self.futures = []



def LaunchSubprocess(cmd : list, display_output : bool = False):
    return subprocess.run(cmd) if display_output else subprocess.run(
        cmd,
        stdout = subprocess.PIPE,
        stderr = subprocess.PIPE)
        


def UpdateGitDependency(name : str, path : Path, url : str):
    if path.exists():
        LaunchSubprocess([
            'git',
            '-C', str(path),
            'pull'
        ]).check_returncode()
    else:
        LaunchSubprocess([
            'git',
            '-C', str(path.parent),
            'clone',
            '--depth', '1', '--single-branch',
            url,
            path.stem
        ]).check_returncode()



def UpdateArchiveDependency(name : str, path : Path, url : str):
    # Download:
    try:
        temp_path = Path(str(path) + '.tmp')
        with requests.Session() as session:
            with session.get(url, stream = True) as result:
                result.raise_for_status()
                with open(temp_path, 'wb') as file:
                    shutil.copyfileobj(result.raw, file)
    except Exception as err:
        print(f'Failed to download dependency "{name}": {err}')
    # Extract:
    try:
        if url.endswith('.tar.gz'):
            archive_path = temp_path.with_suffix('.tar.gz')
            extract_path = path.with_name(path.name + '-temp')
            temp_path.rename(archive_path)
            with tarfile.open(archive_path) as file:
                file.extractall(extract_path)
            entries = [ file for file in extract_path.iterdir() ]
            if len(entries) == 1 and entries[0].is_dir():
                Path(entries[0]).replace(path)
            extract_path.rmdir()
        elif url.endswith('.zip'):
            archive_path = temp_path.with_suffix('.zip')
            temp_path.rename(archive_path)
            zipfile.ZipFile(archive_path).extractall(path)
    except Exception as err:
        print(f'Failed to extract dependency "{name}": {err}')



def UpdateHoudini():
    UpdateGitDependency(
        'HoudiniEngine',
        carla_ue_plugin_root_path,
        houdini_url)
    # LaunchSubprocess([
    #     'git', '-C', str(houdini_plugin_path), 'checkout', houdini_commit_hash
    # ]).check_returncode()
    # LaunchSubprocess([
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



def UpdateDependency(dep : dict):
    name = dep['name']
    download_path = build_path / f'{name}-source'
    urls = dep.get('urls', [])
    assert type(urls) == type([])
    for url in urls:
        try:
            if str(url).endswith('.git'):
                UpdateGitDependency(name, download_path, url)
            elif str(url).endswith('.tar.gz') or str(url).endswith('.zip'):
                if download_path.exists():
                    print(f'Dependency "{name}" already downloaded. Delete "{download_path}" if you wish for it to be re-downloaded.')
                else:
                    UpdateArchiveDependency(name, download_path, url)
            return
        finally:
            pass
    print(f'Failed to update dependency "{name}".')
    assert False



def UpdateDependencies(c : ConfigureContext):
    dependencies = []
    with open(dependency_list_file_path, 'r') as file:
        dependencies = json.load(file)
    for dep in dependencies:
        name = dep['name']
        version = dep.get('version', 'N/A')
        print(f'Updating {name} (version {version}).')
        try:
            c.Dispatch(UpdateDependency, dep)
        except Exception as err:
            print(f'Failed to update dependency "{name}": {err}')



def BuildDependencies(context : ConfigureContext):
    pass



def BuildCarlaUnrealEditor():
    if os.name == 'nt':
        LaunchSubprocess([
            ue_workspace_path / 'Engine' / 'Build' / 'BatchFiles' / 'Build.bat',
            'CarlaUE4', 'Win64', 'Development', '-WaitMutex', '-FromMsBuild',
            carla_ue_path / 'CarlaUE4.uproject'
        ]).check_returncode()



def BuildCarlaUE(context : ConfigureContext):
    context.Dispatch(UpdateHoudini)
    context.Wait()
    context.Dispatch(BuildCarlaUnrealEditor)



def ConfigureLibCarla(configuration : str):
    return LaunchSubprocess([
        'cmake',
        '-G', cmake_generator,
        f'-DCMAKE_BUILD_TYPE={configuration}',
        '-DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"',
        f'-DCMAKE_INSTALL_PREFIX={libcarla_install_path_client}',
        workspace_path
    ])



def BuildLibCarlaCore(configuration : str):
    return LaunchSubprocess([
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



def BuildLibCarlaMain(c : ConfigureContext):
    if not c.arg.skip_libcarla_server:
        c.Dispatch(BuildLibCarlaServer)
    if not c.arg.skip_libcarla_client:
        c.Dispatch(BuildLibCarlaClient)



def BuildCarlaUEMain(c : ConfigureContext):
    print('Building Carla UE Editor')
    optional_modules = {}
    optional_modules['CarSim'] = c.arg.use_unity
    optional_modules['Chrono'] = c.arg.use_unity
    optional_modules['Unity'] = c.arg.use_unity
    optional_modules['Omniverse'] = c.arg.use_omniverse
    if optional_modules['Omniverse']:
        c.Dispatch(InstallNVIDIAOmniverse)
    BuildCarlaUE(c)
    c.Wait()



def BuildPythonAPIMain(c : ConfigureContext):
    print('Building Python API')
    pass



def ParseCommandLine():
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
        '-update-dependencies',
        action='store_true',
        help = 'Whether to update the CARLA dependencies.')
    arg_parser.add_argument(
        '-build-dependencies',
        action='store_true',
        help = 'Whether to build the CARLA dependencies.')
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
    return arg_parser.parse_args()



def Main():
    print('Started.')
    
    build_path.mkdir(exist_ok = True)

    arg = ParseCommandLine()
    c = ConfigureContext(arg)

    if arg.update_dependencies or True:
        UpdateDependencies(c)
        c.Wait()
    
    if arg.build_dependencies or True:
        BuildDependencies(c)
        c.Wait()

    if arg.build_libcarla:
        BuildLibCarlaMain(c)
        c.Wait()

    if arg.build_python_api:
        BuildPythonAPIMain(c)
        c.Wait()

    if arg.build_carla_ue:
        BuildCarlaUEMain(c)
        c.Wait()
    
    for ext in [ '*.tmp', '*.zip', '*.tar.gz' ]:
        for e in build_path.glob(ext):
            e.unlink(missing_ok = True)

    print('Done.')



if __name__ == '__main__':
    Main()