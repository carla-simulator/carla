from argparse import ArgumentParser
from tarfile import TarFile
from pathlib import Path
import requests
import shutil
import json

WORKSPACE_PATH = Path(__file__).parent.resolve()
CONTENT_PARENT_PATH = WORKSPACE_PATH / 'Unreal' / 'CarlaUE4' / 'Content'
INTERMEDIATE_FILE_PATH = CONTENT_PARENT_PATH / f'Carla.tar.gz'
CONTENT_PATH = CONTENT_PARENT_PATH / 'Carla'
CONTENT_VERSION_FILE_PATH = WORKSPACE_PATH / 'Util' / 'ContentVersions.json'

if CONTENT_PATH.exists():
    print(f'"{CONTENT_PATH}" already exists.')
    exit(-1)

arg_parser = ArgumentParser(description = __doc__)
arg_parser.add_argument(
    '--content-version',
    default = None,
    type = str,
    help = 'Version of the Carla UE Content to download.')
arg_parser.add_argument(
    '--display-progress',
    default = False,
    type = bool,
    help = 'Whether to display download progress.')
ARGV = arg_parser.parse_args()
CONTENT_VERSION = ARGV.content_version
DISPLAY_PROGRESS = ARGV.display_progress

print ('Creating UE content directories.')
CONTENT_PARENT_PATH.mkdir(exist_ok = True)
CONTENT_PATH.mkdir(exist_ok = True)

print ('Reading UE content version URLs.')
content_version_map = None
with open(CONTENT_VERSION_FILE_PATH, 'r') as file:
    content_version_map = json.load(file)

assert content_version_map != None
assert type(content_version_map) is dict

if CONTENT_VERSION is None:
    CONTENT_VERSION = content_version_map['latest']
    assert CONTENT_VERSION in content_version_map
    print(f'No content version was specified, using "latest" ({CONTENT_VERSION}).')

VERSION_INFO = content_version_map[CONTENT_VERSION]
IDENTIFIER = VERSION_INFO['id']
STORAGE_KIND = VERSION_INFO['type']

url = None
if STORAGE_KIND == 'google-drive':
    url = f'https://drive.google.com/open?id={IDENTIFIER}'
elif STORAGE_KIND == 'aws':
    url = f'http://carla-assets.s3.amazonaws.com/{IDENTIFIER}.tar.gz'
assert url != None

print(f'Downloading "{url}".')

try:
    # TODO: This may not be optimal for our case:
    with requests.Session() as session:
        with session.get(url, stream = True) as result:
            with open(INTERMEDIATE_FILE_PATH, 'wb') as file:
                shutil.copyfileobj(result.raw, file)
except:
    print('Failed to download Carla UE content.')
    exit(-1)
finally:
    if INTERMEDIATE_FILE_PATH.exists():
        assert INTERMEDIATE_FILE_PATH.is_file()
        print('Extracting CARLA content.')
        TarFile.open(INTERMEDIATE_FILE_PATH).extractall(CONTENT_PATH)
        INTERMEDIATE_FILE_PATH.unlink()
        print('Done.')
