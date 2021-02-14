# coding: utf-8

import argparse
import glob
import os
import sys
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass
import carla

# import carla

class Osm2Xodr:
    def __init__(self, osm_path, xodr_path, carla_host):
        self.osm_path = osm_path
        self.xodr_path = xodr_path
        self.client = carla.Client(carla_host, 2000)
        self.client.set_timeout(10.0)

    def read_data_from_file(self, file_path):
        file = open(file_path, 'r')
        file_data = file.read()

        file.close()
        return file_data

    def osm_data(self):
        return self.read_data_from_file(self.osm_path)

    def save_data_to_file(self, data, file_path):
        f = open(file_path, 'w')
        f.write(data)
        f.close()

    def save_xodr(self):
        self.save_data_to_file(self.xodr_data(), self.xodr_path)

    def xodr_data(self):
        return carla.Osm2Odr.convert(self.osm_data(), carla.Osm2OdrSettings())

def main(osm_path, xodr_path, carla_host):
    osm_2_xodr = Osm2Xodr(osm_path, xodr_path, carla_host)
    osm_2_xodr.save_xodr()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='This script is a middleware for Carla-Veins synchronization.')
    parser.add_argument('--osm_path')
    parser.add_argument('--xodr_path')
    parser.add_argument('--carla_host', default='127.0.0.1')

    args = parser.parse_args()
    main(args.osm_path, args.xodr_path, args.carla_host)
