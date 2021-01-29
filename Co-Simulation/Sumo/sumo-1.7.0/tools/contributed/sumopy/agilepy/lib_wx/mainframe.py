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

# @file    mainframe.py
# @author  Joerg Schweizer
# @date


import wx
import wx.py as py  # pyshell
import sys
import os
import string
import time
from collections import OrderedDict

from wxmisc import KEYMAP, AgileMenuMixin, AgileToolbarFrameMixin, AgileStatusbar, AgileMenubar


from os.path import *
from os import getcwd

import objpanel
from agilepy.lib_base.logger import Logger

# We first have to set an application-wide help provider.  Normally you
# would do this in your app's OnInit or in other startup code...
provider = wx.SimpleHelpProvider()
wx.HelpProvider_Set(provider)


def make_moduleguis(appdir, dirlist):
    moduleguilist = []
    for modulesdir in dirlist:
        make_moduleguilist(appdir, moduleguilist, modulesdir)
        # print 'make_moduleguis: moduleguilist=\n',moduleguilist

    moduleguilist.sort()
    moduleguis = OrderedDict()
    for initpriority, wxgui in moduleguilist:
        moduleguis[wxgui.get_ident()] = wxgui
    return moduleguis


def make_moduleguilist(appdir, moduleguilist, modulesdir):
    # print 'make_moduleguilist appdir,modulesdir',appdir,modulesdir
    # print '  check dir',os.path.join(appdir, modulesdir)
    # for modulename in os.listdir(modulesdir):
    #    is_noimport = (modulename in ['__init__.py',]) | (modulename.split('.')[-1] == 'pyc')
    #    is_dir = os.path.isdir(os.path.join(modulesdir, modulename))
    #    if (not is_noimport)& (not is_dir):
    #        mn  = modulename.split('.')[0]
    #        for n in modulename.split('.')[1:-1]:
    #            mn += '.'+n
    #
    #        lib = __import__(modulesdir + '.' + mn )
    #        #module = getattr(lib,mn)
    #        #print '  imported',mn,modulesdir+'.' + mn

    for modulename in os.listdir(os.path.join(appdir, modulesdir)):  # use walk module to get recursive
        # if os.path.isdir(os.path.join(os.getcwd(), pluginname)): # is never a dir???

        is_noimport = (modulename in ['__init__.py', ]) | (modulename.split('.')[-1] == 'pyc')
        is_dir = os.path.isdir(os.path.join(appdir, modulesdir, modulename))

        # print '  modulename',modulename,is_noimport,is_dir
        if (not is_noimport) & is_dir:

            lib = __import__(modulesdir+'.'+modulename)
            module = getattr(lib, modulename)
            # print '   imported modulename',modulename,module,hasattr(module,'get_wxgui')
            # has  module gui support specified in __init__.py
            if hasattr(module, 'get_wxgui'):
                wxgui = module.get_wxgui()
                # print '    wxgui',wxgui
                if wxgui is not None:
                    # print '  append',(wxgui.get_initpriority(), wxgui)
                    moduleguilist.append((wxgui.get_initpriority(), wxgui))


class MainSplitter(wx.SplitterWindow):
    def __init__(self, parent, ID=wx.ID_ANY):
        wx.SplitterWindow.__init__(self, parent, ID,
                                   style=wx.SP_LIVE_UPDATE
                                   )

        self.SetMinimumPaneSize(20)

        #sty = wx.BORDER_SUNKEN

        #emptyobj = cm.BaseObjman('empty')
        self._objbrowser = objpanel.NaviPanel(self,
                                              None,
                                              #show_title = False
                                              #size = w.DefaultSize,
                                              #style = wx.DEFAULT_DIALOG_STYLE|wx.MAXIMIZE_BOX|wx.RESIZE_BORDER,
                                              # choose_id=False,choose_attr=False,
                                              # func_choose_id=None,
                                              # func_change_obj=None,
                                              #panelstyle = 'default',
                                              immediate_apply=True,
                                              buttons=[],
                                              standartbuttons=['apply', 'restore'],
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

        self._n_views = 0
        self._viewnames = []
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
        # self.SetSashGravity(0.2) # notebook too small
        self.SetSashPosition(400, True)
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
        self._viewnames.append(name)
        #self._views[name] = view
        # self._viewtabs.SetSelection(p)
        # self._viewtabs.Show(True)
        return view

    def select_view(self, ind=0, name=None):
        if name is not None:
            if name in self._viewnames:
                ind = self._viewnames.index(name)
                self._viewtabs.ChangeSelection(ind)
            else:
                return False
        else:
            self._viewtabs.ChangeSelection(ind)

    def browse_obj(self, obj, **kwargs):
        self._objbrowser.change_obj(obj, **kwargs)

    def OnSashChanged(self, evt):
        #print("sash changed to %s\n" % str(evt.GetSashPosition()))
        pass

    def OnSashChanging(self, evt):
        #print("sash changing to %s\n" % str(evt.GetSashPosition()))
        # uncomment this to not allow the change
        # evt.SetSashPosition(-1)
        # self.canvas.OnSize()
        pass


class AgileMainframe(AgileToolbarFrameMixin, wx.Frame):
    """
    Simple wx frame with some special features.
    """

    def __init__(self, parent=None,   title='mainframe',
                 moduledirs=[], args=[], appdir='',
                 is_maximize=False, is_centerscreen=True,
                 pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=wx.DEFAULT_FRAME_STYLE,
                 name='theframe', size_toolbaricons=(24, 24)):

        self._args = args
        # print 'AgileMainframe.__init__',title,appdir

        # Forcing a specific style on the window.
        #   Should this include styles passed?

        wx.Frame.__init__(self, parent, wx.ID_ANY, title,
                          pos, size=size, style=style, name=name)
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
        if is_maximize:
            self.Maximize()
        if is_centerscreen:
            self.CenterOnScreen()

        #################################################################
        # create statusbar
        #self.statusbar = AgileStatusbar(self)
        self.statusbar = AgileStatusbar(self)
        self.SetStatusBar(self.statusbar)
        # self.count=0.0

        #################################################################
        # create toolbar

        self.init_toolbar(size=size_toolbaricons)
        #
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
        # self.make_menu()
        # self.menubar.append_menu('tools')
        self.SetMenuBar(self.menubar)
        # self.Show(True) #NO!!

        #################################################################
        # init logger
        self._logger = Logger()
        self._logger.add_callback(self.write_message, 'message')
        self._logger.add_callback(self.write_action, 'action')
        self._logger.add_callback(self.show_progress, 'progress')
        #################################################################
        self._moduleguis = make_moduleguis(appdir, moduledirs)

        for modulename, modulegui in self._moduleguis.iteritems():
            # print '  init gui of module',modulename
            modulegui.init_widgets(self)
        #################################################################
        # event section: specify in App

        #wx.EVT_BUTTON(self, 1003, self.on_close)
        # wx.EVT_CLOSE(self, self.on_close)
        #wx.EVT_IDLE(self, self.on_idle)

    def refresh_moduleguis(self):
        # print 'refresh_moduleguis',len(self._moduleguis)
        self.browse_obj(None)
        for modulename, modulegui in self._moduleguis.iteritems():
            # print '  refresh gui of module',modulename
            modulegui.refresh_widgets()

    def get_modulegui(self, modulename):
        return self._moduleguis[modulename]

    def write_message(self, text, **kwargs):
        self.statusbar.write_message(text)

    def write_action(self, text, **kwargs):
        self.statusbar.write_action(text)
        self.statusbar.write_message('')

    def show_progress(self, percent, **kwargs):
        self.statusbar.set_progress(percent)

    def get_logger(self):
        return self._logger

    def set_logger(self, logger):
        self._logger = logger

    def get_args(self):
        return self._args

    def browse_obj(self, obj, **kwargs):
        self._splitter.browse_obj(obj, **kwargs)

    def make_menu(self):
        """
        Creates manu. To be overridden.
        """
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

    def select_view(self, ind=0, name=None):
        self._splitter.select_view(ind=ind, name=name)

    def on_size(self, event=None):
        # print 'Mainframe.on_size'
        # self.tc.SetSize(self.GetSize())
        # self.tc.SetSize(self.GetSize())
        # self._viewtabs.SetSize(self.GetSize())
        # pass
        #wx.LayoutAlgorithm().LayoutWindow(self, self.p1)
        #wx.LayoutAlgorithm().LayoutWindow(self, self.p1)

        # important:
        #wx.LayoutAlgorithm().LayoutWindow(self, self._viewtabs)
        wx.LayoutAlgorithm().LayoutWindow(self, self._splitter)
        # if event:
        #    event.Skip()

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
        print 'Mainframe.on_close'
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

    def write_to_statusbar(self, data, key='message'):
        self.statusbar[key] = str(data)
