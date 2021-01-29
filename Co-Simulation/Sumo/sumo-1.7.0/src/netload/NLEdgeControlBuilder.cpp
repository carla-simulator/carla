/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
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
/// @file    NLEdgeControlBuilder.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Leonhard Luecken
/// @date    Mon, 9 Jul 2001
///
// Interface for building edges
/****************************************************************************/
#include <config.h>

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <iterator>
#include <microsim/MSGlobals.h>
#include <microsim/MSLane.h>
#include <microsim/MSEdge.h>
#include <microsim/MSEdgeControl.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/UtilExceptions.h>
#include "NLBuilder.h"
#include "NLEdgeControlBuilder.h"
#include <utils/iodevices/OutputDevice.h>


// ===========================================================================
// method definitions
// ===========================================================================
NLEdgeControlBuilder::NLEdgeControlBuilder()
    : myCurrentNumericalLaneID(0), myCurrentNumericalEdgeID(0), myEdges(0), myCurrentLaneIndex(-1) {
    myActiveEdge = (MSEdge*) nullptr;
    myLaneStorage = new std::vector<MSLane*>();
}


NLEdgeControlBuilder::~NLEdgeControlBuilder() {
    delete myLaneStorage;
}


void
NLEdgeControlBuilder::beginEdgeParsing(
    const std::string& id, const SumoXMLEdgeFunc function,
    const std::string& streetName,
    const std::string& edgeType,
    int priority,
    const std::string& bidi,
    double distance) {
    // closeEdge might not have been called because the last edge had an error, so we clear the lane storage
    myLaneStorage->clear();
    myActiveEdge = buildEdge(id, function, streetName, edgeType, priority, distance);
    if (MSEdge::dictionary(id) != nullptr) {
        throw InvalidArgument("Another edge with the id '" + id + "' exists.");
    }
    myEdges.push_back(myActiveEdge);
    if (bidi != "") {
        myBidiEdges[myActiveEdge] = bidi;
    }
}


MSLane*
NLEdgeControlBuilder::addLane(const std::string& id,
                              double maxSpeed, double length,
                              const PositionVector& shape, double width,
                              SVCPermissions permissions, int index, bool isRampAccel,
                              const std::string& type) {
    MSLane* lane = new MSLane(id, maxSpeed, length, myActiveEdge, myCurrentNumericalLaneID++, shape, width, permissions, index, isRampAccel, type);
    myLaneStorage->push_back(lane);
    myCurrentLaneIndex = index;
    return lane;
}


void
NLEdgeControlBuilder::addStopOffsets(const std::map<SVCPermissions, double>& stopOffsets) {

    if (myCurrentLaneIndex == -1) {
        setDefaultStopOffsets(stopOffsets);
    } else {
        updateCurrentLaneStopOffsets(stopOffsets);
    }
}



std::string
NLEdgeControlBuilder::reportCurrentEdgeOrLane() const {
    std::stringstream ss;
    if (myCurrentLaneIndex != -1) {
        ss << "lane " << myCurrentLaneIndex << " of ";
    }
    ss << "edge '" << myActiveEdge->getID() << "'";
    return ss.str();
}


void
NLEdgeControlBuilder::updateCurrentLaneStopOffsets(const std::map<SVCPermissions, double>& stopOffsets) {
    assert(myLaneStorage->size() != 0);
    if (stopOffsets.size() == 0) {
        return;
    }
    if (myLaneStorage->back()->getStopOffsets().size() != 0) {
        std::stringstream ss;
        ss << "Duplicate stopOffset definition for lane " << myLaneStorage->back()->getIndex() << " on edge " << myActiveEdge->getID() << "!";
        WRITE_WARNING(ss.str())
    } else {
        myLaneStorage->back()->setStopOffsets(stopOffsets);
    }
}


void
NLEdgeControlBuilder::setDefaultStopOffsets(std::map<SVCPermissions, double> stopOffsets) {
    if (myCurrentDefaultStopOffsets.size() != 0) {
        std::stringstream ss;
        ss << "Duplicate stopOffset definition for edge " << myActiveEdge->getID() << ". Ignoring duplicate specification.";
        WRITE_WARNING(ss.str())
    } else {
        myCurrentDefaultStopOffsets = stopOffsets;
    }
}


void
NLEdgeControlBuilder::applyDefaultStopOffsetsToLanes() {
    assert(myActiveEdge != 0);
    if (myCurrentDefaultStopOffsets.size() == 0) {
        return;
    }
    for (MSLane* l : *myLaneStorage) {
        if (l->getStopOffsets().size() == 0) {
            l->setStopOffsets(myCurrentDefaultStopOffsets);
        }
    }
}


void
NLEdgeControlBuilder::addNeigh(const std::string id) {
    myLaneStorage->back()->addNeigh(id);
}


MSEdge*
NLEdgeControlBuilder::closeEdge() {
    applyDefaultStopOffsetsToLanes();
    std::vector<MSLane*>* lanes = new std::vector<MSLane*>();
    lanes->reserve(myLaneStorage->size());
    copy(myLaneStorage->begin(), myLaneStorage->end(), back_inserter(*lanes));
    myLaneStorage->clear();
    myActiveEdge->initialize(lanes);
    myCurrentDefaultStopOffsets.clear();
    return myActiveEdge;
}


void
NLEdgeControlBuilder::closeLane() {
    myCurrentLaneIndex = -1;
}


MSEdgeControl*
NLEdgeControlBuilder::build(double networkVersion) {
    for (MSEdgeVector::iterator i1 = myEdges.begin(); i1 != myEdges.end(); i1++) {
        (*i1)->closeBuilding();
    }
    for (MSEdgeVector::iterator i1 = myEdges.begin(); i1 != myEdges.end(); i1++) {
        (*i1)->buildLaneChanger();
    }
    // mark internal edges belonging to a roundabout (after all edges are build)
    if (MSGlobals::gUsingInternalLanes) {
        for (MSEdgeVector::iterator i1 = myEdges.begin(); i1 != myEdges.end(); i1++) {
            MSEdge* edge = *i1;
            if (edge->isInternal()) {
                if (edge->getNumSuccessors() != 1 || edge->getNumPredecessors() != 1) {
                    throw ProcessError("Internal edge '" + edge->getID() + "' is not properly connected (probably a manually modified net.xml).");
                }
                if (edge->getSuccessors()[0]->isRoundabout() || edge->getPredecessors()[0]->isRoundabout()) {
                    edge->markAsRoundabout();
                }
            }
        }
    }
    if (!deprecatedVehicleClassesSeen.empty()) {
        WRITE_WARNING("Deprecated vehicle classes '" + toString(deprecatedVehicleClassesSeen) + "' in input network.");
        deprecatedVehicleClassesSeen.clear();
    }
    // check for bi-directional edges (this are edges in opposing direction and superposable/congruent shapes)
    if (myBidiEdges.size() > 0 || networkVersion > 1.0) {
        for (auto& item : myBidiEdges) {
            item.first->checkAndRegisterBiDirEdge(item.second);
        }
        //WRITE_MESSAGE("Loaded " + toString(myBidiEdges.size()) + " bidirectional edges");
    } else {
        // legacy network
        for (MSEdge* e : myEdges) {
            e->checkAndRegisterBiDirEdge();
        }
    }
    return new MSEdgeControl(myEdges);
}


MSEdge*
NLEdgeControlBuilder::buildEdge(const std::string& id, const SumoXMLEdgeFunc function,
                                const std::string& streetName, const std::string& edgeType, const int priority, const double distance) {
    return new MSEdge(id, myCurrentNumericalEdgeID++, function, streetName, edgeType, priority, distance);
}

void NLEdgeControlBuilder::addCrossingEdges(const std::vector<std::string>& crossingEdges) {
    myActiveEdge->setCrossingEdges(crossingEdges);
}


/****************************************************************************/
