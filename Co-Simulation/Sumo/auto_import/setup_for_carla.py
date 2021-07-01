import argparse
import os
import sys
import json
from datetime import datetime
from subprocess import run


from util.func import (
    data_from_json,
)



# def sumocfg_src(env):
#     return '''<?xml version="1.0" encoding="UTF-8"?>
# <configuration xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://sumo.dlr.de/xsd/sumoConfiguration.xsd">
#
#     <input>
#         <net-file value="''' + env["src_dir"]["net_file_name"] + ''''"/>
#         <route-files value="''' + env["src_dir"]["trip_file_name"] + '''"/>
#         <additional-files value="''' + env["src_dir"]["poly_file_name"] + '''"/>
#     </input>
#
#     <processing>
#         <ignore-route-errors value="true"/>
#     </processing>
#
#     <routing>
#         <device.rerouting.adaptation-steps value="180"/>
#     </routing>
#
#     <report>
#         <verbose value="true"/>
#         <duration-log.statistics value="true"/>
#         <no-step-log value="true"/>
#     </report>
# </configuration>'''


def import_info(args, env):
    result = env["package2map2infos"][str(args.use_package_name)][str(args.use_map_name)]
    return result


def import_package_dir_func(args):
    return f"{args.carla_import_dir}/{args.use_package_name}"

def import_map_dir_func(args):
    return f"{args.carla_import_dir}/{args.use_package_name}/{args.use_map_name}"

# ----- main -----
def setup(args, env):
    import_infos = import_info(args, env)
    import_package_dir = import_package_dir_func(args)
    import_map_dir = import_map_dir_func(args)
    package_json_file_path = f"{import_package_dir}/{args.use_package_name}.json"
    map_info = {
        "source": f"{args.use_map_name}/{import_infos['import_fbx_file_name']}",
        "use_carla_materials": "false",
        "name": args.use_map_name,
        "xodr": f"{args.use_map_name}/{import_infos['import_xodr_file_name']}"
    }


    if os.path.exists(os.path.expanduser(import_package_dir)) is False:
        os.mkdir(os.path.expanduser(import_package_dir))

    if os.path.exists(os.path.expanduser(import_map_dir)) is False:
        os.mkdir(os.path.expanduser(import_map_dir))

    d = {}
    if os.path.exists(os.path.expanduser(package_json_file_path)) is False:
        d = {"maps": [map_info], "props": []}

    else:
        d = data_from_json(os.path.expanduser(package_json_file_path))
        has_same_map = False

        for i in range(0, len(d["maps"])):
            # print(f"json: {d['maps'][i]['name']}, new: {map_info['name']}, bool: {d['maps'][i]['name'] == map_info['name']}")
            if d["maps"][i]["name"] == map_info["name"]:
                d["maps"][i] = map_info
                has_same_map = True
                break

            else:
                continue

        if has_same_map is False:
            d["maps"].append(map_info)

    with open(os.path.expanduser(package_json_file_path), mode='w') as f:
        f.write(json.dumps(d))

def main(args, env):
    import_infos = import_info(args, env)
    import_package_dir = import_package_dir_func(args)
    import_map_dir = import_map_dir_func(args)
    import_xodr_path = f"{import_map_dir}/{import_infos['import_xodr_file_name']}"
    import_fbx_path = f"{import_map_dir}/{import_infos['import_fbx_file_name']}"


    src_dir = f"{env['examples_dir']}/{args.use_map_name}"

    # import_xodr.
    if 'origin_osm_path' in import_infos.keys():
        run(f"cp {import_infos['origin_osm_path']} {args.osm2xodr_dir}/map.osm", shell=True)
        run(f"python3 main.py", shell=True, cwd=args.osm2xodr_dir)
        run(f"cp {args.osm2xodr_dir}/output.xodr {import_xodr_path}", shell=True)
    else: # use origin_fbx_path
        run(f"cp {import_infos['origin_xodr_path']} {import_xodr_path}", shell=True)


    # import_fbx.
    run(f"cp {import_infos['origin_fbx_path']} {import_fbx_path}", shell=True)


if __name__ == "__main__":
    env = data_from_json("./env.json")
    use_package_name = list(env["package2map2infos"].keys())[0]
    use_map_name = list(env["package2map2infos"][use_package_name].keys())[0]

    # ----- get args -----
    parser = argparse.ArgumentParser(description='This script is a middleware for make import.')

    parser.add_argument('--use_package_name', default=use_package_name, choices=env["package2map2infos"].keys())
    parser.add_argument('--use_map_name', default=use_map_name, choices=env["package2map2infos"][use_package_name].keys())

    parser.add_argument('--carla_root_dir', default=env["carla_root_dir"])
    parser.add_argument('--carla_import_dir', default=env["carla_import_dir"])
    parser.add_argument('--netconvert_carla_dir', default=env["netconvert_carla_dir"])
    parser.add_argument('--osm2xodr_dir', default=env["osm2xodr_dir"])

    #  ----- for sumo -----
    args = parser.parse_args()
    setup(args, env)
    main(args, env)
