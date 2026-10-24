# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Exercise build.sh credential forwarding without Docker, network or builds."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


class TestMonolithCredentials(unittest.TestCase):
    def setUp(self):
        self.script = Path(__file__).resolve().parents[1] / 'build.sh'
        self.bash = os.environ.get('BASH_EXE') or shutil.which('bash')
        if not self.bash:
            self.skipTest('bash is required')
        self.temp_root = Path(os.environ.get('CARLA_DOCKER_TEST_TMPDIR', tempfile.gettempdir())).resolve()
        self.temp_root.mkdir(parents=True, exist_ok=True)
        self.fixture = Path(tempfile.mkdtemp(prefix='carla-docker-test-', dir=self.temp_root)).resolve()
        self.addCleanup(self.clean_fixture)
        self.docker_dir = self.fixture / 'Util/Docker'
        self.docker_dir.mkdir(parents=True)
        shutil.copyfile(self.script, self.docker_dir / 'build.sh')
        for directory in ('examples', 'util'):
            folder = self.fixture / 'PythonAPI' / directory
            folder.mkdir(parents=True)
            (folder / 'requirements.txt').write_text('# test fixture\n')
        self.bin = self.fixture / 'bin'
        self.bin.mkdir()
        self.log = self.fixture / 'docker.log'
        # This recorder accepts exactly the two expected build invocations.
        # It never delegates to another executable or logs credential values.
        self.write_executable('docker', r'''#!/bin/bash
set -eu
[[ "$1" == build ]] || exit 90
shift
target=base
user_secret=false
token_secret=false
for ((i=1; i<=$#; i++)); do
  value="${!i}"
  [[ "$value" != *CARLA_TEST_DUMMY_* ]] || exit 91
  if [[ "$value" == --target ]]; then
    ((i+=1))
    target="${!i}"
  elif [[ "$value" == --secret ]]; then
    ((i+=1))
    case "${!i}" in
      id=epic_user,env=EPIC_USER) user_secret=true ;;
      id=epic_token,env=EPIC_TOKEN) token_secret=true ;;
      *) exit 92 ;;
    esac
  fi
done
case "$target" in
  base)
    [[ "$user_secret" == false && "$token_secret" == false ]] || exit 93
    printf 'base\n' >> "$CARLA_DOCKER_TEST_LOG"
    ;;
  monolith)
    [[ "$user_secret" == true && "$token_secret" == true ]] || exit 94
    [[ "${EPIC_USER:-}" == CARLA_TEST_DUMMY_USER && "${EPIC_TOKEN:-}" == CARLA_TEST_DUMMY_TOKEN ]] || exit 86
    printf 'monolith-secrets-available\n' >> "$CARLA_DOCKER_TEST_LOG"
    ;;
  *) exit 95 ;;
esac
''')
        self.write_executable('getent', '#!/bin/bash\n[[ "$*" == "group docker" ]] || exit 96\nprintf "docker:x:998:\\n"\n')

    def write_executable(self, name, text):
        path = self.bin / name
        with path.open('w', encoding='utf-8', newline='\n') as stream:
            stream.write(text)
        path.chmod(0o755)

    def clean_fixture(self):
        resolved = self.fixture.resolve()
        assert resolved.parent == self.temp_root and resolved.name.startswith('carla-docker-test-')
        shutil.rmtree(resolved)

    def run_script(self, *args, credentials=None, dotenv=False):
        env = {k:v for k,v in os.environ.items()
               if not k.startswith(('EPIC_', 'BASH_FUNC_')) and k not in ('BASH_ENV', 'ENV')}
        env['PATH'] = str(self.bin) + os.pathsep + env.get('PATH', '')
        env['CARLA_DOCKER_TEST_LOG'] = self.log.as_posix()
        env['CARLA_DOCKER_TEST_RECORDER'] = (self.bin / 'docker').as_posix()
        if credentials:
            env.update(EPIC_USER='CARLA_TEST_DUMMY_USER', EPIC_TOKEN='CARLA_TEST_DUMMY_TOKEN')
        if dotenv:
            (self.fixture / '.env').write_text('EPIC_USER=CARLA_TEST_DUMMY_USER\nEPIC_TOKEN=CARLA_TEST_DUMMY_TOKEN\n')
        # Fail closed before executing build.sh if PATH does not select our recorder.
        guard = '[[ $(command -v docker) -ef "$CARLA_DOCKER_TEST_RECORDER" ]] || exit 97; exec bash "$@"'
        return subprocess.run([self.bash, '-c', guard, 'fixture',
                               str(self.docker_dir / 'build.sh'), *args],
                              cwd=self.fixture, env=env, capture_output=True, text=True, timeout=60)

    def assert_monolith_success(self, result):
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(self.log.read_text().splitlines(), ['base', 'monolith-secrets-available'])

    def test_cli_credentials(self):
        self.assert_monolith_success(self.run_script(
            '--monolith', '--epic-user=CARLA_TEST_DUMMY_USER', '--epic-token=CARLA_TEST_DUMMY_TOKEN'))

    def test_inherited_environment_credentials(self):
        self.assert_monolith_success(self.run_script('--monolith', credentials=True))

    def test_cli_credentials_as_separate_arguments(self):
        self.assert_monolith_success(self.run_script(
            '--monolith', '--epic-user', 'CARLA_TEST_DUMMY_USER',
            '--epic-token', 'CARLA_TEST_DUMMY_TOKEN'))

    def test_dotenv_credentials(self):
        self.assert_monolith_success(self.run_script('--monolith', dotenv=True))

    def test_missing_credentials_fail_before_monolith(self):
        result = self.run_script('--monolith')
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('Missing EPIC credentials', result.stdout)
        self.assertEqual(self.log.read_text().splitlines(), ['base'])

    def test_base_build_needs_no_credentials(self):
        result = self.run_script('--base')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(self.log.read_text().splitlines(), ['base'])


if __name__ == '__main__':
    unittest.main()
