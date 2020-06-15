/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    NLDetectorBuilder.cpp
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Clemens Honomichl
/// @author  Michael Behrisch
/// @author  Christian Roessel
/// @author  Jakob Erdmann
/// @date    Mon, 15 Apr 2002
///
// Builds detectors for microsim
/****************************************************************************/
#include <config.h>

#include <string>
#include <iostream>
#include <microsim/MSNet.h>
#include <microsim/MSLane.h>
#include <microsim/MSEdge.h>
#include <microsim/output/MSInductLoop.h>
#include <microsim/output/MSE2Collector.h>
// #include <microsim/output/MSMultiLaneE2Collector.h>
#include <microsim/output/MSVTypeProbe.h>
#include <microsim/output/MSRouteProbe.h>
#include <microsim/output/MSMeanData_Net.h>
#include <microsim/output/MSMeanData_Emissions.h>
#include <microsim/output/MSMeanData_Harmonoise.h>
#include <microsim/output/MSMeanData_Amitran.h>
#include <microsim/output/MSInstantInductLoop.h>
#include <microsim/MSGlobals.h>
#include <microsim/actions/Command_SaveTLCoupledDet.h>
#include <microsim/actions/Command_SaveTLCoupledLaneDet.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/StringUtils.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/StringUtils.h>
#include "NLDetectorBuilder.h"
#include <microsim/output/MSDetectorControl.h>

#include <mesosim/MEInductLoop.h>
#include <mesosim/MELoop.h>
#include <mesosim/MESegment.h>


// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * NLDetectorBuilder::E3DetectorDefinition-methods
 * ----------------------------------------------------------------------- */
NLDetectorBuilder::E3DetectorDefinition::E3DetectorDefinition(const std::string& id,
        const std::string& device, double haltingSpeedThreshold,
        SUMOTime haltingTimeThreshold, SUMOTime splInterval,
        const std::string& vTypes, bool openEntry) :
    myID(id), myDevice(device),
    myHaltingSpeedThreshold(haltingSpeedThreshold),
    myHaltingTimeThreshold(haltingTimeThreshold),
    mySampleInterval(splInterval),
    myVehicleTypes(vTypes),
    myOpenEntry(openEntry) {
}


NLDetectorBuilder::E3DetectorDefinition::~E3DetectorDefinition() {}


/* -------------------------------------------------------------------------
 * NLDetectorBuilder-methods
 * ----------------------------------------------------------------------- */
NLDetectorBuilder::NLDetectorBuilder(MSNet& net)
    : myNet(net), myE3Definition(nullptr) {}


NLDetectorBuilder::~NLDetectorBuilder() {
    delete myE3Definition;
}


void
NLDetectorBuilder::buildInductLoop(const std::string& id,
                                   const std::string& lane, double pos, SUMOTime splInterval,
                                   const std::string& device, bool friendlyPos,
                                   const std::string& vTypes) {
    checkSampleInterval(splInterval, SUMO_TAG_E1DETECTOR, id);
    // get and check the lane
    MSLane* clane = getLaneChecking(lane, SUMO_TAG_E1DETECTOR, id);
    // get and check the position
    pos = getPositionChecking(pos, clane, friendlyPos, id);
    // build the loop
    MSDetectorFileOutput* loop = createInductLoop(id, clane, pos, vTypes);
    // add the file output
    myNet.getDetectorControl().add(SUMO_TAG_INDUCTION_LOOP, loop, device, splInterval);
}


void
NLDetectorBuilder::buildInstantInductLoop(const std::string& id,
        const std::string& lane, double pos,
        const std::string& device, bool friendlyPos,
        const std::string& vTypes) {
    // get and check the lane
    MSLane* clane = getLaneChecking(lane, SUMO_TAG_INSTANT_INDUCTION_LOOP, id);
    // get and check the position
    pos = getPositionChecking(pos, clane, friendlyPos, id);
    // build the loop
    MSDetectorFileOutput* loop = createInstantInductLoop(id, clane, pos, device, vTypes);
    // add the file output
    myNet.getDetectorControl().add(SUMO_TAG_INSTANT_INDUCTION_LOOP, loop);
}


void
NLDetectorBuilder::buildE2Detector(const std::string& id, MSLane* lane, double pos, double endPos, double length,
                                   const std::string& device, SUMOTime frequency,
                                   SUMOTime haltingTimeThreshold, double haltingSpeedThreshold, double jamDistThreshold,
                                   const std::string& vTypes, bool friendlyPos, bool showDetector,
                                   MSTLLogicControl::TLSLogicVariants* tlls, MSLane* toLane) {

    bool tlsGiven = tlls != nullptr;
    bool toLaneGiven = toLane != nullptr;
    bool posGiven = pos != std::numeric_limits<double>::max();
    bool endPosGiven = endPos != std::numeric_limits<double>::max();

    assert(posGiven || endPosGiven);

    // Check positioning
    if (posGiven) {
        if (pos >= lane->getLength() || (pos < 0 && -pos > lane->getLength())) {
            std::stringstream ss;
            ss << "The given position (=" << pos << ") for detector '" << id
               << "' does not lie on the given lane '" << lane->getID()
               << "' with length " << lane->getLength();
            if (friendlyPos) {
                double newPos = pos > 0 ? lane->getLength() - POSITION_EPS : 0.;
                ss << " (adjusting to new position " << newPos;
                WRITE_WARNING(ss.str());
                pos = newPos;
            } else {
                ss << " (0 <= pos < lane->getLength() is required)";
                throw InvalidArgument(ss.str());
            }
        }
    }
    if (endPosGiven) {
        if (endPos > lane->getLength() || (endPos <= 0 && -endPos >= lane->getLength())) {
            std::stringstream ss;
            ss << "The given end position (=" << endPos << ") for detector '" << id
               << "' does not lie on the given lane '" << lane->getID()
               << "' with length " << lane->getLength();
            if (friendlyPos) {
                double newEndPos = endPos > 0 ? lane->getLength() : POSITION_EPS;
                ss << " (adjusting to new position " << newEndPos;
                WRITE_WARNING(ss.str());
                pos = newEndPos;
            } else {
                ss << " (0 <= pos < lane->getLength() is required)";
                throw InvalidArgument(ss.str());
            }
        }
    }

    MSE2Collector* det = nullptr;
    if (tlsGiven) {
        // Detector connected to TLS
        det =  createE2Detector(id, DU_USER_DEFINED, lane, pos, endPos, length, haltingTimeThreshold, haltingSpeedThreshold, jamDistThreshold, vTypes, showDetector);
        myNet.getDetectorControl().add(SUMO_TAG_LANE_AREA_DETECTOR, det);
        // add the file output (XXX: Where's the corresponding delete?)
        if (toLaneGiven) {
            // Detector also associated to specific link
            MSLane* lastLane = det->getLastLane();
            MSLink* link = MSLinkContHelper::getConnectingLink(*lastLane, *toLane);
            if (link == nullptr) {
                throw InvalidArgument(
                    "The detector '" + id + "' cannot be build as no connection between lanes '"
                    + lastLane->getID() + "' and '" + toLane->getID() + "' exists.");
            }
            new Command_SaveTLCoupledLaneDet(*tlls, det, myNet.getCurrentTimeStep(), OutputDevice::getDevice(device), link);
        } else {
            // detector for tls but without specific link
            new Command_SaveTLCoupledDet(*tlls, det, myNet.getCurrentTimeStep(), OutputDevice::getDevice(device));
        }
    } else {
        // User specified detector for xml-output
        checkSampleInterval(frequency, SUMO_TAG_E2DETECTOR, id);
        det =  createE2Detector(id, DU_USER_DEFINED, lane, pos, endPos, length, haltingTimeThreshold, haltingSpeedThreshold, jamDistThreshold, vTypes, showDetector);
        myNet.getDetectorControl().add(SUMO_TAG_LANE_AREA_DETECTOR, det, device, frequency);
    }

}

void
NLDetectorBuilder::buildE2Detector(const std::string& id, std::vector<MSLane*> lanes, double pos, double endPos,
                                   const std::string& device, SUMOTime frequency,
                                   SUMOTime haltingTimeThreshold, double haltingSpeedThreshold, double jamDistThreshold,
                                   const std::string& vTypes, bool friendlyPos, bool showDetector,
                                   MSTLLogicControl::TLSLogicVariants* tlls, MSLane* toLane) {

    bool tlsGiven = tlls != nullptr;
    bool toLaneGiven = toLane != nullptr;
    assert(pos != std::numeric_limits<double>::max());
    assert(endPos != std::numeric_limits<double>::max());
    assert(lanes.size() != 0);

    const MSLane* const firstLane = lanes[0];
    const MSLane* const lastLane = lanes.back();

    // Check positioning
    if (pos >= firstLane->getLength() || (pos < 0 && -pos > firstLane->getLength())) {
        std::stringstream ss;
        ss << "The given position (=" << pos << ") for detector '" << id
           << "' does not lie on the given lane '" << firstLane->getID()
           << "' with length " << firstLane->getLength();
        if (friendlyPos) {
            double newPos = pos > 0 ? firstLane->getLength() - POSITION_EPS : 0.;
            ss << " (adjusting to new position " << newPos;
            WRITE_WARNING(ss.str());
            pos = newPos;
        } else {
            ss << " (0 <= pos < lane->getLength() is required)";
            throw InvalidArgument(ss.str());
        }
    }
    if (endPos > lastLane->getLength() || (endPos <= 0 && -endPos >= lastLane->getLength())) {
        std::stringstream ss;
        ss << "The given end position (=" << endPos << ") for detector '" << id
           << "' does not lie on the given lane '" << lastLane->getID()
           << "' with length " << lastLane->getLength();
        if (friendlyPos) {
            double newEndPos = endPos > 0 ? lastLane->getLength() : POSITION_EPS;
            ss << " (adjusting to new position " << newEndPos;
            WRITE_WARNING(ss.str());
            pos = newEndPos;
        } else {
            ss << " (0 <= pos < lane->getLength() is required)";
            throw InvalidArgument(ss.str());
        }
    }

    MSE2Collector* det = nullptr;
    if (tlsGiven) {
        // Detector connected to TLS
        det = createE2Detector(id, DU_USER_DEFINED, lanes, pos, endPos, haltingTimeThreshold, haltingSpeedThreshold, jamDistThreshold, vTypes, showDetector);
        myNet.getDetectorControl().add(SUMO_TAG_LANE_AREA_DETECTOR, det);
        // add the file output (XXX: Where's the corresponding delete?)
        if (toLaneGiven) {
            // Detector also associated to specific link
            const MSLane* const lastDetLane = det->getLastLane();
            const MSLink* const link = MSLinkContHelper::getConnectingLink(*lastDetLane, *toLane);
            if (link == nullptr) {
                throw InvalidArgument(
                    "The detector '" + id + "' cannot be build as no connection between lanes '"
                    + lastDetLane->getID() + "' and '" + toLane->getID() + "' exists.");
            }
            new Command_SaveTLCoupledLaneDet(*tlls, det, myNet.getCurrentTimeStep(), OutputDevice::getDevice(device), link);
        } else {
            // detector for tls but without specific link
            new Command_SaveTLCoupledDet(*tlls, det, myNet.getCurrentTimeStep(), OutputDevice::getDevice(device));
        }
    } else {
        // User specified detector for xml-output
        checkSampleInterval(frequency, SUMO_TAG_E2DETECTOR, id);

        det = createE2Detector(id, DU_USER_DEFINED, lanes, pos, endPos, haltingTimeThreshold, haltingSpeedThreshold, jamDistThreshold, vTypes, showDetector);
        myNet.getDetectorControl().add(SUMO_TAG_LANE_AREA_DETECTOR, det, device, frequency);
    }

}



void
NLDetectorBuilder::beginE3Detector(const std::string& id,
                                   const std::string& device, SUMOTime splInterval,
                                   double haltingSpeedThreshold,
                                   SUMOTime haltingTimeThreshold,
                                   const std::string& vTypes, bool openEntry) {
    checkSampleInterval(splInterval, SUMO_TAG_E3DETECTOR, id);
    myE3Definition = new E3DetectorDefinition(id, device, haltingSpeedThreshold, haltingTimeThreshold, splInterval, vTypes, openEntry);
}


void
NLDetectorBuilder::addE3Entry(const std::string& lane,
                              double pos, bool friendlyPos) {
    if (myE3Definition == nullptr) {
        return;
    }
    MSLane* clane = getLaneChecking(lane, SUMO_TAG_E3DETECTOR, myE3Definition->myID);
    // get and check the position
    pos = getPositionChecking(pos, clane, friendlyPos, myE3Definition->myID);
    // build and save the entry
    myE3Definition->myEntries.push_back(MSCrossSection(clane, pos));
}


void
NLDetectorBuilder::addE3Exit(const std::string& lane,
                             double pos, bool friendlyPos) {
    if (myE3Definition == nullptr) {
        return;
    }
    MSLane* clane = getLaneChecking(lane, SUMO_TAG_E3DETECTOR, myE3Definition->myID);
    // get and check the position
    pos = getPositionChecking(pos, clane, friendlyPos, myE3Definition->myID);
    // build and save the exit
    myE3Definition->myExits.push_back(MSCrossSection(clane, pos));
}


std::string
NLDetectorBuilder::getCurrentE3ID() const {
    if (myE3Definition == nullptr) {
        return "<unknown>";
    }
    return myE3Definition->myID;
}


void
NLDetectorBuilder::endE3Detector() {
    if (myE3Definition == nullptr) {
        return;
    }
    // If E3 own entry or exit detectors
    if (myE3Definition->myEntries.size() > 0 || myE3Definition->myExits.size() > 0) {
        // create E3 detector
        MSDetectorFileOutput* det = createE3Detector(myE3Definition->myID,
                                    myE3Definition->myEntries, myE3Definition->myExits,
                                    myE3Definition->myHaltingSpeedThreshold, myE3Definition->myHaltingTimeThreshold, myE3Definition->myVehicleTypes,
                                    myE3Definition->myOpenEntry);
        // add to net
        myNet.getDetectorControl().add(SUMO_TAG_ENTRY_EXIT_DETECTOR, det, myE3Definition->myDevice, myE3Definition->mySampleInterval);
    } else
        WRITE_WARNING(toString(SUMO_TAG_E3DETECTOR) + " with id = '" + myE3Definition->myID + "' will not be created because is empty (no " + toString(SUMO_TAG_DET_ENTRY) + " or " + toString(SUMO_TAG_DET_EXIT) + " was defined)")

        // clean up
        delete myE3Definition;
    myE3Definition = nullptr;
}


void
NLDetectorBuilder::buildVTypeProbe(const std::string& id,
                                   const std::string& vtype, SUMOTime frequency,
                                   const std::string& device) {
    checkSampleInterval(frequency, SUMO_TAG_VTYPEPROBE, id);
    new MSVTypeProbe(id, vtype, OutputDevice::getDevice(device), frequency);
}


void
NLDetectorBuilder::buildRouteProbe(const std::string& id, const std::string& edge,
                                   SUMOTime frequency, SUMOTime begin,
                                   const std::string& device,
                                   const std::string& vTypes) {
    checkSampleInterval(frequency, SUMO_TAG_ROUTEPROBE, id);
    MSEdge* e = getEdgeChecking(edge, SUMO_TAG_ROUTEPROBE, id);
    MSRouteProbe* probe = new MSRouteProbe(id, e, id + "_" + toString(begin), id + "_" + toString(begin - frequency), vTypes);
    // add the file output
    myNet.getDetectorControl().add(SUMO_TAG_ROUTEPROBE, probe, device, frequency, begin);
}

MSDetectorFileOutput*
NLDetectorBuilder::createInductLoop(const std::string& id,
                                    MSLane* lane, double pos,
                                    const std::string& vTypes, bool) {
    if (MSGlobals::gUseMesoSim) {
        return new MEInductLoop(id, MSGlobals::gMesoNet->getSegmentForEdge(lane->getEdge(), pos), pos, vTypes);
    }
    return new MSInductLoop(id, lane, pos, vTypes, false);
}


MSDetectorFileOutput*
NLDetectorBuilder::createInstantInductLoop(const std::string& id,
        MSLane* lane, double pos, const std::string& od,
        const std::string& vTypes) {
    return new MSInstantInductLoop(id, OutputDevice::getDevice(od), lane, pos, vTypes);
}


MSE2Collector*
NLDetectorBuilder::createE2Detector(const std::string& id,
                                    DetectorUsage usage, MSLane* lane, double pos, double endPos, double length,
                                    SUMOTime haltingTimeThreshold, double haltingSpeedThreshold, double jamDistThreshold,
                                    const std::string& vTypes, bool /* showDetector */) {
    return new MSE2Collector(id, usage, lane, pos, endPos, length, haltingTimeThreshold, haltingSpeedThreshold, jamDistThreshold, vTypes);
}

MSE2Collector*
NLDetectorBuilder::createE2Detector(const std::string& id,
                                    DetectorUsage usage, std::vector<MSLane*> lanes, double pos, double endPos,
                                    SUMOTime haltingTimeThreshold, double haltingSpeedThreshold, double jamDistThreshold,
                                    const std::string& vTypes, bool /* showDetector */) {
    return new MSE2Collector(id, usage, lanes, pos, endPos, haltingTimeThreshold, haltingSpeedThreshold, jamDistThreshold, vTypes);
}

MSDetectorFileOutput*
NLDetectorBuilder::createE3Detector(const std::string& id,
                                    const CrossSectionVector& entries,
                                    const CrossSectionVector& exits,
                                    double haltingSpeedThreshold,
                                    SUMOTime haltingTimeThreshold,
                                    const std::string& vTypes,
                                    bool openEntry) {
    return new MSE3Collector(id, entries, exits, haltingSpeedThreshold, haltingTimeThreshold, vTypes, openEntry);
}


double
NLDetectorBuilder::getPositionChecking(double pos, MSLane* lane, bool friendlyPos,
                                       const std::string& detid) {
    // check whether it is given from the end
    if (pos < 0) {
        pos += lane->getLength();
    }
    // check whether it is on the lane
    if (pos > lane->getLength()) {
        if (friendlyPos) {
            pos = lane->getLength();
        } else {
            throw InvalidArgument("The position of detector '" + detid + "' lies beyond the lane's '" + lane->getID() + "' end.");
        }
    }
    if (pos < 0) {
        if (friendlyPos) {
            pos = 0.;
        } else {
            throw InvalidArgument("The position of detector '" + detid + "' lies before the lane's '" + lane->getID() + "' begin.");
        }
    }
    return pos;
}


void
NLDetectorBuilder::createEdgeLaneMeanData(const std::string& id, SUMOTime frequency,
        SUMOTime begin, SUMOTime end, const std::string& type,
        const bool useLanes, const bool withEmpty, const bool printDefaults,
        const bool withInternal, const bool trackVehicles, const int detectPersons,
        const double maxTravelTime, const double minSamples,
        const double haltSpeed, const std::string& vTypes,
        const std::string& writeAttributes,
        const std::string& device) {
    if (begin < 0) {
        throw InvalidArgument("Negative begin time for meandata dump '" + id + "'.");
    }
    if (end < 0) {
        end = SUMOTime_MAX;
    }
    if (end <= begin) {
        throw InvalidArgument("End before or at begin for meandata dump '" + id + "'.");
    }
    checkStepLengthMultiple(begin, " for meandata dump '" + id + "'");
    MSMeanData* det = nullptr;
    if (type == "" || type == "performance" || type == "traffic") {
        det = new MSMeanData_Net(id, begin, end, useLanes, withEmpty,
                                 printDefaults, withInternal, trackVehicles, detectPersons, maxTravelTime, minSamples, haltSpeed, vTypes, writeAttributes);
    } else if (type == "emissions" || type == "hbefa") {
        if (type == "hbefa") {
            WRITE_WARNING("The netstate type 'hbefa' is deprecated. Please use the type 'emissions' instead.");
        }
        det = new MSMeanData_Emissions(id, begin, end, useLanes, withEmpty,
                                       printDefaults, withInternal, trackVehicles, maxTravelTime, minSamples, vTypes, writeAttributes);
    } else if (type == "harmonoise") {
        det = new MSMeanData_Harmonoise(id, begin, end, useLanes, withEmpty,
                                        printDefaults, withInternal, trackVehicles, maxTravelTime, minSamples, vTypes, writeAttributes);
    } else if (type == "amitran") {
        det = new MSMeanData_Amitran(id, begin, end, useLanes, withEmpty,
                                     printDefaults, withInternal, trackVehicles, detectPersons, maxTravelTime, minSamples, haltSpeed, vTypes, writeAttributes);
    } else {
        throw InvalidArgument("Invalid type '" + type + "' for meandata dump '" + id + "'.");
    }
    if (det != nullptr) {
        if (frequency < 0) {
            frequency = end - begin;
        } else {
            checkStepLengthMultiple(frequency, " for meandata dump '" + id + "'");
        }
        MSNet::getInstance()->getDetectorControl().add(det, device, frequency, begin);
    }
}




// ------ Value checking/adapting methods ------
MSEdge*
NLDetectorBuilder::getEdgeChecking(const std::string& edgeID, SumoXMLTag type,
                                   const std::string& detid) {
    // get and check the lane
    MSEdge* edge = MSEdge::dictionary(edgeID);
    if (edge == nullptr) {
        throw InvalidArgument("The lane with the id '" + edgeID + "' is not known (while building " + toString(type) + " '" + detid + "').");
    }
    return edge;
}


MSLane*
NLDetectorBuilder::getLaneChecking(const std::string& laneID, SumoXMLTag type,
                                   const std::string& detid) {
    // get and check the lane
    MSLane* lane = MSLane::dictionary(laneID);
    if (lane == nullptr) {
        throw InvalidArgument("The lane with the id '" + laneID + "' is not known (while building " + toString(type) + " '" + detid + "').");
    }
    return lane;
}


void
NLDetectorBuilder::checkSampleInterval(SUMOTime splInterval, SumoXMLTag type, const std::string& id) {
    if (splInterval < 0) {
        throw InvalidArgument("Negative sampling frequency (in " + toString(type) + " '" + id + "').");
    }
    if (splInterval == 0) {
        throw InvalidArgument("Sampling frequency must not be zero (in " + toString(type) + " '" + id + "').");
    }
    checkStepLengthMultiple(splInterval, " (in " + toString(type) + " '" + id + "')");
}


/****************************************************************************/
