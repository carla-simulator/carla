/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2015-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSParkingArea.cpp
/// @author  Mirco Sturari
/// @author  Jakob Erdmann
/// @date    Tue, 19.01.2016
///
// A area where vehicles can park next to the road
/****************************************************************************/
#include <config.h>

#include <cassert>
#include <utils/common/WrappingCommand.h>
#include <utils/vehicle/SUMOVehicle.h>
#include <utils/geom/Position.h>
#include <utils/geom/GeomHelper.h>
#include <microsim/MSEventControl.h>
#include <microsim/MSNet.h>
#include <microsim/MSVehicleType.h>
#include "MSLane.h"
#include <microsim/transportables/MSTransportable.h>
#include "MSParkingArea.h"
#include "MSGlobals.h"

//#define DEBUG_RESERVATIONS
//#define DEBUG_COND2(obj) (obj.getID() == "v.3")
#define DEBUG_COND2(obj) (obj.isSelected())


// ===========================================================================
// method definitions
// ===========================================================================
MSParkingArea::MSParkingArea(const std::string& id,
                             const std::vector<std::string>& lines,
                             MSLane& lane,
                             double begPos, double endPos,
                             int capacity,
                             double width, double length, double angle, const std::string& name,
                             bool onRoad) :
    MSStoppingPlace(id, lines, lane, begPos, endPos, name),
    myCapacity(0),
    myOnRoad(onRoad),
    myWidth(width),
    myLength(length),
    myAngle(angle),
    myEgressBlocked(false),
    myReservationTime(-1),
    myReservations(0),
    myReservationMaxLength(0),
    myNumAlternatives(0),
    myLastStepOccupancy(0),
    myUpdateEvent(nullptr) {
    // initialize unspecified defaults
    if (myWidth == 0) {
        myWidth = SUMO_const_laneWidth;
    }
    const double spaceDim = capacity > 0 ? myLane.interpolateLanePosToGeometryPos((myEndPos - myBegPos) / capacity) : 7.5;
    if (myLength == 0) {
        myLength = spaceDim;
    }

    const double offset = MSGlobals::gLefthand ? -1 : 1;
    myShape = lane.getShape().getSubpart(
                  lane.interpolateLanePosToGeometryPos(begPos),
                  lane.interpolateLanePosToGeometryPos(endPos));
    if (!myOnRoad) {
        myShape.move2side((lane.getWidth() / 2. + myWidth / 2.) * offset);
    }
    // Initialize space occupancies if there is a road-side capacity
    // The overall number of lots is fixed and each lot accepts one vehicle regardless of size
    for (int i = 0; i < capacity; ++i) {
        const Position f = myShape.positionAtOffset(spaceDim * (i));
        const Position s = myShape.positionAtOffset(spaceDim * (i + 1));

        Position pos;
        if (myAngle == 0) {
            // parking parallel to the road
            pos = s;
        } else {
            // angled parking
            double hlp_angle = fabs(((double)atan2((s.x() - f.x()), (f.y() - s.y())) * (double) 180.0 / (double)M_PI) - 180);
            if (myAngle >= 0 && myAngle <= 90) {
                pos.setx((f.x() + s.x()) / 2 - (myWidth / 2) * (1 - cos(myAngle / 180 * M_PI))*cos(hlp_angle / 180 * M_PI));
                pos.sety((f.y() + s.y()) / 2 + (myWidth / 2) * (1 - cos(myAngle / 180 * M_PI))*sin(hlp_angle / 180 * M_PI));
                pos.setz((f.z() + s.z()) / 2);
            } else if (myAngle > 90 && myAngle <= 180) {
                pos.setx((f.x() + s.x()) / 2 - (myWidth / 2) * (1 + cos(myAngle / 180 * M_PI))*cos(hlp_angle / 180 * M_PI));
                pos.sety((f.y() + s.y()) / 2 + (myWidth / 2) * (1 + cos(myAngle / 180 * M_PI))*sin(hlp_angle / 180 * M_PI));
                pos.setz((f.z() + s.z()) / 2);
            } else if (myAngle > 180 && myAngle <= 270) {
                pos.setx((f.x() + s.x()) / 2 - (myLength)*sin((myAngle - hlp_angle) / 180 * M_PI) - (myWidth / 2) * (1 + cos(myAngle / 180 * M_PI))*cos(hlp_angle / 180 * M_PI));
                pos.sety((f.y() + s.y()) / 2 + (myLength)*cos((myAngle - hlp_angle) / 180 * M_PI) + (myWidth / 2) * (1 + cos(myAngle / 180 * M_PI))*sin(hlp_angle / 180 * M_PI));
                pos.setz((f.z() + s.z()) / 2);
            } else if (myAngle > 270 && myAngle < 360) {
                pos.setx((f.x() + s.x()) / 2 - (myLength)*sin((myAngle - hlp_angle) / 180 * M_PI) - (myWidth / 2) * (1 - cos(myAngle / 180 * M_PI))*cos(hlp_angle / 180 * M_PI));
                pos.sety((f.y() + s.y()) / 2 + (myLength)*cos((myAngle - hlp_angle) / 180 * M_PI) + (myWidth / 2) * (1 - cos(myAngle / 180 * M_PI))*sin(hlp_angle / 180 * M_PI));
                pos.setz((f.z() + s.z()) / 2);
            } else {
                pos = (f + s) * 0.5;
            }
        }

        addLotEntry(pos.x(), pos.y(), pos.z(),
                    myWidth, myLength,
                    ((double) atan2((s.x() - f.x()), (f.y() - s.y())) * (double) 180.0 / (double) M_PI) + myAngle);
        mySpaceOccupancies.back().myEndPos = myBegPos + MAX2(POSITION_EPS, spaceDim * (i + 1));
    }
    computeLastFreePos();
}

MSParkingArea::~MSParkingArea() {}

void
MSParkingArea::addLotEntry(double x, double y, double z,
                           double width, double length, double angle) {
    LotSpaceDefinition lsd;
    lsd.index = (int)mySpaceOccupancies.size();
    lsd.vehicle = nullptr;
    lsd.myPosition = Position(x, y, z);
    lsd.myWidth = width;
    lsd.myLength = length;
    lsd.myRotation = angle;
    // If we are modelling parking set the end position to the lot position relative to the lane
    //   rather than the end of the parking area - this results in vehicles stopping nearer the space
    //   and re-entering the lane nearer the space. (If we are not modelling parking the vehicle will usually
    //    enter the space and re-enter at the end of the parking area.)
    if (MSGlobals::gModelParkingManoeuver) {
        const double offset = this->getLane().getShape().nearest_offset_to_point2D(lsd.myPosition);
        if (offset <  getBeginLanePosition()) {
            lsd.myEndPos =  getBeginLanePosition() + POSITION_EPS;
        } else {
            if (this->getLane().getLength() > offset) {
                lsd.myEndPos = offset;
            } else {
                lsd.myEndPos = this->getLane().getLength() - POSITION_EPS;
            }
        }
        // Work out the angle of the lot relative to the lane  (-90 adjusts for the way the bay is drawn )
        double relativeAngle = fmod(lsd.myRotation - 90., 360) - fmod(RAD2DEG(this->getLane().getShape().rotationAtOffset(lsd.myEndPos)), 360) + 0.5;
        if (relativeAngle < 0.) {
            relativeAngle += 360.;
        }
        lsd.myManoeuverAngle = relativeAngle;

        //   if p2.y is -ve the lot is on LHS of lane relative to lane direction
        //    we need to know this because it inverts the complexity of the parking manoeuver
        Position p2 = this->getLane().getShape().transformToVectorCoordinates(lsd.myPosition);
        if (p2.y() < (0. + POSITION_EPS)) {
            lsd.mySideIsLHS = true;
        } else {
            lsd.mySideIsLHS = false;
        }
    } else {
        lsd.myEndPos = myEndPos;
        lsd.myManoeuverAngle = int(angle); // unused unless gModelParkingManoeuver is true
        lsd.mySideIsLHS = true;
    }


    mySpaceOccupancies.push_back(lsd);
    myCapacity++;
    computeLastFreePos();
}

int
MSParkingArea::getLastFreeLotAngle() const {
    assert(myLastFreeLot >= 0);
    assert(myLastFreeLot < (int)mySpaceOccupancies.size());

    const LotSpaceDefinition& lsd = mySpaceOccupancies[myLastFreeLot];
    if (lsd.mySideIsLHS) {
        return abs(int(lsd.myManoeuverAngle)) % 180;
    } else {
        return abs(abs(int(lsd.myManoeuverAngle)) % 180 - 180) % 180;
    }
}

double
MSParkingArea::getLastFreeLotGUIAngle() const {
    assert(myLastFreeLot >= 0);
    assert(myLastFreeLot < (int)mySpaceOccupancies.size());

    const LotSpaceDefinition& lsd = mySpaceOccupancies[myLastFreeLot];
    if (lsd.myManoeuverAngle > 180.) {
        return DEG2RAD(lsd.myManoeuverAngle - 360.);
    } else {
        return DEG2RAD(lsd.myManoeuverAngle);
    }
}


double
MSParkingArea::getLastFreePos(const SUMOVehicle& forVehicle) const {
    if (myCapacity == (int)myEndPositions.size()) {
        // keep enough space so that  parking vehicles can leave
        return myLastFreePos - forVehicle.getVehicleType().getMinGap() - POSITION_EPS;
    } else {
        // XXX if (forVehicle.getLane() == myLane && forVehicle.getPositionOnLane() > myLastFreePos) {
        //        find freePos beyond vehicle position }
        return myLastFreePos;
    }
}

Position
MSParkingArea::getVehiclePosition(const SUMOVehicle& forVehicle) const {
    for (const auto& lsd : mySpaceOccupancies) {
        if (lsd.vehicle == &forVehicle) {
            return lsd.myPosition;
        }
    }
    return Position::INVALID;
}


double
MSParkingArea::getInsertionPosition(const SUMOVehicle& forVehicle) const {
    for (const auto& lsd : mySpaceOccupancies) {
        if (lsd.vehicle == &forVehicle) {
            return lsd.myEndPos;
        }
    }
    return -1;
}


double
MSParkingArea::getVehicleAngle(const SUMOVehicle& forVehicle) const {
    for (const auto& lsd : mySpaceOccupancies) {
        if (lsd.vehicle == &forVehicle) {
            return (lsd.myRotation - 90.) * (double) M_PI / (double) 180.0;
        }
    }
    return 0;
}

double
MSParkingArea::getGUIAngle(const SUMOVehicle& forVehicle) const {
    for (const auto& lsd : mySpaceOccupancies) {
        if (lsd.vehicle == &forVehicle) {
            if (lsd.myManoeuverAngle > 180.) {
                return DEG2RAD(lsd.myManoeuverAngle - 360.);
            } else {
                return DEG2RAD(lsd.myManoeuverAngle);
            }
        }
    }
    return 0.;
}

int
MSParkingArea::getManoeuverAngle(const SUMOVehicle& forVehicle) const {
    for (const auto& lsd : mySpaceOccupancies) {
        if (lsd.vehicle == &forVehicle) {
            if (lsd.mySideIsLHS) {
                return abs(int(lsd.myManoeuverAngle)) % 180;
            } else {
                return abs(abs(int(lsd.myManoeuverAngle)) % 180 - 180) % 180;
            }
        }
    }
    return 0;
}


void
MSParkingArea::enter(SUMOVehicle* veh) {
    double beg = veh->getPositionOnLane() + veh->getVehicleType().getMinGap();
    double end = veh->getPositionOnLane() - veh->getVehicleType().getLength();
    assert(myLastFreePos >= 0);
    assert(myLastFreeLot < (int)mySpaceOccupancies.size());
    if (myUpdateEvent == nullptr) {
        myUpdateEvent = new WrappingCommand<MSParkingArea>(this, &MSParkingArea::updateOccupancy);
        MSNet::getInstance()->getEndOfTimestepEvents()->addEvent(myUpdateEvent);
    }
    mySpaceOccupancies[myLastFreeLot].vehicle = veh;
    myEndPositions[veh] = std::pair<double, double>(beg, end);
    computeLastFreePos();
}


void
MSParkingArea::leaveFrom(SUMOVehicle* what) {
    assert(myEndPositions.find(what) != myEndPositions.end());
    if (myUpdateEvent == nullptr) {
        myUpdateEvent = new WrappingCommand<MSParkingArea>(this, &MSParkingArea::updateOccupancy);
        MSNet::getInstance()->getEndOfTimestepEvents()->addEvent(myUpdateEvent);
    }
    for (auto& lsd : mySpaceOccupancies) {
        if (lsd.vehicle == what) {
            lsd.vehicle = nullptr;
            break;
        }
    }
    myEndPositions.erase(myEndPositions.find(what));
    computeLastFreePos();
}


SUMOTime
MSParkingArea::updateOccupancy(SUMOTime /* currentTime */) {
    myLastStepOccupancy = getOccupancy();
    myUpdateEvent = nullptr;
    return 0;
}


void
MSParkingArea::computeLastFreePos() {
    myLastFreeLot = -1;
    myLastFreePos = myBegPos;
    myEgressBlocked = false;
    for (auto& lsd : mySpaceOccupancies) {
        if (lsd.vehicle == nullptr
                || (getOccupancy() == getCapacity()
                    && lsd.vehicle->remainingStopDuration() <= 0
                    && !lsd.vehicle->isStoppedTriggered())) {
            if (lsd.vehicle == nullptr) {
                myLastFreeLot = lsd.index;
                myLastFreePos = lsd.myEndPos;
            } else {
                // vehicle wants to exit the parking area
                myLastFreeLot = lsd.index;
                myLastFreePos = lsd.myEndPos - lsd.vehicle->getVehicleType().getLength() - POSITION_EPS;
                myEgressBlocked = true;
            }
            break;
        } else {
            myLastFreePos = MIN2(myLastFreePos,
                                 lsd.myEndPos - lsd.vehicle->getVehicleType().getLength() - NUMERICAL_EPS);
        }
    }
}


double
MSParkingArea::getLastFreePosWithReservation(SUMOTime t, const SUMOVehicle& forVehicle) {
    if (forVehicle.getLane() != &myLane) {
        // for different lanes, do not consider reservations to avoid lane-order
        // dependency in parallel simulation
#ifdef DEBUG_RESERVATIONS
        if (DEBUG_COND2(forVehicle)) {
            std::cout << SIMTIME << " pa=" << getID() << " freePosRes veh=" << forVehicle.getID() << " other lane\n";
        }
#endif
        if (myNumAlternatives > 0 && getOccupancy() == getCapacity()) {
            // ensure that the vehicle reaches the rerouter lane
            return MAX2(myBegPos, MIN2(POSITION_EPS, myEndPos));
        } else {
            return getLastFreePos(forVehicle);
        }
    }
    if (t > myReservationTime) {
#ifdef DEBUG_RESERVATIONS
        if (DEBUG_COND2(forVehicle)) {
            std::cout << SIMTIME << " pa=" << getID() << " freePosRes veh=" << forVehicle.getID() << " first reservation\n";
        }
#endif
        myReservationTime = t;
        myReservations = 1;
        myReservationMaxLength = forVehicle.getVehicleType().getLength();
        for (const auto& lsd : mySpaceOccupancies) {
            if (lsd.vehicle != nullptr) {
                myReservationMaxLength = MAX2(myReservationMaxLength, lsd.vehicle->getVehicleType().getLength());
            }
        }
        return getLastFreePos(forVehicle);
    } else {
        if (myCapacity > getOccupancy() + myReservations) {
#ifdef DEBUG_RESERVATIONS
            if (DEBUG_COND2(forVehicle)) {
                std::cout << SIMTIME << " pa=" << getID() << " freePosRes veh=" << forVehicle.getID() << " res=" << myReservations << " enough space\n";
            }
#endif
            myReservations++;
            myReservationMaxLength = MAX2(myReservationMaxLength, forVehicle.getVehicleType().getLength());
            return getLastFreePos(forVehicle);
        } else {
            if (myCapacity == 0) {
                return getLastFreePos(forVehicle);
            } else {
#ifdef DEBUG_RESERVATIONS
                if (DEBUG_COND2(forVehicle)) std::cout << SIMTIME << " pa=" << getID() << " freePosRes veh=" << forVehicle.getID()
                                                           << " res=" << myReservations << " resTime=" << myReservationTime << " reserved full, maxLen=" << myReservationMaxLength << " endPos=" << mySpaceOccupancies[0].myEndPos << "\n";
#endif
                return (mySpaceOccupancies[0].myEndPos
                        - myReservationMaxLength
                        - forVehicle.getVehicleType().getMinGap()
                        - NUMERICAL_EPS);
            }
        }
    }
}


double
MSParkingArea::getWidth() const {
    return myWidth;
}


double
MSParkingArea::getLength() const {
    return myLength;
}


double
MSParkingArea::getAngle() const {
    return myAngle;
}


int
MSParkingArea::getCapacity() const {
    return myCapacity;
}


int
MSParkingArea::getOccupancy() const {
    return (int)myEndPositions.size() - (myEgressBlocked ? 1 : 0);
}


int
MSParkingArea::getOccupancyIncludingBlocked() const {
    return (int)myEndPositions.size();
}

void
MSParkingArea::notifyEgressBlocked() {
    computeLastFreePos();
}


/****************************************************************************/
