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

# @file    publictransportservices_wxgui.py
# @author  Joerg Schweizer
# @date

import wx
import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
from agilepy.lib_wx.ogleditor import *
from agilepy.lib_wx.objpanel import ObjPanel
from agilepy.lib_base.processes import Process
from agilepy.lib_wx.processdialog import ProcessDialog
from coremodules.network.network import SumoIdsConf, MODES
#import publictransportservices as pt


class PublicTransportWxGuiMixin:
    """Contains publictransport specific functions that communicate between the widgets of the main wx gui
    and the functions of the plugin.
    """

    def refresh_pt(self, is_refresh):
        if is_refresh:
            neteditor = self.get_neteditor()
            neteditor.add_toolclass(AddPtlineTool)

    def add_menu_pt(self, menubar):
        menubar.append_menu('demand/public transport',
                            bitmap=self.get_icon("fig_public_transport_24px.png"),
                            )

        # menubar.append_item( 'demand/turnflows/import turnflows...',
        #    self.on_import_turnflows,
        #    info='Import turnflows from file.',
        #    bitmap = self.get_agileicon("Document_Import_24px.png"),
        #    )

        menubar.append_item('demand/public transport/clear all line services',
                            self.on_clear_ptlines,
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_DELETE, wx.ART_MENU),
                            )

        menubar.append_item('demand/public transport/guess routes',
                            self.on_guess_routes,
                            info='Guess routes between public transport stops.',
                            bitmap=self.get_icon("route3_24px.png"),
                            )

        menubar.append_item('demand/public transport/build links',
                            self.on_build_links,
                            info='Build servive links of public transport network.',
                            bitmap=self.get_icon("route3_24px.png"),
                            )

    def on_build_links(self, event=None):
        self._demand.ptlines.ptlinks.get_value().build()
        self._mainframe.browse_obj(self._demand.ptlines.ptlinks.get_value())
        if event:
            event.Skip()

    def on_guess_routes(self, event=None):
        self._demand.ptlines.guess_routes()
        self._mainframe.browse_obj(self._demand.ptlines)
        if event:
            event.Skip()

    def on_clear_ptlines(self, event=None):
        """
        Clear all PT line services
        """
        self._demand.ptlines.clear()
        self._mainframe.browse_obj(self._demand.ptlines)
        if event:
            event.Skip()


class AddPtlineTool(SelectTool):
    """
    Public transport toolfor OGL canvas.
    """

    def __init__(self, parent, mainframe=None):
        """
        To be overridden by specific tool.
        """
        self.init_common('ptlineadder', parent, 'Add PT line',
                         info="""Tool to add public transport lines
                                1. Edit all parameters in the options panel.
                                2. Click on a sequence of public transport stops where the vehicle should stop. (Cycle overlapping stops with <SHIFT>+<LEFT MOUSE>, <LEFT MOUSE> to set final)
                                3. Optionally, click on a sequence of connected edges where the PT vehicle should run. (Cycle overlapping edges with <SHIFT>+<LEFT MOUSE>, <LEFT MOUSE> to set final)
                                4. Click Add Line to add the PT line.
                                The routing (Step 3) can be done automatically later. 
                                """,
                         is_textbutton=False,
                         )

        self._init_select(is_show_selected=False)

        # self.add_options_common()
        # make options

        self.add(cm.AttrConf('linename', default='',
                             groupnames=['options'],
                             perm='rw',
                             name='Line name',
                             info='This is the official name or number of the line. Note that the same line may have several line services for different service periods.',
                             ))

        self.add(cm.AttrConf('id_vtype', 0,
                             groupnames=['options'],
                             choices={'None': 0},
                             name='Veh. type ID',
                             info='Vehicle type used to derve this line.',
                             #xmltag = 'type',
                             ))

        self.add(cm.AttrConf('hour_offset', 0,
                             groupnames=['options'],
                             name='Offset hours',
                             unit='h',
                             perm='rw',
                             info='Hour of the day when service starts. This time will be added to the begin time, which is fo fine tuning.',
                             ))

        self.add(cm.AttrConf('time_begin', 0,
                             groupnames=['options'],
                             name='Begin time',
                             unit='s',
                             perm='rw',
                             info='Time when service begins.',
                             ))

        self.add(cm.AttrConf('duration', 0,
                             groupnames=['options'],
                             name='Duration',
                             unit='s',
                             perm='rw',
                             info='Time duration in seconds.',
                             ))

        # self.add(cm.AttrConf(  'time_end', 0,
        #                        groupnames = ['options'],
        #                        name = 'End time',
        #                        perm='rw',
        #                        unit = 's',
        #                        info = 'Time when service ends.',
        #                        ))

        self.add(cm.AttrConf('period', 0,
                             groupnames=['options'],
                             name='Interval',
                             perm='rw',
                             unit='s',
                             info='Time interval between consecutive vehicles.',
                             ))

        self.add(cm.AttrConf('time_dwell', 20,
                             groupnames=['options'],
                             perm='rw',
                             name='Dwell time',
                             untit='s',
                             info='Dwell time in a stop while passengers are boarding/alighting.'
                             ))

        self.add(cm.AttrConf('ids_stop', [],
                             perm='r',
                             #choices =  net.ptstops.stopnames.get_indexmap(),
                             groupnames=['options'],
                             name='PT stop IDs',
                             info='Sequence of IDs od stops or stations of a public transort line.',
                             ))

        self.add(cm.AttrConf('ids_edge', [],
                             perm='r',
                             groupnames=['options'],
                             name='Edge IDs',
                             info='Sequence of edge IDs constituting this public transport line.',
                             xmltag='edges',
                             ))

    def set_button_info(self, bsize=(32, 32)):
        # print 'set_button_info select tool'  self.get_icon("icon_sumo_24px.png")
        iconpath = os.path.join(os.path.dirname(__file__), 'images')
        self._bitmap = wx.Bitmap(os.path.join(iconpath, 'fig_public_transport_32px.png'), wx.BITMAP_TYPE_PNG)
        self._bitmap_sel = self._bitmap

    def set_cursor(self):
        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        if self._canvas is not None:
            self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_RIGHT_ARROW))

    def activate(self, canvas=None):
        """
        This call by metacanvas??TooldsPallet signals that the tool has been
        activated and can now interact with metacanvas.
        """
        # print 'activate'
        SelectTool.activate(self, canvas)

        # make lanes invisible, because they disturb
        lanedraws = self.get_drawobj_by_ident('lanedraws')
        self._is_lane_visible_before = None
        if lanedraws:
            self._is_lane_visible_before = lanedraws.is_visible()
            lanedraws.set_visible(False)

        vtypechoices = self.get_scenario().demand.vtypes.ids_sumo.get_indexmap()
        # print '  vtypechoices',vtypechoices
        self.id_vtype.set_value(vtypechoices.get('bus', 0))
        self.id_vtype.choices = vtypechoices
        canvas.draw()

    def deactivate(self):
        """
        This call by metacanvas signals that the tool has been
        deactivated and can now interact with metacanvas.
        """

        self._is_active = False
        # self.unhighlight()

        # reset lane visibility
        if self._is_lane_visible_before is not None:
            lanedraws = self.get_drawobj_by_ident('lanedraws')
            if lanedraws:
                lanedraws.set_visible(self._is_lane_visible_before)
                self._canvas.draw()

        self.deactivate_select()

    def on_left_down_select(self, event):
        # same as on select tool but avoid deselection after execution

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
                # attention: on_execute_selection must take care of selected
                # objects in list with self.unselect_all()
                #is_draw |= self.unselect_all()

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

    def on_execute_selection(self, event):
        """
        Definively execute operation on currently selected drawobjects.
        """
        # print 'AddTurnflowTool.on_execute_selection',self.get_netelement_current(),len(self)
        # self.set_objbrowser()
        # self.highlight_current()
        self.unhighlight_current()
        # self.unhighlight()
        netelement_current = self.get_netelement_current()
        if netelement_current is not None:
            (elem, id_elem) = netelement_current
            if elem.get_ident() == 'edges':
                # print '  check',self.id_fromedge.get_value()
                self.ids_edge.get_value().append(id_elem)

                # add potential to-edges to selection
                #edgedraws = self.get_drawobj_by_ident('edgedraws')
                # self.unselect_all()
                # for id_edge in edges.get_outgoing(self.id_fromedge.value):
                #    self.add_selection(edgedraws, id_edge)

                self.unselect_all()  # includes unhighlight
                # self.highlight()
                self.parent.refresh_optionspanel(self)

            if elem.get_ident() == 'ptstops':
                # print '  check',self.id_fromedge.get_value()
                self.ids_stop.get_value().append(id_elem)
                self.unselect_all()
                self.parent.refresh_optionspanel(self)
            else:
                self.unselect_all()

        return True

    # def highlight(self):
    #    edges = self.get_edges()
    #    drawing = self.get_drawing()
    #    if self.id_fromedge.value>=0:
    #        drawing.highlight_element(edges, self.id_fromedge.value, is_update = True)
    #    if self.id_toedge.value>=0:
    #        drawing.highlight_element(edges, self.id_toedge.value, is_update = True)

    # def unhighlight(self):
    #    edges = self.get_edges()
    #    drawing = self.get_drawing()
    #    if self.id_fromedge.value>=0:
    #        drawing.unhighlight_element(edges, self.id_fromedge.value, is_update = True)
    #    if self.id_toedge.value>=0:
    #        drawing.unhighlight_element(edges, self.id_toedge.value, is_update = True)

    def on_change_selection(self, event):
        """
        Called after selection has been changed with SHIFT-click
        Do operation on currently selected drawobjects.
        """
        # self.set_objbrowser()
        # self.parent.refresh_optionspanel(self)
        return False

    def get_netelement_current(self):
        mainframe = self.parent.get_mainframe()
        if mainframe is not None:
            drawobj, _id = self.get_current_selection()
            if drawobj is not None:
                obj = drawobj.get_netelement()
                return obj, _id
            else:
                return None
        else:
            return None

    def get_scenario(self):
        # get net and scenario via netdrawing
        return self.get_drawing().get_net().parent

    def get_ptlines(self):
        return self.get_scenario().demand.ptlines

    def get_edges(self):
        return self.get_scenario().net.edges

    def on_add_line(self, event=None):
        self._optionspanel.apply()
        edges = self.get_edges().ids_sumo
        ptlines = self.get_ptlines()
        id_line = ptlines.make(linename=self.linename.value,
                               time_begin=int(self.hour_offset.value*3600) + self.time_begin.value,
                               time_end=int(self.hour_offset.value*3600) + self.time_begin.value + self.duration.value,
                               period=self.period.value,
                               time_dwell=self.time_dwell.value,
                               ids_stop=self.ids_stop.value,
                               ids_edge=self.ids_edge.value,
                               id_vtype=self.id_vtype.value,
                               )

        mainframe = self.parent.get_mainframe()
        if mainframe is not None:
            mainframe.browse_obj(ptlines)

        self.ids_stop.set_value([])
        self.ids_edge.set_value([])
        # self.highlight()
        self.parent.refresh_optionspanel(self)
        # self._canvas.draw()

    def on_clear_route(self, event=None):
        # self.unhighlight()
        self.ids_edge.set_value([])  # set empty
        self.unselect_all()

        self.parent.refresh_optionspanel(self)
        self._canvas.draw()

    def on_clear_stops(self, event=None):
        # self.unhighlight()
        self.ids_stop.set_value([])  # set empty
        self.unselect_all()

        self.parent.refresh_optionspanel(self)
        self._canvas.draw()

    def get_optionspanel(self, parent, size=wx.DefaultSize):
        """
        Return tool option widgets on given parent
        """
        size = (200, -1)
        buttons = [('Add line', self.on_add_line, 'Add public transport line.'),
                   ('Clear route', self.on_clear_route, 'Clear edges (the route) of public transport service.'),
                   ('Clear stops', self.on_clear_stops, 'Clear stopsof public transport service.'),
                   ]
        defaultbuttontext = 'Add line'
        self._optionspanel = ObjPanel(parent, obj=self,
                                      attrconfigs=None,
                                      groupnames=['options'],
                                      func_change_obj=None,
                                      show_groupnames=False, show_title=True, is_modal=False,
                                      mainframe=self.parent.get_mainframe(),
                                      pos=wx.DefaultPosition, size=size, style=wx.MAXIMIZE_BOX | wx.RESIZE_BORDER,
                                      immediate_apply=True, panelstyle='default',  # 'instrumental'
                                      buttons=buttons, defaultbutton=defaultbuttontext,
                                      standartbuttons=[],  # standartbuttons=['restore']
                                      )

        return self._optionspanel
