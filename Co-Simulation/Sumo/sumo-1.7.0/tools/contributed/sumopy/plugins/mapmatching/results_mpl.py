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

# @file    results_mpl.py
# @author  Joerg Schweizer
# @date

import os
import numpy as np
from collections import OrderedDict
#import  matplotlib as mpl
from agilepy.lib_base.geometry import *
from agilepy.lib_base.misc import get_inversemap
from coremodules.misc.matplottools import *
import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
from agilepy.lib_base.geometry import *
from agilepy.lib_base.processes import Process
from mapmatching import COLOR_MATCHED_ROUTE, COLOR_SHORTEST_ROUTE, COLOR_FASTEST_ROUTE


try:
    from scipy import interpolate
    is_scipy = True
except:
    is_scipy = False


def is_sublist(l, s):
    sub_set = False
    if s == []:
        sub_set = True
    elif s == l:
        sub_set = True
    elif len(s) > len(l):
        sub_set = False

    else:
        for i in range(len(l)):
            if l[i] == s[0]:
                n = 1
                while (n < len(s)) and (l[i+n] == s[n]):
                    n += 1

                if n == len(s):
                    sub_set = True

    return sub_set


from numpy.linalg import inv


def kf_update(X, P, Y, H, R):
    IM = dot(H, X)
    IS = R + dot(H, dot(P, H.T))
    K = dot(P, dot(H.T, inv(IS)))
    X = X + dot(K, (Y-IM))
    P = P - dot(K, dot(IS, K.T))
    LH = gauss_pdf(Y, IM, IS)
    return (X, P, K, IM, IS, LH)


def gauss_pdf(X, M, S):
    if M.shape()[1] == 1:
        DX = X - tile(M, X.shape()[1])
        E = 0.5 * sum(DX * (dot(inv(S), DX)), axis=0)
        E = E + 0.5 * M.shape()[0] * log(2 * pi) + 0.5 * log(det(S))
        P = exp(-E)
    elif X.shape()[1] == 1:
        DX = tile(X, M.shape()[1]) - M
        E = 0.5 * sum(DX * (dot(inv(S), DX)), axis=0)
        E = E + 0.5 * M.shape()[0] * log(2 * pi) + 0.5 * log(det(S))
        P = exp(-E)
    else:
        DX = X-M
        E = 0.5 * dot(DX.T, dot(inv(S), DX))
        E = E + 0.5 * M.shape()[0] * log(2 * pi) + 0.5 * log(det(S))
        P = exp(-E)
    return (P[0], E[0])


class NoderesultPlotter(PlotoptionsMixin, Process):
    def __init__(self, results, name='Node results plotter',
                 info="Plots nodes related results of GPS trips using matplotlib",
                 logger=None, **kwargs):

        self._init_common('noderesultplotter', parent=results, name=name,
                          info=info, logger=logger)

        # print 'Resultplotter.__init__',results,self.parent
        attrsman = self.get_attrsman()

        self.is_grid = attrsman.add(cm.AttrConf('is_grid', kwargs.get('is_grid', True),
                                                groupnames=['options'],
                                                name='Show grid?',
                                                info='If True, shows a grid on the graphical representation.',
                                                ))

        self.titletext = attrsman.add(cm.AttrConf('titletext', kwargs.get('titletext', ''),
                                                  groupnames=['options'],
                                                  name='Title text',
                                                  info='Title text. Empty text means no title.',
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

        self.width_line = attrsman.add(cm.AttrConf('width_line', kwargs.get('width_line', 1.0),
                                                   groupnames=['options'],
                                                   name='Line width',
                                                   info='Line width of plot.',
                                                   ))
        self.add_save_options()

    def show(self):
        print 'show noderesults', len(self.parent.nodesresults)
        # if self.axis  is None:

        nodesresults = self.parent.nodesresults

        if len(nodesresults) == 0:
            return False

        mapmatching = self.parent.parent
        trips = mapmatching.trips
        #points = mapmatching.points
        routes = trips.get_routes()
        scenario = mapmatching.get_scenario()
        edges = scenario.net.edges
        nodes = scenario.net.nodes

        ids_noderes = nodesresults.get_ids()
        nodetypes = nodes.types[nodesresults.ids_node[ids_noderes]]
        times_wait = nodesresults.times_wait[ids_noderes]
        nodetypeset = set(nodetypes)

        map_type_to_typename = get_inversemap(nodes.types.choices)
        map_typename_to_times_wait = {}
        for thistype in nodetypeset:
            map_typename_to_times_wait[map_type_to_typename[thistype]] = np.mean(times_wait[nodetypes == thistype])

        self.init_figures()
        fig = self.create_figure()
        ax = fig.add_subplot(111)

        #colors = np.array(COLORS,dtype = np.object)
        inds_plot = np.arange(len(map_typename_to_times_wait), dtype=np.int32)
        bar_width = 0.45
        opacity = 0.5
        #error_config = {'ecolor': '0.3'}

        rects = ax.barh(inds_plot,
                        map_typename_to_times_wait.values(),
                        # align='center',
                        alpha=opacity,
                        height=bar_width, color=get_colors(inds_plot),
                        #yerr=std_women, error_kw=error_config,
                        linewidth=self.width_line,
                        # facecolor=colors[inds][inds_nz],
                        )

        ax.set_yticks(inds_plot + bar_width / 2)
        ax.set_yticklabels(map_typename_to_times_wait.keys())
        ax.legend()

        #ax.legend(loc='best',shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        if self.titletext != '':
            ax.set_title(self.titletext, fontsize=self.size_titlefont)
        ax.set_xlabel('Average wait times [s]', fontsize=self.size_labelfont)
        ax.set_ylabel('Intersection type', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))
        # fig.tight_layout()
        if self.is_save:
            self.save_fig('virtualpop_strategy_share_current')

        plt.show()
        # show_plot()

    def get_scenario(self):
        return self.parent.get_scenario()


class SpeedprofilePlotter(PlotoptionsMixin, Process):
    def __init__(self, results, name='Speedprofile plotter with Matplotlib',
                 info="Plots the speed profile of a selected GPS trip using matplotlib",
                 logger=None, **kwargs):

        self._init_common('speedprofileplotter', parent=results, name=name,
                          info=info, logger=logger)

        # print 'Resultplotter.__init__',results,self.parent
        attrsman = self.get_attrsman()

        self.id_trip = attrsman.add(cm.AttrConf('id_trip', kwargs.get('id_trip', -1),
                                                groupnames=['options'],
                                                name='Trip ID',
                                                info='ID of GPS trip to be plotted.',
                                                ))

        self.is_plot_similar_trips = attrsman.add(cm.AttrConf('is_plot_similar_trips', kwargs.get('is_plot_similar_trips', True),
                                                              groupnames=['options'],
                                                              name='Plot similar trips',
                                                              info='If True, plot all trips which contain the same route as the given trip.',
                                                              ))

        self.method_interp = attrsman.add(cm.AttrConf('method_interp', kwargs.get('method_interp', 'slinear'),
                                                      groupnames=['options'],
                                                      choices=['linear', 'nearest', 'zero',
                                                               'slinear', 'quadratic', 'cubic'],
                                                      name='Interpolation method',
                                                      info='GPS point interpolation method.',
                                                      ))

        self.color_point = attrsman.add(cm.AttrConf('color_point', kwargs.get('color_point', np.array([0.0, 0.4, 0.6, 0.6], np.float32)),
                                                    groupnames=['options'],
                                                    perm='wr',
                                                    metatype='color',
                                                    name='Point color',
                                                    info='Color of GPS-points.',
                                                    ))

        self.size_point = attrsman.add(cm.AttrConf('size_point', kwargs.get('size_point', 10.0),
                                                   groupnames=['options'],
                                                   name='Point size',
                                                   info='Point size of GPS points.',
                                                   ))

        self.color_line = attrsman.add(cm.AttrConf('color_line', kwargs.get('color_line', np.array([1.0, 0.4, 0.0, 0.6], np.float32)),
                                                   groupnames=['options'],
                                                   perm='wr',
                                                   metatype='color',
                                                   name='Line color',
                                                   info='Color of plotted line in diagram.',
                                                   ))

        self.width_line = attrsman.add(cm.AttrConf('width_line', kwargs.get('width_line', 3.0),
                                                   groupnames=['options'],
                                                   name='Line width',
                                                   info='Line width of plot.',
                                                   ))

        self.alpha_line = attrsman.add(cm.AttrConf('alpha_line', kwargs.get('alpha_line', 0.3),
                                                   groupnames=['options'],
                                                   name='Line transp.',
                                                   info='Line transparency of plot.',
                                                   ))

        self.is_grid = attrsman.add(cm.AttrConf('is_grid', kwargs.get('is_grid', True),
                                                groupnames=['options'],
                                                name='Show grid?',
                                                info='If True, shows a grid on the graphical representation.',
                                                ))

        self.titletext = attrsman.add(cm.AttrConf('titletext', kwargs.get('titletext', ''),
                                                  groupnames=['options'],
                                                  name='Title text',
                                                  info='Title text. Empty text means no title.',
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
        self.add_save_options()

    def plot_speed_over_time(self, ax, id_trip, id_route, edges, i_min=None, i_max=None):
        print 'plot_speed_over_time', id_trip, type(id_trip), self.parent.parent

        #mapmatching = self.parent.parent
        #trips = mapmatching.trips

        routeresults = self.get_routeresults()

        #id_route = trips.ids_route_matched[id_trip]
        id_routeres = routeresults.ids_route.get_id_from_index(id_route)

        # tripresults.pointsposition[id_routeres],\
        # tripresults.pointsspeed[id_routeres],
        # tripresults.pointstime[id_routeres],
        # ids_pointedges,

        if i_min is None:
            #ids_pointedge = routeresults.ids_pointedges[id_routeres]
            #pointspositions = routeresults.pointspositions[id_routeres]
            speeds = routeresults.pointsspeeds[id_routeres]
            times = routeresults.pointstimes[id_routeres]
        else:
            #ids_pointedge = routeresults.ids_pointedges[id_routeres][i_min:i_max]
            #pointspositions = routeresults.pointspositions[id_routeres][i_min:i_max]
            speeds = routeresults.pointsspeeds[id_routeres][i_min:i_max]
            times = routeresults.pointstimes[id_routeres][i_min:i_max]
        # print '  id_route,id_routeres',id_route,id_routeres
        # print '  ids_pointedge',ids_pointedge
        n_point = len(times)

        x = np.array(times, dtype=np.float32)
        y = np.array(speeds, dtype=np.float32)*3.6  # in km/h
        #ax = init_plot()
        print '  x', x
        print '  y', y
        #ax.plot(locations, speeds, color = self.color_line[:2], lw = self.width_line ,alpha=0.9 ,zorder = 0)

        if is_scipy:
            #tck = interpolate.splrep(x, y, s=0)
            #xnew = np.linspace(np.min(x), np.max(x), 200)
            #ynew = interpolate.splev(xnew, tck, der=0)
            # if 1:
            f_inter = interpolate.interp1d(x, y, kind=self.method_interp)
            xnew = np.linspace(np.min(x), np.max(x), 200)
            ynew = f_inter(xnew)

            ax.plot(xnew, ynew, color=self.color_line, lw=self.width_line, alpha=self.alpha_line)
            ax.plot(x, y, 'o', markersize=self.size_point, color=self.color_point, alpha=self.alpha_line)
        else:
            ax.plot(x, y, 'o-', markersize=self.size_point, color=self.color_line,
                    lw=self.width_line, markerfacecolor=self.color_point, alpha=self.alpha_line)

    def plot_speed_over_way(self, ax, id_trip, id_route, edges, i_min=None, i_max=None):
        print 'plot_speed_over_way', id_trip, type(id_trip), self.parent.parent

        #mapmatching = self.parent.parent
        #trips = mapmatching.trips

        routeresults = self.get_routeresults()

        #id_route = trips.ids_route_matched[id_trip]
        id_routeres = routeresults.ids_route.get_id_from_index(id_route)

        # tripresults.pointsposition[id_routeres],\
        # tripresults.pointsspeed[id_routeres],
        # tripresults.pointstime[id_routeres],
        # ids_pointedges,

        if i_min is None:
            ids_pointedge = routeresults.ids_pointedges[id_routeres]
            pointspositions = routeresults.pointspositions[id_routeres]
            speeds = routeresults.pointsspeeds[id_routeres]
            times = routeresults.pointstimes[id_routeres]
        else:
            ids_pointedge = routeresults.ids_pointedges[id_routeres][i_min:i_max]
            pointspositions = routeresults.pointspositions[id_routeres][i_min:i_max]
            speeds = routeresults.pointsspeeds[id_routeres][i_min:i_max]
            times = routeresults.pointstimes[id_routeres][i_min:i_max]
        # print '  id_route,id_routeres',id_route,id_routeres
        # print '  ids_pointedge',ids_pointedge
        n_point = len(ids_pointedge)
        offsets = np.zeros(n_point, dtype=np.float32)
        offset = 0
        id_edge_last = ids_pointedge[0]
        for i, id_edge in zip(np.arange(n_point), ids_pointedge):
            if id_edge != id_edge_last:
                offset += edges.lengths[ids_pointedge[i-1]]
                id_edge_last = id_edge
            offsets[i] = offset

        x = offsets+pointspositions
        y = np.array(speeds, dtype=np.float32)*3.6  # in km/h
        #ax = init_plot()
        print '  offsets', offsets
        print '  position', pointspositions
        print '  x', x
        print '  y', y
        #ax.plot(locations, speeds, color = self.color_line[:2], lw = self.width_line ,alpha=0.9 ,zorder = 0)

        if is_scipy:
            #tck = interpolate.splrep(x, y, s=0)
            #xnew = np.linspace(np.min(x), np.max(x), 200)
            #ynew = interpolate.splev(xnew, tck, der=0)
            # if 1:
            f_inter = interpolate.interp1d(x, y, kind=self.method_interp)
            xnew = np.linspace(x.min(), x.max(), 200)
            ynew = f_inter(xnew)

            ax.plot(xnew, ynew, color=self.color_line, lw=self.width_line, alpha=self.alpha_line)
            ax.plot(x, y, 'o', markersize=self.size_point, color=self.color_point, alpha=self.alpha_line)
        else:
            ax.plot(x, y, 'o-', markersize=self.size_point, color=self.color_line,
                    lw=self.width_line, markerfacecolor=self.color_point, alpha=self.alpha_line)

    def show(self):
        print 'show', self.id_trip, type(self.id_trip), self.parent.parent
        # if self.axis  is None:

        if self.id_trip >= 0:
            id_trip = self.id_trip
            mapmatching = self.parent.parent
            trips = mapmatching.trips
            #points = mapmatching.points
            routes = trips.get_routes()
            scenario = mapmatching.get_scenario()
            edges = scenario.net.edges
            nodes = scenario.net.nodes

            routeresults = self.get_routeresults()
            id_route = trips.ids_route_matched[id_trip]
            route = routes.ids_edges[id_route]
            id_routeres = routeresults.ids_route.get_id_from_index(id_route)

            edgesresults = self.parent.edgesresults

            # tripresults.pointsposition[id_routeres],\
            # tripresults.pointsspeed[id_routeres],
            # tripresults.pointstime[id_routeres],
            # ids_pointedges,
            self.init_figures()
            fig = self.create_figure()
            ax = fig.add_subplot(111)

            fig2 = self.create_figure()
            ax2 = fig2.add_subplot(111)

            self.plot_speed_over_way(ax, id_trip, id_route, edges)
            self.plot_speed_over_time(ax2, id_trip, id_route, edges)

            # get_color()
            # is_sublist
            #id_route = trips.ids_route_matched[id_trip]
            #route = routes.ids_edge[id_route]
            ids_pointedge = routeresults.ids_pointedges[id_routeres]
            if self.is_plot_similar_trips:
                #id_routeres = routeresults.ids_route.get_ids_from_indices(ids_route)

                id_pointedge_first = ids_pointedge[0]
                id_pointedge_last = ids_pointedge[-1]

                ids_routeres_speed = routeresults.get_ids()
                ids_route_speed = routeresults.ids_route[ids_routeres_speed]
                # print '  route',route
                for id_trip_speed, id_route_speed, route_speed, ids_pointedge_speed in zip(
                        routes.ids_trip[ids_route_speed],
                        ids_route_speed, routes.ids_edges[ids_route_speed],
                        routeresults.ids_pointedges[ids_routeres_speed]):
                    # print '    ids_pointedge_speed',ids_pointedge_speed
                    # print '    route[0],is_inlist',route[0],ids_pointedge_speed.count(route[0]),type(ids_pointedge_speed)
                    # print '    route_speed',route_speed

                    # is_sublist(route,route_speed):# |  is_sublist(route_speed,route):
                    if is_sublist(route_speed, route):
                        i = ids_pointedge_speed.index(id_pointedge_first)
                        j = ids_pointedge_speed.index(id_pointedge_last)
                        n_pointedge = len(ids_pointedge_speed)
                        while (ids_pointedge_speed[j] == id_pointedge_last) & (j < n_pointedge-1):
                            j += 1
                        self.plot_speed_over_way(ax, id_trip_speed, id_route_speed, edges, i, j)
                        self.plot_speed_over_time(ax2, id_trip_speed, id_route_speed, edges, i, j)

            # plot edge info
            colors = [(0.2, 0.2, 0.2, 0.7), (0.8, 0.8, 0.8, 0.7)]
            ymin, ymax = ax.get_ylim()
            x = 0
            i = 0
            id_edge = -1
            i_point = 0
            pointstime = routeresults.pointstimes[id_routeres]
            print '  len(ids_pointedge)', len(ids_pointedge)
            print '  len(pointstime)', len(pointstime)

            # if len(pointstime)>1:
            #    t_last = pointstime[1]
            # else:
            #t_last_edge = pointstime[0]
            #t_last = pointstime[0]
            # print '  ids_pointedge\n',ids_pointedge
            # print '  pointstime\n',pointstime
            # print '  pointsspeeds\n',routeresults.pointsspeeds[id_routeres]
            for id_pointedge,  t, v, pos in zip(
                ids_pointedge,
                pointstime,
                routeresults.pointsspeeds[id_routeres],
                routeresults.pointspositions[id_routeres],
            ):

                # print '  id_pointedge,t',id_pointedge,id_pointedge != id_edge,'%.1f, x%.2fm, t=%.2fs, v=%.2fkm/h'%(x,pos,t,v*3.6)
                if id_pointedge != id_edge:
                    # edges have changed
                    if id_edge != -1:
                        x += edges.lengths[id_edge]  # add length of old id_edge

                    #ax.plot([x,x+length],[ymin,ymin],color = colors[i%2],lw = 3*self.width_line)
                    x_point = x+pos
                    ax.plot([x_point, x_point], [ymin, ymax], 'k--')
                    ax.text(x_point, ymax, ' Edge: %s ' % (str(id_pointedge)),
                            verticalalignment='top',
                            horizontalalignment='left',
                            rotation='vertical',
                            fontsize=int(0.8*self.size_labelfont))  # baseline

                    t_last = t
                    ax2.plot([t_last, t_last], [ymin, ymax], 'k--')
                    ax2.text(t_last+1, ymax, ' Edge: %s ' % (str(id_pointedge)),
                             verticalalignment='top',
                             horizontalalignment='left',
                             rotation='vertical',
                             fontsize=int(0.8*self.size_labelfont))  # baseline

                    # put wait times, if any available
                    if edgesresults.ids_edge.has_index(id_pointedge):
                        id_edgeres = edgesresults.ids_edge.get_id_from_index(id_pointedge)
                        times_wait_tls = edgesresults.times_wait_tls[id_edgeres]
                        if times_wait_tls > 0:
                            time_wait_edge = times_wait_tls
                            color_time = 'g'
                        else:
                            time_wait_edge = edgesresults.times_wait_junc[id_edgeres]
                            color_time = 'b'

                        ax2.text(t_last+2, ymin, r'  $ T_W$=%ds' % time_wait_edge,
                                 verticalalignment='bottom',
                                 horizontalalignment='left',
                                 rotation='vertical',
                                 color=color_time,
                                 fontsize=int(0.8*self.size_labelfont))

                    i += 1

                    #t_last_edge = t

                    id_edge = id_pointedge

                i_point += 1

            #ax.legend(loc='best',shadow=True, fontsize=self.size_labelfont)
            ax2.grid(self.is_grid)
            if self.titletext != '':
                ax2.set_title(self.titletext, fontsize=self.size_titlefont)

            ax2.set_xlabel('Time [s]', fontsize=self.size_labelfont)
            ax2.set_ylabel('Speed [km/h]', fontsize=self.size_labelfont)
            ax2.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
            ax2.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))

            #ax.legend(loc='best',shadow=True, fontsize=self.size_labelfont)
            ax.grid(self.is_grid)
            if self.titletext != '':
                ax.set_title(self.titletext, fontsize=self.size_titlefont)

            ax.set_xlabel('Distance [m]', fontsize=self.size_labelfont)
            ax.set_ylabel('Speed [km/h]', fontsize=self.size_labelfont)
            ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
            ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))

            # if self.is_save:
            #    self.save_fig('routeana_speedprofile')

            plt.show()
            # show_plot()

    def get_routeresults(self):
        return self.parent.routesresults_matched

    def get_scenario(self):
        return self.parent.get_scenario()


class EdgeresultPlotter(PlotoptionsMixin, Process):
    def __init__(self, results, name='Plot edge results with Matplotlib',
                 info="Creates plots of different edge results using matplotlib",
                 logger=None, **kwargs):

        self._init_common('routeresultplotter', parent=results, name=name,
                          info=info, logger=logger)

        # print 'Resultplotter.__init__',results,self.parent
        attrsman = self.get_attrsman()

        self.plotthemefuncs = {'average_speeds': self.plot_speeds_average,
                               'inmove_speeds': self.plot_speeds_inmotion,
                               'times_wait': self.plot_times_wait,
                               'times_wait_tls': self.plot_times_wait_tls,
                               'number_matched_routes': self.plot_numbers_tot_matched,
                               'number_shortest': self.plot_numbers_tot_shortest,
                               'total_deviation': self.plot_differences_dist_tot_shortest,
                               'relative_deviation': self.plot_differences_dist_rel_shortest,
                               'probabilities_matched_routes': self.plot_probabilities_tot_matched,
                               'flows_est_matched_routes': self.plot_flows_est_matched_routes,
                               }
        self.plottheme = attrsman.add(cm.AttrConf('plottheme', kwargs.get('plottheme', 'average_speeds'),
                                                  groupnames=['options'],
                                                  choices=self.plotthemefuncs.keys(),
                                                  name='Plot theme',
                                                  info='Theme  or edge attribute to be plottet.',
                                                  ))

        self.add_plotoptions(**kwargs)
        self.add_save_options(**kwargs)

    def plot_all_themes(self):
        for plottheme in self.plotthemefuncs.keys():
            self.plottheme = plottheme
            self.show()

    def show(self):
        print 'EdgeresultPlotter.show', self.plottheme
        # if self.axis  is None:
        #axis = init_plot()
        self.init_figures()
        fig = self.create_figure()
        axis = fig.add_subplot(111)
        self.plotthemefuncs[self.plottheme](axis)

        print '  self.is_save', self.is_save
        if not self.is_save:
            print '  show_plot'
            show_plot()
        else:
            figname = 'edgeplot_'+self.plottheme
            # print '  savefig',figname

            # self.save_fig('edgeplot_'+self.plottheme)

            rootfilepath = self.get_scenario().get_rootfilepath()

            fig.savefig("%s_%s.%s" % (rootfilepath, figname, self.figformat),
                        format=self.figformat,
                        dpi=self.resolution,
                        # orientation='landscape',
                        orientation='portrait',
                        transparent=True)
            plt.close(fig)

    def get_edgeresults(self):
        return self.parent.edgesresults  # must have attribute 'ids_edge'

    def plot_differences_dist_rel_shortest(self, ax):
        edgesresults = self.get_edgeresults()
        ids_result = edgesresults.select_ids(edgesresults.numbers_tot_shortest.get_value() > 0)
        #resultattrconf = getattr(self.parent.edgesresults, self.edgeattrname)

        self.plot_results_on_map(ax, ids_result,
                                 edgesresults.differences_dist_tot_shortest[ids_result] /
                                 edgesresults.numbers_tot_shortest[ids_result],
                                 title='Deviation generated per user',
                                 valuelabel='Generated deviation per trip [m]',
                                 )

    def plot_differences_dist_tot_shortest(self, ax):
        edgesresults = self.get_edgeresults()
        ids_result = edgesresults.get_ids()
        #resultattrconf = getattr(self.parent.edgesresults, self.edgeattrname)
        deviation_tot = edgesresults.differences_dist_tot_shortest
        self.plot_results_on_map(ax, ids_result,
                                 deviation_tot[ids_result]/1000,
                                 title='Total deviation generated per edge',
                                 valuelabel='Generated total deviation [km]',
                                 )

    def plot_numbers_tot_shortest(self, ax):
        edgesresults = self.get_edgeresults()
        ids_result = edgesresults.get_ids()
        #resultattrconf = getattr(self.parent.edgesresults, self.edgeattrname)
        numbers_tot_shortest = edgesresults.numbers_tot_shortest
        self.plot_results_on_map(ax, ids_result,
                                 numbers_tot_shortest[ids_result],
                                 title='Edge usage from shortest routes',
                                 valuelabel='Usage in number of persons',
                                 )

    def plot_numbers_tot_matched(self, ax):
        edgesresults = self.get_edgeresults()
        ids_result = edgesresults.get_ids()
        #resultattrconf = getattr(self.parent.edgesresults, self.edgeattrname)
        numbers_tot_matched = edgesresults.numbers_tot_matched
        self.plot_results_on_map(ax, ids_result,
                                 numbers_tot_matched[ids_result],
                                 title='Edge usage from matched routes',
                                 valuelabel='Usage in number of persons',
                                 )

    def plot_speeds_average(self, ax):
        edgesresults = self.parent.edgesresults

        print 'plot_speeds_average'

        #ids_result = edgesresults.get_ids()
        ids_result = edgesresults.select_ids(edgesresults.numbers_tot_matched.get_value() > 0)

        #resultattrconf = getattr(self.parent.edgesresults, self.edgeattrname)
        speeds_average = self.parent.edgesresults.speeds_average
        print '  speeds_average', speeds_average[ids_result]
        print '  ids_result', ids_result

        self.plot_results_on_map(ax, ids_result,
                                 speeds_average[ids_result]*3.6,
                                 title='Average edge speeds',
                                 valuelabel='Average edge speeds [km/h]',
                                 )

    def plot_speeds_inmotion(self, ax):
        edgesresults = self.parent.edgesresults
        #ids_result = edgesresults.get_ids()
        ids_result = edgesresults.select_ids(edgesresults.numbers_tot_matched.get_value() > 0)

        #resultattrconf = getattr(self.parent.edgesresults, self.edgeattrname)
        speeds = self.parent.edgesresults.speeds_inmotion

        print '  speeds_average', speeds[ids_result]
        print '  ids_result', ids_result

        self.plot_results_on_map(ax, ids_result,
                                 speeds[ids_result]*3.6,
                                 title='Average edge speeds in motion',
                                 valuelabel='Average edge speeds in motion [km/h]',
                                 )

    def plot_times_wait(self, ax):
        edgesresults = self.parent.edgesresults
        #ids_result = edgesresults.get_ids()
        ids_result = edgesresults.select_ids(edgesresults.numbers_tot_matched.get_value() > 0)

        #resultattrconf = getattr(self.parent.edgesresults, self.edgeattrname)
        times = edgesresults.times_wait
        self.plot_results_on_map(ax, ids_result,
                                 times[ids_result],
                                 title='Average wait times',
                                 valuelabel='Average wait times [s]',
                                 )

    def plot_times_wait_tls(self, ax):
        #ids_result = self.parent.edgesresults.get_ids()
        #resultattrconf = getattr(self.parent.edgesresults, self.edgeattrname)
        edgesresults = self.parent.edgesresults
        ids_result = edgesresults.select_ids(edgesresults.numbers_tot_matched.get_value() > 0)
        times = edgesresults.times_wait_tls
        self.plot_results_on_map(ax, ids_result,
                                 times[ids_result],
                                 title='Average wait times at Traffic Lights',
                                 valuelabel='Average wait times at TLS [s]',
                                 )

    def plot_probabilities_tot_matched(self, ax):
        ids_result = self.parent.edgesresults.get_ids()
        #resultattrconf = getattr(self.parent.edgesresults, self.edgeattrname)
        self.plot_results_on_map(ax, ids_result,
                                 self.parent.edgesresults.probabilities_tot_matched[ids_result],
                                 title='Probabilities',
                                 valuelabel=r'Enter probabilities [\%]',
                                 )

    def plot_flows_est_matched_routes(self, ax):
        ids_result = self.parent.edgesresults.get_ids()
        #resultattrconf = getattr(self.parent.edgesresults, self.edgeattrname)
        self.plot_results_on_map(ax, ids_result,
                                 self.parent.edgesresults.flows_est[ids_result],
                                 title='Flows',
                                 valuelabel=r'Estimated flows [1/h]',
                                 )

    def do(self):
        # print 'do',self.edgeattrname
        self.show()
        return True


class RouteresultPlotter(PlotoptionsMixin, Process):
    def __init__(self, results, name='Plot route results with Matplotlib',
                 info="Creates plots of different route results using matplotlib",
                 logger=None, **kwargs):

        self._init_common('routeresultplotter', parent=results, name=name,
                          info=info, logger=logger)

        print 'Resultplotter.__init__', results, self.parent
        attrsman = self.get_attrsman()

        # comprison matched fastest
        self.is_plot_timedistrib = attrsman.add(cm.AttrConf('is_plot_timedistrib', kwargs.get('is_plot_timedistrib', True),
                                                            groupnames=['options'],
                                                            name='Plot time distribution',
                                                            info='Plot cumulative distribution on trip duration of matched route and fastest route.',
                                                            ))

        # comprison matched shortest
        self.is_plot_lengthdistrib = attrsman.add(cm.AttrConf('is_plot_lengthdistrib', kwargs.get('is_plot_lengthdistrib', True),
                                                              groupnames=['options'],
                                                              name='Plot length distribution',
                                                              info='Plot cumulative distribution on length of matched route and shortest route.',
                                                              ))

        self.is_plot_lengthprob = attrsman.add(cm.AttrConf('is_plot_lengthprob', kwargs.get('is_plot_lengthprob', False),
                                                           groupnames=['options'],
                                                           name='Plot length probabilities',
                                                           info='Plot probabilities length of matched route and shortest route.',
                                                           ))

        self.is_plot_lengthdistrib_by_class = attrsman.add(cm.AttrConf('is_plot_lengthdistrib_by_class', kwargs.get('is_plot_lengthdistrib_by_class', False),
                                                                       groupnames=['options'],
                                                                       name='Plot class length distribution',
                                                                       info='Plot mean values of length of matched route and shortest route for different trip length classes.',
                                                                       ))

        self.distance_class = attrsman.add(cm.AttrConf('distance_class', kwargs.get('distance_class', 2000),
                                                       groupnames=['options'],
                                                       name='Class distance',
                                                       info='Distance to generate trip length classes.',
                                                       ))

        self.is_plot_lengthratio = attrsman.add(cm.AttrConf('is_plot_lengthratio', kwargs.get('is_plot_lengthratio', False),
                                                            groupnames=['options'],
                                                            name='Plot length ratio',
                                                            info='Plot  cumulative distribution  on length ratio between shortest route and matched route.',
                                                            ))

        self.is_plot_lengthoverlap = attrsman.add(cm.AttrConf('is_plot_lengthoverlap', kwargs.get('is_plot_lengthoverlap', False),
                                                              groupnames=['options'],
                                                              name='Plot length overlap',
                                                              info='Plot  cumulative distribution  on overlap between shortest route and matched route.',
                                                              ))

        self.is_plot_mixshare = attrsman.add(cm.AttrConf('is_plot_mixshare', kwargs.get('is_plot_mixshare', False),
                                                         groupnames=['options'],
                                                         name='Plot mixed share',
                                                         info='Plot  cumulative distribution of share of mixed access roads of shortest route and matched route.',
                                                         ))

        self.is_plot_exclusiveshare = attrsman.add(cm.AttrConf('is_plot_exclusiveshare', kwargs.get('is_plot_exclusiveshare', False),
                                                               groupnames=['options'],
                                                               name='Plot exclusive share',
                                                               info='Plot  cumulative distribution of share of exclusive access roads of shortest route and matched route.',
                                                               ))

        self.is_plot_lowpriorityshare = attrsman.add(cm.AttrConf('is_plot_lowpriorityshare', kwargs.get('is_plot_lowpriorityshare', False),
                                                                 groupnames=['options'],
                                                                 name='Plot low priority share',
                                                                 info='Plot  cumulative distribution of share of elow priority roads of shortest route and matched route.',
                                                                 ))

        self.is_plot_nodesdensity = attrsman.add(cm.AttrConf('is_plot_nodesdensity', kwargs.get('is_plot_nodesdensity', False),
                                                             groupnames=['options'],
                                                             name='Plot node ratio',
                                                             info='Plot  cumulative distribution of node ratio between shortest route and matched route.',
                                                             ))

        self.is_plot_tldensity = attrsman.add(cm.AttrConf('is_plot_tldensity', kwargs.get('is_plot_tldensity', False),
                                                          groupnames=['options'],
                                                          name='Plot TL ratio',
                                                          info='Plot  cumulative distribution of traffic light ratio between shortest route and matched route.',
                                                          ))
        self.is_prioritychangedensity = attrsman.add(cm.AttrConf('is_prioritychangedensity', kwargs.get('is_prioritychangedensity', False),
                                                                 groupnames=['options'],
                                                                 name='Plot prio. change dens.',
                                                                 info='Plot  cumulative distribution of priority change denities between shortest route and matched route.',
                                                                 ))

        # comprison  non-overlapping matched and shortest
        self.is_plot_lengthratio_nonoverlap = attrsman.add(cm.AttrConf('is_plot_lengthratio_nonoverlap', kwargs.get('is_plot_lengthratio_nonoverlap', False),
                                                                       groupnames=['options'],
                                                                       name='Plot length ratio non-overlap',
                                                                       info='Plot cumulative distribution  on length ratio between non-overlapping parts of  shortest route and matched route.',
                                                                       ))

        self.is_plot_mixshare_nonoverlap = attrsman.add(cm.AttrConf('is_plot_mixshare_nonoverlap', kwargs.get('is_plot_mixshare_nonoverlap', False),
                                                                    groupnames=['options'],
                                                                    name='Plot mixed share non-overlap',
                                                                    info='Plot  cumulative distribution of share of mixed access roads of non-overlapping parts of shortest route and matched route.',
                                                                    ))

        self.is_plot_exclusiveshare_nonoverlap = attrsman.add(cm.AttrConf('is_plot_exclusiveshare_nonoverlap', kwargs.get('is_plot_exclusiveshare_nonoverlap', False),
                                                                          groupnames=['options'],
                                                                          name='Plot exclusive share non-overlap',
                                                                          info='Plot  cumulative distribution of share of exclusive access roads of non-overlapping parts of shortest route and matched route.',
                                                                          ))

        self.is_plot_lowpriorityshare_nonoverlap = attrsman.add(cm.AttrConf('is_plot_lowpriorityshare_nonoverlap', kwargs.get('is_plot_lowpriorityshare_nonoverlap', False),
                                                                            groupnames=['options'],
                                                                            name='Plot low priority share non-overlap',
                                                                            info='Plot  cumulative distribution of share of low priority roads of non-overlapping parts of shortest route and matched route.',
                                                                            ))

        self.is_plot_nodesdensity_nonoverlap = attrsman.add(cm.AttrConf('is_plot_nodesdensity_nonoverlap', kwargs.get('is_plot_nodesdensity_nonoverlap', False),
                                                                        groupnames=['options'],
                                                                        name='Plot node ratio non-overlap',
                                                                        info='Plot  cumulative distribution of node ratio between non-overlapping parts of shortest route and matched route.',
                                                                        ))

        self.is_plot_tldensity_nonoverlap = attrsman.add(cm.AttrConf('is_plot_tldensity_nonoverlap', kwargs.get('is_plot_tldensity_nonoverlap', False),
                                                                     groupnames=['options'],
                                                                     name='Plot TL ratio non-overlap',
                                                                     info='Plot  cumulative distribution of traffic light ratio between non-overlapping parts of shortest route and matched route.',
                                                                     ))

        self.is_prioritychangedensity_nonoverlap = attrsman.add(cm.AttrConf('is_prioritychangedensity_nonoverlap', kwargs.get('is_prioritychangedensity_nonoverlap', False),
                                                                            groupnames=['options'],
                                                                            name='Plot prio. change dens. non-overlap',
                                                                            info='Plot  cumulative distribution of priority change denities between non-overlapping parts of shortest route and matched route.',
                                                                            ))

        # other
        self.n_bins = attrsman.add(cm.AttrConf('n_bins', kwargs.get('n_bins', 10),
                                               groupnames=['options'],
                                               name='Bin number',
                                               info='Number of bins for histograms.',
                                               ))

        # self.add_plotoptions(**kwargs)
        self.is_title = attrsman.add(cm.AttrConf('is_title', kwargs.get('is_title', False),
                                                 groupnames=['options'],
                                                 name='Show title',
                                                 info='Show title of diagrams.',
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

        self.width_line = attrsman.add(cm.AttrConf('width_line', kwargs.get('width_line', 2),
                                                   groupnames=['options'],
                                                   perm='wr',
                                                   name='Line width',
                                                   info='Width of plotted lines.',
                                                   ))

        self.color_line = attrsman.add(cm.AttrConf('color_line', kwargs.get('color_line', np.array([0, 0, 0, 1], dtype=np.float32)),
                                                   groupnames=['options'],
                                                   perm='wr',
                                                   metatype='color',
                                                   name='Line color',
                                                   info='Color of line in various diagrams.',
                                                   ))

        # COLOR_MATCHED_ROUTE,COLOR_SHORTEST_ROUTE,COLOR_FASTEST_ROUTE
        self.color_matched = attrsman.add(cm.AttrConf('color_matched', kwargs.get('color_matched', COLOR_MATCHED_ROUTE.copy()),
                                                      groupnames=['options'],
                                                      perm='wr',
                                                      metatype='color',
                                                      name='Color matched data',
                                                      info='Color of matched data in various diagrams.',
                                                      ))

        self.color_shortest = attrsman.add(cm.AttrConf('color_shortest', kwargs.get('color_shortest', COLOR_SHORTEST_ROUTE.copy()),
                                                       groupnames=['options'],
                                                       perm='wr',
                                                       metatype='color',
                                                       name='Color shortest route data',
                                                       info='Color of shortest route data in various diagrams.',
                                                       ))

        self.color_fastest = attrsman.add(cm.AttrConf('color_fastest', kwargs.get('color_fastest', COLOR_FASTEST_ROUTE.copy()),
                                                      groupnames=['options'],
                                                      perm='wr',
                                                      metatype='color',
                                                      name='Color fastest route data',
                                                      info='Color of fastest route data in various diagrams.',
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

        self.is_grid = attrsman.add(cm.AttrConf('is_grid', kwargs.get('is_grid', True),
                                                groupnames=['options'],
                                                name='Show grid?',
                                                info='If True, shows a grid on the graphical representation.',
                                                ))
        self.color_background = attrsman.add(cm.AttrConf('color_background', kwargs.get('color_background', np.array([1, 1, 1, 1], dtype=np.float32)),
                                                         groupnames=['options'],
                                                         perm='wr',
                                                         metatype='color',
                                                         name='Background color',
                                                         info='Background color of schematic network in the background.',
                                                         ))

        self.add_save_options(**kwargs)

    def show(self):
        # print 'show',self.edgeattrname
        # if self.axis  is None:
        self.init_figures()
        plt.rc('lines', linewidth=self.width_line)
        # plt.rc('axes', prop_cycle=(cycler('color', ['r', 'g', 'b', 'y']) +
        #                    cycler('linestyle', ['-', '--', ':', '-.'])))
        if self.is_plot_lengthratio:
            self.plot_lengthratio()

        if self.is_plot_lengthoverlap:
            self.plot_lengthoverlap()

        if self.is_plot_lengthdistrib:
            self.plot_lengthdistrib()

        if self.is_plot_lengthprob:
            self.plot_lengthprob()

        if self.is_plot_lengthdistrib_by_class:
            self.plot_lengthdistrib_by_class()

        # time
        if self.is_plot_timedistrib:
            self.plot_timedistrib()
        # --
        if self.is_plot_lengthdistrib:
            self.plot_lengthdistrib()

        if self.is_plot_mixshare:
            self.plot_mixshare()

        if self.is_plot_exclusiveshare:
            self.plot_exclusiveshare()

        if self.is_plot_lowpriorityshare:
            self.plot_lowpriorityshare()

        if self.is_plot_nodesdensity:
            self.plot_nodesdensity()

        if self.is_plot_tldensity:
            self.plot_tldensity()

        if self.is_prioritychangedensity:
            self.plot_prioritychangedensity()

        # non overlapping
        if self.is_plot_lengthratio_nonoverlap:
            self.plot_lengthratio_nonoverlap()

        if self.is_plot_mixshare_nonoverlap:
            self.plot_mixshare_nonoverlap()

        if self.is_plot_exclusiveshare_nonoverlap:
            self.plot_exclusiveshare_nonoverlap()

        if self.is_plot_lowpriorityshare_nonoverlap:
            self.plot_lowpriorityshare_nonoverlap()

        if self.is_plot_nodesdensity_nonoverlap:
            self.plot_nodesdensity_nonoverlap()

        if self.is_plot_tldensity_nonoverlap:
            self.plot_tldensity_nonoverlap()

        if self.is_prioritychangedensity_nonoverlap:
            self.plot_prioritychangedensity_nonoverlap()

        if not self.is_save:
            show_plot()

    def plot_tldensity(self):
        print 'plot_tldensity'
        fig = self.create_figure()
        results = self.parent
        routesresults_shortest = results.routesresults_shortest
        routesresults_matched = results.routesresults_matched
        edgesresults = results.edgesresults

        ax = fig.add_subplot(111)
        ids_valid = routesresults_matched.select_ids(np.logical_and(
            routesresults_shortest.distances.get_value() > 0, routesresults_matched.distances.get_value() > 0))

        dists_match = routesresults_matched.distances[ids_valid]
        dists_shortest = routesresults_shortest.distances[ids_valid]

        matched = routesresults_matched.numbers_nodes_tls[ids_valid]/dists_match*1000
        shortest = routesresults_shortest.numbers_nodes_tls[ids_valid]/dists_shortest*1000

        x_min = min(np.min(matched), np.min(shortest))
        x_max = 10.0  # max(np.max(matched),np.max(shortest))
        bins = np.linspace(x_min, x_max, self.n_bins)
        bincenters = self.plot_hist(ax, matched, bins=bins, color=self.color_matched, label='matched:' +
                                    '$\mu = %.2f/km$, $\sigma=%.2f/km$' % (np.mean(matched), np.std(matched)))
        bincenters = self.plot_hist(ax, shortest, bins=bins, color=self.color_shortest, label='shortest:' +
                                    '$\mu = %.2f/km$, $\sigma=%.2f/km$' % (np.mean(shortest), np.std(shortest)))

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        if self.is_title:
            ax.set_title('Node densities of matched and shortest route', fontsize=self.size_titlefont)
        ax.set_xlabel('Traffic light density [1/km]', fontsize=self.size_labelfont)
        ax.set_ylabel('Probability distribution', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))
        if self.is_save:
            self.save_fig('routeana_tldensity')

    def plot_nodesdensity(self):
        print 'plot_nodesdensity'
        fig = self.create_figure()
        results = self.parent

        routesresults_shortest = results.routesresults_shortest
        routesresults_matched = results.routesresults_matched
        edgesresults = results.edgesresults

        ax = fig.add_subplot(111)
        ids_valid = routesresults_matched.select_ids(np.logical_and(
            routesresults_shortest.distances.get_value() > 0, routesresults_matched.distances.get_value() > 0))

        dists_match = routesresults_matched.distances[ids_valid]
        dists_shortest = routesresults_shortest.distances[ids_valid]

        matched = routesresults_matched.numbers_nodes[ids_valid]/dists_match*1000
        shortest = routesresults_shortest.numbers_nodes[ids_valid]/dists_shortest*1000

        x_min = min(np.min(matched), np.min(shortest))
        x_max = max(np.max(matched), np.max(shortest))
        bins = np.linspace(x_min, x_max, self.n_bins)
        bincenters = self.plot_hist(ax, matched, bins=bins, color=self.color_matched, label='matched:' +
                                    '$\mu = %.2f/km$, $\sigma=%.2f/km$' % (np.mean(matched), np.std(matched)))
        bincenters = self.plot_hist(ax, shortest, bins=bins, color=self.color_shortest, label='shortest:' +
                                    '$\mu = %.2f/km$, $\sigma=%.2f/km$' % (np.mean(shortest), np.std(shortest)))

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        if self.is_title:
            ax.set_title('Node densities of matched and shortest route', fontsize=self.size_titlefont)
        ax.set_xlabel('Node density [1/km]', fontsize=self.size_labelfont)
        ax.set_ylabel('Probability distribution', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))
        if self.is_save:
            self.save_fig('routeana_nodesdensity')

    def plot_prioritychangedensity(self):
        print 'plot_prioritychangedensity'
        fig = self.create_figure()
        results = self.parent
        routesresults_shortest = results.routesresults_shortest
        routesresults_matched = results.routesresults_matched
        edgesresults = results.edgesresults

        ax = fig.add_subplot(111)
        ids_valid = routesresults_matched.select_ids(np.logical_and(
            routesresults_shortest.distances.get_value() > 0, routesresults_matched.distances.get_value() > 0))

        dists_match = routesresults_matched.distances[ids_valid]
        dists_shortest = routesresults_shortest.distances[ids_valid]

        matched = routesresults_matched.numbers_prioritychange[ids_valid]/dists_match*1000
        shortest = routesresults_shortest.numbers_prioritychange[ids_valid]/dists_shortest*1000

        x_min = min(np.min(matched), np.min(shortest))
        x_max = max(np.max(matched), np.max(shortest))
        bins = np.linspace(x_min, x_max, self.n_bins)
        bincenters = self.plot_hist(ax, matched, bins=bins, color=self.color_matched, label='matched:' +
                                    '$\mu = %.2f/km$, $\sigma=%.2f/km$' % (np.mean(matched), np.std(matched)))
        bincenters = self.plot_hist(ax, shortest, bins=bins, color=self.color_shortest, label='shortest:' +
                                    '$\mu = %.2f/km$, $\sigma=%.2f/km$' % (np.mean(shortest), np.std(shortest)))

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        if self.is_title:
            ax.set_title('Priority change dens. of matched and shortest route', fontsize=self.size_titlefont)
        ax.set_xlabel('Priority change density [1/km]', fontsize=self.size_labelfont)
        ax.set_ylabel('Probability distribution', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))
        if self.is_save:
            self.save_fig('routeana_nodesdensity')

    def plot_lowpriorityshare(self):
        print 'plot_lowpriorityshare'
        fig = self.create_figure()
        results = self.parent
        routesresults_shortest = results.routesresults_shortest
        routesresults_matched = results.routesresults_matched
        edgesresults = results.edgesresults

        ax = fig.add_subplot(111)
        ids_valid = routesresults_matched.select_ids(np.logical_and(
            routesresults_shortest.distances.get_value() > 0, routesresults_matched.distances.get_value() > 0))

        dists_match = routesresults_matched.distances[ids_valid]
        dists_shortest = routesresults_shortest.distances[ids_valid]

        matched = routesresults_matched.lengths_low_priority[ids_valid]/dists_match*100
        shortest = routesresults_shortest.lengths_low_priority[ids_valid]/dists_shortest*100

        x_min = min(np.min(matched), np.min(shortest))
        x_max = 15.0  # max(np.max(matched),np.max(shortest))
        bins = np.linspace(x_min, x_max, self.n_bins)
        bincenters = self.plot_hist(ax, matched, bins=bins, color=self.color_matched, label='matched:' +
                                    '$\mu = %.2f$%%, $\sigma=%.2f$%%' % (np.mean(matched), np.std(matched)))
        bincenters = self.plot_hist(ax, shortest, bins=bins, color=self.color_shortest, label='shortest:' +
                                    '$\mu = %.2f$%%, $\sigma=%.2f$%%' % (np.mean(shortest), np.std(shortest)))

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        if self.is_title:
            ax.set_title('Share of low priority roads of matched and shortest route', fontsize=self.size_titlefont)
        ax.set_xlabel('Low priority road share [%]', fontsize=self.size_labelfont)
        ax.set_ylabel('Probability distribution', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))
        if self.is_save:
            self.save_fig('routeana_lowpriorityshare')

    def plot_exclusiveshare(self):
        print 'plot_exclusiveshare'
        fig = self.create_figure()
        results = self.parent
        routesresults_shortest = results.routesresults_shortest
        routesresults_matched = results.routesresults_matched
        edgesresults = results.edgesresults

        ax = fig.add_subplot(111)
        ids_valid = routesresults_matched.select_ids(np.logical_and(
            routesresults_shortest.distances.get_value() > 0, routesresults_matched.distances.get_value() > 0))

        dists_match = routesresults_matched.distances[ids_valid]
        dists_shortest = routesresults_shortest.distances[ids_valid]

        matched = routesresults_matched.lengths_exclusive[ids_valid]/dists_match*100
        shortest = routesresults_shortest.lengths_exclusive[ids_valid]/dists_shortest*100

        x_min = min(np.min(matched), np.min(shortest))
        x_max = max(np.max(matched), np.max(shortest))
        bins = np.linspace(x_min, x_max, self.n_bins)
        bincenters = self.plot_hist(ax, matched, bins=bins, color=self.color_matched, label='matched:' +
                                    '$\mu = %.2f$%%, $\sigma=%.2f$%%' % (np.mean(matched), np.std(matched)))
        bincenters = self.plot_hist(ax, shortest, bins=bins, color=self.color_shortest, label='shortest:' +
                                    '$\mu = %.2f$%%, $\sigma=%.2f$%%' % (np.mean(shortest), np.std(shortest)))

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        if self.is_title:
            ax.set_title('Share of exclusive access roads of matched and shortest route', fontsize=self.size_titlefont)
        ax.set_xlabel('Exclusive access road share [%]', fontsize=self.size_labelfont)
        ax.set_ylabel('Probability distribution', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))
        if self.is_save:
            self.save_fig('routeana_exclusiveshare')

    def plot_mixshare(self):
        print 'plot_mixshare'
        fig = self.create_figure()
        results = self.parent
        routesresults_shortest = results.routesresults_shortest
        routesresults_matched = results.routesresults_matched
        edgesresults = results.edgesresults

        ax = fig.add_subplot(111)
        ids_valid = routesresults_matched.select_ids(np.logical_and(
            routesresults_shortest.distances.get_value() > 0, routesresults_matched.distances.get_value() > 0))

        dists_match = routesresults_matched.distances[ids_valid]
        dists_shortest = routesresults_shortest.distances[ids_valid]

        matched = routesresults_matched.lengths_mixed[ids_valid]/dists_match*100
        shortest = routesresults_shortest.lengths_mixed[ids_valid]/dists_shortest*100

        x_min = min(np.min(matched), np.min(shortest))
        x_max = max(np.max(matched), np.max(shortest))
        bins = np.linspace(x_min, x_max, self.n_bins)
        bincenters = self.plot_hist(ax, matched, bins=bins, color=self.color_matched, label='matched:' +
                                    '$\mu = %.2f$%%, $\sigma=%.2f$%%' % (np.mean(matched), np.std(matched)))
        bincenters = self.plot_hist(ax, shortest, bins=bins, color=self.color_shortest, label='shortest:' +
                                    '$\mu = %.2f$%%, $\sigma=%.2f$%%' % (np.mean(shortest), np.std(shortest)))

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        if self.is_title:
            ax.set_title('Share of mixed reserved access roads of matched and shortest route',
                         fontsize=self.size_titlefont)
        ax.set_xlabel('Mixed reserved access road share [%]', fontsize=self.size_labelfont)
        ax.set_ylabel('Probability distribution', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))
        if self.is_save:
            self.save_fig('routeana_mixshare')

    def plot_lengthdistrib_by_class(self):
        print 'plot_lengthdistrib_by_class'
        fig = self.create_figure()
        results = self.parent
        routesresults_shortest = results.routesresults_shortest
        routesresults_matched = results.routesresults_matched
        edgesresults = results.edgesresults

        ax = fig.add_subplot(111)
        ids_valid = routesresults_matched.select_ids(np.logical_and(
            routesresults_shortest.distances.get_value() > 0, routesresults_matched.distances.get_value() > 0))

        dists_match = routesresults_matched.distances[ids_valid]
        dists_shortest = routesresults_shortest.distances[ids_valid]
        x_min = min(np.min(dists_match), np.min(dists_shortest))
        x_max = max(np.max(dists_match), np.max(dists_shortest))

        dists_class = np.arange(0, int(x_max), self.distance_class)
        dists_class_center = (dists_class+0.5*self.distance_class)[1:]
        n_class = len(dists_class)-1
        means_match = np.zeros(n_class, dtype=np.float32)
        stds_match = np.zeros(n_class, dtype=np.float32)
        means_shortest = np.zeros(n_class, dtype=np.float32)
        stds_shortest = np.zeros(n_class, dtype=np.float32)
        xticklabels = []
        ratiolabels = []
        for dist_lower, dist_upper, i in zip(dists_class[:-1], dists_class[1:], range(n_class)):
            xticklabels.append('%d - %d' % (float(dist_lower)/1000, float(dist_upper)/1000))
            inds = np.logical_and(dists_match > dist_lower, dists_match < dist_upper)
            means_match[i] = np.mean(dists_match[inds])
            stds_match[i] = np.std(dists_match[inds])

            #inds = np.logical_and(dists_shortest>dist_lower,dists_shortest<dist_upper)
            means_shortest[i] = np.mean(dists_shortest[inds])
            stds_shortest[i] = np.std(dists_shortest[inds])

            ratiolabel = ''
            if (not np.isnan(means_shortest[i])) & (not np.isnan(means_match[i])):
                if means_match[i] > 0:
                    ratiolabel = '%d%%' % (means_shortest[i]/means_match[i]*100)
            ratiolabels.append(ratiolabel)

        print '  dists_class_center', dists_class_center
        print '  means_match', means_match
        print '  stds_match', stds_match
        print '  means_shortest', means_shortest
        print '  stds_shortest', stds_shortest

        x = np.arange(n_class, dtype=np.float32)  # the x locations for the groups
        width = 0.35       # the width of the bars

        #        ax.bar(ind + width, women_means, width, color='y', yerr=women_std)
        bars1 = ax.bar(x-width, means_match, width, color=self.color_matched, yerr=stds_match)
        bars2 = ax.bar(x+0*width, means_shortest, width, color=self.color_shortest, yerr=stds_shortest)
        #bars1 = ax.bar(dists_class_center+0.35*self.distance_class, means_match, 0.25*self.distance_class, color=self.color_matched, yerr=stds_match)
        #bars2 = ax.bar(dists_class_center-0.35*self.distance_class, means_shortest, 0.25*self.distance_class, color=self.color_shortest, yerr=stds_shortest)

        #ax.legend(loc='best',shadow=True, fontsize=self.size_labelfont)
        ax.legend((bars1[0], bars2[0]), ('matched', 'shortest'),
                  shadow=True, fontsize=self.size_labelfont, loc='best')

        # if self.is_grid:
        ax.yaxis.grid(self.is_grid)

        if self.is_title:
            ax.set_title('Mean length by trip length class', fontsize=self.size_titlefont)
        ax.set_xlabel('Length classes [km]', fontsize=self.size_labelfont)
        ax.set_ylabel('Mean length [m]', fontsize=self.size_labelfont)

        ax.set_xticks(x)
        ax.set_xticklabels(xticklabels)
        # self._autolabel_bars(ax,bars1,means_match)
        self._autolabel_bars(ax, bars2, ratiolabels)

        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))
        if self.is_save:
            self.save_fig('routeana_lengthdistrib_by_class')

    def _autolabel_bars(self, ax, bars, labels):
        """
        Attach a text label above each bar displaying its height
        """
        for rect, label in zip(bars, labels):
            height = rect.get_height()
            if not np.isnan(height):
                ax.text(rect.get_x() + rect.get_width()/2., 1.05*height,
                        '%s' % label,
                        ha='center', va='bottom',
                        fontsize=int(0.8*self.size_labelfont),
                        )

    def plot_lengthdistrib(self):

        fig = self.create_figure()
        results = self.parent
        routesresults_shortest = results.routesresults_shortest
        routesresults_matched = results.routesresults_matched
        edgesresults = results.edgesresults

        ax = fig.add_subplot(111)
        ids_valid = routesresults_matched.select_ids(np.logical_and(routesresults_shortest.distances.get_value() > 0,
                                                                    routesresults_matched.distances.get_value() > 0,
                                                                    # routesresults_matched.distances.get_value()<20000)
                                                                    ))
        print 'plot_lengthdistrib', len(ids_valid)
        # print '  ids_valid',ids_valid
        if len(ids_valid) == 0:
            return False

        dists_match = routesresults_matched.distances[ids_valid]
        dists_shortest = routesresults_shortest.distances[ids_valid]

        x_min = min(np.min(dists_match), np.min(dists_shortest))
        x_max = max(np.max(dists_match), np.max(dists_shortest))
        bins = np.linspace(x_min, x_max, self.n_bins)
        bincenters = self.plot_hist(ax, dists_match, bins=bins, color=self.color_matched,
                                    label='matched:'+'$\mu = %dm$, $\sigma=%dm$' % (np.mean(dists_match), np.std(dists_match)))
        bincenters = self.plot_hist(ax, dists_shortest, bins=bins, color=self.color_shortest, label='shortest:' +
                                    '$\mu = %dm$, $\sigma=%dm$' % (np.mean(dists_shortest), np.std(dists_shortest)))

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        if self.is_title:
            ax.set_title('Length distribution of matched and shortest route', fontsize=self.size_titlefont)
        ax.set_xlabel('Length [m]', fontsize=self.size_labelfont)
        ax.set_ylabel('Probability distribution', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))
        if self.is_save:
            self.save_fig('routeana_lengthdistrib')
        return True

    def plot_timedistrib(self):
        print 'plot_timedistrib'
        fig = self.create_figure()
        results = self.parent
        mapmatching = results.parent
        trips = mapmatching.trips
        #routesresults_fastest = results.routesresults_fastest
        #routesresults_matched = results.routesresults_matched
        edgesresults = results.edgesresults

        ax = fig.add_subplot(111)
        #ids_overlength = routesresults_matched.select_ids(np.logical_and(routesresults_shortest.distances.get_value()>0, routesresults_matched.distances.get_value()>20000))
        # print '  len(ids_overlength)',len(ids_overlength)
        # print '  ids_overlength',ids_overlength

        ids_valid = trips.select_ids(
            np.logical_and(trips.lengths_route_matched.get_value() > 0,
                           trips.durations_route_fastest.get_value() > 0,
                           # routesresults_matched.distances.get_value()<20000)
                           ))

        if len(ids_valid) == 0:
            return False

        times_fastest = trips.durations_route_fastest[ids_valid]
        times_match = times_fastest+trips.timelosses_route_fastest[ids_valid]

        x_min = min(np.min(times_fastest), np.min(times_match))
        x_max = max(np.max(times_fastest), np.max(times_match))
        bins = np.linspace(x_min, x_max, self.n_bins)
        bincenters = self.plot_hist(ax, times_match, bins=bins, color=self.color_matched,
                                    label='matched:'+'$\mu = %dm$, $\sigma=%dm$' % (np.mean(times_match), np.std(times_match)))
        bincenters = self.plot_hist(ax, times_fastest, bins=bins, color=self.color_fastest, label='fastest:' +
                                    '$\mu = %dm$, $\sigma=%dm$' % (np.mean(times_fastest), np.std(times_fastest)))

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        if self.is_title:
            ax.set_title('Triptime distribution of matched and fastest route', fontsize=self.size_titlefont)
        ax.set_xlabel('Time [s]', fontsize=self.size_labelfont)
        ax.set_ylabel('Probability distribution', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))
        if self.is_save:
            self.save_fig('routeana_timedistrib')

        return True

    def plot_lengthprob(self):
        print 'plot_lengthprob'
        fig = self.create_figure()
        results = self.parent
        routesresults_shortest = results.routesresults_shortest
        routesresults_matched = results.routesresults_matched
        edgesresults = results.edgesresults

        ax = fig.add_subplot(111)
        ids_valid = routesresults_matched.select_ids(np.logical_and(
            routesresults_shortest.distances.get_value() > 0, routesresults_matched.distances.get_value() > 0))

        dists_match = routesresults_matched.distances[ids_valid]
        dists_shortest = routesresults_shortest.distances[ids_valid]

        x_min = min(np.min(dists_match), np.min(dists_shortest))
        x_max = max(np.max(dists_match), np.max(dists_shortest))
        bins = np.linspace(x_min, x_max, self.n_bins)
        w_bin = bins[1]-bins[0]
        bincenters = self.plot_hist(ax, dists_match, bins=bins,
                                    color=self.color_matched,
                                    label='matched:' +
                                    '$\mu = %dm$, $\sigma=%dm$' % (np.mean(dists_match), np.std(dists_match)),
                                    is_rel_frequ=True,
                                    is_percent=True,
                                    )
        bincenters = self.plot_hist(ax, dists_shortest, bins=bins,
                                    color=self.color_shortest,
                                    label='shortest:' +
                                    '$\mu = %dm$, $\sigma=%dm$' % (np.mean(dists_shortest), np.std(dists_shortest)),
                                    is_rel_frequ=True,
                                    is_percent=True,
                                    )

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        if self.is_title:
            ax.set_title('Relative frequency of matched and shortest route', fontsize=self.size_titlefont)
        ax.set_xlabel('Length [m]', fontsize=self.size_labelfont)
        ax.set_ylabel('Relative frequency [%]', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))
        if self.is_save:
            self.save_fig('routeana_lengthprob')

    def plot_lengthoverlap(self):
        print 'plot_lengthoverlap'
        fig = self.create_figure()
        results = self.parent
        routesresults_shortest = results.routesresults_shortest
        routesresults_matched = results.routesresults_matched
        edgesresults = results.edgesresults

        ax = fig.add_subplot(111)
        bins = np.linspace(0.0, 1.0, self.n_bins)

        ids_valid = routesresults_matched.select_ids(np.logical_and(
            routesresults_shortest.distances.get_value() > 0, routesresults_matched.distances.get_value() > 0))
        values = routesresults_shortest.lengths_overlap_matched[ids_valid]/routesresults_matched.distances[ids_valid]
        bincenters = self.plot_hist(ax, values,
                                    bins=bins, histtype='bar',
                                    label=r'$\mu = %.2f$, $\sigma=%.2f$' % (np.mean(values), np.std(values))
                                    )

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        if self.is_title:
            ax.set_title('Share of overlap between shortest path and matched path', fontsize=self.size_titlefont)
        ax.set_xlabel('Overlap share between shortest and matched path', fontsize=self.size_labelfont)
        ax.set_ylabel('Probability distribution', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))

        self.set_figmargins()
        if self.is_save:
            self.save_fig('routeana_lengthoverlap')

    def plot_lengthratio(self):
        print 'plot_lengthratio'
        fig = self.create_figure()
        results = self.parent
        routesresults_shortest = results.routesresults_shortest
        routesresults_matched = results.routesresults_matched
        edgesresults = results.edgesresults

        ax = fig.add_subplot(111)
        bins = np.linspace(0.0, 1.0, self.n_bins)

        ids_valid = routesresults_matched.select_ids(np.logical_and(
            routesresults_shortest.distances.get_value() > 0, routesresults_matched.distances.get_value() > 0))
        values = routesresults_shortest.distances[ids_valid]/routesresults_matched.distances[ids_valid]
        bincenters = self.plot_hist(ax, values,
                                    bins=bins, histtype='bar',
                                    label=r'$\mu = %.2f$, $\sigma=%.2f$' % (np.mean(values), np.std(values))
                                    )

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        if self.is_title:
            ax.set_title('Ratio distance shortest path over matched path', fontsize=self.size_titlefont)
        ax.set_xlabel('Ratio shortest path length/matched path length', fontsize=self.size_labelfont)
        ax.set_ylabel('Probability distribution', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))

        self.set_figmargins()
        if self.is_save:
            self.save_fig('routeana_lengthratio')


# -------------------------------------------------------------------------------
    # non-overlap

    def plot_lengthratio_nonoverlap(self):
        print 'plot_lengthratio_nonoverlap'
        fig = self.create_figure()
        results = self.parent
        routesresults_shortest = results.routesresults_shortest
        routesresults_matched = results.routesresults_matched
        routesresults_shortest_nonoverlap = results.routesresults_shortest_nonoverlap
        routesresults_matched_nonoverlap = results.routesresults_matched_nonoverlap

        edgesresults = results.edgesresults

        ax = fig.add_subplot(111)
        bins = np.linspace(0.0, 1.0, self.n_bins)

        ids_valid = routesresults_matched.select_ids(np.logical_and(
            routesresults_shortest_nonoverlap.distances.get_value() > 0,
            routesresults_matched_nonoverlap.distances.get_value() > 0)
        )

        values = routesresults_shortest_nonoverlap.distances[ids_valid] / \
            routesresults_matched_nonoverlap.distances[ids_valid]
        bincenters = self.plot_hist(ax, values,
                                    bins=bins, histtype='bar',
                                    label=r'$\mu = %.2f$, $\sigma=%.2f$' % (np.mean(values), np.std(values))
                                    )

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        if self.is_title:
            ax.set_title('Ratio distance of non-overlapping shortest over matched path', fontsize=self.size_titlefont)
        ax.set_xlabel('Ratio shortest n.o. path length/matched path length', fontsize=self.size_labelfont)
        ax.set_ylabel('Probability distribution', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))

        self.set_figmargins()
        if self.is_save:
            self.save_fig('routeana_lengthratio_nonoverlap')

    def plot_tldensity_nonoverlap(self):
        print 'plot_tldensity_nonoverlap'
        fig = self.create_figure()
        results = self.parent
        routesresults_shortest = results.routesresults_shortest
        routesresults_matched = results.routesresults_matched
        routesresults_shortest_nonoverlap = results.routesresults_shortest_nonoverlap
        routesresults_matched_nonoverlap = results.routesresults_matched_nonoverlap
        edgesresults = results.edgesresults

        ax = fig.add_subplot(111)
        ids_valid = routesresults_matched.select_ids(np.logical_and(
            routesresults_shortest_nonoverlap.distances.get_value() > 0, routesresults_matched_nonoverlap.distances.get_value() > 0))

        dists_match = routesresults_matched_nonoverlap.distances[ids_valid]
        dists_shortest = routesresults_shortest_nonoverlap.distances[ids_valid]

        matched = routesresults_matched_nonoverlap.numbers_nodes_tls[ids_valid]/dists_match*1000
        shortest = routesresults_shortest_nonoverlap.numbers_nodes_tls[ids_valid]/dists_shortest*1000

        x_min = min(np.min(matched), np.min(shortest))
        x_max = 10.0  # max(np.max(matched),np.max(shortest))
        bins = np.linspace(x_min, x_max, self.n_bins)
        bincenters = self.plot_hist(ax, matched, bins=bins, color=self.color_matched, label='matched:' +
                                    '$\mu = %.2f/km$, $\sigma=%.2f/km$' % (np.mean(matched), np.std(matched)))
        bincenters = self.plot_hist(ax, shortest, bins=bins, color=self.color_shortest, label='shortest:' +
                                    '$\mu = %.2f/km$, $\sigma=%.2f/km$' % (np.mean(shortest), np.std(shortest)))

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        if self.is_title:
            ax.set_title('Node densities of non-overlapping matched and shortest route', fontsize=self.size_titlefont)
        ax.set_xlabel('Traffic light density  n.o. [1/km]', fontsize=self.size_labelfont)
        ax.set_ylabel('Probability distribution', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))
        if self.is_save:
            self.save_fig('routeana_tldensity_nonoverlap')

    def plot_nodesdensity_nonoverlap(self):
        print 'plot_nodesdensity_nonoverlap'
        fig = self.create_figure()
        results = self.parent
        routesresults_shortest = results.routesresults_shortest
        routesresults_matched = results.routesresults_matched
        routesresults_shortest_nonoverlap = results.routesresults_shortest_nonoverlap
        routesresults_matched_nonoverlap = results.routesresults_matched_nonoverlap
        edgesresults = results.edgesresults

        ax = fig.add_subplot(111)
        ids_valid = routesresults_matched.select_ids(np.logical_and(
            routesresults_shortest_nonoverlap.distances.get_value() > 0, routesresults_matched_nonoverlap.distances.get_value() > 0))

        dists_match = routesresults_matched_nonoverlap.distances[ids_valid]
        dists_shortest = routesresults_shortest_nonoverlap.distances[ids_valid]

        matched = routesresults_matched_nonoverlap.numbers_nodes[ids_valid]/dists_match*1000
        shortest = routesresults_shortest_nonoverlap.numbers_nodes[ids_valid]/dists_shortest*1000

        x_min = min(np.min(matched), np.min(shortest))
        x_max = max(np.max(matched), np.max(shortest))
        bins = np.linspace(x_min, x_max, self.n_bins)
        bincenters = self.plot_hist(ax, matched, bins=bins, color=self.color_matched, label='matched:' +
                                    '$\mu = %.2f/km$, $\sigma=%.2f/km$' % (np.mean(matched), np.std(matched)))
        bincenters = self.plot_hist(ax, shortest, bins=bins, color=self.color_shortest, label='shortest:' +
                                    '$\mu = %.2f/km$, $\sigma=%.2f/km$' % (np.mean(shortest), np.std(shortest)))

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        if self.is_title:
            ax.set_title('Node densities of non-overlapping matched and shortest route', fontsize=self.size_titlefont)
        ax.set_xlabel('Node density  n.o. [1/km]', fontsize=self.size_labelfont)
        ax.set_ylabel('Probability distribution', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))
        if self.is_save:
            self.save_fig('routeana_nodesdensity_nonoverlap')

    def plot_prioritychangedensity_nonoverlap(self):
        print 'plot_prioritychangedensity_nonoverlap'
        fig = self.create_figure()
        results = self.parent
        routesresults_shortest = results.routesresults_shortest
        routesresults_matched = results.routesresults_matched
        routesresults_shortest_nonoverlap = results.routesresults_shortest_nonoverlap
        routesresults_matched_nonoverlap = results.routesresults_matched_nonoverlap
        edgesresults = results.edgesresults

        ax = fig.add_subplot(111)
        ids_valid = routesresults_matched.select_ids(np.logical_and(
            routesresults_shortest_nonoverlap.distances.get_value() > 0, routesresults_matched_nonoverlap.distances.get_value() > 0))

        dists_match = routesresults_matched_nonoverlap.distances[ids_valid]
        dists_shortest = routesresults_shortest_nonoverlap.distances[ids_valid]

        matched = routesresults_matched_nonoverlap.numbers_prioritychange[ids_valid]/dists_match*1000
        shortest = routesresults_shortest_nonoverlap.numbers_prioritychange[ids_valid]/dists_shortest*1000

        x_min = min(np.min(matched), np.min(shortest))
        x_max = max(np.max(matched), np.max(shortest))
        bins = np.linspace(x_min, x_max, self.n_bins)
        bincenters = self.plot_hist(ax, matched, bins=bins, color=self.color_matched, label='matched:' +
                                    '$\mu = %.2f/km$, $\sigma=%.2f/km$' % (np.mean(matched), np.std(matched)))
        bincenters = self.plot_hist(ax, shortest, bins=bins, color=self.color_shortest, label='shortest:' +
                                    '$\mu = %.2f/km$, $\sigma=%.2f/km$' % (np.mean(shortest), np.std(shortest)))

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        if self.is_title:
            ax.set_title('Priority change dens. of non-overlapping matched and shortest route',
                         fontsize=self.size_titlefont)
        ax.set_xlabel('Priority change density  n.o. [1/km]', fontsize=self.size_labelfont)
        ax.set_ylabel('Probability distribution', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))
        if self.is_save:
            self.save_fig('routeana_nodesdensity_nonoverlap')

    def plot_lowpriorityshare_nonoverlap(self):
        print 'plot_lowpriorityshare_nonoverlap'
        fig = self.create_figure()
        results = self.parent
        routesresults_shortest = results.routesresults_shortest
        routesresults_matched = results.routesresults_matched
        routesresults_shortest_nonoverlap = results.routesresults_shortest_nonoverlap
        routesresults_matched_nonoverlap = results.routesresults_matched_nonoverlap
        edgesresults = results.edgesresults

        ax = fig.add_subplot(111)
        ids_valid = routesresults_matched.select_ids(np.logical_and(
            routesresults_shortest_nonoverlap.distances.get_value() > 0, routesresults_matched_nonoverlap.distances.get_value() > 0))

        dists_match = routesresults_matched_nonoverlap.distances[ids_valid]
        dists_shortest = routesresults_shortest_nonoverlap.distances[ids_valid]

        matched = routesresults_matched_nonoverlap.lengths_low_priority[ids_valid]/dists_match*100
        shortest = routesresults_shortest_nonoverlap.lengths_low_priority[ids_valid]/dists_shortest*100

        x_min = min(np.min(matched), np.min(shortest))
        x_max = 15.0  # max(np.max(matched),np.max(shortest))
        bins = np.linspace(x_min, x_max, self.n_bins)
        bincenters = self.plot_hist(ax, matched, bins=bins, color=self.color_matched, label='matched:' +
                                    '$\mu = %.2f$%%, $\sigma=%.2f$%%' % (np.mean(matched), np.std(matched)))
        bincenters = self.plot_hist(ax, shortest, bins=bins, color=self.color_shortest, label='shortest:' +
                                    '$\mu = %.2f$%%, $\sigma=%.2f$%%' % (np.mean(shortest), np.std(shortest)))

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        if self.is_title:
            ax.set_title('Share of low priority roads of non-overlapping matched and shortest route',
                         fontsize=self.size_titlefont)
        ax.set_xlabel('Low priority road share  n.o. [%]', fontsize=self.size_labelfont)
        ax.set_ylabel('Probability distribution', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))
        if self.is_save:
            self.save_fig('routeana_lowpriorityshare_nonoverlap')

    def plot_exclusiveshare_nonoverlap(self):
        print 'plot_exclusiveshare_nonoverlap'
        fig = self.create_figure()
        results = self.parent
        routesresults_shortest = results.routesresults_shortest
        routesresults_matched = results.routesresults_matched
        routesresults_shortest_nonoverlap = results.routesresults_shortest_nonoverlap
        routesresults_matched_nonoverlap = results.routesresults_matched_nonoverlap
        edgesresults = results.edgesresults

        ax = fig.add_subplot(111)
        ids_valid = routesresults_matched.select_ids(np.logical_and(
            routesresults_shortest_nonoverlap.distances.get_value() > 0, routesresults_matched_nonoverlap.distances.get_value() > 0))

        dists_match = routesresults_matched_nonoverlap.distances[ids_valid]
        dists_shortest = routesresults_shortest_nonoverlap.distances[ids_valid]

        matched = routesresults_matched_nonoverlap.lengths_exclusive[ids_valid]/dists_match*100
        shortest = routesresults_shortest_nonoverlap.lengths_exclusive[ids_valid]/dists_shortest*100

        x_min = min(np.min(matched), np.min(shortest))
        x_max = max(np.max(matched), np.max(shortest))
        bins = np.linspace(x_min, x_max, self.n_bins)
        bincenters = self.plot_hist(ax, matched, bins=bins, color=self.color_matched, label='matched:' +
                                    '$\mu = %.2f$%%, $\sigma=%.2f$%%' % (np.mean(matched), np.std(matched)))
        bincenters = self.plot_hist(ax, shortest, bins=bins, color=self.color_shortest, label='shortest:' +
                                    '$\mu = %.2f$%%, $\sigma=%.2f$%%' % (np.mean(shortest), np.std(shortest)))

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        if self.is_title:
            ax.set_title('Share of exclusive access roads of non-overlapping matched and shortest route',
                         fontsize=self.size_titlefont)
        ax.set_xlabel('Exclusive access road share  n.o. [%]', fontsize=self.size_labelfont)
        ax.set_ylabel('Probability distribution', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))
        if self.is_save:
            self.save_fig('routeana_exclusiveshare_nonoverlap')

    def plot_mixshare_nonoverlap(self):
        print 'plot_mixshare_nonoverlap'
        fig = self.create_figure()
        results = self.parent
        routesresults_shortest = results.routesresults_shortest
        routesresults_matched = results.routesresults_matched
        routesresults_shortest_nonoverlap = results.routesresults_shortest_nonoverlap
        routesresults_matched_nonoverlap = results.routesresults_matched_nonoverlap
        edgesresults = results.edgesresults

        ax = fig.add_subplot(111)
        ids_valid = routesresults_matched.select_ids(np.logical_and(
            routesresults_shortest_nonoverlap.distances.get_value() > 0, routesresults_matched_nonoverlap.distances.get_value() > 0))

        dists_match = routesresults_matched_nonoverlap.distances[ids_valid]
        dists_shortest = routesresults_shortest_nonoverlap.distances[ids_valid]

        matched = routesresults_matched_nonoverlap.lengths_mixed[ids_valid]/dists_match*100
        shortest = routesresults_shortest_nonoverlap.lengths_mixed[ids_valid]/dists_shortest*100

        x_min = min(np.min(matched), np.min(shortest))
        x_max = max(np.max(matched), np.max(shortest))
        bins = np.linspace(x_min, x_max, self.n_bins)
        bincenters = self.plot_hist(ax, matched, bins=bins, color=self.color_matched, label='matched:' +
                                    '$\mu = %.2f$%%, $\sigma=%.2f$%%' % (np.mean(matched), np.std(matched)))
        bincenters = self.plot_hist(ax, shortest, bins=bins, color=self.color_shortest, label='shortest:' +
                                    '$\mu = %.2f$%%, $\sigma=%.2f$%%' % (np.mean(shortest), np.std(shortest)))

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        if self.is_title:
            ax.set_title('Share of mixed reserved access roads of non-overlapping matched and shortest route',
                         fontsize=self.size_titlefont)
        ax.set_xlabel('Mixed reserved access road share n.o. [%]', fontsize=self.size_labelfont)
        ax.set_ylabel('Probability distribution', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))
        if self.is_save:
            self.save_fig('routeana_mixshare_nonoverlap')
# -------------------------------------------------------------------------------

    def do(self):
        # print 'do',self.edgeattrname
        self.show()

    def get_scenario(self):
        return self.parent.get_scenario()
