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

# @file    ogleditor.py
# @author  Joerg Schweizer
# @date

#!/usr/bin/env python;
"""OpenGL editor"""
if __name__ == '__main__':
    __version__ = "0.1a"
    __licence__ = """licensed under the GPL."""
    __copyright__ = "(c) 2012-2016 University of Bologna - DICAM"
    __author__ = "Joerg Schweizer"

    __usage__ = """USAGE:
    from command line:
    python ogleditor.py
    python ogleditor.py <scenariobasename> <scenariodir>

    use for debugging
    python ogleditor.py --debug > debug.txt 2>&1
    """
    print __copyright__

###############################################################################
# IMPORTS

#import wxversion
# wxversion.select("2.8")

import wx
from collections import OrderedDict
from wx import glcanvas
from wx.lib.buttons import GenBitmapTextButton, GenBitmapButton
try:
    from OpenGL.GL import *
    from OpenGL.GLU import *  # project , unproject , tess
    from OpenGL.GLUT import *
    from OpenGL.raw.GL.ARB.vertex_array_object import glGenVertexArrays, \
        glBindVertexArray

    from OpenGL.arrays import vbo
    import numpy as np

except ImportError:
    raise ImportError, "Required dependencies numpy or OpenGL not present"

import sys
import os
import types
if __name__ == '__main__':
    try:
        FILEDIR = os.path.dirname(os.path.abspath(__file__))
    except:
        FILEDIR = os.path.dirname(os.path.abspath(sys.argv[0]))
    sys.path.append(os.path.join(FILEDIR, "..", ".."))
    #IMAGEDIR = os.path.join(APPDIR,"lib_base","images")

IMAGEDIR = os.path.join(os.path.dirname(__file__), "images")

import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am

from agilepy.lib_base.geometry import *

# wx gui stuff
from wxmisc import *
from toolbox import *


FLATHEAD = 0
BEVELHEAD = 1
TRIANGLEHEAD = 2
ARROWHEAD = 3
LEFTTRIANGLEHEAD = 4
RIGHTTRIANGLEHEAD = 5

LINEHEADS = {  # 'flat':0,# not a style
    'bevel': BEVELHEAD,
    'triangle': TRIANGLEHEAD,
    'arrow': ARROWHEAD,
    'lefttriangle': LEFTTRIANGLEHEAD,
    'righttriangle': RIGHTTRIANGLEHEAD,
}

# to be deleted
#import  test_glcanvas as testogl
# class TestVbo(Vbo):
# def draw(self, resolution):
##
##
##
# print 'draw',self.get_ident(),self
# glEnableClientState(GL_VERTEX_ARRAY)
# glEnableClientState(GL_COLOR_ARRAY)
##
# self._colorvbo.bind()
##        glColorPointer(4, GL_FLOAT, 0, None)
##
# self._vertexvbo.bind()
# self._indexvbo.bind()
##        glVertexPointer(3, GL_FLOAT, 0, None)
##
# print '  self._n_vpo *(self._n_vpo-1)*self._n_drawobjs',self._n_vpo *(self._n_vpo-1)*self._n_drawobjs
# print '  len(self._vertexvbo)',len(self._vertexvbo)
# print '  len(self._colorvbo)',len(self._colorvbo)
# n_vpo = len(self._vertexvbo)/self._n_vpe/self._n_drawobjs+1
# glDrawElements(self._glelement, self._n_vpo *(self._n_vpo-1)*self._n_drawobjs, GL_UNSIGNED_INT, None)
##        glDrawElements(self._glelement, len(self._vertexvbo), GL_UNSIGNED_INT, None)
##
# glDisableClientState(GL_VERTEX_ARRAY)
# glDisableClientState(GL_COLOR_ARRAY)
##
# self._vertexvbo.unbind()
# self._indexvbo.unbind()
# self._colorvbo.unbind()


def normalize(v):
    norm = np.linalg.norm(v)
    if norm == 0:
        return v
    return v/norm


class Vbo:
    def __init__(self, ident, glelement, n_vpe, objtype=''):
        self._ident = ident
        self._glelement = glelement
        self._n_vpe = n_vpe
        self._objtype = objtype
        self.reset()

    def reset(self):
        self._vertexvbo = None
        self._indexvbo = None
        self._colorvbo = None
        self._inds = None

    def get_objtype(self):
        return self._objtype

    def is_fill(self):
        return self._objtype == 'fill'

    def get_ident(self):
        return self._ident

    def get_vpe(self):
        return self._n_vpe

    def update_vertices(self, vertices, n=None, inds=None):
        """

        n = number of draw objects
        """
        self._inds = inds
        # print 'update_vertices',self.get_ident(),n
        self._n_drawobjs = n
        #
        #vertices = np.zeros((self._n_elem_fill, self._n_vpe_fill * 3),dtype=np.float32)
        # len(vertices) = n_elem = (n_vpo-1)*n
        #n_elem = len(vertices)/n_drawobjs+self._noncyclic

        # n_vpo = vertex per draw object
        # n_elem = (n_vpo-1)*n_drawobjs
        # n_vpo = vertex per draw object
        #self._n_vpo = n_elem/n_drawobjs+1
        #n_vert = len(self._vertexvbo_fill)/self._n_vpe_fill/len(self)+1
        #n_elem_fill = (n_vert-1)*n
        # len(_vertexvbo_fill) =  self._n_vpe * self._n_elem_fill =  self._n_vpe * (n_vert-1)*n
        # print '  len(vertices),n_vpo,n_elem',len(vertices),self._n_vpo,n_elem

        #glDrawElements(GL_TRIANGLES, self._n_vert*self._n_elem_fill, GL_UNSIGNED_INT, None)
        #  self._n_vert*self._n_elem_fill = n_vert * (n_vert-1)*n
        # repeat for each of the n objects:  self._n_vpe_fill*(n_vert-1)
        # print '  vertices=\n',vertices
        # print '  vertices.reshape((-1,3))=\n',vertices.reshape((-1,3))
        self._vertexvbo = vbo.VBO(vertices.reshape((-1, 3)))
        self._indexvbo = vbo.VBO(np.arange(len(self._vertexvbo), dtype=np.int32), target=GL_ELEMENT_ARRAY_BUFFER)
        #self._indexvbo = vbo.VBO(np.arange(len(vertices.reshape((-1,3)) ), dtype=np.int32), target=GL_ELEMENT_ARRAY_BUFFER)

    def destroy(self):
        del self._vertexvbo
        del self._indexvbo
        del self._colorvbo
        self.reset()

    def update_colors(self, colors):

        if self._vertexvbo is None:
            return
        # print 'update_colors',self._n_drawobjs,len(colors)
        if len(colors) == 0:
            return
        if self._n_drawobjs is None:
            n_repreat = len(self._vertexvbo)/len(self._inds)  # self._n_drawobjs
            self._colorvbo = vbo.VBO(np.repeat(colors[self._inds], n_repreat, 0))
        else:
            n_repreat = len(self._vertexvbo)/self._n_drawobjs
            self._colorvbo = vbo.VBO(np.repeat(colors, n_repreat, 0))

    def draw(self, resolution):

        # glEnableClientState(GL_VERTEX_ARRAY)
        # glEnableClientState(GL_COLOR_ARRAY)
        if self._vertexvbo is None:
            return
        # print 'Vbo.draw',self.get_ident(),self._n_drawobjs,len(self._vertexvbo)
        # if self._n_drawobjs in (0,None): return
        if self._n_drawobjs == 0:
            return
        self._colorvbo.bind()
        glColorPointer(4, GL_FLOAT, 0, None)

        self._vertexvbo.bind()
        self._indexvbo.bind()
        glVertexPointer(3, GL_FLOAT, 0, None)

        # print '  self._n_vpo *(self._n_vpo-1)*self._n_drawobjs',self._n_vpo *(self._n_vpo-1)*self._n_drawobjs
        # print '  len(self._vertexvbo)',len(self._vertexvbo)
        # print '  len(self._indexvbo)',len(self._indexvbo)
        # print '  len(self._colorvbo)',len(self._colorvbo)
        #n_vpo = len(self._vertexvbo)/self._n_vpe/self._n_drawobjs+1
        #glDrawElements(self._glelement, self._n_vpo *(self._n_vpo-1)*self._n_drawobjs, GL_UNSIGNED_INT, None)
        glDrawElements(self._glelement, len(self._vertexvbo), GL_UNSIGNED_INT, None)

        # glDisableClientState(GL_VERTEX_ARRAY)
        # glDisableClientState(GL_COLOR_ARRAY)

        self._vertexvbo.unbind()
        self._indexvbo.unbind()
        self._colorvbo.unbind()


class SelectToolMixin(BaseTool):
    """
    Mixin for Selection tools for OGL canvas.
    """

    def _init_select(self, is_show_selected=False, detectwidth=0.1, detectpix=5):
        """
        Define selection specific attributes.
        """
        self.detectpix = detectpix
        self.detectwidth = detectwidth  # in meter
        self.is_show_selected = is_show_selected

        if is_show_selected:
            groupnames = ['options']
        else:
            groupnames = ['_private_']
        self.add_col(am.TabIdsArrayConf('drawobjects',
                                        groupnames=groupnames,
                                        name='Object',
                                        info='Selected Draw objects with id',
                                        ))

    def activate_select(self, canvas=None):
        """
        This call by metacanvas signals that the tool has been
        activated and can now interact with metacanvas.
        """
        BaseTool.activate(self, canvas)
        self._idcounter = 0

    def deactivate_select(self):
        """
        This call by metacanvas signals that the tool has been
        deactivated and can now interact with metacanvas.
        """
        drawing = self._canvas.get_drawing()

        if len(self) > 0:
            # ungighlight selected objects
            is_draw = False
            for drawobj, _id in self.drawobjects.value:  # drawing.get_drawobjs():
                is_draw |= drawobj.unhighlight([_id], is_update=True)
            if is_draw:
                self._canvas.draw()

            # remove selected objects
            self.del_rows(self.get_ids())

        BaseTool.deactivate(self)

    def iterate_selection(self):
        self.unhighlight_current()
        self._idcounter += 1
        if self._idcounter == len(self):
            self._idcounter = 0
        self.highlight_current()

    def on_left_down_select(self, event):
        # print 'on_left_down_select'
        is_draw = False

        if len(self) > 0:
            if event.ShiftDown():

                self.iterate_selection()
                self.on_change_selection(event)
                is_draw = True
            else:
                # print '  on_execute_selection 1'
                is_draw |= self.on_execute_selection(event)
                is_draw |= self.unselect_all()
                if self.is_show_selected:
                    self.parent.refresh_optionspanel(self)

        else:
            is_draw |= self.pick_all(event)
            self.highlight_current()

            if not event.ShiftDown():
                if self.is_preselected():
                    self.coord_last = self._canvas.unproject(event.GetPosition())
                    # print '  on_execute_selection 2'
                    is_draw |= self.on_execute_selection(event)
                    # attention: on_execute_selection must take care of selected
                    # objects in list with self.unselect_all()

            else:
                self.coord_last = self._canvas.unproject(event.GetPosition())

            if self.is_show_selected:
                self.parent.refresh_optionspanel(self)

        return is_draw

    def is_preselected(self):
        """
        Check if there are preselected drawobjects in the list.
        """
        return len(self) > 0

    def on_execute_selection(self, event):
        """
        Definively execute operation on currently selected drawobjects.
        """
        self.set_objbrowser()

        return False

    def on_change_selection(self, event):
        """
        Called after selection has been changed with SHIFT-click
        Do operation on currently selected drawobjects.
        """
        self.set_objbrowser()
        return False

    def set_objbrowser(self):
        pass

    def aboard_select(self, event):
        # print 'aboard',self.ident
        is_draw = self.unselect_all()
        return is_draw

    def get_current_selection(self):
        if len(self) > 0:
            (drawobj, _id) = self.drawobjects[self.get_ids()[self._idcounter]]
            return drawobj, _id
        else:
            return None, -1

    def is_tool_allowed_on_selection(self):
        drawobj, _id = self.get_current_selection()
        if drawobj is not None:
            return drawobj.is_tool_allowed(self, _id)
        else:
            False

    def pick_all(self, event):
        """
        Pick all objets with id, that are near the pointer
        coordinates.
        """

        p = self._canvas.unproject_event(event)

        if self.detectpix > 0:
            # detect pixel sensitivity is given
            # calculate detectwidth based on current resolution
            self.detectwidth = self._canvas.get_resolution()*self.detectpix

        # print 'pick_all',self.detectwidth,self.detectpix,self._canvas.get_resolution()

        self._idcounter = 0
        is_draw = False
        drawing = self._canvas.get_drawing()
        # print '  ',drawing.get_drawobjs()
        for drawobj in drawing.get_drawobjs():
            if drawobj.is_visible():
                ids_pick = drawobj.pick(p, detectwidth=self.detectwidth)
                # print '  pick',drawobj.get_ident(),ids_pick
                if len(ids_pick) > 0:
                    for id_pick in ids_pick:
                        is_draw |= self.add_selection(drawobj, id_pick, event)

        return is_draw

    def unhighlight_current(self):
        # print 'unhighlight_current',len(self),self._idcounter
        if len(self) > 0:
            (drawobj, _id) = self.drawobjects[self.get_ids()[self._idcounter]]
            drawobj.unhighlight([_id])

    def highlight_current(self):
        # print 'highlight_current',len(self),self._idcounter
        if len(self) > 0:
            (drawobj, _id) = self.drawobjects[self.get_ids()[self._idcounter]]
            drawobj.highlight([_id])

    def unselect_all(self):
        # print 'unselect_all',len(self)
        self._idcounter = 0
        if len(self) > 0:
            is_draw = True
            for drawobj, _id in self.drawobjects.value:
                if _id in drawobj:
                    drawobj.unhighlight([_id])

            self.del_rows(self.get_ids())
        else:
            is_draw = False

        if is_draw:
            self.parent.refresh_optionspanel(self)
        # print '  len should be 0:',len(self),is_draw
        return is_draw

    def add_selection(self, drawobj, id_pick, event=None):
        is_draw = False
        drawobjid = self.drawobjects.convert_type((drawobj, id_pick))
        ids_match = self.select_ids(self.drawobjects.value == drawobjid)
        if len(ids_match) == 0:
            self.add_rows(1, drawobjects=[(drawobj, id_pick)])
            is_draw = True
        return is_draw


class AddLineTool(BaseTool):
    """
    Mixin for Selection tools for OGL canvas.
    """

    def __init__(self, parent, mainframe=None):
        self.init_common('add_line', parent, 'Add line tool',
                         info='Click on canvas to add a line. Double click to finish, right click to aboard.',
                         is_textbutton=False,
                         )
        self.init_options()

    def init_options(self):
        self.add(cm.AttrConf('width',  0.2,
                             groupnames=['options'],
                             perm='rw',
                             name='Width',
                             is_save=True,
                             info='Line width',
                             ))

        self.add(cm.AttrConf('color',  np.array([1.0, 0.0, 0.0, 0.8], dtype=np.float32),
                             groupnames=['options'],
                             perm='rw',
                             metatype='color',
                             name='Fill color',
                             is_save=True,
                             info='Line color with format [r,g,b,a]',
                             ))

        # self.add(cm.AttrConf( 'color_fill',  np.array([1.0,0.0,0.0,0.8], dtype=np.float32),
        #                                groupnames = ['options'],
        #                                perm='rw',
        #                                metatype = 'color',
        #                                name = 'Fill color',
        #                                is_save = True,
        #                                info = 'Object fill color with format [r,g,b,a]',
        #                                ))

    def set_button_info(self, bsize=(32, 32)):
        # print 'set_button_info select tool'
        self._bitmap = wx.Bitmap(os.path.join(IMAGEDIR, 'line_24px.png'), wx.BITMAP_TYPE_PNG)
        self._bitmap_sel = wx.Bitmap(os.path.join(IMAGEDIR, 'line_24px.png'), wx.BITMAP_TYPE_PNG)

    def set_cursor(self):
        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        if self._canvas is not None:
            self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_HAND))

    def activate(self, canvas=None):
        """
        This call by metacanvas??TooldsPallet signals that the tool has been
        activated and can now interact with metacanvas.
        """
        # print 'activate',self.ident
        self._is_active = True
        self._canvas = canvas
        # self._canvas.del_handles()
        canvas.activate_tool(self)
        self.set_cursor()
        self.is_animated = False

    def deactivate(self):
        """
        This call by metacanvas??? ToolePallet signals that the tool has been
        deactivated and can now interact with metacanvas.
        """

        is_draw = self.aboard()
        self._canvas.deactivate_tool()
        self._canvas = None
        self._is_active = False
        return is_draw

    def on_left_down(self, event):
        if not self.is_animated:
            return self.begin_animation(event)

    def on_left_dclick(self, event):
        if self.is_animated:
            return self.end_animation(event)

    def on_motion(self, event):
        if self.is_animated:
            return self.animate(event)
        else:
            return False

    def begin_animation(self, event):
        # print 'AddLineTool'
        #self.drawobj_anim, _id, self.ind_vert =  self.get_current_vertexselection()
        self.drawobj_anim = self._canvas.get_drawing().get_drawobj_by_ident('fancylines')
        self.coord_last = self._canvas.unproject(event.GetPosition())
        vert = np.concatenate((self.coord_last, self.coord_last), 1).reshape((2, 3))
        # print '  vert ',vert#,self.width.get_value(),self.color.get_value(),
        _id = self.drawobj_anim.add_drawobj(vert,
                                            self.width.get_value(),  # width
                                            self.color.get_value(),
                                            beginstyle=TRIANGLEHEAD,
                                            endstyle=TRIANGLEHEAD,
                                            )
        self.ind_vert = 1
        self.drawobj_anim.begin_animation(_id)

        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_CROSS))
        self.is_animated = True

        self.parent.refresh_optionspanel(self)
        return True  # True for redrawing

    def end_animation(self, event):
        self.drawobj_anim.end_animation(is_del_last_vert=False)
        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_HAND))

        del self.drawobj_anim
        #del self.id_anim
        self.is_animated = False
        self.parent.refresh_optionspanel(self)
        return True  # True for redrawing

    def animate(self, event):
        # print 'animate'
        is_draw = False
        coords = self._canvas.unproject(event.GetPosition())
        vertex_delta = coords-self.coord_last
        if np.any(np.abs(vertex_delta) > 0):
            is_draw = self.drawobj_anim.stretch_animation(coords, vertex_delta, self.ind_vert)
            if is_draw:
                self.coord_last = coords
        return is_draw

    def aboard(self, event=None):
        # print 'aboard',self.ident

        is_draw = False
        if self.is_animated:
            #id_admin = self.drawobj_anim.get_id_admin()
            self.drawobj_anim.del_animation(is_del_main=True)
            del self.drawobj_anim
            self.is_animated = False
            is_draw = True
            self.parent.refresh_optionspanel(self)
        return is_draw  # True for redrawing

    # def set_objbrowser(self):
    #    pass

    def get_optionspanel(self, parent, size=(200, -1)):
        """
        Return tool option widgets on given parent
        """
        if self.is_animated:
            # show option of currently selected drawobj
            self._optionspanel = ObjPanel(parent, obj=self.drawobj_anim,
                                          id=self.drawobj_anim.get_id_anim(),
                                          attrconfigs=None,
                                          #tables = None,
                                          # table = None, id=None, ids=None,
                                          groupnames=['options'],
                                          func_change_obj=None,
                                          show_groupnames=False, show_title=True, is_modal=False,
                                          mainframe=self.parent.get_mainframe(),
                                          pos=wx.DefaultPosition, size=size, style=wx.MAXIMIZE_BOX | wx.RESIZE_BORDER,
                                          func_apply=self.on_apply_option,
                                          immediate_apply=False, panelstyle='default',  # 'instrumental'
                                          standartbuttons=['apply', 'restore'])
        else:

            self._optionspanel = ObjPanel(parent, obj=self,
                                          id=None,
                                          attrconfigs=None,
                                          #tables = None,
                                          # table = None, id=None, ids=None,
                                          groupnames=['options'],
                                          func_change_obj=None,
                                          show_groupnames=False, show_title=True, is_modal=False,
                                          mainframe=self.parent.get_mainframe(),
                                          pos=wx.DefaultPosition, size=size, style=wx.MAXIMIZE_BOX | wx.RESIZE_BORDER,
                                          func_apply=self.on_apply_option,
                                          immediate_apply=False, panelstyle='default',  # 'instrumental'
                                          standartbuttons=['apply', 'restore'])

        return self._optionspanel

    def on_apply_option(self, drawobj, id, ids):
        # print '\n\n\non_apply_option drawobj, id, ids',drawobj, id, ids
        if self.is_animated:
            if drawobj == self.drawobj_anim:
                drawobj._update_colorvbo()
                drawobj._update_vertexvbo()

            self._canvas.draw()


class AddCircleTool(AddLineTool):
    """
    Mixin for Selection tools for OGL canvas.
    """

    def __init__(self, parent, mainframe=None):
        self.init_common('add_circle', parent, 'Add circle tool',
                         info='Click on canvas to add a circle. Double click to finish, right click to aboard.',
                         is_textbutton=False,
                         )
        self.init_options()

    def init_options(self):

        self.add(cm.AttrConf('color',  np.array([1.0, 0.0, 0.0, 0.8], dtype=np.float32),
                             groupnames=['options'],
                             perm='rw',
                             metatype='color',
                             name='Color',
                             is_save=True,
                             info='Line color with format [r,g,b,a]',
                             ))

        self.add(cm.AttrConf('color_fill',  np.array([1.0, 0.0, 0.0, 0.8], dtype=np.float32),
                             groupnames=['options'],
                             perm='rw',
                             metatype='color',
                             name='Fill color',
                             is_save=True,
                             info='Circle fill color with format [r,g,b,a]',
                             ))

    def set_button_info(self, bsize=(32, 32)):
        # print 'set_button_info select tool'
        self._bitmap = wx.Bitmap(os.path.join(IMAGEDIR, 'circle_empty_24px.png'), wx.BITMAP_TYPE_PNG)
        self._bitmap_sel = wx.Bitmap(os.path.join(IMAGEDIR, 'circle_empty_24px.png'), wx.BITMAP_TYPE_PNG)

    def begin_animation(self, event):
        # print 'AddLineTool'
        #self.drawobj_anim, _id, self.ind_vert =  self.get_current_vertexselection()
        self.drawobj_anim = self._canvas.get_drawing().get_drawobj_by_ident('circles')
        self.coord_last = self._canvas.unproject(event.GetPosition())
        #vert = np.concatenate((self.coord_last,self.coord_last),1).reshape((2,3))
        # print '  vert ',vert#,self.width.get_value(),self.color.get_value(),

        _id = self.drawobj_anim.add_drawobj(1.0*self.coord_last,
                                            0.0,
                                            self.color.get_value(),
                                            self.color_fill.get_value(),
                                            )

        self.drawobj_anim.begin_animation(_id)

        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_CROSS))
        self.is_animated = True

        self.parent.refresh_optionspanel(self)
        return True  # True for redrawing

    def end_animation(self, event):
        self.drawobj_anim.end_animation(is_del_last_vert=False)
        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_HAND))

        del self.drawobj_anim
        #del self.id_anim
        self.is_animated = False
        self.parent.refresh_optionspanel(self)
        return True  # True for redrawing

    def animate(self, event):
        # print 'animate'
        is_draw = False
        coords = self._canvas.unproject(event.GetPosition())
        vertex_delta = coords-self.coord_last
        if np.any(np.abs(vertex_delta) > 0):
            is_draw = self.drawobj_anim.stretch_animation(coords, vertex_delta, 1)
            if is_draw:
                self.coord_last = coords
        return is_draw


class AddPolylineTool(AddLineTool):
    """
    Mixin for Selection tools for OGL canvas.
    """

    def __init__(self, parent, mainframe=None):
        self.init_common('add_polyline', parent, 'Add polyline tool',
                         info='Click on canvas to add a poly line. Add a vertex with a single click, double click to finish, right click to aboard.',
                         is_textbutton=False,
                         )
        self.init_options()

    def set_button_info(self, bsize=(32, 32)):
        # print 'set_button_info select tool'
        self._bitmap = wx.Bitmap(os.path.join(IMAGEDIR, 'polyline_24px.png'), wx.BITMAP_TYPE_PNG)
        self._bitmap_sel = wx.Bitmap(os.path.join(IMAGEDIR, 'polyline_24px.png'), wx.BITMAP_TYPE_PNG)

    def on_left_down(self, event):
        if not self.is_animated:
            return self.begin_animation(event)
        else:
            # add vertex
            return self.append_vert_to_anim(event)

    def begin_animation(self, event):
        # print 'AddLineTool.begin_animation'
        #self.drawobj_anim, _id, self.ind_vert =  self.get_current_vertexselection()
        self.drawobj_anim = self._canvas.get_drawing().get_drawobj_by_ident('polylines')
        self.coord_last = self._canvas.unproject(event.GetPosition())
        #vertices = [list(self.coord_last),list(self.coord_last) ]
        vertices = [1.0*self.coord_last, 1.0*self.coord_last]  # attention, we need copies here!!
        # print '  vertices ',vertices#,self.width.get_value(),self.color.get_value(),

        _id = self.drawobj_anim.add_drawobj(vertices,
                                            self.width.get_value(),  # width
                                            self.color.get_value(),
                                            beginstyle=TRIANGLEHEAD,
                                            endstyle=TRIANGLEHEAD,
                                            )
        self.ind_vert = 1
        self.drawobj_anim.begin_animation(_id)

        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_CROSS))
        self.is_animated = True

        self.parent.refresh_optionspanel(self)
        return True  # True for redrawing

    def end_animation(self, event):
        self.drawobj_anim.end_animation(is_del_last_vert=True)
        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_HAND))

        del self.drawobj_anim
        #del self.id_anim
        self.is_animated = False
        self.parent.refresh_optionspanel(self)
        return True  # True for redrawing

    def append_vert_to_anim(self, event):
        self.coord_last = self._canvas.unproject(event.GetPosition())

        self.ind_vert = self.drawobj_anim.append_vert_to_animation(self.coord_last)

        return True


class AddPolygonTool(AddPolylineTool):
    """
    Mixin for Selection tools for OGL canvas.
    """

    def __init__(self, parent, mainframe=None):
        self.init_common('add_polygon', parent, 'Add polygon tool',
                         info='Click on canvas to add a polygon. Add a vertex with a single click, double click to finish, right click to aboard.',
                         is_textbutton=False,
                         )
        self.init_options()

    def init_options(self):
        self.add(cm.AttrConf('color',  np.array([1.0, 0.0, 0.0, 0.8], dtype=np.float32),
                             groupnames=['options'],
                             perm='rw',
                             metatype='color',
                             name='Fill color',
                             is_save=True,
                             info='Line color with format [r,g,b,a]',
                             ))

        # self.add(cm.AttrConf( 'color_fill',  np.array([1.0,0.0,0.0,0.8], dtype=np.float32),
        #                                groupnames = ['options'],
        #                                perm='rw',
        #                                metatype = 'color',
        #                                name = 'Fill color',
        #                                is_save = True,
        #                                info = 'Object fill color with format [r,g,b,a]',
        #                                ))

    def set_button_info(self, bsize=(32, 32)):
        # print 'set_button_info select tool'
        self._bitmap = wx.Bitmap(os.path.join(IMAGEDIR, 'polygon2_24px.png'), wx.BITMAP_TYPE_PNG)
        self._bitmap_sel = wx.Bitmap(os.path.join(IMAGEDIR, 'polygon2_24px.png'), wx.BITMAP_TYPE_PNG)

    def begin_animation(self, event):
        # print 'AddLineTool.begin_animation'
        #self.drawobj_anim, _id, self.ind_vert =  self.get_current_vertexselection()
        self.drawobj_anim = self._canvas.get_drawing().get_drawobj_by_ident('polygons')
        self.coord_last = self._canvas.unproject(event.GetPosition())
        #vertices = [list(self.coord_last),list(self.coord_last) ]
        vertices = [1.0*self.coord_last, 1.0*self.coord_last, ]  # attention, we need copies here!!
        # print '  vertices ',vertices#,self.width.get_value(),self.color.get_value(),

        _id = self.drawobj_anim.add_drawobj(vertices,
                                            color=self.color.get_value(),
                                            )
        self.ind_vert = 1
        self.drawobj_anim.begin_animation(_id)

        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_CROSS))
        self.is_animated = True

        self.parent.refresh_optionspanel(self)
        return True  # True for redrawing


class SelectTool(SelectToolMixin):
    """
    Selection tool for OGL canvas.
    """

    def __init__(self, parent, detectwidth=0.1,  detectpix=5, mainframe=None):
        """
        To be overridden by specific tool.
        """
        self.init_common('select', parent, 'Selection tool',
                         info='Select objects in cancvas',
                         is_textbutton=False,
                         )
        self._init_select(is_show_selected=True, detectwidth=detectwidth, detectpix=detectpix)

    def set_button_info(self, bsize=(32, 32)):
        # print 'set_button_info select tool'  Select_32px

        # wx.ART_INFORMATION
        self._bitmap = wx.ArtProvider.GetBitmap(wx.ART_INFORMATION, wx.ART_TOOLBAR)
        self._bitmap_sel = wx.ArtProvider.GetBitmap(wx.ART_INFORMATION, wx.ART_TOOLBAR)

        #self._bitmap = wx.Bitmap(os.path.join(IMAGEDIR,'Cursor-Click-icon_24px.png'),wx.BITMAP_TYPE_PNG)
        # self._bitmap_sel=wx.Bitmap(os.path.join(IMAGEDIR,'Cursor-Click-icon_24px.png'),wx.BITMAP_TYPE_PNG)

        #self._bitmap = wx.Bitmap(os.path.join(IMAGEDIR,'selectIcon.bmp'),wx.BITMAP_TYPE_BMP)
        # self._bitmap_sel=wx.Bitmap(os.path.join(IMAGEDIR,'selectIconSel.bmp'),wx.BITMAP_TYPE_BMP)

    def set_cursor(self):
        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        if self._canvas is not None:
            self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_QUESTION_ARROW))

    def deactivate(self):
        """
        This call by metacanvas signals that the tool has been
        deactivated and can now interact with metacanvas.
        """
        self.deactivate_select()
        self._is_active = False

    def activate(self, canvas=None):
        """
        This call by metacanvas signals that the tool has been
        activated and can now interact with metacanvas.
        """
        self.activate_select(canvas)  # calls also set_cursor
        self._is_active = True

    def on_left_down(self, event):
        return self.on_left_down_select(event)

    def aboard(self, event):
        # print 'aboard',self.ident
        return self.aboard_select(event)


class ConfigureTool(SelectToolMixin):
    """
    Selection tool for OGL canvas.
    """

    def __init__(self, parent, detectwidth=0.1,  detectpix=5, mainframe=None):
        """
        To be overridden by specific tool.
        """
        self.init_common('configure', parent, 'Configure tool',
                         info='Drawing options.',
                         is_textbutton=False,
                         )

        self._init_select(is_show_selected=False, detectwidth=detectwidth, detectpix=detectpix)

        # self.drawing =
        #attrsman.add(   cm.ObjConf( Nodes(self) ) )

        # self.add_col(am.TabIdsArrayConf(    'drawobjects',
        #                                    groupnames = ['options'],
        #                                    name = 'Object',
        #                                    info = 'Selected Draw objects with id',
        #                                    ))

    def set_button_info(self, bsize=(32, 32)):
        # print 'set_button_info select tool'
        self._bitmap = wx.Bitmap(os.path.join(IMAGEDIR, 'gtk_configure_24px.png'), wx.BITMAP_TYPE_PNG)
        self._bitmap_sel = wx.Bitmap(os.path.join(IMAGEDIR, 'gtk_configure_24px.png'), wx.BITMAP_TYPE_PNG)

    def set_cursor(self):
        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        if self._canvas is not None:
            self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_QUESTION_ARROW))

    def deactivate(self):
        """
        This call by metacanvas signals that the tool has been
        deactivated and can now interact with metacanvas.
        """
        self.deactivate_select()
        self._is_active = False

    def activate(self, canvas=None):
        """
        This call by metacanvas signals that the tool has been
        activated and can now interact with metacanvas.
        """
        self.activate_select(canvas)  # calls also set_cursor
        self._is_active = True

    def on_left_down(self, event):
        return self.on_left_down_select(event)

    def aboard(self, event):
        # print 'aboard',self.ident
        return self.aboard_select(event)

    def set_objbrowser(self):
        self.parent.refresh_optionspanel(self)

    def get_optionspanel(self, parent):
        """
        Return tool option widgets on given parent
        """
        drawobj, _id = self.get_current_selection()
        if drawobj is None:
            # no current selection-> show options of whole drawing
            size = (200, -1)
            self._optionspanel = NaviPanel(parent, obj=self._canvas.get_drawing(),
                                           attrconfigs=None,
                                           #tables = None,
                                           # table = None, id=None, ids=None,
                                           groupnames=['options'],
                                           mainframe=self.parent.get_mainframe(),
                                           #pos=wx.DefaultPosition, size=size, style = wx.MAXIMIZE_BOX|wx.RESIZE_BORDER,
                                           func_apply=self.on_apply_option,
                                           immediate_apply=False, panelstyle='default',  # 'instrumental'
                                           standartbuttons=['apply', 'restore'])
        else:
            # show option of currently selected drawobj
            size = (200, -1)
            self._optionspanel = ObjPanel(parent, obj=drawobj,
                                          id=_id,
                                          attrconfigs=None,
                                          #tables = None,
                                          # table = None, id=None, ids=None,
                                          groupnames=['options'],
                                          func_change_obj=None,
                                          show_groupnames=False, show_title=True, is_modal=False,
                                          mainframe=self.parent.get_mainframe(),
                                          pos=wx.DefaultPosition, size=size, style=wx.MAXIMIZE_BOX | wx.RESIZE_BORDER,
                                          func_apply=self.on_apply_option,
                                          immediate_apply=False, panelstyle='default',  # 'instrumental'
                                          standartbuttons=['apply', 'restore'])

        return self._optionspanel

    def on_apply_option(self, drawobj, id, ids):
        # print '\n\n\non_apply_option drawobj, id, ids',drawobj, id, ids
        drawing = self._canvas.get_drawing()
        drawobjs = drawing.get_drawobjs(is_anim=True)

        if drawobj in drawobjs:
            drawobj._update_colorvbo()
            drawobj._update_vertexvbo()

        self._canvas.draw()


class HandleTool(SelectTool):
    """
    General  tool to help select handles.
    """

    def __init__(self, parent, detectwidth=0.1,  detectpix=5, mainframe=None):
        """
        To be overridden by specific tool.
        """
        self.init_common('select_handles', parent, 'Handle selection',
                         info='Select handles on cancvas',
                         is_textbutton=False,
                         )

        self._init_select(detectwidth=detectwidth, detectpix=detectpix)

    def _init_select(self, is_show_selected=True, detectwidth=0.1,  detectpix=5):

        SelectTool._init_select(self, is_show_selected, detectwidth=detectwidth, detectpix=detectpix)
        self.add_col(am.ArrayConf('inds_vertex', -1,
                                  groupnames=['options'],  # ['_private_'], #'options',
                                  name='Vertex index',
                                  info='Vertex index.',
                                  ))

    def set_button_info(self, bsize=(32, 32)):
        # print 'set_button_info select tool'  Select_32px

        # wx.ART_INFORMATION
        self._bitmap = wx.Bitmap(os.path.join(IMAGEDIR, 'Cursor-Click-icon_24px.png'), wx.BITMAP_TYPE_PNG)
        self._bitmap_sel = wx.Bitmap(os.path.join(IMAGEDIR, 'Cursor-Click-icon_24px.png'), wx.BITMAP_TYPE_PNG)

        #self._bitmap = wx.Bitmap(os.path.join(IMAGEDIR,'selectIcon.bmp'),wx.BITMAP_TYPE_BMP)
        # self._bitmap_sel=wx.Bitmap(os.path.join(IMAGEDIR,'selectIconSel.bmp'),wx.BITMAP_TYPE_BMP)

    def set_cursor(self):
        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        if self._canvas is not None:
            self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_RIGHT_ARROW))

    def get_current_vertexselection(self):
        if len(self) > 0:
            id_selected = self.get_ids()[self._idcounter]
            (drawobj, _id) = self.drawobjects[id_selected]
            ind_vertex = self.inds_vertex[id_selected]
            return drawobj, _id, ind_vertex
        else:
            return None, -1, -1

    def pick_all(self, event):
        """
        Pick all objets with id and vertex index, that are near the pointer
        coordinates.
        """
        # print 'pick_all'
        p = self._canvas.unproject_event(event)

        if self.detectpix > 0:
            # detect pixel sensitivity is given
            # calculate detectwidth based on current resolution
            self.detectwidth = self._canvas.get_resolution()*self.detectpix

        self._idcounter = 0
        is_draw = False
        drawing = self._canvas.get_drawing()
        # print '  ',drawing.get_drawobjs()
        for drawobj in drawing.get_drawobjs():
            # print '  ',drawobj,drawobj.get_ident()
            # experiment
            if drawobj.is_visible():
                handles = drawobj.pick_handle(p, detectwidth=self.detectwidth)
                if len(handles) > 0:
                    # print '  handles',drawobj.get_ident(),handles
                    for id_handle, ind_vertex in handles:
                        is_draw |= self.add_selection(drawobj, id_handle, ind_vertex, event)
                        #is_draw |= self.select(drawobj,id_pick, event)
                        # break
        return is_draw

    def add_selection(self, drawobj, id_handle, ind_vertex, event):
        is_draw = False
        drawobjid = self.drawobjects.convert_type((drawobj, id_handle))
        ids_match = self.select_ids(self.drawobjects.value == drawobjid)
        if len(ids_match) == 0:
            self.add_rows(1, drawobjects=[(drawobj, id_handle)], inds_vertex=[ind_vertex])
            is_draw = True
        return is_draw


class DeleteTool(SelectTool):
    """
    Delete tool for OGL canvas.
    """

    def __init__(self, parent, detectwidth=0.1,  detectpix=5, mainframe=None):
        """
        To be overridden by specific tool.
        """
        self.init_common('delete', parent, 'Delete tool',
                         info='Select and delete objects in canvas.',
                         is_textbutton=False,
                         )
        self._init_select(is_show_selected=True, detectwidth=detectwidth, detectpix=detectpix)

    def set_button_info(self, bsize=(32, 32)):
        self._bitmap = wx.ArtProvider.GetBitmap(wx.ART_DELETE, wx.ART_TOOLBAR)
        self._bitmap_sel = wx.ArtProvider.GetBitmap(wx.ART_DELETE, wx.ART_TOOLBAR)

    def set_cursor(self):
        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        if self._canvas is not None:
            # self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_RIGHT_ARROW))
            pass

    def on_execute_selection(self, event):
        """
        Definively execute operation on currently selected drawobjects.
        """
        if self.is_tool_allowed_on_selection():
            drawobj, _id = self.get_current_selection()

            if drawobj is not None:
                drawobj.del_drawobj(_id)
                self.unselect_all()
                is_draw = True
            else:
                is_draw = False
            return is_draw
        else:
            return False

    def on_change_selection(self, event):
        """
        Called after selection has been changed with SHIFT-click
        Do operation on currently selected drawobjects.
        """
        return False


class MoveTool(SelectTool):
    """
    Move tool for OGL canvas.
    """

    def __init__(self, parent, detectwidth=0.1,  detectpix=5, mainframe=None):
        """
        To be overridden by specific tool.
        """
        self.drawobj_anim, self.id_anim = (None, -1)

        self.init_common('move', parent, 'Move tool',
                         info='Select and drag objects in cancvas.',
                         is_textbutton=False,
                         )
        self._init_select(is_show_selected=True, detectwidth=detectwidth, detectpix=detectpix)

    def set_button_info(self, bsize=(32, 32)):
        self._bitmap = wx.Bitmap(os.path.join(IMAGEDIR, 'arrow_cursor_drag_24px.png'), wx.BITMAP_TYPE_PNG)
        self._bitmap_sel = wx.Bitmap(os.path.join(IMAGEDIR, 'arrow_cursor_drag_24px.png'), wx.BITMAP_TYPE_PNG)

    def activate(self, canvas=None):
        """
        This call by metacanvas signals that the tool has been
        activated and can now interact with metacanvas.
        """
        self.activate_select(canvas)
        self.is_animated = False
        self._is_active = True

    def set_cursor(self):
        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        if self._canvas is not None:
            self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_HAND))

    def deactivate(self):
        """
        This call by metacanvas signals that the tool has been
        deactivated and can now interact with metacanvas.
        """
        # self.deactivate_select()
        #self.is_animated = False
        is_draw = self.aboard()
        self._canvas.deactivate_tool()
        self._canvas = None
        self._is_active = False
        return is_draw

    def on_left_down(self, event):
        if self.is_animated:
            return self.end_animation(event)
        else:
            return self.on_left_down_select(event)

    def on_motion(self, event):
        if self.is_animated:
            return self.animate(event)
        else:
            return False

    def on_execute_selection(self, event):
        """
        Definively execute operation on currently selected drawobjects.
        """
        if self.is_tool_allowed_on_selection():
            if not self.is_animated:
                return self.begin_animation(event)
            else:
                return self.end_animation(event)

        else:
            return False

    def on_change_selection(self, event):
        """
        Called after selection has been changed with SHIFT-click
        Do operation on currently selected drawobjects.
        """
        return False

    def begin_animation(self, event):
        self.drawobj_anim, _id = self.get_current_selection()
        #self.drawobj_anim, self.id_anim = drawobj.get_anim(_id)
        self.drawobj_anim.begin_animation(_id)

        # done in select method
        #self.coord_last = self._canvas.unproject(event.GetPosition())

        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_CROSS))
        self.is_animated = True
        return True  # True for redrawing

    def end_animation(self, event):
        #drawobj, _id =  self.get_current_selection()
        #drawobj.set_anim(_id, (self.drawobj_anim, self.id_anim))
        self.drawobj_anim.end_animation()
        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_HAND))

        del self.drawobj_anim
        #del self.id_anim
        self.is_animated = False

        self.unselect_all()
        return True  # True for redrawing

    def animate(self, event):
        is_draw = False
        coords = self._canvas.unproject(event.GetPosition())
        vertex_delta = coords-self.coord_last
        if np.any(np.abs(vertex_delta) > 0):
            is_draw = self.drawobj_anim.move_animation(coords, vertex_delta)
            if is_draw:
                self.coord_last = coords
        return is_draw

    def aboard(self, event=None):
        # print 'aboard',self.ident

        is_draw = False
        if self.is_animated:
            self.drawobj_anim.del_animation()
            del self.drawobj_anim
            self.is_animated = False
            is_draw = True

        is_draw |= self.aboard_select(event)
        return is_draw  # True for redrawing


class StretchTool(HandleTool):
    """
    Stretch tool for OGL canvas.
    """

    def __init__(self, parent, detectwidth=0.1,  detectpix=5, mainframe=None):
        """
        To be overridden by specific tool.
        """
        self.drawobj_anim, self.id_anim = (None, -1)

        self.init_common('stretch', parent, 'Stretch tool',
                         info='Select vertex and stretch objetc in cancvas.',
                         is_textbutton=False,
                         )

        self._init_select(is_show_selected=True, detectwidth=detectwidth, detectpix=detectpix)

    def set_button_info(self, bsize=(32, 32)):
        # print 'set_button_info select tool'  Select_32px

        self._bitmap = wx.Bitmap(os.path.join(IMAGEDIR, 'move_vertex_24px.png'), wx.BITMAP_TYPE_PNG)
        self._bitmap_sel = wx.Bitmap(os.path.join(IMAGEDIR, 'move_vertex_24px.png'), wx.BITMAP_TYPE_PNG)

        # self._bitmap = wx.Bitmap(os.path.join(IMAGEDIR,'Cursor-Click-icon_24px.png'),wx.BITMAP_TYPE_PNG)
        # self._bitmap_sel=wx.Bitmap(os.path.join(IMAGEDIR,'Cursor-Click-icon_24px.png'),wx.BITMAP_TYPE_PNG)

    def set_cursor(self):
        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        if self._canvas is not None:
            self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_HAND))

    def activate(self, canvas=None):
        """
        This call by metacanvas signals that the tool has been
        activated and can now interact with metacanvas.
        """
        self.activate_select(canvas)
        self.is_animated = False
        self._is_active = True

    def deactivate(self):
        """
        This call by metacanvas signals that the tool has been
        deactivated and can now interact with metacanvas.
        """
        is_draw = self.aboard()
        self._canvas.deactivate_tool()
        self._canvas = None
        self._is_active = False
        return is_draw
        # self.deactivate_select()
        #self.is_animated = False

    def on_left_down(self, event):
        if self.is_animated:
            return self.end_animation(event)
        else:
            # this call will set self.last_coord
            # and call self.on_execute_selection
            # in case a suitable object has been selected
            return self.on_left_down_select(event)

    def on_motion(self, event):
        if self.is_animated:
            return self.animate(event)
        else:
            return False

    def on_execute_selection(self, event):
        """
        Definively execute operation on currently selected drawobjects.
        """
        if self.is_tool_allowed_on_selection():
            if not self.is_animated:
                return self.begin_animation(event)
            else:
                return self.end_animation(event)
        else:
            return False

    def on_change_selection(self, event):
        """
        Called after selection has been changed with SHIFT-click
        Do operation on currently selected drawobjects.
        """
        return False

    def begin_animation(self, event):
        # print 'Stretchtool.begin_animation'
        self.drawobj_anim, _id, self.ind_vert = self.get_current_vertexselection()

        # print '  self.drawobj_anim, _id, self.ind_vert=',self.drawobj_anim, _id, self.ind_vert
        #self.drawobj_anim, self.id_anim = drawobj.get_anim(_id)
        self.drawobj_anim.begin_animation(_id)
        self.coord_last = self._canvas.unproject(event.GetPosition())

        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_CROSS))
        self.is_animated = True
        return True  # True for redrawing

    def end_animation(self, event):
        #drawobj, _id =  self.get_current_selection()
        #drawobj.set_anim(_id, (self.drawobj_anim, self.id_anim))
        self.drawobj_anim.end_animation()
        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_HAND))

        del self.drawobj_anim
        #del self.id_anim
        self.is_animated = False

        self.unselect_all()
        return True  # True for redrawing

    def animate(self, event):
        # print 'animate'
        is_draw = False
        coords = self._canvas.unproject(event.GetPosition())
        vertex_delta = coords-self.coord_last
        if np.any(np.abs(vertex_delta) > 0):
            is_draw = self.drawobj_anim.stretch_animation(coords, vertex_delta, self.ind_vert)
            if is_draw:
                self.coord_last = coords
        return is_draw

    def aboard(self, event=None):
        # print 'aboard',self.ident

        is_draw = False
        if self.is_animated:
            self.drawobj_anim.del_animation()
            del self.drawobj_anim
            self.is_animated = False
            is_draw = True

        is_draw |= self.aboard_select(event)
        return is_draw  # True for redrawing


class DrawobjMixin(am.ArrayObjman):
    def init_common(self, ident, parent=None, name=None,
                    linewidth=1,
                    is_parentobj=False,
                    is_outline=True,  # show outline
                    is_fill=False,
                    is_fillable=True,
                    n_vert=2,  # number of vertex per draw object
                    c_highl=0.3, **kwargs):

        self._init_objman(ident, parent=parent, name=name, **kwargs)

        self._vbos = OrderedDict()

        self._n_vert = n_vert

        # ident of drawobject used for animations
        self._ident_drawobj_anim = 'anim_lines'

        self.add(am.AttrConf('color_anim',  np.array([0.9, 0.9, 0.9, 0.5], dtype=np.float32),
                             groupnames=['_private_'],
                             perm='rw',
                             metatype='color',
                             name='Anim. color',
                             is_save=True,
                             info='Color of animated object(s) in [r,g,b,a]',
                             ))

        # TODO: these 2 could be eliminated and replaced by function
        # self._n_elem = 0# will be adjusted in update_vertexvbo
        #self._n_elem_fill = 0#

        self.add(cm.AttrConf('_is_visible', True,
                             groupnames=['options'],
                             perm='wr',
                             is_save=True,
                             is_plugin=False,
                             name='Visible?',
                             info='If True, object are visible, otherwise it is not drwan.',
                             ))

        if is_fillable > 0:
            # objects can potentially be filled
            self.add(cm.AttrConf('_is_fill', is_fill,
                                 groupnames=['_private'],
                                 perm='wr',
                                 is_save=True,
                                 is_plugin=False,
                                 name='Fill',
                                 info='If True, object are filled.',
                                 ))
            self.add(cm.AttrConf('_is_outline', is_outline,
                                 groupnames=['_private'],
                                 perm='rw',
                                 is_save=True,
                                 is_plugin=False,
                                 name='Outline',
                                 info='If True, outline of object is shown.',
                                 ))

            self.add_col(am.ArrayConf('colors_fill',  np.zeros(4, dtype=np.float32),
                                      dtype=np.float32,
                                      groupnames=['options'],
                                      perm='rw',
                                      metatype='color',
                                      name='Fill color',
                                      is_save=True,
                                      info='Object fill color with format [r,g,b,a]',
                                      ))

            self.add_col(am.ArrayConf('colors_fill_highl',  np.zeros(4, dtype=np.float32),
                                      dtype=np.float32,
                                      groupnames=['_private'],
                                      perm='rw',
                                      metatype='color',
                                      name='Fill color highl',
                                      is_save=False,
                                      info='Object fill color when highlighting with format [r,g,b,a]',
                                      ))

        else:
            # no filling foreseen
            self.add(cm.AttrConf('_is_fill', False,
                                 groupnames=['_private'],
                                 perm='r',
                                 is_save=True,
                                 is_plugin=False,
                                 name='Filled',
                                 info='If True, object is filled.',
                                 ))

            self.add(cm.AttrConf('_is_outline', True,
                                 groupnames=['_private'],
                                 perm='r',
                                 is_save=True,
                                 is_plugin=False,
                                 name='Outline',
                                 info='If True, outline of object is shown.',
                                 ))
        if is_outline:
            self.add_col(am.ArrayConf('colors',  np.zeros(4, dtype=np.float32),
                                      dtype=np.float32,
                                      groupnames=['options'],
                                      perm='rw',
                                      metatype='color',
                                      name='Color',
                                      is_save=True,
                                      info='Object Vertex color with format [r,g,b,a]',
                                      ))

            self.add_col(am.ArrayConf('colors_highl',  np.zeros(4, dtype=np.float32),
                                      dtype=np.float32,
                                      groupnames=['_private'],
                                      perm='rw',
                                      metatype='color',
                                      name='Color highl',
                                      is_save=False,
                                      info='Added object color when highlighting with format [r,g,b,a]',
                                      ))

        self.add(cm.AttrConf('linewidth', linewidth,
                             groupnames=['options'],
                             perm='wr',
                             is_save=True,
                             is_plugin=False,
                             name='Line width',
                             info='Line width in pixel',
                             ))

        # print 'init_common',self.format_ident(),self._is_fill.value,self._is_outline.value

        self.add(cm.AttrConf('c_highl', c_highl,
                             groupnames=['_private'],
                             perm='wr',
                             is_save=True,
                             is_plugin=False,
                             name='Highl. const.',
                             info='Highlight constant takes values beteewn 0 and 1 and is the amount of brightness added for highlighting',
                             ))

        self.add_col(am.ArrayConf('are_highlighted',  False,
                                  dtype=np.bool,
                                  groupnames=['options'],
                                  perm='rw',
                                  name='highlighted',
                                  is_save=False,
                                  info='If true, object is highlighted',
                                  ))
        # self.add_col(am.ArrayConf( 'have_handles',  False,
        #                            dtype=np.bool,
        #                            groupnames = ['options'],
        #                            perm='rw',
        #                            name = 'has handles',
        #                            is_save = False,
        #                            info = 'If true, object has handles',
        #                            ))

        if is_parentobj:
            self.add_col(am.IdsArrayConf('ids_parent', parent,
                                         is_save=True,
                                         name=parent.format_ident()+'[ID]',
                                         info='ID of '+parent.get_name()+' object.',
                                         ))

    def get_vertices_array(self):
        return self.vertices.value

    def get_vertices(self, ids):
        return self.vertices[ids]

    def set_vertices(self, ids, vertices, is_update=True):
        self.vertices[ids] = vertices
        if is_update:
            self._update_vertexvbo()

    def is_tool_allowed(self, tool, id_drawobj=-1):
        """
        Returns True if this tool can be applied to this drawobj.
        Optionally a particular drawobj can be specified with id_drawobj.
        """
        # basic tools:
        #tool.ident in   'configure','select_handles','delete','move','stretch'
        return True

    def is_outline(self):
        return self._is_outline.get_value()

    def is_fill(self):
        return self._is_fill.get_value()

    def is_visible(self):
        return self._is_visible.value

    def set_visible(self, is_visible):
        self._is_visible.set_value(is_visible)
        # print 'set_visible',self._is_visible.value

    def del_drawobj(self, _id, is_update=True):
        self.del_row(_id)
        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()

    def begin_animation(self, id_target):
        # print 'begin_animation',id_target
        # print '  self.vertices.value',self.vertices.value
        # print '  ids',self.get_ids()
        self._id_target = id_target
        self._drawobj_anim = self.parent.get_drawobj_by_ident(self._ident_drawobj_anim)
        self.id_anim = self._drawobj_anim.add_drawobj(np.array(self.vertices[id_target]),
                                                      self.color_anim.value,
                                                      )
        # print 'begin_animation',self.ident,_id,self._drawobj_anim
        return True

    def end_animation(self, is_del_last_vert=False):
        # print 'end_animation',self.ident,self._id_target
        # print '  verices =',self._drawobj_anim.vertices[self.id_anim]
        self.set_vertices(self._id_target, self._drawobj_anim.get_vertices(self.id_anim))  # .copy()
        self.del_animation()
        # self._drawobj_anim.del_drawobj(self.id_anim)
        self._update_vertexvbo()
        self._update_colorvbo()
        return True

    def del_animation(self, is_del_main=False):
        # print 'end_animation',self.ident,_id,self._drawobj_anim
        self._drawobj_anim.del_drawobj(self.id_anim)
        self._drawobj_anim = None

        if is_del_main:
            self.del_drawobj(self._id_target)

        self.id_anim = -1
        return True

    def move_animation(self, vertex, vertex_delta):

        # if self.ident == 'polylines':
        #    print 'move_animation',self.ident,self._id_target, vertex_delta
        #    print '  before self._drawobj_anim.vertices[self.id_anim]',self._drawobj_anim.vertices[self.id_anim]
        #    ans =  self._drawobj_anim.move(self.id_anim, vertex, vertex_delta)
        #    print '  after self._drawobj_anim.vertices[self.id_anim]',self._drawobj_anim.vertices[self.id_anim]
        #    return ans
        # else:
        return self._drawobj_anim.move(self.id_anim, vertex, vertex_delta)

    def get_id_anim(self):
        return self._id_target

    def stretch_animation(self, vertex, vertex_delta, ind_vertex):
        # print 'stretch',self.ident,_id
        # print '  ',vertex,vertex_delta,ind_vertex
        # if self.ident == 'polylines':
        #    print 'stretch_animation',self.ident,self._id_target, ind_vertex, vertex_delta
        #    print '  before self._drawobj_anim.vertices[self.id_anim]',self._drawobj_anim.vertices[self.id_anim]
        #    ans =  self._drawobj_anim.stretch(self.id_anim,vertex, vertex_delta, ind_vertex)
        #    print '  after self._drawobj_anim.vertices[self.id_anim]',self._drawobj_anim.vertices[self.id_anim]
        #    return ans
        # else:
        return self._drawobj_anim.stretch(self.id_anim, vertex, vertex_delta, ind_vertex)

    def get_anim(self, _id):
        drawobj_anim = self.parent.get_drawobj_by_ident('anim_lines')

        id_anim = drawobj_anim.add_drawobj(np.array(self.vertices[_id]),
                                           [0.9, 0.9, 0.9, 0.5]
                                           )
        # print 'get_anim',self.ident,_id,self.vertices[_id]
        return drawobj_anim, id_anim

    def set_anim(self, _id, drawobjelem_anim):
        # print 'set_anim',self.ident,_id,drawobjelem_anim
        (drawobj_anim, id_anim) = drawobjelem_anim
        # print '    self.vertices[_id]=',self.vertices[_id]
        # print '    drawobj_anim.vertices[id_anim]=',drawobj_anim.vertices[id_anim]
        self.vertices[_id] = drawobj_anim.vertices[id_anim]  # .copy()
        drawobj_anim.del_drawobj(id_anim)
        self._update_vertexvbo()
        return True

    def move(self, _id, vertex, vertex_delta):
        # print 'move',self.ident,_id, vertex_delta
        self.vertices[_id] += vertex_delta
        # print '    vertices[id_anim]=',self.vertices[_id]
        self._update_vertexvbo()
        return True

    def stretch(self, _id, vertex, vertex_delta, ind_vertex):
        # print 'stretch',self.ident,_id,ind_vertex
        # print '    vertex_delta',vertex_delta
        # print '    before vertices[_id]=',self.vertices[_id]
        self.vertices[_id][ind_vertex] += vertex_delta
        # print '    after vertices[_id]=',self.vertices[_id]
        self._update_vertexvbo()
        return True

    def add_vbo(self, vbo):
        self._vbos[vbo.get_ident()] = vbo

    def get_vbo(self, ident):
        return self._vbos[ident]

    def get_vbos(self):
        return self._vbos.values()

    def del_vbo(self, key):
        del self._vbos[key]

    def get_n_vert(self):
        return self._n_vert

    def _get_colors_highl(self, colors):
        return np.clip(colors+self.c_highl.value*np.ones(colors.shape, dtype=np.float32), 0, 1)-colors

    def set_colors(self, ids, colors, colors_highl=None, is_update=True):
        self.colors[ids] = colors
        if colors_highl is None:
            self.colors_highl[ids] = self._get_colors_highl(colors)
        else:
            self.colors_highl[ids] = colors_highl
        if is_update:
            self._update_colorvbo()

    def _update_colorvbo(self):
        n = len(self)
        if n == 0:
            for _vbo in self.get_vbos():
                _vbo.destroy()
        #n_vert = self.get_n_vert()
        # print '_update_colorvbo fancyline'

        if self._is_outline.value:
            colors = self.colors.value + self.are_highlighted.value.reshape(n, 1)*self.colors_highl.value
            for _vbo in self.get_vbos():
                if not _vbo.is_fill():
                    _vbo.update_colors(colors)

        if self._is_fill.value:
            colors = self.colors_fill.value + self.are_highlighted.value.reshape(n, 1)*self.colors_fill_highl.value
            for _vbo in self.get_vbos():
                if _vbo.is_fill():
                    _vbo.update_colors(colors)

    def draw(self, resolution=1.0):
        # print 'draw n=',id(self),self.ident,len(self),self._is_visible.value
        if len(self) == 0:
            return
        if self._is_visible.value:
            glLineWidth(self.linewidth.value)
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
            glEnable(GL_BLEND)

            # main drawing
            glEnableClientState(GL_VERTEX_ARRAY)
            glEnableClientState(GL_COLOR_ARRAY)

            # print 'draw',self.format_ident(), self._is_fill.value, self._is_outline.value

            for _vbo in self.get_vbos():
                _vbo.draw(resolution)

            # draw handles
            glDisableClientState(GL_VERTEX_ARRAY)
            glDisableClientState(GL_COLOR_ARRAY)

    def highlight(self, ids, is_update=True):
        # print 'highlight=',ids
        # print '  inds',self._inds
        # print '  inds[ids]=',self._inds[ids]
        if len(ids) > 0:
            self.are_highlighted.value[self._inds[ids]] = True
            is_draw = True
        else:
            is_draw = False

        if is_update:
            self._update_colorvbo()

        return is_draw

    def unhighlight(self, ids=None, is_update=True):

        if ids is None:
            if np.any(self.are_highlighted.value):
                inds_highl = self._inds  # np.flatnonzero(self.are_highlighted.value)
                self.are_highlighted.value[:] = False
                is_draw = True
            else:
                is_draw = False
        else:
            inds_highl = self._inds[ids]
            if np.any(self.are_highlighted.value[inds_highl]):
                self.are_highlighted.value[inds_highl] = False
                is_draw = True
            else:
                is_draw = False

        # print 'unhighlight' ,  inds_highl
        # print '  self.are_highlighted.value',self.are_highlighted.value

        if is_update:
            self._update_colorvbo()
        return is_draw

    def pick_handle(self, coord):
        """
        Retuns list [ id, ind_vert] when handle is near coord,
        otherwise []

        """
        return []


class Lines(DrawobjMixin):
    def __init__(self, ident,  parent, name='Lines',
                 is_parentobj=False,
                 n_vert=2,  # 2 verts for line draw obj
                 c_highl=0.3,
                 linewidth=3,
                 **kwargs):

        self.init_common(ident, parent=parent, name=name,
                         linewidth=linewidth,
                         is_parentobj=is_parentobj,
                         n_vert=n_vert,
                         c_highl=c_highl,
                         is_fillable=False,
                         **kwargs)

        # ident of drawobject used for animations
        # must be defined AFTER init_common
        self._ident_drawobj_anim = 'anim_lines'

        self.add_vbo(Vbo('line', GL_LINES, 2, objtype='outline'))  # 2 verts for OGL line element

        self.add_col(am.ArrayConf('vertices',  np.zeros((n_vert, 3), dtype=np.float32),
                                  dtype=np.float32,
                                  groupnames=['_private'],
                                  perm='rw',
                                  name='Vertex',
                                  unit='m',
                                  is_save=True,
                                  info='Vertex coordinate vectors of points. Example with 2 vertex: [[[x11,y11,z11],[x12,y12,z12]],[[x21,y21,z21],[x22,y22,z122]],...]',
                                  ))

    def get_boundingbox(self):
        if len(self) == 0:
            return None
        vv = self.get_vertices_array()  # self.get_vertices.value
        # TODOD: can be done faster
        return np.array([[np.min(vv[:, :, 0]), np.min(vv[:, :, 1]), np.min(vv[:, :, 2])],
                         [np.max(vv[:, :, 0]), np.max(vv[:, :, 1]), np.max(vv[:, :, 2])]
                         ], float)

    def add_drawobj(self, vertices, color, color_highl=None, is_update=True):
        if color_highl is None:
            colors_highl = self._get_colors_highl(np.array([color]))

        _id = self.add_row(vertices=vertices,
                           colors=color,
                           colors_highl=colors_highl[0],
                           )
        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()
        return _id

    def add_drawobjs(self, vertices, colors, colors_highl=None, is_update=True):
        if colors_highl is None:
            colors_highl = self._get_colors_highl(colors)

        ids = self.add_rows(len(vertices),
                            vertices=vertices,
                            colors=colors,
                            colors_highl=colors_highl,
                            )
        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()
        return ids

    def _update_vertexvbo(self):
        self.get_vbo('line').update_vertices(self.get_vertices_array().reshape((-1, 6)), len(self))

    def _make_handlevbo(self, x, y, resolution):
        # print '_get_handlevbo'
        # print '  x =\n',x
        # print '  y =\n',y
        dx = resolution * 5.0
        dy = resolution * 5.0
        z = np.zeros(x.shape, dtype=np.float32)
        n = len(z)
        hvertices = np.concatenate((x-dx, y-dy, z, x+dx, y-dy, z, x+dx, y+dy, z, x-dx, y+dy, z, ), 1).reshape(-1, 4, 3)
        # print '  hvertices =\n',hvertices
        self._vertexvbo_handles = vbo.VBO(hvertices.reshape((-1, 3)))
        self._indexvbo_handles = vbo.VBO(np.arange(4*n, dtype=np.int32), target=GL_ELEMENT_ARRAY_BUFFER)

        colors = np.repeat(np.array([[0.9, 0.8, 0.7, 0.5]], dtype=np.float32), n, 0)
        self._colorvbo_handles = vbo.VBO(colors[np.array(np.arange(0, n, 1.0/4), int)])

    def pick(self, p, detectwidth=0.1):
        """
        Returns a binary vector which is True values for lines that have been selected
        by point p.

        In particular, an element of this vector is True if the minimum distance
        between the respective line to point p is less than detectwidth
        """
        # print 'pick',self.get_ident(),len(self)
        if len(self) == 0:
            return np.array([], np.int)

        vertices = self.get_vertices_array()
        # print '  vertices',vertices
        x1 = vertices[:, 0, 0]
        y1 = vertices[:, 0, 1]

        x2 = vertices[:, 1, 0]
        y2 = vertices[:, 1, 1]

        return self._ids[get_dist_point_to_segs(p, x1, y1, x2, y2, is_ending=True) < detectwidth**2]

    def pick_handle(self, coord, detectwidth=0.1):
        """
        Retuns list [ id, ind_vert] when handle is near coord,
        otherwise []

        """
        # print 'pick_handle',self.get_ident(),len(self)
        dw = detectwidth**2

        if len(self) == 0:
            return np.zeros((0, 2), np.int)

        if self.ident not in ['lines', 'fancylines']:
            return np.zeros((0, 2), np.int)

        vertices = self.get_vertices_array()
        handles = []
        # print '  vertices',vertices
        # print '  vertices.shape',vertices.shape
        dx = vertices[:, 0, 0]-coord[0]
        dy = vertices[:, 0, 1]-coord[1]

        ids = self._ids[dx*dx+dy*dy < dw]
        handle1 = np.ones((len(ids), 2), np.int)
        handle1[:, 0] = ids
        handle1[:, 1] = 0
        # print '  ',d,handle1

        dx = vertices[:, 1, 0]-coord[0]
        dy = vertices[:, 1, 1]-coord[1]
        ids = self._ids[dx*dx+dy*dy < dw]
        handle2 = np.ones((len(ids), 2), np.int)
        handle2[:, 0] = ids
        handle2[:, 1] = 1
        # print '  ',d,handle2
        handles = np.concatenate((handle1, handle2), 0)

        return handles

    def draw_old_handles(self, resolution=1.0):
        # print 'draw n=',len(self)
        glLineWidth(self.linewidth.value)

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        glEnable(GL_BLEND)

        # main drawing
        glEnableClientState(GL_VERTEX_ARRAY)
        glEnableClientState(GL_COLOR_ARRAY)

        self._colorvbo.bind()
        glColorPointer(4, GL_FLOAT, 0, None)

        self._vertexvbo.bind()
        self._indexvbo.bind()
        glVertexPointer(3, GL_FLOAT, 0, None)

        glDrawElements(GL_LINES, self._n_vert*len(self), GL_UNSIGNED_INT, None)

        glDisableClientState(GL_VERTEX_ARRAY)
        glDisableClientState(GL_COLOR_ARRAY)
        self._vertexvbo.unbind()
        self._indexvbo.unbind()
        self._colorvbo.unbind()

        # draw handles
        if 1:
            vertices = self.vertices.value
            x1 = vertices[:, 0, 0]
            y1 = vertices[:, 0, 1]

            x2 = vertices[:, 1, 0]
            y2 = vertices[:, 1, 1]
            self._make_handlevbo(x2.reshape(-1, 1), y2.reshape(-1, 1), resolution)

            glLineWidth(2)
            glEnableClientState(GL_VERTEX_ARRAY)
            glEnableClientState(GL_COLOR_ARRAY)

            self._colorvbo_handles.bind()
            glColorPointer(4, GL_FLOAT, 0, None)

            self._vertexvbo_handles.bind()
            self._indexvbo_handles.bind()
            glVertexPointer(3, GL_FLOAT, 0, None)

            # GL_LINE_STRIP, GL_QUADS,  GL_LINES,  GL_LINE_LOOP,  GL_POINTS
            glDrawElements(GL_QUADS, 4*len(self), GL_UNSIGNED_INT, None)

            glDisableClientState(GL_VERTEX_ARRAY)
            glDisableClientState(GL_COLOR_ARRAY)
            self._vertexvbo_handles.unbind()
            self._indexvbo_handles.unbind()
            self._colorvbo_handles.unbind()


class Rectangles(Lines):
    def __init__(self, ident,  parent, name='Rectangles',
                 is_parentobj=False,
                 is_fill=True,
                 is_outline=True,  # currently only fill implemented
                 c_highl=0.3,
                 linewidth=3,
                 **kwargs):

        self.init_common(ident, parent=parent, name=name,
                         linewidth=linewidth,  # for outline only
                         is_parentobj=is_parentobj,
                         is_fill=is_fill,
                         is_outline=is_outline,
                         n_vert=1,  # vertex points for ending
                         c_highl=c_highl,
                         **kwargs)

        # ident of drawobject used for animations
        # must be defined AFTER init_common
        self._ident_drawobj_anim = 'anim_rectangles'

        # if is_outline:
        #    self.add_vbo(Vbo('outline',GL_LINES,2, noncyclic = 0))

        if is_fill:
            self.add_vbo(Vbo('rectangle_filled', GL_QUADS, 4, objtype='fill'))
        if is_outline:
            self.add_vbo(Vbo('rectangle_outline', GL_LINES, 2, objtype='outline'))

        self.add_col(am.ArrayConf('offsets',  np.zeros(3, dtype=np.float32),
                                  dtype=np.float32,
                                  groupnames=['_private'],
                                  perm='rw',
                                  name='Offset',
                                  unit='m',
                                  is_save=True,
                                  info='Offset of lower left corner',
                                  ))

        self.add_col(am.ArrayConf('widths',  0.5,
                                  dtype=np.float32,
                                  groupnames=['option'],
                                  perm='rw',
                                  name='Width',
                                  unit='m',
                                  is_save=True,
                                  info='Rectangle width',
                                  ))

        self.add_col(am.ArrayConf('lengths',  0.5,
                                  dtype=np.float32,
                                  groupnames=['option'],
                                  perm='rw',
                                  name='length',
                                  unit='m',
                                  is_save=True,
                                  info='Rectangle length',
                                  ))

        self.add_col(am.ArrayConf('rotangles_xy',  0.0,
                                  dtype=np.float32,
                                  groupnames=['option'],
                                  perm='rw',
                                  name='xy rot. angle',
                                  is_save=True,
                                  info='Counter clockwise rotation angle in xy plane',
                                  ))

    def add_drawobj(self, offset, width, length, rotangle_xy,
                    color, color_fill=None,
                    color_highl=None, color_fill_highl=None,
                    is_update=True):
        # print 'Fancylines.add_drawobj'
        if color_highl is None:
            color_highl = self._get_colors_highl(np.array([color]))[0]

        if color_fill is None:
            color_fill = color

        if color_fill_highl is None:
            color_fill_highl = self._get_colors_highl(np.array([color_fill]))[0]

        # print '  ids',self.get_ids()
        _id = self.add_row(offsets=offset,
                           widths=width,
                           lengths=length,
                           rotangles_xy=rotangle_xy,
                           colors=color,
                           colors_highl=color_highl,
                           colors_fill=color_fill,
                           colors_fill_highl=color_fill_highl,
                           )
        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()
        return _id

    def add_drawobjs(self, offsets, widths, lengths, rotangles_xy,
                     colors, colors_fill=None,
                     colors_highl=None, colors_fill_highl=None,
                     is_update=True):
        # print 'add_drawobjs'
        if colors_highl is None:
            colors_highl = self._get_colors_highl(colors)

        n = len(offsets)
        if colors_fill_highl is None:
            colors_fill_highl = self._get_colors_highl(colors_fill)
        ids = self.add_rows(n,
                            offsets=offsets,
                            widths=widths,
                            lengths=lengths,
                            rotangles_xy=rotangles_xy,
                            colors=colors,
                            colors_highl=colors_highl,
                            colors_fill=colors_fill,
                            colors_fill_highl=colors_fill_highl,
                            )
        # self.print_attrs()
        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()
        return ids

    def begin_animation(self, id_target):

        self._id_target = id_target
        self._drawobj_anim = self.parent.get_drawobj_by_ident(self._ident_drawobj_anim)
        self.id_anim = self._drawobj_anim.add_drawobj(1.0*self.offsets[id_target],
                                                      1.0*self.widths[id_target],
                                                      1.0*self.lengths[id_target],
                                                      1.0*self.rotangles_xy[id_target],
                                                      color=self.color_anim.value,
                                                      )
        # print 'begin_animation',self.ident,_id,self._drawobj_anim
        return True

    def end_animation(self, is_del_last_vert=False):
        # print 'end_animation',self.ident,self._id_target
        # print '  verices =',self._drawobj_anim.vertices[self.id_anim]
        self.set_offsets(self._id_target, self._drawobj_anim.get_offsets(self.id_anim))  # .copy()
        self.del_animation()
        # self._drawobj_anim.del_drawobj(self.id_anim)
        self._update_vertexvbo()
        self._update_colorvbo()
        return True

    def set_anim(self, _id, drawobjelem_anim):
        # print 'set_anim',self.ident,_id,drawobjelem_anim
        (drawobj_anim, id_anim) = drawobjelem_anim
        # print '    self.vertices[_id]=',self.vertices[_id]
        # print '    drawobj_anim.vertices[id_anim]=',drawobj_anim.vertices[id_anim]
        self.vertices[_id] = drawobj_anim.vertices[id_anim]  # .copy()
        drawobj_anim.del_drawobj(id_anim)
        self._update_vertexvbo()
        return True

    def move(self, _id, vertex, vertex_delta):
        # print 'move',self.ident,_id, vertex_delta
        self.offsets[_id] += vertex_delta
        # print '    vertices[id_anim]=',self.vertices[_id]
        self._update_vertexvbo()
        return True

    def stretch(self, _id, vertex, vertex_delta, ind_vertex):
        # print 'stretch',self.ident,_id,ind_vertex
        # print '    vertex_delta',vertex_delta
        # print '    before vertices[_id]=',self.vertices[_id]
        #self.vertices[_id][ind_vertex] += vertex_delta
        # print '    after vertices[_id]=',self.vertices[_id]
        self._update_vertexvbo()
        return True

    def get_offsets_array(self):
        return self.offsets.value

    def get_offsets(self, ids):
        return self.offsets[ids]

    def set_offsets(self, ids, values):
        self.offsets[ids] = values

    def get_widths_array(self):
        return self.widths.value

    def get_widths(self, ids):
        return self.widths[ids]

    def set_widths(self, ids, values):
        self.widths[ids] = values

    def get_lengths_array(self):
        return self.lengths.value

    def get_lengths(self, ids):
        return self.lengths[ids]

    def set_lengths(self, ids, values):
        self.lengths[ids] = values

    def get_rotangles_xy_array(self):
        return self.rotangles_xy.value

    def get_rotangles_xy(self, ids):
        return self.rotangles_xy[ids]

    def set_rotangles_xy(self, ids, values):
        self.rotangles_xy[ids] = values

    def _update_vertexvbo(self):
        # print '_update_vertexvbo',self.format_ident()
        self._make_lines()
        n = len(self)
        if self.is_fill():
            self.get_vbo('rectangle_filled').update_vertices(self._vertices, n)
        if self.is_outline():
            self.get_vbo('rectangle_outline').update_vertices(self._linevertices, n)

    def _make_lines(self):
        # print '_make_lines'

        vertices = self.get_vertices_array()

        n = len(self)
        n_lpe = 4  # lines per element (here 4 lines for a rectangle)
        n_lines_tot = n*n_lpe

        linevertices = np.zeros((n, 2*n_lpe, 3), np.float32)

        # fix first and last point of each rectangle
        linevertices[:, 0, :] = vertices[:, 0, :]
        linevertices[:, -1, :] = vertices[:, 0, :]
        # do rest of the vertices by doubling orginal
        linevertices[:, 1:-1, :] = np.repeat(vertices[:, 1:, :], 2, 1)
        # print '  linevertices\n',linevertices

        #vertexinds = np.zeros((n_lines_tot,2),np.int32)
        inds = self.get_inds()
        vertexinds = np.repeat(inds, 2*n_lpe).reshape((n_lines_tot, 2))
        # print '  vertexinds\n',vertexinds
        #rectinds = np.zeros(n_lines_tot,np.int32)
        rectinds = np.repeat(inds, n_lpe)

        self._vertices = vertices
        self._linevertices = linevertices.reshape((2*n_lines_tot, 3))
        # print '  self._linevertices',self._linevertices
        self._rectinds = rectinds
        self._vertexinds = vertexinds

    def get_vertices_array(self):
        # print 'Rectangles.get_vertices_array'
        n = len(self)
        offsets = self.get_offsets_array()
        widths = self.get_widths_array()
        lengths = self.get_lengths_array()
        alphas = self.get_rotangles_xy_array()
        # print 'widths',widths
        # print 'lengths',lengths
        # print '  alphas',alphas
        sin_alphas = np.sin(alphas)
        cos_alphas = np.cos(alphas)

        zeros = np.zeros(n, np.float32)
        vertices = np.zeros((n, 4, 3), np.float32)

        vertices[:, 0, :] = offsets
        vertices[:, 1, :] = offsets
        vertices_rot = rotate_vertices(widths, zeros,
                                       sin_alphas=sin_alphas,
                                       cos_alphas=cos_alphas,
                                       is_array=True,
                                       )

        # print ' vertices_rot = ',vertices_rot.shape,vertices_rot,
        vertices[:, 1, :2] += rotate_vertices(widths, zeros,
                                              sin_alphas=sin_alphas,
                                              cos_alphas=cos_alphas,
                                              is_array=True,
                                              )

        vertices[:, 2, :] = offsets
        vertices[:, 2, :2] += rotate_vertices(widths, lengths,
                                              sin_alphas=sin_alphas,
                                              cos_alphas=cos_alphas,
                                              is_array=True,
                                              )

        vertices[:, 3, :] = offsets
        vertices[:, 3, :2] += rotate_vertices(zeros, lengths,
                                              sin_alphas=sin_alphas,
                                              cos_alphas=cos_alphas,
                                              is_array=True,
                                              )

        return vertices

    def pick(self, p, detectwidth=0.1):
        """
        Returns a binary vector which is True values for lines that have been selected
        by point p.

        """
        # print 'pick'
        if len(self) == 0:
            return np.array([], np.int)

        vertices = self.get_vertices_array()
        # print '  vertices.shape',vertices.shape,'\n',vertices
        inds1 = is_inside_triangles(p,  vertices[:, 0, 0], vertices[:, 0, 1],
                                    vertices[:, 1, 0], vertices[:, 1, 1],
                                    vertices[:, 2, 0], vertices[:, 2, 1])

        inds2 = is_inside_triangles(p,  vertices[:, 0, 0], vertices[:, 0, 1],
                                    vertices[:, 2, 0], vertices[:, 2, 1],
                                    vertices[:, 3, 0], vertices[:, 3, 1])

        return self._ids[np.flatnonzero(inds1 | inds2)]


class Fancylines(Lines):
    def __init__(self, ident,  parent, name='Fancy lines',
                 is_parentobj=False,
                 is_fill=True,
                 is_outline=False,  # currently only fill implemented
                 arrowstretch=2.5,
                 arrowlength=0.5,
                 is_lefthalf=True,
                 is_righthalf=True,
                 c_highl=0.3,
                 linewidth=3,
                 **kwargs):

        self.init_common(ident, parent=parent, name=name,
                         linewidth=linewidth,  # for outline only
                         is_parentobj=is_parentobj,
                         is_fill=is_fill,
                         is_outline=is_outline,
                         n_vert=3,  # vertex points for ending
                         c_highl=c_highl,
                         **kwargs)

        # ident of drawobject used for animations
        # must be defined AFTER init_common
        self._ident_drawobj_anim = 'anim_fancylines'

        # if is_outline:
        #    self.add_vbo(Vbo('outline',GL_LINES,2, noncyclic = 0))

        if is_fill:
            self.add_vbo(Vbo('line_fill', GL_QUADS, 4, objtype='fill'))
            for style in LINEHEADS.keys():
                self.add_vbo(Vbo(('begin', 'fill', style), GL_TRIANGLES, 3, objtype='fill'))
                self.add_vbo(Vbo(('end', 'fill', style), GL_TRIANGLES, 3, objtype='fill'))

        self.add(cm.AttrConf('arrowstretch', arrowstretch,
                             groupnames=['options'],
                             perm='wr',
                             is_save=True,
                             is_plugin=False,
                             name='Arrow stretch',
                             info='Arrow stretch factor.',
                             ))

        # self.add(cm.AttrConf(  'arrowlength', arrowlength,
        #                                groupnames = ['options'],
        #                                perm='wr',
        #                                is_save = True,
        #                                is_plugin = False,
        #                                name = 'Arrow length',
        #                                info = 'Arrow length.',
        #                                ))

        self.add(cm.AttrConf('is_lefthalf', is_lefthalf,
                             groupnames=['options'],
                             perm='wr',
                             is_save=True,
                             is_plugin=False,
                             name='Show left',
                             info='Show left half of line.',
                             ))

        self.add(cm.AttrConf('is_righthalf', is_righthalf,
                             groupnames=['options'],
                             perm='wr',
                             is_save=True,
                             is_plugin=False,
                             name='Show right',
                             info='Show right half of line.',
                             ))

        self.add_col(am.ArrayConf('beginstyles', 0,
                                  groupnames=['options'],
                                  perm='rw',
                                  choices=LINEHEADS,
                                  is_save=True,
                                  is_plugin=False,
                                  name='Begin style',
                                  info='Style of the line extremity at the beginning.',
                                  ))

        self.add_col(am.ArrayConf('endstyles', 0,
                                  groupnames=['options'],
                                  perm='rw',
                                  choices=LINEHEADS,
                                  is_save=True,
                                  is_plugin=False,
                                  name='End style',
                                  info='Style of the line extremity at the ending.',
                                  ))

        self.add_col(am.ArrayConf('vertices',  np.zeros((2, 3), dtype=np.float32),
                                  dtype=np.float32,
                                  groupnames=['_private'],
                                  perm='rw',
                                  name='Vertex',
                                  unit='m',
                                  is_save=True,
                                  info='Vertex coordinate vectors of points. Example with 2 vertex: [[[x11,y11,z11],[x12,y12,z12]],[[x21,y21,z21],[x22,y22,z122]],...]',
                                  ))

        self.add_col(am.ArrayConf('widths',  0.5,
                                  dtype=np.float32,
                                  groupnames=['option'],
                                  perm='rw',
                                  name='Width',
                                  unit='m',
                                  is_save=True,
                                  info='Line width',
                                  ))

    def add_drawobj(self, vertices, width, color, color_highl=None,
                    beginstyle=None,
                    endstyle=None,
                    is_update=True):
        # print 'Fancylines.add_drawobj'
        if color_highl is None:
            color_highl = self._get_colors_highl(np.array([color]))[0]

        # print '  ids',self.get_ids()
        _id = self.add_row(vertices=vertices,
                           widths=width,
                           colors=color,
                           colors_highl=color_highl,
                           colors_fill=color,
                           colors_fill_highl=color_highl,
                           beginstyles=beginstyle,
                           endstyles=endstyle,

                           )
        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()
        return _id

    def add_drawobjs(self, vertices, widths, colors, colors_highl=None,
                     beginstyles=None, endstyles=None,
                     is_update=True):

        if colors_highl is None:
            colors_highl = self._get_colors_highl(colors)

        n = len(vertices)
        if beginstyles is None:
            beginstyles = np.zeros(n)
        if endstyles is None:
            endstyles = np.zeros(n)
        ids = self.add_rows(len(vertices),
                            vertices=vertices,
                            widths=widths,
                            colors=colors,
                            colors_highl=colors_highl,
                            colors_fill=colors,
                            colors_fill_highl=colors_highl,
                            beginstyles=beginstyles,
                            endstyles=endstyles,
                            )
        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()
        return ids

    def begin_animation(self, id_target):

        self._id_target = id_target
        self._drawobj_anim = self.parent.get_drawobj_by_ident(self._ident_drawobj_anim)
        self.id_anim = self._drawobj_anim.add_drawobj(np.array(self.get_vertices(id_target)),
                                                      1.0*self.get_widths(id_target),
                                                      self.color_anim.value,
                                                      beginstyle=self.beginstyles[id_target],
                                                      endstyle=self.endstyles[id_target],
                                                      )
        # print 'begin_animation',self.ident,_id,self._drawobj_anim
        return True

    def get_widths_array(self):
        return self.widths.value

    def get_widths(self, ids):
        return self.widths[ids]

    def set_widths(self, ids, values):
        self.widths[ids] = values

    def _update_vertexvbo(self):
        # print '_update_vertexvbo',self.format_ident()

        vertices = self.get_vertices_array()
        x1 = vertices[:, 0, 0]
        y1 = vertices[:, 0, 1]
        z1 = vertices[:, 0, 2]

        x2 = vertices[:, 1, 0]
        y2 = vertices[:, 1, 1]
        z2 = vertices[:, 1, 2]
        dx = x2-x1
        dy = y2-y1
        alpha_xy = np.arctan2(dy, dx)
        length_xy = np.sqrt(dx*dx + dy*dy)
        halfwidth = 0.5*self.get_widths_array()
        x1_new = x1.copy()
        y1_new = y1.copy()
        x2_new = x2.copy()
        y2_new = y2.copy()
        # print '  origvert'
        # print '  x1_new=',x1_new,x1_new.dtype
        # print '  x2_new=',x2_new,x2_new.dtype
        if self._is_fill.value:
            for style, id_style in LINEHEADS.iteritems():

                # begin
                inds_style = np.flatnonzero(self.beginstyles.value == id_style)

                if len(inds_style) > 0:

                    # print '     style',style,len(inds_style)
                    # print '         x1_new=',x1_new,x1_new.dtype
                    # print '         x2_new=',x2_new,x2_new.dtype
                    self._update_vertexvbo_begin_fill(style, inds_style, x1_new, y1_new,
                                                      z1, x2, y2, z2, length_xy, alpha_xy, halfwidth)

                # end
                inds_style = np.flatnonzero(self.endstyles.value == id_style)
                if len(inds_style) > 0:
                    self._update_vertexvbo_end_fill(style, inds_style, x1, y1, z1, x2_new,
                                                    y2_new, z2, length_xy, alpha_xy, halfwidth)

            self._update_vertexvbo_line_fill(x1_new, y1_new, z1, x2_new, y2_new, z2,
                                             length_xy, alpha_xy, halfwidth, len(self))

    def _update_vertexvbo_line_fill(self, x1, y1, z1, x2, y2, z2, length_xy, alpha_xy, halfwidth, n):
        # print '_update_vertexvbo_line_fill'
        # this allows different color indexing for polyline
        # TODO n
        # if inds_colors is None:
        #n = len(self)
        # else:
        #    n = None

        #n_vert = self.get_n_vert()
        #dphi = np.pi/(n_vert-1)

        #beta = alpha_xy+np.pi/2
        # print '  alpha_xy\n',alpha_xy/np.pi*180
        # print '  halfwidth\n',halfwidth
        # print '  x1\n',x1
        # print '  length_xy=',length_xy
        # print '  length_xy-self.widths.value=',(length_xy-self.widths.value)

        # self.is_righthalf.value
        xr1 = x1 + self.is_lefthalf.value * halfwidth * np.cos(alpha_xy+np.pi/2)
        yr1 = y1 + self.is_lefthalf.value * halfwidth * np.sin(alpha_xy+np.pi/2)

        xr2 = x2 + self.is_lefthalf.value * halfwidth * np.cos(alpha_xy+np.pi/2)
        yr2 = y2 + self.is_lefthalf.value * halfwidth * np.sin(alpha_xy+np.pi/2)

        xr3 = x2 + self.is_righthalf.value * halfwidth * np.cos(alpha_xy-np.pi/2)
        yr3 = y2 + self.is_righthalf.value * halfwidth * np.sin(alpha_xy-np.pi/2)

        xr4 = x1 + self.is_righthalf.value * halfwidth * np.cos(alpha_xy-np.pi/2)
        yr4 = y1 + self.is_righthalf.value * halfwidth * np.sin(alpha_xy-np.pi/2)

        # print '_update_vertexvbo ',n,n_vert,self._is_fill.value
        # print '  x\n',x
        # print '  y\n',y
        # print '  z\n',z
        n_vpe = 4
        vertices = np.zeros((len(xr1), n_vpe * 3), dtype=np.float32)

        vertices[:, 0] = xr1
        vertices[:, 1] = yr1
        vertices[:, 2] = z1

        vertices[:, 3] = xr2
        vertices[:, 4] = yr2
        vertices[:, 5] = z2

        vertices[:, 6] = xr3
        vertices[:, 7] = yr3
        vertices[:, 8] = z2

        vertices[:, 9] = xr4
        vertices[:, 10] = yr4
        vertices[:, 11] = z1

        # print '  vertices=\n',vertices
        self.get_vbo('line_fill').update_vertices(vertices, n)

        #self._vertexvbo_fill = vbo.VBO(vertices.reshape((-1,3)))
        #self._indexvbo_fill = vbo.VBO(np.arange(self._n_vert*self._n_elem_fill, dtype=np.int32), target=GL_ELEMENT_ARRAY_BUFFER)

    def _update_vertexvbo_begin_fill(self, style, inds_style, x1_new, y1_new, z1, x2, y2, z2, length_xy, alpha_xy, halfwidth):
        ident_vbo = ('begin', 'fill', style)
        headvbo = self.get_vbo(ident_vbo)
        # print '_update_vertexvbo_begin_fill'

        n = len(inds_style)
        alphastretch = 1.0
        n_vert = 6
        if style == 'bevel':
            n_vert = 6
            arrowstretch = 1.0

        if style == 'triangle':
            n_vert = 3
            arrowstretch = self.arrowstretch.value

        if style == 'arrow':
            n_vert = 3
            alphastretch = 1.2
            arrowstretch = self.arrowstretch.value
        radius = arrowstretch*halfwidth[inds_style]
        if style != 'bevel':
            x1_new[inds_style] = x2[inds_style] - (length_xy[inds_style]-radius)*np.cos(alpha_xy[inds_style])
            y1_new[inds_style] = y2[inds_style] - (length_xy[inds_style]-radius)*np.sin(alpha_xy[inds_style])

        # print '         x1_new=',x1_new,x1_new.dtype,halfwidth[inds_style]
        # print '         y1_new=',y1_new,y1_new.dtype

        if self.is_righthalf.value:
            alpha_end = np.pi/2
        else:
            n_vert = int(0.5*n_vert+0.5)
            alpha_end = 0.0

        if self.is_lefthalf.value:
            alpha_start = -np.pi/2
        else:
            n_vert = int(0.5*n_vert+0.5)
            alpha_start = 0.0

        dphi = (alpha_end-alpha_start)/(n_vert-1)
        alphas = alpha_xy[inds_style]+(alphastretch*np.arange(alpha_start, alpha_end+dphi, dphi)+np.pi).reshape(-1, 1)

        # print '  alpha_xy\n',alpha_xy/np.pi*180
        # print '  alphas=alpha0+phi\n',alphas/np.pi*180

        x = np.cos(alphas) * radius + x1_new[inds_style]
        y = np.sin(alphas) * radius + y1_new[inds_style]

        z = np.ones((n_vert, 1)) * z1[inds_style]

        # print '_update_vertexvbo ',n,n_vert,self._is_fill.value
        # print '  x\n',x
        # print '  y\n',y
        # print '  z\n',z

        # print '_update_vertexvbo_fill',n,len(x),n_vert,self._n_vpe_fill
        xf = x.transpose().flatten()  # +0.5
        yf = y.transpose().flatten()  # +0.5
        zf = z.transpose().flatten()
        xcf = (np.ones((n_vert, 1)) * x1_new[inds_style]).transpose().flatten()
        ycf = (np.ones((n_vert, 1)) * y1_new[inds_style]).transpose().flatten()
        # print '  xcf\n',xcf
        # print '  ycf\n',ycf

        # create and compose main vertices array
        n_elem_fill = (n_vert-1)*n

        n_vpe = headvbo.get_vpe()
        # print '  n_elem_fill ',self._n_elem_fill
        # print '  n_vpe',n_vpe
        vertices = np.zeros((n_elem_fill, n_vpe * 3), dtype=np.float32)
        # print '  vertices.reshape((-1,3)).shape',vertices.reshape((-1,3)).shape
        #zf = z.transpose().flatten()

        ind = np.arange(0, n*n_vert).reshape(n, n_vert)[:, :-1].flatten()

        vertices[:, 0] = xf[ind]
        vertices[:, 1] = yf[ind]
        vertices[:, 2] = zf[ind]

        vertices[:, 3] = xcf[ind]
        vertices[:, 4] = ycf[ind]
        vertices[:, 5] = zf[ind]

        ind2 = np.arange(0, n*n_vert).reshape(n, n_vert)[:, 1:].flatten()
        vertices[:, 6] = xf[ind2]
        vertices[:, 7] = yf[ind2]
        vertices[:, 8] = zf[ind2]
        # print '  vertices=\n',vertices
        headvbo.update_vertices(vertices, inds=inds_style)

        #self._vertexvbo_fill = vbo.VBO(vertices.reshape((-1,3)))
        #self._indexvbo_fill = vbo.VBO(np.arange(self._n_vert*self._n_elem_fill, dtype=np.int32), target=GL_ELEMENT_ARRAY_BUFFER)

    def _update_vertexvbo_end_fill(self, style, inds_style, x1, y1, z1, x2_new, y2_new, z2, length_xy, alpha_xy, halfwidth):
        ident_vbo = ('end', 'fill', style)
        headvbo = self.get_vbo(ident_vbo)
        # print '_update_vertexvbo_end_fill',style

        n = len(inds_style)
        alphastretch = 1.0
        n_vert = 6
        arrowstretch = 1.0
        if style == 'bevel':
            n_vert = 6

        if style == 'triangle':
            n_vert = 3
            arrowstretch = self.arrowstretch.value

        if style == 'arrow':
            n_vert = 3
            alphastretch = 1.2
            arrowstretch = self.arrowstretch.value
        radius = arrowstretch*halfwidth[inds_style]

        if style != 'bevel':
            x2_new[inds_style] = x1[inds_style] + (length_xy[inds_style]-radius)*np.cos(alpha_xy[inds_style])
            y2_new[inds_style] = y1[inds_style] + (length_xy[inds_style]-radius)*np.sin(alpha_xy[inds_style])

        if self.is_lefthalf.value:
            alpha_end = np.pi/2
        else:
            n_vert = int(0.5*n_vert+0.5)
            alpha_end = 0.0

        if self.is_righthalf.value:
            alpha_start = -np.pi/2
        else:
            n_vert = int(0.5*n_vert+0.5)
            alpha_start = 0.0

        dphi = (alpha_end-alpha_start)/(n_vert-1)
        alphas = alpha_xy[inds_style]+alphastretch*np.arange(alpha_start, alpha_end+dphi, dphi).reshape(-1, 1)

        dphi = np.pi/(n_vert-1)

        # print '  alpha_xy\n',alpha_xy/np.pi*180
        # print '  alphas=alpha0+phi\n',alphas/np.pi*180
        # print '  n_vert',n_vert,dphi/np.pi*180

        # print '  length_xy=',length_xy
        # print '  length_xy-self.widths.value=',(length_xy-self.widths.value)
        # print '  x2=',x2,x2.dtype

        # print '  y2_new=',y2_new,y2_new.dtype
        x = np.cos(alphas) * radius + x2_new[inds_style]
        y = np.sin(alphas) * radius + y2_new[inds_style]

        z = np.ones((n_vert, 1)) * z2[inds_style]

        # print '_update_vertexvbo ',n,n_vert,self._is_fill.value
        # print '  x\n',x
        # print '  y\n',y
        # print '  z\n',z

        # print '_update_vertexvbo_fill',n,len(x),n_vert,self._n_vpe_fill
        xf = x.transpose().flatten()  # +0.5
        yf = y.transpose().flatten()  # +0.5
        zf = z.transpose().flatten()
        xcf = (np.ones((n_vert, 1)) * x2_new[inds_style]).transpose().flatten()
        ycf = (np.ones((n_vert, 1)) * y2_new[inds_style]).transpose().flatten()
        # print '  xcf\n',xcf
        # print '  ycf\n',ycf

        # create and compose main vertices array
        n_elem_fill = (n_vert-1)*n

        n_vpe = headvbo.get_vpe()
        # print '  n_elem_fill ',self._n_elem_fill
        # print '  n_vpe',n_vpe
        vertices = np.zeros((n_elem_fill, n_vpe * 3), dtype=np.float32)
        # print '  vertices.reshape((-1,3)).shape',vertices.reshape((-1,3)).shape
        #zf = z.transpose().flatten()

        ind = np.arange(0, n*n_vert).reshape(n, n_vert)[:, :-1].flatten()

        vertices[:, 0] = xf[ind]
        vertices[:, 1] = yf[ind]
        vertices[:, 2] = zf[ind]

        vertices[:, 3] = xcf[ind]
        vertices[:, 4] = ycf[ind]
        vertices[:, 5] = zf[ind]

        ind2 = np.arange(0, n*n_vert).reshape(n, n_vert)[:, 1:].flatten()
        vertices[:, 6] = xf[ind2]
        vertices[:, 7] = yf[ind2]
        vertices[:, 8] = zf[ind2]
        # print '  vertices=\n',vertices
        headvbo.update_vertices(vertices, inds=inds_style)

        #self._vertexvbo_fill = vbo.VBO(vertices.reshape((-1,3)))
        #self._indexvbo_fill = vbo.VBO(np.arange(self._n_vert*self._n_elem_fill, dtype=np.int32), target=GL_ELEMENT_ARRAY_BUFFER)

    def pick(self, p, detectwidth=0.1):
        """
        Returns a binary vector which is True values for lines that have been selected
        by point p.

        """
        # print 'pick'
        if len(self) == 0:
            return np.array([], np.int)

        vertices = self.get_vertices_array()
        x1 = vertices[:, 0, 0]
        y1 = vertices[:, 0, 1]

        x2 = vertices[:, 1, 0]
        y2 = vertices[:, 1, 1]

        # print '  x1', x1
        # print '  x2', x2
        dw = 0.5*(self.get_widths_array()+detectwidth)
        return self._ids[get_dist_point_to_segs(p, x1, y1, x2, y2, is_ending=True) < dw*dw]


class Polylines(Fancylines):
    def __init__(self, ident,  parent, name='Polylines', joinstyle=FLATHEAD, **kwargs):
        Fancylines.__init__(self, ident,  parent, name=name, **kwargs)

        # ident of drawobject used for animations
        # must be defined AFTER init_common
        self._ident_drawobj_anim = 'anim_polylines'

        # FLATHEAD = 0
        # BEVELHEAD = 1
        self.add(cm.AttrConf('joinstyle', joinstyle,
                             groupnames=['options'],
                             perm='rw',
                             is_save=True,
                             is_plugin=False,
                             name='Joinstyle',
                             info='Joinstyle of line segments.',
                             ))

        self.delete('vertices')
        self.add_col(am.ListArrayConf('vertices',
                                      # None,
                                      # dtype=np.object,
                                      groupnames=['_private'],
                                      perm='rw',
                                      name='Vertex',
                                      unit='m',
                                      is_save=True,
                                      info='Vertex coordinate vectors of points. Example with 2 vertex: [[[x11,y11,z11],[x12,y12,z12]],[[x21,y21,z21],[x22,y22,z122]],...]',
                                      ))

    def begin_animation(self, id_target):

        self._id_target = id_target
        self._drawobj_anim = self.parent.get_drawobj_by_ident(self._ident_drawobj_anim)

        self.id_anim = self._drawobj_anim.add_drawobj(  # make a copy as array, otherwise instance is copied
                                                        np.array(self.get_vertices(id_target), np.float32).tolist(),
                                                        np.array(self.get_widths(id_target), np.float32),
                                                        self.color_anim.get_value(),
                                                        beginstyle=self.beginstyles[id_target],
                                                        endstyle=self.endstyles[id_target],
        )
        # print 'begin_animation',self.ident,id_target,self._drawobj_anim
        # print '  vert=',self.vertices[id_target]
        return True

    def append_vert_to_animation(self, vert):
        # print 'append_vert_to_animation',vert, type(vert)
        # for _id in self.get_ids():
        #    print '   main.vertices',_id,self.vertices[_id], type(self.vertices[_id])
        # for _id in self._drawobj_anim.get_ids():
        #    print '   anim.vertices',_id,self._drawobj_anim.vertices[_id], type(self._drawobj_anim.vertices[_id])
        self._drawobj_anim.get_vertices(self.id_anim).append(vert)
        #vertex = self._drawobj_anim.get_vertices(self.id_anim).tolist()
        # self._drawobj_anim.set_vertices.append(vert.tolist())
        # print '  vertices',self._drawobj_anim.vertices[self.id_anim],type(self._drawobj_anim.vertices[self.id_anim])
        #vertices = self._drawobj_anim.vertices[self.id_anim]
        # vertices.append(vert)
        # self._drawobj_anim.vertices[self.id_anim]= vertices#list(np.array(vertices,np.float32))

        # print '  vertices',vertices,type(vertices)
        # print '  self._drawobj_anim.vertices[self.id_anim]',self._drawobj_anim.vertices[self.id_anim],type(self._drawobj_anim.vertices[self.id_anim])
        self._drawobj_anim._update_vertexvbo()
        self._drawobj_anim._update_colorvbo()
        return len(self._drawobj_anim.get_vertices(self.id_anim))-1  # vertex ind of next

    def end_animation(self, is_del_last_vert=False):
        # print 'end_animation',self.ident,self._id_target
        # print '  verices =',self._drawobj_anim.vertices[self.id_anim]
        if is_del_last_vert:
            # cut of last one because duplicated after adding
            self.set_vertices(self._id_target, self._drawobj_anim.get_vertices(self.id_anim)[:-1])
        else:
            self.set_vertices(self._id_target, self._drawobj_anim.get_vertices(self.id_anim))

        self.del_animation()
        # self._drawobj_anim.del_drawobj(self.id_anim)
        self._update_vertexvbo()
        self._update_colorvbo()
        return True

    def _make_lines(self):
        n_lines_tot = 0
        for polyline in self.get_vertices_array():
            n_seg = len(polyline)
            # if n_seg>1:
            n_lines_tot += n_seg-1

        linevertices = np.zeros((n_lines_tot, 2, 3), np.float32)
        vertexinds = np.zeros((n_lines_tot, 2), np.int32)

        polyinds = np.zeros(n_lines_tot, np.int32)
        linebeginstyles = np.zeros(n_lines_tot, np.int32)
        lineendstyles = np.zeros(n_lines_tot, np.int32)

        beginstyles = self.beginstyles.value
        endstyles = self.endstyles.value
        joinstyle = self.joinstyle.value
        ind = 0
        ind_line = 0
        for polyline in self.get_vertices_array():
            # print '  =======',len(polyline)
            n_seg = len(polyline)
            # print '  polyline\n',polyline

            if n_seg > 1:
                polyvinds = range(n_seg)
                # print '  polyvinds\n',polyvinds
                vi = np.zeros((2*n_seg-2), np.int32)
                vi[0] = polyvinds[0]
                vi[-1] = polyvinds[-1]

                # Important type conversion!!
                v = np.zeros((2*n_seg-2, 3), np.float32)
                v[0] = polyline[0]
                v[-1] = polyline[-1]
                if len(v) > 2:
                    v[1:-1] = np.repeat(polyline[1:-1], 2, 0)
                    vi[1:-1] = np.repeat(polyvinds[1:-1], 2)

                n_lines = len(v)/2
                # print '  v\n',v
                inds_polyline = range(ind_line, ind_line+n_lines)

                polyinds[inds_polyline] = ind

                linebeginstyles[inds_polyline] = joinstyle
                linebeginstyles[inds_polyline[0]] = beginstyles[ind]

                lineendstyles[inds_polyline] = joinstyle
                lineendstyles[inds_polyline[-1]] = endstyles[ind]

                linevertices[inds_polyline] = v.reshape((-1, 2, 3))
                vertexinds[inds_polyline] = vi.reshape((-1, 2))

                ind_line += n_lines
                ind += 1
            else:
                # empty polygon treatment
                pass

            # print '  linevertex\n',linevertices

        self._linevertices = linevertices
        self._polyinds = polyinds
        self._vertexinds = vertexinds
        self._linebeginstyles = linebeginstyles
        self._lineendstyles = lineendstyles

    def _update_vertexvbo(self):
        # print '_update_vertexvbo',self.ident
        self._make_lines()
        vertices = self._linevertices
        x1 = vertices[:, 0, 0]
        y1 = vertices[:, 0, 1]
        z1 = vertices[:, 0, 2]

        x2 = vertices[:, 1, 0]
        y2 = vertices[:, 1, 1]
        z2 = vertices[:, 1, 2]
        dx = x2-x1
        dy = y2-y1
        alpha_xy = np.arctan2(dy, dx)
        length_xy = np.sqrt(dx*dx + dy*dy)
        halfwidth = 0.5*self.get_widths_array()[self._polyinds]
        # if self.ident =='lanedraws':
        #    print '_update_vertexvbo',self.ident
        #    print '  halfwidth',halfwidth
        #    print '  x1',x1
        #    print '  y1',y1

        x1_new = x1.copy()
        y1_new = y1.copy()
        x2_new = x2.copy()
        y2_new = y2.copy()
        # print '  origvert'
        # print '  x1_new=',x1_new,x1_new.dtype
        # print '  x2_new=',x2_new,x2_new.dtype
        if self._is_fill.value:
            for style, id_style in LINEHEADS.iteritems():

                # begin
                inds_style = np.flatnonzero(self._linebeginstyles == id_style)

                if len(inds_style) > 0:

                    # print '     style',style,len(inds_style)
                    # print '         x1_new=',x1_new,x1_new.dtype
                    # print '         x2_new=',x2_new,x2_new.dtype
                    self._update_vertexvbo_begin_fill(style, inds_style, x1_new, y1_new,
                                                      z1, x2, y2, z2, length_xy, alpha_xy, halfwidth)

                # end
                inds_style = np.flatnonzero(self._lineendstyles == id_style)
                if len(inds_style) > 0:
                    self._update_vertexvbo_end_fill(style, inds_style, x1, y1, z1, x2_new,
                                                    y2_new, z2, length_xy, alpha_xy, halfwidth)

            self._update_vertexvbo_line_fill(x1_new, y1_new, z1, x2_new, y2_new, z2,
                                             length_xy, alpha_xy, halfwidth, len(vertices))

    def _update_colorvbo(self):
        n = len(self._polyinds)
        #n_vert = self.get_n_vert()
        # print '_update_colorvbo fancyline'
        #self._linecolors = np.array(linecolors, np.float32)
        #self._linecolors_highl = np.array(linecolors_highl, np.float32)
        if self._is_outline.value:
            colors = self.colors.value[self._polyinds] + \
                self.are_highlighted.value[self._polyinds].reshape(n, 1)*self.colors_highl.value[self._polyinds]
            for _vbo in self.get_vbos():
                if not _vbo.is_fill():
                    _vbo.update_colors(colors)

        if self._is_fill.value:
            colors = self.colors_fill.value[self._polyinds] + self.are_highlighted.value[self._polyinds].reshape(
                n, 1)*self.colors_fill_highl.value[self._polyinds]
            for _vbo in self.get_vbos():
                if _vbo.is_fill():
                    _vbo.update_colors(colors)

    def pick(self, p, detectwidth=0.1):
        """
        Returns a binary vector which is True values for lines that have been selected
        by point p.

        In particular, an element of this vector is True if the minimum distance
        between the respective line to point p is less than detectwidth
        """
        # print 'pick'
        if len(self) == 0:
            return np.array([], np.int)

        vertices = self._linevertices
        x1 = vertices[:, 0, 0]
        y1 = vertices[:, 0, 1]

        x2 = vertices[:, 1, 0]
        y2 = vertices[:, 1, 1]

        # print '  x1', x1
        # print '  x2', x2
        halfwidths = 0.5*(self.get_widths_array()[self._polyinds]+detectwidth)
        return self._ids[self._polyinds[get_dist_point_to_segs(p, x1, y1, x2, y2, is_ending=True) < halfwidths*halfwidths]]

    def pick_handle(self, coord, detectwidth=0.1):
        """
        Retuns list [ id, ind_vert] when handle is near coord,
        otherwise []

        """
        # print 'pick_handle',self.get_ident(),len(self)
        dw = detectwidth**2

        if len(self) == 0:
            return np.zeros((0, 2), np.int)

        # if self.ident not in [ 'lines','fancylines','polylines']:
        #    return np.zeros((0,2),np.int)

        vertices = self._linevertices  # self.get_vertices_array()
        handles = []
        # print '  vertices',vertices
        # print '  vertices.shape',vertices.shape
        dx = vertices[:, 0, 0]-coord[0]
        dy = vertices[:, 0, 1]-coord[1]
        inds = np.flatnonzero(dx*dx+dy*dy < dw)

        ids = self._ids[self._polyinds[inds]]
        handle1 = np.ones((len(ids), 2), np.int)
        handle1[:, 0] = ids
        handle1[:, 1] = self._vertexinds[inds, 0]
        # print '  ',d,handle1

        dx = vertices[:, 1, 0]-coord[0]
        dy = vertices[:, 1, 1]-coord[1]
        inds = np.flatnonzero(dx*dx+dy*dy < dw)
        ids = self._ids[self._polyinds[inds]]
        handle2 = np.ones((len(ids), 2), np.int)
        handle2[:, 0] = ids
        handle2[:, 1] = self._vertexinds[inds, 1]
        # print '  ',d,handle2
        handles = np.concatenate((handle1, handle2), 0)

        return handles

    def get_boundingbox(self):
        if len(self) == 0:
            return None
        vv = self._linevertices
        # TODOD: can be done faster
        return np.array([[np.min(vv[:, :, 0]), np.min(vv[:, :, 1]), np.min(vv[:, :, 2])],
                         [np.max(vv[:, :, 0]), np.max(vv[:, :, 1]), np.max(vv[:, :, 2])]
                         ], float)


class Circles(DrawobjMixin):

    def __init__(self, ident,  parent, name='Circles',
                 is_parentobj=False,
                 is_fill=True,  # Fill objects,
                 is_outline=True,  # show outlines
                 c_highl=0.3,
                 n_vert=7,  # default number of vertex per circle
                 linewidth=2,
                 **kwargs):

        self.init_common(ident, parent=parent, name=name,
                         linewidth=linewidth,
                         is_parentobj=is_parentobj,
                         n_vert=n_vert,
                         is_fill=is_fill,
                         is_outline=is_outline,
                         c_highl=c_highl,
                         **kwargs)
        # ident of drawobject used for animations
        # must be defined AFTER init_common
        self._ident_drawobj_anim = 'anim_circles'

        if is_outline:
            self.add_vbo(Vbo('outline', GL_LINES, 2, objtype='outline'))
        if is_fill:
            self.add_vbo(Vbo('fill', GL_TRIANGLES, 3, objtype='fill'))

        self.add_col(am.ArrayConf('centers',  np.zeros(3, dtype=np.float32),
                                  dtype=np.float32,
                                  groupnames=['_private'],
                                  perm='rw',
                                  name='Center',
                                  unit='m',
                                  is_save=True,
                                  info='Center coordinate. Example: [x,y,z]',
                                  ))

        self.add_col(am.ArrayConf('radii',  1.0,
                                  dtype=np.float32,
                                  groupnames=['_private'],
                                  perm='rw',
                                  name='Radius',
                                  is_save=True,
                                  info='Circle radius',
                                  ))

    def is_tool_allowed(self, tool, id_drawobj=-1):
        """
        Returns True if this tool can be applied to this drawobj.
        Optionally a particular drawobj can be specified with id_drawobj.
        """
        # basic tools:
        # return tool.ident not in   ['configure','select_handles','delete','move','stretch']
        return True

    def get_centers_array(self):
        return self.centers.value

    def get_radii_array(self):
        return self.radii.value

    def get_boundingbox(self):
        if len(self) == 0:
            return None
        vv = self.get_centers_array()
        rad = self.get_radii_array()
        # TODOD: can be done faster
        return np.array([[np.min(vv[:, 0]-rad), np.min(vv[:, 1]-rad), np.min(vv[:, 2])],
                         [np.max(vv[:, 0]+rad), np.max(vv[:, 1]+rad), np.max(vv[:, 2])]
                         ], float)

    def add_drawobj(self, center, radius,
                    color, color_fill=None,
                    color_highl=None, color_fill_highl=None,
                    is_update=True):

        if color_fill is None:
            color_fill = color

        if color_highl is None:
            color_highl = self._get_colors_highl(np.array([color]))[0]

        if color_fill_highl is None:
            color_fill_highl = self._get_colors_highl(np.array([color_fill]))[0]

        _id = self.add_row(centers=center,
                           radii=radius,
                           colors=color,
                           colors_highl=color_highl,
                           colors_fill=color_fill,
                           colors_fill_highl=color_fill_highl,
                           )
        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()
        return _id

    def add_drawobjs(self, centers, radii,
                     colors, colors_fill=None,
                     colors_highl=None, colors_fill_highl=None,
                     is_update=True):
        # print 'Circles.add_drawobjs',self._is_fill.value,self._is_outline.value
        if colors_fill is None:
            colors_fill = colors

        if colors_highl is None:
            colors_highl = self._get_colors_highl(colors)

        if colors_fill_highl is None:
            colors_fill_highl = self._get_colors_highl(colors_fill)

        # print '  colors',colors[:2]
        # print '  colors_highl',colors_highl[:2]

        ids = self.add_rows(len(centers),
                            centers=centers,
                            radii=radii,
                            colors=colors,
                            colors_highl=colors_highl,
                            colors_fill=colors_fill,
                            colors_fill_highl=colors_fill_highl,
                            )
        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()
        return ids

    def begin_animation(self, id_target):

        self._id_target = id_target
        self._drawobj_anim = self.parent.get_drawobj_by_ident(self._ident_drawobj_anim)
        self.id_anim = self._drawobj_anim.add_drawobj(np.array(self.centers[id_target]),
                                                      1.0*self.radii[id_target],
                                                      self.color_anim.value,
                                                      )
        # print 'begin_animation',self.ident,_id,self._drawobj_anim
        return True

    def set_anim(self, _id, drawobjelem_anim):
        # print 'set_anim',self.ident,_id,drawobj_anim
        (drawobj_anim, id_anim) = drawobjelem_anim
        # print '    self.vertices[_id]=',self.vertices[_id]
        # print '    drawobjset_anim.vertices[id_anim]=',drawobjset_anim.vertices[id_anim]
        self.centers[_id] = np.array(drawobj_anim.centers[id_anim])
        self.radii[_id] = 1.0*drawobj_anim.radii[id_anim]
        drawobj_anim.del_drawobj(id_anim)
        self._update_vertexvbo()
        return True

    def end_animation(self, is_del_last_vert=False):
        # print 'end_animation',self.ident,_id,self._drawobj_anim
        self.centers[self._id_target] = np.array(self._drawobj_anim.centers[self.id_anim])
        self.radii[self._id_target] = 1.0*self._drawobj_anim.radii[self.id_anim]

        self._drawobj_anim.del_drawobj(self.id_anim)
        self._update_vertexvbo()
        return True

    def move(self, _id, vertex, vertex_delta):
        # print 'move',self.ident,_id, vertex_delta
        self.centers[_id] += vertex_delta
        # print '    vertices[id_anim]=',self.vertices[_id]
        self._update_vertexvbo()
        return True

    def stretch(self, _id, vertex, vertex_delta, ind_vertex):
        # print 'move',self.ident,_id, v
        #self.vertices[_id, ind_vertex] += vertex_delta

        self.radii[_id] = np.sqrt(np.sum((vertex - self.centers[_id])**2))
        # print '    vertices[id_anim]=',self.vertices[_id]
        self._update_vertexvbo()
        return True

    def _update_vertexvbo(self):

        n = len(self)
        n_vert = self.get_n_vert()

        # print '_update_vertexvbo ',n,n_vert,self._is_fill.value

        # compute x,y,z of vertices of n objects
        dphi = 2*np.pi/(n_vert-1)
        phi = np.arange(0, 2*np.pi+dphi, dphi).reshape(-1, 1)

        centers = self.get_centers_array()
        radii = self.get_radii_array()
        x = np.cos(phi)*radii+centers[:, 0]

        y = np.sin(phi)*radii+centers[:, 1]

        z = np.ones((n_vert, 1)) * centers[:, 2]
        # print '  x.shape=', x.shape
        if self._is_outline.value:
            self._update_vertexvbo_lines(x, y, z)

        if self._is_fill.value:
            self._update_vertexvbo_fill(x, y, z)

    def _update_vertexvbo_fill(self, x, y, z):
        n = len(self)
        n_vert = self.get_n_vert()
        # print '_update_vertexvbo_fill',n,len(x),n_vert#,self._n_vpe_fill
        xf = x.transpose().flatten()  # +0.5
        yf = y.transpose().flatten()  # +0.5
        zf = z.transpose().flatten()

        centers = self.get_centers_array()
        #radii = self.get_radii_array()

        xcf = (np.ones((n_vert, 1)) * centers[:, 0]).transpose().flatten()
        ycf = (np.ones((n_vert, 1)) * centers[:, 1]).transpose().flatten()

        # create and compose main vertices array
        n_elem_fill = (n_vert-1)*n

        #glDrawElements(GL_TRIANGLES, self._n_vert*self._n_elem_fill, GL_UNSIGNED_INT, None)
        #  self._n_vert*self._n_elem_fill = n_vert * (n_vert-1)*n
        # repeat for each of the n objects:  self._n_vpe_fill*(n_vert-1)
        # self._n_vpe * self._n_elem_fill =  self._n_vpe * (n_vert-1)*n

        n_vpe_fill = self.get_vbo('fill').get_vpe()
        # print '  n_elem_fill',n_elem_fill,n_vpe_fill
        vertices = np.zeros((n_elem_fill, n_vpe_fill * 3), dtype=np.float32)
        # print '  vertices.reshape((-1,3)).shape',vertices.reshape((-1,3)).shape
        zf = z.transpose().flatten()

        ind = np.arange(0, n*n_vert).reshape(n, n_vert)[:, :-1].flatten()

        vertices[:, 0] = xf[ind]
        vertices[:, 1] = yf[ind]
        vertices[:, 2] = zf[ind]

        vertices[:, 3] = xcf[ind]
        vertices[:, 4] = ycf[ind]
        vertices[:, 5] = zf[ind]

        ind2 = np.arange(0, n*n_vert).reshape(n, n_vert)[:, 1:].flatten()
        vertices[:, 6] = xf[ind2]
        vertices[:, 7] = yf[ind2]
        vertices[:, 8] = zf[ind2]

        self.get_vbo('fill').update_vertices(vertices, n)
        #self._vertexvbo_fill = vbo.VBO(vertices.reshape((-1,3)))
        #self._indexvbo_fill = vbo.VBO(np.arange(self._n_vert*self._n_elem_fill, dtype=np.int32), target=GL_ELEMENT_ARRAY_BUFFER)

    def _update_vertexvbo_lines(self, x, y, z):
        # print '_update_vertexvbo_lines',len(x)
        n = len(self)
        n_vert = self.get_n_vert()

        # create and compose main vertices array
        n_elem = (n_vert-1)*n

        n_vpe = 2  # vertices per OGL element

        vertices = np.zeros((n_elem, n_vpe * 3), dtype=np.float32)

        # print '  vertices.reshape((-1,3)).shape',vertices.reshape((-1,3)).shape

        xf = x.transpose().flatten()
        yf = y.transpose().flatten()
        zf = z.transpose().flatten()

        ind = np.arange(0, n*n_vert).reshape(n, n_vert)[:, :-1].flatten()

        vertices[:, 0] = xf[ind]
        vertices[:, 1] = yf[ind]
        vertices[:, 2] = zf[ind]

        ind2 = np.arange(0, n*n_vert).reshape(n, n_vert)[:, 1:].flatten()
        vertices[:, 3] = xf[ind2]
        vertices[:, 4] = yf[ind2]
        vertices[:, 5] = zf[ind2]

        self.get_vbo('outline').update_vertices(vertices, n)

    def pick(self, p, detectwidth=0.1):
        """
        Returns a binary vector which is True values for circles that have been selected
        by point p.

        In particular, an element is selected if point p is within the circle
        """
        if len(self) == 0:
            return np.array([], np.int)

        #centers = self.centers.value
        centers = self.get_centers_array()
        radii = self.get_radii_array()+0.5*detectwidth
        dx = centers[:, 0]-p[0]
        dy = centers[:, 1]-p[1]
        return self._ids[dx*dx+dy*dy < (radii*radii)]

    def pick_handle(self, coord, detectwidth=0.1):
        """
        Retuns list [ id, ind_vert] when handle is near coord,
        otherwise []

        """
        if len(self) == 0:
            return np.zeros((0, 2), np.int)
        # print 'pick_handle',self.get_ident(),len(self)
        dw = detectwidth**2

        centers = self.get_centers_array()
        radii = self.get_radii_array()
        dx = centers[:, 0]-coord[0]
        dy = centers[:, 1]-coord[1]
        r = dx*dx+dy*dy
        ids = self._ids[(r > radii*radii-dw) & (r < radii*radii+dw)]
        #handles =  np.concatenate(( ids.reshape((len(ids),1)), np.zeros((len(ids),1),np.int)),1)

        # print '  ids',ids
        # print '  handles',handles
        return np.concatenate((ids.reshape((len(ids), 1)), np.zeros((len(ids), 1), np.int)), 1)


class Polygons(DrawobjMixin):
    def __init__(self, ident,  parent, name='Polygons', **kwargs):
        self.init_common(ident, parent=parent, name=name,
                         is_fill=False,
                         is_outline=True,
                         **kwargs)
        # ident of drawobject used for animations
        # must be defined AFTER init_common
        self._ident_drawobj_anim = 'anim_polygons'

        if self._is_outline:
            self.add_vbo(Vbo('outline', GL_LINES, 2, objtype='outline'))
        if self._is_fill:
            self.add_vbo(Vbo('fill', GL_TRIANGLES, 3, objtype='fill'))

        self.delete('vertices')
        self.add_col(am.ListArrayConf('vertices',
                                      # None,
                                      # dtype=np.object,
                                      groupnames=['_private'],
                                      perm='rw',
                                      name='Vertex',
                                      unit='m',
                                      is_save=True,
                                      info='3D coordinate list of Polygon Points.',
                                      ))

    def add_drawobj(self, vertex,
                    color, color_fill=None,
                    color_highl=None, color_fill_highl=None,
                    is_update=True):

        if color_fill is None:
            color_fill = color

        if color_highl is None:
            color_highl = self._get_colors_highl(np.array([color]))[0]

        if color_fill_highl is None:
            color_fill_highl = self._get_colors_highl(np.array([color_fill]))[0]

        _id = self.add_row(vertices=vertex,
                           colors=color,
                           colors_highl=color_highl,
                           colors_fill=color_fill,
                           colors_fill_highl=color_fill_highl,
                           )
        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()
        return _id

    def add_drawobjs(self, vertices,
                     colors, colors_fill=None,
                     colors_highl=None, colors_fill_highl=None,
                     is_update=True):

        if colors_fill is None:
            colors_fill = colors

        if colors_highl is None:
            colors_highl = self._get_colors_highl(colors)

        if colors_fill_highl is None:
            colors_fill_highl = self._get_colors_highl(colors_fill)

        ids = self.add_rows(len(vertices),
                            vertices=vertices,
                            colors=colors,
                            colors_highl=colors_highl,
                            colors_fill=colors_fill,
                            colors_fill_highl=colors_fill_highl,
                            )
        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()
        return ids

    def begin_animation(self, id_target):

        self._id_target = id_target
        self._drawobj_anim = self.parent.get_drawobj_by_ident(self._ident_drawobj_anim)
        self.id_anim = self._drawobj_anim.add_drawobj(  # make a copy as array, otherwise instance is copied
                                                        np.array(self.get_vertices(id_target), np.float32).tolist(),
                                                        self.color_anim.get_value(),
        )
        # print 'begin_animation',self.ident,_id,self._drawobj_anim
        return True

    def append_vert_to_animation(self, vert):
        # print 'append_vert_to_animation',vert, type(vert)
        # for _id in self.get_ids():
        #    print '   main.vertices',_id,self.vertices[_id], type(self.vertices[_id])
        # for _id in self._drawobj_anim.get_ids():
        #    print '   anim.vertices',_id,self._drawobj_anim.vertices[_id], type(self._drawobj_anim.vertices[_id])

        self._drawobj_anim.get_vertices(self.id_anim).append(vert)
        # print '  vertices',self._drawobj_anim.vertices[self.id_anim],type(self._drawobj_anim.vertices[self.id_anim])
        #vertices = self._drawobj_anim.vertices[self.id_anim]
        # vertices.append(vert)
        # self._drawobj_anim.vertices[self.id_anim]= vertices#list(np.array(vertices,np.float32))

        # print '  vertices',vertices,type(vertices)
        # print '  self._drawobj_anim.vertices[self.id_anim]',self._drawobj_anim.vertices[self.id_anim],type(self._drawobj_anim.vertices[self.id_anim])
        self._drawobj_anim._update_vertexvbo()
        self._drawobj_anim._update_colorvbo()
        return len(self._drawobj_anim.get_vertices(self.id_anim))-1  # vertex ind of next

    # def get_vertices_array(self):
    #    return self.vertices.value

    def _make_lines(self):
        n_lines_tot = 0
        for polyline in self.get_vertices_array():
            n_lines_tot += len(polyline)

        linevertices = np.zeros((n_lines_tot, 2, 3), np.float32)
        vertexinds = np.zeros((n_lines_tot, 2), np.int32)

        polyinds = np.zeros(n_lines_tot, np.int32)
        linebeginstyles = np.zeros(n_lines_tot, np.int32)
        lineendstyles = np.zeros(n_lines_tot, np.int32)

        ind = 0
        ind_line = 0

        for polyline in self.get_vertices_array():  # self.vertices.value:
            # print '  ======='

            # print '  polyline\n',polyline
            polyvinds = range(len(polyline))
            # print '  polyvinds\n',polyvinds
            vi = np.zeros((2*len(polyline)), np.int32)
            vi[0] = polyvinds[0]
            vi[-2] = polyvinds[-2]
            vi[-1] = polyvinds[-1]
            # print '  vi\n',vi

            # Important type conversion!!
            v = np.zeros((2*len(polyline), 3), np.float32)
            v[0] = polyline[0]
            v[-2] = polyline[-1]
            v[-1] = polyline[0]
            # print '  v\n',v
            if len(v) > 3:
                v[1:-1] = np.repeat(polyline[1:], 2, 0)
                vi[1:-1] = np.repeat(polyvinds[1:], 2)
            n_lines = len(v)/2
            # print '  v\n',v
            inds_polyline = range(ind_line, ind_line+n_lines)

            polyinds[inds_polyline] = ind
            linevertices[inds_polyline] = v.reshape((-1, 2, 3))
            vertexinds[inds_polyline] = vi.reshape((-1, 2))
            ind_line += n_lines
            ind += 1

        self._linevertices = linevertices
        self._vertexinds = vertexinds
        self._polyinds = polyinds

    def _update_vertexvbo(self):
        # print '_update_vertexvbo',self.format_ident()
        self._make_lines()

        if self._is_outline.value:
            # print '  linevertices.reshape((-1,6))',self._linevertices.reshape((-1,6)),len(self._linevertices)
            self.get_vbo('outline').update_vertices(self._linevertices.reshape((-1, 6)), len(self._linevertices))
            #self._update_vertexvbo_line_fill(x1_new,y1_new,z1,x2_new,y2_new,z2,length_xy, alpha_xy,halfwidth,len(vertices))

        # if self._is_fill.value:
        #    self._update_vertexvbo_line_fill(x1_new,y1_new,z1,x2_new,y2_new,z2,length_xy, alpha_xy,halfwidth,len(vertices))

    def _update_colorvbo(self):
        n = len(self._polyinds)
        #n_vert = self.get_n_vert()
        # print 'Polygons._update_colorvbo',self.ident,n
        # print '  colors',self.colors.value
        # print '  are_highlighted',self.are_highlighted.value
        #self._linecolors = np.array(linecolors, np.float32)
        #self._linecolors_highl = np.array(linecolors_highl, np.float32)
        if (self._is_outline.value) & (n > 0):
            colors = self.colors.value[self._polyinds] + \
                self.are_highlighted.value[self._polyinds].reshape(n, 1)*self.colors_highl.value[self._polyinds]
            self.get_vbo('outline').update_colors(colors)

        # if self._is_fill.value:
        #    colors = self.colors_fill.value[self._polyinds]  + self.are_highlighted.value[self._polyinds].reshape(n,1)*self.colors_fill_highl.value[self._polyinds]
        #    self.get_vbo('fill').update_colors(colors)

    def pick(self, p, detectwidth=0.1):
        """
        Returns a binary vector which is True values for lines that have been selected
        by point p.

        In particular, an element of this vector is True if the minimum distance
        between the respective line to point p is less than detectwidth
        """
        if len(self) == 0:
            return np.array([], np.int)

        vertices = self._linevertices
        x1 = vertices[:, 0, 0]
        y1 = vertices[:, 0, 1]

        x2 = vertices[:, 1, 0]
        y2 = vertices[:, 1, 1]

        return self._ids[self._polyinds[get_dist_point_to_segs(p, x1, y1, x2, y2, is_ending=True) < detectwidth*detectwidth]]

    def pick_handle(self, coord, detectwidth=0.1):
        """
        Retuns list [ id, ind_vert] when handle is near coord,
        otherwise []

        """
        # print 'pick_handle',self.get_ident(),len(self),detectwidth
        dw = detectwidth ** 2  # np.sqrt(detectwidth)

        if len(self) == 0:
            return np.zeros((0, 2), np.int)

        # if self.ident not in [ 'lines','fancylines','polylines']:
        #    return np.zeros((0,2),np.int)

        vertices = self._linevertices  # self.get_vertices_array()
        handles = []
        # print '  vertices',vertices
        # print '  vertices.shape',vertices.shape
        dx = vertices[:, 0, 0]-coord[0]
        dy = vertices[:, 0, 1]-coord[1]
        inds = np.flatnonzero(dx*dx+dy*dy < dw)

        ids = self._ids[self._polyinds[inds]]
        handle1 = np.ones((len(ids), 2), np.int)
        handle1[:, 0] = ids
        handle1[:, 1] = self._vertexinds[inds, 0]
        # print '  ',d,handle1

        dx = vertices[:, 1, 0]-coord[0]
        dy = vertices[:, 1, 1]-coord[1]
        inds = np.flatnonzero(dx*dx+dy*dy < dw)
        ids = self._ids[self._polyinds[inds]]
        handle2 = np.ones((len(ids), 2), np.int)
        handle2[:, 0] = ids
        handle2[:, 1] = self._vertexinds[inds, 1]
        # print '  ',d,handle2
        handles = np.concatenate((handle1, handle2), 0)

        # print '  found',len(np.flatnonzero(handles))
        return handles

    def get_boundingbox(self):
        if len(self) == 0:
            return None
        vv = self._linevertices
        # TODOD: can be done faster
        return np.array([[np.min(vv[:, :, 0]), np.min(vv[:, :, 1]), np.min(vv[:, :, 2])],
                         [np.max(vv[:, :, 0]), np.max(vv[:, :, 1]), np.max(vv[:, :, 2])]
                         ], float)


class OGLdrawing(am.ArrayObjman):
    """
    Class holding an ordered list of all OGL draw objects.
    This class manages also the order in which the draw objects are rendered.

    The basic idea is that an instance of this class can be passed to
    different OGL rendering canvas.
    """

    def __init__(self, ident='drawing', parent=None, name='OGL drawing', info='List of OGL draw objects'):
        """
        Holds all drawing objects and provides basic access.
        """
        self._init_objman(ident, parent=parent, name=name, info=info)
        self.add_col(cm.ObjsConf('drawobjects',
                                 groupnames=['options'],
                                 name='Draw object',
                                 info='Object, containing data and rendering methods.',
                                 ))

        self.add_col(am.ArrayConf('idents', '',
                                  dtype='object',
                                  perm='r',
                                  is_index=True,
                                  name='ID',
                                  info='Ident string of draw object.',
                                  ))

        self.add_col(am.ArrayConf('layers', -1.0,
                                  groupnames=['options'],
                                  perm='rw',
                                  #is_index = True,
                                  name='Layer',
                                  info='Layer defines the order in which drawobjects are drawn.',
                                  ))

    def get_drawobjs(self, is_anim=False):
        # print 'get_drawobjs'
        ids = []
        # efficient only if there are few number of different layers
        # TODO: use the new sort function of indexed cols
        for layer in sorted(set(self.layers.value)):
            inds = np.flatnonzero(self.layers.value == layer)
            ids += list(self.get_ids(inds))

        # print '  ', ids
        if not is_anim:
            ids_noanim = []
            for _id in ids:
                # print '  check',_id,self.drawobjects[_id].get_ident(),self.drawobjects[_id].get_name(),self.drawobjects[_id].get_ident().count('anim')
                if self.drawobjects[_id].get_ident().count('anim') == 0:
                    ids_noanim.append(_id)
            # print '  ids_noanim',ids_noanim
            return self.drawobjects[ids_noanim]
        else:
            return self.drawobjects[ids]  # .value.values()
        # return self.drawobjects.value.values()

    def has_drawobj_with_ident(self, ident):
        return self.idents.has_index(ident)

    def get_drawobj_by_ident(self, ident):
        if self.has_drawobj_with_ident(ident):
            _id = self.idents.get_id_from_index(ident)
            return self.drawobjects[_id]
        else:
            None

    def add_drawobj(self, drawobj, layer=0):
        id_drawobj = self.add_row(drawobjects=drawobj,
                                  idents=drawobj.get_ident(),
                                  layers=layer,
                                  )
        return id_drawobj

    def del_drawobj_by_ident(self, ident):
        if self.idents.has_index(ident):
            _id = self.idents.get_id_from_index(ident)
            self.del_row(_id)
            return True
        else:
            return False


class OGLnavcanvas(wx.Panel):
    """
    Canvas with some navigation tools.
    """

    def __init__(self, parent,
                 mainframe=None,
                 size=wx.DefaultSize,
                 ):

        wx.Panel.__init__(self, parent, wx.ID_ANY, size=size)
        sizer = wx.BoxSizer(wx.VERTICAL)

        # initialize GL canvas
        self._canvas = OGLcanvas(self, mainframe,)

        # navigation bar
        self._navbar = self.make_navbar()

        # compose navcanvas  window
        sizer.Add(self._canvas, 1, wx.GROW)
        sizer.Add(self._navbar, 0, wx.ALL | wx.ALIGN_LEFT | wx.GROW, 4)  # from NaviPanelTest
        # finish panel setup
        self.SetSizer(sizer)
        sizer.Fit(self)

    def get_canvas(self):
        return self._canvas

    def make_navbar(self):
        """
        Initialize toolbar which consist of navigation buttons
        """
        bottonsize = (16, 16)
        bottonborder = 10
        toolbarborder = 1

        # toolbar

        ##
        navbar = wx.BoxSizer(wx.HORIZONTAL)

        bitmap = wx.Bitmap(os.path.join(IMAGEDIR, 'gtk_fit_zoom_24px.png'), wx.BITMAP_TYPE_PNG)
        b = wx.BitmapButton(self, -1, bitmap, bottonsize)
        b.SetToolTipString("Zoom to fit")
        self.Bind(wx.EVT_BUTTON, self.on_zoom_tofit, b)
        navbar.Add(b, flag=wx.ALL, border=toolbarborder)  # ,border=toolbarborder ,flag=wx.ALL, # 0, wx.ALL, 5

        bitmap = wx.Bitmap(os.path.join(IMAGEDIR, 'gtk_in_zoom_24px.png'), wx.BITMAP_TYPE_PNG)
        b = wx.BitmapButton(self, -1, bitmap, bottonsize,
                            (bitmap.GetWidth()+bottonborder, bitmap.GetHeight()+bottonborder))
        b.SetToolTipString("Zoom in")
        self.Bind(wx.EVT_BUTTON, self.on_zoom_in, b)
        navbar.Add(b, flag=wx.ALL, border=toolbarborder)

        bitmap = wx.Bitmap(os.path.join(IMAGEDIR, 'gtk_out_zoom_24px.png'), wx.BITMAP_TYPE_PNG)
        b = wx.BitmapButton(self, -1, bitmap, bottonsize,
                            (bitmap.GetWidth()+bottonborder, bitmap.GetHeight()+bottonborder))
        b.SetToolTipString("Zoom out")
        self.Bind(wx.EVT_BUTTON, self.on_zoom_out, b)
        navbar.Add(b, flag=wx.ALL, border=toolbarborder)

        bitmap = wx.Bitmap(os.path.join(IMAGEDIR, 'icon_select_components.png'), wx.BITMAP_TYPE_PNG)
        b = wx.BitmapButton(self, -1, bitmap, bottonsize,
                            (bitmap.GetWidth()+bottonborder, bitmap.GetHeight()+bottonborder))
        b.SetToolTipString("Select drawing components")
        #b = wx.Button(self, label="Show/Hide")
        self.Bind(wx.EVT_BUTTON,  self.popup_showhide, b)
        navbar.Add(b)
        # navbar.AddStretchSpacer()
        # navbar.AddSpacer(3)

        self.coordsdisplay = wx.StaticText(self, -1, 'x,y',  style=wx.TE_RIGHT)
        navbar.Add(self.coordsdisplay, wx.ALL | wx.EXPAND, 5, border=toolbarborder)  # ,flag=wx.RIGHT no effect??
        # self.path = wx.TextCtrl(self, -1,'Try me', style=wx.TE_RIGHT)#size=(-1, -1))#,size=(300, -1))
        #self.path.Bind(wx.EVT_CHAR, self.on_test)
        #navbar.Add(self.path,1, wx.EXPAND, border=toolbarborder)

        #b.Bind(wx.EVT_BUTTON, self.popup_showhide)

        #b=wx.Button(self, wx.wx.ID_ZOOM_IN)
        #b.SetToolTipString("Zoom in")
        #self.Bind(wx.EVT_BUTTON, self.on_test, b)
        #navbar.Add(b, flag=wx.ALL, border=toolbarborder)

        #bitmap = wx.ArtProvider_GetBitmap(wx.ART_GO_FORWARD,wx.ART_TOOLBAR)
        # b = wx.BitmapButton(self, -1, bitmap, bottonsize,
        #               (bitmap.GetWidth()+bottonborder, bitmap.GetHeight()+bottonborder))
        #b.SetToolTipString("Go forward in browser history.")
        #self.Bind(wx.EVT_BUTTON, self.on_test, b)
        #navbar.Add(b,0, wx.EXPAND, border=toolbarborder)
        # bottons.append(b)

        # self.add_tool(
        #    'home',self.on_go_home,
        #    wx.ArtProvider.GetBitmap(wx.ART_GO_HOME, wx.ART_TOOLBAR, tsize),
        #    'show panel of root instance')

        # self.toolbar.AddSeparator()

        # panel.SetAutoLayout(True)
        # panel.SetSizer(buttons)

        navbar.Fit(self)
        return navbar

    def display_coords(self, coords):
        # self.coordsdisplay.SetLabel(str(coords[0])+','+str(coords[1]))
        self.coordsdisplay.SetLabel('%8.3f,%8.3f' % (coords[0], coords[1]))

    def get_navbar(self):
        return self._navbar

    def on_zoom_tofit(self, event=None):
        self._canvas.zoom_tofit()
        if event:
            event.Skip()

    def on_zoom_out(self, event=None):
        self._canvas.zoom_out()
        if event:
            event.Skip()

    def on_zoom_in(self, event=None):
        self._canvas.zoom_in()
        if event:
            event.Skip()

    def popup_showhide(self, event):
        #btn = event.GetEventObject()
        drawing = self._canvas.get_drawing()

        # Create the popup menu
        self._menu_showhide = AgilePopupMenu(self)
        if drawing:
            for drawobj in drawing.get_drawobjs():
                # print '  path=',drawobj.get_name(),drawobj.is_visible()
                self._menu_showhide.append_item(
                    drawobj.get_name(),
                    self.showhide_drawobjs,
                    info='Show/hide '+drawobj.get_name(),
                    check=drawobj.is_visible(),
                )

        self.PopupMenu(self._menu_showhide)
        self._menu_showhide.Destroy()
        event.Skip()

    def showhide_drawobjs(self, event):
        # print 'showhide_drawobjs'
        drawing = self._canvas.get_drawing()
        if drawing:
            for drawobj in drawing.get_drawobjs():
                menuitem = self._menu_showhide.get_menuitem(drawobj.get_name())
                # print '  set_visible=',drawobj.get_name(),menuitem.IsChecked()
                drawobj.set_visible(menuitem.IsChecked())

            self._canvas.draw()
        event.Skip()

    def on_test(self, event=None):
        print 'this is a test'


class OGLcanvas(glcanvas.GLCanvas):
    def __init__(self, parent,  mainframe=None):
        if mainframe is None:
            self._mainframe = parent
        else:
            self._mainframe = mainframe

        self.eyex = 0.0
        self.eyey = 0.0
        self.eyez = -30000.0  # -9.0

        self.centerx = 0.0
        self.centery = 0.0
        self.centerz = 0.0

        self.upx = -1.0
        self.upy = 0.0
        self.upz = 0.0

        self.g_Width = 600
        self.g_Height = 600

        self.g_nearPlane = 10.0  # 10.
        self.g_farPlane = 10.0**8  # 10.0**8 #10000.

        self.action = ""
        self.pos_start = (0.0, 0.0)
        self.trans_start = (0.0, 0.0)

        self.resetView(is_draw=False)

        self._tool = None

        # Forcing a specific style on the window.
        #   Should this include styles passed?
        style = wx.DEFAULT_FRAME_STYLE | wx.NO_FULL_REPAINT_ON_RESIZE

        attribList = (glcanvas.WX_GL_RGBA,  # RGBA
                      glcanvas.WX_GL_DOUBLEBUFFER,  # Double Buffered
                      glcanvas.WX_GL_DEPTH_SIZE, 24)  # 24 bit

        glcanvas.GLCanvas.__init__(self, parent, -1, attribList=attribList)

        self._wxversion = wx.__version__[:3]
        if self._wxversion == '2.8':
            self.GLinitialized = False
        else:
            self.init = False  # 3.0
            self.context = glcanvas.GLContext(self)

        # Set the event handlers.
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.processEraseBackgroundEvent)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_PAINT, self.processPaintEvent)

        self.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDown)
        self.Bind(wx.EVT_LEFT_DCLICK, self.OnLeftDclick)
        self.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)
        self.Bind(wx.EVT_RIGHT_DOWN, self.OnRightDown)
        self.Bind(wx.EVT_RIGHT_UP, self.OnRightUp)
        self.Bind(wx.EVT_MOTION, self.OnMotion)
        self.Bind(wx.EVT_MOUSEWHEEL, self.OnWheel)

        #self.Bind(wx.EVT_CHAR, self.on_key_down)
        # wx.EVT_KEY_DOWN(self, self.on_key_down) # NO, does not get focus
        # EVT_LEFT_DCLICK
        # EVT_MIDDLE_DCLICK
        # EVT_RIGHT_DCLICK
        # EVT_LEAVE_WINDOW
        # EVT_ENTER_WINDOW
        # EVT_MOUSE_EVENTS

        # self.SetFocus()

        self._drawing = None
        # if drawing is not None:
        #    self.set_drawing(drawing)

        # this is needed to initialize GL projections for unproject
        wx.CallAfter(self.OnSize)

    def set_drawing(self, drawing):
        if self._drawing != drawing:

            # if self._tool  is not None:
            #    self._tool.force_deactivation()

            del self._drawing
            self._drawing = drawing
            self.draw()

    def activate_tool(self, tool):
        # called from tool
        self._tool = tool

    def deactivate_tool(self):
        # called from tool

        # print 'deactivate_tool',self._tool.format_ident()
        # make sure cursor gets back to normal
        self.SetCursor(wx.NullCursor)

        # self.SetCursor(wx.StockCursor(wx.CURSOR_ARROW))
        self._tool = None
        self.draw()

    def get_tool(self):
        return self._tool

    def get_drawing(self):
        return self._drawing

    def resetView(self, is_draw=True):
        self.zoom = 65.

        self.xRotate = 180.
        self.yRotate = 180.
        self.zRotate = -90.0
        self.trans = (0.0, 0.0)

        if is_draw:
            self.draw()

    def on_key_down(self, event):
        is_draw = False
        # print 'on_key_down',event.KeyCode(),type(event.KeyCode())
        if event.KeyCode() == 'x':
            self.xRotate += 5.0
            is_draw |= True

        if is_draw:
            self.draw()
            event.Skip()

    def OnWheel(self, event, is_draw=False):
        #EventType = FloatCanvas.EVT_FC_MOUSEWHEEL
        #
        Rot = event.GetWheelRotation()
        # print 'OnWheel!!',Rot,event.ControlDown(),event.ShiftDown()
        if (not event.ShiftDown()) & event.ControlDown():  # event.ControlDown(): # zoom
            if Rot < 0:
                self.zoom_in(is_draw=False)
            else:
                self.zoom_out(is_draw=False)
            is_draw |= True

        if (event.ShiftDown()) & event.ControlDown():  # event.ControlDown(): # zoom
            if Rot < 0:
                self.xRotate -= 5.0
            else:
                self.xRotate += 5.0
            is_draw |= True

        elif self._tool is not None:
            is_draw |= self._tool.on_wheel(event)
            self.draw()
            event.Skip()

        if is_draw:
            self.draw()
            event.Skip()

    def get_resolution(self):
        """
        Resolution in GL unit per scren pixel
        """
        if (self.g_Width == 0) & (self.g_Height == 0):
            # there is no window space
            return 1.0

        v_top = self.unproject((0.5*self.g_Width, 0))
        v_bot = self.unproject((0.5*self.g_Width, self.g_Height))

        v_left = self.unproject((0, 0.5 * self.g_Height))
        v_right = self.unproject((self.g_Width,  0.5*self.g_Height))

        dy = np.abs(v_bot[1]-v_top[1])
        dx = np.abs(v_right[1]-v_left[1])

        if dx > dy:
            return dx/self.g_Width
        else:
            return dy/self.g_Height

    def zoom_tofit(self, event=None, is_draw=True):
        # print 'zoom_tofit',is_draw
        #p = self._canvas.unproject_event(event)
        #vetrex = np.array([p[0], p[1], 0.0, 1.0],float)
        #p_screen = self._canvas.project(vetrex)
        # print 'SelectTool.on_left_down (px,py)=',p
        # print '  (x,y)=',event.GetPosition(),p_screen
        self.resetView()

        vv_min, vv_max = self.get_boundingbox()
        dv = vv_max - vv_min

        # print 'vv_min',vv_min
        # print 'vv_max',vv_max

        p_min = self.project(vv_min)
        p_max = self.project(vv_max)

        # print 'p_min',p_min
        # print 'p_max',p_max

        dp = np.abs(np.array(p_max-p_min, float))

        # print '  dp',dp,dp==np.nan,np.nan
        if np.isnan(np.sum(dp)):
            return

        window = np.array([self.g_Width, self.g_Height], float)
        zoomfactor = 0.8*np.min(window/dp[:2])

        # print '  zoomfactor,zoom',zoomfactor,self.zoom

        self.zoom /= zoomfactor
        # print '  zoomfactor',zoomfactor,self.zoom

        self.draw()
        #vv_min, vv_max = self.get_boundingbox()

        # lowerleft corner
        vv_target = self.unproject((0.0, self.g_Height))

        # print '  vv_min',vv_min
        # print '  vv_target',vv_target
        # print '  trans',self.trans
        dv = 0.9*vv_target-vv_min

        # print '  dv',dv
        newtrans = np.array(self.trans) + dv[:2]
        self.trans = tuple(newtrans)
        # print '  trans',self.trans
        self.draw()

    def get_boundingbox(self):
        drawing = self.get_drawing()
        vv_min = np.inf * np.ones((1, 3), float)
        vv_max = -np.inf * np.ones((1, 3), float)
        if drawing:
            for drawobj in drawing.get_drawobjs():
                bb = drawobj.get_boundingbox()
                if bb is not None:
                    v_min, v_max = bb
                    # print '    v_min',v_min
                    # print '    v_max',v_max
                    vv_min = np.concatenate((vv_min, v_min.reshape(1, 3)), 0)
                    vv_max = np.concatenate((vv_max, v_max.reshape(1, 3)), 0)
        # print '  vv_min',vv_min
        # print '  vv_max',vv_max
        vv_min = np.min(vv_min, 0)
        vv_max = np.max(vv_max, 0)
        return vv_min, vv_max

    def zoom_in(self, event=None, is_draw=True):
        self.zoom *= 0.9
        if is_draw:
            self.draw()

    def zoom_out(self, event=None, is_draw=True):
        self.zoom *= 1.1
        if is_draw:
            self.draw()

    def OnLeftDown(self, event, is_draw=False):
        if (event.ControlDown() & event.ShiftDown()) & (self.action == ''):
            self.action = 'drag'
            self.BeginGrap(event)
            event.Skip()

        elif self._tool is not None:
            is_draw = self._tool.on_left_down(event)

        if is_draw:
            self.draw()
            event.Skip()

    def OnLeftDclick(self, event, is_draw=False):
        if self._tool is not None:
            is_draw = self._tool.on_left_dclick(event)

        if is_draw:
            self.draw()
            event.Skip()

    def OnLeftUp(self, event, is_draw=False):

        if self.action == 'drag':
            self.EndGrap(event)
            self.action == ''
            event.Skip()

        elif self._tool is not None:
            is_draw = self._tool.on_left_up(event)

        if is_draw:
            self.draw()
            event.Skip()

    def OnRightDown(self, event, is_draw=False):
        if self._tool is not None:
            is_draw = self._tool.on_right_down(event)

        if is_draw:
            self.draw()
            event.Skip()

    def OnRightUp(self, event, is_draw=False):

        if self._tool is not None:
            is_draw = self._tool.on_right_up(event)

        if is_draw:
            self.draw()
            event.Skip()

    def OnMotion(self, event, is_draw=False):
        # print 'OnMotion',event.GetPosition()
        self.GetParent().display_coords(self.unproject_event(event))

        if (event.ControlDown() & event.ShiftDown() & (self.action == 'drag')):
            self.MoveGrap(event)
            is_draw |= True
            event.Skip()

        elif (self.action == 'drag'):
            self.EndGrap(event)
            is_draw |= True
            self.action == ''
            event.Skip()

        elif self._tool is not None:
            is_draw |= self._tool.on_motion(event)

        if is_draw:
            self.draw()

    def BeginGrap(self, event):
        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        self._cursor_last = self.GetCursor()
        self.SetCursor(wx.StockCursor(wx.CURSOR_HAND))
        self.pos_start = event.GetPosition()
        self.trans_start = self.trans
        # print 'BeginGrap',self.trans

    def MoveGrap(self, event):

        x, y = self.unproject(event.GetPosition())[0:2]
        x0, y0 = self.unproject(self.pos_start)[0:2]

        self.trans = (self.trans_start[0] + (x-x0), self.trans_start[1] + (y-y0))
        # print 'MoveGrap',self.trans, x,y

    def EndGrap(self, event):
        # print 'EndGrap'
        self.SetCursor(self._cursor_last)
        self.action = ''

    #
    # wxPython Window Handlers

    def processEraseBackgroundEvent(self, event):
        """Process the erase background event."""
        pass  # Do nothing, to avoid flashing on MSWin

    def OnSize(self, event=None, win=None):
        """Process the resize event."""

        if self.GetContext():
            # Make sure the frame is shown before calling SetCurrent.
            self.Show()
            if self._wxversion != '2.8':
                self.SetCurrent(self.context)
            else:
                self.SetCurrent()

            size = self.GetClientSize()
            self.OnReshape(size.width, size.height)
            self.Refresh(False)
        if event:
            event.Skip()

    def processPaintEvent(self, event):
        """Process the drawing event."""

        # This is a 'perfect' time to initialize OpenGL ... only if we need to

        if self._wxversion != '2.8':
            self.SetCurrent(self.context)
            if not self.init:
                self.OnInitGL()
        else:
            self.SetCurrent()
            if not self.GLinitialized:
                self.OnInitGL()

        self.draw()
        event.Skip()

    #
    # GLFrame OpenGL Event Handlers

    def OnInitGL(self):
        """Initialize OpenGL for use in the window."""

        if self._wxversion == '2.8':
            self.GLinitialized = True
        else:
            self.InitGL()
            self.init = True

        glClearColor(0, 0, 0, 1)

    def InitGL(self):
        '''
        From 3.0 Demo
        Initialize GL
        '''
        pass

#        # set viewing projection
#        glClearColor(0.0, 0.0, 0.0, 1.0)
#        glClearDepth(1.0)
#
#        glMatrixMode(GL_PROJECTION)
#        glLoadIdentity()
#        gluPerspective(40.0, 1.0, 1.0, 30.0)
#
#        glMatrixMode(GL_MODELVIEW)
#        glLoadIdentity()
#        gluLookAt(0.0, 0.0, 10.0,
#                  0.0, 0.0, 0.0,
#                  0.0, 1.0, 0.0)

    def set_color_background(self, color):
        glClearColor(color[0], color[1], color[2], color[3])
        self.draw()

    def OnReshape(self, width, height):
        """Reshape the OpenGL viewport based on the dimensions of the window."""
        #global g_Width, g_Height
        self.g_Width = width
        self.g_Height = height
        if self._wxversion != '2.8':
            self.SetCurrent(self.context)
        glViewport(0, 0, self.g_Width, self.g_Height)

    def draw(self, *args, **kwargs):
        """Draw the window."""
        # print 'OGLCanvas.draw id(self._drawing)',id(self._drawing)
        if self.GetContext():
            if self._wxversion != '2.8':
                self.SetCurrent(self.context)

            # Clear frame buffer and depth buffer
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
            # Set up viewing transformation, looking down -Z axis
            glLoadIdentity()
            gluLookAt(self.eyex, self.eyey, self.eyez, self.centerx, self.centery,
                      self.centerz, self.upx, self.upy, self.upz)  # -.1,0,0

            # Set perspective (also zoom)
            glMatrixMode(GL_PROJECTION)
            glLoadIdentity()
            # the window corner OpenGL coordinates are (-+1, -+1)
            glOrtho(-1, 1, 1, -1, -1, 1)

            aspect = float(self.g_Width)/float(self.g_Height)

            gluPerspective(self.zoom, aspect, self.g_nearPlane, self.g_farPlane)
            glMatrixMode(GL_MODELVIEW)
            self.polarView()

            # resolution in GL unit per scren pixel
            resolution = self.get_resolution()
            # print '  get_resolution',resolution

            # draw actual scene
            if self._drawing:
                # self._drawing.print_attrs()
                for drawobj in self._drawing.get_drawobjs(is_anim=True):
                    #checkobj = self._drawing.get_drawobj_by_ident(drawobj.ident)
                    # if checkobj is not None:
                    #    print '\n  draw.drawobj',drawobj.ident, checkobj.ident
                    # else:
                    #    print '\n  draw.drawobj',drawobj.ident, checkobj,self._drawing.has_drawobj_with_ident(drawobj.ident)
                    drawobj.draw(resolution)

            self.SwapBuffers()

    def polarView(self):
        glTranslatef(self.trans[1], 0.0, 0.0)
        glTranslatef(0.0, -self.trans[0], 0.0)

        glRotatef(-self.zRotate, 0.0, 0.0, 1.0)
        glRotatef(-self.xRotate, 1.0, 0.0, 0.0)
        glRotatef(-self.yRotate, 0.0, 1.0, 0.0)

    def get_intersection(self, v_near, v_far):
        # 150918
        # idea from http://www.bfilipek.com/2012/06/select-mouse-opengl.html
        # https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
        d = -v_near + v_far

        t = -v_near[2]/d[2]
        v_inter = v_near+t*d

        return v_inter

    def unproject_event(self, event):
        return self.unproject(event.GetPosition())[0:2]

    def unproject_noglu(self, pos_display):
        """
        Unproject without GLU function gluUnProject
        (does not work properly)
        """
        print 'unproject'
        mousex, mousey = pos_display
        # http://antongerdelan.net/opengl/raycasting.html
        # http://www1.cs.columbia.edu/~cs4160/html04f/slides/transformations.pdf

        # The next step is to transform it into 3d normalised device coordinates.
        x = (2.0 * mousex) / self.g_Width - 1.0
        y = 1.0 - (2.0 * mousey) / self.g_Height
        z = 1.0
        ray_nds = np.array([x, y, z], dtype=np.float32)

        ray_clip = np.array([[ray_nds[0], ray_nds[1], -1.0, 1.0]], dtype=np.float32).transpose()
        print 'ray_clip=\n', ray_clip

        # projection matrix
        aspect = float(self.g_Width)/float(self.g_Height)
        f = 1.0/np.tan(self.zoom/2.0)
        z_near = self.g_nearPlane
        z_far = self.g_farPlane
        a = (z_far+z_near)/(z_near-z_far)
        b = (2*z_far*z_near)/(z_near-z_far)

        projection_matrix = np.array([
            [f/aspect,  0.,     0.,     0.],
            [0.,        f,      0.,     0.],
            [0.,        0.,     a,     b],
            [0.,        0.,     -1.,    0.],
        ], dtype=np.float32)

        # 4d Eye (Camera) Coordinates
        ray_eye = np.dot(np.linalg.inv(projection_matrix), ray_clip)
        ray_eye = np.array([[ray_eye[0], ray_eye[1], -1.0, 0.0]], float).transpose()
        print '  ray_eye=\n', ray_eye

        #  4d World Coordinates
        #gluLookAt(self.eyex, self.eyey, self.eyez, self.centerx,self.centery, self.centerz, self.upx, self.upy, self.upz)
        # print self.centerx,self.centery, self.centerz
        # print self.eyex, self.eyey, self.eyez

        f_vec = np.array([
            [self.centerx - self.eyex],
            [self.centery - self.eyey],
            [self.centerz - self.eyez],
        ], dtype=np.float32).flatten()
        f_norm = normalize(f_vec)
        # print 'f_vec=\n',f_vec
        # print 'f_norm=\n',f_norm,np.linalg.norm(f_norm)
        up_vec = np.array([
            [self.upx],
            [self.upy],
            [self.upz],
        ], dtype=np.float32).flatten()
        up_norm = normalize(up_vec)

        fxup = np.cross(f_norm, up_norm)
        s = normalize(fxup)
        sxf = np.cross(s, f_norm)
        u = normalize(sxf)

        view_matrix = np.array([
            [s[0],              s[1],           s[2],     0.],
            [u[0],              u[1],           u[2],     0.],
            [-f_norm[0],        -f_norm[1],     -f_norm[2],     0.],
            [0.,                0.,             0.,             1.],
        ], dtype=np.float32)

        ray_wor = np.dot(np.linalg.inv(view_matrix), ray_eye)
        # print 'ray_wor=\n',ray_wor,ray_wor[0:3,0]
        ray_wor = normalize(ray_wor[0:3, 0])
        print '  ray_wor=\n', ray_wor
        i0 = 8*3

        eye_vec = np.array([self.eyex, self.eyey, self.eyez], dtype=np.float32)
        t = 1.0
        # vertexvbo[i0:i0+3]=eye_vec+ray_wor*t

        # self.vertexarray[8]=eye_vec+ray_wor*t
        # self.vertexvbo.set_array(self.vertexarray)
        v_inter = eye_vec+ray_wor*t
        print '  ray pointer=', v_inter

        return v_inter[0:2]

    def unproject(self, pos_display):
        """Get the world coordinates for viewCoordinate for the event
        """

        x = pos_display[0]
        y = self.g_Height-pos_display[1]

        modelviewmatrix = glGetDoublev(GL_MODELVIEW_MATRIX)
        projectionmatrix = glGetDoublev(GL_PROJECTION_MATRIX)
        viewport = glGetInteger(GL_VIEWPORT)

        z = 0.0
        worldCoordinate_near = np.array(gluUnProject(x, y, z),
                                        dtype=np.float32)

        z = 1.0
        worldCoordinate_far = np.array(gluUnProject(
            x, y, z,
            modelviewmatrix,
            projectionmatrix,
            viewport,), dtype=np.float32)

        v_inter = self.get_intersection(worldCoordinate_near, worldCoordinate_far)
        return v_inter

    def project(self, vertex):
        """
        http://stackoverflow.com/questions/3792481/how-to-get-screen-coordinates-from-a-3d-point-opengl
        """
        modelviewmatrix = glGetDoublev(GL_MODELVIEW_MATRIX)
        projectionmatrix = glGetDoublev(GL_PROJECTION_MATRIX)
        viewport = glGetInteger(GL_VIEWPORT)

        coords = np.array(gluProject(vertex[0], vertex[1], vertex[2],
                                     modelviewmatrix, projectionmatrix,
                                     viewport))

        coords[1] = self.g_Height - coords[1]
        # print 'project',coords

        return coords


class OGLcanvasTools(ToolsPanel):
    """
    Shows a toolpallet with different tools and an options panel.
    Here tools are added which
    """

    def __init__(self, parent):
        ToolsPanel.__init__(self, parent, n_buttoncolumns=3, size=wx.DefaultSize)

        # add ainitial tool
        self.add_initial_tool(SelectTool(self))

        # self.add_tool(HandleTool(self))
        self.add_tool(StretchTool(self))
        self.add_tool(MoveTool(self))

        self.add_tool(AddLineTool(self))
        self.add_tool(AddCircleTool(self))
        self.add_tool(AddPolylineTool(self))
        self.add_tool(AddPolygonTool(self))

        self.add_tool(DeleteTool(self))

        self.add_tool(ConfigureTool(self))
        # more tools can be added later...


class OGleditor(wx.Panel):

    def __init__(self, parent,
                 mainframe=None,
                 size=wx.DefaultSize,
                 is_menu=False,  # create menu items
                 Debug=0,
                 ):

        self._drawing = None

        # if drawing is not None:....
        self.prefix_anim = 'anim_'
        self.layer_anim = 1000.0

        wx.Panel.__init__(self, parent, wx.ID_ANY, size=size)
        sizer = wx.BoxSizer(wx.HORIZONTAL)

        self._mainframe = mainframe

        # initialize GL canvas
        navcanvas = OGLnavcanvas(self, mainframe)
        #self._canvas = OGLcanvas(self)
        self._canvas = navcanvas.get_canvas()

        # compose tool pallet here
        self._toolspanel = OGLcanvasTools(self)

        # compose editor window

        # works but toolpanel changes size!!!
        # sizer.Add(self._toolspanel,0, wx.ALL | wx.ALIGN_LEFT | wx.GROW, 4)# from NaviPanelTest
        # sizer.Add(navcanvas,1,wx.GROW)

        sizer.Add(self._toolspanel, 0, wx.EXPAND)
        sizer.Add(navcanvas, 1, wx.EXPAND)

        # navbar
        #sizer.Add(self._canvas,1,wx.GROW)#
        # sizer.Add(self._navbar,0, wx.ALL | wx.ALIGN_LEFT | wx.GROW, 4)# from NaviPanelTest

        # finish panel setup
        self.SetSizer(sizer)
        # sizer.Fit(self)
        # self.Layout()

        # no use:
        #wx.EVT_SIZE(self, self.on_size)

    def get_canvas(self):
        # DEPRICATED: canvas should not be needed outside netwdit
        # this is used mainly by the tools to know on which canvas to operate on
        # if multiple canvas, use the one with focus
        return self._canvas

    def draw(self):

        self._canvas.draw()
        # in the future there can be the same drawing on multiple canvases

    def get_toolbox(self):
        return self._toolspanel

    def add_toolclass(self, Toolclass, **kwargs):
        """
        Add a new Toolclas to the tool-panel on the right side of the editor
        """
        self.get_toolbox().add_toolclass(Toolclass, **kwargs)

    def set_drawing(self, drawing):
        # self._toolspanel.reset_initial_tool()
        # here ad some additional drawing objects for animation
        # self.add_drawobjs_anim(drawing)
        self._drawing = drawing
        self._canvas.set_drawing(drawing)
        self._toolspanel.reset_initial_tool()

        # wx.CallAfter(self._canvas.zoom_tofit)

    def get_drawing(self):
        return self._drawing  # self._canvas.get_drawing()

    def add_drawobjs_anim(self, drawing=None):

        if drawing is None:
            drawing = self._drawing
        drawobjs_anim = [Lines(self.prefix_anim+'lines', drawing,
                               linewidth=1),

                         Rectangles(self.prefix_anim+'rectangles', drawing,
                                    linewidth=1,
                                    ),

                         Fancylines(self.prefix_anim+'fancylines', drawing,
                                    is_lefthalf=True,
                                    is_righthalf=True,
                                    linewidth=1,
                                    ),

                         Lines(self.prefix_anim+'lines', drawing,
                               linewidth=1),

                         Polylines(self.prefix_anim+'polylines', drawing,
                                   joinstyle=FLATHEAD,
                                   is_lefthalf=True,
                                   is_righthalf=True,
                                   linewidth=1,
                                   ),

                         Polygons(self.prefix_anim+'polygons', drawing,
                                  linewidth=1),

                         Circles(self.prefix_anim+'circles', drawing,
                                 is_fill=False,  # Fill objects,
                                 is_outline=True,  # show outlines
                                 linewidth=1,
                                 ),
                         ]

        for drawobj in drawobjs_anim:
            drawing.add_drawobj(drawobj, layer=self.layer_anim)

    def get_mainframe(self):
        # this is used mainly by the tools to know on which mainframe to operate on
        return self._mainframe

    def on_size(self, event=None):
        # print 'on_size',self._toolspanel.GetSize()
        # self._toolspanel.SetSize((300,-1))
        # self.tc.SetSize(self.GetSize())
        # self.tc.SetSize(self.GetSize())
        # self._viewtabs.SetSize(self.GetSize())
        # pass
        #wx.LayoutAlgorithm().LayoutWindow(self, self.p1)
        #wx.LayoutAlgorithm().LayoutWindow(self, self.p1)

        # important:
        #wx.LayoutAlgorithm().LayoutWindow(self, self._toolspanel)

        if event:
            event.Skip()


if __name__ == '__main__':
    ###############################################################################
    # MAIN FRAME

    from mainframe import AgileToolbarFrameMixin

    class OGLeditorMainframe(AgileToolbarFrameMixin, wx.Frame):
        """
        Simple wx frame with some special features.
        """

        def __init__(self, title, pos=wx.DefaultPosition,
                     size=(1000, 500), style=wx.DEFAULT_FRAME_STYLE,
                     name='frame'):

            # Forcing a specific style on the window.
            #   Should this include styles passed?
            style = wx.DEFAULT_FRAME_STYLE | wx.NO_FULL_REPAINT_ON_RESIZE
            wx.Frame.__init__(self, None, wx.NewId(), title, pos, size=size, style=style, name=name)
            self.gleditor = OGleditor(self)

            self.Show()  # must be here , before putting stuff on canvas

            #################################################################
            # create the menu bar

            self.menubar = AgileMenubar(self)
            self.make_menu()
            # self.menubar.append_menu('tools')
            self.SetMenuBar(self.menubar)
            self.on_test()

        def make_menu(self, event=None):
            self.menubar.append_menu('file')
            # self.menubar.append_item('file/test',self.on_test,\
            #                    shortkey='Ctrl+t',info='Draw test objects')

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

            if 1:
                lines = Lines('lines', drawing)
                lines.add_drawobjs(vertices, colors)
                drawing.add_drawobj(lines)

 # -------------------------------------------------------------------------------

            if 1:
                rectangles = Rectangles('rectangles', drawing,
                                        is_fill=True,
                                        is_outline=True)

                colors = np.array([
                    [0.2, 0.9, 0.0, 0.9],    # 0
                    [0.9, 0.2, 0.0, 0.9],    # 1
                ])

                colors2 = np.array([
                    [0.9, 0.9, 0.5, 0.3],    # 0
                    [0.9, 0.9, 0.5, 0.3],    # 1
                ])

                rectangles.add_drawobjs([[3.0, 0.0, 0.0], [0.0, 3.0, 0.0]],  # offsets
                                        widths=[1.0, 1.6],
                                        lengths=[2.0, 0.4],
                                        rotangles_xy=[0, 0.5],
                                        colors=colors,
                                        colors_fill=colors2)
                drawing.add_drawobj(rectangles)


# -------------------------------------------------------------------------------
            if 1:
                fancylines = Fancylines('fancylines', drawing,
                                        arrowstretch=1.0,
                                        is_lefthalf=True,
                                        is_righthalf=True
                                        )

                colors_fancy = np.array([
                    [0.0, 0.9, 0.0, 0.9],    # 0
                    # [0.9,0.0,0.0,0.9],    # 1
                ])
                vertices_fancy = np.array([
                    [[0.0, -1.0, 0.0], [2, -1.0, 0.0]],  # 0 green
                    # [[0.0,-1.0,0.0],[0.0,-5.0,0.0]],# 1 red
                ])

                widths = [0.5,
                          # 0.3,
                          ]
                # print '  vertices_fancy\n',vertices_fancy
                # FLATHEAD = 0
                #BEVELHEAD = 1
                #TRIANGLEHEAD = 2
                #ARROWHEAD = 3
                fancylines.add_drawobjs(vertices_fancy,
                                        widths,  # width
                                        colors_fancy,
                                        beginstyles=[TRIANGLEHEAD, ],
                                        endstyles=[TRIANGLEHEAD, ],
                                        )
                drawing.add_drawobj(fancylines, layer=10)
# -------------------------------------------------------------------------------
            if 1:
                polylines = Polylines('polylines', drawing,
                                      joinstyle=FLATHEAD,
                                      arrowstretch=1.0,
                                      is_lefthalf=True,
                                      is_righthalf=True
                                      )
                colors_poly = np.array([
                    [0.0, 0.8, 0.5, 0.9],    # 0
                    [0.8, 0.0, 0.5, 0.9],    # 1
                ])

                vertices_poly = np.array([
                    [[0.0, 2.0, 0.0], [5.0, 2.0, 0.0], [5.0, 7.0, 0.0], [10.0, 7.0, 0.0]],  # 0 green
                    [[0.0, -2.0, 0.0], [-2.0, -2.0, 0.0]],  # 1 red
                ], np.object)

                widths = [0.5,
                          0.3,
                          ]
                # print '  vertices_poly\n',vertices_poly
                polylines.add_drawobjs(vertices_poly,
                                       widths,  # width
                                       colors_poly,
                                       beginstyles=[TRIANGLEHEAD, TRIANGLEHEAD],
                                       endstyles=[TRIANGLEHEAD, TRIANGLEHEAD])
                drawing.add_drawobj(polylines, layer=-2)

# -------------------------------------------------------------------------------
            if 1:
                polygons = Polygons('polygons', drawing, linewidth=5)
                colors_poly = np.array([
                    [0.0, 0.9, 0.9, 0.9],    # 0
                    [0.8, 0.2, 0.2, 0.9],    # 1
                ])

                vertices_poly = [
                    [[0.0, 2.0, 0.0], [5.0, 2.0, 0.0], [5.0, 7.0, 0.0], ],  # 0 green
                    [[0.0, -2.0, 0.0], [-2.0, -2.0, 0.0], [-2.0, 0.0, 0.0]],  # 1 red
                ]

                print '  vertices_polygon\n', vertices_poly
                polygons.add_drawobjs(vertices_poly,
                                      colors_poly)
                polygons.add_drawobj([[5.0, -2.0, 0.0], [3.0, -2.0, 0.0], [3.0, 0.0, 0.0]],
                                     [0.8, 0.2, 0.8, 0.9])
                drawing.add_drawobj(polygons)


# -------------------------------------------------------------------------------

            if 1:
                circles = Circles('circles', drawing,
                                  is_fill=False,  # Fill objects,
                                  is_outline=True,  # show outlines
                                  )
                circles.add_drawobjs([[0.0, 0.0, 0.0], [1.0, 0.5, 0.0]], [0.5, 0.25], colors2o, colors2)
                drawing.add_drawobj(circles)

                # drawing.add_drawobj(testogl.triangles)
                # drawing.add_drawobj(testogl.rectangles)
# -------------------------------------------------------------------------------

            #canvas = self.gleditor.get_canvas()
            # canvas.set_drawing(drawing)

            #lines.add_drawobj([[0.0,0.0,0.0],[-0.2,-0.8,0.0]], [0.0,0.9,0.9,0.9])
            #circles.add_drawobj([1.5,0.0,0.0],0.6,colors2o[0], colors2[0])

            # canvas.zoom_tofit()
            # wx.CallAfter(canvas.zoom_tofit)
            self.gleditor.add_drawobjs_anim(drawing)
            self.gleditor.set_drawing(drawing)
            if event:
                event.Skip()

        def get_objbrowser(self):
            return self._objbrowser

    class OGLeditorApp(wx.App):
        def OnInit(self):
            # wx.InitAllImageHandlers()
            self.mainframe = OGLeditorMainframe("OGLeditor")
            #sumopyicon = wx.Icon(os.path.join(IMAGEDIR,'icon_sumopy.png'),wx.BITMAP_TYPE_PNG, 16,16)
            # wx.Frame.SetIcon(self.mainframe,sumopyicon)
            # self.mainframe.SetIcon(sumopyicon)

            # if True: #len(sys.argv)>=2:
            #    from  lib.net import readNet
            #    filepath = '/home/joerg/projects/sumopy/bologna/bologna4.net.xml'#sys.argv[1]
            #    _net = readNet(filepath)
            # else:
            #    _net = None

            self.SetTopWindow(self.mainframe)
            return True

    ###########################################################################
    # MAINLOOP

    if len(sys.argv) == 3:
        ident = sys.argv[1]
        dirpath = sys.argv[2]
    else:
        ident = None
        dirpath = None
        app = OGLeditorApp(0)

    if 0:  # len(noplugins)>0:
        _msg = """The following plugins could not be plugged in:\n\n  """ + noplugins[:-2] +\
            """\n\nIf you like to use these plugins, please check messages on console for Import errors and install the required packages.
        """
        dlg = wx.MessageDialog(None, _msg, "Warning from Plugins", wx.OK | wx.ICON_WARNING)
        dlg.ShowModal()
        dlg.Destroy()

    app.MainLoop()
