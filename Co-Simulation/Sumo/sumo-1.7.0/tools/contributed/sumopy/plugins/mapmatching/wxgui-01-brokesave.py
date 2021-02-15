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

# @file    wxgui-01-brokesave.py
# @author  Joerg Schweizer
# @date

import os
import wx
import numpy as np

from agilepy.lib_wx.modulegui import ModuleGui
from agilepy.lib_wx.ogleditor import *
from agilepy.lib_base.processes import Process
from agilepy.lib_wx.processdialog import ProcessDialog

from coremodules.network import routing
from coremodules.demand import demand

import mapmatching


class WxGui(ModuleGui):
    """Contains functions that communicate between the widgets of the main wx gui
    and the functions of the plugin.
    """

    def __init__(self, ident):
        self._mapmatching = None
        self._demand = None
        self._results = None
        self._init_common(ident,  priority=100001,
                          icondirpath=os.path.join(os.path.dirname(__file__), 'images'))

    def get_module(self):
        return self._mapmatching

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
        # print 'demand refresh_widgets',scenario.net
        is_refresh = False
        if self._demand != scenario.demand:
            del self._demand
            del self._mapmatching
            self._demand = scenario.demand
            self._mapmatching = mapmatching.Mapmatching('mapmatching', self._demand)
            #self._mapmatching = self._demand.add_demandobject(ident = 'mapmatching', DemandClass = mapmatching.Mapmatching)
            is_refresh = True

    def make_menu(self):
        menubar = self._mainframe.menubar
        menubar.append_menu('plugins/mapmatching',
                            bitmap=self.get_icon("icon_gps.png"),
                            )
        menubar.append_item('plugins/mapmatching/browse',
                            self.on_browse,  # common function in modulegui
                            info='View and browse mapmatching in object panel.',
                            bitmap=self.get_agileicon('icon_browse_24px.png'),  # ,
                            )

        menubar.append_menu('plugins/mapmatching/import',
                            bitmap=self.get_agileicon("Document_Import_24px.png"),
                            )

        menubar.append_item('plugins/mapmatching/import/European cycling challange...',
                            self.on_import_ecc,
                            info=self.on_import_ecc.__doc__.strip(),
                            bitmap=self.get_agileicon("Document_Import_24px.png"),
                            )

        menubar.append_item('plugins/mapmatching/project points',
                            self.on_project_points,
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_FILE_SAVE_AS, wx.ART_MENU),
                            )

        menubar.append_item('plugins/mapmatching/safe as...',
                            self.on_save_as,
                            info='Save all mapmatching  data in a new Python binary file.',
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_FILE_SAVE_AS, wx.ART_MENU),
                            )

        menubar.append_item('plugins/mapmatching/open...',
                            self.on_open,
                            info='Open previousely saved mapmatching data from a Python binary file.',
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_FILE_OPEN, wx.ART_MENU),
                            )

    def on_import_ecc(self, event=None):
        """
        Import and filter data from a European cycling challange. 
        """
        p = mapmatching.EccTracesImporter(self._mapmatching, logger=self._mainframe.get_logger())
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
            self._mainframe.browse_obj(self._mapmatching.trips)

    def on_project_points(self, event=None):
        self._mapmatching.points.project()
        self._mainframe.browse_obj(self._mapmatching.points)

        if event:
            event.Skip()

    def on_browse(self, event=None):

        self._mainframe.browse_obj(self._mapmatching)
        if event:
            event.Skip()

    def on_save_as(self, event=None):
        if self._mapmatching is None:
            return
        scenario = self.get_scenario()
        wildcards_all = "All files (*.*)|*.*"
        wildcards_obj = "Python binary result files (*.mmatch.obj)|*.mmatch.obj|Python binary files (*.obj)|*.obj"
        wildcards = wildcards_obj+"|"+wildcards_all

        # Finally, if the directory is changed in the process of getting files, this
        # dialog is set up to change the current working directory to the path chosen.
        dlg = wx.FileDialog(
            self._mainframe, message="Save mapmatching to file",
            defaultDir=scenario.get_workdirpath(),
            defaultFile=scenario.get_rootfilepath()+'.mmatch.obj',
            wildcard=wildcards,
            style=wx.SAVE | wx.CHANGE_DIR
        )
        val = dlg.ShowModal()
        # Show the dialog and retrieve the user response. If it is the OK response,
        # process the data.
        if val == wx.ID_OK:
            # This returns a Python list of files that were selected.
            filepath = dlg.GetPath()
            if len(filepath) > 0:
                # now set new filename and workdir
                self._mapmatching.save(filepath)

        # Destroy the dialog. Don't do this until you are done with it!
        # BAD things can happen otherwise!
        dlg.Destroy()

    def on_open(self, event=None):

        wildcards_all = "All files (*.*)|*.*"
        wildcards_obj = "Python binary mapmatching files (*.mmatch.obj)|*.mmatch.obj|Python binary files (*.obj)|*.obj"
        wildcards = wildcards_obj+"|"+wildcards_all

        # Finally, if the directory is changed in the process of getting files, this
        # dialog is set up to change the current working directory to the path chosen.
        dlg = wx.FileDialog(
            self._mainframe, message="Open mapmatching file",
            defaultDir=self.get_scenario().get_workdirpath(),
            #defaultFile = os.path.join(scenario.get_workdirpath(), scenario.format_ident()+'.obj'),
            wildcard=wildcards,
            style=wx.OPEN | wx.CHANGE_DIR
        )

        # Show the dialog and retrieve the user response. If it is the OK response,
        # process the data.
        is_new = False
        if dlg.ShowModal() == wx.ID_OK:
            # This returns a Python list of files that were selected.
            filepath = dlg.GetPath()
            if len(filepath) > 0:
                if self._mapmatching is not None:
                    # browse away from results
                    # self._mainframe.browse_obj(self._results.get_scenario())
                    del self._mapmatching

                self._mapmatching = mapmatching.load_mapmatching(filepath,
                                                                 self.get_scenario().demand,
                                                                 logger=self._mainframe.get_logger()
                                                                 )
                is_new = True

        # Destroy the dialog. Don't do this until you are done with it!
        # BAD things can happen otherwise!
        dlg.Destroy()

        if is_new:
            # this should update all widgets for the new scenario!!
            # print 'call self._mainframe.refresh_moduleguis()'
            self._mainframe.browse_obj(self._mapmatching)
            self._mainframe.select_view(name="Network")  # !!!!!!!!tricky, crashes without
            self.refresh_widgets()
