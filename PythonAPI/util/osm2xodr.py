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

    def save_xodr(self, args):
        self.save_data_to_file(self.xodr_data(args), self.xodr_path)

    def xodr_data(self, args):
        setting = carla.Osm2OdrSettings()

        setting.use_offsets = args.use_offsets
        setting.offset_x = args.offset_x
        setting.offset_y = args.offset_y

        return carla.Osm2Odr.convert(
            self.osm_data(),
            setting
        )

def main(osm_path, xodr_path, carla_host, args):
    osm_2_xodr = Osm2Xodr(osm_path, xodr_path, carla_host)
    osm_2_xodr.save_xodr(args)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='This script is a middleware for Carla-Veins synchronization.')
    parser.add_argument('--osm_path')
    parser.add_argument('--xodr_path')
    parser.add_argument('--carla_host', default='127.0.0.1')

    parser.add_argument('--use_offsets', action='store_true')
    parser.add_argument('--offset_x', type=float, default=0.0)
    parser.add_argument('--offset_y', type=float, default=0.0)

    args = parser.parse_args()
    main(args.osm_path, args.xodr_path, args.carla_host, args)
