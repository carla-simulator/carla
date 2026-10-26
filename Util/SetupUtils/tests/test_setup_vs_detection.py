# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Run Visual Studio discovery and its real callers without installing anything."""

import base64
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]
STUB_SOURCE = r'''
using System;
using System.IO;
using System.Reflection;
using System.Text;

class SetupCommand {
    static int Main(string[] args) {
        string name = Path.GetFileNameWithoutExtension(
            Assembly.GetExecutingAssembly().Location).ToLowerInvariant();
        using (StreamWriter writer = new StreamWriter(
                Environment.GetEnvironmentVariable("CARLA_VS_TEST_LOG"), true)) {
            writer.Write(name);
            foreach (string arg in args)
                writer.Write("|" + Convert.ToBase64String(Encoding.UTF8.GetBytes(arg)));
            writer.WriteLine();
        }
        if (name == "vswhere") {
            string result = Environment.GetEnvironmentVariable("CARLA_VS_TEST_RESULT");
            if (!String.IsNullOrEmpty(result)) Console.WriteLine(result);
            return 0;
        }
        // Never let a failed detection test download, install, or build anything.
        if (name == "curl" || name == "powershell" || name == "git" || name == "msbuild")
            return 91;
        if (name == "cmake" && Environment.GetEnvironmentVariable("CARLA_VS_TEST_ACTIVATED") != "1")
            return 92;
        return 0;
    }
}
'''


@unittest.skipUnless(os.name == 'nt', 'Visual Studio setup tests require Windows cmd.exe')
class TestSetupVisualStudioDetection(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        compiler = Path(os.environ['WINDIR']) / 'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
        if not compiler.is_file():
            raise unittest.SkipTest('The Windows .NET Framework C# compiler is required')
        cls.tools_directory = tempfile.TemporaryDirectory(prefix='carla-vs-tools-')
        cls.addClassCleanup(cls.tools_directory.cleanup)
        cls.tools = Path(cls.tools_directory.name)
        source = cls.tools / 'SetupCommand.cs'
        source.write_text(STUB_SOURCE, encoding='utf-8')
        cls.stub = cls.tools / 'record.exe'
        subprocess.run([str(compiler), '/nologo', '/out:' + str(cls.stub), str(source)],
                       check=True, capture_output=True, text=True)
        for name in ['cmake', 'ninja', 'python', 'curl', 'powershell', 'git', 'msbuild']:
            shutil.copyfile(cls.stub, cls.tools / (name + '.exe'))

    def setUp(self):
        temporary = tempfile.TemporaryDirectory(prefix='carla-vs-detection-')
        self.addCleanup(temporary.cleanup)
        self.directory = Path(temporary.name)
        self.workspace = self.directory / 'Source'
        self.workspace.mkdir()
        self.utility = self.workspace / 'Util' / 'SetupUtils'
        self.utility.mkdir(parents=True)
        shutil.copyfile(ROOT / 'CarlaSetup.bat', self.workspace / 'CarlaSetup.bat')
        for filename in ['FindVisualStudio.bat', 'InstallPrerequisites.bat']:
            shutil.copyfile(ROOT / 'Util' / 'SetupUtils' / filename, self.utility / filename)
        (self.workspace / 'Unreal' / 'CarlaUnreal' / 'Content').mkdir(parents=True)
        self.engine = self.directory / 'Engine'
        self.engine.mkdir()
        self.program_files = self.directory / 'Program Files'
        self.program_files_x86 = self.directory / 'Program Files (x86)'
        self.query = self.program_files_x86 / 'Microsoft Visual Studio' / 'Installer' / 'vswhere.exe'
        self.query.parent.mkdir(parents=True)
        shutil.copyfile(self.stub, self.query)
        self.installation = self.directory / 'Custom Drive' / 'Visual Studio Custom Installation'
        self.activation = self.make_installation(self.installation)
        self.activation_log = self.directory / 'activation.log'
        self.log = self.directory / 'commands.log'
        self.environment = os.environ.copy()
        self.environment.update({
            'PROGRAMFILES': str(self.program_files),
            'PROGRAMFILES(X86)': str(self.program_files_x86),
            'CARLA_VS_TEST_PROGRAM_FILES': str(self.program_files),
            'CARLA_VS_TEST_PROGRAM_FILES_X86': str(self.program_files_x86),
            'CARLA_UNREAL_ENGINE_PATH': str(self.engine),
            'CARLA_VS_TEST_LOG': str(self.log),
            'CARLA_VS_TEST_RESULT': str(self.installation),
            'CARLA_VS_TEST_ACTIVATION_EXIT': '0',
            'CARLA_VS_TEST_ACTIVATION_LOG': str(self.activation_log),
            'PATH': str(self.tools) + os.pathsep + str(Path(os.environ['WINDIR']) / 'System32'),
        })
        self.environment.pop('CARLA_VS_TEST_ACTIVATED', None)

    def make_installation(self, installation):
        activation = installation / 'VC' / 'Auxiliary' / 'Build' / 'vcvars64.bat'
        activation.parent.mkdir(parents=True)
        activation.write_bytes(b'@echo off\r\nset "CARLA_VS_TEST_ACTIVATED=1"\r\n'
                               b'echo activated>"%CARLA_VS_TEST_ACTIVATION_LOG%"\r\n'
                               b'exit /b %CARLA_VS_TEST_ACTIVATION_EXIT%\r\n')
        return activation

    def run_batch(self, script, arguments='', delayed_expansion=False):
        wrapper = self.workspace / 'run-test.cmd'
        wrapper.write_text(
            '@echo off\n'
            'set "ProgramFiles=%CARLA_VS_TEST_PROGRAM_FILES%"\n'
            'set "ProgramFiles(x86)=%CARLA_VS_TEST_PROGRAM_FILES_X86%"\n'
            + ('setlocal EnableDelayedExpansion\n' if delayed_expansion else 'setlocal DisableDelayedExpansion\n') +
            'set "vs_env_bat=STALE_PATH"\n'
            'call "' + script + '" ' + arguments + '\n'
            'set "test_exit=%errorlevel%"\n'
            'echo CALL_RETURNED=1\n'
            'echo "VS_ENV_BAT=[%vs_env_bat%]"\n'
            'echo ACTIVATED=[%CARLA_VS_TEST_ACTIVATED%]\n'
            'set "mode_probe=DELAYED_VALUE"\n'
            'echo DELAYED_MODE=[!mode_probe!]\n'
            'exit /b %test_exit%\n', encoding='utf-8')
        return subprocess.run([os.environ['COMSPEC'], '/d', '/c', str(wrapper)],
                              cwd=self.workspace, env=self.environment,
                              capture_output=True, text=True, timeout=20)

    def commands(self):
        if not self.log.exists():
            return []
        result = []
        for line in self.log.read_text(encoding='utf-8-sig').splitlines():
            name, *encoded = line.split('|')
            result.append((name, [base64.b64decode(arg).decode('utf-8') for arg in encoded]))
        return result

    def assert_query_requirements(self):
        queries = [args for name, args in self.commands() if name == 'vswhere']
        self.assertEqual(len(queries), 1)
        self.assertEqual(queries[0], ['-latest', '-products', '*', '-version', '[17.0,18.0)',
                                     '-requires', 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64',
                                     '-property', 'installationPath'])

    def test_setup_keeps_helper_location_after_python_root_arguments(self):
        python_root = self.directory / 'PythonInstallation'
        for option in ['--python-root', '-pyroot']:
            with self.subTest(option=option):
                self.log.unlink(missing_ok=True)
                self.activation_log.unlink(missing_ok=True)
                result = self.run_batch(
                    'CarlaSetup.bat', '--skip-prerequisites ' + option + ' ' + str(python_root))

                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assert_query_requirements()
                self.assertTrue(self.activation_log.is_file())
                self.assertEqual(sum(name == 'cmake' for name, _ in self.commands()), 3)

    def test_prerequisites_keeps_helper_location_after_python_path_arguments(self):
        python = self.tools / 'python.exe'
        for option in ['--python-path', '-pypath']:
            with self.subTest(option=option):
                self.log.unlink(missing_ok=True)
                result = self.run_batch(
                    'Util\\SetupUtils\\InstallPrerequisites.bat', option + ' ' + str(python))

                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assert_query_requirements()
                self.assertIn('Found Visual Studio 2022.', result.stdout)

    def test_helper_detects_custom_installation_with_spaces(self):
        result = self.run_batch('Util\\SetupUtils\\FindVisualStudio.bat')

        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('VS_ENV_BAT=[' + str(self.activation) + ']', result.stdout)
        self.assert_query_requirements()

    def test_empty_query_clears_stale_result_and_returns_failure(self):
        self.environment['CARLA_VS_TEST_RESULT'] = ''
        result = self.run_batch('Util\\SetupUtils\\FindVisualStudio.bat')

        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('VS_ENV_BAT=[]', result.stdout)

    def test_nonexistent_activation_script_is_not_a_usable_result(self):
        self.activation.unlink()
        result = self.run_batch('Util\\SetupUtils\\FindVisualStudio.bat')

        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('VS_ENV_BAT=[]', result.stdout)

    def test_missing_vswhere_returns_failure_without_guessing_edition_paths(self):
        self.query.unlink()
        self.make_installation(self.program_files / 'Microsoft Visual Studio' / '2022' / 'Community')
        result = self.run_batch('Util\\SetupUtils\\FindVisualStudio.bat')

        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('VS_ENV_BAT=[]', result.stdout)
        self.assertEqual(self.commands(), [])

    def test_vswhere_location_falls_back_to_program_files(self):
        fallback = self.program_files / 'Microsoft Visual Studio' / 'Installer' / 'vswhere.exe'
        fallback.parent.mkdir(parents=True)
        shutil.move(str(self.query), str(fallback))
        result = self.run_batch('Util\\SetupUtils\\FindVisualStudio.bat')

        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn(str(self.activation), result.stdout)
        self.assert_query_requirements()

    def test_helper_preserves_exclamation_when_called_with_expansion_disabled(self):
        installation = self.directory / 'VS !Custom & Installation'
        activation = self.make_installation(installation)
        self.environment['CARLA_VS_TEST_RESULT'] = str(installation)
        result = self.run_batch('Util\\SetupUtils\\FindVisualStudio.bat')

        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('VS_ENV_BAT=[' + str(activation) + ']', result.stdout)
        self.assertIn('DELAYED_MODE=[!mode_probe!]', result.stdout)

    def test_setup_preserves_literal_exclamation_and_callers_expansion_mode(self):
        installation = self.directory / 'VS !Custom & Installation'
        self.make_installation(installation)
        self.environment['CARLA_VS_TEST_RESULT'] = str(installation)
        result = self.run_batch('CarlaSetup.bat', '--skip-prerequisites', delayed_expansion=True)

        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertTrue(self.activation_log.is_file())
        self.assertIn('DELAYED_MODE=[DELAYED_VALUE]', result.stdout)
        self.assertEqual([name for name, _ in self.commands()], ['vswhere', 'cmake', 'cmake', 'cmake'])

    def test_prerequisites_preserve_exclamation_and_callers_expansion_mode(self):
        installation = self.directory / 'VS !Custom & Installation'
        self.make_installation(installation)
        self.environment['CARLA_VS_TEST_RESULT'] = str(installation)
        result = self.run_batch('Util\\SetupUtils\\InstallPrerequisites.bat', delayed_expansion=True)

        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('Found Visual Studio 2022.', result.stdout)
        self.assertIn('DELAYED_MODE=[DELAYED_VALUE]', result.stdout)
        self.assertEqual([name for name, _ in self.commands()], ['vswhere', 'ninja', 'python', 'python', 'python'])

    def test_setup_activates_custom_installation_and_reaches_cmake_stubs(self):
        result = self.run_batch('CarlaSetup.bat', '--skip-prerequisites')

        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertTrue(self.activation_log.is_file())
        self.assertIn('CALL_RETURNED=1', result.stdout)
        self.assertIn('DELAYED_MODE=[!mode_probe!]', result.stdout)
        self.assertEqual([name for name, _ in self.commands()], ['vswhere', 'cmake', 'cmake', 'cmake'])
        self.assert_query_requirements()

    def test_setup_missing_vs_returns_to_caller_with_failure(self):
        self.environment['CARLA_VS_TEST_RESULT'] = ''
        result = self.run_batch('CarlaSetup.bat', '--skip-prerequisites')

        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('CALL_RETURNED=1', result.stdout)
        self.assertFalse(self.activation_log.exists())
        self.assertEqual([name for name, _ in self.commands()], ['vswhere'])

    def test_setup_propagates_activation_failure_without_building(self):
        self.environment['CARLA_VS_TEST_ACTIVATION_EXIT'] = '7'
        result = self.run_batch('CarlaSetup.bat', '--skip-prerequisites')

        self.assertEqual(result.returncode, 7, result.stdout + result.stderr)
        self.assertIn('CALL_RETURNED=1', result.stdout)
        self.assertEqual([name for name, _ in self.commands()], ['vswhere'])

    def test_prerequisites_reuse_custom_installation_without_downloading(self):
        result = self.run_batch('Util\\SetupUtils\\InstallPrerequisites.bat')

        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('Found Visual Studio 2022.', result.stdout)
        self.assertEqual([name for name, _ in self.commands()], ['vswhere', 'ninja', 'python', 'python', 'python'])
        self.assertIn('DELAYED_MODE=[!mode_probe!]', result.stdout)
        self.assert_query_requirements()

    def test_prerequisites_missing_vs_uses_existing_download_failure_path(self):
        self.environment['CARLA_VS_TEST_RESULT'] = ''
        result = self.run_batch('Util\\SetupUtils\\InstallPrerequisites.bat')

        self.assertEqual(result.returncode, 91, result.stdout + result.stderr)
        self.assertIn('CALL_RETURNED=1', result.stdout)
        self.assertEqual([name for name, _ in self.commands()], ['vswhere', 'curl'])


if __name__ == '__main__':
    unittest.main()
