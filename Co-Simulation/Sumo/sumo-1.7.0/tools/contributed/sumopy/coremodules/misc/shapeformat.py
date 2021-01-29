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

# @file    shapeformat.py
# @author  Joerg Schweizer
# @date


import time
import os
import types
import re
import numpy as np
from xml.sax import saxutils, parse, handler

import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
from agilepy.lib_base.processes import Process  # ,CmlMixin,ff,call
#from coremodules.scenario import scenario
from agilepy.lib_base.misc import get_inversemap
import shapefile


try:
    try:
        import pyproj
        IS_PROJ = True
    except:
        from mpl_toolkits.basemap import pyproj
        IS_PROJ = True

except:
    IS_PROJ = False
    print 'Import error: in order to run the traces plugin please install the following modules:'
    print '   mpl_toolkits.basemap and shapely'
    print 'Please install these modules if you want to use it.'
    print __doc__
    raise


try:
    from osgeo import osr
    IS_GDAL = True
except:
    IS_GDAL = False
    print 'WARNING: GDAL module is not installed.'


# Value Shape Type
SHAPETYPES = {
    'Null Shape': 0,
    'Point': 1,
    'PolyLine': 3,
    'Polygon': 5,
    'MultiPoint': 8,
    'PointZ': 11,
    'PolygonZ': 13,
    'MultiPointZ': 15,
    'PointM': 21,
    'PolyLineM': 23,
    'PolygonM': 25,
    'MultiPointM': 28,
    'MultiPatch': 31,
}
# 0 Null Shape
# 1 Point
# 3 PolyLine
# 5 Polygon
# 8 MultiPoint
# 11 PointZ
# 13 PolyLineZ
# 15 PolygonZ
# 18 MultiPointZ
# 21 PointM
# 23 PolyLineM
# 25 PolygonM
# 28 MultiPointM
# 31 MultiPatch

VEHICLECLASSCODE = {
    "": 'X',  # All
    "private": 'I',
    "emergency": 'E',
    "authority": 'I',
    "army": 'I',
    "vip": 'I',
    "passenger": 'I',
    "hov": 'I',
    "taxi": 'P',
    "bus": 'P',
    "coach": 'P',
    "delivery": 'I',
                "truck": 'I',
                "trailer": 'I',
                "tram": 'P',
                "rail_urban": 'P',
                "rail": 'P',
                "rail_electric": 'P',
                "motorcycle": 'I',
                "moped": 'I',
                "bicycle": 'B',
                "pedestrian": 'F',
                "evehicle": 'I',
                "custom1": 'P',
                "custom2": 'I',
}


def guess_utm_from_coord(coord):
    """
    Returns UTM projection parameters from  an example point 
    with coord = (lat,lon)
    """
    zone = get_zone(coord)
    return "+proj=utm +zone=%d +ellps=WGS84 +datum=WGS84 +units=m +no_defs" % zone


def get_zone(coordinates):
    if 56 <= coordinates[1] < 64 and 3 <= coordinates[0] < 12:
        return 32
    if 72 <= coordinates[1] < 84 and 0 <= coordinates[0] < 42:
        if coordinates[0] < 9:
            return 31
        elif coordinates[0] < 21:
            return 33
        elif coordinates[0] < 33:
            return 35
        return 37
    return int((coordinates[0] + 180) / 6) + 1


def get_letter(coordinates):
    return 'CDEFGHJKLMNPQRSTUVWXX'[int((coordinates[1] + 80) / 8)]


def get_shapeproj(projparams):
    """
    Returns text for shape .prj file
    This makes use of links like 
    http://spatialreference.org/ref/epsg/4326/prj/
    http://spatialreference.org/ref/epsg/wgs-84-utm-zone-32n/prj/
    +proj=utm +zone=32 +ellps=WGS84 +datum=WGS84 +units=m +no_defs
    +init=EPSG:23032
    """
    import urllib
    # print 'get_shapeproj',projparams
    params = {}
    for elem in projparams.split('+'):
        if elem.find('=') > 0:
            attr, val = elem.split('=')
            params[attr.strip()] = val.strip()
    # print 'params',params

    if params.has_key('init'):
        if params['init'].lower().find('epsg') >= 0:
            epgs, number = params['init'].lower().split(':')
            # print   epgs,number
            html = 'http://spatialreference.org/ref/epsg/%s/prj/' % number

    elif params.has_key('datum'):
        if params['datum'].lower().find('wgs') >= 0:
            number = params['datum'][3:]
            # print 'wgs', params['zone']+'n'
            html = 'http://spatialreference.org/ref/epsg/wgs-%s-%s-zone-%sn/prj/' % (
                number, params['proj'], params['zone'])

    # print 'html=',html
    f = urllib.urlopen(html)
    return (f.read())


def get_proj4_from_shapefile(filepath):

    parts = os.path.basename(filepath).split('.')
    basename = ''
    for part in parts[:-1]:
        basename += part

    dirname = os.path.dirname(filepath)

    shapefilepath = os.path.join(dirname, basename)

    projparams = ''
    projfilepath = shapefilepath+'.prj'
    if not os.path.isfile(projfilepath):
        projfilepath = shapefilepath+'.PRJ'

    if os.path.isfile(projfilepath):
        prj_file = open(projfilepath, 'r')
        prj_txt = prj_file.read()
        proj4 = ''
        if IS_GDAL:
            srs = osr.SpatialReference()
            if srs.ImportFromWkt(prj_txt):
                raise ValueError("Error importing PRJ information from: %s" % shapeprojpath)
            # srs.ImportFromESRI([prj_txt])
            srs.AutoIdentifyEPSG()
            # print 'Shape prj is: %s' % prj_txt
            # print 'WKT is: %s' % srs.ExportToWkt()
            # print 'Proj4 is: %s' % srs.ExportToProj4()

            # print 'EPSG is: %s' % srs.GetAuthorityCode(None)
            proj4 = srs.ExportToProj4()

        else:
            res = re.search(r'(GCS)_([a-zA-Z]+)_([0-9]+)', prj_txt)
            if res is not None:
                groups = res.groups()
                if len(groups) == 3:
                    gcs, proj, date = groups
                    proj4 = '+proj=longlat +datum=%s%s +no_defs' % (proj, date[2:])
    else:
        print 'WARNING: found no prj file', projfilepath
    return proj4
    # return "+proj=utm +zone=32 +ellps=WGS84 +datum=WGS84 +units=m +no_defs"
    # return '+proj=longlat +datum=WGS84 +ellps=WGS84 +a=6378137.0 +f=298.257223563 +pm=0.0  +no_defs'


def get_shapefile(filepath):
    """
    Returns shapefile handler and proj4 parameter string
    of shape file with given path.
    """
    parts = os.path.basename(filepath).split('.')
    basename = ''
    for part in parts[:-1]:
        basename += part

    dirname = os.path.dirname(filepath)

    shapefilepath = os.path.join(dirname, basename)

    print 'import_shapefile *%s*' % (shapefilepath), type(str(shapefilepath))
    sf = shapefile.Reader(str(shapefilepath))

    return sf


def get_fieldinfo(field):
    """
    Returns attrname,default,dtype,digits_fraction
    """

    attrname, ftype, flen, fdigit = field
    attrname = attrname.strip()

    dtype = 'object'
    default = ''
    digits_fraction = None

    if ftype == 'C':
        dtype = 'object'
        default = ''

    elif ftype == 'N':
        if fdigit > 0:
            digits_fraction = fdigit
            dtype = 'float32'
            default = 0.0
        else:
            dtype = 'int32'
            default = 0

    return attrname, default, dtype, digits_fraction


class ShapefileImporter(Process):
    def __init__(self,  filepath,
                 coordsconfig,
                 map_attrconfig2shapeattr=None,
                 attrconfigs=None,
                 ident='shapefileimporter', parent=None,
                 name='Shapefile importer',
                 info='Import of shape files',
                 logger=None,
                 **kwargs):

        self._filepath = filepath
        self._sf = get_shapefile(filepath)
        projparams_shape = get_proj4_from_shapefile(filepath)
        projparams_target_default = self.guess_targetproj()

        print 'ShapefileImporter.__init__', filepath  # ,projparams_target_default, projparams_shape
        self._init_common(ident,
                          parent=parent,
                          name=name,
                          logger=logger,
                          info='Import workouts and GPS points of a European cycling challange.',
                          )

        attrsman = self.set_attrsman(cm.Attrsman(self))

        # if projparams_target == '':
        #    projparams_target = ''
        #    is_guess_targetproj = True
        #    is_use_shapeproj = False

        #projparams_shape = projparams_shapefile

        self.make_fieldinfo()

        # self.id_mode = attrsman.add(am.AttrConf('id_mode',  modechoices['bicycle'],
        #                                groupnames = ['options'],
        #                                choices = modechoices,
        #                                name = 'Mode',
        #                                info = 'Transport mode to be matched.',
        #                                ))

        self.filepath = attrsman.add(
            cm.AttrConf('filepath', filepath,
                        groupnames=['parameters'],
                        perm='r',
                        name='Shape file',
                        wildcards='Shape file (*.shp)|*.shp',
                        metatype='filepath',
                        info="""File path of shape file.""",
                        ))

        self.projparams_shape = attrsman.add(cm.AttrConf('projparams_shape', projparams_shape,
                                                         groupnames=['parameters', ],
                                                         perm='rw',
                                                         name='Shape projection',
                                                         info='Projection4 parameters of shape data.',
                                                         ))

        self.is_guess_targetproj = attrsman.add(cm.AttrConf('is_guess_targetproj', kwargs.get("is_guess_targetproj", True),
                                                            groupnames=['parameters', ],
                                                            perm='r',
                                                            name='Guess target projection?',
                                                            info='If selected, target projection will be guessed based on coordinates of the shapefile.',
                                                            ))

        self.projparams_target = attrsman.add(cm.AttrConf('projparams_target', kwargs.get("projparams_target", projparams_target_default),
                                                          groupnames=['parameters', ],
                                                          perm='r',
                                                          name='Target projection',
                                                          info='Projection4 parameters of target, where the coordinates are imported. These are typically the scenario coordinates.',
                                                          ))

        self.is_use_shapeproj = attrsman.add(cm.AttrConf('is_use_shapeproj', kwargs.get("is_use_shapeproj", False),
                                                         groupnames=['parameters', ],
                                                         perm='r',
                                                         name='Use shapefile projection?',
                                                         info='If selected, projection in shape file will be used to interpret projection. If not selected, target projection will be used.',
                                                         ))

        self.is_use_targetproj = attrsman.add(cm.AttrConf('is_use_targetproj', kwargs.get("is_use_targetproj", True),
                                                          groupnames=['parameters', ],
                                                          perm='r',
                                                          name='Use target projection?',
                                                          info='If selected, target will be used to interpret projection.',
                                                          ))

        self.is_autoffset = attrsman.add(cm.AttrConf('is_autoffset', kwargs.get("is_autoffset", True),
                                                     groupnames=['parameters', ],
                                                     perm='r',
                                                     name='Auto offset?',
                                                     info='If selected, offset will be determined automatically.',
                                                     ))

        self.offset = attrsman.add(cm.AttrConf('offset', kwargs.get("offset", np.array([0.0, 0.0, 0.0], dtype=np.float32)),
                                               groupnames=['parameters', ],
                                               perm='r',
                                               name='Offset',
                                               info='Network offset in WEP coordinates',
                                               ))

        self._coordsconfig = coordsconfig

        if map_attrconfig2shapeattr is None:
            # generate attrconfs with group 'options'
            # and default attrconfmap
            self._map_attrconfig2shapeattr = {}

        else:
            self._map_attrconfig2shapeattr = map_attrconfig2shapeattr

    def make_fieldinfo(self):

        self._fieldinfo = {}
        fields = self._sf.fields
        #records = sf.records()
        for ind_field, field in zip(xrange(1, len(fields)), fields[1:]):
            attrname, default, dtype, digits_fraction = get_fieldinfo(field)
            self._fieldinfo[attrname] = (ind_field-1, default, dtype, digits_fraction)

    def get_projections(self):

        proj1 = None
        proj2 = None

        if self.is_guess_targetproj:
            self.projparams_target = self.guess_targetproj()
        # print 'get_projections'
        # print '  projparams_shape',self.projparams_shape
        # print '  projparams_target',self.projparams_target
        if IS_PROJ:
            try:
                proj_shape = pyproj.Proj(self.projparams_shape)
            except:
                proj_shape = None

            try:
                proj_target = pyproj.Proj(self.projparams_target)
            except:
                proj_target = None

            if self.is_use_targetproj & (proj_target is not None):
                if (self.is_use_shapeproj) & (proj_shape is not None):
                    proj1 = proj_shape
                    proj2 = proj_target
                else:
                    proj1 = proj_target

            elif (self.is_use_shapeproj) & (proj_shape is not None):
                proj1 = proj_shape

            return proj1, proj2, self.offset

        else:
            # no projection available
            return None, None, self.offset

    def guess_targetproj(self):
        shapes = self._sf.shapes()
        #shapetype = shapes[3].shapeType
        shape_rec = self._sf.shapeRecord(0)
        points = shape_rec.shape.points
        return guess_utm_from_coord(points[0])

    def _get_attrconfs_shapeinds(self):
        attrconfs = []
        shapeinds = []
        for attrconf, shapeattrname in self._map_attrconfig2shapeattr.iteritems():
            attrconfs.append(attrconf)
            shapeinds.append(self._fieldinfo[shapeattrname][0])
        return attrconfs, shapeinds

    def is_ready(self):
        return True

    def import_shapes(self):

        shapes = self._sf.shapes()
        shapetype = shapes[3].shapeType
        records = self._sf.records()
        n_records = len(records)
        if n_records == 0:
            return False

        proj_shape, proj_target, offset = self.get_projections()

        if proj_shape is None:
            return []

        if self.is_autoffset:
            offset = np.array([0.0, 0.0, 0.0], dtype=np.float32)

        coordsconfig = self._coordsconfig
        ids = coordsconfig.get_manager().add_rows(n_records)
        print 'import_shape n_records', n_records, shapetype

        if shapetype == 3:
            for ind_rec, id_attr in zip(xrange(n_records), ids):
                print '  ind_rec', ind_rec
                shape_rec = self._sf.shapeRecord(ind_rec)
                points = shape_rec.shape.points
                print '  points', points
                n_points = len(points)
                shape = np.zeros((n_points, 3), dtype=np.float32) + offset
                if proj_target is not None:
                    for ind, point in zip(xrange(n_points), points):
                        shape[ind, 0:2] += np.array(pyproj.transform(proj_shape, proj_target, point[0], point[1]))
                else:
                    for ind, point in zip(xrange(n_points), points):
                        shape[ind, 0:2] += proj_shape(point[0], point[1])

                coordsconfig[id_attr] = list(shape)

        elif shapetype == 1:
            for ind_rec, id_attr in zip(xrange(n_records), ids):
                print '  ind_rec', ind_rec, id_attr
                shape_rec = self._sf.shapeRecord(ind_rec)
                points = shape_rec.shape.points
                print '  points', points
                n_points = len(points)
                vert = offset.copy()
                point = points[0]
                if proj_target is not None:
                    x, y = pyproj.transform(proj_shape, proj_target, point[0], point[1])
                    # print '  x,y',x,y
                    vert[0:2] += [x, y]
                    #shape[0:2] = pyproj.transform(proj_shape, proj_target ,point[0], point[1])
                else:
                    x, y = proj_shape(point[0], point[1])
                    # print '  x,y',x,y
                    vert[0:2] += [x, y]
                    #vert[0:2] = proj_shape(point[0], point[1])

                # print ' vert',vert
                coordsconfig[id_attr] = vert

            if self.is_autoffset:
                coords = coordsconfig.get_value()

                x_min, y_min, z_min = np.min(coords, 0)
                # print '  x_min,y_min,z_min',x_min,y_min,z_min

                coordsconfig.set_value(coords-np.min(coords, 0))
        #
        return ids

    def do(self):
        print self.ident+'.do'
        #fields = self._sf.fields
        #records = self._sf.records()
        attrconfs, shapeinds = self._get_attrconfs_shapeinds()

        # import shape info
        ids = self.import_shapes()

        n_attrs = len(attrconfs)
        n_records = len(ids)

        # import no attributes from table
        if (n_attrs == 0) | (n_records == 0):
            return True  # successfully imported no data
        shaperecords = self._sf.shapeRecord

        print '  attrconfs', attrconfs
        print '  shapeinds', shapeinds
        print '  fieldinfo', self._fieldinfo

        # return True

        return self.import_data(shaperecords, ids, attrconfs, shapeinds)

    def import_data(self, shaperecords, ids, attrconfs, shapeinds):

        n_records = len(ids)

        values_invalid = ['NULL', '\n']
        for ind_rec, id_attr in zip(xrange(n_records), ids):
            shape_rec = shaperecords(ind_rec)

            # print '  shape_rec',id_attr,shape_rec.record
            # use first field as id, but will also be a regular attribute
            #id_egde = shape_rec.record[0]
            #attrrow = {}
            # print '\n id_egde',id_egde
            for ind_sfield, attrconf in zip(shapeinds, attrconfs):
                # for i,field in zip(xrange(n),fields[1:]):
                val = shape_rec.record[ind_sfield]
                # print '    ind_sfield',ind_sfield,'attrname',attrconf.attrname,'type',type(val),'>>%s<<'%(repr(val))
                if val not in values_invalid:
                    attrconf[id_attr] = val


class OxNodesImporter(ShapefileImporter):
    def __init__(self,  filepath, oximporter, **kwargs):
        net = oximporter.get_scenario().net
        ShapefileImporter.__init__(self, filepath,
                                   net.nodes.coords,
                                   parent=oximporter,
                                   map_attrconfig2shapeattr={
                                       net.nodes.ids_sumo: 'osmid',
                                       net.nodes.types: 'highway'},
                                   #logger = kwargs['logger'],
                                   **kwargs)

    def import_data(self, shaperecords, ids, attrconfs, shapeinds):
        net = self.parent.get_scenario().net
        nodetypes = net.nodes.types
        typemap = nodetypes.choices

        sep = ';'
        # "priority":0,
        # "traffic_light":1,
        # "right_before_left":2,
        # "unregulated":3,
        # "priority_stop":4,
        # "traffic_light_unregulated":5,
        # "allway_stop":6,
        # "rail_signal":7,
        # "zipper":8,
        # "traffic_light_right_on_red":9,
        # "rail_crossing":10,
        # "dead_end":11,

        map_shapetype_sumotype = {
            'motorway_junction': typemap['zipper'],
            'crossing': typemap['rail_crossing'],
            'traffic_signals': typemap['traffic_light'],
        }

        n_records = len(ids)

        values_invalid = ['NULL', '\n']
        for ind_rec, id_attr in zip(xrange(n_records), ids):
            shape_rec = shaperecords(ind_rec)

            # print '  shape_rec',id_attr,shape_rec.record
            # use first field as id, but will also be a regular attribute
            #id_egde = shape_rec.record[0]
            #attrrow = {}
            # print '\n id_egde',id_egde
            for ind_sfield, attrconf in zip(shapeinds, attrconfs):
                # for i,field in zip(xrange(n),fields[1:]):
                val = shape_rec.record[ind_sfield]
                if attrconf == nodetypes:
                    elements = val.split(sep)
                    for element in elements:
                        if element in map_shapetype_sumotype:
                            nodetypes[id_attr] = map_shapetype_sumotype[element]
                            break

                else:
                    # map_shapetype_sumotype
                    # print '    ind_sfield',ind_sfield,'attrname',attrconf.attrname,'type',type(val),'>>%s<<'%(repr(val))

                    if val not in values_invalid:
                        attrconf[id_attr] = val


class OxEdgesImporter(ShapefileImporter):
    def __init__(self,  filepath, oximporter, **kwargs):
        net = oximporter.get_scenario().net
        ShapefileImporter.__init__(self, filepath,
                                   net.edges.shapes,
                                   parent=oximporter,
                                   map_attrconfig2shapeattr={\
                                       # net.edges.ids_sumo:'osmid',
                                       # net.edges.types:'highway',
                                       # net.edges.nums_lanes:'lanes',
                                       net.edges.lengths: 'length',
                                       net.edges.widths: 'width',
                                       net.edges.names: 'name',
                                   },
                                   **kwargs)

        # self._map_shapeattr_func = {\
        #                            'to':self.set_tonode,
        #                            'from':self.set_fromnode,
        #                            'maxspeed':self.set_speed_max,
        #                            }

    def import_shapes(self):

        shapes = self._sf.shapes()
        shapetype = shapes[3].shapeType
        records = self._sf.records()
        n_records = len(records)
        if n_records == 0:
            return False

        proj_shape, proj_target, offset = self.get_projections()

        if proj_shape is None:
            return False

        if self.is_autoffset:
            offset = np.array([0.0, 0.0, 0.0], dtype=np.float32)

        coordsconfig = self._coordsconfig
        ids = coordsconfig.get_manager().add_rows(n_records)
        print 'import_shape n_records', n_records, shapetype

        if shapetype == 3:
            for ind_rec, id_attr in zip(xrange(n_records), ids):
                print '  ind_rec', ind_rec
                shape_rec = self._sf.shapeRecord(ind_rec)
                points = shape_rec.shape.points
                print '  points', points
                n_points = len(points)
                shape = np.zeros((n_points, 3), dtype=np.float32) + offset
                if proj_target is not None:
                    for ind, point in zip(xrange(n_points), points):
                        shape[ind, 0:2] += np.array(pyproj.transform(proj_shape, proj_target, point[0], point[1]))
                else:
                    for ind, point in zip(xrange(n_points), points):
                        shape[ind, 0:2] += proj_shape(point[0], point[1])

                coordsconfig[id_attr] = list(shape)

        elif shapetype == 1:
            for ind_rec, id_attr in zip(xrange(n_records), ids):
                print '  ind_rec', ind_rec, id_attr
                shape_rec = self._sf.shapeRecord(ind_rec)
                points = shape_rec.shape.points
                print '  points', points
                n_points = len(points)
                vert = offset.copy()
                point = points[0]
                if proj_target is not None:
                    x, y = pyproj.transform(proj_shape, proj_target, point[0], point[1])
                    # print '  x,y',x,y
                    vert[0:2] += [x, y]
                    #shape[0:2] = pyproj.transform(proj_shape, proj_target ,point[0], point[1])
                else:
                    x, y = proj_shape(point[0], point[1])
                    # print '  x,y',x,y
                    vert[0:2] += [x, y]
                    #vert[0:2] = proj_shape(point[0], point[1])

                # print ' vert',vert
                coordsconfig[id_attr] = vert

            if self.is_autoffset:
                coords = coordsconfig.get_value()

                x_min, y_min, z_min = np.min(coords, 0)
                # print '  x_min,y_min,z_min',x_min,y_min,z_min

                coordsconfig.set_value(coords-np.min(coords, 0))
        #
        return ids

    def make_structure(self):
        """
        Make a dictionary self._structure
        with shape row index as key
        and lane data as value.
        Lane data has the following format:
        ([])

        """
        net = self.parent.get_scenario().net
        fieldinfo = self._fieldinfo

        ind_highway = fieldinfo['highway']
        ind_oneway = fieldinfo['oneway']
        ind_lanes = fieldinfo['lanes']
        ind_maxspeed = fieldinfo['maxspeed']

        shapes = self._sf.shapes()
        shapetype = shapes[3].shapeType
        records = self._sf.records()
        n_records = len(records)
        if n_records == 0:
            return False
        for ind_rec, id_attr in zip(xrange(n_records), ids):
            pass

    def do(self):
        print self.ident+'.do'
        #fields = self._sf.fields
        #records = self._sf.records()
        attrconfs, shapeinds = self._get_attrconfs_shapeinds()

        # import shape info
        ids = self.import_shapes()

        n_attrs = len(attrconfs)
        n_records = len(ids)

        # import no attributes from table
        if (n_attrs == 0) | (n_records == 0):
            return True  # successfully imported no data
        shaperecords = self._sf.shapeRecord

        print '  attrconfs', attrconfs
        print '  shapeinds', shapeinds
        print '  fieldinfo', self._fieldinfo

        # return True

        return self.import_data(shaperecords, ids, attrconfs, shapeinds)

    def import_data(self, shaperecords, ids, attrconfs, shapeinds):
        net = self.parent.get_scenario().net
        fieldinfo = self._fieldinfo

        #funcs = []
        #funcshapeinds = []

        # for attrconf, func in self._map_shapeattr_func.iteritems():
        #    funcs.append(attrconf)
        #    shapeinds.append(self._fieldinfo[shapeattrname][0])

        ind_from = fieldinfo['from']
        ind_to = fieldinfo['to']
        ind_highway = fieldinfo['highway']
        ind_oneway = fieldinfo['oneway']
        ind_lanes = fieldinfo['lanes']
        ind_maxspeed = fieldinfo['maxspeed']

        n_records = len(ids)
        ids_fromnode = net.edges.ids_fromnode
        ids_tonode = net.edges.ids_tonode
        speeds_max = net.edges.speeds_max
        nums_lanes = net.edges.nums_lanes

        nodemap = net.nodes.get_indexmap()
        sep = ';'
        sepc = ','
        values_invalid = ['NULL', '\n']
        for ind_rec, id_edge in zip(xrange(n_records), ids):
            shape_rec = shaperecords(ind_rec)
            for ind_sfield, attrconf in zip(shapeinds, attrconfs):
                # for i,field in zip(xrange(n),fields[1:]):
                val = shape_rec.record[ind_sfield]
                if val not in values_invalid:
                    attrconf[id_edge] = val

            ids_fromnode[id_edge] = nodemap[shape_rec[ind_from]]
            ids_tonode[id_edge] = nodemap[shape_rec[ind_from]]

            val = shape_rec[ind_maxspeed]
            if val not in values_invalid:
                speeds_max[id_edge] = float(val)/3.6

            val = shape_rec[ind_lanes]
            n_lanes = 1
            if val.isdigit():
                n_lanes = int(val)

            elif val not in values_invalid:
                lanedata = shape_rec[val].split(sep)
                if len(lanedata) > 0:
                    pass

        return True


class OxImporter(Process):
    def __init__(self,   scenario,
                 nodeshapefilepath, edgeshapefilepath, polyshapefilepath,
                 ident='oximporter',
                 name='OSMnx importer',
                 info='Import of network imported with the help of osmnx.',
                 logger=None, **kwargs):

        print 'OxImporter.__init__'

        self._init_common(ident,
                          parent=scenario,
                          name=name,
                          logger=logger,
                          info=info,
                          )

        attrsman = self.set_attrsman(cm.Attrsman(self))

        # self.id_mode = attrsman.add(am.AttrConf('id_mode',  modechoices['bicycle'],
        #                                groupnames = ['options'],
        #                                choices = modechoices,
        #                                name = 'Mode',
        #                                info = 'Transport mode to be matched.',
        #                                ))

        self.nodeshapefilepath = attrsman.add(
            cm.AttrConf('nodeshapefilepath', nodeshapefilepath,
                        groupnames=['options'],
                        perm='r',
                        name='Node shape file',
                        wildcards='Node Shape file (*.shp)|*.shp',
                        metatype='filepath',
                        info="""File path of node shape file.""",
                        ))

        # def import_xnet(net, nodeshapefilepath, edgeshapefilepath, polyshapefilepath):
        #import_nodeshapes(net, nodeshapefilepath)

    def get_scenario(self):
        return self.parent

    def do(self):
        print self.ident+'.do'

        projparams_target = self.parent.net.get_projparams()
        if projparams_target in ("", "!"):
            # net has no valid projection
            projparams_target = ""
            is_guess_targetproj = True
        else:
            is_guess_targetproj = True

        OxNodesImporter(self.nodeshapefilepath, self,
                        projparams_target=projparams_target,
                        is_guess_targetproj=is_guess_targetproj,
                        logger=self.get_logger(),
                        ).do()

        # ShapefileImporter(  self.nodeshapefilepath,
        #                    self.parent.net.nodes.coords,
        #                    map_attrconfig2shapeattr = {},
        #                    projparams_target = projparams_target,
        #                    is_guess_targetproj = is_guess_targetproj,
        #                    logger = self.get_logger(),
        #                    ).do()
        return True


class Shapedata(am.ArrayObjman):
    """
    Contains data and methods for handling shapefiles.
    https://code.google.com/p/pyshp/wiki/PyShpDocs
    http://www.esri.com/library/whitepapers/pdfs/shapefile.pdf
    """

    def __init__(self, ident='shapedata', parent=None, name='Shapefile data',
                 filepath="",
                 projparams_shape="+init=EPSG:23032",
                 offset=[0.0, 0.0],
                 shapetype=3,
                 log=None, **kwargs):

        self._init_objman(ident=ident,
                          parent=parent,
                          name=name,
                          version=0.0,
                          **kwargs)

        self.add(cm.AttrConf('filepath', filepath,
                             groupnames=['params', ],
                             perm='rw',
                             name='Shape file',
                             wildcards='Shape files (*.shp)|*.shp',
                             metatype='filepath',
                             info="""Shape file path.""",
                             ))

        self.add(cm.AttrConf('_projparams', projparams_shape,
                             groupnames=['params', ],
                             perm='rw',
                             name='Projection',
                             info='Projection parameters'
                             ))

        self.add(cm.AttrConf('_offset', offset,
                             groupnames=['params', ],
                             perm='r',
                             name='Offset',
                             info='Offset in WEP coordinates'
                             ))

        self.add(cm.AttrConf('shapetype', shapetype,
                             groupnames=['params', ],
                             choices=SHAPETYPES,
                             perm='rw',
                             name='Shape Type',
                             info='Shape  type.'
                             ))

        # todo: make these init dependent on shapetype
        if shapetype == 1:  # 'Point':1,

            self.add_col(am.ArrayConf('coords',  np.zeros(3, dtype=np.float32),
                                      dtype=np.float32,
                                      groupnames=['_private'],
                                      perm='rw',
                                      name='Coords',
                                      unit='m',
                                      info='Point coordinates.',
                                      ))
        else:
            self.add_col(am.ListArrayConf('shapes',
                                          groupnames=['_private'],
                                          perm='rw',
                                          name='Shape',
                                          unit='deg',
                                          info='Shape is a list of coordinates.',
                                          #is_plugin = True,
                                          xmltag='shape',
                                          ))

        self._log = log

        self._init_attributes()
        self._init_constants()

    def _init_attributes(self):
        """
        To be overridden
        """
        pass

    def _init_constants(self):
        """
        To be overridden
        """
        pass

    def adjust_fieldlength(self):
        # print 'adjust_fieldlength'
        for attrconf in self.get_group('shapeattrs'):

            attrname, ftype, flen, fdigit = attrconf.field

            fftype = ftype.lower()
            len_max = 0
            # print '    ',attrname,attrconf.field,fftype,attrconf.get_attr()
            if fftype == 'c':
                for s in attrconf.get_value():
                    # print '    s=',s,type(s)
                    if len(s) > len_max:
                        len_max = len(s)
            elif fftype == 'n':
                for val in attrconf.get_value():
                    l = len(str(int(val)))
                    if l > len_max:
                        len_max = l
                if fdigit > 0:
                    len_max += 1+fdigit  # includes length of "." and digits

            attrconf.field = (attrname, ftype, len_max, fdigit)

    def get_basefilepath(self, filepath):

        parts = os.path.basename(filepath).split('.')
        basename = ''
        for part in parts[:-1]:
            basename += part

        dirname = os.path.dirname(filepath)
        #shapefilepath = os.path.join(dirname,basename)
        return os.path.join(dirname, basename)

    def export_shapefile(self, filepath=None):
        """
        Exports shape data into given file.
        If no file is given, the the default file path will be selected.
        """
        # https://code.google.com/p/pyshp/
        print '\nexport_shapefile', filepath
        #proj_target, offset_target = self.parent.get_proj_offset()
        if len(self) == 0:
            return False

        if filepath is None:
            filepath = self.filepath.get_value()
        else:
            self.filepath.set_value(filepath)

        basefilepath = self.get_basefilepath(filepath)
        shapetype = self.shapetype.get_value()
        w = shapefile.Writer()
        w.shapeType = shapetype
        w.autoBalance = 1
        shapes = []
        # print '  shapeattrs:',self.get_group('shapeattrs')
        attrnames = []
        for attrconf in self.get_group('shapeattrs'):
            # print '    ',attrconf.field
            w.field(*attrconf.field)
            attrnames.append(attrconf)

        # for key in self.get_keys():
        ids_shape = self.get_ids()
        offset = self._offset.get_value()

        # print '  offset',offset
        if shapetype == 1:  # 'Point':1,
            # single points need a scpecial treatment
            for id_shape, coord in zip(ids_shape, self.coords[ids_shape]):
                # print '  id_shape',id_shape,coord
                # shapes.append(self.shapes.get(key).tolist())
                # print '    shape',(np.array(shape, dtype = np.float32)[:,:2] - offset).tolist()
                w.poly(shapeType=shapetype, parts=[[coord[:2] - offset], ])
                #id = self.get_id_from_key(key)
                attrs = {}
                for attrconf in self.get_group('shapeattrs'):
                    val = attrconf[id_shape]
                    if attrconf.field[1] == 'N':
                        if val == np.nan:
                            val = 'NONE'
                    attrs[attrconf.attrname] = val
                # print '  record',key,' attrs=', attrs
                w.record(**attrs)
        else:

            for id_shape, shape in zip(ids_shape, self.shapes[ids_shape]):
                # print '  id_shape',id_shape, np.array(shape, dtype = np.float32)
                # shapes.append(self.shapes.get(key).tolist())
                # print '    shape',(np.array(shape, dtype = np.float32)[:,:2] - offset).tolist()
                w.poly(shapeType=shapetype, parts=[(np.array(shape, dtype=np.float32)[:, :2] - offset).tolist(), ])
                #id = self.get_id_from_key(key)
                attrs = {}
                for attrconf in self.get_group('shapeattrs'):
                    val = attrconf[id_shape]
                    if attrconf.field[1] == 'N':
                        if val == np.nan:
                            val = 'NONE'
                    attrs[attrconf.attrname] = val
                # print '  record',key,' attrs=', attrs
                w.record(**attrs)
        # a null shape: w.null()

        #w.poly(shapeType=3, parts=shapes)
        # try:
        w.save(filepath)
        # except:
        #    print 'WARNING in export_shapefile:\n no shapefile written. Probably fields are not large enough.'
        #    return

        try:
            shapeproj = get_shapeproj(self._projparams.get_value())
            # create the PRJ file
            prjfile = open("%s.prj" % basefilepath, "w")
            prjfile.write(shapeproj)
            prjfile.close()
            return True
        except:
            print 'WARNING in export_shapefile:\n no projection file written (probably no Internet connection).'
            print 'Open shapefile with projection: %s.' % self._projparams.get_value()
            # raise
            return False

    def add_field(self, field):
        """
        Configures field array and returns fieldname
        """
        # print 'add_field',field,'len=',len(field)
        attrname, ftype, flen, fdigit = field
        attrname = attrname.strip()

        dtype = 'object'
        digits_fraction = None

        if ftype == 'C':
            dtype = 'object'
            default = ''

        elif ftype == 'N':
            if fdigit > 0:
                digits_fraction = fdigit
                dtype = 'float32'
                default = 0.0
            else:
                dtype = 'int32'
                default = 0

        self.add_col(am.ArrayConf(attrname, default,
                                  dtype=dtype,
                                  groupnames=['shapeattrs'],
                                  perm='rw',
                                  name=attrname,
                                  field=field,
                                  info='Shape attribute: '+attrname,
                                  ))
        return attrname

    def import_shapefile(self, filepath=None, projparams=None, projparams_target=None, offset_target=(0.0, 0.0)):
        #from mpl_toolkits.basemap import pyproj
        if filepath is None:
            filepath = self.filepath.get_value()
        else:
            self.filepath.set_value(filepath)

        if projparams == None:
            # here we could guess from shapes
            projparams = "+init=EPSG:23032"

        if projparams_target != None:
            proj_target = pyproj.Proj(projparams_target)
            self._projparams, self._offset = (proj_target, offset_target)
        else:
            proj_target = None
            self._projparams, self._offset = (projparams, offset_target)

        if IS_PROJ:
            proj_shape = pyproj.Proj(projparams)
        else:
            # no projection possible
            proj_target = None
            proj_shape = None

        print 'import_shapefile *%s*' % (basefilepath), type(str(basefilepath))
        sf = shapefile.Reader(str(basefilepath))

        shapes = sf.shapes()
        self.shapetype = shapes[3].shapeType
        fields = sf.fields
        records = sf.records()

        attrnames = []
        for field in fields[1:]:
            attrnames.append(self.add_field(field))

        # print '  fields',len(fields),fields

        n = len(attrnames)
        for ind in xrange(len(records)):
            shape_rec = sf.shapeRecord(ind)

            # use first field as id, but will also be a regular attribute
            id_egde = shape_rec.record[0]
            attrrow = {}
            print '\n id_egde', id_egde
            for i, field in zip(xrange(n), fields[1:]):
                val = shape_rec.record[i]
                # print '  ',i,attrnames[i],'>>'+repr(val)+'<<', type(val)
                if field[1] == 'N':
                    if type(val) == types.StringType:
                        val = -1

                attrrow[attrnames[i]] = val

            shape = []
            if proj_target != None:

                for point in shape_rec.shape.points:
                    shape.append(np.array(pyproj.transform(proj_shape, proj_target, point[0], point[1]))+offset_target)
            else:
                for point in shape_rec.shape.points:
                    shape.append(np.array([point[0], point[1]])+offset_target)

            attrrow['shapes'] = np.array(shape, float)

            #self.set_row(id_egde, **attrrow)
            id = self.add_row(key=id_egde)

            for attrname, val in attrrow.iteritems():
                # print '  ',attrname,'>>'+repr(val)+'<<', type(val)
                getattr(self, attrname)[id] = val

        #if self._log: self._log.w('imported %d traces done.'%len(ids_trace))
        return True


def nodes_to_shapefile(net, filepath, dataname='nodeshapedata',
                       parent=None, log=None):
    """
    Export network nodes to shapefile.
    """
    nodes = net.nodes
    shapedata = Shapedata(parent, dataname, name='Node shape data',
                          filepath=filepath,
                          shapetype=SHAPETYPES['Point'],
                          projparams_shape=net.get_projparams(),
                          offset=net.get_offset(), log=log)

    #attrname, ftype, flen, fdigit = field
    attrlist = [
        ('id', 'id', 'ID_NODE', 'N', 12, 0),
        ('types', 'val', 'TYPE', 'C', 20, 0),
        # ('radii','val','RADIUS','N',5,3),
    ]

    map_nodetypes = get_inversemap(nodes.types.choices)
    nodetypes = np.zeros(max(map_nodetypes.keys())+1, dtype=np.object)
    nodetypes[map_nodetypes.keys()] = map_nodetypes.values()

    print 'nodes_to_shapefile', filepath

    for attr in attrlist:
        shapedata.add_field(attr[2:])

    ids_node = nodes.get_ids()

    ids_shape = shapedata.add_rows(len(nodes))
    # print '  shapedata.ID_ARC',shapedata.ID_ARC,'dir',dir(shapedata.ID_ARC)
    shapedata.ID_NODE[ids_shape] = ids_node
    shapedata.coords[ids_shape] = nodes.coords[ids_node]
    shapedata.TYPE[ids_shape] = nodetypes[nodes.types[ids_node]]

    # copy rest of attributes
    for netattrname, gettype, shapeattrname, x1, x2, x3 in attrlist:
        if netattrname not in ('id', 'types'):
            getattr(shapedata, shapeattrname)[ids_shape] = getattr(nodes, netattrname)[ids_node]

    shapedata.adjust_fieldlength()
    shapedata.export_shapefile()
    return True


def edges_to_shapefile(net, filepath, dataname='edgeshapedata',
                       is_access=True, parent=None, log=None):
    """
    Export network edges to shapefile.
    """
    edges = net.edges
    shapedata = Shapedata(parent, dataname, name='Edge shape data',
                          filepath=filepath,
                          shapetype=SHAPETYPES['PolyLine'],
                          projparams_shape=net.get_projparams(),
                          offset=net.get_offset(), log=log)

    #attrname, ftype, flen, fdigit = field
    attrlist = [
        ('id', 'id', 'ID_ARC', 'N', 12, 0),
        ('ids_fromnode', 'id', 'ID_FROMNOD', 'N', 12, 0),
        ('ids_tonode', 'id', 'ID_TONOD', 'N', 12, 0),
        ('lengths', 'val', 'LENGTH', 'N', 6, 3),
        ('widths', 'val', 'WIDTH', 'N', 3, 3),
        ('priorities', 'val', 'PRIO', 'N', 2, 0),
        ('nums_lanes', 'val', 'N_LANE', 'N', 2, 0),
        ('speeds_max', 'val', 'SPEED_MAX', 'N', 6, 3),
    ]

    print 'edges_to_shapefile', filepath

    for attr in attrlist:
        shapedata.add_field(attr[2:])

    if is_access:
        shapedata.add_field(('ACCESS', 'C', 64, 0))

    ids_edge = edges.get_ids()

    ids_shape = shapedata.add_rows(len(edges))
    # print '  shapedata.ID_ARC',shapedata.ID_ARC,'dir',dir(shapedata.ID_ARC)
    shapedata.ID_ARC[ids_shape] = ids_edge
    shapedata.shapes[ids_shape] = edges.shapes[ids_edge]

    for netattrname, gettype, shapeattrname, x1, x2, x3 in attrlist:
        if netattrname not in ('id', 'access'):
            getattr(shapedata, shapeattrname)[ids_shape] = getattr(edges, netattrname)[ids_edge]

    if is_access:
        accesses = np.zeros(len(ids_edge), dtype=object)
        accesses[:] = ''
        for mode, shapemode in VEHICLECLASSCODE.iteritems():
            # here we select this mode for access level 1 and 2
            #                     -1  0       1          2
            accessvec = np.array(['', 'X', shapemode, shapemode], dtype=np.unicode)

            if net.modes.has_modename(mode):
                accesslevels = edges.get_accesslevels(net.modes.get_id_mode(mode))
                accesses += accessvec[accesslevels[ids_edge]+1]

        for id_shape, access_str in zip(ids_shape, accesses):
            access = list(set(access_str))
            if len(access) == 0:
                # no dedicated access: all can go
                shapedata.ACCESS[id_shape] = 'X'
            else:
                access.sort()
                shapedata.ACCESS[id_shape] = ''.join(access)

    shapedata.adjust_fieldlength()
    shapedata.export_shapefile()
    return True


def facilities_to_shapefile(facilities, filepath, dataname='facilitydata',
                            is_access=True, parent=None, log=None):
    """
    Export network edges to shapefile.
    """
    net = facilities.get_net()
    shapedata = Shapedata(parent, dataname, name='Facilities shape data',
                          filepath=filepath,
                          shapetype=SHAPETYPES['PolyLine'],
                          projparams_shape=net.get_projparams(),
                          offset=net.get_offset(), log=log)

    #attrname, ftype, flen, fdigit = field
    attrlist = [
        ('id', 'id', 'ID_FACIL', 'N', 32, 0),
        ('ids_roadedge_closest', 'id', 'ID_ARC', 'N', 32, 0),
        ('positions_roadedge_closest', 'id', 'POS_ARC', 'N', 12, 5),
        ('osmkeys', 'val', 'OSMKEY', 'C', 32, 0),
    ]

    print 'facilities_to_shapefile', filepath

    for attr in attrlist:
        shapedata.add_field(attr[2:])

    ids_fac = facilities.get_ids()

    ids_shape = shapedata.add_rows(len(ids_fac))
    # print '  shapedata.ID_ARC',shapedata.ID_ARC,'dir',dir(shapedata.ID_ARC)
    shapedata.ID_FACIL[ids_shape] = ids_fac
    shapedata.shapes[ids_shape] = facilities.shapes[ids_fac]

    for netattrname, gettype, shapeattrname, x1, x2, x3 in attrlist:
        if netattrname not in ('id',):
            getattr(shapedata, shapeattrname)[ids_shape] = getattr(facilities, netattrname)[ids_fac]

    shapedata.adjust_fieldlength()
    shapedata.export_shapefile()
    return True
