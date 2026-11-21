#!/usr/bin/env python3
# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Build private sensor producers around an existing Linux Python API target."""

import argparse
import hashlib
import json
from pathlib import Path
import shlex
import subprocess
import sys


def digest(path):
    """Record immutable production inputs before and after the fixture build."""
    return hashlib.sha256(path.read_bytes()).hexdigest()


def sensor_command(build):
    """Find the actual production sensor-binding translation unit and flags."""
    entries = json.loads((build / 'compile_commands.json').read_text(encoding='utf-8'))
    return next(entry for entry in entries if entry['file'].endswith('/SensorData.cpp'))


def compile_command(build, source, wrapper, output_object):
    """Reuse configured flags while sending all new outputs to the fixture."""
    entry = sensor_command(build)
    command = entry.get('arguments') or shlex.split(entry['command'])
    original_object = (build / command[command.index('-o') + 1]).resolve()
    command[command.index('-c') + 1] = str(wrapper)
    command[command.index('-o') + 1] = str(output_object)
    command.insert(1, '-I' + str(source.parent))
    for option in ('-MF', '-MT', '-MQ'):
        if option in command:
            index = command.index(option)
            del command[index:index + 2]
    command = [argument for argument in command if argument not in ('-MD', '-MMD')]
    return command, original_object


def link_command(build, output_module, output_object):
    """Replace exactly one object in the existing native Python module link."""
    commands = subprocess.check_output(['ninja', '-C', str(build), '-t', 'commands', 'carla'], text=True)
    command = shlex.split(commands.splitlines()[-1])
    command = command[2:] if command[:2] == [':', '&&'] else command
    command = command[:command.index('&&')] if '&&' in command else command
    original_module = (build / command[command.index('-o') + 1]).resolve()
    matches = [argument for argument in command if argument.endswith('/SensorData.cpp.o')]
    if len(matches) != 1:
        raise RuntimeError('Expected exactly one production SensorData object')
    command = [str(output_object) if argument == matches[0] else str(build / argument)
               if argument.endswith('.o') and not Path(argument).is_absolute() else argument
               for argument in command]
    command[command.index('-o') + 1] = str(output_module / original_module.name)
    return command, original_module


def write_wrapper(source, output):
    """Include production code unchanged, then register test-only native factories."""
    wrapper = output / 'SensorDataFixture.cpp'
    factory = Path(__file__).resolve().with_name('sensor_data_test_factory.inc')
    wrapper.write_text(
        '#define export_sensor_data export_sensor_data_production\n'
        f'#include "{source.as_posix()}"\n'
        '#undef export_sensor_data\n'
        f'#include "{factory.as_posix()}"\n'
        'void export_sensor_data() {\n'
        '  export_sensor_data_production();\n'
        '  RegisterSensorDataTestFactories();\n'
        '}\n', encoding='utf-8')
    return wrapper


def build_fixture(build, source, output):
    """Compile and link an isolated component; preserve configured source/caches."""
    output.mkdir(parents=True, exist_ok=True)
    wrapper = write_wrapper(source, output)
    command, original_object = compile_command(build, source, wrapper, output / 'SensorDataFixture.cpp.o')
    link, original_module = link_command(build, output, output / 'SensorDataFixture.cpp.o')
    if output == build or output / original_module.name == original_module:
        raise ValueError('The fixture output must differ from the production binding build')
    guards = {path: digest(path) for path in (source, original_object, original_module)}
    with (output / 'build.log').open('w', encoding='utf-8') as log:
        for arguments in (command, link):
            log.write(shlex.join(arguments) + '\n')
            log.flush()
            subprocess.run(arguments, cwd=build, stdout=log, stderr=subprocess.STDOUT, check=True)
    if any(digest(path) != before for path, before in guards.items()):
        raise RuntimeError('A production input changed during the fixture build')
    report = {'source': str(source), 'source_sha256': digest(source), 'binding_build': str(build),
              'output_module': str(output / original_module.name),
              'output_sha256': digest(output / original_module.name), 'protected_inputs_unchanged': True,
              'scope': 'Production sensor source plus private native producer factories; other binding objects '
                       'and configured libraries unchanged. A component extension, not a full production build.'}
    (output / 'provenance.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(report, indent=2))


def main():
    """Require an existing persistent Linux backend build and isolated output."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binding-build', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--source', type=Path, help='Explicit source overlay; defaults to configured production source')
    arguments = parser.parse_args()
    if not sys.platform.startswith('linux'):
        parser.error('This configured-command fixture builder currently supports Linux; core C API tests are portable')
    build = arguments.binding_build.resolve()
    source = (arguments.source or Path(sensor_command(build)['file'])).resolve()
    build_fixture(build, source, arguments.output.resolve())


if __name__ == '__main__':
    main()
