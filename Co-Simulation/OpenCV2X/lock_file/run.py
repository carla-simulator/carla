import glob
import json
import os
import time

def unlock(dst):
    os.unlink(dst)

def lock(src, dst):
    while make_symlink(src, dst) is False:
        continue

def make_symlink(src, dst):
    try:
        os.symlink(src, dst)
        return True
    except:
        return False

if __name__ == "__main__":
    data_sync_dir = "./../carla-veins-data/"
    veh_id = 0
    c = 0

    sensor_data = json.dumps([{"id": veh_id, "position": [1, 2]}])
    sensor_data_file_name = f"{veh_id}_sensor.json"
    sensor_lock_file_name = f"{veh_id}_sensor.json.lock"

    packet_data = json.dumps([{"id": veh_id, "sender": veh_id + 1}])
    packet_data_file_name = f"{veh_id}_packet.json"
    packet_lock_file_name = f"{veh_id}_packet.json.lock"


    while True:
        c = c + 1
        sensor_data = json.dumps([{"id": veh_id, "position": [1, 2], "c": c}])
        packet_data = json.dumps([{"id": veh_id, "sender": veh_id + 1, "c": c}])

        # write sensor data
        time.sleep(1)
        lock(data_sync_dir + sensor_data_file_name, data_sync_dir + sensor_lock_file_name)
        with open(data_sync_dir + sensor_data_file_name, mode="a") as f:
            # print(sensor_data)
            f.write(sensor_data + "\n")
        unlock(data_sync_dir + sensor_lock_file_name)


        # read packets
        lock(data_sync_dir + packet_data_file_name, data_sync_dir + packet_lock_file_name)
        try:
            with open(data_sync_dir + packet_data_file_name) as f:
                print(f.readlines())
        except:
            pass

        with open(data_sync_dir + packet_data_file_name, mode="w") as f:
            f.write("")
        unlock(data_sync_dir + packet_lock_file_name)
