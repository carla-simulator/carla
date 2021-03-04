# coding: utf-8

import argparse
import logging
import os
import sys

from util.func import (
    data_from_json,
)

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
    import traci
else:
    sys.exit("Please declare environment variable 'SUMO_HOME'")

# ----- GLOBAL_VARS -----
CTRL_C_PRESSED_MESSAGE = "ctrl-c is pressed."

# ----- Class -----
class TracisSyncronizer:
    def __init__(self, main_sumo_host_port, other_sumo_host_ports, order):
        self.main_traci = traci.connect(host=main_sumo_host_port.split(":")[0], port=int(main_sumo_host_port.split(":")[1]))
        self.other_tracis = [traci.connect(host=o_host_port.split(":")[0], port=int(o_host_port.split(":")[1])) for o_host_port in other_sumo_host_ports]

        # ----- set order -----
        self.tracis = [self.main_traci] + self.other_tracis
        for tmp_traci in self.tracis:
            tmp_traci.setOrder(order)

    def start(self):
        while self.check_sumo_finish is not False:
            self.main_traci.simulationStep()
            print(self.main_traci.simulation.getTime())

            for o_traci in self.other_tracis:
                self.sync_tracis(self.main_traci, o_traci)
                o_traci.simulationStep()

    def check_sumo_finish(self):
        try:
            for tmp_traci in self.tracis:
                if traci.simulation.getMinExpectedNumber() <= 0:
                    return True
                else:
                    continue
            return False

        except Exception as e:
            logging.log(e)
            return True

    def close_tracis(self):
        try:
            if 0 < len(self.tracis):
                self.tracis[0].close()
                self.tracis.pop(0)
                return self.close_tracis()

            return self.tracis

        except Exception as e:
            logging.error(e)
            logging.error(f"{len(self.tracis)} tracis are remained.")
            return self.tracis

    def simulationStep(self):
        for tmp_traci in self.tracis:
            tmp_traci.simulationStep()

    def sync_tracis(self, m_traci, o_traci):
        # ----- add departed vehicle -----
        diff_add_vehicle_ids = set(m_traci.vehicle.getIDList()) - set(o_traci.vehicle.getIDList())
        for dav_id in diff_add_vehicle_ids:
            new_route_id = str(m_traci.vehicle.getIDCount() + 1)

            try:
                o_traci.route.add(
                    routeID=new_route_id,
                    edges=m_traci.vehicle.getRoute(dav_id)
                )
                o_traci.vehicle.add(
                    vehID=dav_id,
                    routeID=new_route_id
                )

            except Exception as e:
                logging.error(e)
                continue

        # ----- remove vehicles -----
        diff_remove_vehicle_ids = set(o_traci.vehicle.getIDList()) - set(m_traci.vehicle.getIDList())
        for drv_id in diff_remove_vehicle_ids:
            try:
                o_traci.vehicle.remove(drv_id)
            except Exception as e:
                logging.error(e)
                continue

        # ----- sync vehicle positions -----
        for m_veh_id in m_traci.vehicle.getIDList():
            try:
                o_traci.vehicle.moveToXY(
                    vehID=m_veh_id,
                    edgeID=m_traci.lane.getEdgeID(m_traci.vehicle.getLaneID(m_veh_id)),
                    lane=int(str(m_traci.vehicle.getLaneID(m_veh_id)).split('_')[1]),
                    x=m_traci.vehicle.getPosition(m_veh_id)[0],
                    y=m_traci.vehicle.getPosition(m_veh_id)[1],
                    angle=m_traci.vehicle.getAngle(m_veh_id)
                )
            except Exception as e:
                logging.error(e)
                continue

# ----- function -----
def start_tracis_syncronizer(main_sumo_host_port, other_sumo_host_ports, order):
    tracis_syncronizer = TracisSyncronizer(main_sumo_host_port, other_sumo_host_ports, order)

    try:
        tracis_syncronizer.start()
        tracis_syncronizer.close_tracis()
    except KeyboardInterrupt:
        logging.info(CTRL_C_PRESSED_MESSAGE)
        tracis_syncronizer.close_tracis()
    except Exception as e:
        logging.error(e)
        tracis_syncronizer.close_tracis()

# ----- main -----
if __name__ == "__main__":
    env = data_from_json("./env.json")

    # ----- get args -----
    parser = argparse.ArgumentParser(description='This script is a middleware for tracis synchronization.')

    parser.add_argument('--main_sumo_host_port', default=f"127.0.0.1:{env['carla_sumo_port']}")
    parser.add_argument('--other_sumo_host_ports', nargs='*', default=f"{env['vagrant_ip']}:{env['veins_sumo_port']}")
    parser.add_argument('--sumo_order', type=int, default=1)
    parser.add_argument('--log_file_path', default="./log/tracis_logger.log")

    args = parser.parse_args()

    # ----- set logging -----
    logging.basicConfig(
        handlers=[logging.FileHandler(filename=args.log_file_path), logging.StreamHandler(sys.stdout)],
        format='[%(asctime)s] {%(filename)s:%(lineno)d} %(levelname)s: %(message)s',
        level=logging.DEBUG
    )

    start_tracis_syncronizer(
        main_sumo_host_port=args.main_sumo_host_port,
        other_sumo_host_ports=args.other_sumo_host_ports,
        order=args.sumo_order
    )
