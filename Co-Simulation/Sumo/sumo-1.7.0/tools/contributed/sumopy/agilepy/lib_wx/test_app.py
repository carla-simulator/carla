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

# @file    test_app.py
# @author  Joerg Schweizer
# @date

import os
import sys

import wx
from wx.lib.wordwrap import wordwrap


if __name__ == '__main__':
    try:
        APPDIR = os.path.dirname(os.path.abspath(__file__))
    except:
        APPDIR = os.path.dirname(os.path.abspath(sys.argv[0]))
    AGILEDIR = os.path.join(APPDIR, '..')
    print 'APPDIR,AGILEDIR', APPDIR, AGILEDIR
    sys.path.append(AGILEDIR)
    libpaths = [AGILEDIR, os.path.join(AGILEDIR, "lib_base"), os.path.join(AGILEDIR, "lib_wx"), ]
    for libpath in libpaths:
        print '  libpath=', libpath
        lp = os.path.abspath(libpath)
        if not lp in sys.path:
            # print ' append',lp
            sys.path.append(lp)

from mainframe import *

# import corepackages
#from test_glcanvas import *
from ogleditor import *
##
##import wx
##

# try:
##    dirName = os.path.dirname(os.path.abspath(__file__))
# except:
##    dirName = os.path.dirname(os.path.abspath(sys.argv[0]))
##
# sys.path.append(os.path.split(dirName)[0])


IMAGEDIR = os.path.join(os.path.dirname(__file__), "images")
ICONPATH = os.path.join(IMAGEDIR, 'icon_color_small.png')  # None


class MyApp(wx.App):
    def __init__(self, redirect=False, filename=None):
        wx.App.__init__(self, redirect, filename)
        #self.frame = wx.Frame(None, wx.ID_ANY, title='My Title')
        self.mainframe = AgileMainframe(title='MyApp', size_toolbaricons=(32, 32))
        if ICONPATH is not None:
            icon = wx.Icon(ICONPATH, wx.BITMAP_TYPE_PNG, 16, 16)
            self.mainframe.SetIcon(icon)

        self.gleditor = self.mainframe.add_view("OGleditor", OGleditor)

        self.mainframe.Show()
        self.on_test()
        self.make_menu()
        self.make_toolbar()
        #canvas = gleditor.get_canvas()
        # canvas.add_element(lines)
        # canvas.add_element(triangles)
        # canvas.add_element(rectangles)

    def make_toolbar(self):
        tsize = self.mainframe.get_size_toolbaricons()
        new_bmp = wx.ArtProvider.GetBitmap(wx.ART_NEW, wx.ART_TOOLBAR, tsize)
        open_bmp = wx.ArtProvider.GetBitmap(wx.ART_FILE_OPEN, wx.ART_TOOLBAR, tsize)
        save_bmp = wx.ArtProvider.GetBitmap(wx.ART_FILE_SAVE, wx.ART_TOOLBAR, tsize)
        #cut_bmp = wx.ArtProvider.GetBitmap(wx.ART_CUT, wx.ART_TOOLBAR, tsize)
        #copy_bmp = wx.ArtProvider.GetBitmap(wx.ART_COPY, wx.ART_TOOLBAR, tsize)
        #paste_bmp= wx.ArtProvider.GetBitmap(wx.ART_PASTE, wx.ART_TOOLBAR, tsize)

        self.mainframe.add_tool('new', self.on_open, new_bmp, 'create new doc')
        self.mainframe.add_tool('open', self.on_open, open_bmp, 'Open doc')
        self.mainframe.add_tool('save', self.on_save, save_bmp, 'Save doc')
        # self.toolbar.AddSeparator()
        # self.add_tool('cut',self.on_open,cut_bmp,'Cut')
        # self.add_tool('copy',self.on_open,copy_bmp,'Copy')
        # self.add_tool('paste',self.on_open,paste_bmp,'Paste')

    def make_menu(self):
        self.mainframe.menubar.append_menu('file')
        self.mainframe.menubar.append_menu('file/doc')

        self.mainframe.menubar.append_item('file/doc/open', self.on_open,
                                           shortkey='Ctrl+o', info='open it out')

        self.mainframe.menubar.append_item('file/doc/save', self.on_save,
                                           shortkey='Ctrl+s', info='save it out')

    def on_save(self, event):
        print 'save it!!'

    def on_open(self, event):
        """Open a document"""
        #wildcards = CreateWildCards() + "All files (*.*)|*.*"
        print 'open it!!'

    def on_test(self, event=None):
        print '\non_test'
        vertices = np.array([
            [[0.0, 0.0, 0.0], [0.2, 0.0, 0.0]],  # 0 green
            [[0.0, 0.0, 0.0], [0.0, 0.9, 0.0]],  # 1 red
        ])

        colors = np.array([
            [0.0, 0.9, 0.0, 0.9],    # 0
            [0.9, 0.0, 0.0, 0.9],    # 1
        ])

        colors2 = np.array([
            [0.5, 0.9, 0.5, 0.5],    # 0
            [0.9, 0.5, 0.9, 0.5],    # 1
        ])
        colors2o = np.array([
            [0.8, 0.9, 0.8, 0.9],    # 0
            [0.9, 0.8, 0.9, 0.9],    # 1
        ])

        drawing = OGLdrawing()
# -------------------------------------------------------------------------------

        if 0:
            lines = Lines('lines', drawing)
            lines.add_drawobjs(vertices, colors)
            drawing.add_drawobj(lines)
# -------------------------------------------------------------------------------
        if 0:
            fancylines = Fancylines('fancylines', drawing)
            vertices_fancy = np.array([
                [[0.0, -1.0, 0.0], [2, -1.0, 0.0]],  # 0 green
                [[0.0, -1.0, 0.0], [0.0, -5.0, 0.0]],  # 1 red
            ])

            widths = [0.5,
                      0.3,
                      ]
            # print '  vertices_fancy\n',vertices_fancy
            # FLATHEAD = 0
            #BEVELHEAD = 1
            #TRIANGLEHEAD = 2
            #ARROWHEAD = 3
            fancylines.add_drawobjs(vertices_fancy,
                                    widths,  # width
                                    colors,
                                    beginstyles=[TRIANGLEHEAD, TRIANGLEHEAD],
                                    endstyles=[ARROWHEAD, ARROWHEAD])
            drawing.add_drawobj(fancylines)
# -------------------------------------------------------------------------------
        if 0:
            polylines = Polylines('polylines', drawing, joinstyle=BEVELHEAD)
            colors_poly = np.array([
                [0.0, 0.8, 0.5, 0.9],    # 0
                [0.8, 0.0, 0.5, 0.9],    # 1
            ])

            vertices_poly = np.array([
                [[0.0, 2.0, 0.0], [5.0, 2.0, 0.0], [5.0, 7.0, 0.0], [0.0, 7.0, 0.0]],  # 0 green
                [[0.0, -2.0, 0.0], [-2.0, -2.0, 0.0]],  # 1 red
            ], np.object)

            widths = [0.5,
                      0.3,
                      ]
            # print '  vertices_poly\n',vertices_poly
            polylines.add_drawobjs(vertices_poly,
                                   widths,  # width
                                   colors_poly,
                                   beginstyles=[ARROWHEAD, ARROWHEAD],
                                   endstyles=[ARROWHEAD, ARROWHEAD])
            drawing.add_drawobj(polylines)

# -------------------------------------------------------------------------------
        if 1:
            polygons = Polygons('polygons', drawing, linewidth=5)
            colors_poly = np.array([
                [0.0, 0.9, 0.9, 0.9],    # 0
                [0.8, 0.2, 0.2, 0.9],    # 1
            ])

            vertices_poly = np.array([
                [[0.0, 2.0, 0.0], [5.0, 2.0, 0.0], [5.0, 7.0, 0.0], [0.0, 7.0, 0.0]],  # 0 green
                [[0.0, -2.0, 0.0], [-2.0, -2.0, 0.0], [-2.0, 0.0, 0.0]],  # 1 red
            ], np.object)

            print '  vertices_polygon\n', vertices_poly
            polygons.add_drawobjs(vertices_poly,
                                  colors_poly)
            drawing.add_drawobj(polygons)

        canvas = self.gleditor.get_canvas()
        canvas.set_drawing(drawing)

        #lines.add_drawobj([[0.0,0.0,0.0],[-0.2,-0.8,0.0]], [0.0,0.9,0.9,0.9])
        #circles.add_drawobj([1.5,0.0,0.0],0.6,colors2o[0], colors2[0])

        # canvas.zoom_tofit()
        wx.CallAfter(canvas.zoom_tofit)


if __name__ == '__main__':
    # if len(sys.argv)==3:
    #    ident = sys.argv[1]
    #    dirpath = sys.argv[2]
    # else:
    #    ident =  None
    #    dirpath = None
    myapp = MyApp(0)

    myapp.MainLoop()
