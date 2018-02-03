# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import math
import numpy as np

from carla.planner.graph import string_to_floats

# Constant definition enumeration

PIXEL = 0
WORLD = 1
NODE = 2


class Converter(object):

    def __init__(self, city_file,  pixel_density, node_density):

        self._node_density = node_density
        self._pixel_density = pixel_density
        with open(city_file, 'r') as f:
            # The offset of the world from the zero coordinates ( The
            # coordinate we consider zero)
            self._worldoffset = string_to_floats(f.readline())

            angles = string_to_floats(f.readline())

            # If there is an rotation between the world and map coordinates.
            self._worldrotation = np.array([
                [math.cos(math.radians(angles[2])), -math.sin(math.radians(angles[2])), 0.0],
                [math.sin(math.radians(angles[2])), math.cos(math.radians(angles[2])), 0.0],
                [0.0, 0.0, 1.0]])

            # Ignore for now, these are offsets for map coordinates and scale
            # (not used).
            _ = f.readline()

            # The offset of the map zero coordinate.
            self._mapoffset = string_to_floats(f.readline())

    def convert_to_node(self, input_data):
        """
        Receives a data type (Can Be Pixel or World )
        :param input_data: position in some coordinate
        :return: A vector representing a node
        """

        input_type = self._check_input_type(input_data)
        if input_type == PIXEL:
            return self._pixel_to_node(input_data)
        elif input_type == WORLD:
            return self._world_to_node(input_data)
        else:
            raise ValueError('Invalid node to be converted')

    def convert_to_pixel(self, input_data):

        """
        Receives a data type (Can Be Node or World )
        :param input_data: position in some coordinate
        :return: A vector with pixel coordinates
        """

        input_type = self._check_input_type(input_data)

        if input_type == NODE:
            return self._node_to_pixel(input_data)
        elif input_type == WORLD:
            return self._world_to_pixel(input_data)
        else:
            raise ValueError('Invalid node to be converted')

    def convert_to_world(self, input_data):

        """
        Receives a data type (Can Be Pixel or Node )
        :param input_data: position in some coordinate
        :return: vector with world coordinates
        """

        input_type = self._check_input_type(input_data)
        if input_type == NODE:
            return self._node_to_world(input_data)
        elif input_type == PIXEL:
            return self._pixel_to_world(input_data)
        else:
            raise ValueError('Invalid node to be converted')

    def _node_to_pixel(self, node):
        """
        Conversion from node format (graph) to pixel (image)
        :param node:
        :return: pixel
        """
        pixel = [((node[0] + 2) * self._node_density)
            , ((node[1] + 2) * self._node_density)]
        return pixel

    def _pixel_to_node(self, pixel):
        """
        Conversion from pixel format (image) to node (graph)
        :param node:
        :return: pixel
        """
        node = [int(((pixel[0]) / self._node_density) - 2)
            , int(((pixel[1]) / self._node_density) - 2)]

        return tuple(node)

    def _pixel_to_world(self, pixel):
        """
        Conversion from pixel format (image) to world (3D)
        :param pixel:
        :return: world
        """

        relative_location = [pixel[0] * self._pixel_density,
                             pixel[1] * self._pixel_density]

        world = [
            relative_location[0] + self._mapoffset[0] - self._worldoffset[0],
            relative_location[1] + self._mapoffset[1] - self._worldoffset[1],
            22
        ]

        return world

    def _world_to_pixel(self, world):
        """
        Conversion from world format (3D) to pixel
        :param world:
        :return: pixel
        """

        rotation = np.array([world[0], world[1], world[2]])
        rotation = rotation.dot(self._worldrotation)

        relative_location = [rotation[0] + self._worldoffset[0] - self._mapoffset[0],
                             rotation[1] + self._worldoffset[1] - self._mapoffset[1],
                             rotation[2] + self._worldoffset[2] - self._mapoffset[2]]



        pixel = [math.floor(relative_location[0] / float(self._pixel_density)),
                 math.floor(relative_location[1] / float(self._pixel_density))]

        return pixel

    def _world_to_node(self, world):
        return self._pixel_to_node(self._world_to_pixel(world))

    def _node_to_world(self, node):

        return self._pixel_to_world(self._node_to_pixel(node))

    def _check_input_type(self, input_data):
        if len(input_data) > 2:
            return WORLD
        elif type(input_data[0]) is int:
            return NODE
        else:
            return PIXEL
