#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    signal_POIs_from_xodr.py
# @author  Gerald Richter; gerald.richter@ait.ac.at
# @date    2017-08-06

"""
# what does it do:
- extract signal records from an xodr file that was converted
    to a SUMO net using netconvert
- generate an additionals file containing pois of type='signal'
- ensure POIs are positioned and associated to the appropriate edge's lanes

# example call:
  signal_POIs_from_xodr.py data/OpenDrive/scen.xodr data/sumo/net.net.xml
  -> will create a file data/sumo/signals.add.xml
"""

import os
import numpy as np
# import pandas as pd         # want to drop that dep
import lxml.etree as lET

# polygon & POI
# http://www.sumo.dlr.de/userdoc/Simulation/Shapes.html
#
# <poly id="<POLYGON_ID>" type="<TYPENAME>" color="<COLOR>"
# fill="<FILL_OPTION>" layer="<LAYER_NO>" shape="<2D-POSITION>[
# <2D-POSITION>]*"/>
#
# <poi id="<POLYGON_ID>" type="<TYPENAME>" color="<RED>,<GREEN>,<BLUE>"
# layer="<LAYER_NO>" [(x="<X_POS>" y="<Y_POS>") | (lane="<LANE_ID>"
# pos="<LANE_POS>" [posLat="<LATERAL_POS>"])]/>

# xodr:
# road/signal
#     road:id
# net:  (from netconvert conversion)
# edge:id <- [-]<road:id>.<meters>.<cm>      # for driving
# lane:id <- [-]<road:id>.<meters>.<cm>_<lane>      # for driving
# lane:id <- :<road:id>.<meters>.<cm>_<lane>        # for internal


def lol_T(lol):
    """transpose the list-of-lists way"""
    return list(map(list, zip(*lol)))


def find_upstream_lin_m(lm_soff, lim_lin_m):
    """for a linear metrage array lm_soff, find the index
    of the max value, smaller lim_lin_m
    """
    # NOTE: more elegant; np.digitize()
    sel_ls_s = lm_soff < lin_m
    sel_ls_s &= lm_soff == lm_soff[sel_ls_s].max()
    return np.r_[range(len(sel_ls_s))][sel_ls_s].item()


def calculate_lin_m_width(wr_attribs, lin_m):
    """determine the width at given linear meterage
    wr_attribs hold parameters (sOffset, a,b,c,d)
    lin_m is relative to last lane section offset
    """
    wr_attribs = {k: float(v) for k, v in wr_attribs.iteritems()}
    ds = lin_m - wr_attribs['sOffset']
    pa = np.r_[[wr_attribs[k] for k in 'abcd']]
    va = np.r_[[1., ds, ds * ds, ds * ds * ds]]
    return (pa * va).sum()  # wr_attribs['a']


def get_OD_lane_widths(lane_sec, cur_ls_sOff):
    """takes a lane section element and its offest against the road
    returns [(lane_id, width), ...]
    """
    global xodr_ns

    # get width record for each lane
    ls_lanes = lane_sec.xpath('.//ns:lane', namespaces={'ns': xodr_ns})
    # container for each lane's calculated width at position
    lin_m_width = []  # [(lane_id, width), ]
    for lane in ls_lanes:
        width = 0.
        # determine last upstream width record
        l_soff = lane.xpath('.//ns:width/@sOffset', namespaces={'ns': xodr_ns})
        if len(l_soff) == 0:      # in case there is no width rec (xodr lane 0)
            l_soff = [0.0]
            # width = 0.
        else:
            l_soff = np.r_[l_soff].astype(float)
            l_soff += cur_ls_sOff         # to get actual linear meters along road
            # find relevant width record index
            wr_ind = find_upstream_lin_m(l_soff, lin_m)
            wr_attribs = lane.xpath('.//ns:width',
                                    namespaces={'ns': xodr_ns})[wr_ind].attrib
            # subtract outer element offset
            width = calculate_lin_m_width(wr_attribs, lin_m - cur_ls_sOff)
        lin_m_width.append((int(lane.attrib['id']), width))

    return lin_m_width


def extract_lanes_width_data(rte, ):
    """extract lane width data below given root xml tree element rte
    rte usually is a road or lane section
    return data as recarray with columns (id, sOffset, a,b,c,d)
    """
    global xodr_ns

    # get the lanes
    # print(rte.xpath('.//ns:lane/@id', namespaces={'ns':xodr_ns}))
    c_names = ['id', ]
    # fetch the lane ids
    ln_ids = rte.xpath('.//ns:lane/@id', namespaces={'ns': xodr_ns})
    olane_ra = [list(map(int, ln_ids)), ]
    # grab ANY lane/width for definition of keys
    wid_tmpl = rte.xpath('//ns:lane/ns:width',  namespaces={'ns': xodr_ns})[0].attrib
    c_names.extend(list(wid_tmpl.keys()))
    wd_par_num = []       # container for the numerics
    for lnid in ln_ids:
        # there should be only one
        wd_pars = rte.xpath('.//ns:lane[@id="%s"]/ns:width' % lnid,
                            namespaces={'ns': xodr_ns})
        if wd_pars:     # list does contain something
            wd_par_num.append(list(map(float, wd_pars[0].values())))
        else:
            wd_par_num.append([0., ] * len(wid_tmpl.keys()))
    # transpose the list-of-lists way:
    olane_ra.extend(lol_T(wd_par_num))
    olane_ra = np.rec.fromarrays(olane_ra, names=c_names)

    return olane_ra


if __name__ == "__main__":

    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("xodr_file", type=str,
                        help="file path of open drive file")
    parser.add_argument("net_file", type=str,
                        help="file path of net file")
    # parser.add_argument("workLog", type=str, help="work log file")
    args = parser.parse_args()

    net_Fp = args.net_file  # td_Dp+'/sumo/net.net.xml'
    xodr_Fp = args.xodr_file  # td_Dp+'/OpenDrive/scen_T01.02.xodr'

    # parse XODR tree
    otree = lET.parse(xodr_Fp)
    oroot = otree.getroot()
    # grab the docs namespace for xpath shortcut
    xodr_ns = oroot.nsmap[None]
    # nasty ns addressing
    roads = otree.xpath('ns:road', namespaces={'ns': xodr_ns})
    """
  # tried to drop ns, but no to avail
  import lxml.objectify
  lxml.objectify.deannotate(oroot, cleanup_namespaces=True)
  """

    # parse SUMO net xml tree:
    ntree = lET.parse(net_Fp)
    edges = ntree.xpath("edge[@type!='internal']")  # 'edge')
    # get non internal lanes
    nlanes = ntree.xpath("edge[@type!='internal']/lane")

    # get similarity ids incl. possible sign
    edge_ids = (e.attrib['id'].split('.', 1) for e in edges)
    # edge_df = pd.DataFrame(edge_ids, columns=('rd_ref','lin_m',))   # 'lane'))
    # edge_df.lin_m = edge_df.lin_m.astype(float)

    lane_ids = (l.attrib['id'].split('.', 1) for l in nlanes)
    lane_ids = ([c, l[0], ] + l[1].split('_') for c, l in enumerate(lane_ids))
    lane_ra = lol_T(list(lane_ids))
    lane_ra.append([l.attrib['width'] for l in nlanes])
    # get max len of string id
    idS_max = max(map(len, lane_ra[1]))
    lane_ra = np.rec.fromarrays(lane_ra, names=('index', 'rd_ref', 'lin_m', 'lane', 'width'),
                                formats=['i4', 'U%d' % idS_max, 'f4', 'i2', 'f4'])

    # create new tree root element for SUMO additionals file
    aroot = lET.Element('additional')
    atree = aroot.getroottree()

    # walk through all XODR roads
    for r_cnt, r in enumerate(roads, 1):
        road_id = r.attrib['id']
        print("* parsing road %2d : %s" % (r_cnt, road_id))
        road_len = float(r.attrib['length'])

        # find matching SUMO-net elements
        # edges matching the road-id
        # edge_sel = edge_df['rd_ref'].apply(lambda s:road_id in s)
        # xsd: every edge must have at least 1 lane
        # lanes matching the road-id
        lane_id_sel = np.fromiter(map(lambda s: road_id in s, lane_ra.rd_ref),
                                  np.dtype(bool))

        # got to find right lanes:laneSection
        # + with attribute s <= running road s of signal
        olane_ra = extract_lanes_width_data(r)

        # get the signals element for this road
        # sigs = r.xpath('ns:signals', namespaces={'ns':xodr_ns})
        # expecting just 1 element in signals
        # assert len(sigs) == 1
        # can use relative addressing instead of: sigs[0].xpath('ns:signal',

        # TODO: preselect, as we dont want TLS @dynamic='no'
        for s_cnt, s in enumerate(r.xpath('.//ns:signal', namespaces={'ns': xodr_ns}), 1):
            poi = {'id': s.attrib['id'], 'layer': '10',
                   'type': 'signal',
                   'width': '0.75', 'height': '0.75', 'fill': 'true', }  # just to show
            poi_params = [{'key': key, 'value': s.attrib.get(key, '')}
                          for key in s.attrib if key not in ('id', 's', 't')]
            print("* + parsing signal %2d : %s" % (s_cnt, poi['id']))
            lin_m = float(s.attrib['s'])        # get linear meter start

            # select the SUMO net lanes relevant
            lin_m_sel = lane_ra.lin_m < lin_m
            # join the lane selector with meterage limitations
            lane_sel = lane_id_sel & lin_m_sel
            # assuming we dont have to check for lanes of different length
            # take max linear meterage
            max_lin_m = lane_ra.lin_m[lane_sel].max()
            lane_sel &= (lane_ra.lin_m == max_lin_m)
            # need to calc that:
            # (lin.meterage of unsplit road) - lin.m of start of fitting split edge)
            poi['pos'] = "%e" % (lin_m - max_lin_m)

            # get the fitting laneSection from xodr
            # find last upstream elementa by s offsets of the starts
            ls_soff = np.r_[r.xpath('.//ns:laneSection/@s',
                                    namespaces={'ns': xodr_ns})].astype(float)
            ls_ind = find_upstream_lin_m(ls_soff, lin_m)
            lane_sec = r.xpath('.//ns:laneSection', namespaces={'ns': xodr_ns})[ls_ind]
            cur_ls_sOff = ls_soff[ls_ind]

            # TODO: future - process laneOffset, as another offset
            #       cur_ls_sOff +=- laneOffset
            lin_m_width = get_OD_lane_widths(lane_sec, cur_ls_sOff)

            # transposing to [[lane_ids...], [lane_widths...]]
            lin_m_width = lol_T(lin_m_width)
            # get in order 0,-1,-2,...
            decr_ord_ind = np.argsort(lin_m_width[0])[::-1]
            lin_m_width = [np.take(r, decr_ord_ind) for r in lin_m_width]
            # drop 0 width lanes, which are not translated to SUMO net
            lin_m_width = [r[lin_m_width[1] > 0.] for r in lin_m_width]
            num_oLanes = len(lin_m_width[0])      # number of xodr lanes at pos
            # add lane limits as lateral coordinates
            lin_m_width.append(-lin_m_width[-1].cumsum())

            # posLat transformation
            def_nRef_lane = 0             # default SUMO net reference lane
            nRef_lane = def_nRef_lane
            # the OD t-offset against lane 0, left border
            od_t_l0l = float(s.attrib['t'])       # lateral xodr position

            # locate on which xodr lane this offset lands. might be good info
            closest_lane_index = np.digitize(od_t_l0l, lin_m_width[-1]).item()
            # catch stray to right
            closest_lane_index = min(closest_lane_index, num_oLanes - 1)
            net_laneInds = list(reversed(range(len(lin_m_width[0]))))
            # get corresponding SUMO lane num_id
            nRef_lane = net_laneInds[closest_lane_index]
            # finally pack the sumo lane ids also
            lin_m_width.append(net_laneInds)

            lnid_sel = lane_ra.lane == nRef_lane
            lnid_gt_sel = lane_ra.lane > nRef_lane  # for lanes to left
            # assoc SUMO net lane
            nlane_wid = lane_ra[lane_sel & lnid_sel].width.item()
            nlleft_wid = lane_ra[lane_sel & lnid_gt_sel].width.sum()
            # width(sumo_ref.lane)/2. +width(all lanes left sumo ref.) + (od_t_l0l)
            posLat = nlane_wid / 2. + nlleft_wid + od_t_l0l
            poi['posLat'] = "%.2f" % posLat
            poi['lane'] = nlanes[lane_ra[lane_sel & lnid_sel].index[0]].attrib['id']
            # print(od_t_l0l, lin_m_width, net_laneInds, nRef_lane)
            # print(poi)

            #   signal:validity
            sign_valids = s.xpath('ns:validity', namespaces={'ns': xodr_ns})
            # TODO: test this part
            for v in sign_valids:
                # translate the xodr lane-range    #"<from_lane> <to_lane>"
                sumo_lids = [lin_m_width[3][int(v.attrib[atn]) == lin_m_width[0]].item()
                             for atn in 'fromLane toLane'.split()]
                poi_params.append({'key': 'validity',
                                   'value': " ".join(map(str, sumo_lids))})
            if len(sign_valids) == 0:
                print(": INFO : default signal validity for all lanes")

            # TODO: pick and insert image refs for the signal's <type>-<subtype>

            # changing data type
            poi = lET.Element('poi', attrib=poi)
            # append all params
            [poi.append(lET.Element('param', kvd)) for kvd in poi_params]
            aroot.append(poi)

    sig_Fp = os.path.join(os.path.dirname(net_Fp), 'signals.add.xml')
    print('-' * 3)
    ans = input('? write sumo additionals file with POIs ([N] / y): ')
    if ans.lower() == 'y':
        atree.write(sig_Fp, pretty_print=True)
        print('-' * 5, '\n', '* stored signal data to:', sig_Fp)
