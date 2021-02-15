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

# @file    toolbox.py
# @author  Joerg Schweizer
# @date

import sys
import os
import string
import time
if __name__ == '__main__':
    try:
        FILEDIR = os.path.dirname(os.path.abspath(__file__))
    except:
        FILEDIR = os.path.dirname(os.path.abspath(sys.argv[0]))
    sys.path.append(os.path.join(FILEDIR, "..", ".."))

IMAGEDIR = os.path.join(os.path.dirname(__file__), "images")

import wx
from wx.lib.buttons import GenBitmapTextButton, GenBitmapButton

from objpanel import ObjPanel, NaviPanel

import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am


class BaseTool(am.ArrayObjman):
    """
    This is a base tool class for Agilecanvas.
    It must handle all mouse or keyboard events,
    must create and draw helplines and finally
    modify the state of client which are grafically
    represented on the canvas.
    """

    def __init__(self, parent):
        """
        To be overridden by specific tool.
        """
        self.init_common('select', parent, 'Selection tool',
                         info='Select objects in cancvas',
                         is_textbutton=True,
                         )

    def set_button_info(self, bsize=(32, 32)):
        # print 'set_button_info select tool'
        self._bitmap = wx.Bitmap(os.path.join(IMAGEDIR, 'selectIcon.bmp'), wx.BITMAP_TYPE_BMP)
        self._bitmap_sel = wx.Bitmap(os.path.join(IMAGEDIR, 'selectIconSel.bmp'), wx.BITMAP_TYPE_BMP)

    def set_cursor(self):
        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        if self._canvas is not None:
            # self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_QUESTION_ARROW))
            pass

    def get_button(self, parent, bottonsize=(32, 32), bottonborder=10):
        """
        Returns button widget.
        Called  when toolbar is created.
        """
        # simple stockbuttons
        #b=wx.Button(parent, wx.ID_DELETE)

        id = wx.NewId()
        bitmap = self._bitmap

        if self._is_textbutton:
            b = GenBitmapTextToggleButton(parent, id, bitmap, self.ident.title(), name=self.get_name())
        else:
            b = GenBitmapToggleButton(parent, id, bitmap,
                                      (bitmap.GetWidth()+bottonborder, bitmap.GetHeight()+bottonborder),
                                      name=self.get_name())
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

    def init_common(self, ident, parent, name, info=None, is_textbutton=False):
        # print 'Agiletool.__init__',ident,name
        #self.name = name
        self._is_textbutton = is_textbutton
        self._canvas = None
        self._init_objman(ident, parent=parent, name=name.title(), info=info)
        #attrsman = self.set_attrsman(cm.Attrsman(self))
        self._is_active = False

        # print ' call set_button',self.ident
        self.set_button_info()
        self._optionspanel = None

    def get_optionspanel(self, parent, size=wx.DefaultSize):
        """
        Return tool option widgets on given parent
        """
        size = (200, -1)
        self._optionspanel = ObjPanel(parent, obj=self,
                                      attrconfigs=None,
                                      #tables = None,
                                      # table = None, id=None, ids=None,
                                      groupnames=['options'],
                                      func_change_obj=None,
                                      show_groupnames=False, show_title=True, is_modal=False,
                                      mainframe=self.parent.get_mainframe(),
                                      pos=wx.DefaultPosition, size=size, style=wx.MAXIMIZE_BOX | wx.RESIZE_BORDER,
                                      immediate_apply=False, panelstyle='default',  # 'instrumental'
                                      standartbuttons=['apply', 'restore'])

        return self._optionspanel

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

    def get_drawing(self):
        return self.parent.get_drawing()

    def get_drawobj_by_ident(self, ident):
        return self.get_drawing().get_drawobj_by_ident(ident)

    def deactivate(self):
        """
        This call by metacanvas??? ToolePallet signals that the tool has been
        deactivated and can now interact with metacanvas.
        """
        self._canvas.deactivate_tool()
        self._canvas = None
        self._is_active = False

    def is_active(self):
        return self._is_active

    def force_deactivation(self):
        """
        Explicit call to deactivate this tool in the tools panel.
        """
        self.parent.unselect_tool()

    def on_left_down(self, event):
        return False

    def on_left_up(self, event):
        return False

    def on_left_dclick(self, event):
        return False

    def on_right_down(self, event):
        return False

    def on_right_up(self, event):
        return self.aboard(event)

    def aboard(self):
        return False

    def on_wheel(self, event):
        return False

    def on_motion(self, event):
        return False  # return True if something moved


class DelTool(BaseTool):
    def __init__(self, parent):
        """
        To be overridden by specific tool.
        """
        self.init_common('delete', parent, 'Delete', info='Delete objects in cancvas')

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

    def __init__(self, parent, tools=[], callback=None, n_buttoncolumns=3):
        """
        callback is a function that is called when a tool has been selected.
        The function is called as:
            callback(tool)

        """
        # the metacanvas object with which the pallet should apply th tools

        # callback when a new tool gets selected (NOT in USE)
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

        # self.sizer.Fit(self)
        # self.SetMaxSize((300,-1))

    def has_tool(self, newtool):
        for tool, b in self._id_to_tool.values():
            if tool.get_ident() == newtool.get_ident():
                return True
        return False

    def get_tool_by_ident(self, ident):
        # print 'get_tool_by_ident',ident
        for tool, b in self._id_to_tool.values():
            # print '  tool',tool.get_ident()
            if tool.get_ident() == ident:
                return tool

        return None

    def add_tool(self, tool):
        """
        Add a tool to the pallet. 
        """
        if not self.has_tool(tool):
            # print 'add_tool',tool
            bottonsize = (32, 32)
            bottonborder = 10
            toolbarborder = 1

            b = tool.get_button(self, bottonsize=bottonsize, bottonborder=bottonborder)
            self.Bind(wx.EVT_BUTTON, self.on_select, b)

            _id = b.GetId()
            self._id_to_tool[_id] = (tool, b)

            #self.sizer.Add(b, 0, wx.GROW)
            self.sizer.Add(b, 0, wx.EXPAND, border=toolbarborder)
            # self.sizer.Add(b)
            # print ' _id =',_id
            return _id
        else:
            return -1

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
        """
        Called from a pressed button
        """
        _id = event.GetEventObject().GetId()
        # print '\n on_select',_id,self._id#,self._id_to_tool[_id]

        if _id != self._id:
            if self._id_to_tool.has_key(_id):

                (tool, button) = self._id_to_tool[_id]
                # print '  new tool',tool.get_name()
                self.unselect()
                self._id = _id

                # this will cause the main OGL editor to activate the
                # tool with the current canvas
                self.GetParent().set_tool(tool)
                # if self._callback is not None:
                #    self._callback(tool)
                event.Skip()
                return tool

        return None

    def select(self, _id):
        """
        Select explicitelt a tool with _id.
        """
        # print '\nselect',_id,self._id,self._id_to_tool

        if _id != self._id:
            if self._id_to_tool.has_key(_id):

                (tool, button) = self._id_to_tool[_id]

                # print '  explicitly press button'
                if hasattr(button, 'SetToggle'):
                    button.SetToggle(True)
                else:
                    button.SetFocus()
                    # print 'button.SetFocus',button.SetFocus.__doc__
                    # pass

                # print '  new tool',tool.get_name()
                # self.unselect()
                self._id = _id

                self.GetParent().set_tool(tool)
                # if self._callback is not None:
                #    self._callback(tool)
                return tool

        return None

    def unselect(self):
        """
        Unselect currently selected tool.
        """
        if self._id_to_tool.has_key(self._id):
            (tool, button) = self._id_to_tool[self._id]

            if tool.is_active() == True:
                # Disactivate current tool
                tool.deactivate()

                if hasattr(button, 'SetToggle'):
                    button.SetToggle(False)
                else:
                    # button.SetFocus()
                    # print 'button.SetFocus',button.SetFocus.__doc__
                    pass


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
    Shows a toolpallet with different tools and an options panel.
    """

    def __init__(self, parent, size=wx.DefaultSize, size_title=150,  **kwargs):

        #size = wx.DefaultSize
        #size = (300,-1)
        wx.Panel.__init__(self, parent, wx.NewId(), wx.DefaultPosition, size)
        # wx.DefaultSize
        # sizer=wx.BoxSizer(wx.VERTICAL)
        sizer = wx.StaticBoxSizer(wx.StaticBox(parent, wx.NewId(), "test"), wx.VERTICAL)

        self._toolspalett = ToolPalett(self, **kwargs)

        # self._toolspalett.add_tool(BaseTool(self))

        # create initial option panel
        self._optionspanel = wx.Window(self)
        self._optionspanel.SetBackgroundColour("pink")
        wx.StaticText(self._optionspanel, -1, "Tool Options", (size_title, -1))

        # OK, but toolspane changes size with optionpanel
        #sizer.Add(self._toolspalett,0, wx.ALL | wx.ALIGN_LEFT | wx.GROW, 4)
        # sizer.Add(self._optionspanel,1,wx.GROW)# wx.EXPAND

        sizer.Add(self._toolspalett, 0,  wx.EXPAND)
        sizer.Add(self._optionspanel, 1, wx.EXPAND)

        # finish panel setup
        self.SetSizer(sizer)
        sizer.Fit(self)

        # self.SetSize(parent.GetSize())
        # self.SetMaxSize((300,-1))

    def get_canvas(self):
        # ask the OGL editor for the currently active canvas in focus
        return self.GetParent().get_canvas()

    def get_drawing(self):
        return self.get_canvas().get_drawing()

    def get_mainframe(self):
        return self.GetParent().get_mainframe()

    def add_tool(self, tool):
        return self._toolspalett.add_tool(tool)

    def add_toolclass(self, ToolClass, **kwargs):
        # init and add
        return self._toolspalett.add_tool(ToolClass(self, **kwargs))

    def add_initial_tool(self, tool):
        self._id_initialtool = self.add_tool(tool)

    def reset_initial_tool(self):
        self.set_tool_with_id(self._id_initialtool)

    def reset_initial_tool(self):
        self.set_tool_with_id(self._id_initialtool)

    def set_tool_with_id(self, _id):
        """
        Explicitely set a tool from tool pallet using its id.
        Used to set initial tool.
        """
        # print 'set_tool_with_id',_id
        return self._toolspalett.select(_id)

    def set_tool(self, tool):
        """
        Called by toolpallet after new tool has been selected.
        """
        # Activate current tool
        # then tool wil set itself to canvas
        tool.activate(self.get_canvas())

        # set options of current tool
        self.refresh_optionspanel(tool)

    def get_tool_by_ident(self, ident):
        return self._toolspalett.get_tool_by_ident(ident)

    def refresh_optionspanel(self, tool):
        sizer = self.GetSizer()
        sizer.Remove(1)
        self._optionspanel.Destroy()

        self._optionspanel = tool.get_optionspanel(self)  # , size = self.GetSize())
        # self._optionspanel.SetSize((100,0))
        # if id is not None:
        #    self.objpanel=ObjPanel(self,obj,id=id,func_change_obj=self.change_obj)
        # else:
        #    self.objpanel=ObjPanel(self,obj,func_change_obj=self.change_obj)

        # ok, but chanes sice of whole palle
        # sizer.Add(self._optionspanel,1,wx.GROW)

        sizer.Add(self._optionspanel, 1, wx.EXPAND)

        # self.Refresh()
        # sizer.Fit(self)
        sizer.Layout()
        # self.GetParent().Layout()

    def unselect_tool(self):
        """
        Unselect currently selected tool.
        """
        self._toolspalett.unselect()
