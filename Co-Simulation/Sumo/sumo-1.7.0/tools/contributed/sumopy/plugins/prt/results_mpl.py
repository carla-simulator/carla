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
import matplotlib as mpl
from matplotlib.patches import Arrow, Circle, Wedge, Polygon, FancyArrow
from matplotlib.collections import PatchCollection
import matplotlib.colors as colors
import matplotlib.cm as cmx
import matplotlib.pyplot as plt
import matplotlib.image as image

import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
from agilepy.lib_base.geometry import *
from agilepy.lib_base.processes import Process

COLORS = ['#1f77b4', '#aec7e8', '#ff7f0e', '#ffbb78', '#2ca02c',
          '#98df8a', '#d62728', '#ff9896', '#9467bd', '#c5b0d5',
          '#8c564b', '#c49c94', '#e377c2', '#f7b6d2', '#7f7f7f',
          '#c7c7c7', '#bcbd22', '#dbdb8d', '#17becf', '#9edae5']


class StopresultsPlotter(Process):
    def __init__(self, results, name='Plot PRT stop results with Matplotlib',
                 info="Creates plots of PRT stop results using matplotlib",
                 logger=None, **kwargs):

        self._init_common('stopresultsplotter', parent=results, name=name,
                          info=info, logger=logger)

        print 'StopresultsPlotter.__init__', results, self.parent, len(self.get_stopresults())
        attrsman = self.get_attrsman()

        stops = self.get_stopresults().get_prtstops()
        choices_stop = {}
        for id_stop in stops.get_ids():
            choices_stop[str(id_stop)] = id_stop

        self.id_stop_plot = attrsman.add(cm.AttrConf('id_stop_plot', kwargs.get('id_stop_plot', stops.get_ids()[0]),
                                                     groupnames=['options'],
                                                     choices=choices_stop,
                                                     name='ID stop plot',
                                                     info='Plot results of PRT stop with this ID.',
                                                     ))

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

        self.color_line = attrsman.add(cm.AttrConf('color_line', kwargs.get('color_line', np.array([0, 0, 1, 1], dtype=np.float32)),
                                                   groupnames=['options'],
                                                   perm='wr',
                                                   metatype='color',
                                                   name='Line color',
                                                   info='Color of line in various diagrams.',
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

    def get_stopresults(self):
        return self.parent.prtstopresults

    def show(self):
        stopresults = self.get_stopresults()
        print 'show', stopresults
        # print '  dir(vehicleman)',dir(vehicleman)

        print '  len(stopresults)', len(stopresults)
        if len(stopresults) > 0:
            i_fig = 0
            plt.close("all")

            #i_fig +=1;fig = plt.figure(i_fig)
            # self.plot_waiting_person(fig)

            i_fig += 1
            fig = plt.figure(i_fig)
            self.plot_waiting_person_number(fig)

            #i_fig +=1;fig = plt.figure(i_fig)
            # self.plot_waiting_person_time(fig)

            i_fig += 1
            fig = plt.figure(i_fig)
            self.plot_waiting_person_number_stop(fig)

            i_fig += 1
            fig = plt.figure(i_fig)
            self.plot_flow_stop(fig)

            #i_fig +=1;fig = plt.figure(i_fig)
            # self.plot_flows_compare(fig)

            #i_fig +=1;fig = plt.figure(i_fig)
            # self.plot_flows_compare_stop(fig)

            plt.show()

    def plot_flow_stop(self, fig):
        print 'plot_flow_stop'
        id_stop = self.id_stop_plot
        stopresults = self.get_stopresults()
        ax = fig.add_subplot(111)

        n_stop, n_steps = stopresults.get_dimensions()
        t_step = stopresults.time_step.get_value()

        time = np.arange(n_steps, dtype=np.float32)*t_step
        i = 0

        flow_veh_av = np.mean(stopresults.inflows_veh[id_stop])*3600
        flow_pers_av = np.mean(stopresults.inflows_person[id_stop])*3600

        ax.plot(time, stopresults.inflows_veh[id_stop]*3600,
                label='Effective vehicle',
                color='b',
                linestyle='--', linewidth=self.width_line,
                marker='s', markersize=4*self.width_line,
                )

        ax.plot(time, stopresults.inflows_veh_sched[id_stop]*3600,
                label='Scheduled vehicle',
                color='c',
                linestyle=':', linewidth=self.width_line,
                marker='^', markersize=4*self.width_line,
                )

        ax.plot(time, stopresults.inflows_person[id_stop]*3600,
                label='Effective pers.',
                color='g',
                linestyle='-', linewidth=self.width_line,
                marker='o', markersize=4*self.width_line,
                )

        ax.plot([time[0], time[-1]], [flow_veh_av, flow_veh_av],
                label='Average vehicle',
                color='fuchsia',
                linestyle='-', linewidth=2*self.width_line,
                )

        ax.plot([time[0], time[-1]], [flow_pers_av, flow_pers_av],
                label='Average person',
                color='darkorange',
                linestyle='-', linewidth=2*self.width_line,
                )

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        if self.is_title:
            ax.set_title('Vehicle and person in-flows over time at PRT stop ID=%d' %
                         id_stop, fontsize=self.size_titlefont)
        ax.set_xlabel('Time [s]', fontsize=self.size_labelfont)
        ax.set_ylabel('In-flows [1/h]', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))

    def plot_waiting_person_time(self, fig):
        print 'plot_waiting_person_time'
        stopresults = self.get_stopresults()
        ax = fig.add_subplot(111)

        n_stop, n_steps = stopresults.get_dimensions()
        t_step = stopresults.time_step.get_value()
        #inds_stop = np.arange(n_stop, dtype = np.int32)
        time = np.arange(n_steps, dtype=np.float32)*t_step
        # works:ax.plot(time.reshape(n_steps,1),numbers_person_wait.reshape(n_steps,-1))
        i = 0
        for id_stop in stopresults.ids_stop.get_value():
            ax.plot(time, 1.0/60.0*stopresults.waittimes_tot[id_stop],
                    COLORS[i], linewidth=self.width_line,
                    label='PRT Stop ID=%d' % id_stop)
            i += 1

        if self.is_title:
            ax.set_title('Number of waiting persons over time', fontsize=self.size_titlefont)

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        ax.set_xlabel('Time [s]', fontsize=self.size_labelfont)
        ax.set_ylabel('Waiting times of passengers [min]', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))

    def plot_waiting_person_number_stop(self, fig):
        print 'plot_waiting_person_number_stop'
        stopresults = self.get_stopresults()
        #ax1 = fig.add_subplot(211)
        #ax2 = fig.add_subplot(212)
        ax = fig.add_subplot(111)

        n_stop, n_steps = stopresults.get_dimensions()
        t_step = stopresults.time_step.get_value()
        #inds_stop = np.arange(n_stop, dtype = np.int32)
        time = np.arange(n_steps, dtype=np.float32)*t_step
        # works:ax.plot(time.reshape(n_steps,1),numbers_person_wait.reshape(n_steps,-1))
        id_stop = self.id_stop_plot
        ax.plot(time, stopresults.numbers_person_wait[id_stop],
                'g', linewidth=self.width_line,
                label='PRT Stop ID=%d' % id_stop)

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        ax.set_xlabel('Time [s]', fontsize=self.size_labelfont)
        ax.set_ylabel('Number of waiting passengers', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))

    def plot_waiting_person_number(self, fig):
        print 'plot_waiting_person_number'
        stopresults = self.get_stopresults()
        #ax1 = fig.add_subplot(211)
        #ax2 = fig.add_subplot(212)
        ax = fig.add_subplot(111)

        n_stop, n_steps = stopresults.get_dimensions()
        t_step = stopresults.time_step.get_value()
        #inds_stop = np.arange(n_stop, dtype = np.int32)
        time = np.arange(n_steps, dtype=np.float32)*t_step
        # works:ax.plot(time.reshape(n_steps,1),numbers_person_wait.reshape(n_steps,-1))
        i = 0
        for id_stop in stopresults.ids_stop.get_value():
            print '  id_stop', id_stop
            ax.plot(time, stopresults.numbers_person_wait[id_stop],
                    COLORS[i], linewidth=self.width_line,
                    label='PRT Stop ID=%d' % id_stop)
            i += 1

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        ax.set_xlabel('Time [s]', fontsize=self.size_labelfont)
        ax.set_ylabel('Number of waiting passengers', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))

    def plot_flows_compare(self, fig):
        print 'plot_flows_compare'
        stopresults = self.get_stopresults()
        #time_update_flows = self.parent.vehicleman.time_update_flows.get_value()
        time_update_flows = 10
        ax = fig.add_subplot(111)

        n_stop, n_steps = stopresults.get_dimensions()
        t_step = stopresults.time_step.get_value()

        time = np.arange(n_steps, dtype=np.float32)*t_step
        i = 0
        flowmatrix = np.zeros((10, 10), dtype=np.int32)
        for id_stop in stopresults.ids_stop.get_value():
            print '    id_stop', id_stop
            # print '      sched',stopresults.inflows_veh_sched[id_stop]
            # print '      eff  ',stopresults.inflows_veh[id_stop]
            flowmatrix[np.array(time_update_flows*stopresults.inflows_veh_sched[id_stop], dtype=np.int32),
                       np.array(time_update_flows*stopresults.inflows_veh[id_stop], dtype=np.int32)] += 1
            # ax.plot(stopresults.inflows_veh_sched[id_stop]*3600,stopresults.inflows_veh[id_stop]*3600,
            #        COLORS[i],linewidth =self.width_line,
            #        label = 'PRT Stop ID=%d (effective)'%id_stop)

            i += 1
        print 'flowmatrix', flowmatrix
        # ax.matshow(flowmatrix)

        cax = ax.matshow(flowmatrix, cmap=cmx.get_cmap('PuBu'))
        cbar = fig.colorbar(cax)
        #ax.legend(loc='best',shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        ax.set_xlabel('Scheduled arrivals per interval', fontsize=self.size_labelfont)
        ax.set_ylabel('Effective arrivals per interval', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))

    def plot_flows_compare_stop(self, fig):
        print 'plot_flows_compare_stop'
        stopresults = self.get_stopresults()
        id_stop = self.id_stop_plot
        #time_update_flows = self.parent.vehicleman.time_update_flows.get_value()
        time_update_flows = 10
        ax = fig.add_subplot(111)

        n_stop, n_steps = stopresults.get_dimensions()
        t_step = stopresults.time_step.get_value()

        i = 0
        flows_sched = stopresults.inflows_veh_sched[id_stop]
        flows_eff = stopresults.inflows_veh[id_stop]

        x = flows_sched  # -np.mean(flows_sched)
        y = flows_eff  # -np.mean(flows_eff)
        flowcorr = np.correlate(x, y, 'full')/np.sqrt(np.sum(x*x)*np.sum(y*y))

        time = np.arange(-n_steps+1, n_steps, dtype=np.float32)*t_step

        print '    len(flowcorr),n_steps', len(flowcorr), len(time), n_steps

        ax.plot(time, flowcorr,
                COLORS[i], linewidth=self.width_line,
                label='PRT Stop ID=%d' % id_stop)

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        ax.set_xlabel('Time delay [s]', fontsize=self.size_labelfont)
        ax.set_ylabel('Cross-Correlation', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))

    def plot_flows(self, fig):
        print 'plot_flows'
        stopresults = self.get_stopresults()
        ax = fig.add_subplot(111)

        n_stop, n_steps = stopresults.get_dimensions()
        t_step = stopresults.time_step.get_value()

        time = np.arange(n_steps, dtype=np.float32)*t_step
        i = 0
        for id_stop in stopresults.ids_stop.get_value():
            ax.plot(time, stopresults.inflows_veh[id_stop]*3600,
                    COLORS[i], linewidth=self.width_line,
                    label='PRT Stop ID=%d (effective)' % id_stop)

            ax.plot(time, stopresults.inflows_veh_sched[id_stop]*3600,
                    COLORS[i], linestyle='--', linewidth=self.width_line,
                    label='PRT Stop ID=%d (scheduled)' % id_stop)

            ax.plot(time, stopresults.inflows_person[id_stop]*3600,
                    COLORS[i], linestyle=':', linewidth=self.width_line,
                    label='PRT Stop ID=%d (person)' % id_stop)

            i += 1

        ax.legend(loc='best', shadow=True, fontsize=self.size_labelfont)
        ax.grid(self.is_grid)
        ax.set_xlabel('Time [s]', fontsize=self.size_labelfont)
        ax.set_ylabel('In-flows [1/h]', fontsize=self.size_labelfont)
        ax.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))

        #('inflows_veh', {'name':'Vehicle in-flows', 'unit':'1/s',     'dtype':np.float32, 'info':'Vehicle flow into the stop over time.'}),
        #        ('inflows_veh_sched', {'name':'Sched. vehicle in-flows', 'unit':'1/s', 'dtype':np.float32, 'info':'Scheduled vehicle flow into the stop over time.'}),
        #        ('inflows_person', {'name':'Person in-flows', 'unit':'1/s', 'dtype':np.float32, 'info':'Person flow into the stop over time.'}),

    def plot_waiting_person(self, fig):
        print 'plot_waiting_person'
        stopresults = self.get_stopresults()
        ax1 = fig.add_subplot(211)
        ax2 = fig.add_subplot(212)
        #ax = fig.add_subplot(111)

        n_stop, n_steps = stopresults.get_dimensions()
        t_step = stopresults.time_step.get_value()
        #inds_stop = np.arange(n_stop, dtype = np.int32)
        time = np.arange(n_steps, dtype=np.float32)*t_step
        # works:ax.plot(time.reshape(n_steps,1),numbers_person_wait.reshape(n_steps,-1))
        i = 0
        for id_stop in stopresults.ids_stop.get_value():
            ax1.plot(time, stopresults.numbers_person_wait[id_stop],
                     COLORS[i], linewidth=self.width_line,
                     label='PRT Stop ID=%d' % id_stop)
            ax2.plot(time, 1.0/60.0*stopresults.waittimes_tot[id_stop],
                     COLORS[i], linewidth=self.width_line,
                     label='PRT Stop ID=%d' % id_stop)
            i += 1

        ax1.legend(loc='best', shadow=True, fontsize=14)
        ax1.grid(self.is_grid)
        ax1.set_xlabel('Time [s]', fontsize=14)
        ax1.set_ylabel('Number of waiting passengers', fontsize=14)
        ax1.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax1.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))

        ax2.legend(loc='best', shadow=True, fontsize=14)
        ax2.grid(self.is_grid)
        ax2.set_xlabel('Time [s]', fontsize=14)
        ax2.set_ylabel('Waiting times of passengers [min]', fontsize=14)
        ax2.tick_params(axis='x', labelsize=int(0.8*self.size_labelfont))
        ax2.tick_params(axis='y', labelsize=int(0.8*self.size_labelfont))

    def do(self):
        # print 'do',self.edgeattrname
        self.show()
        return True

    def get_scenario(self):
        return self._scenario
