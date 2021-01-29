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

# @file    publictransportnet_wxgui.py
# @author  Joerg Schweizer
# @date

import wx
import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
from agilepy.lib_wx.ogleditor import *
from agilepy.lib_wx.objpanel import ObjPanel
from agilepy.lib_base.processes import Process
from agilepy.lib_wx.processdialog import ProcessDialog
from network import SumoIdsConf, MODES
import publictransportnet as pt


class PtStopDrawings(Rectangles):
    def __init__(self, ident, ptstops, parent,   **kwargs):

        Rectangles.__init__(self, ident,  parent,
                            name='PT Stop drawings',
                            is_parentobj=False,
                            is_fill=True,
                            is_outline=True,
                            c_highl=0.3,
                            linewidth=4,
                            **kwargs)

        self.delete('offsets')
        self.delete('widths')
        self.delete('lengths')
        self.delete('rotangles_xy')

        self.add(cm.AttrConf('color_outline', np.array([0.093, 0.738, 0.1093, 1.0], np.float32),
                             groupnames=['options'],
                             perm='wr',
                             metatype='color',
                             name='Default color',
                             info='Default color.',
                             ))
        self.add(cm.AttrConf('color_fill', np.array([0.96, 0.875, 0.7, 0.8], np.float32),
                             groupnames=['options'],
                             perm='wr',
                             metatype='color',
                             name='Default color',
                             info='Default color.',
                             ))

        self.set_netelement(ptstops)

    def get_netelement(self):
        return self._ptstops

    def set_netelement(self, ptstops):
        # print '\nset_netelement ptstops',len(ptstops)
        self._ptstops = ptstops
        if len(self) > 0:
            self.clear_rows()  # self.del_rows(1*self.get_ids())

        ids = self._ptstops.get_ids()
        #self._inds_map = self._parking.get_inds(ids)
        self.add_rows(ids=ids)
        self.update()

    def get_vertices_array(self):
        # print 'Rectangles.get_vertices_array'
        return self._vertices_array

    def update_vertices_array(self):
        n = len(self)
        # print 'update_vertices_array',n
        lanes = self._ptstops.parent.lanes
        #offsets = self.get_offsets_array()
        #widths = self.get_widths_array()
        #lengths = self.get_lengths_array()
        #alphas = self.get_rotangles_xy_array()
        ids = self.get_ids()
        ids_lane = self._ptstops.ids_lane[ids]
        lengths = self._ptstops.widths[ids]
        widths = self._ptstops.positions_to[ids]-self._ptstops.positions_from[ids]
        widths_lane = lanes.widths[ids_lane]

        offsets = np.zeros((n, 3), np.float32)
        coords_to = np.zeros((n, 3), np.float32)

        get_coord_from_pos = lanes.get_coord_from_pos
        #lane = self.get_lane(ident)
        i = 0
        for id_lane, pos_from, pos_to in zip(
            ids_lane,
            self._ptstops.positions_from[ids],
            self._ptstops.positions_to[ids]
        ):
            offsets[i] = get_coord_from_pos(id_lane, pos_from)
            coords_to[i] = get_coord_from_pos(id_lane, pos_to)

            i += 1

        deltas = coords_to-offsets
        alphas = np.arctan2(deltas[:, 1], deltas[:, 0])
        #x_label = x_vec[0]+0.5*deltax+displacement_text*np.cos(angles_text)
        #    y_label = y_vec[0]+0.5*deltay+displacement_text*np.sin(angles_text)
        # print '  widths',widths
        # print '  widths_lane',widths_lane
        # print '  lengths',lengths
        # print '  alphas',alphas
        # print '  offsets',offsets
        # shift stop positions by on lane width
        # TODO: introduce  parameter left/right of lane
        offsets[:, 0] += widths_lane*np.cos(alphas-np.pi/2)
        offsets[:, 1] += widths_lane*np.sin(alphas-np.pi/2)
        # print '  offsets',offsets

        sin_alphas = np.sin(alphas)
        cos_alphas = np.cos(alphas)
        zeros = np.zeros(n, np.float32)
        vertices = np.zeros((n, 4, 3), np.float32)

        vertices[:, 0, :] = offsets
        vertices[:, 1, :] = offsets
        vertices_rot = rotate_vertices(widths, zeros,
                                       sin_alphas=sin_alphas,
                                       cos_alphas=cos_alphas,
                                       is_array=True,
                                       )

        # print ' vertices_rot = ',vertices_rot.shape,vertices_rot,
        vertices[:, 1, :2] += rotate_vertices(widths, zeros,
                                              sin_alphas=sin_alphas,
                                              cos_alphas=cos_alphas,
                                              is_array=True,
                                              )

        vertices[:, 2, :] = offsets
        vertices[:, 2, :2] += rotate_vertices(widths, lengths,
                                              sin_alphas=sin_alphas,
                                              cos_alphas=cos_alphas,
                                              is_array=True,
                                              )

        vertices[:, 3, :] = offsets
        vertices[:, 3, :2] += rotate_vertices(zeros, lengths,
                                              sin_alphas=sin_alphas,
                                              cos_alphas=cos_alphas,
                                              is_array=True,
                                              )

        self._vertices_array = vertices

    def update(self, is_update=True):
        # assumes that arrsy structure did not change
        # print 'update'

        n = len(self)
        if n == 0:
            return
        self.update_vertices_array()

        self.colors.value[:] = np.ones((n, 4), np.float32)*self.color_outline.get_value()
        self.colors_highl.value[:] = self._get_colors_highl(self.colors.get_value())
        self.colors_fill.value[:] = np.ones((n, 4), np.float32)*self.color_fill.get_value()
        self.colors_fill_highl.value[:] = self._get_colors_highl(self.colors_fill.get_value())

        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()

    def update_colours(self, ids=None, is_update=True):
        if ids is None:
            ids = self.get_ids()
        n = len(ids)
        inds = self.get_inds(ids)
        self.colors.value[inds] = np.ones((n, 4), np.float32)*self.color_outline.get_value()
        self.colors_highl.value[inds] = self._get_colors_highl(self.colors.value[inds])
        self.colors_fill.value[inds] = np.ones((n, 4), np.float32)*self.color_fill.get_value()
        self.colors_fill_highl.value[inds] = self._get_colors_highl(self.colors_fill.value[inds])

        #self.colors_highl.value[:] = self._get_colors_highl(self.colors.value)
        if is_update:
            self._update_colorvbo()

    def get_offsets_array(self):
        pass

    def get_offsets(self, ids):
        pass

    def set_offsets(self, ids, values):
        #self.offsets[ids] = values
        pass

    def get_widths_array(self):
        pass

    def get_widths(self, ids):
        pass

    def set_widths(self, ids, values):
        #self.widths[ids] = values
        pass

    def get_lengths_array(self):
        pass

    def get_lengths(self, ids):
        pass

    def set_lengths(self, ids, values):
        self.lengths[ids] = values

    def get_rotangles_xy_array(self):
        pass

    def get_rotangles_xy(self, ids):
        pass

    def set_rotangles_xy(self, ids, values):
        pass


class PtWxGuiMixin:
    """Contains Public Transport specific functions that communicate between the widgets of the main wx gui
    and the functions of the plugin.
    """

    def refresh_pt(self, is_refresh):
        if is_refresh:
            neteditor = self.get_neteditor()
            # neteditor.add_toolclass(AddTurnflowTool)

            # add or refresh parking drawing
            drawing = neteditor.get_drawing()
            drawing.set_element('ptstopdraws', PtStopDrawings,
                                self._net.ptstops, layer=50)

    def add_menu_pt(self, menubar):
        menubar.append_menu('network/public transport',
                            bitmap=self.get_icon("icon_station.png"),
                            )

        menubar.append_item('network/public transport/import stops...',
                            self.on_import_ptstops,
                            info='Import stops from xml file.',
                            bitmap=self.get_agileicon("Document_Import_24px.png"),
                            )

        # menubar.append_item( 'network/public transport/normalize turn-probabilities',
        #    self.on_normalize_turnprobabilities,
        #    #info='Makes sure that sum of turn probabilities from an edge equals 1.',
        #    #bitmap = self.get_icon("Document_Import_24px.png"),
        #    )

        # menubar.append_item( 'network/public transport/turnflows to routes',
        #    self.on_turnflows_to_routes,
        #    #info='Makes sure that sum of turn probabilities from an edge equals 1.',
        #    #bitmap = self.get_icon("Document_Import_24px.png"),
        #    )

        menubar.append_item('network/public transport/adjust stops',
                            self.on_adjust_ptstops,
                            info='Adjust public transport stops.',
                            #bitmap = wx.ArtProvider.GetBitmap(wx.ART_DELETE,wx.ART_MENU),
                            )

        menubar.append_item('network/public transport/provide stop access...',
                            self.on_provide_stopaccess,
                            )

        menubar.append_item('network/public transport/clear stops',
                            self.on_clear_ptstops,
                            info='Clear public transport stops.',
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_DELETE, wx.ART_MENU),
                            )

    def on_import_ptstops(self, event=None):
        wildcards_all = "All files (*.*)|*.*"
        wildcards_xml = "XML files (*.xml)|*.xml"
        wildcards_stops = "SUMO additional files (*add.xml)|*.add.xml| SUMO stop files (*stops.xml)|*.stops.xml"
        wildcards = wildcards_stops+"|"+wildcards_xml+"|"+wildcards_all

        # Finally, if the directory is changed in the process of getting files, this
        # dialog is set up to change the current working directory to the path chosen.
        defaultfilepath = self._net.ptstops.get_stopfilepath()
        print '  defaultfilepath', defaultfilepath

        dlg = wx.FileDialog(
            self._mainframe, message="Open stops XML file",
            #defaultDir = self._net.get_workdirpath(),
            defaultFile=defaultfilepath,
            wildcard=wildcards,
            style=wx.OPEN | wx.CHANGE_DIR
        )

        # Show the dialog and retrieve the user response. If it is the OK response,
        # process the data.
        if dlg.ShowModal() == wx.ID_OK:
            # This returns a Python list of files that were selected.
            filepath = dlg.GetPath()
            if len(filepath) > 0:

                self._net.ptstops.import_sumostops(filepath, logger=self._mainframe.get_logger())
                self._mainframe.browse_obj(self._net.ptstops)
                # this should update all widgets for the new scenario!!
                # print 'call self._mainframe.refresh_moduleguis()'
                self._mainframe.refresh_moduleguis()

        # Destroy the dialog. Don't do this until you are done with it!
        # BAD things can happen otherwise!
        dlg.Destroy()

    def on_clear_ptstops(self, event=None):
        """Deletes all public transport stops on the network."""
        self._net.ptstops.clear_stops()
        self._mainframe.browse_obj(self._net.ptstops)
        self._mainframe.refresh_moduleguis()

    def on_adjust_ptstops(self, event=None):
        """
        Adjust the positions of stops and give them a minimum length
        of 20m.
        """
        self._net.ptstops.adjust()
        self._mainframe.browse_obj(self._net.ptstops)
        self._mainframe.refresh_moduleguis()

    def on_provide_stopaccess(self, event=None):
        """
        Make sure, all public transport stops are accessible by foot or bike
        from the road network. 
        """
        p = pt.StopAccessProvider(self._net, logger=self._mainframe.get_logger())
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
