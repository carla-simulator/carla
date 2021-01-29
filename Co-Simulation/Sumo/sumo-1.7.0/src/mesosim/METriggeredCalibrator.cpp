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
/// @file    METriggeredCalibrator.cpp
/// @author  Daniel Krajzewicz
/// @date    Tue, May 2005
///
// Calibrates the flow on a segment to a specified one
/****************************************************************************/
#include <config.h>

#include <string>
#include <algorithm>
#include <cmath>
#include <microsim/MSGlobals.h>
#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <microsim/MSEventControl.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/output/MSRouteProbe.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/common/ToString.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/StringTokenizer.h>
#include <utils/xml/XMLSubSys.h>
#include <utils/common/StringUtils.h>
#include <utils/options/OptionsCont.h>
#include <utils/vehicle/SUMOVehicleParserHelper.h>
#include <utils/distribution/RandomDistributor.h>
#include <utils/vehicle/SUMOVehicleParameter.h>
#include "MELoop.h"
#include "MESegment.h"
#include "MEVehicle.h"
#include "METriggeredCalibrator.h"


// ===========================================================================
// method definitions
// ===========================================================================
METriggeredCalibrator::METriggeredCalibrator(const std::string& id,
        const MSEdge* const edge, const double pos,
        const std::string& aXMLFilename,
        const std::string& outputFilename,
        const SUMOTime freq, const double length,
        const MSRouteProbe* probe,
        const double invalidJamThreshold,
        const std::string& vTypes) :
    MSCalibrator(id, edge, (MSLane*)nullptr, pos, aXMLFilename, outputFilename, freq, length, probe, invalidJamThreshold, vTypes, false),
    mySegment(MSGlobals::gMesoNet->getSegmentForEdge(*edge, pos)) {
    myEdgeMeanData.setDescription("meandata_calibrator_" + getID());
    mySegment->addDetector(&myEdgeMeanData);
}


METriggeredCalibrator::~METriggeredCalibrator() {
    if (myCurrentStateInterval != myIntervals.end()) {
        // need to do it here and not in MSCalibrator because otherwise meandata is gone
        intervalEnd();
        // but avoid to call it again in MSCalibrator
        myCurrentStateInterval = myIntervals.end();
    }
    mySegment->removeDetector(&myEdgeMeanData);
}


bool
METriggeredCalibrator::tryEmit(MESegment* s, MEVehicle* vehicle) {
    if (s->initialise(vehicle, vehicle->getParameter().depart)) {
        if (!MSNet::getInstance()->getVehicleControl().addVehicle(vehicle->getID(), vehicle)) {
            throw ProcessError("Emission of vehicle '" + vehicle->getID() + "' in calibrator '" + getID() + "'failed!");
        }
        return true;
    }
    return false;
}


SUMOTime
METriggeredCalibrator::execute(SUMOTime currentTime) {
    // get current simulation values (valid for the last simulation second)
    // XXX could we miss vehicle movements if this is called less often than every DELTA_T (default) ?
    mySegment->prepareDetectorForWriting(myEdgeMeanData);

    // check whether an adaptation value exists
    if (isCurrentStateActive(currentTime)) {
        // all happens in isCurrentStateActive()
        myAmActive = true;
    } else {
        myAmActive = false;
        myEdgeMeanData.reset(); // discard collected values
        if (!mySpeedIsDefault) {
            // if not, reset adaptation values
            mySegment->getEdge().setMaxSpeed(myDefaultSpeed);
            MESegment* first = MSGlobals::gMesoNet->getSegmentForEdge(mySegment->getEdge());
            const double jamThresh = OptionsCont::getOptions().getFloat("meso-jam-threshold");
            while (first != nullptr) {
                first->setSpeed(myDefaultSpeed, currentTime, jamThresh);
                first = first->getNextSegment();
            }
            mySpeedIsDefault = true;
        }
        if (myCurrentStateInterval == myIntervals.end()) {
            // keep calibrator alive but do not call again
            return TIME2STEPS(86400);
        }
        return myFrequency;
    }
    const bool calibrateFlow = myCurrentStateInterval->q >= 0;
    const bool calibrateSpeed = myCurrentStateInterval->v >= 0;
    // we are active
    if (!myDidSpeedAdaption && calibrateSpeed && myCurrentStateInterval->v != mySegment->getEdge().getSpeedLimit()) {
        mySegment->getEdge().setMaxSpeed(myCurrentStateInterval->v);
        MESegment* first = MSGlobals::gMesoNet->getSegmentForEdge(mySegment->getEdge());
        while (first != nullptr) {
            first->setSpeed(myCurrentStateInterval->v, currentTime, -1);
            first = first->getNextSegment();
        }
        mySpeedIsDefault = false;
        myDidSpeedAdaption = true;
    }
    // clear invalid jams
    bool hadInvalidJam = false;
    while ((calibrateFlow || calibrateSpeed) && invalidJam()) {
        hadInvalidJam = true;
        if (!myHaveWarnedAboutClearingJam) {
            WRITE_WARNING("Clearing jam at calibrator '" + getID() + "' at time " + time2string(currentTime));
        }
        // remove one vehicle currently on the segment
        if (mySegment->vaporizeAnyCar(currentTime, this)) {
            myClearedInJam++;
        } else {
            if (!myHaveWarnedAboutClearingJam) {
                // this frequenly happens for very short edges
                WRITE_WARNING("Could not clear jam at calibrator '" + getID() + "' at time " + time2string(currentTime));
            }
            break;
        }
        myHaveWarnedAboutClearingJam = true;
    }
    if (calibrateFlow) {
        // flow calibration starts here ...
        // compute the number of vehicles that should have passed the calibrator within the time
        // rom begin of the interval
        const double totalHourFraction = STEPS2TIME(myCurrentStateInterval->end - myCurrentStateInterval->begin) / (double) 3600.;
        const int totalWishedNum = (int)std::floor(myCurrentStateInterval->q * totalHourFraction + 0.5); // round to closest int
        int adaptedNum = passed() + myClearedInJam;
        if (!hadInvalidJam) {
            // only add vehicles if we do not have an invalid upstream jam to prevent spill-back
            const double hourFraction = STEPS2TIME(currentTime - myCurrentStateInterval->begin + DELTA_T) / (double) 3600.;
            const int wishedNum = (int)std::floor(myCurrentStateInterval->q * hourFraction + 0.5); // round to closest int
            // only the difference between inflow and aspiredFlow should be added, thus
            // we should not count vehicles vaporized from a jam here
            // if we have enough time left we can add missing vehicles later
            const int relaxedInsertion = (int)std::floor(STEPS2TIME(myCurrentStateInterval->end - currentTime) / 3);
            const int insertionSlack = MAX2(0, adaptedNum + relaxedInsertion - totalWishedNum);
            // increase number of vehicles
            //std::cout << "time:" << STEPS2TIME(currentTime) << " w:" << wishedNum << " s:" << insertionSlack << " before:" << adaptedNum;
            while (wishedNum > adaptedNum + insertionSlack && remainingVehicleCapacity() > maximumInflow()) {
                SUMOVehicleParameter* pars = myCurrentStateInterval->vehicleParameter;
                const MSRoute* route = myProbe != nullptr ? myProbe->sampleRoute() : nullptr;
                if (route == nullptr) {
                    route = MSRoute::dictionary(pars->routeid);
                }
                if (route == nullptr) {
                    WRITE_WARNING("No valid routes in calibrator '" + getID() + "'.");
                    break;
                }
                if (!route->contains(myEdge)) {
                    WRITE_WARNING("Route '" + route->getID() + "' in calibrator '" + getID() + "' does not contain edge '" + myEdge->getID() + "'.");
                    break;
                }
                MSVehicleType* vtype = MSNet::getInstance()->getVehicleControl().getVType(pars->vtypeid);
                assert(route != 0 && vtype != 0);
                // build the vehicle
                const SUMOTime depart = mySegment->getNextInsertionTime(currentTime);
                SUMOVehicleParameter* newPars = new SUMOVehicleParameter(*pars);
                newPars->id = getNewVehicleID();
                newPars->depart = depart;
                newPars->routeid = route->getID();
                MEVehicle* vehicle;
                try {
                    vehicle = static_cast<MEVehicle*>(MSNet::getInstance()->getVehicleControl().buildVehicle(
                                                          newPars, route, vtype, false, false));
                } catch (const ProcessError& e) {
                    if (!MSGlobals::gCheckRoutes) {
                        WRITE_WARNING(e.what());
                        vehicle = nullptr;
                        break;
                    } else {
                        throw e;
                    }
                }
                vehicle->setSegment(mySegment); // needed or vehicle will not be registered (XXX why?)
                vehicle->setEventTime(currentTime); // XXX superfluous?
                // move vehicle forward when the route does not begin at the calibrator's edge
                const MSEdge* myedge = &mySegment->getEdge();
                bool atDest = false;
                while (vehicle->getEdge() != myedge) {
                    // let the vehicle move to the next edge
                    atDest = vehicle->moveRoutePointer();
                }
                // insert vehicle into the net
                if (atDest || !tryEmit(mySegment, vehicle)) {
                    //std::cout << "F ";
                    MSNet::getInstance()->getVehicleControl().deleteVehicle(vehicle, true);
                    break;
                }
                //std::cout << "I ";
                myInserted++;
                adaptedNum++;
            }
        }
        //std::cout << " after:" << adaptedNum << "\n";
        // we only remove vehicles once we really have to
        while (totalWishedNum < adaptedNum) {
            if (!mySegment->vaporizeAnyCar(currentTime, this)) {
                // @bug: short edges may be jumped in a single step, giving us no chance to remove a vehicle
                break;
            }
            myRemoved++;
            adaptedNum--;
        }
    }
    if (myCurrentStateInterval->end <= currentTime + myFrequency) {
        intervalEnd();
    }
    //assert(!invalidJam());
    if (invalidJam()) {
        WRITE_WARNING("DEBUG: Could not clear jam at calibrator '" + getID() + "' at time " + time2string(currentTime));
    }
    return myFrequency;
}


bool
METriggeredCalibrator::invalidJam() const {
    if (mySegment->getBruttoOccupancy() == 0.) {
        return false;
    }
    // maxSpeed reflects the calibration target
    const bool toSlow = mySegment->getMeanSpeed() < myInvalidJamThreshold * mySegment->getEdge().getSpeedLimit();
    return toSlow && remainingVehicleCapacity() < maximumInflow();
}


int
METriggeredCalibrator::remainingVehicleCapacity() const {
    const SUMOVehicleParameter* pars = myCurrentStateInterval->vehicleParameter;
    const MSVehicleType* vtype = MSNet::getInstance()->getVehicleControl().getVType(pars->vtypeid);
    return mySegment->remainingVehicleCapacity(vtype->getLengthWithGap());
}


void
METriggeredCalibrator::reset() {
    myEdgeMeanData.reset();
}


/****************************************************************************/
