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

# @file    result_oglviewer.py
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

    SUMOPYDIR = os.path.join(APPDIR, '..', '..')
    sys.path.append(os.path.join(SUMOPYDIR))

import numpy as np

from agilepy.lib_wx.ogleditor import *
from coremodules.network.network_editor import NetSelectTool, EdgeDrawings, NodeDrawings
from coremodules.landuse.wxgui import FacilityDrawings
from collections import OrderedDict

COLORMAP_JET = np.array([
    (0.0, 0.0, 0.5, 1.0),
    (0.0, 0.0, 0.517825311942959, 1.0),
    (0.0, 0.0, 0.535650623885918, 1.0),
    (0.0, 0.0, 0.553475935828877, 1.0),
    (0.0, 0.0, 0.571301247771836, 1.0),
    (0.0, 0.0, 0.589126559714795, 1.0),
    (0.0, 0.0, 0.60695187165775399, 1.0),
    (0.0, 0.0, 0.62477718360071299, 1.0),
    (0.0, 0.0, 0.64260249554367199, 1.0),
    (0.0, 0.0, 0.66042780748663099, 1.0),
    (0.0, 0.0, 0.67825311942958999, 1.0),
    (0.0, 0.0, 0.69607843137254899, 1.0),
    (0.0, 0.0, 0.71390374331550799, 1.0),
    (0.0, 0.0, 0.73172905525846699, 1.0),
    (0.0, 0.0, 0.74955436720142599, 1.0),
    (0.0, 0.0, 0.76737967914438499, 1.0),
    (0.0, 0.0, 0.78520499108734398, 1.0),
    (0.0, 0.0, 0.80303030303030298, 1.0),
    (0.0, 0.0, 0.82085561497326198, 1.0),
    (0.0, 0.0, 0.83868092691622098, 1.0),
    (0.0, 0.0, 0.85650623885917998, 1.0),
    (0.0, 0.0, 0.87433155080213898, 1.0),
    (0.0, 0.0, 0.89215686274509798, 1.0),
    (0.0, 0.0, 0.90998217468805698, 1.0),
    (0.0, 0.0, 0.92780748663101598, 1.0),
    (0.0, 0.0, 0.94563279857397498, 1.0),
    (0.0, 0.0, 0.96345811051693397, 1.0),
    (0.0, 0.0, 0.98128342245989297, 1.0),
    (0.0, 0.0, 0.99910873440285197, 1.0),
    (0.0, 0.0, 1.0, 1.0),
    (0.0, 0.0, 1.0, 1.0),
    (0.0, 0.0, 1.0, 1.0),
    (0.0, 0.0019607843137254902, 1.0, 1.0),
    (0.0, 0.0176470588235293, 1.0, 1.0),
    (0.0, 0.033333333333333333, 1.0, 1.0),
    (0.0, 0.049019607843137254, 1.0, 1.0),
    (0.0, 0.064705882352941183, 1.0, 1.0),
    (0.0, 0.080392156862744993, 1.0, 1.0),
    (0.0, 0.096078431372549025, 1.0, 1.0),
    (0.0, 0.11176470588235295, 1.0, 1.0),
    (0.0, 0.12745098039215685, 1.0, 1.0),
    (0.0, 0.14313725490196066, 1.0, 1.0),
    (0.0, 0.1588235294117647, 1.0, 1.0),
    (0.0, 0.17450980392156862, 1.0, 1.0),
    (0.0, 0.19019607843137254, 1.0, 1.0),
    (0.0, 0.20588235294117635, 1.0, 1.0),
    (0.0, 0.22156862745098038, 1.0, 1.0),
    (0.0, 0.2372549019607843, 1.0, 1.0),
    (0.0, 0.25294117647058822, 1.0, 1.0),
    (0.0, 0.26862745098039204, 1.0, 1.0),
    (0.0, 0.28431372549019607, 1.0, 1.0),
    (0.0, 0.29999999999999999, 1.0, 1.0),
    (0.0, 0.31568627450980391, 1.0, 1.0),
    (0.0, 0.33137254901960772, 1.0, 1.0),
    (0.0, 0.34705882352941175, 1.0, 1.0),
    (0.0, 0.36274509803921567, 1.0, 1.0),
    (0.0, 0.3784313725490196, 1.0, 1.0),
    (0.0, 0.39411764705882341, 1.0, 1.0),
    (0.0, 0.40980392156862744, 1.0, 1.0),
    (0.0, 0.42549019607843136, 1.0, 1.0),
    (0.0, 0.44117647058823528, 1.0, 1.0),
    (0.0, 0.45686274509803909, 1.0, 1.0),
    (0.0, 0.47254901960784312, 1.0, 1.0),
    (0.0, 0.48823529411764705, 1.0, 1.0),
    (0.0, 0.50392156862745097, 1.0, 1.0),
    (0.0, 0.51960784313725494, 1.0, 1.0),
    (0.0, 0.53529411764705859, 1.0, 1.0),
    (0.0, 0.55098039215686279, 1.0, 1.0),
    (0.0, 0.56666666666666665, 1.0, 1.0),
    (0.0, 0.58235294117647063, 1.0, 1.0),
    (0.0, 0.59803921568627449, 1.0, 1.0),
    (0.0, 0.61372549019607847, 1.0, 1.0),
    (0.0, 0.62941176470588234, 1.0, 1.0),
    (0.0, 0.64509803921568631, 1.0, 1.0),
    (0.0, 0.66078431372548996, 1.0, 1.0),
    (0.0, 0.67647058823529416, 1.0, 1.0),
    (0.0, 0.69215686274509802, 1.0, 1.0),
    (0.0, 0.707843137254902, 1.0, 1.0),
    (0.0, 0.72352941176470587, 1.0, 1.0),
    (0.0, 0.73921568627450984, 1.0, 1.0),
    (0.0, 0.75490196078431371, 1.0, 1.0),
    (0.0, 0.77058823529411768, 1.0, 1.0),
    (0.0, 0.78627450980392133, 1.0, 1.0),
    (0.0, 0.80196078431372553, 1.0, 1.0),
    (0.0, 0.81764705882352939, 1.0, 1.0),
    (0.0, 0.83333333333333337, 1.0, 1.0),
    (0.0, 0.84901960784313724, 1.0, 1.0),
    (0.0, 0.86470588235294121, 0.99620493358633777, 1.0),
    (0.0, 0.88039215686274508, 0.98355471220746371, 1.0),
    (0.0, 0.89607843137254906, 0.97090449082858954, 1.0),
    (0.0094876660341554168, 0.9117647058823527, 0.95825426944971559, 1.0),
    (0.022137887413029723, 0.9274509803921569, 0.94560404807084131, 1.0),
    (0.034788108791903853, 0.94313725490196076, 0.93295382669196714, 1.0),
    (0.047438330170777983, 0.95882352941176474, 0.92030360531309297, 1.0),
    (0.060088551549652112, 0.97450980392156861, 0.9076533839342189, 1.0),
    (0.072738772928526235, 0.99019607843137258, 0.89500316255534473, 1.0),
    (0.085388994307400365, 1.0, 0.88235294117647056, 1.0),
    (0.098039215686274495, 1.0, 0.8697027197975965, 1.0),
    (0.11068943706514844, 1.0, 0.85705249841872255, 1.0),
    (0.12333965844402275, 1.0, 0.84440227703984827, 1.0),
    (0.13598987982289687, 1.0, 0.8317520556609741, 1.0),
    (0.14864010120177101, 1.0, 0.81910183428209993, 1.0),
    (0.16129032258064513, 1.0, 0.80645161290322587, 1.0),
    (0.17394054395951927, 1.0, 0.7938013915243517, 1.0),
    (0.18659076533839339, 1.0, 0.78115117014547764, 1.0),
    (0.19924098671726753, 1.0, 0.76850094876660346, 1.0),
    (0.21189120809614148, 1.0, 0.75585072738772952, 1.0),
    (0.22454142947501579, 1.0, 0.74320050600885512, 1.0),
    (0.23719165085388991, 1.0, 0.73055028462998106, 1.0),
    (0.24984187223276405, 1.0, 0.717900063251107, 1.0),
    (0.26249209361163817, 1.0, 0.70524984187223283, 1.0),
    (0.27514231499051228, 1.0, 0.69259962049335866, 1.0),
    (0.2877925363693864, 1.0, 0.67994939911448449, 1.0),
    (0.30044275774826057, 1.0, 0.66729917773561032, 1.0),
    (0.31309297912713452, 1.0, 0.65464895635673637, 1.0),
    (0.3257432005060088, 1.0, 0.6419987349778622, 1.0),
    (0.33839342188488292, 1.0, 0.62934851359898802, 1.0),
    (0.35104364326375709, 1.0, 0.61669829222011385, 1.0),
    (0.3636938646426312, 1.0, 0.60404807084123968, 1.0),
    (0.37634408602150532, 1.0, 0.59139784946236562, 1.0),
    (0.38899430740037944, 1.0, 0.57874762808349156, 1.0),
    (0.40164452877925361, 1.0, 0.56609740670461739, 1.0),
    (0.4142947501581275, 1.0, 0.55344718532574344, 1.0),
    (0.42694497153700184, 1.0, 0.54079696394686905, 1.0),
    (0.43959519291587595, 1.0, 0.52814674256799488, 1.0),
    (0.45224541429475007, 1.0, 0.51549652118912082, 1.0),
    (0.46489563567362424, 1.0, 0.50284629981024676, 1.0),
    (0.47754585705249836, 1.0, 0.49019607843137258, 1.0),
    (0.49019607843137247, 1.0, 0.47754585705249841, 1.0),
    (0.50284629981024664, 1.0, 0.46489563567362435, 1.0),
    (0.5154965211891207, 1.0, 0.45224541429475018, 1.0),
    (0.52814674256799488, 1.0, 0.43959519291587601, 1.0),
    (0.5407969639468686, 1.0, 0.42694497153700228, 1.0),
    (0.55344718532574311, 1.0, 0.41429475015812778, 1.0),
    (0.56609740670461728, 1.0, 0.40164452877925361, 1.0),
    (0.57874762808349134, 1.0, 0.38899430740037955, 1.0),
    (0.59139784946236551, 1.0, 0.37634408602150538, 1.0),
    (0.60404807084123968, 1.0, 0.3636938646426312, 1.0),
    (0.61669829222011374, 1.0, 0.35104364326375714, 1.0),
    (0.62934851359898791, 1.0, 0.33839342188488297, 1.0),
    (0.64199873497786197, 1.0, 0.32574320050600891, 1.0),
    (0.65464895635673614, 1.0, 0.31309297912713474, 1.0),
    (0.66729917773561032, 1.0, 0.30044275774826057, 1.0),
    (0.67994939911448438, 1.0, 0.28779253636938651, 1.0),
    (0.69259962049335855, 1.0, 0.27514231499051234, 1.0),
    (0.70524984187223261, 1.0, 0.26249209361163817, 1.0),
    (0.71790006325110678, 1.0, 0.24984187223276411, 1.0),
    (0.73055028462998095, 1.0, 0.23719165085388993, 1.0),
    (0.74320050600885468, 1.0, 0.22454142947501621, 1.0),
    (0.75585072738772918, 1.0, 0.2118912080961417, 1.0),
    (0.76850094876660335, 1.0, 0.19924098671726753, 1.0),
    (0.78115117014547741, 1.0, 0.18659076533839347, 1.0),
    (0.79380139152435159, 1.0, 0.1739405439595193, 1.0),
    (0.80645161290322565, 1.0, 0.16129032258064513, 1.0),
    (0.81910183428209982, 1.0, 0.14864010120177107, 1.0),
    (0.83175205566097399, 1.0, 0.1359898798228969, 1.0),
    (0.84440227703984805, 1.0, 0.12333965844402273, 1.0),
    (0.85705249841872222, 1.0, 0.11068943706514867, 1.0),
    (0.86970271979759628, 1.0, 0.098039215686274495, 1.0),
    (0.88235294117647045, 1.0, 0.085388994307400434, 1.0),
    (0.89500316255534462, 1.0, 0.072738772928526263, 1.0),
    (0.90765338393421868, 1.0, 0.060088551549652092, 1.0),
    (0.92030360531309285, 1.0, 0.047438330170778031, 1.0),
    (0.93295382669196703, 1.0, 0.03478810879190386, 1.0),
    (0.94560404807084075, 0.98838053740014586, 0.022137887413030133, 1.0),
    (0.95825426944971526, 0.973856209150327, 0.0094876660341556285, 1.0),
    (0.97090449082858932, 0.95933188090050858, 0.0, 1.0),
    (0.98355471220746349, 0.94480755265069016, 0.0, 1.0),
    (0.99620493358633766, 0.93028322440087174, 0.0, 1.0),
    (1.0, 0.91575889615105321, 0.0, 1.0),
    (1.0, 0.9012345679012348, 0.0, 1.0),
    (1.0, 0.88671023965141638, 0.0, 1.0),
    (1.0, 0.87218591140159796, 0.0, 1.0),
    (1.0, 0.85766158315177943, 0.0, 1.0),
    (1.0, 0.84313725490196101, 0.0, 1.0),
    (1.0, 0.82861292665214259, 0.0, 1.0),
    (1.0, 0.81408859840232406, 0.0, 1.0),
    (1.0, 0.79956427015250564, 0.0, 1.0),
    (1.0, 0.78503994190268722, 0.0, 1.0),
    (1.0, 0.7705156136528688, 0.0, 1.0),
    (1.0, 0.75599128540305072, 0.0, 1.0),
    (1.0, 0.74146695715323196, 0.0, 1.0),
    (1.0, 0.72694262890341343, 0.0, 1.0),
    (1.0, 0.71241830065359502, 0.0, 1.0),
    (1.0, 0.69789397240377649, 0.0, 1.0),
    (1.0, 0.68336964415395807, 0.0, 1.0),
    (1.0, 0.66884531590413965, 0.0, 1.0),
    (1.0, 0.65432098765432123, 0.0, 1.0),
    (1.0, 0.63979665940450281, 0.0, 1.0),
    (1.0, 0.62527233115468439, 0.0, 1.0),
    (1.0, 0.61074800290486586, 0.0, 1.0),
    (1.0, 0.59622367465504744, 0.0, 1.0),
    (1.0, 0.58169934640522891, 0.0, 1.0),
    (1.0, 0.56717501815541049, 0.0, 1.0),
    (1.0, 0.55265068990559207, 0.0, 1.0),
    (1.0, 0.53812636165577366, 0.0, 1.0),
    (1.0, 0.52360203340595557, 0.0, 1.0),
    (1.0, 0.50907770515613682, 0.0, 1.0),
    (1.0, 0.49455337690631829, 0.0, 1.0),
    (1.0, 0.48002904865649987, 0.0, 1.0),
    (1.0, 0.46550472040668145, 0.0, 1.0),
    (1.0, 0.45098039215686292, 0.0, 1.0),
    (1.0, 0.4364560639070445, 0.0, 1.0),
    (1.0, 0.42193173565722608, 0.0, 1.0),
    (1.0, 0.40740740740740755, 0.0, 1.0),
    (1.0, 0.39288307915758913, 0.0, 1.0),
    (1.0, 0.37835875090777071, 0.0, 1.0),
    (1.0, 0.36383442265795229, 0.0, 1.0),
    (1.0, 0.34931009440813376, 0.0, 1.0),
    (1.0, 0.33478576615831535, 0.0, 1.0),
    (1.0, 0.32026143790849693, 0.0, 1.0),
    (1.0, 0.30573710965867851, 0.0, 1.0),
    (1.0, 0.29121278140886042, 0.0, 1.0),
    (1.0, 0.27668845315904156, 0.0, 1.0),
    (1.0, 0.26216412490922314, 0.0, 1.0),
    (1.0, 0.24763979665940472, 0.0, 1.0),
    (1.0, 0.23311546840958619, 0.0, 1.0),
    (1.0, 0.21859114015976777, 0.0, 1.0),
    (1.0, 0.20406681190994935, 0.0, 1.0),
    (1.0, 0.18954248366013093, 0.0, 1.0),
    (1.0, 0.1750181554103124, 0.0, 1.0),
    (1.0, 0.16049382716049398, 0.0, 1.0),
    (1.0, 0.14596949891067557, 0.0, 1.0),
    (1.0, 0.13144517066085715, 0.0, 1.0),
    (1.0, 0.11692084241103862, 0.0, 1.0),
    (1.0, 0.1023965141612202, 0.0, 1.0),
    (1.0, 0.087872185911401779, 0.0, 1.0),
    (0.99910873440285231, 0.07334785766158336, 0.0, 1.0),
    (0.98128342245989386, 0.058823529411765274, 0.0, 1.0),
    (0.96345811051693431, 0.044299201161946411, 0.0, 1.0),
    (0.94563279857397531, 0.029774872912127992, 0.0, 1.0),
    (0.92780748663101631, 0.015250544662309573, 0.0, 1.0),
    (0.90998217468805731, 0.00072621641249104307, 0.0, 1.0),
    (0.89215686274509831, 0.0, 0.0, 1.0),
    (0.8743315508021392, 0.0, 0.0, 1.0),
    (0.8565062388591802, 0.0, 0.0, 1.0),
    (0.8386809269162212, 0.0, 0.0, 1.0),
    (0.8208556149732622, 0.0, 0.0, 1.0),
    (0.80303030303030321, 0.0, 0.0, 1.0),
    (0.78520499108734421, 0.0, 0.0, 1.0),
    (0.76737967914438521, 0.0, 0.0, 1.0),
    (0.74955436720142621, 0.0, 0.0, 1.0),
    (0.73172905525846721, 0.0, 0.0, 1.0),
    (0.71390374331550821, 0.0, 0.0, 1.0),
    (0.69607843137254966, 0.0, 0.0, 1.0),
    (0.6782531194295901, 0.0, 0.0, 1.0),
    (0.6604278074866311, 0.0, 0.0, 1.0),
    (0.6426024955436721, 0.0, 0.0, 1.0),
    (0.6247771836007131, 0.0, 0.0, 1.0),
    (0.60695187165775399, 0.0, 0.0, 1.0),
    (0.589126559714795, 0.0, 0.0, 1.0),
    (0.571301247771836, 0.0, 0.0, 1.0),
    (0.553475935828877, 0.0, 0.0, 1.0),
    (0.535650623885918, 0.0, 0.0, 1.0),
    (0.517825311942959, 0.0, 0.0, 1.0),
    (0.5, 0.0, 0.0, 1.0),
], np.float32)


class ResultSelectTool(NetSelectTool):
    """
    Result Selection tool for result OGL canvas.
    """

    def __init__(self, parent, mainframe=None):
        """
        To be overridden by specific tool.
        It is important that all drawobjects provide the method 
        get_netelement and retien the objectmanager related to the drawobject 
        """
        self.init_common('resultselect', parent, 'View Options',
                         info='Select canvas objects to browse results',
                         is_textbutton=False,
                         )

        self._init_select(is_show_selected=False)

        self.init_options()

    def set_results(self, results):
        """
        Set results is necessary to define the attribute choice
        of edgeattrname
        """
        # edgeresultes....
        edgeresults = results.edgeresults
        attrnames_edgeresults = OrderedDict()
        edgeresultattrconfigs = edgeresults.get_group_attrs('results')
        edgeresultattrnames = edgeresultattrconfigs.keys()
        # edgeresultattrnames.sort()
        for attrname in edgeresultattrnames:
            attrconfig = edgeresultattrconfigs[attrname]

            attrnames_edgeresults[attrconfig.format_symbol()] = attrconfig.attrname
        self.edgeattrname.choices = attrnames_edgeresults
        self.parent.refresh_optionspanel(self)
        # now somebody needs to take care and refresh the browser..a bit messy

    def init_options(self):

        self.add(cm.AttrConf('edgeattrname', 'entered',
                             choices=['entered'],  # will be set set_results
                             groupnames=['options'],
                             name='Edge Quantity',
                             info='The edge related quantity to be plotted.',
                             ))

        # self.add(cm.AttrConf(  'is_show_network', False,
        #                                groupnames = ['options'],
        #                                name = 'Show network',
        #                                info = 'Shows a schematic network in the background.',
        #                                ))
       #
        # self.add(cm.AttrConf(  'is_show_title', True,
        #                                groupnames = ['options'],
        #                                name = 'Show tile',
        #                                info = 'Shows title and unit.',
        #                                ))

        self.add(cm.AttrConf('resultwidth', 10.0,
                             groupnames=['options'],
                             name='Result width',
                             unit='m',
                             info='Maximum width of graphical resuls on map.',
                             ))

        self.add(cm.AttrConf('length_arrowhead', 10.0,
                             groupnames=['options'],
                             name='Arrow length',
                             unit='m',
                             info='Length of arrowhead on result map.',
                             ))

        self.add(cm.AttrConf('is_widthvalue', False,
                             groupnames=['options'],
                             name='Value width?',
                             info='If True, the arrow width of the graphical representation is proportional to the result value.',
                             ))

        self.add(cm.AttrConf('is_colorvalue', True,
                             groupnames=['options'],
                             name='Value color?',
                             info='If True, the arrows of the graphical representation are filled with a colour representing the result value.',
                             ))

        self.add(cm.AttrConf('color_fill', np.array([0.3, 0.3, 1.0, 0.95], dtype=np.float32),
                             groupnames=['options'],
                             perm='wr',
                             metatype='color',
                             name='Fill color',
                             info='Fill color of result arrows in graphical representation. Only valid if no color-fill is chosen.',
                             ))

        self.add(cm.AttrConf('color_net', np.array([0.8, 0.8, 0.8, 0.8], dtype=np.float32),
                             groupnames=['options'],
                             perm='wr',
                             metatype='color',
                             name='Network color',
                             info='Color of network and buildings shown in the background. Use the  selection tool of the Canvas toolbar to show edges, nodes and facilities',
                             ))

        self.add(cm.AttrConf('color_background', np.array([0.0, 0.0, 0.0, 0.0], dtype=np.float32),
                             groupnames=['options'],
                             perm='wr',
                             metatype='color',
                             name='Background color',
                             info='Background color of canvas.',
                             ))

    def get_optionspanel(self, parent, size=(200, -1)):
        """
        Return tool option widgets on given parent
        """
        # opject panel shows always the option of this tool
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

    def on_apply_option(self, drawobj, id, ids):
        # print 'on_apply_option drawobj, id, ids',drawobj, id, ids
        drawing = self._canvas.get_drawing()

        # print '  drawing',drawing
        if drawing is not None:
            edgeresultdraws = drawing.get_drawobj_by_ident('edgeresultdraws')
            edgeresultdraws.configure(self.edgeattrname.get_value(),
                                      resultwidth=self.resultwidth.get_value(),
                                      length_arrowhead=self.length_arrowhead.get_value(),
                                      is_widthvalue=self.is_widthvalue.get_value(),
                                      is_colorvalue=self.is_colorvalue.get_value(),
                                      color_fill=self.color_fill.get_value(),
                                      )

            # drawobj.update()
            for ident_drawobj in ['facilitydraws', 'nodedraws', 'edgedraws']:
                drawing.get_drawobj_by_ident(ident_drawobj).set_color_default(self.color_net.get_value())

            self._canvas.set_color_background(self.color_background.get_value())

            self._canvas.draw()


class SimpleFacilityDrawings(FacilityDrawings):
    def __init__(self, ident, facilities, parent,  # self, facilities, parent,
                 color_default=np.array([0.8, 0.8, 0.8, 0.8]),
                 **kwargs):

        Polygons.__init__(self, ident,  parent,
                          name='Facility drawings',
                          linewidth=1,
                          **kwargs)

        self.delete('vertices')

        self.add(cm.AttrConf('color_facility_default', color_default,
                             groupnames=['options'],
                             perm='wr',
                             metatype='color',
                             name='Default color',
                             info='Default facility color.',
                             ))

        self.set_facilities(facilities)

    def set_color_default(self, color):
        self.color_facility_default.set_value(color)
        self.update()

    def is_tool_allowed(self, tool, id_drawobj=-1):
        """
        Returns True if this tool can be applied to this drawobj.
        Optionally a particular drawobj can be specified with id_drawobj.
        """
        # basic tools:
        return tool.ident not in ['configure', 'select_handles', 'delete', 'move', 'stretch']
        # return tool.ident not in ['delete',]

    def get_netelement(self):
        return self._facilities

    def get_vertices_array(self):
        return self._facilities.shapes[self.get_ids()]  # .value[self._inds_map]

    def get_vertices(self, ids):
        return self._facilities.shapes[ids]

    def set_vertices(self, ids, vertices, is_update=True):
        pass

    def set_element(self, facilities):
        # this is to unify set method
        self.set_facilities(facilities)

    def update(self, is_update=True):
        # assumes that arrsy structure did not change
        # print 'FacilityDrawings.update'
        n = len(self)
        self.colors.value[:] = np.ones((n, 4), np.float32)*self.color_facility_default.value
        self.colors_highl.value[:] = self._get_colors_highl(self.colors.value)

        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()


class SimpleNodeDrawings(NodeDrawings):
    def __init__(self, ident, nodes, parent,
                 color_node=np.array([0.8, 0.8, 0.8, 0.8]),
                 **kwargs):

        Circles.__init__(self, ident,  parent, name='Node drawings',
                         is_fill=False,  # Fill objects,
                         is_outline=True,  # show outlines
                         n_vert=21,  # default number of vertex per circle
                         linewidth=2,
                         **kwargs)

        self.delete('centers')
        self.delete('radii')

        self.add(cm.AttrConf('color_node_default', color_node,
                             groupnames=['options', 'nodecolors'],
                             metatype='color',
                             perm='wr',
                             name='Default color',
                             info='Default node color.',
                             ))

        self.set_netelement(nodes)

    def set_color_default(self, color):
        self.color_node_default.set_value(color)
        self.update()

    def is_tool_allowed(self, tool, id_drawobj=-1):
        """
        Returns True if this tool can be applied to this drawobj.
        Optionally a particular drawobj can be specified with id_drawobj.
        """
        # basic tools:
        return tool.ident not in ['configure', 'select_handles', 'delete', 'move', 'stretch']


class SimpleEdgeDrawings(EdgeDrawings):
    def __init__(self, ident, edges, parent,
                 info="Simplified edge representation for result viewing",
                 color_edge=np.array([0.8, 0.8, 0.8, 0.8]),
                 width_edge=2.0,
                 **kwargs):

        Polylines.__init__(self, ident,  parent, name='Edge drawings',
                           is_lefthalf=True,
                           is_righthalf=True,
                           arrowstretch=1.0,
                           joinstyle=BEVELHEAD,
                           **kwargs)

        self.delete('vertices')
        self.delete('widths')

        self.add(cm.AttrConf('color_default', color_edge,
                             groupnames=['options', 'edgecolors'],
                             perm='wr',
                             metatype='color',
                             name='Default color',
                             info='Default edge color.',
                             ))

        self.add(cm.AttrConf('width_edge', width_edge,
                             groupnames=['options'],
                             name='Edge width',
                             unit='m',
                             info='Maximum width of graphical resuls on map.',
                             ))

        self.set_netelement(edges)

    def set_color_default(self, color):
        self.color_default.set_value(color)
        self.update()

    def get_widths_array(self):
        # double because only the right half is shown
        # add a little bit to the width to make it a little wider than the lanes contained
        # return 2.2*self._edges.widths.value[self._inds_map]
        return self.width_edge.value*np.ones(len(self), np.float32)

    def set_vertices(self, ids, vertices, is_update=True):
        pass

    def get_widths(self, ids):
        return self.width_edge.value*np.ones(len(ids), np.float32)

    def set_widths(self, ids, values):
        pass

    def is_tool_allowed(self, tool, id_drawobj=-1):
        """
        Returns True if this tool can be applied to this drawobj.
        Optionally a particular drawobj can be specified with id_drawobj.
        """
        # basic tools:
        return tool.ident not in ['configure', 'select_handles', 'delete', 'move', 'stretch']

    def update(self, is_update=True):
        """
        Update color, assume that there have not been structural changes of the arrays
        """
        # assumes that edges have been set in set_edges
        self.colors_fill.value[:] = np.ones((len(self), 1), np.float32)*self.color_default.value
        self.colors_fill_highl.value[:] = self._get_colors_highl(self.colors_fill.value)

        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()


class ResultviewerTools(ToolsPanel):
    """
    Shows a toolpallet with different tools and an options panel.
    Here tools are added which 
    """

    def __init__(self, parent):
        ToolsPanel.__init__(self, parent, n_buttoncolumns=2, size_title=150)
        # add and set initial tool

        self.add_initial_tool(ResultSelectTool(self))
        # self.add_initial_tool(NetSelectTool(self))
        #self.add_tool(StretchTool(self, detectpix = 10))
        #self.add_tool(MoveTool(self, detectpix = 5))
        #self.add_tool(ConfigureTool(self, detectpix = 10))
        #self.add_tool(NetDeleteTool(self, detectpix = 5))

        # more tools can be added later...
        # self.reset_initial_tool()


class EdgeresultDrawings(Polylines):
    def __init__(self, ident, edgeresults, parent,   **kwargs):

        self.resultsattr = None

        # joinstyle
        # FLATHEAD = 0
        # BEVELHEAD = 1
        Polylines.__init__(self, ident,  parent, name='Edge result drawings',
                           is_lefthalf=False,
                           is_righthalf=True,  # reverse for english roads
                           arrowstretch=1.0,
                           joinstyle=FLATHEAD,  # BEVELHEAD,
                           **kwargs)

        self.delete('vertices')
        # self.delete('widths')

        self.add(cm.AttrConf('color_default', np.array([0.2, 0.2, 1.0, 0.8], np.float32),
                             groupnames=['options', 'colors'],
                             perm='wr',
                             metatype='color',
                             name='Default color',
                             info='Default color.',
                             ))

        self.set_element(edgeresults)

    def get_netelement(self):
        """
        This is used for the selection tool
        """
        # print 'EdgeresultDrawings.get_netelement'
        return self._edgeresults

    def get_vertices_array(self):
        return self._edges.shapes[self._edgeresults.ids_edge[self.get_ids()]]  # .value[self._inds_map]#[self.get_ids()]

    def get_vertices(self, ids):
        return self._edges.shapes[self._edgeresults.ids_edge[ids]]

    def set_vertices(self, ids, vertices, is_update=True):
        pass
        #self._edges.set_shapes(self._edgeresults.ids_edge[ids], vertices)

    def is_tool_allowed(self, tool, id_drawobj=-1):
        """
        Returns True if this tool can be applied to this drawobj.
        Optionally a particular drawobj can be specified with id_drawobj.
        """
        # basic tools:
        return tool.ident not in ['configure', 'select_handles', 'delete', 'move', 'stretch']
        # return tool.ident not in   ['delete',]

    def set_element(self, edgeresults):
        self._edgeresults = edgeresults
        self._edges = edgeresults.ids_edge.get_linktab()
        #self._inds_edges = self._edges.get_inds()
        self.clear_rows()
        # if len(self)>0:
        #    self.del_rows(self.get_ids())

        ids = self._edgeresults.get_ids()
        #self._inds_map = self._edges.get_inds(ids)
        n = len(ids)
        self.add_rows(ids=ids,
                      beginstyles=np.ones(n)*FLATHEAD,
                      endstyles=np.ones(n)*TRIANGLEHEAD,
                      )
        self.update()

    def configure(self, attrname,
                  resultwidth=15.0,
                  length_arrowhead=5.0,
                  is_widthvalue=True,
                  is_colorvalue=False,
                  color_fill=np.array([0.3, 0.3, 1.0, 0.95], dtype=np.float32),
                  ):
        """
        Configures all sorts of parameters and updates everything at once.
        Called by tool.
        """
        self.resultsattr = getattr(self._edgeresults, attrname)
        print 'configure', self.resultsattr.attrname, is_widthvalue, is_colorvalue
        # used for normalization
        if len(self.resultsattr.get_value()) == 0:
            return
        self.val_max = float(np.max(self.resultsattr.get_value()))
        ids = self.get_ids()
        n = len(self)
        if self.val_max > 10.0**-6:
            values_norm = np.array(self.resultsattr[ids], dtype=np.float32)/self.val_max
        else:
            values_norm = np.zeros(n, np.float32)

        if is_widthvalue:
            # adapt width to resultvalue
            # multiply factor two because of halfwidth
            self.widths[ids] = 2.0*values_norm * resultwidth

        else:
            # fixed width
            self.widths[ids] = 2.0*resultwidth

        self.color_default.set_value(color_fill)

        if is_colorvalue:
            inds_color = np.array(values_norm*(len(COLORMAP_JET)-1), np.int32)
            # print '  len(COLORMAP_JET)-1',len(COLORMAP_JET)-1
            # print '  inds_color',inds_color
            self.colors_fill[ids] = COLORMAP_JET[inds_color]
        else:
            self.colors_fill.value[:] = np.ones((n, 1), np.float32)*self.color_default.value

        self.update()

    def update(self, is_update=True):
        """
        Update color, assume that there have not been structural changes of the arrays
        """
        # assumes that edges have been set in set_edges
        # print 'Edgedrawing.update'
        #edgeinds = self._edges.get_inds()

        self.colors_fill_highl.value[:] = self._get_colors_highl(self.colors_fill.value)

        if is_update:
            self._update_vertexvbo()
            self._update_colorvbo()


NETDRAWINGS = [
    ('nodedraws', SimpleNodeDrawings, 'nodes', 100),
    ('edgedraws', SimpleEdgeDrawings, 'edges', 100),
    #('lanedraws', LaneDrawings, 'lanes', 15),
    #('connectiondraws', ConnectionDrawings, 'connections', 25),
    #('crossingsdraws', CrossingDrawings, 'crossings', 30),
]
RESULTDRAWINGS = [
    ('edgeresultdraws', EdgeresultDrawings, 'edgeresults', 10),
]


class Resultviewer(OGleditor):
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
        self._toolspanel = ResultviewerTools(self)

        # compose editor window
        sizer.Add(self._toolspanel, 0, wx.ALL | wx.ALIGN_LEFT | wx.GROW, 4)  # from NaviPanelTest
        # sizer.Add(self._canvas,1,wx.GROW)# from NaviPanelTest
        sizer.Add(navcanvas, 1, wx.GROW)

        # finish panel setup
        self.SetSizer(sizer)
        sizer.Fit(self)

    def set_resultsdrawings(self, results):
        for ident_drawob, DrawobjClass, attrname, layer in RESULTDRAWINGS:
            self.set_elemetdrawing(ident_drawob, DrawobjClass, getattr(results, attrname), layer)

        # add also some simple netdrawings
        self.set_netdrawings(results.get_scenario().net)

        # and also simplified buildings
        self.set_elemetdrawing('facilitydraws', SimpleFacilityDrawings, getattr(
            results.get_scenario().landuse, 'facilities'), 200)

        # TODO: these initialization methods and set_(real)element
        # should be unified and fimplified

    def set_elemetdrawing(self, ident_drawobj, DrawClass, element, layer=50):
        drawing = self._drawing
        drawobj = drawing.get_drawobj_by_ident(ident_drawobj)
        if drawobj is not None:
            drawobj.set_element(element)
        else:
            drawobj = DrawClass(ident_drawobj, element, drawing)
            drawing.add_drawobj(drawobj, layer)

    def set_results(self, results, is_redraw=False):
        if self._drawing is None:

            #drawing = OpenGLdrawing()
            drawing = OGLdrawing()
            self.set_drawing(drawing)
            self.add_drawobjs_anim()
            self.set_resultsdrawings(results)
            # print 'set_net',net,id(net),drawing,id(drawing)

        else:
            # self.set_drawing(OGLdrawing())
            # self.add_drawobjs_anim()
            self.set_resultsdrawings(results)
            # pass

        # this tool needs a link to resulte
        tool = self._toolspanel.get_tool_by_ident('resultselect')
        # print 'set_results',tool
        if tool is not None:
            tool.set_results(results)
            # self._toolspanel.set_tool(tool)
            pass

        if is_redraw:
            self.draw()
            pass
        return self._drawing  # returned for test purposes

    def set_netdrawings(self, net):
        for ident_drawob, DrawobjClass, netattrname, layer in NETDRAWINGS:
            self.set_netdrawing(ident_drawob, DrawobjClass, getattr(net, netattrname), layer)

    def set_netdrawing(self, ident_drawobj, DrawClass, netelement, layer=50):
        drawing = self._drawing
        drawobj = drawing.get_drawobj_by_ident(ident_drawobj)
        if drawobj is not None:
            drawobj.set_netelement(netelement)
        else:
            drawobj = DrawClass(ident_drawobj, netelement, drawing)
            drawing.add_drawobj(drawobj, layer)

    def set_net(self, net, is_redraw=False):
        if self._drawing is None:
            # drawing should be present before showing network elements
            pass

        else:
            self.set_netdrawings(net)

        # self._canvas.set_drawing(drawing)
        if is_redraw:
            self.draw()

        return self._drawing  # returned for test purposes


class ResultviewrMainframe(AgileToolbarFrameMixin, wx.Frame):
    """
    Standalone result viewer....under costruction.
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
        drawing = OGLdrawing()

        edgesdrawings = EdgeDrawings(self._net.edges, drawing)
        drawing.add_drawobj(edgesdrawings)

        nodesdrawings = NodeDrawings(self._net.nodes, drawing)
        drawing.add_drawobj(nodesdrawings)

        canvas = self.gleditor.get_canvas()
        canvas.set_drawing(drawing)
        wx.CallAfter(canvas.zoom_tofit)
