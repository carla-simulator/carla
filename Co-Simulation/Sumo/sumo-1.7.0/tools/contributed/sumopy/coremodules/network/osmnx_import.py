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

# @file    osmnx_import.py
# @author  Joerg Schweizer
# @date


import os
import json
import metworkx as nx

import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
import agilepy.lib_base.xmlman as xm


from netconvert import *
from coremodules.misc.shapeformat import guess_utm_from_coord


def import_nx(graphx, net, projparams=''):
    """
    Import networkx graphx into net
    """

    for id_node_sumo, nbrsdict in graphx.adjacency():
        print '  id_node_sumo', id_node_sumo
        for nbr, eattr in nbrsdict.items():
            print '    nbr, eattr', nbr, eattr

    if projparams == "":
        projparams_target = guess_utm_from_coord()


class OxImporter(Process):
    def __init__(self,  scenario,
                 ident='oximporter',
                 name='OSMnx importer',
                 info='Import of network imported with the help of osmnx.',
                 logger=None, **kwargs):

        print 'OxImporter.__init__'

        self._init_common(ident,
                          parent=scenario,
                          name=name,
                          logger=logger,
                          info=info,
                          )

        attrsman = self.set_attrsman(cm.Attrsman(self))

        # self.id_mode = attrsman.add(am.AttrConf('id_mode',  modechoices['bicycle'],
        #                                groupnames = ['options'],
        #                                choices = modechoices,
        #                                name = 'Mode',
        #                                info = 'Transport mode to be matched.',
        #                                ))

        self.osmdatafilepaths = attrsman.add(
            cm.AttrConf('osmdatafilepaths', kwargs.get('osmdatafilepaths', ''),
                        groupnames=['options'],
                        perm='rw',
                        name='OSM data file(s)',
                        wildcards='Jason file (*.json)|*.json',
                        metatype='filepaths',
                        info="""One or several jason filepaths holding OSM related information.""",
                        ))

        self.nxnetworkpaths = attrsman.add(
            cm.AttrConf('nxnetworkpaths', kwargs.get('nxnetworkpaths', ''),
                        groupnames=['options'],
                        perm='rw',
                        name='Networkx file(s)',
                        wildcards='NetworkX file (*.obj)|*.obj',
                        metatype='filepaths',
                        info="""One or several network X filepaths containing topological information of simplified network.""",
                        ))

        # def import_xnet(net, nodeshapefilepath, edgeshapefilepath, polyshapefilepath):
        #import_nodeshapes(net, nodeshapefilepath)

    def get_scenario(self):
        return self.parent

    def do(self):
        print self.ident+'.do'

        net = self.get_scenario().net
        projparams_target = net.get_projparams()
        if projparams_target in ("", "!"):
            projparams_target = ""

        for filepath in self.nxnetworkpaths:
            graphx = cm.load_obj(filepath)
            import_nx(graphx, net, projparams=projparams_target)
            projparams_target = net.get_projparams()

        # OxNodesImporter(self.nodeshapefilepath, self,
        #                projparams_target = projparams_target,
        #                is_guess_targetproj = is_guess_targetproj,
        #                logger = self.get_logger(),
        #                ).do()

        # ShapefileImporter(  self.nodeshapefilepath,
        #                    self.parent.net.nodes.coords,
        #                    map_attrconfig2shapeattr = {},
        #                    projparams_target = projparams_target,
        #                    is_guess_targetproj = is_guess_targetproj,
        #                    logger = self.get_logger(),
        #                    ).do()
        return True
