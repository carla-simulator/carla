#!/usr/bin/env python3

"""
Some example functions for parsing sensor data for control and visualisation. For more information
about sensor data, see: http://carla.readthedocs.io/en/latest/cameras_and_sensors/

Similar C++ code can be found in Util/ImageConverter
"""

from __future__ import print_function
from __future__ import division

import numpy as np
from PIL import Image
import random
import time
import argparse
import os
import shutil

from carla import CARLA
from carla import Control

SINGLE_EPISODE_LENGTH = 20  # this example runs for one episode of specified length

def parse_bgra(measurements):
    """Parses BGRA images from measurements. Returns 4D RGBA array and PIL image"""

    rgba, im = None, None
    if 'BGRA' in measurements:
        rgba = measurements['BGRA'][0][:, :, [2, 1, 0, 3]]  # swap the R and B channels
        im = Image.fromarray(rgba, 'RGBA')
    else:
        print('No BGRA image in measurements')

    return rgba, im


def parse_depth(measurements):
    """Parses depth images from measurements. Returns 2D depth array and PIL image (for visualisation)"""

    scaled_depth, im = None, None
    if 'Depth' in measurements:
        depth = measurements['Depth'][0].astype(np.float32)

        # Decode depth value from colour channels
        depth = (depth[:, :, 2] + depth[:, :, 1]*256 + depth[:, :, 0]*256*256) / (256*256*256 - 1)
        scaled_depth = depth * 1000  # scaled to metres

        # To visualise the depth we take the log and rescale it
        log_depth = np.log(scaled_depth)
        im = Image.fromarray((255 * log_depth / np.max(log_depth)).astype(np.int8), 'L')

    else:
        print('No depth image in measurements')

    return scaled_depth, im


def parse_labels(measurements):
    """Parses labels images from measurements. Returns 2D labels array and PIL image (for visualisation)"""

    labels, im = None, None
    if 'Labels' in measurements:
        labels = measurements['Labels'][0][:, :, 2]  # label info is in the R channel
        seg = np.zeros((labels.shape[0], labels.shape[1], 3))

        # Assign an arbitrary colour for each label
        label_colours = {
            0: [0, 0, 0],         # None
            1: [70, 70, 70],      # Buildings
            2: [190, 153, 153],   # Fences
            3: [250, 170, 160],   # Other
            4: [255, 0, 0],       # Pedestrians
            5: [153, 153, 153],   # Poles
            6: [255, 255, 50],    # Road markings
            7: [128, 64, 128],    # Roads
            8: [244, 35, 232],    # Sidewalks
            9: [107, 142, 35],    # Vegetation
            10: [0, 0, 142],      # Vehicles
            11: [102, 102, 156],  # Walls
            12: [0, 220, 255]     # Traffic Signs
        }
        for i in range(labels.shape[0]):
            for j in range(labels.shape[1]):
                seg[i, j, :] = label_colours[labels[i, j]]

        im = Image.fromarray(seg.astype(np.int8), 'RGB')
    else:
        print('No labels image in measurements')

    return labels, im


def main(ini_file, port=2000, host='127.0.0.1', save_images=True):
    """
    Simple use example for sensor image parsing functions. Runs carla for one episode, saving sensor
    visualisations in the images directory. See carla_example.py for more detailed comments.
    """

    if os.path.exists('example images'):
        shutil.rmtree('example images')  # delete previous images directory
    os.mkdir('example images')

    carla = CARLA(host, port)
    carla.loadConfigurationFile(ini_file)
    carla.newEpisode(0)
    step = 1

    while step <= SINGLE_EPISODE_LENGTH:
        try:
            print('Current step: {}/{}'.format(step, SINGLE_EPISODE_LENGTH))
            measurements = carla.getMeasurements()

            # Parse sensor data and visualisations
            lbls, lbls_im = parse_labels(measurements)
            rgba, rgba_im = parse_bgra(measurements)
            dep, dep_im = parse_depth(measurements)

            # Note: lbls, rgba and dep can now be used for control code

            if save_images:
                lbls_im.save('example images/labels' + str(step) + '.png')
                rgba_im.save('example images/RGBA' + str(step) + '.png')
                dep_im.save('example images/depth' + str(step) + '.png')

            # Random control
            control = Control()
            control.throttle = 0.9
            control.steer = (random.random() * 2) - 1
            carla.sendCommand(control)

            step += 1

        except Exception:
            time.sleep(1)

    carla.stop()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Run the carla client example that connects to a server')
    parser.add_argument('--host', metavar='HOST', type=str, help='host to connect to', default='127.0.0.1')
    parser.add_argument('--port', metavar='PORT', type=int, help='port to connect to', default=2000)
    parser.add_argument("-c", "--config", help="path to ini file", type=str, default="CarlaSettings.ini")
    args = parser.parse_args()

    main(args.config, port=args.port, host=args.host, save_images=True)
