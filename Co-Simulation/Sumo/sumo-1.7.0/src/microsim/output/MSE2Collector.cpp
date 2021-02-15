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
/// @file    MSE2Collector.cpp
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Robbin Blokpoel
/// @author  Jakob Erdmann
/// @author  Leonhard Luecken
/// @date    Mon Feb 03 2014 10:13 CET
///
// An areal detector covering a sequence of consecutive lanes
/****************************************************************************/


/* TODO:
 * tests:
 *  - subsecond variant, ballistic variant
 * allow omitting jam processing (?)
 *
 * Meso-compatibility? (esp. enteredLane-argument for MSBaseVehicle::notifyEnter() is not treated)
 * Compatibility without internal lanes?
 * Include leftVehicles into output?
 */
#include <config.h>

#include <cassert>
#include <algorithm>
#ifdef HAVE_FOX
#include <utils/foxtools/FXConditionalLock.h>
#endif
#include <microsim/MSLane.h>
#include <microsim/MSNet.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSVehicleType.h>
#include "MSE2Collector.h"

//#define DEBUG_E2_CONSTRUCTOR
//#define DEBUG_E2_NOTIFY_ENTER_AND_LEAVE
//#define DEBUG_E2_NOTIFY_MOVE
//#define DEBUG_E2_MAKE_VEHINFO
//#define DEBUG_E2_DETECTOR_UPDATE
//#define DEBUG_E2_TIME_ON_DETECTOR
//#define DEBUG_E2_JAMS
//#define DEBUG_E2_XML_OUT
//#define DEBUG_COND (true)
//#define DEBUG_COND (getID()=="e2Detector_e5.601A_1_SAa")
//#define DEBUG_COND (getID()=="702057")
//#define DEBUG_COND (getID()=="det0")

MSE2Collector::MSE2Collector(const std::string& id,
                             DetectorUsage usage, MSLane* lane, double startPos, double endPos, double length,
                             SUMOTime haltingTimeThreshold, double haltingSpeedThreshold, double jamDistThreshold,
                             const std::string& vTypes) :
    MSMoveReminder(id, lane, false),
    MSDetectorFileOutput(id, vTypes),
    myUsage(usage),
    myJamHaltingSpeedThreshold(haltingSpeedThreshold),
    myJamHaltingTimeThreshold(haltingTimeThreshold),
    myJamDistanceThreshold(jamDistThreshold),
    myNumberOfEnteredVehicles(0),
    myNumberOfSeenVehicles(0),
    myNumberOfLeftVehicles(0) {
    reset();

#ifdef DEBUG_E2_CONSTRUCTOR
    if (DEBUG_COND) {
        std::cout << "\n" << "Creating MSE2Collector " << id
                  << " with lane = " << lane->getID()
                  << " startPos = " << startPos
                  << " endPos = " << endPos
                  << " length = " << length
                  << " haltingTimeThreshold = " << haltingTimeThreshold
                  << " haltingSpeedThreshold = " << haltingSpeedThreshold
                  << " jamDistThreshold = " << jamDistThreshold
                  << std::endl;
    }
#endif

    assert(lane != 0);

    // check that exactly one of length, startPos, endPos is invalid
    bool lengthInvalid = length == std::numeric_limits<double>::max() || length <= 0;
    bool endPosInvalid = endPos == std::numeric_limits<double>::max();
    bool posInvalid = startPos == std::numeric_limits<double>::max();

    // check and normalize positions (assure positive values for pos and endPos, snap to lane-ends)
    if (lengthInvalid) {
        // assume that the detector is only located on a single lane
        if (posInvalid) {
            WRITE_WARNING("No valid detector length and start position given. Assuming startPos = 0 and length = end position");
            startPos = 0;
        }
        if (endPosInvalid) {
            WRITE_WARNING("No valid detector length and end position given. Assuming endPos = lane length and length = endPos-startPos");
            endPos = lane->getLength();
        }
        endPos = endPos < 0 ? lane->getLength() + endPos : endPos;
        startPos = startPos < 0 ? lane->getLength() + startPos : startPos;
        bool valid = endPos <= lane->getLength() && 0 <= startPos && startPos < endPos;
        if (!valid) {
            throw InvalidArgument("Error in specification for E2Detector '" + id + "'. Positional argument is malformed. 0 <= pos < endPos <= lane.getLength() is required.");
        }
        // snap detector ends to lane ends
        endPos = snap(endPos, lane->getLength(), POSITION_EPS);
        startPos = snap(startPos, 0., POSITION_EPS);
        length = endPos - startPos;
    } else if (posInvalid) {
        // endPosInvalid == false
        endPos = endPos < 0 ? lane->getLength() + endPos : endPos;
        endPos = snap(endPos, lane->getLength(), POSITION_EPS);
    } else {
        // posInvalid == false
        startPos = startPos < 0 ? lane->getLength() + startPos : startPos;
        startPos = snap(startPos, 0., POSITION_EPS);
    }

    myStartPos = startPos;
    myEndPos = endPos;

    std::vector<MSLane*> lanes;
    if (posInvalid) {
        lanes = selectLanes(lane, length, "bw");
    } else if (endPosInvalid) {
        lanes = selectLanes(lane, length, "fw");
    } else {
        // assuming detector is only located at a single lane
        lanes.push_back(lane);
    }

    initAuxiliaries(lanes);
    checkPositioning(endPosInvalid, length);
    addDetectorToLanes(lanes);
}


MSE2Collector::MSE2Collector(const std::string& id,
                             DetectorUsage usage, std::vector<MSLane*> lanes, double startPos, double endPos,
                             SUMOTime haltingTimeThreshold, double haltingSpeedThreshold, double jamDistThreshold,
                             const std::string& vTypes) :
    MSMoveReminder(id, lanes[lanes.size() - 1], false), // assure that lanes.size() > 0 at caller side!!!
    MSDetectorFileOutput(id, vTypes),
    myUsage(usage),
    myFirstLane(lanes[0]),
    myLastLane(lanes[lanes.size() - 1]),
    myStartPos(startPos),
    myEndPos(endPos),
    myJamHaltingSpeedThreshold(haltingSpeedThreshold),
    myJamHaltingTimeThreshold(haltingTimeThreshold),
    myJamDistanceThreshold(jamDistThreshold),
    myNumberOfEnteredVehicles(0),
    myNumberOfSeenVehicles(0),
    myNumberOfLeftVehicles(0) {
    reset();

    for (std::vector<MSLane*>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
        assert((*i) != 0);
    }

#ifdef DEBUG_E2_CONSTRUCTOR
    if (DEBUG_COND) {
        std::cout << "\n" << "Creating MSE2Collector " << id
                  << " with endLane = " << myLastLane->getID()
                  << " endPos = " << endPos
                  << " startLane = " << myFirstLane->getID()
                  << " startPos = " << startPos
                  << " haltingTimeThreshold = " << haltingTimeThreshold
                  << " haltingSpeedThreshold = " << haltingSpeedThreshold
                  << " jamDistThreshold = " << jamDistThreshold
                  << std::endl;
    }
#endif

    myStartPos = myStartPos < 0 ? lanes[0]->getLength() + myStartPos : myStartPos;
    myEndPos = myEndPos < 0 ? lanes[lanes.size() - 1]->getLength() + myEndPos : myEndPos;

    if (myStartPos < POSITION_EPS) {
        myStartPos = 0;
    }
    if (myEndPos > lanes[lanes.size() - 1]->getLength() - POSITION_EPS) {
        myEndPos = lanes[lanes.size() - 1]->getLength();
    }


    initAuxiliaries(lanes);
    checkPositioning();
    addDetectorToLanes(lanes);
}


void
MSE2Collector::checkPositioning(bool posGiven, double desiredLength) {
    // check if detector was truncated
    if (desiredLength > 0 && myDetectorLength < desiredLength - NUMERICAL_EPS) {
        std::stringstream ss;
        ss << "Cannot build detector of length " << desiredLength
           << " because no further continuation lane was found for lane '" << (posGiven ? myLastLane->getID() : myFirstLane->getID())
           << "'! Truncated detector at length " << myDetectorLength << ".";
        WRITE_WARNING(ss.str());
    }

    if (myDetectorLength < POSITION_EPS && (myStartPos > 0. || myEndPos < myLastLane->getLength())) {
        // assure minimal detector length
        double prolong = POSITION_EPS - myDetectorLength;
        double startPos = MAX2(0., myStartPos - prolong); // new startPos
        prolong -= myStartPos - startPos;
        myStartPos = startPos;
        if (prolong > 0.) {
            myEndPos = MIN2(myEndPos + prolong, myLastLane->getLength());
        }
        WRITE_WARNING("Adjusted detector positioning to meet requirement length >= " + toString(POSITION_EPS)
                      + ". New position is [" + toString(myStartPos) + "," + toString(myEndPos) + "]");
    }

    // do some regularization snapping...
    myStartPos = snap(myStartPos, 0., POSITION_EPS);
    myStartPos = snap(myStartPos, myFirstLane->getLength() - POSITION_EPS, POSITION_EPS);
    myStartPos = snap(myStartPos, 0., POSITION_EPS);
    myEndPos = snap(myEndPos, myFirstLane->getLength(), POSITION_EPS);
    myEndPos = snap(myEndPos, POSITION_EPS, POSITION_EPS);
    myEndPos = snap(myEndPos, myFirstLane->getLength(), POSITION_EPS);
    recalculateDetectorLength();

#ifdef DEBUG_E2_CONSTRUCTOR
    if (DEBUG_COND) {
        std::stringstream ss;
        //    ss << std::setprecision(32) << myEndPos << " : " << POSITION_EPS;
        //    std::cout << ss.str() << std::endl;
        std::cout << "myStartPos = " << myStartPos << std::endl;
        std::cout << "myEndPos = " << myEndPos << std::endl;
        std::cout << "myLastLane->getLength() = " << myLastLane->getLength() << std::endl;
    }
#endif


    assert((myStartPos >= POSITION_EPS  || myStartPos == 0) && myStartPos < myFirstLane->getLength());
    assert(myEndPos <= myLastLane->getLength() - POSITION_EPS || myEndPos == myLastLane->getLength());
    assert(myFirstLane != myLastLane || myEndPos - myStartPos > 0);
}


double
MSE2Collector::snap(double value, double snapPoint, double snapDist) {
    if (fabs(value - snapPoint) < snapDist) {
        return snapPoint;
    } else {
        return value;
    }
}


void
MSE2Collector::recalculateDetectorLength() {
    std::vector<std::string>::const_iterator i;
    std::vector<MSLane*> lanes;
    // get real lanes
    for (i = myLanes.begin(); i != myLanes.end(); ++i) {
        MSLane* lane = MSLane::dictionary(*i);
        lanes.push_back(lane);
    }

    // sum up their lengths
    std::vector<MSLane*>::const_iterator j;
    MSLane* previous = nullptr;
    myDetectorLength = 0;
    for (j = lanes.begin(); j != lanes.end(); ++j) {
        // lane length
        myDetectorLength += (*j)->getLength();
        if (previous != nullptr && !MSGlobals::gUsingInternalLanes) {
            // eventually link length
            myDetectorLength += previous->getLinkTo(*j)->getLength();
        }
        previous = *j;
    }
    // substract uncovered area on first and last lane
    myDetectorLength -= myStartPos;
    myDetectorLength -= myLastLane->getLength() - myEndPos;

#ifdef DEBUG_E2_CONSTRUCTOR
    if (DEBUG_COND) {
        std::cout << "Total detector length after recalculation = " << myDetectorLength << std::endl;
    }
#endif
}


MSE2Collector::~MSE2Collector() {
    // clear move notifications
    clearState();
}


std::vector<MSLane*>
MSE2Collector::selectLanes(MSLane* lane, double length, std::string dir) {
    // direction of detector extension
    assert(dir == "fw" || dir == "bw");
    bool fw = dir == "fw";
    double linkLength = 0; // linkLength (used if no internal lanes are present)
    bool substractedLinkLength = false; // whether linkLength was substracted during the last iteration.

#ifdef DEBUG_E2_CONSTRUCTOR
    if (DEBUG_COND) {
        std::cout << "\n" << "selectLanes()" << (fw ? "(forward)" : "(backward)") << std::endl;
    }
#endif
    std::vector<MSLane*> lanes;
    // Selected lanes are stacked into vector 'lanes'. If dir == "bw" lanes will be reversed after this is done.
    // The length is reduced while adding lanes to the detector
    // First we adjust the starting value for length (in the first iteration, the whole length of the first considered lane is substracted,
    // while it might only be partially covered by the detector)
    if (fw) {
        assert(myStartPos != std::numeric_limits<double>::max());
        length += myStartPos;
    } else {
        assert(myEndPos != std::numeric_limits<double>::max());
        length += lane->getLength() - myEndPos;
    }
    length = MAX2(POSITION_EPS, length); // this assures to add at least one lane to lanes
    while (length >= POSITION_EPS && lane != nullptr) {
        // Break loop for length <= NUMERICAL_EPS to avoid placement of very small
        // detector piece on the end or beginning of one lane due to numerical rounding errors.
        lanes.push_back(lane);
#ifdef DEBUG_E2_CONSTRUCTOR
        if (DEBUG_COND) {
            std::cout << "Added lane " << lane->getID()
                      << " (length: " << lane->getLength() << ")" << std::endl;
        }
#endif

        length -= lane->getLength();

        // proceed to upstream predecessor
        if (fw) {
            lane = lane->getCanonicalSuccessorLane();
        } else {
            lane = lane->getCanonicalPredecessorLane();
        }


        substractedLinkLength = false;
        if (lane != nullptr && !MSGlobals::gUsingInternalLanes && length > POSITION_EPS) {
            // In case wher no internal lanes are used,
            // take into account the link length for the detector range
            linkLength = 0;
            if (fw) {
                linkLength = lanes.back()->getLinkTo(lane)->getLength();
            } else {
                linkLength = lane->getLinkTo(lanes.back())->getLength();
            }
            length -= linkLength;
            substractedLinkLength = true;
        }


#ifdef DEBUG_E2_CONSTRUCTOR
        if (DEBUG_COND) {
            if (lane != 0) {
                std::cout << (fw ? "Successor lane: " : "Predecessor lane: ") << "'" << lane->getID() << "'";
            }
            std::cout << std::endl;
        }
#endif
    }

    if (substractedLinkLength) {
        // if the link's length was substracted during the last step,
        // the start/endPos would lie on a non-existing internal lane,
        // therefore revert and truncate detector part on the non-existing internal lane.
        length += linkLength;
    }


    // 1) At this point a negative <length> means that not the whole last stored lane lanes[lanes.size()-1]
    // should be added to the detector, but the detector should spare out a part with length = -<length>
    // If this part is too small (of length < POSITION_EPS) we take the whole lane
    // 2) The whole lane is also taken for the case that <length> is positive. This corresponds to on
    // of three situations: i) <length> < POSITION_EPS (break condition -> don't take too small pieces on the next lane)
    //                 ii&iii) <length> >= POS_EPSILON may arise either if no continuation lane was found (lane==0), or
    //                         in case of not using internal lanes if the detector end/start falls on a link.
    // In all cases we take the whole last lane.
    if (fw) {
        if (length > -POSITION_EPS) {
            myEndPos = lanes[lanes.size() - 1]->getLength();
        } else if (length < 0) {
            myEndPos = lanes[lanes.size() - 1]->getLength() + length;
        }
    } else {
        if (length > -POSITION_EPS) {
            myStartPos = 0;
        } else if (length < 0) {
            myStartPos = -length;
        }
    }

    // reverse lanes if lane selection was backwards
    if (!fw) {
        std::reverse(lanes.begin(), lanes.end());
    }

    return lanes;
}

void
MSE2Collector::addDetectorToLanes(std::vector<MSLane*>& lanes) {
#ifdef DEBUG_E2_CONSTRUCTOR
    if (DEBUG_COND) {
        std::cout << "\n" << "Adding detector " << myID << " to lanes:" << std::endl;
    }
#endif
    for (std::vector<MSLane*>::iterator l = lanes.begin(); l != lanes.end(); ++l) {
        (*l)->addMoveReminder(this);
#ifdef DEBUG_E2_CONSTRUCTOR
        if (DEBUG_COND) {
            std::cout << (*l)->getID() << std::endl;
        }
#endif
    }
}

void
MSE2Collector::initAuxiliaries(std::vector<MSLane*>& lanes) {
    // Checks integrity of myLanes, adds internal-lane information, inits myLength, myFirstLane, myLastLane, myOffsets, myEndPos/myStartPos
    myFirstLane = lanes[0];
    myLastLane = lanes[lanes.size() - 1];

#ifdef DEBUG_E2_CONSTRUCTOR
    if (DEBUG_COND) {
        std::cout << "\n" << "Initializing auxiliaries:"
                  << "\nFirst lane: " << myFirstLane->getID() << " (startPos = " << myStartPos << ")"
                  << "\nLast lane: " << myLastLane->getID() << " (endPos = " << myEndPos << ")"
                  << std::endl;
    }
#endif

    // Init myOffsets and myDetectorLength.
    // The loop below runs through the given lanes assuming the possibility that only non-internal lanes are given
    // or at least not all relevant internal lanes are considered. During this a new, complete list of lane ids is
    // built into myLanes.
    myLanes.clear();

    // myDetectorLength will be increased in the loop below, always giving
    // the offset of the currently considered lane to the detector start
    myDetectorLength = -myStartPos;
    myOffsets.clear();

    // loop over detector lanes and accumulate offsets with respect to the first lane's begin
    // (these will be corrected afterwards by substracting the start position.)
    std::vector<MSLane*>::iterator il = lanes.begin();

    // start on an internal lane?
    // (This may happen if specifying the detector by its upstream
    //  length starting from a given end position)
    const MSLane* internal = (*il)->isInternal() ? *il : nullptr;

#ifdef DEBUG_E2_CONSTRUCTOR
    if (DEBUG_COND) {
        std::cout << "\n" << "Initializing offsets:" << std::endl;
    }
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4127) // do not warn about constant conditional expression
#endif
    while (true) {
#ifdef _MSC_VER
#pragma warning(pop)
#endif
        // Consider the next internal lanes
        while (internal != nullptr) {
            myLanes.push_back(internal->getID());
            myOffsets.push_back(myDetectorLength);

#ifdef DEBUG_E2_CONSTRUCTOR
            if (DEBUG_COND) {
                std::cout << "Offset for lane " << internal->getID() << " = " << myDetectorLength
                          << std::endl;
            }
#endif

            myDetectorLength += internal->getLength();
            if (internal->getID() == myLastLane->getID()) {
                break;
            }

            // There should be a unique continuation for each internal lane
            assert(internal->getLinkCont().size() == 1);

            internal = internal->getLinkCont()[0]->getViaLaneOrLane();
            if (!internal->isInternal()) {
                // passed the junction
                internal = nullptr;
                break;
            }
        }

        // Consider the next non-internal lane
        // This is the first lane in the first iteration, if it is non-internal
        // However, it can equal myLanes.end() if the myLastLane is internal. In that case we break.

        // Move il to next non-internal
        while (il != lanes.end() && (*il)->isInternal()) {
            il++;
        }
        if (il == lanes.end()) {
            break;
        }

        // There is still a non-internal lane to consider
        MSLane* lane = *il;
        myLanes.push_back(lane->getID());

#ifdef DEBUG_E2_CONSTRUCTOR
        if (DEBUG_COND) {
            std::cout << "Offset for lane " << lane->getID() << " = " << myDetectorLength
                      << std::endl;
        }
#endif

        // Offset to detector start for this lane
        myOffsets.push_back(myDetectorLength);

        // Add the lanes length to the detector offset
        myDetectorLength += lane->getLength();

        // Get the next lane if this lane isn't the last one
        if (++il == lanes.end()) {
            break;
        }

        if ((*il)->isInternal()) {
            // next lane in myLanes is internal
            internal = *il;
            continue;
        }

        // find the connection to next
        const MSLink* link = lane->getLinkTo(*il);
        if (link == nullptr) {
            throw InvalidArgument("Lanes '" + lane->getID() + "' and '" + (*il)->getID() + "' are not consecutive in defintion of e2Detector '" + getID() + "'");
        }

        if (!MSGlobals::gUsingInternalLanes) {
            myDetectorLength += link->getLength();
        } else {
            internal = link->getViaLane();
        }
    }

    // Substract distance not covered on the last considered lane
    bool fw = myEndPos == std::numeric_limits<double>::max();
    if (fw) {
        myDetectorLength -= myStartPos;
    } else {
        myDetectorLength -= myLastLane->getLength() - myEndPos;
    }

#ifdef DEBUG_E2_CONSTRUCTOR
    if (DEBUG_COND) {
        std::cout << "Total detector length after initAuxiliaries() = " << myDetectorLength << std::endl;
    }
#endif

// make lanes a complete list including internal lanes
    lanes = getLanes();
}


std::vector<MSLane*>
MSE2Collector::getLanes() {
    std::vector<MSLane*> res;
    for (std::vector<std::string>::const_iterator i = myLanes.begin(); i != myLanes.end(); ++i) {
        res.push_back(MSLane::dictionary(*i));
    }
    return res;
}


bool
MSE2Collector::notifyMove(SUMOTrafficObject& tObject, double oldPos,
                          double newPos, double newSpeed) {
    if (!tObject.isVehicle()) {
        return false;
    }
    SUMOVehicle& veh = static_cast<SUMOVehicle&>(tObject);
#ifdef HAVE_FOX
    FXConditionalLock lock(myNotificationMutex, MSGlobals::gNumSimThreads > 1);
#endif
    VehicleInfoMap::iterator vi = myVehicleInfos.find(veh.getID());
    assert(vi != myVehicleInfos.end()); // all vehicles calling notifyMove() should have called notifyEnter() before

    const std::string& vehID = veh.getID();
    VehicleInfo& vehInfo = *(vi->second);

    // position relative to the detector start
    double relPos = vehInfo.entryOffset + newPos;

    // update current distance to the detector end
    vehInfo.distToDetectorEnd = myDetectorLength - relPos;

#ifdef DEBUG_E2_NOTIFY_MOVE
    if (DEBUG_COND) {
        std::cout << "\n" << SIMTIME
                  << " MSE2Collector::notifyMove() (detID = " << myID << "on lane '" << myLane->getID() << "')"
                  << " called by vehicle '" << vehID << "'"
                  << " at relative position " << relPos
                  << ", distToDetectorEnd = " << vehInfo.distToDetectorEnd << std::endl;
    }
#endif

    // Check whether vehicle has reached the detector begin
    if (relPos <= 0) {
        // detector not yet reached, request being informed further
#ifdef DEBUG_E2_NOTIFY_MOVE
        if (DEBUG_COND) {
            std::cout << "Vehicle has not yet reached the detector start position." << std::endl;
        }
#endif
        return true;
    } else if (!vehInfo.hasEntered) {
        vehInfo.hasEntered = true;
        myNumberOfEnteredVehicles++;
        myNumberOfSeenVehicles++;
    }


    // determine whether vehicle has moved beyond the detector's end
    bool vehPassedDetectorEnd = - vehInfo.exitOffset <= newPos - veh.getVehicleType().getLength();

    // determine whether vehicle has been on the detector at all
    bool vehicleEnteredLaneAfterDetector = vehPassedDetectorEnd && (-vehInfo.exitOffset <= oldPos - veh.getVehicleType().getLength());
    // ... if not, dont create any notification at all
    if (vehicleEnteredLaneAfterDetector) {
#ifdef DEBUG_E2_NOTIFY_MOVE
        if (DEBUG_COND) {
            std::cout << "Vehicle entered lane behind detector." << std::endl;
        }
#endif
    } else {
        myMoveNotifications.push_back(makeMoveNotification(veh, oldPos, newPos, newSpeed, vehInfo));
    }


    if (vehPassedDetectorEnd) {
#ifdef DEBUG_E2_NOTIFY_MOVE
        if (DEBUG_COND) {
            std::cout << "Vehicle has left the detector longitudinally." << std::endl;
        }
#endif
        // Vehicle is beyond the detector, unsubscribe and register removal from myVehicleInfos
        myLeftVehicles.insert(vehID);
        return false;
    } else {
        // Receive further notifications
        return true;
    }
}

bool
MSE2Collector::notifyLeave(SUMOTrafficObject& tObject, double /* lastPos */, MSMoveReminder::Notification reason, const MSLane* enteredLane) {
    if (!tObject.isVehicle()) {
        return false;
    }
    SUMOVehicle& veh = static_cast<SUMOVehicle&>(tObject);
#ifdef DEBUG_E2_NOTIFY_ENTER_AND_LEAVE
    if (DEBUG_COND) {
        std::cout << "\n" << SIMTIME << " notifyLeave() (detID = " << myID << "on lane '" << myLane->getID() << "')"
                  << "called by vehicle '" << veh.getID() << "'" << std::endl;
    }
#endif

#ifdef HAVE_FOX
    FXConditionalLock lock(myNotificationMutex, MSGlobals::gNumSimThreads > 1);
#endif
    if (reason == MSMoveReminder::NOTIFICATION_JUNCTION) {
        // vehicle left lane via junction, unsubscription and registering in myLeftVehicles when
        // moving beyond the detector end is controlled in notifyMove.
#ifdef DEBUG_E2_NOTIFY_ENTER_AND_LEAVE
        if (DEBUG_COND) {
            std::cout << SIMTIME << " Left longitudinally (along junction) -> keep subscription [handle exit in notifyMove()]" << std::endl;
        }
#endif

        if (std::find(myLanes.begin(), myLanes.end(), enteredLane->getID()) == myLanes.end()) {
            // Entered lane is not part of the detector
            VehicleInfoMap::iterator vi = myVehicleInfos.find(veh.getID());
            // Determine exit offset, where vehicle left the detector
            double exitOffset = vi->second->entryOffset - myOffsets[vi->second->currentOffsetIndex] - vi->second->currentLane->getLength();
            vi->second->exitOffset = MAX2(vi->second->exitOffset, exitOffset);
#ifdef DEBUG_E2_NOTIFY_ENTER_AND_LEAVE
            if (DEBUG_COND) {
                std::cout << SIMTIME << " Vehicle '" << veh.getID() << "' leaves the detector. Exit offset = " << vi->second->exitOffset << std::endl;
            }
#endif
        }

        return true;
    } else {
        VehicleInfoMap::iterator vi = myVehicleInfos.find(veh.getID());
        // erase vehicle, which leaves in a non-longitudinal way, immediately
        if (vi->second->hasEntered) {
            myNumberOfLeftVehicles++;
        }
        delete vi->second;
        myVehicleInfos.erase(vi);
#ifdef DEBUG_E2_NOTIFY_ENTER_AND_LEAVE
        if (DEBUG_COND) {
            std::cout << SIMTIME << " Left non-longitudinally (lanechange, teleport, parking, etc) -> discard subscription" << std::endl;
        }
#endif
        return false;
    }
}


bool
MSE2Collector::notifyEnter(SUMOTrafficObject& tObject, MSMoveReminder::Notification reason, const MSLane* enteredLane) {
    if (!tObject.isVehicle()) {
        return false;
    }
    SUMOVehicle& veh = static_cast<SUMOVehicle&>(tObject);
#ifdef DEBUG_E2_NOTIFY_ENTER_AND_LEAVE
    if (DEBUG_COND) {
        std::cout << std::endl << SIMTIME << " notifyEnter() (detID = " << myID << " on lane '" << myLane->getID() << "')"
                  << " called by vehicle '" << veh.getID()
                  << "' entering lane '" << (enteredLane != 0 ? enteredLane->getID() : "NULL") << "'" << std::endl;
    }
#endif
    // notifyEnter() should only be called for lanes of the detector
    assert(std::find(myLanes.begin(), myLanes.end(), enteredLane->getID()) != myLanes.end());
    assert(veh.getLane() == enteredLane);

    if (!vehicleApplies(veh)) {
        // That's not my type...
        return false;
    }

    // determine whether the vehicle entered the lane behind the detector end
    // e.g. due to lane change manoeuver
    if (reason != NOTIFICATION_JUNCTION) {
        const double vehBackPos = veh.getBackPositionOnLane(enteredLane);
        bool vehEnteredBehindDetectorEnd = (enteredLane == myLastLane) && myEndPos <= vehBackPos;
        if (vehEnteredBehindDetectorEnd) {
            // this vehicle cannot influence detector readings, do not subscribe
            // to move notifications
#ifdef DEBUG_E2_NOTIFY_ENTER_AND_LEAVE
            if (DEBUG_COND) {
                std::cout << "Vehicle entered the lane behind the detector, ignoring it." << std::endl;
                std::cout << "(myEndPos = " << this->myEndPos << ", veh.getBackPositionOnLane() = " << vehBackPos << ")" << std::endl;
            }
#endif
            return false;
        }
    }

#ifdef DEBUG_E2_NOTIFY_ENTER_AND_LEAVE
    if (DEBUG_COND) {
        if (!veh.isOnRoad()) {
            // Vehicle is teleporting over the edge
            std::cout << "Vehicle is off road (teleporting over edge)..." << std::endl;
        }
    }
#endif

#ifdef HAVE_FOX
    FXConditionalLock lock(myNotificationMutex, MSGlobals::gNumSimThreads > 1);
#endif
    const std::string& vehID = veh.getID();
    VehicleInfoMap::iterator vi = myVehicleInfos.find(vehID);
    if (vi != myVehicleInfos.end()) {
        // Register move current offset to the next lane
        if (vi->second->currentLane != enteredLane) {
            vi->second->currentOffsetIndex++;
            vi->second->currentLane = enteredLane;
        }

#ifdef DEBUG_E2_NOTIFY_ENTER_AND_LEAVE
        if (DEBUG_COND) {
            std::cout << SIMTIME << " Vehicle '" << veh.getID() << "' on lane '" << veh.getLane()->getID()
                      << "' already known. No new VehicleInfo is created.\n"
                      << "enteredLane = " << enteredLane->getID() << "\nmyLanes[vi->offset] = " << myLanes[vi->second->currentOffsetIndex]
                      << std::endl;
        }
#endif
        assert(myLanes[vi->second->currentOffsetIndex] == enteredLane->getID());

        // but don't add a second subscription for another lane
        return false;
    }



#ifdef DEBUG_E2_NOTIFY_ENTER_AND_LEAVE
    if (DEBUG_COND) {
        std::cout << SIMTIME << " Adding VehicleInfo for vehicle '" << veh.getID() << "'." << std::endl;
    }
#endif

    // Add vehicle info
    myVehicleInfos.insert(std::make_pair(vehID, makeVehicleInfo(veh, enteredLane)));
    // Subscribe to vehicle's movement notifications
    return true;
}


MSE2Collector::VehicleInfo*
MSE2Collector::makeVehicleInfo(const SUMOVehicle& veh, const MSLane* enteredLane) const {
    // The vehicle's distance to the detector end
    int j = (int)(std::find(myLanes.begin(), myLanes.end(), enteredLane->getID()) - myLanes.begin());
    assert(j >= 0 && j < (int)myLanes.size());
    double entryOffset = myOffsets[j];
    double distToDetectorEnd = myDetectorLength - (entryOffset + veh.getPositionOnLane());
    bool onDetector = -entryOffset < veh.getPositionOnLane() && distToDetectorEnd > -veh.getVehicleType().getLength();

#ifdef DEBUG_E2_MAKE_VEHINFO
    if (DEBUG_COND) {
        std::cout << SIMTIME << " Making VehicleInfo for vehicle '" << veh.getID() << "'."
                  << "\ndistToDetectorEnd = " << distToDetectorEnd
                  << "\nveh.getPositionOnLane() = " << veh.getPositionOnLane()
                  << "\nentry lane offset (lane begin from detector begin) = " << entryOffset
                  << std::endl;
    }
#endif
    return new VehicleInfo(veh.getID(), veh.getVehicleType().getID(), veh.getVehicleType().getLength(), veh.getVehicleType().getMinGap(), enteredLane, entryOffset, j,
                           myOffsets[j] - myDetectorLength, distToDetectorEnd, onDetector);
}

void
MSE2Collector::detectorUpdate(const SUMOTime /* step */) {

#ifdef DEBUG_E2_DETECTOR_UPDATE
    if (DEBUG_COND) {
        std::cout << "\n" << SIMTIME << " detectorUpdate() for detector '" << myID << "'"
                  << "\nmyCurrentMeanSpeed = " << myCurrentMeanSpeed
                  << "\nmyCurrentMeanLength = " << myCurrentMeanLength
                  << "\nmyNumberOfEnteredVehicles = " << myNumberOfEnteredVehicles
                  << "\nmyNumberOfLeftVehicles = " << myNumberOfLeftVehicles
                  << "\nmyNumberOfSeenVehicles = " << myNumberOfSeenVehicles
                  << std::endl;
    }
#endif

// sort myMoveNotifications (required for jam processing) ascendingly according to vehicle's distance to the detector end
// (min = myMoveNotifications[0].distToDetectorEnd)
    std::sort(myMoveNotifications.begin(), myMoveNotifications.end(), compareMoveNotification);

    // reset values concerning current time step (these are updated in integrateMoveNotification() and aggregateOutputValues())
    myCurrentVehicleSamples = 0;
    myCurrentMeanSpeed = 0;
    myCurrentMeanLength = 0;
    myCurrentStartedHalts = 0;
    myCurrentHaltingsNumber = 0;

    JamInfo* currentJam = nullptr;
    std::vector<JamInfo*> jams;
    std::map<std::string, SUMOTime> haltingVehicles;
    std::map<std::string, SUMOTime> intervalHaltingVehicles;

    // go through the list of vehicles positioned on the detector
    for (std::vector<MoveNotificationInfo*>::iterator i = myMoveNotifications.begin(); i != myMoveNotifications.end(); ++i) {
        // The ID of the vehicle that has sent this notification in the last step
        const std::string& vehID = (*i)->id;
        VehicleInfoMap::iterator vi = myVehicleInfos.find(vehID);

        if (vi == myVehicleInfos.end()) {
            // The vehicle has already left the detector by lanechange, teleport, etc. (not longitudinal)
            integrateMoveNotification(nullptr, *i);
        } else {
            // Add move notification infos to detector values and VehicleInfo
            integrateMoveNotification(vi->second, *i);
        }
        // construct jam structure
        bool isInJam = checkJam(i, haltingVehicles, intervalHaltingVehicles);
        buildJam(isInJam, i, currentJam, jams);
    }

    // extract some aggregated values from the jam structure
    processJams(jams, currentJam);

    // Aggregate and normalize values for the detector output
    aggregateOutputValues();

    // save information about halting vehicles
    myHaltingVehicleDurations = haltingVehicles;
    myIntervalHaltingVehicleDurations = intervalHaltingVehicles;

#ifdef DEBUG_E2_DETECTOR_UPDATE
    if (DEBUG_COND) {
        std::cout << "\n" << SIMTIME << " Current lanes for vehicles still on or approaching the detector:" << std::endl;
    }
#endif
// update current and entered lanes for remaining vehicles
    VehicleInfoMap::iterator iv;
    for (iv = myVehicleInfos.begin(); iv != myVehicleInfos.end(); ++iv) {
#ifdef DEBUG_E2_DETECTOR_UPDATE
        if (DEBUG_COND) {
            std::cout << " Vehicle '" << iv->second->id << "'" << ": '"
                      << iv->second->currentLane->getID() << "'"
                      << std::endl;
        }
#endif
    }

#ifdef DEBUG_E2_DETECTOR_UPDATE
    if (DEBUG_COND) {
        std::cout << SIMTIME << " Discarding vehicles that have left the detector:" << std::endl;
    }
#endif
// Remove the vehicles that have left the detector
    std::set<std::string>::const_iterator i;
    for (i = myLeftVehicles.begin(); i != myLeftVehicles.end(); ++i) {
        VehicleInfoMap::iterator j = myVehicleInfos.find(*i);
        delete j->second;
        myVehicleInfos.erase(*i);
        myNumberOfLeftVehicles++;
#ifdef DEBUG_E2_DETECTOR_UPDATE
        if (DEBUG_COND) {
            std::cout << "Erased vehicle '" << *i << "'" << std::endl;
        }
#endif
    }
    myLeftVehicles.clear();

    // reset move notifications
    for (std::vector<MoveNotificationInfo*>::iterator j = myMoveNotifications.begin(); j != myMoveNotifications.end(); ++j) {
        delete *j;
    }
    myMoveNotifications.clear();
}


void
MSE2Collector::aggregateOutputValues() {
    myTimeSamples += 1;
    // compute occupancy values (note myCurrentMeanLength is still not normalized here, but holds the sum of all vehicle lengths)
    const double currentOccupancy = myCurrentMeanLength / myDetectorLength * (double) 100.;
    myCurrentOccupancy = currentOccupancy;
    myOccupancySum += currentOccupancy;
    myMaxOccupancy = MAX2(myMaxOccupancy, currentOccupancy);
    // compute jam values
    myMeanMaxJamInVehicles += myCurrentMaxJamLengthInVehicles;
    myMeanMaxJamInMeters += myCurrentMaxJamLengthInMeters;
    myMaxJamInVehicles = MAX2(myMaxJamInVehicles, myCurrentMaxJamLengthInVehicles);
    myMaxJamInMeters = MAX2(myMaxJamInMeters, myCurrentMaxJamLengthInMeters);
    // compute information about vehicle numbers
    const int numVehicles = (int)myMoveNotifications.size();
    myMeanVehicleNumber += numVehicles;
    myMaxVehicleNumber = MAX2(numVehicles, myMaxVehicleNumber);
    // norm current values
    myCurrentMeanSpeed = numVehicles != 0 ? myCurrentMeanSpeed / myCurrentVehicleSamples : -1;
    myCurrentMeanLength = numVehicles != 0 ? myCurrentMeanLength / (double) numVehicles : -1;
}



void
MSE2Collector::integrateMoveNotification(VehicleInfo* vi, const MoveNotificationInfo* mni) {

#ifdef DEBUG_E2_DETECTOR_UPDATE
    if (DEBUG_COND) {
        std::cout << SIMTIME << " integrateMoveNotification() for vehicle '" << mni->id << "'"
                  << "\ntimeOnDetector = " << mni->timeOnDetector
                  << "\nlengthOnDetector = " << mni->lengthOnDetector
                  << "\ntimeLoss = " << mni->timeLoss
                  << "\nspeed = " << mni->speed
                  << std::endl;
    }
#endif

// Accumulate detector values
    myVehicleSamples += mni->timeOnDetector;
    myTotalTimeLoss += mni->timeLoss;
    mySpeedSum += mni->speed * mni->timeOnDetector;
    myCurrentVehicleSamples += mni->timeOnDetector;
    myCurrentMeanSpeed += mni->speed * mni->timeOnDetector;
    myCurrentMeanLength += mni->lengthOnDetector;

    if (vi != nullptr) {
        // Accumulate individual values for the vehicle.
        // @note vi==0 occurs, if the vehicle info has been erased at
        //       notifyLeave() in case of a non-longitudinal exit (lanechange, teleport, etc.)
        vi->totalTimeOnDetector += mni->timeOnDetector;
        vi->accumulatedTimeLoss += mni->timeLoss;
        vi->lastAccel = mni->accel;
        vi->lastSpeed = mni->speed;
        vi->lastPos = myStartPos + vi->entryOffset + mni->newPos;
        vi->onDetector = mni->onDetector;
    }
}



MSE2Collector::MoveNotificationInfo*
MSE2Collector::makeMoveNotification(const SUMOVehicle& veh, double oldPos, double newPos, double newSpeed, const VehicleInfo& vehInfo) const {
#ifdef DEBUG_E2_NOTIFY_MOVE
    if (DEBUG_COND) {
        std::cout << SIMTIME << " makeMoveNotification() for vehicle '" << veh.getID() << "'"
                  << " oldPos = " << oldPos << " newPos = " << newPos << " newSpeed = " << newSpeed
                  << std::endl;
    }
#endif

    // Timefraction in [0,TS] the vehicle has spend on the detector in the last step
    double timeOnDetector;
    // Note that at this point, vehInfo.currentLane points to the lane at the beginning of the last timestep,
    // and vehInfo.enteredLanes is a list of lanes entered in the last timestep
    double timeLoss;
    calculateTimeLossAndTimeOnDetector(veh, oldPos, newPos, vehInfo, timeOnDetector, timeLoss);

    // The length of the part of the vehicle on the detector at the end of the last time step
    // may be shorter than vehicle's length if its back reaches out
    double lengthOnDetector = MAX2(MIN2(vehInfo.length, newPos + vehInfo.entryOffset), 0.);

    // XXX: Fulfulling the specifications of the documentation (lengthOnDetector = time integral
    //      over length of the vehicle's part on the detector) would be much more cumbersome.
    double distToExit = -vehInfo.exitOffset - newPos;
    // Eventually decrease further to account for the front reaching out
    lengthOnDetector = MAX2(0., lengthOnDetector + MIN2(0., distToExit));

    // whether the vehicle is still on the detector at the end of the time step
    bool stillOnDetector = -distToExit < vehInfo.length;

#ifdef DEBUG_E2_NOTIFY_MOVE
    if (DEBUG_COND) {
        std::cout << SIMTIME << " lengthOnDetector = " << lengthOnDetector
                  << "\nvehInfo.exitOffset = " << vehInfo.exitOffset
                  << " vehInfo.entryOffset = " << vehInfo.entryOffset
                  << " distToExit = " << distToExit
                  << std::endl;
    }
#endif

    /* Store new infos */
    return new MoveNotificationInfo(veh.getID(), oldPos, newPos, newSpeed, veh.getAcceleration(),
                                    myDetectorLength - (vehInfo.entryOffset + newPos),
                                    timeOnDetector, lengthOnDetector, timeLoss, stillOnDetector);
}

void
MSE2Collector::buildJam(bool isInJam, std::vector<MoveNotificationInfo*>::const_iterator mni, JamInfo*& currentJam, std::vector<JamInfo*>& jams) {
#ifdef DEBUG_E2_JAMS
    if (DEBUG_COND) {
        std::cout << SIMTIME << " buildJam() for vehicle '" << (*mni)->id << "'" << std::endl;
    }
#endif
    if (isInJam) {
        // The vehicle is in a jam;
        //  it may be a new one or already an existing one
        if (currentJam == nullptr) {
#ifdef DEBUG_E2_JAMS
            if (DEBUG_COND) {
                std::cout << SIMTIME << " vehicle '" << (*mni)->id << "' forms the start of the first jam" << std::endl;
            }
#endif
            // the vehicle is the first vehicle in a jam
            currentJam = new JamInfo();
            currentJam->firstStandingVehicle = mni;
        } else {
            // ok, we have a jam already. But - maybe it is too far away
            //  ... honestly, I can hardly find a reason for doing this,
            //  but jams were defined this way in an earlier version...
            MoveNotificationInfo* lastVeh = *currentJam->lastStandingVehicle;
            MoveNotificationInfo* currVeh = *mni;
            if (lastVeh->distToDetectorEnd - currVeh->distToDetectorEnd > myJamDistanceThreshold) {
#ifdef DEBUG_E2_JAMS
                if (DEBUG_COND) {
                    std::cout << SIMTIME << " vehicle '" << (*mni)->id << "' forms the start of a new jam" << std::endl;
                }
#endif
                // yep, yep, yep - it's a new one...
                //  close the frist, build a new
                jams.push_back(currentJam);
                currentJam = new JamInfo();
                currentJam->firstStandingVehicle = mni;
            }
        }
        currentJam->lastStandingVehicle = mni;
    } else {
        // the vehicle is not part of a jam...
        //  maybe we have to close an already computed jam
        if (currentJam != nullptr) {
#ifdef DEBUG_E2_JAMS
            if (DEBUG_COND) {
                std::cout << SIMTIME << " Closing current jam." << std::endl;
            }
#endif
            jams.push_back(currentJam);
            currentJam = nullptr;
        }
    }
}


bool
MSE2Collector::checkJam(std::vector<MoveNotificationInfo*>::const_iterator mni, std::map<std::string, SUMOTime>& haltingVehicles, std::map<std::string, SUMOTime>& intervalHaltingVehicles) {
#ifdef DEBUG_E2_JAMS
    if (DEBUG_COND) {
        std::cout << SIMTIME << " CheckJam() for vehicle '" << (*mni)->id << "'" << std::endl;
    }
#endif
    // jam-checking begins
    bool isInJam = false;
    // first, check whether the vehicle is slow enough to be counted as halting
    if ((*mni)->speed < myJamHaltingSpeedThreshold) {
        myCurrentHaltingsNumber++;
        // we have to track the time it was halting;
        // so let's look up whether it was halting before and compute the overall halting time
        bool wasHalting = myHaltingVehicleDurations.count((*mni)->id) > 0;
        if (wasHalting) {
            haltingVehicles[(*mni)->id] = myHaltingVehicleDurations[(*mni)->id] + DELTA_T;
            intervalHaltingVehicles[(*mni)->id] = myIntervalHaltingVehicleDurations[(*mni)->id] + DELTA_T;
        } else {
#ifdef DEBUG_E2_JAMS
            if (DEBUG_COND) {
                std::cout << SIMTIME << " vehicle '" << (*mni)->id << "' starts halting." << std::endl;
            }
#endif
            haltingVehicles[(*mni)->id] = DELTA_T;
            intervalHaltingVehicles[(*mni)->id] = DELTA_T;
            myCurrentStartedHalts++;
            myStartedHalts++;
        }
        // we now check whether the halting time is large enough
        if (haltingVehicles[(*mni)->id] > myJamHaltingTimeThreshold) {
            // yep --> the vehicle is a part of a jam
            isInJam = true;
        }
    } else {
        // is not standing anymore; keep duration information
        std::map<std::string, SUMOTime>::iterator v = myHaltingVehicleDurations.find((*mni)->id);
        if (v != myHaltingVehicleDurations.end()) {
            myPastStandingDurations.push_back(v->second);
            myHaltingVehicleDurations.erase(v);
        }
        v = myIntervalHaltingVehicleDurations.find((*mni)->id);
        if (v != myIntervalHaltingVehicleDurations.end()) {
            myPastIntervalStandingDurations.push_back((*v).second);
            myIntervalHaltingVehicleDurations.erase(v);
        }
    }
#ifdef DEBUG_E2_JAMS
    if (DEBUG_COND) {
        std::cout << SIMTIME << " vehicle '" << (*mni)->id << "'" << (isInJam ? "is jammed." : "is not jammed.") << std::endl;
    }
#endif
    return isInJam;
}


void
MSE2Collector::processJams(std::vector<JamInfo*>& jams, JamInfo* currentJam) {
    // push last jam
    if (currentJam != nullptr) {
        jams.push_back(currentJam);
        currentJam = nullptr;
    }

#ifdef DEBUG_E2_JAMS
    if (DEBUG_COND) {
        std::cout << "\n" << SIMTIME << " processJams()"
                  << "\nNumber of jams: " << jams.size() << std::endl;
    }
#endif

    // process jam information
    myCurrentMaxJamLengthInMeters = 0;
    myCurrentMaxJamLengthInVehicles = 0;
    myCurrentJamLengthInMeters = 0;
    myCurrentJamLengthInVehicles = 0;
    for (std::vector<JamInfo*>::const_iterator i = jams.begin(); i != jams.end(); ++i) {
        // compute current jam's values
        MoveNotificationInfo* lastVeh = *((*i)->lastStandingVehicle);
        MoveNotificationInfo* firstVeh = *((*i)->firstStandingVehicle);
        const double jamLengthInMeters = MAX2(lastVeh->distToDetectorEnd, 0.) -
                                         MAX2(firstVeh->distToDetectorEnd, 0.) +
                                         lastVeh->lengthOnDetector;
        const int jamLengthInVehicles = (int) distance((*i)->firstStandingVehicle, (*i)->lastStandingVehicle) + 1;
        // apply them to the statistics
        myCurrentMaxJamLengthInMeters = MAX2(myCurrentMaxJamLengthInMeters, jamLengthInMeters);
        myCurrentMaxJamLengthInVehicles = MAX2(myCurrentMaxJamLengthInVehicles, jamLengthInVehicles);
        myJamLengthInMetersSum += jamLengthInMeters;
        myJamLengthInVehiclesSum += jamLengthInVehicles;
        myCurrentJamLengthInMeters += jamLengthInMeters;
        myCurrentJamLengthInVehicles += jamLengthInVehicles;
#ifdef DEBUG_E2_JAMS
        if (DEBUG_COND) {
            std::cout << SIMTIME << " processing jam nr." << ((int) distance((std::vector<JamInfo*>::const_iterator) jams.begin(), i) + 1)
                      << "\njamLengthInMeters = " << jamLengthInMeters
                      << " jamLengthInVehicles = " << jamLengthInVehicles
                      << std::endl;
        }
#endif
    }
    myCurrentJamNo = (int) jams.size();

    // clean up jam structure
    for (std::vector<JamInfo*>::iterator i = jams.begin(); i != jams.end(); ++i) {
        delete *i;
    }
}

void
MSE2Collector::calculateTimeLossAndTimeOnDetector(const SUMOVehicle& veh, double oldPos, double newPos, const VehicleInfo& vi, double& timeOnDetector, double& timeLoss) const {
    assert(veh.getID() == vi.id);
    assert(newPos + vi.entryOffset >= 0);

    if (oldPos == newPos) {
        // vehicle is stopped
        timeLoss = TS;
        timeOnDetector = TS;
        return;
    }

    // Eventual positional offset of the detector start from the lane's start
    double entryPos = MAX2(-vi.entryOffset, 0.);
    // Time of this vehicle entering the detector in the last time step
    double entryTime = 0;
    // Take into account the time before entering the detector, if there is.
    if (oldPos < entryPos) {
        // Vehicle entered the detector in the last step, either traversing the detector start or somewhere in the middle.
        entryTime = MSCFModel::passingTime(oldPos, entryPos, newPos, veh.getPreviousSpeed(), veh.getSpeed());
    }
    // speed at detector entry
    double entrySpeed = MSCFModel::speedAfterTime(entryTime, veh.getPreviousSpeed(), newPos - oldPos);
    // Calculate time spent on detector until reaching newPos or a detector exit
    double exitPos = MIN2(newPos, -vi.exitOffset + vi.length);
    assert(entryPos < exitPos);

    // calculate vehicle's time spent on the detector
    double exitTime;
    if (exitPos == newPos) {
        exitTime  = TS;
    } else {
        exitTime = MSCFModel::passingTime(oldPos, exitPos, newPos, veh.getPreviousSpeed(), veh.getSpeed());
    }

    // Vehicle's Speed when leaving the detector
    double exitSpeed = MSCFModel::speedAfterTime(exitTime, veh.getPreviousSpeed(), newPos - oldPos);

    // Maximal speed on vehicle's current lane (== lane before last time step)
    // Note: this disregards the possibility of different maximal speeds on different traversed lanes.
    //       (we accept this as discretization error)
    double vmax = MAX2(veh.getLane()->getVehicleMaxSpeed(&veh), NUMERICAL_EPS);

    // Time loss suffered on the detector
    timeOnDetector = exitTime - entryTime;
    timeLoss = MAX2(0., timeOnDetector * (vmax - (entrySpeed + exitSpeed) / 2) / vmax);

#ifdef DEBUG_E2_TIME_ON_DETECTOR
    if (DEBUG_COND) {
        std::cout << SIMTIME << " calculateTimeLoss() for vehicle '" << veh.getID() << "'"
                  << " oldPos = " << oldPos << " newPos = " << newPos
                  << " entryPos = " << entryPos << " exitPos = " << exitPos
                  << " timeOnDetector = " << timeOnDetector
                  << " timeLoss = " << timeLoss
                  << std::endl;
    }
#endif
}


void
MSE2Collector::writeXMLDetectorProlog(OutputDevice& dev) const {
    dev.writeXMLHeader("detector", "det_e2_file.xsd");
}

void
MSE2Collector::writeXMLOutput(OutputDevice& dev, SUMOTime startTime, SUMOTime stopTime) {
    dev << "   <interval begin=\"" << time2string(startTime) << "\" end=\"" << time2string(stopTime) << "\" " << "id=\"" << getID() << "\" ";

    const double meanSpeed = myVehicleSamples != 0 ? mySpeedSum / myVehicleSamples : -1;
    const double meanOccupancy = myTimeSamples != 0 ? myOccupancySum / (double) myTimeSamples : 0;
    const double meanJamLengthInMeters = myTimeSamples != 0 ? myMeanMaxJamInMeters / (double) myTimeSamples : 0;
    const double meanJamLengthInVehicles = myTimeSamples != 0 ? myMeanMaxJamInVehicles / (double) myTimeSamples : 0;
    const double meanVehicleNumber = myTimeSamples != 0 ? (double) myMeanVehicleNumber / (double) myTimeSamples : 0;
    const double meanTimeLoss = myNumberOfSeenVehicles != 0 ? myTotalTimeLoss / myNumberOfSeenVehicles : -1;

    SUMOTime haltingDurationSum = 0;
    SUMOTime maxHaltingDuration = 0;
    int haltingNo = 0;
    for (std::vector<SUMOTime>::iterator i = myPastStandingDurations.begin(); i != myPastStandingDurations.end(); ++i) {
        haltingDurationSum += (*i);
        maxHaltingDuration = MAX2(maxHaltingDuration, (*i));
        haltingNo++;
    }
    for (std::map<std::string, SUMOTime> ::iterator i = myHaltingVehicleDurations.begin(); i != myHaltingVehicleDurations.end(); ++i) {
        haltingDurationSum += (*i).second;
        maxHaltingDuration = MAX2(maxHaltingDuration, (*i).second);
        haltingNo++;
    }
    const SUMOTime meanHaltingDuration = haltingNo != 0 ? haltingDurationSum / haltingNo : 0;

    SUMOTime intervalHaltingDurationSum = 0;
    SUMOTime intervalMaxHaltingDuration = 0;
    int intervalHaltingNo = 0;
    for (std::vector<SUMOTime>::iterator i = myPastIntervalStandingDurations.begin(); i != myPastIntervalStandingDurations.end(); ++i) {
        intervalHaltingDurationSum += (*i);
        intervalMaxHaltingDuration = MAX2(intervalMaxHaltingDuration, (*i));
        intervalHaltingNo++;
    }
    for (std::map<std::string, SUMOTime> ::iterator i = myIntervalHaltingVehicleDurations.begin(); i != myIntervalHaltingVehicleDurations.end(); ++i) {
        intervalHaltingDurationSum += (*i).second;
        intervalMaxHaltingDuration = MAX2(intervalMaxHaltingDuration, (*i).second);
        intervalHaltingNo++;
    }
    const SUMOTime intervalMeanHaltingDuration = intervalHaltingNo != 0 ? intervalHaltingDurationSum / intervalHaltingNo : 0;

#ifdef DEBUG_E2_XML_OUT
    if (DEBUG_COND) {
        std::stringstream ss;
        ss  << "sampledSeconds=\"" << myVehicleSamples << "\" "
            << "myTimeSamples=\"" << myTimeSamples << "\" "
            << "myOccupancySum=\"" << myOccupancySum << "\" "
            << "myMeanVehicleNumber=\"" << myMeanVehicleNumber << "\" "
            << "nVehEntered=\"" << myNumberOfEnteredVehicles << "\" "
            << "meanSpeed=\"" << meanSpeed << "\"";
        std::cout << ss.str() << std::endl;
    }
#endif


    dev << "sampledSeconds=\"" << myVehicleSamples << "\" "
        << "nVehEntered=\"" << myNumberOfEnteredVehicles << "\" "
        << "nVehLeft=\"" << myNumberOfLeftVehicles << "\" "
        << "nVehSeen=\"" << myNumberOfSeenVehicles << "\" "
        << "meanSpeed=\"" << meanSpeed << "\" "
        << "meanTimeLoss=\"" << meanTimeLoss << "\" "
        << "meanOccupancy=\"" << meanOccupancy << "\" "
        << "maxOccupancy=\"" << myMaxOccupancy << "\" "
        << "meanMaxJamLengthInVehicles=\"" << meanJamLengthInVehicles << "\" "
        << "meanMaxJamLengthInMeters=\"" << meanJamLengthInMeters << "\" "
        << "maxJamLengthInVehicles=\"" << myMaxJamInVehicles << "\" "
        << "maxJamLengthInMeters=\"" << myMaxJamInMeters << "\" "
        << "jamLengthInVehiclesSum=\"" << myJamLengthInVehiclesSum << "\" "
        << "jamLengthInMetersSum=\"" << myJamLengthInMetersSum << "\" "
        << "meanHaltingDuration=\"" << STEPS2TIME(meanHaltingDuration) << "\" "
        << "maxHaltingDuration=\"" << STEPS2TIME(maxHaltingDuration) << "\" "
        << "haltingDurationSum=\"" << STEPS2TIME(haltingDurationSum) << "\" "
        << "meanIntervalHaltingDuration=\"" << STEPS2TIME(intervalMeanHaltingDuration) << "\" "
        << "maxIntervalHaltingDuration=\"" << STEPS2TIME(intervalMaxHaltingDuration) << "\" "
        << "intervalHaltingDurationSum=\"" << STEPS2TIME(intervalHaltingDurationSum) << "\" "
        << "startedHalts=\"" << myStartedHalts << "\" "
        << "meanVehicleNumber=\"" << meanVehicleNumber << "\" "
        << "maxVehicleNumber=\"" << myMaxVehicleNumber << "\" "
        << "/>\n";
    reset();

}

void
MSE2Collector::reset() {
    myVehicleSamples = 0;
    myTotalTimeLoss = 0.;
    myNumberOfEnteredVehicles = 0;
    myNumberOfSeenVehicles -= myNumberOfLeftVehicles;
    myNumberOfLeftVehicles = 0;
    myMaxVehicleNumber = 0;

    mySpeedSum = 0;
    myStartedHalts = 0;
    myJamLengthInMetersSum = 0;
    myJamLengthInVehiclesSum = 0;
    myOccupancySum = 0;
    myMaxOccupancy = 0;
    myMeanMaxJamInVehicles = 0;
    myMeanMaxJamInMeters = 0;
    myMaxJamInVehicles = 0;
    myMaxJamInMeters = 0;
    myTimeSamples = 0;
    myMeanVehicleNumber = 0;
    for (std::map<std::string, SUMOTime>::iterator i = myIntervalHaltingVehicleDurations.begin(); i != myIntervalHaltingVehicleDurations.end(); ++i) {
        (*i).second = 0;
    }
    myPastStandingDurations.clear();
    myPastIntervalStandingDurations.clear();
}


int
MSE2Collector::getCurrentVehicleNumber() const {
    int result = 0;
    for (VehicleInfoMap::const_iterator it = myVehicleInfos.begin(); it != myVehicleInfos.end(); it++) {
        if (it->second->onDetector) {
            result++;
        }
    }
    return result;
}



std::vector<std::string>
MSE2Collector::getCurrentVehicleIDs() const {
    std::vector<std::string> ret;
    for (VehicleInfoMap::const_iterator i = myVehicleInfos.begin(); i != myVehicleInfos.end(); ++i) {
        if (i->second->onDetector) {
            ret.push_back(i->second->id);
        }
    }
    std::sort(ret.begin(), ret.end());
    return ret;
}


std::vector<MSE2Collector::VehicleInfo*>
MSE2Collector::getCurrentVehicles() const {
    std::vector<VehicleInfo*> res;
    VehicleInfoMap::const_iterator i;
    for (i = myVehicleInfos.begin(); i != myVehicleInfos.end(); ++i) {
        if (i->second->onDetector) {
            res.push_back(i->second);
        }
    }
    return res;
}



int
MSE2Collector::getEstimatedCurrentVehicleNumber(double speedThreshold) const {

    //    double distance = std::numeric_limits<double>::max();
    double thresholdSpeed = myLane->getSpeedLimit() / speedThreshold;

    int count = 0;
    for (VehicleInfoMap::const_iterator it = myVehicleInfos.begin();
            it != myVehicleInfos.end(); it++) {
        if (it->second->onDetector) {
            //            if (it->position < distance) {
            //                distance = it->position;
            //            }
            //            const double realDistance = myLane->getLength() - distance; // the closer vehicle get to the light the greater is the distance
            const double realDistance = it->second->distToDetectorEnd;
            if (it->second->lastSpeed <= thresholdSpeed || it->second->lastAccel > 0) { //TODO speed less half of the maximum speed for the lane NEED TUNING
                count = (int)(realDistance / (it->second->length + it->second->minGap)) + 1;
            }
        }
    }

    return count;
}

double
MSE2Collector::getEstimateQueueLength() const {

    if (myVehicleInfos.empty()) {
        return -1;
    }

    double distance = std::numeric_limits<double>::max();
    double realDistance = 0;
    bool flowing =  true;
    for (VehicleInfoMap::const_iterator it = myVehicleInfos.begin();
            it != myVehicleInfos.end(); it++) {
        if (it->second->onDetector) {
            distance = MIN2(it->second->lastPos, distance);
            //  double distanceTemp = myLane->getLength() - distance;
            if (it->second->lastSpeed <= 0.5) {
                realDistance = distance - it->second->length + it->second->minGap;
                flowing = false;
            }
            //            DBG(
            //                std::ostringstream str;
            //                str << time2string(MSNet::getInstance()->getCurrentTimeStep())
            //                << " MSE2Collector::getEstimateQueueLength::"
            //                << " lane " << myLane->getID()
            //                << " vehicle " << it->second.id
            //                << " positionOnLane " << it->second.position
            //                << " vel " << it->second.speed
            //                << " realDistance " << realDistance;
            //                WRITE_MESSAGE(str.str());
            //            )
        }
    }
    if (flowing) {
        return 0;
    } else {
        return myLane->getLength() - realDistance;
    }
}


void
MSE2Collector::clearState() {
    for (std::vector<MoveNotificationInfo*>::iterator j = myMoveNotifications.begin(); j != myMoveNotifications.end(); ++j) {
        delete *j;
    }
    myMoveNotifications.clear();

    // clear vehicle infos
    for (VehicleInfoMap::iterator j = myVehicleInfos.begin(); j != myVehicleInfos.end(); ++j) {
        delete j->second;
    }
    myVehicleInfos.clear();
}

/****************************************************************************/
