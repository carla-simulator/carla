# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Exercise the actual Windows rebuild recipe and batch parser with inert tools."""
import os
from pathlib import Path
import re
import shutil
import stat
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]
RECORDER = r'''
using System;
using System.IO;
using System.Reflection;

class CommandRecorder {
    static int Main(string[] args) {
        string name = Path.GetFileNameWithoutExtension(
            Assembly.GetExecutingAssembly().Location).ToLowerInvariant();
        File.AppendAllText(Environment.GetEnvironmentVariable("CARLA_REBUILD_TEST_LOG"),
            name + " " + String.Join(" ", args) + Environment.NewLine);
        return name == "python" ? 0 : 91;
    }
}
'''


def validate_tree(directory, parent):
    """Refuse a recursive cleanup or batch run outside this fresh fixture."""
    directory, parent = Path(directory), Path(parent).resolve()
    resolved = directory.resolve()
    if (resolved.parent != parent or resolved == parent
            or not resolved.name.startswith("carla-rebuild-")):
        raise RuntimeError("unsafe rebuild fixture root")
    pending = [directory]
    while pending:
        item = pending.pop()
        info = item.lstat()
        if item.is_symlink() or getattr(info, "st_file_attributes", 0) & stat.FILE_ATTRIBUTE_REPARSE_POINT:
            raise RuntimeError("reparse points are forbidden in rebuild fixtures")
        if os.path.commonpath([str(resolved), str(item.resolve())]) != str(resolved):
            raise RuntimeError("fixture path escaped its root")
        if item.is_dir():
            pending.extend(item.iterdir())


def remove_fixture(directory, parent):
    validate_tree(directory, parent)
    shutil.rmtree(directory)


@unittest.skipUnless(os.name == "nt", "requires Windows cmd.exe")
class WindowsRebuild(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        compiler = Path(os.environ["WINDIR"]) / "Microsoft.NET/Framework64/v4.0.30319/csc.exe"
        if not compiler.is_file():
            raise unittest.SkipTest("requires the Windows .NET Framework C# compiler")
        cls.parent = Path(os.environ.get("CARLA_REBUILD_TEST_TMP", tempfile.gettempdir())).resolve()
        cls.parent.mkdir(parents=True, exist_ok=True)
        cls.tools = Path(tempfile.mkdtemp(prefix="carla-rebuild-tools-", dir=str(cls.parent)))
        cls.addClassCleanup(remove_fixture, cls.tools, cls.parent)
        source = cls.tools / "CommandRecorder.cs"
        source.write_text(RECORDER, encoding="utf-8")
        executable = cls.tools / "record.exe"
        subprocess.run([str(compiler), "/nologo", "/out:" + str(executable), str(source)],
                       check=True, capture_output=True, text=True, timeout=30)
        for name in ("python", "git", "curl", "powershell", "wget", "bitsadmin",
                     "cmake", "msbuild", "UE4Editor"):
            shutil.copyfile(executable, cls.tools / (name + ".exe"))

    def setUp(self):
        self.directory = Path(tempfile.mkdtemp(prefix="carla-rebuild-case-", dir=str(self.parent)))
        self.addCleanup(remove_fixture, self.directory, self.parent)
        self.workspace = self.directory / "Source"
        self.project = self.workspace / "Unreal/CarlaUE4"
        self.engine = self.directory / "Engine"
        self.script = self.workspace / "Util/BuildTools/BuildCarlaUE4.bat"
        self.script.parent.mkdir(parents=True)
        shutil.copyfile(ROOT / "Util/BuildTools/BuildCarlaUE4.bat", self.script)
        self.log = self.directory / "commands.log"
        self.environment = os.environ.copy()
        self.environment.update({
            "ROOT_PATH": str(self.workspace) + os.sep,
            "UE4_ROOT": str(self.engine),
            "PATH": str(self.tools) + os.pathsep + str(Path(os.environ["WINDIR"]) / "System32"),
            "CARLA_REBUILD_TEST_LOG": str(self.log),
            "CARLA_REBUILD_BUILD_EXIT": "0",
            "CARLA_REBUILD_ASSERT_CLEAN": "",
            "CARLA_REBUILD_REQUIRE_DEPS": "0",
            "CARLA_REBUILD_LIB_EXIT": "0",
            "CARLA_REBUILD_MARKERS": str(self.directory),
        })
        self.cleaned = [self.project / name for name in (
            "Binaries", "Build", "Saved", "Intermediate", "Plugins/Carla/Binaries",
            "Plugins/Carla/Intermediate", "Plugins/HoudiniEngine", ".vs")]
        for directory in self.cleaned:
            directory.mkdir(parents=True)
            (directory / "sentinel.txt").write_text("fixture only", encoding="utf-8")
        self.solution = self.project / "CarlaUE4.sln"
        self.solution.write_text("fixture only", encoding="utf-8")
        (self.project / "Config").mkdir()
        (self.project / "CarlaUE4.uproject").write_text("{}", encoding="utf-8")
        dependencies = self.project / "Plugins/Converters/get_dependencies.bat"
        dependencies.parent.mkdir()
        dependencies.write_bytes(
            b'@echo off\r\necho dependencies>>"%CARLA_REBUILD_TEST_LOG%"\r\nexit /b 0\r\n')
        for filename, marker in (("BuildLibCarla.bat", "libcarla"),
                                 ("BuildOSM2ODR.bat", "osm2odr"),
                                 ("BuildPythonAPI.bat", "pythonapi")):
            (self.script.parent / filename).write_text(
                '@echo off\n'
                + ('if not "%CARLA_REBUILD_LIB_EXIT%"=="0" exit /b %CARLA_REBUILD_LIB_EXIT%\n'
                   if marker == "libcarla" else '')
                + 'echo ' + marker + ' %*>>"%CARLA_REBUILD_TEST_LOG%"\n'
                + 'echo ready>"%CARLA_REBUILD_MARKERS%/' + marker + '.ready"\n'
                + 'exit /b 0\n', encoding="utf-8")
        build = self.engine / "Engine/Build/BatchFiles/Build.bat"
        build.parent.mkdir(parents=True)
        build.write_bytes(
            b'@echo off\r\necho build %*>>"%CARLA_REBUILD_TEST_LOG%"\r\n'
            b'if not "%CARLA_REBUILD_ASSERT_CLEAN%"=="" if exist "%CARLA_REBUILD_ASSERT_CLEAN%" exit /b 94\r\n'
            b'if "%CARLA_REBUILD_REQUIRE_DEPS%"=="1" if not exist "%CARLA_REBUILD_MARKERS%/libcarla.ready" exit /b 95\r\n'
            b'if "%CARLA_REBUILD_REQUIRE_DEPS%"=="1" if not exist "%CARLA_REBUILD_MARKERS%/osm2odr.ready" exit /b 96\r\n'
            b'exit /b %CARLA_REBUILD_BUILD_EXIT%\r\n')

    def run_commands(self, commands):
        validate_tree(self.directory, self.parent)
        if (Path(self.environment["ROOT_PATH"]).resolve() != self.workspace.resolve()
                or Path(self.environment["UE4_ROOT"]).resolve() != self.engine.resolve()):
            raise RuntimeError("batch roots must be fixture-owned")
        wrapper = self.directory / "run.cmd"
        wrapper.write_text('@echo off\n' + ''.join(
            'call "' + str(script) + '" ' + arguments
            + '\nif errorlevel 1 exit /b %errorlevel%\n'
            for script, arguments in commands) + 'exit /b 0\n', encoding="utf-8")
        return subprocess.run([os.environ["COMSPEC"], "/d", "/c", str(wrapper)],
                              cwd=self.workspace, env=self.environment,
                              capture_output=True, text=True, timeout=15)

    def run_batch(self, arguments):
        return self.run_commands([(self.script, arguments)])

    def run_rebuild_recipe(self):
        makefile = (ROOT / "Util/BuildTools/Windows.mk").read_text(encoding="utf-8")
        block = re.search(r"^rebuild:[^\n]*\n((?:\t[^\n]*\n)+)", makefile, re.MULTILINE)
        self.assertIsNotNone(block)
        commands = re.findall(
            r'^\t@"\$\{CARLA_BUILD_TOOLS_FOLDER\}/([^"/]+)"\s+([^\n]+)',
            block.group(1), re.MULTILINE)
        allowed = {"BuildCarlaUE4.bat", "BuildLibCarla.bat", "BuildOSM2ODR.bat", "BuildPythonAPI.bat"}
        if len(commands) != 4 or {name for name, _ in commands} != allowed:
            self.fail("unexpected rebuild commands; refusing to execute recipe")
        if any(not set(arguments.split()).issubset({"--build", "--clean", "--rebuild"})
               for _, arguments in commands):
            self.fail("unexpected rebuild arguments; refusing to execute recipe")
        return self.run_commands([(self.script.parent / name, arguments)
                                  for name, arguments in commands])

    def commands(self):
        return self.log.read_text(encoding="utf-8-sig").splitlines() if self.log.exists() else []

    def assert_builds(self, expected):
        calls = [line for line in self.commands() if line.startswith("build ")]
        self.assertEqual(len(calls), expected, self.commands())
        if expected:
            self.assertTrue(calls[0].startswith("build CarlaUE4Editor "), calls)
        if expected == 2:
            self.assertTrue(calls[1].startswith("build CarlaUE4 "), calls)

    def test_make_rebuild_cleans_before_building_both_targets(self):
        self.environment["CARLA_REBUILD_ASSERT_CLEAN"] = str(self.cleaned[0])
        self.environment["CARLA_REBUILD_REQUIRE_DEPS"] = "1"
        result = self.run_rebuild_recipe()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertFalse(any(path.exists() for path in self.cleaned), result.stdout)
        self.assertFalse(self.solution.exists())
        self.assert_builds(2)
        calls = self.commands()
        first_build = next(i for i, line in enumerate(calls) if line.startswith("build "))
        self.assertLess(calls.index("libcarla --rebuild"), first_build)
        self.assertLess(calls.index("osm2odr --rebuild"), first_build)
        self.assertEqual(calls[-1], "pythonapi --rebuild")

    def test_dependency_failure_prevents_engine_cleanup_and_build(self):
        self.environment["CARLA_REBUILD_LIB_EXIT"] = "41"
        result = self.run_rebuild_recipe()
        self.assertEqual(result.returncode, 41, result.stdout + result.stderr)
        self.assert_builds(0)
        self.assertTrue(all(path.exists() for path in self.cleaned))

    def test_build_only_preserves_existing_intermediates(self):
        result = self.run_batch("--build")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertTrue(all(path.exists() for path in self.cleaned))
        self.assertTrue(self.solution.exists())
        self.assert_builds(2)

    def test_clean_only_does_not_invoke_engine_build(self):
        result = self.run_batch("--clean")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertFalse(any(path.exists() for path in self.cleaned))
        self.assertFalse(self.solution.exists())
        self.assert_builds(0)

    def test_engine_failure_stops_before_second_target(self):
        self.environment["CARLA_REBUILD_BUILD_EXIT"] = "37"
        result = self.run_batch("--clean --build")
        self.assertEqual(result.returncode, 37, result.stdout + result.stderr)
        self.assert_builds(1)

    def test_help_preserves_files_without_running_dependencies_or_builds(self):
        result = self.run_batch("--help")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertTrue(all(path.exists() for path in self.cleaned))
        self.assertEqual(self.commands(), [])

    def test_external_root_is_rejected_before_batch_execution(self):
        self.environment["ROOT_PATH"] = str(ROOT)
        with self.assertRaisesRegex(RuntimeError, "batch roots must be fixture-owned"):
            self.run_batch("--clean --build")
        self.assertTrue(all(path.exists() for path in self.cleaned))
        self.assertEqual(self.commands(), [])


if __name__ == "__main__":
    unittest.main()
