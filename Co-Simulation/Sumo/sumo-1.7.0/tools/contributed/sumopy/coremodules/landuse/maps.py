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

# @file    maps.py
# @author  Joerg Schweizer
# @date

import os
import numpy as np
import wx
import urllib
from collections import OrderedDict
import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
import agilepy.lib_base.xmlman as xm
from agilepy.lib_base.processes import Process
#import timeit
#t = timeit.Timer()
#t_start = t.timer()

#from matplotlib import pyplot as plt

IS_MAPSUPPORT = True
try:
    from PIL import ImageFilter, Image, ImageChops, ImagePath, ImageDraw
except:
    print "WARNING: Maps requires PIL module."
    IS_MAPSUPPORT = False

try:
    import pyproj

except:
    try:
        from mpl_toolkits.basemap import pyproj

    except:
        print "WARNING: Maps requires pyproj module."
        IS_MAPSUPPORT = False
        # print __doc__
        # raise


URL_GOOGLEMAP = "http://maps.googleapis.com/maps/api/staticmap?"


def download_googlemap(filepath, bbox, proj, size=640, filetype='gif', maptype='satellite'):
    # https://developers.google.com/maps/documentation/static-maps/intro#Paths
    x_sw, y_sw = bbox[0]
    x_ne, y_ne = bbox[1]

    # 01    11
    #
    #
    # 00    10
    lon00, lat00 = proj(x_sw, y_sw, inverse=True)
    lon10, lat10 = proj(x_ne, y_sw, inverse=True)
    lon11, lat11 = proj(x_ne, y_ne, inverse=True)
    lon01, lat01 = proj(x_sw, y_ne, inverse=True)

    size_x = size_y = size/2
    urllib.urlretrieve(URL_GOOGLEMAP+"size=%dx%d&visible=%.6f,%.6f|%.6f,%.6f&format=%s&maptype=%s&scale=2"
                       % (size_x, size_y, lat00, lon00, lat11, lon11, filetype.upper(), maptype), filepath)

    bbox_lonlat = np.array([[lon00, lat00], [lon11, lat11]])
    return bbox_lonlat


def download_googlemap_bb(filepath, bbox, proj, size=640, filetype='gif', maptype='satellite', color="0xff0000ff"):
    # https://developers.google.com/maps/documentation/static-maps/intro#Paths
    x_sw, y_sw = bbox[0]
    x_ne, y_ne = bbox[1]

    # 01    11
    #
    #
    # 00    10
    lon00, lat00 = proj(x_sw, y_sw, inverse=True)
    lon10, lat10 = proj(x_ne, y_sw, inverse=True)
    lon11, lat11 = proj(x_ne, y_ne, inverse=True)
    lon01, lat01 = proj(x_sw, y_ne, inverse=True)

    size_x = size_y = size/2

    url = URL_GOOGLEMAP + \
        "size=%dx%d&format=%s&maptype=%s&scale=2&path=color:%s|weight:1" % (
            size_x, size_y, filetype.upper(), maptype, color)
    url += "|%.6f,%.6f" % (lat00, lon00)
    url += "|%.6f,%.6f" % (lat10, lon10)
    url += "|%.6f,%.6f" % (lat11, lon11)
    url += "|%.6f,%.6f" % (lat01, lon01)
    url += "|%.6f,%.6f" % (lat00, lon00)

    # urllib.urlretrieve (URL_GOOGLEMAP+"size=%dx%d&format=%s&maptype=%s&scale=2&path=color:0xff0000ff|weight:1|%.5f,%.5f|%.5f,%.5f|%.5f,%.5f|%.5f,%.5f"\
    #    %(size_x,size_y,filetype,maptype,lat00,lon00, lat11,lon11, lat01,lon01, lat10,lon10), filepath)
    # print 'url=',url
    urllib.urlretrieve(url, filepath)
    bbox_lonlat = np.array([[lon00, lat00], [lon11, lat11]])
    return bbox_lonlat


def estimate_angle(filepath,
                   rect=[(72, 36), (1243, 69), (1210, 1244), (39, 1211)],
                   ):

    im = Image.open(filepath).convert("RGB")
    print 'estimate_angle image', filepath,  "%dx%d" % im.size, im.mode, im.getbands()
    imr, img, imb = im.split()

    # calculate width and height of bbox in pixel from measured rectangle
    wr = int(np.sqrt((rect[1][0]-rect[0][0])**2+(rect[1][1]-rect[0][1])**2))
    #wr_check = int(np.sqrt((rect[2][0]-rect[3][0])**2+(rect[2][1]-rect[3][1])**2))
    hr = int(np.sqrt((rect[3][0]-rect[0][0])**2+(rect[3][1]-rect[0][1])**2))
    #h_check = int(np.sqrt((rect[2][0]-rect[1][0])**2+(rect[2][1]-rect[1][1])**2))

    xcb = im.size[0]/2
    ycb = im.size[1]/2

    bbox = [(xcb-wr/2, ycb-hr/2), (xcb+wr/2, ycb-hr/2), (xcb+wr/2, ycb+hr/2), (xcb-wr/2, ycb+hr/2), (xcb-wr/2, ycb-hr/2)]
    im_bbox = ImageChops.constant(im, 0)
    draw = ImageDraw.Draw(im_bbox)
    draw.line(bbox, fill=255)
    del draw

    angles = np.arange(-2.0, 2.0, 0.01)
    matches = np.zeros(len(angles))
    for i in xrange(len(angles)):
        im_bbox_rot = im_bbox.rotate(angles[i])  # gimp 1.62
        im_corr = ImageChops.multiply(imr, im_bbox_rot)
        # im_corr.show()
        im_corr_arr = np.asarray(im_corr)
        matches[i] = np.sum(im_corr_arr)/255
        # print ' angles[i],matches[i]',angles[i],matches[i]

    angle_opt = angles[np.argmax(matches)]
    print '  angle_opt', angle_opt
    return -angle_opt, bbox
    # im_box.show()


class MapsImporter(Process):
    def __init__(self,  maps, logger=None, **kwargs):
        print 'MapsImporter.__init__', maps, maps.parent.get_ident()
        self._init_common('mapsimporter', name='Background maps importer',
                          logger=logger,
                          info='Downloads and converts background maps.',
                          )
        self._maps = maps

        attrsman = self.set_attrsman(cm.Attrsman(self))
        #self.net = attrsman.add(   cm.ObjConf( network.Network(self) ) )
        # self.status = attrsman.add(cm.AttrConf(
        #                            'status', 'preparation',
        #                            groupnames = ['_private','parameters'],
        #                            perm='r',
        #                            name = 'Status',
        #                            info = 'Process status: preparation-> running -> success|error.'
        #                            ))

        self.width_tile = attrsman.add(cm.AttrConf('width_tile', kwargs.get('width_tile', 500.0),
                                                   groupnames=['options'],
                                                   choices=OrderedDict([("500", 500.0),
                                                                        ("1000", 1000.0),
                                                                        ("2000", 2000.0),
                                                                        ("4000", 4000.0),
                                                                        ("8000", 8000.0),
                                                                        ]),
                                                   perm='rw',
                                                   name='Tile width',
                                                   unit='m',
                                                   info='Tile width in meter of quadratic tile. This is the real width of one tile that will be downloaded.',
                                                   ))

        self.size_tile = attrsman.add(cm.AttrConf('size_tile', kwargs.get('size_tile', 1280),
                                                  groupnames=['options'],
                                                  perm='rw',
                                                  name='Tile size',
                                                  info='Tile size in pixel. This is the size of one tile that will be downloaded and determins the map resolution. Maximum is 1280.',
                                                  ))

        self.n_tiles = attrsman.add(cm.FuncConf('n_tiles', 'get_n_tiles', 0,
                                                groupnames=['options'],
                                                name='Number of tiles',
                                                #info = 'Delete a row.',
                                                ))

        # self.add_option(  'maptype',kwargs.get('maptype','satellite'),
        #                     choices = ['satellite',]
        #                     perm='rw',
        #                     name = 'Map type',
        #                     info = 'Type of map to be downloaded.',
        #                     )
        # self.add_option(  'filetype',kwargs.get('filetype','png'),
        #                     choices = ['png',]
        #                     perm='rw',
        #                     name = 'File type',
        #                     info = 'Image file format to be downloaded.',
        #                     )

        # self.add_option(  'mapserver',kwargs.get('mapserver','google'),
        #                     choices = ['google',]
        #                     perm='rw',
        #                     name = 'Map server',
        #                     info = 'Map server from where to download. Some servers require username and password.',
        #                     )

        # self.add_option(  'username',kwargs.get('username',''),
        #                     perm='rw',
        #                     name = 'User',
        #                      info = 'User name of map server (if required).',
        #                     )

        # self.add_option(  'password',kwargs.get('password',''),
        #                     perm='rw',
        #                     name = 'User',
        #                     info = 'User name of map server (if required).',
        #                     )

        self.is_remove_orig = attrsman.add(cm.AttrConf('is_remove_orig', kwargs.get('is_remove_orig', True),
                                                       groupnames=['options'],
                                                       perm='rw',
                                                       name='Remove originals',
                                                       info='Remove original files. Original, untransformed files are not necessary, but can be kept.',
                                                       ))

    def get_n_tiles(self):
        """
        The number of tiles to be downloaded. Please do not download more han 300 tiles, otherwise map server is likely to be offended.
        """
        return self._maps.get_n_tiles(self.width_tile)

    def do(self):
        self.update_params()

        print 'MapsImporter.do'
        # self._maps.download(maptype = self.maptype, mapserver = self.mapserver,
        #            filetype = 'png', rootfilepath = None,
        #            width_tile = self.width_tile,  size_tile = self.size_tile,
        #            is_remove_orig = True):
        self._maps.download(maptype='satellite', mapserver='google',
                            filetype='png', rootfilepath=None,
                            width_tile=self.width_tile,  size_tile=self.size_tile,
                            is_remove_orig=self.is_remove_orig)
        #import_xml(self, rootname, dirname, is_clean_nodes = True)
        # self.run_cml(cml)
        # if self.status == 'success':
        return True

    def update_params(self):
        """
        Make all parameters consistent.
        example: used by import OSM to calculate/update number of tiles
        from process dialog
        """
        pass


class Maps(am.ArrayObjman):
    def __init__(self, landuse, **kwargs):

        self._init_objman(ident='maps',
                          parent=landuse,
                          name='Maps',
                          info='Information on background maps.',
                          **kwargs)
        self._init_attributes()

    def _init_attributes(self):
        # print 'maps._init_attributes'

        # self.add(cm.AttrConf(  'width_tile',500,
        #                    groupnames = ['state'],
        #                    perm='r',
        #                    name = 'Tile width',
        #                    unit = 'm',
        #                    info = 'Tile width in meter of quadratic tile. This is the real wdith of one tile that will be downloaded.',
        #                    ))

        # self.add(cm.AttrConf(  'size_tile',1280,
        #                    groupnames = ['state'],
        #                    perm='r',
        #                    name = 'Tile size',
        #                    info = 'Tile size in pixel. This is the size of one tile that will be downloaded.',
        #                    ))

        if self.has_attrname('width_tile'):
            # no longer attributes
            self.delete('width_tile')
            self.delete('size_tile')
            # put r/w permissione to older version
            # self.get_config('width_tile').set_perm('rw')
            # self.get_config('size_tile').set_perm('rw')

        self.add_col(am.ArrayConf('bboxes',  np.zeros((2, 2), dtype=np.float32),
                                  groupnames=['state'],
                                  perm='r',
                                  name='BBox',
                                  unit='m',
                                  info='Bounding box of map in network coordinate system (lower left coord, upper right coord).',
                                  is_plugin=True,
                                  ))

        self.add_col(am.ArrayConf('filenames',  None,
                                  dtype=np.object,
                                  groupnames=['state'],
                                  perm='rw',
                                  metatype='filepath',
                                  name='File',
                                  info='Image file name.',
                                  ))

    def write_decals(self, fd, indent=4,  rootdir=None):
        print 'write_decals', len(self)
        net = self.parent.get_net()
        if rootdir is None:
            rootdir = os.path.dirname(net.parent.get_rootfilepath())

        #proj = pyproj.Proj(str(net.get_projparams()))
        #offset = net.get_offset()
        #width_tile = self.width_tile.value
        #size_tile = self.size_tile.value
        for filename, bbox in zip(self.filenames.get_value(), self.bboxes.get_value()):
            #x0, y0 = proj(bbox_lonlat[0][0], bbox_lonlat[0][1])
            #x1, y1 = proj(bbox_lonlat[1][0],bbox_lonlat[1][1])
            #bbox = np.array([[x0, y0, 0.0],[x1, y1 ,0.0]],np.float32)

            #bbox_tile = [[x_sw,y_sw ],[x_ne,y_ne]]
            #x0,y0 = bbox_abs[0]+offset
            #x1,y1 = bbox_abs[1]+offset

            #bbox = np.array([[x0, y0, 0.0],[x1, y1 ,0.0]],np.float32)
            # print '  bbox decal',bbox
            xc, yc = 0.5*(bbox[0]+bbox[1])
            zc = 0.0
            width_tile = bbox[1, 0] - bbox[0, 0]
            # print '  xc,yc',xc,yc
            # print '  width_tile',width_tile,bbox
            if filename == os.path.basename(filename):
                # filename does not contain path info
                filepath = filename  # os.path.join(rootdir,filename)
            else:
                # filename contains path info (can happen if interactively inserted)
                filepath = filename

            calxml = '<decal filename="%s" centerX="%.2f" centerY="%.2f" centerZ="0.00" width="%.2f" height="%.2f" altitude="0.00" rotation="0.00" tilt="0.00" roll="0.00" layer="0.00"/>\n' % (
                filepath, xc, yc, width_tile, width_tile)
            fd.write(indent*' '+calxml)

    def clear_all(self):
        """
        Remove all map information.
        """
        self.clear_rows()
        # here we could also delete files ??

    def update_netoffset(self, deltaoffset):
        """
        Called when network offset has changed.
        Children may need to adjust theur coordinates.
        """
        bboxes = self.bboxes.get_value()
        bboxes[:, :, :2] = bboxes[:, :, :2] + deltaoffset

    def get_n_tiles(self, width_tile):
        """
        Estimates number of necessary tiles.
        """
        net = self.parent.get_net()
        bbox_sumo, bbox_lonlat = net.get_boundaries()
        x0 = bbox_sumo[0]  # -0.5*width_tile
        y0 = bbox_sumo[1]  # -0.5*width_tile
        width = bbox_sumo[2]-x0
        height = bbox_sumo[3]-y0
        nx = int(width/width_tile+0.5)
        ny = int(height/width_tile+0.5)
        return nx*ny

    def download(self, maptype='satellite', mapserver='google',
                 filetype='png', rootfilepath=None,
                 width_tile=1000.0,  size_tile=1280,
                 is_remove_orig=True):

        self.clear_rows()
        net = self.parent.get_net()
        if rootfilepath is None:
            rootfilepath = net.parent.get_rootfilepath()

        bbox_sumo, bbox_lonlat = net.get_boundaries()

        offset = net.get_offset()
        # latlon_sw=np.array([bbox_lonlat[1],bbox_lonlat[0]],np.float32)
        # latlon_ne=np.array([bbox_lonlat[3],bbox_lonlat[2]],np.float32)

        x0 = bbox_sumo[0]  # -0.5*width_tile
        y0 = bbox_sumo[1]  # -0.5*width_tile
        width = bbox_sumo[2]-x0
        height = bbox_sumo[3]-y0

        print 'download to', rootfilepath

        #            '+proj=utm +zone=32 +ellps=WGS84 +datum=WGS84 +units=m +no_defs'
        #params_proj="+proj=utm +zone=32 +ellps=WGS84 +datum=WGS84 +units=m +no_defs"
        params_proj = net.get_projparams()
        proj = pyproj.Proj(str(params_proj))
        # print '  params_proj',params_proj,IS_MAPSUPPORT
        # these values are measured manually and are only valid for  size_tile = 256/640
        # width_tile_eff= width_tile#*float(2*size_tile)/238.0#500m 1205.0*width_tile#m 1208
        # height_tile_eff = width_tile#*float(2*size_tile)/238.0# 500m1144.0*width_tile#m 1140

        nx = int(width/width_tile+0.5)+1
        ny = int(height/width_tile+0.5)+1
        print '  offset', offset
        print '  bbox_sumo', bbox_sumo
        print '  width_tile', width_tile, 'm'
        print '  Will download %dx%d= %d maps' % (nx, ny, nx*ny)
        #latlon_tile = np.array([(latlon_ne[0]-latlon_sw[0])/ny, (latlon_ne[1]-latlon_sw[1])/nx])
        #filepaths = []
        #centers = []
        #
        #
        #  0    1
        #  3    2
        #
        #          0        1         2          3

        angle = None
        bbox = None
        ids_map = []
        for ix in xrange(nx):
            for iy in xrange(ny):

                # tile in SUMO network coords. These are the saved coords
                x_tile = x0+ix*width_tile
                y_tile = y0+iy*width_tile
                print '  x_tile,y_tile', x_tile, y_tile
                bb = np.array([[x_tile, y_tile], [x_tile+width_tile, y_tile+width_tile]], np.float32)

                # tile in absolute coordinates. Coords used for download
                x_sw = x_tile-offset[0]
                y_sw = y_tile-offset[1]

                x_ne = x_sw+width_tile
                y_ne = y_sw+width_tile
                bbox_tile = [[x_sw, y_sw], [x_ne, y_ne]]

                filepath = rootfilepath+'_map%04dx%04d.%s' % (ix, iy, filetype)
                # print '  filepath=',filepath

                if angle is None:
                    download_googlemap_bb(filepath, bbox_tile, proj,
                                          size=size_tile,
                                          filetype=filetype, maptype=maptype)
                    angle, bbox = estimate_angle(filepath)

                bbox_tile_lonlat = download_googlemap_bb(filepath, bbox_tile, proj,
                                                         size=size_tile, filetype=filetype,
                                                         maptype=maptype, color="0x0000000f")

                print '  bbox_tile', bbox_tile
                print '  bbox_tile_lonlat', bbox_tile_lonlat

                im = Image.open(filepath).convert("RGB")
                if 1:
                    print '    downloaded image', filepath,  "%dx%d" % im.size, im.mode, im.getbands()
                    # print '    x_sw,y_sw',x_sw,y_sw
                    # print '    x_ne,y_ne',x_ne,y_ne

                # print '  start rotation'
                im_rot = im.rotate(angle)  # gimp 1.62
                # im_rot.show()
                region = im_rot.crop([bbox[0][0], bbox[0][1], bbox[2][0], bbox[2][1]])
                regsize = region.size
                # print ' regsize',regsize
                im_crop = Image.new('RGB', (regsize[0], regsize[1]), (0, 0, 0))
                im_crop.paste(region, (0, 0, regsize[0], regsize[1]))
                im_tile = im_crop.resize((1024, 1024))
                # im_crop.show()
                outfilepath = rootfilepath+'_rot%04dx%04d.%s' % (ix, iy, filetype)

                # print 'save ',outfilepath,"%dx%d" % im_crop.size,im_crop.getbands()
                im_tile.save(outfilepath, filetype.upper())

                # print '  bb_orig=',bb

                #lon0, lat0 = proj(x_tile-offset[0], y_tile-offset[1])
                #lon1, lat1 = proj(x_tile+width_tile-offset[0], y_tile+width_tile-offset[1])

                # print '  bb',bb.shape,bb
                # print '  outfilepath',outfilepath,os.path.basename(outfilepath)
                # print '  saved bbox',np.array([[x_tile-offset[0], y_tile-offset[1]],[x_tile+width_tile-offset[0], y_tile+width_tile-offset[1]]],np.float32)
                # print '  saved bbox',bbox_tile_lonlat
                id_map = self.add_row(filenames=os.path.basename(outfilepath),
                                      # bbox_tile,#bbox_tile_lonlat#np.array([[lon0, lat0],[lon1, lat1]],np.float32),
                                      bboxes=bb,
                                      )
                ids_map.append(id_map)

                if is_remove_orig:
                    # remove original file
                    os.remove(filepath)

        return ids_map


if __name__ == '__main__':
    ############################################################################
    ###
    pass
