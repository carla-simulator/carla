# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Capture actual prerequisite installer arguments without installing software."""

import base64
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
class InstallerBoundary {
    static int Main(string[] args) {
        string executable = Assembly.GetExecutingAssembly().Location;
        string name = Path.GetFileNameWithoutExtension(executable).ToLowerInvariant();
        string[] encoded = Array.ConvertAll(args, value =>
            Convert.ToBase64String(System.Text.Encoding.UTF8.GetBytes(value)));
        File.AppendAllText(Environment.GetEnvironmentVariable("CARLA_INSTALLER_TEST_LOG"),
                           name + "|" + String.Join("|", encoded) + "\n");
        if (name == "curl") {
            string target = Path.Combine(Environment.GetEnvironmentVariable("CARLA_INSTALLER_TEST_ROOT"), "Temp");
            if (args.Length != 3 || args[0] != "-L" || args[1] != "-O" ||
                    args[2] != "https://aka.ms/vs/17/release/vs_community.exe" ||
                    !String.Equals(Directory.GetCurrentDirectory(), target,
                                   StringComparison.OrdinalIgnoreCase)) return 91;
            File.Copy(executable, Path.Combine(target, "vs_community.exe"));
            return 0;
        }
        if (name == "vs_community" || name == "ninja" || name == "python") return 0;
        return 91;
    }
}
'''


@unittest.skipUnless(os.name == 'nt', 'The prerequisite script requires Windows cmd.exe')
class TestVisualStudioInstallerArguments(unittest.TestCase):
    """Check the complete production script at the native installer boundary."""

    def test_documented_component_arguments(self):
        """Require separate add switches and the documented SDK component ID."""
        compiler = Path(os.environ['WINDIR']) / 'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
        if not compiler.is_file():
            self.skipTest('The Windows .NET Framework C# compiler is required')
        with tempfile.TemporaryDirectory(prefix='carla-installer-arguments-') as temporary:
            workspace = Path(temporary).resolve()
            tools = workspace / 'tools'
            tools.mkdir()
            source = tools / 'InstallerBoundary.cs'
            source.write_text(RECORDER, encoding='utf-8')
            recorder = tools / 'recorder.exe'
            subprocess.run([str(compiler), '/nologo', '/out:' + str(recorder), str(source)],
                           capture_output=True, text=True, check=True)
            for name in ['curl', 'ninja', 'python', 'powershell', 'setx']:
                shutil.copyfile(recorder, tools / (name + '.exe'))
            shutil.copyfile(ROOT / 'Util/SetupUtils/InstallPrerequisites.bat', workspace / 'prerequisites.bat')
            log = workspace / 'commands.log'
            environment = os.environ.copy()
            environment.update({
                'PATH': str(tools) + os.pathsep + str(Path(os.environ['WINDIR']) / 'System32'),
                'CARLA_INSTALLER_TEST_ROOT': str(workspace),
                'CARLA_INSTALLER_TEST_LOG': str(log),
            })
            wrapper = workspace / 'run-test.cmd'
            wrapper.write_text('@echo off\nset "ProgramFiles=' + str(workspace / 'Programs') + '"\n'
                               'call prerequisites.bat\nexit /b %errorlevel%\n', encoding='utf-8')
            result = subprocess.run([os.environ['COMSPEC'], '/d', '/c', str(wrapper)], cwd=workspace,
                                    env=environment, capture_output=True, text=True, check=False, timeout=20)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            commands = [line.split('|') for line in log.read_text().splitlines()]
            self.assertEqual([parts[0] for parts in commands],
                             ['curl', 'vs_community', 'ninja', 'python', 'python', 'python'])
            arguments = [base64.b64decode(value).decode('utf-8') for value in commands[1][1:]]
            with self.subTest(contract='one add switch per distinct workload or component'):
                components = [value for value in arguments if value.startswith('Microsoft.')]
                self.assertEqual(len(components), 12)
                self.assertEqual(arguments,
                                 [argument for component in components for argument in ['--add', component]] +
                                 ['--installWhileDownloading', '--passive', '--wait'])
            with self.subTest(contract='valid SDK22621 component ID'):
                self.assertIn('Microsoft.VisualStudio.Component.Windows11SDK.22621', arguments)
                self.assertNotIn('Microsoft.VisualStudio.Component.Windows10SDK.22621', arguments)


if __name__ == '__main__':
    unittest.main()
