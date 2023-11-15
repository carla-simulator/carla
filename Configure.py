from concurrent.futures import ProcessPoolExecutor, as_completed
from argparse import ArgumentParser
from pathlib import Path
import subprocess, tarfile, zipfile, requests, psutil, shutil, glob, json, sys, os

def FindExecutable(candidates : list):
	for e in candidates:
		ec = subprocess.call(
			[ 'where' if os.name == 'nt' else 'whereis', e ],
			stdout = subprocess.PIPE,
			stderr = subprocess.PIPE,
			shell = True)
		if ec == 0:
			return e
	return None

c_compiler = FindExecutable([
	'clang-cl',
	'cl'
] if os.name == 'nt' else [
	'clang',
	'gcc'
])

cpp_compiler = FindExecutable([
	'clang-cl',
	'cl'
] if os.name == 'nt' else [
	'clang++',
	'g++'
])

linker = FindExecutable([
	'llvm-link',
	'link'
] if os.name == 'nt' else [
	'lld',
	'ld'
])

lib = FindExecutable([
	'llvm-lib',
	'lib',
	'llvm-ar'
] if os.name == 'nt' else [
	'llvm-ar',
	'ar'
])

# Basic paths
workspace_path = Path(__file__).parent.resolve()
libcarla_path = workspace_path / 'LibCarla'
python_api_path = workspace_path / 'PythonAPI'
examples_path = workspace_path / 'Examples'
build_path = workspace_path / 'Build'
dist_path = workspace_path / 'Dist'
util_path = workspace_path / 'Util'
docker_utils_path = util_path / 'DockerUtils'
build_tools_path = util_path / 'BuildTools'
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
executable_extension = '.exe' if os.name == 'nt' else ''
library_extension = '.lib' if os.name == 'nt' else '.a'
object_extension = '.obj' if os.name == 'nt' else '.o'
shell_script_extension = '.bat' if os.name == 'nt' else ''
# Unreal Engine
ue_workspace_path = os.getenv('UE4_ROOT')
if ue_workspace_path is None:
	print('Could not find Unreal Engine workspace. Please set the environment variable UE4_ROOT as specified in the docs.')
ue_workspace_path = Path(ue_workspace_path)
# Dependencies
dependency_list_file_path = util_path / 'Dependencies.json'
# Houdini
houdini_url = 'https://github.com/sideeffects/HoudiniEngineForUnreal.git'
houdini_plugin_path = carla_ue_plugin_root_path / 'HoudiniEngine'
houdini_commit_hash = '55b6a16cdf274389687fce3019b33e3b6e92a914'
houdini_patch_path = util_path / 'Patches' / 'houdini_patch.txt'
# Omniverse
omniverse_plugin_path = ue_workspace_path / 'Engine' / 'Plugins' / 'Marketplace' / 'NVIDIA' / 'Omniverse'
omniverse_patch_path = util_path / 'Patches' / 'omniverse_4.26'
# Script settings
parallelism = psutil.cpu_count(logical = True)
force_sequential = False
cmake_generator = 'Ninja'

dependencies = {
    'boost' :
	{
        'from' :
        [
            {
                'url': 'https://boostorg.jfrog.io/artifactory/main/release/1.83.0/source/boost_1_83_0.zip',
                'type': 'download'
            },
            {
                'url': 'https://carla-releases.s3.eu-west-3.amazonaws.com/Backup/boost_1_83_0.zip',
                'type': 'download'
            }
        ]
    },
    'chrono' :
	{
        'from' :
        [
            {
                'url' : 'https://github.com/projectchrono/chrono.git',
                'type' : 'git',
                'tag-or-branch' : '8.0.0'
            }
        ]
    },
    'eigen' :
	{
        'from' :
        [
            {
                'url' : 'https://gitlab.com/libeigen/eigen.git',
                'type' : 'git',
                'branch' : '3.4.0'
            }
        ]
    },
    'libpng' :
	{
        'from' :
        [
            {
                'url' : 'https://github.com/glennrp/libpng.git',
                'type' : 'git',
                'tag-or-branch' : 'v1.6.40'
            }
        ]
    },
    'proj' :
	{
        'from' :
        [
            {
                'url' : 'https://download.osgeo.org/proj/proj-7.2.1.tar.gz',
                'type' : 'download'
            },
            {
                'url' : 'https://github.com/madler/zlib.git',
                'type' : 'git'
            }
        ]
    },
    'gtest' :
	{
        'from' :
        [
            {
                'url' : 'https://github.com/google/googletest.git',
                'type' : 'git',
                'tag-or-branch' : 'v1.14.0'
            }
        ]
    },
    'zlib' :
	{
        'from' :
        [
            {
                'url' : 'https://zlib.net/current/zlib.tar.gz',
                'type' : 'download'
            }
        ]
    },
    'xercesc' :
	{
        'from' :
        [
            {
                'url' : 'https://github.com/apache/xerces-c.git',
                'type' : 'git',
                'tag-or-branch' : 'v3.2.4'
            },
            {
                'url' : 'https://archive.apache.org/dist/xerces/c/3/sources/xerces-c-3.2.3.zip',
                'type' : 'download'
            },
            {
                'url' : 'https://carla-releases.s3.eu-west-3.amazonaws.com/Backup/xerces-c-3.2.3.zip',
                'type' : 'download'
            }
        ]
    },
    'sqlite' :
	{
        'from' :
        [
            {
                'url' : 'https://www.sqlite.org/2021/sqlite-amalgamation-3340100.zip',
                'type' : 'download'
            }
        ]
    },
    'rpclib' :
	{
        'from' :
        [
            {
                'url' : 'https://github.com/rpclib/rpclib.git',
                'type' : 'git',
                'tag-or-branch' : 'v2.3.0'
            }
        ]
    },
    'recast' :
	{
        'from' :
        [
            {
                'url' : 'https://github.com/recastnavigation/recastnavigation.git',
                'type' : 'git',
                'tag-or-branch' : '1.6.0'
            }
        ]
    },
	'libosmscout' :
	{
		'from' :
		[
			{
				'url' : 'https://github.com/Framstag/libosmscout.git',
				'type' : 'git',
				'tag-or-branch' : 'master'
			}
		]
	},
	'lunasvg' :
	{
		'from' :
		[
			{
				'url' : 'https://github.com/sammycage/lunasvg.git',
				'type' : 'git',
				'tag-or-branch' : 'master'
			}
		]
	},
	'sumo' :
	{
		'from' :
		[
			{
				'url' : 'https://github.com/carla-simulator/sumo.git',
				'type' : 'git',
				'tag-or-branch' : 'carla_osm2odr'
			}
		]
	}
}



class ConfigureContext:

	def __init__(self, arg):
		self.pool = ProcessPoolExecutor(parallelism)
		self.futures = []
		self.arg = arg
	
	def Dispatch(self, callable, *arg):
		if len(arg) == 0:
			if force_sequential:
				callable()
			else:
				self.futures.append(self.pool.submit(callable))
		else:
			if force_sequential:
				callable(*arg)
			else:
				self.futures.append(self.pool.submit(callable, *arg))
	
	def Wait(self):
		if len(self.futures) == 0 or force_sequential:
			return
		for future in as_completed(self.futures):
			try:
				future.result()
			except Exception as e:
				Log(f'Failed to run task: {e}')
			finally:
				pass
		self.futures = []



def Log(message):
	message = str(message)
	message += '\n'
	print(message, end='')



def LaunchSubprocess(
		cmd : list,
		display_output : bool = False,
		working_directory : Path = None):
	
	if display_output or force_sequential:
		return subprocess.run(cmd, cwd = working_directory)
	else:
		return subprocess.run(
			cmd,
			stdout = subprocess.PIPE,
			stderr = subprocess.PIPE,
			cwd = working_directory)

def LaunchSubprocessImmediate(
		cmd : list,
		display_output : bool = False,
		working_directory : Path = None):
	sp = LaunchSubprocess(cmd, display_output, working_directory)
	try:
		sp.check_returncode()
	except:
		stdout = sp.stdout.decode() if sp.stdout else ''
		stderr = sp.stderr.decode() if sp.stderr else ''
		error_message = (
			f'Failed to run task {cmd}.\n'
			f' stdout:\n'
			f' {stdout}\n'
			f' stderr:\n'
			f' {stderr}\n'
		)
		print(error_message)



def UpdateGitRepository(path : Path, url : str, branch : str = None):
	if path.exists():
		LaunchSubprocessImmediate([
			'git',
			'-C', str(path),
			'pull'
		])
	else:
		cmd = [
			'git',
			'-C', str(path.parent),
			'clone',
			'--depth', '1', '--single-branch'
		]
		if branch != None:
			cmd.append('-b')
			cmd.append(branch)
		cmd.append(url)
		cmd.append(path.stem)
		LaunchSubprocessImmediate(cmd)



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



def UpdateDependency(name : str, dep : dict):
	download_path = build_path / f'{name}-source'
	sources = dep.get('from', [])
	assert type(sources) == type([])
	for source in sources:
		url = source['url']
		source_type = source['type']
		try:
			if source_type == 'git':
				branch = source.get('branch-or-tag', None)
				UpdateGitRepository(download_path, url, branch)
			elif source_type == 'download':
				if download_path.exists():
					Log(f'Dependency "{name}" already present. Delete "{download_path}" if you wish for it to be downloaded again.')
				else:
					DownloadDependency(name, download_path, url)
			return
		finally:
			pass
	Log(f'Failed to update dependency "{name}".')
	assert False



def ConfigureLibCarlaClient():
	cmd = [ 'cmake' ]
	return LaunchSubprocessImmediate(cmd)



def ConfigureLibCarlaServer():
	cmd = [ 'cmake' ]
	return LaunchSubprocessImmediate(cmd)



def BuildLibCarlaClient():
	ConfigureLibCarlaClient()



def BuildLibCarlaServer():
	ConfigureLibCarlaServer()



def BuildLibCarlaMain(c : ConfigureContext):
	if not c.arg.skip_libcarla_server:
		Log('Building LibCarla Server')
		c.Dispatch(BuildLibCarlaServer)
	if not c.arg.skip_libcarla_client:
		Log('Building LibCarla Client')
		c.Dispatch(BuildLibCarlaClient)



def BuildCarlaUEMain():
	if os.name == 'nt':
		LaunchSubprocessImmediate([
			ue_workspace_path / 'Engine' / 'Build' / 'BatchFiles' / 'Build.bat',
			'CarlaUE4', 'Win64', 'Development', '-WaitMutex', '-FromMsBuild',
			carla_ue_path / 'CarlaUE4.uproject'
		])
	else:
		pass



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



def BuildCarlaUEMain(c : ConfigureContext):
	Log('Building Carla UE Editor')
	# c.arg.use_carsim
	# c.arg.use_chrono
	# c.arg.use_unity
	if c.arg.use_omniverse:
		c.Dispatch(InstallNVIDIAOmniverse)
	c.Wait()
	c.Dispatch(BuildCarlaUEMain)
	c.Wait()



def BuildPythonAPIMain(c : ConfigureContext):
	Log('Building Python API')
	pass



def SetupUnrealEngine(c : ConfigureContext):
	Log('Setting up Unreal Engine.')



def UpdateDependencies(c : ConfigureContext):
	for name, dep in dependencies.items():
		Log(f'Updating {name}.')
		try:
			c.Dispatch(UpdateDependency, name, dep)
		except Exception as err:
			Log(f'Failed to update "{name}": {err}')



def GetDefaultCMakeConfigureCommandLine(source_path : Path, build_path : Path) -> list:
	return [
		'cmake',
		'-G', cmake_generator,
		'-S', source_path,
		'-B', build_path,
		'-DCMAKE_C_COMPILER=' + c_compiler,
		'-DCMAKE_CXX_COMPILER=' + cpp_compiler,
		'-DCMAKE_BUILD_TYPE=Release',
		'-DCMAKE_CXX_FLAGS_RELEASE="/MD"'
	]



def DefaultBuild(path : Path):
	LaunchSubprocessImmediate([ 'cmake', '--build', path ], display_output = True)



def DefaultInstall(path : Path, prefix : Path):
	LaunchSubprocessImmediate([ 'cmake', '--install', path, '--prefix', prefix ], display_output = True)



boost_toolset = 'msvc-14.2'
boost_toolset_short = 'vc142'
boost_source_path = build_path / 'boost-source'
boost_build_path = build_path / 'boost-build'
boost_install_path = build_path / 'boost-install'
boost_include_path = boost_install_path / 'include'
boost_library_path = boost_install_path / 'lib'
boost_b2_path = boost_source_path / f'b2{executable_extension}'

def ConfigureBoost():
	if boost_b2_path.exists():
		return
	LaunchSubprocessImmediate(
		[ boost_source_path / f'bootstrap{shell_script_extension}' ],
		working_directory = boost_source_path)

def BuildBoost():
	LaunchSubprocessImmediate([
		boost_b2_path,
		f'-j{parallelism}',
		'--layout=system',
		f'--build-dir={boost_build_path}', # ???
		'--with-system',
		'--with-filesystem',
		'--with-python',
		'--with-date_time',
		'architecture=x86',
		'address-model=64',
		f'toolset={boost_toolset}',
		'variant=release',
		'link=static',
		'runtime-link=shared',
		'threading=multi',
		f'--prefix={boost_build_path}',
		f'--libdir={boost_library_path}',
		f'--includedir={boost_include_path}',
		'install'
	], display_output = True, working_directory = boost_source_path)



eigen_source_path = build_path / 'eigen-source'
eigen_build_path = build_path / 'eigen-build'
eigen_install_path = build_path / 'eigen-install'

# def ConfigureEigen():
# 	pass

# def BuildEigen():
# 	pass



chrono_source_path = build_path / 'chrono-source'
chrono_build_path = build_path / 'chrono-build'
chrono_install_path = build_path / 'chrono-install'

def ConfigureChrono():
	cmd = GetDefaultCMakeConfigureCommandLine(chrono_source_path, chrono_build_path)
	cmd.extend([
		f'-DEIGEN3_INCLUDE_DIR={eigen_source_path}',
		'-DENABLE_MODULE_VEHICLE=ON',
		chrono_source_path
	])
	return LaunchSubprocessImmediate(cmd)

def BuildChrono():
	return DefaultBuild(chrono_build_path)



gtest_source_path = build_path / 'gtest-source'
gtest_build_path = build_path / 'gtest-build'
gtest_install_path = build_path / 'gtest-install'

def ConfigureGTest():
	cmd = GetDefaultCMakeConfigureCommandLine(gtest_source_path, gtest_build_path)
	cmd.extend([
		gtest_source_path
	])
	return LaunchSubprocessImmediate(cmd)

def BuildGTest():
	return DefaultBuild(gtest_build_path)



zlib_source_path = build_path / 'zlib-source'
zlib_build_path = build_path / 'zlib-build'
zlib_install_path = build_path / 'zlib-install'
zlib_library_path = zlib_build_path / f'zlib{library_extension}'

def ConfigureZLib():
	cmd = GetDefaultCMakeConfigureCommandLine(zlib_source_path, zlib_build_path)
	cmd.extend([
		zlib_source_path
	])
	return LaunchSubprocessImmediate(cmd)

def BuildZLib():
	return DefaultBuild(zlib_build_path)



libpng_source_path = build_path / 'libpng-source'
libpng_build_path = build_path / 'libpng-build'
libpng_install_path = build_path / 'libpng-install'

def ConfigureLibPNG():
	cmd = GetDefaultCMakeConfigureCommandLine(libpng_source_path, libpng_build_path)
	cmd.extend([
		'-DPNG_TESTS=OFF',
		'-DPNG_TOOLS=OFF',
		'-DPNG_BUILD_ZLIB=ON',
		f'-DZLIB_INCLUDE_DIRS={zlib_source_path};{zlib_build_path}',
		f'-DZLIB_LIBRARIES={zlib_library_path}',
		libpng_source_path
	])
	return LaunchSubprocessImmediate(cmd)

def BuildLibPNG():
	return DefaultBuild(libpng_build_path)



sqlite_source_path = build_path / 'sqlite-source'
sqlite_build_path = build_path / 'sqlite-build'
sqlite_install_path = build_path / 'sqlite-install'
sqlite_executable_path = sqlite_build_path / f'sqlite{executable_extension}'
sqlite_library_path = sqlite_build_path / f'sqlite{library_extension}'

def BuildSQLite():
	sqlite_build_path.mkdir(exist_ok = True)
	sqlite_sources = glob.glob(str(sqlite_source_path / '**' / '*.c'), recursive = True)

	if os.name == 'nt' and 'clang' in c_compiler:
		if not sqlite_executable_path.exists():
			cmd = [
				c_compiler,
				f'-fuse-ld={linker}', '-march=native', '/O2', '/MD', '/EHsc',
			]
			cmd.extend(sqlite_sources)
			cmd.append('-o')
			cmd.append(sqlite_executable_path)
			LaunchSubprocessImmediate(cmd)
		if not sqlite_library_path.exists():
			cmd = [
				c_compiler,
				f'-fuse-ld={lib}', '-march=native', '/O2', '/MD', '/EHsc',
			]
			cmd.extend(sqlite_sources)
			cmd.append('-o')
			cmd.append(sqlite_library_path)
			LaunchSubprocessImmediate(cmd)
	else:
		pass



proj_source_path = build_path / 'proj-source'
proj_build_path = build_path / 'proj-build'
proj_install_path = build_path / 'proj-install'

def ConfigureProj():
	cmd = GetDefaultCMakeConfigureCommandLine(proj_source_path, proj_build_path)
	cmd.extend([
		f'-DSQLITE3_INCLUDE_DIR={sqlite_source_path}',
		f'-DSQLITE3_LIBRARY={sqlite_library_path}',
		f'-DEXE_SQLITE3={sqlite_executable_path}',
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
		proj_source_path
	])
	return LaunchSubprocessImmediate(cmd)

def BuildProj():
	return DefaultBuild(proj_build_path)



recast_source_path = build_path / 'recast-source'
recast_build_path = build_path / 'recast-build'
recast_install_path = build_path / 'recast-install'

def ConfigureRecast():
	cmd = GetDefaultCMakeConfigureCommandLine(recast_source_path, recast_build_path)
	cmd.extend([
		'-DRECASTNAVIGATION_DEMO=OFF',
		'-DRECASTNAVIGATION_TESTS=OFF',
		'-DRECASTNAVIGATION_EXAMPLES=OFF',
		recast_source_path
	])
	return LaunchSubprocessImmediate(cmd)

def BuildRecast():
	return DefaultBuild(recast_build_path)



rpclib_source_path = build_path / 'rpclib-source'
rpclib_build_path = build_path / 'rpclib-build'
rpclib_install_path = build_path / 'rpclib-install'

def ConfigureRPCLib():
	cmd = GetDefaultCMakeConfigureCommandLine(rpclib_source_path, rpclib_build_path)
	cmd.extend([
		'-DRPCLIB_BUILD_TESTS=OFF',
		'-DRPCLIB_GENERATE_COMPDB=OFF',
		'-DRPCLIB_BUILD_EXAMPLES=OFF',
		'-DRPCLIB_ENABLE_LOGGING=OFF',
		'-DRPCLIB_ENABLE_COVERAGE=OFF',
		'-DRPCLIB_MSVC_STATIC_RUNTIME=OFF',
		rpclib_source_path
	])
	return LaunchSubprocessImmediate(cmd)

def BuildRPCLib():
	return DefaultBuild(rpclib_build_path)



xercesc_source_path = build_path / 'xercesc-source'
xercesc_build_path = build_path / 'xercesc-build'
xercesc_install_path = build_path / 'xercesc-install'

def ConfigureXercesc():
	cmd = GetDefaultCMakeConfigureCommandLine(xercesc_source_path, xercesc_build_path)
	cmd.extend([
		xercesc_source_path
	])
	return LaunchSubprocessImmediate(cmd)

def BuildXercesc():
	return DefaultBuild(xercesc_build_path)



libosmscout_source_path = build_path / 'libosmscout-source'
libosmscout_build_path = build_path / 'libosmscout-build'
libosmscout_install_path = build_path / 'libosmscout-install'

def ConfigureLibOSMScout():
	cmd = GetDefaultCMakeConfigureCommandLine(libosmscout_source_path, libosmscout_build_path)
	cmd.extend([
		'-DOSMSCOUT_BUILD_TOOL_STYLEEDITOR=OFF',
		'-DOSMSCOUT_BUILD_TOOL_OSMSCOUT2=OFF',
		'-DOSMSCOUT_BUILD_TESTS=OFF',
		'-DOSMSCOUT_BUILD_CLIENT_QT=OFF',
		'-DOSMSCOUT_BUILD_DEMOS=OFF',
		libosmscout_source_path
	])
	return LaunchSubprocessImmediate(cmd)

def BuildLibOSMScout():
	return DefaultBuild(libosmscout_build_path)



lunasvg_source_path = build_path / 'lunasvg-source'
lunasvg_build_path = build_path / 'lunasvg-build'
lunasvg_install_path = build_path / 'lunasvg-install'

def ConfigureLunaSVG():
	cmd = GetDefaultCMakeConfigureCommandLine(lunasvg_source_path, lunasvg_build_path)
	cmd.extend([
		lunasvg_source_path
	])
	return LaunchSubprocessImmediate(cmd)

def BuildLunaSVG():
	return DefaultBuild(lunasvg_build_path)



sumo_source_path = build_path / 'sumo-source'
sumo_build_path = build_path / 'sumo-build'
sumo_install_path = build_path / 'sumo-install'

def ConfigureSUMO():
	cmd = GetDefaultCMakeConfigureCommandLine(sumo_source_path, sumo_build_path)
	xercesc_library = glob.glob(xercesc_install_path / 'lib' / f'*{library_extension}', recursive = True)[0]
	cmd.extend([
		'-DSUMO_LIBRARIES=OFF',
		f'-DZLIB_INCLUDE_DIR={zlib_source_path}',
		f'-DZLIB_LIBRARY={zlib_install_path}/zlib{library_extension}',
        f'-DPROJ_INCLUDE_DIR={proj_install_path}/include',
        f'-DPROJ_LIBRARY={proj_install_path}/lib/proj{library_extension}',
        f'-DXercesC_INCLUDE_DIR={xercesc_install_path}/include',
        f'-DXercesC_LIBRARY={xercesc_library}',
		sumo_source_path
	])
	return LaunchSubprocessImmediate(cmd)

def BuildSUMO():
	return DefaultBuild(sumo_build_path)



def BuildDependencies(c : ConfigureContext):
	Log('--- BUILDING SQLITE ---')
	c.Dispatch(BuildSQLite)
	Log('--- CONFIGURING ZLIB ---')
	c.Dispatch(ConfigureZLib)
	Log('--- BUILDING ZLIB ---')
	c.Dispatch(BuildZLib)
	Log('--- CONFIGURING BOOST ---')
	c.Dispatch(ConfigureBoost)
	Log('--- CONFIGURING GOOGLETEST ---')
	c.Dispatch(ConfigureGTest)
	Log('--- CONFIGURING LIBPNG ---')
	c.Dispatch(ConfigureLibPNG)
	Log('--- CONFIGURING PROJ ---')
	c.Dispatch(ConfigureProj)
	Log('--- CONFIGURING RECAST ---')
	c.Dispatch(ConfigureRecast)
	Log('--- CONFIGURING RPCLIB ---')
	c.Dispatch(ConfigureRPCLib)
	Log('--- CONFIGURING XERCES-C ---')
	c.Dispatch(ConfigureXercesc)
	Log('--- CONFIGURING LIBOSMSCOUT ---')
	c.Dispatch(ConfigureLibOSMScout)
	Log('--- CONFIGURING LUNASVG ---')
	c.Dispatch(ConfigureLunaSVG)
	Log('--- CONFIGURING SUMO ---')
	c.Dispatch(ConfigureSUMO)
	if c.arg.use_chrono:
		Log('--- CONFIGURING CHRONO ---')
		c.Dispatch(ConfigureChrono)
	c.Wait()
	Log('--- BUILDING BOOST ---')
	BuildBoost()
	Log('--- BUILDING GOOGLETEST ---')
	BuildGTest()
	Log('--- BUILDING LIBPNG ---')
	BuildLibPNG()
	Log('--- BUILDING PROJ ---')
	BuildProj()
	Log('--- BUILDING RECAST ---')
	BuildRecast()
	Log('--- BUILDING RPCLIB ---')
	BuildRPCLib()
	Log('--- BUILDING XERCESC ---')
	BuildXercesc()
	Log('--- BUILDING LUNASVG ---')
	BuildLunaSVG()
	Log('--- BUILDING LIBOSMSCOUT ---')
	BuildLibOSMScout()
	Log('--- BUILDING SUMO ---')
	BuildSUMO()
	if c.arg.use_chrono:
		Log('--- BUILDING CHRONO ---')
		BuildChrono()
	c.Wait()
	DefaultInstall(gtest_build_path, gtest_install_path)
	DefaultInstall(libpng_build_path, libpng_install_path)
	DefaultInstall(proj_build_path, proj_install_path)
	DefaultInstall(recast_build_path, recast_install_path)
	DefaultInstall(rpclib_build_path, rpclib_install_path)
	DefaultInstall(xercesc_build_path, xercesc_install_path)
	DefaultInstall(libosmscout_build_path, libosmscout_install_path)
	if c.arg.use_chrono:
		DefaultInstall(chrono_build_path, chrono_install_path)



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
	Log('Started.')

	build_path.mkdir(exist_ok = True)
	
	arg = ParseCommandLine()
	
	c = ConfigureContext(arg)

	if arg.clean:
		try:
			shutil.rmtree(build_path)
		finally:
			Log(f'Failed to remove {build_path}.')
			exit(-1)

	if arg.update_dependencies or True:
		UpdateDependencies(c)
		c.Wait()
		
	for ext in [ '*.tmp', '*.zip', '*.tar.gz' ]:
		for e in build_path.glob(ext):
			e.unlink(missing_ok = True)
	
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
	
	Log('Done.')

if __name__ == '__main__':
	try:
		Main()
	except Exception as err:
		Log(err)
		URL_SUFFIX = 'how_to_build_on_windows/\n' if os.name == "nt" else 'build_linux/\n'
		ERROR_MESSAGE = (
			'\n'
			'Ok, an error ocurred, don\'t panic!\n'
			'We have different platforms where you can find some help:\n'
			'\n'
			'- Make sure you have read the documentation:\n'
			f'    https://carla.readthedocs.io/en/latest/{URL_SUFFIX}'
			'\n'
			'- If the problem persists, submit an issue on our GitHub page:\n'
			'    https://github.com/carla-simulator/carla/issues\n'
			'\n'
			'- Or just use our Discord server!\n'
			'    We\'ll be glad to help you there:\n'
			'    https://discord.gg/42KJdRj\n'
		)
		Log(ERROR_MESSAGE)
		exit(-1)