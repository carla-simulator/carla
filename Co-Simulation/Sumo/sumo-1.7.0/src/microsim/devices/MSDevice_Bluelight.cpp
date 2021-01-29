/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2013-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSDevice_Bluelight.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @author  Laura Bieker
/// @date    01.06.2017
///
// A device for emergency vehicle. The behaviour of other traffic participants will be triggered with this device.
// For example building a rescue lane.
/****************************************************************************/
#include <config.h>

#include <utils/common/StringUtils.h>
#include <utils/options/OptionsCont.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/vehicle/SUMOVehicle.h>
#include <microsim/MSNet.h>
#include <microsim/MSLane.h>
#include <microsim/MSEdge.h>
#include <microsim/MSVehicle.h>
#include <microsim/lcmodels/MSAbstractLaneChangeModel.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/MSVehicleType.h>
#include "MSDevice_Tripinfo.h"
#include "MSDevice_Bluelight.h"

//#define DEBUG_BLUELIGHT

// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static initialisation methods
// ---------------------------------------------------------------------------
void
MSDevice_Bluelight::insertOptions(OptionsCont& oc) {
    oc.addOptionSubTopic("Bluelight Device");
    insertDefaultAssignmentOptions("bluelight", "Bluelight Device", oc);

    oc.doRegister("device.bluelight.reactiondist", new Option_Float(25.0));
    oc.addDescription("device.bluelight.reactiondist", "Bluelight Device", "Set the distance at which other drivers react to the blue light and siren sound");
}


void
MSDevice_Bluelight::buildVehicleDevices(SUMOVehicle& v, std::vector<MSVehicleDevice*>& into) {
    OptionsCont& oc = OptionsCont::getOptions();
    if (equippedByDefaultAssignmentOptions(oc, "bluelight", v, false)) {
        MSDevice_Bluelight* device = new MSDevice_Bluelight(v, "bluelight_" + v.getID(),
                getFloatParam(v, oc, "bluelight.reactiondist", oc.getFloat("device.bluelight.reactiondist"), false));
        into.push_back(device);
    }
}


// ---------------------------------------------------------------------------
// MSDevice_Bluelight-methods
// ---------------------------------------------------------------------------
MSDevice_Bluelight::MSDevice_Bluelight(SUMOVehicle& holder, const std::string& id,
                                       double reactionDist) :
    MSVehicleDevice(holder, id),
    myReactionDist(reactionDist) {
#ifdef DEBUG_BLUELIGHT
    std::cout << "initialized device '" << id << "' with myReactionDist=" << myReactionDist << "\n";
#endif
}


MSDevice_Bluelight::~MSDevice_Bluelight() {
}


bool
MSDevice_Bluelight::notifyMove(SUMOTrafficObject& veh, double /* oldPos */,
                               double /* newPos */, double newSpeed) {
#ifdef DEBUG_BLUELIGHT
    std::cout << "device '" << getID() << "' notifyMove: newSpeed=" << newSpeed << "\n";
#else
    UNUSED_PARAMETER(newSpeed);
#endif
    // check whether another device is present on the vehicle:
    /*MSDevice_Tripinfo* otherDevice = static_cast<MSDevice_Tripinfo*>(veh.getDevice(typeid(MSDevice_Tripinfo)));
    if (otherDevice != 0) {
        std::cout << "  veh '" << veh.getID() << " has device '" << otherDevice->getID() << "'\n";
    }*/
    //violate red lights  this only need to be done once so shift it todo
    MSVehicle& ego = dynamic_cast<MSVehicle&>(veh);
    MSVehicle::Influencer& redLight = ego.getInfluencer();
    redLight.setSpeedMode(7);
    if (veh.getWaitingTime() > TIME2STEPS(1)) {
        ego.getLaneChangeModel().setParameter(toString(SUMO_ATTR_LCA_STRATEGIC_PARAM), "-1");
    } else {
        ego.getLaneChangeModel().setParameter(toString(SUMO_ATTR_LCA_STRATEGIC_PARAM),
                                              ego.getVehicleType().getParameter().getLCParamString(SUMO_ATTR_LCA_STRATEGIC_PARAM, "1"));
    }
    // build a rescue lane for all vehicles on the route of the emergency vehicle within the range of the siren
    MSVehicleType* vt = MSNet::getInstance()->getVehicleControl().getVType(veh.getVehicleType().getID());
    vt->setPreferredLateralAlignment(LATALIGN_ARBITRARY);
    MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
    //std::string currentEdgeID = veh.getEdge()->getID();
    //use edges on the way of the emergency vehicle
    std::vector<const MSLane*> myUpcomingLanes = ego.getUpcomingLanesUntil(myReactionDist);
    std::vector<std::string> myUpcomingEdges;
    //get edgeIDs from Lanes
    for (const MSLane* const l :  myUpcomingLanes) {
        myUpcomingEdges.push_back(l->getEdge().getID());
    }
    for (MSVehicleControl::constVehIt vit = vc.loadedVehBegin(); vit != vc.loadedVehEnd(); ++vit) {
        MSVehicle* veh2 = dynamic_cast<MSVehicle*>(vit->second);
        assert(veh2 != nullptr);
        //Vehicle only from edge should react
        if (std::find(myUpcomingEdges.begin(), myUpcomingEdges.end(), veh2->getEdge()->getID()) != myUpcomingEdges.end()) { //currentEdgeID == veh2->getEdge()->getID()) {
            if (veh2->getDevice(typeid(MSDevice_Bluelight)) != nullptr) {
                // emergency vehicles should not react
                continue;
            }
            const int numLanes = (int)veh2->getEdge()->getLanes().size();
            //make sure that vehicle are still building the a rescue lane
            if (influencedVehicles.count(veh2->getID()) > 0) {
                //Vehicle gets a new Vehicletype to change the alignment and the lanechange options
                MSVehicleType& t = veh2->getSingularType();
                //Setting the lateral alignment to build a rescue lane
                if (veh2->getLane()->getIndex() == numLanes - 1) {
                    t.setPreferredLateralAlignment(LATALIGN_LEFT);
                    // the alignement is changet to left for the vehicle std::cout << "New alignment to left for vehicle: " << veh2->getID() << " " << veh2->getVehicleType().getPreferredLateralAlignment() << "\n";
                } else {
                    t.setPreferredLateralAlignment(LATALIGN_RIGHT);
                    // the alignement is changet to right for the vehicle std::cout << "New alignment to right for vehicle: " << veh2->getID() << " " << veh2->getVehicleType().getPreferredLateralAlignment() << "\n";
                }
            }

            double distanceDelta = veh.getPosition().distanceTo(veh2->getPosition());
            //emergency vehicle has to slow down when entering the resuce lane
            if (distanceDelta <= 10 && veh.getID() != veh2->getID() && influencedVehicles.count(veh2->getID()) > 0 && veh2->getSpeed() < 1) {
                // set ev speed to 20 km/h 0 5.56 m/s
                std::vector<std::pair<SUMOTime, double> > speedTimeLine;
                speedTimeLine.push_back(std::make_pair(MSNet::getInstance()->getCurrentTimeStep(), veh.getSpeed()));
                speedTimeLine.push_back(std::make_pair(MSNet::getInstance()->getCurrentTimeStep() + TIME2STEPS(2), 5.56));
                redLight.setSpeedTimeLine(speedTimeLine);
            }

            // the perception of the sound of the siren should be around 25 meters
            // todo only vehicles in front of the emergency vehicle should react
            if (distanceDelta <= myReactionDist && veh.getID() != veh2->getID() && influencedVehicles.count(veh2->getID()) == 0) {
                //online a percentage of vehicles should react to the emergency vehicle to make the behaviour more realistic
                double reaction = RandHelper::rand();
                MSVehicle::Influencer& lanechange = veh2->getInfluencer();

                //other vehicle should not use the rescue lane so they should not make any lane changes
                lanechange.setLaneChangeMode(1605);//todo change lane back
                //const int numLanes = (int)veh2->getEdge()->getLanes().size();
                // the vehicles should react according to the distance to the emergency vehicle taken from real world data
                double reactionProb = 0.189; // todo works only for one second steps
                if (distanceDelta < 12.5) {
                    reactionProb = 0.577;
                }
                if (reaction < reactionProb) {
                    influencedVehicles.insert(veh2->getID());
                    influencedTypes.insert(std::make_pair(veh2->getID(), veh2->getVehicleType().getID()));

                    //Vehicle gets a new Vehicletype to change the alignment and the lanechange options
                    MSVehicleType& t = veh2->getSingularType();
                    //Setting the lateral alignment to build a rescue lane
                    if (veh2->getLane()->getIndex() == numLanes - 1) {
                        t.setPreferredLateralAlignment(LATALIGN_LEFT);
                        // the alignement is changet to left for the vehicle std::cout << "New alignment to left for vehicle: " << veh2->getID() << " " << veh2->getVehicleType().getPreferredLateralAlignment() << "\n";
                    } else {
                        t.setPreferredLateralAlignment(LATALIGN_RIGHT);
                        // the alignement is changet to right for the vehicle std::cout << "New alignment to right for vehicle: " << veh2->getID() << " " << veh2->getVehicleType().getPreferredLateralAlignment() << "\n";
                    }
                    // disable strategic lane-changing
                    veh2->getLaneChangeModel().setParameter(toString(SUMO_ATTR_LCA_STRATEGIC_PARAM), "-1");
                }
            }

        } else { //if vehicle is passed all vehicles which had to react should get their state back after they leave the communication range
            if (influencedVehicles.count(veh2->getID()) > 0) {
                double distanceDelta = veh.getPosition().distanceTo(veh2->getPosition());
                if (distanceDelta > myReactionDist && veh.getID() != veh2->getID()) {
                    influencedVehicles.erase(veh2->getID());
                    std::map<std::string, std::string>::iterator it = influencedTypes.find(veh2->getID());
                    if (it != influencedTypes.end()) {
                        // The vehicle gets back its old VehicleType after the emergency vehicle have passed them
                        MSVehicleType* targetType = MSNet::getInstance()->getVehicleControl().getVType(it->second);
                        //targetType is nullptr if the vehicle type has already changed to its old vehicleType
                        if (targetType != nullptr) {
                            veh2->replaceVehicleType(targetType);
                            veh2->getLaneChangeModel().setParameter(toString(SUMO_ATTR_LCA_STRATEGIC_PARAM),
                                                                    targetType->getParameter().getLCParamString(SUMO_ATTR_LCA_STRATEGIC_PARAM, "1"));
                        }
                    }
                }
            }
        }
    }
    return true; // keep the device
}


bool
MSDevice_Bluelight::notifyEnter(SUMOTrafficObject& veh, MSMoveReminder::Notification reason, const MSLane* /* enteredLane */) {
#ifdef DEBUG_BLUELIGHT
    std::cout << "device '" << getID() << "' notifyEnter: reason=" << reason << " currentEdge=" << veh.getEdge()->getID() << "\n";
#else
    UNUSED_PARAMETER(veh);
    UNUSED_PARAMETER(reason);
#endif
    return true; // keep the device
}


bool
MSDevice_Bluelight::notifyLeave(SUMOTrafficObject& veh, double /*lastPos*/, MSMoveReminder::Notification reason, const MSLane* /* enteredLane */) {
#ifdef DEBUG_BLUELIGHT
    std::cout << "device '" << getID() << "' notifyLeave: reason=" << reason << " currentEdge=" << veh.getEdge()->getID() << "\n";
#else
    UNUSED_PARAMETER(veh);
    UNUSED_PARAMETER(reason);
#endif
    return true; // keep the device
}


void
MSDevice_Bluelight::generateOutput(OutputDevice* tripinfoOut) const {
    if (tripinfoOut != nullptr) {
        tripinfoOut->openTag("bluelight");
        tripinfoOut->closeTag();
    }
}

std::string
MSDevice_Bluelight::getParameter(const std::string& key) const {
    if (key == "reactiondist") {
        return toString(myReactionDist);
    }
    throw InvalidArgument("Parameter '" + key + "' is not supported for device of type '" + deviceName() + "'");
}


void
MSDevice_Bluelight::setParameter(const std::string& key, const std::string& value) {
    double doubleValue;
    try {
        doubleValue = StringUtils::toDouble(value);
    } catch (NumberFormatException&) {
        throw InvalidArgument("Setting parameter '" + key + "' requires a number for device of type '" + deviceName() + "'");
    }
    if (key == "reactiondist") {
        myReactionDist = doubleValue;
    } else {
        throw InvalidArgument("Setting parameter '" + key + "' is not supported for device of type '" + deviceName() + "'");
    }
}


/****************************************************************************/
