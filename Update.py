# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
# 
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from pathlib import Path
import subprocess
import os

CARLA_ROOT_PATH = Path(__file__).parent
CARLA_CONTENT_ROOT_PATH = CARLA_ROOT_PATH / 'Unreal' / 'CarlaUnreal' / 'Content'
CARLA_CONTENT_PATH = CARLA_CONTENT_ROOT_PATH / 'Carla'
CARLA_CONTENT_REPO_URL = 'https://bitbucket.org/carla-simulator/carla-content.git'
SHELL_EXT = '.bat' if os.name == 'nt' else '.sh'



def LoadContentVersionMap():
  import json
  content_map = None
  with open(CARLA_ROOT_PATH / 'Util' / 'ContentVersions.json', 'rb') as file:
    content_map = json.load(file)
  return dict(content_map)



def UpdateContent(
    content_version : str = None,
    use_clone : bool = False,
    tag_or_branch : str = None,
    shallow_clone : bool = True) -> None:
  if CARLA_CONTENT_PATH.exists() and (CARLA_CONTENT_PATH / '.git').exists() and not use_clone:
    print('Attempted to download Carla Content, but a previously cloned Content repo was found.')
    return
  if use_clone:
    if CARLA_CONTENT_PATH.exists():
      cmd = [
        'git',
        '-C',
        CARLA_CONTENT_PATH,
        'pull',
        'origin'
      ]
      subprocess.run(cmd).check_returncode()
    else:
      cmd = [
        'git',
        '-C',
        CARLA_CONTENT_ROOT_PATH,
        'clone'
      ]
      if tag_or_branch != None:
        cmd.extend([ '-b', tag_or_branch ])
      if shallow_clone:
        cmd.extend([ '--single-branch', '--depth', '1' ])
      cmd.extend([
        CARLA_CONTENT_REPO_URL,
        'Carla'
      ])
      subprocess.run(cmd).check_returncode()
  else:
    CONTENT_VERSION_MAP = LoadContentVersionMap()
    entry = None
    if content_version == None:
      content_version = CONTENT_VERSION_MAP['latest']
      entry = CONTENT_VERSION_MAP[content_version]
    else:
      content_version = content_version.lower()
      entry = CONTENT_VERSION_MAP[content_version]
    assert entry
    content_source_type = entry['type']
    content_id = entry['id']
    assert any([
      content_source_type == 'aws',
      content_source_type == 'google-drive'
    ])
    CARLA_CONTENT_DOWNLOAD_URL = f'https://carla-assets.s3.us-east-005.backblazeb2.com/{content_id}.tar.gz'
    cmd = [
      CARLA_ROOT_PATH / 'Util' / 'DownloadAndExtractContent' + SHELL_EXT,
      CARLA_CONTENT_DOWNLOAD_URL,
      CARLA_CONTENT_ROOT_PATH / 'CarlaContent.tar.gz',
      CARLA_CONTENT_ROOT_PATH / 'Carla'
    ]
    for e in cmd:
      e = str(e)
    assert subprocess.call(cmd) == 0



# Main entry point, if called directly:

if __name__ == '__main__':
  from argparse import ArgumentParser
  argp = ArgumentParser()
  argp.add_argument(
    '--version',
    help = 'Target content version.',
    type = str,
    default = None
  )
  argp.add_argument(
    '--clone',
    help = 'Enable clone instead of direct download.',
    action = 'store_true'
  )
  argp.add_argument(
    '--tag',
    help = 'Set the Content repo branch or tag when cloning.',
    type = str,
    default = None
  )
  argp.add_argument(
    '--shallow-clone',
    help = 'Enable shadow cloning.',
    action = 'store_true'
  )
  ARGV = argp.parse_args()
  UpdateContent(
    content_version = ARGV.version,
    use_clone = ARGV.clone,
    tag_or_branch = ARGV.tag,
    shallow_clone = ARGV.shallow_clone)
