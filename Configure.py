from concurrent.futures import ProcessPoolExecutor, as_completed
from argparse import ArgumentParser
from pathlib import Path
import subprocess, tarfile, zipfile, requests, psutil, shutil, glob, json, sys, os

def TestForExecutablePresence(name):
	if os.name == 'nt':
		return subprocess.call(
			[ 'where', name ],
			stdout = subprocess.PIPE,
			stderr = subprocess.PIPE,
			shell = True) == 0
	else:
		return subprocess.call(
			[ 'whereis', name ],
			stdout = subprocess.PIPE,
			stderr = subprocess.PIPE,
			shell = True) == 0

def FindExistingExecutable(candidates : list):
	for e in candidates:
		if TestForExecutablePresence(e):
			return e
	return None

c_compiler_list = [
	'clang-cl',
	'cl'
] if os.name == 'nt' else [
	'clang',
	'gcc'
]

cpp_compiler_list = [
	'clang-cl',
	'cl'
] if os.name == 'nt' else [
	'clang++',
	'g++'
]

linker_list = [
	'llvm-link',
	'link'
] if os.name == 'nt' else [
	'lld',
	'ld'
]

library_list = [
	'llvm-lib',
	'lib',
	'llvm-ar'
] if os.name == 'nt' else [
	'llvm-ar',
	'ar'
]

c_compiler = FindExistingExecutable(c_compiler_list)
cpp_compiler = FindExistingExecutable(cpp_compiler_list)
linker = FindExistingExecutable(linker_list)
lib = FindExistingExecutable(library_list)

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

readthedocs_build_url = 'http://carla.readthedocs.io/en/latest/' + (
	'how_to_build_on_windows/' if os.name == "nt" else 'build_linux/'
)

error_message = (
	f'\n'
	f'Ok, an error ocurred, don\'t panic!\n'
	f'We have different platforms where you can find some help:\n'
	f'\n'
	f'- Make sure you have read the documentation:\n'
	f'    {readthedocs_build_url}\n'
	f'\n'
	f'- If the problem persists, submit an issue on our GitHub page:\n'
	f'    https://github.com/carla-simulator/carla/issues\n'
	f'\n'
	f'- Or just use our Discord server!\n'
	f'    We\'ll be glad to help you there:\n'
	f'    https://discord.gg/42KJdRj\n'
)



class ConfigureContext:

	def __init__(self, arg):
		self.pool = ProcessPoolExecutor(parallelism)
		self.futures = []
		self.arg = arg
	
	def Dispatch(self, callable, arg = None):
		if arg is None:
			if force_sequential:
				callable()
			else:
				self.futures.append(self.pool.submit(callable))
		else:
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



def UpdateGitRepository(path : Path, url : str, branch : str = None):
	if path.exists():
		LaunchSubprocess([
			'git',
			'-C', str(path),
			'pull'
		]).check_returncode()
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
		LaunchSubprocess(cmd).check_returncode()



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



def UpdateDependency(dep : dict):
	name = dep['name']
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
	return LaunchSubprocess(cmd)



def ConfigureLibCarlaServer():
	cmd = [ 'cmake' ]
	return LaunchSubprocess(cmd)



def BuildLibCarlaClient():
	ConfigureLibCarlaClient().check_returncode()



def BuildLibCarlaServer():
	ConfigureLibCarlaServer().check_returncode()



def BuildLibCarlaMain(c : ConfigureContext):
	if not c.arg.skip_libcarla_server:
		Log('Building LibCarla Server')
		c.Dispatch(BuildLibCarlaServer)
	if not c.arg.skip_libcarla_client:
		Log('Building LibCarla Client')
		c.Dispatch(BuildLibCarlaClient)



def BuildCarlaUEMain():
	if os.name == 'nt':
		LaunchSubprocess([
			ue_workspace_path / 'Engine' / 'Build' / 'BatchFiles' / 'Build.bat',
			'CarlaUE4', 'Win64', 'Development', '-WaitMutex', '-FromMsBuild',
			carla_ue_path / 'CarlaUE4.uproject'
		]).check_returncode()
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
	dependencies = []
	with open(dependency_list_file_path, 'r') as file:
		dependencies = json.load(file)
	for dep in dependencies:
		name = dep['name']
		Log(f'Updating {name}.')
		try:
			c.Dispatch(UpdateDependency, dep)
		except Exception as err:
			Log(f'Failed to update "{name}": {err}')



def DefaultBuild(path : Path):
	LaunchSubprocess([ 'cmake', '--build', path ], display_output = True).check_returncode()



def DefaultInstall(path : Path, prefix : Path):
	LaunchSubprocess([ 'cmake', '--install', path, '--prefix', prefix ], display_output = True).check_returncode()



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
	LaunchSubprocess(
		[ boost_source_path / f'bootstrap{shell_script_extension}' ],
		working_directory = boost_source_path).check_returncode()

def BuildBoost():
	LaunchSubprocess([
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
	], display_output = True, working_directory = boost_source_path).check_returncode()



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
	LaunchSubprocess([
		'cmake',
		'-G', cmake_generator,
		'-S', chrono_source_path,
		'-B', chrono_build_path,
		'-DCMAKE_C_COMPILER=' + c_compiler,
		'-DCMAKE_CXX_COMPILER=' + cpp_compiler,
		'-DCMAKE_BUILD_TYPE=Release',
		'-DCMAKE_CXX_FLAGS_RELEASE="/MD"',
		f'-DEIGEN3_INCLUDE_DIR={eigen_source_path}',
		'-DENABLE_MODULE_VEHICLE=ON',
		chrono_source_path
	]).check_returncode()

def BuildChrono():
	return DefaultBuild(chrono_build_path)



gtest_source_path = build_path / 'gtest-source'
gtest_build_path = build_path / 'gtest-build'
gtest_install_path = build_path / 'gtest-install'

def ConfigureGTest():
	LaunchSubprocess([
		'cmake',
		'-G', cmake_generator,
		'-S', gtest_source_path,
		'-B', gtest_build_path,
		'-DCMAKE_C_COMPILER=' + c_compiler,
		'-DCMAKE_CXX_COMPILER=' + cpp_compiler,
		'-DCMAKE_BUILD_TYPE=Release',
		'-DCMAKE_CXX_FLAGS_RELEASE="/MD"',
		gtest_source_path
	]).check_returncode()

def BuildGTest():
	return DefaultBuild(gtest_build_path)



zlib_source_path = build_path / 'zlib-source'
zlib_build_path = build_path / 'zlib-build'
zlib_install_path = build_path / 'zlib-install'
zlib_library_path = zlib_build_path / f'zlib{library_extension}'

def ConfigureZLib():
	LaunchSubprocess([
		'cmake',
		'-G', cmake_generator,
		'-S', zlib_source_path,
		'-B', zlib_build_path,
		'-DCMAKE_C_COMPILER=' + c_compiler,
		'-DCMAKE_CXX_COMPILER=' + cpp_compiler,
		'-DCMAKE_BUILD_TYPE=Release',
		'-DCMAKE_CXX_FLAGS_RELEASE="/MD"',
		zlib_source_path
	]).check_returncode()

def BuildZLib():
	return DefaultBuild(zlib_build_path)



libpng_source_path = build_path / 'libpng-source'
libpng_build_path = build_path / 'libpng-build'
libpng_install_path = build_path / 'libpng-install'

def ConfigureLibPNG():
	LaunchSubprocess([
		'cmake',
		'-G', cmake_generator,
		'-S', libpng_source_path,
		'-B', libpng_build_path,
		'-DCMAKE_C_COMPILER=' + c_compiler,
		'-DCMAKE_CXX_COMPILER=' + cpp_compiler,
		'-DCMAKE_BUILD_TYPE=Release',
		'-DCMAKE_CXX_FLAGS_RELEASE="/MD"',
		'-DPNG_TESTS=OFF',
		'-DPNG_TOOLS=OFF',
		'-DPNG_BUILD_ZLIB=ON',
		f'-DZLIB_INCLUDE_DIRS={zlib_source_path};{zlib_build_path}',
		f'-DZLIB_LIBRARIES={zlib_library_path}',
		libpng_source_path
	]).check_returncode()

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
			LaunchSubprocess(cmd).check_returncode()
		if not sqlite_library_path.exists():
			cmd = [
				c_compiler,
				f'-fuse-ld={lib}', '-march=native', '/O2', '/MD', '/EHsc',
			]
			cmd.extend(sqlite_sources)
			cmd.append('-o')
			cmd.append(sqlite_library_path)
			LaunchSubprocess(cmd).check_returncode()
	else:
		pass



proj_source_path = build_path / 'proj-source'
proj_build_path = build_path / 'proj-build'
proj_install_path = build_path / 'proj-install'

def ConfigureProj():
	LaunchSubprocess([
		'cmake',
		'-G', cmake_generator,
		'-S', proj_source_path,
		'-B', proj_build_path,
		'-DCMAKE_C_COMPILER=' + c_compiler,
		'-DCMAKE_CXX_COMPILER=' + cpp_compiler,
		'-DCMAKE_BUILD_TYPE=Release',
		'-DCMAKE_CXX_FLAGS_RELEASE="/MD"',
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
	]).check_returncode()

def BuildProj():
	return DefaultBuild(proj_build_path)



recast_source_path = build_path / 'recast-source'
recast_build_path = build_path / 'recast-build'
recast_install_path = build_path / 'recast-install'

def ConfigureRecast():
	LaunchSubprocess([
		'cmake',
		'-G', cmake_generator,
		'-S', recast_source_path,
		'-B', recast_build_path,
		'-DCMAKE_C_COMPILER=' + c_compiler,
		'-DCMAKE_CXX_COMPILER=' + cpp_compiler,
		'-DCMAKE_CXX_FLAGS_RELEASE="/MD"',
		'-DCMAKE_BUILD_TYPE=Release',
		'-DRECASTNAVIGATION_DEMO=OFF',
		'-DRECASTNAVIGATION_TESTS=OFF',
		'-DRECASTNAVIGATION_EXAMPLES=OFF',
		recast_source_path
	]).check_returncode()

def BuildRecast():
	return DefaultBuild(recast_build_path)



rpclib_source_path = build_path / 'rpclib-source'
rpclib_build_path = build_path / 'rpclib-build'
rpclib_install_path = build_path / 'rpclib-install'

def ConfigureRPCLib():
	LaunchSubprocess([
		'cmake',
		'-G', cmake_generator,
		'-S', rpclib_source_path,
		'-B', rpclib_build_path,
		'-DCMAKE_C_COMPILER=' + c_compiler,
		'-DCMAKE_CXX_COMPILER=' + cpp_compiler,
		'-DCMAKE_CXX_FLAGS_RELEASE="/MD"',
		'-DCMAKE_BUILD_TYPE=Release',
		'-DRPCLIB_BUILD_TESTS=OFF',
		'-DRPCLIB_GENERATE_COMPDB=OFF',
		'-DRPCLIB_BUILD_EXAMPLES=OFF',
		'-DRPCLIB_ENABLE_LOGGING=OFF',
		'-DRPCLIB_ENABLE_COVERAGE=OFF',
		'-DRPCLIB_MSVC_STATIC_RUNTIME=OFF',
		rpclib_source_path
	]).check_returncode()

def BuildRPCLib():
	return DefaultBuild(rpclib_build_path)



xercesc_source_path = build_path / 'xercesc-source'
xercesc_build_path = build_path / 'xercesc-build'
xercesc_install_path = build_path / 'xercesc-install'

def ConfigureXercesc():
	LaunchSubprocess([
		'cmake',
		'-G', cmake_generator,
		'-S', xercesc_source_path,
		'-B', xercesc_build_path,
		'-DCMAKE_C_COMPILER=' + c_compiler,
		'-DCMAKE_CXX_COMPILER=' + cpp_compiler,
		'-DCMAKE_CXX_FLAGS_RELEASE="/MD"',
		'-DCMAKE_BUILD_TYPE=Release',
		xercesc_source_path
	]).check_returncode()

def BuildXercesc():
	return DefaultBuild(xercesc_build_path)



def BuildDependencies(c : ConfigureContext):
	Log('--- BUILDING SQLITE ---')
	c.Dispatch(BuildSQLite)
	Log('--- CONFIGURING ZLIB ---')
	c.Dispatch(ConfigureZLib)
	c.Wait()
	Log('--- BUILDING ZLIB ---')
	c.Dispatch(BuildZLib)
	Log('--- CONFIGURING BOOST ---')
	c.Dispatch(ConfigureBoost)
	Log('--- CONFIGURING GOOGLETEST ---')
	c.Dispatch(ConfigureGTest)
	Log('--- CONFIGURING LIBPNG ---')
	c.Dispatch(ConfigureLibPNG)
	c.Wait()
	Log('--- CONFIGURING PROJ ---')
	c.Dispatch(ConfigureProj)
	Log('--- CONFIGURING RECAST ---')
	c.Dispatch(ConfigureRecast)
	Log('--- CONFIGURING RPCLIB ---')
	c.Dispatch(ConfigureRPCLib)
	Log('--- CONFIGURING XERCES-C ---')
	c.Dispatch(ConfigureXercesc)
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
		Log(error_message)
		exit(-1)