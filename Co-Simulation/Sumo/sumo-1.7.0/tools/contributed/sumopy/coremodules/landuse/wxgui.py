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
import shutil
import tempfile

from agilepy.lib_wx.modulegui import ModuleGui
from agilepy.lib_wx.ogleditor import *
from agilepy.lib_wx.processdialog import ProcessDialog, ProcessDialogInteractive
import landuse
import maps

from coremodules.misc import shapeformat


class ParkingDrawings(Lines):
    def __init__(self, ident, parking, parent,   **kwargs):

        Lines.__init__(self, ident,  parent,
                       name='Parking drawings',
                       linewidth=3,
                       **kwargs)

        self.delete('vertices')

        self.add(cm.AttrConf('color_default', np.array([0.0, 0.0, 0.7, 1.0], np.float32),
                             groupnames=['options'],
                             perm='wr',
                             metatype='color',
                             name='Default color',
                             info='Default color.',
                             ))

        self.set_parking(parking)

    def get_netelement(self):
        return self._parking

    def get_vertices_array(self):
        # print 'get_vertices_array'
        # print '  vertices',self._parking.vertices.value
        return self._parking.vertices[self.get_ids()]  # .value[self._inds_map]

    def set_netelement(self, element):
        self.set_parking(element)

    def set_parking(self, parking):
        # print '\nset_parking',len(parking)
        self._parking = parking
        if len(self) > 0:
            self.clear_rows()  # self.del_rows(1*self.get_ids())

        ids = self._parking.get_ids()
        #self._inds_map = self._parking.get_inds(ids)
        self.add_rows(ids=ids)
        self.update()

    def update(self, is_update=True):
        # assumes that arrsy structure did not change
        # print 'FacilityDrawings.update'
        n = len(self)
        self.colors.value[:] = np.ones((n, 4), np.float32)*self.color_default.get_value()
        self.colors_highl.value[:] = self._get_colors_highl(self.colors.value)

        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()


class ZoneDrawings(Polygons):
    def __init__(self, ident, zones, parent,   **kwargs):

        Polygons.__init__(self, ident,  parent,
                          name='Transport assignment zone drawings',
                          linewidth=4,
                          **kwargs)

        self.delete('vertices')

        self.add(cm.AttrConf('color_zone', np.array([0.3, 0.9, 0.5, 0.8], dtype=np.float32),
                             groupnames=['options'],
                             perm='wr',
                             metatype='color',
                             name='TAZ color',
                             info='Default color of traffic assignment zone.',
                             ))

        self.set_netelement(zones)

    def get_netelement(self):
        return self._zones

    def get_vertices_array(self):
        # if self._is_not_synched:
        #    self._inds_map = self._zones.get_inds(self._zones.get_ids())
        #    self._is_not_synched = False
        return self._zones.shapes[self.get_ids()]  # .value[self._inds_map]

    def set_netelement(self, zones):
        # print '\nset_facilities',facilities.get_ids()
        self._zones = zones
        if len(self) > 0:
            self.clear_rows()  # self.del_rows(self.get_ids())

        ids = self._zones.get_ids()
        #self._inds_map = self._zones.get_inds(ids)
        self.add_rows(ids=ids)

        # plugins to keep grapgics syncronized with netelements
        zones.unplug()
        zones.shapes.plugin.add_event(cm.EVTADDITEM, self.on_add_element)
        zones.plugin.add_event(cm.EVTDELITEM, self.on_del_element)

        self._id_target = -1
        self.update()

    def update(self, is_update=True):
        # assumes that arrsy structure did not change
        self.update_colours(is_update=False)

        if is_update:
            self.update_internal()

    def update_colours(self, ids=None, is_update=True):
        if ids is None:
            ids = self.get_ids()

        inds = self._zones.get_inds(ids)
        # print 'update_colours',ids, inds, type(inds)
        # print '  ids_landusetype',self._zones.ids_landusetype.value
        landusetypes = self._zones.parent.landusetypes
        for id_landusetype in landusetypes.get_ids():
            # print '  is=',id_landusetype,self._zones.ids_landusetype.value[inds]==id_landusetype,ids
            ids_zone = ids[self._zones.ids_landusetype.value[inds] == id_landusetype]
            self.colors[ids_zone] = np.ones((len(ids_zone), 4), np.float32)*landusetypes.colors[id_landusetype]

        self.colors_highl.value[:] = self._get_colors_highl(self.colors.value)
        if is_update:
            self._update_colorvbo()

    def update_internal(self):
        # print 'update_internal'
        # print '    len(self),len(self._zones)',len(self),len(self._zones)
        #self._is_not_synched = True
        self._update_vertexvbo()
        self._update_colorvbo()

    def make_zone(self, shape, zonename='', id_landusetype=-1):
        # print 'make_zone shape',shape,type(shape)
        # print '  id_landusetype',id_landusetype
        return self._zones.make(zonename=zonename, shape=shape,
                                id_landusetype=id_landusetype)

    def on_add_element(self, shapes, ids):
        print 'on_add_element', shapes.attrname, ids
        if shapes == self._zones.shapes:
            self._id_target = ids[0]
            self.add_row(_id=self._id_target,
                         colors=self.color_zone.value,
                         colors_highl=self._get_colors_highl(self.color_zone.value)
                         )

            #self._inds_map = self._zones.get_inds(self._zones.get_ids())
            # self._update_vertexvbo()
            # self._update_colorvbo()

    def begin_animation(self, id_target):
        # print 'ZoneDrawings.begin_animation zones.shapes=\n',id_target,self._id_target, self._zones.shapes[id_target]
        if self._id_target == -1:
            self._id_target = id_target
        self._drawobj_anim = self.parent.get_drawobj_by_ident(self._ident_drawobj_anim)
        self.id_anim = self._drawobj_anim.add_drawobj(np.array(self._zones.shapes[self._id_target], np.float32).tolist(),
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
        self.update_colours(ids=np.array([self._id_target]))  # must pass np array
        self.del_animation()
        # print '  self.get_vertices_array()=\n',self.get_vertices_array()
        # self._drawobj_anim.del_drawobj(self.id_anim)

        self.update_internal()
        #self._inds_map = self._zones.get_inds(self._zones.get_ids())
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

    def on_del_element(self, obj, ids):
        """
        callback from netelement
        """
        if len(ids) != 1:
            return False
        # print 'on_del_element',obj,ids,self._id_target
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


class AddZoneTool(AddPolygonTool):
    """
    Mixin for Selection tools for OGL canvas.
    """

    def __init__(self, parent, mainframe=None):
        self.init_common('add_zone', parent, 'Add zone tool',
                         info='Click on canvas to add a zone. Add a vertex with a single click, double click to finish, right click to aboard.',
                         is_textbutton=False,
                         )
        self.init_options()

    def init_options(self):
        self.add(cm.AttrConf('zonename',  '',
                             groupnames=['options'],
                             perm='rw',
                             name='Zone name',
                             info='Name of zone. Must be unique. If left blanc, a zone number will be automatically assigned.',
                             ))

        self.add(cm.AttrConf('id_landusetype',  '',
                             groupnames=['options'],
                             choices={'': -1},
                             perm='rw',
                             name='Type',
                             info='Landusetype of this zone. Use Mixed when unknown or ambiguous.',
                             ))

    def activate(self, canvas=None):
        """
        This call by metacanvas??TooldsPallet signals that the tool has been
        activated and can now interact with metacanvas.
        """
        # print 'activate'
        AddPolygonTool.activate(self, canvas)

        landusetypes = self.get_landusetypes()
        typekeychoice = landusetypes.typekeys.get_indexmap().copy()
        typekeychoice[''] = -1
        # print '  typekeys',landusetypes.typekeys.value
        # print '  typekeychoice',typekeychoice
        self.id_landusetype.choices = typekeychoice
        self.id_landusetype.set_value(typekeychoice['mixed'])
        canvas.draw()

    def get_scenario(self):
        # get net and scenario via netdrawing
        return self.get_drawing().get_net().parent

    def get_landusetypes(self):
        return self.get_scenario().landuse.landusetypes

    def get_optionspanel(self, parent, size=(200, -1)):
        """
        Return tool option widgets on given parent
        """
        self._optionspanel = ObjPanel(parent, obj=self,
                                      id=None,
                                      attrconfigs=None,
                                      #tables = None,
                                      # table = None, id=None, ids=None,
                                      groupnames=['options'],
                                      func_change_obj=None,
                                      show_groupnames=False, show_title=True, is_modal=False,
                                      mainframe=self.parent.get_mainframe(),
                                      pos=wx.DefaultPosition, size=size, style=wx.MAXIMIZE_BOX | wx.RESIZE_BORDER,
                                      func_apply=self.on_apply_option,
                                      immediate_apply=False, panelstyle='default',  # 'instrumental'
                                      standartbuttons=['apply', 'restore'])

        return self._optionspanel

    def set_button_info(self, bsize=(32, 32)):
        # print 'set_button_info select tool'  self.get_icon("icon_sumo_24px.png")
        iconpath = os.path.join(os.path.dirname(__file__), 'images')
        self._bitmap = wx.Bitmap(os.path.join(iconpath, 'fig_zone_24px.png'), wx.BITMAP_TYPE_PNG)
        self._bitmap_sel = self._bitmap

    def begin_animation(self, event):
        # print 'AddLineTool.begin_animation'
        #self.drawobj_anim, _id, self.ind_vert =  self.get_current_vertexselection()
        self._optionspanel.apply()
        self.drawobj_anim = self._canvas.get_drawing().get_drawobj_by_ident('zonedraws')
        self.coord_last = self._canvas.unproject(event.GetPosition())
        #vertices = [list(self.coord_last),list(self.coord_last) ]
        vertices = [1.0*self.coord_last, 1.0*self.coord_last, ]  # attention, we need copies here!!
        # print '  vertices ',vertices#,self.width.get_value(),self.color.get_value(),

        # make drawobj make a zone
        if self.id_landusetype.get_value() >= 0:
            _id = self.drawobj_anim.make_zone(vertices,
                                              zonename=self.zonename.get_value(),
                                              id_landusetype=self.id_landusetype.get_value(),
                                              )
            self.ind_vert = 1
            self.drawobj_anim.begin_animation(_id)

            # http://www.wxpython.org/docs/api/wx.Cursor-class.html
            self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_CROSS))
            self.is_animated = True

            self.parent.refresh_optionspanel(self)
            return True  # True for redrawing
        else:
            # option values not valid
            return False


class FacilityDrawings(ZoneDrawings):
    def __init__(self, ident, facilities, parent,   **kwargs):

        Polygons.__init__(self, ident,  parent,
                          name='Facility drawings',
                          linewidth=1,
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

    # def is_tool_allowed(self, tool, id_drawobj = -1):
    #    """
    #    Returns True if this tool can be applied to this drawobj.
    #    Optionally a particular drawobj can be specified with id_drawobj.
    #    """
    #    # basic tools:
    #    #return tool.ident not in   ['configure','select_handles','delete','move','stretch']
    #    return tool.ident not in ['delete',]

    def get_netelement(self):
        return self._facilities

    def get_vertices_array(self):
        return self._facilities.shapes[self.get_ids()]  # .value[self._inds_map]

    def get_vertices(self, ids):
        return self._facilities.shapes[ids]

    def set_vertices(self, ids, vertices, is_update=True):
        self._facilities.set_shapes(ids, vertices)
        if is_update:
            self._update_vertexvbo()

    def set_netelement(self, element):
        self.set_facilities(element)

    def set_facilities(self, facilities):
        # print '\nset_facilities'#,facilities.get_ids()

        self._facilities = facilities
        # print '  vertices ',self.get_vertices(facilities.get_ids())
        if len(self) > 0:
            self.clear_rows()  # del_rows(self.get_ids())

        ids = self._facilities.get_ids()
        #self._inds_map = self._facilities.get_inds(ids)
        self.add_rows(ids=ids)
        self._id_target = -1

        # plugins to keep grapgics syncronized with netelements
        facilities.unplug()
        facilities.shapes.plugin.add_event(cm.EVTADDITEM, self.on_add_element)
        facilities.plugin.add_event(cm.EVTDELITEM, self.on_del_element)

        self.update()

    def update(self, is_update=True):
        # assumes that arrsy structure did not change
        # print 'FacilityDrawings.update'
        n = len(self)

        if n == 0:
            return

        self.colors.value[:] = np.ones((n, 4), np.float32)*self.color_facility_default.value
        self.colors_highl.value[:] = self._get_colors_highl(self.colors.value)

        landusetypes = self._facilities.get_landusetypes()
        ids_landusetype = self._facilities.ids_landusetype
        for id_landusetype in landusetypes.get_ids():
            #inds = np.flatnonzero(ids_landusetype == id_landusetype)
            #color = landusetypes.colors[id_landusetype]
            #self.colors.value[self._inds_map[inds]] = color
            #self.colors.value[self._inds_map[np.flatnonzero(ids_landusetype == id_landusetype)]] = landusetypes.colors[id_landusetype]
            self.colors[self._facilities.select_ids(
                ids_landusetype.value == id_landusetype)] = landusetypes.colors[id_landusetype]
        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()

    def update_colours(self, ids=None, is_update=True):
        if ids is None:
            ids = self.get_ids()

        inds = self._facilities.get_inds(ids)
        # print 'update_colours',ids, inds, type(inds)
        # print '    self.colors.value',self.colors.value
        landusetypes = self._facilities.parent.landusetypes
        # print '  ids_landusetype',self._facilities.ids_landusetype.value[inds]
        for id_landusetype in landusetypes.get_ids():
            # print '  is=',id_landusetype,self._zones.ids_landusetype.value[inds]==id_landusetype,ids
            ids_fac = ids[self._facilities.ids_landusetype.value[inds] == id_landusetype]
            # print '    ids_fac',ids_fac

            self.colors[ids_fac] = np.ones((len(ids_fac), 4), np.float32)*landusetypes.colors[id_landusetype]

        self.colors_highl.value[:] = self._get_colors_highl(self.colors.value)
        if is_update:
            self._update_colorvbo()

    def make(self, shape, id_sumo=None, osmkey='', id_landusetype=-1):
        # print 'make_zone shape',shape,type(shape)
        # print '  id_landusetype',id_landusetype
        if id_sumo == '':
            id_sumo = None
        return self._facilities.make(id_sumo=id_sumo,
                                     osmkey=osmkey, shape=shape,
                                     id_landusetype=id_landusetype)

    def on_add_element(self, shapes, ids):
        # print 'on_add_element',shapes.attrname,ids
        if shapes == self._facilities.shapes:
            self._id_target = ids[0]
            id_target = self.add_row(_id=self._id_target,
                                     colors=self.color_facility_default.get_value(),
                                     colors_highl=self._get_colors_highl(self.color_facility_default.get_value())
                                     )
            # print '    self.colors.value',self.colors.value, id_target,self._id_target
            #self._inds_map = self._zones.get_inds(self._zones.get_ids())
            # self._update_vertexvbo()
            # self._update_colorvbo()

    def begin_animation(self, id_target):
        # print 'ZoneDrawings.begin_animation zones.shapes=\n',id_target,self._id_target, self._zones.shapes[id_target]
        if self._id_target == -1:
            self._id_target = id_target
        self._drawobj_anim = self.parent.get_drawobj_by_ident(self._ident_drawobj_anim)
        self.id_anim = self._drawobj_anim.add_drawobj(np.array(self._facilities.shapes[self._id_target], np.float32).tolist(),
                                                      self.color_anim.value,
                                                      )
        # print 'begin_animation',self.ident,_id,self._drawobj_anim
        return True

    def end_animation(self, is_del_last_vert=False):
        # print 'ZoneDrawings.end_animation',self.ident,self._id_target,self.id_anim

        # print '  verices =',self._drawobj_anim.vertices[self.id_anim]
        # print '  self._drawobj_anim.vertices[self.id_anim]=',self._drawobj_anim.vertices[self.id_anim]
        shape = self._drawobj_anim.vertices[self.id_anim]
        self._facilities.set_shape(self._id_target, shape)
        #self._facilities.coords[self._id_target] = self._zones.get_coords_from_shape(shape)
        self.update_colours(ids=np.array([self._id_target]))  # must pass np array
        self.del_animation()
        # print '  self.get_vertices_array()=\n',self.get_vertices_array()
        # self._drawobj_anim.del_drawobj(self.id_anim)

        # self.update_internal()
        #self._inds_map = self._zones.get_inds(self._zones.get_ids())
        self._update_vertexvbo()
        self._update_colorvbo()
        return True

    def del_elem(self, id_fac):
        """
        Deletes an element from network and then in on canvas
        through callback on_del_element 
        """
        # print 'del_elem'

        self._facilities.del_element(id_fac)


class AddFacilityTool(AddZoneTool):
    """
    Mixin for Selection tools for OGL canvas.
    """

    def __init__(self, parent, mainframe=None):
        self.init_common('add_facility', parent, 'Add buildings tool',
                         info='Click on canvas to add a building or facility. Add a vertex with a single click, double click to finish, right click to aboard.',
                         is_textbutton=False,
                         )
        self.init_options()

    def init_options(self):

        self.add(cm.AttrConf('id_sumo',  '',
                             groupnames=['options'],
                             perm='rw',
                             name='Unique name',
                             info='Unique name. When left blanc, a number will be assigned automatically.',
                             ))

        self.add(cm.AttrConf('id_landusetype',  '',
                             groupnames=['options'],
                             choices={'': -1},
                             perm='rw',
                             name='Type',
                             info='Landusetype of this facility. Use Mixed when unknown or ambiguous.',
                             ))

        self.add(cm.AttrConf('osmkey',  '',
                                        groupnames=['options'],
                                        perm='rw',
                             name='OSM key',
                             info='OSM key of facility.',
                             ))

    def set_button_info(self, bsize=(32, 32)):
        # print 'set_button_info select tool'  self.get_icon("icon_sumo_24px.png")
        iconpath = os.path.join(os.path.dirname(__file__), 'images')
        self._bitmap = wx.Bitmap(os.path.join(iconpath, 'city-icon_24px.png'), wx.BITMAP_TYPE_PNG)
        self._bitmap_sel = self._bitmap

    def begin_animation(self, event):
        # print 'AddLineTool.begin_animation'
        #self.drawobj_anim, _id, self.ind_vert =  self.get_current_vertexselection()
        self._optionspanel.apply()
        self.drawobj_anim = self._canvas.get_drawing().get_drawobj_by_ident('facilitydraws')
        self.coord_last = self._canvas.unproject(event.GetPosition())
        #vertices = [list(self.coord_last),list(self.coord_last) ]
        vertices = [1.0*self.coord_last, 1.0*self.coord_last, ]  # attention, we need copies here!!
        # print '  vertices ',vertices#,self.width.get_value(),self.color.get_value(),

        # make drawobj make a zone
        if self.id_landusetype.get_value() >= 0:
            _id = self.drawobj_anim.make(vertices, id_sumo=self.id_sumo.get_value(),
                                         osmkey=self.osmkey.get_value(),
                                         id_landusetype=self.id_landusetype.get_value(),
                                         )
            self.ind_vert = 1
            self.drawobj_anim.begin_animation(_id)
            self.id_sumo.set_value('')

            # http://www.wxpython.org/docs/api/wx.Cursor-class.html
            self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_CROSS))
            self.is_animated = True

            self.parent.refresh_optionspanel(self)
            return True  # True for redrawing
        else:
            # option values not valid
            return False


class WxGui(ModuleGui):
    """Contains functions that communicate between the widgets of the main wx gui
    and the functions of the plugin.
    """

    def __init__(self, ident):
        self._landuse = None
        self._canvas = None
        self._init_common(ident,  priority=100,
                          icondirpath=os.path.join(os.path.dirname(__file__), 'images'))

    def get_module(self):
        return self._landuse

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
        scenario = self.get_scenario()
        # print 'landuse refresh_widgets',id(self._landuse),id(scenario.landuse),scenario.is_modified()
        # if self._landuse:
        #    print '   self._landuse.is_modified',self._landuse.is_modified(),self._landuse == scenario.landuse
        is_refresh = False
        if self._landuse != scenario.landuse:
            # print '  make new landuse instance'
            del self._landuse
            self._landuse = scenario.landuse
            is_refresh = True

        elif self._landuse.is_modified():
            # print '   self._landuse.is_modified',self._landuse.is_modified()
            is_refresh = True

        elif self.get_canvas() != self._canvas:
            is_refresh = True

        # print  '  is_refresh',is_refresh
        if is_refresh | self._is_needs_refresh:
            self._is_needs_refresh = False
            # print '  is_refresh',is_refresh,id(self._landuse)
            neteditor = self.get_neteditor()
            #canvas = self.get_canvas()
            drawing = self.get_drawing()  # canvas.get_drawing()

            # add or refresh facility drawing
            drawing.set_element('facilitydraws', FacilityDrawings,
                                self._landuse.facilities, layer=5)
            #drawobj = drawing.get_drawobj_by_ident('facilitydraws')
            # if drawobj is not None:
            #    drawobj.set_facilities(self._landuse.facilities)
            # else:
            #    facilitydrawings = FacilityDrawings(self._landuse.facilities, drawing)
            #    drawing.add_drawobj(facilitydrawings, layer = 5)

            # add or refresh zone drawing
            drawing.set_element('zonedraws', ZoneDrawings,
                                self._landuse.zones, layer=100)
            #drawobj = drawing.get_drawobj_by_ident('zonedraws')
            # if drawobj is not None:
            #    drawobj.set_netelement(self._landuse.zones)
            # else:
            #    zonedrawings = ZoneDrawings(self._landuse.zones, drawing)
            #    drawing.add_drawobj(zonedrawings, layer = 100)

            # add or refresh parking drawing
            drawing.set_element('parkingdraws', ParkingDrawings,
                                self._landuse.parking, layer=50)

            #drawobj = drawing.get_drawobj_by_ident('parkingdraws')
            # if drawobj is not None:
            #    drawobj.set_parking(self._landuse.parking)
            # else:
            #    parkingdrawings = ParkingDrawings(self._landuse.parking, drawing)
            #    drawing.add_drawobj(parkingdrawings, layer = 50)

            neteditor.get_toolbox().add_toolclass(AddZoneTool)  # will check if tool is already there
            neteditor.get_toolbox().add_toolclass(AddFacilityTool)
            neteditor.draw()

        self._canvas = self.get_canvas()

    def make_menu(self):
        # print 'make_menu'
        menubar = self._mainframe.menubar
        menubar.append_menu('landuse')

        menubar.append_item('landuse/browse',
                            self.on_browse_obj,  # common function in modulegui
                            info='View and browse landuse in object panel.',
                            bitmap=self.get_agileicon('icon_browse_24px.png'),  # ,
                            )

        # wx.Bitmap(os.path.join(IMAGEDIR,'icon_sumo_24px.png'),wx.BITMAP_TYPE_PNG)
        menubar.append_menu('landuse/maps',
                            bitmap=self.get_icon("map_24px.png"),
                            )

        menubar.append_item('landuse/maps/download...',
                            self.on_import_backgroundmaps,
                            info='Download backgroundmaps from the Intenet. Requires active Internet connection, and python packages pyproj and PIL.',
                            bitmap=self.get_icon('map_add_24px.png'),
                            )

        menubar.append_item('landuse/maps/clear',
                            self.on_clear_backgroundmaps,
                            info='Clear all background maps.',
                            bitmap=self.get_icon('map_del_24px.png'),
                            )

        menubar.append_menu('landuse/zones',
                            bitmap=self.get_icon("fig_zone_24px.png"),
                            )

        menubar.append_item('landuse/zones/identify zone edges',
                            self.on_refresh_zoneedges,
                            info='Identify network edges that are located within each zone.',
                            #bitmap = self.get_icon('Files-Osm-icon_24.png'),#
                            )

        menubar.append_item('landuse/zones/clear all',
                            self.on_clear_zones,
                            info='Delete all zones.',
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_DELETE, wx.ART_MENU),
                            )

        menubar.append_menu('landuse/facilities',
                            bitmap=self.get_icon("city-icon_24px.png"),
                            )

        menubar.append_item('landuse/facilities/import from poly file...',
                            self.on_import_poly, info='Import SUMO poly xml file...',
                            #bitmap = wx.ArtProvider.GetBitmap(wx.ART_NORMAL_FILE,wx.ART_MENU)
                            bitmap=self.get_agileicon("Document_Import_24px.png"),
                            )

        menubar.append_item('landuse/facilities/export to poly file...',
                            self.on_export_poly, info='Export facilities to SUMO poly xml file...',
                            #bitmap = wx.ArtProvider.GetBitmap(wx.ART_NORMAL_FILE,wx.ART_MENU)
                            bitmap=self.get_agileicon("Document_Export_24px.png"),
                            )
        menubar.append_item('landuse/facilities/export to shape file...',
                            self.on_facilities_to_shapefile,
                            #bitmap = wx.ArtProvider.GetBitmap(wx.ART_NORMAL_FILE,wx.ART_MENU)
                            bitmap=self.get_agileicon("Document_Export_24px.png"),
                            )

        menubar.append_item('landuse/facilities/clean osm file...',
                            self.on_clean_osm,
                            info='Cleans OSM file from strange characters. Use if you have trouble importing from OSM.',
                            bitmap=self.get_icon('Files-Osm-icon_24.png'),
                            )

        menubar.append_item('landuse/facilities/import from osm...',
                            self.on_import_osm,
                            info='Import landuse from osm files.',
                            bitmap=self.get_icon('Files-Osm-icon_24.png'),
                            )

        menubar.append_item('landuse/facilities/generate facilities...',
                            self.on_generate_facilities,
                            )

        menubar.append_item('landuse/facilities/identify zone',
                            self.on_identify_taz,
                            info='Identify the traffic assignment zone for each facility.',
                            #bitmap = self.get_icon('Files-Osm-icon_24.png'),#
                            )

        menubar.append_item('landuse/facilities/find closest edge',
                            self.on_identify_closest_edge,
                            info='Find for each building the closes access to the network. This will be the point on the network where people  access the facility.',
                            #bitmap = self.get_icon('Files-Osm-icon_24.png'),#
                            )
        menubar.append_item('landuse/facilities/update all facilities',
                            self.on_update_facilities,
                            )
        menubar.append_item('landuse/facilities/clear all facilities',
                            self.on_clear_facilities,
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_DELETE, wx.ART_MENU),
                            )

        menubar.append_menu('landuse/parking',
                            #bitmap = self.get_icon("Document_Import_24px.png"),
                            )
        menubar.append_item('landuse/parking/generate parking...',
                            self.on_make_parking,
                            #bitmap = self.get_icon('Files-Osm-icon_24.png'),#
                            )
        menubar.append_item('landuse/parking/clear parking',
                            self.on_clear_parking,
                            info='Delete all parking.',
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_DELETE, wx.ART_MENU),
                            )

    def on_refresh_zoneedges(self, event=None):
        self._landuse.zones.refresh_zoneedges()
        self._mainframe.browse_obj(self._landuse.zones)

    def on_identify_taz(self, event=None):
        self._landuse.facilities.identify_taz()
        self._mainframe.browse_obj(self._landuse.facilities)

    def on_identify_closest_edge(self, event=None):
        self._landuse.facilities.identify_closest_edge()
        self._mainframe.browse_obj(self._landuse.facilities)

    def on_make_parking(self, event=None):
        """
        Generate on road parking areas on the street network
        """
        # self._landuse.parking.make_parking()
        #self._canvas = canvas.draw()
        #drawing = self.get_drawing().get_drawobj_by_ident('parkingdraws')

        # TODO: make a proper import mask that allows to set parameters
        # self._landuse.maps.download()
        proc = landuse.ParkingGenerator('parkinggenerator', self._landuse.parking, logger=self._mainframe.get_logger())
        dlg = ProcessDialog(self._mainframe, proc, immediate_apply=True)

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
            self._mainframe.browse_obj(self._landuse.parking)
            self._mainframe.refresh_moduleguis()

    def on_clear_parking(self, event=None):
        self._landuse.parking.clear()
        #self._canvas = canvas.draw()
        #drawing = self.get_drawing().get_drawobj_by_ident('parkingdraws')
        self._mainframe.browse_obj(self._landuse.parking)
        self._mainframe.refresh_moduleguis()

    def on_clean_osm(self, event=None):

        scenario = self.get_scenario()
        wildcards_all = "All files (*.*)|*.*"
        wildcards_xml = "Poly xml files (*.osm.xml)|*.osm.xml|XML files (*.xml)|*.xml"
        wildcards = wildcards_xml+"|"+wildcards_all

        # Finally, if the directory is changed in the process of getting files, this
        # dialog is set up to change the current working directory to the path chosen.
        dlg = wx.FileDialog(
            self._mainframe, message="Choose one or more osm files to clean",
            defaultDir=scenario.get_workdirpath(),
            defaultFile=os.path.join(scenario.get_workdirpath(), scenario.get_rootfilename()+'osm.xml'),
            wildcard=wildcards,
            style=wx.OPEN | wx.MULTIPLE | wx.CHANGE_DIR
        )

        # Show the dialog and retrieve the user response. If it is the OK response,
        # process the data.
        if dlg.ShowModal() == wx.ID_OK:
            # This returns a Python list of files that were selected.
            paths = dlg.GetPaths()
            # print 'You selected %d files:' % len(paths)
            if len(paths) > 0:

                for path in paths:
                    # print '           %s' % path
                    path_temp = path+'.clean'
                    landuse.clean_osm(path, path_temp)
                    #shutil.copy (path_temp, path)
                    shutil.move(path_temp, path)

                self._mainframe.browse_obj(self._landuse)
                self._mainframe.refresh_moduleguis()

                # inform plugins

        # Destroy the dialog. Don't do this until you are done with it!
        # BAD things can happen otherwise!
        dlg.Destroy()

    def on_import_poly(self, event=None):
        # TODO: here we could make a nice dialog asking if
        # existing file should be overwritten.
        #self.write_to_statusbar('Import facilities', key='action')
        # Create the dialog. In this case the current directory is forced as the starting
        # directory for the dialog, and no default file name is forced. This can easilly
        # be changed in your program. This is an 'open' dialog, and allows multitple
        # file selections as well.
        #
        scenario = self.get_scenario()
        wildcards_all = "All files (*.*)|*.*"
        wildcards_xml = "Poly xml files (*.poly.xml)|*.poly.xml|XML files (*.xml)|*.xml"
        wildcards = wildcards_xml+"|"+wildcards_all

        # Finally, if the directory is changed in the process of getting files, this
        # dialog is set up to change the current working directory to the path chosen.
        dlg = wx.FileDialog(
            self._mainframe, message="Choose one or more poly files",
            defaultDir=scenario.get_workdirpath(),
            defaultFile=scenario.get_rootfilepath()+'.poly.xml',
            wildcard=wildcards,
            style=wx.OPEN | wx.MULTIPLE | wx.CHANGE_DIR
        )

        # Show the dialog and retrieve the user response. If it is the OK response,
        # process the data.
        if dlg.ShowModal() == wx.ID_OK:
            # This returns a Python list of files that were selected.
            paths = dlg.GetPaths()
            # print 'You selected %d files:' % len(paths)
            if len(paths) > 0:

                for path in paths:
                    # print '           %s' % path
                    #readNet(path, net = net)
                    dirname = os.path.dirname(path)
                    filename_raw = os.path.basename(path)
                    filename = filename_raw.split('.')[0]
                    self._landuse.import_polyxml(filename, dirname=dirname)

                self._mainframe.browse_obj(self._landuse)
                self._is_needs_refresh = True
                self._mainframe.refresh_moduleguis()

        # Destroy the dialog. Don't do this until you are done with it!
        # BAD things can happen otherwise!
        dlg.Destroy()

    def on_clear_zones(self, event=None):
        self._landuse.zones.clear()
        self._mainframe.browse_obj(self._landuse.zones)
        self._is_needs_refresh = True
        self._mainframe.refresh_moduleguis()

    def on_generate_facilities(self, event=None):
        """
        Generates mobility plans using different strategies..
        """
        self._landuse.facilities.unplug()
        self.proc = landuse.FacilityGenerator('facilitygenerator',
                                              self._landuse.facilities,
                                              logger=self._mainframe.get_logger()
                                              )

        dlg = ProcessDialogInteractive(self._mainframe,
                                       self.proc,
                                       title=self.proc.get_name(),
                                       func_close=self.close_process_facilities,
                                       )

        dlg.CenterOnScreen()

        # this does not return until the dialog is closed.
        #val = dlg.ShowModal()
        dlg.Show()
        dlg.MakeModal(True)

    def close_process_facilities(self, dlg):
        # called before destroying the process dialog
        if self.proc.status == 'success':
            self._mainframe.browse_obj(self._landuse.facilities)
            self._is_needs_refresh = True
            self._mainframe.refresh_moduleguis()

    def on_clear_facilities(self, event=None):
        """Delete all facilities."""
        self._landuse.facilities.clear()
        self._mainframe.browse_obj(self._landuse.facilities)
        self._is_needs_refresh = True
        self._mainframe.refresh_moduleguis()

    def on_update_facilities(self, event=None):
        """Update area and capacities of all facilities."""
        self._landuse.facilities.update()
        self._mainframe.browse_obj(self._landuse.facilities)
        #self._is_needs_refresh = True
        # self._mainframe.refresh_moduleguis()

    def on_export_poly(self, event=None):
        scenario = self.get_scenario()
        wildcards_all = "All files (*.*)|*.*"
        wildcards_xml = "Poly xml files (*.poly.xml)|*.poly.xml|XML files (*.xml)|*.xml"
        wildcards = wildcards_xml+"|"+wildcards_all

        # Finally, if the directory is changed in the process of getting files, this
        # dialog is set up to change the current working directory to the path chosen.
        dlg = wx.FileDialog(
            self._mainframe, message="Choose one or more poly files",
            defaultDir=scenario.get_workdirpath(),
            defaultFile=scenario.get_rootfilepath()+'.poly.xml',
            wildcard=wildcards,
            style=wx.SAVE | wx.CHANGE_DIR
        )

        # Show the dialog and retrieve the user response. If it is the OK response,
        # process the data.
        if dlg.ShowModal() == wx.ID_OK:
            # This returns a Python list of files that were selected.
            path = dlg.GetPath()
            # print 'You selected %d files:' % len(paths)
            if path is not "":
                self._landuse.export_polyxml(path)

        dlg.Destroy()

    def on_facilities_to_shapefile(self, event=None):
        """
        Export facility data to shape file.
        """
        # print 'on_edges_to_shapefile'

        dirpath = self._landuse.parent.get_workdirpath()
        defaultFile = self._landuse.parent.get_rootfilename()+'.facil.shp'
        wildcards_all = 'All files (*.*)|*.*|SHP files (*.shp)|*.shp'
        dlg = wx.FileDialog(None, message='Export facilities to shapefile',
                            defaultDir=dirpath, defaultFile=defaultFile,
                            wildcard=wildcards_all, style=wx.SAVE | wx.CHANGE_DIR)
        if dlg.ShowModal() == wx.ID_OK:
            filepath = dlg.GetPath()

        else:
            return

        shapeformat.facilities_to_shapefile(self._landuse.facilities,
                                            filepath,
                                            log=self._mainframe.get_logger())

    def on_import_osm(self, event=None):
        importer = landuse.OsmPolyImporter(self._landuse, logger=self._mainframe.get_logger())
        dlg = ProcessDialog(self._mainframe, importer)

        dlg.CenterOnScreen()

        # this does not return until the dialog is closed.
        val = dlg.ShowModal()
        # print '  val,val == wx.ID_OK',val,wx.ID_OK,wx.ID_CANCEL,val == wx.ID_CANCEL
        # print '  status =',dlg.get_status()
        if dlg.get_status() != 'success':  # val == wx.ID_CANCEL:
            dlg.Destroy()

        if dlg.get_status() == 'success':
            dlg.apply()
            dlg.Destroy()
            self._mainframe.browse_obj(self._landuse)
            self._is_needs_refresh = True
            self._mainframe.refresh_moduleguis()

    def on_clear_backgroundmaps(self, event=None):
        self._landuse.maps.clear_all()
        self._mainframe.browse_obj(self._landuse.maps)

    def on_import_backgroundmaps(self, event=None):
        # TODO: make a proper import mask that allows to set parameters
        # self._landuse.maps.download()
        importer = maps.MapsImporter(self._landuse.maps, logger=self._mainframe.get_logger())
        dlg = ProcessDialog(self._mainframe, importer, immediate_apply=True)

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
            self._mainframe.browse_obj(self._landuse.maps)
