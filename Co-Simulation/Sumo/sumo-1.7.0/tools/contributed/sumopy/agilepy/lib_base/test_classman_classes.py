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

# @file    test_classman_classes.py
# @author  Joerg Schweizer
# @date

"""
Test for callsman
Provides test classes and some test functions for plugin.
"""

from classman import *
from arrayman import *
import xmlman as xm


def on_event_delattr(attrconfig):
    print 'EVENT: Attribute', attrconfig.attrname, 'will be deleted!!'


def on_event_setattr(attrconfig):
    print 'EVENT: Attribute', attrconfig.attrname, 'has been set to a new value', attrconfig.format_value()


def on_event_getattr(attrconfig):
    print 'EVENT: Attribute', attrconfig.attrname, 'has been retrieved the value', attrconfig.format_value()


def on_event_additem(attrconfig, keys):
    print 'EVENT: Attribute', attrconfig.attrname, ':added keys:', keys


def on_event_delitem(attrconfig, keys):
    print 'EVENT: Attribute', attrconfig.attrname, ':delete keys:', keys


def on_event_setitem(attrconfig, keys):
    print 'EVENT: Attribute', attrconfig.attrname, ':set keys:', keys


def on_event_getitem(attrconfig, keys):
    print 'EVENT: Attribute', attrconfig.attrname, ':get keys:', keys


class Segments(ArrayObjman):

    def __init__(self, ident='segments',  parent=None,  **kwargs):

        self._init_objman(ident, parent=parent, xmltag=('segments', 'segment', 'ids_ref'), **kwargs)

        self.add_col(ArrayConf('ids_ref', '',
                               dtype='object',
                               perm='r',
                               is_index=True,
                               name='ID Ref',
                               info='Ref ID',
                               xmltag='id_ref',
                               ))

        self.add_col(ArrayConf('vertices',  np.zeros((2, 3), float),
                               groupnames=['internal'],
                               perm='rw',
                               name='Vertex',
                               is_save=True,
                               info='Vertex coordinate vectors of points. with format [[[x11,y11,z11],[x12,y12,z12]],[[x21,y21,z21],[x22,y22,z122]],...]',
                               xmltag='vertex',
                               ))

        self.add_col(IdsArrayConf('ids_parent', parent,
                                  groupnames=['state'],
                                  is_save=True,
                                  name='ID '+parent.get_ident(),
                                  info='ID of '+parent.get_name()+' object.',
                                  xmltag='id_poly',
                                  ))


class Polylines (ArrayObjman):
    def __init__(self, ident='polyline',  parent=None, name='Polyline',
                 info='Polyline [ segid11, segid12,...]', **kwargs):
        self._init_objman(ident, parent=parent, xmltag=('polylines', 'polyline', 'ids_osm'), **kwargs)
        # print '__init__',self.get_name(),self.format_ident()

        self.add_col(ArrayConf('ids_osm', '',
                               dtype='object',
                               perm='r',
                               is_index=True,
                               name='ID OSM',
                               info='Edge ID of OSM',
                               xmltag='id_osm',
                               ))

        # initialize line segments
        segments = Segments(parent=self)
        self.add(ObjConf(segments, groupnames=['drawobjects']))

        # print '   segments',segments
        # print '   self.segments',self.segments,type(self.segments)
        # create table with id lists to  segments
        self.add_col(IdlistsArrayConf('ids_segments', segments,
                                      groupnames=['elements'],
                                      is_save=True,
                                      name='IDs Segs',
                                      info='List with IDs to Line segments.',
                                      xmltag='segments',
                                      ))

    def draw(self, pointvertices, id_osm):
        """
        pointvertices = [
                [0.0,0.0,0.0],
                [0.2,0.0,0.0],
                ]
        """
        vertices = []
        print 'draw', self.ident
        for i in xrange(1, len(pointvertices)):
            vertices.append([pointvertices[i-1], pointvertices[i]])
        n_vert = len(vertices)
        _id = self.add_row(ids_osm=id_osm)
        cod = []
        #import string
        clist = np.array(['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'], np.object)
        print '  ', len(vertices), clist[:len(vertices)]
        ids = self.segments.value.suggest_ids(len(vertices))
        ids_segs = self.segments.value.add_rows(ids=ids, vertices=vertices, ids_parent=n_vert*[_id], ids_ref=clist[ids])
        self.ids_segments[_id] = list(ids_segs)  # put here list, otherwise numpy thinks it is a numeric array
        return _id


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

        self.add_col(ArrayConf('ids_sumo', '',
                               dtype='object',
                               perm='r',
                               is_index=True,
                               name='ID SUMO',
                               info='Edge ID of SUMO network',
                               ))


class Selection(ArrayObjman):
    def __init__(self, ident,  parent=None,  **kwargs):
        self._init_objman(ident, parent=parent, **kwargs)
        self.add_col(TabIdsArrayConf('obj_ids',
                                     name='Object[id]',
                                     info='Draw obj and ids',
                                     ))


class Collection(ArrayObjman):
    def __init__(self, ident,  parent=None,  **kwargs):
        self._init_objman(ident, parent=parent, **kwargs)
        self.add_col(TabIdListArrayConf('tab_id_lists',
                                        name='[Tab1[id1],Tab2[id1],...]',
                                        info='Collection of different items from different tables.',
                                        ))


class TestClass(BaseObjman):
    def __init__(self, ident='testobj',  parent=None, name='Test Object'):
        self._init_objman(ident, parent=parent, name=name, xmltag=ident)
        attrsman = self.set_attrsman(Attrsman(self))

        self.netfilepath = attrsman.add(AttrConf('netfilepath', 'pathtofile',
                                                 groupnames=['state'],
                                                 perm='rw',
                                                 is_save=True,
                                                 name='Network file',
                                                 metatype='filepath',
                                                 info='Network file path',
                                                 xmltag='netfilepath',
                                                 ))

        self.workdirpath = attrsman.add(AttrConf('workdirpath', 'pathtodir',
                                                 groupnames=['state'],
                                                 perm='rw',
                                                 is_save=True,
                                                 name='Workdir',
                                                 metatype='dirpath',
                                                 info='Working directory.',
                                                 xmltag='workdirpath',
                                                 ))

        self.access = attrsman.add(AttrConf('access', ['bus', 'bike', 'tram'],
                                            groupnames=['state'],
                                            perm='rw',
                                            is_save=True,
                                            name='Access list',
                                            info='List with vehicle classes that have access',
                                            xmltag='access',
                                            ))

        self.allowed = attrsman.add(AttrConf('allowed', 0, choices={'bus': 0, 'bike': 1, 'tram': 2},
                                             groupnames=['state'],
                                             perm='rw',
                                             is_save=True,
                                             name='Access list',
                                             info='List with vehicle classes that have access',
                                             xmltag='allowed',
                                             ))

        self.emissiontype = attrsman.add(AttrConf('emissiontype', 'Euro 0',
                                                  groupnames=['state'],
                                                  perm='rw',
                                                  is_save=True,
                                                  name='Emission type',
                                                  info='Emission type of vehicle',
                                                  xmltag='emissiontype',
                                                  ))

        self.x = attrsman.add(NumConf('x', 1.0,
                                      digits_integer=None, digits_fraction=4,
                                      minval=0.0, maxval=None,
                                      groupnames=['state'],
                                      perm='rw',
                                      is_save=True,
                                      unit='m',
                                      is_plugin=True,
                                      name='position',
                                      info='Test object x position',
                                      xmltag='x',
                                      ))

        self.x_thresh = attrsman.add(NumConf('x_thresh', 0.0,
                                             digits_integer=None, digits_fraction=4,
                                             minval=0.0, maxval=None,
                                             groupnames=['state'],
                                             perm='rw',
                                             is_save=True,
                                             unit='m',
                                             is_plugin=True,
                                             name='position threshold',
                                             info='Test object x position threshold',
                                             xmltag='x_thesh',
                                             ))

        self.is_pos_ok = attrsman.add(FuncConf('is_pos_ok', 'on_is_pos_ok', False,
                                               groupnames=['state'],
                                               name='Pos OK',
                                               info='True if position greater than thhreshold.',
                                               ))

        attrsman.print_attrs()

    def on_is_pos_ok(self):
        """
        True if position greater than thhreshold.
        """
        print 'on_is_pos_ok', self.x > self.x_thresh
        return self.x > self.x_thresh


class TestClass3(BaseObjman):
    def __init__(self, ident='testobj3',  parent=None, name='Test Object3'):
        self._init_objman(ident=ident,  parent=parent, name=name, xmltag='testobj3')
        attrsman = self.set_attrsman(Attrsman(self))

        self.child1 = attrsman.add(ObjConf(parent.child1, is_child=False))

        self.y = attrsman.add(AttrConf('y', 0.0,
                                       groupnames=['state'],
                                       perm='r',
                                       is_save=True,
                                       unit='m',
                                       metatype='length',
                                       is_plugin=True,
                                       name='position',
                                       info='Test object  y position',
                                       ))


class TestClass2(BaseObjman):
    def __init__(self, ident='testobj2',  parent=None, name='Test Object2', xmltag='testobj2'):
        self._init_objman(ident, parent=parent, name=name, xmltag=xmltag)
        attrsman = self.set_attrsman(Attrsman(self))

        self.child1 = attrsman.add(ObjConf(TestClass('child1', self))
                                   )

        print 'TestClass2.child1', self.child1

        self.child3 = attrsman.add(ObjConf(TestClass3('child3', self))
                                   )


class TestTabman(BaseObjman):
    def __init__(self, ident='test_tabman',  parent=None, name='Test Table manage'):
        self._init_objman(ident, parent=parent, name=name)
        tm = Tabman(obj=self)
        self.set_attrsman(tm)
        self.surname = attrsman.add_col(ColConf('surname', 'xx',
                                                groupnames=['state'],
                                                perm='rw',
                                                is_save=True,
                                                name='Family name',
                                                info='Name of Family',
                                                ))

        self.streetname = attrsman.add_col(ColConf('streetname', 'via della...',
                                                   groupnames=['state'],
                                                   perm='rw',
                                                   is_save=False,
                                                   name='Street name',
                                                   info='Name of the street',
                                                   ))

        #_id = attrsman.suggest_id()
        # print '_id =',_id
        # self.attrman.add(_id)

        # print 'self.streetname',self.streetname,type(self.streetname)
        # self.streetname[1]='yyy'
        # print 'self.streetname',self.streetname,type(self.streetname)
        attrsman.add_rows(5)
        attrsman.streetname[3] = 'ssss'
        attrsman.streetname[[1, 2]] = ['aa', 55]
        print 'test get', attrsman.streetname[[1, 2]]
        # self.streetname[1]+='zzz'
        attrsman.del_rows([1, 3])
        attrsman.del_row(5)
        # attrsman.delete('streetname')


class TestTableObjMan(TableObjman):
    def __init__(self, ident='test_tableobjman_simple',  parent=None, name='Test Table Object Manager'):
        self._init_objman(ident, parent=parent, name=name, xmltag=('testtab', 'row', None))

        # ATTENTION!!
        # do not use x = self.add(...) or self.add_col(...)
        # This would overwrite the configuration with the value
        # because  the attribute is the configuration, which is set by Attrman
        # While the add method is returning the value
        self.add(AttrConf('x', 0.0,
                          groupnames=['state'],
                          perm='r',
                          is_save=True,
                          unit='m',
                          metatype='length',
                          is_plugin=False,
                          name='position',
                          info='Test object x position',
                          xmltag='pos',
                          ))

        self.add(AttrConf('is_pos_ok', False,
                          groupnames=['state'],
                          perm='rw',
                          is_save=True,
                          name='Pos OK',
                          info='True if position is OK',
                          xmltag='pos_ok',
                          ))

        self.add_col(ColConf('surname', 'xx',
                             groupnames=['state'],
                             perm='r',
                             is_save=True,
                             name='Family name',
                             info='Name of Family',
                             xmltag='surname',
                             ))

        self.add_col(ColConf('streetname', 'via della...',
                             groupnames=['state'],
                             perm='rw',
                             is_save=True,
                             name='Street name',
                             info='Name of the street',
                             xmltag='streetname',
                             ))

        fruits = ['allpes', 'bananas', 'oranges']
        self.add_col(ColConf('fruits', fruits[0],
                             groupnames=['state'],
                             choices=fruits,
                             perm='rw',
                             is_save=False,
                             name='Fruits',
                             info='Choose a fruit.',
                             ))

        self.add_col(NumcolConf('distances', 0.0,
                                digits_integer=None, digits_fraction=4,
                                minval=0.0, maxval=None,
                                groupnames=['state'],
                                perm='rw',
                                is_save=True,
                                name='Distance',
                                unit='m',
                                info='Distance of the street',
                                xmltag='distances',
                                ))

        self.add(FuncConf('new_row', 'on_new_row', None,
                          groupnames=['rowfunctions', '_private'],
                          name='New row',
                          info='Add a new row.',
                          ))
        self.add(FuncConf('delete_row', 'on_del_row', None,
                          groupnames=['rowfunctions', '_private'],
                          name='Del row',
                          #info = 'Delete a row.',
                          ))

        #_id = attrsman.suggest_id()
        # print '_id =',_id
        # self.attrman.add(_id)

        # print 'self.streetname',self.streetname,type(self.streetname)
        # self.streetname[1]='yyy'
        # print 'self.streetname',self.streetname,type(self.streetname)
        self.add_rows(5)
        self.streetname[3] = 'ssss'
        self.surname[[1, 2, 3, 4]] = ['a', 'bb', 'ccc', 'dddd']
        self.streetname[[1, 2]] = ['vv', 'dd']
        # print '\n\ntest get',self.streetname[[1,2,3]]
        # self.streetname[1]+='zzz'
        # self.del_rows([1,3])
        # self.del_row(5)
        # self.delete('streetname')
        # self.delete('is_pos_ok')
        # print 'dir',dir(self)

    def on_new_row(self, ids):
        """
        True if position greater than thhreshold.
        """
        self.add_row()

    def on_del_row(self, id_row):
        """
        True if position greater than thhreshold.
        """
        print 'on_del_row', id_row
        self.del_row(id_row)
        print '  ids after del', self.get_ids()


class TestTableObjManNocols(TableObjman):
    """
    Table manager without columns...for test purposes
    """

    def __init__(self, ident='test_tableobjman_simple_nocols',  parent=None, name='Test Table Object Manager'):
        self._init_objman(ident, parent=parent, name=name)

        # ATTENTION!!
        # do not use x = self.add(...) or c=self.add_col(...)
        # This would overwrite the configuration with the value
        # because  the attribute is the configuration, which is set by Attrman
        # While the add method is returning the value
        self.add(AttrConf('x', 0.0,
                          groupnames=['state'],
                          perm='r',
                          is_save=True,
                          unit='m',
                          metatype='length',
                          is_plugin=False,
                          name='position',
                          info='Test object x position',
                          ))

        self.add(AttrConf('is_pos_ok', False,
                          groupnames=['state'],
                          perm='rw',
                          is_save=True,
                          name='Pos OK',
                          info='True if position is OK',
                          ))


class ZonesTab(ArrayObjman):
    def __init__(self, ident,  parent=None, **kwargs):
        self._init_objman(ident, parent=parent, **kwargs)

        self.add_col(ColConf('zonetypes', 0,
                             choices={
                                 "priority": 0,
                                 "traffic_light": 1,
                                 "right_before_left": 2,
                                 "unregulated": 3,
                                 "priority_stop": 4,
                                 "traffic_light_unregulated": 5,
                                 "allway_stop": 6,
                                 "rail_signal": 7,
                                 "zipper": 8,
                                 "traffic_light_right_on_red": 9,
                                 "rail_crossing": 10,
                             },
                             is_plugin=True,
                             #dtype = np.int32,
                             perm='rw',
                             #is_index = True,
                             name='Type',
                             info='Zone type.',
                             ))

        self.add_col(NumcolConf('shapes', [],
                                groupnames=['state'],
                                perm='rw',
                                is_plugin=True,
                                is_save=True,
                                name='Shape',
                                info='Shape of zone which is a list of (x,y) coordinates',
                                ))


class OdTripsTab(ArrayObjman):
    def __init__(self, ident,  parent, zones, **kwargs):
        self._init_objman(ident, parent=parent, **kwargs)

        self.add_col(IdsArrayConf('ids_orig', zones,
                                  groupnames=['state'],
                                  is_save=True,
                                  name='ID Orig',
                                  info='ID of traffic assignment zone of origin of trip.',
                                  ))

        self.add_col(IdsConf('ids_dest', zones,
                             groupnames=['state'],
                             is_save=True,
                             name='ID Dest',
                             info='ID of traffic assignment zone of destination of trip.',
                             ))

        self.add_col(NumArrayConf('tripnumbers', 0,
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

        self.add_col(NumArrayConf('t_start', 0.0,
                                  groupnames=['state'],
                                  perm='rw',
                                  is_save=True,
                                  name='Start time',
                                  unit='s',
                                  info='Start time of interval',
                                  ))

        self.add_col(NumArrayConf('t_end', 3600.0,
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
demand.zones = demand.get_attrsman().add(ObjConf(zones))
EVTDELITEM = 20  # delete attribute
EVTSETITEM = 21  # set attribute
EVTGETITEM = 22  # get attribute
EVTADDITEM = 23  # add/create attribute
zones.shapes.plugin.add_event(EVTADDITEM, on_event_additem)
shapes = [[(0.0, 10.0), (10.0, 10.0), (10.0, 0.0)],
          [(10.0, 20.0), (20.0, 20.0), (20.0, 10.0)],
          [(20.0, 30.0), (30.0, 20.0), (30.0, 20.0)],
          ]
zones.add_rows(3, shapes=shapes)

odintervals = OdIntervalsTab('odintervals', parent=demand, info='OD data for different time intervals')
demand.odintervals = demand.get_attrsman().add(ObjConf(odintervals))
odintervals.add_rows(2, t_start=[0, 3600], t_end=[3600, 7200])
for id_odmodes in odintervals.get_ids():
    odmodes = OdModesTab((odintervals.odmodes.attrname, id_odmodes), parent=odintervals)
    odintervals.odmodes[id_odmodes] = odmodes

    odmodes.add_rows(2)
    for id_odtrips in odmodes.get_ids():
        odtrips = OdTripsTab((odmodes.odtrips.attrname, id_odtrips), odmodes, zones)
        odtrips.add_rows(3, ids_orig=[3, 2, 1], ids_dest=[3, 3, 3], tripnumbers=[10, 200, 555])
        odmodes.odtrips[id_odtrips] = odtrips

# demand.attrsman.print_attrs()
# odintervals.print_attrs()

# -------------------------------------------------------------------------------


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
drawing = BaseObjman('drawing')

lines = Lines('lines', parent=drawing)
drawing.lines = drawing.get_attrsman().add(ObjConf(lines))
lines.add_rows(3, vertices=vertices, polygons=polygons, ids_sumo=ids_sumo)

triangles = Lines('triangles', parent=drawing)
drawing.triangles = drawing.get_attrsman().add(ObjConf(triangles))
triangles.add_rows(3, vertices=2*vertices, polygons=polygons, ids_sumo=['xxx10', 'xx22', 'xx333'])

selection = Selection('selection', parent=drawing)
drawing.selection = drawing.get_attrsman().add(ObjConf(selection))
selection.add_rows(2, obj_ids=[(lines, 2), (triangles, 1)])

collections = Collection('collections', parent=drawing)
drawing.collections = drawing.get_attrsman().add(ObjConf(collections))
collections.add_rows(2, tab_id_lists=[[(lines, 2), (triangles, 1)],
                                      [(lines, 2), (triangles, 1), (lines, 1)]]
                     )

# -------------------------------------------------------------------------------

pointvertices = [
                [0.0, 0.0, 0.0],
                [0.2, 0.0, 0.0],
                [0.3, 0.5, 0.0],
                [0.2, 0.5, 0.0],
                [0.0, 0.5, 0.1],
                [-1.5, -0.5, 0.0],
]

pointvertices2 = [
    [0.0, 0.3, 0.0],
    [0.2, 0.3, 0.0],
    [0.3, 0.8, 0.0],
    [0.2, 0.8, 0.0],
    [0.0, 0.8, 0.1],
    [-1.5, -0.8, 0.0],
]

pointvertices3 = [
    [0.5, 0.3, 0.0],
    [-1.5, -0.8, 0.0],
]


polylines = Polylines()
polylines.draw(pointvertices, 'aa10&1')
polylines.draw(pointvertices2, 'bb2210&1')
polylines.draw(pointvertices3, '5b2310&1')
polylines.print_attrs()
xm.write_obj_to_xml(polylines, 'test_polylines.xml')
