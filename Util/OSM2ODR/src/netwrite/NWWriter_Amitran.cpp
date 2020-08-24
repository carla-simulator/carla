/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    NWWriter_Amitran.cpp
/// @author  Michael Behrisch
/// @date    13.03.2014
///
// Exporter writing networks using the Amitran format
/****************************************************************************/
#include <config.h>

#include <utils/common/MsgHandler.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBEdgeCont.h>
#include <netbuild/NBNode.h>
#include <netbuild/NBNodeCont.h>
#include <netbuild/NBNetBuilder.h>
#include <utils/options/OptionsCont.h>
#include <utils/iodevices/OutputDevice.h>
#include "NWWriter_DlrNavteq.h"
#include "NWWriter_Amitran.h"



// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static methods
// ---------------------------------------------------------------------------
void
NWWriter_Amitran::writeNetwork(const OptionsCont& oc, NBNetBuilder& nb) {
    // check whether an amitran-file shall be generated
    if (!oc.isSet("amitran-output")) {
        return;
    }
    NBEdgeCont& ec = nb.getEdgeCont();
    OutputDevice& device = OutputDevice::getDevice(oc.getString("amitran-output"));
    device << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    device << "<network xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"http://sumo.dlr.de/xsd/amitran/network.xsd\">\n";
    // write nodes
    int index = 0;
    NBNodeCont& nc = nb.getNodeCont();
    std::set<NBNode*> singleRoundaboutNodes;
    std::set<NBNode*> multiRoundaboutNodes;
    const std::set<EdgeSet>& roundabouts = ec.getRoundabouts();
    for (std::set<EdgeSet>::const_iterator i = roundabouts.begin(); i != roundabouts.end(); ++i) {
        for (EdgeSet::const_iterator j = (*i).begin(); j != (*i).end(); ++j) {
            if ((*j)->getNumLanes() > 1) {
                multiRoundaboutNodes.insert((*j)->getFromNode());
            } else {
                singleRoundaboutNodes.insert((*j)->getFromNode());
            }
        }
    }
    std::map<NBNode*, int> nodeIds;
    for (std::map<std::string, NBNode*>::const_iterator i = nc.begin(); i != nc.end(); ++i) {
        device << "    <node id=\"" << index;
        nodeIds[i->second] = index++;
        if (singleRoundaboutNodes.count(i->second) > 0) {
            device << "\" type=\"roundaboutSingle\"/>\n";
            continue;
        }
        if (multiRoundaboutNodes.count(i->second) > 0) {
            device << "\" type=\"roundaboutMulti\"/>\n";
            continue;
        }
        switch (i->second->getType()) {
            case SumoXMLNodeType::TRAFFIC_LIGHT:
            case SumoXMLNodeType::TRAFFIC_LIGHT_NOJUNCTION:
            case SumoXMLNodeType::TRAFFIC_LIGHT_RIGHT_ON_RED:
                device << "\" type=\"trafficLight";
                break;
            case SumoXMLNodeType::PRIORITY:
                device << "\" type=\"priority";
                break;
            case SumoXMLNodeType::PRIORITY_STOP:
                device << "\" type=\"priorityStop";
                break;
            case SumoXMLNodeType::RIGHT_BEFORE_LEFT:
                device << "\" type=\"rightBeforeLeft";
                break;
            case SumoXMLNodeType::ALLWAY_STOP:
                device << "\" type=\"allwayStop";
                break;
            case SumoXMLNodeType::ZIPPER:
                device << "\" type=\"zipper";
                break;
            case SumoXMLNodeType::RAIL_SIGNAL:
                device << "\" type=\"railSignal";
                break;
            case SumoXMLNodeType::RAIL_CROSSING:
                device << "\" type=\"railCrossing";
                break;
            case SumoXMLNodeType::DEAD_END:
            case SumoXMLNodeType::DEAD_END_DEPRECATED:
                device << "\" type=\"deadEnd";
                break;
            case SumoXMLNodeType::DISTRICT:
            case SumoXMLNodeType::NOJUNCTION:
            case SumoXMLNodeType::INTERNAL:
            case SumoXMLNodeType::UNKNOWN:
                break;
        }
        device << "\"/>\n";
    }
    // write edges
    index = 0;
    for (std::map<std::string, NBEdge*>::const_iterator i = ec.begin(); i != ec.end(); ++i) {
        device << "    <link id=\"" << index++
               << "\" from=\"" << nodeIds[i->second->getFromNode()]
               << "\" to=\"" << nodeIds[i->second->getToNode()]
               << "\" roadClass=\"" << NWWriter_DlrNavteq::getRoadClass((*i).second)
               << "\" length=\"" << int(1000 * i->second->getLoadedLength())
               << "\" speedLimitKmh=\"" << int(3.6 * (*i).second->getSpeed() + 0.5)
               << "\" laneNr=\"" << (*i).second->getNumLanes()
               << "\"/>\n";
    }
    device << "</network>\n";
    device.close();
}


/****************************************************************************/
