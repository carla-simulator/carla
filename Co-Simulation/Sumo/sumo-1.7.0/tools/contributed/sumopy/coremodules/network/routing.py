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

# @file    routing.py
# @author  Joerg Schweizer
# @date

import os
import numpy as np

from agilepy.lib_base.processes import Process, CmlMixin, ff, call


class priorityDictionary(dict):
    def __init__(self):
        '''Initialize priorityDictionary by creating binary heap
            of pairs (value,key).  Note that changing or removing a dict entry will
            not remove the old pair from the heap until it is found by smallest() or
            until the heap is rebuilt.'''
        self.__heap = []
        dict.__init__(self)

    def smallest(self):
        '''Find smallest item after removing deleted items from heap.'''
        if len(self) == 0:
            raise IndexError, "smallest of empty priorityDictionary"
        heap = self.__heap
        while heap[0][1] not in self or self[heap[0][1]] != heap[0][0]:
            lastItem = heap.pop()
            insertionPoint = 0
            while 1:
                smallChild = 2*insertionPoint+1
                if smallChild+1 < len(heap) and \
                        heap[smallChild][0] > heap[smallChild+1][0]:
                    smallChild += 1
                if smallChild >= len(heap) or lastItem <= heap[smallChild]:
                    heap[insertionPoint] = lastItem
                    break
                heap[insertionPoint] = heap[smallChild]
                insertionPoint = smallChild
        return heap[0][1]

    def __iter__(self):
        '''Create destructive sorted iterator of priorityDictionary.'''
        def iterfn():
            while len(self) > 0:
                x = self.smallest()
                yield x
                del self[x]
        return iterfn()

    def __setitem__(self, key, val):
        '''Change value stored in dictionary and add corresponding
            pair to heap.  Rebuilds the heap if the number of deleted items grows
            too large, to avoid memory leakage.'''
        dict.__setitem__(self, key, val)
        heap = self.__heap
        if len(heap) > 2 * len(self):
            self.__heap = [(v, k) for k, v in self.iteritems()]
            self.__heap.sort()  # builtin sort likely faster than O(n) heapify
        else:
            newPair = (val, key)
            insertionPoint = len(heap)
            heap.append(None)
            while insertionPoint > 0 and val < heap[(insertionPoint-1)//2][0]:
                heap[insertionPoint] = heap[(insertionPoint-1)//2]
                insertionPoint = (insertionPoint-1)//2
            heap[insertionPoint] = newPair

    def setdefault(self, key, val):
        '''Reimplement setdefault to call our customized __setitem__.'''
        if key not in self:
            self[key] = val
        return self[key]

    def update(self, other):
        for key in other.keys():
            self[key] = other[key]


def dijkstra(id_node_start, nodes, edges, ids_node_target=None,  weights={}):
    """
    OUTDATED!!! see edgedijkstra
    Calculates minimum cost tree and minimum route costs from 
    id_node_start to all nodes of the network or to 
    target nodes given in set ids_node_target.
    Attention does not take into consideration missing connectors!!
    """
    # print '\n\ndijkstraPlain',id_node_start.getID()
    # dictionary of final distances
    D = {}
    # dictionary of predecessors
    P = {}
    # est.dist. of non-final vert.
    Q = priorityDictionary()
    Q[id_node_start] = 0
    for v in Q:
        D[v] = Q[v]

        if ids_node_target is not None:
            ids_node_target.discard(v)
            # if ids_node_target.discard(v):
            if len(ids_node_target) == 0:
                return (D, P)
        # print ' v=',v.getID(),len(v.getOutgoing())
        for id_edge in nodes.ids_outgoing[v]:
            # print '    ',edge.getID(),edge._to.getID()
            w = edges.ids_tonode[id_edge]
            #vwLength = D[v] + weights.get(edge,edge._cost)
            vwLength = D[v] + weights.get(id_edge, edges.lengths[id_edge])
            if w not in D and (w not in Q or vwLength < Q[w]):
                Q[w] = vwLength
                P[w] = id_edge
    return (D, P)


def edgedijkstra_backwards(id_edge_start, cost_limit,
                           weights=None, bstar=None):
    """
    Calculates minimum cost tree and minimum route costs from 
    id_edge_start to all edges of the network or to 
    target edges given in set ids_edge_target.

    """
    ids_origin = set()
    # print 'edgedijkstra_backwards id_edge_start',id_edge_start,'cost_limit',cost_limit
    # dictionary of final distances
    D = {}

    # dictionary of predecessors
    P = {}
    # est.dist. of non-final vert.

    if weights[id_edge_start] < 0:
        print '  no access id_edge_start, weights', id_edge_start, weights[id_edge_start]
        return ([], {}, {})

    Q = priorityDictionary()
    Q[id_edge_start] = weights[id_edge_start]
    ids_edges_nochange = set()
    for e in Q:
        if (e not in ids_edges_nochange) & (e not in ids_origin):

            D[e] = Q[e]
            has_changed = False

            # print '  --------------'
            # print '  toedge',e,'ids_bedge',bstar[e]
            # print '    D=',D
            # print '    Q=',Q
            if not bstar.has_key(e):
                print 'WARNING in edgedijkstra: bstar has no edge', e
                print 'routes = \n', P
                return ([], None, P)

            for id_edge in bstar[e]:
                if 0:
                    weight_tot = D[e] + weights[id_edge]
                    newstate = '|'
                    if id_edge not in D:
                        newstate += '*D'
                    if id_edge not in Q:
                        newstate += '*Q'
                    elif weight_tot < Q[id_edge]:
                        newstate += '<Q'
                    else:
                        newstate += '>Q|'
                    print '    id_bedge', id_edge, 'w=%.2f,w_tot=%.2f' % (
                        weights[id_edge], weight_tot), weights[id_edge] >= 0, D[e] + weights[id_edge] < cost_limit, id_edge not in D, (id_edge not in Q or weight_tot < Q[id_edge]), newstate

                if weights[id_edge] >= 0:  # edge accessible?
                    weight_tot = D[e] + weights[id_edge]
                    if weight_tot < cost_limit:
                        if id_edge not in D and (id_edge not in Q or weight_tot < Q[id_edge]):
                            Q[id_edge] = weight_tot
                            P[id_edge] = e
                            has_changed = True
                    else:
                        # print '   **found origin',e
                        ids_origin.add(e)

            # print '  has_changed',e,has_changed
            if not has_changed:
                #    break
                ids_edges_nochange.add(e)

    # print '  P',P
    # print '  D',D
    return (ids_origin, D, P)  # returns in tree with all reachable destinations


def edgedijkstra(id_edge_start, ids_edge_target=None,
                 weights=None, fstar=None):
    """
    Calculates minimum cost tree and minimum route costs from 
    id_edge_start to all edges of the network or to 
    target edges given in set ids_edge_target.

    """
    ids_target = ids_edge_target.copy()
    # print 'edgedijkstra'
    # dictionary of final distances
    D = {}

    # dictionary of predecessors
    P = {}
    # est.dist. of non-final vert.

    if weights[id_edge_start] < 0:
        print '  WARNING in edgedijkstra: no access id_edge_start, weights', id_edge_start, weights[id_edge_start]
        return ({}, {})

    Q = priorityDictionary()
    Q[id_edge_start] = weights[id_edge_start]

    for e in Q:
        D[e] = Q[e]

        if ids_target is not None:
            ids_target.discard(e)
            if len(ids_target) == 0:
                return (D, P)
        if not fstar.has_key(e):
            print 'WARNING in edgedijkstra: fstar has no edge', e
            print 'routes = \n', P
            return (None, P)
        for id_edge in fstar[e]:
            if weights[id_edge] >= 0:  # edge accessible?
                weight_tot = D[e] + weights[id_edge]
                if id_edge not in D and (id_edge not in Q or weight_tot < Q[id_edge]):
                    Q[id_edge] = weight_tot
                    P[id_edge] = e
    return (D, P)  # returns in tree with all reachable destinations


def get_mincostroute_edge2edge(id_rootedge, id_targetedge, D=None, P=None,
                               weights=None, fstar=None):
    """
    Returns cost and shortest path from rootedge to a specific targetedge.
    D, P must be precalculated for rootnode with function dijkstraPlainEdge

    """
    if D is None:
        D, P = edgedijkstra(id_rootedge, set([id_targetedge, ]),
                            weights=weights, fstar=fstar)

    route = [id_targetedge]
    if not P.has_key(id_targetedge):
        return 0.0, []

    e = id_targetedge
    while e != id_rootedge:
        id_edge = P[e]
        route.append(id_edge)
        e = id_edge
    # route.append(e)
    route.reverse()
    return D[id_targetedge], route


def get_mincostroute_node2node(id_rootnode, id_targetnode, D, P, edges):
    """
    Returns cost and shortest path from rootnode to a specific targetnode.
    D, P must be precalculated for rootnode with function dijkstraPlain

    """
    # print 'getMinCostRoute node_start=%s, edge_end =%s node_end=%s'%(rootnode.getID(),P[targetnode].getID(),targetnode.getID())
    id_node = id_targetnode
    route = []
    if not P.has_key(id_targetnode):
        return 0.0, []

    while id_node != id_rootnode:
        id_edge = P[id_node]
        route.append(id_edge)
        id_node = edges.ids_fromnode[id_edge]

    # for edge in route:
    #    print '        ',edge.getID()
    route.reverse()
    return D[id_targetnode], route


def duaroute(tripfilepath, netfilepath, routefilepath, options='-v --ignore-errors'):
    """
    Simple shortes path duaoute function
    """
    #  do not use options: --repair --remove-loops
    cmd = 'duarouter '+options+' --trip-files %s --net-file %s --output-file %s'\
        % (ff(tripfilepath), ff(netfilepath), ff(routefilepath))
    return call(cmd)


def init_random(self, **kwargs):
    optiongroup = 'random'
    self.add_option('is_timeseed', kwargs.get('is_timeseed', False),
                    groupnames=[optiongroup, 'options', ],
                    name='Time seed',
                    perm='rw',
                    info='Initialises the random number generator with the current system time.',
                    cml='--random',
                    )

    self.add_option('seed', kwargs.get('seed', 23423),
                    groupnames=[optiongroup, 'options', ],
                    name='Random seed',
                    perm='rw',
                    info='Initialises the random number generator with the given value.',
                    cml='--seed',
                    )


class RouterMixin(CmlMixin, Process):
    def init_tripsrouter(self, ident, net,
                         trips,
                         netfilepath=None,
                         outfilepath=None,
                         name='Duarouter',
                         info='Generates routes from trips, flows or previous routes',
                         is_export_net=True,
                         logger=None, cml='duarouter'):

        self._init_common(ident, name=name,
                          parent=net,
                          logger=logger,
                          info=info,
                          )

        self.init_cml(cml)  # pass main shell command
        self.is_export_net = is_export_net
        self._trips = trips

        if netfilepath is None:
            netfilepath = net.get_filepath()

        self.add_option('netfilepath', netfilepath,
                        groupnames=['_private'],
                        cml='--net-file',
                        perm='r',
                        name='Net file',
                        wildcards='Net XML files (*.net.xml)|*.net.xml',
                        metatype='filepath',
                        info='SUMO Net file in XML format.',
                        )

        if outfilepath is None:
            outfilepath = trips.get_routefilepath()

        self.add_option('outfilepath', outfilepath,
                        groupnames=['_private'],
                        cml='--output-file',
                        perm='r',
                        name='Out routefile',
                        wildcards='Route XML files (*.rou.xml)|*.rou.xml',
                        metatype='filepath',
                        info='Output file of the routing process, which is a SUMO route file in XML format.',
                        )

    def init_options_time(self, **kwargs):
        optiongroup = 'time'
        self.add_option('time_begin', kwargs.get('time_begin', -1),
                        groupnames=[optiongroup, 'options', ],
                        name='Start time',
                        perm='rw',
                        info='Defines the begin time; Previous trips will be discarded. The value of  -1 takes all routes from the beginning.',
                        unit='s',
                        cml='--begin',
                        is_enabled=lambda self: self.time_begin >= 0.0,
                        )

        self.add_option('time_end', kwargs.get('time_end', -1),
                        groupnames=[optiongroup, 'options', ],
                        name='End time',
                        perm='rw',
                        info='Defines the end time; Later trips will be discarded; The value of -1 takes all routes to the end.',
                        unit='s',
                        cml='--end',
                        is_enabled=lambda self: self.time_end >= 0.0,
                        )

    def init_options_processing_common(self, **kwargs):
        optiongroup = 'processing'

        self.add_option('n_alternatives_max', kwargs.get('n_alternatives_max', 5),
                        name='Max. alternatives',
                        info='Maximum number of considered route alternatives.',
                        cml='--max-alternatives',
                        groupnames=[optiongroup, 'options', ],
                        perm='rw',
                        )

        self.add_option('is_ignore_errors', kwargs.get('is_ignore_errors', True),
                        name='Ignore disconnected',
                        info='Continue if a route could not be build.',
                        cml='--ignore-errors',
                        groupnames=[optiongroup, 'options', ],
                        perm='rw',
                        )

        self.add_option('n_threads', kwargs.get('n_threads', 0),
                        name='Parallel threads',
                        info="The number of parallel execution threads used for routing.",
                        cml='--routing-threads',
                        groupnames=[optiongroup, 'options', ],
                        perm='rw',
                        )

    def init_options_processing_dua(self, **kwargs):
        optiongroup = 'processing'

        self.add_option('time_preload', kwargs.get('time_preload', 200),
                        name='Preload time',
                        unit='s',
                        info='Load routes for the next number of seconds ahead.',
                        cml='--route-steps',
                        groupnames=[optiongroup, 'options', ],
                        perm='rw',
                        )
        # self.add_option('is_randomize_flows',kwargs.get('is_randomize_flows',False),
        #                name = 'Preload time',
        #                info = 'generate random departure times for flow input.',
        #                cml = '--randomize-flows',
        #                groupnames = [optiongroup,'options',],#
        #                perm='rw',
        #                )

        self.add_option('is_remove_loops', kwargs.get('is_remove_loops', False),
                        name='Remove loops',
                        info='Remove loops within the route; Remove turnarounds at start and end of the route. May cause errors!',
                        cml='--remove-loops',
                        groupnames=[optiongroup, 'options', ],
                        perm='rw',
                        )

        self.add_option('is_repair', kwargs.get('is_repair', False),
                        name='Repair',
                        info='Tries to correct a false route. May cause errors!',
                        cml='--repair',
                        groupnames=[optiongroup, 'options', ],
                        perm='rw',
                        )

        self.add_option('is_repair_from', kwargs.get('is_repair_from', False),
                        name='Repair start',
                        info='Tries to correct an invalid starting edge by using the first usable edge instead.',
                        cml='--repair.from',
                        groupnames=[optiongroup, 'options', ],
                        perm='rw',
                        )

        self.add_option('is_repair_to', kwargs.get('is_repair_from', False),
                        name='Repair end',
                        info='Tries to correct an invalid destination edge by using the last usable edge instead.',
                        cml='--repair.to',
                        groupnames=[optiongroup, 'options', ],
                        perm='rw',
                        )

        self.add_option('is_bulkrouting', kwargs.get('is_bulkrouting', False),
                        name='Bulk routing?',
                        info="Aggregate routing queries with the same origin.",
                        cml='--bulk-routing',
                        groupnames=[optiongroup, 'options', ],
                        perm='rw',
                        )

        # --weights.interpolate <BOOL> 	Interpolate edge weights at interval boundaries; default: false
        # --weight-period <TIME> 	Aggregation period for the given weight files; triggers rebuilding of Contraction Hierarchy; default: 3600
        # --weights.expand <BOOL> 	Expand weights behind the simulation's end; default: false

        # --with-taz <BOOL> 	Use origin and destination zones (districts) for in- and output; default: false

    def init_options_methods(self, **kwargs):
        optiongroup = 'methods'

        self.add_option('method_routechoice', kwargs.get('method_routechoice', 'gawron'),
                        name='Routechoice method',
                        choices=['gawron', 'logit', 'lohse'],
                        info="Mathematical model used for route choice.",
                        cml='--route-choice-method',
                        groupnames=[optiongroup, 'options', ],
                        perm='rw',
                        )

        self.add_option('beta_gawron', kwargs.get('beta_gawron', 0.3),
                        name="Gawron's 'beta'",
                        info="Gawron's 'beta' parameter.",
                        cml='--gawron.beta',
                        groupnames=[optiongroup, 'options', ],
                        perm='rw',
                        is_enabled=lambda self: self.method_routechoice is 'gawron',
                        )

        self.add_option('a_gawron', kwargs.get('a_gawron', 0.05),
                        name="Gawron's 'a'",
                        info="Gawron's 'a' parameter.",
                        cml='--gawron.a',
                        groupnames=[optiongroup, 'options', ],
                        perm='rw',
                        is_enabled=lambda self: self.method_routechoice is 'gawron',
                        )

        self.add_option('beta_logit', kwargs.get('beta_logit', 0.15),
                        name="Logit's 'beta'",
                        info="C-Logit's 'beta' parameter.",
                        cml='--logit.beta',
                        groupnames=[optiongroup, 'options', ],
                        perm='rw',
                        is_enabled=lambda self: self.method_routechoice is 'logit',
                        )

        self.add_option('gamma_logit', kwargs.get('gamma_logit', 1.0),
                        name="Logit's 'gamma'",
                        info="C-Logit's 'gamma' parameter.",
                        cml='--logit.gamma',
                        groupnames=[optiongroup, 'options', ],
                        perm='rw',
                        is_enabled=lambda self: self.method_routechoice is 'logit',
                        )

        self.add_option('theta_logit', kwargs.get('theta_logit', 0.01),
                        name="Logit's 'theta'",
                        info="C-Logit's 'theta' parameter.",
                        cml='--logit.theta',
                        groupnames=[optiongroup, 'options', ],
                        perm='rw',
                        is_enabled=lambda self: self.method_routechoice is 'logit',
                        )

        self.add_option('algorithm_routing', kwargs.get('algorithm_routing', 'dijkstra'),
                        name='Routing algorithm',
                        choices=['dijkstra', 'astar', 'CH', 'CHWrapper'],
                        info="Select among routing algorithms.",
                        cml='--routing-algorithm',
                        groupnames=[optiongroup, 'options', ],
                        perm='rw',
                        )

        self.add_option('is_keep_all_routes', kwargs.get('is_keep_all_routes', False),
                        name='Keep all routes?',
                        info="Save even routes with near zero probability.",
                        cml='--keep-all-routes',
                        groupnames=[optiongroup, 'options', ],
                        perm='rw',
                        )
        self.add_option('is_skip_new_routes', kwargs.get('is_skip_new_routes', False),
                        name='Skip new routes?',
                        info="Only reuse routes from input, do not calculate new ones.",
                        cml='--skip-new-routes',
                        groupnames=[optiongroup, 'options', ],
                        perm='rw',
                        )

    def do(self):
        if self.is_export_net:
            # first export current net
            self.parent.export_netxml(self.netfilepath)

        if self.is_export_trips:
            self._trips.export_trips_xml(self.tripfilepaths)

        self.update_params()
        cml = self.get_cml()

        # print 'SumonetImporter.do',cml
        self.run_cml(cml)
        if self.status == 'success':
            print '  Routing done.'
            if os.path.isfile(self.outfilepath):
                # print '  outfile exists, start importing routes'
                self._trips.import_routes_xml(self.outfilepath,
                                              is_clear_trips=False,
                                              is_generate_ids=False,
                                              is_add=True)
                return True
            return False
        return False


class DuaRouter(RouterMixin):
    def __init__(self, net, trips,
                 tripfilepaths=None,
                 outfilepath=None,
                 is_export_net=True,
                 logger=None,
                 **kwargs):
        print 'DuaRouter.__init__ net, trips', net, trips
        self.init_tripsrouter('duarouter', net,  # net becomes parent
                              trips,
                              outfilepath=outfilepath,
                              logger=logger,
                              is_export_net=is_export_net,
                              )

        if tripfilepaths is None:
            if trips is not None:
                tripfilepaths = trips.get_tripfilepath()
                self.is_export_trips = True
            else:
                self.is_export_trips = False

        else:
            self.is_export_trips = False
        print '  tripfilepaths', tripfilepaths
        if tripfilepaths is not None:
            self.add_option('tripfilepaths', tripfilepaths,
                            groupnames=['_private'],
                            cml='--trip-files',
                            perm='r',
                            name='Trip file(s)',
                            wildcards='Trip XML files (*.trip.xml)|*.trip.xml',
                            metatype='filepaths',
                            info='SUMO Trip files in XML format.',
                            )

        self.init_options_time(**kwargs)
        self.init_options_methods(**kwargs)
        self.init_options_processing_common(**kwargs)
        self.init_options_processing_dua(**kwargs)
        init_random(self, **kwargs)


class MacroRouter(RouterMixin):
    """
    Macroscopic router
    in development
    """

    def __init__(self, net, trips,
                 tripfilepaths=None,
                 netfilepath=None,
                 outfilepath=None,
                 is_export_net=True,
                 logger=None,
                 **kwargs):
        print 'MacroRouter.__init__ net, trips', net, trips
        self.init_tripsrouter('macrorouter', net,  # net becomes parent
                              trips,
                              netfilepath=netfilepath,
                              outfilepath=outfilepath,
                              name='Macroscopic router',
                              info='Generates routes from trips, flows or previous routes',
                              is_export_net=is_export_net,
                              logger=logger,
                              cml='marouter'
                              )

        if tripfilepaths is None:
            if trips is not None:
                tripfilepaths = trips.get_tripfilepath()
                self.is_export_trips = True
            else:
                self.is_export_trips = False

        else:
            self.is_export_trips = False
        print '  tripfilepaths', tripfilepaths
        if tripfilepaths is not None:
            self.add_option('tripfilepaths', tripfilepaths,
                            groupnames=['_private'],
                            cml='--route-files',
                            perm='r',
                            name='Trip file(s)',
                            wildcards='Trip XML files (*.trip.xml)|*.trip.xml',
                            metatype='filepaths',
                            info='SUMO Trip files in XML format.',
                            )

        self.init_options_time(**kwargs)
        self.init_options_methods(**kwargs)

        # marouter specific
        optiongroup = 'methods'
        self.add_option('n_iter_max', kwargs.get('n_iter_max', 20),
                        name='Max. Iterations',
                        info="maximal number of iterations for new route searching in incremental and stochastic user assignment.",
                        cml='--max-iterations',
                        groupnames=[optiongroup, 'options', ],
                        perm='rw',
                        )

        self.add_option('n_iter_inner_max', kwargs.get('n_iter_inner_max', 1000),
                        name='Max. inner Iter.',
                        info="maximal number of inner iterations for user equilibrium calcuation in the stochastic user assignment.",
                        cml='--max-inner-iterations',
                        groupnames=[optiongroup, 'options', ],
                        perm='rw',
                        )

        self.init_options_processing_common(**kwargs)
        init_random(self, **kwargs)
