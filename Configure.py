from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path
import subprocess, tarfile, zipfile, argparse, requests, psutil, shutil, glob, json, sys, os

# Constants:
EXE_EXT = '.exe' if os.name == 'nt' else ''
LIB_EXT = '.lib' if os.name == 'nt' else '.a'
OBJ_EXT = '.obj' if os.name == 'nt' else '.o'
SHELL_EXT = '.bat' if os.name == 'nt' else ''
WORKSPACE_PATH = Path(__file__).parent.resolve()
LIBCARLA_PATH = WORKSPACE_PATH / 'LibCarla'
LIBCARLA_SOURCE_PATH = LIBCARLA_PATH / 'source'
PYTHON_API_PATH = WORKSPACE_PATH / 'PythonAPI' / 'carla'
EXAMPLES_PATH = WORKSPACE_PATH / 'Examples'
UTIL_PATH = WORKSPACE_PATH / 'Util'
DOCKER_UTILS_PATH = UTIL_PATH / 'DockerUtils'
PATCHES_PATH = UTIL_PATH / 'Patches'
CARLA_UE_PATH = WORKSPACE_PATH / 'Unreal' / 'CarlaUE4'
CARLA_UE_PLUGIN_ROOT_PATH = CARLA_UE_PATH / 'Plugins'
CARLA_UE_PLUGIN_PATH = CARLA_UE_PLUGIN_ROOT_PATH / 'Carla'
CARLA_UE_PLUGIN_DEPENDENCIES_PATH = CARLA_UE_PLUGIN_ROOT_PATH / 'CarlaDependencies'
CARLA_UE_CONTENT_PATH = CARLA_UE_PATH / 'Content'
CARLA_UE_CONTENT_CARLA_PATH = CARLA_UE_CONTENT_PATH / 'Carla'
CARLA_UE_CONTENT_VERSIONS_FILE_PATH = WORKSPACE_PATH / 'Util' / 'ContentVersions.json'
FALLBACK_CARLA_VERSION_STRING = '0.9.14'
LOGICAL_PROCESSOR_COUNT = psutil.cpu_count(logical = True)
DEFAULT_PARALLELISM = LOGICAL_PROCESSOR_COUNT + (2 if LOGICAL_PROCESSOR_COUNT >= 8 else 0)
READTHEDOCS_URL_SUFFIX = 'how_to_build_on_windows/\n' if os.name == "nt" else 'build_linux/\n'
DEFAULT_ERROR_MESSAGE = (
	'\n'
	'Ok, an error ocurred, don\'t panic!\n'
	'We have different platforms where you can find some help:\n'
	'\n'
	'- Make sure you have read the documentation:\n'
	f'    https://carla.readthedocs.io/en/latest/{READTHEDOCS_URL_SUFFIX}'
	'\n'
	'- If the problem persists, submit an issue on our GitHub page:\n'
	'    https://github.com/carla-simulator/carla/issues\n'
	'\n'
	'- Or just use our Discord server!\n'
	'    We\'ll be glad to help you there:\n'
	'    https://discord.gg/42KJdRj\n'
)

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

DEFAULT_C_COMPILER = FindExecutable([
	'cl',
	'clang-cl',
] if os.name == 'nt' else [
	'clang',
	'gcc',
])

DEFAULT_CPP_COMPILER = FindExecutable([
	'cl',
	'clang-cl',
] if os.name == 'nt' else [
	'clang++',
	'g++',
])

DEFAULT_LINKER = FindExecutable([
	'link',
	'llvm-link',
] if os.name == 'nt' else [
	'lld',
	'ld',
])

DEFAULT_LIB = FindExecutable([
	'lib',
	'llvm-lib',
	'llvm-ar'
] if os.name == 'nt' else [
	'llvm-ar',
	'ar',
])

argp = argparse.ArgumentParser(description = __doc__)
def AddDirective(name : str, help : str):
	argp.add_argument(f'--{name}', action = 'store_true', help = help)
def AddStringOption(name : str, default : str, help : str):
	argp.add_argument(f'--{name}', type = str, default = str(default), help = f'{help} (default = "{default}").')
def AddIntOption(name : str, default : int, help : str):
	argp.add_argument(f'--{name}', type = int, default = int(default), help = f'{help} (default = {default}).')
AddDirective(
	'launch',
	'Build and open the CarlaUE4 project in the Unreal Engine editor.')
AddDirective(
	'launch-only',
	'Open the CARLA project in the Unreal Engine editor, skipping all build steps.')
AddDirective(
	'import',
	f'Import maps and assets from "{WORKSPACE_PATH / "Import"}" into Unreal.')
AddDirective(
	'package',
	'Build a packaged version of CARLA ready for distribution.')
AddDirective(
	'docs',
	'Build the CARLA documentation, through Doxygen.')
AddDirective(
	'clean',
	'Delete all build files.')
AddDirective(
	'rebuild',
	'Delete all build files and recompiles.')
AddDirective(
	'check-libcarla',
	'Run unit the test suites for LibCarla')
AddDirective(
	'check-python-api',
	'Run unit the test suites for PythonAPI')
AddDirective(
	'check',
	'Run unit the test suites for LibCarla and PythonAPI')
AddDirective(
	'carla-ue',
	'Build the CARLA Unreal Engine plugin and project.')
AddDirective(
	'python-api',
	'Build the CARLA Python API.')
AddDirective(
	'libcarla-client',
	'Build the LibCarla Client module.')
AddDirective(
	'libcarla-server',
	'Build the LibCarla Server module.')
AddDirective(
	'update-deps',
	'Download all project dependencies.')
AddDirective(
	'build-deps',
	'Build all project dependencies.')
AddDirective(
	'configure-sequential',
	'Whether to disable parallelism in the configuration script.')
AddDirective(
	'no-log',
	'Whether to disable saving logs.')
AddDirective(
	'pytorch',
	'Whether to enable PyTorch.')
AddDirective(
	'chrono',
	'Whether to enable Chrono.')
AddDirective(
	'osm2odr',
	'Whether to enable OSM2ODR.')
AddDirective(
	'osm-world-renderer',
	'Whether to enable OSM World Renderer.')
AddDirective(
	'nv-omniverse',
	'Whether to enable the NVIDIA Omniverse Plugin.')
AddDirective(
	'march-native',
	'Whether to add "-march=native" to C/C++ compile flags.')
AddDirective(
	'rss',
	'Whether to enable RSS.')
AddIntOption(
	'parallelism',
	DEFAULT_PARALLELISM,
	'Set the configure/build parallelism.')
AddIntOption(
	'c-standard',
	11,
	'Set the target C standard.')
AddIntOption(
	'cpp-standard',
	14,
	'Set the target C++ standard.')
AddStringOption(
	'c-compiler',
	DEFAULT_C_COMPILER,
	'Set the target C compiler.')
AddStringOption(
	'cpp-compiler',
	DEFAULT_CPP_COMPILER,
	'Set the target C++ compiler.')
AddStringOption(
	'linker',
	DEFAULT_LINKER,
	'Set the target linker.')
AddStringOption(
	'ar',
	DEFAULT_LIB,
	'Set the target ar/lib tool.')
AddStringOption(
	'version',
	FALLBACK_CARLA_VERSION_STRING,
	'Override the CARLA version.')
AddStringOption(
	'generator',
	'Ninja',
	'Set the CMake generator.')
AddStringOption(
	'build-path',
	WORKSPACE_PATH / 'Build',
	'Set the CARLA build path.')
AddStringOption(
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
ENABLE_CHRONO = ARGV.chrono
ENABLE_OSM2ODR = ARGV.osm2odr or ARGV.python_api
ENABLE_OSM_WORLD_RENDERER = ARGV.osm_world_renderer
ENABLE_LIBCARLA = ARGV.libcarla_client or ARGV.libcarla_server or ARGV.python_api
ENABLE_PYTHON_API = ARGV.python_api
ENABLE_NVIDIA_OMNIVERSE = ARGV.nv_omniverse
UPDATE_DEPENDENCIES = ARGV.update_deps
BUILD_DEPENDENCIES = ARGV.build_deps
PARALLELISM = ARGV.parallelism
# Root paths:
CARLA_VERSION_STRING = ARGV.version
BUILD_PATH = Path(ARGV.build_path)
LOG_PATH = BUILD_PATH / 'BuildLogs'
DEPENDENCIES_PATH = BUILD_PATH / 'Dependencies'
LIBCARLA_BUILD_PATH = BUILD_PATH / 'libcarla-build'
LIBCARLA_INSTALL_PATH = BUILD_PATH / 'libcarla-install'
# Language options
C_COMPILER = ARGV.c_compiler
CPP_COMPILER = ARGV.cpp_compiler
LINKER = ARGV.linker
LIB = ARGV.ar
C_STANDARD = ARGV.c_standard
CPP_STANDARD = ARGV.cpp_standard
C_COMPILER_CLI_TYPE = 'msvc' if 'cl' in C_COMPILER else 'gnu'
CPP_COMPILER_CLI_TYPE = 'msvc' if 'cl' in CPP_COMPILER else 'gnu'
C_COMPILER_IS_CLANG = 'clang' in C_COMPILER
CPP_COMPILER_IS_CLANG = 'clang' in CPP_COMPILER
C_ENABLE_MARCH_NATIVE = ARGV.march_native and C_COMPILER_CLI_TYPE == 'gnu'
CPP_ENABLE_MARCH_NATIVE = ARGV.march_native and CPP_COMPILER_CLI_TYPE == 'gnu'
LIB_IS_AR = 'ar' in LIB
# Unreal Engine
UNREAL_ENGINE_PATH = Path(ARGV.ue_path)
# Dependencies:
# Boost
BOOST_TOOLSET = 'msvc-14.2'
BOOST_SOURCE_PATH = DEPENDENCIES_PATH / 'boost-source'
BOOST_BUILD_PATH = DEPENDENCIES_PATH / 'boost-build'
BOOST_INSTALL_PATH = DEPENDENCIES_PATH / 'boost-install'
BOOST_INCLUDE_PATH = BOOST_INSTALL_PATH / 'include'
BOOST_LIBRARY_PATH = BOOST_INSTALL_PATH / 'lib'
BOOST_B2_PATH = BOOST_SOURCE_PATH / f'b2{EXE_EXT}'
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
ZLIB_LIB_PATH = ZLIB_LIBRARY_PATH / f'zlib{LIB_EXT}'
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
SQLITE_LIB_PATH = SQLITE_BUILD_PATH / f'sqlite{LIB_EXT}'
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
ENABLE_RSS = ARGV.rss

# Basic IO functions:

def Log(message):
	message = str(message)
	message += '\n'
	print(message, end='')

def LaunchSubprocess(
		cmd : list,
		display_output : bool = False,
		working_directory : Path = None):
	stdstream_value = None if display_output or SEQUENTIAL else subprocess.PIPE
	return subprocess.run(
		cmd,
		stdout = stdstream_value,
		stderr = stdstream_value,
		cwd = working_directory)

def LaunchSubprocessImmediate(
		cmd : list,
		display_output : bool = False,
		working_directory : Path = None,
		log_name : str = None):
	sp = LaunchSubprocess(cmd, display_output, working_directory)
	log_content = None
	try:
		sp.check_returncode()
	except:
		stdout = sp.stdout.decode() if sp.stdout else ''
		stderr = sp.stderr.decode() if sp.stderr else ''
		log_content = (
			f' stdout:\n'
			f' {stdout}\n'
			f' stderr:\n'
			f' {stderr}\n'
		)
		error_message = (
			f'Failed to run task {cmd}.\n' +
			log_content
		)
		print(error_message)
	finally:
		if not ARGV.no_log and log_name != None:
			if log_content == None:
				stdout = sp.stdout.decode() if sp.stdout else ''
				stderr = sp.stderr.decode() if sp.stderr else ''
				log_content = (
					f' stdout:\n'
					f' {stdout}\n'
					f' stderr:\n'
					f' {stderr}\n'
				)
			with open(LOG_PATH / (log_name + '.log'), 'w') as file:
				file.write(log_content)

# Convenience classes for listing dependencies:



class Download:

	def __init__(self, url : str):
		self.url = url



class GitRepository:

	def __init__(self, url : str, tag : str = None):
		self.url = url
		self.tag = tag



class Dependency:

	def __init__(self, name : str, *sources):
		self.name = name
		self.sources = [ *sources ]
		assert all(
			type(e) is Download or
			type(e) is GitRepository
			for e in self.sources)



class Task:

	def __init__(self, name : str, in_edge_names : list, body, *args):
		self.name = name
		self.body = body
		self.args = args
		self.in_edge_done_count = 0
		self.in_edge_names = in_edge_names
		self.in_edges = []
		self.out_edges = [] # Filled right before task graph execution.
		self.done = False

	def CreateSubprocessTask(name : str, in_edges : list, command : list):
		return Task(name, in_edges, LaunchSubprocessImmediate, command, False, None, name)
	
	def CreateCMakeConfigureDefaultCommandLine(source_path : Path, build_path : Path):
		cpp_flags_release = '/MD'
		if CPP_ENABLE_MARCH_NATIVE:
			cpp_flags_release += ' -march=native'
		return [
			'cmake',
			'-G', ARGV.generator,
			'-S', source_path,
			'-B', build_path,
			'-DCMAKE_C_COMPILER=' + C_COMPILER,
			'-DCMAKE_CXX_COMPILER=' + CPP_COMPILER,
			'-DCMAKE_BUILD_TYPE=Release',
			f'-DCMAKE_CXX_FLAGS_RELEASE={cpp_flags_release}',
		]

	def CreateCMakeConfigureDefault(name : str, in_edges : list, source_path : Path, build_path : Path, *args, install_path : Path = None):
		cmd = Task.CreateCMakeConfigureDefaultCommandLine(source_path, build_path)
		if install_path != None:
			cmd.append('-DCMAKE_INSTALL_PREFIX=' + str(install_path))
		cmd.extend([ *args ])
		cmd.append(source_path)
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
		if len(task.in_edge_names) == 0:
			self.sources.append(task.name)
		self.task_map[task.name] = self.tasks[-1]
		return task
	
	def Validate(self):
		return True

	def Execute(self, sequential : bool = False):
		if len(self.tasks) == 0:
			return
		from collections import deque
		assert self.Validate()
		prior_sequential = self.sequential
		self.sequential = sequential
		try:
			for task in self.tasks:
				for in_edge_name in task.in_edge_names:
					in_edge = self.task_map.get(in_edge_name, None)
					assert in_edge != None
					task.in_edges.append(in_edge)

			for task in self.tasks:
				for in_edge_name in task.in_edge_names:
					in_edge = self.task_map.get(in_edge_name, None)
					assert in_edge != None
					in_edge.out_edges.append(task)

			futures = []
			future_map = {}
			task_queue = deque()
			active_count = 0
			done_count = 0

			def UpdateOutEdges(task):
				nonlocal task_queue
				assert len(task.in_edges) == len(task.in_edge_names)
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
						Log(f'{e.name} - done')
						UpdateOutEdges(e)
					assert active_count == len(done_tasks)
					done_count += len(done_tasks)
					active_count = 0
					future_map = {}
					futures = []

			task_queue.extend([ self.task_map[e] for e in self.sources ])
			with ProcessPoolExecutor(self.parallelism) as pool:
				while len(task_queue) != 0:
					while len(task_queue) != 0 and active_count < self.parallelism:
						task = task_queue.popleft()
						Log(f'{task.name} - start')
						if not self.sequential:
							active_count += 1
							future = pool.submit(task.Run)
							future_map[future] = task
							futures.append(future)
						else:
							task.Run()
							Log(f'{task.name} - done')
							task.done = True
							done_count += 1
							UpdateOutEdges(task)
					Flush()
				Flush()

			if done_count != len(self.tasks):
				pending_tasks = []
				for e in self.tasks:
					if not e.done:
						pending_tasks.append(e)
				Log(f'{len(self.tasks) - done_count} did not complete: {pending_tasks}.')
				assert False
					
		finally:
			self.sequential = prior_sequential
			self.Reset()



# Constants:

DEFAULT_DEPENDENCIES = [

	Dependency(
		'boost',
		Download('https://boostorg.jfrog.io/artifactory/main/release/1.83.0/source/boost_1_83_0.zip'),
		Download('https://carla-releases.s3.eu-west-3.amazonaws.com/Backup/boost_1_83_0.zip')),

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
		GitRepository('https://github.com/madler/zlib.git'),
		Download('https://zlib.net/current/zlib.tar.gz')),

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

CHRONO_DEPENDENCIES = [

	Dependency(
		'chrono',
		GitRepository('https://github.com/projectchrono/chrono.git', tag = '8.0.0')),

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
		GitRepository('https://github.com/carla-simulator/sumo.git', tag = 'carla_osm2odr')),

]

DEPENDENCY_MAP = {
	**{ e.name : e for e in DEFAULT_DEPENDENCIES },
	**{ e.name : e for e in CHRONO_DEPENDENCIES },
	**{ e.name : e for e in OSM_WORLD_RENDERER_DEPENDENCIES },
	**{ e.name : e for e in OSM2ODR_DEPENDENCIES }
}

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

def BuildLibCarlaMain(task_graph : TaskGraph):
	task_graph.Add(Task.CreateCMakeConfigureDefault(
		'configure-libcarla',
		[],
		WORKSPACE_PATH,
		LIBCARLA_BUILD_PATH,
		f'-DCARLA_DEPENDENCIES_PATH={DEPENDENCIES_PATH}',
		f'-DBUILD_LIBCARLA_SERVER={"ON" if ARGV.libcarla_server else "OFF"}',
		f'-DBUILD_LIBCARLA_CLIENT={"ON" if ARGV.libcarla_client else "OFF"}',
		f'-DBUILD_OSM_WORLD_RENDERER={"ON" if ENABLE_OSM_WORLD_RENDERER else "OFF"}',
		f'-DLIBCARLA_PYTORCH={"ON" if ARGV.pytorch else "OFF"}'))
	task_graph.Add(Task.CreateCMakeBuildDefault(
		'build-libcarla',
		[ 'configure-libcarla' ],
		LIBCARLA_BUILD_PATH))
	task_graph.Add(Task.CreateCMakeInstallDefault(
		'install-libcarla',
		[ 'build-libcarla' ],
		LIBCARLA_BUILD_PATH,
		LIBCARLA_INSTALL_PATH))

def BuildCarlaUEMain():
	assert UNREAL_ENGINE_PATH.exists()
	if os.name == 'nt':
		LaunchSubprocessImmediate([
			UNREAL_ENGINE_PATH / 'Engine' / 'Build' / 'BatchFiles' / 'Build.bat',
			'CarlaUE4', 'Win64', 'Development', '-WaitMutex', '-FromMsBuild',
			CARLA_UE_PATH / 'CarlaUE4.uproject'
		])
	else:
		pass

def BuildCarlaUE(task_graph : TaskGraph):
	Log('Building Carla Unreal Engine Editor')
	if ENABLE_NVIDIA_OMNIVERSE:
		task_graph.Add(Task('install-nv-omniverse', [], InstallNVIDIAOmniverse))
	task_graph.Add(Task('build-carla-ue', [], BuildCarlaUEMain))

def BuildPythonAPIMain():
	content = ''
	with open(PYTHON_API_PATH / 'setup.py.in', 'r') as file:
		content = file.read()
	content = content.format_map(globals())
	if os.name == 'nt':
		content = content.replace(os.sep, '\\\\')
	with open(PYTHON_API_PATH / 'setup.py', 'w') as file:
		file.write(content)
	LaunchSubprocessImmediate([
		sys.executable, 'setup.py', 'bdist_wheel', 'bdist_egg'
	], working_directory = PYTHON_API_PATH)

def BuildPythonAPI(task_graph : TaskGraph):
	task_graph.Add(Task('build-python-api', [ 'install-libcarla' ], BuildPythonAPIMain))

def SetupUnrealEngine(task_graph : TaskGraph):
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

def UpdateDependencies(task_graph : TaskGraph):
	DEPENDENCIES_PATH.mkdir(exist_ok = True)
	unique_deps = set(DEFAULT_DEPENDENCIES)
	if ENABLE_OSM_WORLD_RENDERER:
		unique_deps.update(OSM_WORLD_RENDERER_DEPENDENCIES)
	if ENABLE_OSM2ODR:
		unique_deps.update(OSM2ODR_DEPENDENCIES)
	if ENABLE_CHRONO:
		unique_deps.update(CHRONO_DEPENDENCIES)
	for dep in unique_deps:
		name = dep.name
		task_graph.Add(Task(f'update-{name}', [], UpdateDependency, dep))
	task_graph.Execute()

def CleanDownloadsMain():
	for ext in [ '*.tmp', '*.zip', '*.tar.gz' ]:
		for e in DEPENDENCIES_PATH.glob(ext):
			e.unlink(missing_ok = True)

def CleanDownloads(task_graph : TaskGraph):
	task_graph.Add(Task('clean-downloads', [], CleanDownloadsMain))

def ConfigureBoost():
	if BOOST_B2_PATH.exists():
		return
	LaunchSubprocessImmediate(
		[ BOOST_SOURCE_PATH / f'bootstrap{SHELL_EXT}' ],
		working_directory = BOOST_SOURCE_PATH)

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
	sqlite_sources = [ Path(e) for e in sqlite_sources ]
	if not SQLITE_EXE_PATH.exists():
		cmd = [ C_COMPILER ]
		cmd.extend([
			f'-std=c{C_STANDARD}',
			f'-fuse-ld={LINKER}',
			'-O2',
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
			cmd.append('-o')
			cmd.append(SQLITE_EXE_PATH)
		LaunchSubprocessImmediate(cmd, log_name = 'build-sqlite-exe')
	if not SQLITE_LIB_PATH.exists():
		if C_COMPILER_IS_CLANG:
			cmd = [
				C_COMPILER,
				f'-fuse-ld={LIB}',
			]
			cmd.extend([
				f'-std=c{C_STANDARD}',
				'-march=native',
				'-O2',
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
			cmd.append('/Fo:' if C_COMPILER_CLI_TYPE == 'msvc' else '-o')
			cmd.append(SQLITE_LIB_PATH)
			LaunchSubprocessImmediate(
				cmd,
				log_name = 'build-sqlite-lib')
		else:
			objs = []
			for e in sqlite_sources:
				cmd = [
					C_COMPILER,
					'-c' if C_COMPILER_CLI_TYPE == 'gnu' else '/c',
				]
				cmd.extend([
					f'-std=c{C_STANDARD}',
					'-march=native',
					'-O2',
				] if C_COMPILER_CLI_TYPE == 'gnu' else [
					f'/std:c{C_STANDARD}',
					'/O2',
					'/Qvec',
					'/MD',
					'/EHsc',
				])
				cmd.append(e)
				obj_path = SQLITE_BUILD_PATH / f'{e.name}{OBJ_EXT}'
				cmd.append('/Fo:' if C_COMPILER_CLI_TYPE == 'msvc' else '-o')
				cmd.append(obj_path)
				LaunchSubprocessImmediate(cmd, log_name = f'build-sqlite-{e.stem}')
				objs.append(obj_path)
			cmd = [
				LIB,
				f'/OUT:{SQLITE_LIB_PATH}',
			]
			cmd.extend(objs)
			LaunchSubprocessImmediate(cmd, log_name = 'build-sqlite-lib')

def FindXercesC():
	return glob.glob(f'{XERCESC_INSTALL_PATH}/**/xerces-c*{LIB_EXT}', recursive=True)[0]

def ConfigureSUMO():
	cmd = Task.CreateCMakeConfigureDefaultCommandLine(
		SUMO_SOURCE_PATH,
		SUMO_BUILD_PATH)
	cmd.extend([
		SUMO_SOURCE_PATH,
		SUMO_BUILD_PATH,
		f'-DZLIB_INCLUDE_DIR={ZLIB_INCLUDE_PATH}',
		f'-DZLIB_LIBRARY={ZLIB_LIBRARY_PATH}',
        f'-DPROJ_INCLUDE_DIR={PROJ_INSTALL_PATH}/include',
        f'-DPROJ_LIBRARY={PROJ_INSTALL_PATH}/lib/proj{LIB_EXT}',
        f'-DXercesC_INCLUDE_DIR={XERCESC_INSTALL_PATH}/include',
        f'-DXercesC_LIBRARY={FindXercesC()}',
		'-DSUMO_LIBRARIES=OFF',
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
	LaunchSubprocessImmediate(cmd)

def BuildDependencies(task_graph : TaskGraph):
	# Configure:
	task_graph.Add(Task('build-sqlite', [], BuildSQLite))
	task_graph.Add(Task('configure-boost', [], ConfigureBoost))
	task_graph.Add(Task.CreateCMakeConfigureDefault(
		'configure-zlib',
		[],
		ZLIB_SOURCE_PATH,
		ZLIB_BUILD_PATH,
		install_path = ZLIB_INSTALL_PATH))
	task_graph.Add(Task.CreateCMakeBuildDefault(
		'build-zlib',
		[ 'configure-zlib' ],
		ZLIB_BUILD_PATH))
	task_graph.Add(Task.CreateCMakeInstallDefault(
		'install-zlib',
		[ 'build-zlib' ],
		ZLIB_BUILD_PATH,
		ZLIB_INSTALL_PATH))
	task_graph.Add(Task.CreateCMakeConfigureDefault(
		'configure-gtest',
		[],
		GTEST_SOURCE_PATH,
		GTEST_BUILD_PATH))
	task_graph.Add(Task.CreateCMakeConfigureDefault(
		'configure-libpng',
		[],
		LIBPNG_SOURCE_PATH,
		LIBPNG_BUILD_PATH,
		'-DPNG_TESTS=OFF',
		'-DPNG_TOOLS=OFF',
		'-DPNG_BUILD_ZLIB=ON',
		f'-DZLIB_INCLUDE_DIRS={ZLIB_INCLUDE_PATH}',
		f'-DZLIB_LIBRARIES={ZLIB_LIB_PATH}'))
	task_graph.Add(Task.CreateCMakeConfigureDefault(
		'configure-proj',
		[ 'build-sqlite' ],
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
		'-DBUILD_TESTING=OFF'))
	task_graph.Add(Task.CreateCMakeConfigureDefault(
		'configure-recast',
		[],
		RECAST_SOURCE_PATH,
		RECAST_BUILD_PATH,
		'-DRECASTNAVIGATION_DEMO=OFF',
		'-DRECASTNAVIGATION_TESTS=OFF',
		'-DRECASTNAVIGATION_EXAMPLES=OFF'))
	task_graph.Add(Task.CreateCMakeConfigureDefault(
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
	task_graph.Add(Task.CreateCMakeConfigureDefault(
		'configure-xercesc',
		[],
		XERCESC_SOURCE_PATH,
		XERCESC_BUILD_PATH))
	if ENABLE_OSM_WORLD_RENDERER:
		task_graph.Add(Task.CreateCMakeConfigureDefault(
			'configure-libosmscout',
			[],
			LIBOSMSCOUT_SOURCE_PATH,
			LIBOSMSCOUT_BUILD_PATH,
			'-DOSMSCOUT_BUILD_TOOL_STYLEEDITOR=OFF',
			'-DOSMSCOUT_BUILD_TOOL_OSMSCOUT2=OFF',
			'-DOSMSCOUT_BUILD_TESTS=OFF',
			'-DOSMSCOUT_BUILD_CLIENT_QT=OFF',
			'-DOSMSCOUT_BUILD_DEMOS=OFF'))
		task_graph.Add(Task.CreateCMakeConfigureDefault(
			'configure-lunasvg',
			[],
			LUNASVG_SOURCE_PATH,
			LUNASVG_BUILD_PATH))
	if ENABLE_CHRONO:
		task_graph.Add(Task.CreateCMakeConfigureDefault(
			'configure-chrono',
			[],
			CHRONO_SOURCE_PATH,
			CHRONO_BUILD_PATH,
			f'-DEIGEN3_INCLUDE_DIR={EIGEN_SOURCE_PATH}',
			'-DENABLE_MODULE_VEHICLE=ON'))
	task_graph.Execute()
	# Build:
	task_graph.Add(Task('build-boost', [], BuildAndInstallBoost))
	task_graph.Add(Task.CreateCMakeBuildDefault('build-gtest', [], GTEST_BUILD_PATH))
	task_graph.Add(Task.CreateCMakeBuildDefault('build-libpng', [], LIBPNG_BUILD_PATH))
	task_graph.Add(Task.CreateCMakeBuildDefault('build-proj', [], PROJ_BUILD_PATH))
	task_graph.Add(Task.CreateCMakeBuildDefault('build-recast', [], RECAST_BUILD_PATH))
	task_graph.Add(Task.CreateCMakeBuildDefault('build-rpclib', [], RPCLIB_BUILD_PATH))
	task_graph.Add(Task.CreateCMakeBuildDefault('build-xercesc', [], XERCESC_BUILD_PATH))
	if ENABLE_OSM_WORLD_RENDERER:
		task_graph.Add(Task.CreateCMakeBuildDefault('build-lunasvg', [], LUNASVG_BUILD_PATH))
		task_graph.Add(Task.CreateCMakeBuildDefault('build-libosmscout', [], LIBOSMSCOUT_BUILD_PATH))
	if ENABLE_OSM2ODR:
		task_graph.Add(Task('configure-sumo', [ 'build-xercesc' ], ConfigureSUMO))
		task_graph.Add(Task.CreateCMakeBuildDefault('build-sumo', [ 'configure-sumo' ], SUMO_BUILD_PATH))
	if ENABLE_CHRONO:
		task_graph.Add(Task.CreateCMakeBuildDefault('build-chrono', [], CHRONO_BUILD_PATH))
	task_graph.Execute(sequential = True) # The underlying build system should already parallelize.
	# Install:
	task_graph.Add(Task.CreateCMakeInstallDefault('install-gtest', [], GTEST_BUILD_PATH, GTEST_INSTALL_PATH))
	task_graph.Add(Task.CreateCMakeInstallDefault('install-libpng', [], LIBPNG_BUILD_PATH, LIBPNG_INSTALL_PATH))
	task_graph.Add(Task.CreateCMakeInstallDefault('install-proj', [], PROJ_BUILD_PATH, PROJ_INSTALL_PATH))
	task_graph.Add(Task.CreateCMakeInstallDefault('install-recast', [], RECAST_BUILD_PATH, RECAST_INSTALL_PATH))
	task_graph.Add(Task.CreateCMakeInstallDefault('install-rpclib', [], RPCLIB_BUILD_PATH, RPCLIB_INSTALL_PATH))
	task_graph.Add(Task.CreateCMakeInstallDefault('install-xercesc', [], XERCESC_BUILD_PATH, XERCESC_INSTALL_PATH))
	if ENABLE_OSM_WORLD_RENDERER:
		task_graph.Add(Task.CreateCMakeInstallDefault('install-lunasvg', [], LUNASVG_BUILD_PATH, LUNASVG_INSTALL_PATH))
		task_graph.Add(Task.CreateCMakeInstallDefault('install-libosmscout', [], LIBOSMSCOUT_BUILD_PATH, LIBOSMSCOUT_INSTALL_PATH))
	if ENABLE_OSM2ODR:
		task_graph.Add(Task.CreateCMakeInstallDefault('install-sumo', [], SUMO_BUILD_PATH, SUMO_INSTALL_PATH))
	if ENABLE_CHRONO:
		task_graph.Add(Task.CreateCMakeInstallDefault('install-chrono', [], CHRONO_BUILD_PATH, CHRONO_INSTALL_PATH))

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
		if BUILD_DEPENDENCIES:
			BuildDependencies(task_graph)
		if ENABLE_LIBCARLA:
			BuildLibCarlaMain(task_graph)
		if ENABLE_PYTHON_API:
			BuildPythonAPI(task_graph)
		if ARGV.carla_ue:
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