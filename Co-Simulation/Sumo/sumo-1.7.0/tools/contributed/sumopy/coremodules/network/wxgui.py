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

from agilepy.lib_wx.modulegui import ModuleGui
from agilepy.lib_wx.processdialog import ProcessDialog


import network
import routing
import netgenerate
import netconvert
import networktools
from publictransportnet_wxgui import PtWxGuiMixin
from network_editor import *
from coremodules.misc import shapeformat


class WxGui(PtWxGuiMixin, ModuleGui):
    """Contains functions that communicate between the widgets of the main wx gui
    and the functions of the plugin.
    """

    def __init__(self, ident):
        self._net = None
        self._init_common(ident,  priority=10,
                          icondirpath=os.path.join(os.path.dirname(__file__), 'images'))

    def get_module(self):
        return self._net

    def get_scenario(self):
        return self._mainframe.get_modulegui('coremodules.scenario').get_module()

    def get_neteditor(self):
        return self._neteditor

    def init_widgets(self, mainframe):
        """
        Set mainframe and initialize widgets to various places.
        """
        self._mainframe = mainframe
        self._neteditor = mainframe.add_view("Network", Neteditor)

        # mainframe.browse_obj(self._net)
        self.make_menu()
        self.make_toolbar()

    def refresh_widgets(self):
        """
        Check through mainframe what the state of the application is
        and reset widgets. For exampe enable/disable widgets
        dependent on the availability of data. 
        """
        is_refresh = False
        scenario = self.get_scenario()
        # print '\n\nnetwork refresh_widgets',self._net != scenario.net,scenario.net.is_modified(),id(self._net)
        if self._net != scenario.net:
            # print '  scenario has a new network'
            # print '  self._net != scenario.net',self._net,scenario.net
            del self._net
            self._net = scenario.net
            is_refresh = True

        elif self._net.is_modified():
            # print '   network is_modified',self._net.is_modified()
            is_refresh = True

        # print  '  is_refresh',is_refresh
        if is_refresh:
            drawing = self._neteditor.set_net(self._net)
            #canvas = self._neteditor.get_canvas()

        self.refresh_pt(is_refresh)

    def make_menu(self):
        # print 'make_menu'
        menubar = self._mainframe.menubar
        menubar.append_menu('network', bitmap=self.get_icon('icon_graph.png'))

        menubar.append_item('network/browse',
                            self.on_browse_obj,  # common function in modulegui
                            info='View and browse network in object panel.',
                            bitmap=self.get_agileicon('icon_browse_24px.png'),  # ,
                            )

        # import
        menubar.append_menu('network/import',
                            bitmap=self.get_agileicon("Document_Import_24px.png"),
                            )

        menubar.append_item('network/import/from sumo net.xml ...',
                            self.on_import_sumonet,
                            info='Import network from sumo net.xml file.',
                            bitmap=self.get_icon('icon_sumo_24px.png'),  # ,
                            )

        menubar.append_item('network/import/from osm.xml ...',
                            self.on_import_osm,
                            info='Import network from osm files.',
                            bitmap=self.get_icon('Files-Osm-icon_24.png'),
                            )

        # exports
        menubar.append_menu('network/export',
                            bitmap=self.get_agileicon("Document_Export_24px.png"),
                            )

        menubar.append_item('network/export/sumo net.xml',
                            self.on_export_sumonet,
                            info='Export network to sumo net.xml file.',
                            bitmap=self.get_icon('icon_sumo_24px.png'),  # ,
                            )

        menubar.append_item('network/export/edges to shape...',
                            self.on_edges_to_shapefile,
                            bitmap=self.get_icon('Files-Osm-icon_24.png'),
                            )
        menubar.append_item('network/export/nodes to shape...',
                            self.on_nodes_to_shapefile,
                            bitmap=self.get_icon('Files-Osm-icon_24.png'),
                            )
        # generate
        menubar.append_menu('network/generate',
                            bitmap=self.get_icon("icon_graph.png"),
                            )

        menubar.append_item('network/generate/grid network...',
                            self.on_generate_grid,
                            info='Generate a grid network with netgenerate.',
                            bitmap=self.get_icon('icon_sumo_24px.png'),  # ,
                            )

        menubar.append_item('network/generate/spider network...',
                            self.on_generate_spider,
                            info='Generate a spider network with netgenerate.',
                            bitmap=self.get_icon('icon_sumo_24px.png'),  # ,
                            )

        menubar.append_item('network/generate/random network...',
                            self.on_generate_random,
                            info='Generate a random network with netgenerate.',
                            bitmap=self.get_icon('icon_sumo_24px.png'),  # ,
                            )

        menubar.append_item('network/edit with SUMO netedit...',
                            self.on_netedit,
                            info="Edit network with SUMO's netedit.",
                            bitmap=self.get_icon('netedit.png'),
                            )

        menubar.append_item('network/edit with SUMO netedit on map...',
                            self.on_netedit_on_map,
                            info="Edit network with SUMO's netedit. In addition to the network, backround maps are shown. You need to download the maps before with landuse/import maps.",
                            bitmap=self.get_icon('netedit.png'),
                            )

        menubar.append_item('network/show with SUMO GUI on map...',
                            self.on_sumogui,
                            info="Show network with sumo-gui on background map.",
                            bitmap=self.get_icon('icon_sumo_24px.png'),  # ,
                            )

        menubar.append_menu('network/tools',
                            #bitmap = self.get_agileicon("Document_Export_24px.png"),
                            )

        # menubar.append_item( 'network/refresh',
        #    self.on_refresh,
        #    info='Refresh graph.',
        #    #bitmap = wx.ArtProvider.GetBitmap(wx.ART_NORMAL_FILE,wx.ART_MENU),
        #    )

        menubar.append_item('network/tools/netconvert...',
                            self.on_netconvert,
                            info='Modify global properties of the network.',
                            #bitmap = wx.ArtProvider.GetBitmap(wx.ART_NORMAL_FILE,wx.ART_MENU),
                            )

        menubar.append_item('network/tools/TLS generate...',
                            self.on_generate_tls,
                            #info='Modify global properties of the network.',
                            #bitmap = wx.ArtProvider.GetBitmap(wx.ART_NORMAL_FILE,wx.ART_MENU),
                            )

        menubar.append_item('network/tools/TL signals generate...',
                            self.on_generate_tlsignals,
                            #bitmap = wx.ArtProvider.GetBitmap(wx.ART_NORMAL_FILE,wx.ART_MENU),
                            )

        menubar.append_item('network/tools/connect footpath',
                            self.on_connect_footpath,
                            #bitmap = wx.ArtProvider.GetBitmap(wx.ART_NORMAL_FILE,wx.ART_MENU),
                            )

        menubar.append_item('network/tools/complete bikenetwork...',
                            self.on_complete_bikenetwork,
                            #bitmap = wx.ArtProvider.GetBitmap(wx.ART_NORMAL_FILE,wx.ART_MENU),
                            )

        menubar.append_item('network/tools/enlarge nodes',
                            self.on_clean_codes,
                            info='Enlarge nodes and cut back edges and lanes to the border of the node, so that connections become clearer visible end editable.',
                            #bitmap = wx.ArtProvider.GetBitmap(wx.ART_NORMAL_FILE,wx.ART_MENU),
                            )

        menubar.append_item('network/tools/set Tls blinking to off',
                            self.on_blinking_to_off,
                            #bitmap = wx.ArtProvider.GetBitmap(wx.ART_NORMAL_FILE,wx.ART_MENU),
                            )

        self.add_menu_pt(menubar)

        # menubar.append_item( 'network/correct spread',
        #    self.on_correct_spread,
        #    info='Corrects spread type for older versions.',
        #    #bitmap = wx.ArtProvider.GetBitmap(wx.ART_NORMAL_FILE,wx.ART_MENU),
        #    )

        # menubar.append_item( 'network/correct endpoint',
        #    self.on_correct_endpoint,
        #    info='Corrects send points for older versions.',
        #    #bitmap = wx.ArtProvider.GetBitmap(wx.ART_NORMAL_FILE,wx.ART_MENU),
        #    )

        # menubar.append_item( 'network/clear Tls programs',
        #    self.on_clear_tlss,
        #    info='This will clear all traffic light programs, which can be regenerated with netconvert.',
        #    bitmap = wx.ArtProvider.GetBitmap(wx.ART_DELETE,wx.ART_MENU),
        #    )

        menubar.append_item('network/clear network',
                            self.on_clear_net,
                            info='This will clear the entire network.',
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_DELETE, wx.ART_MENU),
                            )
        # menubar.append_item( 'network/test routing',
        #    self.on_test_routing,
        #    info='test routing.',
        #    #bitmap = wx.ArtProvider.GetBitmap(wx.ART_NORMAL_FILE,wx.ART_MENU),
        #    )
        # menubar.append_item( 'network/test',
        #    self.on_test,
        #    info='Test graph.',
        #    #bitmap = wx.ArtProvider.GetBitmap(wx.ART_NORMAL_FILE,wx.ART_MENU),
        #    )

    def on_clear_net(self, event=None):
        # print 'on_clear_net',id(self._net)
        self._net.clear_net()
        self._mainframe.refresh_moduleguis()

    # def on_clear_tlss(self,event = None):
    #    #print 'on_clear_net',id(self._net)
    #    self._net.tlss.clear_tlss()
    #    self._mainframe.refresh_moduleguis()

    def on_netedit(self, event=None):
        if self._net.call_netedit():
            self._mainframe.refresh_moduleguis()

    def on_netedit_on_map(self, event=None):
        if self._net.call_netedit(is_maps=True):
            self._mainframe.refresh_moduleguis()

    def on_sumogui(self, event=None):
        self._net.call_sumogui(is_maps=True, is_poly=True)

    def on_test_routing(self, event=None):
        D, P = routing.dijkstra(54, self._net.nodes, self._net.edges, set([42, 82]))
        cost, route = routing.get_mincostroute_node2node(54, 42, D, P, self._net.edges)
        print ' route:', route
        print ' cost', cost
        print '  firstnode, lastnode', self._net.edges.ids_fromnode[route[0]], self._net.edges.ids_tonode[route[-1]]

        D, P = routing.edgedijkstra(29, self._net.nodes, self._net.edges, set([106, 82]))
        cost, route = routing.get_mincostroute_edge2edge(29, 82, D, P)
        print ' route:', route
        print ' cost', cost
        # print  '  firstnode, lastnode',self._net.edges.ids_fromnode[route[0]],self._net.edges.ids_tonode[route[-1]]

    def on_clean_codes(self, event=None):
        self._net.clean_nodes(is_reshape_edgelanes=True)
        self._mainframe.browse_obj(self._net.nodes)
        self._mainframe.refresh_moduleguis()  # this will also replace the drawing!!

    def on_blinking_to_off(self, event=None):
        self._net.tlss.change_states('o', 'O')
        self._mainframe.browse_obj(self._net.tlss)

    def on_connect_footpath(self, event=None):
        self._net.complete_connections()
        self._mainframe.browse_obj(self._net.edges)
        self._mainframe.refresh_moduleguis()  # this will also replace the drawing!!

    def on_refresh(self, event=None):
        # print 'on_refresh neteditor',id(self._neteditor.get_drawing())

        canvas = self._neteditor.get_canvas()
        wx.CallAfter(canvas.zoom_tofit)
        self._mainframe.browse_obj(self._net)

    def on_correct_spread(self, event=None):
        self._net.edges.correct_spread()
        self._mainframe.refresh_moduleguis()

    def on_correct_endpoint(self, event=None):
        self._net.edges.correct_endpoint()
        self._mainframe.refresh_moduleguis()
        if event:
            event.Skip()

    def on_complete_bikenetwork(self, event=None):
        """
        Make network more permeable for bikes. 
        """
        p = networktools.BikenetworkCompleter(self._net, logger=self._mainframe.get_logger())
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
            self._mainframe.browse_obj(self._net.edges)
            self._mainframe.refresh_moduleguis()
            #self._is_needs_refresh = True
            # self.refresh_widgets()

    def on_import_sumonet(self, event=None):
        # TODO: here we should actually replace the current network
        # so we would need a clear net method in scenario
        # alternatively we could merge properly
        sumonetimporter = network.SumonetImporter(self._net, logger=self._mainframe.get_logger())
        dlg = ProcessDialog(self._mainframe, sumonetimporter)

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

            self._mainframe.browse_obj(self._net)

            # this should update all widgets for the new scenario!!
            # print 'call self._mainframe.refresh_moduleguis()'
            self._mainframe.refresh_moduleguis()

    def on_import_osm(self, event=None):
        # TODO: here we should actually replace the current network
        # so we would need a clear net method in scenario
        # alternatively we could merge properly
        importer = network.OsmImporter(self._net, logger=self._mainframe.get_logger())
        dlg = ProcessDialog(self._mainframe, importer)

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

            self._mainframe.browse_obj(self._net.nodes)

            # this should update all widgets for the new scenario!!
            # print 'call self._mainframe.refresh_moduleguis()'
            self._mainframe.refresh_moduleguis()

    def on_generate_grid(self, event=None):
        # TODO: here we should actually replace the current network
        # so we would need a clear net method in scenario
        # alternatively we could merge properly
        generator = netgenerate.GridGenerate(self._net, logger=self._mainframe.get_logger())
        dlg = ProcessDialog(self._mainframe, generator)

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
            dlg.apply()
            dlg.Destroy()

            self._mainframe.browse_obj(self._net)

            # this should update all widgets for the new scenario!!
            # print 'call self._mainframe.refresh_moduleguis()'
            self._mainframe.refresh_moduleguis()

    def on_generate_spider(self, event=None):
        # TODO: here we should actually replace the current network
        # so we would need a clear net method in scenario
        # alternatively we could merge properly
        generator = netgenerate.SpiderGenerate(self._net, logger=self._mainframe.get_logger())
        dlg = ProcessDialog(self._mainframe, generator)

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
            dlg.apply()
            dlg.Destroy()

            self._mainframe.browse_obj(self._net)

            # this should update all widgets for the new scenario!!
            # print 'call self._mainframe.refresh_moduleguis()'
            self._mainframe.refresh_moduleguis()

    def on_generate_random(self, event=None):
        # TODO: here we should actually replace the current network
        # so we would need a clear net method in scenario
        # alternatively we could merge properly
        generator = netgenerate.RandomGenerate(self._net, logger=self._mainframe.get_logger())
        dlg = ProcessDialog(self._mainframe, generator)

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
            dlg.apply()
            dlg.Destroy()

            self._mainframe.browse_obj(self._net)

            # this should update all widgets for the new scenario!!
            # print 'call self._mainframe.refresh_moduleguis()'
            self._mainframe.refresh_moduleguis()

    def on_netconvert(self, event=None):
        # TODO: here we should actually replace the current network
        # so we would need a clear net method in scenario
        # alternatively we could merge properly
        obj = netconvert.NetConvert(self._net, logger=self._mainframe.get_logger())
        dlg = ProcessDialog(self._mainframe, obj)

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
            dlg.apply()
            dlg.Destroy()

            self._mainframe.browse_obj(self._net)

            # this should update all widgets for the new scenario!!
            # print 'call self._mainframe.refresh_moduleguis()'
            self._mainframe.refresh_moduleguis()

    def on_generate_tlsignals(self, event=None):
        """Generates or regenerates sinals for traffic lights."""
        # TODO: here we should actually replace the current network
        # so we would need a clear net method in scenario
        # alternatively we could merge properly
        obj = netconvert.TlSignalGenerator(self._net, logger=self._mainframe.get_logger())
        dlg = ProcessDialog(self._mainframe, obj)

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
            dlg.apply()
            dlg.Destroy()

            self._mainframe.browse_obj(self._net)

            # this should update all widgets for the new scenario!!
            # print 'call self._mainframe.refresh_moduleguis()'
            self._mainframe.refresh_moduleguis()

    def on_generate_tls(self, event=None):
        """Generates traffic light systems"""
        # TODO: here we should actually replace the current network
        # so we would need a clear net method in scenario
        # alternatively we could merge properly
        obj = networktools.TlsGenerator(self._net, logger=self._mainframe.get_logger())
        dlg = ProcessDialog(self._mainframe, obj)

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
            dlg.apply()
            dlg.Destroy()

            self._mainframe.browse_obj(self._net)

            # this should update all widgets for the new scenario!!
            # print 'call self._mainframe.refresh_moduleguis()'
            self._mainframe.refresh_moduleguis()

    def on_export_sumonet(self, event=None):
        print 'on_export_sumonet'
        if self._net.parent is not None:
            rootname = self._net.parent.get_rootfilename()
            rootdirpath = self._net.parent.get_workdirpath()
        else:
            rootname = self._net.get_ident()
            rootdirpath = os.getcwd()

        netfilepath = os.path.join(rootdirpath, rootname+'.net.xml')
        wildcards_all = 'All files (*.*)|*.*|XML files (*net.xml)|*net.xml'

        dlg = wx.FileDialog(None, message='Write trips to SUMO xml',
                            defaultDir=rootdirpath,
                            defaultFile=netfilepath,
                            wildcard=wildcards_all, style=wx.SAVE | wx.CHANGE_DIR)
        if dlg.ShowModal() == wx.ID_OK:
            filepath = dlg.GetPath()

        else:
            return

        self._net.export_netxml(filepath)

    def on_edges_to_shapefile(self, event=None):
        """
        Export Network edge data to shape file.
        """
        # print 'on_edges_to_shapefile'

        dirpath = self._net.parent.get_workdirpath()
        defaultFile = self._net.parent.get_rootfilename()+'.edges.shp'
        wildcards_all = 'All files (*.*)|*.*|SHP files (*.shp)|*.shp'
        dlg = wx.FileDialog(None, message='Export Edges to shapefile',
                            defaultDir=dirpath, defaultFile=defaultFile,
                            wildcard=wildcards_all, style=wx.SAVE | wx.CHANGE_DIR)
        if dlg.ShowModal() == wx.ID_OK:
            filepath = dlg.GetPath()

        else:
            return

        shapeformat.edges_to_shapefile(self._net,
                                       filepath,
                                       log=self._mainframe.get_logger())

    def on_nodes_to_shapefile(self, event=None):
        """
        Export Network nodes data to shape file.
        """
        print 'on_nodes_to_shapefile'

        dirpath = self._net.parent.get_workdirpath()
        defaultFile = self._net.parent.get_rootfilename()+'.nodes.shp'
        wildcards_all = 'All files (*.*)|*.*|SHP files (*.shp)|*.shp'
        dlg = wx.FileDialog(None, message='Export nodes to shapefile',
                            defaultDir=dirpath, defaultFile=defaultFile,
                            wildcard=wildcards_all, style=wx.SAVE | wx.CHANGE_DIR)
        if dlg.ShowModal() == wx.ID_OK:
            filepath = dlg.GetPath()

        else:
            return

        shapeformat.nodes_to_shapefile(self._net,
                                       filepath,
                                       log=self._mainframe.get_logger())
