#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


import glob
import os
import sys
import sqlite3

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

def main():
    # We start creating the client
    client = carla.Client('localhost', 2000)
    client.set_timeout(5.0)
    world = client.get_world()

    connection = sqlite3.connect('carla_bbs.db')
    c = connection.cursor()
    #Check buildings bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.Buildings)
    for _i in range(0, len(world_bbs)) :
        db_data = c.execute('''SELECT * FROM Buildings WHERE id = ''' + str(_i)).fetchall()
        db_data_list = []
        for _j in db_data :
            db_data_list.append(_j) 
        if(world_bbs[_i].location.x != db_data_list[0][1]):
            print("[TAG = Buildings] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
        if(world_bbs[_i].location.y != db_data_list[0][2]):
            print("[TAG = Buildings] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
        if(world_bbs[_i].location.z != db_data_list[0][3]):
            print("[TAG = Buildings] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
        if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
            print("[TAG = Buildings] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
        if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
            print("[TAG = Buildings] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
        if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
            print("[TAG = Buildings] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
        if(world_bbs[_i].extent.x != db_data_list[0][7]):
            print("[TAG = Buildings] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
        if(world_bbs[_i].extent.y != db_data_list[0][8]):
            print("[TAG = Buildings] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
        if(world_bbs[_i].extent.z != db_data_list[0][9]):
            print("[TAG = Buildings] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    #Check fences bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.Fences)
    if len(world_bbs) != 0 :
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM Fences WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 
            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = Fences] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = Fences] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = Fences] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = Fences] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = Fences] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = Fences] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = Fences] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = Fences] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = Fences] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    #TODO : Arreglar espacios de DB Data
    #Check other bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.Other)
    if len(world_bbs) != 0 :
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM Other WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 
            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = Other] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = Other] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = Other] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = Other] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = Other] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = Other] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = Other] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = Other] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = Other] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    #Check pedestrians bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.Pedestrians)
    if len(world_bbs) != 0 :
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM Pedestrians WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 
            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = Pedestrians] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = Pedestrians] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = Pedestrians] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = Pedestrians] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = Pedestrians] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = Pedestrians] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = Pedestrians] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = Pedestrians] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = Pedestrians] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    #Check poles bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.Poles)
    if len(world_bbs) != 0 :
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM Poles WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 
            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = Poles] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = Poles] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = Poles] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = Poles] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = Poles] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = Poles] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = Poles] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = Poles] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = Poles] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    #Check roadlines bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.RoadLines)
    if len(world_bbs) != 0 :
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM RoadLines WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 
            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = RoadLines] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = RoadLines] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = RoadLines] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = RoadLines] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = RoadLines] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = RoadLines] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = RoadLines] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = RoadLines] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = RoadLines] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    #Check roads bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.Roads)
    if len(world_bbs) != 0 :
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM Roads WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 
            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = Roads] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = Roads] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = Roads] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = Roads] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = Roads] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = Roads] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = Roads] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = Roads] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = Roads] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    #Check sidewalks bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.Sidewalks)
    if len(world_bbs) != 0 :
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM Sidewalks WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 

            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = Sidewalks] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = Sidewalks] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = Sidewalks] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = Sidewalks] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = Sidewalks] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = Sidewalks] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = Sidewalks] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = Sidewalks] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = Sidewalks] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    #Check trafficsigns bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.TrafficSigns)
    if len(world_bbs) != 0 :  
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM TrafficSigns WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 
            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = TrafficSigns] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = TrafficSigns] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = TrafficSigns] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = TrafficSigns] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = TrafficSigns] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = TrafficSigns] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = TrafficSigns] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = TrafficSigns] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = TrafficSigns] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    #Check vegetation bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.Vegetation)
    if len(world_bbs) != 0 :
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM Vegetation WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 
            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = Vegetation] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = Vegetation] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = Vegetation] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = Vegetation] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = Vegetation] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = Vegetation] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = Vegetation] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = Vegetation] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = Vegetation] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    #Check vehicles bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.Vehicles)
    if len(world_bbs) != 0 :
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM Vehicles WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 
            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = Vehicles] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = Vehicles] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = Vehicles] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = Vehicles] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = Vehicles] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = Vehicles] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = Vehicles] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = Vehicles] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = Vehicles] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    #Check walls bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.Walls)
    if len(world_bbs) != 0 :
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM Walls WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 
            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = Walls] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = Walls] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = Walls] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = Walls] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = Walls] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = Walls] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = Walls] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = Walls] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = Walls] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    #Check sky bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.Sky)
    if len(world_bbs) != 0 :
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM Sky WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 
            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = Sky] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = Sky] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = Sky] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = Sky] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = Sky] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = Sky] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = Sky] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = Sky] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = Sky] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    #Check ground bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.Ground)
    if len(world_bbs) != 0 :
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM Ground WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 
            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = Ground] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = Ground] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = Ground] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = Ground] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = Ground] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = Ground] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = Ground] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = Ground] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = Ground] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
        
    #Check bridge bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.Bridge)
    if len(world_bbs) != 0 :
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM Bridge WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 
            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = Bridge] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = Bridge] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = Bridge] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = Bridge] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = Bridge] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = Bridge] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = Bridge] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = Bridge] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = Bridge] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    #Check railtrack bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.RailTrack)
    if len(world_bbs) != 0 :
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM RailTrack WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 
            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = RailTrack] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = RailTrack] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = RailTrack] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = RailTrack] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = RailTrack] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = RailTrack] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = RailTrack] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = RailTrack] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = RailTrack] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    #Check guardrail bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.GuardRail)
    if len(world_bbs) != 0 :
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM GuardRail WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 
            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = GuardRail] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = GuardRail] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = GuardRail] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = GuardRail] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = GuardRail] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = GuardRail] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = GuardRail] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = GuardRail] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = GuardRail] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    #Check trafficlight bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.TrafficLight)
    if len(world_bbs) != 0 :
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM TrafficLight WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 
            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = TrafficLight] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = TrafficLight] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = TrafficLight] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = TrafficLight] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = TrafficLight] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = TrafficLight] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = TrafficLight] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = TrafficLight] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = TrafficLight] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    #Check static bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.Static)
    if len(world_bbs) != 0 :
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM Static WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 
            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = Static] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = Static] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = Static] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = Static] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = Static] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = Static] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = Static] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = Static] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = Static] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    #Check dynamic bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.Dynamic)
    if len(world_bbs) != 0 :
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM Dynamic WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 
            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = Dynamic] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = Dynamic] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = Dynamic] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = Dynamic] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = Dynamic] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = Dynamic] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = Dynamic] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = Dynamic] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = Dynamic] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    #Check water bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.Water)
    if len(world_bbs) != 0 :
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM Water WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 
            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = Water] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = Water] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = Water] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = Water] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = Water] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = Water] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = Water] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = Water] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = Water] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    #Check terrain bounding boxes
    world_bbs = world.get_level_bbs(carla.CityObjectLabel.Terrain)
    if len(world_bbs) != 0 :
        for _i in range(0, len(world_bbs)) :
            db_data = c.execute('''SELECT * FROM Terrain WHERE id = ''' + str(_i)).fetchall()
            db_data_list = []
            for _j in db_data :
                db_data_list.append(_j) 
            if(world_bbs[_i].location.x != db_data_list[0][1]):
                print("[TAG = Terrain] BB TransX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.x) + " [DB Data] : " + str(db_data_list[0][1]))
            if(world_bbs[_i].location.y != db_data_list[0][2]):
                print("[TAG = Terrain] BB TransY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.y) + " [DB Data] : " + str(db_data_list[0][2]))
            if(world_bbs[_i].location.z != db_data_list[0][3]):
                print("[TAG = Terrain] BB TransZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].location.z) + " [DB Data] : " + str(db_data_list[0][3]))
            if(world_bbs[_i].rotation.roll != db_data_list[0][4]):
                print("[TAG = Terrain] BB Roll -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.roll) + " [DB Data] : " + str(db_data_list[0][4]))
            if(world_bbs[_i].rotation.pitch != db_data_list[0][5]):
                print("[TAG = Terrain] BB Pitch -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.pitch) + " [DB Data] : " + str(db_data_list[0][5]))
            if(world_bbs[_i].rotation.yaw != db_data_list[0][6]):
                print("[TAG = Terrain] BB Yaw -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].rotation.yaw) + " [DB Data] : " + str(db_data_list[0][6]))
            if(world_bbs[_i].extent.x != db_data_list[0][7]):
                print("[TAG = Terrain] BB ExtentX -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.x) + " [DB Data] : " + str(db_data_list[0][7]))
            if(world_bbs[_i].extent.y != db_data_list[0][8]):
                print("[TAG = Terrain] BB ExtentY -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.y) + " [DB Data] : " + str(db_data_list[0][8]))
            if(world_bbs[_i].extent.z != db_data_list[0][9]):
                print("[TAG = Terrain] BB ExtentZ -> ID : " + str(_i) + " | [World Data] " + str(world_bbs[_i].extent.z) + " [DB Data] : " + str(db_data_list[0][9]))
    
    connection.close()

if __name__ == "__main__":
    try: 
        main()
    except KeyboardInterrupt:
        print(' - Exited by user.')