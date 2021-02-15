#!/usr/bin/env python
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

# @file    sumopy_gui.py
# @author  Joerg Schweizer
# @date

"""SUMOPy is intended to expand the user-base of the traffic micro-simulator SUMO (Simulation of Urban MObility) by providing a user-friendly, yet flexible simulation suite.

A further scope of SUMOPy is to manage the huge amount of data necessary to run complex multi-modal simulations.
This includes different demand generation models as well as a large range of modes, such as public transport,
bicycle and Personal Rapid Transit (PRT). SUMOPy consists of a GUI interface, network editor as well as a simple to use scripting language which facilitates the use of SUMO.
"""
__appname__ = "SUMOPy"
__version__ = "2.2"
__licence__ = """SUMOPy as well as SUMO is licensed under the GPL."""
__copyright__ = "(c) 2012-2019 University of Bologna - DICAM"
__author__ = "Joerg Schweizer"
__usage__ = """USAGE:
from command line:
    Open with empty scenario:
        python sumopy_gui.py

    Open new scenario and  import all SUMO xml files with scenariobasename:
        python sumopy_gui.py <scenariobasename> <scenariodir>

    Open binary scenario file:
        python sumopy_gui.py <path/scenarioname.obj>

use for debugging:
    python sumopy_gui.py --debug > debug.txt 2>&1
"""

print __appname__+' version'+__version__+'\n'+__copyright__

###############################################################################
# IMPORTS
import os
import sys

import wxversion
try:
    try:
        wxversion.select("3.0")
    except:
        wxversion.select("2.8")

    #
except:
    sys.exit('ERROR: wxPython versions 2.8 not available.')

import wx

moduledirs = ['coremodules', 'plugins']
APPDIR = ''
if __name__ == '__main__':
    if False:  # 'SUMO_HOME' in os.environ:
        APPDIR = os.path.join(os.environ['SUMO_HOME'], 'tools', 'contributed', 'sumopy')
    else:
        try:
            APPDIR = os.path.dirname(os.path.abspath(__file__))
        except:
            APPDIR = os.path.dirname(os.path.abspath(sys.argv[0]))

    # print 'APPDIR',APPDIR
    #libpaths = [APPDIR,]
    sys.path.append(APPDIR)
    #libpaths = [APPDIR, os.path.join(APPDIR,"agilepy"), os.path.join(APPDIR,"agilepy","lib_base"), os.path.join(APPDIR,"agilepy","lib_wx")]
    for moduledir in moduledirs:
        #    #print '  libpath=',libpath
        lp = os.path.abspath(os.path.join(APPDIR, moduledir))
        if not lp in sys.path:
            # print ' append',lp
            sys.path.append(lp)

#

from agilepy.lib_wx.mainframe import AgileMainframe

###############################################################################


class MyApp(wx.App):

    def __init__(self, redirect=False, filename=None):
        wx.App.__init__(self, redirect, filename)
        # print 'init',__appname__, sys.argv
        self.mainframe = AgileMainframe(
            title=__appname__,
            moduledirs=moduledirs,  # subdirectories containing modules
            appdir=APPDIR,
            args=sys.argv,
            is_maximize=True, is_centerscreen=True,
            size_toolbaricons=(32, 32),
            style=wx.DEFAULT_FRAME_STYLE | wx.NO_FULL_REPAINT_ON_RESIZE
        )
        icon = wx.Icon(os.path.join(APPDIR, 'images', 'icon_sumopy.png'), wx.BITMAP_TYPE_PNG, 16, 16)
        self.mainframe.SetIcon(icon)
        self.mainframe.Show()

        self.mainframe.refresh_moduleguis()

        self.mainframe.menubar.append_menu('About')
        self.mainframe.menubar.append_item('About/Info...', self.on_about,
                                           info='About SUMOPy',
                                           bitmap=wx.ArtProvider.GetBitmap(wx.ART_INFORMATION, wx.ART_MENU)
                                           )

    def on_about(self, event):
        info = wx.AboutDialogInfo()

        #info.SetIcon(wx.Icon('hunter.png', wx.BITMAP_TYPE_PNG))
        info.SetName(__appname__)
        info.SetVersion(__version__)
        info.SetDescription(__doc__)
        info.SetCopyright(__copyright__)
        info.SetWebSite('https://sumo-sim.org/')
        # info.SetWebSite('http://distart041.ing.unibo.it/~mait/projects/sim/users_guide/users_guide.html')
        info.SetLicence(__licence__)
        info.AddDeveloper(__author__)
        info.AddDeveloper("Supported by the SUMO team at the German Aerospace Center (DLR)!")
        info.AddDocWriter(__author__)
        info.AddArtist(__author__)
        #info.AddTranslator('Jan Bodnar')

        wx.AboutBox(info)
        event.Skip()


if __name__ == '__main__':
    myapp = MyApp(0)
    myapp.MainLoop()
