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

# @file    wxgui-01.py
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


class WxGui(ModuleGui):
    """Contains functions that communicate between PRT plugin and the widgets of the main wx gui
    and the functions of the plugin.
    """

    def __init__(self, ident):
        self._prtservice = None
        self._demand = None
        self._results = None
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
        print 'prtgui.refresh_widgets', self._demand != scenario.demand
        is_refresh = False
        if self._demand != scenario.demand:
            del self._demand
            del self._prtservice
            self._demand = scenario.demand
            self._prtservice = self._demand.add_demandobject('prtservice', prt.PrtService)
            is_refresh = True
        #    #self.get_neteditor().get_toolbox().add_toolclass(AddZoneTool)
        #    #print '  odintervals',self._demand.odintervals#,self.odintervals.times_start
        #    #print ' ',dir(self._demand.odintervals)
        #
        #

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
            menubar.append_item('plugins/prt/add vehicles...',
                                self.on_add_vehicles,
                                info='Add PRT vehicles to network.',
                                # bitmap = self.get_icon('icon_sumo.png'),#,
                                )
            menubar.append_item('plugins/prt/clear vehicles.',
                                self.on_clear_vehicles,
                                info='Clear all PRT vehicles from network.',
                                # bitmap = self.get_icon('icon_sumo.png'),#,
                                )
            menubar.append_item('plugins/prt/make plans for PRT ...',
                                self.on_make_plans_prt,
                                info='Make mobility plans for the virtual population using PRT. The preferred mode must be "custom1".',
                                # bitmap = self.get_icon('icon_sumo.png'),#,
                                )
            menubar.append_item('plugins/prt/simulate with TRACI...',
                                self.on_prt_traci,
                                info='Define simulation parameters and simulate PRT with SUMO and interactive control via TRACI.',
                                bitmap=self.get_icon('icon_sumo.png'),  # ,
                                )

    def on_make_stops(self, event=None):
        self._prtservice.prtstops.make_from_net()
        self._mainframe.browse_obj(self._prtservice.prtstops)

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
        self._mainframe.browse_obj(self._prtservice)

    def on_make_plans_prt(self, event=None):
        self._prtservice.make_plans_prt()
        self._mainframe.browse_obj(self.get_scenario().demand.virtualpop.get_plans())

    def on_prt_traci(self, event=None):
        self.prepare_results()
        self.simulator = prt.SumoPrt(self.get_scenario(),
                                     prtservice=self._prtservice,
                                     results=self._results,
                                     logger=self._mainframe.get_logger(),
                                     is_gui=True,
                                     is_export_net=True,
                                     is_export_poly=True,
                                     is_export_rou=True,
                                     is_prompt_filepaths=False,
                                     is_quit_on_end=True,
                                     is_start=True,
                                     )
        self.open_sumodialog_interactive()

    def prepare_results(self, ident_results='simresults'):

        # TODO: here we should actually replace the current network
        # so we would need a clear net method in scenario
        # alternatively we could merge properly
        scenario = self.get_scenario()
        if self._results is None:
            self._results = results.Simresults(ident=ident_results,
                                               scenario=scenario)
        else:
            if self._results.parent != scenario:
                # uups scenario changed
                del self._results
                self._results = results.Simresults(ident=ident_results, scenario=scenario)

        return self._results

    def open_sumodialog_interactive(self):
        dlg = ProcessDialogInteractive(self._mainframe,
                                       self.simulator,
                                       title='SUMO-PRT Dialog',
                                       func_close=self.close_sumodialog_interactive,
                                       )

        dlg.CenterOnScreen()

        # this does not return until the dialog is closed.
        #val = dlg.ShowModal()
        print 'open_sumodialog_interactive'
        dlg.Show()
        dlg.MakeModal(True)
        # print '  val,val == wx.ID_OK',val,wx.ID_OK,wx.ID_CANCEL,val == wx.ID_CANCEL
        # print '  status =',dlg.get_status()
        # print 'returned to main window self.simulator.status',self.simulator.status

    def close_sumodialog_interactive(self, dlg):
        # called before destroying the dialog
        if self.simulator.status == 'success':
            self.simulator.import_results()

            self._mainframe.browse_obj(self._results)
            self._mainframe.select_view(name="Result viewer")  # !!!!!!!!tricky, crashes without
            self.refresh_widgets()
            # print 'call self._mainframe.refresh_moduleguis()'
            # self._mainframe.refresh_moduleguis()
