# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Actual batch regressions with guarded native tool recorders, no real builds."""

import base64
import os
from pathlib import Path
import shutil
import stat
import subprocess
import tempfile
import unittest
import uuid


ROOT = Path(__file__).resolve().parents[3]
RECORDER = r'''
using System;
using System.IO;
using System.Text;
class Recorder {
    static int Main(string[] args) {
        string root = Environment.GetEnvironmentVariable("CARLA_PYBUILD_TEST_ROOT");
        string token = Environment.GetEnvironmentVariable("CARLA_PYBUILD_TEST_TOKEN");
        if (String.IsNullOrEmpty(root) || String.IsNullOrEmpty(token) ||
            !File.Exists(Path.Combine(root, ".fixture-marker")) ||
            File.ReadAllText(Path.Combine(root, ".fixture-marker")) != token) return 96;
        string cwd = Path.GetFullPath(Environment.CurrentDirectory);
        if (!cwd.StartsWith(Path.GetFullPath(root) + Path.DirectorySeparatorChar,
                            StringComparison.OrdinalIgnoreCase)) return 95;
        string tool = Path.GetFileNameWithoutExtension(System.Reflection.Assembly.GetExecutingAssembly().Location);
        File.AppendAllText(Path.Combine(root, "calls.log"), tool + "\t" +
            Convert.ToBase64String(Encoding.UTF8.GetBytes(String.Join("\0", args))) + "\n");
        if (args.Length == 2 && args[0] == "--carla-recorder-self-check" && args[1] == token) {
            File.WriteAllText(Path.Combine(root, ".recorder-ready"), token);
            return 0;
        }
        if (tool == "cmake" && Environment.GetEnvironmentVariable("CARLA_PYBUILD_TEST_CMAKE") == "1") return 0;
        if (tool != "python") return 97;
        if (args.Length >= 2 && args[0] == "-m" && args[1] == "build") {
            int code = Int32.Parse(Environment.GetEnvironmentVariable("CARLA_PYBUILD_TEST_BUILD_EXIT") ?? "0");
            if (code != 0) return code;
            if (Environment.GetEnvironmentVariable("CARLA_PYBUILD_TEST_NO_WHEEL") != "1") {
                Directory.CreateDirectory("dist/.tmp");
                File.WriteAllText("dist/.tmp/carla_fixture-py3-none-any.whl", "inert wheel marker");
            }
            return 0;
        }
        if (args.Length >= 2 && args[0] == "-m" && args[1] == "pip")
            return Int32.Parse(Environment.GetEnvironmentVariable("CARLA_PYBUILD_TEST_INSTALL_EXIT") ?? "0");
        return 97;
    }
}
'''


def validate_fixture(directory, parent):
    parent = Path(parent).resolve()
    resolved = Path(directory).resolve()
    if resolved.parent != parent or not resolved.name.startswith('carla-pybuild-'):
        raise RuntimeError('unsafe Python build fixture root')
    pending = [Path(directory)]
    while pending:
        item = pending.pop()
        info = item.lstat()
        if item.is_symlink() or getattr(info, 'st_file_attributes', 0) & stat.FILE_ATTRIBUTE_REPARSE_POINT:
            raise RuntimeError('reparse points are forbidden in build fixtures')
        if os.path.commonpath([str(resolved), str(item.resolve())]) != str(resolved):
            raise RuntimeError('build fixture escaped its root')
        if item.is_dir():
            pending.extend(item.iterdir())


def remove_fixture(directory, parent):
    validate_fixture(directory, parent)
    shutil.rmtree(directory)


@unittest.skipUnless(os.name == 'nt', 'requires Windows cmd.exe')
class WindowsPythonBuildErrors(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        compiler = Path(os.environ['WINDIR']) / 'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
        if not compiler.is_file():
            raise unittest.SkipTest('requires the installed .NET Framework C# compiler')
        cls.parent = Path(os.environ.get('CARLA_PYBUILD_TEST_TMP', tempfile.gettempdir())).resolve()
        cls.parent.mkdir(parents=True, exist_ok=True)
        cls.tools = Path(tempfile.mkdtemp(prefix='carla-pybuild-tools-', dir=cls.parent))
        cls.addClassCleanup(remove_fixture, cls.tools, cls.parent)
        source = cls.tools / 'Recorder.cs'
        source.write_text(RECORDER, encoding='utf-8')
        executable = cls.tools / 'recorder.exe'
        subprocess.run([str(compiler), '/nologo', '/out:' + str(executable), str(source)],
                       capture_output=True, text=True, check=True, timeout=30)
        for tool in ('python', 'git', 'cmake', 'curl', 'wget', 'powershell', 'bitsadmin',
                     'pip', 'ninja', 'make', 'msbuild'):
            shutil.copyfile(executable, cls.tools / (tool + '.exe'))

    def setUp(self):
        self.directory = Path(tempfile.mkdtemp(prefix='carla-pybuild-case-', dir=self.parent))
        self.addCleanup(remove_fixture, self.directory, self.parent)
        self.client = self.directory / 'PythonAPI/carla'
        self.client.mkdir(parents=True)
        (self.client / 'dist/.tmp').mkdir(parents=True)
        self.token = uuid.uuid4().hex
        (self.directory / '.fixture-marker').write_text(self.token, encoding='utf-8')
        self.environment = os.environ.copy()
        self.environment.update({
            'ROOT_PATH': str(self.directory) + os.sep,
            'CARLA_PYBUILD_TEST_ROOT': str(self.directory),
            'CARLA_PYBUILD_TEST_TOKEN': self.token,
            'CARLA_PYBUILD_TEST_BUILD_EXIT': '0',
            'CARLA_PYBUILD_TEST_INSTALL_EXIT': '0',
            'CARLA_PYBUILD_TEST_NO_WHEEL': '0',
            'CARLA_PYBUILD_TEST_CMAKE': '0',
            'COPYCMD': '/Y',
        })
        self.environment.pop('GENERATOR', None)
        self.environment.pop('BUILD_DIR', None)
        self.wheel = self.client / 'dist/carla_fixture-py3-none-any.whl'
        self.temporary_wheel = self.client / 'dist/.tmp/carla_fixture-py3-none-any.whl'

    def run_batch(self, relative_source, arguments='', tools=None):
        validate_fixture(self.directory, self.parent)
        validate_fixture(self.tools, self.parent)
        if Path(self.environment['ROOT_PATH']).resolve() != self.directory.resolve():
            raise RuntimeError('ROOT_PATH must be fixture-owned')
        script = self.directory / Path(relative_source).name
        shutil.copyfile(ROOT / relative_source, script)
        tools = tools or self.tools
        wrapper = self.directory / 'run.cmd'
        wrapper.write_text(
            '@echo off\nsetlocal\n'
            + 'set "PATH=' + str(tools) + ';' + str(Path(os.environ['WINDIR']) / 'System32') + '"\n'
            + 'set "PATHEXT=.EXE"\n'
            + 'where python > "' + str(self.directory / 'where.txt') + '" 2>nul\n'
            + 'if not "%errorlevel%"=="0" exit /b 98\n'
            + 'set /p FOUND=<"' + str(self.directory / 'where.txt') + '"\n'
            + 'if /i not "%FOUND%"=="' + str(tools / 'python.exe') + '" exit /b 98\n'
            + 'python --carla-recorder-self-check ' + self.token + '\n'
            + 'if not "%errorlevel%"=="0" exit /b 98\n'
            + 'if not exist "' + str(self.directory / '.recorder-ready') + '" exit /b 98\n'
            + 'call "' + str(script) + '" ' + arguments + '\nexit /b %errorlevel%\n', encoding='utf-8')
        return subprocess.run([os.environ['COMSPEC'], '/d', '/c', str(wrapper)],
                              cwd=self.client, env=self.environment, capture_output=True, text=True, timeout=20)

    def calls(self, tool='python'):
        log = self.directory / 'calls.log'
        if not log.exists():
            return []
        result = []
        for line in log.read_text(encoding='utf-8-sig').splitlines():
            name, encoded = line.split('\t')
            arguments = base64.b64decode(encoded).decode().split('\0')
            if name == tool and arguments[0] != '--carla-recorder-self-check':
                result.append(arguments)
        return result

    def run_python_build(self, arguments=''):
        return self.run_batch('Util/BuildTools/BuildPythonAPI.bat', arguments)

    def assert_failed(self, result, code=None):
        self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
        if code is not None:
            self.assertEqual(result.returncode, code & 0xffffffff, result.stdout + result.stderr)
        self.assertNotIn('successfully installed', result.stdout)

    def test_build_only_copies_wheel_without_installing(self):
        result = self.run_python_build('--build-wheel')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertTrue(self.wheel.is_file())
        self.assertEqual(len(self.calls()), 1)
        self.assertFalse((self.client / 'dist/.tmp').exists())

    def test_default_build_installs_and_copies_wheel(self):
        result = self.run_python_build()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertTrue(self.wheel.is_file())
        self.assertEqual([call[1] for call in self.calls()], ['build', 'pip'])

    def test_failed_build_stops_before_install(self):
        self.environment['CARLA_PYBUILD_TEST_BUILD_EXIT'] = '23'
        result = self.run_python_build()
        self.assert_failed(result, 23)
        self.assertEqual([call[1] for call in self.calls()], ['build'])
        self.assertFalse(self.wheel.exists())

    def test_failed_build_does_not_install_stale_wheel(self):
        stale = self.client / 'dist/.tmp/stale.whl'
        stale.write_text('previous attempt', encoding='utf-8')
        self.environment['CARLA_PYBUILD_TEST_BUILD_EXIT'] = '23'
        result = self.run_python_build()
        self.assert_failed(result, 23)
        self.assertEqual([call[1] for call in self.calls()], ['build'])
        self.assertEqual(stale.read_text(encoding='utf-8'), 'previous attempt')

    def test_failed_install_stops_before_copy_and_preserves_wheel(self):
        self.environment['CARLA_PYBUILD_TEST_INSTALL_EXIT'] = '31'
        result = self.run_python_build()
        self.assert_failed(result, 31)
        self.assertEqual([call[1] for call in self.calls()], ['build', 'pip'])
        self.assertFalse(self.wheel.exists())
        self.assertTrue(self.temporary_wheel.exists())

    def test_negative_native_build_exit_does_not_install_stale_wheel(self):
        stale = self.client / 'dist/.tmp/stale.whl'
        stale.write_text('previous attempt', encoding='utf-8')
        self.environment['CARLA_PYBUILD_TEST_BUILD_EXIT'] = '-1073741819'
        result = self.run_python_build()
        self.assert_failed(result, -1073741819)
        self.assertEqual([call[1] for call in self.calls()], ['build'])
        self.assertTrue(stale.exists())

    def test_negative_native_install_exit_preserves_wheel(self):
        self.environment['CARLA_PYBUILD_TEST_INSTALL_EXIT'] = '-1073741819'
        result = self.run_python_build()
        self.assert_failed(result, -1073741819)
        self.assertFalse(self.wheel.exists())
        self.assertTrue(self.temporary_wheel.exists())

    def test_missing_output_is_an_error_without_install(self):
        self.environment['CARLA_PYBUILD_TEST_NO_WHEEL'] = '1'
        result = self.run_python_build()
        self.assert_failed(result, 1)
        self.assertEqual([call[1] for call in self.calls()], ['build'])

    def test_missing_recorder_fails_closed_before_script(self):
        empty_tools = self.directory / 'empty-tools'
        empty_tools.mkdir()
        result = self.run_batch('Util/BuildTools/BuildPythonAPI.bat', tools=empty_tools)
        self.assertEqual(result.returncode, 98)
        self.assertFalse((self.directory / '.recorder-ready').exists())
        self.assertEqual(self.calls(), [])

    def test_external_root_is_rejected_before_script(self):
        self.environment['ROOT_PATH'] = str(ROOT)
        with self.assertRaisesRegex(RuntimeError, 'ROOT_PATH must be fixture-owned'):
            self.run_python_build()
        self.assertEqual(self.calls(), [])

    def rpc_args(self, build, generator='Visual Studio 17 2022'):
        return '--build-dir "' + str(build) + '\\" --generator "' + generator + '"'

    def test_existing_rpc_install_accepts_quoted_path_with_spaces(self):
        build = self.directory / 'Build with spaces'
        (build / 'rpclib-install').mkdir(parents=True)
        result = self.run_batch('Util/InstallersWin/install_rpclib.bat', self.rpc_args(build))
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(self.calls('git'), [])
        self.assertEqual(self.calls('cmake'), [])

    def test_existing_rpc_install_keeps_plain_path_fast_path(self):
        build = self.directory / 'Build'
        (build / 'rpclib-install').mkdir(parents=True)
        result = self.run_batch('Util/InstallersWin/install_rpclib.bat', self.rpc_args(build))
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(self.calls('cmake'), [])

    def test_rpc_configure_preserves_generator_and_source_arguments(self):
        build = self.directory / 'Build with spaces'
        (build / 'rpclib-src/build').mkdir(parents=True)
        self.environment['CARLA_PYBUILD_TEST_CMAKE'] = '1'
        result = self.run_batch('Util/InstallersWin/install_rpclib.bat', self.rpc_args(build))
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        calls = self.calls('cmake')
        self.assertEqual(len(calls), 2, calls)
        self.assertEqual(calls[0][calls[0].index('-G') + 1], 'Visual Studio 17 2022')
        self.assertEqual(calls[0][-1], str(build / 'rpclib-src'))
        self.assertEqual(calls[1], ['--build', '.', '--config', 'Release', '--target', 'install'])

    def test_rpc_defaults_to_visual_studio_without_generator_argument(self):
        (self.directory / 'rpclib-src/build').mkdir(parents=True)
        self.environment['CARLA_PYBUILD_TEST_CMAKE'] = '1'
        result = self.run_batch('Util/InstallersWin/install_rpclib.bat')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        calls = self.calls('cmake')
        self.assertEqual(len(calls), 2, calls)
        self.assertEqual(calls[0][calls[0].index('-G') + 1], 'Visual Studio 17 2022')

    def test_rpc_non_visual_studio_generator_omits_platform_argument(self):
        build = self.directory / 'Build'
        (build / 'rpclib-src/build').mkdir(parents=True)
        self.environment['CARLA_PYBUILD_TEST_CMAKE'] = '1'
        result = self.run_batch('Util/InstallersWin/install_rpclib.bat', self.rpc_args(build, 'Ninja'))
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        calls = self.calls('cmake')
        self.assertEqual(len(calls), 2, calls)
        self.assertEqual(calls[0][calls[0].index('-G') + 1], 'Ninja')
        self.assertNotIn('-A', calls[0])
        self.assertEqual(calls[0][-1], str(build / 'rpclib-src'))

    def test_rpc_preserves_inherited_quoted_generator(self):
        build = self.directory / 'Build'
        (build / 'rpclib-src/build').mkdir(parents=True)
        self.environment['CARLA_PYBUILD_TEST_CMAKE'] = '1'
        self.environment['GENERATOR'] = '"Visual Studio 17 2022"'
        result = self.run_batch('Util/InstallersWin/install_rpclib.bat',
                                '--build-dir "' + str(build) + '\\"')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        calls = self.calls('cmake')
        self.assertEqual(calls[0][calls[0].index('-G') + 1], 'Visual Studio 17 2022')


if __name__ == '__main__':
    unittest.main()
