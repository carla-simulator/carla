# coding: utf-8

import argparse
import glob
import logging
import os
import sys
import time

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
class TraciHandler:
    def __init__(self, traci, env):
        self.traci = traci


class TracisSyncronizer:
    def __init__(self, main_sumo_host_port, other_sumo_host_ports, order, carla_veins_data_dir):
        self.carla_veins_data_dir = carla_veins_data_dir

        self.main_traci = traci.connect(host=main_sumo_host_port.split(":")[0], port=int(main_sumo_host_port.split(":")[1]))
        self.other_tracis = [traci.connect(host=o_host_port.split(":")[0], port=int(o_host_port.split(":")[1])) for o_host_port in other_sumo_host_ports]

        # ----- set order -----
        self.tracis = [self.main_traci] + self.other_tracis
        for tmp_traci in self.tracis:
            tmp_traci.setOrder(order)

    def start(self, time_to_start):
        for tmp_traci in self.tracis:
            if tmp_traci.simulation.getTime() < time_to_start:
                tmp_traci.simulationStep(time_to_start)
            else:
                pass

        while self.check_sumo_finish() is False:
            self.main_traci.simulationStep()

            self.check_lock(self.carla_veins_data_dir)

            for o_traci in self.other_tracis:
                self.sync_tracis(self.main_traci, o_traci)
                o_traci.simulationStep()


    def check_lock(self, carla_veins_data_dir):
        time_out_thresiold = 10
        total_time = 0
        sleep_time = 0.1


        while True:
            if len(glob.glob(f"{carla_veins_data_dir}/*.lock")) <= 0:
                break
            else:
                time.sleep(sleep_time)

                total_time = total_time + sleep_time
                total_time = total_time if total_time < time_out_thresiold else 0

                if total_time == 0:
                    for tmp_traci in self.tracis:
                        self.__ping_to_traci(tmp_traci)

                else:
                    pass

    def check_sumo_finish(self):
        return False

        # try:
        #     for tmp_traci in self.tracis:
        #         if traci.simulation.getMinExpectedNumber() <= 0:
        #             return True
        #         else:
        #             continue
        #     return False
        #
        # except Exception as e:
        #     logging.log(e)
        #     return True

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
        # diff_add_vehicle_ids = set(m_traci.vehicle.getIDList()) - set(o_traci.vehicle.getIDList())
        # for dav_id in diff_add_vehicle_ids:
        #     new_route_id = str(m_traci.vehicle.getIDCount() + 1)
        #
        #     try:
        #         o_traci.route.add(
        #             routeID=new_route_id,
        #             edges=m_traci.vehicle.getRoute(dav_id)
        #         )
        #         o_traci.vehicle.add(
        #             vehID=dav_id,
        #             routeID=new_route_id
        #         )
        #
        #     except Exception as e:
        #         logging.error(e)
        #         continue

        # ----- remove vehicles -----
        diff_remove_vehicle_ids = set(o_traci.vehicle.getIDList()) - set(m_traci.vehicle.getIDList())
        for drv_id in diff_remove_vehicle_ids:
            try:
                o_traci.vehicle.remove(drv_id)
            except Exception as e:
                logging.error(e)
                continue

        # ----- sync vehicle positions -----
        for m_veh_id in list( set(m_traci.vehicle.getIDList()) & set(o_traci.vehicle.getIDList()) ):
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


    def __ping_to_traci(self, tmp_traci):
        """
        This method is used to avoid traci connection timeout.
        """

        tmp_traci.simulation.getTime()


# ----- function -----
def start_tracis_syncronizer(main_sumo_host_port, other_sumo_host_ports, order, time_to_start, carla_veins_data_dir):
    tracis_syncronizer = TracisSyncronizer(main_sumo_host_port, other_sumo_host_ports, order, carla_veins_data_dir)

    try:
        tracis_syncronizer.start(time_to_start)
    except KeyboardInterrupt:
        logging.info(CTRL_C_PRESSED_MESSAGE)
    except Exception as e:
        logging.error(e)
    finally:
        tracis_syncronizer.close_tracis()

# ----- main -----
if __name__ == "__main__":
    env = data_from_json("./env_for_veins.json")

    # ----- get args -----
    parser = argparse.ArgumentParser(description='This script is a middleware for tracis synchronization.')

    parser.add_argument('--data_dir', default=(env['carla_veins_dynamic_data_dir'] if bool(env['is_dynamic_simulation']) else env['carla_veins_static_data_dir']))
    parser.add_argument('--main_sumo_host_port', default=f"127.0.0.1:{env['carla_sumo_port']}")
    parser.add_argument('--other_sumo_host_ports', nargs='*', default=f"{env['vagrant_ip']}:{env['veins_sumo_port']}")
    parser.add_argument('--sumo_order', type=int, default=1)
    parser.add_argument('--time_to_start', type=float, default=0)
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
        order=args.sumo_order,
        time_to_start=args.time_to_start,
        carla_veins_data_dir=args.data_dir
    )
