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

# @file    test_classman_tables.py
# @author  Joerg Schweizer
# @date

#from classman import *

from test_classman_classes import *
from classman import *
from arrayman import *
is_all = 0
if 1 | is_all:
    pass
if 1 | is_all:
    print 'Lines, Poly example'

    ###########################################################################
    # Instance creation

    lines = Lines('lines')
    # vertices = [  [0.0,10.0,10.0,10.0,10.0,0.0],
    #            [10.0,20.0,20.0,20.0,20.0,10.0],
    #            [20.0,30.0,30.0,20.0,30.0,20.0],
    #            ]

    # vertices = [  [(0.0,10.0),(10.0,10.0)],
    #            [(10.0,20.0),(20.0,20.0)],
    #            [(20.0,30.0),(30.0,20.0)],
    #            ]
    vertices = [
        [[0.0, 0.0, 0.0], [0.2, 0.0, 0.0]],  # 0
        [[0.3, 0.0, 0.0], [0.9, 0.0, 0.0]],  # 1
        [[0.5, 0.0, 0.1], [1.9, 0.0, 0.0]],  # 2
    ]
    polygons = [
        np.array([[0.0, 0.0, 0.0], [0.2, 0.0, 0.0], [0.2, 0.0, 0.1], [0.3, 0.3, 0.3]]),  # 0
        np.array([[0.3, 0.0, 0.0], [0.9, 0.0, 0.0]]),  # 1
        np.array([[0.5, 0.0, 0.1], [1.9, 0.0, 0.0], [0.2, 0.2, 0.2]]),  # 2
    ]
    ids_sumo = ['aa10', 'bb22', 'cc333']
    # lines.add_rows(3)
    lines.add_rows(3, vertices=vertices, polygons=polygons, ids_sumo=ids_sumo)
    lines.print_attrs()
    print '\n  indexmap', lines.ids_sumo.get_indexmap()
    print 'direct access vertex=\n', lines.vertices.value
    print 'id for index bb22=', lines.ids_sumo.get_id_from_index('bb22')
    print 'ids for index bb22,cc333=', lines.ids_sumo.get_ids_from_indices(['bb22', 'cc333'])
    # lines.del_row(2)
    lines.print_attrs()
    print 'id for index bb22=', lines.ids_sumo.get_id_from_index('cc333')

    lines.ids_sumo[2] = 'xy'

    print '\n  indexmap', lines.ids_sumo.get_indexmap()
    lines.print_attrs()
if 0 | is_all:
    class Lines(TableObjman):

        def __init__(self, ident,  parent=None,  **kwargs):

            self._init_objman(ident, parent=parent, **kwargs)

            self.add_col(ColConf('vertices',  np.zeros((2, 3), float),
                                 groupnames=['internal'],
                                 perm='rw',
                                 name='Vertex',
                                 is_save=True,
                                 info='Vertex coordinate vectors of points. with format [[[x11,y11,z11],[x12,y12,z12]],[[x21,y21,z21],[x22,y22,z122]],...]',
                                 ))

            self.add_col(ColConf('polygons', None,
                                 dtype='object',
                                 groupnames=['landuse'],
                                 perm='rw',
                                 name='Polygon',
                                 info='Polygons [[ (x11,y11,z11), (x12,y12,z13), (x13, y13,z13),...],[...]]',
                                 ))

            self.add_col(ColConf('ids_sumo', None,
                                 dtype='object',
                                 is_index=True,
                                 perm='rw',
                                 name='Polygon',
                                 info='Polygons [[ (x11,y11,z11), (x12,y12,z13), (x13, y13,z13),...],[...]]',
                                 ))

    ###########################################################################
    # Instance creation

    lines = Lines('lines')
    # vertices = [  [0.0,10.0,10.0,10.0,10.0,0.0],
    #            [10.0,20.0,20.0,20.0,20.0,10.0],
    #            [20.0,30.0,30.0,20.0,30.0,20.0],
    #            ]

    # vertices = [  [(0.0,10.0),(10.0,10.0)],
    #            [(10.0,20.0),(20.0,20.0)],
    #            [(20.0,30.0),(30.0,20.0)],
    #            ]
    vertices = [
        [[0.0, 0.0, 0.0], [0.2, 0.0, 0.0]],  # 0
        [[0.3, 0.0, 0.0], [0.9, 0.0, 0.0]],  # 1
        [[0.5, 0.0, 0.1], [1.9, 0.0, 0.0]],  # 2
    ]
    polygons = [
        np.array([[0.0, 0.0, 0.0], [0.2, 0.0, 0.0], [0.2, 0.0, 0.1], [0.3, 0.3, 0.3]]),  # 0
        np.array([[0.3, 0.0, 0.0], [0.9, 0.0, 0.0]]),  # 1
        np.array([[0.5, 0.0, 0.1], [1.9, 0.0, 0.0], [0.2, 0.2, 0.2]]),  # 2
    ]
    ids_sumo = ['aa10', 'bb22', 'cc333']
    # lines.add_rows(3)
    lines.add_rows(3, vertices=vertices, polygons=polygons, ids_sumo=ids_sumo)
    lines.print_attrs()
    print 'direct access vertex=\n', lines.vertices.value
    print 'direct access polygons=\n', lines.polygons.value
    print 'id for index bb22=', lines.ids_sumo.get_id_from_index('bb22')
    print 'ids for index bb22,cc333=', lines.ids_sumo.get_ids_from_indices(['bb22', 'cc333'])
    lines.del_row(2)
    lines.print_attrs()
    print 'id for index bb22=', lines.ids_sumo.get_id_from_index('cc333')
if 0 | is_all:
    class Lines(ArrayObjman):

        def __init__(self, ident,  parent=None,  **kwargs):

            self._init_objman(ident, parent=parent, **kwargs)

            self.add_col(ArrayConf('vertices',  np.zeros((2, 3), float),
                                   groupnames=['internal'],
                                   perm='rw',
                                   name='Vertex',
                                   is_save=True,
                                   info='Vertex coordinate vectors of points. with format [[[x11,y11,z11],[x12,y12,z12]],[[x21,y21,z21],[x22,y22,z122]],...]',
                                   ))

            self.add_col(ArrayConf('polygons', None,
                                   dtype='object',
                                   groupnames=['landuse'],
                                   perm='rw',
                                   name='Polygon',
                                   info='Polygons [[ (x11,y11,z11), (x12,y12,z13), (x13, y13,z13),...],[...]]',
                                   ))

    ###########################################################################
    # Instance creation

    lines = Lines('lines')
    # vertices = [  [0.0,10.0,10.0,10.0,10.0,0.0],
    #            [10.0,20.0,20.0,20.0,20.0,10.0],
    #            [20.0,30.0,30.0,20.0,30.0,20.0],
    #            ]

    # vertices = [  [(0.0,10.0),(10.0,10.0)],
    #            [(10.0,20.0),(20.0,20.0)],
    #            [(20.0,30.0),(30.0,20.0)],
    #            ]
    vertices = [
        [[0.0, 0.0, 0.0], [0.2, 0.0, 0.0]],  # 0
        [[0.3, 0.0, 0.0], [0.9, 0.0, 0.0]],  # 1
        [[0.5, 0.0, 0.1], [1.9, 0.0, 0.0]],  # 2
    ]
    polygons = [
        np.array([[0.0, 0.0, 0.0], [0.2, 0.0, 0.0], [0.2, 0.0, 0.1], [0.3, 0.3, 0.3]]),  # 0
        np.array([[0.3, 0.0, 0.0], [0.9, 0.0, 0.0]]),  # 1
        np.array([[0.5, 0.0, 0.1], [1.9, 0.0, 0.0], [0.2, 0.2, 0.2]]),  # 2
    ]
    # lines.add_rows(3)
    lines.add_rows(3, vertices=vertices, polygons=polygons)
    lines.print_attrs()
    print 'direct access vertex=\n', lines.vertices.value
    print 'direct access polygons=\n', lines.polygons.value

if 0 | is_all:
    class ZonesTab(ArrayObjman):
        def __init__(self, ident,  parent=None, **kwargs):
            self._init_objman(ident, parent=parent, **kwargs)

            self.add_col(ColConf('shapes', [],
                                 groupnames=['state'],
                                 perm='rw',
                                 is_save=True,
                                 name='Shape',
                                 info='Shape of zone which is a list of (x,y) coordinates',
                                 ))

    class OdTripsTab(ArrayObjman):
        def __init__(self, ident,  parent, zones, **kwargs):
            self._init_objman(ident, parent=parent, **kwargs)

            self.add_col(IdsConf('ids_orig', zones, is_child=False,
                                 groupnames=['state'],
                                 is_save=True,
                                 name='ID Orig',
                                 info='ID of traffic assignment zone of origin of trip.',
                                 ))

            self.add_col(IdsConf('ids_dest', zones, is_child=False,
                                 groupnames=['state'],
                                 is_save=True,
                                 name='ID Dest',
                                 info='ID of traffic assignment zone of destination of trip.',
                                 ))

            self.add_col(ColConf('tripnumbers', 0,
                                 groupnames=['state'],
                                 perm='rw',
                                 is_save=True,
                                 name='Trip number',
                                 info='Number of trips from zone with ID Orig to zone with ID Dest.',
                                 ))

    class OdModesTab(ArrayObjman):
        def __init__(self, ident,  parent=None, **kwargs):
            self._init_objman(ident, parent=parent, **kwargs)

            self.add_col(ObjsConf('odtrips',
                                  groupnames=['state'],
                                  is_save=True,
                                  name='OD matrix',
                                  info='Matrix with trips from origin to destintion',
                                  ))

    class OdIntervalsTab(ArrayObjman):
        def __init__(self, ident,  parent=None, **kwargs):
            self._init_objman(ident, parent=parent, **kwargs)

            self.add_col(ColConf('t_start', 0.0,
                                 groupnames=['state'],
                                 perm='rw',
                                 is_save=True,
                                 name='Start time',
                                 unit='s',
                                 info='Start time of interval',
                                 ))

            self.add_col(ColConf('t_end', 3600.0,
                                 groupnames=['state'],
                                 perm='rw',
                                 is_save=True,
                                 name='End time',
                                 unit='s',
                                 info='End time of interval',
                                 ))

            self.add_col(ObjsConf('odmodes',
                                  groupnames=['state'],
                                  is_save=True,
                                  name='Modes',
                                  info='Transport mode',
                                  ))

    ###########################################################################
    # Instance creation

    demand = BaseObjman('demand')

    zones = ZonesTab('zones', parent=demand)
    demand.zones = demand.attrsman.add(ObjConf(zones))
    shapes = [[(0.0, 10.0), (10.0, 10.0), (10.0, 0.0)],
              [(10.0, 20.0), (20.0, 20.0), (20.0, 10.0)],
              [(20.0, 30.0), (30.0, 20.0), (30.0, 20.0)],
              ]
    zones.add_rows(3, shapes=shapes)

    odintervals = OdIntervalsTab('odintervals', parent=demand, info='OD data for different time intervals')
    demand.odintervals = demand.attrsman.add(ObjConf(odintervals))
    odintervals.add_rows(2, t_start=[0, 3600], t_end=[3600, 7200])
    for id_odmodes in odintervals.get_ids():
        odmodes = OdModesTab((odintervals.odmodes.attrname, id_odmodes), parent=odintervals)
        odintervals.odmodes[id_odmodes] = odmodes

        odmodes.add_rows(2)
        for id_odtrips in odmodes.get_ids():
            odtrips = OdTripsTab((odmodes.odtrips.attrname, id_odtrips), odmodes, zones)
            odtrips.add_rows(3, ids_orig=[3, 2, 1], ids_dest=[3, 3, 3], tripnumbers=[10, 200, 555])
            odmodes.odtrips[id_odtrips] = odtrips

    demand.attrsman.print_attrs()
    odintervals.print_attrs()

    for id_odmodes in odintervals.get_ids():
        print '\nMODE:'
        odintervals.odmodes[id_odmodes].print_attrs()
        print '\nTRIPS:'
        for id_odtrips in odmodes.get_ids():
            odmodes.odtrips[id_odtrips].print_attrs()


if 0 | is_all:
    class ZonesTab(TableObjman):
        def __init__(self, ident,  parent=None, **kwargs):
            self._init_objman(ident, parent=parent, **kwargs)

            self.add_col(ColConf('shapes', [],
                                 groupnames=['state'],
                                 perm='rw',
                                 is_save=True,
                                 name='Shape',
                                 info='Shape of zone which is a list of (x,y) coordinates',
                                 ))

    class OdTripsTab(TableObjman):
        def __init__(self, ident,  parent, zones, **kwargs):
            self._init_objman(ident, parent=parent, **kwargs)

            self.add_col(IdsConf('ids_orig', zones, is_child=False,
                                 groupnames=['state'],
                                 is_save=True,
                                 name='ID Orig',
                                 info='ID of traffic assignment zone of origin of trip.',
                                 ))

            self.add_col(IdsConf('ids_dest', zones, is_child=False,
                                 groupnames=['state'],
                                 is_save=True,
                                 name='ID Dest',
                                 info='ID of traffic assignment zone of destination of trip.',
                                 ))

            self.add_col(ColConf('tripnumbers', 0,
                                 groupnames=['state'],
                                 perm='rw',
                                 is_save=True,
                                 name='Trip number',
                                 info='Number of trips from zone with ID Orig to zone with ID Dest.',
                                 ))

    class OdModesTab(TableObjman):
        def __init__(self, ident,  parent=None, **kwargs):
            self._init_objman(ident, parent=parent, **kwargs)

            self.add_col(ObjsConf('odtrips',
                                  groupnames=['state'],
                                  is_save=True,
                                  name='OD matrix',
                                  info='Matrix with trips from origin to destintion',
                                  ))

    class OdIntervalsTab(TableObjman):
        def __init__(self, ident,  parent=None, **kwargs):
            self._init_objman(ident, parent=parent, **kwargs)

            self.add_col(ColConf('t_start', 0.0,
                                 groupnames=['state'],
                                 perm='rw',
                                 is_save=True,
                                 name='Start time',
                                 unit='s',
                                 info='Start time of interval',
                                 ))

            self.add_col(ColConf('t_end', 3600.0,
                                 groupnames=['state'],
                                 perm='rw',
                                 is_save=True,
                                 name='End time',
                                 unit='s',
                                 info='End time of interval',
                                 ))

            self.add_col(ObjsConf('odmodes',
                                  groupnames=['state'],
                                  is_save=True,
                                  name='Modes',
                                  info='Transport mode',
                                  ))

    ###########################################################################
    # Instance creation

    demand = BaseObjman('demand')

    zones = ZonesTab('zones', parent=demand)
    demand.zones = demand.attrsman.add(ObjConf(zones))
    shapes = [[(0.0, 10.0), (10.0, 10.0), (10.0, 0.0)],
              [(10.0, 20.0), (20.0, 20.0), (20.0, 10.0)],
              [(20.0, 30.0), (30.0, 20.0), (30.0, 20.0)],
              ]
    zones.add_rows(3, shapes=shapes)

    odintervals = OdIntervalsTab('odintervals', parent=demand, info='OD data for different time intervals')
    demand.odintervals = demand.attrsman.add(ObjConf(odintervals, is_child=True))
    odintervals.add_rows(2, t_start=[0, 3600], t_end=[3600, 7200])
    for id_odmodes in odintervals.get_ids():
        odmodes = OdModesTab((odintervals.odmodes.attrname, id_odmodes), parent=odintervals)
        odintervals.odmodes[id_odmodes] = odmodes

        odmodes.add_rows(2)
        for id_odtrips in odmodes.get_ids():
            odtrips = OdTripsTab((odmodes.odtrips.attrname, id_odtrips), odmodes, zones)
            odtrips.add_rows(3, ids_orig=[3, 2, 1], ids_dest=[3, 3, 3], tripnumbers=[10, 200, 555])
            odmodes.odtrips[id_odtrips] = odtrips

    demand.attrsman.print_attrs()
    odintervals.print_attrs()

    for id_odmodes in odintervals.get_ids():
        print '\nMODE:'
        odintervals.odmodes[id_odmodes].print_attrs()
        print '\nTRIPS:'
        for id_odtrips in odmodes.get_ids():
            odmodes.odtrips[id_odtrips].print_attrs()


if 0 | is_all:

    net = BaseObjman('net')

    # TODO: could be put in 2 statements
    edges = TableObjman('edges', parent=net, info='Network edges')
    nodes = TableObjman('nodes', parent=net, info='Network nodes')

    net.edges = net.get_attrsman().add(ObjConf(edges, is_child=True))
    net.nodes = net.get_attrsman().add(ObjConf(nodes, is_child=True))

    net.edges.add_col(IdsConf('ids_node_from', net.nodes,
                              groupnames=['state'],
                              is_save=True,
                              name='ID from nodes',
                              info='ID from nodes',
                              ))

    net.edges.add_col(IdsConf('ids_node_to', net.nodes,
                              groupnames=['state'],
                              is_save=True,
                              name='ID to nodes',
                              info='ID to nodes',
                              ))

    net.nodes.add_col(ColConf('coords', (0.0, 0.0),
                              groupnames=['state'],
                              perm='rw',
                              is_save=True,
                              name='Coords',
                              info='Coordinates',
                              ))

    net.nodes.add_rows(4,
                       #            1         2          3          4
                       coords=[(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)],
                       )

    net.edges.add_rows(2)
    net.edges.ids_node_from[[1, 2]] = [1, 4]
    net.edges.ids_node_to[[1, 2]] = [3, 2]
    net.get_attrsman().print_attrs()
    net.edges.print_attrs()
    net.nodes.print_attrs()

if 0 | is_all:
    tab1 = TableObjman('simple_table')

    tab1.add_col(ColConf('surname', 'xx',
                         groupnames=['state'],
                         perm='rw',
                         is_save=True,
                         name='Family name',
                         info='Name of Family',
                         ))

    tab1.add_col(ColConf('streetname', 'via della...',
                         groupnames=['state'],
                         perm='rw',
                         is_save=False,
                         name='Street name',
                         info='Name of the street',
                         ))
    tab1.add_rows(4,
                  surname=['walt', 'greg', 'bob', 'duck'],
                  streetname=['a', 'bb', 'ccc', 'dddd'],
                  )

    print 'direct access: tab1.surname.value', tab1.surname.value
    tab1.print_attrs()

if 0 | is_all:

    tab1 = TableObjman('tab1')
    print '\ntab1.ident', tab1.ident

    tab2 = TableObjman('tab2', parent=tab1)
    print '\ntab2.ident', tab2.ident

    # TODO: seperate attrname from linked obj ident because restrictive and makes problems with multiple tab destinations
    # this should be possible ...following the path of attrnames of absident
    # -
    tab1.add_col(IdsConf('tab1', tab2))

    tab2.add_col(IdsConf('tab2', tab1, is_child=False))

    tab2.add_col(ColConf('surname', 'xx',
                         groupnames=['state'],
                         perm='rw',
                         is_save=True,
                         name='Family name',
                         info='Name of Family',
                         ))

    tab2.add_col(ColConf('streetname', 'via della...',
                         groupnames=['state'],
                         perm='rw',
                         is_save=False,
                         name='Street name',
                         info='Name of the street',
                         ))
    tab2.add_rows(4,
                  surname=['walt', 'greg', 'bob', 'duck'],
                  streetname=['a', 'bb', 'ccc', 'dddd'],
                  tab1=[2, 1, 3, 1, ],
                  )

    tab2.print_attrs()

    tab1.add_rows(3,
                  tab2=[3, 1, 2],
                  )

    tab1.print_attrs()


if 0 | is_all:
    obj = TestTabman()

    print '\nobj.ident', obj.ident

    # streetname
    # print 'This is the value of the attribute: obj.streetname=',obj.streetname
    # print 'This is the configuration instance of the attribute x',obj.attrsman.x
    obj.attrsman.print_attrs()


if 0 | is_all:
    obj = TestTableObjMan()

    print '\nobj.ident', obj.ident

    # streetname
    # print 'This is the value of the attribute: obj.streetname=',obj.streetname
    # print 'This is the configuration instance of the attribute x',obj.attrsman.x
    obj.print_attrs()
