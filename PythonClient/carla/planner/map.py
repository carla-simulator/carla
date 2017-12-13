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
from carla.planner.graph import  string_to_node,string_to_floats


def color_to_angle(color):
    return ((float(color)/255.0)) * 2*math.pi




"""

        with open(city_file + '.txt', 'r') as file:

            linewordloffset = file.readline()
            # The offset of the world from the zero coordinates ( The
            # coordinate we consider zero)
            self.worldoffset = string_to_floats(linewordloffset)

            # WARNING: for now just considering the y angle
            lineworldangles = file.readline()
            self.angles = string_to_floats(lineworldangles)
            #self.worldrotation = np.array([[math.cos(math.radians(self.angles[0])),0,math.sin(math.radians(self.angles[0])) ],[0,1,0],[-math.sin(math.radians(self.angles[0])),0,math.cos(math.radians(self.angles[0]))]])

            self.worldrotation = np.array([[math.cos(math.radians(self.angles[2])), -math.sin(math.radians(self.angles[2])), 0.0], [
                                          math.sin(math.radians(self.angles[2])), math.cos(math.radians(self.angles[2])), 0.0], [0.0, 0.0, 1.0]])

            # Ignore for now
            lineworscale = file.readline()

            linemapoffset = file.readline()

            # The offset of the map zero coordinate
            self.mapoffset = string_to_floats(linemapoffset)

            # the graph resolution.
            linegraphres = file.readline()
            self.resolution = string_to_node(linegraphres)

"""

# The map contains a graph


class CarlaMap(object):
    def __init__(self, city):
        dir_path = os.path.dirname(__file__)
        city_file = os.path.join(dir_path, city + '.txt')

        city_map_file = os.path.join(dir_path, city + '.png')
        city_map_file_lanes = os.path.join(dir_path, city + 'Lanes.png')
        city_map_file_center = os.path.join(dir_path, city + 'Central.png')


        # The built graph. This is the exact same graph that unreal builds. This
        # is a generic structure used for many cases

        self._graph = Graph(city_file)

        self._grid = Grid(self._graph)

        with open(city_file, 'r') as file:

            linewordloffset = file.readline()
            # The offset of the world from the zero coordinates ( The
            # coordinate we consider zero)
            self.worldoffset = string_to_floats(linewordloffset)

            lineworldangles = file.readline()
            self.angles = string_to_floats(lineworldangles)

            # If tere is an rotation between the world and map coordinates.
            self.worldrotation = np.array([
                [math.cos(math.radians(self.angles[2])), -math.sin(math.radians(self.angles[2])), 0.0],
                [math.sin(math.radians(self.angles[2])), math.cos(math.radians(self.angles[2])), 0.0],
                [0.0, 0.0, 1.0]])

            # Ignore for now, these are offsets for map coordinates and scale
            # (not used).
            map_scale = file.readline()
            linemapoffset = file.readline()

            # The offset of the map zero coordinate.
            self.mapoffset = string_to_floats(linemapoffset)


        # The number of game units per pixel. For now this is fixed.
        self.pixel_density = 16.43
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

    def get_position_on_map(self, world):
        """Get the position on the map for a certain world position."""
        relative_location = []
        pixel = []

        rotation = np.array([world[0], world[1], world[2]])
        rotation = rotation.dot(self.worldrotation)

        relative_location.append(rotation[0] + self.worldoffset[0] - self.mapoffset[0])
        relative_location.append(rotation[1] + self.worldoffset[1] - self.mapoffset[1])
        relative_location.append(rotation[2] + self.worldoffset[2] - self.mapoffset[2])

        pixel.append(math.floor(relative_location[0] / float(self.pixel_density)))
        pixel.append(math.floor(relative_location[1] / float(self.pixel_density)))

        return pixel

    def get_position_on_graph(self, world):
        """Get the position on the map for a certain world position."""
        pixel = self.get_position_on_map(world)
        return self._graph.project_pixel(pixel)



    def get_position_on_world(self, pixel):
        """Get world position of a certain map position."""
        relative_location = []
        world_vertex = []
        relative_location.append(pixel[0] * self.pixel_density)
        relative_location.append(pixel[1] * self.pixel_density)

        world_vertex.append(relative_location[0] + self.mapoffset[0] - self.worldoffset[0])
        world_vertex.append(relative_location[1] + self.mapoffset[1] - self.worldoffset[1])
        world_vertex.append(22)  # Z does not matter for now.

        return world_vertex

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



    def make_map_world(self, world):

        relative_location = []
        pixel = []

        rotation = np.array([world[0], world[1], world[2]])
        rotation = rotation.dot(self.worldrotation)

        relative_location.append(
            rotation[0] + self.worldoffset[0] - self.mapoffset[0])
        relative_location.append(
            rotation[1] + self.worldoffset[1] - self.mapoffset[1])
        relative_location.append(
            rotation[2] + self.worldoffset[2] - self.mapoffset[2])

        pixel.append(math.floor(relative_location[
                     0]/float(self.pixel_density)))
        pixel.append(math.floor(relative_location[
                     1]/float(self.pixel_density)))

        return pixel

    def make_map_node(self, node):
        pixel = []
        pixel.append((node[0] + 2) * self.node_density)
        pixel.append((node[1] + 2) * self.node_density)

        return pixel

    def make_world_map(self, pixel):

        relative_location = []
        world_vertex = []
        relative_location.append(pixel[0]*self.pixel_density)
        relative_location.append(pixel[1]*self.pixel_density)

        world_vertex.append(
            relative_location[0]+self.mapoffset[0] - self.worldoffset[0])
        world_vertex.append(
            relative_location[1]+self.mapoffset[1] - self.worldoffset[1])
        world_vertex.append(22)
        return world_vertex

    def make_world_node(self, node):

        return self.make_world_map(self.make_map_node(node))



    def get_target_ori(self, target_pos):

        relative_location = []
        pixel = []
        rotation = np.array([target_pos[0], target_pos[1], target_pos[2]])
        rotation = rotation.dot(self.worldrotation)

        # print 'rot ', rotation

        relative_location.append(
            rotation[0] + self.worldoffset[0] - self.mapoffset[0])
        relative_location.append(
            rotation[1] + self.worldoffset[1] - self.mapoffset[1])
        relative_location.append(
            rotation[2] + self.worldoffset[2] - self.mapoffset[2])
        # print 'trans ', relative_location

        pixel.append(math.floor(relative_location[
                     0]/float(self.pixel_density)))
        pixel.append(math.floor(relative_location[
                     1]/float(self.pixel_density)))
        # print self.map_image.shape
        ori = self.map_image[int(pixel[1]), int(pixel[0]), 2]
        ori = color_to_angle(ori)


        return (-math.cos(ori), -math.sin(ori))


    def get_walls_directed(self,node_source,source_ori,node_target,target_ori):
        """ 
        This is the most hacky function. Instead of planning on two ways,
        we basically use a one way road and interrupt the other road by adding
        an artificial wall.

        """

        np.set_printoptions( linewidth =206,threshold=np.nan)
        
        print self._grid._structure

        final_walls = self._grid.get_wall_source(node_source,source_ori,node_target)
        #print 'Returned final ',final_walls
        final_walls = final_walls.union(self._grid.get_wall_target(
                node_target,target_ori,node_source))
        return final_walls


    def get_walls(self):


        return self._grid.walls



    def get_distance_closest_node(self, pos):
        import collections
        distance = []
        for node_iter in self.graph.intersection_nodes():

            distance.append(sldist(node_iter, pos))

        return sorted(distance)[0]


