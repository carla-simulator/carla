# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Exercise the real Linux setup script with fail-closed command recorders."""

import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]
if os.name == "nt":
    candidate = Path(os.environ.get("ProgramFiles", r"C:\Program Files")) / "Git/bin/bash.exe"
    BASH = str(candidate) if candidate.is_file() else None
else:
    BASH = shutil.which("bash")

RECORDER = r'''
import json, os, pathlib, sys
name, args = sys.argv[1], sys.argv[2:]
with open(os.environ["CARLA_CONTENT_TEST_LOG"], "a", encoding="utf-8") as log:
    log.write(json.dumps([name, args]) + "\n")
checkout = pathlib.Path(os.environ["CARLA_CONTENT_TEST_CHECKOUT"])
state = os.environ.get("CARLA_CONTENT_TEST_STATE", "parent")
if name == "git":
    if args == ["--fixture-recorder-check"]:
        print("CARLA_RECORDING_STUB")
        sys.exit(0)
    if len(args) == 4 and args[0] == "-C" and args[2:] == ["rev-parse", "--show-toplevel"]:
        assert pathlib.Path(args[1]).resolve() == checkout.resolve()
        result = pathlib.Path(os.environ.get("CARLA_CONTENT_TEST_ROOT", str(checkout))) if state in ("valid", "partial") else pathlib.Path(os.environ["CARLA_CONTENT_TEST_WORKSPACE"])
        print(result.as_posix())
        sys.exit(0)
    if len(args) == 5 and args[0] == "-C" and args[2:] == ["rev-parse", "--verify", "HEAD^{commit}"]:
        assert pathlib.Path(args[1]).resolve() == checkout.resolve()
        if state == "valid":
            print("0" * 40)
            sys.exit(0)
        sys.exit(128)
    if len(args) == 7 and args[0] == "-C" and args[2:] == ["clone", "-b", "ue58-dev-carla", "https://bitbucket.org/carla-simulator/carla-content.git", "Carla"]:
        assert pathlib.Path(args[1]).resolve() == checkout.parent.resolve()
        if checkout.exists() and (not checkout.is_dir() or any(checkout.iterdir())):
            sys.exit(128)
        checkout.mkdir(parents=True, exist_ok=True)
        (checkout / ".git").mkdir()
        clone_exit = int(os.environ.get("CARLA_CONTENT_TEST_CLONE_EXIT", "0"))
        if clone_exit:
            (checkout / ".git/partial").write_text("interrupted clone", encoding="utf-8")
            sys.exit(clone_exit)
        (checkout / ".git/HEAD").write_text("fixture commit", encoding="utf-8")
        (checkout / "Maps").mkdir()
        (checkout / "Maps/Fixture.umap").write_bytes(b"fixture content")
        sys.exit(0)
elif name == "bash":
    if args[:2] == ["-x", "Util/SetupUtils/InstallPrerequisites.sh"]:
        sys.exit(int(os.environ.get("CARLA_CONTENT_TEST_INSTALL_EXIT", "0")))
    if args == ["Util/SetupUtils/SetupDLSS.sh"]:
        sys.exit(0)
elif name == "cmake":
    if args[:2] == ["-G", "Ninja"] or args[:2] == ["--build", "Build"]:
        sys.exit(0)
# Unexpected clone, install, build, shell, network or destructive operations
# never fall through to a real executable on the developer's PATH.
sys.exit(91)
'''


def shell_executable(path):
    path = Path(path).resolve()
    if os.name == "nt":
        return "/" + path.drive[0].lower() + path.as_posix()[2:]
    return str(path)


@unittest.skipUnless(BASH, "The Linux setup regression requires Bash (Git Bash on Windows)")
class TestLinuxContentCheckout(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory(prefix="carla-linux-content-")
        self.addCleanup(temporary.cleanup)
        self.directory = Path(temporary.name)
        self.workspace = self.directory / "Source"
        self.workspace.mkdir()
        self.script = self.workspace / "CarlaSetup.sh"
        self.script.write_text((ROOT / "CarlaSetup.sh").read_text(encoding="utf-8"),
                               encoding="utf-8", newline="\n")
        self.checkout = self.workspace / "Unreal/CarlaUnreal/Content/Carla"
        self.log = self.directory / "commands.jsonl"
        tools = self.directory / "tools"
        tools.mkdir()
        recorder = tools / "recorder.py"
        recorder.write_text(RECORDER, encoding="utf-8")
        for name in ("git", "bash", "cmake", "sudo", "curl", "wget", "make", "ninja",
                     "python", "python3", "rm", "rmdir", "mv", "ln"):
            command = tools / name
            command.write_text("#!/bin/bash\nexec " + shlex.quote(shell_executable(sys.executable))
                               + " " + shlex.quote(recorder.as_posix()) + " " + name + ' "$@"\n',
                               encoding="utf-8", newline="\n")
            command.chmod(0o755)
        engine = self.directory / "Engine"
        engine.mkdir()
        self.environment = os.environ.copy()
        self.environment.update({
            "PATH": str(tools) + os.pathsep + os.environ.get("PATH", ""),
            "GIT_LOCAL_CREDENTIALS": "fixture-not-a-token",
            "CARLA_UNREAL_ENGINE_PATH": str(engine),
            "DLSS_SDK": "disabled",
            "CARLA_CONTENT_TEST_LOG": str(self.log),
            "CARLA_CONTENT_TEST_CHECKOUT": str(self.checkout),
            "CARLA_CONTENT_TEST_WORKSPACE": str(self.workspace),
            "CARLA_CONTENT_TEST_STATE": "parent",
            "CARLA_CONTENT_TEST_CLONE_EXIT": "0",
            "CARLA_CONTENT_TEST_INSTALL_EXIT": "0",
            "CARLA_CONTENT_TEST_STUB_PATH": shell_executable(tools),
        })
        self.environment.pop("BASH_ENV", None)
        self.environment.pop("ENV", None)

    def run_setup(self, *arguments):
        # Git Bash's Windows launcher prepends its own executables to PATH.
        # Override PATH inside that shell, verify all dangerous entry points,
        # then source the actual script without spawning another launcher.
        wrapper = '''
export PATH="$CARLA_CONTENT_TEST_STUB_PATH:/usr/bin:/bin"
hash -r
for tool in git bash cmake sudo curl wget make ninja python python3 rm rmdir mv ln; do
    [ "$(command -v "$tool")" = "$CARLA_CONTENT_TEST_STUB_PATH/$tool" ] || exit 90
done
[ "$(git --fixture-recorder-check)" = CARLA_RECORDING_STUB ] || exit 90
: > "$CARLA_CONTENT_TEST_LOG"
script=$1
shift
source "$script" "$@"
'''
        return subprocess.run([BASH, "--noprofile", "--norc", "-c", wrapper, "content-fixture", str(self.script),
                               *(arguments or ("--skip-prerequisites",))],
                              cwd=self.workspace, env=self.environment,
                              capture_output=True, text=True, timeout=30)

    def commands(self):
        return [json.loads(line) for line in self.log.read_text(encoding="utf-8").splitlines()] if self.log.exists() else []

    def clone_commands(self):
        return [args for name, args in self.commands()
                if name == "git" and args[2:3] == ["clone"]]

    def assert_completed(self, result):
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual([name for name, _ in self.commands() if name == "cmake"], ["cmake"] * 3)
        self.assertFalse(any(name in {"sudo", "curl", "wget", "make", "rm", "rmdir", "mv", "ln"}
                             for name, _ in self.commands()))

    def assert_cloned(self):
        clones = self.clone_commands()
        self.assertEqual(len(clones), 1)
        self.assertEqual(clones[0][2:], ["clone", "-b", "ue58-dev-carla",
                                       "https://bitbucket.org/carla-simulator/carla-content.git", "Carla"])
        self.assertEqual((self.checkout / "Maps/Fixture.umap").read_bytes(), b"fixture content")

    def existing_files(self):
        return {path.relative_to(self.workspace): path.read_bytes()
                for path in self.workspace.rglob("*") if path.is_file()}

    def assert_rejected_without_changes(self):
        before = self.existing_files()
        result = self.run_setup()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn("not a valid Git checkout", result.stdout + result.stderr)
        self.assertEqual(self.existing_files(), before)
        self.assertEqual(self.clone_commands(), [])
        self.assertFalse(any(name in {"bash", "cmake", "rm", "rmdir", "mv", "ln"}
                             for name, _ in self.commands()))

    def test_absent_content_parent_is_created_and_cloned(self):
        self.assert_completed(self.run_setup())
        self.assert_cloned()

    def test_empty_content_parent_retries_the_missing_checkout(self):
        self.checkout.parent.mkdir(parents=True)
        self.assert_completed(self.run_setup())
        self.assert_cloned()

    def test_empty_checkout_directory_can_be_cloned_without_removal(self):
        self.checkout.mkdir(parents=True)
        self.assert_completed(self.run_setup())
        self.assert_cloned()

    def test_other_parent_content_is_preserved_while_cloning(self):
        self.checkout.parent.mkdir(parents=True)
        keep = self.checkout.parent / "authored.data"
        keep.write_bytes(b"user content")
        self.assert_completed(self.run_setup())
        self.assert_cloned()
        self.assertEqual(keep.read_bytes(), b"user content")

    def test_valid_committed_checkout_is_reused_without_changes(self):
        (self.checkout / ".git").mkdir(parents=True)
        (self.checkout / "authored.data").write_bytes(b"user modifications")
        self.environment["CARLA_CONTENT_TEST_STATE"] = "valid"
        before = self.existing_files()
        self.assert_completed(self.run_setup())
        self.assertEqual(self.clone_commands(), [])
        self.assertEqual(self.existing_files(), before)

    def test_valid_worktree_git_file_is_supported(self):
        self.checkout.mkdir(parents=True)
        (self.checkout / ".git").write_text("gitdir: fixture metadata", encoding="utf-8")
        self.environment["CARLA_CONTENT_TEST_STATE"] = "valid"
        self.assert_completed(self.run_setup())
        self.assertEqual(self.clone_commands(), [])

    def test_parent_repository_does_not_validate_a_nonempty_invalid_child(self):
        self.checkout.mkdir(parents=True)
        (self.checkout / "authored.data").write_bytes(b"do not delete")
        self.assert_rejected_without_changes()

    def test_partial_repository_without_head_is_preserved_and_rejected(self):
        (self.checkout / ".git").mkdir(parents=True)
        (self.checkout / ".git/partial").write_bytes(b"interrupted metadata")
        self.environment["CARLA_CONTENT_TEST_STATE"] = "partial"
        self.assert_rejected_without_changes()

    def test_non_directory_checkout_path_is_preserved_and_rejected(self):
        self.checkout.parent.mkdir(parents=True)
        self.checkout.write_bytes(b"user file")
        self.assert_rejected_without_changes()

    def test_failed_clone_stops_before_other_setup_and_retains_partial_files(self):
        self.environment["CARLA_CONTENT_TEST_CLONE_EXIT"] = "17"
        result = self.run_setup()
        self.assertEqual(result.returncode, 17, result.stdout + result.stderr)
        self.assertEqual((self.checkout / ".git/partial").read_text(), "interrupted clone")
        self.assertFalse(any(name in {"bash", "cmake"} for name, _ in self.commands()))
        self.log.unlink()
        self.environment["CARLA_CONTENT_TEST_STATE"] = "partial"
        self.environment["CARLA_CONTENT_TEST_CLONE_EXIT"] = "0"
        self.assert_rejected_without_changes()

    def test_prerequisite_failure_still_stops_before_content_changes(self):
        self.environment["CARLA_CONTENT_TEST_INSTALL_EXIT"] = "23"
        result = self.run_setup("--interactive")
        self.assertEqual(result.returncode, 23, result.stdout + result.stderr)
        self.assertFalse(self.checkout.parent.exists())
        self.assertEqual([name for name, _ in self.commands()], ["bash"])

    def test_missing_git_recorder_fails_before_the_setup_script(self):
        (Path(self.environment["CARLA_CONTENT_TEST_LOG"]).parent / "tools/git").unlink()
        result = self.run_setup()
        self.assertEqual(result.returncode, 90, result.stdout + result.stderr)
        self.assertFalse(self.checkout.parent.exists())
        self.assertEqual(self.commands(), [])

    def link_checkout(self, target):
        self.checkout.parent.mkdir(parents=True)
        try:
            self.checkout.symlink_to(target, target_is_directory=True)
        except OSError as error:
            self.skipTest("Directory symlink creation is unavailable: " + str(error))

    def test_valid_linked_checkout_with_spaced_target_is_reused(self):
        target = self.directory / "Shared content"
        target.mkdir()
        (target / "authored.data").write_bytes(b"linked user content")
        self.link_checkout(target)
        self.environment["CARLA_CONTENT_TEST_STATE"] = "valid"
        self.environment["CARLA_CONTENT_TEST_ROOT"] = str(target)
        self.assert_completed(self.run_setup())
        self.assertEqual(self.clone_commands(), [])
        self.assertEqual((target / "authored.data").read_bytes(), b"linked user content")

    def test_broken_checkout_link_is_preserved_and_rejected(self):
        target = self.directory / "missing target"
        self.link_checkout(target)
        self.assert_rejected_without_changes()
        self.assertEqual(os.readlink(self.checkout), str(target))
        self.assertFalse(target.exists())

    def test_empty_linked_directory_is_not_populated_implicitly(self):
        target = self.directory / "empty shared content"
        target.mkdir()
        self.link_checkout(target)
        self.assert_rejected_without_changes()
        self.assertEqual(list(target.iterdir()), [])

    def test_launch_flag_keeps_existing_build_target(self):
        result = self.run_setup("--skip-prerequisites", "--launch")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        cmake_calls = [args for name, args in self.commands() if name == "cmake"]
        self.assertEqual(cmake_calls[-1], ["--build", "Build", "--target", "launch"])


if __name__ == "__main__":
    unittest.main()
