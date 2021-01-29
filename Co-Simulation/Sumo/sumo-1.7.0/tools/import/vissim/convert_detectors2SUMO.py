#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2009-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    convert_detectors2SUMO.py
# @author  Lukas Grohmann <Lukas.Grohmann@ait.ac.at>
# @date    Aug 14 2015

"""
Parses induction loops and travel time measurements from a VISSIM .inpx file
and writes converted information to a given .add.xml file.
see documentation
"""
from __future__ import absolute_import
from __future__ import print_function

import argparse
from xml.dom import minidom
from xml.dom.minidom import Document

# want the sumolib tools
import sys
import os
import numpy as np
THIS_PATH = os.path.abspath(__file__)
addpath = os.path.abspath(THIS_PATH + '/../../../sumolib')
if addpath not in sys.path:
    sys.path.append(addpath)
import geomhelper  # noqa


def dict_from_node_attributes(node):
    """takes a xml node and returns a dictionary with its attributes"""
    return dict((attn, node.getAttribute(attn)) for attn in
                node.attributes.keys())


def nparr_from_dict_list(dicl_tab, col_ns, col_ts):
    """converts a dictionary into an np array table structure"""
    return np.array([tuple(rd.get(cn, '-1') for cn in col_ns) for rd in
                     dicl_tab], dtype=np.dtype(list(zip(col_ns, col_ts))))


def get_induction_loops(inpx_doc):
    induction_tab = [dict_from_node_attributes(nd) for nd in
                     inpx_doc.getElementsByTagName('dataCollectionPoint')]
    return induction_tab


def get_travel_time_detectors(inpx_doc):
    travel_time_tab = []
    for detector in \
            inpx_doc.getElementsByTagName('vehicleTravelTimeMeasurement'):
        travel_time_d = dict_from_node_attributes(detector)
        start = detector.getElementsByTagName('start')[0]
        travel_time_d['startLink'] = start.getAttribute('link')
        travel_time_d['startPos'] = start.getAttribute('pos')
        end = detector.getElementsByTagName('end')[0]
        travel_time_d['endLink'] = end.getAttribute('link')
        travel_time_d['endPos'] = end.getAttribute('pos')
        travel_time_tab.append(travel_time_d)
    return travel_time_tab


def get_detector_coords_from_link(link_id, link_tab, pathlen):
    link = [li for li in link_tab if li["no"] == link_id]
    if len(link) > 0:
        points = link[0]["points"]
        return get_point_on_polyline(points, float(pathlen))
    else:
        print("link not found")


def convert_vissim_to_sumo_coords(vissim_point, net_offset):
    sumo_loop_coords = [vissim_point[0] + float(net_offset[0]),
                        vissim_point[1] + float(net_offset[1])]
    return sumo_loop_coords


def create_measurement_file(induction_tab, travel_time_tab,
                            edge_tab, link_tab, net_offset):
    result_doc = Document()
    root = result_doc.createElement("additional")
    result_doc.appendChild(root)

    for loop in induction_tab:
        ind_loop = result_doc.createElement("inductionLoop")
        ind_loop.setAttribute("id", "_".join([loop["no"], loop["name"]]))
        sumo_lane = "_".join([loop["lane"].split(" ")[0],
                              str(int(loop["lane"].split(" ")[1]) - 1)])
        ind_loop.setAttribute("lane", sumo_lane)

        pathlen = loop["pos"]
        link_id = loop["lane"].split(" ")[0]
        lane_index = str(int(loop["lane"].split(" ")[1]) - 1)
        vissim_loop_coords = get_detector_coords_from_link(link_id,
                                                           link_tab,
                                                           pathlen)
        sumo_loop_coords = convert_vissim_to_sumo_coords(vissim_loop_coords,
                                                         net_offset)
        polyline = [lane for lane in
                    [edge for edge in edge_tab if edge["id"] == link_id][
                        0]["lanes"]
                    if lane["index"] == lane_index][0]["shape"].split(" ")
        shape = []
        for point in polyline:
            shape.append(point.split(","))
        edge_offset = geomhelper.polygonOffsetWithMinimumDistanceToPoint(
            sumo_loop_coords,
            [[float(coord) for coord in point] for point in shape])
        ind_loop.setAttribute("pos", str(edge_offset))
        ind_loop.setAttribute("file", "ind_out.xml")
        ind_loop.setAttribute("freq", "900")
        root.appendChild(ind_loop)

    for det in travel_time_tab:
        travel_time = result_doc.createElement("entryExitDetector")
        travel_time.setAttribute("id", det["no"])
        travel_time.setAttribute("freq", "900")
        travel_time.setAttribute("file", "time_out.xml")

        start_edge = [edge for edge in edge_tab if
                      edge["id"] == det["startLink"]]
        if len(start_edge) > 0:
            start_point = get_detector_coords_from_link(start_edge[0]["id"],
                                                        link_tab,
                                                        det["startPos"])
            sumo_point = convert_vissim_to_sumo_coords(start_point, net_offset)
            for lane in start_edge[0]["lanes"]:
                det_entry = result_doc.createElement("detEntry")
                polyline = lane["shape"].split(" ")
                shape = []
                for point in polyline:
                    shape.append(point.split(","))
                start_offset = geomhelper.polygonOffsetWithMinimumDistanceToPoint(
                    sumo_point,
                    [[float(coord) for coord in point] for point in shape])
                det_entry.setAttribute("lane", lane["id"])
                if start_offset < float(lane["length"]):
                    det_entry.setAttribute("pos", str(start_offset))
                else:
                    det_entry.setAttribute("pos", lane["length"])
                travel_time.appendChild(det_entry)
        end_edge = [edge for edge in edge_tab if
                    edge["id"] == det["endLink"]]
        if len(end_edge) > 0:
            end_point = get_detector_coords_from_link(end_edge[0]["id"],
                                                      link_tab,
                                                      det["endPos"])
            sumo_point = convert_vissim_to_sumo_coords(end_point, net_offset)
            for lane in end_edge[0]["lanes"]:
                det_exit = result_doc.createElement("detExit")
                polyline = lane["shape"].split(" ")
                shape = []
                for point in polyline:
                    shape.append(point.split(","))
                end_offset = geomhelper.polygonOffsetWithMinimumDistanceToPoint(
                    sumo_point,
                    [[float(coord) for coord in point] for point in shape])
                det_exit.setAttribute("lane", lane["id"])
                if end_offset < float(lane["length"]):
                    det_exit.setAttribute("pos", str(end_offset))
                else:
                    det_exit.setAttribute("pos", lane["length"])
                travel_time.appendChild(det_exit)
        root.appendChild(travel_time)
    return result_doc


def get_point_on_polyline(points, pathlen):
    points = np.array(points, dtype=float)
    index, rem_len = get_segment_of_polyline(points, pathlen)
    # check if index is reasonable value
    if index <= 0:
        print("WARNING: got invalid point on polyline")
        return None
    P = np.array(points[index - 1])
    # if the remaining length is within tolerance, snap to initial point
    if rem_len <= 1.0e-3:
        return P
    Q = np.array(points[index])
    PQ = Q - P  # Vektior PQ
    vn = PQ / np.linalg.norm(PQ)                # normierter Richtungsvektor
    return P + vn * rem_len


def get_segment_of_polyline(points, pathlen):
    """take a polyline and return the segment index where pathlen along the polyline lies
    """
    # check if pathlen is < 0
    if pathlen < 0:
        return 0, None
    seg_lens = get_segment_lengths(points)
    # check if pathlen is longer than polyline
    #   with a tolerance of 1e-4
    if pathlen > sum(seg_lens) + 1e-3:
        return -1, pathlen - sum(seg_lens)
    lm_segG = np.r_[0., np.cumsum(seg_lens)]
    index = np.digitize([pathlen], lm_segG).item()
    return (index, pathlen - lm_segG[index - 1])


def get_segment_lengths(points):
    dxyz = np.diff(points, axis=0)
    return np.linalg.norm(dxyz, axis=1)


def get_vissim_data(inpxdoc):
    link_tab = []
    for link in inpx_doc.getElementsByTagName('link'):
        link_d = {}
        link_d['no'] = link.getAttribute('no')
        link_d['lanes'] = []
        link_d['points'] = []
        for lane in link.getElementsByTagName('lane'):
            link_d['lanes'].append({'width': lane.getAttribute('width')})
        link_tab.append(link_d)
        for point in link.getElementsByTagName('point3D'):
            link_d['points'].append([point.getAttribute('x'),
                                     point.getAttribute('y')])

    from_to_tab = []
    for lin in inpxdoc.getElementsByTagName('link'):
        if lin.hasChildNodes():
            lep_d = {}      # link end point dict
            for ept in ('fromLinkEndPt', 'toLinkEndPt'):
                lep_nd = lin.getElementsByTagName(ept)
                ch0 = ept[0]    # identifier 'f'rom / 't'o
                if len(lep_nd) > 0:
                    dic = dict_from_node_attributes(lep_nd.item(0))
                    dic['link'], dic['lane'] = dic['lane'].split(' ')
                    lep_d.update(dict((ch0 + '_' + key, value)
                                      for key, value in dic.items()))
            lep_d.update({'_link': link_d['no'], })
            from_to_tab.append(lep_d)
    # which columns to pick ?
    from_to_tab = nparr_from_dict_list(
        from_to_tab,
        '_link f_link f_lane t_link t_lane'.split(),
        'O O i O i'.split())
    return link_tab, from_to_tab


def get_sumo_data(sumodoc):
    """parse the SUMO data"""
    junc_tab = []
    conn_tab = []
    edge_tab = []
    for edge in sumo_doc.getElementsByTagName('edge'):
        edge_d = dict_from_node_attributes(edge)
        edge_d['lanes'] = []
        for lane in edge.getElementsByTagName('lane'):
            edge_d['lanes'].append(dict_from_node_attributes(lane))
        edge_tab.append(edge_d)
    junc_tab = [dict_from_node_attributes(nd) for
                nd in sumodoc.getElementsByTagName('junction')]
    col_n = ('id', 'type', 'x', 'y', 'incLanes', 'intLanes')
    col_t = ('O', ) * 6
    junc_tab = nparr_from_dict_list(junc_tab, col_n, col_t)
    conn_tab = [dict_from_node_attributes(nd) for
                nd in sumodoc.getElementsByTagName('connection')]
    col_n = ('from', 'to', 'fromLane', 'toLane', 'via')
    col_t = ('O', ) * 5
    conn_tab = nparr_from_dict_list(conn_tab, col_n, col_t)
    return junc_tab, conn_tab, edge_tab


def get_conn_verb_rel(conn_tab, from_to_tab):
    """returns 2 dictionaries, which contains the relation between connections
    and verbinder"""

    conn_link_d = {}  # key = verbinder.id, value = list<connection.id>
    link_conn_d = {}  # key = connection.id, value = verbinder.id
    for conn in conn_tab:
        #
        if ':' not in conn['from']:
            link = from_to_tab[
                (from_to_tab['f_link'] == conn['from'].split("[")[0]) & (
                    from_to_tab['t_link'] == conn['to'].split("[")[0])]
            if len(link) > 0:
                # dictionary to get the connection id for a given verbinder id
                link_conn_d[conn['via']] = link['_link'][0]
                if link["_link"][0] in conn_link_d:
                    conn_link_d[link["_link"][0]].append(conn['via'])
                else:
                    conn_link_d[link["_link"][0]] = [conn['via']]
            else:
                print("from: " + conn['from'] + "to: " + conn['to'])
    return link_conn_d, conn_link_d


# MAIN
if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='detector conversion utility (VISSIM.inpx to SUMO)')
    parser.add_argument('--vissim-input', '-V', type=str,
                        help='VISSIM inpx file path')
    parser.add_argument('--output-file', '-o', type=str,
                        help='output file name')
    parser.add_argument('--SUMO-net', '-S', type=str,
                        help='SUMO net file path')
    args = parser.parse_args()
    print("\n", args, "\n")
    print('\n---\n\n* loading VISSIM net:\n\t', args.vissim_input)
    sumo_doc = minidom.parse(args.SUMO_net)
    inpx_doc = minidom.parse(args.vissim_input)
    net_offset = sumo_doc.getElementsByTagName('location')[0].getAttribute(
        'netOffset').split(',')
    link_tab, from_to_tab = get_vissim_data(inpx_doc)
    junc_tab, conn_tab, edge_tab = get_sumo_data(sumo_doc)

    conn_link_d = get_conn_verb_rel(conn_tab, from_to_tab)
    induction_tab = get_induction_loops(inpx_doc)
    travel_time_tab = get_travel_time_detectors(inpx_doc)

    result_doc = create_measurement_file(induction_tab,
                                         travel_time_tab,
                                         edge_tab,
                                         link_tab,
                                         net_offset)

    with open("%s.add.xml" % args.output_file, "w") as ofh:
        result_doc.writexml(ofh, addindent='    ', newl='\n')
        ofh.close()
