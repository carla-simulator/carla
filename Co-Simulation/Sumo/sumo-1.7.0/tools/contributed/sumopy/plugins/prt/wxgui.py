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

# @file    wxgui.py
# @author  Joerg Schweizer
# @date

import os
import wx
import numpy as np

from agilepy.lib_wx.modulegui import ModuleGui
from agilepy.lib_wx.ogleditor import *
from agilepy.lib_base.processes import Process
from agilepy.lib_wx.processdialog import ProcessDialog, ProcessDialogInteractive

from coremodules.network import routing
from coremodules.demand import demand
from coremodules.simulation import sumo, results
import prt

try:
    import results_mpl as results_mpl
    is_mpl = True  # we have matplotlib support
except:
    print "WARNING: python matplotlib package not installed, no matplotlib plots."
    is_mpl = False


class AddPrtCompressorTool(SelectTool):
    """
    Public transport toolfor OGL canvas.
    """

    def __init__(self, parent, mainframe=None):
        """
        To be overridden by specific tool.
        """
        self.init_common('prtcompressoradder', parent, 'Add PRT compressor/decompressor',
                         info="""Tool to add a PRT compressor/decompressor
                                1. Select compressor/decompressor and edit parameters in the options panel.
                                2. Click on a sequence of PRT edges that become shunt-lines of the compressor/decompressor. (Cycle overlapping edges with <SHIFT>+<LEFT MOUSE>, <LEFT MOUSE> to set final)
                                3. Click the Add button to add the compressor/decompressor.
                                """,
                         is_textbutton=False,
                         )

        self._init_select(is_show_selected=False)

        tooltypechoices = ['Add compressor', 'Add decompressor']
        self.add(cm.AttrConf('tooltype', tooltypechoices[0],
                             groupnames=['options'],
                             choices=tooltypechoices,
                             perm='rw',
                             name='Tool type',
                             info="""Type of tool/action used.""",
                             ))

        # self.add_options_common()
        # make options
        self.add(cm.AttrConf('ids_edge', [],
                             perm='r',
                             groupnames=['options'],
                             name='Shunt edge IDs',
                             info='Sequence of edge IDs used as shunt lines.',
                             xmltag='edges',
                             ))

    def set_button_info(self, bsize=(32, 32)):
        # print 'set_button_info select tool'  self.get_icon("icon_sumo_24px.png")
        iconpath = os.path.join(os.path.dirname(__file__), 'images')
        self._bitmap = wx.Bitmap(os.path.join(iconpath, 'icon_compressor.png'), wx.BITMAP_TYPE_PNG)
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

        #vtypechoices = self.get_scenario().demand.vtypes.ids_sumo.get_indexmap()
        # print '  vtypechoices',vtypechoices
        # self.id_vtype.set_value(vtypechoices.get('bus',0))
        #self.id_vtype.choices = vtypechoices
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

                # self.unselect_all()# includes unhighlight
                # self.highlight()
                self.parent.refresh_optionspanel(self)

            else:
                self.unselect_all()

        return True

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

    def get_edges(self):
        return self.get_scenario().net.edges

    def on_add_compressor(self, event=None):
        self._optionspanel.apply()
        #edges = self.get_edges().ids_sumo

        #['Add compressor','Add decompressor']
        mainframe = self.parent.get_mainframe()
        if self.tooltype.get_value() == 'Add compressor':
            compressors = self.get_scenario().simulation.prtservice.compressors
            id_line = compressors.make(ids_shuntedge=self.ids_edge.get_value())
            if mainframe is not None:
                mainframe.browse_obj(compressors)
        else:
            decompressors = self.get_scenario().simulation.prtservice.decompressors
            id_line = decompressors.make(ids_shuntedge=self.ids_edge.get_value())
            if mainframe is not None:
                mainframe.browse_obj(decompressors)

        # self.ids_stop.set_value([])
        self.ids_edge.set_value([])
        # self.highlight()
        self.parent.refresh_optionspanel(self)
        # self._canvas.draw()

    def on_clear_edges(self, event=None):
        # self.unhighlight()
        self.ids_edge.set_value([])  # set empty
        self.unselect_all()

        self.parent.refresh_optionspanel(self)
        self._canvas.draw()

    def get_optionspanel(self, parent, size=wx.DefaultSize):
        """
        Return tool option widgets on given parent
        """
        size = (200, -1)
        buttons = [('Add', self.on_add_compressor, 'Add PRT compressor or decompressor.'),
                   ('Clear edges', self.on_clear_edges, 'Clear shunt edges from selection.'),
                   ]
        defaultbuttontext = 'Add'
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


class ResultDialog(ProcessDialog):
    def _get_buttons(self):
        buttons = [('Plot and close',   self.on_run,      'Plot in matplotlib window and close this window thereafter.'),
                   ('Plot',   self.on_show,      'Plot in matplotlib window.'),
                   ]
        defaultbutton = 'Plot and close'
        standartbuttons = ['cancel', ]

        return buttons, defaultbutton, standartbuttons

    def on_show(self, event):
        self.process.show()


class WxGui(ModuleGui):
    """Contains functions that communicate between PRT plugin and the widgets of the main wx gui
    and the functions of the plugin.
    """

    def __init__(self, ident):
        self._prtservice = None
        self._demand = None
        self._simulation = None
        self._init_common(ident,  priority=100002,
                          icondirpath=os.path.join(os.path.dirname(__file__), 'images'))

    def get_module(self):
        return self._prtservice

    def get_scenario(self):
        return self._mainframe.get_modulegui('coremodules.scenario').get_scenario()

    def get_neteditor(self):
        return self._mainframe.get_modulegui('coremodules.network').get_neteditor()

    def init_widgets(self, mainframe):
        """
        Set mainframe and initialize widgets to various places.
        """
        self._mainframe = mainframe
        #self._neteditor = mainframe.add_view("Network", Neteditor)

        # mainframe.browse_obj(self._module)
        self.make_menu()
        self.make_toolbar()

    def refresh_widgets(self):
        """
        Check through mainframe what the state of the application is
        and reset widgets. For exampe enable/disable widgets
        dependent on the availability of data. 
        """
        scenario = self.get_scenario()
        print 'prtgui.refresh_widgets', self._simulation != scenario.simulation

        is_refresh = False
        if self._simulation != scenario.simulation:
            del self._demand
            del self._prtservice
            del self._simulation
            self._demand = scenario.demand
            self._simulation = scenario.simulation
            self._prtservice = self._simulation.add_simobject(ident='prtservice', SimClass=prt.PrtService)
            is_refresh = True
            neteditor = self.get_neteditor()
            neteditor.get_toolbox().add_toolclass(AddPrtCompressorTool)
            # if (self._prtservice is not None)&(self._simulation is not None):
            # print ' self._simulation.results,self._prtservice._results:', self._simulation.results,self._prtservice.get_results(),id(self._simulation.results), id(self._prtservice.get_results())
            # print '  self._simulation.results',id(self._simulation.results)

    def make_menu(self):
        menubar = self._mainframe.menubar
        menubar.append_menu('plugins/prt', bitmap=self.get_icon('icon_prt.png'),)
        if sumo.traci is not None:
            menubar.append_item('plugins/prt/browse',
                                self.on_browse_obj,  # common function in modulegui
                                info='View and browse PRT in object panel.',
                                bitmap=self.get_agileicon('icon_browse_24px.png'),  # ,
                                )

            menubar.append_item('plugins/prt/make stops',
                                self.on_make_stops,
                                info='Make PRT stops from PT stops with PRT access ("custom1").',
                                # bitmap = self.get_icon('icon_sumo.png'),#,
                                )
            menubar.append_item('plugins/prt/update compressors',
                                self.on_update_compressors,
                                # bitmap = self.get_icon('icon_sumo.png'),#,
                                )
            menubar.append_item('plugins/prt/clear compressors',
                                self.on_clear_compressors,
                                bitmap=wx.ArtProvider.GetBitmap(wx.ART_DELETE, wx.ART_MENU),
                                )
            menubar.append_item('plugins/prt/update decompressors',
                                self.on_update_decompressors,
                                # bitmap = self.get_icon('icon_sumo.png'),#,
                                )
            menubar.append_item('plugins/prt/clear decompressors',
                                self.on_clear_decompressors,
                                bitmap=wx.ArtProvider.GetBitmap(wx.ART_DELETE, wx.ART_MENU),
                                )

            menubar.append_item('plugins/prt/make merge nodes',
                                self.on_make_merges,
                                info='Make PRT merge nodes.',
                                # bitmap = self.get_icon('icon_sumo.png'),#,
                                )
            menubar.append_item('plugins/prt/calculate stop to stop times',
                                self.on_make_times_stop_to_stop,
                                # bitmap = self.get_icon('icon_sumo.png'),#,
                                )

            menubar.append_item('plugins/prt/add vehicles...',
                                self.on_add_vehicles,
                                info='Add PRT vehicles to network.',
                                # bitmap = self.get_icon('icon_sumo.png'),#,
                                )

            menubar.append_item('plugins/prt/clear vehicles.',
                                self.on_clear_vehicles,
                                info='Clear all PRT vehicles from network.',
                                bitmap=wx.ArtProvider.GetBitmap(wx.ART_DELETE, wx.ART_MENU),
                                )

            menubar.append_menu('plugins/prt/results',
                                bitmap=self.get_icon('icon_results_24px.png'),  # ,
                                )
            if is_mpl:
                menubar.append_item('plugins/prt/results/stopresults with matplotlib',
                                    self.on_mpl_stopresults,
                                    bitmap=self.get_icon('icon_mpl.png'),  # ,
                                    )

    def on_make_stops(self, event=None):
        self._prtservice.prtstops.make_from_net()
        self._mainframe.browse_obj(self._prtservice.prtstops)

    def on_update_compressors(self, event=None):
        """Update all compressors (determine detector edge)"""
        self._prtservice.compressors.update_all()
        self._mainframe.browse_obj(self._prtservice.compressors)

    def on_clear_compressors(self, event=None):
        """Delete all compressors"""
        self._prtservice.compressors.clear()
        self._mainframe.browse_obj(self._prtservice.compressors)

    def on_update_decompressors(self, event=None):
        """Update all decompressors (determine detector edge)"""
        self._prtservice.decompressors.update_all()
        self._mainframe.browse_obj(self._prtservice.decompressors)

    def on_clear_decompressors(self, event=None):
        """Delete all decompressors"""
        self._prtservice.decompressors.clear()
        self._mainframe.browse_obj(self._prtservice.decompressors)

    def on_make_merges(self, event=None):
        self._prtservice.mergenodes.make_from_net()
        self._mainframe.browse_obj(self._prtservice.mergenodes)

    def on_make_times_stop_to_stop(self, event=None):
        """Determine stop to sto time matrix"""

        self._prtservice.make_times_stop_to_stop()
        self._mainframe.browse_obj(self._prtservice)

    def on_add_vehicles(self, event=None):
        p = prt.VehicleAdder(self._prtservice.prtvehicles, logger=self._mainframe.get_logger())
        dlg = ProcessDialog(self._mainframe, p, immediate_apply=True)

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
            self._mainframe.browse_obj(self._prtservice.prtvehicles)

    def on_clear_vehicles(self, event=None):
        self._prtservice.prtvehicles.clear()
        self._mainframe.browse_obj(self._prtservice.prtvehicles)

    def on_mpl_stopresults(self, event=None):
        print 'on_mpl_stopresults', id(self._simulation.results)  # ,id(self._prtservice.get_results())
        if self._prtservice is not None:
            if self._simulation is not None:
                resultplotter = results_mpl.StopresultsPlotter(self._simulation.results,  # self._prtservice.get_results(),
                                                               logger=self._mainframe.get_logger())
                dlg = ResultDialog(self._mainframe, resultplotter)

                dlg.CenterOnScreen()

                # this does not return until the dialog is closed.
                val = dlg.ShowModal()
                if dlg.get_status() != 'success':  # val == wx.ID_CANCEL:
                    dlg.Destroy()

                elif dlg.get_status() == 'success':
                    # apply current widget values to scenario instance
                    dlg.apply()
                    dlg.Destroy()
