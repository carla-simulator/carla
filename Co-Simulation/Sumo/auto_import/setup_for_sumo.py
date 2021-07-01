import argparse
import os
import sys
import json
from datetime import datetime
from subprocess import run


from util.func import (
    data_from_json,
)



def sumocfg_src(env):
    return '''<?xml version="1.0" encoding="UTF-8"?>
<configuration xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://sumo.dlr.de/xsd/sumoConfiguration.xsd">

    <input>
        <net-file value="''' + env["src_dir"]["net_file_name"] + '''"/>
        <route-files value="''' + env["src_dir"]["trip_file_name"] + '''"/>
        <additional-files value="''' + env["src_dir"]["poly_file_name"] + '''"/>
    </input>

    <processing>
        <ignore-route-errors value="true"/>
    </processing>

    <routing>
        <device.rerouting.adaptation-steps value="180"/>
    </routing>

    <report>
        <verbose value="true"/>
        <duration-log.statistics value="true"/>
        <no-step-log value="true"/>
    </report>
</configuration>'''

def sumocfg_src_without_poly(env):
    return '''<?xml version="1.0" encoding="UTF-8"?>
<configuration xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://sumo.dlr.de/xsd/sumoConfiguration.xsd">

    <input>
        <net-file value="''' + env["src_dir"]["net_file_name"] + '''"/>
        <route-files value="''' + env["src_dir"]["trip_file_name"] + '''"/>
    </input>

    <processing>
        <ignore-route-errors value="true"/>
    </processing>

    <routing>
        <device.rerouting.adaptation-steps value="180"/>
    </routing>

    <report>
        <verbose value="true"/>
        <duration-log.statistics value="true"/>
        <no-step-log value="true"/>
    </report>
</configuration>'''


def import_info(args, env):
    result = env["package2map2infos"][str(args.use_package_name)][str(args.use_map_name)]
    return result


def import_package_dir_func(args):
    return f"{args.carla_import_dir}/{args.use_package_name}"

def import_map_dir_func(args):
    return f"{args.carla_import_dir}/{args.use_package_name}/{args.use_map_name}"

# ----- main -----
def main(args, env):
    import_infos = import_info(args, env)
    import_package_dir = import_package_dir_func(args)
    import_map_dir = import_map_dir_func(args)
    import_xodr_path = f"{import_map_dir}/{import_infos['import_xodr_file_name']}"
    import_fbx_path = f"{import_map_dir}/{import_infos['import_fbx_file_name']}"


    src_dir = f"{env['examples_dir']}/{args.use_map_name}"

    # generate net file from xodr file
    run(f"python3 netconvert_carla.py --guess-tls {import_xodr_path}", shell=True, cwd=os.path.expanduser(args.netconvert_carla_dir))
    run(f"mkdir {src_dir}", shell=True)
    run(f"cp {args.netconvert_carla_dir}/net.net.xml {src_dir}/{env['src_dir']['net_file_name']}", shell=True)
    run(f"cp {import_xodr_path} {src_dir}/{env['src_dir']['xodr_file_name']}", shell=True)

    # generate poly file from net and osm file
    if 'origin_osm_path' in import_infos.keys():
        run(f"cp {import_infos['origin_osm_path']} {src_dir}/{env['src_dir']['osm_file_name']}", shell=True)
        run(f"polyconvert --net-file {env['src_dir']['net_file_name']} --osm-files {env['src_dir']['osm_file_name']} -o {env['src_dir']['poly_file_name']}", shell=True, cwd=os.path.expanduser(src_dir))
    else:
        pass

    # generate trip file
    run(f"python {args.sumo_home_dir}/tools/randomTrips.py --allow-fringe -n {env['src_dir']['net_file_name']} -o {env['src_dir']['trip_file_name']}", shell=True, cwd=os.path.expanduser(src_dir))

    # generate_sumocfg
    with open(f"{src_dir}/{env['src_dir']['sumocfg_file_name']}", mode='w') as f:
        f.write(sumocfg_src(env))


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
    parser.add_argument('--sumo_home_dir', default=env["sumo_home_dir"])

    #  ----- for sumo -----
    args = parser.parse_args()
    main(args, env)
