#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Helper functions to read and fix broken OpenDRIVE files.
"""

import glob
import os
import sys
from enum import Enum
import xml.etree.ElementTree as ET
from xml.dom import minidom
import argparse

import math

# OpenDRIVE classes and functions
class Geometry(object):
    def __init__(self, geom_xodr):
        self.geom_xodr = geom_xodr
    def get_x(self):
        return float(self.geom_xodr.attrib['x'])
    def set_x(self, x):
        self.geom_xodr.attrib['x'] = str(x)
    def get_y(self):
        return float(self.geom_xodr.attrib['y'])
    def set_y(self, y):
        self.geom_xodr.attrib['y'] = str(y)

class Lane(object):
    def __init__(self, lane_xodr):
        self.lane_xodr = lane_xodr

    def get_id(self):
        return int(self.lane_xodr.attrib['id'])

class LaneSection(object):
    def __init__(self, lane_section_xodr):
        self.lane_section_xodr = lane_section_xodr
        left_xodr = self.lane_section_xodr.find('left')
        lane_dict = {}
        if left_xodr != None:
            for lane_xodr in left_xodr.findall('lane'):
                lane = Lane(lane_xodr)
                lane_dict[lane.get_id()] = lane
        right_xodr = self.lane_section_xodr.find('right')
        if right_xodr != None:
            for lane_xodr in right_xodr.findall('lane'):
                lane = Lane(lane_xodr)
                lane_dict[lane.get_id()] = lane
        center_xodr = self.lane_section_xodr.find('center')
        if center_xodr != None:
          for lane_xodr in center_xodr.findall('lane'):
              lane = Lane(lane_xodr)
              lane_dict[lane.get_id()] = lane
        self.lanes = lane_dict

    def get_s(self):
        return float(self.lane_section_xodr.attrib['s'])
    def get_lanes(self):
        return self.lanes

class RoadLink(object):
    def __init__(self, link_xodr):
        self.link_xodr = link_xodr

    def get_elementType(self):
        return int(self.link_xodr.attrib['elementType'])
    def set_elementType(self, elementType):
        self.link_xodr.attrib['elementType'] = str(elementType)

    def get_elementId(self):
        return int(self.link_xodr.attrib['elementId'])
    def set_elementId(self, elementId):
        self.link_xodr.attrib['elementId'] = str(elementId)

    def get_contactPoint(self):
        if 'contactPoint' in self.link_xodr.attrib:
            return self.link_xodr.attrib['contactPoint']
        else:
            return None
    def set_contactPoint(self, contactPoint):
        self.link_xodr.attrib['contactPoint'] = contactPoint

class Road(object):
    def __init__(self, road_xodr):
        self.road_xodr = road_xodr

    def get_id(self):
        return int(self.road_xodr.attrib['id'])
    def get_predecessor(self):
        link_xodr = self.road_xodr.find('link')
        return RoadLink(link_xodr.find('predecessor'))
    def get_successor(self):
        link_xodr = self.road_xodr.find('link')
        return RoadLink(link_xodr.find('predecessor'))
    def get_lane_sections(self):
        lanes_xodr = self.road_xodr.find('lanes')
        lane_sections = []
        for lane_section_xodr in lanes_xodr.findall('laneSection'):
            lane_sections.append(LaneSection(lane_section_xodr))
        return lane_sections
    def get_lanes_at_end(self):
        sections = self.get_lane_sections()
        section_at_end = sections[0]
        for lane_section in sections:
            if lane_section.get_s() > section_at_end.get_s():
                section_at_end = lane_section
        return section_at_end.lanes
    def get_lanes_at_start(self):
        sections = self.get_lane_sections()
        section_at_start = sections[0]
        for lane_section in sections:
            if lane_section.get_s() < section_at_start.get_s():
                section_at_start = lane_section
        return section_at_start.lanes
    def get_geometries(self):
        geoms = []
        plain_xodr = self.road_xodr.find('planView')
        for geom_xodr in plain_xodr.findall('geometry'):
            geoms.append(Geometry(geom_xodr))
        return geoms

class LaneLink(object):
    def __init__(self, link_xodr):
        self.link_xodr = link_xodr
    def get_from(self):
        return int(self.link_xodr.attrib['from'])
    def set_from(self, from_val):
        self.link_xodr.attrib['from'] = str(from_val)
    def get_to(self):
        return int(self.link_xodr.attrib['to'])
    def set_to(self, to_val):
        self.link_xodr.attrib['to'] = str(to_val)

class Connection(object):
    def __init__(self, conn_xodr):
        self.conn_xodr = conn_xodr

    def get_id(self):
        return int(self.conn_xodr.attrib['id'])
    def get_incomingRoad(self):
        return int(self.conn_xodr.attrib['incomingRoad'])
    def get_connectingRoad(self):
        return int(self.conn_xodr.attrib['connectingRoad'])
    def get_contactPoint(self):
        return self.conn_xodr.attrib['contactPoint']
    def get_lane_links(self):
        lane_links = []
        for link_xodr in self.conn_xodr.findall('laneLink'):
            lane_links.append(LaneLink(link_xodr))
        return lane_links

class Junction(object):
    def __init__(self, junc_xodr):
        self.junc_xodr = junc_xodr

    def get_id(self):
        return self.junc_xodr.attrib['id']

    def get_connections(self):
        conns = []
        for conn_xodr in self.junc_xodr.findall('connection'):
            conns.append(Connection(conn_xodr))
        return conns

def get_roads(xodr_root):
    road_dict = {}
    for road_xodr in xodr_root.findall('road'):
        road = Road(road_xodr)
        road_dict[road.get_id()] = road
    return road_dict

def get_junctions(xodr_root):
    junc_dict = {}
    for junc_xodr in xodr_root.findall('junction'):
        junction = Junction(junc_xodr)
        junc_dict[junction.get_id()] = junction
    return junc_dict

def sign(val):
    if val > 0:
        return 1
    return -1

def center_map_func(road_dict):
    geometries = []
    for road_id in road_dict:
        road = road_dict[road_id]
        for geom in road.get_geometries():
            geometries.append(geom)
    min_x = geometries[0].get_x()
    min_y = geometries[0].get_x()
    max_x = geometries[0].get_y()
    max_y = geometries[0].get_y()
    for geom in geometries:
        min_x = min(min_x, geom.get_x())
        max_x = max(max_x, geom.get_x())
        min_y = min(min_y, geom.get_y())
        max_y = max(max_y, geom.get_y())
    center_x = (min_x + max_x)*0.5
    center_y = (min_y + max_y)*0.5
    for geom in geometries:
        new_x = geom.get_x() - center_x
        new_y = geom.get_y() - center_y
        geom.set_x(new_x)
        geom.set_y(new_y)

def fix_opendrive(xodr_string, center_map=False):
    """
    Fixes an OpenDRIVE string searching for wrong lane ids in junction connections.
    """
    xodr_root = ET.fromstring(xodr_string)

    road_dict = get_roads(xodr_root)
    junc_dict = get_junctions(xodr_root)

    for junc_id in junc_dict:
        junction = junc_dict[junc_id]
        for conn in junction.get_connections():
            # incomming_road = road_dict[conn.get_incomingRoad()]
            connecting_road = road_dict[conn.get_connectingRoad()]
            connecting_lanes = None
            if conn.get_contactPoint == 'start':
                connecting_lanes = connecting_road.get_lanes_at_start()
            else:
                connecting_lanes = connecting_road.get_lanes_at_end()
            for lane_link in conn.get_lane_links():
                to_val = lane_link.get_to()
                if to_val not in connecting_lanes:
                    print('Missing lane ' + str(lane_link.get_to()) + ' in road ' + str(connecting_road.get_id()) + ', fixing...')
                    lane_increment = -1*sign(to_val)
                    new_to = to_val
                    while new_to not in connecting_lanes and new_to != 0:
                        new_to = new_to + lane_increment
                    if new_to == 0:
                        raise Exception('Error: Could not find suitable lane connection in junction ' + str(junction.get_id()))
                    lane_link.set_to(new_to)

    if center_map:
        center_map_func(road_dict)

    return ET.tostring(xodr_root, encoding='utf-8')
