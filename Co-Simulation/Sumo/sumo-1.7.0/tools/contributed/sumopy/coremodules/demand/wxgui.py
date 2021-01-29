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
from agilepy.lib_wx.processdialog import ProcessDialog

from coremodules.network import routing
from coremodules.scenario.scenario import load_scenario

import demand
import turnflows
import origin_to_destination_wxgui as odgui
import turnflows_wxgui as turnflowsgui
import virtualpop_wxgui as virtualpopgui
import publictransportservices_wxgui as pt


class TripDrawings(Polygons):
    def __init__(self, facilities, parent,   **kwargs):

        Polygons.__init__(self, 'facilitydraws',  parent,
                          name='Facility drawings',
                          linewidth=1,
                          detectwidth=3.0,
                          **kwargs)

        self.delete('vertices')

        self.add(cm.AttrConf('color_facility_default', np.array([0.921875,  0.78125,  0.4375, 1.0], np.float32),
                             groupnames=['options'],
                             perm='wr',
                             metatype='color',
                             name='Default color',
                             info='Default facility color.',
                             ))

        self.set_facilities(facilities)

    def get_netelement(self):
        return self._facilities

    def get_vertices_array(self):
        return self._facilities.shapes.value[self._inds_map]

    def set_facilities(self, facilities):
        # print '\nset_facilities',facilities.get_ids()
        self._facilities = facilities
        if len(self) > 0:
            self.del_rows(self.get_ids())

        ids = self._facilities.get_ids()
        self._inds_map = self._facilities.get_inds(ids)
        self.add_rows(ids=ids)
        self.update()

    def update(self, is_update=True):
        # assumes that arrsy structure did not change
        # print 'FacilityDrawings.update'
        n = len(self)
        self.colors.value[:] = np.ones((n, 4), np.float32)*self.color_facility_default.value
        self.colors_highl.value[:] = self._get_colors_highl(self.colors.value)

        landusetypes = self._facilities.get_landusetypes()
        ids_landusetype = self._facilities.ids_landusetype.value
        for id_landusetype in landusetypes.get_ids():
            # inds = np.flatnonzero(ids_landusetype == id_landusetype)
            # color = landusetypes.colors[id_landusetype]
            # self.colors.value[self._inds_map[inds]] = color
            self.colors.value[self._inds_map[np.flatnonzero(
                ids_landusetype == id_landusetype)]] = landusetypes.colors[id_landusetype]

        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()


class RouteDrawings(Polygons):
    def __init__(self, zones, parent,   **kwargs):

        Polygons.__init__(self, 'zonedraws',  parent,
                          name='Transport assignment zone drawings',
                          linewidth=4,
                          detectwidth=5.0,
                          **kwargs)

        self.delete('vertices')

        self.add(cm.AttrConf('color_default', np.array([0.3, 0.9, 0.5, 0.8], dtype=np.float32),
                             groupnames=['options'],
                             perm='wr',
                             metatype='color',
                             name='Default color',
                             info='Default facility color.',
                             ))

        self.set_netelement(zones)

    def get_netelement(self):
        return self._zones

    def get_vertices_array(self):
        if self._is_not_synched:
            self._inds_map = self._zones.get_inds(self._zones.get_ids())
            self._is_not_synched = False
        return self._zones.shapes.value[self._inds_map]

    def set_netelement(self, zones):
        # print '\nset_facilities',facilities.get_ids()
        self._zones = zones
        if len(self) > 0:
            self.del_rows(self.get_ids())

        ids = self._zones.get_ids()
        # self._inds_map = self._zones.get_inds(ids)
        self.add_rows(ids=ids)

        # plugins to keep grapgics syncronized with netelements
        zones.shapes.plugin.add_event(cm.EVTADDITEM, self.on_add_element)
        zones.plugin.add_event(cm.EVTDELITEM, self.on_del_element)

        self._id_target = -1
        self.update()

    def update(self, is_update=True):
        # assumes that arrsy structure did not change
        # print 'FacilityDrawings.update'
        n = len(self)
        self.colors.value[:] = np.ones((n, 4), np.float32)*self.color_default.value
        self.colors_highl.value[:] = self._get_colors_highl(self.colors.value)

        if is_update:
            self.update_internal()

    def update_internal(self):
        # print 'update_internal'
        # print '    len(self),len(self._zones)',len(self),len(self._zones)
        self._is_not_synched = True
        self._update_vertexvbo()
        self._update_colorvbo()

    def make_zone(self, shape, color):
        # print 'make_zone shape',shape,type(shape)
        return self._zones.make(shape=shape)

    def on_add_element(self, shapes, ids):
        # print 'on_add_element',shapes.attrname,ids
        if shapes == self._zones.shapes:
            self._id_target = ids[0]
            self.add_row(_id=self._id_target,
                         colors=self.color_default.value,
                         colors_highl=self._get_colors_highl(self.color_default.value)
                         )
            # self._inds_map = self._zones.get_inds(self._zones.get_ids())
            # self._update_vertexvbo()
            # self._update_colorvbo()

    def begin_animation(self, id_target):
        # print 'ZoneDrawings.begin_animation zones.shapes=\n',id_target,self._id_target, self._zones.shapes[id_target]
        if self._id_target == -1:
            self._id_target = id_target
        self._drawobj_anim = self.parent.get_drawobj_by_ident(self._ident_drawobj_anim)
        self.id_anim = self._drawobj_anim.add_drawobj(self._zones.shapes[self._id_target],
                                                      self.color_anim.value,
                                                      )
        # print 'begin_animation',self.ident,_id,self._drawobj_anim
        return True

    def end_animation(self, is_del_last_vert=False):
        # print 'ZoneDrawings.end_animation',self.ident,self._id_target,self.id_anim

        # print '  verices =',self._drawobj_anim.vertices[self.id_anim]
        # print '  self._drawobj_anim.vertices[self.id_anim]=',self._drawobj_anim.vertices[self.id_anim]
        shape = self._drawobj_anim.vertices[self.id_anim]
        self._zones.shapes[self._id_target] = shape
        self._zones.coords[self._id_target] = self._zones.get_coords_from_shape(shape)

        self.del_animation()
        # print '  self.get_vertices_array()=\n',self.get_vertices_array()
        # self._drawobj_anim.del_drawobj(self.id_anim)
        self.update_internal()
        # self._inds_map = self._zones.get_inds(self._zones.get_ids())
        # self._update_vertexvbo()
        # self._update_colorvbo()
        return True

    def del_elem(self, id_zone):
        """
        Deletes an element from network and then in on canvas
        through callback on_del_element
        """
        # print 'del_elem'
        # print '    len(self),len(self._zones)',len(self),len(self._zones)

        self._zones.del_element(id_zone)

    def on_del_element(self, shapes, ids):
        """
        callback from netelement
        """
        # print 'on_del_element',shapes.attrname,ids,self._id_target
        # print '    len(self),len(self._zones)',len(self),len(self._zones)
        # print '  verices =',self._drawobj_anim.vertices[self.id_anim]
        self._is_not_synched = True
        self.del_drawobj(ids[0])
        # print '    len(self),len(self._zones)',len(self),len(self._zones)
        # wx.CallAfter(self.update_internal)
        # self.update_internal()
        # print '  after CallAfter'
        # print '    len(self),len(self._zones)',len(self),len(self._zones)
        return True

    def del_animation(self, is_del_main=False):
        # print 'end_animation',self.ident,_id,self._drawobj_anim
        self._drawobj_anim.del_drawobj(self.id_anim)
        self._drawobj_anim = None

        if is_del_main:
            # self.del_drawobj(self._id_target)
            # delete first element from net, which will
            # call back on_del_netelement where the main drawobj is deleted
            self.del_elem(self._id_target)

        self._id_target = -1
        self.id_anim = -1
        return True


class WxGui(turnflowsgui.TurnflowWxGuiMixin,
            odgui.OdFlowsWxGuiMixin,
            virtualpopgui.VirtualpopWxGuiMixin,
            pt.PublicTransportWxGuiMixin,
            ModuleGui):

    """Contains functions that communicate between the widgets of the main wx gui
    and the functions of the plugin.
    """

    def __init__(self, ident):
        self._demand = None
        self._init_common(ident,  priority=1000,
                          icondirpath=os.path.join(os.path.dirname(__file__), 'images'))

    def get_module(self):
        return self._demand

    def get_scenario(self):
        return self._mainframe.get_modulegui('coremodules.scenario').get_scenario()

    def get_neteditor(self):
        return self._mainframe.get_modulegui('coremodules.network').get_neteditor()

    def init_widgets(self, mainframe):
        """
        Set mainframe and initialize widgets to various places.
        """
        self._mainframe = mainframe
        # self._neteditor = mainframe.add_view("Network", Neteditor)

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
            self._demand = scenario.demand
            is_refresh = True
            # self.get_neteditor().get_toolbox().add_toolclass(AddZoneTool)
            # print '  odintervals',self._demand.odintervals#,self.odintervals.times_start
            # print ' ',dir(self._demand.odintervals)

        elif self._demand.is_modified():
            is_refresh = True

        self.refresh_odflow(is_refresh)
        self.refresh_turnflow(is_refresh)
        self.refresh_pt(is_refresh)

    def make_menu(self):
        # print 'make_menu'
        menubar = self._mainframe.menubar
        menubar.append_menu('demand')

        menubar.append_item('demand/browse',
                            self.on_browse_obj,  # common function in modulegui
                            info='View and browse demand in object panel.',
                            bitmap=self.get_agileicon('icon_browse_24px.png'),  # ,
                            )
        # ----------------------------------------------------------------------
        menubar.append_menu('demand/vehicles',
                            bitmap=self.get_icon("vehicle_24px.png"),
                            )

        menubar.append_item('demand/vehicles/clear vehicle types',
                            self.on_clear_vtypes,
                            info='Remove all vehicle types .',
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_DELETE, wx.ART_MENU),
                            )

        menubar.append_item('demand/vehicles/import vehicle types from xml...',
                            self.on_import_vtypes_xml,
                            info='Import vehicle types from xml file',
                            bitmap=self.get_agileicon("Document_Import_24px.png"),
                            )

        menubar.append_item('demand/vehicles/load defaults',
                            self.on_load_vtypes_defaults,
                            info='Load default vehicle types, removing all existing vehicle types.',
                            # bitmap = self.get_icon("route3_24px.png"),
                            )

        # ----------------------------------------------------------------------
        self.add_menu_odflows(menubar)
        # ----------------------------------------------------------------------
        self.add_menu_turnflow(menubar)
        # ----------------------------------------------------------------------
        self.add_menu_pt(menubar)
        # ----------------------------------------------------------------------

        menubar.append_menu('demand/trips and routes',
                            bitmap=self.get_icon("trip3_24px.png"),
                            )

        menubar.append_item('demand/trips and routes/trips to routes with fastest path',
                            self.on_route,
                            bitmap=self.get_icon("route3_24px.png"),
                            )
        menubar.append_item('demand/trips and routes/trips to routes with fastest path, del. disconnected',
                            self.on_route_del_disconnected,
                            bitmap=self.get_icon("route3_24px.png"),
                            )

        menubar.append_item('demand/trips and routes/trips to routes with duarouter',
                            self.on_duaroute,
                            bitmap=self.get_icon("route3_24px.png"),
                            )

        # menubar.append_item( 'demand/trips and routes/trips to routes with uncongested net ...',
        #    self.on_route_uncongested,
        #    bitmap = self.get_icon("route3_24px.png"),
        #    )

        # menubar.append_item( 'demand/trips and routes/trips to routes with congested net ...',
        #    self.on_route_congested,
        #    bitmap = self.get_icon("route3_24px.png"),
        #    )

        menubar.append_item('demand/trips and routes/export trips to SUMO xml...',
                            self.on_export_sumotrips,
                            info='Export all trips to SUMO XML format.',
                            bitmap=self.get_agileicon("Document_Export_24px.png"),
                            )

        # menubar.append_item( 'demand/trips and routes/export routes to SUMO xml...',
        #    self.on_export_sumoroutes,
        #    info='Export all trips to SUMO XML format.',
        #    bitmap = self.get_agileicon("Document_Export_24px.png"),
        #    )

        menubar.append_item('demand/trips and routes/import trips from SUMO xml...',
                            self.on_import_trips_xml,
                            info=self.on_import_trips_xml.__doc__.strip(),
                            bitmap=self.get_agileicon("Document_Import_24px.png"),
                            )

        menubar.append_item('demand/trips and routes/import trips with routes from SUMO xml...',
                            self.on_import_triproutes_xml,
                            info=self.on_import_triproutes_xml.__doc__.strip(),
                            bitmap=self.get_agileicon("Document_Import_24px.png"),
                            )

        menubar.append_item('demand/trips and routes/import routes to existing trips from SUMO xml...',
                            self.on_import_routes_alternative_xml,
                            info=self.on_import_routes_alternative_xml.__doc__.strip(),
                            bitmap=self.get_agileicon("Document_Import_24px.png"),
                            )

        menubar.append_item('demand/trips and routes/import trips from scenario...',
                            self.on_import_trips_from_scenario,
                            info='Import trips from another scenario',
                            bitmap=self.get_agileicon("Document_Import_24px.png"),
                            )

        menubar.append_item('demand/trips and routes/clear all trips and routes',
                            self.on_clear_trips,
                            info='Clear all trips with respective routes.',
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_DELETE, wx.ART_MENU),
                            )

        menubar.append_item('demand/trips and routes/clear all routes',
                            self.on_clear_routes,
                            info='Clear all routes, trips will remain.',
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_DELETE, wx.ART_MENU),
                            )

        # ----------------------------------------------------------------------

        self.add_menu_virtualpop(menubar)

        # ----------------------------------------------------------------------
        menubar.append_item('demand/export all routes to SUMO xml...',
                            self.on_export_all_sumoroutes,
                            info='Export all routes available in the demand database to SUMO XML format.',
                            bitmap=self.get_agileicon("Document_Export_24px.png"),
                            )


# (self._menuitem_draw_route, id_item,) = menubar.append_item(
#            'plugins/traces/draw selected route in network',
# self.on_renew_objectbrowser,
#            info='Enable visualization of routes in network tab. Double-click on trace in table.',
# check=True)
# self._menuitem_draw_route.Check(False)
#
# (self._menuitem_plot_route, id_item,) = menubar.append_item(
#            'plugins/traces/add plot selected route',
# self.on_renew_objectbrowser,
#            info='Enable adding of routes to graphics in trace tab. Double-click on trace in table.',
# check=True)
# self._menuitem_plot_route.Check(False)

    def on_clear_vtypes(self, event=None):
        self._demand.vtypes.clear_vtypes()
        self._mainframe.browse_obj(self._demand.vtypes)
        # if event:  event.Skip()

    def on_load_vtypes_defaults(self, event=None):
        self._demand.vtypes.clear_vtypes()
        self._demand.vtypes.add_vtypes_default()
        self._mainframe.browse_obj(self._demand.vtypes)
        # if event:  event.Skip()

    def on_clear_trips(self, event=None):
        self._demand.trips.clear_trips()
        self._mainframe.browse_obj(self._demand.trips)
        # if event:  event.Skip()

    def on_clear_routes(self, event=None):
        self._demand.trips.clear_routes()
        self._mainframe.browse_obj(self._demand.trips)
        # if event:  event.Skip()

    def on_route(self, event=None):
        """Generates routes from current trip info. Uses a python implementation of a fastest path router.
        """
        # self._demand.trips.clear_routes()
        self._demand.trips.route()
        self._mainframe.browse_obj(self._demand.trips)

    def on_route_del_disconnected(self, event=None):
        """Generates routes from current trip info and deletes disconnected trips. Uses a python implementation of
        a fastest path router.
        """
        # self._demand.trips.clear_routes()
        self._demand.trips.route(is_del_disconnected=True)
        self._mainframe.browse_obj(self._demand.trips)

    def on_duaroute(self, event=None):
        """Generates routes from current trip info. Uses simple shortest path routing based on duarouter.
        """
        # self._demand.trips.clear_routes()
        self._demand.trips.duaroute(is_export_net=True, is_export_trips=True)
        self._mainframe.browse_obj(self._demand.trips)

    def on_route_uncongested(self, event=None):
        """Generates routes from current trip info using routing methods with uncongested network assumption.
        Based on duarouter.
        """
        # self._demand.trips.clear_routes()

        obj = routing.DuaRouter(self.get_scenario().net,
                                trips=self._demand.trips,
                                logger=self._mainframe.get_logger())

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

            self._mainframe.browse_obj(self._demand.trips)

            # this should update all widgets for the new scenario!!
            # print 'call self._mainframe.refresh_moduleguis()'
            # self._mainframe.refresh_moduleguis()

    def on_route_congested(self, event=None):
        """Generates routes from current trip info using routing methods with congested network assumption.
        Based on marouter.
        """
        # self._demand.trips.clear_routes()

        obj = routing.MacroRouter(self.get_scenario().net,
                                  trips=self._demand.trips,
                                  logger=self._mainframe.get_logger())

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

            self._mainframe.browse_obj(self._demand.trips)

            # this should update all widgets for the new scenario!!
            # print 'call self._mainframe.refresh_moduleguis()'
            # self._mainframe.refresh_moduleguis()

    def on_import_vtypes_xml(self, event=None):
        """Select xml file and import new vehicle types.
        """
        filepath = self._demand.trips.get_routefilepath()
        defaultFile = os.path.dirname(filepath)
        dirpath = os.path.dirname(filepath)
        wildcards_all = 'XML files (*.xml)|*.xml|All files (*.*)|*.*'
        dlg = wx.FileDialog(None, message='Import vtypes from SUMO xml',
                            defaultDir=dirpath,
                            defaultFile=defaultFile,
                            wildcard=wildcards_all, style=wx.SAVE | wx.CHANGE_DIR)
        if dlg.ShowModal() == wx.ID_OK:
            filepath = dlg.GetPath()

        else:
            return

        self._demand.vtypes.import_xml(filepath)
        self._mainframe.browse_obj(self._demand.vtypes)

    def on_import_trips_xml(self, event=None):
        """Select xml file and import new trips.
        New trips with associated routes will generated.
        """
        filepath = self._demand.trips.get_routefilepath()
        defaultFile = os.path.dirname(filepath)
        dirpath = os.path.dirname(filepath)
        wildcards_all = 'XML trip files (*.trip.xml)|*.trip.xml|All files (*.*)|*.*'
        dlg = wx.FileDialog(None, message='Import trips and routes from SUMO xml',
                            defaultDir=dirpath,
                            defaultFile=defaultFile,
                            wildcard=wildcards_all, style=wx.SAVE | wx.CHANGE_DIR)
        if dlg.ShowModal() == wx.ID_OK:
            filepath = dlg.GetPath()

        else:
            return

        self._demand.trips.import_trips_xml(filepath, is_clear_trips=False,
                                            is_generate_ids=True)
        self._mainframe.browse_obj(self._demand.trips)

    def on_import_triproutes_xml(self, event=None):
        """Select xml file and import trips and routes.
        New trips with associated routes will generated.
        """
        filepath = self._demand.trips.get_routefilepath()
        defaultFile = os.path.dirname(filepath)
        dirpath = os.path.dirname(filepath)
        wildcards_all = 'XML route files (*.rou.xml)|*.rou.xml|All files (*.*)|*.*'
        dlg = wx.FileDialog(None, message='Import trips and routes from SUMO xml',
                            defaultDir=dirpath,
                            defaultFile=defaultFile,
                            wildcard=wildcards_all, style=wx.SAVE | wx.CHANGE_DIR)
        if dlg.ShowModal() == wx.ID_OK:
            filepath = dlg.GetPath()

        else:
            return

        self._demand.trips.import_routes_xml(filepath, is_clear_trips=False,
                                             is_generate_ids=True,
                                             is_add=False)
        self._mainframe.browse_obj(self._demand.trips)

    def on_import_routes_alternative_xml(self, event=None):
        """Select xml file and import routes.
        Routes will added as route alternative to existing trips.
        """
        filepath = self._demand.trips.get_routefilepath()
        defaultFile = os.path.dirname(filepath)
        dirpath = os.path.dirname(filepath)
        wildcards_all = 'XML route files (*.rou.xml)|*.rou.xml|All files (*.*)|*.*'
        dlg = wx.FileDialog(None, message='Import trips and routes from SUMO xml',
                            defaultDir=dirpath,
                            defaultFile=defaultFile,
                            wildcard=wildcards_all, style=wx.OPEN | wx.CHANGE_DIR
                            )

        if dlg.ShowModal() == wx.ID_OK:
            filepath = dlg.GetPath()

        else:
            return

        self._demand.trips.import_routes_xml(filepath, is_clear_trips=False,
                                             is_generate_ids=False,
                                             is_add=True)
        self._mainframe.browse_obj(self._demand.trips)

    def on_import_trips_from_scenario(self, event=None):
        """Select xml file and import new vehicle types.
        """
        filepath = self.get_scenario().get_rootfilepath()
        # defaultFile = os.path.dirname(filepath)
        dirpath = os.path.dirname(filepath)
        wildcards_all = "All files (*.*)|*.*"
        wildcards_obj = "Python binary files (*.obj)|*.obj"
        wildcards = wildcards_obj+"|"+wildcards_all
        dlg = wx.FileDialog(
            self._mainframe, message="Open scenario file",
            defaultDir=dirpath,
            defaultFile=filepath+'.obj',
            wildcard=wildcards,
            style=wx.OPEN | wx.CHANGE_DIR
        )
        if dlg.ShowModal() == wx.ID_OK:
            filepath = dlg.GetPath()

        else:
            return
        scenario2 = load_scenario(filepath, logger=None)
        self._demand.trips.import_trips_from_scenario(scenario2)
        self._mainframe.browse_obj(self._demand.trips)

    def on_export_sumotrips(self, event=None):
        # print 'on_export_sumotrips'
        filepath = self._demand.trips.get_tripfilepath()
        defaultFile = os.path.dirname(filepath)
        dirpath = os.path.dirname(filepath)
        wildcards_all = '*.*|XML trip files (*.trip.xml)|*.trip.xml|All files (*.*)'
        dlg = wx.FileDialog(None, message='Write trips to SUMO xml',
                            defaultDir=dirpath,
                            defaultFile=defaultFile,
                            wildcard=wildcards_all, style=wx.SAVE | wx.CHANGE_DIR)
        if dlg.ShowModal() == wx.ID_OK:
            filepath = dlg.GetPath()

        else:
            return

        self._demand.trips.export_trips_xml(filepath)

    # def on_export_sumoroutes(self, event = None):
    #    #print 'on_export_sumotrips'
    #    filepath = self._demand.trips.get_routefilepath()
    #    defaultFile = os.path.basename(filepath)
    #    dirpath = os.path.dirname(filepath)
    #    wildcards_all = 'XML route files (*rou.xml)|*rou.xml|All files (*.*)|*.*'
    #    dlg = wx.FileDialog(None, message='Write routes to SUMO xml',
    #                        defaultDir=dirpath,
    #                        defaultFile=defaultFile,
    #                        wildcard=wildcards_all, style=wx.SAVE | wx.CHANGE_DIR)
    #    if dlg.ShowModal() == wx.ID_OK:
    #        filepath = dlg.GetPath()
    #
    #    else:
    #        return
    #
    #    self._demand.trips.export_routes_xml(filepath)

    def on_export_all_sumoroutes(self, event=None):
        # print 'on_export_sumotrips'
        filepath = self._demand.get_routefilepath()
        defaultFile = os.path.basename(filepath)
        dirpath = os.path.dirname(filepath)
        wildcards_all = 'XML route files (*rou.xml)|*rou.xml|All files (*.*)|*.*'
        dlg = wx.FileDialog(None, message='Write all routes to SUMO xml',
                            defaultDir=dirpath,
                            defaultFile=defaultFile,
                            wildcard=wildcards_all, style=wx.SAVE | wx.CHANGE_DIR)
        if dlg.ShowModal() == wx.ID_OK:
            filepath = dlg.GetPath()

        else:
            return

        self._demand.export_routes_xml(filepath)
