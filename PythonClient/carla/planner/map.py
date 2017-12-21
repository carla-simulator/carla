# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB), and the INTEL Visual Computing Lab.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Class used for operating the city map."""

import math
import os


try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')

try:
    from PIL import Image
except ImportError:
    raise RuntimeError('cannot import PIL, make sure pillow package is installed')

from carla.planner.graph import Graph
from carla.planner.grid import Grid
from carla.planner.converter import Converter


def color_to_angle(color):
    return ((float(color)/255.0)) * 2*math.pi

class CarlaMap(object):

    def __init__(self, city, pixel_density, node_density):
        dir_path = os.path.dirname(__file__)
        city_file = os.path.join(dir_path, city + '.txt')

        city_map_file = os.path.join(dir_path, city + '.png')
        city_map_file_lanes = os.path.join(dir_path, city + 'Lanes.png')
        city_map_file_center = os.path.join(dir_path, city + 'Central.png')

        # The built graph. This is the exact same graph that unreal builds. This
        # is a generic structure used for many cases
        self._graph = Graph(city_file)

        self._pixel_density = pixel_density
        self._grid = Grid(self._graph, node_density)
        # The number of game units per pixel. For now this is fixed.

        self._converter = Converter(city_file, pixel_density, node_density)



        # Load the lanes image
        self.map_image_lanes = Image.open(city_map_file_lanes)
        self.map_image_lanes.load()
        self.map_image_lanes = np.asarray(self.map_image_lanes, dtype="int32")
        # Load the image
        self.map_image = Image.open(city_map_file)
        self.map_image.load()
        self.map_image = np.asarray(self.map_image, dtype="int32")

        # Load the lanes image
        self.map_image_center = Image.open(city_map_file_center)
        self.map_image_center.load()
        self.map_image_center = np.asarray(self.map_image_center, dtype="int32")


    def get_graph_resolution(self):

        return self._graph._resolution

    def get_map(self, height=None):
        if height is not None:
            img = Image.fromarray(self.map_image.astype(np.uint8))

            aspect_ratio = height/float(self.map_image.shape[0])

            img = img.resize((int(aspect_ratio*self.map_image.shape[1]),height), Image.ANTIALIAS)
            img.load()
            return np.asarray(img, dtype="int32")
        return np.fliplr(self.map_image)

    def get_map_lanes(self, height=None):
        if size is not None:
            img = Image.fromarray(self.map_image_lanes.astype(np.uint8))
            img = img.resize((size[1], size[0]), Image.ANTIALIAS)
            img.load()
            return np.fliplr(np.asarray(img, dtype="int32"))
        return np.fliplr(self.map_image_lanes)



    def get_lane_orientation(self, world):
        """Get the lane orientation of a certain world position."""
        relative_location = []
        pixel = []
        rotation = np.array([world[0], world[1], world[2]])
        rotation = rotation.dot(self.worldrotation)

        relative_location.append(rotation[0] + self.worldoffset[0] - self.mapoffset[0])
        relative_location.append(rotation[1] + self.worldoffset[1] - self.mapoffset[1])
        relative_location.append(rotation[2] + self.worldoffset[2] - self.mapoffset[2])

        pixel.append(math.floor(relative_location[0] / float(self.pixel_density)))
        pixel.append(math.floor(relative_location[1] / float(self.pixel_density)))

        ori = self.map_image_lanes[int(pixel[1]), int(pixel[0]), 2]
        ori = color_to_angle(ori)

        return (-math.cos(ori), -math.sin(ori))
    def convert_to_node(self,input):
        """
        Receives a data type (Can Be Pixel or World )
        :param input: position in some coordinate
        :return: A node object
        """
        return self._converter.convert_to_node(input)

    def convert_to_pixel(self,input):
        """
        Receives a data type (Can Be Pixel or World )
        :param input: position in some coordinate
        :return: A node object
        """
        return self._converter.convert_to_pixel(input)

    def convert_to_world(self,input):
        """
        Receives a data type (Can Be Pixel or World )
        :param input: position in some coordinate
        :return: A node object
        """
        return self._converter.convert_to_world(input)



    def get_walls_directed(self,node_source,source_ori,node_target,target_ori):
        """
        This is the most hacky function. Instead of planning on two ways,
        we basically use a one way road and interrupt the other road by adding
        an artificial wall.

        """

        final_walls = self._grid.get_wall_source(node_source,source_ori,node_target)

        final_walls = final_walls.union(self._grid.get_wall_target(
                node_target, target_ori, node_source))
        return final_walls


    def get_walls(self):

        return self._grid._walls



    def get_distance_closest_node(self, pos):

        distance = []
        for node_iter in self.graph.intersection_nodes():

            distance.append(sldist(node_iter, pos))

        return sorted(distance)[0]


