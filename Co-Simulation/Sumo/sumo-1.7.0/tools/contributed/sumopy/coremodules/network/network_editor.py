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

# @file    network_editor.py
# @author  Joerg Schweizer
# @date


import os
import sys
import wx
if __name__ == '__main__':
    try:
        APPDIR = os.path.dirname(os.path.abspath(__file__))
    except:
        APPDIR = os.path.dirname(os.path.abspath(sys.argv[0]))
    #AGILEDIR = os.path.join(APPDIR,'..','..','agilepy')

    # sys.path.append(AGILEDIR)
    # sys.path.append(os.path.join(AGILEDIR,"lib_base"))
    # sys.path.append(os.path.join(AGILEDIR,"lib_wx"))

    SUMOPYDIR = os.path.join(APPDIR, '..', '..')
    sys.path.append(os.path.join(SUMOPYDIR))

import numpy as np

from agilepy.lib_wx.ogleditor import *
#from agilepy.lib_wx.mainframe import AgileMainframe
from coremodules.network.network import MODES
#( ident_drawob, DrawobjClass, netattrname, layer )


class NetSelectTool(SelectTool):
    """
    Selection tool for OGL canvas.
    """

    def __init__(self, parent, mainframe=None):
        """
        To be overridden by specific tool.
        """
        self.init_common('select', parent, 'Net Selection',
                         info='Select objects in networks',
                         is_textbutton=False,
                         )

        self._init_select(is_show_selected=True)

    def on_left_down_trans(self, event):

        #vetrex = np.array([p[0], p[1], 0.0, 1.0],float)
        #p_screen = self._canvas.project(vetrex)
        # print 'SelectTool.on_left_down (px,py)=',self._canvas.unproject_event(event)
        # print '  (x,y)=',event.GetPosition(),p_screen
        is_draw = False

        # if drawing:
        if len(self) > 0:
            if event.ShiftDown():
                self.unhighlight_current()
                self._idcounter += 1
                if self._idcounter == len(self):
                    self._idcounter = 0
                self.highlight_current()

                self.parent.refresh_optionspanel(self)
                is_draw = True
            else:
                is_draw = self.unselect_all()
                self._idcounter = 0
                if is_draw:
                    self.parent.refresh_optionspanel(self)
        else:
            is_draw = self.pick_all(event)
            self.highlight_current()
            self.parent.refresh_optionspanel(self)

        return is_draw

    def get_scenario(self):
        # get net and scenario via netdrawing
        return self.get_drawing().get_net().parent

    def set_objbrowser(self):
        #mainframe = self.parent.get_mainframe()
        # if mainframe is not None:
        #    netelement = self.get_netelement_current()
        #    if netelement is not None:
        #        mainframe.browse_obj(netelement[0], id = netelement[1])
        mainframe = self.parent.get_mainframe()
        if mainframe is not None:
            drawobj, _id = self.get_current_selection()
            if drawobj is not None:
                obj = drawobj.get_netelement()
                mainframe.browse_obj(obj, id=_id)

    # def set_setelement_current(self, objids):
    #    """
    #    Sets and highlights the current netelement
    #    """
    #    self.canvas.get_drawing()
    #    self.unselect_all()

    def get_netelement_current(self):
        mainframe = self.parent.get_mainframe()
        if mainframe is not None:
            drawobj, _id = self.get_current_selection()
            if drawobj is not None:
                obj = drawobj.get_netelement()
                return obj, _id
            else:
                return None
        else:
            return None
    # def get_optionspanel(self, parent):
    #    """
    #    Return tool option widgets on given parent
    #    """
    #    drawobj, _id = self.get_current_selection()
    #    if drawobj  is not None:
    #        obj = drawobj.get_netelement()
    #
    #    else:
    #        obj=cm.BaseObjman('empty')
    #        _id = None
    #
    #    #print 'get_optionspanel',drawobj, _id
    #    self._optionspanel = NaviPanel(parent, obj =  obj,
    #                attrconfigs=None, id = _id,
    #                #tables = None,
    #                #table = None, id=None, ids=None,
    #                #groupnames = ['options'],
    #                mainframe=self.parent.get_mainframe(),
    #                immediate_apply=False, panelstyle='default',#'instrumental'
    #                standartbuttons=['apply','restore'])
    #
    #    return self._optionspanel


class NetDeleteTool(DeleteTool):
    """
    Delete tool for OGL canvas.
    """

    def __init__(self, parent,  detectpix=5, mainframe=None):
        """
        To be overridden by specific tool.
        """
        self.init_common('delete', parent, 'Delete tool',
                         info='Select and delete objects in network',
                         is_textbutton=False,
                         )
        self._init_select(is_show_selected=True, detectpix=detectpix)

    def on_execute_selection(self, event):
        """
        Definively execute operation on currently selected drawobjects.
        """
        if self.is_tool_allowed_on_selection():
            drawobj, _id = self.get_current_selection()
            if drawobj is not None:
                drawobj.del_elem(_id)
                self.unselect_all()
                is_draw = True
            else:
                is_draw = False
            return is_draw
        else:
            return False


class AddCrossingTool(SelectTool):
    """
    OD flow toolfor OGL canvas.
    """

    def __init__(self, parent, mainframe=None):
        """
        To be overridden by specific tool.
        """
        self.init_common('crossingadder', parent, 'Add crossing',
                         info="""Tool to add crossings at intersections:
                                1. Select a Node with <LEFT CLICK> on the Network editor to specify a <Node> where to build the crossing. 
                                2. Optionally modify the width of the crossing.
                                3. Select an <Edge> where to build the crossing by cycling through all possible adjacent edges with <SHIFT>+<LEFT MOUSE> and <LEFT CLICK> when the desired <Edge> is highlighted.
                                4. Repeat edge selection until all involved edges are highlighted.  
                                5. Press the "Add" Button to add the crossing. 
                                Press the "Clear" button to clear all selections.
                               """,
                         is_textbutton=False,
                         )

        self._init_select(is_show_selected=False)

        self.add(cm.AttrConf('id_node', -1,
                             groupnames=['options'],
                             name='Node ID',
                             perm='r',
                             info='Node or juction at which crossings should be placed.',
                             ))

        self.add(cm.AttrConf('ids_edge', [],
                             groupnames=['options'],
                             perm='r',
                             name='Edge IDs',
                             info='Edge IDs, accross which the crossing will be build.',
                             ))

        self.add(cm.AttrConf('width', 2.0,
                             groupnames=['options'],
                             name='Width',
                             perm='rw',
                             unit='m',
                             info='Crossing Width.',
                             ))

    def set_button_info(self, bsize=(32, 32)):
        # print 'set_button_info select tool'  self.get_icon("icon_sumo_24px.png")
        iconpath = os.path.join(os.path.dirname(__file__), 'images')
        self._bitmap = wx.Bitmap(os.path.join(iconpath, 'fig_crossing_32px.png'), wx.BITMAP_TYPE_PNG)
        self._bitmap_sel = self._bitmap

    def set_cursor(self):
        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        if self._canvas is not None:
            self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_RIGHT_ARROW))

    def activate(self, canvas=None):
        """
        This call by metacanvas??TooldsPallet signals that the tool has been
        activated and can now interact with metacanvas.
        """
        # print 'activate'
        SelectTool.activate(self, canvas)

        # make lanes invisible, because they disturb
        lanedraws = self.get_drawobj_by_ident('lanedraws')
        self._is_lane_visible_before = None
        if lanedraws:
            self._is_lane_visible_before = lanedraws.is_visible()
            lanedraws.set_visible(False)

        canvas.draw()

    def deactivate(self):
        """
        This call by metacanvas signals that the tool has been
        deactivated and can now interact with metacanvas.
        """

        self._is_active = False
        # self.unhighlight()

        # reset lane visibility
        if self._is_lane_visible_before is not None:
            lanedraws = self.get_drawobj_by_ident('lanedraws')
            if lanedraws:
                lanedraws.set_visible(self._is_lane_visible_before)
                self._canvas.draw()

        self.deactivate_select()

    def on_left_down_select(self, event):
        # same as on select tool but avoid deselection after execution

        # print 'on_left_down_select'
        is_draw = False

        if len(self) > 0:
            if event.ShiftDown():

                self.iterate_selection()
                self.on_change_selection(event)
                is_draw = True
            else:
                # print '  on_execute_selection 1'
                is_draw |= self.on_execute_selection(event)
                # attention: on_execute_selection must take care of selected
                # objects in list with self.unselect_all()
                #is_draw |= self.unselect_all()

                if self.is_show_selected:
                    self.parent.refresh_optionspanel(self)

        else:
            is_draw |= self.pick_all(event)
            self.highlight_current()

            if not event.ShiftDown():
                if self.is_preselected():
                    self.coord_last = self._canvas.unproject(event.GetPosition())
                    # print '  on_execute_selection 2'
                    is_draw |= self.on_execute_selection(event)
                    # attention: on_execute_selection must take care of selected
                    # objects in list with self.unselect_all()

            else:
                self.coord_last = self._canvas.unproject(event.GetPosition())

            if self.is_show_selected:
                self.parent.refresh_optionspanel(self)

        return is_draw

    def on_execute_selection(self, event):
        """
        Definively execute operation on currently selected drawobjects.
        """
        # print 'on_execute_selection',self.get_netelement_current(),len(self)
        # self.set_objbrowser()
        # self.highlight_current()
        self.unhighlight_current()
        # self.unhighlight()
        netelement_current = self.get_netelement_current()
        # print '  netelement_current',netelement_current
        if netelement_current is not None:
            (element, id_elem) = netelement_current
            if element.get_ident() == 'nodes':
                nodes = element
                # print '  selected node',id_elem
                # if self.id_node.get_value() <0:
                #    #print '    set name_orig',zones.ids_sumo[id_zone]
                self.id_node.set_value(id_elem)

                self.ids_edge.set_value([])

                # add potential to-edges to selection
                edgedraws = self.get_drawobj_by_ident('edgedraws')
                self.unselect_all()

                for id_edge in nodes.ids_incoming[id_elem]:
                    self.add_selection(edgedraws, id_edge)

                for id_edge in nodes.ids_outgoing[id_elem]:
                    self.add_selection(edgedraws, id_edge)

                # self.unselect_all()# includes unhighlight
                self.highlight()
                self.parent.refresh_optionspanel(self)

            elif element.get_ident() == 'edges':
                if self.id_node.get_value() >= 0:
                    # print '  selected edge',id_elem
                    if id_elem not in self.ids_edge.get_value():
                        self.ids_edge.get_value().append(id_elem)
                        self.parent.refresh_optionspanel(self)
                        self.highlight()

            else:
                self.unselect_all()

        return True

    def highlight(self):
        edges = self.get_edges()
        drawing = self.get_drawing()
        for id_egde in self.ids_edge.get_value():
            if id_egde > 0:
                drawing.highlight_element(edges, id_egde, is_update=True)

        if self.id_node.get_value() >= 0:
            drawing.highlight_element(self.get_scenario().net.nodes, self.id_node.get_value(), is_update=True)

    def unhighlight(self):
        edges = self.get_edges()
        drawing = self.get_drawing()
        for id_edge in self.ids_edge.get_value():
            if id_edge > 0:
                drawing.unhighlight_element(edges, id_edge, is_update=True)

        if self.id_node.get_value() >= 0:
            drawing.unhighlight_element(self.get_scenario().net.nodes, self.id_node.get_value(), is_update=True)

    def on_change_selection(self, event):
        """
        Called after selection has been changed with SHIFT-click
        Do operation on currently selected drawobjects.
        """
        # self.set_objbrowser()
        # self.parent.refresh_optionspanel(self)
        return False

    def get_netelement_current(self):
        mainframe = self.parent.get_mainframe()
        if mainframe is not None:
            drawobj, _id = self.get_current_selection()
            if drawobj is not None:
                obj = drawobj.get_netelement()
                return obj, _id
            else:
                return None
        else:
            return None

    def get_scenario(self):
        # get net and scenario via netdrawing
        return self.get_drawing().get_net().parent

    def get_edges(self):
        return self.get_scenario().net.edges

    def on_add_crossing(self, event=None):
        self._optionspanel.apply()
        #edges = self.get_edges().ids_sumo
        print 'add crossing'
        # print '  id_node',self.id_node.get_value()
        # print '  ids_edge',self.ids_edge.get_value()
        crossings = self.get_scenario().net.crossings
        id_cross = crossings.make(id_node=self.id_node.get_value(),
                                  ids_edge=self.ids_edge.get_value(),
                                  width=self.width.get_value(),
                                  )

        # this should go into a plugin callback

        # this will completely regenerate all crossings
        crossingsdraws = self.get_drawobj_by_ident('crossingsdraws')
        crossingsdraws.set_netelement(crossings)
        # add_flow( self.t_start.value, self.t_end.value,
        #                                        self.id_mode.value,
        #                                        self.id_fromedge.value,
        #                                        self.flow_generated.value)
        mainframe = self.parent.get_mainframe()
        if mainframe is not None:
            mainframe.browse_obj(crossings, id=id_cross)
        self.on_clear()

        # self.parent.refresh_optionspanel(self)
        # self._canvas.draw()

    def on_clear(self, event=None):
        self.unhighlight()
        self.id_node.set_value(-1)  # set empty
        self.ids_edge.set_value([])  # set empty
        self.unselect_all()

        self.parent.refresh_optionspanel(self)
        self._canvas.draw()

    def get_optionspanel(self, parent, size=wx.DefaultSize):
        """
        Return tool option widgets on given parent
        """
        size = (200, -1)
        buttons = [('Add', self.on_add_crossing, 'Add crossing to network.'),
                   ('Clear', self.on_clear, 'Clear selection.'),
                   #('Save flows', self.on_add, 'Save OD flows to current demand.'),
                   #('Cancel', self.on_close, 'Close wizzard without adding flows.'),
                   ]
        defaultbuttontext = 'Add'
        self._optionspanel = ObjPanel(parent, obj=self,
                                      attrconfigs=None,
                                      groupnames=['options'],
                                      func_change_obj=None,
                                      show_groupnames=False, show_title=True, is_modal=False,
                                      mainframe=self.parent.get_mainframe(),
                                      pos=wx.DefaultPosition, size=size, style=wx.MAXIMIZE_BOX | wx.RESIZE_BORDER,
                                      immediate_apply=True, panelstyle='default',  # 'instrumental'
                                      buttons=buttons, defaultbutton=defaultbuttontext,
                                      standartbuttons=[],  # standartbuttons=['restore']
                                      )

        return self._optionspanel


class NeteditorTools(ToolsPanel):
    """
    Shows a toolpallet with different tools and an options panel.
    Here tools are added which 
    """

    def __init__(self, parent):
        ToolsPanel.__init__(self, parent, n_buttoncolumns=4)
        # add and set initial tool

        self.add_initial_tool(NetSelectTool(self))
        self.add_tool(StretchTool(self, detectpix=10))
        self.add_tool(MoveTool(self, detectpix=5))
        self.add_tool(ConfigureTool(self, detectpix=10))
        self.add_tool(NetDeleteTool(self, detectpix=5))
        # self.add_tool(AddCrossingTool(self))

        # more tools can be added later...


class NodeDrawings(Circles):
    def __init__(self, ident, nodes, parent,   **kwargs):

        Circles.__init__(self, ident,  parent, name='Node drawings',
                         is_parentobj=False,
                         is_fill=False,  # Fill objects,
                         is_outline=True,  # show outlines
                         n_vert=21,  # default number of vertex per circle
                         linewidth=3,
                         **kwargs)

        self.delete('centers')
        self.delete('radii')

        self.add(cm.AttrConf('color_node_default', np.array([0.0, 0.8, 0.8, 1.0], np.float32),
                             groupnames=['options', 'nodecolors'],
                             metatype='color',
                             perm='wr',
                             name='Default color',
                             info='Default node color.',
                             ))

        self.set_netelement(nodes)

    def get_netelement(self):
        return self._nodes

    def get_centers_array(self):
        return self._nodes.coords.value[self._inds_map]

    def get_radii_array(self):
        return self._nodes.radii.value[self._inds_map]

    def is_tool_allowed(self, tool, id_drawobj=-1):
        """
        Returns True if this tool can be applied to this drawobj.
        Optionally a particular drawobj can be specified with id_drawobj.
        """
        # basic tools:
        # return tool.ident not in   ['configure','select_handles','delete','move','stretch']
        return tool.ident not in ['delete', 'stretch']

    def set_netelement(self, nodes):
        # print 'set_nodes'
        self._nodes = nodes
        # if len(self)>0:
        #    self.del_rows(self.get_ids())
        self.clear_rows()

        ids = self._nodes.get_ids()
        n = len(ids)

        self._inds_map = self._nodes.get_inds(ids)

        # print 'color_node_default',self.color_node_default.value
        # print 'colors\n',  np.ones((n,1),np.int32)*self.color_node_default.value
        # print '  dtypes', self.color_node_default.value.dtype,    np.ones((n,1),np.float32).dtype
        # print '  dtypes colors_highl', self.colors_highl.value.dtype,self._get_colors_highl(np.ones((n,1),np.float32)*self.color_node_default.value).dtype
        # print '  colors',np.ones((n,1),np.float32)*self.color_node_default.value
        # print '  dtypes ids',ids.dtype, self.get_ids().dtype
        self.add_rows(ids=ids,
                      colors=np.ones((n, 1), np.float32)*self.color_node_default.value,
                      colors_highl=self._get_colors_highl(np.ones((n, 1), np.float32)*self.color_node_default.value),
                      #centers = self._nodes.coords[ids],
                      #radii = self._nodes.radii[ids],
                      )

        self.update()

    def update(self, is_update=True):

        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()


class EdgeDrawings(Polylines):
    def __init__(self, ident, edges, parent,   **kwargs):

        # joinstyle
        # FLATHEAD = 0
        # BEVELHEAD = 1
        Polylines.__init__(self, ident,  parent, name='Edge drawings',
                           is_lefthalf=False,
                           is_righthalf=True,  # reverse for english roads
                           arrowstretch=1.5,
                           joinstyle=FLATHEAD,  # BEVELHEAD is good for both halfs,
                           **kwargs)

        self.delete('vertices')
        self.delete('widths')

        self.add(cm.AttrConf('color_edge_default', np.array([0.2, 0.2, 1.0, 1.0], np.float32),
                             groupnames=['options', 'edgecolors'],
                             perm='wr',
                             metatype='color',
                             name='Default color',
                             info='Default edge color.',
                             ))

        self.edgeclasses = {
            'bikeway': ('highway.cycleway', [0.9, 0.2, 0.2, 0.9]),
            #
            'footpath': ('highway.footpath', [0.1, 0.8, 0.5, 0.9]),
            'pedestrian': ('highway.pedestrian', [0.1, 0.8, 0.5, 0.9]),
            'footway': ('highway.footway', [0.1, 0.8, 0.5, 0.9]),
            #
            'serviceroad': ('highway.service', [0.4, 0.2, 0.8, 0.9]),
            'delivery': ('highway.delivery', [0.4, 0.2, 0.8, 0.9]),
        }

        for edgeclass, cdata in self.edgeclasses.iteritems():
            edgetype, color = cdata
            self.add(cm.AttrConf('color_'+edgeclass, np.array(color, np.float32),
                                 groupnames=['options', 'edgecolors'],
                                 edgetype=edgetype,
                                 metatype='color',
                                 perm='wr',
                                 name=edgeclass+' color',
                                 info='Color of '+edgeclass+' edge class.',
                                 ))

        self.set_netelement(edges)

    def get_netelement(self):
        return self._edges

    def get_vertices_array(self):
        return self._edges.shapes[self.get_ids()]  # .value[self._inds_map]#[self.get_ids()]

    def get_widths_array(self):
        # double because only the right half is shown
        # add a little bit to the width to make it a little wider than the lanes contained
        # return 2.2*self._edges.widths.value[self._inds_map]
        return 1.1*self._edges.widths[self.get_ids()]  # .value[self._inds_map]

    def get_vertices(self, ids):
        return self._edges.shapes[ids]

    def set_vertices(self, ids, vertices, is_update=True):
        self._edges.set_shapes(ids, vertices)
        if is_update:
            self._update_vertexvbo()
            self.parent.get_drawobj_by_ident('lanedraws').update()
            self.parent.get_drawobj_by_ident('crossingsdraws').update()
            self.parent.get_drawobj_by_ident('connectiondraws').update()

    def get_widths(self, ids):
        return 1.1*self._edges.widths[ids]

    def set_widths(self, ids, values):
        #self._edges.widths[ids] = values/1.1
        pass

    def is_tool_allowed(self, tool, id_drawobj=-1):
        """
        Returns True if this tool can be applied to this drawobj.
        Optionally a particular drawobj can be specified with id_drawobj.
        """
        # basic tools:
        # return tool.ident not in   ['configure','select_handles','delete','move','stretch']
        return tool.ident not in ['delete', ]

    def set_netelement(self, edges):

        self._edges = edges
        #self._inds_edges = self._edges.get_inds()
        self.clear_rows()
        # if len(self)>0:
        #    self.del_rows(self.get_ids())

        ids = self._edges.get_ids()
        #self._inds_map = self._edges.get_inds(ids)
        n = len(ids)
        #self.vertices = self._edges.shapes
        #self.widths = self._edges.widths

        self.add_rows(ids=ids,
                      beginstyles=np.ones(n)*FLATHEAD,
                      endstyles=np.ones(n)*TRIANGLEHEAD,
                      )
        self.update()

    def update(self, is_update=True):
        """
        Update color, assume that there have not been structural changes of the arrays
        """
        # assumes that edges have been set in set_edges
        # print 'Edgedrawing.update'
        #edgeinds = self._edges.get_inds()
        n = len(self)

        # self.widths.value[:] = self._edges.widths_lanes.value[inds_edges]#np.ones(n, np.float)*3.2
        #self.colors.value = np.ones((n,1),np.float32)*np.array([0.9,0.9,0.9,0.9])
        #self.colors_highl.value = self._get_colors_highl(self.colors.value)
        self.colors_fill.value[:] = np.ones((n, 1), np.float32)*self.color_edge_default.value
        for edgeclass, cdata in self.edgeclasses.iteritems():
            edgetype, color = cdata
            # print '  ',edgeclass, np.sum(self._edges.types.value==edgetype)
            # print '  color',getattr(self,'color_'+edgeclass).value
            self.colors_fill[self._edges.select_ids(self._edges.types.value == edgetype)] = getattr(
                self, 'color_'+edgeclass).value
            #self.colors_fill.value[self._inds_map[self._edges.types.value==edgetype]] = getattr(self,'color_'+edgeclass).value

        self.colors_fill_highl.value[:] = self._get_colors_highl(self.colors_fill.value)

        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()


class LaneDrawings(EdgeDrawings):
    def __init__(self, ident, lanes, parent,   **kwargs):

        # joinstyle
        # FLATHEAD = 0
        # BEVELHEAD = 1
        Polylines.__init__(self, ident,  parent, name='Lane drawings',
                           is_lefthalf=True,
                           is_righthalf=True,  # reverse for english roads
                           arrowstretch=1.0,
                           joinstyle=FLATHEAD,  # BEVELHEAD,
                           **kwargs)

        self.delete('vertices')
        self.delete('widths')

        self.add(cm.AttrConf('color_lane_default', np.array([0.2, 0.6, 1.0, 0.7], np.float32),
                             groupnames=['options', 'lanecolors'],
                             perm='wr',
                             metatype='color',
                             name='Default color',
                             info='Default lane color.',
                             ))

        typecolors = {
            'bicycle': [0.8, 0.4, 0.4, 0.8],
            'pedestrian': [0.1, 0.8, 0.5, 0.8],
            'delivery': [0.5, 0.5, 0.8, 0.8],
        }

        net = lanes.parent
        for typename, color in typecolors.iteritems():
            id_mode = net.get_id_mode(typename)
            self.add(cm.AttrConf('color_'+typename, np.array(color, np.float32),
                                 groupnames=['options', 'typecolors'],
                                 id_mode=id_mode,
                                 metatype='color',
                                 perm='wr',
                                 name=typename+' color',
                                 info='Color of '+typename+' lane type.',
                                 ))

        self.set_netelement(lanes)

    def get_netelement(self):
        return self._lanes

    def get_vertices_array(self):
        return self._lanes.shapes[self.get_ids()]  # .value[self._inds_map]#

    def get_vertices(self, ids):
        return self._lanes.shapes[ids]

    def set_vertices(self, ids, vertices, is_update=True):
        self._lanes.shapes[ids] = vertices
        if is_update:
            self._update_vertexvbo()

    def get_widths_array(self):
        # return here only 0.9 so that there remains a little gap between the lanes
        return 0.8*self._lanes.widths[self.get_ids()]  # .value[self._inds_map]#[self.get_ids()]

    def get_widths(self, ids):
        return self._lanes.widths[ids]

    def set_widths(self, ids, widths):
        self._lanes.widths[ids] = widths

    def is_tool_allowed(self, tool, id_drawobj=-1):
        """
        Returns True if this tool can be applied to this drawobj.
        Optionally a particular drawobj can be specified with id_drawobj.
        """
        # basic tools:
        # return tool.ident not in   ['configure','select_handles','delete','move','stretch']
        return tool.ident in ['configure', 'select_handles']

    def set_netelement(self, lanes):

        self._lanes = lanes
        #self._inds_lanes = self._lanes.get_inds()

        # if len(self)>0:
        #    self.del_rows(self.get_ids())
        self.clear_rows()
        # print 'LaneDrawings.set_netelement',len(self),len(self.colors_fill.value),len(lanes)
        ids = self._lanes.get_ids()
        #self._inds_map = self._lanes.get_inds(ids)
        n = len(ids)
        #self.vertices = self._edges.shapes
        #self.widths = self._edges.widths

        self.add_rows(ids=ids,
                      beginstyles=np.ones(n)*FLATHEAD,
                      endstyles=np.ones(n)*TRIANGLEHEAD,
                      )
        self.update()

    def update(self, is_update=True):
        """
        Update color, assume that there have not been structural changes of the arrays
        """
        # print 'Lanedraws.update'
        # assumes that edges have been set in set_edges
        # print 'Edgedrawing.update'
        #edgeinds = self._edges.get_inds()
        n = len(self)

        # self.widths.value[:] = self._edges.widths_lanes.value[inds_edges]#np.ones(n, np.float)*3.2
        #self.colors.value = np.ones((n,1),np.float32)*np.array([0.9,0.9,0.9,0.9])
        #self.colors_highl.value = self._get_colors_highl(self.colors.value)
        self.colors_fill.value[:] = np.ones((n, 1), np.float32)*self.color_lane_default.value

        #lanetype = np.array([21],np.int32)
        #lanetype = [21]
        #inds_ped = self._lanes.ids_modes_allow.value == lanetype
        # for ids_modes_allow in self._lanes.ids_modes_allow.value:
        #     print '   ids_modes_allow = ',ids_modes_allow,type(ids_modes_allow),ids_modes_allow==lanetype
        # print ' inds_ped = ',np.flatnonzero(inds_ped)
        #self.colors_fill.value[self._inds_map[inds_ped]] = np.array([0.9,0.0,0.0,0.8],np.float32)
        for attr in self.get_attrsman().get_group('typecolors'):  # better use get group
            # print '  lane color',attr.attrname,attr.value,attr.id_mode
            #self.colors_fill.value[self._inds_map[self._lanes.ids_mode.value==attr.id_mode]] = attr.value
            #self.colors_fill.value[self._inds_map[self._lanes.ids_mode.value==attr.id_mode]] = attr.value
            self.colors_fill[self._lanes.select_ids(self._lanes.ids_mode.value == attr.id_mode)] = attr.value
        self.colors_fill_highl.value[:] = self._get_colors_highl(self.colors_fill.value)

        #self.colors_fill_highl.value[:] = self._get_colors_highl(self.colors_fill.value)

        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()


class ConnectionDrawings(Fancylines):
    def __init__(self, ident, connections, parent,   **kwargs):

        # joinstyle
        # FLATHEAD = 0
        # BEVELHEAD = 1

        Fancylines.__init__(self, ident,  parent, name='Connection drawings',
                            is_fill=True,
                            is_outline=False,  # currently only fill implemented
                            arrowstretch=2.5,
                            is_lefthalf=True,
                            is_righthalf=True,
                            c_highl=0.3,
                            linewidth=1,
                            **kwargs)

        # self.delete('vertices')
        # self.delete('widths')

        self.add(cm.AttrConf('color_con_default', np.array([0.4, 0.8, 1.0, 0.9], np.float32),
                             groupnames=['options', 'colors'],
                             perm='wr',
                             metatype='color',
                             name='Default color',
                             info='Default connection color.',
                             ))

        self.set_netelement(connections)

    def get_netelement(self):
        return self._connections

    # def get_vertices_array(self):
    #    ids_lane_from = self._connections.ids_fromlane[self._inds_map]
    #    ids_lane_to = self._connections.ids_tolane[self._inds_map]
    #    return self._lanes.shapes.value[self._inds_map]#[self.get_ids()]

    # def get_widths_array(self):
    #    # return here only 0.9 so that there remains a little gap between the lanes
    #    return 0.8*self._lanes.widths.value[self._inds_map]#[self.get_ids()]

    def is_tool_allowed(self, tool, id_drawobj=-1):
        """
        Returns True if this tool can be applied to this drawobj.
        Optionally a particular drawobj can be specified with id_drawobj.
        """
        # basic tools:
        # return tool.ident not in   ['configure','select_handles','delete','move','stretch']
        return tool.ident in ['configure', 'select_handles']

    def set_netelement(self, connections):

        self._connections = connections
        self._lanes = connections.parent.lanes
        #self._inds_lanes = self._lanes.get_inds()

        # if len(self)>0:
        #    self.del_rows(self.get_ids())
        self.clear_rows()

        ids = self._connections.get_ids()
        #self._inds_map = self._connections.get_inds(ids)
        n = len(ids)
        #self.vertices = self._edges.shapes
        #self.widths = self._edges.widths

        self.add_rows(ids=ids,
                      beginstyles=np.ones(n)*FLATHEAD,
                      endstyles=np.ones(n)*TRIANGLEHEAD,
                      colors_fill=np.ones((n, 1), np.float32)*self.color_con_default.value,  # get()
                      colors_fill_highl=self._get_colors_highl(np.ones((n, 1), np.float32)*self.color_con_default.value)
                      )

        self.update()

    def update(self, is_update=True):
        """
        Update color, assume that there have not been structural changes of the arrays
        """
        # print 'Lanedraws.update'
        # assumes that edges have been set in set_edges
        # print 'Edgedrawing.update'
        #edgeinds = self._edges.get_inds()
        #n = len(self)

        # self.widths.value[:] = self._edges.widths_lanes.value[inds_edges]#np.ones(n, np.float)*3.2
        #self.colors.value = np.ones((n,1),np.float32)*np.array([0.9,0.9,0.9,0.9])
        #self.colors_highl.value = self._get_colors_highl(self.colors.value)
        #self.colors_fill.value[:] = np.ones((n,1),np.float32)*self.color_lane_default.value
        #self.colors_fill_highl.value[:] = self._get_colors_highl(self.colors_fill.value)
        ids = self.get_ids()
        ids_lane_from = self._connections.ids_fromlane[ids]
        ids_lane_to = self._connections.ids_tolane[ids]

        shapes_lane_from = self._lanes.shapes[ids_lane_from]
        shapes_lane_to = self._lanes.shapes[ids_lane_to]
        for _id, shape_lane_from, shape_lane_to in zip(ids, shapes_lane_from, shapes_lane_to):
            self.vertices[_id] = [shape_lane_from[-1], shape_lane_to[0]]

        # return self._lanes.shapes.value[self._inds_map]#[self.get_ids()]

        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()


class CrossingDrawings(Fancylines):
    def __init__(self, ident, crossings, parent,   **kwargs):

        # joinstyle
        # FLATHEAD = 0
        # BEVELHEAD = 1

        Fancylines.__init__(self, ident,  parent, name='Crossing drawings',
                            is_fill=True,
                            is_outline=False,  # currently only fill implemented
                            arrowstretch=2.5,
                            is_lefthalf=True,
                            is_righthalf=True,
                            c_highl=0.3,
                            **kwargs)

        # self.delete('vertices')
        self.delete('widths')

        self.add(cm.AttrConf('color_cross_default', np.array([0.8, 0.8, 0.9, 0.8], np.float32),
                             groupnames=['options', 'colors'],
                             perm='wr',
                             metatype='color',
                             name='Default color',
                             info='Default crossing color.',
                             ))

        self.set_netelement(crossings)

    def get_netelement(self):
        return self._crossings

    # def get_vertices_array(self):
    #    ids_lane_from = self._connections.ids_fromlane[self._inds_map]
    #    ids_lane_to = self._connections.ids_tolane[self._inds_map]
    #    return self._lanes.shapes.value[self._inds_map]#[self.get_ids()]

    def get_widths_array(self):
        # return here only 0.9 so that there remains a little gap between the lanes
        return self._crossings.widths[self.get_ids()]  # .value[self._inds_map]#[self.get_ids()]

    def get_widths(self, ids):
        return self._crossings.widths[ids]

    def set_widths(self, ids, widths):
        self._crossings.widths[ids] = widths

    def is_tool_allowed(self, tool, id_drawobj=-1):
        """
        Returns True if this tool can be applied to this drawobj.
        Optionally a particular drawobj can be specified with id_drawobj.
        """
        # basic tools:
        # return tool.ident not in   ['configure','select_handles','delete','move','stretch']
        return tool.ident in ['delete', 'configure']

    def set_netelement(self, crossings):

        self._crossings = crossings
        #self._lanes = crossings.parent.lanes
        edges = crossings.parent.edges
        #self._inds_lanes = self._lanes.get_inds()
        edgewidth = edges.widths
        # print 'set_netelement'
        self.clear()

        # print '  clear_rows done.'

        # if len(self)>0:
        #    self.del_rows(self.get_ids())

        ids = self._crossings.get_ids()
        #self._inds_map = self._crossings.get_inds(ids)
        n = len(ids)
        #self.vertices = self._edges.shapes

        self.add_rows(ids=ids,
                      beginstyles=np.ones(n)*FLATHEAD,
                      endstyles=np.ones(n)*FLATHEAD,
                      colors_fill=np.ones((n, 1), np.float32)*self.color_cross_default.value,  # get()
                      colors_fill_highl=self._get_colors_highl(
                          np.ones((n, 1), np.float32)*self.color_cross_default.value),
                      #widths = crossings.widths[ids],
                      )

        # print '  self.get_ids()\n',self.get_ids()
        # print '  self._crossings.get_ids()\n',self._crossings.get_ids()

        # plugins to keep grapgics syncronized with netelements
        # crossings.shapes.plugin.add_event(cm.EVTADDITEM,self.on_add_element)
        crossings.unplug()
        crossings.plugin.add_event(cm.EVTDELITEM, self.on_del_element)
        # print '  crossings.plugin.add_event done.'
        self.update()
        # print '  update done.'
        # self._crossings.enable_plugin(True)

    def update(self, is_update=True):
        """
        Update color, assume that there have not been structural changes of the arrays
        """
        # print 'Lanedraws.update'
        # assumes that edges have been set in set_edges
        # print 'Edgedrawing.update'
        #edgeinds = self._edges.get_inds()
        #n = len(self)

        # self.widths.value[:] = self._edges.widths_lanes.value[inds_edges]#np.ones(n, np.float)*3.2
        #self.colors.value = np.ones((n,1),np.float32)*np.array([0.9,0.9,0.9,0.9])
        #self.colors_highl.value = self._get_colors_highl(self.colors.value)
        #self.colors_fill.value[:] = np.ones((n,1),np.float32)*self.color_lane_default.value
        #self.colors_fill_highl.value[:] = self._get_colors_highl(self.colors_fill.value)
        ids = self._crossings.get_ids()
        #self._inds_map = self._crossings.get_inds(ids)
        n = len(ids)
        edges = self._crossings.parent.edges
        #self._inds_lanes = self._lanes.get_inds()
        edgewidth = edges.widths

        # return self._lanes.shapes.value[self._inds_map]#[self.get_ids()]
        ids_edges = self._crossings.ids_edges[ids]  # .value[self._inds_map]
        ids_edge = np.zeros(n, np.int32)
        widths_edge = np.zeros(n, np.float32)
        for i in range(n):
            ids_edge[i] = ids_edges[i][0]
            widths_edge[i] = np.sum(edgewidth[ids_edges[i]])
        #widths_edge = edges.widths[ids_edge]
        widths_crossing = self.get_widths_array()

        inds_tonode = np.flatnonzero(edges.ids_tonode[ids_edge] ==
                                     self._crossings.ids_node[ids])  # .value[self._inds_map])
        inds_fromnode = np.flatnonzero(edges.ids_fromnode[ids_edge] ==
                                       self._crossings.ids_node[ids])  # .value[self._inds_map])

        vertices = np.zeros((n, 2, 3), np.float32)

        for ind in inds_tonode:
            vertices[ind, :] = edges.shapes[ids_edge[ind]][-2:]

        for ind in inds_fromnode:
            vertices[ind, :] = edges.shapes[ids_edge[ind]][:2]

        vertices_cross = np.zeros((len(self), 2, 3), np.float32)
        # copy z-coord vertices_cross
        vertices_cross[:, :, 2] = vertices[:, :, 2]

        vertices_delta = vertices[:, 1] - vertices[:, 0]

        #angles = np.arctan2(vertices_delta[:,1],vertices_delta[:,0]) + np.pi/2
        #vertices_cross[:,0] = np.array(vertices[:,1])
        #vertices_cross[:,1,0] = vertices[:,1,0] + np.cos(angles )*10
        #vertices_cross[:,1,1] = vertices[:,1,1] + np.sin(angles )*10

        angles = np.arctan2(vertices_delta[:, 1], vertices_delta[:, 0]) - np.pi/2
        #angles = np.arctan2(vertices_delta[inds_tonode,1],vertices_delta[inds_tonode,0]) - np.pi/2
        #vertices_cross[inds_tonode,0] = vertices[inds_tonode,1]
        vertices_cross[inds_tonode, 0, 0] = vertices[inds_tonode, 1, 0] - \
            0.5*np.cos(angles[inds_tonode])*widths_edge[inds_tonode]
        vertices_cross[inds_tonode, 0, 1] = vertices[inds_tonode, 1, 1] - \
            0.5*np.sin(angles[inds_tonode])*widths_edge[inds_tonode]
        vertices_cross[inds_tonode, 1, 0] = vertices[inds_tonode, 1, 0] + \
            0.5*np.cos(angles[inds_tonode])*widths_edge[inds_tonode]
        vertices_cross[inds_tonode, 1, 1] = vertices[inds_tonode, 1, 1] + \
            0.5*np.sin(angles[inds_tonode])*widths_edge[inds_tonode]

        #angles = np.arctan2(vertices_delta[inds_fromnode,1],vertices_delta[inds_fromnode,0]) - np.pi/2
        #vertices_cross[inds_fromnode,0] = vertices[inds_fromnode,0]
        vertices_cross[inds_fromnode, 0, 0] = vertices[inds_fromnode, 0, 0] - \
            0.5*np.cos(angles[inds_fromnode])*widths_edge[inds_fromnode]
        vertices_cross[inds_fromnode, 0, 1] = vertices[inds_fromnode, 0, 1] - \
            0.5*np.sin(angles[inds_fromnode])*widths_edge[inds_fromnode]
        vertices_cross[inds_fromnode, 1, 0] = vertices[inds_fromnode, 0, 0] + \
            0.5*np.cos(angles[inds_fromnode])*widths_edge[inds_fromnode]
        vertices_cross[inds_fromnode, 1, 1] = vertices[inds_fromnode, 0, 1] + \
            0.5*np.sin(angles[inds_fromnode])*widths_edge[inds_fromnode]

        #vertices_cross[inds_fromnode,0,0] = vertices[inds_fromnode,0,0]+ np.cos(angles[inds_fromnode] +np.pi/2)*widths_crossing[inds_fromnode]
        #vertices_cross[inds_fromnode,0,1] = vertices[inds_fromnode,0,1]+ np.sin(angles[inds_fromnode] +np.pi/2)*widths_crossing[inds_fromnode]
        #vertices_cross[inds_fromnode,1,0] = vertices[inds_fromnode,0,0]+ np.cos(angles[inds_fromnode] +np.pi/2)*widths_crossing[inds_fromnode] + np.cos(angles[inds_fromnode] )*widths_edge[inds_fromnode]
        #vertices_cross[inds_fromnode,1,1] = vertices[inds_fromnode,0,1]+ np.sin(angles[inds_fromnode] +np.pi/2)*widths_crossing[inds_fromnode]   + np.sin(angles[inds_fromnode] )*widths_edge[inds_fromnode]

        #vertices_delta = vertices[:,1] -  vertices[:,0]
        # norm  = get_norm_2d(vertices_delta ).reshape(n,1)# np.sqrt( np.sum((vertices_delta)**2,0))
        # print '  norm.shape  vertices_delta.shape=', norm.shape,vertices_delta.shape
        #vertices_perp_norm = np.zeros((n,2), np.float32)
        #vertices_perp_norm[:,:] = vertices_delta[:,[1,0]]/norm
        #vertices_perp_norm[:,:] = vertices_delta[:,:2]/norm
        #vertices[:,1,:2] =  vertices[:,0,:2] + 20.0*vertices_perp_norm

        self.vertices.value[:] = vertices_cross
        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()

    def del_elem(self, id_cross):
        """
        Deletes an element from network and then in on canvas
        through callback on_del_element 
        """
        # print 'del_elem id_cross',id_cross
        # print '    len(self),len(self._zones)',len(self),len(self._zones)

        self._crossings.del_element(id_cross)

    def on_del_element(self, obj, ids):
        """
        callback from netelement
        """
        if len(ids) != 1:
            return False
        # print 'on_del_element',obj,ids
        # print '    len(self),len(self._zones)',len(self),len(self._zones)
        # print '  verices =',self._drawobj_anim.vertices[self.id_anim]
        self._is_not_synched = True
        # self.set_netelement(self._crossings)

        # print '  len(self),len(self._crossings)',len(self),len(self._crossings)
        # print '  before self.get_ids()\n',self.get_ids()
        # print '  before self._crossings.get_ids()\n',self._crossings.get_ids()

        # self.set_netelement(self._crossings)
        # self.del_drawobj(ids[0])
        _id = ids[0]
        self.del_row(_id)

        # print '  after self.get_ids()\n',self.get_ids()
        # print '  after self._crossings.get_ids()\n',self._crossings.get_ids()

        self._update_vertexvbo()
        self._update_colorvbo()

        # wx.CallAfter(self.update_internal)
        # self.update_internal()
        # print '  after CallAfter'
        # print '    len(self),len(self._zones)',len(self),len(self._zones)
        return True


NETDRAWINGS = [
    ('nodedraws', NodeDrawings, 'nodes', 20),
    ('edgedraws', EdgeDrawings, 'edges', 10),
    ('lanedraws', LaneDrawings, 'lanes', 15),
    ('connectiondraws', ConnectionDrawings, 'connections', 25),
    ('crossingsdraws', CrossingDrawings, 'crossings', 30),
]


class Neteditor(OGleditor):
    def __init__(self,
                 parent,
                 mainframe=None,
                 size=wx.DefaultSize,
                 is_menu=False,  # create menu items
                 Debug=0,
                 ):

        self._drawing = None
        self.prefix_anim = 'anim_'
        self.layer_anim = 1000.0

        wx.Panel.__init__(self, parent, wx.ID_ANY, size=size)
        sizer = wx.BoxSizer(wx.HORIZONTAL)

        self._mainframe = mainframe

        # initialize GL canvas
        navcanvas = OGLnavcanvas(self, mainframe)
        #self._canvas = OGLcanvas(self)
        self._canvas = navcanvas.get_canvas()

        # compose tool pallet here
        self._toolspanel = NeteditorTools(self)

        # compose editor window
        sizer.Add(self._toolspanel, 0, wx.ALL | wx.ALIGN_LEFT | wx.GROW, 4)  # from NaviPanelTest
        # sizer.Add(self._canvas,1,wx.GROW)# from NaviPanelTest
        sizer.Add(navcanvas, 1, wx.GROW)

        # finish panel setup
        self.SetSizer(sizer)
        sizer.Fit(self)

    def set_netdrawings(self, net):
        self._drawing.set_net(net)
        for ident_drawob, DrawobjClass, netattrname, layer in NETDRAWINGS:
            self._drawing.set_element(ident_drawob, DrawobjClass, getattr(net, netattrname), layer)

    def set_net(self, net, is_redraw=False):
        if self._drawing is None:
            self.set_drawing(Netdrawing())
            self.add_drawobjs_anim()
            self.set_netdrawings(net)
            # print 'set_net',net,id(net),drawing,id(drawing)

        else:
            # self.set_drawing(OGLdrawing())
            # self.add_drawobjs_anim()
            self.set_netdrawings(net)
            self._toolspanel.reset_initial_tool()

        # self._canvas.set_drawing(drawing)
        if is_redraw:
            self.draw()

        return self._drawing  # returned for test purposes


class Netdrawing(OGLdrawing):
    def __init__(self, parent=None):
        OGLdrawing.__init__(self, ident='netdrawing', parent=parent,
                            name='Net drawing', info='List of network draw objects')
        self._net = None  # set in set_net
        self.add_col(am.ArrayConf('idents_elem', '',
                                  dtype='object',
                                  groupnames=['options'],
                                  perm='r',
                                  is_index=True,
                                  name='Ident elem',
                                  info='Identification string of network elements.',
                                  ))

    def set_net(self, net):
        self._net = net

    def get_net(self):
        return self._net

    def get_drawobj_by_element(self, element):
        ident = element.format_ident_abs()
        if self.idents_elem.has_index(ident):
            _id = self.idents_elem.get_id_from_index(ident)
            return self.drawobjects[_id]
        else:
            None

    def highlight_element(self, element, ids, is_update=True):
        if not hasattr(ids, '__iter__'):
            ids = [ids]

        drawobj = self.get_drawobj_by_element(element)
        # print 'highlight_element',drawobj,element,ids
        # print '   drawobj',drawobj, element.format_ident_abs(), self.idents_elem._index_to_id.has_key(element.format_ident_abs())
        # print '  self.idents_elem._index_to_id',self.idents_elem._index_to_id.keys()
        if drawobj:
            drawobj.highlight(ids, is_update=is_update)

    def unhighlight_element(self, element, ids, is_update=True):
        if not hasattr(ids, '__iter__'):
            ids = [ids]
        drawobj = self.get_drawobj_by_element(element)
        # print 'unhighlight_element',drawobj,element,ids
        if drawobj:
            drawobj.unhighlight(ids, is_update=is_update)

    def set_element(self, ident_drawobj, DrawClass, element, layer=50):
        ident_abs = element.format_ident_abs()
        # print 'set_element',element,ident_abs,type(ident_abs)
        # self.print_attrs()
        drawobj = self.get_drawobj_by_ident(ident_drawobj)
        if drawobj is not None:
            drawobj.set_netelement(element)

        else:
            drawobj = DrawClass(ident_drawobj, element, self)
            _id = self.add_drawobj(drawobj, layer)
            self.idents_elem[_id] = ident_abs
            # print '  check:' , _id, self.drawobjects[_id], self.idents_elem[_id]
    # do not touch, used for anim drawobj too
    # def add_drawobj(self, drawobj, element, layer = 0):
    #    id_drawobj = self.add_rows( 1, drawobjects = [drawobj],
    #                                idents = [drawobj.get_ident()],
    #                                idents_elem = element.get_ident_abs(),
    #                                layers = [layer],
    #                                )
    #    return  id_drawobj


class NeteditorMainframe(AgileToolbarFrameMixin, wx.Frame):
    """
    Simple wx frame with some special features.
    """

    def __init__(self, net, title='Neteditor', pos=wx.DefaultPosition,
                 size=(1000, 500), style=wx.DEFAULT_FRAME_STYLE,
                 name='frame'):

        self._net = net
        # Forcing a specific style on the window.
        #   Should this include styles passed?
        style = wx.DEFAULT_FRAME_STYLE | wx.NO_FULL_REPAINT_ON_RESIZE
        wx.Frame.__init__(self, None, wx.NewId(), title, pos, size=size, style=style, name=name)
        self.gleditor = Neteditor(self)

        self.Show()  # must be here , before putting stuff on canvas

        self.init_drawing()

    def init_drawing(self):
        drawing = Netdrawing()

        edgesdrawings = EdgeDrawings(self._net.edges, drawing)
        drawing.add_drawobj(edgesdrawings)

        nodesdrawings = NodeDrawings(self._net.nodes, drawing)
        drawing.add_drawobj(nodesdrawings)

        canvas = self.gleditor.get_canvas()
        canvas.set_drawing(drawing)
        wx.CallAfter(canvas.zoom_tofit)


class NeteditorApp(wx.App):
    def __init__(self, net=None, output=False,  **kwargs):
        self._net = net
        # print 'ViewerApp.__init__',self._net
        wx.App.__init__(self, output, **kwargs)
        #wx.App.__init__(self,  **kwargs)

    def OnInit(self):
        # wx.InitAllImageHandlers()
        self.mainframe = NeteditorMainframe(self._net)
        #sumopyicon = wx.Icon(os.path.join(IMAGEDIR,'icon_sumopy.png'),wx.BITMAP_TYPE_PNG, 16,16)
        # wx.Frame.SetIcon(self.mainframe,sumopyicon)
        # self.mainframe.SetIcon(sumopyicon)

        # if True: #len(sys.argv)>=2:
        #    from  lib.net import readNet
        #    filepath = '/home/joerg/projects/sumopy/bologna/bologna4.net.xml'#sys.argv[1]
        #    _net = readNet(filepath)
        # else:
        #    _net = None

        self.SetTopWindow(self.mainframe)
        return True


def netediting(net):
    app = NeteditorApp(net, output=False)  # put in True if you want output to go to it's own window.
    # print 'call MainLoop'
    app.MainLoop()


if __name__ == '__main__':
    ###########################################################################
    # MAINLOOP
    import network
    from agilepy.lib_base.logger import Logger
    net = network.Network(logger=Logger())
    net.import_xml('facsp2', 'testnet')
    app = NeteditorApp(net, output=False)

    app.MainLoop()
