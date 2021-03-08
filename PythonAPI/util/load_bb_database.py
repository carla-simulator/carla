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
    c.execute('''CREATE TABLE Buildings(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE Fences(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE Other(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE Pedestrians(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE Poles(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE RoadLines(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE Roads(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE Sidewalks(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE TrafficSigns(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE Vegetation(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE Vehicles(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE Walls(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE Sky(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE Ground(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE Bridge(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE RailTrack(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE GuardRail(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE TrafficLight(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE Static(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE Dynamic(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE Water(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    c.execute('''CREATE TABLE Terrain(id INT, locationX REAL, locationY REAL, locationZ REAL, rotationRoll REAL, rotationPitch REAL, rotationYaw REAL, extentX REAL, extentY REAL, extentZ REAL)''')
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.Buildings)
    
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO Buildings VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))     
         
    print("Saved carla.CityObjectLabel.Buildings bounding boxes data")
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.Fences)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO Fences VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.Fences bounding boxes data")
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.Other)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO Other VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.Other bounding boxes data")
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.Pedestrians)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO Pedestrians VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.Pedestrians bounding boxes data")
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.Poles)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO Poles VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.Poles bounding boxes data")
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.Roads)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO Roads VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.Poles bounding boxes data")
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.RoadLines)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO Roadlines VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.RoadLines bounding boxes data")
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.Sidewalks)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO Sidewalks VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.Sidewalks bounding boxes data")
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.TrafficSigns)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO TrafficSigns VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.TrafficSigns bounding boxes data")
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.Vegetation)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO Vegetation VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.Vegetation bounding boxes data")
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.Vehicles)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO Vehicles VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.Vehicles bounding boxes data")
        
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.Walls)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO Walls VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.Walls bounding boxes data")
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.Sky)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO Sky VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.Sky bounding boxes data")
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.Ground)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO Ground VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.Ground bounding boxes data")
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.Bridge)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO Bridge VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.Bridge bounding boxes data")
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.RailTrack)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO RailTrack VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.RailTrack bounding boxes data")
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.GuardRail)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO GuardRail VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.GuardRail bounding boxes data")
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.TrafficLight)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO TrafficLight VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.TrafficLight bounding boxes data")
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.Static)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO Static VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.Static bounding boxes data")
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.Dynamic)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO Dynamic VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.Dynamic bounding boxes data")
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.Water)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO Water VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.Water bounding boxes data")
    bounding_boxes = world.get_level_bbs(carla.CityObjectLabel.Terrain)
    for _i in range(0, len(bounding_boxes)):
        c.execute('''INSERT INTO Terrain VALUES(?,?,?,?,?,?,?,?,?,?)''', (_i, 
        bounding_boxes[_i].location.x, bounding_boxes[_i].location.y, bounding_boxes[_i].location.z, 
        bounding_boxes[_i].rotation.roll, bounding_boxes[_i].rotation.pitch, bounding_boxes[_i].rotation.yaw, 
        bounding_boxes[_i].extent.x, bounding_boxes[_i].extent.y, bounding_boxes[_i].extent.z))
    print("Saved carla.CityObjectLabel.Terrain bounding boxes data")
    connection.commit()

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print(' - Exited by user.')