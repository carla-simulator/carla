# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB). Licensed under the MIT license.
"""Execute the shared Bash configuration reader against real temporary files."""

import os
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(os.environ.get("CARLA_SKILL_TEST_ROOT", Path(__file__).resolve().parents[3]))
SCRIPT = ROOT / "Skills" / "_common" / "env_common.sh"
GIT = shutil.which("git")
GIT_BASH = Path(GIT).resolve().parents[1] / "bin" / "bash.exe" if GIT else Path("")
BASH = str(GIT_BASH) if os.name == "nt" and GIT_BASH.is_file() else shutil.which("bash")


def _bash_path(path):
    path = Path(path).resolve()
    if os.name == "nt":
        return "/" + path.drive[0].lower() + path.as_posix()[2:]
    return str(path)


@unittest.skipUnless(BASH, "Bash is needed to exercise the Bash configuration reader")
class TestSkillConfiguration(unittest.TestCase):
    def test_preflight_rejects_an_executable_that_fails_to_run(self):
        preflight = ROOT / "Skills/docs/audit-carla-docs/scripts/check_env.sh"
        with tempfile.TemporaryDirectory(prefix="carla-python-probe-") as directory:
            executable = Path(directory) / "broken-python"
            executable.write_text("#!/usr/bin/env bash\necho unavailable >&2\nexit 42\n", encoding="utf-8")
            executable.chmod(0o755)
            environment = os.environ.copy()
            environment["CARLA_ROOT"] = _bash_path(ROOT)
            environment["PYTHON"] = _bash_path(executable)
            environment["CARLA_TOOLS_CONFIG"] = _bash_path(Path(directory) / "missing.env")
            result = subprocess.run(
                [BASH, "--noprofile", "--norc", _bash_path(preflight)],
                cwd=directory, env=environment, capture_output=True, text=True,
                timeout=10, check=False)
            self.assertNotEqual(result.returncode, 0, result.stdout)
            self.assertNotIn("PASS  " + environment["PYTHON"] + " available", result.stdout)

    def _read(self, user=b"", repository=b"", exported=None):
        with tempfile.TemporaryDirectory(prefix="carla-config-") as directory:
            folder = Path(directory)
            config = folder / "user.env"
            config.write_bytes(user)
            (folder / ".carla-tools.env").write_bytes(repository)
            environment = os.environ.copy()
            environment.pop("CARLA_TEST_CONFIG", None)
            environment["CARLA_TOOLS_CONFIG"] = _bash_path(config)
            if exported is not None:
                environment["CARLA_TEST_CONFIG"] = exported
            command = "source {}; printf '%s' \"${{CARLA_TEST_CONFIG-}}\"".format(
                shlex.quote(_bash_path(SCRIPT)))
            return subprocess.run(
                [BASH, "--noprofile", "--norc", "-c", command],
                cwd=directory, env=environment, capture_output=True, text=True,
                timeout=10, check=False)

    def test_repository_configuration_precedes_user_configuration(self):
        result = self._read(b"CARLA_TEST_CONFIG=user\n", b"CARLA_TEST_CONFIG=repository\n")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "repository")

    def test_exported_value_precedes_both_configuration_files(self):
        result = self._read(b"CARLA_TEST_CONFIG=user\n", b"CARLA_TEST_CONFIG=repo\n", "exported")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "exported")

    def test_unterminated_final_line_is_loaded(self):
        result = self._read(b"CARLA_TEST_CONFIG=last line")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "last line")

    def test_crlf_does_not_become_part_of_the_value(self):
        result = self._read(b"CARLA_TEST_CONFIG=windows path\r\n")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "windows path")

    def test_values_are_literal_and_keep_embedded_equals(self):
        value = "literal=$(printf SHOULD_NOT_RUN)=tail"
        result = self._read(("CARLA_TEST_CONFIG=" + value + "\n").encode())
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, value)

    def test_invalid_keys_are_never_evaluated(self):
        result = self._read(
            b"CARLA_TEST_BAD[$(printf${IFS}EVALUATED>&2)]=ignored\n"
            b"CARLA_TEST_CONFIG=safe\n")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertNotIn("EVALUATED", result.stderr)
        self.assertEqual(result.stdout, "safe")


if __name__ == "__main__":
    unittest.main()
