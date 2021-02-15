#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2015-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    convert_vissimXML_flows_statRoutes.py
# @author  Lukas Grohmann <Lukas.Grohmann@ait.ac.at>
# @author  Gerald Richter <Gerald.Richter@ait.ac.at>
# @date    Jun 09 2015

"""
Parses flows and static routes from a VISSIM .inpx file
and writes converted information to a SUMO routes (.rou.xml) file.
(see source documentation)

example usage:
    python3 convert_vissimXML_flows_statRoutes.py my_VISSIM_scenario.inpx my_VISSIM_scenario.net.xml
    -o my_VISSIM_routes.rou.xml

see also:
    python3 convert_vissimXML_flows_statRoutes.py -h
"""
from __future__ import absolute_import
from __future__ import print_function

import argparse
from xml.dom import minidom
import numpy as np
from xml.dom.minidom import Document


def _dict_from_node_attributes(node):
    """takes xml node and returns a dict with its attributes
    """
    return dict((attn, node.getAttribute(attn)) for attn in
                node.attributes.keys())


# FUNKTIONEN
def parse_flows(xmldoc):
    """parses the vehicleInput flows from the VISSIM data
    :param xmldoc:  input VISSIM xml
    :type xmldoc:   xml.dom.minidom.Document
    :return:    flow data by VISSIM start link id
    :rtype:     dict

    .. note:: time frames are converted from [ms] -> [s]
    .. todo:: remove the redundant col2 in ['flow']
    """
    flw_d = dict()          # local flows dict
    for v_input in xmldoc.getElementsByTagName('vehicleInput'):
        v_input_d = _dict_from_node_attributes(v_input)
        v_input_d["vehComp"] = []
        v_input_d["volType"] = []
        v_input_d["flow"] = []
        for volume in v_input.getElementsByTagName('timeIntervalVehVolume'):
            v_input_d["vehComp"].append(volume.getAttribute('vehComp'))
            v_input_d["volType"].append(volume.getAttribute('volType'))
            # keeping (timeInterval, volume, vehicle composition)
            #   time interval converted to [s]
            v_input_d["flow"].append(
                [float(volume.getAttribute('timeInt').split(" ")[1]) / 1000,
                 float(volume.getAttribute('volume')),
                 float(volume.getAttribute('vehComp'))])        # FIXME: nasty, redundant
        v_input_d["flow"] = np.array(v_input_d["flow"])
        # here goes a VISSIM linkId as key (& value)
        flw_d[v_input_d["link"]] = v_input_d
    return flw_d


def parse_max_acc(xmldoc):
    """parses the vehicle acceleration distributions from the VISSIM data
    :param xmldoc:  input VISSIM xml
    :type xmldoc:   xml.dom.minidom.Document
    :return:    map of 1st acceleration function data point value by str(numeric id)
    :rtype:     dict
    """
    acc_d = dict()
    for max_acc in xmldoc.getElementsByTagName('maxAccelerationFunction'):
        acc_d[max_acc.getAttribute('no')] = max_acc.getElementsByTagName(
            'accelerationFunctionDataPoint')[0].getAttribute('y')
    return acc_d


def parse_speed_avg(xmldoc):
    """parses the vehicle speed distribution from the VISSIM data
    :param xmldoc:  input VISSIM xml
    :type xmldoc:   xml.dom.minidom.Document
    :return:    map of some speed averages by str(numeric id)
    :rtype:     dict

    .. note:: the average is only approximated
    """
    spd_d = dict()           # local speeds dict
    for deSpeeDis in xmldoc.getElementsByTagName('desSpeedDistribution'):
        # get mean speed
        num = 0.
        sum_val = 0.
        data_points = deSpeeDis.getElementsByTagName('speedDistributionDataPoint')
        for point in data_points:
            num += 1
            sum_val += float(point.getAttribute('x'))
        spd_d[deSpeeDis.getAttribute('no')] = str((sum_val / num) / 3.6)
    return spd_d


def parse_length(xmldoc):
    """parses the vehicle type lengths from the VISSIM data
    :param xmldoc:  input VISSIM xml
    :type xmldoc:   xml.dom.minidom.Document
    :return:    map of lenghts by str(numeric type)
    :rtype:     dict
    """
    len_d = dict()
    model_d = dict()
    # get model data
    for model in xmldoc.getElementsByTagName('model2D3D'):
        model_d[model.getAttribute('no')] = model.getElementsByTagName(
            'model2D3DSegment')[0].getAttribute('length')
    # calculate length data
    for model_dist in xmldoc.getElementsByTagName('model2D3DDistribution'):
        elements = model_dist.getElementsByTagName(
            'model2D3DDistributionElement')
        length = 0
        total_probability = 0
        for element in elements:
            total_probability += float(element.getAttribute('share'))
        for element in elements:
            length += (
                float(element.getAttribute('share')) / total_probability) * \
                float(model_d[element.getAttribute('model2D3D')])
        len_d[model_dist.getAttribute('no')] = str(length)
    return len_d


def parse_veh_comp(xmldoc):
    """parses the vehicle composition from the VISSIM data
    :param xmldoc:  input VISSIM xml
    :type xmldoc:   xml.dom.minidom.Document
    :return:    relevant VISSIM vehicleComposition data
    :rtype:     dict of list of dict
    """
    veh_cmp_d = dict()          # local vehicle compositions' dict
    for vehicle_comp in xmldoc.getElementsByTagName('vehicleComposition'):
        rel_flows = vehicle_comp.getElementsByTagName(
            'vehicleCompositionRelativeFlow')
        flow_l = []
        for flow in rel_flows:
            flw_d = {
                'desSpeedDistr': flow.getAttribute('desSpeedDistr'),
                'rel_flow': flow.getAttribute('relFlow'),
                'vehType': flow.getAttribute('vehType'),
            }
            flow_l.append(flw_d)
        # list of dictionaries
        veh_cmp_d[vehicle_comp.getAttribute('no')] = flow_l
    return veh_cmp_d


def parse_vehicle_types(xmldoc, acc_d, length_d):
    """parses the vehicle types from the VISSIM data
    :param xmldoc:  input VISSIM xml
    :type xmldoc:   xml.dom.minidom.Document
    :return:    relevant VISSIM vehicle type data
    :rtype:     dict of dict
    """
    veh_type_d = dict()
    for veh_type in xmldoc.getElementsByTagName('vehicleType'):
        type_d = {
            'id':       veh_type.getAttribute('no'),
            'length':   length_d[veh_type.getAttribute('model2D3DDistr')],
            'acc':      acc_d[veh_type.getAttribute('maxAccelFunc')],
        }
        veh_type_d[veh_type.getAttribute('no')] = type_d
    return veh_type_d


# FIXME: not necessarily nicely done
def gen_verbinder_map(xmldoc):
    """produce dict with boolean values to check if a given link is a verbinder
    :param xmldoc:  input VISSIM xml
    :type xmldoc:   xml.dom.minidom.Document
    :return:    map of VISSIM link id -> bool flag if link is 'Verbinder'
    :rtype:     dict
    """
    # simple implementation by static variable; xmldoc arg is in the way
    # if not hasattr(gen_verbinder_map, "v_dic"):
    # gen_verbinder_map.v_dic = dict()  # doesn't exist yet, so initialize
    is_verbinder_d = dict()
    for link in xmldoc.getElementsByTagName("link"):
        if len(link.getElementsByTagName("fromLinkEndPt")) > 0:
            is_verbinder_d[link.getAttribute("no")] = True
        else:
            is_verbinder_d[link.getAttribute("no")] = False
    # returning a dict...
    return is_verbinder_d


def parse_routes(xmldoc, edge_id_list, verbinder_d):
    """parses the VISSIM route information of statically defined routes ONLY
    :param xmldoc:  input VISSIM xml
    :type xmldoc:   xml.dom.minidom.Document
    :param edge_id_list:    the name says it all; SUMO edge ids
    :param verbinder_d:     bool(verbinder status) of VISSIM link id
    :type verbinder_d:      dict
    :return:    routes by VISSIM start link id, with respective destination routes
    :rtype:     dict

    .. note:: time frames are converted from [ms] -> [s]
    .. todo:: extend for non-static routes
    """
    # create a list of just the split vissim edges (marked by ending char ']')
    split_edge_list = [e for e in edge_id_list if e[-1] == ']']
    rts_by_start_d = dict()     # dictionary[start_link] = list(<Route>)
    # loop over all routing decisions
    for decision in xmldoc.getElementsByTagName('vehicleRoutingDecisionStatic'):
        start_link = decision.getAttribute('link')
        rts_by_start_d[start_link] = []
        for vehRtStatic in decision.getElementsByTagName('vehicleRouteStatic'):
            route_d = {
                "start_link": start_link,       # VISSIM id
                "dest_link": vehRtStatic.getAttribute('destLink'),  # VISSIM id
                "r_id": vehRtStatic.getAttribute('no'),
                "rel_flow": [],
                "links": [start_link, ]     # finally translated to SUMO ids
            }
            # split into separate time intervals' relative flow data
            for tIrFlow in map(str.strip, str(vehRtStatic.getAttribute('relFlow')).split(',')):
                if len(tIrFlow) == 0:
                    continue
                temp = tIrFlow.split()      # get "id", "tInterval:relFlow"
                try:
                    tIrFlow = map(float, temp[1].split(':'))    # grab [tInterval, relFlow]
                except TypeError:
                    print('- WARNING - incomplete relative flow definition in inpx\n',
                          decision.toxml())
                route_d["rel_flow"].append(list(tIrFlow))
            tIrFlow = np.array(route_d["rel_flow"])
            if len(tIrFlow) > 0:
                tIrFlow[:, 0] /= 1000            # VISSIM time intervals [ms]->[s]
                route_d["rel_flow"] = tIrFlow
            else:
                # create something.. 0 rows, 2 cols
                # NOTE: better None, but takes some adaption work
                route_d["rel_flow"] = np.empty((0, 2), dtype="f")

            # get all the intermediary links in their sumo representation
            for link in vehRtStatic.getElementsByTagName('intObjectRef'):
                link_key = link.getAttribute('key')
                if verbinder_d[link_key]:
                    # exclude VISSIM connectors (usually id > 10k)
                    continue
                # collect them all in VISSIM scheme first, then replace them
                route_d["links"].append(link_key)
            route_d["links"].append(route_d["dest_link"])

            # translate to sumo edge ids
            sumo_links = []
            for link_key in route_d["links"]:
                if link_key in edge_id_list:
                    # key is found unmodified in edge_id_list
                    sumo_links.append(link_key)
                else:
                    # extension list *IS* ordered by its splitting sequence as generated
                    sumo_links.extend(e for e in split_edge_list
                                      if e.startswith(link_key + '['))
            # update with sumo ids info
            route_d["links"] = sumo_links

            # add route object to dictionary
            rts_by_start_d[start_link].append(route_d)
    return rts_by_start_d


def calc_route_probability(routes_by_start_d, flow_d):
    """computes the route probabilies
    :param routes_by_start_d:   map by start link id with route dicts as values
    :type routes_by_start_d:    dict
    :param flow_d:      vissim vehicle in-flow data
    :type flow_d:       dict
    """
    for start_link, sl_routes in routes_by_start_d.items():
        if start_link not in flow_d:
            # we got no in-flow data for that route's start link
            print('- skipping probability calc\n',
                  '\tfor route without flow def. for VISSIM start link id:', start_link)
            continue
        # set 0 vectors for all time frames
        absolute_flow = flow_d[start_link]["flow"][:, 1]
        veh_comp = flow_d[start_link]["vehComp"]
        # time frames must have the same limits as flows, as checked before
        #   therefor all route flows for 1 start link must also have same limits
        # get all the startlink-route rel.flows-by-time-window lined up
        sl_rt_relF = np.stack([rt['rel_flow'] for rt in sl_routes])
        # all summed rel.flows by timeframe
        # sl_sum_relF =  sl_rt_relF.sum(axis=0)[:, 1:]         # keep shape (n x timeframes)
        sl_sum_relF = sl_rt_relF.sum(axis=0)[:, 1]          # shape (timeframes, )
        for route in routes_by_start_d[start_link]:
            # set the vehicle type for each route
            route["type"] = veh_comp
            route["probability"] = np.zeros_like(absolute_flow)
            # get a selector for all summed up flows > 0 (= relevant)
            comp_flow_sel = sl_sum_relF > 0.
            route["probability"][comp_flow_sel] = \
                (route["rel_flow"][comp_flow_sel, 1] / sl_sum_relF[comp_flow_sel])


def validate_rel_flow(routes_by_start_d, flow_d):
    """checks if a relative flow is missing and completes it if necessary
    essentially fixing a VISSIM inp -> inpx conversion bug
    :param routes_by_start_d:   map by start link id with route dicts as values
    :type routes_by_start_d:    dict
    :param flow_d:      vissim vehicle in-flow data
    :type flow_d:       dict

    .. note:: *modifies* routes_by_start_d
    """
    # VISSIM BUG!!: Relative Zuflüsse mit dem Wert 1.0 gehen bei der
    # Konversion von .inp zu .inpx verloren

    # compare all rel_flows with the reference flow
    # get all time frame limits from all routes
    # np.concatenate([rt['rel_flow'] for rtl in routes_by_start_d.values() for rt in rtl])
    for start_link, sl_routes in routes_by_start_d.items():
        if start_link not in flow_d:
            # should we remove the routes entry ?
            print('- skipping flow validation\n'
                  '\tfor route without flow def. for VISSIM start link id:', start_link)
            # CHECK: is this ok with later steps ?
            continue
        # grab all the time window starts from the flows
        #   NOTE: need slice here due to redundant veh_comp col.
        ref_time_shape = flow_d.get(start_link)["flow"][:, :2]
        ref_time_shape[:, 1] = 1.        # set to default (VISSIM inp-> inpx BUG)
        for route in sl_routes:
            # check if there is a relative flow def. at all
            if len(route["rel_flow"]) == 0:
                # if not, append default
                route["rel_flow"] = ref_time_shape.copy()
                continue
            else:
                # check if the time frame starts are the same
                assert np.array_equal(ref_time_shape[:, 0], route["rel_flow"][:, 0]),\
                    "\nPROBLEM: flow count and relative flow time frames are not aligned\n\t"\
                    "for VISSIM start link id: " + start_link
        # copy back modifications
        routes_by_start_d[start_link] = sl_routes


def create_vTypeDistribution_elems(veh_comp_d, veh_type_d, speed_d, root):
    """append the vehicle distribution data to the given dom document
    :param veh_comp_d:
    :type veh_comp_d:   dict
    :param veh_type_d:
    :type veh_type_d:   dict
    :param speed_d:
    :type speed_d:      dict
    :param root:   XML root element to append children to

    .. note:: *modifies/extends* XML root element
    """
    # iterate vehicle compositions
    for c_id, comps in veh_comp_d.items():
        v_type_dist = root.ownerDocument.createElement("vTypeDistribution")
        v_type_dist.setAttribute("id", c_id)
        root.appendChild(v_type_dist)
        for comp in comps:
            v_type = root.ownerDocument.createElement("vType")
            v_type.setAttribute(
                "id",
                "t{}_D{}".format(
                    veh_type_d[comp["vehType"]]["id"],
                    c_id))
            v_type.setAttribute("accel", veh_type_d[comp["vehType"]]["acc"])
            v_type.setAttribute("length",
                                veh_type_d[comp["vehType"]]["length"])
            v_type.setAttribute("probability", comp["rel_flow"])
            v_type.setAttribute("maxSpeed", speed_d[comp["desSpeedDistr"]])
            v_type_dist.appendChild(v_type)
    # return route_doc


def create_routeDistribution_elems(routes_by_start_d, root):
    """append the route distribution data into the given dom document
    :param routes_by_start_d:   map by start link id with route dicts as values
    :type routes_by_start_d:    dict
    :param root:   XML root element to append children to

    .. note:: *modifies/extends* XML root element
    """
    # iterating by VISSIM link id
    for start_link in routes_by_start_d:
        if start_link not in flow_d:
            # no flow, no go
            print('- skipping route dist. gen\n'
                  '\tfor route without flow def. for VISSIM start link id:', start_link)
            continue
        if len(routes_by_start_d[start_link]) == 0:
            continue
        ref_time = flow_d[start_link]["flow"][:, 0]
        for ic, time in enumerate(ref_time):
            route_dist = root.ownerDocument.createElement("routeDistribution")
            # just a name
            route_dist.setAttribute("id", "_".join([start_link,
                                                    str(time)]))
            for route in routes_by_start_d[start_link]:
                if np.abs(route["probability"][ic]) != 0:
                    route_node = root.ownerDocument.createElement("route")
                    route_node.setAttribute("id", route["r_id"])
                    route_node.setAttribute("edges",
                                            " ".join(route["links"]))
                    route_node.setAttribute("probability",
                                            str(np.abs(
                                                route["probability"][ic])))
                    route_dist.appendChild(route_node)
            if route_dist.hasChildNodes():
                root.appendChild(route_dist)
    # return route_doc


def create_flow_elems(routes_by_start_d, flow_d, root):
    """append the flow data to the given dom document
    :param routes_by_start_d:   map by start link id with route dicts as values
    :type routes_by_start_d:    dict
    :param flow_d:      vissim vehicle in-flow data
    :type flow_d:       dict
    :param root:   XML root element to append children to

    .. note:: *modifies/extends* XML root element
    """
    sim_end = inpx_doc.getElementsByTagName("simulation")[0].getAttribute("simPeriod")
    dom_flow_l = []
    for start_link in routes_by_start_d:
        if start_link not in flow_d:
            # we got no in-flow data for that route's start link
            print('- skipping flow gen\n'
                  '\tfor route without flow def. for VISSIM start link id:', start_link)
            continue
        if len(routes_by_start_d[start_link]) == 0:
            print('- found no routes by start link:', start_link)
            continue
        flows = flow_d[start_link]["flow"]
        # iterate over all the time frame starts from the flows
        ref_time = flows[:, 0]
        for index, time in enumerate(ref_time):
            in_flow = [fl for fl in flow_d[start_link]["flow"] if
                       fl[0] == time][0]
            if in_flow[1] > 0:
                flow = root.ownerDocument.createElement("flow")
                flow.setAttribute("id", "fl{}_st{}".format(start_link,
                                                           time))
                flow.setAttribute("color", "1,1,0")
                flow.setAttribute("begin", str(time))
                if index < len(ref_time) - 1 and len(ref_time) > 1:
                    flow.setAttribute("end",
                                      str(time + ref_time[index + 1]))
                else:
                    flow.setAttribute("end", sim_end)
                flow.setAttribute("vehsPerHour", str(in_flow[1]))
                flow.setAttribute("type", str(int(in_flow[2])))
                flow.setAttribute('route', "_".join([start_link,
                                                     str(time)]))
                dom_flow_l.append(flow)
    dom_flow_l = sorted(dom_flow_l,
                        key=lambda dom: float(dom.getAttribute("begin")))
    for dom_obj in dom_flow_l:
        root.appendChild(dom_obj)
    # return route_doc


# MAIN
if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='road network conversion utility for static route flows'
        ' (VISSIM.inpx to SUMO); generates SUMO routes definition file from'
        ' given inpx and derived (by netconvert) SUMO net.')
    parser.add_argument('--output-file', '-o', default='routes.rou.xml',
                        help='output file name (default: %(default)s)')
    parser.add_argument('--vissim-file', '-V', dest="vissim_file", required=True,
                        help='VISSIM inpx file path')
    parser.add_argument('--sumo-net-file', '-n', dest="sumo_net_file", required=True,
                        help='SUMO net file path')
    args = parser.parse_args()
    # print("\n", args, "\n")

    #
    # Input data ##########
    #
    print('\n---\n\n* loading VISSIM net:\n\t', args.vissim_file)
    inpx_doc = minidom.parse(args.vissim_file)
    print('\n---\n\n* loading SUMO net:\n\t', args.sumo_net_file,)
    sumo_doc = minidom.parse(args.sumo_net_file)

    print('+ building edge list...')
    # for all normal edges
    sumo_edge_ids = [edge.getAttribute("id") for edge in
                     sumo_doc.getElementsByTagName('edge')
                     if not edge.hasAttribute("function")]
    print('\tOK.')

    print('+ building "Verbinder"("connector") info...')
    # to check if a link is a verbinder
    verbinder_flag = gen_verbinder_map(inpx_doc)
    print('\tOK.')

    print('\n---')
    #
    # Vehicle Speeds, distributions, types ##########
    #
    print('* parsing speeds...')
    # parse vehicle type data
    speed_d = parse_speed_avg(inpx_doc)
    print('* parsing vehicle distributions...')
    # get the vehicle distribution
    vehicle_comp_d = parse_veh_comp(inpx_doc)
    print('* parsing vehicle types...')
    # parse vehTypes and combine the information with acceleration and length data
    vehicle_type_d = parse_vehicle_types(inpx_doc, parse_max_acc(inpx_doc),
                                         parse_length(inpx_doc))
    print('OK.\n---')

    #
    # Flows and Routes ##########
    #
    # TODO: maybe make flows and routes conversion switchable by option ?
    print('* parsing vehicle in-flow definitions...')
    # parse flows
    flow_d = parse_flows(inpx_doc)
    print('* parsing vehicle routes...')
    # parse routes
    routes_by_start_d = parse_routes(inpx_doc, sumo_edge_ids, verbinder_flag)
    print('+ validating relative flows...')
    # validate relative flows
    validate_rel_flow(routes_by_start_d, flow_d)
    print('+ setting route branching probabilities...')
    # computes the probability for each route
    calc_route_probability(routes_by_start_d, flow_d)
    print('OK.\n---')

    #
    # XML generation ##########
    #
    print('* output routes generation...')
    # create dom document and define routes + flows
    result_doc = Document()
    routes_Elem = result_doc.createElement("routes")
    result_doc.appendChild(routes_Elem)

    create_vTypeDistribution_elems(vehicle_comp_d, vehicle_type_d, speed_d, routes_Elem)
    print('-' * 3)
    create_routeDistribution_elems(routes_by_start_d, routes_Elem)
    print('-' * 3)
    create_flow_elems(routes_by_start_d, flow_d, routes_Elem)
    print('OK.\n---')

    print('* wrinting output:')
    # write the data into a .rou.xml file
    out_Fn = args.output_file
    if not out_Fn.endswith('.xml'):
        out_Fn += '.xml'
    with open(out_Fn, "w") as ofh:
        result_doc.writexml(ofh, addindent='    ', newl='\n')
        ofh.close()
    print('. data written to:\n\t', out_Fn)
