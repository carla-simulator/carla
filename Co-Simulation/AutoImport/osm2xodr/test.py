#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Nov 11 14:59:36 2019

@author: jhm
"""
from OSMParser.testing import _test_nodes, TestEntity
        
def test_simple3Way2LaneTCrossing():
    nodes = [TestEntity(1,None,lon=1, lat=1, nodes=0), 
            TestEntity(2,None,lon=1, lat=2, nodes=0),
            TestEntity(3,None,lon=2, lat=2, nodes=0),
            TestEntity(4,None,lon=0, lat=2, nodes=0)]
    ways = [TestEntity(1,{"highway":"residential","lanes":"2","turn:lanes:forward":"through"}, nodes=[1,2]),
            TestEntity(2,{"highway":"residential","lanes":"2","turn:lanes:forward":"through"}, nodes=[2,3]),
            TestEntity(3,{"highway":"residential","lanes":"2","turn:lanes:forward":"through"}, nodes=[2,4])]
    _test_nodes(nodes, ways)

def test_simple3WayoneWayLaneTCrossing():
    nodes = [TestEntity(1,None,lon=1, lat=1, nodes=0), 
            TestEntity(2,None,lon=1, lat=2, nodes=0),
            TestEntity(3,None,lon=2, lat=2, nodes=0),
            TestEntity(4,None,lon=0, lat=2, nodes=0)]
    ways = [TestEntity(1,{"highway":"residential","lanes":"2","oneway":"yes"}, nodes=[1,2]),
            TestEntity(2,{"highway":"residential","lanes":"2","turn:lanes:forward":"through"}, nodes=[2,3]),
            TestEntity(3,{"highway":"residential","lanes":"2","turn:lanes:forward":"through"}, nodes=[2,4])]
    _test_nodes(nodes, ways)

def test_simple3WayoneWayLaneTCrossingOpposite():
    nodes = [TestEntity(1,None,lon=1, lat=1, nodes=0), 
            TestEntity(2,None,lon=1, lat=2, nodes=0),
            TestEntity(3,None,lon=2, lat=2, nodes=0),
            TestEntity(4,None,lon=0, lat=2, nodes=0)]
    ways = [TestEntity(1,{"highway":"residential","lanes":"2","oneway":"yes"}, nodes=[2,1]),
            TestEntity(2,{"highway":"residential","lanes":"2","turn:lanes:forward":"through"}, nodes=[2,3]),
            TestEntity(3,{"highway":"residential","lanes":"2","turn:lanes:forward":"through"}, nodes=[2,4])]
    _test_nodes(nodes, ways)

def test_merge2to3():
    nodes = [TestEntity(1,None,lon=1, lat=1, nodes=0), 
            TestEntity(2,None,lon=1, lat=2, nodes=0),
            TestEntity(3,None,lon=2, lat=2, nodes=0)]
    ways = [TestEntity(1,{"highway":"residential","lanes":"3","lanes:forward":"2"}, nodes=[1,2]),
            TestEntity(2,{"highway":"residential","lanes":"4","lanes:forward":"3"}, nodes=[2,3])]
    _test_nodes(nodes, ways)

def test_merge3to2():
    nodes = [TestEntity(1,None,lon=1, lat=1, nodes=0), 
            TestEntity(2,None,lon=1, lat=2, nodes=0),
            TestEntity(3,None,lon=2, lat=2, nodes=0)]
    ways = [TestEntity(1,{"highway":"residential","lanes":"4","lanes:forward":"3"}, nodes=[1,2]),
            TestEntity(2,{"highway":"residential","lanes":"3","lanes:forward":"2"}, nodes=[2,3])]
    _test_nodes(nodes, ways)

def test_simple3WayoneWayLaneTCrossing2():
    nodes = [TestEntity(1,None,lon=1, lat=1, nodes=0), 
            TestEntity(2,None,lon=1, lat=1.001, nodes=0),
            TestEntity(3,None,lon=1.001, lat=1.001, nodes=0),
            TestEntity(4,None,lon=0.99, lat=1.001, nodes=0)]
    ways = [TestEntity(1,{"highway":"residential","lanes":"2","oneway":"yes"}, nodes=[1,2]),
            TestEntity(2,{"highway":"residential","lanes":"2","turn:lanes:forward":"through"}, nodes=[2,3]),
            TestEntity(3,{"highway":"residential","lanes":"2","turn:lanes:forward":"through"}, nodes=[2,4])]
    _test_nodes(nodes, ways)

def test_3WayTCrossing2():
    nodes = [TestEntity(1,None,lon=1, lat=1, nodes=0), 
            TestEntity(2,None,lon=1, lat=1.001, nodes=0),
            TestEntity(3,None,lon=1.001, lat=1.001, nodes=0),
            TestEntity(4,None,lon=0.999, lat=1.001, nodes=0)]
    ways = [TestEntity(1,{"highway":"residential","lanes":"3", "lanes:forward":"2", "lanes:backward":"1","oneway":"no"}, nodes=[1,2]),
            TestEntity(2,{"highway":"residential","lanes":"3", "lanes:forward":"2", "lanes:backward":"1","oneway":"no"}, nodes=[2,3]),
            TestEntity(3,{"highway":"residential","lanes":"3", "lanes:forward":"2", "lanes:backward":"1","oneway":"no"}, nodes=[2,4])]
    _test_nodes(nodes, ways)


def test_2WayTCrossing():
    nodes = [TestEntity(1,None,lon=1, lat=1, nodes=0), 
            TestEntity(2,None,lon=1, lat=1.001, nodes=0),
            TestEntity(3,None,lon=1.001, lat=1.001, nodes=0)
            ]
    ways = [TestEntity(1,{"highway":"residential","lanes":"5", "lanes:forward":"3", "lanes:backward":"2","oneway":"no"}, nodes=[1,2]),
            TestEntity(2,{"highway":"residential","lanes":"5", "lanes:forward":"2", "lanes:backward":"3","oneway":"no"}, nodes=[3,2])
            ]
    _test_nodes(nodes, ways)

def test_2WayTCrossing2():
    nodes = [TestEntity(1,None,lon=1, lat=1, nodes=0), 
            TestEntity(2,None,lon=1, lat=1.001, nodes=0),
            TestEntity(3,None,lon=1.001, lat=1.001, nodes=0)
            ]
    ways = [TestEntity(1,{"highway":"residential","lanes":"5", "lanes:forward":"3", "lanes:backward":"2","oneway":"no"}, nodes=[1,2]),
            TestEntity(2,{"highway":"residential","lanes":"5", "lanes:forward":"3", "lanes:backward":"2","oneway":"no"}, nodes=[2,3])
            ]
    _test_nodes(nodes, ways)

def test_2WayTCrossing3():
    nodes = [TestEntity(1,None,lon=1, lat=1, nodes=0), 
            TestEntity(2,None,lon=1, lat=1.001, nodes=0),
            TestEntity(3,None,lon=1.001, lat=1.001, nodes=0)
            ]
    ways = [TestEntity(1,{"highway":"residential","lanes":"5", "lanes:forward":"2", "lanes:backward":"3","oneway":"no"}, nodes=[2,1]),
            TestEntity(2,{"highway":"residential","lanes":"5", "lanes:forward":"3", "lanes:backward":"2","oneway":"no"}, nodes=[2,3])
            ]
    _test_nodes(nodes, ways)

def test_2WayTCrossing4():
    nodes = [TestEntity(1,None,lon=1, lat=1, nodes=0), 
            TestEntity(2,None,lon=1, lat=1.001, nodes=0),
            TestEntity(3,None,lon=1.001, lat=1.001, nodes=0)
            ]
    ways = [TestEntity(1,{"highway":"residential","lanes":"5", "lanes:forward":"2", "lanes:backward":"3","oneway":"no"}, nodes=[1,2]),
            TestEntity(2,{"highway":"residential","lanes":"5", "lanes:forward":"3", "lanes:backward":"2","oneway":"no"}, nodes=[3,2])
            ]
    _test_nodes(nodes, ways)

def testSimpleRoad():
        nodes = [TestEntity(1,None,lon=1, lat=1, nodes=0), 
            TestEntity(2,None,lon=1, lat=1.001, nodes=0),
            TestEntity(3,None,lon=1.001, lat=1.002, nodes=0),
            TestEntity(4,None,lon=1.001, lat=1.003, nodes=0)
            ]
        ways = [TestEntity(1,{"highway":"residential","lanes":"5", "lanes:forward":"3", "lanes:backward":"2","oneway":"no"}, nodes=[1,2,3,4])]
        _test_nodes(nodes, ways)

test_2WayTCrossing2()
test_2WayTCrossing4()
test_2WayTCrossing2()
test_2WayTCrossing3()
test_3WayTCrossing2()
test_simple3WayoneWayLaneTCrossing2()
test_merge3to2()
test_simple3WayoneWayLaneTCrossing()