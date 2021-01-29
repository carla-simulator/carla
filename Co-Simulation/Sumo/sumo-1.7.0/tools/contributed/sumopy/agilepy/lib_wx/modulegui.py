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

# @file    modulegui.py
# @author  Joerg Schweizer
# @date

import os
import wx

import agilepy.lib_base.classman as cm
from agilepy import AGILEDIR


def get_agileicon(name):
    return wx.Bitmap(os.path.join(AGILEDIR, 'lib_wx', 'images', name))


class ModuleGui:
    """Manages all GUIs to interact between the widgets from mainframe
    and the specific functions of the module.
    """

    def __init__(self, ident):

        self._init_common(ident,
                          module=cm.BaseObjman('empty'),
                          priority=9999999,
                          icondirpath=os.path.join(os.path.dirname(__file__), 'images')
                          )
        # self._init_constants()

    # def _init_constants(self):
    #    self._is_needs_refresh = False

    def get_initpriority(self):
        return self._initpriority

    def _init_common(self, ident, module=None, priority=9999999, icondirpath=''):
        """
        ident: any string to identify this gui
        module: the module which interacts with this gui
        priority: specifies priority with wich this gui initialized in mainframe
         Initialization of widgets in init_widgets 
        """

        self._ident = ident
        self._set_module(module)
        self._initpriority = priority
        self._icondirpath = icondirpath
        self._is_needs_refresh = False

    def get_icon(self, name):
        return wx.Bitmap(os.path.join(self._icondirpath, name))

    def get_agileicon(self, name):
        return wx.Bitmap(os.path.join(AGILEDIR, 'lib_wx', 'images', name))

    def _set_module(self, module):
        self._module = module

    def get_module(self):
        return self._module

    def get_ident(self):
        return self._ident

    def get_logger(self):
        return self._mainframe.get_logger()

    # def set_logger(self, logger):
    #    self._logger = logger

    def init_widgets(self, mainframe):
        """
        Set mainframe and initialize widgets to various places.
        """
        self._mainframe = mainframe
        self.make_menu()
        self.make_toolbar()

    def refresh_widgets(self):
        """
        Check through mainframe what the state of the application is
        and reset widgets. For exampe enable/disable widgets
        dependent on the availability of data. 
        """
        pass

    def make_toolbar(self):
        pass

    def make_menu(self):
        pass

    def on_browse_obj(self,  event=None):
        self._mainframe.browse_obj(self.get_module())
