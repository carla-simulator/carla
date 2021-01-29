# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2016-2020 German Aerospace Center (DLR) and others.
# SUMOPy module
# Copyright (C) 2012-2017 University of Bologna - DICAM
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    networktools.py
# @author  Joerg Schweizer
# @date


import subprocess
import os
import sys
import string
import numpy as np
from numpy import random
from collections import OrderedDict
import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
import agilepy.lib_base.xmlman as xm
from agilepy.lib_base.processes import Process, CmlMixin
import netconvert
import routing
#from agilepy.lib_base.geometry import get_length_polypoints,get_dist_point_to_segs, get_diff_angle_clockwise
from agilepy.lib_base.geometry import *


class TlsGenerator(netconvert.NetConvertMixin):
    def __init__(self,  net, logger=None, **kwargs):
        print 'TlsGenerate.__init__'
        self._init_common('tlsgenerator',
                          parent=net,
                          name='Trafficlight system generator',
                          logger=logger,
                          info='Detects, sets and configures major traffic light systems.',
                          )

        attrsman = self.set_attrsman(cm.Attrsman(self))

        # self.init_options_tls()
        self.dist_tls_max = 60.0
        self.dist_tls_internode_max = 30.0
        self.length_min_notls = 6.0
        self.ptype = 1  # "static": 1,"actuated": 2,
        self.offset = 0
        self.cycletime = 180
        self.yellowtime = 5
        self.phasetime_min = 6
        self.phasetime_max = 50

        self.id_mode_bike = net.modes.get_id_mode("bicycle")
        self.id_mode_ped = net.modes.get_id_mode("pedestrian")
        self.id_mode_car = net.modes.get_id_mode("passenger")
        self.id_mode_bus = net.modes.get_id_mode("bus")
        self.ids_cycleped = [self.id_mode_bike, self.id_mode_ped]
        #self.id_mode_ped = net.modes.get_id_mode("delivery")

    def is_major_road(self, priority, n_lanes, speed_max):
        return ((priority >= 7) | (n_lanes >= 3)) & (n_lanes >= 2) & (speed_max >= 30/3.6)

    def get_routecost(self, route):
        """
        Centralized function to determine the importance of a route.
        """
        return np.sum(self.parent.edges.lengths[route])

    def do(self):
        print 'TlsGenerator.do'
        net = self.parent

        edges = net.edges
        nodes = net.nodes
        lanes = net.lanes
        connections = net.connections
        tlss = net.tlss

        tlss.clear_tlss()

        # construct major routes
        ids_edges_major = set()
        self.ids_edges_major = ids_edges_major

        ids_edge = edges.get_ids()
        majorroutes = []
        for id_edge, id_sumo, type_spread, shape, \
            n_lanes, ids_lane, priority, id_fromnode, id_tonode, speed_max\
            in zip(ids_edge,
                   edges.ids_sumo[ids_edge],
                   edges.types_spread[ids_edge],
                   edges.shapes[ids_edge],
                   edges.nums_lanes[ids_edge],
                   edges.ids_lanes[ids_edge],
                   edges.priorities[ids_edge],
                   edges.ids_fromnode[ids_edge],
                   edges.ids_tonode[ids_edge],
                   edges.speeds_max[ids_edge],
                   ):
            print '    is_major_road', id_edge, self.is_major_road(
                priority, n_lanes, speed_max), id_edge not in ids_edges_major
            if self.is_major_road(priority, n_lanes, speed_max):
                if id_edge not in ids_edges_major:
                    #dist = 0

                    route_back = self.follow_major_route_backward(id_edge, shape)
                    route_forward = self.follow_major_route_foreward(id_edge, shape)
                    if len(route_back)+len(route_forward) > 0:
                        majorroutes.append(route_back[::-1]+[id_edge]+route_forward)

        self.majorroutes = majorroutes

        print '  majorroutes:'  # ,majorroutes
        # mapping with id_edge as key ind index of  majorroutes
        edges_major = {}
        for i, route in zip(xrange(len(majorroutes)), majorroutes):
            print '    route', i, len(route), route
            for id_edge in route:
                edges_major[id_edge] = i

        self.edges_major = edges_major
        # identify major nodes
        majornodes = []
        self.majorroutecosts = []
        for route in majorroutes:
            # print '    check route',len(route),route
            ids_node = np.zeros(len(route)+1, dtype=np.int32)
            # print '      ids_node',len(ids_node),ids_node
            ids_node[:-1] = edges.ids_fromnode[route]
            ids_node[-1] = edges.ids_fromnode[route[-1]]
            majornodes.append(ids_node)
            self.majorroutecosts.append(self.get_routecost(route))

        # identify major junctions
        majorjuntions = {}
        for i_route1, ids_node1 in zip(xrange(len(majorroutes)), majornodes):
            ids_nodeset1 = set(ids_node1)
            for i_route2, ids_node2 in zip(xrange(len(majorroutes)), majornodes):
                if i_route2 != i_route1:
                    print '    check routes', i_route1, i_route2, 'ids_node_inter', ids_nodeset1.intersection(ids_node2)
                    for id_node_inter in ids_nodeset1.intersection(ids_node2):
                        # go through all nodes that route1 and route2 have in common
                        if majorjuntions.has_key(id_node_inter):
                            # add route index to already existing junction
                            if i_route1 not in majorjuntions[id_node_inter]:
                                majorjuntions[id_node_inter].append(i_route1)
                            if i_route2 not in majorjuntions[id_node_inter]:
                                majorjuntions[id_node_inter].append(i_route2)
                        else:
                            # create new junction and add the two route indexes
                            majorjuntions[id_node_inter] = [i_route1, i_route2]

        self.ids_majornodes = set(majorjuntions.keys())
        self.ids_majornodes_used = set()

        # create mojor TLS
        print '  major junctions:'
        for id_node, inds_route in majorjuntions.iteritems():
            if id_node not in self.ids_majornodes_used:
                # debug
                print '    Next majornode', id_node
                for ind in inds_route:
                    print '      majorroute', majorroutes[ind]

                self.make_majortls(id_node, inds_route)
            # print '    junction', id_node
            # for ind_route in  majorjuntions[id_node]:
            #    print '      ',majorroutes[ind_route]

        return True

    def make_majortls(self, id_node_major, inds_route):
        print 79*'-'
        print 'make_majortls for', id_node_major, inds_route
        edges = self.parent.edges
        nodes = self.parent.nodes
        lanes = self.parent.lanes
        connections = self.parent.connections
        tlss = self.parent.tlss
        edges_major = self.edges_major
        # identify all nodes
        nodes_tls = OrderedDict()

        # put cross node first
        self.init_tlsnode(id_node_major, nodes_tls)
        #edges_major_tls = {}

        # find tls nodes and put info in nodes_tls
        self.find_neighbour_nodes(id_node_major, 0.0, nodes_tls, edges, nodes)

        # build connection conflict matrix
        # identify external incoming and outgoing edges

        # for ind_route in inds_route:
        #    for id_edge in self.majorroutes[ind_route]:
        #        edges_major_tls[id_edge] = ind_route

        # get all connections from incoming links
        #connections_tls = OrderedDict()
        ids_con_tls = []
        #ids_conlane = []
        ids_connodes = []
        convecs = []

        # edges entering the TLS
        # edges are either bikeways, major or other edges
        # no pedestrian edges are considered
        ids_incoming_tls = []  # other edges
        ids_incoming_bike_tls = []  # bikeway edges
        ids_incoming_major_tls = []  # major edges

        # edges leaving the TLS
        # edges are either bikeways, major or other edges
        # no pedestrian edges are considered
        ids_outgoing_tls = []  # other edges
        ids_outgoing_major_tls = []  # major edges
        ids_outgoing_bike_tls = []  # bikeway edges
        ids_nodes_tls = set(nodes_tls.keys())

        ids_cycleped = set(self.ids_cycleped)  # set with bike and ped mode

        # enrich node attributes: is_cycleped, is_crossing
        #  detect all connections, lanes and everything else
        for id_node, nodeattrs in nodes_tls.iteritems():
            print '  check all in id_node', id_node
            is_cycleped = True
            n_cycleped = 0  # count incomin and outgoing cycleped edges
            n_nocycleped_in = 0  # count normal incoming road edges
            n_cycleped_in = 0
            n_cycleped_out = 0
            # make lists with incoming edges
            for id_edge in nodes.ids_incoming[id_node]:

                print '    check incoming', id_edge
                ids_lane = edges.ids_lanes[id_edge]
                #is_cycleped_edge = True
                #is_ped_edge = True
                #is_cycle_edge = True

                ids_lanemodes = set(lanes.ids_mode[ids_lane])
                is_cycleped_edge = ids_cycleped.issuperset(ids_lanemodes)
                is_cycle_edge = (self.id_mode_bike in ids_lanemodes) & ((len(ids_lanemodes) == 1) | is_cycleped_edge)
                is_ped_edge = (self.id_mode_ped in ids_lanemodes) & (len(ids_lanemodes) == 1)
                for id_lane in ids_lane:
                    #is_cp = lanes.ids_mode[id_lane] in self.ids_cycleped
                    #is_cycleped &= is_cp
                    #is_cycleped_edge &= is_cp
                    #is_ped_edge  &= lanes.ids_mode[id_lane] == self.id_mode_ped
                    #is_cycle_edge  &= lanes.ids_mode[id_lane] == self.id_mode_bike

                    for id_con in connections.select_ids(connections.ids_fromlane.get_value() == id_lane):
                        #connections_tls[id_con] = {'vec_in' : lanes.shapes[id_lane][-2:]}
                        ids_con_tls.append(id_con)
                        ids_connodes.append(id_node)

                if not is_cycleped_edge:
                    n_nocycleped_in += 1
                else:
                    n_cycleped += 1
                    n_cycleped_in += 1
                    nodeattrs['ids_cycleped_incoming'].append(id_edge)

                # detect incoming edges
                # ,not is_ped_edge,'not from TLS',edges.ids_fromnode[id_edge] not in ids_nodes_tls
                print '       is external', edges.ids_fromnode[
                    id_edge] not in ids_nodes_tls, 'is_cycle_edge', is_cycle_edge, 'is_major', id_edge in edges_major, 'is_noped'
                if edges.ids_fromnode[id_edge] not in ids_nodes_tls:
                    # from node is not part of the TLS
                    # so it comes from external
                    if is_cycle_edge:
                        # print '        apend ids_incoming_bike_tls',id_edge
                        ids_incoming_bike_tls.append(id_edge)

                    elif id_edge in edges_major:
                        # print '        apend ids_incoming_major_tls',id_edge
                        ids_incoming_major_tls.append(id_edge)

                    elif not is_ped_edge:  # do not consider pedestrian edges
                        # print '        append ids_incoming_tls',id_edge
                        ids_incoming_tls.append(id_edge)
                    # else:
                    #    print '        no incoming id_edge',id_edge

            # make lists with outgoing edges
            for id_edge in nodes.ids_outgoing[id_node]:

                is_cycleped_edge = True
                is_ped_edge = True
                for id_lane in edges.ids_lanes[id_edge]:
                    is_cp = lanes.ids_mode[id_lane] in self.ids_cycleped
                    is_cycleped &= is_cp
                    is_cycleped_edge &= is_cp
                    is_ped_edge &= lanes.ids_mode[id_lane] == self.id_mode_ped

                # print '    check outgoing',id_edge,id_edge in edges_major
                if is_cycleped_edge:
                    n_cycleped += 1
                    n_cycleped_out += 1
                    nodeattrs['ids_cycleped_outgoing'].append(id_edge)

                if edges.ids_tonode[id_edge] not in ids_nodes_tls:
                    if is_cycleped_edge:
                        ids_outgoing_bike_tls.append(id_edge)

                    elif id_edge in edges_major:
                        ids_outgoing_major_tls.append(id_edge)

                    elif not is_ped_edge:  # do not consider pedestrian edges
                        ids_outgoing_tls.append(id_edge)

            # node is pure bike-pedestrian crossing > no signals
            nodeattrs['is_cycleped'] = is_cycleped

            # node crossing between bike-pedestrian and road
            #nodeattrs['is_crossing'] = (n_nocycleped_in == 1)&(n_cycleped>2)
            nodeattrs['is_crossing'] = (n_nocycleped_in == 1) & (n_cycleped_in == 1) & (n_cycleped_out == 1)

        # debug
        if 1:
            print '  nodes_tls:'
            for id_node, nodeattrs in nodes_tls.iteritems():
                print '    id_node', id_node
                for key, val in nodeattrs.iteritems():
                    print '        ', key, val

        n_cons = len(ids_con_tls)
        ids_con_tls = np.array(ids_con_tls, dtype=np.int32)
        ids_connodes = np.array(ids_connodes, dtype=np.int32)
        print '  ids_incoming_major_tls', ids_incoming_major_tls
        print '  ids_outgoing_major_tls', ids_outgoing_major_tls
        print '  ids_incoming_tls', ids_incoming_tls
        print '  ids_outgoing_tls', ids_outgoing_tls

        if len(ids_incoming_tls)+len(ids_incoming_major_tls) < 2:
            print '  Need at least 2 incoming edges. Abandon.'
            return False

        print '  connectors detected:', n_cons
        #ids_conlane = np.array(ids_conlane, dtype = np.int32 )
        #convecs = np.array(convecs, dtype = np.float32 )
        vertices_fromcon = np.zeros((n_cons, 2, 2), dtype=np.float32)
        vertices_tocon = np.zeros((n_cons, 2, 2), dtype=np.float32)
        are_enabeled = np.zeros((n_cons), dtype=np.bool)
        #are_enabeled = np.zeros((n_cons), dtype = np.bool)
        #convecs[:] =  lanes.shapes[ids_conlane][-2:]
        ids_fromlane_tls = connections.ids_fromlane[ids_con_tls]
        ids_tolane_tls = connections.ids_tolane[ids_con_tls]
        for i, shape_fromlane, shape_tolane in zip(
            xrange(n_cons),
            lanes.shapes[ids_fromlane_tls],
            lanes.shapes[ids_tolane_tls]
        ):
            #vertices_con[i] = [shape_fromlane[-1][:2], shape_tolane[0][:2]]
            vertices_fromcon[i] = [shape_fromlane[-2][:2], shape_fromlane[-1][:2]]
            vertices_tocon[i] = [shape_tolane[0][:2], shape_tolane[1][:2]]

        # conflicting connectors with
        # id_connector as key and binary index as value
        # the binary index has a True where the respective connector is in
        # conflict with id_connector of key
        inds_cons_conflict = OrderedDict()

        # merge connectors with
        # id_connector as key and binary index as value
        # the binary index has a True where the respective connector
        # merges into id_connector of key
        inds_cons_merge = OrderedDict()

        ids_conedges_from = lanes.ids_edge[connections.ids_fromlane[ids_con_tls]]
        ids_conedges_to = lanes.ids_edge[connections.ids_tolane[ids_con_tls]]
        #intersections = np.zeros((n_cons,n_cons), dtype = np.bool)
        # print '  intersections',n_cons, intersections.shape

        # debug
        dists_fromcon_node = OrderedDict()
        dists_tocon_node = OrderedDict()

        #dists = np.zeros((n_cons,n_cons), dtype = np.float32)
        va1f = vertices_fromcon[:, 0]
        va2f = vertices_fromcon[:, 1]
        va1t = vertices_tocon[:, 0]
        va2t = vertices_tocon[:, 1]
        va1x = vertices_fromcon[:, 1]
        va2x = vertices_tocon[:, 0]
        centroids = nodes.coords[ids_connodes][:, :2]
        dist_intercheck = 5.0
        for i, id_con,  vertex_fromcon, vertex_tocon, id_node, id_edge_from, id_edge_to in zip(
                xrange(n_cons), ids_con_tls,
                vertices_fromcon, vertices_tocon,
                ids_connodes, ids_conedges_from, ids_conedges_to):

            is_turnaround = edges.ids_tonode[id_edge_to] == edges.ids_fromnode[id_edge_from]
            are_enabeled[i] = (not nodes_tls[id_node]['is_cycleped']) & (not is_turnaround)
            # print '  intersect id_node %d; id_con!%d confl:'%(id_node,id_con)
            vb1f = np.ones((n_cons, 2), dtype=np.float32)*vertex_fromcon[0]
            vb2f = np.ones((n_cons, 2), dtype=np.float32)*vertex_fromcon[1]
            vxf, lasf = line_intersect2(vb1f, vb2f, va1f, va2f)

            vb1t = np.ones((n_cons, 2), dtype=np.float32)*vertex_tocon[0]
            vb2t = np.ones((n_cons, 2), dtype=np.float32)*vertex_tocon[1]
            vxt, last = line_intersect2(vb1t, vb2t, va1t, va2t)

            vb1x = np.ones((n_cons, 2), dtype=np.float32)*vertex_fromcon[1]
            vb2x = np.ones((n_cons, 2), dtype=np.float32)*vertex_tocon[0]
            vxx, lasx = line_intersect2(vb1x, vb2x, va1x, va2x)
            # print '    lasx',lasx

            dists_from = get_norm_2d(centroids-vxf)
            dists_to = get_norm_2d(centroids-vxt)

            inds1 = (dists_from < dist_intercheck) | np.isnan(dists_from)
            inds1 = inds1 & ((dists_to < dist_intercheck) | np.isnan(dists_to))

            inds2 = np.logical_and(lasx.flatten() < 1.5, lasx.flatten() > -0.5)  # | np.isnan(dists_to)

            # print '    inds1',inds1
            # print '    inds2',inds2
            inds = inds2  # np.logical_or(inds1,inds2)#|inds2
            # print '    inds',inds
            inds = inds & are_enabeled[i]
            inds = inds & (ids_connodes == id_node)
            inds = inds & (ids_conedges_from != id_edge_from)

            #inds = inds & (ids_conedges_from!=id_edge_from)
            # print '    inds_final',inds

            if 0:  # debug
                for id_con1, df, dt, id_node1, id_edge1, laf, lat, vf, vt, cent, c1, c2, c3, c4, crit in zip(
                        ids_con_tls, dists_from, dists_to, ids_connodes, ids_conedges_from,
                        lasf, last, vxf, vxt, centroids,
                        dists_from < dist_intercheck,
                        dists_to < dist_intercheck,
                        (ids_connodes == id_node), (ids_conedges_from != id_edge_from),
                        inds):
                    print '    id_con:%d d_from %.2f, d_to %.2f' % (
                        id_con1, df, dt), df < dist_intercheck, dt < dist_intercheck, id_node1 == id_node, id_edge1 != id_edge_from, crit
                    # print '    vf',vf,'vt',vt,'cent',cent,cent-vt

                dists_fromcon_node[id_con] = np.array(dists_from[inds], dtype=np.int32)
                dists_tocon_node[id_con] = np.array(dists_to[inds], dtype=np.int32)

            inds_cons_conflict[id_con] = inds
            # means to lanes are identical
            inds_cons_merge[id_con] = inds & np.isnan(dists_to)

        if 1:  # debug
            print '  show conflicts:', len(inds_cons_conflict)
            for id_con_tls, inds_con_conflict, id_connode in zip(ids_con_tls, inds_cons_conflict.values(), ids_connodes):
                print '   id_connode:%d id_con:%d' % (
                    id_connode, id_con_tls), 'ids_conf', ids_con_tls[inds_con_conflict]

            # print '  id_node %d, id_con %d confl:'%(id_node,id_con)
            # print '    ids_con_conflict',ids_con_tls[inds_con_conflict]
            # print '    dist_fromcon_node',dist_fromcon_node
            # print '    dist_tocon_node',dist_tocon_node
            #line_intersect(a1, a2, b1, b2)

        #ids_fromlane = kwargs['id_fromlane'],
        #ids_tolane = kwargs['id_tolane'],
        # OrderedDict([('s',(1,2)),('p',(3,4)),('a',(5,6)),('m',(7,8))])

        #self.id_mode_bike = net.modes.get_id_mode("bicycle")
        #self.id_mode_ped = net.modes.get_id_mode("pedestrian")
        #self.id_mode_car = net.modes.get_id_mode("passenger")

        print '\n  generate routes accross the TLS'
        #costs = edges.get_times( id_mode = self.id_mode_car , is_check_lanes = True, speed_max = None)
        # use bus here so we can route also on reserved lanes
        costs = edges.get_times(id_mode=self.id_mode_bus, is_check_lanes=True, speed_max=None)
        costs_bike = edges.get_times(id_mode=self.id_mode_bike, is_check_lanes=True,
                                     speed_max=None,
                                     modeconst_excl=-10.0, modeconst_mix=-5.0,)
        #costs = costs_bike
        #costs_bike = costs
        fstar = edges.get_fstar(is_ignor_connections=False)

        routes_tls = []
        routes_tls_cost = []
        ids_routeedge_tls = set()
        print '    create major routes for ids_incoming_major_tls', ids_incoming_major_tls, 'ids_outgoing_major_tls', ids_outgoing_major_tls
        for id_incoming_tls in ids_incoming_major_tls:
            # ind_route =
            # print '    major:id_incoming_tls',id_incoming_tls,type(id_incoming_tls), edges_major.has_key(id_incoming_tls)
            # print '      ind_major',edges_major_tls[id_incoming_tls]
            # print '      len(self.majorroutes)',len(self.majorroutes)

            # select major route
            route_major = self.majorroutes[edges_major[id_incoming_tls]]
            costs_major_tot = self.majorroutecosts[edges_major[id_incoming_tls]]
            print '      id_incoming_tls', id_incoming_tls, 'costs_major_tot', costs_major_tot
            ind_from = route_major.index(id_incoming_tls)
            for id_outgoing_tls in ids_outgoing_major_tls:
                print '       id_incoming_tls -> id_outgoing_tls', id_incoming_tls, '->', id_outgoing_tls
                route = []

                if (id_outgoing_tls in route_major) & (ind_from < (len(route_major)-1)):
                    # major route connects id_incoming_tls and id_outgoing_tls
                    ind_to = route_major.index(id_outgoing_tls)
                    routes, routecosts = self.correct_tls_route(
                        route_major[ind_from:ind_to+1], nodes_tls, is_check_angle=False)

                    if len(routes) == 1:  # single valid route confirmed
                        route = routes[0]
                        # cost of entire major route
                        costs_major = costs_major_tot
                    else:
                        # not on a  major route...will route
                        route = []

                if len(route) == 0:
                    # major route does not connects id_incoming_tls and id_outgoing_tls
                    print '    try to find a route between both edges'
                    dur, route_raw = routing.get_mincostroute_edge2edge(
                        id_incoming_tls, id_outgoing_tls,
                        weights=costs, fstar=fstar)
                    # print 'WARNING: use shortest path routing for major route route_raw',route_raw
                    # make sure route is in TLS
                    if len(route_raw) > 0:
                        # This route is not part of a major route,
                        #  only the first and last edge are part of a major route
                        routes, routecosts = self.correct_tls_route(
                            route_raw, nodes_tls, is_check_angle=True, angle_max=95.0/180*np.pi)
                    else:
                        routes = []
                        routecosts = 0.0

                    if len(routes) == 1:  # single valid route confirmed
                        route = routes[0]
                        # costs are now only the peace of route accross the junction
                        costs_major = routecosts[0]

                    else:
                        # route has been shortened, this means route
                        # has been running outside the TLS or making U-turns
                        # So that's not worth to keep
                        route = []
                        costs_major = 0.0

                print '    =>costs_major,route_major tls', costs_major, route
                if len(route) > 0:
                    routes_tls.append(route)
                    # costs to decide which phase is first
                    # this will have that a long route will receive the
                    # first phase of all traffic lights
                    routes_tls_cost.append(costs_major)
                    ids_routeedge_tls.update(route)

        if 1:  # sp
            print '  major routes_tls', routes_tls, ids_outgoing_bike_tls
            print '  create bike routes for ids_incoming_bike_tls', ids_incoming_bike_tls, ids_outgoing_bike_tls
            for id_incoming_tls in ids_incoming_bike_tls:  # +ids_incoming_tls+ids_incoming_major_tls:

                D, P = routing.edgedijkstra(id_incoming_tls, ids_edge_target=set(ids_outgoing_bike_tls),
                                            weights=costs_bike, fstar=fstar)

                for id_outgoing_tls in ids_outgoing_bike_tls:  # +ids_outgoing_tls+ids_outgoing_major_tls:
                    print '      bike route from %d to %d' % (
                        id_incoming_tls, id_outgoing_tls), 'can route?', (id_incoming_tls in D) & (id_outgoing_tls in D)
                    if (id_incoming_tls in D) & (id_outgoing_tls in D):

                        # avoid route with turnaround
                        if edges.ids_tonode[id_outgoing_tls] != edges.ids_fromnode[id_incoming_tls]:

                            duration_approx, route_raw = routing.get_mincostroute_edge2edge(
                                id_incoming_tls,
                                id_outgoing_tls,
                                D=D, P=P,
                            )
                            if len(route_raw) > 0:
                                # there is a bike accessible route between
                                # id_incoming_tls and id_outgoing_tls
                                # make also sure bike route is straight
                                # no green wave if bike route makes sharp turns
                                routes, routecosts = self.correct_tls_route(
                                    route_raw, nodes_tls, is_check_angle=True, angle_max=60.0/180*np.pi)
                                if len(routes) > 0:
                                    # there are one or several valid subroutes
                                    for route, routecost in zip(routes, routecosts):
                                        routes_tls.append(route)
                                        routes_tls_cost.append(routecost)
                                        ids_routeedge_tls.update(route)

            print '  withbike routes_tls', routes_tls

        ids_outgoing_tls_rest = ids_outgoing_tls+ids_outgoing_major_tls  # +ids_outgoing_bike_tls
        print '  create rest of routes for ids_incoming_tls', ids_incoming_tls, ids_outgoing_tls_rest
        for id_incoming_tls in ids_incoming_tls+ids_incoming_major_tls:  # +ids_incoming_bike_tls:

            D, P = routing.edgedijkstra(id_incoming_tls, ids_edge_target=set(ids_outgoing_tls_rest),
                                        weights=costs, fstar=fstar)

            for id_outgoing_tls in ids_outgoing_tls_rest:
                if not ((id_incoming_tls in ids_outgoing_major_tls) & (id_outgoing_tls in ids_outgoing_major_tls)):
                    # major incoming and outgoing already done with major routes

                    print '    rest route from id_incoming_tls,', id_incoming_tls, 'id_outgoing_tls', id_outgoing_tls
                    # avoid route with turnaround
                    if edges.ids_tonode[id_outgoing_tls] != edges.ids_fromnode[id_incoming_tls]:

                        duration_approx, route_raw = routing.get_mincostroute_edge2edge(
                            id_incoming_tls,
                            id_outgoing_tls,
                            D=D, P=P,
                        )

                        if len(route_raw) > 0:
                            # make sure route is in TLS and no U-turns
                            routes, routecosts = self.correct_tls_route(
                                route_raw, nodes_tls, is_check_angle=True, angle_max=90.0/180*np.pi)
                            if len(routes) > 0:
                                # there are one or several valid subroutes
                                for route, routecost in zip(routes, routecosts):
                                    routes_tls.append(route)
                                    routes_tls_cost.append(routecost)
                                    ids_routeedge_tls.update(route)

        print '\n  all routes_tls', routes_tls

        # add bicycle routes

        # print '\n  *inds_cons_conflict',inds_cons_conflict

        # print '  nodes_tls',nodes_tls.keys()
        # print '  ids_connodes',ids_connodes
        # print '  routes_tls_cost',routes_tls_cost
        # print '  np.argsort(routes_tls_cost)',np.argsort(routes_tls_cost)
        # print '  np.argsort(routes_tls_cost)',np.argsort(routes_tls_cost)[::-1]

        # determine routeconnectordata

        # containing the following data for each route:
        #  ind_route
        #  inds_con_route: used connector index vector
        #  prio,
        #  inds_cons_conflict_route: conflict connector index vector
        #  inds_cons_merge_route : merge connector index vector

        routeconnectordata = []
        for ind_route in np.argsort(routes_tls_cost)[::-1]:
            ids_edges = routes_tls[ind_route]
            routecost = routes_tls_cost[ind_route]
            #ids_edges = self.majorroutes[ind_route]
            #ids_edges = np.array(ids_edges_list, dtype = np.int32)
            print '  Determine connectors for route ind_route', ind_route, ids_edges
            # print '    ids_node',edges.ids_fromnode[ids_edges[:-1]]
            # print '    nodes_tls',nodes_tls.keys()
            # for id_edge, id_innode, id_outnode  in zip(ids_edges, edges.ids_fromnode[ids_edges], edges.ids_tonode[ids_edges]):

            # index vector with is True for all connectors used by this route
            inds_con_route = np.zeros(n_cons, dtype=np.bool)
            prio = -1
            #
            #ids_nodes = list(edges.ids_fromnode[ids_edges])+[edges.ids_tonode[ids_edges[-1]]]

            # print '  ids_nodes',ids_nodes
            # for id_node in ids_nodes:
            for id_edge_from,   id_edge_to, id_node in zip(ids_edges[:-1], ids_edges[1:],  edges.ids_tonode[ids_edges[:-1]]):
                # id_edge_from-> id_node -> id_edge_to

                if id_node in nodes_tls:
                    if not nodes_tls[id_node]['is_cycleped']:
                        # there should not be any cycleped node on the main route
                        prio_edge = edges.priorities[id_edge_from]  # could be averaged
                        if prio_edge > prio:
                            prio = prio_edge
                        # prio = routecost
                        #inds_con_route |= ids_connodes == id_node
                        inds_con_route |= (lanes.ids_edge[ids_fromlane_tls] == id_edge_from) & (
                            lanes.ids_edge[ids_tolane_tls] == id_edge_to)

                print '    id_node,prio', id_node, prio, id_node in nodes_tls, 'n_cons_route', len(
                    np.flatnonzero(inds_con_route))

            # make phase  for ids_edges
            if prio != -1:
                # at least one node of the route is controlled
                inds_cons_conflict_route = np.zeros(len(inds_con_route), dtype=np.bool)
                inds_cons_merge_route = np.zeros(len(inds_con_route), dtype=np.bool)
                ids_con_tls_route = ids_con_tls[inds_con_route]
                for id_con, ind_con in zip(ids_con_tls_route, np.flatnonzero(inds_con_route)):
                    inds_cons_conflict_route |= inds_cons_conflict[id_con] & np.logical_not(inds_cons_merge[id_con])
                    inds_cons_merge_route |= inds_cons_merge[id_con]
                print '      ids_con_alloc', ids_con_tls[inds_con_route]
                print '      ids_con_confl', ids_con_tls[inds_cons_conflict_route]
                print '      ids_con_merge', ids_con_tls[inds_cons_merge_route]
                routeconnectordata.append((ind_route, inds_con_route, prio,
                                           inds_cons_conflict_route, inds_cons_merge_route))

            # print '  phaseconflicts=\n',phaseconflicts
            # print '  phasepriorities=\n',phasepriorities

        # debug:
        for data in routeconnectordata:  # data is sorted by routecosts
            ind_tlsroute, inds_con_route, prio, inds_cons_conflict_route, inds_cons_merge_route = data

            print '  ind_tlsroute', ind_tlsroute, 'c=%.1f' % routes_tls_cost[ind_tlsroute], 'ids_edge', routes_tls[ind_tlsroute]

        # group non-conflicting routes
        n_routes = len(routeconnectordata)
        phasegroups = -1*np.ones(n_routes, dtype=np.int32)

        # connection index vector with True for each used connection in
        # of the routes in this group
        inds_alloc_groups = []

        # connection index vector with True for each connection in conflict
        # with routes in this group
        inds_confict_groups = []
        # connection index vector with True for each connection with a merge
        # with routes in this group
        inds_merge_groups = []

        prios_group = []

        i_group = 0
        i_route = 0
        while i_route < n_routes:
            print '  try i_route', i_route, phasegroups[i_route], phasegroups[i_route] == -1
            if phasegroups[i_route] == -1:

                phasegroups[i_route] = i_group
                ind_tlsroute, inds_con_route, prio, inds_cons_conflict_route, inds_cons_merge_route = routeconnectordata[
                    i_route]

                #inds_cons_conflict_group = inds_cons_conflict.copy()
                print '    Check group', i_group, 'for ind_tlsroute', ind_tlsroute, 'c=%.1f' % routes_tls_cost[
                    ind_tlsroute]

                # print '     inds_cons_conflict',inds_cons_conflict
                # check whether there are other, compatible
                j_route = i_route+1  # from 0..i_route route are already associated with groups
                inds_alloc_group = inds_con_route.copy()
                inds_confict_group = inds_cons_conflict_route.copy()
                inds_merge_group = inds_cons_merge_route.copy()
                prio_group = 1*prio
                print '            init ids_confict_group=', ids_con_tls[inds_confict_group]
                while j_route < n_routes:
                    if phasegroups[j_route] == -1:
                        # no phase group associated with
                        ind_tlsroute2, inds_con_route2, prio2, inds_cons_conflict_route2, inds_cons_merge_route2 = routeconnectordata[
                            j_route]
                        print '      check with ind_tlsroute2', ind_tlsroute2, np.any(
                            inds_cons_conflict_route2 & inds_con_route), np.any(inds_cons_conflict_route & inds_con_route2)
                        # print '        c',inds_con_route
                        # print '        x',inds_cons_conflict_route2
                        if (not np.any(inds_cons_conflict_route2 & inds_alloc_group)) & (not np.any(inds_confict_group & inds_con_route2)):
                            # no mutual conflict connections between route j_route
                            # and current route i_route

                            print '         ids_cons_conflict_route2', ids_con_tls[inds_cons_conflict_route2]
                            print '         inds_con_route          ', ids_con_tls[inds_con_route]
                            # make this route a part of the current group
                            phasegroups[j_route] = i_group
                            inds_confict_group |= inds_cons_conflict_route2
                            inds_merge_group |= inds_cons_merge_route2
                            inds_alloc_group |= inds_con_route2

                            if prio2 > prio_group:
                                prio_group = prio
                            print '           group', phasegroups[j_route], ':put route', ind_tlsroute2, 'to route', ind_tlsroute
                            print '             ids_confict_group=', ids_con_tls[inds_confict_group]

                    j_route += 1

                inds_confict_groups.append(inds_confict_group)
                inds_merge_groups.append(inds_merge_group)
                inds_alloc_groups.append(inds_alloc_group)
                prios_group.append(prio_group)
                # all other routes have been checked
                i_group += 1

            i_route += 1

        # debug:
        for i_group, data in zip(phasegroups, routeconnectordata):
            ind_tlsroute, inds_con_route, prio, inds_cons_conflict_route, inds_cons_merge_route = data
            print '  i_group', i_group, 'ind_tlsroute', ind_tlsroute, 'ids_edge', routes_tls[ind_tlsroute]

        # return
        # create signal phases
        # list with conflict info for each phase
        # conflict info is a vector with length of all connections
        # where 0 = no conflict, 1 = merge conflict, 2 = conflict
        phaseconflicts = []

        # list with all conflicting connections, including those
        # on short edges
        phaseblocks = []

        # list with connections for each phase that must be with green signal
        # 0 = no green signal required, 1 = green signal required
        phaseallocations = []

        # list with priority info for each phase
        # the priority of the connector equals to the priority
        # of the edge that is part of the respective route
        phasepriorities = []

        # print '\n  **inds_cons_conflict',inds_cons_conflict
        for i_group, inds_alloc_groups, inds_confict_groups, inds_merge_groups, prio_group in zip(xrange(max(phasegroups)+1), inds_alloc_groups, inds_confict_groups, inds_merge_groups, prios_group):
            print '    allocate slots for group', i_group
            phaseallocations.append(inds_alloc_groups)

            phaseconflicts.append(2*inds_confict_groups + 1 * (inds_merge_groups & np.logical_not(inds_confict_groups)))
            phasepriorities.append(prio_group)
            # self.add_timeslot(inds_alloc_groups,inds_confict_groups,inds_merge_groups,prio_group,
            #                        ids_con_tls, inds_cons_conflict,inds_cons_merge,
            #                        phaseallocations,phaseconflicts,phaseblocks, phasepriorities,
            #                        ids_fromlane_tls,
            #                        ids_tolane_tls,
            #                        are_enabeled
            #                        )

        n_phase = len(phaseconflicts)

        if n_phase < 2:
            print '  WARNING: a TLS with just one phase makes no sense. Abandone.'
            return

        if 1:  # debug
            print '    Show phases n_phase', n_phase
            for i, inds_phaseconflict, prio in zip(xrange(n_phase), phaseconflicts, phasepriorities):
                print '  phase', i, 'prio', prio
                #ids_con_conf_phase = ids_con_tls[inds_phaseconflict==2]
                # print '    ids_con_conf_phase',ids_con_conf_phase
                #ids_con_merge_phase = ids_con_tls[inds_phaseconflict==1]
                # print '  inds_phaseconflict',inds_phaseconflict,type(inds_phaseconflict)
                # print '  ids_connodes',ids_connodes[inds_phaseconflict>0]
                ids_node_phase = set(ids_connodes[inds_phaseconflict > 0])

                for id_node in ids_node_phase:
                    print '    id_node', id_node
                    print '      ids_conflicts', ids_con_tls[(inds_phaseconflict == 2) & (ids_connodes == id_node)]
                    print '      ids_merge', ids_con_tls[(inds_phaseconflict == 1) & (ids_connodes == id_node)]

        # make sure that all connetors between two edges have the equal signal
        for id_fromedge in set(ids_conedges_from):
            for id_toedge in set(ids_conedges_to):
                # extract index of connections, connecting id_fromedge with id_toedge
                inds_con_check = np.flatnonzero((ids_conedges_from == id_fromedge) & (ids_conedges_to == id_toedge))
                for i in xrange(n_phase):
                    # now if any of those connections is red than make them all red
                    if np.any(phaseconflicts[i][inds_con_check] == 2):
                        phaseconflicts[i][inds_con_check] = 2

        # sort phases in order to reduce number of signal changes
        if 1:
            n_phase = len(phaseconflicts)
            # for i,conficts,inds_alloc,prio in zip(xrange(n_phase),phaseconflicts,phaseallocations,phasepriorities):
            for i in np.arange(n_phase-1):
                conficts = phaseconflicts[i].copy()
                inds = np.arange(i+1, n_phase)
                deltas = np.zeros(len(inds), dtype=np.int32)
                k = 0
                for j in inds:
                    deltas[k] = np.sum(np.abs(conficts-phaseconflicts[j]))
                    k += 1
                ind_min = inds[np.argmin(deltas)]

                phaseconflicts.insert(i+1, phaseconflicts.pop(ind_min))
                phaseallocations.insert(i+1, phaseallocations.pop(ind_min))
                phasepriorities.insert(i+1, phasepriorities.pop(ind_min))

        # make nodes safe
        #
        if 1:
            # go though all nodes of this TLS
            for id_node, nodeattrs in nodes_tls.iteritems():
                print '    safecheck id_node', id_node, 'is_crossing', nodeattrs[
                    'is_crossing'], 'is_cycleped', nodeattrs['is_cycleped']

                #nodeattrs['is_cycleped'] = is_cycleped
                # nodeattrs['is_crossing'] =
                if nodeattrs['is_crossing']:
                    # junction is crossing between cycle ped and a road
                    # in this case there is only one incoming road edge
                    # and at least one cycleped edge

                    inds_con_node = (ids_connodes == id_node) & are_enabeled

                    # get all conflicting connectors of cycleped edges
                    inds_con_check = np.zeros(n_cons, dtype=np.bool)

                    for id_edge_from in nodeattrs['ids_cycleped_incoming']:  # nodes.ids_incoming[id_node]:
                        for id_edge_to in nodeattrs['ids_cycleped_outgoing']:
                            print '      safecheck cycleped from edge %d to edge %d id_edge' % (
                                id_edge_from, id_edge_to)

                            # connector index vector with all connectors for this edge

                            inds_con_edge = inds_con_node & (lanes.ids_edge[ids_fromlane_tls] == id_edge_from) & (
                                lanes.ids_edge[ids_tolane_tls] == id_edge_to)
                            ids_con_edge = ids_con_tls[inds_con_edge]
                            print '          ids_con_edge', ids_con_edge, np.any(inds_con_edge)
                            if np.any(inds_con_edge):
                                # get conflicts for these connections
                                for id_con, ind_con in zip(ids_con_edge, np.flatnonzero(inds_con_edge)):

                                    # if id_con is green, then these must be red in all phases
                                    #                    conflicts                valid           to connector does NOT point to toedge
                                    inds_con_check |= inds_cons_conflict[id_con] & are_enabeled & (
                                        lanes.ids_edge[ids_tolane_tls] != id_edge_to)
                                    print '          id_con %d: inds_con_check' % id_con, ids_con_tls[inds_con_check]

                    print '        ids_con_check', ids_con_tls[inds_con_check]

                    # check if there is a dedicated phase with assigned
                    # cycleped edges
                    prio_green = -1
                    for i, prio in zip(xrange(n_phase), phasepriorities):
                        is_alloc_allgreen = np.all(phaseallocations[i][inds_con_node])

                        if is_alloc_allgreen:
                            # cycleped path is allocated, so make all conflicts red
                            print '      found cycleped green phase', i

                            # make all conflicting red, but this should already
                            # be the case
                            phaseconflicts[i][inds_con_check] = 2
                            if (prio > prio_green):
                                prio_green = prio

                    if prio_green < 0:
                        # no dedicated cycleped phase found
                        # search suitable phase where conflicts are red
                        prio_red = -1
                        i_red = -1
                        for i, prio in zip(xrange(n_phase), phasepriorities):
                            is_conflict_allred = np.all(phaseconflicts[i][inds_con_check] == 2)
                            if is_conflict_allred & (prio > prio_red):
                                i_red = i
                                prio_red = prio

                        # react and make cycleped red (and leave others)
                        # and make cycleped green only in phase i_red
                        # when conflicts are all red
                        for i, prio in zip(xrange(n_phase), phasepriorities):
                            print '          adjust phase', i, 'green', i == i_red
                            if i == i_red:
                                phaseconflicts[i][inds_con_node] = 0
                            else:
                                phaseconflicts[i][inds_con_node] = 2

                elif not nodeattrs['is_cycleped']:
                    # safety check ALL nodes where motorized traffic is involved
                    # including crossings

                    # go through all incoming edges of this node
                    for id_edge in nodes.ids_incoming[id_node]:
                        print '      safecheck icoming id_edge', id_edge

                        # connector index vector with all connectors for this edge
                        inds_con_node = (ids_connodes == id_node) & are_enabeled
                        inds_con_node = inds_con_node & (lanes.ids_edge[ids_fromlane_tls] == id_edge)
                        ids_con_node = ids_con_tls[inds_con_node]

                        if np.any(inds_con_node):
                            # check if  these connections cause any conflict
                            #is_controlled = False
                            # for inds_phaseconflict in phaseconflicts:
                            #    is_controlled |= np.any(inds_con_node & (inds_phaseconflict>0))

                            #nodeattrs['is_controlled'] = is_controlled
                            # if is_controlled:
                            # go through all connectors of this edge in this node
                            for id_con, ind_con in zip(ids_con_node, np.flatnonzero(inds_con_node)):

                                # if id_con is green, then these must be red in all phases
                                inds_con_confcheck = inds_cons_conflict[id_con]
                                print '\n        safecheck id_con', id_con, 'ids_con_confcheck', ids_con_tls[inds_con_confcheck]

                                # go through all phases
                                for i, prio in zip(xrange(n_phase), phasepriorities):
                                    inds_noconfmerge = phaseconflicts[i] <= 1
                                    #inds_noconfmerge_last = phaseconflicts[i-1] <= 1

                                    inds_noconf = phaseconflicts[i] == 0
                                    inds_noconf_last = phaseconflicts[i-1] == 0
                                    print '          safecheck phase', i, 'inds_noconf', inds_noconf[
                                        ind_con], 'inds_noconf_last', inds_noconf_last[ind_con]

                                    if inds_noconfmerge[ind_con]:  # no red at ind_con

                                        # ambiguous connectors are where there are
                                        # no phase conflicts, where there should be
                                        # conflicts, in case ind_con is green

                                        # ambiguos connectors are
                                        # connectors without marked conflics
                                        # which should be in conflict with ind_con
                                        #inds_con_ambiqu = inds_noconf & (inds_con_confcheck>0) & are_enabeled

                                        # ambiguous cons = green cons inds & inds where cons should be red
                                        inds_con_ambiqu = inds_noconf & (inds_con_confcheck > 0) & are_enabeled
                                        print '            inds_con_ambiqu', ids_con_tls[inds_con_ambiqu], 'resolve ambig', (inds_noconf_last[ind_con]) & np.any(
                                            inds_con_ambiqu)

                                        # any ambiguous connector found?
                                        if np.any(inds_con_ambiqu):
                                            # resolve ambiguity by looking at last phase
                                            #inds_noconf_last = phaseconflicts[i-1] == 0
                                            if inds_noconf_last[ind_con]:
                                                # during the last phase ind_con
                                                # has been green and there are ambiguos conflicting connectors
                                                # if np.all(inds_noconf_last[inds_con_ambiqu]==0):
                                                # these should be blocked!!

                                                # due to false conflict recognition
                                                # of connectors, it can happen
                                                # that allocated connectors are in conflict
                                                if np.any(phaseallocations[i][inds_con_ambiqu]):
                                                    print '            set thiscon', id_con, 'to red, instead of allocated.'
                                                    phaseconflicts[i][ind_con] = 2
                                                else:
                                                    print '            set confcons', ids_con_tls[inds_con_ambiqu], 'to red'
                                                    phaseconflicts[i][inds_con_ambiqu] = 2

                                                # for ind_con_ambiqu, raw in zip(inds_con_ambiqu, phaseconflicts[i][inds_con_ambiqu]):
                                                #    self.block_connection(ind_con_ambiqu, 2 , phaseconflicts[i], ids_fromlane_tls, ids_tolane_tls, ids_con_tls,are_enabeled)
                                            else:
                                                # during the last phase this signal
                                                # has been red so let it red
                                                # these should be blocked!!
                                                print '            set thiscon', id_con, 'to red'
                                                phaseconflicts[i][ind_con] = 2
                                                #self.block_connection(ind_con, 2 , phaseconflicts[i], ids_fromlane_tls, ids_tolane_tls, ids_con_tls,are_enabeled)
                                        else:
                                            print '            no ambiguity.'
                                    else:
                                        print '            signal is already red. Try make it green'
                                        # make signal green, if signal has been green in the past and
                                        # if all conflicting connectors are red
                                        #inds_con_ambiqu = (phaseconflicts[i] == 2) & (inds_con_confcheck>0) & are_enabeled
                                        if np.all((phaseconflicts[i] == 2)[(inds_con_confcheck > 0) & are_enabeled]):
                                            phaseconflicts[i][ind_con] = 0

        if 0:  # debug
            print '  After safetycheck: Show phases n_phase', n_phase
            for i, inds_phaseconflict, prio in zip(xrange(n_phase), phaseconflicts, phasepriorities):
                print '   phase', i, 'prio', prio
                #ids_con_conf_phase = ids_con_tls[inds_phaseconflict==2]
                # print '    ids_con_conf_phase',ids_con_conf_phase
                #ids_con_merge_phase = ids_con_tls[inds_phaseconflict==1]
                # print '  inds_phaseconflict',inds_phaseconflict,type(inds_phaseconflict)
                # print '  ids_connodes',ids_connodes[inds_phaseconflict>0]
                ids_node_phase = set(ids_connodes[inds_phaseconflict > 0])

                for id_node in ids_node_phase:
                    print '    id_node', id_node
                    print '      ids_conflicts', ids_con_tls[(inds_phaseconflict == 2) & (ids_connodes == id_node)]
                    print '      ids_merge', ids_con_tls[(inds_phaseconflict == 1) & (ids_connodes == id_node)]

        # push red signs back over short edges
        if 1:
            n_phase = len(phaseconflicts)
            for i in np.arange(n_phase):
                slots = np.zeros(len(inds_con_route), dtype=np.int32)

                slots_raw = phaseconflicts[i]
                # index with all merge and red signals
                inds = np.flatnonzero(slots_raw == 2)
                print '\n      Phase', i, 'block ids_con', ids_con_tls[inds]
                # print '              red  ',ids_con_tls[phaseconflicts[i]==2]
                # print '              merge',ids_con_tls[phaseconflicts[i]==1]
                # go though all connectors with merge and red signals
                for ind_confl, slot_raw in zip(inds, slots_raw[inds]):
                    self.block_connection(ind_confl, slot_raw, slots, ids_fromlane_tls,
                                          ids_tolane_tls, ids_con_tls, are_enabeled)

                # update  phaseconflicts
                phaseconflicts[i] = slots

        if 1:  # debug
            print '  After push back reds: Show phases n_phase', n_phase
            for i, inds_phaseconflict, prio in zip(xrange(n_phase), phaseconflicts, phasepriorities):
                print '   phase', i, 'prio', prio
                #ids_con_conf_phase = ids_con_tls[inds_phaseconflict==2]
                # print '    ids_con_conf_phase',ids_con_conf_phase
                #ids_con_merge_phase = ids_con_tls[inds_phaseconflict==1]
                # print '  inds_phaseconflict',inds_phaseconflict,type(inds_phaseconflict)
                # print '  ids_connodes',ids_connodes[inds_phaseconflict>0]
                ids_node_phase = set(ids_connodes[inds_phaseconflict > 0])

                for id_node in ids_node_phase:
                    print '    id_node', id_node
                    print '      ids_conflicts', ids_con_tls[(inds_phaseconflict == 2) & (ids_connodes == id_node)]
                    print '      ids_merge', ids_con_tls[(inds_phaseconflict == 1) & (ids_connodes == id_node)]

        # create traffic light system in SUMO net
        n_phase = len(phaseconflicts)
        if n_phase <= 1:
            print '   Less than one phase => abbandon.'
            return False

        inds_con_controlled = np.zeros(n_cons, dtype=np.bool)
        inds_con_notls = np.zeros(n_cons, dtype=np.bool)
        inds_con_off = np.zeros(n_cons, dtype=np.bool)
        states = []
        ids_node_sumotls = []
        for id_node, nodeattrs in nodes_tls.iteritems():
            # if nodeattrs['is_crossing']
            # print '  tlscheck id_node',id_node,'is_cycleped',nodeattrs['is_cycleped']
            if not nodeattrs['is_cycleped']:
                inds_con_node = (ids_connodes == id_node) & are_enabeled
                # check if there are conflicting connections

                #inds_con_node = inds_con_node & (lanes.ids_edge[ids_fromlane_tls] == id_edge)
                #ids_con_node = ids_con_tls[inds_con_node]

                if np.any(inds_con_node):
                    # check if  conflicting connections
                    # are used in the varios phases
                    are_con_control = np.zeros(n_cons, dtype=np.bool)
                    are_con_awaysred = np.ones(n_cons, dtype=np.bool)
                    for inds_phaseconflict in phaseconflicts:
                        are_con_control |= inds_con_node & (inds_phaseconflict > 0)
                        are_con_awaysred &= inds_con_node & (inds_phaseconflict > 0)

                    if np.any(are_con_control):
                        # some nodes  are always red
                        # these nodes need to be removed
                        pass

                    if np.any(are_con_control):
                        # node does actually control traffic
                        ids_node_sumotls.append(id_node)
                        #ids_cons_sumotls += list(ids_con_tls[inds_con_node])
                        # print '    add controlled',ids_con_tls[inds_con_node & are_con_control]
                        inds_con_controlled |= inds_con_node & are_con_control

                        # put disabled connections to off
                        inds_con_off |= (ids_connodes == id_node) & np.logical_not(are_con_control)

                        # like this  we do not include disableled connectors
                        #inds_con_off |= inds_con_node & np.logical_not(are_con_control)

                    else:
                        # signals always green...remove connections of this node
                        # print '    add notls',ids_con_tls[ids_connodes == id_node]
                        inds_con_notls |= ids_connodes == id_node

                else:
                    # all connections are disabeled..remove connections of this node
                    # print '    add notls',ids_con_tls[ids_connodes == id_node]
                    inds_con_notls |= ids_connodes == id_node
            else:
                # print '    add notls',ids_con_tls[ids_connodes == id_node]
                inds_con_notls |= ids_connodes == id_node

        #inds_con_off = np.logical_not(inds_con_controlled) & np.logical_not(inds_con_notls)
        #inds_con_off &= are_enabeled
        # these are all enabeled controlled and uncontrolled connectors
        inds_con_sumotls = np.flatnonzero(inds_con_off | inds_con_controlled)
        ids_con_sumotls = ids_con_tls[inds_con_sumotls]

        print '  ids_con_notls', ids_con_tls[inds_con_notls]
        print '  are_enabeled', ids_con_tls[are_enabeled]
        print '  ids_con_controlled', ids_con_tls[inds_con_controlled]

        print '  ids_node_sumotls', ids_node_sumotls
        print '  ids_con_sumotls', ids_con_sumotls
        print '  ids_con_off', ids_con_tls[inds_con_off]

        #n_cons_sumotls = len(inds_con_sumotls)
        states = []
        weights = []
        for i, prio in zip(xrange(n_phase), phasepriorities):
            #state = np.zeros(n_cons_sumotls, dtype = np.int32)
            conflicts_last = phaseconflicts[i-1][inds_con_sumotls]
            conflicts = phaseconflicts[i][inds_con_sumotls]

            conflicts[inds_con_off[inds_con_sumotls]] = 4
            intermediate = (conflicts_last < 2) & (conflicts == 2)
            # print '    conf_last:',conflicts_last
            # print '    conf     :',conflicts
            # print '    inter    :',intermediate
            if np.any(intermediate):
                state = conflicts_last.copy()
                state[inds_con_off[inds_con_sumotls]] = 4
                state[intermediate] = 3
                states.append(state)
                weights.append(-1)  # means yellow phase

            states.append(conflicts.copy())
            weights.append(prio)

        signals = np.array(['G', 'g', 'r', 'y', 'O'], dtype=np.str)
        #
        states_sumotls = []
        for state in states:
            states_sumotls.append(string.join(signals[state], ''))
            print '    state=', states_sumotls[-1]

        id_tls = tlss.suggest_id()
        weights = np.array(weights, dtype=np.float32)
        durations = weights/np.sum(weights)*self.cycletime
        durations[weights == -1] = self.yellowtime
        durations = np.clip(durations, self.phasetime_min, self.phasetime_max)
        durations_min = durations
        durations_max = durations

        tlss.make('tlsgen_'+str(id_tls),  # id_sumo
                  id_prog=None,
                  ptype=self.ptype,  # "static": 1,"actuated": 2,
                  offset=self.offset,
                  durations=durations,
                  durations_min=durations_min,
                  durations_max=durations_max,
                  states=states_sumotls,
                  )
        # TODO: could be included in make
        tlss.set_connections(id_tls, ids_con_sumotls)

        # link controlled nodes to id_tls
        for id_node, nodeattrs in nodes_tls.iteritems():
            # define controlled connections
            # for id_con in ids_con_tls[(ids_connodes == id_node) & np.logical_not(are_enabeled)]:
            #    connections.are_uncontrolled[id_con] = id_con# not in ids_con_sumotls
            connections.are_uncontrolled[ids_con_tls[(ids_connodes == id_node) & inds_con_off]] = True
            if id_node in ids_node_sumotls:
                # it is actually a node with controlled connectors
                nodes.configure_tls(id_node, id_tls,  self.ptype)
                nodes.are_keep_clear[id_node] = False
            else:
                nodes.remove_tls(id_node)

        return True

    def init_slots(self,  inds_con_route, ids_con_tls,
                   inds_cons_conflict, inds_cons_merge,
                   ids_fromlane_tls, ids_tolane_tls, are_enabeled):

        slots = np.zeros(len(inds_con_route), dtype=np.int32)
        slots_blocked = np.zeros(len(inds_con_route), dtype=np.bool)
        ids_con_tls_route = ids_con_tls[inds_con_route]
        for id_con, ind_con in zip(ids_con_tls_route, np.flatnonzero(inds_con_route)):

            print '\ninit_slots for id_con', id_con  # ,ids_con_tls[ind_con],ind_con
            # print '  inds_cons_conflict\n',inds_cons_conflict[id_con]
            # print '  inds_cons_merge\n',inds_cons_merge[id_con]

            # if 0:
            #    slots = np.max([slots, 2*(inds_cons_conflict[id_con]&np.logical_not(inds_cons_merge[id_con])) + 1* inds_cons_merge[id_con]],0)
            # else:
            # print '  inds_cons_conflict[id_con]',inds_cons_conflict[id_con]
            # print '  inds_cons_merge[id_con]',inds_cons_merge[id_con]

            slots_raw = 2*(inds_cons_conflict[id_con] &
                           np.logical_not(inds_cons_merge[id_con])) + 1 * inds_cons_merge[id_con]
            inds = np.flatnonzero(slots_raw)
            print '    block', ids_con_tls[inds]
            for ind_confl, slot_raw in zip(inds, slots_raw[inds]):
                self.block_connection(ind_confl, slot_raw, slots, ids_fromlane_tls,
                                      ids_tolane_tls, ids_con_tls, are_enabeled)

            slots_blocked |= slots_raw | (slots > 0)
            #
            # print '  =>stopslots for cons\n',ids_con_tls[slots == 2]
            # print '  =>mergeslots for cons\n',ids_con_tls[slots == 1]

            # make sure the connector has not been self-blocked
            # this actuallly should not happen except in silly loops
            slots[ind_con] = 0
            # print '  id_con', id_con,ids_con_tls[ind_con],ind_con
            # print '  =>2stopslots for cons\n',ids_con_tls[slots == 2]
            # print '  =>2mergeslots for cons\n',ids_con_tls[slots == 1]

        # no! there may be allocations later, do not block them
        # for id_con_check, inds_con_conf_check in inds_cons_conflict.iteritems():
        #    if id_con_check not in ids_con_tls_route:
        #        inds_con_crit = inds_con_conf_check & np.logical_not(slots_blocked>0)
        #        inds_con_crit = inds_con_crit & are_enabeled
        #
        print '  =>3stopslots for cons\n', ids_con_tls[slots == 2]
        print '  =>3mergeslots for cons\n', ids_con_tls[slots == 1]

        return slots, slots_blocked

    def block_connection(self, ind_con_block, slot_raw, slots, ids_fromlane_tls, ids_tolane_tls, ids_con_tls, are_enabeled):

        # ind_con is connector index that should be blocked

        # slot_raw is the current signal

        # slot is index vector where -1 means that respective connector has been
        # visited
        lanes = self.parent.lanes
        edges = self.parent.edges

        id_fromlane = ids_fromlane_tls[ind_con_block]
        is_disabled = not are_enabeled[ind_con_block]
        print 'block_connection id %d id_fromlane %d' % (ids_con_tls[ind_con_block], id_fromlane), 'nocycleped', lanes.ids_mode[ids_fromlane_tls[
            ind_con_block]] not in self.ids_cycleped, 'slot', slots[ind_con_block], 'L=%dm' % edges.lengths[lanes.ids_edge[id_fromlane]],

        if (slots[ind_con_block] > 0) | is_disabled:
            # ind_con_block has already been tuned to red
            print '*'
            return

        # do not block accross cycleped edges
        if lanes.ids_mode[ids_fromlane_tls[ind_con_block]] not in self.ids_cycleped:
            # to lane is a normal road
            if edges.lengths[lanes.ids_edge[id_fromlane]] < self.length_min_notls:
                # id_fromlane is a short lane.

                # get all connectors entering this shortlane
                inds_con = np.flatnonzero((ids_tolane_tls == id_fromlane))

                # backtrack only if there are connectors and if
                # all connectors have not been visited
                if (len(inds_con) > 0) & np.all(slots[inds_con] >= 0):
                    slots[ind_con_block] = -1  # sign as visited
                    print
                    print '->', ids_con_tls[inds_con]

                    for ind_con in inds_con:
                        self.block_connection(ind_con, 2, slots, ids_fromlane_tls,
                                              ids_tolane_tls, ids_con_tls, are_enabeled)
                    print '.'
                    slots[ind_con_block] = 0  # remove sign as visited

                else:
                    # from lane is connected from outside the TLS:
                    slots[ind_con_block] = 2  # put red light
                    print '-r'
            else:
                # road length acceptable
                slots[ind_con_block] = slot_raw
                print '-s', slot_raw

        else:
            # to lane is bikeway or footpath
            slots[ind_con_block] = slot_raw
            print '-b', slot_raw

        # print '    stopslots %d for cons\n'%ids_con_tls[ind_con_block],ids_con_tls[slots == 2]
        # print '    mergeslots %d for cons\n'%ids_con_tls[ind_con_block],ids_con_tls[slots == 1]

    def add_timeslot(self, inds_con_route, inds_confict_groups, inds_merge_groups, prio,
                     ids_con_tls, inds_cons_conflict, inds_cons_merge,
                     phaseallocations, phaseconflicts, phaseblocks, phasepriorities,
                     ids_fromlane_tls,
                     ids_tolane_tls,
                     are_enabeled):
        """
        Allocates a phase for a particular route though the TLS.
        The route is represented by the connector indexes along the route.
        """
        print '\nadd_timeslot for', ids_con_tls[inds_con_route]
        # go through all connections used by this route and
        # signalize all conflicts
        # for id_con in ids_con_tls[inds_con_route]:
        #    #inds_con_conflict = inds_cons_conflict[ids_con]
        #    print '  check id_con',id_con

        slots, slots_blocked = self.init_slots(inds_con_route,  # all used connections
                                               ids_con_tls,
                                               inds_cons_conflict, inds_cons_merge,
                                               ids_fromlane_tls,
                                               ids_tolane_tls,
                                               are_enabeled)
        n_slots = len(np.flatnonzero(slots))
        print '    n_slots', n_slots, 'n_phases', len(phaseconflicts)

        if n_slots == 0:
            print '  no  conflicts detected'
            pass

        elif len(phaseconflicts) == 0:
            print '  append first phase'
            phaseallocations.append(inds_con_route)
            phaseconflicts.append(slots)
            phaseblocks.append(slots_blocked)
            phasepriorities.append(prio)
        else:
            print '  search phase with minimum signal difference n_phases=', len(phaseallocations)
            n_diff_min = 10**8
            i_phase = 0
            i_phase_min = -1
            for inds_phaseallocation, inds_phaseconflict in zip(phaseallocations, phaseconflicts):
                # check if slots overlap with allocated connections this phase
                print '    compare phase', i_phase
                print '      new  allocations', ids_con_tls[inds_con_route]
                print '      phaseallocations', ids_con_tls[inds_phaseallocation]
                print '      new    conflicts', ids_con_tls[slots == 2]
                print '      phase  conflicts', ids_con_tls[inds_phaseconflict == 2]
                # print '      allocations',ids_con_tls[inds_phaseallocation]
                print '      n_diff =', np.sum(np.any((slots == 2) & inds_phaseallocation))

                if not np.any((slots == 2) & inds_phaseallocation):
                    print '    no conflict in this phase go for a merge'
                    i_phase_min = i_phase
                    n_diff_min = -1  # indicate phase merge

                else:
                    print '    there are conflicts with this phase...count'
                    n_diff = np.sum(np.any((slots == 2) & inds_phaseallocation))
                    #n_diff = np.sum(np.abs(slots - inds_phaseconflict)!=0)
                    # print '    ',inds_phaseconflict,n_diff
                    if n_diff < n_diff_min:
                        n_diff_min = n_diff
                        i_phase_min = i_phase

                i_phase += 1

            print '    finished comparing phases i_phase_min,n_diff_min', i_phase_min, n_diff_min

            if n_diff_min == 0:
                print '  already a phase with suitable signalling, nothing to do'
                pass

            elif n_diff_min == -1:
                print '  there are no phase conflicts, so merge'
                phaseallocations[i_phase_min] = phaseallocations[i_phase_min] | inds_con_route
                phaseconflicts[i_phase_min] = np.max([slots, phaseconflicts[i_phase_min]], 0)
                phaseblocks[i_phase_min] = phaseblocks[i_phase_min] | slots_blocked
                phasepriorities[i_phase_min] = max(prio, phasepriorities[i_phase_min])

            # elif n_diff_min>10**7:
            #    # all existing phases are conflicting

            else:
                # get number of cons which are more restrictive signals
                n_diff_pos = np.sum((slots - phaseconflicts[i_phase_min]) > 0)

                # get number of cons which are less restrictive signals
                n_diff_neg = np.sum((slots - phaseconflicts[i_phase_min]) < 0)

                print '  n_diff_min', n_diff_min, 'n_diff_pos', n_diff_pos, 'n_diff_neg', n_diff_neg, 'i_phase_min', i_phase_min
                # print '  inds_phaseconflict_min',ids_con_tls[phaseconflicts[i_phase_min] >0]
                # print '  inds_phaseconflict',ids_con_tls[slots>0]
                #inds_diff = np.abs(slots - inds_phaseconflict)!=0
                # if (n_diff_pos>0) & (n_diff_neg == 0):
                if (n_diff_pos >= n_diff_neg) & (n_diff_pos <= 2):
                    # only more restrictive
                    print '    put new phase after the phase with minimum difference'
                    phaseallocations.insert(i_phase_min+1, inds_con_route)
                    phaseconflicts.insert(i_phase_min+1, slots)
                    phaseblocks.insert(i_phase_min+1, slots_blocked)
                    phasepriorities.insert(i_phase_min+1, prio)

                # elif (n_diff_pos==0) & (n_diff_neg > 0):
                if (n_diff_pos < n_diff_neg) & (n_diff_neg <= 2):
                    # only less restrictive
                    print '    put new phase before the phase with minimum difference'
                    phaseallocations.insert(i_phase_min, inds_con_route)
                    phaseconflicts.insert(i_phase_min, slots)
                    phaseblocks.insert(i_phase_min, slots_blocked)
                    phasepriorities.insert(i_phase_min, prio)

                else:
                    # mixed changes
                    print '    append en entirely new phase'
                    phaseconflicts.append(slots)
                    phaseblocks.append(slots_blocked)
                    phasepriorities.append(prio)

    def init_tlsnode(self, id_node, nodes_tls):
        print 'init_tlsnode', id_node, 'is in TLS'
        nodeattrs = {}
        if id_node in self.ids_majornodes:
            self.ids_majornodes_used.add(id_node)
            nodeattrs['is_major'] = True
        else:
            nodeattrs['is_major'] = False

        nodeattrs['is_cycleped'] = False
        nodeattrs['is_crossing'] = False
        nodeattrs['ids_cycleped_incoming'] = []
        nodeattrs['ids_cycleped_outgoing'] = []
        nodes_tls[id_node] = nodeattrs

    def find_neighbour_nodes(self, id_node, dist, nodes_tls, edges, nodes):
        print 'find_neighbour_nodes', id_node, dist
        dist_tls_max = self.dist_tls_max
        #ids_edge = nodes.ids_outgoing[id_node]

        ids_edge = nodes.ids_incoming[id_node]
        # print '  ids_edge',ids_edge
        if ids_edge is not None:

            if len(ids_edge) == 1:
                ids_edge_out = nodes.ids_outgoing[id_node]
                if len(ids_edge_out) == 1:
                    # one incoming one outgoing only, check if bidir link
                    if edges.ids_fromnode[ids_edge[0]] == edges.ids_tonode[ids_edge_out[0]]:
                        print '  it is a turnaround only node -> remove node', id_node
                        del nodes_tls[id_node]
                        return
            elif len(ids_edge) == 0:  # no input??
                print '  no input -> remove node', id_node
                del nodes_tls[id_node]
                return

            for id_edge, length, id_edgenode in zip(ids_edge, edges.lengths[ids_edge], edges.ids_fromnode[ids_edge]):
                print '    check in id_edge', id_edge, dist + \
                    length, dist_tls_max, (dist+length < dist_tls_max), (length < self.dist_tls_internode_max)
                if (dist+length < dist_tls_max) & (length < self.dist_tls_internode_max):
                    if id_edgenode not in nodes_tls:
                        self.init_tlsnode(id_edgenode, nodes_tls)
                        self.find_neighbour_nodes(id_edgenode, dist+length, nodes_tls, edges, nodes)
        else:
            # node without incoming??
            print '  node without incoming -> remove node', id_node
            del nodes_tls[id_node]
            return

        ids_edge = nodes.ids_outgoing[id_node]
        if ids_edge is not None:
            if len(ids_edge) == 0:  # no exit
                del nodes_tls[id_node]
                return

            for id_edge, length, id_edgenode in zip(ids_edge, edges.lengths[ids_edge], edges.ids_tonode[ids_edge]):
                # print '    check out id_edge',id_edge,length,(dist+length < dist_tls_max),(length < self.dist_tls_internode_max)
                if (dist+length < dist_tls_max) & (length < self.dist_tls_internode_max):
                    if id_edgenode not in nodes_tls:
                        self.init_tlsnode(id_edgenode, nodes_tls)
                        self.find_neighbour_nodes(id_edgenode, dist+length, nodes_tls, edges, nodes)
        else:
            # node without incoming??
            del nodes_tls[id_node]
            return

    def correct_tls_route(self, route, nodes_tls, is_check_angle=True,
                          angle_max=60.0/180*np.pi, n_edges_min=2):
        """
        Corrects the route crossing a TLS.
        The method makes sure
        that the route rameins within the TLS,
        that no turnarounds are used,
        and for not major routes that the route follows a straight line.
        """
        print 'correct_tls_route', route
        net = self.parent
        edges = net.edges
        #ids_edges_major = self.ids_edges_major
        #route = []
        # ids_edges_major.add(id_edge_next)

        if len(route) == 0:
            return []

        if not is_check_angle:
            angle_max = np.inf

        # determine direction vector of end of edge shape
        shape = edges.shapes[route[0]]
        p01 = shape[-2][:2]
        p11 = shape[-1][:2]
        dir1 = p11-p01

        # go though rest of edges
        ids_edge = route[1:]
        ind_route = 1

        # final result is a list collecting feasible routes
        newroutes = []
        routecosts = []
        #routecost = 0.0
        newroute = [route[0], ]
        for id_edge, id_sumo, type_spread, shape_curr, \
                n_lane, ids_lane, priority, id_fromnode, id_tonode, speed_max\
            in zip(ids_edge,\
               edges.ids_sumo[ids_edge],
               edges.types_spread[ids_edge],
               edges.shapes[ids_edge],
               edges.nums_lanes[ids_edge],
               edges.ids_lanes[ids_edge],
               edges.priorities[ids_edge],
               edges.ids_fromnode[ids_edge],
               edges.ids_tonode[ids_edge],
               edges.speeds_max[ids_edge],
               ):

                # determine direction vector of start of current edge shape
            p02 = shape_curr[0][:2]
            p12 = shape_curr[1][:2]
            dir2 = p12-p02

            # determine angle change
            angle = get_diff_angle_clockwise(dir1, dir2)
            angle_abs = min(angle, 2*np.pi-angle)
            print '      id_edge:%d, angle_abs:%.1f, angle_max: %.1f' % (
                id_edge, angle_abs/np.pi*180, angle_max/np.pi*180), id_fromnode in nodes_tls, (angle_abs < angle_max)
            if id_fromnode in nodes_tls:
                    # route is still in TLS
                p01 = shape_curr[-2][:2]
                p11 = shape_curr[-1][:2]
                dir1 = p11-p01
                if (angle_abs < angle_max):
                        # route is straigt enough
                    ind_route += 1
                    # determine direction vector of end of current edge shape

                    newroute.append(id_edge)

                else:
                    # route is not straight and will be split.
                    # put old route in list and start a new route
                    if len(newroute) >= n_edges_min:
                                # route should contain at least n_edges_min edges
                        newroutes.append(newroute)
                        routecosts.append(self.get_routecost(newroute))

                    # attention, we need the last edge of the old route
                    # in order to include the connector between both routes
                    newroute = [newroute[-1], id_edge, ]
            else:
                # route is using an edge outside the TLS
                # even thou the route may return into the TLS
                # it will be by no means a straight route worth following
                # so better eliminate and return nothing
                return [], []

        if len(newroute) >= n_edges_min:
            # route should contain at least n_edges_min edges
            newroutes.append(newroute)
            routecosts.append(self.get_routecost(newroute))

        print '    ind_route', ind_route, len(route), 'newroutes', newroutes, 'routecosts', routecosts
        return newroutes, routecosts

    def follow_major_route_backward(self, id_edge_next, shape):
        print 'follow_major_route_backward', id_edge_next
        net = self.parent
        edges = net.edges
        ids_edges_major = self.ids_edges_major
        route = []
        ids_edges_major.add(id_edge_next)

        angle_max = 45.0/180*np.pi

        p01 = shape[0][:2]
        p11 = shape[1][:2]
        dir1 = p11-p01

        #v1 = np.concatenate(((x2-x1).reshape(1,-1),(y2-y1).reshape(1,-1)),0)
        #v2 = np.concatenate(((p[0]-x1).reshape(1,-1),(p[1]-y1).reshape(1,-1)),0)
        #angles = get_diff_angle_clockwise(v1,v2)
        is_follow = True
        while is_follow:
            ids_edge = edges.get_incoming(id_edge_next)
            # print '  id_edge_next',id_edge_next,'=>',ids_edge
            ids_edge_eligible = []
            angles_eligible = []
            shapes_eligible = []
            nums_lane_eligible = []
            for id_edge, id_sumo, type_spread, shape, \
                n_lane, ids_lane, priority, id_fromnode, id_tonode, speed_max\
                in zip(ids_edge,
                       edges.ids_sumo[ids_edge],
                       edges.types_spread[ids_edge],
                       edges.shapes[ids_edge],
                       edges.nums_lanes[ids_edge],
                       edges.ids_lanes[ids_edge],
                       edges.priorities[ids_edge],
                       edges.ids_fromnode[ids_edge],
                       edges.ids_tonode[ids_edge],
                       edges.speeds_max[ids_edge],
                       ):

                # print '    check next',id_edge,'major',self.is_major_road( priority,n_lane,speed_max),'not used',(id_edge not in ids_edges_major)
                if self.is_major_road(priority, n_lane, speed_max) & (id_edge not in ids_edges_major):
                    p02 = shape[-2][:2]
                    p12 = shape[-1][:2]
                    dir2 = p12-p02
                    angle = get_diff_angle_clockwise(dir1, dir2)
                    angle_abs = min(angle, 2*np.pi-angle)
                    edgedirection2 = shape[:2]
                    # print '      |angle|=%d'%(angle_abs/np.pi*180),angle/np.pi*180,dir1,dir2

                    if angle_abs < angle_max:
                        if id_edge not in self.ids_edges_major:
                            # print '      choose',id_edge
                            ids_edge_eligible.append(id_edge)
                            angles_eligible.append(angle_abs)
                            shapes_eligible.append(shape)
                            nums_lane_eligible.append(n_lane)

            n_eligible = len(ids_edge_eligible)

            if n_eligible == 0:
                is_follow = False

            else:
                if n_eligible == 1:
                    id_edge_next = ids_edge_eligible[0]
                    shape_next = shapes_eligible[0]

                elif n_eligible > 1:
                    # search edge with maximum number of lanes
                    n_lane_max = max(nums_lane_eligible)
                    inds_lane = np.flatnonzero(np.array(nums_lane_eligible) == n_lane_max)
                    if len(inds_lane) == 1:
                        # unique maximum
                        id_edge_next = ids_edge_eligible[inds_lane[0]]
                        shape_next = shapes_eligible[inds_lane[0]]
                    else:
                        # multiple edges have maximum number of lanes
                        angles_eligible = np.array(angles_eligible, dtype=np.float32)
                        ind_angle = np.argmin(angles_eligible[inds_lane])
                        id_edge_next = np.array(ids_edge_eligible, dtype=np.int32)[inds_lane][ind_angle]
                        shape_next = np.array(shapes_eligible, dtype=np.object)[inds_lane][inds_lane[0]]

                p01 = shape_next[0][:2]
                p11 = shape_next[1][:2]
                dir1 = p11-p01
                # print '    **winner:',id_edge_next,'dir1=',dir1
                route.append(id_edge_next)
                ids_edges_major.add(id_edge_next)

                is_follow = True

        # print '  backward route:',route
        # print
        return route

    def follow_major_route_foreward(self, id_edge_next, shape):
        print 'follow_major_route_foreward', id_edge_next
        net = self.parent
        edges = net.edges
        ids_edges_major = self.ids_edges_major
        route = []
        ids_edges_major.add(id_edge_next)

        angle_max = 45.0/180*np.pi

        p01 = shape[-2][:2]
        p11 = shape[-1][:2]
        dir1 = p11-p01

        #v1 = np.concatenate(((x2-x1).reshape(1,-1),(y2-y1).reshape(1,-1)),0)
        #v2 = np.concatenate(((p[0]-x1).reshape(1,-1),(p[1]-y1).reshape(1,-1)),0)
        #angles = get_diff_angle_clockwise(v1,v2)
        is_follow = True
        while is_follow:
            ids_edge = edges.get_outgoing(id_edge_next)
            # print '  id_edge_next',id_edge_next,'=>',ids_edge
            ids_edge_eligible = []
            angles_eligible = []
            shapes_eligible = []
            nums_lane_eligible = []
            for id_edge, id_sumo, type_spread, shape, \
                n_lane, ids_lane, priority, id_fromnode, id_tonode, speed_max\
                in zip(ids_edge,
                       edges.ids_sumo[ids_edge],
                       edges.types_spread[ids_edge],
                       edges.shapes[ids_edge],
                       edges.nums_lanes[ids_edge],
                       edges.ids_lanes[ids_edge],
                       edges.priorities[ids_edge],
                       edges.ids_fromnode[ids_edge],
                       edges.ids_tonode[ids_edge],
                       edges.speeds_max[ids_edge],
                       ):

                # print '    check next',id_edge,'major',self.is_major_road( priority,n_lane,speed_max),'not used',(id_edge not in ids_edges_major)
                if self.is_major_road(priority, n_lane, speed_max) & (id_edge not in ids_edges_major):
                    p02 = shape[0][:2]
                    p12 = shape[1][:2]
                    dir2 = p12-p02
                    angle = get_diff_angle_clockwise(dir1, dir2)
                    angle_abs = min(angle, 2*np.pi-angle)
                    edgedirection2 = shape[:2]
                    print '      |angle|=%d' % (angle_abs/np.pi*180), angle/np.pi*180, dir1, dir2

                    if angle_abs < angle_max:
                        if id_edge not in self.ids_edges_major:
                            # print '      choose',id_edge
                            ids_edge_eligible.append(id_edge)
                            angles_eligible.append(angle_abs)
                            shapes_eligible.append(shape)
                            nums_lane_eligible.append(n_lane)

            n_eligible = len(ids_edge_eligible)

            if n_eligible == 0:
                is_follow = False

            else:
                if n_eligible == 1:
                    id_edge_next = ids_edge_eligible[0]
                    shape_next = shapes_eligible[0]

                elif n_eligible > 1:
                    # search edge with maximum number of lanes
                    n_lane_max = max(nums_lane_eligible)
                    inds_lane = np.flatnonzero(np.array(nums_lane_eligible) == n_lane_max)
                    # print '      nums_lane_eligible,inds_lane',nums_lane_eligible,inds_lane,type(inds_lane),inds_lane.dtype
                    if len(inds_lane) == 1:
                        # unique maximum
                        id_edge_next = ids_edge_eligible[inds_lane[0]]
                        shape_next = shapes_eligible[inds_lane[0]]
                    else:
                        # multiple edges have maximum number of lanes
                        angles_eligible = np.array(angles_eligible, dtype=np.float32)
                        # print '        angles_eligible',angles_eligible,angles_eligible[inds_lane]
                        ind_angle = np.argmin(angles_eligible[inds_lane])
                        # print '        ind_angle',ind_angle,ids_edge_eligible[inds_lane].shape
                        id_edge_next = np.array(ids_edge_eligible, dtype=np.int32)[inds_lane][ind_angle]
                        shape_next = np.array(shapes_eligible, dtype=np.object)[inds_lane][inds_lane[0]]

                p01 = shape_next[-2][:2]
                p11 = shape_next[-1][:2]
                dir1 = p11-p01
                print '    **winner:', id_edge_next, 'dir1=', dir1
                route.append(id_edge_next)
                ids_edges_major.add(id_edge_next)

                is_follow = True

        # print '  foreward route:',route
        # print
        return route


class BikenetworkCompleter(Process):
    def __init__(self,  net, logger=None, **kwargs):
        print 'Bikenetworkcompleter.__init__'
        self._init_common('bikenetworkcompleter',
                          parent=net,
                          name='Bike network completer',
                          logger=logger,
                          info='Modifies the current network as to allow a higher permeability for bicycles.',
                          )

        attrsman = self.set_attrsman(cm.Attrsman(self))

        self.speed_max_bike = attrsman.add(cm.AttrConf('speed_max_bike', kwargs.get('speed_max_bike', 20/3.6),
                                                       groupnames=['options'],
                                                       perm='rw',
                                                       name='Bike speed limit',
                                                       unit='m/s',
                                                       info='General speed limit applied to edges reserved for bikes only.',
                                                       ))

        self.is_bike_on_ped = attrsman.add(cm.AttrConf('is_bike_on_ped', kwargs.get('is_bike_on_ped', False),
                                                       groupnames=['options'],
                                                       perm='rw',
                                                       name='Bikes on pedestrain ways',
                                                       info='If true, bikeways are also given access on links where only pedestrians are allowed. Furthermore an opposite edge is created if pedestrian link in one-way.',
                                                       ))

        self.is_bike_opp = attrsman.add(cm.AttrConf('is_bike_opp', kwargs.get('is_bike_opp', False),
                                                    groupnames=['options'],
                                                    perm='rw',
                                                    name='Bikes in opposite direction',
                                                    info='If true, bikeways are created in opposite direction of each one-way edges.',
                                                    ))

        self.width_bikelane_opp = attrsman.add(cm.AttrConf('width_bikelane_opp', kwargs.get('width_bikelane_opp', 0.9),
                                                           groupnames=['options'],
                                                           perm='rw',
                                                           name='Bikelane width opp. dir',
                                                           unit='m/s',
                                                           info='Width for created bike lines into the opposite direction of one-way edges.',
                                                           ))

        self.speed_max_bike_opp = attrsman.add(cm.AttrConf('speed_max_bike_opp', kwargs.get('speed_max_bike_opp', 8/3.6),
                                                           groupnames=['options'],
                                                           perm='rw',
                                                           name='Bike speed limit in opp. dir',
                                                           unit='m/s',
                                                           info='General speed limit applied to edges reserved for bikes only and which go into the opposite direction of one-way edges.',
                                                           ))

        self.priority_max = attrsman.add(cm.AttrConf('priority_max', kwargs.get('priority_max', 5),
                                                     groupnames=['options'],
                                                     perm='rw',
                                                     name='Max priority',
                                                     info='Operate only on edges up to this level of priority (1 is lowest 10 is highest).',
                                                     ))

        self.n_lanes_max = attrsman.add(cm.AttrConf('n_lanes_max', kwargs.get('n_lanes_max', 3),
                                                    groupnames=['options'],
                                                    perm='rw',
                                                    name='Max lanes',
                                                    info='Operate only on edges up to this number of lanes. Note that footpath is also a lane.',
                                                    ))

        self.id_mode_bike = MODES["bicycle"]
        self.id_mode_ped = MODES["pedestrian"]
        #self.id_mode_ped = MODES["delivery"]

        self.ids_modes_tocomplete = set([MODES["pedestrian"], MODES["delivery"], MODES["bus"]])

    def do(self):
        print 'BikenetworkCompleter.do'
        edges = self.parent.edges
        nodes = self.parent.nodes
        lanes = self.parent.lanes
        connections = self.parent.connections
        ids_edge = edges.get_ids()
        allow_cycloped = [self.id_mode_bike, self.id_mode_ped]
        ids_edge_update = []

        for id_edge, id_sumo, type_spread, shape, \
            n_lanes, ids_lane, priority, id_fromnode, id_tonode\
            in zip(ids_edge,
                   edges.ids_sumo[ids_edge],
                   edges.types_spread[ids_edge],
                   edges.shapes[ids_edge],
                   edges.nums_lanes[ids_edge],
                   edges.ids_lanes[ids_edge],
                   edges.priorities[ids_edge],
                   edges.ids_fromnode[ids_edge],
                   edges.ids_tonode[ids_edge],
                   ):

            if (n_lanes <= self.n_lanes_max) & (priority <= self.priority_max):
                # a footpath ha been made accessible for bikes
                # check if footpath is a on-way
                ids_incoming = nodes.ids_incoming[id_fromnode]
                ids_outgoing = nodes.ids_outgoing[id_tonode]

                if (ids_incoming is None) | (ids_outgoing is None):
                    is_oneway = True
                else:
                    is_oneway = set(ids_incoming).isdisjoint(ids_outgoing)

                # print '  check:id_edge=',id_edge,'ids_lane=',ids_lane,'is_oneway',is_oneway
                if n_lanes == 1:
                    id_lane = ids_lane[0]

                    is_bikeaccess, is_bikeonlyaccess = self._detect_bikeaccess(id_lane, lanes)
                    # print '  is_bikeaccess',is_bikeaccess, is_bikeonlyaccess
                    if (self.is_bike_on_ped) & (not is_bikeaccess):
                        ids_modes_allow, lanewidths = self._make_bike_on_ped(id_lane, lanes)
                        # print '  ids_modes_allow, lanewidths',ids_modes_allow, lanewidths
                        # print '  ids_incoming',nodes.ids_incoming[id_fromnode]
                        # print '  ids_outgoing',nodes.ids_outgoing[id_tonode]

                        if ids_modes_allow is not None:
                            ids_edge_update.append(id_edge)

                            if is_oneway:  # slow: edges.is_oneway(id_edge):
                                    # print '    add opposite edge with same properties ids_modes_allow',ids_modes_allow
                                edges.types_spread[id_edge] = 0  # right spread
                                #edges.widths[id_edge] = 0.5*lanewidths
                                lanes.widths[id_lane] = 0.5*lanewidths
                                id_edge_opp = edges.make(id_fromnode=id_tonode,
                                                         id_tonode=id_fromnode,
                                                         id_sumo='-'+id_sumo,
                                                         type_edge='',
                                                         num_lanes=1,
                                                         speed_max=self.speed_max_bike,
                                                         priority=priority,
                                                         #length = 0.0,
                                                         shape=shape[::-1],
                                                         type_spread='right',
                                                         #name = '',
                                                         #offset_end = 0.0,
                                                         width_lanes_default=0.5*lanewidths,
                                                         width_sidewalk=-1,
                                                         )
                                id_lane_opp = lanes.make(id_edge=id_edge_opp,
                                                         index=0,
                                                         width=0.5*lanewidths,
                                                         speed_max=self.speed_max_bike,
                                                         ids_modes_allow=ids_modes_allow)

                                edges.ids_lanes[id_edge_opp] = [id_lane_opp]
                                ids_edge_update.append(id_edge_opp)
                                is_oneway = False  # avoid producing another

                    if self.is_bike_opp & is_oneway & (not is_bikeonlyaccess):
                        # create opposite lane with narrow bikelane
                        # but not for bike-only lanes
                        # print '    add opposite edge'
                        edges.types_spread[id_edge] = 0  # right spread
                        edgewidth = edges.widths[id_edge]-self.width_bikelane_opp
                        edges.widths[id_edge] = edgewidth
                        lanes.widths[id_lane] = edgewidth
                        id_edge_opp = edges.make(id_fromnode=id_tonode,
                                                 id_tonode=id_fromnode,
                                                 id_sumo='-'+id_sumo,
                                                 type_edge='',
                                                 num_lanes=1,
                                                 speed_max=self.speed_max_bike_opp,
                                                 priority=0,  # give lowest priority...it's illegal!!
                                                 #length = 0.0,
                                                 shape=shape[::-1],
                                                 type_spread='right',
                                                 #name = '',
                                                 #offset_end = 0.0,
                                                 width_lanes_default=self.width_bikelane_opp,
                                                 width_sidewalk=-1,
                                                 )

                        id_lane_opp = lanes.make(id_edge=id_edge_opp,
                                                 index=0,
                                                 width=self.width_bikelane_opp,
                                                 speed_max=self.speed_max_bike_opp,
                                                 ids_modes_allow=allow_cycloped,
                                                 )

                        edges.ids_lanes[id_edge_opp] = [id_lane_opp]
                        ids_edge_update.append(id_edge_opp)

                elif n_lanes in [2, 3]:
                    is_opp = False
                    if self.is_bike_opp & is_oneway:
                        if n_lanes == 2:
                            # only if rightmost is footpath
                            is_opp = self._detect_pedonlyaccess(ids_lane[0], lanes)
                        elif n_lanes == 3:
                            # only if footpath left and right
                            is_opp = self._detect_pedonlyaccess(ids_lane[0], lanes)\
                                & self._detect_pedonlyaccess(ids_lane[2], lanes)

                    if is_opp:
                        # print '    add opposite edge'
                        edges.types_spread[id_edge] = 0  # right spread
                        edgewidth = edges.widths[id_edge]-self.width_bikelane_opp
                        #edges.widths[id_edge] = edgewidth
                        #lanes.widths[id_lane[-1]] = edgewidth
                        id_edge_opp = edges.make(id_fromnode=id_tonode,
                                                 id_tonode=id_fromnode,
                                                 id_sumo='-'+id_sumo,
                                                 type_edge='',
                                                 num_lanes=1,
                                                 speed_max=self.speed_max_bike_opp,
                                                 priority=0,  # give lowest priority...it's illegal!!
                                                 #length = 0.0,
                                                 shape=shape[::-1],
                                                 type_spread='right',
                                                 #name = '',
                                                 #offset_end = 0.0,
                                                 width_lanes_default=self.width_bikelane_opp,
                                                 width_sidewalk=-1,
                                                 )

                        id_lane_opp = lanes.make(id_edge=id_edge_opp,
                                                 index=0,
                                                 width=self.width_bikelane_opp,
                                                 speed_max=self.speed_max_bike_opp,
                                                 ids_modes_allow=allow_cycloped,
                                                 )

                        edges.ids_lanes[id_edge_opp] = [id_lane_opp]
                        ids_edge_update.append(id_edge_opp)

                    # TODO: in case of 3 lane, we could delete the
                    # left most pedestrian lane

        # generate connections to newly created edges and lanes
        self.parent.complete_connections()

        # rebuild lane shapes
        edges.update(ids=ids_edge_update, is_update_lanes=True)

        return True

    def _detect_pedonlyaccess(self, id_lane, lanes):
        """
        Returns True if lane is only accessible on foot.
        """
        ids_modes_allow = lanes.ids_modes_allow[id_lane]
        if len(ids_modes_allow) == 1:
            return ids_modes_allow[0] == self.id_mode_ped
        else:
            return False

    def _detect_bikeaccess(self, id_lane, lanes):
        """
        Detect if lane is bike accessible.
        Returns  is_bikeaccess, is_bikeonlyaccess
        In case lane is only accessible to bikes,
        then the maximum speed is adjusted to given value.
        """
        ids_modes_allow = lanes.ids_modes_allow[id_lane]
        # print '_detect_bikeaccess id_lane',id_lane,ids_modes_allow,len(ids_modes_allow)
        if len(ids_modes_allow) == 1:
            if ids_modes_allow[0] == self.id_mode_bike:
                # the only allowed mode is bicycle.
                # set default speed limits
                lanes.speeds_max[id_lane] = self.speed_max_bike
                return True, True
            else:
                return False, False
        elif len(ids_modes_allow) > 1:
            return self.id_mode_bike in ids_modes_allow, False

        else:
            # no restrictions
            return self.id_mode_bike not in lanes.ids_modes_disallow[id_lane], False

    def _make_bike_on_ped(self, id_lane, lanes):
        """
        Makes pedestrian lane accessible for bikes.
        """
        ids_modes_allow = set(lanes.ids_modes_allow[id_lane])
        # print '_make_bike_on_ped id_lane',id_lane,ids_modes_allow,len(ids_modes_allow)
        # are modes a subset of modes to be updated?
        if ids_modes_allow.issubset(self.ids_modes_tocomplete):
            # add bike mode
            ids_modes_allow.update([self.id_mode_bike])
            lanes.ids_modes_allow[id_lane] = list(ids_modes_allow)
            return lanes.ids_modes_allow[id_lane], lanes.widths[id_lane]

        return None, None


class TlsjunctionGenerator_old(netconvert.NetConvertMixin):
    def __init__(self,  net, logger=None, **kwargs):
        print 'TlsjunctionGenerator.__init__'
        self._init_common('tlsjunctiongenerator',
                          parent=net,
                          name='Trafficlight Junction generator',
                          logger=logger,
                          info='Generates a traffic light system around junction id_node.',
                          )

        attrsman = self.set_attrsman(cm.Attrsman(self))

        self.id_node = attrsman.add(cm.AttrConf('id_node', kwargs.get('id_node', -1),
                                                groupnames=['options'],
                                                perm='rw',
                                                name='Node ID',
                                                info='Node ID from where traffic light system will be build.',
                                                ))

        # self.init_options_tls()

        self.dist_tls = attrsman.add(cm.AttrConf('dist_tls', kwargs.get('dist_tls', 30.0),
                                                 groupnames=['options'],
                                                 perm='rw',
                                                 name='TLS distance',
                                                 unit='m',
                                                 info='Nodes within this distance are potentially part of the traffic light system',
                                                 ))

    def do(self):
        print 'TlsjunctionGenerator.do'
        net = self.parent
        edges = self.parent.edges
        nodes = self.parent.nodes
        lanes = self.parent.lane
        backtree = []

        for id_edge in net.nodes.ids_incoming[self.id_node]:
            tree = []
            dist = 0
            self.search_main_backward(id_edge, dist, tree)
            backtree.append(tree)

        # print '  backtree',backtree

    def is_major_road(self, priority, n_lanes, speed_max):
        return (priority >= 7) & (n_lanes >= 1) & (speed_max >= 30/3.6)

    def search_main_backward(self, id_edge, dist, tree):
        """
        |
        O
        |\
        | \
        O  O
        |\ |\
        O OO O

        """
        if (dist < self.dist_tls):
            ids_backedges = self.parent.edges.get_incoming(id_edge)

            for id_backedge in self.parent.edges.get_incoming(id_edge):
                if self.is_major_road():
                    id_backedge = 1

    def identify_main_directions(self):
        net = self.parent
        ids_edge = net.nodes.ids_incoming[self.id_node]
        for id_edge, id_sumo, type_spread, shape, \
            n_lanes, ids_lane, priority, id_fromnode, id_tonode, speed_max\
            in zip(ids_edge,
                   edges.ids_sumo[ids_edge],
                   edges.types_spread[ids_edge],
                   edges.shapes[ids_edge],
                   edges.nums_lanes[ids_edge],
                   edges.ids_lanes[ids_edge],
                   edges.priorities[ids_edge],
                   edges.ids_fromnode[ids_edge],
                   edges.ids_tonode[ids_edge],
                   edges.speeds_max[ids_edge],
                   ):
            if self.is_major_road(priority, n_lanes, speed_max):
                dist = 0
                direct = shape[:2]
                is_major = True
                while is_major & (dist < self.joindist_tls):
                    ids_fromedge = edges.get_incoming(id_edge)
