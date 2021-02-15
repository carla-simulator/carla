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

# @file    turnflows_wxgui.py
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
import turnflows


class TurnflowWxGuiMixin:
    """Contains OdFlow spacific functions that communicate between the widgets of the main wx gui
    and the functions of the plugin.
    """

    def refresh_turnflow(self, is_refresh):
        if is_refresh:
            neteditor = self.get_neteditor()
            neteditor.add_toolclass(AddTurnflowTool)

    def add_menu_turnflow(self, menubar):
        menubar.append_menu('demand/turnflows',
                            bitmap=self.get_icon("fig_turnflow_24px.png"),
                            )

        menubar.append_item('demand/turnflows/import turnflows...',
                            self.on_import_turnflows,
                            info='Import turnflows from file.',
                            bitmap=self.get_agileicon("Document_Import_24px.png"),
                            )

        menubar.append_item('demand/turnflows/normalize turn-probabilities',
                            self.on_normalize_turnprobabilities,
                            #info='Makes sure that sum of turn probabilities from an edge equals 1.',
                            #bitmap = self.get_icon("Document_Import_24px.png"),
                            )

        menubar.append_item('demand/turnflows/turnflows to routes',
                            self.on_turnflows_to_routes,
                            #info='Makes sure that sum of turn probabilities from an edge equals 1.',
                            #bitmap = self.get_icon("Document_Import_24px.png"),
                            )

        menubar.append_item('demand/turnflows/clear all turnflows',
                            self.on_clear_turnflows,
                            info='Clear all turnflows.',
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_DELETE, wx.ART_MENU),
                            )

    def on_import_turnflows(self, event=None):
        tfimporter = turnflows.TurnflowImporter(self._demand.turnflows,
                                                logger=self._mainframe.get_logger()
                                                )
        dlg = ProcessDialog(self._mainframe, tfimporter)

        dlg.CenterOnScreen()

        # this does not return until the dialog is closed.
        val = dlg.ShowModal()
        # print '  val,val == wx.ID_OK',val,wx.ID_OK,wx.ID_CANCEL,val == wx.ID_CANCEL
        # print '  status =',dlg.get_status()
        if dlg.get_status() != 'success':  # val == wx.ID_CANCEL:
            # print ">>>>>>>>>Unsuccessful\n"
            dlg.Destroy()

        if dlg.get_status() == 'success':
            # print ">>>>>>>>>successful\n"
            # apply current widget values to scenario instance
            dlg.apply()
            dlg.Destroy()
            self._mainframe.browse_obj(self._demand.turnflows)

    def on_normalize_turnprobabilities(self, event=None):
        """
        Makes sure that sum of turn probabilities from an edge equals 1.
        """
        self._demand.turnflows.normalize_turnprobabilities()
        self._mainframe.browse_obj(self._demand.turnflows)
        # if event:
        #    event.Skip()

    def on_clear_turnflows(self, event=None):
        """Generates routes, based on flow information and turnflow probabilities.
        This function will apply the JTROUTER for each transport mode separately.
        """
        self._demand.turnflows.clear_turnflows()
        self._mainframe.browse_obj(self._demand.turnflows)
        # if event:
        #    event.Skip()

    def on_turnflows_to_routes(self, event=None):
        """Generates routes, based on flow information and turnflow probabilities.
        This function will apply the JTROUTER for each transport mode separately.
        """
        self._demand.turnflows.turnflows_to_routes(is_clear_trips=False)
        self._mainframe.browse_obj(self._demand.trips)


class TurnflowCommonMixin:
    def add_options_common(self, turnflows=None):
        self.add(am.AttrConf('t_start', 0,
                             groupnames=['options'],
                             perm='rw',
                             name='Start time',
                             unit='s',
                             info='Start time of interval',
                             ))

        self.add(am.AttrConf('t_end', 3600,
                             groupnames=['options'],
                             perm='rw',
                             name='End time',
                             unit='s',
                             info='End time of interval',
                             ))

        if turnflows is not None:
            modechoices = turnflows.parent.vtypes.get_modechoices()
            id_mode = modechoices.get('passenger', 0)
        else:
            modechoices = {'': -1}
            id_mode = -1

        # here we ged classes not vehicle type
        # specific vehicle type within a class will be generated later
        self.add(am.AttrConf('id_mode',  id_mode,
                             groupnames=['options'],
                             choices=modechoices,
                             name='Mode',
                             info='Transport mode.',
                             ))


class AddTurnflowTool(TurnflowCommonMixin, SelectTool):
    """
    OD flow toolfor OGL canvas.
    """

    def __init__(self, parent, mainframe=None):
        """
        To be overridden by specific tool.
        """
        self.init_common('turnflowadder', parent, 'Add turn-flow',
                         info="""Tool to at generated flows and turn flows at intersections:
                                Select an edge with <LEFT CLICK> on the Network editor to specify a <From Edge>. 
                                Optionally enter the number of vehicles to be generated on <From Edge> and press button "Add flow".
                                Cycle through the possible adjacent edges by <SHIFT>+<LEFT MOUSE>.
                                <LEFT CLICK> when the desired <To Edge> is highlighted.  
                                Enter the number of vehicles turning from <From Edge> to <To edge> and press button "Add turns".
                                Continue cycling by <SHIFT>+<LEFT MOUSE> and adding more turnflows.
                                Or press "Clear" button to clear all edges and start defining flows for a new intersection.
                                """,
                         is_textbutton=False,
                         )

        self._init_select(is_show_selected=False)

        self.add_options_common()
        # make options

        self.add(cm.AttrConf('id_fromedge', -1,
                             groupnames=['options'],
                             name='From edge ID',
                             perm='r',
                             info='This is the reference edge for the generated flows as well as turn-flows into adjason edges.',
                             ))

        self.add(cm.AttrConf('flow_generated', 0,
                             groupnames=['options'],
                             perm='rw',
                             name='Gen. flow',
                             info='Absolute number of vehicles which are generated on "from edge" during the specified time interval.',
                             xmltag='number',
                             ))

        self.add(cm.AttrConf('id_toedge', -1,
                             groupnames=['options'],
                             name='To edge ID',
                             perm='r',
                             info='Target Edge ID for the specified turnflows. Click on edge to specify.',
                             ))

        self.add(cm.AttrConf('turnflow', 0,
                             groupnames=['options'],
                             perm='rw',
                             name='Turn flow',
                             info='Number of trips from edge of origin to edge of destination during the specified time interval. <SHIFT>+<LEFT MOUSE> to cycle through all possible edges. <LEFT MOUSE> to select.',
                             ))

    def set_button_info(self, bsize=(32, 32)):
        # print 'set_button_info select tool'  self.get_icon("icon_sumo_24px.png")
        iconpath = os.path.join(os.path.dirname(__file__), 'images')
        self._bitmap = wx.Bitmap(os.path.join(iconpath, 'fig_turnflow_32px.png'), wx.BITMAP_TYPE_PNG)
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

        modechoices = self.get_scenario().demand.vtypes.get_modechoices()
        self.id_mode.set_value(modechoices.get('passenger', 0))
        self.id_mode.choices = modechoices
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
            (edges, id_elem) = netelement_current
            if edges.get_ident() == 'edges':
                # print '  check',self.name_orig.get_value(),'*',self.name_orig.get_value() is ''
                if self.id_fromedge.get_value() < 0:
                    # print '    set name_orig',zones.ids_sumo[id_zone]
                    self.id_fromedge.set_value(id_elem)

                else:
                    # print '    set name_dest',zones.ids_sumo[id_zone]
                    self.id_toedge.set_value(id_elem)

                # add potential to-edges to selection
                edgedraws = self.get_drawobj_by_ident('edgedraws')
                self.unselect_all()
                for id_edge in edges.get_outgoing(self.id_fromedge.value):
                    self.add_selection(edgedraws, id_edge)

                # self.unselect_all()# includes unhighlight
                self.highlight()
                self.parent.refresh_optionspanel(self)

            else:
                self.unselect_all()

        return True

    def highlight(self):
        edges = self.get_edges()
        drawing = self.get_drawing()
        if self.id_fromedge.value >= 0:
            drawing.highlight_element(edges, self.id_fromedge.value, is_update=True)
        if self.id_toedge.value >= 0:
            drawing.highlight_element(edges, self.id_toedge.value, is_update=True)

    def unhighlight(self):
        edges = self.get_edges()
        drawing = self.get_drawing()
        if self.id_fromedge.value >= 0:
            drawing.unhighlight_element(edges, self.id_fromedge.value, is_update=True)
        if self.id_toedge.value >= 0:
            drawing.unhighlight_element(edges, self.id_toedge.value, is_update=True)

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

    def get_turnflows(self):
        return self.get_scenario().demand.turnflows

    def get_edges(self):
        return self.get_scenario().net.edges

    def on_add_flow(self, event=None):
        self._optionspanel.apply()
        edges = self.get_edges().ids_sumo

        flows = self.get_turnflows().add_flow(self.t_start.value, self.t_end.value,
                                              self.id_mode.value,
                                              self.id_fromedge.value,
                                              self.flow_generated.value)

        # self.unselect_all()
        # self.unhighlight()
        mainframe = self.parent.get_mainframe()
        if mainframe is not None:
            mainframe.browse_obj(flows)

        self.flow_generated.set_value(0)  # set flow to zero
        # self.highlight()
        self.parent.refresh_optionspanel(self)
        # self._canvas.draw()

    def on_add_turn(self, event=None):
        self._optionspanel.apply()
        edges = self.get_edges().ids_sumo

        turns = self.get_turnflows().add_turn(self.t_start.value, self.t_end.value,
                                              self.id_mode.value,
                                              self.id_fromedge.value,
                                              self.id_toedge.value,
                                              self.turnflow.value)

        # self.unselect_all()
        # self.unhighlight()

        mainframe = self.parent.get_mainframe()
        if mainframe is not None:
            mainframe.browse_obj(turns)

        self.turnflow.set_value(0)  # set flow to zero
        self.id_toedge.set_value(-1)
        self.unhighlight_current()
        # self.highlight()
        self.parent.refresh_optionspanel(self)

        # self.on_clear_edges()

    def on_clear_edges(self, event=None):
        self.unhighlight()
        self.id_fromedge.set_value(-1)  # set empty
        self.id_toedge.set_value(-1)  # set empty
        self.unselect_all()

        self.parent.refresh_optionspanel(self)
        self._canvas.draw()

    def get_optionspanel(self, parent, size=wx.DefaultSize):
        """
        Return tool option widgets on given parent
        """
        size = (200, -1)
        buttons = [('Add flow', self.on_add_flow, 'Add flow generation on from-edge to demand.'),
                   ('Add turns', self.on_add_turn, 'Add turn flow from from-edge to to-edge to demand.'),
                   ('Clear', self.on_clear_edges, 'Clear edge selection.'),
                   #('Save flows', self.on_add, 'Save OD flows to current demand.'),
                   #('Cancel', self.on_close, 'Close wizzard without adding flows.'),
                   ]
        defaultbuttontext = 'Add flow'
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
