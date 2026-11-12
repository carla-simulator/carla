# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Check a running primary server with a pedestrian mesh in Saved/Nav.

Load a pre-existing map that uses client-side walker navigation and pass a
local copy of its Saved/Nav/<MapName>.bin. To exercise shadowing, leave the
bundled mesh in place as well.
"""

import argparse
from pathlib import Path
from tempfile import TemporaryDirectory

import carla


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("saved_mesh", type=Path)
    parser.add_argument("--host", default="localhost")
    parser.add_argument("--port", type=int, default=2000)
    args = parser.parse_args()

    expected = args.saved_mesh.read_bytes()
    assert expected, "Supply a non-empty pedestrian navigation mesh"
    client = carla.Client(args.host, args.port)
    client.set_timeout(30.0)
    with TemporaryDirectory() as cache:
        client.set_files_base_folder(cache)
        world = client.get_world()
        map_name = world.get_map().name.rsplit("/", 1)[-1]
        assert map_name != "OpenDriveMap" and args.saved_mesh.name == map_name + ".bin", map_name
        alias = "Carla/Maps/Nav/" + args.saved_mesh.name
        for folder in ("Nav", "Nav/"):
            files = client.get_required_files(folder, False)
            assert files[:1] == [alias] and files.count(alias) == 1, (folder, files)
        assert client.get_required_files("", False).count(alias) == 1
        assert alias not in client.get_required_files("OpenDrive", False)
        assert world.get_random_location_from_navigation() is not None, "No walkable location"
        downloaded = Path(cache) / client.get_client_version() / alias
        assert downloaded.read_bytes() == expected, "Downloaded mesh differs from Saved/Nav"


if __name__ == "__main__":
    main()
