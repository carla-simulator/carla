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

# @file    results_mpl-01-preimportlib.py
# @author  Joerg Schweizer
# @date

import os
import numpy as np
from collections import OrderedDict
#import  matplotlib as mpl
#from matplotlib.patches import Arrow,Circle, Wedge, Polygon,FancyArrow
#from matplotlib.collections import PatchCollection
#import matplotlib.colors as colors
#import matplotlib.cm as cmx
#import matplotlib.pyplot as plt
#import matplotlib.image as image

from coremodules.misc.matplottools import *

import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
from agilepy.lib_base.geometry import *
from agilepy.lib_base.processes import Process


def init_plot():
    plt.close("all")
    fig = plt.figure()
    ax = fig.add_subplot(111)
    fig.tight_layout()
    return ax


def plot_net(ax, net, color_edge="gray", width_edge=2, color_node=None,
             alpha=0.5):
    for shape in net.edges.shapes.get_value():
        x_vec = np.array(shape)[:, 0]
        y_vec = np.array(shape)[:, 1]
        ax.plot(x_vec, y_vec, color=color_edge, lw=width_edge, alpha=alpha, zorder=-100)

    # do nodes
    if color_node is None:
        is_nodefill = False
        color_node = 'none'
    else:
        is_nodefill = True

    #ax.scatter(x_vec[0], y_vec[0], s=np.pi * (10.0)**2, c=colors, alpha=0.5)
    coords = net.nodes.coords.get_value()
    radii = net.nodes.radii.get_value()
    #ax.scatter(coords[:,0], coords[:,1], s=np.pi * (radii)**2, alpha=0.5)
    #patches = []
    for coord, radius in zip(coords, radii):
        ax.add_patch(Circle(coord, radius=radius,
                            linewidth=width_edge,
                            edgecolor=color_edge,
                            facecolor=color_node,
                            fill=is_nodefill,
                            alpha=alpha,
                            zorder=-50))


def plot_maps(ax, maps, alpha=0.5):
    # print 'plot_maps'
    net = maps.parent.get_net()
    rootdir = os.path.dirname(net.parent.get_rootfilepath())

    for filename, bbox in zip(maps.filenames.get_value(), maps.bboxes.get_value()):
        # print '  ',filename,
        if filename == os.path.basename(filename):
            # filename does not contain path info
            filepath = os.path.join(rootdir, filename)
        else:
            # filename contains path info (can happen if interactively inserted)
            filepath = filename
        # print '  filepath',filepath
        im = image.imread(filepath)
        myaximage = ax.imshow(im, aspect='auto', extent=(
            bbox[0, 0], bbox[1, 0], bbox[0, 1], bbox[1, 1]), alpha=alpha, zorder=-1000)


def plot_facilities(ax, facilities, color_building="gray",
                    color_outline='white', width_line=2,
                    alpha=0.5):

    if color_building is None:
        is_fill = False
        color_building = 'none'
    else:
        is_fill = True

    for shape in facilities.shapes.get_value():
        #x_vec = np.array(shape)[:,0]
        #y_vec = np.array(shape)[:,1]
        # ax.plot(x_vec, y_vec, color = color_building, lw = width_line,
        #        alpha=alpha )
        ax.add_patch(Polygon(np.array(shape)[:, :2],
                             linewidth=width_line,
                             edgecolor=color_outline,
                             facecolor=color_building,
                             fill=is_fill,
                             alpha=alpha,
                             zorder=-200))


def plot_edgevalues_lines(ax, ids_result, config_ids_edge, values,
                          width_max=10.0, alpha=0.8, printformat='%.2f',
                          color_outline=None, color_fill=None,
                          color_label='black', is_antialiased=True,
                          is_fill=True, is_widthvalue=True,
                          arrowshape='left', length_arrowhead=10.0,
                          headwidthstretch=1.3,
                          fontsize=32,
                          valuelabel=''):

    head_width = headwidthstretch*width_max
    fontsize_ticks = int(0.8*fontsize)

    edges = config_ids_edge.get_linktab()
    ids_edges = config_ids_edge[ids_result]
    #values = config_values[ids_result]
    values_norm = np.array(values, dtype=np.float32)/np.max(values)

    patches = []
    displacement = float(width_max)/2.0
    if is_widthvalue:
        linewidths = width_max*values_norm
    else:
        linewidths = width_max * np.ones(len(values_norm), np.float32)

    deltaangle_text = -np.pi/2.0
    displacement_text = displacement+width_max

    jet = cm_jet = plt.get_cmap('jet')
    c_norm = colors.Normalize(vmin=0, vmax=np.max(values))

    # http://stackoverflow.com/questions/8342549/matplotlib-add-colorbar-to-a-sequence-of-line-plots/11558629#11558629
    sm = cmx.ScalarMappable(norm=c_norm, cmap=jet)
    sm.set_array(values_norm)
    for id_edge, value, value_norm, linewidth in zip(ids_edges, values, values_norm, linewidths):
        shape, angles_perb = get_resultshape(edges, id_edge, displacement)
        x_vec = np.array(shape)[:, 0]
        y_vec = np.array(shape)[:, 1]
        deltax = x_vec[-1]-x_vec[0]
        deltay = y_vec[-1]-y_vec[0]
        # http://matplotlib.org/users/pyplot_tutorial.html
        line = mpl.lines.Line2D(x_vec, y_vec, color=sm.to_rgba(value),
                                linewidth=linewidth,
                                antialiased=is_antialiased,
                                alpha=alpha,
                                solid_capstyle='round',
                                zorder=0,
                                )
        ax.add_line(line)

        if printformat is not '':
            angles_text = np.arctan2(deltay, deltax)+deltaangle_text
            # print '  angles_text',printformat%value,angles_text/(np.pi)*180,(angles_text>np.pi/2),(angles_text<3*np.pi/2)
            if (angles_text > np.pi/2) | (angles_text < -np.pi/2):
                angles_text += np.pi
            x_label = x_vec[0]+0.66*deltax+displacement_text*np.cos(angles_text)
            y_label = y_vec[0]+0.66*deltay+displacement_text*np.sin(angles_text)

            ax.text(x_label, y_label, printformat % value,
                    rotation=angles_text/(np.pi)*180,
                    color=color_label,
                    fontsize=fontsize_ticks,
                    zorder=10,
                    )

    if is_fill:
        cbar = plt.colorbar(sm)
        # mpl.setp(cbar.ax.yaxis.get_ticklabels(),  fontsize=fontsize)#weight='bold',
        # cb.ax.tick_params(labelsize=font_size)
        if valuelabel != '':
            cbar.ax.set_ylabel(valuelabel, fontsize=fontsize)  # , weight="bold")
        for l in cbar.ax.yaxis.get_ticklabels():
            # l.set_weight("bold")
            l.set_fontsize(fontsize_ticks)


def plot_edgevalues_arrows(ax, ids_result, config_ids_edge, values,
                           width_max=10.0, alpha=0.8, printformat='%.2f',
                           color_outline=None, color_fill=None,
                           color_label='black', is_antialiased=True,
                           is_fill=True, is_widthvalue=True,
                           arrowshape='left', length_arrowhead=10.0,
                           headwidthstretch=1.3,
                           fontsize=32,
                           valuelabel=''):

    head_width = headwidthstretch*width_max
    fontsize_ticks = int(0.8*fontsize)

    edges = config_ids_edge.get_linktab()
    ids_edges = config_ids_edge[ids_result]
    #values = config_values[ids_result]
    values_norm = np.array(values, dtype=np.float32)/np.max(values)

    patches = []
    displacement = float(width_max)/4.0
    if is_widthvalue:
        linewidths = width_max*values_norm
    else:
        linewidths = width_max * np.ones(len(values_norm), np.float32)

    deltaangle_text = -np.pi/2.0
    displacement_text = displacement+width_max
    for id_edge, value, value_norm, linewidth in zip(ids_edges, values, values_norm, linewidths):
        shape, angles_perb = get_resultshape(edges, id_edge, displacement)
        x_vec = np.array(shape)[:, 0]
        y_vec = np.array(shape)[:, 1]
        deltax = x_vec[-1]-x_vec[0]
        deltay = y_vec[-1]-y_vec[0]

        if printformat is not '':
            angles_text = np.arctan2(deltay, deltax)+deltaangle_text
            if (angles_text > np.pi/2) | (angles_text < -np.pi/2):
                angles_text += np.pi
            x_label = x_vec[0]+0.66*deltax+displacement_text*np.cos(angles_text)
            y_label = y_vec[0]+0.66*deltay+displacement_text*np.sin(angles_text)

            ax.text(x_label, y_label, printformat % value,
                    rotation=angles_text/(np.pi)*180,
                    color=color_label,
                    fontsize=fontsize_ticks,
                    zorder=10,
                    )

        if is_widthvalue:
            head_width = headwidthstretch*linewidth
        arrow = FancyArrow(x_vec[0], y_vec[0], deltax, deltay, width=linewidth,
                           antialiased=is_antialiased,
                           edgecolor=color_outline, facecolor=color_fill,
                           head_width=head_width, head_length=length_arrowhead,
                           length_includes_head=True,
                           fill=True, shape=arrowshape, zorder=0)
        patches.append(arrow)

    if is_fill:
        alpha_patch = alpha
        patchcollection = PatchCollection(patches, cmap=mpl.cm.jet, alpha=alpha_patch)
        patchcollection.set_array(values)
        ax.add_collection(patchcollection)
        cbar = plt.colorbar(patchcollection)
        if valuelabel != '':
            cbar.ax.set_ylabel(valuelabel, fontsize=fontsize)  # , weight="bold")
        for l in cbar.ax.yaxis.get_ticklabels():
            # l.set_weight("bold")
            l.set_fontsize(fontsize_ticks)
    else:
        for patch in patches:
            ax.add_patch(patch)


def get_resultshape(edges, id_edge, dispacement):
    """
    Return resultshape coords for this edge.
    """
    shape = np.array(edges.shapes[id_edge], np.float32)
    n_vert = len(shape)
    resultshape = np.zeros(shape.shape, np.float32)
    #laneshapes = np.zeros((n_lanes,n_vert,3), np.float32)
    angles_perb = get_angles_perpendicular(shape)
    if edges.types_spread[id_edge] == 0:  # right spread

        resultshape[:, 0] = shape[:, 0] + np.cos(angles_perb) * dispacement
        resultshape[:, 1] = shape[:, 1] + np.sin(angles_perb) * dispacement
        return resultshape, angles_perb
    else:
        return shape.copy(), angles_perb


def show_plot():
    plt.show()


class PlotoptionsMixin:
    def add_plotoptions(self, **kwargs):

        attrsman = self.get_attrsman()
        plottypes = ['arrows', 'polygons']
        self.plottype = attrsman.add(cm.AttrConf('plottype', kwargs.get('plottype', 'arrows'),
                                                 choices=plottypes,
                                                 groupnames=['options'],
                                                 name='Plot types',
                                                 info='Plot type representing the results.',
                                                 ))

        self.is_show_title = attrsman.add(cm.AttrConf('is_show_title', kwargs.get('is_show_title', True),
                                                      groupnames=['options'],
                                                      name='Show tile',
                                                      info='Shows title and unit.',
                                                      ))

        self.size_titlefont = attrsman.add(cm.AttrConf('size_titlefont', kwargs.get('size_titlefont', 32),
                                                       groupnames=['options'],
                                                       name='Title fontsize',
                                                       info='Title fontsize.',
                                                       ))

        self.size_labelfont = attrsman.add(cm.AttrConf('size_labelfont', kwargs.get('size_labelfont', 24),
                                                       groupnames=['options'],
                                                       name='Label fontsize',
                                                       info='Label fontsize.',
                                                       ))

        self.resultwidth = attrsman.add(cm.AttrConf('resultwidth', kwargs.get('resultwidth', 10.0),
                                                    groupnames=['options'],
                                                    name='Result width',
                                                    unit='m',
                                                    info='Maximum width of graphical resuls on map.',
                                                    ))

        self.length_arrowhead = attrsman.add(cm.AttrConf('length_arrowhead', kwargs.get('length_arrowhead', 10.0),
                                                         groupnames=['options'],
                                                         name='Arrow length',
                                                         unit='m',
                                                         info='Length of arrowhead on result map.',
                                                         ))

        self.is_widthvalue = attrsman.add(cm.AttrConf('is_widthvalue', kwargs.get('is_widthvalue', False),
                                                      groupnames=['options'],
                                                      name='Value width?',
                                                      info='If True, the arrow width of the graphical representation is proportional to the result value.',
                                                      ))

        self.is_colorvalue = attrsman.add(cm.AttrConf('is_colorvalue', kwargs.get('is_colorvalue', True),
                                                      groupnames=['options'],
                                                      name='Value color?',
                                                      info='If True, the arrows of the graphical representation are filled with a colour representing the result value.',
                                                      ))

        self.color_outline = attrsman.add(cm.AttrConf('color_outline', kwargs.get('color_outline', np.array([0.0, 0.0, 0.0, 0.95], dtype=np.float32)),
                                                      groupnames=['options'],
                                                      perm='wr',
                                                      metatype='color',
                                                      name='Outline color',
                                                      info='Outline color of result arrows in graphical representation. Only valid if no color-fill is chosen.',
                                                      ))

        self.color_fill = attrsman.add(cm.AttrConf('color_fill', kwargs.get('color_fill', np.array([0.3, 0.3, 1.0, 0.95], dtype=np.float32)),
                                                   groupnames=['options'],
                                                   perm='wr',
                                                   metatype='color',
                                                   name='Fill color',
                                                   info='Fill color of result arrows in graphical representation.',
                                                   ))

        self.alpha_results = attrsman.add(cm.AttrConf('alpha_results', kwargs.get('alpha_results',  0.8),
                                                      groupnames=['options'],
                                                      name='Result transparency',
                                                      info='Transparency of result arrows in graphical representation.',
                                                      ))

        self.printformat = attrsman.add(cm.AttrConf('printformat', kwargs.get('printformat', '%.1f'),
                                                    choices=OrderedDict([
                                                        ('Show no values', ''),
                                                        ('x', '%.d'),
                                                        ('x.x', '%.1f'),
                                                        ('x.xx', '%.2f'),
                                                        ('x.xxx', '%.3f'),
                                                        ('x.xxxx', '%.4f'),
                                                    ]),
                                                    groupnames=['options'],
                                                    name='Label formatting',
                                                    info='Print formatting of value label in graphical representation.',
                                                    ))

        self.color_label = attrsman.add(cm.AttrConf('color_label', kwargs.get('color_label', np.array([0, 0, 0, 1], dtype=np.float32)),
                                                    groupnames=['options'],
                                                    perm='wr',
                                                    metatype='color',
                                                    name='Label color',
                                                    info='Color of value label in graphical representation.',
                                                    ))

        self.is_show_network = attrsman.add(cm.AttrConf('is_show_network', kwargs.get('is_show_network', True),
                                                        groupnames=['options'],
                                                        name='Show network',
                                                        info='Shows a schematic network in the background.',
                                                        ))

        self.color_network = attrsman.add(cm.AttrConf('color_network', kwargs.get('color_network', np.array([0.8, 0.8, 0.8, 0.8], dtype=np.float32)),
                                                      groupnames=['options'],
                                                      perm='wr',
                                                      metatype='color',
                                                      name='Network color',
                                                      info='Outline color of schematic network in the background.',
                                                      ))

        self.color_nodes = attrsman.add(cm.AttrConf('color_nodes', kwargs.get('color_nodes', np.array([1, 1, 1, 1], dtype=np.float32)),
                                                    groupnames=['options'],
                                                    perm='wr',
                                                    metatype='color',
                                                    name='Nodes color',
                                                    info='Color of simplified nodes (or juctions in the background.',
                                                    ))
        self.alpha_net = attrsman.add(cm.AttrConf('alpha_net', kwargs.get('alpha_net', 0.5),
                                                  groupnames=['options'],
                                                  name='Net transparency',
                                                  info='Transparency of network (edges and nodes) in graphical representation.',
                                                  ))

        self.is_show_facilities = attrsman.add(cm.AttrConf('is_show_facilities', kwargs.get('is_show_facilities', True),
                                                           groupnames=['options'],
                                                           name='Show facilities',
                                                           info='Shows a schematic facilities (buildings, parks, etc.) in the background.',
                                                           ))

        self.color_facilities = attrsman.add(cm.AttrConf('color_facilities',  kwargs.get('color_facilities', np.array([1, 1, 1, 1], dtype=np.float32)),
                                                         groupnames=['options'],
                                                         perm='wr',
                                                         metatype='color',
                                                         name='Facilities color',
                                                         info='Color of schematic facilities in the background.',
                                                         ))

        self.alpha_facilities = attrsman.add(cm.AttrConf('alpha_facilities', kwargs.get('alpha_facilities', 0.5),
                                                         groupnames=['options'],
                                                         name='Facility transparency',
                                                         info='Transparency of facilities in graphical representation.',
                                                         ))

        self.color_borders = attrsman.add(cm.AttrConf('color_borders', kwargs.get('color_borders', np.array([0.7, 0.7, 0.7, 0.8], dtype=np.float32)),
                                                      groupnames=['options'],
                                                      perm='wr',
                                                      metatype='color',
                                                      name='Border color',
                                                      info='Facility border (or building walls) color of schematic facilities in the background.',
                                                      ))

        self.color_background = attrsman.add(cm.AttrConf('color_background', kwargs.get('color_background', np.array([1, 1, 1, 1], dtype=np.float32)),
                                                         groupnames=['options'],
                                                         perm='wr',
                                                         metatype='color',
                                                         name='Background color',
                                                         info='Background color of schematic network in the background.',
                                                         ))

        self.is_show_maps = attrsman.add(cm.AttrConf('is_show_maps', kwargs.get('is_show_maps', False),
                                                     groupnames=['options'],
                                                     name='Show map?',
                                                     info='If True, shows map as background in graphical representation. This feature requires that maps have been previously downloaded.',
                                                     ))

        self.alpha_maps = attrsman.add(cm.AttrConf('alpha_maps', kwargs.get('alpha_maps', 0.5),
                                                   groupnames=['options'],
                                                   name='Map transparency',
                                                   info='Transparency of background maps in graphical representation.',
                                                   ))

        self.is_grid = attrsman.add(cm.AttrConf('is_grid', kwargs.get('is_grid', False),
                                                groupnames=['options'],
                                                name='Show grid?',
                                                info='If True, shows a grid on the graphical representation.',
                                                ))

        self.axis = None

    def plot_results_on_map(self, axis, ids, values, title='', valuelabel=''):
        net = self.parent.get_scenario().net
        axis.set_axis_bgcolor(self.color_background)
        if self.is_show_network:
            plot_net(axis, net, color_edge=self.color_network, width_edge=2,
                     color_node=self.color_nodes, alpha=self.alpha_net)

        if self.is_show_facilities:
            facilities = self.parent.get_scenario().landuse.facilities
            plot_facilities(axis, facilities, color_building=self.color_facilities,
                            color_outline=self.color_borders,
                            width_line=2, alpha=self.alpha_facilities,
                            )
        if self.is_show_maps:
            plot_maps(axis, self.parent.get_scenario().landuse.maps, alpha=self.alpha_maps)

        if len(ids > 0):
            if self.plottype == 'arrows':  # = ['Arrows','Polygons']
                plot_edgevalues_arrows(axis, ids,
                                       self.parent.edgeresults.ids_edge,
                                       values,
                                       width_max=self.resultwidth,
                                       alpha=self.alpha_results,
                                       printformat=self.printformat,
                                       color_outline=self.color_outline,
                                       color_fill=self.color_fill,
                                       color_label=self.color_label,
                                       is_antialiased=True,
                                       is_fill=self.is_colorvalue,
                                       is_widthvalue=self.is_widthvalue,
                                       length_arrowhead=self.length_arrowhead,
                                       fontsize=self.size_labelfont,
                                       valuelabel=valuelabel,
                                       )
            elif self.plottype == 'polygons':
                plot_edgevalues_lines(axis, ids,
                                      self.parent.edgeresults.ids_edge,
                                      values,  # values for result ids
                                      width_max=self.resultwidth,
                                      alpha=self.alpha_results,
                                      printformat=self.printformat,
                                      color_outline=self.color_outline,
                                      color_fill=self.color_fill,
                                      color_label=self.color_label,
                                      is_antialiased=True,
                                      is_fill=self.is_colorvalue,
                                      is_widthvalue=self.is_widthvalue,
                                      length_arrowhead=self.length_arrowhead,
                                      fontsize=self.size_labelfont,
                                      valuelabel=valuelabel,
                                      )

        if self.is_show_title:
            axis.set_title(title, fontsize=self.size_titlefont)

        axis.axis('equal')
        # ax.legend(loc='best',shadow=True)

        axis.grid(self.is_grid)
        axis.set_xlabel('West-East [m]', fontsize=self.size_labelfont)
        axis.set_ylabel('South-North [m]', fontsize=self.size_labelfont)
        axis.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        axis.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))
        show_plot()


class Flowcomparison(PlotoptionsMixin, Process):
    def __init__(self, results, name='Compare flows with Matplotlib',
                 info="Compare simulated and estimated flows usind Matplotlib",
                 logger=None):

        self._init_common('flowcomparator', parent=results, name=name,
                          info=info, logger=logger)

        # print 'Flowcomparison.__init__',results,self.parent
        attrsman = self.get_attrsman()

        self.add_plotoptions()

    def show(self):
        # print 'show diff'
        # if self.axis  is None:
        plt.close("all")
        fig = plt.figure()
        ax_comp = fig.add_subplot(211)
        ax_net = fig.add_subplot(212)

        # else:
        net = self.parent.get_scenario().net
        flow_sim_conf = getattr(self.parent.edgeresults, 'entered')
        flow_est_conf = getattr(self.parent.edgeresults, 'entered_est')

        ids = self.parent.edgeresults.select_ids(flow_est_conf.get_value() > 0)
        flow_est = flow_est_conf[ids]
        flow_sim = flow_sim_conf[ids]

        if len(flow_est) == 0:
            return

        ax_comp.plot(flow_sim, flow_est, 'o', markerfacecolor=self.color_fill)

        min_sim = np.min(flow_sim)
        max_sim = np.max(flow_sim)
        min_est = np.min(flow_est)
        max_est = np.max(flow_est)

        f_max = np.max([max_sim, max_est])
        # print '  max_sim,max_est,f_max',max_sim,max_est,f_max
        ax_comp.plot([0.0, f_max], [0.0, f_max], '-k', zorder=-1, linewidth=3)

        ax_comp.grid(self.is_grid)
        ax_comp.set_xlabel(flow_sim_conf.format_symbol()+flow_sim_conf.format_unit(show_parentesis=True))
        ax_comp.set_ylabel(flow_est_conf.format_symbol()+flow_est_conf.format_unit(show_parentesis=True))
        ax_comp.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax_comp.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))

        title = 'Diff. between '+flow_sim_conf.get_name()+' and '+flow_est_conf.get_name()
        flow_delta = np.abs(flow_sim_conf[ids]-flow_est_conf[ids])
        self.plot_results_on_map(ax_net, ids, flow_delta, title, valuelabel=r'$f$-$\hat{f}$')

        fig.tight_layout()
        show_plot()

    def do(self):
        # print 'do',self.edgeattrname
        self.show()

    def get_scenario(self):
        return self._scenario


class Resultplotter(PlotoptionsMixin, Process):
    def __init__(self, results, name='Plot results with Matplotlib',
                 info="Creates plots of different results using matplotlib",
                 logger=None, **kwargs):

        self._init_common('resultplotter', parent=results, name=name,
                          info=info, logger=logger)

        # print 'Resultplotter.__init__',results,self.parent
        attrsman = self.get_attrsman()

        # edgeresultes....
        attrnames_edgeresults = OrderedDict()
        edgeresultattrconfigs = self.parent.edgeresults.get_group_attrs('results')
        edgeresultattrnames = edgeresultattrconfigs.keys()
        # edgeresultattrnames.sort()
        for attrname in edgeresultattrnames:
            attrconfig = edgeresultattrconfigs[attrname]

            attrnames_edgeresults[attrconfig.format_symbol()] = attrconfig.attrname

        #attrnames_edgeresults = {'Entered':'entered'}
        self.edgeattrname = attrsman.add(cm.AttrConf('edgeattrname', 'entered',
                                                     choices=attrnames_edgeresults,
                                                     groupnames=['options'],
                                                     name='Edge Quantity',
                                                     info='The edge related quantity to be plotted.',
                                                     ))

        self.add_plotoptions(**kwargs)

    def show(self):
        # print 'show',self.edgeattrname
        # if self.axis  is None:
        axis = init_plot()
        if (self.edgeattrname is not ""):
            resultattrconf = getattr(self.parent.edgeresults, self.edgeattrname)
            ids = self.parent.edgeresults.get_ids()
            title = resultattrconf.get_info()  # +resultattrconf.format_unit(show_parentesis=True)#format_symbol()
            self.plot_results_on_map(axis, ids, resultattrconf[ids], title, valuelabel=resultattrconf.format_symbol())

        show_plot()

    def do(self):
        # print 'do',self.edgeattrname
        self.show()

    def get_scenario(self):
        return self._scenario
