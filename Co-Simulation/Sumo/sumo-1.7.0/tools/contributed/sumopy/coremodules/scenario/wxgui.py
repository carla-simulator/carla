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
import agilepy.lib_base.classman as cm
from coremodules.misc import shapeformat

from agilepy.lib_wx.modulegui import ModuleGui
from agilepy.lib_wx.processdialog import ProcessDialog

import scenario

# if 1: #try:
import networkxtools
#    IS_NETX = True
# else:#except:
#    print 'WARNING: Networkx support not available because no networkx package installed'
#    IS_NETX = False


class WxGui(ModuleGui):
    """Contains functions that communicate between the widgets of the main wx gui
    and the functions of the plugin.
    """

    def __init__(self, ident):
        self._scenario = None
        self._init_common(ident,  priority=1,
                          icondirpath=os.path.join(os.path.dirname(__file__), 'images'))

    def get_module(self):
        return self._scenario

    def get_scenario(self):
        return self._scenario

    def init_widgets(self, mainframe):
        """
        Set mainframe and initialize widgets to various places.
        """
        self._mainframe = mainframe
        #self._neteditor = mainframe.add_view("Network", Neteditor)

        # mainframe.browse_obj(self._module)
        self.make_menu()
        self.make_toolbar()
        args = mainframe.get_args()

        if len(args) == 3:
            # command line provided rootname and dirpath
            rootname = args[1]
            dirpath = args[2]
            name_scenario = rootname
            self._scenario = scenario.Scenario(rootname, workdirpath=dirpath,
                                               name_scenario=name_scenario,
                                               logger=self._mainframe.get_logger())
            self._scenario.import_xml()

        elif len(args) == 2:
            filepath = args[1]
            self._scenario = scenario.load_scenario(filepath, logger=self._mainframe.get_logger())
            #self._scenario = cm.load_obj(filepath)

        else:
            # command line provided nothing
            rootname = 'myscenario'
            # None# this means no directory will be created os.path.join(os.path.expanduser("~"),'sumopy','myscenario')
            dirpath = scenario.DIRPATH_SCENARIO
            name_scenario = 'My Scenario'
            self._scenario = scenario.Scenario(rootname, workdirpath=dirpath,
                                               name_scenario=name_scenario,
                                               logger=self._mainframe.get_logger())

    def refresh_widgets(self):
        """
        Check through mainframe what the state of the application is
        and reset widgets. For exampe enable/disable widgets
        dependent on the availability of data. 
        """
        self._mainframe.browse_obj(self._scenario)

    def make_menu(self):
        # event section
        #wx.EVT_BUTTON(self._mainframe, 1003, self.on_close)
        wx.EVT_CLOSE(self._mainframe, self.on_close)
        #wx.EVT_IDLE(self._mainframe, self.on_idle)

        # print 'make_menu'
        menubar = self._mainframe.menubar
        menubar.append_menu('Scenario')
        # menubar.append_menu( 'Scenario/import',
        #    bitmap = wx.ArtProvider.GetBitmap(wx.ART_NORMAL_FILE,wx.ART_MENU),
        #    )

        menubar.append_menu('Scenario/create',
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_NEW, wx.ART_MENU),
                            )

        menubar.append_item('Scenario/create/new...',
                            self.on_create,
                            info='Create new, empty scenario.',
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_NEW, wx.ART_MENU),
                            )

        menubar.append_item('Scenario/create/create from xml...',
                            self.on_create_from_xml,
                            info='Create scenario from various sumo xml files.',
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_NEW, wx.ART_MENU),
                            )

        if networkxtools.IS_NX:
            menubar.append_item('Scenario/create/create from osmnx...',
                                self.on_create_from_osmnx,
                                bitmap=wx.ArtProvider.GetBitmap(wx.ART_NEW, wx.ART_MENU),
                                )
            menubar.append_item('Scenario/create/import from osmnx in current...',
                                self.on_import_osmnx,
                                bitmap=wx.ArtProvider.GetBitmap(wx.ART_NEW, wx.ART_MENU),
                                )

        menubar.append_item('Scenario/open...',
                            self.on_open,
                            info='Open a new scenario from a Python binary file.',
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_FILE_OPEN, wx.ART_MENU),
                            )

        menubar.append_item('Scenario/browse',
                            self.on_browse_obj,  # common function in modulegui
                            info='View and browse Scenario in object panel.',
                            bitmap=self.get_agileicon('icon_browse_24px.png'),  # ,
                            )

        menubar.append_item('Scenario/safe',
                            self.on_save, shortkey='Ctrl+S',
                            info='Save current scenario in a Python binary file.',
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_FILE_SAVE, wx.ART_MENU),
                            )

        menubar.append_item('Scenario/safe as...',
                            self.on_save_as,
                            info='Save as scenario in a Python binary file.',
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_FILE_SAVE_AS, wx.ART_MENU),
                            )

        menubar.append_item('Scenario/quit...',
                            self.on_close,
                            info='Quit Sumopy', shortkey='Ctrl+Q',
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_QUIT, wx.ART_MENU)
                            )

    def on_close(self, event):
        # self.Close(True)
        # pass
        self.on_exit(event)

    def on_exit(self, event):
        """Called when the application is to be finished"""
        dlg = wx.MessageDialog(self._mainframe,
                               'SUMOPy is about to close.\nDo you want to SAVE the current scenario before closing?',
                               'Closing SUMOPy',
                               wx.YES_NO | wx.CANCEL | wx.ICON_QUESTION)
        ans = dlg.ShowModal()
        dlg.Destroy()
        # print '  ans,wx.ID_CANCEL,wx.ID_YES,wx.ID_NO',ans,wx.ID_CANCEL,wx.ID_YES,wx.ID_NO
        if ans == wx.ID_CANCEL:
            # print ' do not quit!'
            pass

        elif ans == wx.ID_YES:
            # print ' save and quit'
            scenario = self.get_scenario()
            cm.save_obj(scenario, scenario.get_rootfilepath()+'.obj',
                        is_not_save_parent=False)
            self._mainframe.destroy()

        elif ans == wx.ID_NO:
            # print 'quit immediately'
            self._mainframe.destroy()

    def on_create_from_xml(self, event=None):
        # print 'on_create_from_xml'
        scenariocreator = scenario.ScenarioCreator(
            workdirpath=scenario.DIRPATH_SCENARIO,
            logger=self._mainframe.get_logger()
        )
        dlg = ProcessDialog(self._mainframe, scenariocreator)

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

            del self._scenario
            self._scenario = scenariocreator.get_scenario()
            self._scenario.import_xml()
            self._mainframe.browse_obj(self._scenario)
            # this should update all widgets for the new scenario!!
            # print 'call self._mainframe.refresh_moduleguis()'
            self._mainframe.refresh_moduleguis()

    def on_create_from_osmnx(self, event=None):
        """
        Import net and buildings from OSMnx generated
        """

        pass

    def on_import_osmnx(self, event=None):
        """
        Import net and buildings from OSMnx
        """

        # proc = OxScenariocreator(\
        #                            workdirpath = scenario.DIRPATH_SCENARIO,
        #                            logger = self._mainframe.get_logger(),
        #                            )

        proc = networkxtools.OxImporter(self._scenario,
                                        logger=self._mainframe.get_logger(),
                                        )

        dlg = ProcessDialog(self._mainframe, proc)

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

            #del self._scenario
            #self._scenario = scenariocreator.get_scenario()

            # self._scenario.import_xml()
            self._mainframe.browse_obj(self._scenario)
            # this should update all widgets for the new scenario!!
            # print 'call self._mainframe.refresh_moduleguis()'
            self._mainframe.refresh_moduleguis()

    def on_create(self, event=None):
        # print 'on_create'
        scenariocreator = scenario.ScenarioCreator(logger=self._mainframe.get_logger(),
                                                   workdirpath=scenario.DIRPATH_SCENARIO,
                                                   )
        dlg = ProcessDialog(self._mainframe, scenariocreator)

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

            del self._scenario
            self._scenario = scenariocreator.get_scenario()
            self._mainframe.browse_obj(self._scenario)
            # this should update all widgets for the new scenario!!
            # print 'call self._mainframe.refresh_moduleguis()'
            self._mainframe.refresh_moduleguis()

    def on_open(self, event=None):
        wildcards_all = "All files (*.*)|*.*"
        wildcards_obj = "Python binary files (*.obj)|*.obj"
        wildcards = wildcards_obj+"|"+wildcards_all

        # Finally, if the directory is changed in the process of getting files, this
        # dialog is set up to change the current working directory to the path chosen.
        dlg = wx.FileDialog(
            self._mainframe, message="Open scenario file",
            #defaultDir = scenario.get_workdirpath(),
            #defaultFile = os.path.join(scenario.get_workdirpath(), scenario.format_ident()+'.obj'),
            wildcard=wildcards,
            style=wx.OPEN | wx.CHANGE_DIR
        )

        # Show the dialog and retrieve the user response. If it is the OK response,
        # process the data.
        if dlg.ShowModal() == wx.ID_OK:
            # This returns a Python list of files that were selected.
            filepath = dlg.GetPath()
            if len(filepath) > 0:

                del self._scenario
                self._scenario = scenario.load_scenario(filepath, logger=self._mainframe.get_logger())
                self._mainframe.browse_obj(self._scenario)
                # this should update all widgets for the new scenario!!
                # print 'call self._mainframe.refresh_moduleguis()'
                self._mainframe.refresh_moduleguis()

        # Destroy the dialog. Don't do this until you are done with it!
        # BAD things can happen otherwise!
        dlg.Destroy()

    def on_save(self, event=None):
        scenario = self.get_scenario().save()
        if event:
            event.Skip()

    def on_save_as(self, event=None):
        scenario = self.get_scenario()
        wildcards_all = "All files (*.*)|*.*"
        wildcards_obj = "Python binary files (*.obj)|*.obj"
        wildcards = wildcards_obj+"|"+wildcards_all

        # Finally, if the directory is changed in the process of getting files, this
        # dialog is set up to change the current working directory to the path chosen.
        dlg = wx.FileDialog(
            self._mainframe, message="Save scenario to file",
            defaultDir=scenario.get_workdirpath(),
            defaultFile=scenario.get_rootfilepath()+'.obj',
            wildcard=wildcards,
            style=wx.SAVE | wx.CHANGE_DIR
        )

        # Show the dialog and retrieve the user response. If it is the OK response,
        # process the data.
        if dlg.ShowModal() == wx.ID_OK:
            # This returns a Python list of files that were selected.
            filepath = dlg.GetPath()
            if len(filepath) > 0:
                # now set new filename and workdir

                scenario.save(filepath)
                self._mainframe.refresh_moduleguis()

        # Destroy the dialog. Don't do this until you are done with it!
        # BAD things can happen otherwise!
        dlg.Destroy()
