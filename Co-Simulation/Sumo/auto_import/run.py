import argparse
from subprocess import run
from subprocess import Popen

# polyconvert --net-file net.net.xml --osm-files ~/gitlab/osm2xodr/map.osm  -o converted.poly.xml

# ----- main -----
def main(args, env):
    # generate xodr from osm.
    pass

    # import xodr and fbx into carla.
    pass

    # generate net file

if __name__ == "__main__":
    env = data_from_json("./env.json")

    # ----- get args -----
    parser = argparse.ArgumentParser(description='This script is a middleware for make import.')

    parser.add_argument('--osm_path', default=env["osm_path"])
    parser.add_argument('--fbx_path', default=env["fbx_path"])

    #  ----- for calra -----
    parser.add_argument('--import_xodr_path', default=env["import_xodr_path"])
    parser.add_argument('--import_fbx_path', default=env["import_fbx_path"])

    #  ----- for sumo -----

    args = parser.parse_args()
    main(args, env)
