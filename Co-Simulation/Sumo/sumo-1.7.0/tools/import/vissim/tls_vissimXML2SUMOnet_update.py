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

# @file    tls_vissimXML2SUMOnet_update.py
# @author  Lukas Grohmann <Lukas.Grohmann@ait.ac.at>
# @author  Gerald Richter <Gerald.Richter@ait.ac.at>
# @date    Jun 11 2015

"""
Converts a VISSIM-tls-description into a SUMO-tls-description and writes
the appended information to a copy of the given sumo.net file
"""
# TODO: usage doc.ref

from __future__ import print_function
from __future__ import absolute_import

import os
import argparse
from xml.dom import minidom
from copy import deepcopy

import numpy as np


def dict_from_node_attributes(node):
    """takes a xml node and returns a dictionary with its attributes"""
    return dict((attn, node.getAttribute(attn)) for attn in
                node.attributes.keys())


def nparr_from_dict_list(dicl_tab, col_ns, col_ts):
    """converts a dictionary into an np array table structure"""
    return np.array([tuple(rd.get(cn, '0') for cn in col_ns) for rd in
                     dicl_tab], dtype=np.dtype(list(zip(col_ns, col_ts))))


def get_conn_verb_rel(conn_tab, from_to_tab):
    """
    returns a dictionary to get the connection id for a
    given verbinder id and vice versa
    """
    conn_link_d = {}  # key = verbinder.id, value = list<connection.id>
    for conn in conn_tab:
        if ':' not in conn['from']:
            link = from_to_tab[
                (from_to_tab['f_link'] == conn['from'].split("[")[0]) &
                (from_to_tab['t_link'] == conn['to'].split("[")[0])]
            if len(link) > 0:
                if link["_link"][0] in conn_link_d:
                    conn_link_d[link["_link"][0]].append(conn['via'])
                else:
                    conn_link_d[link["_link"][0]] = [conn['via']]
            else:
                print("from: " + conn['from'] + "to: " + conn['to'])
    return conn_link_d


def parse_vissim_net_data(inpxdoc):
    link_tab = []
    from_to_tab = []
    for lin in inpxdoc.getElementsByTagName('link'):
        link_d = dict_from_node_attributes(lin)
        link_tab.append(link_d)
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
    link_tab = nparr_from_dict_list(link_tab, 'no name'.split(), ('O',) * 2)
    # lane_tab = just the number of lanes and width
    from_to_tab = nparr_from_dict_list(
        from_to_tab,
        '_link f_link f_lane t_link t_lane'.split(),
        'O O i O i'.split())
    return link_tab, from_to_tab


def parse_sumo_net_data(sumodoc):
    junc_tab = []
    conn_tab = []
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
    return junc_tab, conn_tab


def compute_signal_tables(disp_name_id_d, signal_state_d, prog_list):
    """
    completes the signal tables with all duration and beginning times
    (in the VISSIm .sig files are only the beginning times of the red and
    green phases as well as the durations of the other phases given )
    """
    for key, program in signal_state_d.items():
        cycletime = int([prog for prog in prog_list
                         if prog["id"] == key][0]["cycletime"])
        for sig_group in program.values():
            sig_seq = sig_group["signal_sequence"]
            sig_tab = deepcopy(disp_name_id_d[sig_seq])
            # durations füllen
            for state in sig_group["durations"]:
                itemindex = np.where(sig_tab == int(state["display"]))
                sig_tab[itemindex[0][0]][itemindex[1][0] + 2] \
                    = int(state["duration"])
            # begins füllen
            for cmd in sig_group["begins"]:
                itemindex = np.where(sig_tab == int(cmd["display"]))
                sig_tab[itemindex[0][0]][itemindex[1][0] + 1] \
                    = int(cmd["begin"])
            # begin Zeiten berechnen
            # bei zufälligem begin Eintrag starten
            i = itemindex[0][0]
            check = 0
            while check != len(sig_tab):
                if sig_tab[i - 1][1] == -1:  # duration bekannt
                    # überlauf cycletime
                    if (sig_tab[i][1] - sig_tab[i - 1][2]) < 0:
                        sig_tab[i - 1][1] = cycletime - \
                            (sig_tab[i - 1][2] - sig_tab[i][1])
                    else:
                        sig_tab[i - 1][1] = sig_tab[i][1] - sig_tab[i - 1][2]
                elif sig_tab[i - 1][2] == -1:  # begin bekannt
                    if sig_tab[i - 1][1] > sig_tab[i][1]:  # überlauf cycletime
                        sig_tab[i - 1][2] = \
                            (cycletime - sig_tab[i - 1][1]) + sig_tab[i][1]
                    else:
                        sig_tab[i - 1][2] = sig_tab[i][1] - sig_tab[i - 1][1]
                i -= 1
                check += 1
            i = 0
            while i < len(sig_tab):
                if (sig_tab[i][1] + sig_tab[i][2]) > cycletime:
                    diff = cycletime - sig_tab[i][1]
                    dur = sig_tab[i][2]
                    sig_tab[i][2] = diff
                    sig_tab = np.insert(
                        sig_tab, i, np.array(
                            (sig_tab[i][0], 0, dur - diff)), 0)
                    break
                i += 1
            sig_tab = sig_tab[np.argsort(sig_tab[:, 1])]
            sig_group["signal_table"] = sig_tab


def sigtable_split_time(signal_state_d, prog_list):
    # FIXME: doc
    reference_time_d = {}
    for key, program in signal_state_d.items():
        cycletime = int([prog for prog in prog_list
                         if prog["id"] == key][0]["cycletime"])
        reference_time = np.array([], dtype="int")
        reference_duration = np.array([], dtype="int")
        for sg in program.values():
            reference_time = np.append(
                reference_time, sg["signal_table"][:, 1])
        reference_time = np.unique(reference_time)
        i = 0
        while i < len(reference_time):
            if i == len(reference_time) - 1:
                ele = cycletime - reference_time[i]
            else:
                ele = reference_time[i + 1] - reference_time[i]
            reference_duration = np.append(reference_duration, ele)
            i += 1
        reference_time_d[key] = {}
        reference_time_d[key]["begin"] = reference_time
        reference_time_d[key]["duration"] = reference_duration
    return reference_time_d


def compute_sumo_signal_tables(reference_time_d,
                               signal_state_d,
                               sig_disp_list,
                               tls_state_vissim2SUMO):
    # FIXME: doc
    for key, program in signal_state_d.items():
        for sg in program.values():
            state = sg["signal_table"]
            ref_time = reference_time_d[key]["begin"]
            sumo_tab = ""
            for time in ref_time:
                i = 0
                while i < len(state):
                    if state[i][1] <= time < state[i][1] + state[i][2]:
                        sumo_state = tls_state_vissim2SUMO[
                            [sig for sig in sig_disp_list if
                             sig["id"] == str(state[i][0])][0]["state"]]
                        sumo_tab = "".join([sumo_tab, sumo_state])
                        break
                    i += 1
            sg["sumo_signal_table"] = sumo_tab


def get_sigcon_junc_relation(sig_con_tab, sig_group_conn_d, junc_tab):
    """
    allocates the VISSIM signalcontrollers to SUMO junctions
    """
    sigCon_junc_d = {}
    for sig_con in sig_con_tab:
        conn_l = []
        for sg in sig_con["_sgs"]:
            if sg["_sg"] in sig_group_conn_d:
                conn_l += sig_group_conn_d[sg["_sg"]]
            else:
                continue
        # intersection
        junction = [
            junc for junc in junc_tab if len(
                set(conn_l).intersection(
                    junc['intLanes'].split(" "))) > 0]
        if len(junction) > 0:
            junction = junction[0]
        else:
            continue
        sigCon_junc_d[sig_con["no"]] = junction["id"]
    return sigCon_junc_d


def get_sigseq_id_list(sig_seq_tab, sig_disp_list):
    # FIXME: doc
    disp_name_id_d = {}
    for seq in sig_seq_tab:
        id_list = []
        names = seq["name"].split("-")
        for name in names:
            id_list.append([[disp for disp in sig_disp_list
                             if disp["name"] == name][0]["id"], -1, -1])
        disp_name_id_d[seq["id"]] = np.array(id_list, dtype="int")
    return disp_name_id_d


def get_sg_connection_data(
        conn_tab,
        sig_con_tab,
        sig_head_d,
        edge_list,
        conn_link_d):
    # FIXME: doc
    sig_group_conn_d = {}  # dic [sigCon ID] =  List <[conn via]>
    for con in sig_con_tab:
        for sg in con['_sgs']:
            # check if a signalHead exists for the given signalGroup
            if sg['_sg'] in sig_head_d:
                for signal in sig_head_d[sg['_sg']]:
                    link = signal['link']
                    lane = str(int(signal['lane']) - 1)
                    # tls on normal edge or verbinder?
                    if is_verbinder_d[link] is False:
                        if link in edge_list:
                            connection = conn_tab[
                                (conn_tab["from"] == link) & (
                                    conn_tab["fromLane"] == lane)]
                        else:
                            check = True
                            split_len = 0
                            while check:
                                if "".join(
                                        [link, "[", str(split_len), "]"]) \
                                        in edge_list:
                                    split_len += 1
                                else:
                                    check = False
                            print("".join([link, "[", str(split_len), "]"]))
                            connection = conn_tab[(conn_tab["from"] == "".join(
                                [link, "[", str(split_len), "]"])) &
                                (conn_tab["fromLane"] == lane)][0]
                    else:
                        connection = conn_tab[
                            (conn_tab["via"] ==
                             [conn for conn in
                              conn_link_d[link] if conn[-1] == lane])]
                    if sg['_sg'] in sig_group_conn_d:
                        for conn in connection:
                            sig_group_conn_d[sg['_sg']].append(conn["via"])
                    else:
                        sig_group_conn_d[sg['_sg']] = []
                        for conn in connection:
                            sig_group_conn_d[sg['_sg']].append(conn["via"])
            else:
                print(sg['_sg'])
    return sig_group_conn_d


def parse_sig_file(sig_file):
    xmldoc = minidom.parse(sig_file)
    print('\n---\n\n* loading signal file:\n\t', sig_file)

    # just getting single head node
    sc_node = xmldoc.getElementsByTagName('sc').item(0)
    sc_id = sc_node.getAttribute('id')

    # get the signal displays; should be just 1 node
    sig_disp_nodes = sc_node.getElementsByTagName('signaldisplays')
    display_nodes = sig_disp_nodes.item(0).getElementsByTagName('display')
    # build for single current signal
    sig_disp_list = [dict_from_node_attributes(disp) for disp in display_nodes]
    [sd.update({'_sc_id': sc_id}) for sd in sig_disp_list]

    # signalsequences
    sig_seq_tab = []
    # sigStat_tab needed for default program
    sigStat_tab = []
    for sig_seq in sc_node.getElementsByTagName('signalsequence'):
        sig_seq_d = dict_from_node_attributes(sig_seq)
        sig_seq_tab.append(sig_seq_d)
        sig_state_l = [dict_from_node_attributes(sst) for
                       sst in sig_seq.getElementsByTagName('state')]
        [sst.update({'_sigSeq_id': sig_seq_d['id']}) for sst in sig_state_l]
        sigStat_tab.extend(sig_state_l)
    sgroup_list = []
    # holds defaultDurations, fixedstates, cmds
    prog_list = []
    # dict[prog_id][signal_id]
    # <signal_sequence>
    # <begins>
    # <durations>
    signal_state_d = {}

    # reading default program; should be just 1 node
    sgs_list = sc_node.getElementsByTagName('sgs')
    prog_id = '0'       # unsaved
    prog_d = dict((('id', prog_id), ))
    prog_list.append(prog_d)

    # default sg einlesen
    for sg in sgs_list.item(0).getElementsByTagName('sg'):
        sg_d = dict_from_node_attributes(sg)
        sg_d.update({'_prog_id': prog_id, })
        sgroup_list.append(sg_d)

    # other sg reading
    progs_node = sc_node.getElementsByTagName('progs').item(0)
    for prog_node in progs_node.getElementsByTagName('prog'):
        prog_d = dict_from_node_attributes(prog_node)
        prog_list.append(prog_d)
        prog_id = prog_d['id']
        signal_state_d[prog_id] = {}
        sg_nl = prog_node.getElementsByTagName(
            'sgs').item(0).getElementsByTagName('sg')
        for sg in sg_nl:
            sg_d = dict_from_node_attributes(sg)
            signal_state_d[prog_id][sg_d["sg_id"]] = {}
            signal_state_d[prog_id][sg_d["sg_id"]][
                "signal_sequence"] = sg_d["signal_sequence"]
            signal_state_d[prog_id][sg_d["sg_id"]]["begins"] = []
            signal_state_d[prog_id][sg_d["sg_id"]]["durations"] = []
            sg_d.update({'_prog_id': prog_id, })
            sgroup_list.append(sg_d)
            # fixedstates
            for fixStat in sg.getElementsByTagName('fixedstates').item(0).\
                    getElementsByTagName('fixedstate'):
                fixst = dict_from_node_attributes(fixStat)
                signal_state_d[prog_id][sg_d["sg_id"]][
                    "durations"].append(fixst)
            # cmds
            for cmd_node in sg.getElementsByTagName('cmds').item(0).\
                    getElementsByTagName('cmd'):
                cmd_d = dict_from_node_attributes(cmd_node)
                signal_state_d[prog_id][sg_d["sg_id"]]["begins"].append(cmd_d)

    return sig_seq_tab, signal_state_d, sig_disp_list, prog_list


def parse_inpx_sig_data(xmldoc):
    """parses the signal data from the .inpx file"""
    sig_controller_tab = []
    sig_head_d = dict()

    for controller in xmldoc.getElementsByTagName('signalController'):
        controller_d = dict_from_node_attributes(controller)
        sgs_l = [dict_from_node_attributes(sgn) for
                 sgn in controller.getElementsByTagName('signalGroup')]
        for sg in sgs_l:
            sg['_sg'] = " ".join([controller.getAttribute('no'), sg['no']])
        controller_d['_sgs'] = sgs_l
        sig_controller_tab.append(controller_d)

    # parse signalHeads
    for s_head_item in xmldoc.getElementsByTagName('signalHead'):
        sig_head = dict_from_node_attributes(s_head_item)
        sig_head['link'], sig_head['lane'] = sig_head['lane'].split(" ")
        # temp = sHead.getAttribute('lane').split(" ") # "link lane"
        if sig_head['sg'] in sig_head_d:
            sig_head_d[sig_head['sg']].append(sig_head)
        else:
            sig_head_d[sig_head['sg']] = [sig_head]
    return sig_controller_tab, sig_head_d


def edit_connections(conn_l, sumodoc, junc_id):
    i = 0
    while i < len(conn_l):
        for via in conn_l[i]:
            connection = [conn for conn in
                          sumodoc.getElementsByTagName("connection")
                          if conn.getAttribute("via") == via][0]
            connection.setAttribute("state", "o")  # CHECK
            connection.setAttribute("linkIndex", str(i))
            connection.setAttribute("tl", junc_id)
        i += 1


def is_verbinder(xmldoc):
    """checks if a given link is a verbinder"""
    is_verbinder_d = dict()
    for link in xmldoc.getElementsByTagName("link"):
        if len(link.getElementsByTagName("fromLinkEndPt")) > 0:
            is_verbinder_d[link.getAttribute("no")] = True
        else:
            is_verbinder_d[link.getAttribute("no")] = False
    return is_verbinder_d


def generate_xml_doc(
        sumo_tls_d, sigCon_junc_d,
        sig_con_tab, reference_time_d,
        sumodoc, prog_list_d, sig_group_conn_d):
    for tls_id, programs in sumo_tls_d.items():
        junc_id = sigCon_junc_d[tls_id]
        default_prog_id = [
            sig for sig in sig_con_tab if sig["no"] == tls_id][0]["progNo"]
        for prog_id, program in programs.items():
            signal_table = []
            for sg_id, sg in program.items():
                if " ".join([tls_id, sg_id]) in sig_group_conn_d:
                    signal_table.append([sg_id, sg["sumo_signal_table"]])
            signal_table = np.array(signal_table)
            signal_table = signal_table[
                signal_table[:, 0].astype("int").argsort()]
            sg_id_l = signal_table[:, 0]
            conn_l = []
            for s_id in sg_id_l:
                conn_l.append(sig_group_conn_d[" ".join([tls_id, s_id])])
            signal_table = np.delete(signal_table, 0, 1)
            signal_table = np.ravel(signal_table)
            state_l = []
            i = 0
            while i < len(signal_table[0]):
                j = 0
                duration = []
                while j < len(signal_table):
                    duration.append(signal_table[j][i])
                    j += 1
                state_l.append("".join(duration))
                i += 1
            duration_l = reference_time_d[tls_id][
                prog_id]["duration"]
            # edit net file
            junction = [junc for junc in sumodoc.getElementsByTagName(
                "junction") if junc.getAttribute("id") == junc_id][0]
            junction.setAttribute("type", "traffic_light")
            net = sumodoc.getElementsByTagName("net")[0]

            edit_connections(conn_l, sumodoc, junc_id)
            tl_logic = sumodoc.createElement("tlLogic")
            tl_logic.setAttribute("id", junc_id)
            tl_logic.setAttribute("type", "static")
            tl_logic.setAttribute("programID",
                                  [prog for prog in prog_list_d[tls_id]
                                      if prog["id"] == prog_id][0]["name"])
            tl_logic.setAttribute("offset", "0.00")
            net.insertBefore(tl_logic, junction)
            for state, duration in zip(state_l, duration_l):
                phase = sumodoc.createElement("phase")
                phase.setAttribute("duration", str(duration / 1000))
                phase.setAttribute("state", state)
                tl_logic.appendChild(phase)

        # create WAUT
        waut = sumodoc.createElement("WAUT")
        waut.setAttribute("startProg",
                          [prog for prog in prog_list_d[tls_id]
                              if prog["id"] == default_prog_id][0]["name"])
        waut.setAttribute("refTime", "100")
        waut.setAttribute("id", "".join(["w", tls_id]))
        # root.appendChild(WAUT)
        net.insertBefore(waut, junction)

        # create waut junction
        waut_junc = sumodoc.createElement("wautJunction")
        waut_junc.setAttribute("junctionID", junc_id)
        waut_junc.setAttribute("wautID", "".join(["w", tls_id]))
        # root.appendChild(wautJunction)
        net.insertBefore(waut_junc, junction)


# global signal color translation definition
tls_state_vissim2SUMO = {'RED': 'r',
                         'REDAMBER': 'u',
                         'GREEN': 'g',
                         'AMBER': 'y',
                         # this should be different: like in SUMO 'o', 'O'
                         'FLASHING_GREEN': 'g',
                         'OFF': 'O'}

# MAIN
if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='TLS conversion utility (VISSIM.inpx to SUMO)')
    parser.add_argument('--vissim-input',
                        '-V', type=str,
                        help='VISSIM inpx file path')
    parser.add_argument('--SUMO-net', '-S', type=str,
                        help='SUMO net file path')
    parser.add_argument('--output-file', '-o', type=str,
                        help='output file name')
    args = parser.parse_args()
    print("\n", args, "\n")
    print('\n---\n\n* loading VISSIM net:\n\t', args.vissim_input)
    xmldoc = minidom.parse(args.vissim_input)
    print('\n---\n\n* loading SUMO net:\n\t', args.SUMO_net,)
    sumodoc = minidom.parse(args.SUMO_net)

    edge_list = []
    for edge in sumodoc.getElementsByTagName('edge'):
        # is it a normal edge ?
        if not edge.hasAttribute("function"):
            edge_list.append(edge.getAttribute("id"))

    # INPX read
    sig_con_tab, sig_head_d = parse_inpx_sig_data(xmldoc)
    link_tab, from_to_tab = parse_vissim_net_data(xmldoc)
    is_verbinder_d = is_verbinder(xmldoc)

    # SUMO NET read
    junc_tab, conn_tab = parse_sumo_net_data(sumodoc)
    conn_link_d = get_conn_verb_rel(conn_tab, from_to_tab)

    # get the connections for every signal group
    sig_group_conn_d = get_sg_connection_data(conn_tab,
                                              sig_con_tab,
                                              sig_head_d,
                                              edge_list,
                                              conn_link_d)
    # related junction id for a given Signal Controller
    sigCon_junc_d = get_sigcon_junc_relation(sig_con_tab,
                                             sig_group_conn_d,
                                             junc_tab)

    # pick all the .sig files from the signalControllers
    sig_files = set(sc[att] for sc in sig_con_tab for att in
                    sc.keys() if 'supplyFile' in att and '.sig' in sc[att])
    # sig_files = ['TestsiteGraz_v01301.sig']  # DEBUG, just 1 file

    reference_time_d = {}
    sumo_tls_d = {}
    prog_list_d = {}

    for sig_file in sig_files:
        sig_file = os.path.join(os.path.dirname(args.vissim_input), sig_file)
        sig_seq_tab = []
        signal_state_d = {}
        sig_disp_list = []
        disp_name_id_d = {}

        # parse .sig files
        sig_seq_tab, signal_state_d, sig_disp_list, \
            prog_list_d[sig_disp_list[0]["_sc_id"]] = parse_sig_file(sig_file)
        tls_id = sig_disp_list[0]["_sc_id"]
        # returns a numpy array with the reference signal Sequence table
        # format: display_id || begin_time || duration
        disp_name_id_d = get_sigseq_id_list(sig_seq_tab, sig_disp_list)

        compute_signal_tables(
            disp_name_id_d, signal_state_d, prog_list_d[tls_id])

        # reference time and duration for every signal program
        # times need to be split, to convert the sig table from VISSIM to SUMO
        reference_time_d[tls_id] = sigtable_split_time(
            signal_state_d, prog_list_d[tls_id])

        compute_sumo_signal_tables(reference_time_d[tls_id],
                                   signal_state_d,
                                   sig_disp_list,
                                   tls_state_vissim2SUMO)

        # Format: [tls id][signal program id][signal group index]
        sumo_tls_d[tls_id] = signal_state_d

    generate_xml_doc(
        sumo_tls_d, sigCon_junc_d, sig_con_tab,
        reference_time_d, sumodoc, prog_list_d, sig_group_conn_d)

    with open("%s.net.xml" % args.output_file, "w") as ofh:
        sumodoc.writexml(ofh, addindent='    ', newl='\n')
        ofh.close()
