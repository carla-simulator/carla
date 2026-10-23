"""Run the Windows bootstrap against recording executables, without downloads.

Run with: python -m unittest discover -s Util/SetupUtils/tests -v
"""

import base64
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]
STUB_SOURCE = r"""
using System;
using System.IO;
using System.Reflection;
using System.Text;

class RecordingCommand {
    static int Main(string[] args) {
        string executable = Assembly.GetExecutingAssembly().Location;
        string name = Path.GetFileNameWithoutExtension(executable).ToLowerInvariant();
        using (StreamWriter log = new StreamWriter(
                Environment.GetEnvironmentVariable("CARLA_TEST_COMMAND_LOG"), true)) {
            log.Write(Convert.ToBase64String(Encoding.UTF8.GetBytes(executable)));
            foreach (string arg in args) {
                log.Write("|" + Convert.ToBase64String(Encoding.UTF8.GetBytes(arg)));
            }
            log.WriteLine();
        }
        // Fail closed if a regression tries to download or run an installer.
        if (name == "curl" || name == "powershell") return 99;
        string failure = Environment.GetEnvironmentVariable(
            "CARLA_TEST_" + name.ToUpperInvariant() + "_EXIT");
        return failure == null ? 0 : Int32.Parse(failure);
    }
}
"""


@unittest.skipUnless(os.name == "nt", "Windows batch scripts require cmd.exe")
class WindowsSetupTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        compiler = Path(os.environ["WINDIR"]) / (
            "Microsoft.NET/Framework64/v4.0.30319/csc.exe"
        )
        if not compiler.is_file():
            raise unittest.SkipTest("The .NET Framework C# compiler is required")
        cls.tools_directory = tempfile.TemporaryDirectory(prefix="carla-test-tools-")
        cls.addClassCleanup(cls.tools_directory.cleanup)
        cls.tools = Path(cls.tools_directory.name)
        source = cls.tools / "RecordingCommand.cs"
        source.write_text(STUB_SOURCE)
        stub = cls.tools / "record.exe"
        subprocess.run(
            [str(compiler), "/nologo", "/out:" + str(stub), str(source)],
            check=True,
            capture_output=True,
            text=True,
        )
        for name in ("cmake", "git", "ninja", "python", "curl", "powershell", "setx"):
            shutil.copyfile(stub, cls.tools / (name + ".exe"))

    def setUp(self):
        directory = tempfile.TemporaryDirectory(prefix="carla-setup-test-")
        self.addCleanup(directory.cleanup)
        self.directory = Path(directory.name) / "Workspace With Spaces"
        self.workspace = self.directory / "Carla Source"
        self.workspace.mkdir(parents=True)
        shutil.copyfile(ROOT / "CarlaSetup.bat", self.workspace / "CarlaSetup.bat")
        prerequisites = self.workspace / "Util/SetupUtils/InstallPrerequisites.bat"
        prerequisites.parent.mkdir(parents=True)
        shutil.copyfile(ROOT / "Util/SetupUtils/InstallPrerequisites.bat", prerequisites)
        self.content = self.workspace / "Unreal/CarlaUnreal/Content"
        (self.content / "Carla").mkdir(parents=True)
        self.engine = self.directory / "UnrealEngine5_carla"
        self.engine.mkdir()
        self.python_root = self.directory / "Python Installation"
        self.python_root.mkdir()
        self.python = self.python_root / "python.exe"
        shutil.copyfile(self.tools / "record.exe", self.python)
        self.program_files = self.directory / "Program Files"
        activation = self.program_files / (
            "Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build/vcvars64.bat"
        )
        activation.parent.mkdir(parents=True)
        activation.write_text("@echo off\nexit /b 0\n")
        self.log = self.directory / "commands.log"
        self.env = {
            key: value for key, value in os.environ.items()
            if key.upper() not in ("CARLA_UNREAL_ENGINE_PATH", "PATH")
            and not key.upper().startswith("CARLA_TEST_")
        }
        self.env.update({
            "PATH": str(self.tools) + os.pathsep + str(Path(os.environ["WINDIR"]) / "System32"),
            "CARLA_UNREAL_ENGINE_PATH": str(self.engine),
            "CARLA_TEST_COMMAND_LOG": str(self.log),
        })

    def run_script(self, arguments="--skip-prerequisites", script="CarlaSetup.bat"):
        # cmd initializes ProgramFiles itself, so set the fixture after it starts.
        runner = self.workspace / "run-test.bat"
        runner.write_text(
            '@echo off\nset "PROGRAMFILES=' + str(self.program_files)
            + '"\ncall ' + script + " " + arguments + "\n"
        )
        return subprocess.run(
            [os.environ["COMSPEC"], "/d", "/v:off", "/c", "run-test.bat"],
            cwd=self.workspace,
            env=self.env,
            capture_output=True,
            text=True,
            timeout=20,
        )

    def commands(self, name):
        if not self.log.exists():
            return []
        records = [
            [base64.b64decode(part).decode("utf-8") for part in line.split("|")]
            for line in self.log.read_text().splitlines()
        ]
        return [record for record in records if Path(record[0]).stem.lower() == name]

    def assert_success(self, result):
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertFalse(self.commands("curl"), "Unexpected download attempted")
        self.assertFalse(self.commands("powershell"), "Unexpected installer attempted")

    def test_existing_sibling_engine_is_passed_to_cmake(self):
        del self.env["CARLA_UNREAL_ENGINE_PATH"]
        self.assert_success(self.run_script())
        self.assertIn("-DCARLA_UNREAL_ENGINE_PATH=" + str(self.engine), self.commands("cmake")[0])
        self.assertFalse(self.commands("git"))
        self.assertFalse(self.commands("setx"), "Reusing an engine must not change the user environment")

    def test_explicit_engine_takes_precedence(self):
        explicit = self.directory / "Explicit Engine"
        explicit.mkdir()
        self.env["CARLA_UNREAL_ENGINE_PATH"] = str(explicit)
        self.assert_success(self.run_script())
        self.assertIn("-DCARLA_UNREAL_ENGINE_PATH=" + str(explicit), self.commands("cmake")[0])

    def test_empty_content_parent_is_not_treated_as_assets(self):
        (self.content / "Carla").rmdir()
        self.assert_success(self.run_script())
        self.assertEqual(len(self.commands("git")), 1)
        self.assertEqual(self.commands("git")[0][1:], [
            "-C", str(self.content), "clone", "-b", "ue58-dev-carla",
            "https://bitbucket.org/carla-simulator/carla-content.git", "Carla",
        ])

    def test_missing_content_parent_is_created(self):
        (self.content / "Carla").rmdir()
        self.content.rmdir()
        self.assert_success(self.run_script())
        self.assertTrue(self.content.is_dir())
        self.assertEqual(len(self.commands("git")), 1)

    def test_existing_content_is_reused(self):
        self.assert_success(self.run_script())
        self.assertFalse(self.commands("git"))

    def test_failed_content_clone_stops_before_configuration(self):
        (self.content / "Carla").rmdir()
        self.env["CARLA_TEST_GIT_EXIT"] = "23"
        result = self.run_script()
        self.assertEqual(result.returncode, 23, result.stdout + result.stderr)
        self.assertFalse(self.commands("cmake"))

    def test_python_root_with_spaces_reaches_cmake(self):
        for argument in (
            '--python-root "' + str(self.python_root) + '"',
            '--python-root="' + str(self.python_root) + '"',
            '-pyroot "' + str(self.python_root) + '"',
        ):
            with self.subTest(argument=argument):
                self.log.unlink(missing_ok=True)
                self.assert_success(self.run_script("--skip-prerequisites " + argument))
                configure = self.commands("cmake")[0]
                self.assertIn("-DPython_ROOT_DIR=" + str(self.python_root), configure)
                self.assertIn("-DPython3_ROOT_DIR=" + str(self.python_root), configure)

    def test_selected_python_is_used_by_prerequisites(self):
        self.assert_success(self.run_script('--python-root "' + str(self.python_root) + '"'))
        commands = self.commands("python")
        self.assertEqual([command[0] for command in commands], [str(self.python)] * 3)
        self.assertEqual([command[1:] for command in commands], [
            ["-V"], ["-m", "pip", "install", "--upgrade", "pip"],
            ["-m", "pip", "install", "-r", "requirements.txt"],
        ])

    def test_prerequisites_can_parse_python_path_when_called_directly(self):
        for argument in (
            '--python-path "' + str(self.python) + '"',
            '--python-path="' + str(self.python) + '"',
            '-pypath "' + str(self.python) + '"',
        ):
            with self.subTest(argument=argument):
                self.log.unlink(missing_ok=True)
                self.assert_success(self.run_script(argument, "Util\\SetupUtils\\InstallPrerequisites.bat"))
                self.assertEqual([command[0] for command in self.commands("python")], [str(self.python)] * 3)

    def test_missing_python_root_is_rejected_before_setup(self):
        for argument in ("--python-root", "--python-root=", "-pyroot"):
            with self.subTest(argument=argument):
                self.assertEqual(self.run_script(argument).returncode, 1)
                self.assertFalse(self.log.exists())


if __name__ == "__main__":
    unittest.main()
