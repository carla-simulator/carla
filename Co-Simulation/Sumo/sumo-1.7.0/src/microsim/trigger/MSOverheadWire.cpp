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
/// @file    MSOverheadWire.cpp
/// @author  Jakub Sevcik (RICE)
/// @author  Jan Prikryl (RICE)
/// @date    2019-12-15
///
// Overhead wires for Electric (equipped with elecHybrid device) vehicles (Overhead wire segments, overhead wire sections, traction substations)
/****************************************************************************/
#include <config.h>

#include <cassert>
#include <tuple>
#include <mutex>
#include <string.h>

#include <utils/vehicle/SUMOVehicle.h>
#include <utils/common/ToString.h>
#include <microsim/MSVehicleType.h>
#include <microsim/MSStoppingPlace.h>
#include <microsim/MSJunction.h>
#include <microsim/MSLane.h>
#include <microsim/MSLink.h>
#include <microsim/MSLinkCont.h>
#include <microsim/MSNet.h>
#include <microsim/devices/MSDevice_ElecHybrid.h>

// due to gOverheadWireSolver
#include <microsim/MSGlobals.h>

// due to solving circuit as endEndOfTimestepEvents
#include <utils/common/StaticCommand.h>
#include <utils/common/WrappingCommand.h>
#include <microsim/MSEventControl.h>

#include "MSOverheadWire.h"
#include "MSTrigger.h"


Command* MSTractionSubstation::myCommandForSolvingCircuit = nullptr;
std::mutex ow_mutex;

// ===========================================================================
// member method definitions
// ===========================================================================

MSOverheadWire::MSOverheadWire(const std::string& overheadWireSegmentID, MSLane& lane, double startPos, double endPos, bool voltageSource) :
    MSStoppingPlace(overheadWireSegmentID, std::vector<std::string>(), lane, startPos, endPos),
    myVoltage(0),
    myChargingVehicle(false),
    myTotalCharge(0),
    myChargingVehicles({}),
                   //TODORICE: some better structure with circuit pointers
                   myTractionSubstation(nullptr),
                   myVoltageSource(voltageSource),
                   myCircuitElementPos(nullptr),
                   myCircuitStartNodePos(nullptr),
myCircuitEndNodePos(nullptr) {
    if (getBeginLanePosition() > getEndLanePosition()) {
        WRITE_WARNING(toString(SUMO_TAG_OVERHEAD_WIRE_SEGMENT) + " with ID = " + getID() + " doesn't have a valid range (" + toString(getBeginLanePosition()) + " < " + toString(getEndLanePosition()) + ").");
    }
}

MSOverheadWire::~MSOverheadWire() {
    if (myTractionSubstation != nullptr) {
        if (myTractionSubstation->getCircuit() != nullptr && myCircuitElementPos != nullptr && myCircuitElementPos->getPosNode() == myCircuitStartNodePos && myCircuitElementPos->getNegNode() == myCircuitEndNodePos) {
            myCircuitElementPos->getPosNode()->eraseElement(myCircuitElementPos);
            myCircuitElementPos->getNegNode()->eraseElement(myCircuitElementPos);
            if (myCircuitEndNodePos->getElements()->size() == 0) {
                myTractionSubstation->getCircuit()->eraseNode(myCircuitEndNodePos);
                delete myCircuitEndNodePos;
            }
            if (myCircuitStartNodePos->getElements()->size() == 0) {
                myTractionSubstation->getCircuit()->eraseNode(myCircuitStartNodePos);
                delete myCircuitStartNodePos;
            }
            myTractionSubstation->getCircuit()->eraseElement(myCircuitElementPos);
            delete myCircuitElementPos;
        }

        if (myTractionSubstation->numberOfOverheadSegments() <= 1) {
            myTractionSubstation->eraseOverheadWireSegmentFromCircuit(this);
            //delete myTractionSubstation;
        } else {
            myTractionSubstation->eraseOverheadWireSegmentFromCircuit(this);
        }
    }
}


MSTractionSubstation::MSTractionSubstation(const std::string& substationId, double voltage) :
    Named(substationId),
    mySubstationVoltage(voltage),
    myChargingVehicle(false),
    myElecHybridCount(0),
    myOverheadWireSegments(),
    myElecHybrid(),
    myCircuit(new Circuit()),
    myForbiddenLanes(),
    myOverheadWireClamps() {
}


void
MSTractionSubstation::addOverheadWireSegmentToCircuit(MSOverheadWire* newOverheadWireSegment) {
    MSLane& lane = const_cast<MSLane&>(newOverheadWireSegment->getLane());
    if (lane.isInternal()) {
        return;
    }

    myOverheadWireSegments.push_back(newOverheadWireSegment);
    newOverheadWireSegment->setTractionSubstation(this);

    if (MSGlobals::gOverheadWireSolver) {
#ifdef HAVE_EIGEN
        if (newOverheadWireSegment->getCircuit()->getNode("negNode_ground") == nullptr) {
            newOverheadWireSegment->getCircuit()->addNode("negNode_ground");
        }

        // convention: pNode is at the beginning of the wire segment, nNode is at the end of the wire segment
        newOverheadWireSegment->setCircuitStartNodePos(newOverheadWireSegment->getCircuit()->addNode("pNode_pos_" + newOverheadWireSegment->getID()));
        newOverheadWireSegment->setCircuitEndNodePos(newOverheadWireSegment->getCircuit()->addNode("nNode_pos_" + newOverheadWireSegment->getID()));
        newOverheadWireSegment->setCircuitElementPos(newOverheadWireSegment->getCircuit()->addElement("pos_" + newOverheadWireSegment->getID(), (newOverheadWireSegment->getLane().getLength())*WIRE_RESISTIVITY, newOverheadWireSegment->getCircuitStartNodePos(), newOverheadWireSegment->getCircuitEndNodePos(), Element::ElementType::RESISTOR_traction_wire));
#else
        WRITE_WARNING("Overhead circuit solver requested, but solver support (Eigen) not compiled in.");
#endif
    }

    const MSLane* connection = nullptr;
    std::string ovrhdSegmentID = ""; //ID of outgoing or incoming overhead wire segment
    MSOverheadWire* ovrhdSegment = nullptr; //pointer to outgoing or incoming overhead wire segment

    /* in version before SUMO 1.0.1 the function getOutgoingLanes() returning MSLane* exists,
       in new version of SUMO the funciton getOutgoingViaLanes() returning MSLane* and MSEdge* pair exists */
    // std::vector<const MSLane*> outgoing = lane.getOutgoingLanes();
    const std::vector<std::pair<const MSLane*, const MSEdge*> > outgoingLanesAndEdges = lane.getOutgoingViaLanes();
    std::vector<const MSLane*> neigboringInnerLanes;
    neigboringInnerLanes.reserve(outgoingLanesAndEdges.size());
    for (size_t it = 0; it < outgoingLanesAndEdges.size(); ++it) {
        neigboringInnerLanes.push_back(outgoingLanesAndEdges[it].first);
    }

    // Check if there is an overhead wire segment on the outgoing lane. If not, do nothing, otherwise find connnecting internal lanes and
    // add all lane (this and inner) to circuit
    for (std::vector<const MSLane*>::iterator it = neigboringInnerLanes.begin(); it != neigboringInnerLanes.end(); ++it) {
        ovrhdSegmentID = MSNet::getInstance()->getStoppingPlaceID(*it, NUMERICAL_EPS, SUMO_TAG_OVERHEAD_WIRE_SEGMENT);
        // If the overhead wire segment is over the outgoing (not internal) lane
        if (ovrhdSegmentID != "" && !(*it)->isInternal()) {
            ovrhdSegment = dynamic_cast<MSOverheadWire*>(MSNet::getInstance()->getStoppingPlace(ovrhdSegmentID, SUMO_TAG_OVERHEAD_WIRE_SEGMENT));
            // If the outgoing overhead wire segment belongs to the same substation as newOverheadWireSegment
            if (ovrhdSegment->getTractionSubstation() == newOverheadWireSegment->getTractionSubstation()) {
                connection = MSLinkContHelper::getInternalFollowingLane(&lane, *it);
                if (connection != nullptr) {
                    //is connection a forbidden lane?
                    if (!(ovrhdSegment->getTractionSubstation()->isForbidden(connection) ||
                            ovrhdSegment->getTractionSubstation()->isForbidden(MSLinkContHelper::getInternalFollowingLane(&lane, connection)) ||
                            ovrhdSegment->getTractionSubstation()->isForbidden(MSLinkContHelper::getInternalFollowingLane(connection, *it)))) {
                        addOverheadWireInnerSegmentToCircuit(newOverheadWireSegment, ovrhdSegment, connection, MSLinkContHelper::getInternalFollowingLane(&lane, connection), MSLinkContHelper::getInternalFollowingLane(connection, *it));
                    }

                } else {
                    if (MSGlobals::gOverheadWireSolver) {
#ifdef HAVE_EIGEN
                        Node* const unusedNode = newOverheadWireSegment->getCircuitEndNodePos();
                        for (MSOverheadWire* const wire : myOverheadWireSegments) {
                            if (wire->getCircuitStartNodePos() == unusedNode) {
                                wire->setCircuitStartNodePos(ovrhdSegment->getCircuitStartNodePos());
                            }
                            if (wire->getCircuitEndNodePos() == unusedNode) {
                                wire->setCircuitEndNodePos(ovrhdSegment->getCircuitStartNodePos());
                            }
                        }
                        newOverheadWireSegment->getCircuit()->replaceAndDeleteNode(unusedNode, ovrhdSegment->getCircuitStartNodePos());
                        // newOverheadWireSegment->getCircuitElementPos()->setPosNode(ovrhdSegment->getCircuitEndNodePos());
                        // newOverheadWireSegment->setCircuitEndNodePos(ovrhdSegment->getCircuitStartNodePos());
                        // TODORICE change voltageSource pNode if necessary
#else
                        WRITE_WARNING("Overhead circuit solver requested, but solver support (Eigen) not compiled in.");
#endif
                    }
                }
            }
        }
    }

    // Check if an incoming lane has an overhead wire segment. If not, do nothing, otherwise find connnecting internal lanes and
    // add all lane (this, incoming and inner)to circuit
    neigboringInnerLanes = lane.getNormalIncomingLanes();
    for (std::vector<const MSLane*>::iterator it = neigboringInnerLanes.begin(); it != neigboringInnerLanes.end(); ++it) {
        ovrhdSegmentID = MSNet::getInstance()->getStoppingPlaceID(*it, (*it)->getLength() - NUMERICAL_EPS, SUMO_TAG_OVERHEAD_WIRE_SEGMENT);
        // If the overhead wire segment is over the incoming (not internal) lane
        if (ovrhdSegmentID != "" && !(*it)->isInternal()) {
            ovrhdSegment = dynamic_cast<MSOverheadWire*>(MSNet::getInstance()->getStoppingPlace(ovrhdSegmentID, SUMO_TAG_OVERHEAD_WIRE_SEGMENT));
            // If the incoming overhead wire segment belongs to the same substation as newOverheadWireSegment
            if (ovrhdSegment->getTractionSubstation() == newOverheadWireSegment->getTractionSubstation()) {
                connection = MSLinkContHelper::getInternalFollowingLane((*it), &lane);
                if (connection != nullptr) {
                    //is connection a forbidden lane?
                    if (!(ovrhdSegment->getTractionSubstation()->isForbidden(connection) ||
                            ovrhdSegment->getTractionSubstation()->isForbidden(MSLinkContHelper::getInternalFollowingLane(*it, connection)) ||
                            ovrhdSegment->getTractionSubstation()->isForbidden(MSLinkContHelper::getInternalFollowingLane(connection, &lane)))) {
                        addOverheadWireInnerSegmentToCircuit(ovrhdSegment, newOverheadWireSegment, connection, MSLinkContHelper::getInternalFollowingLane((*it), connection), MSLinkContHelper::getInternalFollowingLane(connection, &lane));
                    }
                } else {
                    if (MSGlobals::gOverheadWireSolver) {
#ifdef HAVE_EIGEN
                        Node* const unusedNode = newOverheadWireSegment->getCircuitStartNodePos();
                        for (MSOverheadWire* const ows : myOverheadWireSegments) {
                            if (ows->getCircuitStartNodePos() == unusedNode) {
                                ows->setCircuitStartNodePos(ovrhdSegment->getCircuitEndNodePos());
                            }
                            if (ows->getCircuitEndNodePos() == unusedNode) {
                                ows->setCircuitEndNodePos(ovrhdSegment->getCircuitEndNodePos());
                            }
                        }
                        newOverheadWireSegment->getCircuit()->replaceAndDeleteNode(unusedNode, ovrhdSegment->getCircuitEndNodePos());
                        //newOverheadWireSegment->getCircuitElementPos()->setPosNode(ovrhdSegment->getCircuitEndNodePos());
                        //newOverheadWireSegment->setCircuitStartNode_pos(ovrhdSegment->getCircuitEndNodePos());
                        // TODORICE change voltageSource pNode if necessary
#else
                        WRITE_WARNING("Overhead circuit solver requested, but solver support (Eigen) not compiled in.");
#endif
                    }
                }
            }
        }
    }
    if (MSGlobals::gOverheadWireSolver && newOverheadWireSegment->isThereVoltageSource()) {
#ifdef HAVE_EIGEN
        newOverheadWireSegment->getCircuit()->addElement("voltage_source_" + newOverheadWireSegment->getID(), mySubstationVoltage, newOverheadWireSegment->getCircuitStartNodePos(), newOverheadWireSegment->getCircuit()->getNode("negNode_ground"), Element::ElementType::VOLTAGE_SOURCE_traction_wire);
#else
        WRITE_WARNING("Overhead circuit solver requested, but solver support (Eigen) not compiled in.");
#endif
    }
}


void
MSTractionSubstation::addOverheadWireInnerSegmentToCircuit(MSOverheadWire* incomingSegment, MSOverheadWire* outgoingSegment, const MSLane* connection, const MSLane* frontConnection, const MSLane* behindConnection) {
    if (frontConnection == nullptr && behindConnection == nullptr) {
        //addOverheadWire from nNode of newOverheadWireSegment to pNode
        //TODORICE
        //what happens if getCircuit is different? (do not check TractionSubstation but rather getCircuit)
        //what happens if the segment is not at the all length of the specific lane
        //what happens if more segments belong to the one lane
        MSOverheadWire* innerSegment = dynamic_cast<MSOverheadWire*>(MSNet::getInstance()->getStoppingPlace("ovrhd_inner_" + connection->getID(), SUMO_TAG_OVERHEAD_WIRE_SEGMENT));
        myOverheadWireSegments.push_back(innerSegment);
        innerSegment->setTractionSubstation(incomingSegment->getTractionSubstation());
        if (MSGlobals::gOverheadWireSolver) {
#ifdef HAVE_EIGEN
            Element* elem = incomingSegment->getCircuit()->addElement("pos_ovrhd_inner_" + connection->getID(), (connection->getLength()) * WIRE_RESISTIVITY, incomingSegment->getCircuitEndNodePos(), outgoingSegment->getCircuitStartNodePos(), Element::ElementType::RESISTOR_traction_wire);
            innerSegment->setCircuitElementPos(elem);
            innerSegment->setCircuitStartNodePos(incomingSegment->getCircuitEndNodePos());
            innerSegment->setCircuitEndNodePos(outgoingSegment->getCircuitStartNodePos());
#else
            UNUSED_PARAMETER(outgoingSegment);
            WRITE_WARNING("Overhead circuit solver requested, but solver support (Eigen) not compiled in.");
#endif
        }
    } else if (frontConnection != nullptr && behindConnection == nullptr) {
        MSOverheadWire* innerSegment = dynamic_cast<MSOverheadWire*>(MSNet::getInstance()->getStoppingPlace("ovrhd_inner_" + frontConnection->getID(), SUMO_TAG_OVERHEAD_WIRE_SEGMENT));
        MSOverheadWire* innerSegment2 = dynamic_cast<MSOverheadWire*>(MSNet::getInstance()->getStoppingPlace("ovrhd_inner_" + connection->getID(), SUMO_TAG_OVERHEAD_WIRE_SEGMENT));

        innerSegment->setTractionSubstation(incomingSegment->getTractionSubstation());
        myOverheadWireSegments.push_back(innerSegment);
        innerSegment2->setTractionSubstation(incomingSegment->getTractionSubstation());
        myOverheadWireSegments.push_back(innerSegment2);

        if (MSGlobals::gOverheadWireSolver) {
#ifdef HAVE_EIGEN
            Node* betweenFrontNode_pos = incomingSegment->getCircuit()->addNode("betweenFrontNode_pos_" + connection->getID());
            Element* elem = incomingSegment->getCircuit()->addElement("pos_ovrhd_inner_" + frontConnection->getID(), (frontConnection->getLength()) * WIRE_RESISTIVITY, incomingSegment->getCircuitEndNodePos(), betweenFrontNode_pos, Element::ElementType::RESISTOR_traction_wire);
            Element* elem2 = incomingSegment->getCircuit()->addElement("pos_ovrhd_inner_" + connection->getID(), (connection->getLength()) * WIRE_RESISTIVITY, betweenFrontNode_pos, outgoingSegment->getCircuitStartNodePos(), Element::ElementType::RESISTOR_traction_wire);

            innerSegment->setCircuitElementPos(elem);
            innerSegment->setCircuitStartNodePos(incomingSegment->getCircuitEndNodePos());
            innerSegment->setCircuitEndNodePos(betweenFrontNode_pos);

            innerSegment2->setCircuitElementPos(elem2);
            innerSegment2->setCircuitStartNodePos(betweenFrontNode_pos);
            innerSegment2->setCircuitEndNodePos(outgoingSegment->getCircuitStartNodePos());
#else
            WRITE_WARNING("Overhead circuit solver requested, but solver support (Eigen) not compiled in.");
#endif
        }
    } else if (frontConnection == nullptr && behindConnection != nullptr) {
        MSOverheadWire* innerSegment = dynamic_cast<MSOverheadWire*>(MSNet::getInstance()->getStoppingPlace("ovrhd_inner_" + connection->getID(), SUMO_TAG_OVERHEAD_WIRE_SEGMENT));
        MSOverheadWire* innerSegment2 = dynamic_cast<MSOverheadWire*>(MSNet::getInstance()->getStoppingPlace("ovrhd_inner_" + behindConnection->getID(), SUMO_TAG_OVERHEAD_WIRE_SEGMENT));

        innerSegment->setTractionSubstation(incomingSegment->getTractionSubstation());
        myOverheadWireSegments.push_back(innerSegment);
        innerSegment2->setTractionSubstation(incomingSegment->getTractionSubstation());
        myOverheadWireSegments.push_back(innerSegment2);

        if (MSGlobals::gOverheadWireSolver) {
#ifdef HAVE_EIGEN
            Node* betweenBehindNode_pos = incomingSegment->getCircuit()->addNode("betweenBehindNode_pos_" + connection->getID());
            Element* elem = incomingSegment->getCircuit()->addElement("pos_ovrhd_inner_" + connection->getID(), (connection->getLength()) * WIRE_RESISTIVITY, incomingSegment->getCircuitEndNodePos(), betweenBehindNode_pos, Element::ElementType::RESISTOR_traction_wire);
            Element* elem2 = incomingSegment->getCircuit()->addElement("pos_ovrhd_inner_" + behindConnection->getID(), (behindConnection->getLength()) * WIRE_RESISTIVITY, betweenBehindNode_pos, outgoingSegment->getCircuitStartNodePos(), Element::ElementType::RESISTOR_traction_wire);

            innerSegment->setCircuitElementPos(elem);
            innerSegment->setCircuitStartNodePos(incomingSegment->getCircuitEndNodePos());
            innerSegment->setCircuitEndNodePos(betweenBehindNode_pos);

            innerSegment2->setCircuitElementPos(elem2);
            innerSegment2->setCircuitStartNodePos(betweenBehindNode_pos);
            innerSegment2->setCircuitEndNodePos(outgoingSegment->getCircuitStartNodePos());
#else
            WRITE_WARNING("Overhead circuit solver requested, but solver support (Eigen) not compiled in.");
#endif
        }
    } else if (frontConnection != nullptr && behindConnection != nullptr) {
        MSOverheadWire* innerSegment = dynamic_cast<MSOverheadWire*>(MSNet::getInstance()->getStoppingPlace("ovrhd_inner_" + frontConnection->getID(), SUMO_TAG_OVERHEAD_WIRE_SEGMENT));
        MSOverheadWire* innerSegment2 = dynamic_cast<MSOverheadWire*>(MSNet::getInstance()->getStoppingPlace("ovrhd_inner_" + connection->getID(), SUMO_TAG_OVERHEAD_WIRE_SEGMENT));
        MSOverheadWire* innerSegment3 = dynamic_cast<MSOverheadWire*>(MSNet::getInstance()->getStoppingPlace("ovrhd_inner_" + behindConnection->getID(), SUMO_TAG_OVERHEAD_WIRE_SEGMENT));

        innerSegment->setTractionSubstation(incomingSegment->getTractionSubstation());
        myOverheadWireSegments.push_back(innerSegment);
        innerSegment2->setTractionSubstation(incomingSegment->getTractionSubstation());
        myOverheadWireSegments.push_back(innerSegment2);
        innerSegment3->setTractionSubstation(incomingSegment->getTractionSubstation());
        myOverheadWireSegments.push_back(innerSegment3);

        if (MSGlobals::gOverheadWireSolver) {
#ifdef HAVE_EIGEN
            Node* betweenFrontNode_pos = incomingSegment->getCircuit()->addNode("betweenFrontNode_pos_" + connection->getID());
            Node* betweenBehindNode_pos = incomingSegment->getCircuit()->addNode("betweenBehindNode_pos_" + connection->getID());
            Element* elem = incomingSegment->getCircuit()->addElement("pos_ovrhd_inner_" + frontConnection->getID(), (frontConnection->getLength()) * WIRE_RESISTIVITY, incomingSegment->getCircuitEndNodePos(), betweenFrontNode_pos, Element::ElementType::RESISTOR_traction_wire);
            Element* elem2 = incomingSegment->getCircuit()->addElement("pos_ovrhd_inner_" + connection->getID(), (connection->getLength()) * WIRE_RESISTIVITY, betweenFrontNode_pos, betweenBehindNode_pos, Element::ElementType::RESISTOR_traction_wire);
            Element* elem3 = incomingSegment->getCircuit()->addElement("pos_ovrhd_inner_" + behindConnection->getID(), (behindConnection->getLength()) * WIRE_RESISTIVITY, betweenBehindNode_pos, outgoingSegment->getCircuitStartNodePos(), Element::ElementType::RESISTOR_traction_wire);

            innerSegment->setCircuitElementPos(elem);
            innerSegment->setCircuitStartNodePos(incomingSegment->getCircuitEndNodePos());
            innerSegment->setCircuitEndNodePos(betweenFrontNode_pos);

            innerSegment2->setCircuitElementPos(elem2);
            innerSegment2->setCircuitStartNodePos(betweenFrontNode_pos);
            innerSegment2->setCircuitEndNodePos(betweenBehindNode_pos);

            innerSegment3->setCircuitElementPos(elem3);
            innerSegment3->setCircuitStartNodePos(betweenBehindNode_pos);
            innerSegment3->setCircuitEndNodePos(outgoingSegment->getCircuitStartNodePos());
#else
            WRITE_WARNING("Overhead circuit solver requested, but solver support not compiled in.");
#endif
        }
    }
}

void MSTractionSubstation::addOverheadWireClampToCircuit(const std::string id, MSOverheadWire* startSegment, MSOverheadWire* endSegment) {
    PositionVector pos_start = startSegment->getLane().getShape();
    PositionVector pos_end = endSegment->getLane().getShape();
    double distance = pos_start[0].distanceTo2D(pos_end.back());

    if (distance > 10) {
        WRITE_WARNING("The distance between two overhead wires during adding overhead wire clamp '" + id + "' defined for traction substation '" + startSegment->getTractionSubstation()->getID() + "' is " + toString(distance) + " m.")
    }
    getCircuit()->addElement(id, distance * WIRE_RESISTIVITY, startSegment->getCircuitStartNodePos(), endSegment->getCircuitEndNodePos(), Element::ElementType::RESISTOR_traction_wire);
}

void
MSTractionSubstation::eraseOverheadWireSegmentFromCircuit(MSOverheadWire* oldSegment) {
    //myOverheadWireSegments.push_back(static_cast<MSOverheadWire*>(MSNet::getInstance()->getStoppingPlace(overheadWireSegmentID, SUMO_TAG_OVERHEAD_WIRE_SEGMENT)));
    myOverheadWireSegments.erase(std::remove(myOverheadWireSegments.begin(), myOverheadWireSegments.end(), oldSegment), myOverheadWireSegments.end());
}

void
MSOverheadWire::addVehicle(SUMOVehicle& veh) {
    std::lock_guard<std::mutex> guard(ow_mutex);
    setChargingVehicle(true);
    myChargingVehicles.push_back(&veh);
    sort(myChargingVehicles.begin(), myChargingVehicles.end(), vehicle_position_sorter());
}

void
MSOverheadWire::eraseVehicle(SUMOVehicle& veh) {
    std::lock_guard<std::mutex> guard(ow_mutex);
    myChargingVehicles.erase(std::remove(myChargingVehicles.begin(), myChargingVehicles.end(), &veh), myChargingVehicles.end());
    if (myChargingVehicles.size() == 0) {
        setChargingVehicle(false);
    }
    //sort(myChargingVehicles.begin(), myChargingVehicles.end(), vehicle_position_sorter());
}

void
MSOverheadWire::lock() const {
    ow_mutex.lock();
}

void
MSOverheadWire::unlock() const {
    ow_mutex.unlock();
}

void
MSTractionSubstation::addVehicle(MSDevice_ElecHybrid* elecHybrid) {
    myElecHybrid.push_back(elecHybrid);
}

void
MSTractionSubstation::eraseVehicle(MSDevice_ElecHybrid* veh) {
    myElecHybrid.erase(std::remove(myElecHybrid.begin(), myElecHybrid.end(), veh), myElecHybrid.end());
}

void
MSTractionSubstation::writeOut() {
    std::cout << "substation " << getID() << " constrols segments: \n";
    for (std::vector<MSOverheadWire*>::iterator it = myOverheadWireSegments.begin(); it != myOverheadWireSegments.end(); ++it) {
        std::cout << "        " << (*it)->getOverheadWireSegmentName() << "\n";
    }
}


std::string MSOverheadWire::getOverheadWireSegmentName() {
    return toString(getID());
}

MSTractionSubstation::~MSTractionSubstation() {
}

Circuit*
MSOverheadWire::getCircuit() const {
    if (getTractionSubstation() != nullptr) {
        return getTractionSubstation()->getCircuit();
    }
    return nullptr;
}

double
MSOverheadWire::getVoltage() const {
    return myVoltage;
}

void
MSOverheadWire::setVoltage(double voltage) {
    if (voltage < 0) {
        WRITE_WARNING("New " + toString(SUMO_ATTR_VOLTAGE) + " for " + toString(SUMO_TAG_OVERHEAD_WIRE_SEGMENT) + " with ID = " + getID() + " isn't valid (" + toString(voltage) + ").")
    } else {
        myVoltage = voltage;
    }
}

void
MSOverheadWire::setChargingVehicle(bool value) {
    myChargingVehicle = value;
}


void
MSTractionSubstation::setChargingVehicle(bool value) {
    myChargingVehicle = value;
}

bool
MSOverheadWire::vehicleIsInside(const double position) const {
    if ((position >= getBeginLanePosition()) && (position <= getEndLanePosition())) {
        return true;
    } else {
        return false;
    }
}


bool
MSOverheadWire::isCharging() const {
    return myChargingVehicle;
}

bool
MSTractionSubstation::isCharging() const {
    return myChargingVehicle;
}

void
MSTractionSubstation::increaseElecHybridCount() {
    myElecHybridCount++ ;
}

void
MSTractionSubstation::decreaseElecHybridCount() {
    myElecHybridCount-- ;
}

void MSTractionSubstation::addForbiddenLane(MSLane* lane) {
    myForbiddenLanes.push_back(lane);
}

bool MSTractionSubstation::isForbidden(const MSLane* lane) {
    for (std::vector<MSLane*>::iterator it = myForbiddenLanes.begin(); it != myForbiddenLanes.end(); ++it) {
        if (lane == (*it)) {
            return true;
        }
    }
    return false;
}

void
MSTractionSubstation::addClamp(const std::string& id, MSOverheadWire* startPos, MSOverheadWire* endPos) {
    OverheadWireClamp clamp(id, startPos, endPos, false);
    myOverheadWireClamps.push_back(clamp);
}

MSTractionSubstation::OverheadWireClamp*
MSTractionSubstation::findClamp(std::string clampId) {
    for (auto it = myOverheadWireClamps.begin(); it != myOverheadWireClamps.end(); it++) {
        if (it->id == clampId) {
            return &(*it);
        }
    }
    return nullptr;
}

bool
MSTractionSubstation::isAnySectionPreviouslyDefined() {
    if (myOverheadWireSegments.size() > 0 || myForbiddenLanes.size() > 0 || getCircuit()->getLastId() > 0) {
        return true;
    }
    return false;
}

void
MSTractionSubstation::addSolvingCirucitToEndOfTimestepEvents() {
    if (!myChargingVehicle) {
        // myCommandForSolvingCircuit = new StaticCommand<MSTractionSubstation>(&MSTractionSubstation::solveCircuit);
        myCommandForSolvingCircuit = new WrappingCommand<MSTractionSubstation>(this, &MSTractionSubstation::solveCircuit);
        MSNet::getInstance()->getEndOfTimestepEvents()->addEvent(myCommandForSolvingCircuit);
        setChargingVehicle(true);
    }
}

SUMOTime
MSTractionSubstation::solveCircuit(SUMOTime /*currentTime*/) {
    /*Circuit evaluation*/
    setChargingVehicle(false);

#ifdef HAVE_EIGEN
    myCircuit->solve();

    // RICE_TODO: This is a relict of original code without alpha scaling, shall we still keep it here?
    vector<Element*>* elecHybridSources = myCircuit->getCurrentSources();
    double err = 0.0;
    for (auto* it : *elecHybridSources) {
        if (!ISNAN(it->getPowerWanted())) {
            err = MAX2(abs(myCircuit->alphaBest * (it->getPowerWanted() - (-it->getCurrent()) * it->getVoltage())), err);
        }
    }

    // if (err > tolerance || veh_elem->getCurrent()*veh_elem->getVoltage() == NAN) {
    //    WRITE_WARNING("The tolerance of circuit evaluation was not reached until 10 iteration (err = " + toString(err) + "). The tolerance " + toString(tolerance) + " was not reached in iter = " + toString(15));
    //}

    if (getCircuit()->alphaBest != 1.0) {
        WRITE_WARNING("The requested total power could not be delivered by the overhead wire. Only " + toString(getCircuit()->alphaBest) + " of originally requested power was provided.");
    }
#endif

    for (auto* it : myElecHybrid) {

        Element* vehElem = it->getVehElem();
        double voltage = vehElem->getVoltage();
        double current = -vehElem->getCurrent();  // Vehicle is a power source, hence its current flows in opposite direction

        it->setCurrentFromOverheadWire(current);
        it->setVoltageOfOverheadWire(voltage);

        // Calulate energy charged
        //
        // RICE_TODO: This does not take into account the fact that `energyIn` may be lower than the energy requested
        // due to `alpha` scaling ...
        //
        // myEnergyCharged = (TS * voltage * -current * myActOverheadWireSegment->getEfficency()) - (myConsum * 3600);
        // double energyCharged = (TS * vehElem->getPowerWanted() / 3600) - (it->getConsum());
        //
        double energyIn = WATT2WATTHR(voltage * current);  // [Wh]
        double energyCharged = energyIn - it->getConsum();  // [Wh]

        // Convert from [Ws] to [Wh] (3600s / 1h):
        it->setEnergyCharged(energyCharged);

        // Update Battery charge
        it->setActualBatteryCapacity(it->getActualBatteryCapacity() + energyCharged);
        // add charge value for output to myActOverheadWireSegment
        it->getActOverheadWireSegment()->addChargeValueForOutput(energyCharged + it->getConsum(), it);
    }

    return 0;
}

void
MSOverheadWire::addChargeValueForOutput(double WCharged, MSDevice_ElecHybrid* elecHybrid, bool ischarging) {
    std::string status = "charging";
    if (!ischarging) {
        status = "not-charging";
    }

    // update total charge
    myTotalCharge += WCharged;
    // create charge row and insert it in myChargeValues
    charge C(MSNet::getInstance()->getCurrentTimeStep(), elecHybrid->getHolder().getID(), elecHybrid->getHolder().getVehicleType().getID(),
             status, WCharged, elecHybrid->getActualBatteryCapacity(), elecHybrid->getMaximumBatteryCapacity(),
             elecHybrid->getVoltageOfOverheadWire(), myTotalCharge);
    myChargeValues.push_back(C);
}


void
MSOverheadWire::writeOverheadWireSegmentOutput(OutputDevice& output) {
    output.openTag(SUMO_TAG_OVERHEAD_WIRE_SEGMENT);
    output.writeAttr(SUMO_ATTR_ID, myID);
    if (getTractionSubstation() != nullptr) {
        output.writeAttr(SUMO_ATTR_TRACTIONSUBSTATIONID, getTractionSubstation()->getID());
    } else {
        output.writeAttr(SUMO_ATTR_TRACTIONSUBSTATIONID, "");
    }
    output.writeAttr(SUMO_ATTR_TOTALENERGYCHARGED, myTotalCharge);
    output.writeAttr(SUMO_ATTR_CHARGINGSTEPS, myChargeValues.size());
    // start writting
    if (myChargeValues.size() > 0) {
        // First calculate charge for every vehicle
        std::vector<double> charge;
        std::vector<std::tuple<SUMOTime, SUMOTime, std::string> > vectorBeginEndCharge;
        SUMOTime firsTimeStep = myChargeValues.at(0).timeStep;
        // set first value
        charge.push_back(0);
        vectorBeginEndCharge.push_back(std::tuple<SUMOTime, SUMOTime, std::string>(firsTimeStep, 0, myChargeValues.at(0).vehicleID));
        // iterate over charging values
        for (std::vector<MSOverheadWire::charge>::const_iterator i = myChargeValues.begin(); i != myChargeValues.end(); i++) {
            // update chargue
            charge.back() += i->WCharged;
            // update end time
            std::get<1>(vectorBeginEndCharge.back()) = i->timeStep;
            // update timestep of charge
            firsTimeStep += 1000;
            // check if charge is continuous. If not, open a new vehicle tag
            if (((i + 1) != myChargeValues.end()) && (((i + 1)->timeStep) != firsTimeStep)) {
                // set new firsTimeStep of charge
                firsTimeStep = (i + 1)->timeStep;
                charge.push_back(0);
                vectorBeginEndCharge.push_back(std::tuple<SUMOTime, SUMOTime, std::string>(firsTimeStep, 0, (i + 1)->vehicleID));
            }
        }
        // now write values
        firsTimeStep = myChargeValues.at(0).timeStep;
        int vehicleCounter = 0;
        // open tag for first vehicle and write id and type of vehicle
        output.openTag(SUMO_TAG_VEHICLE);
        output.writeAttr(SUMO_ATTR_ID, myChargeValues.at(0).vehicleID);
        output.writeAttr(SUMO_ATTR_TYPE, myChargeValues.at(0).vehicleType);
        output.writeAttr(SUMO_ATTR_TOTALENERGYCHARGED_VEHICLE, charge.at(0));
        output.writeAttr(SUMO_ATTR_CHARGINGBEGIN, time2string(std::get<0>(vectorBeginEndCharge.at(0))));
        output.writeAttr(SUMO_ATTR_CHARGINGEND, time2string(std::get<1>(vectorBeginEndCharge.at(0))));
        output.writeAttr(SUMO_ATTR_MAXIMUMBATTERYCAPACITY, myChargeValues.at(0).maxBatteryCapacity);
        // iterate over charging values
        for (std::vector<MSOverheadWire::charge>::const_iterator i = myChargeValues.begin(); i != myChargeValues.end(); i++) {
            // open tag for timestep and write all parameters
            output.openTag(SUMO_TAG_STEP);
            output.writeAttr(SUMO_ATTR_TIME, time2string(i->timeStep));
            // charge values
            output.writeAttr(SUMO_ATTR_CHARGING_STATUS, i->status);
            output.writeAttr(SUMO_ATTR_ENERGYCHARGED, i->WCharged);
            output.writeAttr(SUMO_ATTR_PARTIALCHARGE, i->totalEnergyCharged);
            // charging values of charging station in this timestep
            output.writeAttr(SUMO_ATTR_VOLTAGE, i->voltage);
            // battery status of vehicle
            output.writeAttr(SUMO_ATTR_ACTUALBATTERYCAPACITY, i->actualBatteryCapacity);
            // close tag timestep
            output.closeTag();
            // update timestep of charge
            firsTimeStep += 1000;
            // check if charge is continuous. If not, open a new vehicle tag
            if (((i + 1) != myChargeValues.end()) && (((i + 1)->timeStep) != firsTimeStep)) {
                // set new firsTimeStep of charge
                firsTimeStep = (i + 1)->timeStep;
                // update counter
                vehicleCounter++;
                // close previous vehicle tag
                output.closeTag();
                // open tag for new vehicle and write id and type of vehicle
                output.openTag(SUMO_TAG_VEHICLE);
                output.writeAttr(SUMO_ATTR_ID, (i + 1)->vehicleID);
                output.writeAttr(SUMO_ATTR_TYPE, (i + 1)->vehicleType);
                output.writeAttr(SUMO_ATTR_TOTALENERGYCHARGED_VEHICLE, charge.at(vehicleCounter));
                output.writeAttr(SUMO_ATTR_CHARGINGBEGIN, std::get<0>(vectorBeginEndCharge.at(vehicleCounter)));
                output.writeAttr(SUMO_ATTR_CHARGINGEND, std::get<1>(vectorBeginEndCharge.at(vehicleCounter)));
            }
        }
        // close vehicle tag
        output.closeTag();
    }
    // close charging station tag
    output.closeTag();
}


/****************************************************************************/
