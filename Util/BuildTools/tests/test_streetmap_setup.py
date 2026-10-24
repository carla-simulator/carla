# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Run the actual StreetMap setup batch file with an inert Git executable."""
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
using System.Reflection;

class Recorder {
    static int Main(string[] args) {
        string root = Environment.GetEnvironmentVariable("CARLA_STREETMAP_FIXTURE");
        string token = Environment.GetEnvironmentVariable("CARLA_STREETMAP_TOKEN");
        if (String.IsNullOrEmpty(root) || String.IsNullOrEmpty(token) ||
            !File.Exists(Path.Combine(root, ".fixture")) ||
            File.ReadAllText(Path.Combine(root, ".fixture")) != token) return 96;
        string cwd = Path.GetFullPath(Environment.CurrentDirectory);
        if (!cwd.StartsWith(Path.GetFullPath(root) + Path.DirectorySeparatorChar,
                            StringComparison.OrdinalIgnoreCase)) return 95;
        if (args.Length == 2 && args[0] == "--fixture-self-check" && args[1] == token)
            return Int32.Parse(Environment.GetEnvironmentVariable("CARLA_STREETMAP_SELF_CHECK_EXIT") ?? "0");
        string tool = Path.GetFileNameWithoutExtension(Assembly.GetExecutingAssembly().Location);
        if (tool != "git") return 97;
        File.AppendAllText(Path.Combine(root, "calls.log"),
            Convert.ToBase64String(Encoding.UTF8.GetBytes(String.Join("\0", args))) + "\n");
        string destination = Path.GetFullPath(Environment.GetEnvironmentVariable("CARLA_STREETMAP_DEST"));
        if (!destination.StartsWith(Path.GetFullPath(root) + Path.DirectorySeparatorChar,
                                    StringComparison.OrdinalIgnoreCase)) return 94;
        if (args.Length > 0 && args[0] == "clone") {
            if (args.Length != 5 || args[1] != "-b" || args[2] != "master" ||
                args[3] != "https://github.com/carla-simulator/StreetMap.git" ||
                !String.Equals(Path.GetFullPath(args[4]).TrimEnd('\\', '/'), destination,
                               StringComparison.OrdinalIgnoreCase)) return 93;
            int status = Int32.Parse(Environment.GetEnvironmentVariable("CARLA_STREETMAP_CLONE_EXIT"));
            if (status != 0) return status;
            if (Environment.GetEnvironmentVariable("CARLA_STREETMAP_SKIP_DIRECTORY") != "1")
                Directory.CreateDirectory(destination);
            return 0;
        }
        if (!String.Equals(cwd, destination, StringComparison.OrdinalIgnoreCase)) return 92;
        if (args.Length == 1 && args[0] == "fetch")
            return Int32.Parse(Environment.GetEnvironmentVariable("CARLA_STREETMAP_FETCH_EXIT"));
        if (args.Length == 2 && args[0] == "checkout" &&
            args[1] == "260273d6b7c3f28988cda31fd33441de7e272958")
            return Int32.Parse(Environment.GetEnvironmentVariable("CARLA_STREETMAP_CHECKOUT_EXIT"));
        return 91;
    }
}
'''


def validate_tree(directory, parent):
    directory, parent = Path(directory), Path(parent).resolve()
    resolved = directory.resolve()
    if resolved.parent != parent or not resolved.name.startswith('carla-streetmap-'):
        raise RuntimeError('unsafe StreetMap fixture root')
    pending = [directory]
    while pending:
        item = pending.pop()
        info = item.lstat()
        if item.is_symlink() or getattr(info, 'st_file_attributes', 0) & stat.FILE_ATTRIBUTE_REPARSE_POINT:
            raise RuntimeError('reparse points are forbidden in fixtures')
        if os.path.commonpath([str(resolved), str(item.resolve())]) != str(resolved):
            raise RuntimeError('fixture path escaped its root')
        if item.is_dir():
            pending.extend(item.iterdir())


def remove_fixture(directory, parent):
    validate_tree(directory, parent)
    shutil.rmtree(directory)


@unittest.skipUnless(os.name == 'nt', 'requires Windows cmd.exe')
class TestStreetMapSetup(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        compiler = Path(os.environ['WINDIR']) / 'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
        if not compiler.is_file():
            raise unittest.SkipTest('requires the Windows .NET Framework C# compiler')
        cls.parent = Path(os.environ.get('CARLA_STREETMAP_TEST_TMP', tempfile.gettempdir())).resolve()
        cls.parent.mkdir(parents=True, exist_ok=True)
        cls.tools = Path(tempfile.mkdtemp(prefix='carla-streetmap-tools-', dir=cls.parent))
        cls.addClassCleanup(remove_fixture, cls.tools, cls.parent)
        source = cls.tools / 'Recorder.cs'
        source.write_text(RECORDER, encoding='utf-8')
        executable = cls.tools / 'git.exe'
        subprocess.run([str(compiler), '/nologo', '/out:' + str(executable), str(source)],
                       check=True, capture_output=True, text=True, timeout=30)
        for tool in ('python', 'cmake', 'curl', 'wget', 'powershell', 'msbuild'):
            shutil.copyfile(executable, cls.tools / (tool + '.exe'))

    def setUp(self):
        self.directory = Path(tempfile.mkdtemp(prefix='carla-streetmap-case-', dir=self.parent))
        self.addCleanup(remove_fixture, self.directory, self.parent)
        name = 'Source With Spaces' if 'spaces' in self._testMethodName else 'Source'
        self.workspace = self.directory / name
        self.script = self.workspace / 'Util/BuildTools/BuildUE4Plugins.bat'
        self.script.parent.mkdir(parents=True)
        shutil.copyfile(ROOT / 'Util/BuildTools/BuildUE4Plugins.bat', self.script)
        self.destination = self.workspace / 'Unreal/CarlaUE4/Plugins/StreetMap'
        self.token = uuid.uuid4().hex
        (self.directory / '.fixture').write_text(self.token, encoding='utf-8')
        self.environment = os.environ.copy()
        self.environment.update({
            'ROOT_PATH': str(self.workspace) + os.sep,
            'PATH': str(self.tools) + os.pathsep + str(Path(os.environ['WINDIR']) / 'System32'),
            'CARLA_STREETMAP_FIXTURE': str(self.directory),
            'CARLA_STREETMAP_TOKEN': self.token,
            'CARLA_STREETMAP_DEST': str(self.destination),
            'CARLA_STREETMAP_CLONE_EXIT': '0',
            'CARLA_STREETMAP_FETCH_EXIT': '0',
            'CARLA_STREETMAP_CHECKOUT_EXIT': '0',
            'CARLA_STREETMAP_SKIP_DIRECTORY': '0',
            'CARLA_STREETMAP_SELF_CHECK_EXIT': '0',
        })

    def run_batch(self, args='--build'):
        validate_tree(self.directory, self.parent)
        validate_tree(self.tools, self.parent)
        if Path(self.environment['ROOT_PATH']).resolve() != self.workspace.resolve():
            raise RuntimeError('ROOT_PATH must be fixture-owned')
        wrapper = self.directory / 'run.cmd'
        wrapper.write_text(
            '@echo off\n'
            'where git > selected-git.txt\n'
            'if not "%errorlevel%"=="0" exit /b 97\n'
            'set /p SELECTED_GIT=<selected-git.txt\n'
            'if /i not "%SELECTED_GIT%"=="' + str(self.tools / 'git.exe') + '" exit /b 97\n'
            '"' + str(self.tools / 'git.exe') + '" --fixture-self-check ' + self.token + '\n'
            'if not "%errorlevel%"=="0" exit /b %errorlevel%\n'
            'call "' + str(self.script) + '" ' + args + '\n'
            'exit /b %errorlevel%\n', encoding='utf-8')
        return subprocess.run([os.environ['COMSPEC'], '/d', '/c', str(wrapper)],
                              cwd=self.workspace, env=self.environment,
                              capture_output=True, text=True, timeout=20)

    def calls(self):
        log = self.directory / 'calls.log'
        if not log.exists():
            return []
        return [base64.b64decode(line).decode().split('\0') for line in log.read_text().splitlines()]

    def test_new_clone_destination_with_spaces(self):
        result = self.run_batch()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertTrue(self.destination.is_dir(), result.stdout)
        self.assertEqual([c[0] for c in self.calls()], ['clone', 'fetch', 'checkout'])
        self.assertEqual(Path(self.calls()[0][4]).resolve(), self.destination.resolve())

    def test_existing_clone_with_spaces_is_not_cloned_again(self):
        self.destination.mkdir(parents=True)
        result = self.run_batch()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual([c[0] for c in self.calls()], ['fetch', 'checkout'])

    def test_clone_failure_stops_setup(self):
        self.environment['CARLA_STREETMAP_CLONE_EXIT'] = '37'
        result = self.run_batch()
        self.assertEqual(result.returncode, 37, result.stdout + result.stderr)
        self.assertEqual([c[0] for c in self.calls()], ['clone'])
        self.assertNotIn('successfully installed', result.stdout)

    def test_missing_clone_directory_stops_before_fetch(self):
        self.environment['CARLA_STREETMAP_SKIP_DIRECTORY'] = '1'
        result = self.run_batch()
        self.assertNotEqual(result.returncode, 0, result.stdout)
        self.assertEqual([c[0] for c in self.calls()], ['clone'])
        self.assertNotIn('successfully installed', result.stdout)

    def test_fetch_failure_stops_before_checkout(self):
        self.environment['CARLA_STREETMAP_FETCH_EXIT'] = '38'
        result = self.run_batch()
        self.assertEqual(result.returncode, 38, result.stdout + result.stderr)
        self.assertEqual([c[0] for c in self.calls()], ['clone', 'fetch'])
        self.assertNotIn('successfully installed', result.stdout)

    def test_checkout_failure_propagates(self):
        self.environment['CARLA_STREETMAP_CHECKOUT_EXIT'] = '39'
        result = self.run_batch()
        self.assertEqual(result.returncode, 39, result.stdout + result.stderr)
        self.assertEqual([c[0] for c in self.calls()], ['clone', 'fetch', 'checkout'])
        self.assertNotIn('successfully installed', result.stdout)

    def assert_negative_failure(self, variable, expected_calls):
        self.environment[variable] = '-1073741819'
        result = self.run_batch()
        self.assertEqual(result.returncode & 0xffffffff, 0xc0000005,
                         result.stdout + result.stderr)
        self.assertEqual([c[0] for c in self.calls()], expected_calls)
        self.assertNotIn('successfully installed', result.stdout)

    def test_negative_clone_failure_propagates(self):
        self.assert_negative_failure('CARLA_STREETMAP_CLONE_EXIT', ['clone'])

    def test_negative_fetch_failure_propagates(self):
        self.assert_negative_failure('CARLA_STREETMAP_FETCH_EXIT', ['clone', 'fetch'])

    def test_negative_checkout_failure_propagates(self):
        self.assert_negative_failure('CARLA_STREETMAP_CHECKOUT_EXIT', ['clone', 'fetch', 'checkout'])

    def test_no_pull_runs_no_git(self):
        result = self.run_batch('--build --no-pull')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(self.calls(), [])

    def test_help_runs_no_git(self):
        result = self.run_batch('--help')
        self.assertIn('Build LibCarla.', result.stdout)
        self.assertEqual(self.calls(), [])

    def test_external_root_is_rejected_before_execution(self):
        self.environment['ROOT_PATH'] = str(self.parent.parent)
        with self.assertRaisesRegex(RuntimeError, 'fixture-owned'):
            self.run_batch()
        self.assertEqual(self.calls(), [])

    def test_negative_recorder_self_check_stops_before_script(self):
        self.environment['CARLA_STREETMAP_SELF_CHECK_EXIT'] = '-1073741819'
        result = self.run_batch()
        self.assertEqual(result.returncode & 0xffffffff, 0xc0000005)
        self.assertEqual(self.calls(), [])
        self.assertNotIn('[Batch params]', result.stdout)


if __name__ == '__main__':
    unittest.main()
