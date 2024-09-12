from pathlib import Path
import subprocess, shutil, sys, os

CARLA_WORKSPACE_PATH = Path(__file__).parent.parent
OUTPUT_PATH = CARLA_WORKSPACE_PATH / 'GraphViz'
SYSTEM_NAME = 'Windows' if os.name == 'nt' else 'Linux'
CONFIGURATION = 'Development'

if OUTPUT_PATH.exists():
    shutil.rmtree(OUTPUT_PATH)
OUTPUT_PATH.mkdir(exist_ok = True)

subprocess.run([
    'cmake',
    '--preset',
    f'{SYSTEM_NAME}-{CONFIGURATION}',
    f'--graphviz={OUTPUT_PATH / "CARLA.dot"}',
]).check_returncode()

try:
    subprocess.run([
        'dot',
        '-Tpng',
        '-o',
        'CARLA.png',
        'CARLA.dot'
    ],
    cwd = OUTPUT_PATH).check_returncode()
except:
    print(f'Failed to generate PNG image from CARLA.dot.')
