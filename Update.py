from argparse import ArgumentParser
from tarfile import TarFile
from pathlib import Path
import requests
import shutil
import json

workspace_path = Path(__file__).parent.resolve()
content_parent_path = workspace_path / 'Unreal' / 'CarlaUE4' / 'Content'
intermediate_file_path = content_parent_path / f'Carla.tar.gz'
content_path = content_parent_path / 'Carla'
content_version_file_path = workspace_path / 'Util' / 'ContentVersions.json'

if content_path.exists():
    print(f'"{content_path}" already exists.')
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
arg = arg_parser.parse_args()
content_version = arg.content_version
display_progress = arg.display_progress

print ('Creating UE content directories.')
content_parent_path.mkdir(exist_ok = True)
content_path.mkdir(exist_ok = True)

print ('Reading UE content version URLs.')
content_version_map = None
with open(content_version_file_path, 'r') as file:
    content_version_map = json.load(file)

assert content_version_map != None
assert type(content_version_map) is dict

if content_version is None:
    content_version = content_version_map['latest']
    assert content_version in content_version_map
    print(f'No content version was specified, using "latest" ({content_version}).')

version_info = content_version_map[content_version]
identifier = version_info['id']
storage_type = version_info['type']

url = None
if storage_type == 'google-drive':
    url = f'https://drive.google.com/open?id={identifier}'
elif storage_type == 'aws':
    url = f'http://carla-assets.s3.amazonaws.com/{identifier}.tar.gz'
assert url != None

print(f'Downloading "{url}".')

try:
    # TODO: This may not be optimal for our case:
    with requests.Session() as session:
        with session.get(url, stream = True) as result:
            with open(intermediate_file_path, 'wb') as file:
                shutil.copyfileobj(result.raw, file)
except:
    print('Failed to download Carla UE content.')
    exit(-1)
finally:
    if intermediate_file_path.exists():
        assert intermediate_file_path.is_file()
        TarFile.open(intermediate_file_path).extractall(content_path)
        intermediate_file_path.unlink()
