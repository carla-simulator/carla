import argparse
import os
import sys
import json
from datetime import datetime
from subprocess import run


from util.func import (
    data_from_json,
)


def sumocfg_src(net_file_name, trip_file_name, poly_file_name):
    return '''<?xml version="1.0" encoding="UTF-8"?>
<configuration xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://sumo.dlr.de/xsd/sumoConfiguration.xsd">

    <input>
        <net-file value="''' + net_file_name + '''"/>
        <route-files value="''' + trip_file_name + '''"/>
        <additional-files value="''' + poly_file_name + '''"/>
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



def sumocfg_src_without_poly(net_file_name, trip_file_name):
    return '''<?xml version="1.0" encoding="UTF-8"?>
<configuration xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://sumo.dlr.de/xsd/sumoConfiguration.xsd">

    <input>
        <net-file value="''' + net_file_name + '''"/>
        <route-files value="''' + trip_file_name + '''"/>
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

def path_and_name(path):
    return os.path.split(path)[0], os.path.split(path)[1]

if __name__ == "__main__":
    env = data_from_json("./env.json")

    # ----- get args -----
    parser = argparse.ArgumentParser(description='This script is a middleware for make import.')

    parser.add_argument('--sumocfg_file_path', default=env["info"][0]["sumocfg_file_path"])
    parser.add_argument('--poly_file_path', default=env["info"][0]["poly_file_path"])
    parser.add_argument('--net_file_path', default=env["info"][0]["net_file_path"])
    parser.add_argument('--trip_file_path', default=env["info"][0]["trip_file_path"])
    parser.add_argument('--rou_file_path', default=env["info"][0]["rou_file_path"])
    parser.add_argument('--seed', type=int, default=env["info"][0]["seed"])
    parser.add_argument('--sumo_home_dir', default=env["sumo_home_dir"])

    #  ----- for sumo -----
    args = parser.parse_args()

    # vehicles departure every 1/p seconds.
    trip_path, trip_name = path_and_name(args.trip_file_path)
    rou_path, rou_name = path_and_name(args.rou_file_path)
    poly_path, poly_name = path_and_name(args.poly_file_path)
    net_path, net_name = path_and_name(args.net_file_path)
    cfg_path, cfg_name = path_and_name(args.sumocfg_file_path)
    for p in range(1, 10):
        # cite from: https://sumo.dlr.de/docs/FAQ.html#How_do_I_get_high_flows.2Fvehicle_densities.3F
        # cite from: https://sumo.dlr.de/docs/Definition_of_Vehicles%2C_Vehicle_Types%2C_and_Routes.html#arrivallane
        new_trip_name = f"{trip_name.split('.')[0]}_p_{p}.trip.xml"
        new_rou_name = f"{trip_name.split('.')[0]}_p_{p}.rou.xml"
        run(f"python {args.sumo_home_dir}/tools/randomTrips.py --validate --allow-fringe --seed {args.seed} -n {args.net_file_path} -o {trip_path}/{new_trip_name} -r {rou_path}/{new_rou_name} -p {1.0/p} --trip-attributes=\" departLane=\\\"best\\\" departSpeed=\\\"{0}\\\" departPos=\\\"random_free\\\" \" ", shell=True)
        # run(f"python {args.sumo_home_dir}/tools/randomTrips.py --seed {args.seed} -n {args.net_file_path} -o {trip_path}/{new_trip_name} -r {rou_path}/{new_rou_name} -p {1.0/p} --trip-attributes=\" departLane=\\\"best\\\" departSpeed=\\\"{0}\\\" departPos=\\\"random_free\\\" \" ", shell=True)

        with open(f"{cfg_path}/{cfg_name.split('.')[0]}_p_{p}.sumocfg", mode='w') as f:
            if os.path.exists(args.poly_file_path):
                f.write(sumocfg_src(net_name, new_rou_name, poly_name))
            else:
                f.write(sumocfg_src_without_poly(net_name, new_rou_name))
