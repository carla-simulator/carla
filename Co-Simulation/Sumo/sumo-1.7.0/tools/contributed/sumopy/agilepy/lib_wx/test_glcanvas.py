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

# @file    test_glcanvas.py
# @author  Joerg Schweizer
# @date

'''
@author: Stou Sandalski (stou@icapsid.net)
@license:  Public Domain
'''

# Uncomment if you have multiple wxWidgets versions
#import wxversion
# wxversion.select('2.8')

import math
import wx

from wx import glcanvas
from wx.lib.buttons import GenBitmapTextButton, GenBitmapButton
try:
    from OpenGL.GL import *
    from OpenGL.GLU import *
    from OpenGL.GLUT import *
    from OpenGL.raw.GL.ARB.vertex_array_object import glGenVertexArrays, \
        glBindVertexArray

    from OpenGL.arrays import vbo
    import numpy as np

except ImportError:
    raise ImportError, "Required dependency OpenGL not present"

import sys
import os
import types
APPDIR = os.path.join(os.path.dirname(__file__), "..")
sys.path.append(os.path.join(APPDIR, "lib_base"))
IMAGEDIR = os.path.join(os.path.dirname(__file__), "images")
import classman as cm

# wx gui stuff
from wxmisc import *

import objpanel
import wxmisc

stockIDs = [
    wx.ID_ABOUT,
    wx.ID_ADD,
    wx.ID_APPLY,
    wx.ID_BOLD,
    wx.ID_CANCEL,
    wx.ID_CLEAR,
    wx.ID_CLOSE,
    wx.ID_COPY,
    wx.ID_CUT,
    wx.ID_DELETE,
    wx.ID_EDIT,
    wx.ID_FIND,
    wx.ID_FILE,
    wx.ID_REPLACE,
    wx.ID_BACKWARD,
    wx.ID_DOWN,
    wx.ID_FORWARD,
    wx.ID_UP,
    wx.ID_HELP,
    wx.ID_HOME,
    wx.ID_INDENT,
    wx.ID_INDEX,
    wx.ID_ITALIC,
    wx.ID_JUSTIFY_CENTER,
    wx.ID_JUSTIFY_FILL,
    wx.ID_JUSTIFY_LEFT,
    wx.ID_JUSTIFY_RIGHT,
    wx.ID_NEW,
    wx.ID_NO,
    wx.ID_OK,
    wx.ID_OPEN,
    wx.ID_PASTE,
    wx.ID_PREFERENCES,
    wx.ID_PRINT,
    wx.ID_PREVIEW,
    wx.ID_PROPERTIES,
    wx.ID_EXIT,
    wx.ID_REDO,
    wx.ID_REFRESH,
    wx.ID_REMOVE,
    wx.ID_REVERT_TO_SAVED,
    wx.ID_SAVE,
    wx.ID_SAVEAS,
    wx.ID_SELECTALL,
    wx.ID_STOP,
    wx.ID_UNDELETE,
    wx.ID_UNDERLINE,
    wx.ID_UNDO,
    wx.ID_UNINDENT,
    wx.ID_YES,
    wx.ID_ZOOM_100,
    wx.ID_ZOOM_FIT,
    wx.ID_ZOOM_IN,
    wx.ID_ZOOM_OUT,

]


class BaseTool(cm.BaseObjman):
    """
    This is a base tool class for Agilecanvas.
    It must handle all mouse or keyboard events,
    must create and draw helplines and finally
    modify the state of client which are grafically
    represented on the canvas.
    """

    def __init__(self, parent, mainframe=None):
        """
        To be overridden by specific tool.
        """
        self.init_common('select', parent, 'Selection tool', mainframe, info='Select objects in cancvas')

    def set_button_info(self, bsize=(32, 32)):
        # print 'set_button_info select tool'
        self._bitmap = wx.Bitmap(os.path.join(IMAGEDIR, 'selectIcon.bmp'), wx.BITMAP_TYPE_BMP)
        self._bitmap_sel = wx.Bitmap(os.path.join(IMAGEDIR, 'selectIconSel.bmp'), wx.BITMAP_TYPE_BMP)

    def get_button(self, parent, bottonsize=(32, 32), bottonborder=10):

        # simple stockbuttons
        #b=wx.Button(parent, wx.ID_DELETE)

        id = wx.NewId()
        bitmap = self._bitmap

        #b=GenBitmapTextToggleButton(parent, id, bitmap,tool.name,name = self.get_name())
        b = GenBitmapToggleButton(parent, id, bitmap, (bitmap.GetWidth()+bottonborder,
                                                       bitmap.GetHeight()+bottonborder), name=self.get_name())
        #b=GenBitmapToggleButton(self, wx.ID_DELETE)
        #b = GenBitmapTextToggleButton(self, id, None, tool.get('name',''), size = (200, 45))

        if bitmap is not None:
            #mask = wx.Mask(bitmap, wx.BLUE)
            # bitmap.SetMask(mask)
            b.SetBitmapLabel(bitmap)
            # bmp=wx.NullBitmap

        bitmap_sel = self._bitmap_sel
        if bitmap_sel is not None:
            #mask = wx.Mask(bmp, wx.BLUE)
            # bmp.SetMask(mask)
            b.SetBitmapSelected(bitmap_sel)

        b.SetUseFocusIndicator(False)

        b.SetUseFocusIndicator(False)
        # b.SetSize((36,140))
        # b.SetBestSize()
        tt = wx.ToolTip(self.get_info())
        b.SetToolTip(tt)  # .SetTip(tool.tooltip)
        return b

    def init_common(self, ident, parent, name, mainframe=None, info=None):
        # print 'Agiletool.__init__',ident,name
        self.name = name
        self.metacanvas = None
        # FSMnamed.__init__(self,ident,parent,name)
        self._init_objman(ident, parent=parent, name=name.title(), info=info)
        attrsman = self.set_attrsman(cm.Attrsman(self))

        self.access = attrsman.add(cm.AttrConf('access', ['bus', 'bike', 'tram'],
                                               groupnames=['options'],
                                               perm='rw',
                                               is_save=True,
                                               name='Access list',
                                               info='List with vehicle classes that have access',
                                               ))

        self.emissiontype = attrsman.add(cm.AttrConf('emissiontype', 'Euro 0',
                                                     groupnames=['options'],
                                                     perm='rw',
                                                     is_save=True,
                                                     name='Emission type',
                                                     info='Emission type of vehicle',
                                                     ))
        # dictionary of drawobjects that will be created during
        # the application of the tool.
        # ident is the number in chronological order of creation,
        # starting with one. Value is the ad hoc instance of a drawing
        # object.
        self.drawobjs = {}
        self.helpobjs = {}

        self.mainframe = mainframe
        # print ' call set_button',self.ident
        self.set_button_info()
        self.targetsets = {}
        # self.optionspanel=None

    def append_drawobj(self, drawobj):
        """
        Append new drawobject
        """
        n = len(self.drawobjs)+1
        self.drawobjs[n] = drawobj

    def pop_drawobj(self):
        """
        Returns most recent drawobject, removing it from the list.
        If there are no more drwobjects in the list None is returned.
        """

        n = len(self.drawobjs)
        if n > 0:
            drawobj = self.drawobjs[n]
            del self.drawobjs[n]
            return drawobj
        else:
            return None

    def get_last_drawobj(self):
        """
        Returns most recent drawobject, without changing the list.
        If there are no more drwobjects in the list None is returned.
        """
        n = len(self.drawobjs)
        if n > 0:
            drawobj = self.drawobjs[n]
            return drawobj
        else:
            return None

    def clear_drawobjs(self):
        """
        Clear list of  drawobjects, while maintaining them on metacanvas.
        """
        self.drawobjs = {}

    def del_drawobjs(self):
        """
        Remove all drawobjects from metacanvas.
        """
        while len(self.drawobjs) > 0:
            drawobj = self.pop_drawobj()
            self.metacanvas.del_obj(drawobj)

    def make_targetsets(self, setnames=None, layer=None):
        """
        Returns a dictionary with instances of targetsets as values
        and setnames as key.
        This allows the tool to select a list of sets from the a specific
        layer to which it can directly communicate.

        If no setnames are given then all sets of the specific layer 
        are returned.

        Can be used for example to change handle settings
        """
        if setnames is not None:
            objsets = {}
            for name in setnames:
                objsets[name] = self.metacanvas.get_objset_from_layer(layer, name)
            self.targetsets = objsets
        else:
            self.targetsets = self.metacanvas.get_objset_from_layer(layer)

    def set_handles(self):
        """
        Set handles to selected object sets which can be connected.
        """
        # put handles on all section objects
        for name_set in self.targetsets.keys():
            self.metacanvas.set_handles(name_set=name_set)

    def get_bitmap_from_file(self, name_bitmap):
        # print 'get_bitmap_from_file :'+"gui/bitmaps/" + name_bitmap + ".bmp"
        return wx.Bitmap("gui/bitmaps/" + name_bitmap + ".bmp",
                         wx.BITMAP_TYPE_BMP)

    # def make_optionpanel(self,panel):

    def get_optionpanel(self, parent):
        """
        Return tool option widgets on given parent
        """
        # print 'get_optionpanel',self
        #button = wx.Button(parent, wx.NewId(), self.name+' Options')
        #button.Bind(wx.EVT_BUTTON, self.set_options)
        # return button

        self.optionspanel = ObjPanel(parent, self, groups=['options'],
                                     standartbuttons=[],
                                     immediate_apply=True,
                                     panelstyle='instrumental')
        return self.optionspanel

    def set_options(self, event):
        """
        Called from options panel.
        """
        print 'set_options', self.ident
        print '  event=', event
        pass

    def set_statusbar(self, key, info):
        pass

    def activate_metacanvas(self, metacanvas):
        """
        This call by metacanvas signals that the tool has been
        activated and can now interact with metacanvas.
        """
        # print 'activate_metacanvas',self.ident
        self.metacanvas = metacanvas
        self.metacanvas.del_handles()
        self.activate()

    def get_metacanvas(self):
        return self.metacanvas

    # def get_pentable(self):
    #    if self.metacanvas:
    #        return self.metacanvas.get_pentable()

    def deactivate_metacanvas(self):
        """
        This call by metacanvas signals that the tool has been
        deactivated and can now interact with metacanvas.
        """

        self.deactivate()
        self.optionspanel = None
        self.metacanvas = None


class DelTool(BaseTool):
    def __init__(self, parent, mainframe=None):
        """
        To be overridden by specific tool.
        """
        self.init_common('delete', parent, 'Delete', mainframe, info='Delete objects in cancvas')

    def set_button_info(self, bsize=(32, 32)):
        # print 'set_button_info select tool'
        self._bitmap = None
        self._bitmap_sel = None

    def get_button(self, parent, bottonsize=(32, 32), bottonborder=10):

        # simple stockbuttons
        b = wx.Button(parent, wx.ID_DELETE, name=self.get_name())

        b.SetSize(bottonsize)
        # b.SetBestSize()
        tt = wx.ToolTip(self.get_info())
        b.SetToolTip(tt)  # .SetTip(tool.tooltip)
        # print 'DelTool.get_button',dir(b)
        return b


class ToolPalett(wx.Panel):
    """
    This is a panel where tools are represented by images and/or text.
    The tools are selected in a radio-button-fashion.

    Each tool has a string as key. Each time the status changes,
    a callback function is called with new and old tool key as argument.
    """

    def __init__(self, parent, tools=[], callback=None, n_buttoncolumns=4):
        """
        callback is a function that is called when a tool has been selected.
        The function is called as:
            callback(tool)

        """
        # the metacanvas object with which the pallet should apply th tools
        self._callback = callback

        # wx.Window.__init__(self,parent,wx.ID_ANY,wx.DefaultPosition,wx.DefaultSize,wx.SUNKEN_BORDER|wx.WANTS_CHARS)
        # wx.Panel.__init__(self,parent,wx.ID_ANY,wx.DefaultPosition,size,wx.RAISED_BORDER|wx.WANTS_CHARS)
        wx.Panel.__init__(self, parent, -1, wx.DefaultPosition, wx.DefaultSize)
        # wx.Panel.__init__(self,parent,wx.ID_ANY,wx.DefaultPosition,(300,600),wx.RAISED_BORDER|wx.WANTS_CHARS)
        self.sizer = wx.GridSizer(0, n_buttoncolumns, 5, 5)
        self.SetSizer(self.sizer)
        self._id_to_tool = {}
        self._id = -1

        for tool in tools:
            self.add_tool(tool)

        self.sizer.Fit(self)
        # self.SetMaxSize((300,300))

    def add_tool(self, tool):
        """
        Add a tool to the pallet. 
        """
        bottonsize = (32, 32)
        bottonborder = 10
        toolbarborder = 1

        if tool is None:
            self.sizer.Add()
            return None
        else:
            b = tool.get_button(self, bottonsize=bottonsize, bottonborder=bottonborder)
            self.Bind(wx.EVT_BUTTON, self.on_select, b)

            self._id_to_tool[b.GetId()] = (tool, b)

            #self.sizer.Add(b, 0, wx.GROW)
            self.sizer.Add(b, 0, wx.EXPAND, border=toolbarborder)
            # self.sizer.Add(b)

            return id

    def add_tool_old(self, tool):
        """
        Add a tool to the pallet. 
        """
        bottonsize = (32, 32)
        bottonborder = 10
        toolbarborder = 1

        if tool is None:
            self.sizer.Add()
            return None
        else:
            id = wx.NewId()
            bitmap = tool.get_buttonbitmap()

            # print '\n add_tool',key,bitmap
            # print 'toolpallet.add_tool: key,name:',tool.key,tool.name
            #b=GenBitmapTextToggleButton(self, id, bitmap,tool.name,name = tool.name)
            b = GenBitmapToggleButton(self, id, bitmap, (bitmap.GetWidth() +
                                                         bottonborder, bitmap.GetHeight()+bottonborder))
            #b=GenBitmapToggleButton(self, wx.ID_DELETE)
            #b = GenBitmapTextToggleButton(self, id, None, tool.get('name',''), size = (200, 45))
            self.Bind(wx.EVT_BUTTON, self.on_select, b)

            if bitmap:
                #mask = wx.Mask(bitmap, wx.BLUE)
                # bitmap.SetMask(mask)
                b.SetBitmapLabel(bitmap)
                # bmp=wx.NullBitmap

            bitmap_sel = tool.get_buttonbitmap_sel()
            if bitmap_sel:
                #mask = wx.Mask(bmp, wx.BLUE)
                # bmp.SetMask(mask)
                b.SetBitmapSelected(bitmap_sel)

            b.SetUseFocusIndicator(False)
            # b.SetSize((36,140))
            # b.SetBestSize()
            tt = wx.ToolTip(tool.get_info())
            b.SetToolTip(tt)  # .SetTip(tool.tooltip)

            self._id_to_tool[id] = (tool, b)

            #self.sizer.Add(b, 0, wx.GROW)

            self.sizer.Add(b, 0, wx.EXPAND, border=toolbarborder)
            # self.sizer.Add(b)

            return id

    def get_tools(self):
        """
        Returns lins with all toll instances
        """
        tools = []
        for (tool, b) in self._id_to_tool.values():
            tools.append(tool)
        return tools

    def refresh(self):
        """
        Reorganizes toolpallet after adding/removing tools.
        Attention is not automatically called.
        """
        self.sizer.Layout()

    def on_select(self, event):

        _id = event.GetEventObject().GetId()
        print '\n on_select', _id, self._id  # ,self._id_to_tool[_id]
        if _id != self._id:
            if self._id_to_tool.has_key(_id):

                (tool, button) = self._id_to_tool[_id]
                print '  new tool', tool.get_name()
                self.unselect()
                self._id = _id
                self.GetParent().set_options(tool)
                if self._callback is not None:
                    self._callback(tool)

    def unselect(self):
        """
        Unselect currently selected tool.
        """
        if self._id_to_tool.has_key(self._id):
            (tool, button) = self._id_to_tool[self._id]
            if hasattr(button, 'SetToggle'):
                button.SetToggle(False)
            else:
                # button.SetFocus()
                # print 'button.SetFocus',button.SetFocus.__doc__
                pass

    def select(self, id):
        """
        Select explicitelt a tool.
        """
        print '\n select', id, self._id, self._id_to_tool

        if id != self._id:
            if self._id_to_tool.has_key(id):
                # unselect previous
                self.unselect()

                # select and activate new tool
                (tool, button) = self._id_to_tool[id]
                button.SetToggle(True)
                self._id = id
                if self._callback is not None:
                    self._callback(tool)


class __ToggleMixin:
    def SetToggle(self, flag):
        self.up = not flag
        self.Refresh()
    SetValue = SetToggle

    def GetToggle(self):
        return not self.up
    GetValue = GetToggle

    def OnLeftDown(self, event):
        if not self.IsEnabled():
            return
        self.saveUp = self.up
        self.up = False  # not self.up
        self.CaptureMouse()
        self.SetFocus()
        self.Refresh()

    def OnLeftUp(self, event):
        if not self.IsEnabled() or not self.HasCapture():
            return
        if self.HasCapture():
            if self.up != self.saveUp:
                self.Notify()
            self.ReleaseMouse()
            self.Refresh()

    def OnKeyDown(self, event):
        event.Skip()


class GenBitmapTextToggleButton(__ToggleMixin, GenBitmapTextButton):
    """A generic toggle bitmap button with text label"""
    pass


class GenBitmapToggleButton(__ToggleMixin, GenBitmapButton):
    """A generic toggle bitmap button with text label"""
    pass


class ToolsPanel(wx.Panel):
    """

    Interactively navigates through objects and displays attributes 
    on a panel.
    """

    def __init__(self, parent):
        wx.Panel.__init__(self, parent, -1, wx.DefaultPosition, wx.DefaultSize)

        sizer = wx.BoxSizer(wx.VERTICAL)

        self._toolspalett = ToolPalett(self)

        # self._toolspalett.add_tool(BaseTool(self))

        # create initial option panel
        self._optionspanel = wx.Window(self)
        self._optionspanel.SetBackgroundColour("pink")
        wx.StaticText(self._optionspanel, -1, "Tool Options", (300, -1))

        sizer.Add(self._toolspalett, 0, wx.ALL | wx.ALIGN_LEFT | wx.GROW, 4)
        sizer.Add(self._optionspanel, 1, wx.GROW)

        # finish panel setup
        self.SetSizer(sizer)
        sizer.Fit(self)

        # self.SetSize(parent.GetSize())

    def add_tool(self, tool):
        self._toolspalett.add_tool(tool)

    def set_options(self, tool):
        #self._optionspanel.change_obj(tool,groupnames = ['options'])
        # self._optionspanel.change_obj(tool)
        sizer = self.GetSizer()
        sizer.Remove(1)
        self._optionspanel.Destroy()

        self._optionspanel = objpanel.ObjPanel(self, obj=tool,
                                               attrconfigs=None,
                                               #tables = None,
                                               # table = None, id=None, ids=None,
                                               groupnames=None,
                                               func_change_obj=None,
                                               show_groupnames=False, show_title=True, is_modal=False,
                                               mainframe=None,
                                               pos=wx.DefaultPosition, size=wx.DefaultSize, style=wx.MAXIMIZE_BOX | wx.RESIZE_BORDER,
                                               immediate_apply=False, panelstyle='default',
                                               standartbuttons=['apply', 'restore'])
        # if id is not None:
        #    self.objpanel=ObjPanel(self,obj,id=id,func_change_obj=self.change_obj)
        # else:
        #    self.objpanel=ObjPanel(self,obj,func_change_obj=self.change_obj)
        sizer.Add(self._optionspanel, 1, wx.GROW)

        self.Refresh()
        # sizer.Fit(self)
        sizer.Layout()


def get_dist_point_to_segs(p, y1, x1, y2, x2, is_ending=True):
    """
    Minimum Distance between a Point p = (x,y) and a Line segments ,
    where vectors x1, y1 are the first  points and x2,y2 are the second points 
    of the line segments.
    Inspired by the description by Paul Bourke,    October 1988
    http://paulbourke.net/geometry/pointlineplane/

    Rewritten in vectorial form by Joerg Schweizer
    """

    y3, x3 = p

    d = np.zeros(len(y1), dtype=np.float32)

    dx21 = (x2-x1)
    dy21 = (y2-y1)

    lensq21 = dx21*dx21 + dy21*dy21

    # indexvector for all zero length lines
    iz = (lensq21 == 0)

    dy = y3-y1[iz]
    dx = x3-x1[iz]

    d[iz] = dx*dx + dy*dy

    lensq21[iz] = 1.0  # replace zeros with 1.0 to avoid div by zero error

    u = (x3-x1)*dx21 + (y3-y1)*dy21
    u = u / lensq21

    x = x1 + u * dx21
    y = y1 + u * dy21

    if is_ending:
        ie = u < 0
        x[ie] = x1[ie]
        y[ie] = y1[ie]
        ie = u > 1
        x[ie] = x2[ie]
        y[ie] = y2[ie]

    dx30 = x3-x
    dy30 = y3-y
    d[~iz] = (dx30*dx30 + dy30*dy30)[~iz]
    return d


def is_inside_triangles(p, x1, y1, x2, y2, x3, y3):
    """
    Returns a binary vector with True if point p is 
    inside a triangle.
    x1,y1,x2,y2,x3,y3 are vectors with the 3 coordiantes of the triangles.
    """
    alpha = ((y2 - y3)*(p[0] - x3) + (x3 - x2)*(p[1] - y3)) \
        / ((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3))

    beta = ((y3 - y1)*(p[0] - x3) + (x1 - x3)*(p[1] - y3)) \
        / ((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3))

    gamma = 1.0 - alpha - beta
    return (alpha > 0) & (beta > 0) & (gamma > 0)


class WxGLTest_orig(glcanvas.GLCanvas):
    def __init__(self, parent):

        glcanvas.GLCanvas.__init__(self, parent, -1, attribList=[glcanvas.WX_GL_DOUBLEBUFFER])
        wx.EVT_PAINT(self, self.OnDraw)
        wx.EVT_SIZE(self, self.OnSize)
        wx.EVT_MOTION(self, self.OnMouseMotion)
        wx.EVT_WINDOW_DESTROY(self, self.OnDestroy)

        self.init = True

    def OnDraw(self, event):
        self.SetCurrent()

        if not self.init:
            self.InitGL()
            self.init = False

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glLoadIdentity()

        # Draw the spiral in 'immediate mode'
        # WARNING: You should not be doing the spiral calculation inside the loop
        # even if you are using glBegin/glEnd, sin/cos are fairly expensive functions
        # I've left it here as is to make the code simpler.
        radius = 1.0
        x = radius*math.sin(0)
        y = radius*math.cos(0)
        glColor(0.0, 1.0, 0.0)
        glBegin(GL_LINE_STRIP)
        for deg in xrange(1000):
            glVertex(x, y, 0.0)
            rad = math.radians(deg)
            radius -= 0.001
            x = radius*math.sin(rad)
            y = radius*math.cos(rad)
        glEnd()

        glEnableClientState(GL_VERTEX_ARRAY)

        spiral_array = []

        # Second Spiral using "array immediate mode" (i.e. Vertex Arrays)
        radius = 0.8
        x = radius*math.sin(0)
        y = radius*math.cos(0)
        glColor(1.0, 0.0, 0.0)
        for deg in xrange(820):
            spiral_array.append([x, y])
            rad = math.radians(deg)
            radius -= 0.001
            x = radius*math.sin(rad)
            y = radius*math.cos(rad)

        glVertexPointerf(spiral_array)
        glDrawArrays(GL_LINE_STRIP, 0, len(spiral_array))
        glFlush()
        self.SwapBuffers()
        return

    def InitGL(self):
        '''
        Initialize GL
        '''

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

    def OnSize(self, event):

        try:
            width, height = event.GetSize()
        except:
            width = event.GetSize().width
            height = event.GetSize().height

        self.Refresh()
        self.Update()

    def OnMouseMotion(self, event):
        x = event.GetX()
        y = event.GetY()

    def OnDestroy(self, event):
        print "Destroying Window"


class Lines:
    """Lines class."""

    def __init__(self, linewidth=3, vertices=None, colors=None):
        self.name = 'Lines'
        self.n_vert_per_elem = 2
        self.linewidth = linewidth
        self.c_highl = 0.3
        self.detectwidth = 0.1  # m
        self.set_attrs(vertices, colors)

    def set_attrs(self, vertices, colors):

        self.vertices = np.array(vertices, dtype=np.float32)
        self._update_vertexvbo()

        self.colors = np.array(colors, dtype=np.float32)
        self.colors_highl = np.zeros((len(colors), 4), dtype=np.float32)
        self._update_colorvbo()

    def _update_vertexvbo(self):
        self._vertexvbo = vbo.VBO(self.vertices.reshape((-1, 3)))
        self._indexvbo = vbo.VBO(np.arange(self.n_vert_per_elem*len(self.vertices),
                                           dtype=np.int32), target=GL_ELEMENT_ARRAY_BUFFER)

    def _update_colorvbo(self):
        #self._colorvbo = vbo.VBO( np.resize( np.repeat(np.clip((self.colors+self.c_highl*self.colors_highl) ,0.0,1.0), self.n_vert_per_elem),(len(self.colors),4)) )
        self._colorvbo = vbo.VBO(np.clip((self.colors+self.colors_highl)
                                         [np.array(np.arange(0, len(self.colors), 1.0/self.n_vert_per_elem), int)], 0.0, 1.0))

    def pick(self, p):
        """
        Returns a binary vector which is True values for lines that have been selected 
        by point p.

        In particular, an element of this vector is True if the minimum distance 
        between the respective line to point p is less than self.detectwidth
        """
        x1 = self.vertices[:, 0, 0]
        y1 = self.vertices[:, 0, 1]

        x2 = self.vertices[:, 1, 0]
        y2 = self.vertices[:, 1, 1]

        return get_dist_point_to_segs(p, x1, y1, x2, y2, is_ending=True) < self.detectwidth**2

    def highlight(self, inds_highl):
        self.colors_highl = np.repeat(self.c_highl*np.array(inds_highl, dtype=np.float32), 4).reshape(-1, 4)
        self._update_colorvbo()

    def draw(self):
        glLineWidth(self.linewidth)

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        glEnable(GL_BLEND)

        glEnableClientState(GL_VERTEX_ARRAY)
        glEnableClientState(GL_COLOR_ARRAY)

        self._colorvbo.bind()
        glColorPointer(4, GL_FLOAT, 0, None)

        self._vertexvbo.bind()
        self._indexvbo.bind()
        glVertexPointer(3, GL_FLOAT, 0, None)

        glDrawElements(GL_LINES, self.n_vert_per_elem*len(self.vertices), GL_UNSIGNED_INT, None)

        glDisableClientState(GL_VERTEX_ARRAY)
        glDisableClientState(GL_COLOR_ARRAY)
        self._vertexvbo.unbind()
        self._indexvbo.unbind()
        self._colorvbo.unbind()


class Rectangles(Lines):

    def __init__(self, linewidth=3, vertices=None, colors=None):
        self.name = 'Rectangles'
        self.n_vert_per_elem = 4
        self.c_highl = 0.3
        self.detectwidth = 0.1  # m
        self.linewidth = linewidth

        self.set_attrs(vertices, colors)

    def pick(self, p):

        x1 = self.vertices[:, 0, 0]
        y1 = self.vertices[:, 0, 1]

        x2 = self.vertices[:, 1, 0]
        y2 = self.vertices[:, 1, 1]

        x3 = self.vertices[:, 2, 0]
        y3 = self.vertices[:, 2, 1]

        x4 = self.vertices[:, 3, 0]
        y4 = self.vertices[:, 3, 1]

        return is_inside_triangles(p, x1, y1, x2, y2, x3, y3) | is_inside_triangles(p, x1, y1, x3, y3, x4, y4)

    def draw(self):
        glLineWidth(self.linewidth)

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        glEnable(GL_BLEND)

        glEnableClientState(GL_VERTEX_ARRAY)
        glEnableClientState(GL_COLOR_ARRAY)

        self._colorvbo.bind()
        glColorPointer(4, GL_FLOAT, 0, None)

        self._vertexvbo.bind()
        self._indexvbo.bind()
        glVertexPointer(3, GL_FLOAT, 0, None)

        glDrawElements(GL_QUADS, self.n_vert_per_elem*len(self.vertices), GL_UNSIGNED_INT, None)

        glDisableClientState(GL_VERTEX_ARRAY)
        glDisableClientState(GL_COLOR_ARRAY)
        self._vertexvbo.unbind()
        self._indexvbo.unbind()
        self._colorvbo.unbind()


class Triangles(Lines):
    """Triangles class."""

    def __init__(self, linewidth=3, vertices=None, colors=None):
        self.name = 'Triangles'  # ,self.__name__
        self.n_vert_per_elem = 3
        self.c_highl = 0.3
        self.detectwidth = 0.1  # m
        self.linewidth = linewidth

        self.set_attrs(vertices, colors)

    def pick(self, p):
        return is_inside_triangles(p, self.vertices[:, 0, 0], self.vertices[:, 0, 1], self.vertices[:, 1, 0], self.vertices[:, 1, 1], self.vertices[:, 2, 0], self.vertices[:, 2, 1])

    def draw(self):
        glLineWidth(self.linewidth)

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        glEnable(GL_BLEND)

        glEnableClientState(GL_VERTEX_ARRAY)
        glEnableClientState(GL_COLOR_ARRAY)

        self._colorvbo.bind()
        glColorPointer(4, GL_FLOAT, 0, None)

        self._vertexvbo.bind()
        self._indexvbo.bind()
        glVertexPointer(3, GL_FLOAT, 0, None)

        glDrawElements(GL_TRIANGLES, self.n_vert_per_elem*len(self.vertices), GL_UNSIGNED_INT, None)

        glDisableClientState(GL_VERTEX_ARRAY)
        glDisableClientState(GL_COLOR_ARRAY)
        self._vertexvbo.unbind()
        self._indexvbo.unbind()
        self._colorvbo.unbind()


class GLFrame(wx.Frame):
    """A simple class for using OpenGL with wxPython."""

    def __init__(self, parent, id=-1, title='', pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=wx.DEFAULT_FRAME_STYLE,
                 name='frame', mainframe=None):

        print '\n\nGLFrame!!'
        if mainframe is None:
            self._mainframe = parent
        else:
            self._mainframe = mainframe

        self._elements = []
        self.elements_selected = []

        self.eyex = 0.0
        self.eyey = 0.0
        self.eyez = -9.0

        self.centerx = 0.0
        self.centery = 0.0
        self.centerz = 0.0

        self.upx = -1.0
        self.upy = 0.0
        self.upz = 0.0

        self.g_Width = 600
        self.g_Height = 600

        self.g_nearPlane = 1.
        self.g_farPlane = 1000.

        self.action = ""
        self.xStart = self.yStart = 0.
        self.xStart
        self.zoom = 65.

        self.xRotate = 0.
        self.yRotate = 0.
        self.zRotate = 0.

        self.xTrans = 0.
        self.yTrans = 0.

        #
        # Forcing a specific style on the window.
        #   Should this include styles passed?
        style = wx.DEFAULT_FRAME_STYLE | wx.NO_FULL_REPAINT_ON_RESIZE

        super(GLFrame, self).__init__(parent, id, title, pos, size, style, name)
        #wx.Frame.__init__(self, parent, id, title, pos, size, style, name)

        self.GLinitialized = False
        attribList = (glcanvas.WX_GL_RGBA,  # RGBA
                      glcanvas.WX_GL_DOUBLEBUFFER,  # Double Buffered
                      glcanvas.WX_GL_DEPTH_SIZE, 24)  # 24 bit

        #
        # Create the canvas
        self.canvas = glcanvas.GLCanvas(self, attribList=attribList)

        #
        # Set the event handlers.
        self.canvas.Bind(wx.EVT_ERASE_BACKGROUND, self.processEraseBackgroundEvent)
        self.canvas.Bind(wx.EVT_SIZE, self.processSizeEvent)
        self.canvas.Bind(wx.EVT_PAINT, self.processPaintEvent)

        self.canvas.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDown)
        self.canvas.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)
        self.canvas.Bind(wx.EVT_MOTION, self.OnMotion)
        self.canvas.Bind(wx.EVT_MOUSEWHEEL, self.OnWheel)

        self.Show()
        # this is needed to initialize GL projections for unproject
        wx.CallAfter(self.processSizeEvent)

    def add_element(self, element):
        self._elements.append(element)
        self.OnDraw()

    def resetView():
        self.zoom = 65.
        self.xRotate = 0.
        self.yRotate = 0.
        self.zRotate = 0.
        self.xTrans = 0.
        self.yTrans = 0.
        self.OnDraw()

    def OnWheel(self, event):
        #EventType = FloatCanvas.EVT_FC_MOUSEWHEEL
        #
        Rot = event.GetWheelRotation()
        # print 'OnWheel!!',Rot,event.ControlDown(),event.ShiftDown()
        if event.ControlDown():  # event.ControlDown(): # zoom
            if Rot < 0:
                self.zoom *= 0.9
            else:
                self.zoom *= 1.1
            self.OnDraw()
            event.Skip()

    def OnLeftDown(self, event):
        ##
        if (event.ControlDown() & event.ShiftDown()) & (self.action == ''):
            self.action = 'drag'
            self.BeginGrap(event)
            event.Skip()

    def OnLeftUp(self, event):
        if self.action == 'drag':
            self.EndGrap(event)
            self.action == ''
            event.Skip()

    def get_intersection(self, v_near, v_far):
        # 150918
        # idea from http://www.bfilipek.com/2012/06/select-mouse-opengl.html
        # https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
        d = -v_near + v_far

        t = -v_near[2]/d[2]
        v_inter = v_near+t*d

        return v_inter

    def OnMotion(self, event):

        p = self.unproject(event)[0:2]
        for element in self._elements:
            inds_pick = element.pick(p)
            element.highlight(inds_pick)

        self.OnDraw()

        if (event.ControlDown() & event.ShiftDown() & (self.action == 'drag')):
            self.MoveGrap(event)
            self.OnDraw()
            event.Skip()

        elif (self.action == 'drag'):
            self.EndGrap(event)
            self.action == ''
            event.Skip()

    def unproject(self, event):
        """Get the world coordinates for viewCoordinate for the event
        """
        mousex, mousey = event.GetPosition()
        x = mousex
        y = self.g_Height-mousey

        modelviewmatrix = glGetDoublev(GL_MODELVIEW_MATRIX)
        projectionmatrix = glGetDoublev(GL_PROJECTION_MATRIX)
        viewport = glGetInteger(GL_VIEWPORT)
        z = 0.0
        worldCoordinate_near = np.array(gluUnProject(
            x, y, z,
            modelviewmatrix,
            projectionmatrix,
            viewport,), dtype=np.float32)
        z = 1.0
        worldCoordinate_far = np.array(gluUnProject(
            x, y, z,
            modelviewmatrix,
            projectionmatrix,
            viewport,), dtype=np.float32)

        v_inter = self.get_intersection(worldCoordinate_near, worldCoordinate_far)
        return v_inter

    def BeginGrap(self, event):

        self.xStart, self.yStart = event.GetPosition()
        # print 'BeginGrap',self.xStart,self.yStart

    def MoveGrap(self, event):
        x, y = event.GetPosition()

        self.xTrans += x-self.xStart
        self.yTrans += y-self.yStart
        # print 'MoveGrap',self.xTrans,self.yTrans
        self.xStart, self.yStart = x, y

    def EndGrap(self, event):
        # print 'EndGrap'
        self.canvas.SetCursor(wx.NullCursor)
        self.action = ''

    #
    # Canvas Proxy Methods

    def GetGLExtents(self):
        """Get the extents of the OpenGL canvas."""
        return self.canvas.GetClientSize()

    def SwapBuffers(self):
        """Swap the OpenGL buffers."""
        self.canvas.SwapBuffers()

    #
    # wxPython Window Handlers

    def processEraseBackgroundEvent(self, event):
        """Process the erase background event."""
        pass  # Do nothing, to avoid flashing on MSWin

    def processSizeEvent(self, event=None):
        """Process the resize event."""
        if self.canvas.GetContext():
            # Make sure the frame is shown before calling SetCurrent.
            self.Show()
            self.canvas.SetCurrent()

            size = self.GetGLExtents()
            self.OnReshape(size.width, size.height)
            self.canvas.Refresh(False)
        if event:
            event.Skip()

    def processPaintEvent(self, event):
        """Process the drawing event."""
        self.canvas.SetCurrent()

        # This is a 'perfect' time to initialize OpenGL ... only if we need to
        if not self.GLinitialized:
            self.OnInitGL()
            self.GLinitialized = True

        self.OnDraw()
        event.Skip()

    #
    # GLFrame OpenGL Event Handlers

    def OnInitGL(self):
        """Initialize OpenGL for use in the window."""
        glClearColor(0, 0, 0, 1)

    def OnReshape(self, width, height):
        """Reshape the OpenGL viewport based on the dimensions of the window."""
        #global g_Width, g_Height
        self.g_Width = width
        self.g_Height = height
        glViewport(0, 0, self.g_Width, self.g_Height)

    def OnDraw(self, *args, **kwargs):
        """Draw the window."""
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

        # draw actual scene
        for element in self._elements:
            element.draw()

        self.SwapBuffers()

    def polarView(self):
        glTranslatef(self.yTrans/100., 0.0, 0.0)
        glTranslatef(0.0, -self.xTrans/100., 0.0)
        glRotatef(-self.zRotate, 0.0, 0.0, 1.0)
        glRotatef(-self.xRotate, 1.0, 0.0, 0.0)
        glRotatef(-self.yRotate, .0, 1.0, 0.0)


class WxGLTest2(glcanvas.GLCanvas):
    def __init__(self, parent, mainframe=None):
        if mainframe is None:
            self._mainframe = parent
        else:
            self._mainframe = mainframe

        self._elements = []
        self.elements_selected = []

        self.eyex = 0.0
        self.eyey = 0.0
        self.eyez = -9.0

        self.centerx = 0.0
        self.centery = 0.0
        self.centerz = 0.0

        self.upx = -1.0
        self.upy = 0.0
        self.upz = 0.0

        self.g_Width = 600
        self.g_Height = 600

        self.g_nearPlane = 1.
        self.g_farPlane = 1000.

        self.action = ""
        self.xStart = self.yStart = 0.
        self.xStart
        self.zoom = 65.

        self.xRotate = 0.
        self.yRotate = 0.
        self.zRotate = 0.

        self.xTrans = 0.
        self.yTrans = 0.

        #
        # Forcing a specific style on the window.
        #   Should this include styles passed?
        style = wx.DEFAULT_FRAME_STYLE | wx.NO_FULL_REPAINT_ON_RESIZE

        attribList = (glcanvas.WX_GL_RGBA,  # RGBA
                      glcanvas.WX_GL_DOUBLEBUFFER,  # Double Buffered
                      glcanvas.WX_GL_DEPTH_SIZE, 24)  # 24 bit

        glcanvas.GLCanvas.__init__(self, parent, -1, attribList=attribList)
        #super(WxGLTest2, self).__init__(parent,-1, attribList=attribList)

        self.GLinitialized = False

        ###
        # Forcing a specific style on the window.
        #   Should this include styles passed?
        #style = wx.DEFAULT_FRAME_STYLE | wx.NO_FULL_REPAINT_ON_RESIZE

        #super(GLFrame, self).__init__(parent, id, title, pos, size, style, name)
        #wx.Frame.__init__(self, parent, id, title, pos, size, style, name)

        #
        # Set the event handlers.
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.processEraseBackgroundEvent)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_PAINT, self.processPaintEvent)

        self.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDown)
        self.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)
        self.Bind(wx.EVT_MOTION, self.OnMotion)
        self.Bind(wx.EVT_MOUSEWHEEL, self.OnWheel)

        # this is needed to initialize GL projections for unproject
        wx.CallAfter(self.OnSize)

    def add_element(self, element):
        self._elements.append(element)
        self.OnDraw()

    def resetView():
        self.zoom = 65.
        self.xRotate = 0.
        self.yRotate = 0.
        self.zRotate = 0.
        self.xTrans = 0.
        self.yTrans = 0.
        self.OnDraw()

    def OnWheel(self, event):
        #EventType = FloatCanvas.EVT_FC_MOUSEWHEEL
        #
        Rot = event.GetWheelRotation()
        # print 'OnWheel!!',Rot,event.ControlDown(),event.ShiftDown()
        if event.ControlDown():  # event.ControlDown(): # zoom
            if Rot < 0:
                self.zoom *= 0.9
            else:
                self.zoom *= 1.1
            self.OnDraw()
            event.Skip()

    def OnLeftDown(self, event):
        ##
        if (event.ControlDown() & event.ShiftDown()) & (self.action == ''):
            self.action = 'drag'
            self.BeginGrap(event)
            event.Skip()

    def OnLeftUp(self, event):
        if self.action == 'drag':
            self.EndGrap(event)
            self.action == ''
            event.Skip()

    def get_intersection(self, v_near, v_far):
        # 150918
        # idea from http://www.bfilipek.com/2012/06/select-mouse-opengl.html
        # https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
        d = -v_near + v_far

        t = -v_near[2]/d[2]
        v_inter = v_near+t*d

        return v_inter

    def OnMotion(self, event):

        p = self.unproject(event)[0:2]
        for element in self._elements:
            inds_pick = element.pick(p)
            element.highlight(inds_pick)

        self.OnDraw()

        if (event.ControlDown() & event.ShiftDown() & (self.action == 'drag')):
            self.MoveGrap(event)
            self.OnDraw()
            event.Skip()

        elif (self.action == 'drag'):
            self.EndGrap(event)
            self.action == ''
            event.Skip()

    def unproject(self, event):
        """Get the world coordinates for viewCoordinate for the event
        """
        mousex, mousey = event.GetPosition()
        x = mousex
        y = self.g_Height-mousey

        modelviewmatrix = glGetDoublev(GL_MODELVIEW_MATRIX)
        projectionmatrix = glGetDoublev(GL_PROJECTION_MATRIX)
        viewport = glGetInteger(GL_VIEWPORT)
        z = 0.0
        worldCoordinate_near = np.array(gluUnProject(
            x, y, z,
            modelviewmatrix,
            projectionmatrix,
            viewport,), dtype=np.float32)
        z = 1.0
        worldCoordinate_far = np.array(gluUnProject(
            x, y, z,
            modelviewmatrix,
            projectionmatrix,
            viewport,), dtype=np.float32)

        v_inter = self.get_intersection(worldCoordinate_near, worldCoordinate_far)
        return v_inter

    def BeginGrap(self, event):

        self.xStart, self.yStart = event.GetPosition()
        # print 'BeginGrap',self.xStart,self.yStart

    def MoveGrap(self, event):
        x, y = event.GetPosition()

        self.xTrans += x-self.xStart
        self.yTrans += y-self.yStart
        # print 'MoveGrap',self.xTrans,self.yTrans
        self.xStart, self.yStart = x, y

    def EndGrap(self, event):
        # print 'EndGrap'
        self.SetCursor(wx.NullCursor)
        self.action = ''

    #
    # Canvas Proxy Methods

    def GetGLExtents(self):
        """Get the extents of the OpenGL canvas."""
        return self.GetClientSize()

    # def SwapBuffers(self):
    #    """Swap the OpenGL buffers."""
    #    self.SwapBuffers()

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
            self.SetCurrent()

            size = self.GetGLExtents()
            self.OnReshape(size.width, size.height)
            self.Refresh(False)
        if event:
            event.Skip()

    def processPaintEvent(self, event):
        """Process the drawing event."""
        self.SetCurrent()

        # This is a 'perfect' time to initialize OpenGL ... only if we need to
        if not self.GLinitialized:
            self.OnInitGL()
            self.GLinitialized = True

        self.OnDraw()
        event.Skip()

    #
    # GLFrame OpenGL Event Handlers

    def OnInitGL(self):
        """Initialize OpenGL for use in the window."""
        glClearColor(0, 0, 0, 1)

    def OnReshape(self, width, height):
        """Reshape the OpenGL viewport based on the dimensions of the window."""
        #global g_Width, g_Height
        self.g_Width = width
        self.g_Height = height
        glViewport(0, 0, self.g_Width, self.g_Height)

    def OnDraw(self, *args, **kwargs):
        """Draw the window."""
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

        # draw actual scene
        for element in self._elements:
            element.draw()
            # causes bad things :AttributeError: 'Implementation' object has no attribute 'glGenBuffers'

        self.SwapBuffers()

    def polarView(self):
        glTranslatef(self.yTrans/100., 0.0, 0.0)
        glTranslatef(0.0, -self.xTrans/100., 0.0)
        glRotatef(-self.zRotate, 0.0, 0.0, 1.0)
        glRotatef(-self.xRotate, 1.0, 0.0, 0.0)
        glRotatef(-self.yRotate, .0, 1.0, 0.0)


class GlEditorSash(wx.SplitterWindow):

    def __init__(self, parent,
                 mainframe=None,
                 size=wx.DefaultSize,
                 is_menu=False,  # create menu items
                 Debug=0,
                 ):

        wx.SplitterWindow.__init__(self, parent, wx.ID_ANY,
                                   style=wx.SP_LIVE_UPDATE,
                                   size=size)
        self.log = log

        self.Bind(wx.EVT_SPLITTER_SASH_POS_CHANGED, self.OnSashChanged)
        self.Bind(wx.EVT_SPLITTER_SASH_POS_CHANGING, self.OnSashChanging)

        # id=wx.ID_ANY
        # pixel_snap=10 # radius in pixels in which a point is selected
        # n_test=5
        # wx.Window.__init__(self,parent,id,wx.DefaultPosition,wx.DefaultSize,wx.SUNKEN_BORDER|wx.WANTS_CHARS)
        # wx.Panel.__init__(self,parent,id,wx.DefaultPosition,size,wx.SUNKEN_BORDER|wx.WANTS_CHARS)
        # self.parent=parent
        self._mainframe = mainframe  # mainframe
        #splitter = Splitter(self)

        self._toolspanel = ToolPalett(self)
        self._toolspanel.add_tool(BaseTool(self))
        #sty = wx.BORDER_SUNKEN
        #self._toolspanel = wx.Window(self, style=sty)
        # self._toolspanel.SetBackgroundColour("pink")
        #wx.StaticText(self._toolspanel, -1, "Object", (50,50))

        self._canvas = WxGLTest2(self)
        #p2 = wx.Window(self, style=sty)
        # p2.SetBackgroundColour("blue")
        #wx.StaticText(p2, -1, "GLeditor", (50,50))

        #self.canvas = wx.Window(splitter, style=sty)
        # self.canvas.SetBackgroundColour("green")
        #wx.StaticText(self.canvas, -1, "Panel two", (50,50))

        #self.canvas = WxGLTest2(splitter)
        #self.canvas = WxGLTest_orig(splitter)

        #nbpanel = wx.Panel(splitter)
        #self._viewtabs = wx.Notebook(nbpanel,wx.ID_ANY, style=wx.CLIP_CHILDREN)
        #sizer = wx.BoxSizer(wx.VERTICAL)
        #sizer.Add(self._viewtabs, 1, wx.ALL|wx.EXPAND, 5)
        # nbpanel.SetSizer(sizer)
        # self.Layout()

        # finally, put the notebook in a sizer for the panel to manage
        # the layout
        #sizer = wx.BoxSizer()
        #sizer.Add(self._viewtabs, 1, wx.EXPAND)
        # self.SetSizer(sizer)

        self.SetMinimumPaneSize(20)
        #splitter.SplitVertically(p1, self.canvas, -100)
        self.SplitVertically(self._toolspanel, self._canvas, -100)

        #wx.EVT_SIZE  (self, self.on_size)
        self.SetSashPosition(500, True)
        # sizer=wx.BoxSizer(wx.VERTICAL)
        # sizer.Add(p1,0, wx.ALL | wx.ALIGN_LEFT | wx.GROW, 4)# from NaviPanelTest
        # sizer.Add(self.canvas,1,wx.GROW)# from NaviPanelTest

        # finish panel setup
        # self.SetSizer(sizer)
        # sizer.Fit(self)
        # self.on_size()
        # self.Show()

    def get_canvas(self):
        return self._canvas

    def OnSashChanged(self, evt):
        #print("sash changed to %s\n" % str(evt.GetSashPosition()))
        pass

    def OnSashChanging(self, evt):
        print("sash changing to %s\n" % str(evt.GetSashPosition()))
        # uncomment this to not allow the change
        # evt.SetSashPosition(-1)
        # evt.SetSashPosition(210)
        # self.canvas.OnSize()
        pass

    def on_size(self, event=None):
        # self.tc.SetSize(self.GetSize())
        # self.tc.SetSize(self.GetSize())
        # self._viewtabs.SetSize(self.GetSize())
        # pass
        #wx.LayoutAlgorithm().LayoutWindow(self, self.p1)
        #wx.LayoutAlgorithm().LayoutWindow(self, self.p1)

        # important:
        #wx.LayoutAlgorithm().LayoutWindow(self, self._viewtabs)

        if event:
            event.Skip()


class GlEditor(wx.Panel):

    def __init__(self, parent,
                 mainframe=None,
                 size=wx.DefaultSize,
                 is_menu=False,  # create menu items
                 Debug=0,
                 ):

        wx.Panel.__init__(self, parent, wx.ID_ANY, size=size)
        sizer = wx.BoxSizer(wx.HORIZONTAL)

        self._mainframe = mainframe

        self._toolspanel = ToolsPanel(self)
        for i in range(5):
            self._toolspanel.add_tool(BaseTool(self))
        self._toolspanel.add_tool(DelTool(self))
        #sty = wx.BORDER_SUNKEN
        #self._toolspanel = wx.Window(self, style=sty)
        # self._toolspanel.SetBackgroundColour("pink")
        #wx.StaticText(self._toolspanel, -1, "Object", (50,50))

        self._canvas = WxGLTest2(self)
        #p2 = wx.Window(self, style=sty)
        # p2.SetBackgroundColour("blue")
        #wx.StaticText(p2, -1, "GLeditor", (50,50))

        sizer.Add(self._toolspanel, 0, wx.ALL | wx.ALIGN_LEFT | wx.GROW, 4)  # from NaviPanelTest
        sizer.Add(self._canvas, 1, wx.GROW)  # from NaviPanelTest

        # finish panel setup
        self.SetSizer(sizer)
        sizer.Fit(self)

    def get_canvas(self):
        return self._canvas

    def on_size(self, event=None):
        # self.tc.SetSize(self.GetSize())
        # self.tc.SetSize(self.GetSize())
        # self._viewtabs.SetSize(self.GetSize())
        # pass
        #wx.LayoutAlgorithm().LayoutWindow(self, self.p1)
        #wx.LayoutAlgorithm().LayoutWindow(self, self.p1)

        # important:
        #wx.LayoutAlgorithm().LayoutWindow(self, self._viewtabs)

        if event:
            event.Skip()


class MainSplitter(wx.SplitterWindow):
    def __init__(self, parent, ID=-1):
        wx.SplitterWindow.__init__(self, parent, ID,
                                   style=wx.SP_LIVE_UPDATE
                                   )

        self.SetMinimumPaneSize(20)

        #sty = wx.BORDER_SUNKEN

        emptyobj = cm.BaseObjman('empty')
        self._objbrowser = objpanel.NaviPanel(self,
                                              emptyobj,
                                              #show_title = False
                                              #size = w.DefaultSize,
                                              #style = wx.DEFAULT_DIALOG_STYLE|wx.MAXIMIZE_BOX|wx.RESIZE_BORDER,
                                              # choose_id=False,choose_attr=False,
                                              # func_choose_id=None,
                                              # func_change_obj=None,
                                              #panelstyle = 'default',
                                              immediate_apply=False,
                                              buttons=[],
                                              standartbuttons=[],
                                              #defaultbutton = defaultbutton,
                                              )

        #p1 = wx.Window(splitter, style=sty)
        # p1.SetBackgroundColour("pink")
        #wx.StaticText(p1, -1, "Object", (50,50))

        #self.canvas = wx.Window(splitter, style=sty)
        # self.canvas.SetBackgroundColour("green")
        #wx.StaticText(self.canvas, -1, "Panel two", (50,50))
        #self.canvas = WxGLTest2(splitter)
        #self.canvas = WxGLTest_orig(splitter)

        #self._viewtabs = wx.Notebook(self,wx.ID_ANY, style=wx.CLIP_CHILDREN)
        self._viewtabs = wx.Notebook(self, -1, size=(21, 21), style=wx.BK_DEFAULT
                                     # wx.BK_TOP
                                     # wx.BK_BOTTOM
                                     # wx.BK_LEFT
                                     # wx.BK_RIGHT
                                     # | wx.NB_MULTILINE
                                     )

        #nbpanel = wx.Panel(splitter)
        #self._viewtabs = wx.Notebook(nbpanel,wx.ID_ANY, style=wx.CLIP_CHILDREN)
        #sizer = wx.BoxSizer(wx.VERTICAL)
        #sizer.Add(self._viewtabs, 1, wx.ALL|wx.EXPAND, 5)
        # nbpanel.SetSizer(sizer)
        # self.Layout()

        # finally, put the notebook in a sizer for the panel to manage
        # the layout
        #sizer = wx.BoxSizer()
        #sizer.Add(self._viewtabs, 1, wx.EXPAND)
        # self.SetSizer(sizer)

        #splitter.SplitVertically(self._objbrowser,self.canvas , -100)
        self.SplitVertically(self._objbrowser, self._viewtabs, -100)

        self.SetSashPosition(500, True)
        self.Bind(wx.EVT_SPLITTER_SASH_POS_CHANGED, self.OnSashChanged)
        self.Bind(wx.EVT_SPLITTER_SASH_POS_CHANGING, self.OnSashChanging)

    def add_view(self, name, ViewClass, **args):
        """
        Add a new view to the notebook.
        """
        # print 'context.add_view',ViewClass
        # print '  args',args
        view = ViewClass(self._viewtabs,
                         mainframe=self.GetParent(),
                         **args
                         )

        # Add network tab with editor
        p = self._viewtabs.AddPage(view, name.title())
        #self._views[name] = view
        # self._viewtabs.SetSelection(p)
        # self._viewtabs.Show(True)
        return view

    def OnSashChanged(self, evt):
        #print("sash changed to %s\n" % str(evt.GetSashPosition()))
        pass

    def OnSashChanging(self, evt):
        #print("sash changing to %s\n" % str(evt.GetSashPosition()))
        # uncomment this to not allow the change
        # evt.SetSashPosition(-1)
        # self.canvas.OnSize()
        pass


class TestMainframe(AgileToolbarFrameMixin, wx.Frame):
    """
    Simple wx frame with some special features.
    """

    def __init__(self, parent=None,  id=-1, title='mainframe', pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=wx.DEFAULT_FRAME_STYLE,
                 name='frame'):

        # Forcing a specific style on the window.
        #   Should this include styles passed?
        style = wx.DEFAULT_FRAME_STYLE | wx.NO_FULL_REPAINT_ON_RESIZE
        wx.Frame.__init__(self, parent, id, title, pos, size=size, style=style, name=name)
        #super(GLFrame, self).__init__(parent, id, title, pos, size, style, name)
        self._splitter = MainSplitter(self)
        self._views = {}
        #wx.EVT_SIZE  (self, self.on_size)
        # sizer=wx.BoxSizer(wx.VERTICAL)
        # sizer.Add(p1,0, wx.ALL | wx.ALIGN_LEFT | wx.GROW, 4)# from NaviPanelTest
        # sizer.Add(self.canvas,1,wx.GROW)# from NaviPanelTest

        # finish panel setup
        # self.SetSizer(sizer)
        # sizer.Fit(self)
        # self.Show()

        # this is needed to initialize GL projections for unproject
        # wx.CallAfter(self.on_size)

        #width,height = self.GetSize()
        #self._splitter.SetSashPosition(300, True)
        # maximize the frame
        self.Maximize()
        # self.CenterOnScreen()
        #################################################################
        # create statusbar
        #self.statusbar = AgileStatusbar(self)
        self.statusbar = AgileStatusbar(self)
        self.SetStatusBar(self.statusbar)
        # self.count=0.0

        #################################################################
        # create toolbar

        tsize = (16, 16)
        self.init_toolbar(size=tsize)

        #new_bmp =  wx.ArtProvider.GetBitmap(wx.ART_NEW, wx.ART_TOOLBAR, tsize)
        #open_bmp = wx.ArtProvider.GetBitmap(wx.ART_FILE_OPEN, wx.ART_TOOLBAR, tsize)
        #save_bmp= wx.ArtProvider.GetBitmap(wx.ART_FILE_SAVE, wx.ART_TOOLBAR, tsize)
        #cut_bmp = wx.ArtProvider.GetBitmap(wx.ART_CUT, wx.ART_TOOLBAR, tsize)
        #copy_bmp = wx.ArtProvider.GetBitmap(wx.ART_COPY, wx.ART_TOOLBAR, tsize)
        #paste_bmp= wx.ArtProvider.GetBitmap(wx.ART_PASTE, wx.ART_TOOLBAR, tsize)

        #self.add_tool('new',self.on_open,new_bmp,'create new doc')
        #self.add_tool('open',self.on_open,open_bmp,'Open doc')
        #self.add_tool('save',self.on_save,save_bmp,'Save doc')
        # self.toolbar.AddSeparator()
        # self.add_tool('cut',self.on_open,cut_bmp,'Cut')
        # self.add_tool('copy',self.on_open,copy_bmp,'Copy')
        # self.add_tool('paste',self.on_open,paste_bmp,'Paste')

        # self.SetToolBar(self.toolbar)

        #################################################################
        # create the menu bar

        self.menubar = AgileMenubar(self)
        self.menubar.append_menu('file')
        self.menubar.append_menu('file/doc')

        self.menubar.append_item('file/doc/open', self.on_open,
                                 shortkey='Ctrl+o', info='open it out')

        self.menubar.append_item('file/doc/save', self.on_save,
                                 shortkey='Ctrl+s', info='save it out')

        # self.menubar.append_menu('edit')
        # self.menubar.append_item('edit/cut',self.cut,\
        #                    shortkey='Ctrl+c',info='cut it out')

        # self.menubar.append_item('edit/toggle',self.toggle_tools,\
        #                    shortkey='Ctrl+t',info='toggle tools')
        # self.menubar.append_menu('tools')
        self.SetMenuBar(self.menubar)
        # self.Show(True) #NO!!

    def add_view(self, name, ViewClass, **args):
        """
        Add a new view to the notebook.
        """
        # print 'context.add_view',ViewClass
        # print '  args',args

        view = self._splitter.add_view(name, ViewClass, **args)
        self._views[name] = view
        # self._viewtabs.SetSelection(p)
        # self._splitter._viewtabs.Show(True)
        return view

    def on_size(self, event=None):
        print 'Mainframe.on_size'
        # self.tc.SetSize(self.GetSize())
        # self.tc.SetSize(self.GetSize())
        # self._viewtabs.SetSize(self.GetSize())
        # pass
        #wx.LayoutAlgorithm().LayoutWindow(self, self.p1)
        #wx.LayoutAlgorithm().LayoutWindow(self, self.p1)

        # important:
        #wx.LayoutAlgorithm().LayoutWindow(self, self._viewtabs)
        wx.LayoutAlgorithm().LayoutWindow(self, self._splitter)
        if event:
            event.Skip()

    def on_save(self, event):
        print 'save it!!'

    def on_open(self, event):
        """Open a document"""
        #wildcards = CreateWildCards() + "All files (*.*)|*.*"
        print 'open it!!'

    def destroy(self):
        """Destroy this object"""
        # self.theDocManager.theDestructor()
        #imgPreferences.saveXml(self.GetStartDirectory() + "/" + imgINI_FILE_NAME)
        ##del self.thePrint
        self.Destroy()

    def on_close(self, event):
        # self.Close(True)
        # pass
        self.destroy()

    def on_exit(self, event):
        """Called when the application is to be finished"""
        self.destroy()

    def on_idle(self, event):
        pass
        #self.count = self.count + 1
        # if self.count >= 100:
        #    self.count = 0

        # self.statusbar.set_progress(self.count)

    def on_about(self, event):
        """Display the information about this application"""
        #dlg = imgDlgAbout(self, -1, "")
        # dlg.ShowModal()
        # dlg.Destroy()
        pass


linewidth = 3
vertices = [
    [[0.0, 0.0, 0.0], [0.2, 0.0, 0.0]],  # 0 green
    [[0.0, 0.0, 0.0], [0.0, 0.9, 0.0]],  # 1 red
]


colors = [
    [0.0, 0.9, 0.0, 0.9],    # 0
    [0.9, 0.0, 0.0, 0.9],    # 1
]
lines = Lines(linewidth=linewidth, vertices=vertices, colors=colors)

linewidth2 = 3
vertices2 = [
    [[0.5, 0.5, 0.0], [0.7, 0.5, 0.0], [0.7, 1.0, 0.0]],  # 0 green
    [[0.8, 0.5, 0.0], [0.9, 0.8, 0.0], [0.8, 0.2, 0.0]],  # 1 orange
]
colors2 = [
    [0.0, 0.9, 0.3, 0.9],    # 0
    [0.9, 0.3, 0.0, 0.9],    # 1
]
triangles = Triangles(linewidth=linewidth2, vertices=vertices2, colors=colors2)

linewidth3 = 3
vertices3 = [
    [[0.5, 0.0, 0.0], [0.7, 0.0, 0.0], [0.7, 0.3, 0.0], [0.5, 0.3, 0.0], ],  # 0
    [[0.1, 0.0, 0.0], [0.3, 0.0, 0.0], [0.3, 0.2, 0.0], [0.1, 0.2, 0.0], ],  # 1
]
colors3 = [
    [0.8, 0.0, 0.8, 0.9],    # 0
    [0.0, 0.6, 0.6, 0.9],    # 1
]
rectangles = Rectangles(linewidth=linewidth3, vertices=vertices3, colors=colors3)

if __name__ == '__main__':

    app = wx.PySimpleApp()

    if 1:

        frame = TestMainframe()
        gleditor = frame.add_view('GL Editor', GlEditor)
        #gleditor = frame._splitter.add_view('GL Editor',GlEditor)
        frame.Show()
        frame.on_size()
        canvas = gleditor.get_canvas()
        canvas.add_element(lines)
        canvas.add_element(triangles)
        canvas.add_element(rectangles)

    app.SetTopWindow(frame)
    app.MainLoop()

    app.Destroy()
