from concurrent.futures import ProcessPoolExecutor, as_completed
from argparse import ArgumentParser
from pathlib import Path
import subprocess, tarfile, zipfile, requests, psutil, shutil, glob, json, sys, os

FORCE_SEQUENTIAL = False

# Script types

class Download:
	def __init__(self, url):
		self.url = url

class GitRepository:
	def __init__(self, url, tag : str = None):
		self.url = url
		self.tag = tag

class Dependency:
	def __init__(
			self,
			name : str,
			*sources):
		self.name = name
		self.sources = [ e for e in sources ]
		assert all(type(e) is Download or type(e) is GitRepository for e in self.sources)

class Task:
	def __init__(
			self,
			name : str,
			in_edges : list,
			body,
			*args):
		self.name = name
		self.body = body
		self.args = args
		self.in_edges = in_edges
		self.out_edges = [] # Filled right before task graph execution.

	def CreateSubprocessTask(name : str, in_edges : list, command : list):
		return Task(name, in_edges, LaunchSubprocessImmediate, command)
	
	def CreateCMakeConfigureDefault(
			name : str,
			in_edges : list,
			source_path : Path,
			build_path : Path,
			*args):
		cmd = [
			'cmake',
			'-G', CMAKE_GENERATOR,
			'-S', source_path,
			'-B', build_path,
			'-DCMAKE_C_COMPILER=' + c_compiler,
			'-DCMAKE_CXX_COMPILER=' + cpp_compiler,
			'-DCMAKE_BUILD_TYPE=Release',
			'-DCMAKE_CXX_FLAGS_RELEASE="/MD"'
		]
		cmd.extend([ *args ])
		cmd.append(source_path)
		return Task.CreateSubprocessTask(name, in_edges, cmd)

	def CreateCMakeBuildDefault(
			name : str,
			in_edges : list,
			build_path : Path,
			*args):
		cmd = [ 'cmake', '--build', build_path ]
		cmd.extend([ *args ])
		return Task.CreateSubprocessTask(name, in_edges, cmd)
	
	def CreateCMakeInstallDefault(
			name : str,
			in_edges : list,
			build_path : Path,
			install_path : Path,
			*args):
		cmd = [ 'cmake', '--install', build_path, '--prefix', install_path ]
		cmd.extend([ *args ])
		return Task.CreateSubprocessTask(name, in_edges, cmd)
	
	def Run(self):
		self.body(*self.args)

class TaskGraph:
	def __init__(self, parallelism : int = None):
		self.sequential = FORCE_SEQUENTIAL
		self.pool = ProcessPoolExecutor(parallelism)
		self.futures = []
		self.tasks = []
		self.sources = []
		self.task_map = {}
	
	def Reset(self):
		self.futures = []
		self.tasks = []
		self.sources = []
		self.task_map = {}

	def Add(self, task):
		self.tasks.append(task)
		if len(task.in_edges) == 0:
			self.sources.append(task.name)
		self.task_map[task.name] = self.tasks[-1]

	def Execute(self):
		for e in self.tasks:
			for in_edge in e.in_edges:
				self.task_map[in_edge].out_edges.append(e)
		for e in self.sources:
			task = self.task_map.get(e, None)
			assert task != None and type(task) is Task
			if not self.sequential:
				self.futures.append(self.pool.submit(task.Run))
			else:
				task.Run()
		if not self.sequential:
			for e in as_completed(self.futures):
				e.result()
		self.Reset()

class Context:
	def __init__(self, args, parallelism):
		self.task_graph = TaskGraph(parallelism)
		self.args = args
		pass

# Constants:

DEFAULT_DEPENDENCIES = [
	Dependency(
		'boost',
		Download('https://boostorg.jfrog.io/artifactory/main/release/1.83.0/source/boost_1_83_0.zip'),
		Download('https://carla-releases.s3.eu-west-3.amazonaws.com/Backup/boost_1_83_0.zip')),
	Dependency(
		'chrono',
		GitRepository('https://github.com/projectchrono/chrono.git', tag = '8.0.0')),
	Dependency(
		'eigen',
		GitRepository('https://gitlab.com/libeigen/eigen.git', tag = '3.4.0')),
	Dependency(
		'libpng',
		GitRepository('https://github.com/glennrp/libpng.git', tag = 'v1.6.40')),
	Dependency(
		'proj',
		GitRepository('https://github.com/OSGeo/PROJ.git', tag = '9.3.0'),
		Download('https://download.osgeo.org/proj/proj-9.3.0.tar.gz')),
	Dependency(
		'gtest',
		GitRepository('https://github.com/google/googletest.git', tag = 'v1.14.0')),
	Dependency(
		'zlib',
		Download('https://zlib.net/current/zlib.tar.gz'),
		GitRepository('https://github.com/madler/zlib.git', tag = 'v1.3')),
	Dependency(
		'xercesc',
		GitRepository('https://github.com/apache/xerces-c.git', tag = 'v3.2.4'),
		Download('https://archive.apache.org/dist/xerces/c/3/sources/xerces-c-3.2.3.zip'),
		Download('https://carla-releases.s3.eu-west-3.amazonaws.com/Backup/xerces-c-3.2.3.zip')),
	Dependency(
		'sqlite',
		Download('https://www.sqlite.org/2021/sqlite-amalgamation-3340100.zip')),
	Dependency(
		'rpclib',
		GitRepository('https://github.com/rpclib/rpclib.git', tag = 'v2.3.0')),
	Dependency(
		'recast',
		GitRepository('https://github.com/recastnavigation/recastnavigation.git', tag = 'v1.6.0')),
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
		'sumo',
		GitRepository('https://github.com/carla-simulator/sumo.git')),
]

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
WORKSPACE_PATH = Path(__file__).parent.resolve()
LIBCARLA_ROOT_PATH = WORKSPACE_PATH / 'LibCarla'
PYTHON_API_PATH = WORKSPACE_PATH / 'PythonAPI'
EXAMPLES_PATH = WORKSPACE_PATH / 'Examples'
BUILD_PATH = WORKSPACE_PATH / 'Build'
LIBCARLA_INSTALL_PATH = BUILD_PATH
DEPENDENCIES_PATH = BUILD_PATH / 'Dependencies'
DIST_PATH = WORKSPACE_PATH / 'Dist'
UTIL_PATH = WORKSPACE_PATH / 'Util'
DOCKER_UTILS_PATH = UTIL_PATH / 'DockerUtils'
BUILD_TOOLS_PATH = UTIL_PATH / 'BuildTools'
# UE plugin
CARLA_UE_PATH = WORKSPACE_PATH / 'Unreal' / 'CarlaUE4'
CARLA_UE_PLUGIN_ROOT_PATH = CARLA_UE_PATH / 'Plugins'
CARLA_UE_PLUGIN_PATH = CARLA_UE_PLUGIN_ROOT_PATH / 'Carla'
CARLA_UE_PLUGIN_DEPENDENCIES_PATH = CARLA_UE_PLUGIN_ROOT_PATH / 'CarlaDependencies'
# PythonAPI
PYTHON_API_SOURCE_PATH = PYTHON_API_PATH / 'carla'
# LibCarla
LIBCARLA_BUILD_PATH = BUILD_PATH
LIBCARLA_TEST_CONTENT_PATH = BUILD_PATH / 'test-content'
# OSM2ODR
OSM2ODR_BUILD_PATH = BUILD_PATH / 'libosm2dr-build'
OSM2ODR_BUILD_PATH_SERVER = BUILD_PATH / 'libosm2dr-build-server'
OSM2ODR_SOURCE_PATH = BUILD_PATH / 'libosm2dr-source'
# Misc
TEST_RESULTS_PATH = BUILD_PATH / 'test-results'
LIBSTDCPP_TOOLCHAIN_PATH = BUILD_PATH / 'LibStdCppToolChain.cmake'
LIBCPP_TOOLCHAIN_PATH = BUILD_PATH / 'LibCppToolChain.cmake'
EXE_EXT = '.exe' if os.name == 'nt' else ''
LIB_EXT = '.lib' if os.name == 'nt' else '.a'
OBJ_EXT = '.obj' if os.name == 'nt' else '.o'
SHELL_EXT = '.bat' if os.name == 'nt' else ''
# Unreal Engine
UE_WORKSPACE_PATH = Path(os.getenv('UE4_ROOT', ''))
if not UE_WORKSPACE_PATH.exists():
	print('Could not find Unreal Engine workspace. Please set the environment variable UE4_ROOT as specified in the docs.')
	exit(-1)
# Omniverse
NV_OMNIVERSE_PLUGIN_PATH = UE_WORKSPACE_PATH / 'Engine' / 'Plugins' / 'Marketplace' / 'NVIDIA' / 'Omniverse'
NV_OMNIVERSE_PATCH_PATH = UTIL_PATH / 'Patches' / 'omniverse_4.26'
# Script settings
DEFAULT_PARALLELISM = psutil.cpu_count(logical = True)
CMAKE_GENERATOR = 'Ninja'

# Dependency paths
BOOST_TOOLSET = 'msvc-14.2'
BOOST_TOOLSET_SHORT = 'vc142'
BOOST_SOURCE_PATH = DEPENDENCIES_PATH / 'boost-source'
BOOST_BUILD_PATH = DEPENDENCIES_PATH / 'boost-build'
BOOST_INSTALL_PATH = DEPENDENCIES_PATH / 'boost-install'
BOOST_INCLUDE_PATH = BOOST_INSTALL_PATH / 'include'
BOOST_LIBRARY_PATH = BOOST_INSTALL_PATH / 'lib'
BOOST_B2_PATH = BOOST_SOURCE_PATH / f'b2{EXE_EXT}'

EIGEN_SOURCE_PATH = DEPENDENCIES_PATH / 'eigen-source'
EIGEN_BUILD_PATH = DEPENDENCIES_PATH / 'eigen-build'
EIGEN_INSTALL_PATH = DEPENDENCIES_PATH / 'eigen-install'

CHRONO_SOURCE_PATH = DEPENDENCIES_PATH / 'chrono-source'
CHRONO_BUILD_PATH = DEPENDENCIES_PATH / 'chrono-build'
CHRONO_INSTALL_PATH = DEPENDENCIES_PATH / 'chrono-install'

GTEST_SOURCE_PATH = DEPENDENCIES_PATH / 'gtest-source'
GTEST_BUILD_PATH = DEPENDENCIES_PATH / 'gtest-build'
GTEST_INSTALL_PATH = DEPENDENCIES_PATH / 'gtest-install'

ZLIB_SOURCE_PATH = DEPENDENCIES_PATH / 'zlib-source'
ZLIB_BUILD_PATH = DEPENDENCIES_PATH / 'zlib-build'
ZLIB_INSTALL_PATH = DEPENDENCIES_PATH / 'zlib-install'
ZLIB_LIBRARY_PATH = ZLIB_BUILD_PATH / f'zlib{LIB_EXT}'

LIBPNG_SOURCE_PATH = DEPENDENCIES_PATH / 'libpng-source'
LIBPNG_BUILD_PATH = DEPENDENCIES_PATH / 'libpng-build'
LIBPNG_INSTALL_PATH = DEPENDENCIES_PATH / 'libpng-install'

SQLITE_SOURCE_PATH = DEPENDENCIES_PATH / 'sqlite-source'
SQLITE_BUILD_PATH = DEPENDENCIES_PATH / 'sqlite-build'
SQLITE_INSTALL_PATH = DEPENDENCIES_PATH / 'sqlite-install'
SQLITE_EXECUTABLE_PATH = SQLITE_BUILD_PATH / f'sqlite{EXE_EXT}'
SQLITE_LIBRARY_PATH = SQLITE_BUILD_PATH / f'sqlite{LIB_EXT}'

PROJ_SOURCE_PATH = DEPENDENCIES_PATH / 'proj-source'
PROJ_BUILD_PATH = DEPENDENCIES_PATH / 'proj-build'
PROJ_INSTALL_PATH = DEPENDENCIES_PATH / 'proj-install'

RECAST_SOURCE_PATH = DEPENDENCIES_PATH / 'recast-source'
RECAST_BUILD_PATH = DEPENDENCIES_PATH / 'recast-build'
RECAST_INSTALL_PATH = DEPENDENCIES_PATH / 'recast-install'

RPCLIB_SOURCE_PATH = DEPENDENCIES_PATH / 'rpclib-source'
RPCLIB_BUILD_PATH = DEPENDENCIES_PATH / 'rpclib-build'
RPCLIB_INSTALL_PATH = DEPENDENCIES_PATH / 'rpclib-install'

XERCESC_SOURCE_PATH = DEPENDENCIES_PATH / 'xercesc-source'
XERCESC_BUILD_PATH = DEPENDENCIES_PATH / 'xercesc-build'
XERCESC_INSTALL_PATH = DEPENDENCIES_PATH / 'xercesc-install'

LIBOSMSCOUT_SOURCE_PATH = DEPENDENCIES_PATH / 'libosmscout-source'
LIBOSMSCOUT_BUILD_PATH = DEPENDENCIES_PATH / 'libosmscout-build'
LIBOSMSCOUT_INSTALL_PATH = DEPENDENCIES_PATH / 'libosmscout-install'

LUNASVG_SOURCE_PATH = DEPENDENCIES_PATH / 'lunasvg-source'
LUNASVG_BUILD_PATH = DEPENDENCIES_PATH / 'lunasvg-build'
LUNASVG_INSTALL_PATH = DEPENDENCIES_PATH / 'lunasvg-install'

SUMO_SOURCE_PATH = DEPENDENCIES_PATH / 'sumo-source'
SUMO_BUILD_PATH = DEPENDENCIES_PATH / 'sumo-build'
SUMO_INSTALL_PATH = DEPENDENCIES_PATH / 'sumo-install'

houdini_url = 'https://github.com/sideeffects/HoudiniEngineForUnreal.git'
houdini_plugin_path = CARLA_UE_PLUGIN_ROOT_PATH / 'HoudiniEngine'
houdini_commit_hash = '55b6a16cdf274389687fce3019b33e3b6e92a914'
houdini_patch_path = UTIL_PATH / 'Patches' / 'houdini_patch.txt'

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

def Log(message):
	message = str(message)
	message += '\n'
	print(message, end='')

def LaunchSubprocess(
		cmd : list,
		display_output : bool = False,
		working_directory : Path = None):
	
	if display_output or FORCE_SEQUENTIAL:
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

def UpdateDependency(dep : Dependency):
	name = dep.name
	Log(f'Updating {name}.')
	download_path = DEPENDENCIES_PATH / f'{name}-source'
	for source in dep.sources:
		try:
			if type(source) is GitRepository:
				branch = source.tag
				UpdateGitRepository(download_path, source.url, branch)
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

def BuildLibCarlaMain(c : Context):
	c.task_graph.Add(Task.CreateCMakeConfigureDefault('configure-libcarla', [], WORKSPACE_PATH, BUILD_PATH,
		f'-DBUILD_LIBCARLA_SERVER={"ON" if c.args.build_libcarla_server else "OFF"}',
		f'-DBUILD_LIBCARLA_CLIENT={"ON" if c.args.build_libcarla_client else "OFF"}',
		f'-DBUILD_OSM_WORLD_RENDERER={"ON" if c.args.build_osm_world_renderer else "OFF"}',
		f'-DLIBCARLA_PYTORCH={"ON" if c.args.build_libcarla_pytorch else "OFF"}'))
	c.task_graph.Add(Task.CreateCMakeBuildDefault('build-libcarla', [], BUILD_PATH))
	c.task_graph.Add(Task.CreateCMakeInstallDefault('install-libcarla', [ 'build-libcarla' ], BUILD_PATH, LIBCARLA_INSTALL_PATH))

def BuildCarlaUECore(c : Context):
	if os.name == 'nt':
		LaunchSubprocessImmediate([
			UE_WORKSPACE_PATH / 'Engine' / 'Build' / 'BatchFiles' / 'Build.bat',
			'CarlaUE4', 'Win64', 'Development', '-WaitMutex', '-FromMsBuild',
			CARLA_UE_PATH / 'CarlaUE4.uproject'
		])
	else:
		pass

def BuildCarlaUEMain(c : Context):
	Log('Building Carla Unreal Engine Editor')
	# c.args.enable_carsim
	# c.args.enable_chrono
	# c.args.enable_unity
	if c.args.enable_omniverse:
		c.task_graph.Add(Task('install-nv_omniverse', [], InstallNVIDIAOmniverse))
	c.task_graph.Add(Task('build-carla_ue', [], BuildCarlaUECore, c))

def BuildPythonAPIMain(c : Context):
	Log('Building Python API')
	pass

def SetupUnrealEngine(c : Context):
	Log('Setting up Unreal Engine.')

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

def UpdateDependencies(c : Context):
	unique_dependencies = set(DEFAULT_DEPENDENCIES)
	if c.args.build_osm_world_renderer:
		unique_dependencies.update(OSM_WORLD_RENDERER_DEPENDENCIES)
	if c.args.build_osm2odr:
		unique_dependencies.update(OSM2ODR_DEPENDENCIES)
	for dep in unique_dependencies:
		name = dep.name
		c.task_graph.Add(Task(f'update-{name}', [], UpdateDependency, dep))
	c.task_graph.Execute()

def ConfigureBoost():
	if BOOST_B2_PATH.exists():
		return
	LaunchSubprocessImmediate(
		[ BOOST_SOURCE_PATH / f'bootstrap{SHELL_EXT}' ],
		working_directory = BOOST_SOURCE_PATH)

def BuildAndInstallBoost():
	LaunchSubprocessImmediate([
		BOOST_B2_PATH,
		f'-j{DEFAULT_PARALLELISM}',
		'architecture=x86',
		'address-model=64',
		f'toolset={BOOST_TOOLSET}',
		'variant=release',
		'link=static',
		'runtime-link=shared',
		'threading=multi',
		'--layout=system',
		'--with-system',
		'--with-filesystem',
		'--with-python',
		'--with-date_time',
		f'--build-dir={BOOST_BUILD_PATH}',
		f'--prefix={BOOST_INSTALL_PATH}',
		f'--libdir={BOOST_LIBRARY_PATH}',
		f'--includedir={BOOST_INCLUDE_PATH}',
		'install'
	], working_directory = BOOST_SOURCE_PATH)

def BuildSQLite():
	SQLITE_BUILD_PATH.mkdir(exist_ok = True)
	sqlite_sources = glob.glob(f'{SQLITE_SOURCE_PATH}/**/*.c', recursive = True)
	if os.name == 'nt' and 'clang' in c_compiler:
		if not SQLITE_EXECUTABLE_PATH.exists():
			cmd = [
				c_compiler,
				f'-fuse-ld={linker}', '-march=native', '/O2', '/MD', '/EHsc',
			]
			cmd.extend(sqlite_sources)
			cmd.append('-o')
			cmd.append(SQLITE_EXECUTABLE_PATH)
			LaunchSubprocessImmediate(cmd)
		if not SQLITE_LIBRARY_PATH.exists():
			cmd = [
				c_compiler,
				f'-fuse-ld={lib}', '-march=native', '/O2', '/MD', '/EHsc',
			]
			cmd.extend(sqlite_sources)
			cmd.append('-o')
			cmd.append(SQLITE_LIBRARY_PATH)
			LaunchSubprocessImmediate(cmd)
	else:
		pass

def FindXercesC():
	return glob.glob(f'{XERCESC_INSTALL_PATH}/**/xerces-c*{LIB_EXT}', recursive=True)[0]

def BuildDependencies(c : Context):

	# Configure:

	c.task_graph.Add(Task('build-sqlite', [], BuildSQLite))

	c.task_graph.Add(Task('configure-boost', [], ConfigureBoost))

	c.task_graph.Add(Task.CreateCMakeConfigureDefault(
		'configure-zlib',
		[],
		ZLIB_SOURCE_PATH,
		ZLIB_BUILD_PATH))

	c.task_graph.Add(Task.CreateCMakeConfigureDefault(
		'configure-gtest',
		[],
		GTEST_SOURCE_PATH,
		GTEST_BUILD_PATH))

	c.task_graph.Add(Task.CreateCMakeConfigureDefault(
		'configure-libpng',
		[],
		LIBPNG_SOURCE_PATH,
		LIBPNG_BUILD_PATH,
		'-DPNG_TESTS=OFF',
		'-DPNG_TOOLS=OFF',
		'-DPNG_BUILD_ZLIB=ON',
		f'-DZLIB_INCLUDE_DIRS={ZLIB_SOURCE_PATH};{ZLIB_BUILD_PATH}',
		f'-DZLIB_LIBRARIES={ZLIB_LIBRARY_PATH}'))

	c.task_graph.Add(Task.CreateCMakeConfigureDefault(
		'configure-proj',
		[],
		PROJ_SOURCE_PATH,
		PROJ_BUILD_PATH,
		f'-DSQLITE3_INCLUDE_DIR={SQLITE_SOURCE_PATH}',
		f'-DSQLITE3_LIBRARY={SQLITE_LIBRARY_PATH}',
		f'-DEXE_SQLITE3={SQLITE_EXECUTABLE_PATH}',
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
		'-DBUILD_TESTING=OFF'))

	c.task_graph.Add(Task.CreateCMakeConfigureDefault(
		'configure-recast',
		[],
		RECAST_SOURCE_PATH,
		RECAST_BUILD_PATH,
		'-DRECASTNAVIGATION_DEMO=OFF',
		'-DRECASTNAVIGATION_TESTS=OFF',
		'-DRECASTNAVIGATION_EXAMPLES=OFF'))

	c.task_graph.Add(Task.CreateCMakeConfigureDefault(
		'configure-rpclib',
		[],
		RPCLIB_SOURCE_PATH,
		RPCLIB_BUILD_PATH,
		'-DRPCLIB_BUILD_TESTS=OFF',
		'-DRPCLIB_GENERATE_COMPDB=OFF',
		'-DRPCLIB_BUILD_EXAMPLES=OFF',
		'-DRPCLIB_ENABLE_LOGGING=OFF',
		'-DRPCLIB_ENABLE_COVERAGE=OFF',
		'-DRPCLIB_MSVC_STATIC_RUNTIME=OFF'))

	c.task_graph.Add(Task.CreateCMakeConfigureDefault(
		'configure-xercesc',
		[],
		XERCESC_SOURCE_PATH,
		XERCESC_BUILD_PATH))

	if c.args.build_osm_world_renderer:
		c.task_graph.Add(Task.CreateCMakeConfigureDefault(
			'configure-libosmscout',
			[],
			LIBOSMSCOUT_SOURCE_PATH,
			LIBOSMSCOUT_BUILD_PATH,
			'-DOSMSCOUT_BUILD_TOOL_STYLEEDITOR=OFF',
			'-DOSMSCOUT_BUILD_TOOL_OSMSCOUT2=OFF',
			'-DOSMSCOUT_BUILD_TESTS=OFF',
			'-DOSMSCOUT_BUILD_CLIENT_QT=OFF',
			'-DOSMSCOUT_BUILD_DEMOS=OFF'))

		c.task_graph.Add(Task.CreateCMakeConfigureDefault(
			'configure-lunasvg',
			[],
			LUNASVG_SOURCE_PATH,
			LUNASVG_BUILD_PATH))

	if c.args.build_osm2odr:
		c.task_graph.Add(Task.CreateCMakeConfigureDefault(
			'configure-sumo',
			[],
			SUMO_SOURCE_PATH,
			SUMO_BUILD_PATH,
			'-DSUMO_LIBRARIES=OFF',
			f'-DZLIB_INCLUDE_DIR={ZLIB_SOURCE_PATH}',
			f'-DZLIB_LIBRARY={ZLIB_INSTALL_PATH}/zlib{LIB_EXT}',
        	f'-DPROJ_INCLUDE_DIR={PROJ_INSTALL_PATH}/include',
        	f'-DPROJ_LIBRARY={PROJ_INSTALL_PATH}/lib/proj{LIB_EXT}',
        	f'-DXercesC_INCLUDE_DIR={XERCESC_INSTALL_PATH}/include',
        	f'-DXercesC_LIBRARY={FindXercesC()}'))

	if c.args.enable_chrono:
		c.task_graph.Add(Task.CreateCMakeConfigureDefault(
			'configure-chrono',
			[],
			CHRONO_SOURCE_PATH,
			CHRONO_BUILD_PATH,
			f'-DEIGEN3_INCLUDE_DIR={EIGEN_SOURCE_PATH}',
			'-DENABLE_MODULE_VEHICLE=ON'))
	c.task_graph.Execute()

	# Build:

	c.task_graph.sequential = True
	c.task_graph.Add(Task('build-boost', [], BuildAndInstallBoost))
	c.task_graph.Add(Task.CreateCMakeBuildDefault('build-zlib', [], ZLIB_BUILD_PATH))
	c.task_graph.Add(Task.CreateCMakeBuildDefault('build-gtest', [], GTEST_BUILD_PATH))
	c.task_graph.Add(Task.CreateCMakeBuildDefault('build-libpng', [], LIBPNG_BUILD_PATH))
	c.task_graph.Add(Task.CreateCMakeBuildDefault('build-proj', [], PROJ_BUILD_PATH))
	c.task_graph.Add(Task.CreateCMakeBuildDefault('build-recast', [], RECAST_BUILD_PATH))
	c.task_graph.Add(Task.CreateCMakeBuildDefault('build-rpclib', [], RPCLIB_BUILD_PATH))
	c.task_graph.Add(Task.CreateCMakeBuildDefault('build-xercesc', [], XERCESC_BUILD_PATH))
	if c.args.build_osm_world_renderer:
		c.task_graph.Add(Task.CreateCMakeBuildDefault('build-lunasvg', [], LUNASVG_BUILD_PATH))
		c.task_graph.Add(Task.CreateCMakeBuildDefault('build-libosmscout', [], LIBOSMSCOUT_BUILD_PATH))
	if c.args.build_osm2odr:
		c.task_graph.Add(Task.CreateCMakeBuildDefault('build-sumo', [], SUMO_BUILD_PATH))
	if c.args.enable_chrono:
		c.task_graph.Add(Task.CreateCMakeBuildDefault('build-chrono', [], CHRONO_BUILD_PATH))
	c.task_graph.Execute()
	c.task_graph.sequential = False
		
	# Install:

	c.task_graph.Add(Task.CreateCMakeInstallDefault('install-gtest', [], GTEST_BUILD_PATH, GTEST_INSTALL_PATH))
	c.task_graph.Add(Task.CreateCMakeInstallDefault('install-libpng', [], LIBPNG_BUILD_PATH, LIBPNG_INSTALL_PATH))
	c.task_graph.Add(Task.CreateCMakeInstallDefault('install-proj', [], PROJ_BUILD_PATH, PROJ_INSTALL_PATH))
	c.task_graph.Add(Task.CreateCMakeInstallDefault('install-recast', [], RECAST_BUILD_PATH, RECAST_INSTALL_PATH))
	c.task_graph.Add(Task.CreateCMakeInstallDefault('install-rpclib', [], RPCLIB_BUILD_PATH, RPCLIB_INSTALL_PATH))
	c.task_graph.Add(Task.CreateCMakeInstallDefault('install-xercesc', [], XERCESC_BUILD_PATH, XERCESC_INSTALL_PATH))
	if c.args.build_osm_world_renderer:
		c.task_graph.Add(Task.CreateCMakeInstallDefault('install-lunasvg', [], LUNASVG_BUILD_PATH, LUNASVG_INSTALL_PATH))
		c.task_graph.Add(Task.CreateCMakeInstallDefault('install-libosmscout', [], LIBOSMSCOUT_BUILD_PATH, LIBOSMSCOUT_INSTALL_PATH))
	if c.args.build_osm2odr:
		c.task_graph.Add(Task.CreateCMakeInstallDefault('install-sumo', [], SUMO_BUILD_PATH, SUMO_INSTALL_PATH))
	if c.args.enable_chrono:
		c.task_graph.Add(Task.CreateCMakeInstallDefault('install-chrono', [], CHRONO_BUILD_PATH, CHRONO_INSTALL_PATH))
	c.task_graph.Execute()



def ParseCommandLine():
	arg_parser = ArgumentParser(description = __doc__)

	BUILD_LIBCARLA_CLIENT_OVERRIDE = True
	BUILD_LIBCARLA_SERVER_OVERRIDE = True
	BUILD_LIBCARLA_PYTORCH_OVERRIDE = True
	BUILD_PYTHON_API_OVERRIDE = True
	BUILD_CARLA_UE_OVERRIDE = False
	UPDATE_DEPENDENCIES_OVERRIDE = True
	BUILD_DEPENDENCIES_OVERRIDE = True
	BUILD_OSM2ODR_OVERRIDE = False
	BUILD_OSM_WORLD_RENDERER_OVERRIDE = False
	BUILD_PACKAGE_OVERRIDE = True
	CLEAN_OVERRIDE = False
	
	arg_parser.add_argument(
		'-build-libcarla-client',
		action='store_true',
		default = BUILD_LIBCARLA_CLIENT_OVERRIDE,
		help = 'Whether to build LibCarla Client.')
	
	arg_parser.add_argument(
		'-build-libcarla-server',
		action='store_true',
		default = BUILD_LIBCARLA_SERVER_OVERRIDE,
		help = 'Whether to build LibCarla Server.')
	
	arg_parser.add_argument(
		'-build-libcarla-pytorch',
		action='store_true',
		default = BUILD_LIBCARLA_PYTORCH_OVERRIDE,
		help = 'Whether to build LibCarla-PyTorch.')
	
	arg_parser.add_argument(
		'-build-python-api',
		action='store_true',
		default = BUILD_PYTHON_API_OVERRIDE,
		help = 'Whether to build the CARLA Python API.')
	
	arg_parser.add_argument(
		'-build-carla-unreal',
		action='store_true',
		default = BUILD_CARLA_UE_OVERRIDE,
		help = 'Build to build the Unreal Engine Carla backend.')
	
	arg_parser.add_argument(
		'-update-dependencies',
		action='store_true',
		default = UPDATE_DEPENDENCIES_OVERRIDE,
		help = 'Whether to update the CARLA dependencies.')
	
	arg_parser.add_argument(
		'-build-dependencies',
		action='store_true',
		default = BUILD_DEPENDENCIES_OVERRIDE,
		help = 'Whether to build the CARLA dependencies.')
	
	arg_parser.add_argument(
		'-build-osm-world-renderer',
		action='store_true',
		default = BUILD_OSM_WORLD_RENDERER_OVERRIDE,
		help = 'Whether to build OSM World Renderer.')
	
	arg_parser.add_argument(
		'-build-osm2odr',
		action='store_true',
		default = BUILD_OSM2ODR_OVERRIDE,
		help = 'Whether to build OSM2ODR.')
	
	arg_parser.add_argument(
		'-package',
		action='store_true',
		default = BUILD_PACKAGE_OVERRIDE,
		help = 'Whether to package Carla.')
	
	arg_parser.add_argument(
		'-clean',
		action='store_true',
		default = CLEAN_OVERRIDE,
		help = 'Clean build files.')
	
	# Carla UE:
	
	arg_parser.add_argument(
		'-enable-carsim',
		action='store_true',
		help = 'Whether to enable plugin "CarSim".')
	
	arg_parser.add_argument(
		'-enable-chrono',
		action='store_true',
		help = 'Whether to enable plugin "Chrono".')
	
	arg_parser.add_argument(
		'-enable-unity',
		action='store_true',
		help = 'Whether to enable plugin "Unity".')
	
	arg_parser.add_argument(
		'-enable-omniverse',
		action='store_true',
		help = 'Whether to enable plugin "NVIDIA Omniverse".')
	
	return arg_parser.parse_args()



def Main():
	Log('Started.')

	BUILD_PATH.mkdir(exist_ok = True)
	DEPENDENCIES_PATH.mkdir(exist_ok = True)
	
	arg = ParseCommandLine()
	c = Context(arg, DEFAULT_PARALLELISM)

	if arg.clean:
		try:
			shutil.rmtree(BUILD_PATH)
		finally:
			Log(f'Failed to remove {BUILD_PATH}.')
			exit(-1)

	if arg.update_dependencies or True:
		UpdateDependencies(c)
		
	for ext in [ '*.tmp', '*.zip', '*.tar.gz' ]:
		for e in DEPENDENCIES_PATH.glob(ext):
			e.unlink(missing_ok = True)
	
	if arg.build_dependencies or True:
		BuildDependencies(c)

	BuildLibCarlaMain(c)

	if arg.build_python_api:
		BuildPythonAPIMain(c)

	if arg.build_carla_unreal:
		BuildCarlaUEMain(c)
	
	c.task_graph.Execute()
	
	Log('Done.')

if __name__ == '__main__':
	try:
		Main()
	except Exception as err:
		Log(err)
		Log(ERROR_MESSAGE)
		exit(-1)