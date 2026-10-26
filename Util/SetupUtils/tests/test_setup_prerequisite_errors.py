# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Exercise the real Windows bootstrap's prerequisite failure boundary."""

import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]
RECORDER = r'''
using System;
using System.IO;
using System.Reflection;
class Recorder {
    static int Main() {
        string name = Path.GetFileNameWithoutExtension(
            Assembly.GetExecutingAssembly().Location).ToLowerInvariant();
        File.AppendAllText(Environment.GetEnvironmentVariable("CARLA_PREREQ_TEST_LOG"), name + "\n");
        if (name == "vswhere") {
            Console.WriteLine(Environment.GetEnvironmentVariable("CARLA_PREREQ_TEST_VS_INSTALLATION"));
            return 0;
        }
        // All download, installer, environment mutation and engine tools fail closed.
        return name == "cmake" ? 0 : 91;
    }
}
'''


@unittest.skipUnless(os.name == 'nt', 'The bootstrap requires Windows cmd.exe')
class TestSetupPrerequisiteErrors(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        compiler = Path(os.environ['WINDIR']) / 'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
        if not compiler.is_file():
            raise unittest.SkipTest('The Windows .NET Framework C# compiler is required')
        temporary = tempfile.TemporaryDirectory(prefix='carla-prereq-tools-')
        cls.addClassCleanup(temporary.cleanup)
        cls.tools = Path(temporary.name)
        source = cls.tools / 'Recorder.cs'
        source.write_text(RECORDER, encoding='utf-8')
        cls.recorder = cls.tools / 'record.exe'
        subprocess.run([str(compiler), '/nologo', '/out:' + str(cls.recorder), str(source)],
                       check=True, capture_output=True, text=True)
        for name in ['cmake', 'git', 'curl', 'powershell', 'msbuild', 'setx']:
            shutil.copyfile(cls.recorder, cls.tools / (name + '.exe'))

    def setUp(self):
        temporary = tempfile.TemporaryDirectory(prefix='carla-prerequisite-errors-')
        self.addCleanup(temporary.cleanup)
        self.workspace = Path(temporary.name)
        shutil.copyfile(ROOT / 'CarlaSetup.bat', self.workspace / 'CarlaSetup.bat')
        prerequisites = self.workspace / 'Util' / 'SetupUtils' / 'InstallPrerequisites.bat'
        prerequisites.parent.mkdir(parents=True)
        prerequisites.write_bytes(b'@echo off\r\necho fixture prerequisite output\r\n'
                                 b'exit /b %CARLA_PREREQ_TEST_EXIT%\r\n')
        discovery = ROOT / 'Util' / 'SetupUtils' / 'FindVisualStudio.bat'
        if discovery.is_file():
            shutil.copyfile(discovery, prerequisites.with_name('FindVisualStudio.bat'))
        (self.workspace / 'Unreal' / 'CarlaUnreal' / 'Content').mkdir(parents=True)
        self.engine = self.workspace / 'Engine'
        self.engine.mkdir()
        self.program_files = self.workspace / 'Programs'
        activation = self.program_files / 'Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build/vcvars64.bat'
        activation.parent.mkdir(parents=True)
        activation.write_bytes(b'@echo off\r\nexit /b 0\r\n')
        self.vs_installation = activation.parents[3]
        query = self.program_files / 'Microsoft Visual Studio' / 'Installer' / 'vswhere.exe'
        query.parent.mkdir(parents=True)
        shutil.copyfile(self.recorder, query)
        self.log = self.workspace / 'commands.log'

    def run_setup(self, exit_code, arguments=''):
        environment = os.environ.copy()
        environment.update({
            'PATH': str(self.tools) + os.pathsep + str(Path(os.environ['WINDIR']) / 'System32'),
            'CARLA_UNREAL_ENGINE_PATH': str(self.engine),
            'CARLA_PREREQ_TEST_EXIT': str(exit_code),
            'CARLA_PREREQ_TEST_LOG': str(self.log),
            'CARLA_PREREQ_TEST_VS_INSTALLATION': str(self.vs_installation),
        })
        wrapper = self.workspace / 'run-test.cmd'
        wrapper.write_text('@echo off\nset "PROGRAMFILES=' + str(self.program_files) + '"\n'
                           'set "ProgramFiles(x86)=' + str(self.program_files) + '"\n'
                           'call CarlaSetup.bat ' + arguments + '\nexit /b %errorlevel%\n',
                           encoding='utf-8')
        return subprocess.run([os.environ['COMSPEC'], '/d', '/c', str(wrapper)],
                              cwd=self.workspace, env=environment,
                              capture_output=True, text=True, timeout=15)

    def test_failed_prerequisites_report_the_original_exit_code_and_stop(self):
        for exit_code in [1, 23, -1, -1073741515]:
            with self.subTest(exit_code=exit_code):
                result = self.run_setup(exit_code)

                self.assertEqual(result.returncode, exit_code & 0xffffffff)
                self.assertIn('Prerequisites installation failed with exit code ' + str(exit_code),
                              result.stdout + result.stderr)
                self.assertIn('fixture prerequisite output', result.stdout)
                self.assertFalse(self.log.exists(), 'Setup continued to external tools after failure')

    def test_successful_prerequisites_continue_without_a_failure_message(self):
        result = self.run_setup(0)

        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        commands = self.log.read_text().splitlines()
        self.assertEqual([name for name in commands if name != 'vswhere'], ['cmake'] * 3)
        self.assertNotIn('Prerequisites installation failed', result.stdout + result.stderr)

    def test_skipped_prerequisites_do_not_report_the_stub_failure(self):
        result = self.run_setup(23, '--skip-prerequisites')

        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        commands = self.log.read_text().splitlines()
        self.assertEqual([name for name in commands if name != 'vswhere'], ['cmake'] * 3)
        self.assertNotIn('fixture prerequisite output', result.stdout)


if __name__ == '__main__':
    unittest.main()
