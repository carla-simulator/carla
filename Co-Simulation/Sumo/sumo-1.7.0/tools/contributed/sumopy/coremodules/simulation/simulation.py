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

# @file    simulation.py
# @author  Joerg Schweizer
# @date


import numpy as np
import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
import agilepy.lib_base.xmlman as xm
from agilepy.lib_base.misc import random_choice, get_inversemap
import results


class Simulation(cm.BaseObjman):
    def __init__(self, scenario,  name='Simulation',
                 info='Simulation, contains simulation specific parameters and methods.', **kwargs):
        # print 'Network.__init__',name,kwargs
        self._init_objman(ident='simulation',
                          parent=scenario,
                          name=name,
                          info=info,
                          version=0.2,
                          **kwargs)
        attrsman = self.set_attrsman(cm.Attrsman(self))
        self._init_attributes()
        self._init_constants()

    def _init_attributes(self):
        print 'Simulation._init_attributes'  # ,dir(self)
        attrsman = self.get_attrsman()

        # if self.get_version()<0.2:
        #    self.delete('results')

        self.results = attrsman.add(cm.ObjConf(
            results.Simresults('results', parent=self),
            is_child=False,
            is_save=False,  # will not be saved
            groups=['results']))
        # upgrade
        # self.results.set_save(False)
        # print '  self.results', self.results

    def _init_constants(self):
        # no! for attrs onlyself.do_not_save_attrs(['results',])# redundant is_save = False
        pass

    def get_scenario(self):
        return self.parent

    def add_simobject(self, obj=None, ident=None, SimClass=None, **kwargs):

        if obj is not None:
            ident = obj.get_ident()

        if not hasattr(self, ident):
            if obj is None:
                # init simobject and make it a child of simulation
                obj = SimClass(ident, self, **kwargs)
                is_child = True
            else:
                # link to simobject, which must be a child of another object
                is_child = False

            attrsman = self.get_attrsman()
            attrsman.add(cm.ObjConf(obj,
                                    groupnames=['simulation objects'],
                                    is_child=is_child,
                                    ))

            setattr(self, ident, obj)

        return getattr(self, ident)

    def get_simobjects(self):
        #demandobjects = set([])
        # for ident, conf in self.get_group_attrs('').iteritems():
        #    demandobjects.add(conf.get_value())
        return self.get_attrsman().get_group_attrs('simulation objects').values()
