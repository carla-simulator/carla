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

import mapmatching


#import results_mpl as results_mpl
try:
    import results_mpl as results_mpl
    is_mpl = True  # we have matplotlib support
except:
    print "WARNING: python matplotlib package not installed, no matplotlib plots."
    is_mpl = False


class GpsPointsDrawings(Circles):
    def __init__(self, ident, gpspoints, parent,   **kwargs):

        Circles.__init__(self, ident,  parent, name='GPS points',
                         is_parentobj=False,
                         is_fill=True,  # Fill objects,
                         is_outline=False,  # show outlines
                         n_vert=11,  # default number of vertex per circle
                         linewidth=3,
                         **kwargs)

        self.delete('centers')
        self.delete('radii')

        self.add(cm.AttrConf('color_default', np.array([1.0, 0.8, 0.1, 0.5], np.float32),
                             groupnames=['options', 'colors'],
                             metatype='color',
                             perm='wr',
                             name='Default color',
                             info='Default point color.',
                             ))

        self.set_netelement(gpspoints)

    def get_netelement(self):
        return self._gpspoints

    def get_centers_array(self):
        # return self._gpspoints.coords.value[self._inds_map]
        return self._gpspoints.coords[self.get_ids()]

    def get_radii_array(self):
        return self._gpspoints.radii[self.get_ids()]
        # return self._gpspoints.radii.value[self._inds_map]

    def is_tool_allowed(self, tool, id_drawobj=-1):
        """
        Returns True if this tool can be applied to this drawobj.
        Optionally a particular drawobj can be specified with id_drawobj.
        """
        # basic tools:
        return tool.ident not in ['select_handles', 'delete', 'move', 'stretch']  # 'configure',
        # return tool.ident not in   ['delete','stretch']

    def set_netelement(self, gpspoints):
        # print 'set_nodes'
        self._gpspoints = gpspoints
        # if len(self)>0:
        #    self.del_rows(self.get_ids())
        self.clear_rows()

        ids = self._gpspoints.get_ids_selected()
        n = len(ids)

        #self._inds_map = self._gpspoints.get_inds(ids)

        # print 'color_node_default',self.color_node_default.value
        # print 'colors\n',  np.ones((n,1),np.int32)*self.color_node_default.value
        self.add_rows(ids=ids,
                      #colors = np.ones((n,1),np.int32)*self.color_default.value,
                      #colors_highl = self._get_colors_highl(np.ones((n,1),np.int32)*self.color_default.value),
                      colors_fill=np.ones((n, 1), np.int32)*self.color_default.value,
                      colors_highl_highl=self._get_colors_highl(np.ones((n, 1), np.int32)*self.color_default.value),
                      #centers = self._nodes.coords[ids],
                      #radii = self._nodes.radii[ids],
                      )

        self.update()

    def update(self, is_update=True):

        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()


class GpsRoutesDrawings(Polylines):
    def __init__(self, ident, edges, parent,   **kwargs):

        # joinstyle
        # FLATHEAD = 0
        # BEVELHEAD = 1
        Polylines.__init__(self, ident,  parent, name='GPS routes drawings',
                           is_lefthalf=True,
                           is_righthalf=True,
                           arrowstretch=1.5,
                           joinstyle=BEVELHEAD,  # FLATHEAD,#BEVELHEAD is good for both halfs,
                           **kwargs)

        # self.delete('vertices')
        # self.delete('widths')
        # self.delete('colors')

        self.add(cm.AttrConf('width_default', 4.0,
                             groupnames=['options'],
                             perm='wr',
                             name='Default width',
                             info='Default route width of drawing.',
                             ))

        self.add(cm.AttrConf('color_default', np.array([1.0, 0.4, 0.0, 0.6], np.float32),
                             groupnames=['options'],
                             perm='wr',
                             metatype='color',
                             name='Default color',
                             info='Default route color.',
                             ))

        self.set_netelement(edges)

    def get_netelement(self):
        return self._routes

    # def get_vertices_array(self):
    #    return self._routes.shapes[self.get_ids()]#.value[self._inds_map]#[self.get_ids()]

    # def get_widths_array(self):
    #    # double because only the right half is shown
    #    # add a little bit to the width to make it a little wider than the lanes contained
    #    #return 2.2*self._edges.widths.value[self._inds_map]
    #    return 1.1*self._edges.widths[self.get_ids()]#.value[self._inds_map]

    # def get_vertices(self, ids):
    #    return self._edges.shapes[ids]

    # def set_vertices(self, ids, vertices, is_update = True):
    #    self._edges.set_shapes(ids, vertices)
    #    if is_update:
    #        self._update_vertexvbo()
    #        self.parent.get_drawobj_by_ident('lanedraws').update()
    #        self.parent.get_drawobj_by_ident('crossingsdraws').update()
    #        self.parent.get_drawobj_by_ident('connectiondraws').update()

    # def get_widths(self, ids):
    #    return 1.1*self._edges.widths[ids]

    # def set_widths(self, ids, values):
    #    #self._edges.widths[ids] = values/1.1
    #    pass

    def is_tool_allowed(self, tool, id_drawobj=-1):
        """
        Returns True if this tool can be applied to this drawobj.
        Optionally a particular drawobj can be specified with id_drawobj.
        """
        # basic tools:
        return tool.ident not in ['configure', 'select_handles', 'delete', 'move', 'stretch']
        # return tool.ident not in   ['delete',]

    def set_netelement(self, routes):

        self._routes = routes
        #self._inds_edges = self._edges.get_inds()
        self.clear_rows()
        # if len(self)>0:
        #    self.del_rows(self.get_ids())

        ids = self._routes.parent.get_ids_route_selected()
        #self._inds_map = self._edges.get_inds(ids)
        n = len(ids)
        #self.vertices = self._edges.shapes
        #self.widths = self._edges.widths
        # print '\n\nGpsRoutesDrawings.set_netelement',n
        # print '  ids.dtype',ids.dtype
        # print '  self._ids.dtype',self._ids.dtype
        # print '  self._inds.dtype',self._inds.dtype
        # print '  ids',ids
        self.add_rows(ids=ids,
                      beginstyles=np.ones(n, dtype=np.float32)*FLATHEAD,
                      endstyles=np.ones(n, dtype=np.float32)*TRIANGLEHEAD,
                      widths=np.ones(n, dtype=np.float32)*self.width_default.get_value()
                      )
        self.vertices[ids] = self._routes.get_shapes(ids)
        self.update()

    def update(self, is_update=True):
        """
        Update color, assume that there have not been structural changes of the arrays
        """
        # assumes that edges have been set in set_edges
        # print 'Edgedrawing.update'
        #edgeinds = self._edges.get_inds()
        n = len(self)
        ids = self.get_ids()

        self.colors_fill.value[:] = self._routes.colors[ids]
        #self.colors_fill.value[:] = np.ones((n,1),np.float32)*self.color_default.value
        self.colors_fill_highl.value[:] = self._get_colors_highl(self.colors_fill.value)

        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()


class WxGui(ModuleGui):
    """Contains functions that communicate between the widgets of the main wx gui
    and the functions of the plugin.
    """

    def __init__(self, ident):
        self._mapmatching = None
        self._matchprocess = None
        self._results = None
        self._scenario = None
        self._canvas = None
        self._init_common(ident,  priority=100001,
                          icondirpath=os.path.join(os.path.dirname(__file__), 'images'))

        self._is_needs_refresh = False

    def get_module(self):
        return self._mapmatching

    def get_scenario(self):
        return self._mainframe.get_modulegui('coremodules.scenario').get_scenario()

    def get_neteditor(self):
        return self._mainframe.get_modulegui('coremodules.network').get_neteditor()

    def get_canvas(self):
        return self.get_neteditor().get_canvas()

    def get_drawing(self):
        return self.get_canvas().get_drawing()

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
        print 'MapmatchingWxGui.refresh_widgets'
        scenario = self.get_scenario()
        is_refresh = False
        if self._scenario != scenario:
            del self._scenario
            del self._mapmatching
            del self._results
            self._scenario = scenario
            self._mapmatching = scenario.demand.add_demandobject(
                ident='mapmatching', DemandClass=mapmatching.Mapmatching)
            #self._mapmatching =  mapmatching.Mapmatching('mapmatching', scenario)
            self._matchprocess = None
            self._results = mapmatching.Matchresults('matchresults',
                                                     self._mapmatching,
                                                     )
            is_refresh = True

        if is_refresh | self._is_needs_refresh:
            self._is_needs_refresh = False
            print '  is_refresh', is_refresh, self._is_needs_refresh
            neteditor = self.get_neteditor()
            #canvas = self.get_canvas()
            drawing = self.get_drawing()  # canvas.get_drawing()

            # add or refresh facility drawing
            drawing.set_element('gpspointsdraws', GpsPointsDrawings,
                                self._mapmatching.points, layer=150)

            drawing.set_element('gpsroutesdraws', GpsRoutesDrawings,
                                self._mapmatching.trips.get_routes(), layer=149)

            # neteditor.get_toolbox().add_toolclass(AddZoneTool)# will check if tool is already there
            # neteditor.get_toolbox().add_toolclass(AddFacilityTool)
            neteditor.draw()

        self._canvas = self.get_canvas()

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

        # menubar.append_item( 'plugins/mapmatching/open...',
        #    self.on_open,
        #    bitmap = wx.ArtProvider.GetBitmap(wx.ART_FILE_OPEN,wx.ART_MENU),
        #    )

        menubar.append_menu('plugins/mapmatching/import',
                            bitmap=self.get_agileicon("Document_Import_24px.png"),
                            )

        menubar.append_item('plugins/mapmatching/import/European cycling challange...',
                            self.on_import_ecc,
                            # info=self.on_import_ecc.__doc__.strip(),
                            bitmap=self.get_agileicon("Document_Import_24px.png"),
                            )

        menubar.append_item('plugins/mapmatching/import/GPX file...',
                            self.on_import_gpx,
                            # info=self.on_import_ecc.__doc__.strip(),
                            bitmap=self.get_agileicon("Document_Import_24px.png"),
                            )

        # menubar.append_item( 'plugins/mapmatching/project points',
        #    self.on_project_points,
        #    #bitmap = wx.ArtProvider.GetBitmap(wx.ART_FILE_SAVE_AS,wx.ART_MENU),
        #    )

        menubar.append_item('plugins/mapmatching/match with birgillito method...',
                            self.on_match_birgil,
                            #bitmap = wx.ArtProvider.GetBitmap(wx.ART_FILE_SAVE_AS,wx.ART_MENU),
                            )

        menubar.append_item('plugins/mapmatching/shortest path routing...',
                            self.on_route_shortest,
                            #bitmap = wx.ArtProvider.GetBitmap(wx.ART_FILE_SAVE_AS,wx.ART_MENU),
                            )

        menubar.append_item('plugins/mapmatching/fastest path routing...',
                            self.on_route_fastest,
                            #bitmap = wx.ArtProvider.GetBitmap(wx.ART_FILE_SAVE_AS,wx.ART_MENU),
                            )


# -------------------------------------------------------------------------------

        menubar.append_menu('plugins/mapmatching/person analysis',
                            bitmap=self.get_icon('icon_results_24px.png'),  # ,
                            info='Person analysis tools'
                            )

        menubar.append_item('plugins/mapmatching/person analysis/analyze',
                            self.on_analyze_persons,
                            #bitmap = wx.ArtProvider.GetBitmap(wx.ART_FILE_SAVE_AS,wx.ART_MENU),
                            )

        menubar.append_item('plugins/mapmatching/person analysis/save matched in csv...',
                            self.on_export_persons_csv,
                            info='Save persons with matched trips in a CSV file.',
                            bitmap=self.get_agileicon("Document_Export_24px.png"),
                            )

# -------------------------------------------------------------------------------

        menubar.append_menu('plugins/mapmatching/route analysis',
                            bitmap=self.get_icon('icon_results_24px.png'),  # ,
                            info='Route analysis tools'
                            )

        menubar.append_item('plugins/mapmatching/route analysis/browse',
                            self.on_browse_results,  # common function in modulegui
                            bitmap=self.get_agileicon('icon_browse_24px.png'),  # ,
                            )

        menubar.append_item('plugins/mapmatching/route analysis/analyze...',
                            self.on_routeanalyze,  # common function in modulegui
                            # bitmap = self.get_agileicon('icon_browse_24px.png'),#,
                            )

        if is_mpl:
            menubar.append_item('plugins/mapmatching/route analysis/plot route results...',
                                self.on_plot_routeresults,
                                bitmap=results_mpl.get_mplicon(),  # ,
                                )
            menubar.append_item('plugins/mapmatching/route analysis/plot edge results...',
                                self.on_plot_edgeresults,
                                bitmap=results_mpl.get_mplicon(),  # ,
                                )
            menubar.append_item('plugins/mapmatching/route analysis/plot speed profiles...',
                                self.on_plot_speedprofiles,
                                bitmap=results_mpl.get_mplicon(),  # ,
                                )
            menubar.append_item('plugins/mapmatching/route analysis/plot node results...',
                                self.on_plot_noderesults,
                                bitmap=results_mpl.get_mplicon(),  # ,
                                )

        menubar.append_item('plugins/mapmatching/route analysis/safe as...',
                            self.on_save_results,
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_FILE_SAVE_AS, wx.ART_MENU),
                            )

        menubar.append_item('plugins/mapmatching/route analysis/open...',
                            self.on_open_results,
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_FILE_OPEN, wx.ART_MENU),
                            )

        menubar.append_item('plugins/mapmatching/route analysis/route results to shape...',
                            self.on_routes_to_shapefile,
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_FILE_SAVE_AS, wx.ART_MENU),
                            )

        menubar.append_item('plugins/mapmatching/route analysis/edge results to shape...',
                            self.on_edgesresults_to_shapefile,
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_FILE_SAVE_AS, wx.ART_MENU),
                            )

        menubar.append_item('plugins/mapmatching/route analysis/GPS points to shape...',
                            self.on_points_to_shapefile,
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_FILE_SAVE_AS, wx.ART_MENU),
                            )

# -------------------------------------------------------------------------------

        menubar.append_menu('plugins/mapmatching/filter and select',
                            # bitmap = self.get_icon('icon_results_24px.png'),#,
                            info='Filter and select GPS trips.'
                            )

        menubar.append_item('plugins/mapmatching/filter and select/select traces by geometry...',
                            self.on_geomfilter_trips,
                            #bitmap = wx.ArtProvider.GetBitmap(wx.ART_FILE_SAVE_AS,wx.ART_MENU),
                            )

        menubar.append_item('plugins/mapmatching/filter and select/filter trips...',
                            self.on_postmatchfilter_trips,
                            #bitmap = wx.ArtProvider.GetBitmap(wx.ART_FILE_SAVE_AS,wx.ART_MENU),
                            )

        menubar.append_item('plugins/mapmatching/filter and select/select all trips',
                            self.on_select_all_trips,
                            )

        menubar.append_item('plugins/mapmatching/filter and select/unselect all trips',
                            self.on_unselect_all_trips,
                            )

        menubar.append_item('plugins/mapmatching/filter and select/invert selected trips',
                            self.on_invert_selected_trips,
                            )
# -------------------------------------------------------------------------------

        menubar.append_menu('plugins/mapmatching/delete',
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_DELETE, wx.ART_MENU),
                            info='Delete tools.'
                            )

        menubar.append_item('plugins/mapmatching/delete/delete unselected trips',
                            self.on_delete_unselected,
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_DELETE, wx.ART_MENU),
                            )

        menubar.append_item('plugins/mapmatching/delete/delete routes',
                            self.on_clear_routes,
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_DELETE, wx.ART_MENU),
                            )

        menubar.append_item('plugins/mapmatching/delete/delete all',
                            self.on_clear_all,
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_DELETE, wx.ART_MENU),
                            )

# -------------------------------------------------------------------------------

        menubar.append_item('plugins/mapmatching/redraw GPS data',
                            self.on_redraw,
                            )

    def on_plot_routeresults(self, event=None):
        """
        Plot route results of route analysis in Matplotlib plotting envitonment.
        """
        if is_mpl:
            resultplotter = results_mpl.RouteresultPlotter(self._results,
                                                           logger=self._mainframe.get_logger()
                                                           )
            dlg = results_mpl.ResultDialog(self._mainframe, resultplotter)

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

    def on_plot_edgeresults(self, event=None):
        """
        Plot edge results of route analysis in Matplotlib plotting envitonment.
        """
        if is_mpl:
            resultplotter = results_mpl.EdgeresultPlotter(self._results,
                                                          logger=self._mainframe.get_logger()
                                                          )
            dlg = results_mpl.ResultDialog(self._mainframe, resultplotter)

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

    def on_plot_noderesults(self, event=None):
        """
        Plot node results of route analysis in Matplotlib plotting envitonment.
        """
        if is_mpl:
            resultplotter = results_mpl.NoderesultPlotter(self._results,
                                                          logger=self._mainframe.get_logger()
                                                          )
            dlg = results_mpl.ResultDialog(self._mainframe, resultplotter)

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

    def on_plot_speedprofiles(self, event=None):
        """
        Plot speedprofiles of route analysis in Matplotlib plotting envitonment.
        """
        if is_mpl:
            resultplotter = results_mpl.SpeedprofilePlotter(self._results,
                                                            logger=self._mainframe.get_logger()
                                                            )
            dlg = results_mpl.ResultDialog(self._mainframe, resultplotter)

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

    def on_routes_to_shapefile(self, event=None):
        """
        Export route results to shape file.
        """
        # print 'on_routes_to_shapefile'
        scenario = self._mapmatching.get_scenario()
        dirpath = scenario.get_workdirpath()
        defaultFile = scenario.get_rootfilename()+'.routeres.shp'
        wildcards_all = 'All files (*.*)|*.*|SHP files (*.shp)|*.shp'
        dlg = wx.FileDialog(None, message='Export route results to shapefile',
                            defaultDir=dirpath, defaultFile=defaultFile,
                            wildcard=wildcards_all, style=wx.SAVE | wx.CHANGE_DIR)
        if dlg.ShowModal() == wx.ID_OK:
            filepath = dlg.GetPath()

        else:
            return

        mapmatching.routes_to_shapefile(self._mapmatching,
                                        self._results,
                                        filepath,
                                        log=self._mainframe.get_logger())

    def on_edgesresults_to_shapefile(self, event=None):
        """
        Export edge results to shape file.
        """
        print 'on_nodes_to_shapefile'
        scenario = self._mapmatching.get_scenario()
        dirpath = scenario.get_workdirpath()
        defaultFile = scenario.get_rootfilename()+'.edgeres.shp'
        wildcards_all = 'All files (*.*)|*.*|SHP files (*.shp)|*.shp'
        dlg = wx.FileDialog(None, message='Export edge results to shapefile',
                            defaultDir=dirpath, defaultFile=defaultFile,
                            wildcard=wildcards_all, style=wx.SAVE | wx.CHANGE_DIR)
        if dlg.ShowModal() == wx.ID_OK:
            filepath = dlg.GetPath()

        else:
            return

        mapmatching.edgesresults_to_shapefile(self._mapmatching,
                                              self._results,
                                              filepath,
                                              log=self._mainframe.get_logger())

    def on_points_to_shapefile(self, event=None):
        """
        Export GPS points to shapefile.
        """
        print 'on_points_to_shapefile'
        scenario = self._mapmatching.get_scenario()
        dirpath = scenario.get_workdirpath()
        defaultFile = scenario.get_rootfilename()+'.points.shp'
        wildcards_all = 'All files (*.*)|*.*|SHP files (*.shp)|*.shp'
        dlg = wx.FileDialog(None, message='Export GPS points to shapefile',
                            defaultDir=dirpath, defaultFile=defaultFile,
                            wildcard=wildcards_all, style=wx.SAVE | wx.CHANGE_DIR)
        if dlg.ShowModal() == wx.ID_OK:
            filepath = dlg.GetPath()

        else:
            return

        mapmatching.points_to_shapefile(self._mapmatching,
                                        filepath,
                                        log=self._mainframe.get_logger())

    def on_save_results(self, event=None):
        """
        Save mapmatching analysis results to binary file.
        """
        if self._results is None:
            return
        scenario = self.get_scenario()
        wildcards_all = "All files (*.*)|*.*"
        wildcards_obj = "Python binary mapmatch files (*.mmatch.obj)|*.mmatch.obj|Python binary files (*.obj)|*.obj"
        wildcards = wildcards_obj+"|"+wildcards_all

        # Finally, if the directory is changed in the process of getting files, this
        # dialog is set up to change the current working directory to the path chosen.
        dlg = wx.FileDialog(
            self._mainframe, message="Save results to file",
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
                self._results.save(filepath)

        # Destroy the dialog. Don't do this until you are done with it!
        # BAD things can happen otherwise!
        dlg.Destroy()

    def on_open_results(self, event=None):

        wildcards_all = "All files (*.*)|*.*"
        wildcards_obj = "Python binary mapmatch files (*.mmatch.obj)|*.mmatch.obj|Python binary files (*.obj)|*.obj"
        wildcards = wildcards_obj+"|"+wildcards_all

        # Finally, if the directory is changed in the process of getting files, this
        # dialog is set up to change the current working directory to the path chosen.
        dlg = wx.FileDialog(
            self._mainframe, message="Open results file",
            defaultDir=self.get_scenario().get_workdirpath(),
            #defaultFile = os.path.join(scenario.get_workdirpath(), scenario.format_ident()+'.obj'),
            wildcard=wildcards,
            style=wx.OPEN | wx.CHANGE_DIR
        )

        # Show the dialog and retrieve the user response. If it is the OK response,
        # process the data.
        is_newresults = False
        if dlg.ShowModal() == wx.ID_OK:
            # This returns a Python list of files that were selected.
            filepath = dlg.GetPath()
            if len(filepath) > 0:
                if self._results is not None:
                    # browse away from results
                    # self._mainframe.browse_obj(self._results.get_scenario())
                    del self._results

                self._results = mapmatching.load_results(filepath,
                                                         parent=self._mapmatching,
                                                         logger=self._mainframe.get_logger()
                                                         )
                is_newresults = True

        # Destroy the dialog. Don't do this until you are done with it!
        # BAD things can happen otherwise!
        dlg.Destroy()

        if is_newresults:
            # this should update all widgets for the new scenario!!
            # print 'call self._mainframe.refresh_moduleguis()'
            self._mainframe.browse_obj(self._results)
            # self._mainframe.select_view(name = "Result viewer") #!!!!!!!!tricky, crashes without
            self._is_needs_refresh = True
            self.refresh_widgets()
            # wx.CallAfter(self.refresh_widgets)
            # self._mainframe.refresh_moduleguis()
            #if event: event.Skip()

    def on_select_all_trips(self, event=None):
        """
        Select all GPS trips.
        """
        self._mapmatching.trips.select_all()
        self._mainframe.browse_obj(self._mapmatching.trips)
        self._is_needs_refresh = True
        self.refresh_widgets()

    def on_unselect_all_trips(self, event=None):
        """
        Unselect all GPS trips.
        """
        self._mapmatching.trips.unselect_all()
        self._mainframe.browse_obj(self._mapmatching.trips)
        self._is_needs_refresh = True
        self.refresh_widgets()

    def on_invert_selected_trips(self, event=None):
        """
        Invert selected GPS trips, all selected will be unselected and vice versa.
        """
        self._mapmatching.trips.invert_selection()
        self._mainframe.browse_obj(self._mapmatching.trips)
        self._is_needs_refresh = True
        self.refresh_widgets()

    def on_clear_all(self, event=None):
        """
        Clear all GPS points, routes and persons.
        """
        self._mapmatching.clear_all()
        self._mainframe.browse_obj(self._mapmatching)
        self._is_needs_refresh = True
        self.refresh_widgets()

    def on_clear_routes(self, event=None):
        """
        Clear matched routes and minimal distance routes.
        """
        self._mapmatching.clear_routes()
        self._mainframe.browse_obj(self._mapmatching)
        self._is_needs_refresh = True
        self.refresh_widgets()

    def on_delete_unselected(self, event=None):
        """
        Delete unselected trips.
        """
        self._mapmatching.delete_unselected_trips()
        self._mainframe.browse_obj(self._mapmatching.trips)
        #self._is_needs_refresh = True
        # self.refresh_widgets()

    def is_matchprocess(self, ident):
        if self._matchprocess is None:
            return False
        else:
            return self._matchprocess.ident == ident

    def on_match_birgil(self, event=None):
        """
        Match selected traces with Birgillito's method. 
        """
        # self.prepare_results()
        if not self.is_matchprocess('birgilmatcher'):
            self._matchprocess = mapmatching.BirgilMatcher('birgilmatcher',
                                                           self._mapmatching,
                                                           logger=self._mainframe.get_logger(),
                                                           )

        dlg = ProcessDialogInteractive(self._mainframe,
                                       self._matchprocess,
                                       #title = 'Mapmatching with Birgillito method',
                                       func_close=self.close_match_birgil,
                                       )

        dlg.CenterOnScreen()

        # this does not return until the dialog is closed.
        #val = dlg.ShowModal()
        # print 'open_sumodialog_interactive'
        dlg.Show()
        dlg.MakeModal(True)
        # print '  val,val == wx.ID_OK',val,wx.ID_OK,wx.ID_CANCEL,val == wx.ID_CANCEL
        # print '  status =',dlg.get_status()
        # print 'returned to main window self.simulator.status',self.simulator.status

    def close_match_birgil(self, dlg):
        # called before destroying the dialog
        if dlg.get_status() == 'success':
            #p = self._mapmatchprocess

            self._mainframe.browse_obj(self._mapmatching.trips)
            self._is_needs_refresh = True
            self.refresh_widgets()

    def on_geomfilter_trips(self, event=None):
        """
        Select GPS traces to satisfy geometric requirements.
        This should be done before the mapmatching process. 
        """
        p = mapmatching.TripGeomfilter(self._mapmatching, logger=self._mainframe.get_logger())
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
            self._is_needs_refresh = True
            self.refresh_widgets()

    def on_postmatchfilter_trips(self, event=None):
        """
        Select trips by different parameters to ensure the quality of the mapmatching results.
        This should be done after the map-matching process. 
        """
        p = mapmatching.PostMatchfilter(self._mapmatching, logger=self._mainframe.get_logger())
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
            self._is_needs_refresh = True
            self.refresh_widgets()

    def on_route_shortest(self, event=None):
        """
        Shortest path routing of matched routes.
        """
        p = mapmatching.Shortestrouter('shortestpathrouter', self._mapmatching, logger=self._mainframe.get_logger())
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
            self._is_needs_refresh = True
            self.refresh_widgets()

    def on_route_fastest(self, event=None):
        """
        Fastest path routing of matched routes.
        """
        p = mapmatching.Fastestrouter('fastestpathrouter', self._mapmatching,
                                      matchresults=self._results,
                                      logger=self._mainframe.get_logger()
                                      )
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
            self._is_needs_refresh = True
            self.refresh_widgets()

    def on_redraw(self, event=None):
        self._mainframe.browse_obj(self._mapmatching)
        self._is_needs_refresh = True
        self.refresh_widgets()

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
            self._is_needs_refresh = True
            self.refresh_widgets()

    def on_import_gpx(self, event=None):
        """
        Import and filter data from GPX file. 
        """
        p = mapmatching.GpxImporter(self._mapmatching, logger=self._mainframe.get_logger())
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
            self._is_needs_refresh = True
            self.refresh_widgets()

    def on_project_points(self, event=None):
        self._mapmatching.points.project()
        self._mainframe.browse_obj(self._mapmatching.points)

        if event:
            event.Skip()

    def on_browse(self, event=None):

        self._mainframe.browse_obj(self._mapmatching)
        if event:
            event.Skip()

    def on_browse_results(self, event=None):
        """
        Browse mapmatching analyses results
        """
        self._mainframe.browse_obj(self._results)
        if event:
            event.Skip()

    def on_analyze_persons(self, event=None):
        """
        Analyze the trips of each person in the database.
        Ensure that mapmatching and shortest trip routing 
        has been previously executed.
        """

        self._mapmatching.persons.analyze()
        self._mainframe.browse_obj(self._mapmatching.persons)

    def on_export_persons_csv(self, event=None):
        if self._results is None:
            return
        scenario = self._results.get_scenario()
        wildcards_all = "All files (*.*)|*.*"
        wildcards_obj = "CSV files (*.csv)|*.csv|Text file (*.txt)|*.txt"
        wildcards = wildcards_obj+"|"+wildcards_all

        # Finally, if the directory is changed in the process of getting files, this
        # dialog is set up to change the current working directory to the path chosen.
        dlg = wx.FileDialog(
            self._mainframe, message="Export persons to CSV file",
            defaultDir=scenario.get_workdirpath(),
            defaultFile=scenario.get_rootfilepath()+'.gpspersons.csv',
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
                persons = self._mapmatching.persons
                ids_pers = persons.select_ids(persons.lengths_tot_route_matched.get_value() > 0)
                self._mapmatching.persons.export_csv(filepath, ids=ids_pers)

        # Destroy the dialog. Don't do this until you are done with it!
        # BAD things can happen otherwise!
        dlg.Destroy()

    def on_routeanalyze(self, event=None):
        """
        Analyze attributes of matched and alternative routes. 
        """
        p = mapmatching.Routesanalyzer('routeanalyzer',
                                       self._mapmatching,
                                       self._results,
                                       logger=self._mainframe.get_logger())

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
            self._mainframe.browse_obj(self._results)
            self._is_needs_refresh = True
            self.refresh_widgets()
