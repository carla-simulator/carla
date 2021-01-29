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
/// @file    NBPTStop.cpp
/// @author  Gregor Laemmel
/// @date    Tue, 20 Mar 2017
///
// The representation of a single pt stop
/****************************************************************************/
#include <config.h>

#include <utils/iodevices/OutputDevice.h>
#include <utils/common/StringUtils.h>
#include "NBPTStop.h"
#include "NBEdge.h"
#include "NBEdgeCont.h"


// ===========================================================================
// method definitions
// ===========================================================================
NBPTStop::NBPTStop(std::string ptStopId, Position position, std::string edgeId, std::string origEdgeId, double length,
                   std::string name, SVCPermissions svcPermissions, double parkingLength) :
    myPTStopId(ptStopId),
    myPosition(position),
    myEdgeId(edgeId),
    myOrigEdgeId(origEdgeId),
    myPTStopLength(length),
    myName(name),
    myParkingLength(parkingLength),
    myPermissions(svcPermissions),
    myBidiStop(nullptr),
    myIsLoose(origEdgeId == ""),
    myIsMultipleStopPositions(false) {
}

std::string
NBPTStop::getID() const {
    return myPTStopId;
}

const std::string
NBPTStop::getOrigEdgeId() const {
    return myOrigEdgeId;
}


const std::string
NBPTStop::getEdgeId() const {
    return myEdgeId;
}


const std::string
NBPTStop::getName() const {
    return myName;
}


const Position&
NBPTStop::getPosition() const {
    return myPosition;
}

void
NBPTStop::mirrorX() {
    myPosition.mul(1, -1);
}

void
NBPTStop::computeExtent(double center, double edgeLength) {
    myStartPos = MAX2(0.0, center - myPTStopLength / 2.);
    myEndPos = MIN2(center + myPTStopLength / 2., edgeLength);
}


void
NBPTStop::addLine(const std::string& line) {
    const std::string l = StringUtils::escapeXML(line);
    if (std::find(myLines.begin(), myLines.end(), l) == myLines.end()) {
        myLines.push_back(l);
    }
}


void
NBPTStop::write(OutputDevice& device) {
    device.openTag(SUMO_TAG_BUS_STOP);
    device.writeAttr(SUMO_ATTR_ID, myPTStopId);
    if (!myName.empty()) {
        device.writeAttr(SUMO_ATTR_NAME, StringUtils::escapeXML(myName));
    }
    device.writeAttr(SUMO_ATTR_LANE, myLaneId);
    device.writeAttr(SUMO_ATTR_STARTPOS, myStartPos);
    device.writeAttr(SUMO_ATTR_ENDPOS, myEndPos);
    device.writeAttr(SUMO_ATTR_FRIENDLY_POS, "true");
    if (myLines.size() > 0) {
        device.writeAttr(SUMO_ATTR_LINES, toString(myLines));
    }
    if (myParkingLength > 0) {
        device.writeAttr(SUMO_ATTR_PARKING_LENGTH, myParkingLength);
    }
    if (!myAccesses.empty()) {
        std::sort(myAccesses.begin(), myAccesses.end());
        for (auto tuple : myAccesses) {
            device.openTag(SUMO_TAG_ACCESS);
            device.writeAttr(SUMO_ATTR_LANE, std::get<0>(tuple));
            device.writeAttr(SUMO_ATTR_POSITION, std::get<1>(tuple));
            device.writeAttr(SUMO_ATTR_LENGTH, std::get<2>(tuple));
            device.writeAttr(SUMO_ATTR_FRIENDLY_POS, true);
            device.closeTag();
        }
    }
    device.closeTag();
}


void
NBPTStop::reshiftPosition(const double offsetX, const double offsetY) {
    myPosition.add(offsetX, offsetY, 0);
    for (NBPTPlatform& platformCand : myPlatformCands) {
        platformCand.reshiftPosition(offsetX, offsetY);
    }
}


SVCPermissions
NBPTStop::getPermissions() const {
    return myPermissions;
}


void
NBPTStop::addPlatformCand(NBPTPlatform platform) {
    myPlatformCands.push_back(platform);
}


const std::vector<NBPTPlatform>&
NBPTStop::getPlatformCands() {
    return myPlatformCands;
}


bool
NBPTStop::getIsMultipleStopPositions() const {
    return myIsMultipleStopPositions;
}


void
NBPTStop::setIsMultipleStopPositions(bool multipleStopPositions) {
    myIsMultipleStopPositions = multipleStopPositions;
}


double
NBPTStop::getLength() const {
    return myPTStopLength;
}


bool
NBPTStop::setEdgeId(std::string edgeId, const NBEdgeCont& ec) {
    myEdgeId = edgeId;
    return findLaneAndComputeBusStopExtent(ec);
}


void
NBPTStop::registerAdditionalEdge(std::string wayId, std::string edgeId) {
    myAdditionalEdgeCandidates[wayId] = edgeId;
}


const std::map<std::string, std::string>&
NBPTStop::getMyAdditionalEdgeCandidates() const {
    return myAdditionalEdgeCandidates;
}


void
NBPTStop::setMyOrigEdgeId(const std::string& myOrigEdgeId) {
    NBPTStop::myOrigEdgeId = myOrigEdgeId;
}


void
NBPTStop::setMyPTStopLength(double myPTStopLength) {
    NBPTStop::myPTStopLength = myPTStopLength;
}

bool
NBPTStop::findLaneAndComputeBusStopExtent(const NBEdgeCont& ec) {
    NBEdge* edge = ec.getByID(myEdgeId);
    return findLaneAndComputeBusStopExtent(edge);
}

bool
NBPTStop::findLaneAndComputeBusStopExtent(const NBEdge* edge) {
    if (edge != nullptr) {
        myEdgeId = edge->getID();
        int laneNr = -1;
        for (const auto& it : edge->getLanes()) {
            if ((it.permissions & getPermissions()) == getPermissions()) {
                ++laneNr;
                break;
            }
            laneNr++;
        }
        if (laneNr != -1) {
            myLaneId = edge->getLaneID(laneNr);
            const PositionVector& shape = edge->getLaneShape(laneNr);
            double offset = shape.nearest_offset_to_point2D(getPosition(), false);
            offset = offset * edge->getLoadedLength() / edge->getLength();
            computeExtent(offset, edge->getLoadedLength());
            return true;
        }
    }
    return myEdgeId == ""; // loose stop. Try later when processing lines
}


void
NBPTStop::setMyPTStopId(std::string id) {
    myPTStopId = id;
}

void
NBPTStop::clearAccess() {
    myAccesses.clear();
}

void
NBPTStop::addAccess(std::string laneID, double offset, double length) {
    const std::string newEdgeID = SUMOXMLDefinitions::getEdgeIDFromLane(laneID);
    // avoid duplicate access
    for (auto it = myAccesses.begin(); it != myAccesses.end();) {
        if (SUMOXMLDefinitions::getEdgeIDFromLane(std::get<0>(*it)) == newEdgeID) {
            it = myAccesses.erase(it);
        } else {
            it++;
        }
    }
    myAccesses.push_back(std::make_tuple(laneID, offset, length));
}


/****************************************************************************/
