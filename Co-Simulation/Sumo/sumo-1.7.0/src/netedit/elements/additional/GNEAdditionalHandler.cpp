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
/// @file    GNEAdditionalHandler.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Nov 2015
///
// Builds trigger objects for netedit
/****************************************************************************/
#include <config.h>
#include <utils/xml/XMLSubSys.h>
#include <netedit/changes/GNEChange_Additional.h>
#include <netedit/changes/GNEChange_TAZElement.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNENet.h>
#include <utils/options/OptionsCont.h>
#include <utils/vehicle/SUMORouteHandler.h>

#include "GNEAdditionalHandler.h"
#include "GNEBusStop.h"
#include "GNEAccess.h"
#include "GNECalibrator.h"
#include "GNECalibratorFlow.h"
#include "GNEChargingStation.h"
#include "GNEClosingLaneReroute.h"
#include "GNEClosingReroute.h"
#include "GNEContainerStop.h"
#include "GNEDestProbReroute.h"
#include "GNEDetectorE1.h"
#include "GNEDetectorE2.h"
#include "GNEDetectorE3.h"
#include "GNEDetectorEntryExit.h"
#include "GNEDetectorE1Instant.h"
#include "GNEParkingArea.h"
#include "GNEParkingSpace.h"
#include "GNERerouter.h"
#include "GNERerouterSymbol.h"
#include "GNERerouterInterval.h"
#include "GNERouteProbReroute.h"
#include "GNEParkingAreaReroute.h"
#include "GNERouteProbe.h"
#include "GNEVaporizer.h"
#include "GNEVariableSpeedSign.h"
#include "GNEVariableSpeedSignSymbol.h"
#include "GNEVariableSpeedSignStep.h"
#include "GNETAZ.h"
#include "GNETAZSourceSink.h"


// ===========================================================================
// GNEAdditionalHandler method definitions
// ===========================================================================

GNEAdditionalHandler::GNEAdditionalHandler(const std::string& file, GNENet* net, GNEAdditional* additionalParent) :
    ShapeHandler(file, *net->getAttributeCarriers()),
    myNet(net) {
    myLastInsertedElement = new LastInsertedElement();
    // check if we're loading values of another additionals (example: Rerouter values)
    if (additionalParent) {
        myLastInsertedElement->insertElement(additionalParent->getTagProperty().getTag());
        myLastInsertedElement->commitAdditionalInsertion(additionalParent);
    }
    // define default values for shapes
    setDefaults("", RGBColor::RED, Shape::DEFAULT_LAYER_POI, true);
}


GNEAdditionalHandler::~GNEAdditionalHandler() {
    delete myLastInsertedElement;
}


void
GNEAdditionalHandler::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    // Obtain tag of element
    SumoXMLTag tag = static_cast<SumoXMLTag>(element);
    // check if we're parsing a parameter
    if (tag == SUMO_TAG_PARAM) {
        // push element int stack
        myLastInsertedElement->insertElement(tag);
        // parse parameter
        parseParameter(attrs);
    } else if (tag != SUMO_TAG_NOTHING) {
        // push element int stack
        if (tag == SUMO_TAG_TRAIN_STOP) {
            // ensure that access elements can find their parent in myLastInsertedElement
            tag = SUMO_TAG_BUS_STOP;
        }
        myLastInsertedElement->insertElement(tag);
        // Call parse and build depending of tag
        switch (tag) {
            case SUMO_TAG_POLY:
                return parseAndBuildPoly(attrs);
            case SUMO_TAG_POI:
                return parseAndBuildPOI(attrs);
            default:
                // build additional
                buildAdditional(myNet, true, tag, attrs, myLastInsertedElement);
        }
    }
}


void
GNEAdditionalHandler::myEndElement(int element) {
    // Obtain tag of element
    SumoXMLTag tag = static_cast<SumoXMLTag>(element);
    switch (tag) {
        case SUMO_TAG_TAZ: {
            GNETAZElement* TAZ = myLastInsertedElement->getLastInsertedTAZElement();
            if (TAZ != nullptr) {
                if (TAZ->getTAZElementShape().size() == 0) {
                    Boundary b;
                    if (TAZ->getChildAdditionals().size() > 0) {
                        for (const auto& i : TAZ->getChildAdditionals()) {
                            b.add(i->getCenteringBoundary());
                        }
                        PositionVector boundaryShape;
                        boundaryShape.push_back(Position(b.xmin(), b.ymin()));
                        boundaryShape.push_back(Position(b.xmax(), b.ymin()));
                        boundaryShape.push_back(Position(b.xmax(), b.ymax()));
                        boundaryShape.push_back(Position(b.xmin(), b.ymax()));
                        boundaryShape.push_back(Position(b.xmin(), b.ymin()));
                        TAZ->setAttribute(SUMO_ATTR_SHAPE, toString(boundaryShape), myNet->getViewNet()->getUndoList());
                    }
                }
            }
            break;
        }
        default:
            break;
    }
    // pop last inserted element
    myLastInsertedElement->popElement();
    // execute myEndElement of ShapeHandler (needed to update myLastParameterised)
    ShapeHandler::myEndElement(element);
}


Position
GNEAdditionalHandler::getLanePos(const std::string& poiID, const std::string& laneID, double lanePos, double lanePosLat) {
    std::string edgeID;
    int laneIndex;
    NBHelpers::interpretLaneID(laneID, edgeID, laneIndex);
    NBEdge* edge = myNet->retrieveEdge(edgeID)->getNBEdge();
    if (edge == nullptr || laneIndex < 0 || edge->getNumLanes() <= laneIndex) {
        WRITE_ERROR("Lane '" + laneID + "' to place poi '" + poiID + "' on is not known.");
        return Position::INVALID;
    }
    if (lanePos < 0) {
        lanePos = edge->getLength() + lanePos;
    }
    if (lanePos < 0 || lanePos > edge->getLength()) {
        WRITE_WARNING("lane position " + toString(lanePos) + " for poi '" + poiID + "' is not valid.");
    }
    return edge->getLanes()[laneIndex].shape.positionAtOffset(lanePos, -lanePosLat);
}


bool
GNEAdditionalHandler::buildAdditional(GNENet* net, bool allowUndoRedo, SumoXMLTag tag, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    // Call parse and build depending of tag
    switch (tag) {
        case SUMO_TAG_BUS_STOP:
        case SUMO_TAG_TRAIN_STOP:
            return parseAndBuildBusStop(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_ACCESS:
            return parseAndBuildAccess(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_CONTAINER_STOP:
            return parseAndBuildContainerStop(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_CHARGING_STATION:
            return parseAndBuildChargingStation(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_E1DETECTOR:
        case SUMO_TAG_INDUCTION_LOOP:
            return parseAndBuildDetectorE1(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_E2DETECTOR:
        case SUMO_TAG_E2DETECTOR_MULTILANE:
        case SUMO_TAG_LANE_AREA_DETECTOR:
            return parseAndBuildDetectorE2(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_E3DETECTOR:
        case SUMO_TAG_ENTRY_EXIT_DETECTOR:
            return parseAndBuildDetectorE3(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_DET_ENTRY:
            return parseAndBuildDetectorEntry(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_DET_EXIT:
            return parseAndBuildDetectorExit(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_INSTANT_INDUCTION_LOOP:
            return parseAndBuildDetectorE1Instant(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_ROUTEPROBE:
            return parseAndBuildRouteProbe(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_VAPORIZER:
            return parseAndBuildVaporizer(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_TAZ:
            return parseAndBuildTAZ(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_TAZSOURCE:
            return parseAndBuildTAZSource(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_TAZSINK:
            return parseAndBuildTAZSink(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_VSS:
            return parseAndBuildVariableSpeedSign(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_STEP:
            return parseAndBuildVariableSpeedSignStep(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_CALIBRATOR:
        case SUMO_TAG_LANECALIBRATOR:
            return parseAndBuildCalibrator(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_PARKING_AREA:
            return parseAndBuildParkingArea(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_PARKING_SPACE:
            return parseAndBuildParkingSpace(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_FLOW_CALIBRATOR:
            return parseAndBuildCalibratorFlow(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_REROUTER:
            return parseAndBuildRerouter(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_INTERVAL:
            return parseAndBuildRerouterInterval(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_CLOSING_LANE_REROUTE:
            return parseAndBuildRerouterClosingLaneReroute(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_CLOSING_REROUTE:
            return parseAndBuildRerouterClosingReroute(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_DEST_PROB_REROUTE:
            return parseAndBuildRerouterDestProbReroute(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_PARKING_ZONE_REROUTE:
            return parseAndBuildRerouterParkingAreaReroute(net, allowUndoRedo, attrs, insertedAdditionals);
        case SUMO_TAG_ROUTE_PROB_REROUTE:
            return parseAndBuildRerouterRouteProbReroute(net, allowUndoRedo, attrs, insertedAdditionals);
        default:
            return false;
    }
}


GNEAdditional*
GNEAdditionalHandler::buildBusStop(GNENet* net, bool allowUndoRedo, const std::string& id, GNELane* lane,
                                   const double startPos, const double endPos, const int parametersSet,
                                   const std::string& name, const std::vector<std::string>& lines, int personCapacity, double parkingLength,
                                   bool friendlyPosition, bool blockMovement) {
    if (net->retrieveAdditional(SUMO_TAG_BUS_STOP, id, false) == nullptr) {
        GNEAdditional* busStop = new GNEBusStop(id, lane, net, startPos, endPos, parametersSet, name, lines, personCapacity, parkingLength, friendlyPosition, blockMovement);
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_BUS_STOP));
            net->getViewNet()->getUndoList()->add(new GNEChange_Additional(busStop, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            net->getAttributeCarriers()->insertAdditional(busStop);
            lane->addChildElement(busStop);
            busStop->incRef("buildBusStop");
        }
        return busStop;
    } else {
        throw ProcessError("Could not build " + toString(SUMO_TAG_BUS_STOP) + " with ID '" + id + "' in netedit; probably declared twice.");
    }
}


GNEAdditional*
GNEAdditionalHandler::buildAccess(GNENet* net, bool allowUndoRedo, GNEAdditional* busStop, GNELane* lane, double pos, const std::string& length, bool friendlyPos, bool blockMovement) {
    // Check if busStop parent and lane is correct
    if (lane == nullptr) {
        throw ProcessError("Could not build " + toString(SUMO_TAG_ACCESS) + " in netedit; " +  toString(SUMO_TAG_LANE) + " doesn't exist.");
    } else if (busStop == nullptr) {
        throw ProcessError("Could not build " + toString(SUMO_TAG_ACCESS) + " in netedit; " +  toString(SUMO_TAG_BUS_STOP) + " parent doesn't exist.");
    } else if (!accessCanBeCreated(busStop, lane->getParentEdge())) {
        throw ProcessError("Could not build " + toString(SUMO_TAG_ACCESS) + " in netedit; " +  toString(SUMO_TAG_BUS_STOP) + " parent already owns a Acces in the edge '" + lane->getParentEdge()->getID() + "'");
    } else {
        GNEAdditional* access = new GNEAccess(busStop, lane, net, pos, length, friendlyPos, blockMovement);
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_ACCESS));
            net->getViewNet()->getUndoList()->add(new GNEChange_Additional(access, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            net->getAttributeCarriers()->insertAdditional(access);
            lane->addChildElement(access);
            busStop->addChildElement(access);
            access->incRef("buildAccess");
        }
        return access;
    }
}


GNEAdditional*
GNEAdditionalHandler::buildContainerStop(GNENet* net, bool allowUndoRedo, const std::string& id, GNELane* lane, const double startPos, const double endPos, const int parametersSet,
        const std::string& name, const std::vector<std::string>& lines, bool friendlyPosition, bool blockMovement) {
    if (net->retrieveAdditional(SUMO_TAG_CONTAINER_STOP, id, false) == nullptr) {
        GNEAdditional* containerStop = new GNEContainerStop(id, lane, net, startPos, endPos, parametersSet, name, lines, friendlyPosition, blockMovement);
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_CONTAINER_STOP));
            net->getViewNet()->getUndoList()->add(new GNEChange_Additional(containerStop, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            net->getAttributeCarriers()->insertAdditional(containerStop);
            lane->addChildElement(containerStop);
            containerStop->incRef("buildContainerStop");
        }
        return containerStop;
    } else {
        throw ProcessError("Could not build " + toString(SUMO_TAG_CONTAINER_STOP) + " with ID '" + id + "' in netedit; probably declared twice.");
    }
}


GNEAdditional*
GNEAdditionalHandler::buildChargingStation(GNENet* net, bool allowUndoRedo, const std::string& id, GNELane* lane, const double startPos, const double endPos, const int parametersSet,
        const std::string& name, double chargingPower, double efficiency, bool chargeInTransit, SUMOTime chargeDelay, bool friendlyPosition, bool blockMovement) {
    if (net->retrieveAdditional(SUMO_TAG_CHARGING_STATION, id, false) == nullptr) {
        GNEAdditional* chargingStation = new GNEChargingStation(id, lane, net, startPos, endPos, parametersSet, name, chargingPower, efficiency, chargeInTransit, chargeDelay, friendlyPosition, blockMovement);
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_CHARGING_STATION));
            net->getViewNet()->getUndoList()->add(new GNEChange_Additional(chargingStation, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            net->getAttributeCarriers()->insertAdditional(chargingStation);
            lane->addChildElement(chargingStation);
            chargingStation->incRef("buildChargingStation");
        }
        return chargingStation;
    } else {
        throw ProcessError("Could not build " + toString(SUMO_TAG_CHARGING_STATION) + " with ID '" + id + "' in netedit; probably declared twice.");
    }
}


GNEAdditional*
GNEAdditionalHandler::buildParkingArea(GNENet* net, bool allowUndoRedo, const std::string& id, GNELane* lane, const double startPos, const double endPos, const int parametersSet,
                                       const std::string& name, bool friendlyPosition, int roadSideCapacity, bool onRoad, double width, const std::string& length, double angle, bool blockMovement) {
    if (net->retrieveAdditional(SUMO_TAG_PARKING_AREA, id, false) == nullptr) {
        GNEAdditional* parkingArea = new GNEParkingArea(id, lane, net, startPos, endPos, parametersSet, name, friendlyPosition, roadSideCapacity, onRoad, width, length, angle, blockMovement);
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_PARKING_AREA));
            net->getViewNet()->getUndoList()->add(new GNEChange_Additional(parkingArea, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            net->getAttributeCarriers()->insertAdditional(parkingArea);
            lane->addChildElement(parkingArea);
            parkingArea->incRef("buildParkingArea");
        }
        return parkingArea;
    } else {
        throw ProcessError("Could not build " + toString(SUMO_TAG_PARKING_AREA) + " with ID '" + id + "' in netedit; probably declared twice.");
    }
}


GNEAdditional*
GNEAdditionalHandler::buildParkingSpace(GNENet* net, bool allowUndoRedo, GNEAdditional* parkingAreaParent, Position pos, double width, double length, double angle, bool blockMovement) {
    GNEAdditional* parkingSpace = new GNEParkingSpace(net, parkingAreaParent, pos, width, length, angle, blockMovement);
    if (allowUndoRedo) {
        net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_PARKING_SPACE));
        net->getViewNet()->getUndoList()->add(new GNEChange_Additional(parkingSpace, true), true);
        net->getViewNet()->getUndoList()->p_end();
    } else {
        net->getAttributeCarriers()->insertAdditional(parkingSpace);
        parkingAreaParent->addChildElement(parkingSpace);
        parkingSpace->incRef("buildParkingSpace");
    }
    return parkingSpace;
}


GNEAdditional*
GNEAdditionalHandler::buildDetectorE1(GNENet* net, bool allowUndoRedo, const std::string& id, GNELane* lane, double pos, SUMOTime freq, const std::string& filename, const std::string& vehicleTypes, const std::string& name, bool friendlyPos, bool blockMovement) {
    if (net->retrieveAdditional(SUMO_TAG_E1DETECTOR, id, false) == nullptr) {
        GNEAdditional* detectorE1 = new GNEDetectorE1(id, lane, net, pos, freq, filename, vehicleTypes, name, friendlyPos, blockMovement);
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_E1DETECTOR));
            net->getViewNet()->getUndoList()->add(new GNEChange_Additional(detectorE1, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            net->getAttributeCarriers()->insertAdditional(detectorE1);
            lane->addChildElement(detectorE1);
            detectorE1->incRef("buildDetectorE1");
        }
        return detectorE1;
    } else {
        throw ProcessError("Could not build " + toString(SUMO_TAG_E1DETECTOR) + " with ID '" + id + "' in netedit; probably declared twice.");
    }
}


GNEAdditional*
GNEAdditionalHandler::buildSingleLaneDetectorE2(GNENet* net, bool allowUndoRedo, const std::string& id, GNELane* lane, double pos, double length, const std::string& freq, const std::string& trafficLight,
        const std::string& filename, const std::string& vehicleTypes, const std::string& name, SUMOTime timeThreshold, double speedThreshold, double jamThreshold, bool friendlyPos, bool blockMovement) {
    if (net->retrieveAdditional(SUMO_TAG_E2DETECTOR, id, false) == nullptr) {
        GNEAdditional* detectorE2 = new GNEDetectorE2(id, lane, net, pos, length, freq, trafficLight, filename, vehicleTypes, name, timeThreshold, speedThreshold, jamThreshold, friendlyPos, blockMovement);
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_E2DETECTOR));
            net->getViewNet()->getUndoList()->add(new GNEChange_Additional(detectorE2, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            net->getAttributeCarriers()->insertAdditional(detectorE2);
            lane->addChildElement(detectorE2);
            detectorE2->incRef("buildDetectorE2");
        }
        return detectorE2;
    } else {
        throw ProcessError("Could not build " + toString(SUMO_TAG_E2DETECTOR) + " with ID '" + id + "' in netedit; probably declared twice.");
    }
}


GNEAdditional*
GNEAdditionalHandler::buildMultiLaneDetectorE2(GNENet* net, bool allowUndoRedo, const std::string& id, const std::vector<GNELane*>& lanes, double pos, double endPos, const std::string& freq, const std::string& trafficLight,
        const std::string& filename, const std::string& vehicleTypes, const std::string& name, SUMOTime timeThreshold, double speedThreshold, double jamThreshold, bool friendlyPos, bool blockMovement) {
    if (net->retrieveAdditional(SUMO_TAG_E2DETECTOR_MULTILANE, id, false) == nullptr) {
        GNEDetectorE2* detectorE2 = new GNEDetectorE2(id, lanes, net, pos, endPos, freq, trafficLight, filename, vehicleTypes, name, timeThreshold, speedThreshold, jamThreshold, friendlyPos, blockMovement);
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_E2DETECTOR_MULTILANE));
            net->getViewNet()->getUndoList()->add(new GNEChange_Additional(detectorE2, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            net->getAttributeCarriers()->insertAdditional(detectorE2);
            // use to avoid LNK2019
            GNEAdditional* detectorE2Additional = detectorE2;
            for (const auto& lane : lanes) {
                lane->addChildElement(detectorE2Additional);
            }
            detectorE2->incRef("buildDetectorE2Multilane");
        }
        return detectorE2;
    } else {
        throw ProcessError("Could not build " + toString(SUMO_TAG_E2DETECTOR_MULTILANE) + " with ID '" + id + "' in netedit; probably declared twice.");
    }
}


GNEAdditional*
GNEAdditionalHandler::buildDetectorE3(GNENet* net, bool allowUndoRedo, const std::string& id, Position pos, SUMOTime freq, const std::string& filename, const std::string& vehicleTypes,
                                      const std::string& name, SUMOTime timeThreshold, double speedThreshold, bool blockMovement) {
    if (net->retrieveAdditional(SUMO_TAG_E3DETECTOR, id, false) == nullptr) {
        GNEAdditional* detectorE3 = new GNEDetectorE3(id, net, pos, freq, filename, vehicleTypes, name, timeThreshold, speedThreshold, blockMovement);
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_E3DETECTOR));
            net->getViewNet()->getUndoList()->add(new GNEChange_Additional(detectorE3, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            net->getAttributeCarriers()->insertAdditional(detectorE3);
            detectorE3->incRef("buildDetectorE3");
        }
        return detectorE3;
    } else {
        throw ProcessError("Could not build " + toString(SUMO_TAG_E3DETECTOR) + " with ID '" + id + "' in netedit; probably declared twice.");
    }
}


GNEAdditional*
GNEAdditionalHandler::buildDetectorEntry(GNENet* net, bool allowUndoRedo, GNEAdditional* E3Parent, GNELane* lane, double pos, bool friendlyPos, bool blockMovement) {
    // Check if Detector E3 parent and lane is correct
    if (lane == nullptr) {
        throw ProcessError("Could not build " + toString(SUMO_TAG_DET_ENTRY) + " in netedit; " +  toString(SUMO_TAG_LANE) + " doesn't exist.");
    } else if (E3Parent == nullptr) {
        throw ProcessError("Could not build " + toString(SUMO_TAG_DET_ENTRY) + " in netedit; " +  toString(SUMO_TAG_E3DETECTOR) + " parent doesn't exist.");
    } else {
        GNEAdditional* entry = new GNEDetectorEntryExit(SUMO_TAG_DET_ENTRY, net, E3Parent, lane, pos, friendlyPos, blockMovement);
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_DET_ENTRY));
            net->getViewNet()->getUndoList()->add(new GNEChange_Additional(entry, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            net->getAttributeCarriers()->insertAdditional(entry);
            lane->addChildElement(entry);
            E3Parent->addChildElement(entry);
            entry->incRef("buildDetectorEntry");
        }
        return entry;
    }
}


GNEAdditional*
GNEAdditionalHandler::buildDetectorExit(GNENet* net, bool allowUndoRedo, GNEAdditional* E3Parent, GNELane* lane, double pos, bool friendlyPos, bool blockMovement) {
    // Check if Detector E3 parent and lane is correct
    if (lane == nullptr) {
        throw ProcessError("Could not build " + toString(SUMO_TAG_DET_EXIT) + " in netedit; " +  toString(SUMO_TAG_LANE) + " doesn't exist.");
    } else if (E3Parent == nullptr) {
        throw ProcessError("Could not build " + toString(SUMO_TAG_DET_EXIT) + " in netedit; " +  toString(SUMO_TAG_E3DETECTOR) + " parent doesn't exist.");
    } else {
        GNEAdditional* exit = new GNEDetectorEntryExit(SUMO_TAG_DET_EXIT, net, E3Parent, lane, pos, friendlyPos, blockMovement);
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_DET_EXIT));
            net->getViewNet()->getUndoList()->add(new GNEChange_Additional(exit, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            net->getAttributeCarriers()->insertAdditional(exit);
            lane->addChildElement(exit);
            E3Parent->addChildElement(exit);
            exit->incRef("buildDetectorExit");
        }
        return exit;
    }
}


GNEAdditional*
GNEAdditionalHandler::buildDetectorE1Instant(GNENet* net, bool allowUndoRedo, const std::string& id, GNELane* lane, double pos, const std::string& filename, const std::string& vehicleTypes, const std::string& name, bool friendlyPos, bool blockMovement) {
    if (net->retrieveAdditional(SUMO_TAG_INSTANT_INDUCTION_LOOP, id, false) == nullptr) {
        GNEAdditional* detectorE1Instant = new GNEDetectorE1Instant(id, lane, net, pos, filename, vehicleTypes, name, friendlyPos, blockMovement);
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_INSTANT_INDUCTION_LOOP));
            net->getViewNet()->getUndoList()->add(new GNEChange_Additional(detectorE1Instant, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            net->getAttributeCarriers()->insertAdditional(detectorE1Instant);
            lane->addChildElement(detectorE1Instant);
            detectorE1Instant->incRef("buildDetectorE1Instant");
        }
        return detectorE1Instant;
    } else {
        throw ProcessError("Could not build " + toString(SUMO_TAG_INSTANT_INDUCTION_LOOP) + " with ID '" + id + "' in netedit; probably declared twice.");
    }
}


GNEAdditional*
GNEAdditionalHandler::buildCalibrator(GNENet* net, bool allowUndoRedo, const std::string& id, GNELane* lane, double pos, const std::string& name, const std::string& outfile, const SUMOTime freq, const std::string& routeprobe, bool centerAfterCreation) {
    if (net->retrieveAdditional(SUMO_TAG_CALIBRATOR, id, false) == nullptr) {
        GNEAdditional* calibrator = new GNECalibrator(id, net, lane, pos, freq, name, outfile, routeprobe);
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_CALIBRATOR));
            net->getViewNet()->getUndoList()->add(new GNEChange_Additional(calibrator, true), true);
            net->getViewNet()->getUndoList()->p_end();
            // center after creation
            if (centerAfterCreation) {
                net->getViewNet()->centerTo(calibrator->getPositionInView(), false);
            }
        } else {
            net->getAttributeCarriers()->insertAdditional(calibrator);
            lane->addChildElement(calibrator);
            calibrator->incRef("buildCalibrator");
        }
        return calibrator;
    } else {
        throw ProcessError("Could not build " + toString(SUMO_TAG_CALIBRATOR) + " with ID '" + id + "' in netedit; probably declared twice.");
    }
}


GNEAdditional*
GNEAdditionalHandler::buildCalibrator(GNENet* net, bool allowUndoRedo, const std::string& id, GNEEdge* edge, double pos, const std::string& name, const std::string& outfile, const SUMOTime freq, const std::string& routeprobe, bool centerAfterCreation) {
    if (net->retrieveAdditional(SUMO_TAG_CALIBRATOR, id, false) == nullptr) {
        GNEAdditional* calibrator = new GNECalibrator(id, net, edge, pos, freq, name, outfile, routeprobe);
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_CALIBRATOR));
            net->getViewNet()->getUndoList()->add(new GNEChange_Additional(calibrator, true), true);
            net->getViewNet()->getUndoList()->p_end();
            // center after creation
            if (centerAfterCreation) {
                net->getViewNet()->centerTo(calibrator->getPositionInView(), false);
            }
        } else {
            net->getAttributeCarriers()->insertAdditional(calibrator);
            edge->addChildElement(calibrator);
            calibrator->incRef("buildCalibrator");
        }
        return calibrator;
    } else {
        throw ProcessError("Could not build " + toString(SUMO_TAG_CALIBRATOR) + " with ID '" + id + "' in netedit; probably declared twice.");
    }
}


GNEAdditional*
GNEAdditionalHandler::buildCalibratorFlow(GNENet* net, bool allowUndoRedo, GNEAdditional* calibratorParent, GNEDemandElement* route, GNEDemandElement* vType,
        const std::string& vehsPerHour, const std::string& speed, const RGBColor& color, const std::string& departLane, const std::string& departPos,
        const std::string& departSpeed, const std::string& arrivalLane, const std::string& arrivalPos, const std::string& arrivalSpeed, const std::string& line,
        int personNumber, int containerNumber, bool reroute, const std::string& departPosLat, const std::string& arrivalPosLat, SUMOTime begin, SUMOTime end) {

    // create Flow and add it to calibrator parent
    GNEAdditional* flow = new GNECalibratorFlow(calibratorParent, vType, route, vehsPerHour, speed, color, departLane, departPos, departSpeed,
            arrivalLane, arrivalPos, arrivalSpeed, line, personNumber, containerNumber, reroute,
            departPosLat, arrivalPosLat, begin, end);
    if (allowUndoRedo) {
        net->getViewNet()->getUndoList()->p_begin("add " + flow->getTagStr());
        net->getViewNet()->getUndoList()->add(new GNEChange_Additional(flow, true), true);
        net->getViewNet()->getUndoList()->p_end();
    } else {
        calibratorParent->addChildElement(flow);
        flow->incRef("buildCalibratorFlow");
    }
    return flow;
}


GNEAdditional*
GNEAdditionalHandler::buildRerouter(GNENet* net, bool allowUndoRedo, const std::string& id, Position pos, const std::vector<GNEEdge*>& edges, double prob, const std::string& name, const std::string& file, bool off, SUMOTime timeThreshold, const std::string& vTypes, bool blockMovement) {
    if (net->retrieveAdditional(SUMO_TAG_REROUTER, id, false) == nullptr) {
        // create reroute
        GNEAdditional* rerouter = new GNERerouter(id, net, pos, name, file, prob, off, timeThreshold, vTypes, blockMovement);
        // create rerouter Symbols
        std::vector<GNEAdditional*> rerouterSymbols;
        for (const auto& edge : edges) {
            rerouterSymbols.push_back(new GNERerouterSymbol(rerouter, edge));
        }
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_REROUTER));
            net->getViewNet()->getUndoList()->add(new GNEChange_Additional(rerouter, true), true);
            for (const auto& rerouterSymbol : rerouterSymbols) {
                net->getViewNet()->getUndoList()->add(new GNEChange_Additional(rerouterSymbol, true), true);
            }
            net->getViewNet()->getUndoList()->p_end();
        } else {
            net->getAttributeCarriers()->insertAdditional(rerouter);
            // add symbols
            for (int i = 0; i < (int)edges.size(); i++) {
                edges.at(i)->addChildElement(rerouterSymbols.at(i));
                rerouterSymbols.at(i)->incRef("buildRerouterSymbol");
            }
            rerouter->incRef("buildRerouter");
        }
        // parse rerouter children
        if (!file.empty()) {
            // we assume that rerouter values files is placed in the same folder as the additional file
            std::string currentAdditionalFilename = FileHelpers::getFilePath(OptionsCont::getOptions().getString("additional-files"));
            // Create additional handler for parse rerouter values
            GNEAdditionalHandler rerouterValuesHandler(currentAdditionalFilename + file, net, rerouter);
            // disable validation for rerouters
            XMLSubSys::setValidation("never", "auto", "auto");
            // Run parser
            if (!XMLSubSys::runParser(rerouterValuesHandler, currentAdditionalFilename + file, false)) {
                WRITE_MESSAGE("Loading of " + file + " failed.");
            }
            // enable validation for rerouters
            XMLSubSys::setValidation("auto", "auto", "auto");
        }
        return rerouter;
    } else {
        throw ProcessError("Could not build " + toString(SUMO_TAG_REROUTER) + " with ID '" + id + "' in netedit; probably declared twice.");
    }
}


GNEAdditional*
GNEAdditionalHandler::buildRerouterInterval(GNENet* net, bool allowUndoRedo, GNEAdditional* rerouterParent, SUMOTime begin, SUMOTime end) {
    // check if new interval will produce a overlapping
    if (checkOverlappingRerouterIntervals(rerouterParent, begin, end)) {
        // create rerouter interval and add it into rerouter parent
        GNEAdditional* rerouterInterval = new GNERerouterInterval(rerouterParent, begin, end);
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + rerouterInterval->getTagStr());
            net->getViewNet()->getUndoList()->add(new GNEChange_Additional(rerouterInterval, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            rerouterParent->addChildElement(rerouterInterval);
            rerouterInterval->incRef("buildRerouterInterval");
        }
        return rerouterInterval;
    } else {
        throw ProcessError("Could not build " + toString(SUMO_TAG_INTERVAL) + " with begin '" + toString(begin) + "' and '" + toString(end) + "' in '" + rerouterParent->getID() + "' due overlapping.");
    }
}


GNEAdditional*
GNEAdditionalHandler::buildClosingLaneReroute(GNENet* net, bool allowUndoRedo, GNEAdditional* rerouterIntervalParent, GNELane* closedLane, SVCPermissions permissions) {
    // create closing lane reorute
    GNEAdditional* closingLaneReroute = new GNEClosingLaneReroute(rerouterIntervalParent, closedLane, permissions);
    // add it to interval parent depending of allowUndoRedo
    if (allowUndoRedo) {
        net->getViewNet()->getUndoList()->p_begin("add " + closingLaneReroute->getTagStr());
        net->getViewNet()->getUndoList()->add(new GNEChange_Additional(closingLaneReroute, true), true);
        net->getViewNet()->getUndoList()->p_end();
    } else {
        rerouterIntervalParent->addChildElement(closingLaneReroute);
        closingLaneReroute->incRef("buildClosingLaneReroute");
    }
    return closingLaneReroute;
}


GNEAdditional*
GNEAdditionalHandler::buildClosingReroute(GNENet* net, bool allowUndoRedo, GNEAdditional* rerouterIntervalParent, GNEEdge* closedEdge, SVCPermissions permissions) {
    // create closing reroute
    GNEAdditional* closingReroute = new GNEClosingReroute(rerouterIntervalParent, closedEdge, permissions);
    // add it to interval parent depending of allowUndoRedo
    if (allowUndoRedo) {
        net->getViewNet()->getUndoList()->p_begin("add " + closingReroute->getTagStr());
        net->getViewNet()->getUndoList()->add(new GNEChange_Additional(closingReroute, true), true);
        net->getViewNet()->getUndoList()->p_end();
    } else {
        rerouterIntervalParent->addChildElement(closingReroute);
        closingReroute->incRef("buildClosingReroute");
    }
    return closingReroute;
}


GNEAdditional*
GNEAdditionalHandler::builDestProbReroute(GNENet* net, bool allowUndoRedo, GNEAdditional* rerouterIntervalParent, GNEEdge* newEdgeDestination, double probability) {
    // create dest probability reroute
    GNEAdditional* destProbReroute = new GNEDestProbReroute(rerouterIntervalParent, newEdgeDestination, probability);
    // add it to interval parent depending of allowUndoRedo
    if (allowUndoRedo) {
        net->getViewNet()->getUndoList()->p_begin("add " + destProbReroute->getTagStr());
        net->getViewNet()->getUndoList()->add(new GNEChange_Additional(destProbReroute, true), true);
        net->getViewNet()->getUndoList()->p_end();
    } else {
        rerouterIntervalParent->addChildElement(destProbReroute);
        destProbReroute->incRef("builDestProbReroute");
    }
    return destProbReroute;
}


GNEAdditional*
GNEAdditionalHandler::builParkingAreaReroute(GNENet* net, bool allowUndoRedo, GNEAdditional* rerouterIntervalParent, GNEAdditional* newParkingArea, double probability, bool visible) {
    // create dest probability reroute
    GNEAdditional* parkingAreaReroute = new GNEParkingAreaReroute(rerouterIntervalParent, newParkingArea, probability, visible);
    // add it to interval parent depending of allowUndoRedo
    if (allowUndoRedo) {
        net->getViewNet()->getUndoList()->p_begin("add " + parkingAreaReroute->getTagStr());
        net->getViewNet()->getUndoList()->add(new GNEChange_Additional(parkingAreaReroute, true), true);
        net->getViewNet()->getUndoList()->p_end();
    } else {
        rerouterIntervalParent->addChildElement(parkingAreaReroute);
        parkingAreaReroute->incRef("builParkingAreaReroute");
    }
    return parkingAreaReroute;
}


GNEAdditional*
GNEAdditionalHandler::buildRouteProbReroute(GNENet* net, bool allowUndoRedo, GNEAdditional* rerouterIntervalParent, const std::string& newRouteId, double probability) {
    // create rout prob rereoute
    GNEAdditional* routeProbReroute = new GNERouteProbReroute(rerouterIntervalParent, newRouteId, probability);
    // add it to interval parent depending of allowUndoRedo
    if (allowUndoRedo) {
        net->getViewNet()->getUndoList()->p_begin("add " + routeProbReroute->getTagStr());
        net->getViewNet()->getUndoList()->add(new GNEChange_Additional(routeProbReroute, true), true);
        net->getViewNet()->getUndoList()->p_end();
    } else {
        rerouterIntervalParent->addChildElement(routeProbReroute);
        routeProbReroute->incRef("buildRouteProbReroute");
    }
    return routeProbReroute;
}


GNEAdditional*
GNEAdditionalHandler::buildRouteProbe(GNENet* net, bool allowUndoRedo, const std::string& id, GNEEdge* edge, const std::string& freq, const std::string& name, const std::string& file, SUMOTime begin, bool centerAfterCreation) {
    if (net->retrieveAdditional(SUMO_TAG_ROUTEPROBE, id, false) == nullptr) {
        GNEAdditional* routeProbe = new GNERouteProbe(id, net, edge, freq, name, file, begin);
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_ROUTEPROBE));
            net->getViewNet()->getUndoList()->add(new GNEChange_Additional(routeProbe, true), true);
            net->getViewNet()->getUndoList()->p_end();
            // center after creation
            if (centerAfterCreation) {
                net->getViewNet()->centerTo(routeProbe->getPositionInView(), false);
            }
        } else {
            net->getAttributeCarriers()->insertAdditional(routeProbe);
            edge->addChildElement(routeProbe);
            routeProbe->incRef("buildRouteProbe");
        }
        return routeProbe;
    } else {
        throw ProcessError("Could not build " + toString(SUMO_TAG_ROUTEPROBE) + " with ID '" + id + "' in netedit; probably declared twice.");
    }
}


GNEAdditional*
GNEAdditionalHandler::buildVariableSpeedSign(GNENet* net, bool allowUndoRedo, const std::string& id, Position pos, const std::vector<GNELane*>& lanes, const std::string& name, bool blockMovement) {
    if (net->retrieveAdditional(SUMO_TAG_VSS, id, false) == nullptr) {
        // create VSS
        GNEAdditional* variableSpeedSign = new GNEVariableSpeedSign(id, net, pos, name, blockMovement);
        // create VSS Symbols
        std::vector<GNEAdditional*> VSSSymbols;
        for (const auto& lane : lanes) {
            VSSSymbols.push_back(new GNEVariableSpeedSignSymbol(variableSpeedSign, lane));
        }
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_VSS));
            net->getViewNet()->getUndoList()->add(new GNEChange_Additional(variableSpeedSign, true), true);
            for (const auto& VSSSymbol : VSSSymbols) {
                net->getViewNet()->getUndoList()->add(new GNEChange_Additional(VSSSymbol, true), true);
            }
            net->getViewNet()->getUndoList()->p_end();
        } else {
            net->getAttributeCarriers()->insertAdditional(variableSpeedSign);
            // add symbols
            for (int i = 0; i < (int)lanes.size(); i++) {
                lanes.at(i)->addChildElement(VSSSymbols.at(i));
                VSSSymbols.at(i)->incRef("buildVariableSpeedSignSymbol");
            }
            variableSpeedSign->incRef("buildVariableSpeedSign");
        }
        return variableSpeedSign;
    } else {
        throw ProcessError("Could not build " + toString(SUMO_TAG_VSS) + " with ID '" + id + "' in netedit; probably declared twice.");
    }
}


GNEAdditional*
GNEAdditionalHandler::buildVariableSpeedSignStep(GNENet* net, bool allowUndoRedo, GNEAdditional* VSSParent, double time, double speed) {
    // create Variable Speed Sign
    GNEAdditional* variableSpeedSignStep = new GNEVariableSpeedSignStep(VSSParent, time, speed);
    // add it depending of allow undoRedo
    if (allowUndoRedo) {
        net->getViewNet()->getUndoList()->p_begin("add " + variableSpeedSignStep->getTagStr());
        net->getViewNet()->getUndoList()->add(new GNEChange_Additional(variableSpeedSignStep, true), true);
        net->getViewNet()->getUndoList()->p_end();
    } else {
        VSSParent->addChildElement(variableSpeedSignStep);
        variableSpeedSignStep->incRef("buildVariableSpeedSignStep");
    }
    return variableSpeedSignStep;
}


GNEAdditional*
GNEAdditionalHandler::buildVaporizer(GNENet* net, bool allowUndoRedo, GNEEdge* edge, SUMOTime startTime, SUMOTime endTime, const std::string& name, bool centerAfterCreation) {
    GNEAdditional* vaporizer = new GNEVaporizer(net, edge, startTime, endTime, name);
    if (allowUndoRedo) {
        net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_VAPORIZER));
        net->getViewNet()->getUndoList()->add(new GNEChange_Additional(vaporizer, true), true);
        net->getViewNet()->getUndoList()->p_end();
        // center after creation
        if (centerAfterCreation) {
            net->getViewNet()->centerTo(vaporizer->getPositionInView(), false);
        }
    } else {
        net->getAttributeCarriers()->insertAdditional(vaporizer);
        edge->addChildElement(vaporizer);
        vaporizer->incRef("buildVaporizer");
    }
    return vaporizer;
}


GNETAZElement*
GNEAdditionalHandler::buildTAZ(GNENet* net, bool allowUndoRedo, const std::string& id, const PositionVector& shape, const RGBColor& color, const std::vector<GNEEdge*>& edges, bool blockMovement) {
    GNETAZElement* TAZ = new GNETAZ(id, net, shape, color, blockMovement);
    // disable updating geometry of TAZ children during insertion (because in large nets provokes slowdowns)
    net->disableUpdateGeometry();
    if (allowUndoRedo) {
        net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_TAZ));
        net->getViewNet()->getUndoList()->add(new GNEChange_TAZElement(TAZ, true), true);
        // create TAZEdges
        for (const auto& edge : edges) {
            // create TAZ Source using GNEChange_Additional
            GNETAZElement* TAZSource = new GNETAZSourceSink(SUMO_TAG_TAZSOURCE, TAZ, edge, 1);
            net->getViewNet()->getUndoList()->add(new GNEChange_TAZElement(TAZSource, true), true);
            // create TAZ Sink using GNEChange_Additional
            GNETAZElement* TAZSink = new GNETAZSourceSink(SUMO_TAG_TAZSINK, TAZ, edge, 1);
            net->getViewNet()->getUndoList()->add(new GNEChange_TAZElement(TAZSink, true), true);
        }
        net->getViewNet()->getUndoList()->p_end();
    } else {
        net->getAttributeCarriers()->insertTAZElement(TAZ);
        TAZ->incRef("buildTAZ");
        for (const auto& edge : edges) {
            // create TAZ Source
            GNETAZElement* TAZSource = new GNETAZSourceSink(SUMO_TAG_TAZSOURCE, TAZ, edge, 1);
            TAZSource->incRef("buildTAZ");
            TAZ->addChildElement(TAZSource);
            // create TAZ Sink
            GNETAZElement* TAZSink = new GNETAZSourceSink(SUMO_TAG_TAZSINK, TAZ, edge, 1);
            TAZSink->incRef("buildTAZ");
            TAZ->addChildElement(TAZSink);
        }
    }
    // enable updating geometry again and update geometry of TAZ
    net->enableUpdateGeometry();
    // update TAZ Frame
    TAZ->updateGeometry();
    TAZ->updateParentAdditional();
    return TAZ;
}


GNETAZElement*
GNEAdditionalHandler::buildTAZSource(GNENet* net, bool allowUndoRedo, GNETAZElement* TAZ, GNEEdge* edge, double departWeight) {
    GNETAZElement* TAZSink = nullptr;
    // first check if a TAZSink in the same edge for the same TAZ
    for (const auto& TAZElement : TAZ->getChildTAZElements()) {
        if ((TAZElement->getTagProperty().getTag() == SUMO_TAG_TAZSINK) && (TAZElement->getAttribute(SUMO_ATTR_EDGE) == edge->getID())) {
            TAZSink = TAZElement;
        }
    }
    // check if TAZSink has to be created
    if (TAZSink == nullptr) {
        // Create TAZ with weight 0 (default)
        TAZSink = new GNETAZSourceSink(SUMO_TAG_TAZSINK, TAZ, edge, 1);
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_TAZSINK));
            net->getViewNet()->getUndoList()->add(new GNEChange_TAZElement(TAZSink, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            net->getAttributeCarriers()->insertTAZElement(TAZSink);
            TAZSink->incRef("buildTAZSource");
        }
    }
    // now check check if TAZSource exist
    GNETAZElement* TAZSource = nullptr;
    // first check if a TAZSink in the same edge for the same TAZ
    for (const auto& TAZElement : TAZ->getChildTAZElements()) {
        if ((TAZElement->getTagProperty().getTag() == SUMO_TAG_TAZSOURCE) && (TAZElement->getAttribute(SUMO_ATTR_EDGE) == edge->getID())) {
            TAZSource = TAZElement;
        }
    }
    // check if TAZSource has to be created
    if (TAZSource == nullptr) {
        // Create TAZ only with departWeight
        TAZSource = new GNETAZSourceSink(SUMO_TAG_TAZSOURCE, TAZ, edge, departWeight);
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_TAZSOURCE));
            net->getViewNet()->getUndoList()->add(new GNEChange_TAZElement(TAZSource, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            net->getAttributeCarriers()->insertTAZElement(TAZSource);
            TAZSource->incRef("buildTAZSource");
        }
    } else {
        // update TAZ Attribute
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("update " + toString(SUMO_TAG_TAZSOURCE));
            TAZSource->setAttribute(SUMO_ATTR_WEIGHT, toString(departWeight), net->getViewNet()->getUndoList());
            net->getViewNet()->getUndoList()->p_end();
        } else {
            TAZSource->setAttribute(SUMO_ATTR_WEIGHT, toString(departWeight), nullptr);
            TAZSource->incRef("buildTAZSource");
        }
    }
    return TAZSource;
}


GNETAZElement*
GNEAdditionalHandler::buildTAZSink(GNENet* net, bool allowUndoRedo, GNETAZElement* TAZ, GNEEdge* edge, double arrivalWeight) {
    GNETAZElement* TAZSource = nullptr;
    // first check if a TAZSink in the same edge for the same TAZ
    for (const auto& TAZElement : TAZ->getChildTAZElements()) {
        if ((TAZElement->getTagProperty().getTag() == SUMO_TAG_TAZSOURCE) && (TAZElement->getAttribute(SUMO_ATTR_EDGE) == edge->getID())) {
            TAZSource = TAZElement;
        }
    }
    // check if TAZSource has to be created
    if (TAZSource == nullptr) {
        // Create TAZ with empty value
        TAZSource = new GNETAZSourceSink(SUMO_TAG_TAZSOURCE, TAZ, edge, 1);
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_TAZSOURCE));
            net->getViewNet()->getUndoList()->add(new GNEChange_TAZElement(TAZSource, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            net->getAttributeCarriers()->insertTAZElement(TAZSource);
            TAZSource->incRef("buildTAZSink");
        }
    }
    GNETAZElement* TAZSink = nullptr;
    // first check if a TAZSink in the same edge for the same TAZ
    for (const auto& TAZElement : TAZ->getChildTAZElements()) {
        if ((TAZElement->getTagProperty().getTag() == SUMO_TAG_TAZSINK) && (TAZElement->getAttribute(SUMO_ATTR_EDGE) == edge->getID())) {
            TAZSink = TAZElement;
        }
    }
    // check if TAZSink has to be created
    if (TAZSink == nullptr) {
        // Create TAZ only with arrivalWeight
        TAZSink = new GNETAZSourceSink(SUMO_TAG_TAZSINK, TAZ, edge, arrivalWeight);
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("add " + toString(SUMO_TAG_TAZSINK));
            net->getViewNet()->getUndoList()->add(new GNEChange_TAZElement(TAZSink, true), true);
            net->getViewNet()->getUndoList()->p_end();
        } else {
            net->getAttributeCarriers()->insertTAZElement(TAZSink);
            TAZSink->incRef("buildTAZSink");
        }
    } else {
        // update TAZ Attribute
        if (allowUndoRedo) {
            net->getViewNet()->getUndoList()->p_begin("update " + toString(SUMO_TAG_TAZSINK));
            TAZSink->setAttribute(SUMO_ATTR_WEIGHT, toString(arrivalWeight), net->getViewNet()->getUndoList());
            net->getViewNet()->getUndoList()->p_end();
        } else {
            TAZSink->setAttribute(SUMO_ATTR_WEIGHT, toString(arrivalWeight), nullptr);
            TAZSink->incRef("buildTAZSink");
        }
    }
    return TAZSink;
}


double
GNEAdditionalHandler::getPosition(double pos, GNELane& lane, bool friendlyPos, const std::string& additionalID) {
    if (pos < 0) {
        pos = lane.getLaneShapeLength() + pos;
    }
    if (pos > lane.getLaneShapeLength()) {
        if (friendlyPos) {
            pos = lane.getLaneShapeLength() - (double) 0.1;
        } else {
            WRITE_WARNING("The position of additional '" + additionalID + "' lies beyond the lane's '" + lane.getID() + "' length.");
        }
    }
    return pos;
}


bool GNEAdditionalHandler::checkAndFixDetectorPosition(double& pos, const double laneLength, const bool friendlyPos) {
    if (fabs(pos) > laneLength) {
        if (!friendlyPos) {
            return false;
        } else if (pos < 0) {
            pos = 0;
        } else if (pos > laneLength) {
            pos = laneLength - 0.01;
        }
    }
    return true;
}


bool GNEAdditionalHandler::fixE2DetectorPosition(double& pos, double& length, const double laneLength, const bool friendlyPos) {
    if ((pos < 0) || ((pos + length) > laneLength)) {
        if (!friendlyPos) {
            return false;
        } else if (pos < 0) {
            pos = 0;
        } else if (pos > laneLength) {
            pos = laneLength - 0.01;
            length = 0;
        } else if ((pos + length) > laneLength) {
            length = laneLength - pos - 0.01;
        }
    }
    return true;
}


bool
GNEAdditionalHandler::accessCanBeCreated(GNEAdditional* busStopParent, GNEEdge* edge) {
    // check that busStopParent is a busStop
    assert(busStopParent->getTagProperty().getTag() == SUMO_TAG_BUS_STOP);
    // check if exist another acces for the same busStop in the given edge
    for (auto i : busStopParent->getChildAdditionals()) {
        for (auto j : edge->getLanes()) {
            if (i->getAttribute(SUMO_ATTR_LANE) == j->getID()) {
                return false;
            }
        }
    }
    return true;
}


bool
GNEAdditionalHandler::checkOverlappingRerouterIntervals(GNEAdditional* rerouter, SUMOTime newBegin, SUMOTime newEnd) {
    // check that rerouter is correct
    assert(rerouter->getTagProperty().getTag() == SUMO_TAG_REROUTER);
    // declare a vector to keep sorted rerouter children
    std::vector<std::pair<SUMOTime, SUMOTime>> sortedIntervals;
    // iterate over child additional
    for (const auto& rerouterChild : rerouter->getChildAdditionals()) {
        if (!rerouterChild->getTagProperty().isSymbol()) {
            sortedIntervals.push_back(std::make_pair((SUMOTime)0., (SUMOTime)0.));
            // set begin and end
            sortedIntervals.back().first = TIME2STEPS(rerouterChild->getAttributeDouble(SUMO_ATTR_BEGIN));
            sortedIntervals.back().second = TIME2STEPS(rerouterChild->getAttributeDouble(SUMO_ATTR_END));
        }
    }
    // add new intervals
    sortedIntervals.push_back(std::make_pair(newBegin, newEnd));
    // sort children
    std::sort(sortedIntervals.begin(), sortedIntervals.end());
    // check overlapping after sorting
    for (int i = 0; i < (int)sortedIntervals.size() - 1; i++) {
        if (sortedIntervals.at(i).second > sortedIntervals.at(i + 1).first) {
            return false;
        }
    }
    return true;
}




bool
GNEAdditionalHandler::parseAndBuildVaporizer(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of Vaporizer
    const std::string edgeID = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_VAPORIZER, SUMO_ATTR_ID, abort);
    SUMOTime begin = GNEAttributeCarrier::parseAttributeFromXML<SUMOTime>(attrs, "", SUMO_TAG_VAPORIZER, SUMO_ATTR_BEGIN, abort);
    SUMOTime end = GNEAttributeCarrier::parseAttributeFromXML<SUMOTime>(attrs, "", SUMO_TAG_VAPORIZER, SUMO_ATTR_END, abort);
    std::string name = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_VAPORIZER, SUMO_ATTR_NAME, abort);
    // extra check for center element after creation
    bool centerAfterCreation = attrs.hasAttribute(GNE_ATTR_CENTER_AFTER_CREATION);
    // Continue if all parameters were successfully loaded
    if (!abort) {
        // get GNEEdge
        GNEEdge* edge = net->retrieveEdge(edgeID, false);
        // check that all parameters are valid
        if (edge == nullptr) {
            WRITE_WARNING("The edge '" + edgeID + "' to use within the " + toString(SUMO_TAG_VAPORIZER) + " is not known.");
        } else if (net->retrieveAdditional(SUMO_TAG_VAPORIZER, edgeID, false) != nullptr) {
            WRITE_WARNING("There is already a " + toString(SUMO_TAG_VAPORIZER) + " in the edge '" + edgeID + "'.");
        } else if (begin > end) {
            WRITE_WARNING("Time interval of " + toString(SUMO_TAG_VAPORIZER) + " isn't valid. Attribute '" + toString(SUMO_ATTR_BEGIN) + "' is greater than attribute '" + toString(SUMO_ATTR_END) + "'.");
        } else {
            // build vaporizer
            GNEAdditional* additionalCreated = buildVaporizer(net, allowUndoRedo, edge, begin, end, name, centerAfterCreation);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildTAZ(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of Vaporizer
    const std::string id = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_TAZ, SUMO_ATTR_ID, abort);
    const PositionVector shape = GNEAttributeCarrier::parseAttributeFromXML<PositionVector>(attrs, id, SUMO_TAG_TAZ, SUMO_ATTR_SHAPE, abort);
    RGBColor color = GNEAttributeCarrier::parseAttributeFromXML<RGBColor>(attrs, id, SUMO_TAG_TAZ, SUMO_ATTR_COLOR, abort);
    // parse Netedit attributes
    bool blockMovement = false;
    if (attrs.hasAttribute(GNE_ATTR_BLOCK_MOVEMENT)) {
        blockMovement = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, SUMO_TAG_TAZ, GNE_ATTR_BLOCK_MOVEMENT, abort);
    }
    // check edges
    std::vector<std::string> edgeIDs;
    if (attrs.hasAttribute(SUMO_ATTR_EDGES)) {
        std::string parsedAttribute = attrs.get<std::string>(SUMO_ATTR_EDGES, id.c_str(), abort, false);
        edgeIDs = GNEAttributeCarrier::parse<std::vector<std::string> >(parsedAttribute);
    }
    // check if all edge IDs are valid
    std::vector<GNEEdge*> edges;
    for (auto i : edgeIDs) {
        GNEEdge* edge = net->retrieveEdge(i, false);
        if (edge == nullptr) {
            WRITE_WARNING("Invalid " + toString(SUMO_TAG_EDGE) + " with ID = '" + i + "' within taz '" + id + "'.");
            abort = true;
        } else {
            edges.push_back(edge);
        }
    }
    // Continue if all parameters were successfully loaded
    if (!abort) {
        // check that all parameters are valid
        if (net->retrieveTAZElement(SUMO_TAG_TAZ, id, false) != nullptr) {
            WRITE_WARNING("There is another " + toString(SUMO_TAG_TAZ) + " with the same ID='" + id + "'.");
        } else {
            // save ID of last created element
            GNETAZElement* TAZElementCreated = buildTAZ(net, allowUndoRedo, id, shape, color, edges, blockMovement);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitTAZElementInsertion(TAZElementCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildTAZSource(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of Vaporizer
    const std::string edgeID = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_TAZSOURCE, SUMO_ATTR_ID, abort);
    const double departWeight = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, edgeID, SUMO_TAG_TAZSOURCE, SUMO_ATTR_WEIGHT, abort);
    // Continue if all parameters were successfully loaded
    if (!abort) {
        // get edge and TAZ
        GNEEdge* edge = net->retrieveEdge(edgeID, false);
        GNETAZElement* TAZ = nullptr;
        // obtain parent depending if we're loading or creating it using GNEAdditionalFrame
        if (insertedAdditionals) {
            TAZ = insertedAdditionals->getTAZElementParent(net, SUMO_TAG_TAZ);
        } else {
            bool ok = true;
            TAZ = net->retrieveTAZElement(SUMO_TAG_TAZ, attrs.get<std::string>(GNE_ATTR_PARENT, "", ok));
        }
        // check that all parameters are valid
        if (edge == nullptr) {
            WRITE_WARNING("The edge '" + edgeID + "' to use within the " + toString(SUMO_TAG_TAZSOURCE) + " is not known.");
        } else if (TAZ == nullptr) {
            WRITE_WARNING("A " + toString(SUMO_TAG_TAZSOURCE) + " must be declared within the definition of a " + toString(SUMO_TAG_TAZ) + ".");
        } else {
            // save ID of last created element
            GNETAZElement* additionalCreated = buildTAZSource(net, allowUndoRedo, TAZ, edge, departWeight);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitTAZElementInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildTAZSink(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of Vaporizer
    const std::string edgeID = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_TAZSINK, SUMO_ATTR_ID, abort);
    const double arrivalWeight = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, edgeID, SUMO_TAG_TAZSINK, SUMO_ATTR_WEIGHT, abort);
    // Continue if all parameters were successfully loaded
    if (!abort) {
        // get edge and TAZ
        GNEEdge* edge = net->retrieveEdge(edgeID, false);
        GNETAZElement* TAZ = nullptr;
        // obtain parent depending if we're loading or creating it using GNEAdditionalFrame
        if (insertedAdditionals) {
            TAZ = insertedAdditionals->getTAZElementParent(net, SUMO_TAG_TAZ);
        } else {
            bool ok = true;
            TAZ = net->retrieveTAZElement(SUMO_TAG_TAZ, attrs.get<std::string>(GNE_ATTR_PARENT, "", ok));
        }
        // check that all parameters are valid
        if (edge == nullptr) {
            WRITE_WARNING("The edge '" + edgeID + "' to use within the " + toString(SUMO_TAG_TAZSINK) + " is not known.");
        } else if (TAZ == nullptr) {
            WRITE_WARNING("A " + toString(SUMO_TAG_TAZSINK) + " must be declared within the definition of a " + toString(SUMO_TAG_TAZ) + ".");
        } else {
            // save ID of last created element
            GNETAZElement* additionalCreated = buildTAZSink(net, allowUndoRedo, TAZ, edge, arrivalWeight);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitTAZElementInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildRouteProbe(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of RouteProbe
    std::string id = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_ROUTEPROBE, SUMO_ATTR_ID, abort);
    std::string edgeId = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_ROUTEPROBE, SUMO_ATTR_EDGE, abort);
    std::string freq = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_ROUTEPROBE, SUMO_ATTR_FREQUENCY, abort);
    std::string name = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_ROUTEPROBE, SUMO_ATTR_NAME, abort);
    std::string file = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_ROUTEPROBE, SUMO_ATTR_FILE, abort);
    SUMOTime begin = GNEAttributeCarrier::parseAttributeFromXML<SUMOTime>(attrs, id, SUMO_TAG_ROUTEPROBE, SUMO_ATTR_BEGIN, abort);
    // extra check for center element after creation
    bool centerAfterCreation = attrs.hasAttribute(GNE_ATTR_CENTER_AFTER_CREATION);
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // get edge
        GNEEdge* edge = net->retrieveEdge(edgeId, false);
        // check that all elements are valid
        if (net->retrieveAdditional(SUMO_TAG_ROUTEPROBE, id, false) != nullptr) {
            WRITE_WARNING("There is another " + toString(SUMO_TAG_ROUTEPROBE) + " with the same ID='" + id + "'.");
        } else if (edge == nullptr) {
            // Write error if lane isn't valid
            WRITE_WARNING("The edge '" + edgeId + "' to use within the " + toString(SUMO_TAG_ROUTEPROBE) + " '" + id + "' is not known.");
        } else {
            // Freq needs an extra check, because it can be empty
            if (GNEAttributeCarrier::canParse<double>(freq)) {
                if (GNEAttributeCarrier::parse<double>(freq) < 0) {
                    WRITE_WARNING(toString(SUMO_ATTR_FREQUENCY) + "of " + toString(SUMO_TAG_ROUTEPROBE) + "'" + id + "' cannot be negative.");
                    freq = "";
                }
            } else {
                if (freq.empty()) {
                    WRITE_WARNING(toString(SUMO_ATTR_FREQUENCY) + "of " + toString(SUMO_TAG_ROUTEPROBE) + "'" + id + "' cannot be parsed to float.");
                }
                freq = "";
            }
            // save ID of last created element
            GNEAdditional* additionalCreated = buildRouteProbe(net, allowUndoRedo, id, edge, freq, name, file, begin, centerAfterCreation);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildCalibratorFlow(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of calibrator flows
    std::string vehicleTypeID = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_FLOW_CALIBRATOR, SUMO_ATTR_TYPE, abort);
    std::string routeID = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_FLOW_CALIBRATOR, SUMO_ATTR_ROUTE, abort);
    std::string vehsPerHour = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_FLOW_CALIBRATOR, SUMO_ATTR_VEHSPERHOUR, abort);
    std::string speed = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_FLOW_CALIBRATOR, SUMO_ATTR_SPEED, abort);
    RGBColor color = GNEAttributeCarrier::parseAttributeFromXML<RGBColor>(attrs, "", SUMO_TAG_FLOW_CALIBRATOR, SUMO_ATTR_COLOR, abort);
    std::string departLane = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_FLOW_CALIBRATOR, SUMO_ATTR_DEPARTLANE, abort);
    std::string departPos = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_FLOW_CALIBRATOR, SUMO_ATTR_DEPARTPOS, abort);
    std::string departSpeed = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_FLOW_CALIBRATOR, SUMO_ATTR_DEPARTSPEED, abort);
    std::string arrivalLane = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_FLOW_CALIBRATOR, SUMO_ATTR_ARRIVALLANE, abort);
    std::string arrivalPos = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_FLOW_CALIBRATOR, SUMO_ATTR_ARRIVALPOS, abort);
    std::string arrivalSpeed = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_FLOW_CALIBRATOR, SUMO_ATTR_ARRIVALSPEED, abort);
    std::string line = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_FLOW_CALIBRATOR, SUMO_ATTR_LINE, abort);
    int personNumber = GNEAttributeCarrier::parseAttributeFromXML<int>(attrs, "", SUMO_TAG_FLOW_CALIBRATOR, SUMO_ATTR_PERSON_NUMBER, abort);
    int containerNumber = GNEAttributeCarrier::parseAttributeFromXML<int>(attrs, "", SUMO_TAG_FLOW_CALIBRATOR, SUMO_ATTR_CONTAINER_NUMBER, abort);
    bool reroute = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, "", SUMO_TAG_FLOW_CALIBRATOR, SUMO_ATTR_REROUTE, abort);
    std::string departPosLat = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_FLOW_CALIBRATOR, SUMO_ATTR_DEPARTPOS_LAT, abort);
    std::string arrivalPosLat = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_FLOW_CALIBRATOR, SUMO_ATTR_ARRIVALPOS_LAT, abort);
    SUMOTime begin = GNEAttributeCarrier::parseAttributeFromXML<SUMOTime>(attrs, "", SUMO_TAG_FLOW_CALIBRATOR, SUMO_ATTR_BEGIN, abort);
    SUMOTime end = GNEAttributeCarrier::parseAttributeFromXML<SUMOTime>(attrs, "", SUMO_TAG_FLOW_CALIBRATOR, SUMO_ATTR_END, abort);
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // obtain route, vehicle type and calibrator parent
        GNEDemandElement* route = net->retrieveDemandElement(SUMO_TAG_ROUTE, routeID, false);
        GNEDemandElement* vtype = net->retrieveDemandElement(SUMO_TAG_VTYPE, vehicleTypeID, false);
        GNEAdditional* calibrator = nullptr;
        // obtain parent depending if we're loading or creating it using GNEAdditionalFrame
        if (insertedAdditionals) {
            calibrator = insertedAdditionals->getAdditionalParent(net, SUMO_TAG_CALIBRATOR);
        } else {
            bool ok = true;
            calibrator = net->retrieveAdditional(SUMO_TAG_CALIBRATOR, attrs.get<std::string>(GNE_ATTR_PARENT, "", ok));
        }
        // check that all elements are valid
        if (route == nullptr) {
            WRITE_WARNING(toString(SUMO_TAG_FLOW_CALIBRATOR) + " cannot be created; their " + toString(SUMO_TAG_ROUTE) + " with ID = '" + routeID + "' doesn't exist");
            abort = true;
        } else if (vtype == nullptr) {
            WRITE_WARNING(toString(SUMO_TAG_FLOW_CALIBRATOR) + " cannot be created; their " + toString(SUMO_TAG_VTYPE) + " with ID = '" + vehicleTypeID + "' doesn't exist");
            abort = true;
        } else if ((vehsPerHour.empty()) && (speed.empty())) {
            WRITE_WARNING(toString(SUMO_TAG_FLOW_CALIBRATOR) + " cannot be created; At least parameters " + toString(SUMO_ATTR_VEHSPERHOUR) + " or " + toString(SUMO_ATTR_SPEED) + " has to be defined");
            abort = true;
        } else if (calibrator != nullptr) {
            // save ID of last created element
            GNEAdditional* additionalCreated = buildCalibratorFlow(net, allowUndoRedo, calibrator, route, vtype, vehsPerHour, speed, color, departLane, departPos, departSpeed, arrivalLane, arrivalPos, arrivalSpeed,
                                               line, personNumber, containerNumber, reroute, departPosLat, arrivalPosLat, begin, end);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildVariableSpeedSign(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of VSS
    std::string id = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_VSS, SUMO_ATTR_ID, abort);
    std::string name = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_VSS, SUMO_ATTR_NAME, abort);
    GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_VSS, SUMO_ATTR_FILE, abort); // deprecated
    std::string lanesIDs = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_VSS, SUMO_ATTR_LANES, abort);
    Position pos = GNEAttributeCarrier::parseAttributeFromXML<Position>(attrs, id, SUMO_TAG_VSS, SUMO_ATTR_POSITION, abort);
    // parse Netedit attributes
    bool blockMovement = false;
    if (attrs.hasAttribute(GNE_ATTR_BLOCK_MOVEMENT)) {
        blockMovement = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, SUMO_TAG_VSS, GNE_ATTR_BLOCK_MOVEMENT, abort);
    }
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // obtain lanes
        std::vector<GNELane*> lanes;
        if (GNEAttributeCarrier::canParse<std::vector<GNELane*> >(net, lanesIDs, true)) {
            lanes = GNEAttributeCarrier::parse<std::vector<GNELane*> >(net, lanesIDs);
        }
        // check that all elements are valid
        if (net->retrieveAdditional(SUMO_TAG_VSS, id, false) != nullptr) {
            WRITE_WARNING("There is another " + toString(SUMO_TAG_VSS) + " with the same ID='" + id + "'.");
        } else if (lanes.size() == 0) {
            WRITE_WARNING("A Variable Speed Sign needs at least one lane.");
        } else {
            // save ID of last created element
            GNEAdditional* additionalCreated = buildVariableSpeedSign(net, allowUndoRedo, id, pos, lanes, name, blockMovement);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildVariableSpeedSignStep(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // Load step values
    double time = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, "", SUMO_TAG_STEP, SUMO_ATTR_TIME, abort);
    double speed = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, "", SUMO_TAG_STEP, SUMO_ATTR_SPEED, abort);
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // get Variable Speed Signal
        GNEAdditional* variableSpeedSign = nullptr;
        // obtain parent depending if we're loading or creating it using GNEAdditionalFrame
        if (insertedAdditionals) {
            variableSpeedSign = insertedAdditionals->getAdditionalParent(net, SUMO_TAG_VSS);
        } else {
            bool ok = true;
            variableSpeedSign = net->retrieveAdditional(SUMO_TAG_VSS, attrs.get<std::string>(GNE_ATTR_PARENT, "", ok));
        }
        // check that all parameters are valid
        if (variableSpeedSign != nullptr) {
            // save ID of last created element
            GNEAdditional* additionalCreated = buildVariableSpeedSignStep(net, allowUndoRedo, variableSpeedSign, time, speed);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildRerouter(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of Rerouter
    std::string id = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_REROUTER, SUMO_ATTR_ID, abort);
    std::string edgesIDs = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_REROUTER, SUMO_ATTR_EDGES, abort);
    std::string name = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_REROUTER, SUMO_ATTR_NAME, abort);
    std::string file = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_REROUTER, SUMO_ATTR_FILE, abort);
    double probability = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, id, SUMO_TAG_REROUTER, SUMO_ATTR_PROB, abort);
    bool off = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, SUMO_TAG_REROUTER, SUMO_ATTR_OFF, abort);
    SUMOTime timeThreshold = attrs.getOptSUMOTimeReporting(SUMO_ATTR_HALTING_TIME_THRESHOLD, id.c_str(), abort, 0);
    const std::string vTypes = attrs.getOpt<std::string>(SUMO_ATTR_VTYPES, id.c_str(), abort, "");
    Position pos = GNEAttributeCarrier::parseAttributeFromXML<Position>(attrs, id, SUMO_TAG_REROUTER, SUMO_ATTR_POSITION, abort);
    // parse Netedit attributes
    bool blockMovement = false;
    if (attrs.hasAttribute(GNE_ATTR_BLOCK_MOVEMENT)) {
        blockMovement = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, SUMO_TAG_REROUTER, GNE_ATTR_BLOCK_MOVEMENT, abort);
    }
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // obtain edges
        std::vector<GNEEdge*> edges;
        if (GNEAttributeCarrier::canParse<std::vector<GNEEdge*> >(net, edgesIDs, true)) {
            edges = GNEAttributeCarrier::parse<std::vector<GNEEdge*> >(net, edgesIDs);
        }
        // check that all elements are valid
        if (net->retrieveAdditional(SUMO_TAG_REROUTER, id, false) != nullptr) {
            WRITE_WARNING("There is another " + toString(SUMO_TAG_REROUTER) + " with the same ID='" + id + "'.");
        } else if (edges.size() == 0) {
            WRITE_WARNING("A rerouter needs at least one Edge");
        } else {
            // save ID of last created element
            GNEAdditional* additionalCreated = buildRerouter(net, allowUndoRedo, id, pos, edges, probability, name,
                                               file, off, timeThreshold, vTypes, blockMovement);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildRerouterInterval(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of Rerouter
    SUMOTime begin = GNEAttributeCarrier::parseAttributeFromXML<SUMOTime>(attrs, "", SUMO_TAG_INTERVAL, SUMO_ATTR_BEGIN, abort);
    SUMOTime end = GNEAttributeCarrier::parseAttributeFromXML<SUMOTime>(attrs, "", SUMO_TAG_INTERVAL, SUMO_ATTR_END, abort);
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // obtain rerouter
        GNEAdditional* rerouter;
        // obtain parent depending if we're loading or creating it using GNEAdditionalFrame
        if (insertedAdditionals) {
            rerouter = insertedAdditionals->getAdditionalParent(net, SUMO_TAG_REROUTER);
        } else {
            bool ok = true;
            rerouter = net->retrieveAdditional(SUMO_TAG_REROUTER, attrs.get<std::string>(GNE_ATTR_PARENT, "", ok));
        }
        // special case for load multiple intervals in the same rerouter
        if (rerouter == nullptr) {
            GNEAdditional* lastInsertedRerouterInterval = nullptr;
            // obtain parent depending if we're loading or creating it using GNEAdditionalFrame
            if (insertedAdditionals) {
                lastInsertedRerouterInterval = insertedAdditionals->getAdditionalParent(net, SUMO_TAG_INTERVAL);
            } else {
                bool ok = true;
                lastInsertedRerouterInterval = net->retrieveAdditional(SUMO_TAG_INTERVAL, attrs.get<std::string>(GNE_ATTR_PARENT, "", ok));
            }
            if (lastInsertedRerouterInterval) {
                rerouter = lastInsertedRerouterInterval->getParentAdditionals().at(0);
            }
        }
        // check that rerouterInterval can be created
        if (begin >= end) {
            WRITE_WARNING(toString(SUMO_TAG_INTERVAL) + " cannot be created; Attribute " + toString(SUMO_ATTR_END) + " must be greather than " + toString(SUMO_ATTR_BEGIN) + ".");
        } else if (rerouter != nullptr) {
            // save ID of last created element
            GNEAdditional* additionalCreated = buildRerouterInterval(net, allowUndoRedo, rerouter, begin, end);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildRerouterClosingLaneReroute(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of Rerouter
    std::string laneID = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_CLOSING_LANE_REROUTE, SUMO_ATTR_ID, abort);
    std::string allow = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_CLOSING_LANE_REROUTE, SUMO_ATTR_ALLOW, abort);
    std::string disallow = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_CLOSING_LANE_REROUTE, SUMO_ATTR_DISALLOW, abort);
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // obtain lane and rerouter interval
        GNELane* lane = net->retrieveLane(laneID, false, true);
        GNEAdditional* rerouterInterval = nullptr;
        // obtain parent depending if we're loading or creating it using GNEAdditionalFrame
        if (insertedAdditionals) {
            rerouterInterval = insertedAdditionals->getAdditionalParent(net, SUMO_TAG_INTERVAL);
        } else {
            bool ok = true;
            rerouterInterval = net->retrieveAdditional(SUMO_TAG_INTERVAL, attrs.get<std::string>(GNE_ATTR_PARENT, "", ok));
        }
        // check that all elements are valid
        if (lane == nullptr) {
            WRITE_WARNING("The lane '" + laneID + "' to use within the " + toString(SUMO_TAG_CLOSING_LANE_REROUTE) + " is not known.");
        } else if (rerouterInterval != nullptr) {
            // save ID of last created element
            GNEAdditional* additionalCreated = buildClosingLaneReroute(net, allowUndoRedo, rerouterInterval, lane, parseVehicleClasses(allow, disallow));
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildRerouterClosingReroute(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of Rerouter
    std::string edgeID = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_CLOSING_REROUTE, SUMO_ATTR_ID, abort);
    std::string allow = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_CLOSING_REROUTE, SUMO_ATTR_ALLOW, abort);
    std::string disallow = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_CLOSING_REROUTE, SUMO_ATTR_DISALLOW, abort);
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // obtain edge and rerouter interval
        GNEEdge* edge = net->retrieveEdge(edgeID, false);
        GNEAdditional* rerouterInterval = nullptr;
        // obtain parent depending if we're loading or creating it using GNEAdditionalFrame
        if (insertedAdditionals) {
            rerouterInterval = insertedAdditionals->getAdditionalParent(net, SUMO_TAG_INTERVAL);
        } else {
            bool ok = true;
            rerouterInterval = net->retrieveAdditional(SUMO_TAG_INTERVAL, attrs.get<std::string>(GNE_ATTR_PARENT, "", ok));
        }
        // check that all elements are valid
        if (edge == nullptr) {
            WRITE_WARNING("The edge '" + edgeID + "' to use within the " + toString(SUMO_TAG_CLOSING_REROUTE) + " is not known.");
        } else if (rerouterInterval != nullptr) {
            // save ID of last created element
            GNEAdditional* additionalCreated = buildClosingReroute(net, allowUndoRedo, rerouterInterval, edge, parseVehicleClasses(allow, disallow));
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildRerouterDestProbReroute(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of Rerouter
    std::string edgeID = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_DEST_PROB_REROUTE, SUMO_ATTR_ID, abort);
    double probability = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, "", SUMO_TAG_DEST_PROB_REROUTE, SUMO_ATTR_PROB, abort);
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // obtain edge and rerouter interval
        GNEEdge* edge = net->retrieveEdge(edgeID, false);
        GNEAdditional* rerouterInterval = nullptr;
        // obtain parent depending if we're loading or creating it using GNEAdditionalFrame
        if (insertedAdditionals) {
            rerouterInterval = insertedAdditionals->getAdditionalParent(net, SUMO_TAG_INTERVAL);
        } else {
            bool ok = true;
            rerouterInterval = net->retrieveAdditional(SUMO_TAG_INTERVAL, attrs.get<std::string>(GNE_ATTR_PARENT, "", ok));
        }
        // check that all elements are valid
        if (edge == nullptr) {
            WRITE_WARNING("The edge '" + edgeID + "' to use within the " + toString(SUMO_TAG_DEST_PROB_REROUTE) + " is not known.");
        } else if (rerouterInterval != nullptr) {
            // save ID of last created element
            GNEAdditional* additionalCreated = builDestProbReroute(net, allowUndoRedo, rerouterInterval, edge, probability);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildRerouterParkingAreaReroute(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of Rerouter
    std::string parkingAreaID = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_PARKING_ZONE_REROUTE, SUMO_ATTR_ID, abort);
    double probability = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, "", SUMO_TAG_PARKING_ZONE_REROUTE, SUMO_ATTR_PROB, abort);
    bool visible = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, "", SUMO_TAG_PARKING_ZONE_REROUTE, SUMO_ATTR_VISIBLE, abort);
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // obtain edge and rerouter interval
        GNEAdditional* parkingArea = net->retrieveAdditional(SUMO_TAG_PARKING_AREA, parkingAreaID, false);
        GNEAdditional* rerouterInterval = nullptr;
        // obtain parent depending if we're loading or creating it using GNEAdditionalFrame
        if (insertedAdditionals) {
            rerouterInterval = insertedAdditionals->getAdditionalParent(net, SUMO_TAG_INTERVAL);
        } else {
            bool ok = true;
            rerouterInterval = net->retrieveAdditional(SUMO_TAG_INTERVAL, attrs.get<std::string>(GNE_ATTR_PARENT, "", ok));
        }
        // check that all elements are valid
        if (parkingArea == nullptr) {
            WRITE_WARNING("The parkingArea '" + parkingAreaID + "' to use within the " + toString(SUMO_TAG_PARKING_ZONE_REROUTE) + " is not known.");
        } else if (rerouterInterval != nullptr) {
            // save ID of last created element
            GNEAdditional* additionalCreated = builParkingAreaReroute(net, allowUndoRedo, rerouterInterval, parkingArea, probability, visible);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildRerouterRouteProbReroute(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of Rerouter
    std::string routeID = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_ROUTE_PROB_REROUTE, SUMO_ATTR_ID, abort);
    double probability = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, "", SUMO_TAG_ROUTE_PROB_REROUTE, SUMO_ATTR_PROB, abort);
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // obtain rerouter interval
        GNEAdditional* rerouterInterval = nullptr;
        // obtain parent depending if we're loading or creating it using GNEAdditionalFrame
        if (insertedAdditionals) {
            rerouterInterval = insertedAdditionals->getAdditionalParent(net, SUMO_TAG_INTERVAL);
        } else {
            bool ok = true;
            rerouterInterval = net->retrieveAdditional(SUMO_TAG_INTERVAL, attrs.get<std::string>(GNE_ATTR_PARENT, "", ok));
        }
        // check that all elements are valid
        if (rerouterInterval != nullptr) {
            // save ID of last created element
            GNEAdditional* additionalCreated = buildRouteProbReroute(net, allowUndoRedo, rerouterInterval, routeID, probability);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildBusStop(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of bus stop
    std::string id = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_BUS_STOP, SUMO_ATTR_ID, abort);
    std::string laneId = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_BUS_STOP, SUMO_ATTR_LANE, abort);
    std::string startPos = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_BUS_STOP, SUMO_ATTR_STARTPOS, abort);
    std::string endPos = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_BUS_STOP, SUMO_ATTR_ENDPOS, abort);
    std::string name = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_BUS_STOP, SUMO_ATTR_NAME, abort);
    std::vector<std::string> lines = GNEAttributeCarrier::parseAttributeFromXML<std::vector<std::string> >(attrs, id, SUMO_TAG_BUS_STOP, SUMO_ATTR_LINES, abort);
    const int personCapacity = GNEAttributeCarrier::parseAttributeFromXML<int>(attrs, id, SUMO_TAG_BUS_STOP, SUMO_ATTR_PERSON_CAPACITY, abort);
    const double parkingLength = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, id, SUMO_TAG_BUS_STOP, SUMO_ATTR_PARKING_LENGTH, abort);
    bool friendlyPosition = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, SUMO_TAG_BUS_STOP, SUMO_ATTR_FRIENDLY_POS, abort);
    // parse Netedit attributes
    bool blockMovement = false;
    if (attrs.hasAttribute(GNE_ATTR_BLOCK_MOVEMENT)) {
        blockMovement = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, SUMO_TAG_BUS_STOP, GNE_ATTR_BLOCK_MOVEMENT, abort);
    }
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // get pointer to lane
        GNELane* lane = net->retrieveLane(laneId, false, true);
        // check that all elements are valid
        if (net->retrieveAdditional(SUMO_TAG_BUS_STOP, id, false) != nullptr) {
            WRITE_WARNING("There is another " + toString(SUMO_TAG_BUS_STOP) + " with the same ID='" + id + "'.");
        } else if (lane == nullptr) {
            // Write error if lane isn't valid
            WRITE_WARNING("The lane '" + laneId + "' to use within the " + toString(SUMO_TAG_BUS_STOP) + " '" + id + "' is not known.");
        } else {
            // declare variables for start and end position
            double startPosDouble = 0;
            double endPosDouble = lane->getParentEdge()->getNBEdge()->getFinalLength();
            const double stoppingPlaceLength = (endPosDouble - startPosDouble);
            int parametersSet = 0;
            // check if startPos and endPos were defined
            if (GNEAttributeCarrier::canParse<double>(startPos)) {
                startPosDouble = GNEAttributeCarrier::parse<double>(startPos);
                parametersSet |= STOPPINGPLACE_STARTPOS_SET;
            }
            if (GNEAttributeCarrier::canParse<double>(endPos)) {
                endPosDouble = GNEAttributeCarrier::parse<double>(endPos);
                parametersSet |= STOPPINGPLACE_ENDPOS_SET;
            }
            // check if stoppingPlace has to be adjusted
            SUMORouteHandler::StopPos checkStopPosResult = SUMORouteHandler::checkStopPos(startPosDouble, endPosDouble, lane->getParentEdge()->getNBEdge()->getFinalLength(), POSITION_EPS, friendlyPosition);
            // update start and end positions depending of checkStopPosResult
            if (checkStopPosResult == SUMORouteHandler::StopPos::STOPPOS_INVALID_STARTPOS) {
                startPosDouble = 0;
                endPosDouble = stoppingPlaceLength;
            } else if (checkStopPosResult == SUMORouteHandler::StopPos::STOPPOS_INVALID_ENDPOS) {
                startPosDouble = lane->getParentEdge()->getNBEdge()->getFinalLength() - stoppingPlaceLength;
                endPosDouble = lane->getParentEdge()->getNBEdge()->getFinalLength();
            } else if (checkStopPosResult == SUMORouteHandler::StopPos::STOPPOS_INVALID_LANELENGTH) {
                // Write error if position isn't valid
                WRITE_WARNING("Invalid position for " + toString(SUMO_TAG_BUS_STOP) + " with ID = '" + id + "'.");
                return false;
            }
            // save ID of last created element
            GNEAdditional* additionalCreated = buildBusStop(net, allowUndoRedo, id, lane, startPosDouble, endPosDouble, parametersSet,
                                               name, lines, personCapacity, parkingLength, friendlyPosition, blockMovement);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildContainerStop(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of container stop
    std::string id = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_CONTAINER_STOP, SUMO_ATTR_ID, abort);
    std::string laneId = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_CONTAINER_STOP, SUMO_ATTR_LANE, abort);
    std::string startPos = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_CONTAINER_STOP, SUMO_ATTR_STARTPOS, abort);
    std::string endPos = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_CONTAINER_STOP, SUMO_ATTR_ENDPOS, abort);
    std::string name = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_CONTAINER_STOP, SUMO_ATTR_NAME, abort);
    std::vector<std::string> lines = GNEAttributeCarrier::parseAttributeFromXML<std::vector<std::string> >(attrs, id, SUMO_TAG_CONTAINER_STOP, SUMO_ATTR_LINES, abort);
    bool friendlyPosition = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, SUMO_TAG_CONTAINER_STOP, SUMO_ATTR_FRIENDLY_POS, abort);
    // parse Netedit attributes
    bool blockMovement = false;
    if (attrs.hasAttribute(GNE_ATTR_BLOCK_MOVEMENT)) {
        blockMovement = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, SUMO_TAG_CONTAINER_STOP, GNE_ATTR_BLOCK_MOVEMENT, abort);
    }
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // get pointer to lane
        GNELane* lane = net->retrieveLane(laneId, false, true);
        // check that all elements are valid
        if (net->retrieveAdditional(SUMO_TAG_CONTAINER_STOP, id, false) != nullptr) {
            WRITE_WARNING("There is another " + toString(SUMO_TAG_CONTAINER_STOP) + " with the same ID='" + id + "'.");
        } else if (lane == nullptr) {
            // Write error if lane isn't valid
            WRITE_WARNING("The lane '" + laneId + "' to use within the " + toString(SUMO_TAG_CONTAINER_STOP) + " '" + id + "' is not known.");
        } else {
            // declare variables for start and end position
            double startPosDouble = 0;
            double endPosDouble = lane->getParentEdge()->getNBEdge()->getFinalLength();
            const double stoppingPlaceLength = (endPosDouble - startPosDouble);
            int parametersSet = 0;
            // check if startPos and endPos were defined
            if (GNEAttributeCarrier::canParse<double>(startPos)) {
                startPosDouble = GNEAttributeCarrier::parse<double>(startPos);
                parametersSet |= STOPPINGPLACE_STARTPOS_SET;
            }
            if (GNEAttributeCarrier::canParse<double>(endPos)) {
                endPosDouble = GNEAttributeCarrier::parse<double>(endPos);
                parametersSet |= STOPPINGPLACE_ENDPOS_SET;
            }
            // check if stoppingPlace has to be adjusted
            SUMORouteHandler::StopPos checkStopPosResult = SUMORouteHandler::checkStopPos(startPosDouble, endPosDouble, lane->getParentEdge()->getNBEdge()->getFinalLength(), POSITION_EPS, friendlyPosition);
            // update start and end positions depending of checkStopPosResult
            if (checkStopPosResult == SUMORouteHandler::StopPos::STOPPOS_INVALID_STARTPOS) {
                startPosDouble = 0;
                endPosDouble = stoppingPlaceLength;
            } else if (checkStopPosResult == SUMORouteHandler::StopPos::STOPPOS_INVALID_ENDPOS) {
                startPosDouble = lane->getParentEdge()->getNBEdge()->getFinalLength() - stoppingPlaceLength;
                endPosDouble = lane->getParentEdge()->getNBEdge()->getFinalLength();
            } else if (checkStopPosResult == SUMORouteHandler::StopPos::STOPPOS_INVALID_LANELENGTH) {
                // Write error if position isn't valid
                WRITE_WARNING("Invalid position for " + toString(SUMO_TAG_CONTAINER_STOP) + " with ID = '" + id + "'.");
                return false;
            }
            // save ID of last created element
            GNEAdditional* additionalCreated = buildContainerStop(net, allowUndoRedo, id, lane, startPosDouble, endPosDouble, parametersSet,
                                               name, lines, friendlyPosition, blockMovement);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildAccess(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of Entry
    std::string laneId = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_ACCESS, SUMO_ATTR_LANE, abort);
    std::string position = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_ACCESS, SUMO_ATTR_POSITION, abort);
    std::string length = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_ACCESS, SUMO_ATTR_LENGTH, abort);
    bool friendlyPos = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, "", SUMO_TAG_ACCESS, SUMO_ATTR_FRIENDLY_POS, abort);
    // parse Netedit attributes
    bool blockMovement = false;
    if (attrs.hasAttribute(GNE_ATTR_BLOCK_MOVEMENT)) {
        blockMovement = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, "", SUMO_TAG_ACCESS, GNE_ATTR_BLOCK_MOVEMENT, abort);
    }
    // Check if parsing of parameters was correct
    if (!abort) {
        double posDouble = GNEAttributeCarrier::parse<double>(position);
        // get lane and busStop parent
        GNELane* lane = net->retrieveLane(laneId, false, true);
        GNEAdditional* busStop = nullptr;
        // obtain parent depending if we're loading or creating it using GNEAdditionalFrame
        if (insertedAdditionals) {
            busStop = insertedAdditionals->getAdditionalParent(net, SUMO_TAG_BUS_STOP);
        } else {
            bool ok = true;
            busStop = net->retrieveAdditional(SUMO_TAG_BUS_STOP, attrs.get<std::string>(GNE_ATTR_PARENT, "", ok));
        }
        // check that all parameters are valid
        if (lane == nullptr) {
            WRITE_WARNING("The lane '" + laneId + "' to use within the " + toString(SUMO_TAG_ACCESS) + " is not known.");
        } else if (busStop == nullptr) {
            WRITE_WARNING("A " + toString(SUMO_TAG_ACCESS) + " must be declared within the definition of a " + toString(SUMO_TAG_BUS_STOP) + ".");
        } else if (!checkAndFixDetectorPosition(posDouble, lane->getLaneShapeLength(), friendlyPos)) {
            WRITE_WARNING("Invalid position for " + toString(SUMO_TAG_ACCESS) + ".");
        } else if (!accessCanBeCreated(busStop, lane->getParentEdge())) {
            WRITE_WARNING("Edge '" + lane->getParentEdge()->getID() + "' already has an Access for busStop '" + busStop->getID() + "'");
        } else {
            // save ID of last created element
            GNEAdditional* additionalCreated = buildAccess(net, allowUndoRedo, busStop, lane, posDouble, length, friendlyPos, blockMovement);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildChargingStation(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of charging station
    std::string id = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_CHARGING_STATION, SUMO_ATTR_ID, abort);
    std::string laneId = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_CHARGING_STATION, SUMO_ATTR_LANE, abort);
    std::string startPos = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_CHARGING_STATION, SUMO_ATTR_STARTPOS, abort);
    std::string endPos = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_CHARGING_STATION, SUMO_ATTR_ENDPOS, abort);
    std::string name = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_CHARGING_STATION, SUMO_ATTR_NAME, abort);
    double chargingPower = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, id, SUMO_TAG_CHARGING_STATION, SUMO_ATTR_CHARGINGPOWER, abort);
    double efficiency = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, id, SUMO_TAG_CHARGING_STATION, SUMO_ATTR_EFFICIENCY, abort);
    bool chargeInTransit = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, SUMO_TAG_CHARGING_STATION, SUMO_ATTR_CHARGEINTRANSIT, abort);
    SUMOTime chargeDelay = GNEAttributeCarrier::parseAttributeFromXML<SUMOTime>(attrs, id, SUMO_TAG_CHARGING_STATION, SUMO_ATTR_CHARGEDELAY, abort);
    bool friendlyPosition = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, SUMO_TAG_CHARGING_STATION, SUMO_ATTR_FRIENDLY_POS, abort);
    // parse Netedit attributes
    bool blockMovement = false;
    if (attrs.hasAttribute(GNE_ATTR_BLOCK_MOVEMENT)) {
        blockMovement = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, SUMO_TAG_CHARGING_STATION, GNE_ATTR_BLOCK_MOVEMENT, abort);
    }
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // get pointer to lane
        GNELane* lane = net->retrieveLane(laneId, false, true);
        // check that all elements are valid
        if (net->retrieveAdditional(SUMO_TAG_CHARGING_STATION, id, false) != nullptr) {
            WRITE_WARNING("There is another " + toString(SUMO_TAG_CHARGING_STATION) + " with the same ID='" + id + "'.");
        } else if (lane == nullptr) {
            // Write error if lane isn't valid
            WRITE_WARNING("The lane '" + laneId + "' to use within the " + toString(SUMO_TAG_CHARGING_STATION) + " '" + id + "' is not known.");
        } else {
            // declare variables for start and end position
            double startPosDouble = 0;
            double endPosDouble = lane->getParentEdge()->getNBEdge()->getFinalLength();
            const double stoppingPlaceLength = (endPosDouble - startPosDouble);
            int parametersSet = 0;
            // check if startPos and endPos were defined
            if (GNEAttributeCarrier::canParse<double>(startPos)) {
                startPosDouble = GNEAttributeCarrier::parse<double>(startPos);
                parametersSet |= STOPPINGPLACE_STARTPOS_SET;
            }
            if (GNEAttributeCarrier::canParse<double>(endPos)) {
                endPosDouble = GNEAttributeCarrier::parse<double>(endPos);
                parametersSet |= STOPPINGPLACE_ENDPOS_SET;
            }
            // check if stoppingPlace has to be adjusted
            SUMORouteHandler::StopPos checkStopPosResult = SUMORouteHandler::checkStopPos(startPosDouble, endPosDouble, lane->getParentEdge()->getNBEdge()->getFinalLength(), POSITION_EPS, friendlyPosition);
            // update start and end positions depending of checkStopPosResult
            if (checkStopPosResult == SUMORouteHandler::StopPos::STOPPOS_INVALID_STARTPOS) {
                startPosDouble = 0;
                endPosDouble = stoppingPlaceLength;
            } else if (checkStopPosResult == SUMORouteHandler::StopPos::STOPPOS_INVALID_ENDPOS) {
                startPosDouble = lane->getParentEdge()->getNBEdge()->getFinalLength() - stoppingPlaceLength;
                endPosDouble = lane->getParentEdge()->getNBEdge()->getFinalLength();
            } else if (checkStopPosResult == SUMORouteHandler::StopPos::STOPPOS_INVALID_LANELENGTH) {
                // Write error if position isn't valid
                WRITE_WARNING("Invalid position for " + toString(SUMO_TAG_CHARGING_STATION) + " with ID = '" + id + "'.");
                return false;
            }
            // save ID of last created element
            GNEAdditional* additionalCreated = buildChargingStation(net, allowUndoRedo, id, lane, startPosDouble, endPosDouble, parametersSet,
                                               name, chargingPower, efficiency, chargeInTransit, chargeDelay, friendlyPosition, blockMovement);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildParkingArea(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of charging station
    std::string id = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_PARKING_AREA, SUMO_ATTR_ID, abort);
    std::string laneId = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_PARKING_AREA, SUMO_ATTR_LANE, abort);
    std::string startPos = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_PARKING_AREA, SUMO_ATTR_STARTPOS, abort);
    std::string endPos = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_PARKING_AREA, SUMO_ATTR_ENDPOS, abort);
    std::string name = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_PARKING_AREA, SUMO_ATTR_NAME, abort);
    bool friendlyPosition = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, SUMO_TAG_PARKING_AREA, SUMO_ATTR_FRIENDLY_POS, abort);
    int roadSideCapacity = GNEAttributeCarrier::parseAttributeFromXML<int>(attrs, id, SUMO_TAG_PARKING_AREA, SUMO_ATTR_ROADSIDE_CAPACITY, abort);
    bool onRoad = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, SUMO_TAG_PARKING_AREA, SUMO_ATTR_ONROAD, abort);
    double width = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, id, SUMO_TAG_PARKING_AREA, SUMO_ATTR_WIDTH, abort);
    std::string length = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_PARKING_AREA, SUMO_ATTR_LENGTH, abort);
    double angle = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, id, SUMO_TAG_PARKING_AREA, SUMO_ATTR_ANGLE, abort);
    // parse Netedit attributes
    bool blockMovement = false;
    if (attrs.hasAttribute(GNE_ATTR_BLOCK_MOVEMENT)) {
        blockMovement = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, SUMO_TAG_PARKING_AREA, GNE_ATTR_BLOCK_MOVEMENT, abort);
    }
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // get pointer to lane
        GNELane* lane = net->retrieveLane(laneId, false, true);
        // check that all elements are valid
        if (net->retrieveAdditional(SUMO_TAG_PARKING_AREA, id, false) != nullptr) {
            WRITE_WARNING("There is another " + toString(SUMO_TAG_PARKING_AREA) + " with the same ID='" + id + "'.");
        } else if (lane == nullptr) {
            // Write error if lane isn't valid
            WRITE_WARNING("The lane '" + laneId + "' to use within the " + toString(SUMO_TAG_PARKING_AREA) + " '" + id + "' is not known.");
        } else {
            // declare variables for start and end position
            double startPosDouble = 0;
            double endPosDouble = lane->getParentEdge()->getNBEdge()->getFinalLength();
            const double stoppingPlaceLength = (endPosDouble - startPosDouble);
            int parametersSet = 0;
            // check if startPos and endPos were defined
            if (GNEAttributeCarrier::canParse<double>(startPos)) {
                startPosDouble = GNEAttributeCarrier::parse<double>(startPos);
                parametersSet |= STOPPINGPLACE_STARTPOS_SET;
            }
            if (GNEAttributeCarrier::canParse<double>(endPos)) {
                endPosDouble = GNEAttributeCarrier::parse<double>(endPos);
                parametersSet |= STOPPINGPLACE_ENDPOS_SET;
            }
            // check if stoppingPlace has to be adjusted
            SUMORouteHandler::StopPos checkStopPosResult = SUMORouteHandler::checkStopPos(startPosDouble, endPosDouble, lane->getParentEdge()->getNBEdge()->getFinalLength(), POSITION_EPS, friendlyPosition);
            // update start and end positions depending of checkStopPosResult
            if (checkStopPosResult == SUMORouteHandler::StopPos::STOPPOS_INVALID_STARTPOS) {
                startPosDouble = 0;
                endPosDouble = stoppingPlaceLength;
            } else if (checkStopPosResult == SUMORouteHandler::StopPos::STOPPOS_INVALID_ENDPOS) {
                startPosDouble = lane->getParentEdge()->getNBEdge()->getFinalLength() - stoppingPlaceLength;
                endPosDouble = lane->getParentEdge()->getNBEdge()->getFinalLength();
            } else if (checkStopPosResult == SUMORouteHandler::StopPos::STOPPOS_INVALID_LANELENGTH) {
                // Write error if position isn't valid
                WRITE_WARNING("Invalid position for " + toString(SUMO_TAG_PARKING_AREA) + " with ID = '" + id + "'.");
                return false;
            }
            // save ID of last created element
            GNEAdditional* additionalCreated = buildParkingArea(net, allowUndoRedo, id, lane, startPosDouble, endPosDouble, parametersSet,
                                               name, friendlyPosition, roadSideCapacity, onRoad, width, length, angle, blockMovement);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildParkingSpace(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of Parking Spaces
    Position pos = GNEAttributeCarrier::parseAttributeFromXML<Position>(attrs, "", SUMO_TAG_PARKING_SPACE, SUMO_ATTR_POSITION, abort);
    double width = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, "", SUMO_TAG_PARKING_SPACE, SUMO_ATTR_WIDTH, abort);
    double length = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, "", SUMO_TAG_PARKING_SPACE, SUMO_ATTR_LENGTH, abort);
    double angle = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, "", SUMO_TAG_PARKING_SPACE, SUMO_ATTR_ANGLE, abort);
    // parse Netedit attributes
    bool blockMovement = false;
    if (attrs.hasAttribute(GNE_ATTR_BLOCK_MOVEMENT)) {
        blockMovement = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, "", SUMO_TAG_PARKING_SPACE, GNE_ATTR_BLOCK_MOVEMENT, abort);
    }
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // get Parking Area Parent
        GNEAdditional* parkingAreaParent = nullptr;
        // obtain parent depending if we're loading or creating it using GNEAdditionalFrame
        if (insertedAdditionals) {
            parkingAreaParent = insertedAdditionals->getAdditionalParent(net, SUMO_TAG_PARKING_AREA);
        } else {
            bool ok = true;
            parkingAreaParent = net->retrieveAdditional(SUMO_TAG_PARKING_AREA, attrs.get<std::string>(GNE_ATTR_PARENT, "", ok));
        }
        // check that Parking Area Parent exists
        if (parkingAreaParent != nullptr) {
            // save ID of last created element
            GNEAdditional* additionalCreated = buildParkingSpace(net, allowUndoRedo, parkingAreaParent, pos, width, length, angle, blockMovement);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildCalibrator(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // due there is two differents calibrators, has to be parsed in a different way
    std::string edgeID, laneId, id;
    // change tag depending of XML parmeters
    if (attrs.hasAttribute(SUMO_ATTR_EDGE)) {
        id = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_CALIBRATOR, SUMO_ATTR_ID, abort);
        edgeID = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_CALIBRATOR, SUMO_ATTR_EDGE, abort);
        std::string outfile = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_CALIBRATOR, SUMO_ATTR_OUTPUT, abort);
        double position = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, id, SUMO_TAG_CALIBRATOR, SUMO_ATTR_POSITION, abort);
        std::string name = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_CALIBRATOR, SUMO_ATTR_NAME, abort);
        SUMOTime freq = GNEAttributeCarrier::parseAttributeFromXML<SUMOTime>(attrs, id, SUMO_TAG_CALIBRATOR, SUMO_ATTR_FREQUENCY, abort);
        std::string routeProbe = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_CALIBRATOR, SUMO_ATTR_ROUTEPROBE, abort);
        // extra check for center element after creation
        bool centerAfterCreation = attrs.hasAttribute(GNE_ATTR_CENTER_AFTER_CREATION);
        // Continue if all parameters were sucesfully loaded
        if (!abort) {
            // get pointer and edge
            GNEEdge* edge = net->retrieveEdge(edgeID, false);
            // check that all elements are valid
            if (net->retrieveAdditional(SUMO_TAG_CALIBRATOR, id, false) != nullptr) {
                WRITE_WARNING("There is another " + toString(SUMO_TAG_CALIBRATOR) + " with the same ID='" + id + "'.");
            } else if (edge == nullptr) {
                WRITE_WARNING("The  edge '" + edgeID + "' to use within the " + toString(SUMO_TAG_CALIBRATOR) + " '" + id + "' is not known.");
            } else {
                // save ID of last created element
                GNEAdditional* additionalCreated = buildCalibrator(net, allowUndoRedo, id, edge, position, name, outfile, freq, routeProbe, centerAfterCreation);
                // check if insertion has to be commited
                if (insertedAdditionals) {
                    insertedAdditionals->commitAdditionalInsertion(additionalCreated);
                }
                return true;
            }
        }
    } else if (attrs.hasAttribute(SUMO_ATTR_LANE)) {
        id = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_LANECALIBRATOR, SUMO_ATTR_ID, abort);
        laneId = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_LANECALIBRATOR, SUMO_ATTR_LANE, abort);
        std::string outfile = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_LANECALIBRATOR, SUMO_ATTR_OUTPUT, abort);
        double position = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, id, SUMO_TAG_LANECALIBRATOR, SUMO_ATTR_POSITION, abort);
        std::string name = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_LANECALIBRATOR, SUMO_ATTR_NAME, abort);
        SUMOTime freq = GNEAttributeCarrier::parseAttributeFromXML<SUMOTime>(attrs, id, SUMO_TAG_LANECALIBRATOR, SUMO_ATTR_FREQUENCY, abort);
        std::string routeProbe = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_LANECALIBRATOR, SUMO_ATTR_ROUTEPROBE, abort);
        // extra check for center element after creation
        bool centerAfterCreation = attrs.hasAttribute(GNE_ATTR_CENTER_AFTER_CREATION);
        // Continue if all parameters were sucesfully loaded
        if (!abort) {
            // get pointer to lane
            GNELane* lane = net->retrieveLane(laneId, false, true);
            // check that all elements are valid
            if (net->retrieveAdditional(SUMO_TAG_LANECALIBRATOR, id, false) != nullptr) {
                WRITE_WARNING("There is another " + toString(SUMO_TAG_CALIBRATOR) + " with the same ID='" + id + "'.");
            } else if (lane == nullptr) {
                WRITE_WARNING("The lane '" + laneId + "' to use within the " + toString(SUMO_TAG_CALIBRATOR) + " '" + id + "' is not known.");
            } else {
                // save ID of last created element
                GNEAdditional* additionalCreated = buildCalibrator(net, allowUndoRedo, id, lane, position, name, outfile, freq, routeProbe, centerAfterCreation);
                // check if insertion has to be commited
                if (insertedAdditionals) {
                    insertedAdditionals->commitAdditionalInsertion(additionalCreated);
                }
                return true;
            }
        }
    } else {
        WRITE_WARNING("additional " + toString(SUMO_TAG_CALIBRATOR) + " must have either a lane or an edge attribute.");
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildDetectorE1(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of E1
    std::string id = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_E1DETECTOR, SUMO_ATTR_ID, abort);
    std::string laneId = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_E1DETECTOR, SUMO_ATTR_LANE, abort);
    double position = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, id, SUMO_TAG_E1DETECTOR, SUMO_ATTR_POSITION, abort);
    SUMOTime frequency = GNEAttributeCarrier::parseAttributeFromXML<SUMOTime>(attrs, id, SUMO_TAG_E1DETECTOR, SUMO_ATTR_FREQUENCY, abort);
    std::string file = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_E1DETECTOR, SUMO_ATTR_FILE, abort);
    std::string vehicleTypes = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_E1DETECTOR, SUMO_ATTR_VTYPES, abort);
    std::string name = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_E1DETECTOR, SUMO_ATTR_NAME, abort);
    bool friendlyPos = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, SUMO_TAG_E1DETECTOR, SUMO_ATTR_FRIENDLY_POS, abort);
    // parse Netedit attributes
    bool blockMovement = false;
    if (attrs.hasAttribute(GNE_ATTR_BLOCK_MOVEMENT)) {
        blockMovement = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, SUMO_TAG_E1DETECTOR, GNE_ATTR_BLOCK_MOVEMENT, abort);
    }
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // get pointer to lane
        GNELane* lane = net->retrieveLane(laneId, false, true);
        // check that all elements are valid
        if (net->retrieveAdditional(SUMO_TAG_E1DETECTOR, id, false) != nullptr) {
            WRITE_WARNING("There is another " + toString(SUMO_TAG_E1DETECTOR) + " with the same ID='" + id + "'.");
        } else if (lane == nullptr) {
            // Write error if lane isn't valid
            WRITE_WARNING("The lane '" + laneId + "' to use within the " + toString(SUMO_TAG_E1DETECTOR) + " '" + id + "' is not known.");
        } else if (!checkAndFixDetectorPosition(position, lane->getLaneShapeLength(), friendlyPos)) {
            WRITE_WARNING("Invalid position for " + toString(SUMO_TAG_E1DETECTOR) + " with ID = '" + id + "'.");
        } else {
            // save ID of last created element
            GNEAdditional* additionalCreated = buildDetectorE1(net, allowUndoRedo, id, lane, position, frequency, file, vehicleTypes, name, friendlyPos, blockMovement);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildDetectorE2(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    // Tag E2 detectors can build either E2 single lanes or E2 multilanes, depending of attribute "lanes"
    SumoXMLTag E2Tag = attrs.hasAttribute(SUMO_ATTR_LANES) ? SUMO_TAG_E2DETECTOR_MULTILANE : SUMO_TAG_E2DETECTOR;
    bool abort = false;
    // start parsing ID
    std::string id = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", E2Tag, SUMO_ATTR_ID, abort);
    // parse attributes of E2 SingleLanes
    std::string laneId = (E2Tag == SUMO_TAG_E2DETECTOR) ? GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, E2Tag, SUMO_ATTR_LANE, abort) : "";
    double length = (E2Tag == SUMO_TAG_E2DETECTOR) ? GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, id, E2Tag, SUMO_ATTR_LENGTH, abort) : 0;
    // parse attributes of E2 Multilanes
    std::string laneIds = (E2Tag == SUMO_TAG_E2DETECTOR_MULTILANE) ? GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, E2Tag, SUMO_ATTR_LANES, abort) : "";
    double endPos = (E2Tag == SUMO_TAG_E2DETECTOR_MULTILANE) ? GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, id, E2Tag, SUMO_ATTR_ENDPOS, abort) : 0;
    // parse common attributes
    double position = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, id, E2Tag, SUMO_ATTR_POSITION, abort);
    std::string frequency = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, E2Tag, SUMO_ATTR_FREQUENCY, abort);
    const std::string trafficLight = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, E2Tag, SUMO_ATTR_TLID, abort);
    std::string file = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, E2Tag, SUMO_ATTR_FILE, abort);
    std::string vehicleTypes = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, E2Tag, SUMO_ATTR_VTYPES, abort);
    std::string name = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, E2Tag, SUMO_ATTR_NAME, abort);
    SUMOTime haltingTimeThreshold = GNEAttributeCarrier::parseAttributeFromXML<SUMOTime>(attrs, id, E2Tag, SUMO_ATTR_HALTING_TIME_THRESHOLD, abort);
    double haltingSpeedThreshold = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, id, E2Tag, SUMO_ATTR_HALTING_SPEED_THRESHOLD, abort);
    double jamDistThreshold = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, id, E2Tag, SUMO_ATTR_JAM_DIST_THRESHOLD, abort);
    bool friendlyPos = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, E2Tag, SUMO_ATTR_FRIENDLY_POS, abort);
    // parse Netedit attributes
    bool blockMovement = false;
    if (attrs.hasAttribute(GNE_ATTR_BLOCK_MOVEMENT)) {
        blockMovement = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, E2Tag, GNE_ATTR_BLOCK_MOVEMENT, abort);
    }
    // cont attribute is deprecated
    GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, E2Tag, SUMO_ATTR_CONT, abort);
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // check if at leas lane or laneIDS are defined
        if (laneId.empty() && laneIds.empty()) {
            WRITE_WARNING("A " + toString(E2Tag) + " needs at least a lane or a list of lanes.");
        } else if (!frequency.empty() && !trafficLight.empty()) {
            WRITE_WARNING("Frecuency and TL cannot be defined at the same time.");
        } else {
            // update frequency (temporal)
            if (frequency.empty() && trafficLight.empty()) {
                frequency = "900.00";
            }
            // get pointer to lane
            GNELane* lane = net->retrieveLane(laneId, false, true);
            // get list of lanes
            std::vector<GNELane*> lanes;
            bool laneConsecutives = true;
            if (GNEAttributeCarrier::canParse<std::vector<GNELane*> >(net, laneIds, false)) {
                lanes = GNEAttributeCarrier::parse<std::vector<GNELane*> >(net, laneIds);
                // check if lanes are consecutives
                laneConsecutives = GNEAttributeCarrier::lanesConsecutives(lanes);
            }
            // check that all elements are valid
            if (net->retrieveAdditional(E2Tag, id, false) != nullptr) {
                // write error if neither lane nor lane aren't defined
                WRITE_WARNING("There is another " + toString(E2Tag) + " with the same ID='" + id + "'.");
            } else if (attrs.hasAttribute(SUMO_ATTR_LANE) && (lane == nullptr)) {
                // Write error if lane isn't valid
                WRITE_WARNING("The lane '" + laneId + "' to use within the " + toString(E2Tag) + " '" + id + "' is not known.");
            } else if (attrs.hasAttribute(SUMO_ATTR_LANES) && lanes.empty()) {
                // Write error if lane isn't valid
                WRITE_WARNING("The list of lanes cannot be empty.");
            } else if (attrs.hasAttribute(SUMO_ATTR_LANES) && lanes.empty()) {
                // Write error if lane isn't valid
                WRITE_WARNING("The list of lanes '" + laneIds + "' to use within the " + toString(E2Tag) + " '" + id + "' isn't valid.");
            } else if (!lanes.empty() && !laneConsecutives) {
                WRITE_WARNING("The lanes '" + laneIds + "' to use within the " + toString(E2Tag) + " '" + id + "' aren't consecutives.");
            } else if (lane && !fixE2DetectorPosition(position, length, lane->getParentEdge()->getNBEdge()->getFinalLength(), friendlyPos)) {
                WRITE_WARNING("Invalid position for " + toString(E2Tag) + " with ID = '" + id + "'.");
            } else if (!lanes.empty() && !fixE2DetectorPosition(position, length, lanes.front()->getParentEdge()->getNBEdge()->getFinalLength(), friendlyPos)) {
                WRITE_WARNING("Invalid position for " + toString(E2Tag) + " with ID = '" + id + "'.");
            } else if (!lanes.empty() && !fixE2DetectorPosition(endPos, length, lanes.back()->getParentEdge()->getNBEdge()->getFinalLength(), friendlyPos)) {
                WRITE_WARNING("Invalid end position for " + toString(E2Tag) + " with ID = '" + id + "'.");
            } else if (lane) {
                // save ID of last created element
                GNEAdditional* additionalCreated = buildSingleLaneDetectorE2(net, allowUndoRedo, id, lane, position, length, frequency, trafficLight, file, vehicleTypes,
                                                   name, haltingTimeThreshold, haltingSpeedThreshold, jamDistThreshold, friendlyPos, blockMovement);
                // check if insertion has to be commited
                if (insertedAdditionals) {
                    insertedAdditionals->commitAdditionalInsertion(additionalCreated);
                }
                return true;
            } else {
                // save ID of last created element
                GNEAdditional* additionalCreated = buildMultiLaneDetectorE2(net, allowUndoRedo, id, lanes, position, endPos, frequency, trafficLight, file, vehicleTypes,
                                                   name, haltingTimeThreshold, haltingSpeedThreshold, jamDistThreshold, friendlyPos, blockMovement);
                // check if insertion has to be commited
                if (insertedAdditionals) {
                    insertedAdditionals->commitAdditionalInsertion(additionalCreated);
                }
                return true;
            }
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildDetectorE3(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of E3
    std::string id = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_E3DETECTOR, SUMO_ATTR_ID, abort);
    SUMOTime frequency = GNEAttributeCarrier::parseAttributeFromXML<SUMOTime>(attrs, id, SUMO_TAG_E3DETECTOR, SUMO_ATTR_FREQUENCY, abort);
    std::string file = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_E3DETECTOR, SUMO_ATTR_FILE, abort);
    std::string vehicleTypes = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_E3DETECTOR, SUMO_ATTR_VTYPES, abort);
    std::string name = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_E3DETECTOR, SUMO_ATTR_NAME, abort);
    SUMOTime haltingTimeThreshold = GNEAttributeCarrier::parseAttributeFromXML<SUMOTime>(attrs, id, SUMO_TAG_E3DETECTOR, SUMO_ATTR_HALTING_TIME_THRESHOLD, abort);
    double haltingSpeedThreshold = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, id, SUMO_TAG_E3DETECTOR, SUMO_ATTR_HALTING_SPEED_THRESHOLD, abort);
    Position pos = GNEAttributeCarrier::parseAttributeFromXML<Position>(attrs, id, SUMO_TAG_E3DETECTOR, SUMO_ATTR_POSITION, abort);
    // parse Netedit attributes
    bool blockMovement = false;
    if (attrs.hasAttribute(GNE_ATTR_BLOCK_MOVEMENT)) {
        blockMovement = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, SUMO_TAG_E3DETECTOR, GNE_ATTR_BLOCK_MOVEMENT, abort);
    }
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // check that all elements are valid
        if (net->retrieveAdditional(SUMO_TAG_E3DETECTOR, id, false) != nullptr) {
            WRITE_WARNING("There is another " + toString(SUMO_TAG_E3DETECTOR) + " with the same ID='" + id + "'.");
        } else {
            // save ID of last created element
            GNEAdditional* additionalCreated = buildDetectorE3(net, allowUndoRedo, id, pos, frequency, file, vehicleTypes, name, haltingTimeThreshold, haltingSpeedThreshold, blockMovement);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildDetectorEntry(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of Entry
    std::string laneId = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_DET_ENTRY, SUMO_ATTR_LANE, abort);
    double position = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, "", SUMO_TAG_DET_ENTRY, SUMO_ATTR_POSITION, abort);
    bool friendlyPos = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, "", SUMO_TAG_DET_ENTRY, SUMO_ATTR_FRIENDLY_POS, abort);
    // parse Netedit attributes
    bool blockMovement = false;
    if (attrs.hasAttribute(GNE_ATTR_BLOCK_MOVEMENT)) {
        blockMovement = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, "", SUMO_TAG_DET_ENTRY, GNE_ATTR_BLOCK_MOVEMENT, abort);
    }
    // Check if parsing of parameters was correct
    if (!abort) {
        // get lane and E3 parent
        GNELane* lane = net->retrieveLane(laneId, false, true);
        GNEAdditional* E3Parent = nullptr;
        // obtain parent depending if we're loading or creating it using GNEAdditionalFrame
        if (insertedAdditionals) {
            E3Parent = insertedAdditionals->getAdditionalParent(net, SUMO_TAG_E3DETECTOR);
        } else {
            bool ok = true;
            E3Parent = net->retrieveAdditional(SUMO_TAG_E3DETECTOR, attrs.get<std::string>(GNE_ATTR_PARENT, "", ok));
        }
        // check that all parameters are valid
        if (lane == nullptr) {
            WRITE_WARNING("The lane '" + laneId + "' to use within the " + toString(SUMO_TAG_DET_ENTRY) + " is not known.");
        } else if (!checkAndFixDetectorPosition(position, lane->getLaneShapeLength(), friendlyPos)) {
            WRITE_WARNING("Invalid position for " + toString(SUMO_TAG_DET_ENTRY) + ".");
        } else if (E3Parent) {
            // save ID of last created element
            GNEAdditional* additionalCreated = buildDetectorEntry(net, allowUndoRedo, E3Parent, lane, position, friendlyPos, blockMovement);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildDetectorExit(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of Exit
    std::string laneId = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_DET_EXIT, SUMO_ATTR_LANE, abort);
    double position = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, "", SUMO_TAG_DET_EXIT, SUMO_ATTR_POSITION, abort);
    bool friendlyPos = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, "", SUMO_TAG_DET_EXIT, SUMO_ATTR_FRIENDLY_POS, abort);
    // parse Netedit attributes
    bool blockMovement = false;
    if (attrs.hasAttribute(GNE_ATTR_BLOCK_MOVEMENT)) {
        blockMovement = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, "", SUMO_TAG_DET_EXIT, GNE_ATTR_BLOCK_MOVEMENT, abort);
    }
    // Check if parsing of parameters was correct
    if (!abort) {
        // get lane and E3 parent
        GNELane* lane = net->retrieveLane(laneId, false, true);
        GNEAdditional* E3Parent = nullptr;
        // obtain parent depending if we're loading or creating it using GNEAdditionalFrame
        if (insertedAdditionals) {
            E3Parent = insertedAdditionals->getAdditionalParent(net, SUMO_TAG_E3DETECTOR);
        } else {
            bool ok = true;
            E3Parent = net->retrieveAdditional(SUMO_TAG_E3DETECTOR, attrs.get<std::string>(GNE_ATTR_PARENT, "", ok));
        }
        // check that all parameters are valid
        if (lane == nullptr) {
            WRITE_WARNING("The lane '" + laneId + "' to use within the " + toString(SUMO_TAG_DET_EXIT) + " is not known.");
        } else if (!checkAndFixDetectorPosition(position, lane->getLaneShapeLength(), friendlyPos)) {
            WRITE_WARNING("Invalid position for " + toString(SUMO_TAG_DET_EXIT) + ".");
        } else if (E3Parent) {
            // save ID of last created element
            GNEAdditional* additionalCreated = buildDetectorExit(net, allowUndoRedo, E3Parent, lane, position, friendlyPos, blockMovement);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}


bool
GNEAdditionalHandler::parseAndBuildDetectorE1Instant(GNENet* net, bool allowUndoRedo, const SUMOSAXAttributes& attrs, LastInsertedElement* insertedAdditionals) {
    bool abort = false;
    // parse attributes of E1Instant
    std::string id = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_INSTANT_INDUCTION_LOOP, SUMO_ATTR_ID, abort);
    std::string laneId = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_INSTANT_INDUCTION_LOOP, SUMO_ATTR_LANE, abort);
    double position = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, id, SUMO_TAG_INSTANT_INDUCTION_LOOP, SUMO_ATTR_POSITION, abort);
    std::string file = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_INSTANT_INDUCTION_LOOP, SUMO_ATTR_FILE, abort);
    std::string vehicleTypes = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_INSTANT_INDUCTION_LOOP, SUMO_ATTR_VTYPES, abort);
    std::string name = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, id, SUMO_TAG_INSTANT_INDUCTION_LOOP, SUMO_ATTR_NAME, abort);
    bool friendlyPos = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, SUMO_TAG_INSTANT_INDUCTION_LOOP, SUMO_ATTR_FRIENDLY_POS, abort);
    // parse Netedit attributes
    bool blockMovement = false;
    if (attrs.hasAttribute(GNE_ATTR_BLOCK_MOVEMENT)) {
        blockMovement = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, id, SUMO_TAG_INSTANT_INDUCTION_LOOP, GNE_ATTR_BLOCK_MOVEMENT, abort);
    }
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // get pointer to lane
        GNELane* lane = net->retrieveLane(laneId, false, true);
        // check that all elements are valid
        if (net->retrieveAdditional(SUMO_TAG_INSTANT_INDUCTION_LOOP, id, false) != nullptr) {
            WRITE_WARNING("There is another " + toString(SUMO_TAG_INSTANT_INDUCTION_LOOP) + " with the same ID='" + id + "'.");
        } else if (lane == nullptr) {
            // Write error if lane isn't valid
            WRITE_WARNING("The lane '" + laneId + "' to use within the " + toString(SUMO_TAG_INSTANT_INDUCTION_LOOP) + " '" + id + "' is not known.");
        } else if (!checkAndFixDetectorPosition(position, lane->getLaneShapeLength(), friendlyPos)) {
            WRITE_WARNING("Invalid position for " + toString(SUMO_TAG_INSTANT_INDUCTION_LOOP) + " with ID = '" + id + "'.");
        } else {
            // save ID of last created element
            GNEAdditional* additionalCreated = buildDetectorE1Instant(net, allowUndoRedo, id, lane, position, file, vehicleTypes, name, friendlyPos, blockMovement);
            // check if insertion has to be commited
            if (insertedAdditionals) {
                insertedAdditionals->commitAdditionalInsertion(additionalCreated);
            }
            return true;
        }
    }
    return false;
}

// ===========================================================================
// private method definitions
// ===========================================================================

void
GNEAdditionalHandler::parseAndBuildPoly(const SUMOSAXAttributes& attrs) {
    bool abort = false;
    // parse attributes of polygons
    std::string polygonID = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_POLY, SUMO_ATTR_ID, abort);
    PositionVector shape = GNEAttributeCarrier::parseAttributeFromXML<PositionVector>(attrs, polygonID, SUMO_TAG_POLY, SUMO_ATTR_SHAPE, abort);
    double layer = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, polygonID, SUMO_TAG_POLY, SUMO_ATTR_LAYER, abort);
    bool fill = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, "", SUMO_TAG_POLY, SUMO_ATTR_FILL, abort);
    double lineWidth = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, polygonID, SUMO_TAG_POLY, SUMO_ATTR_LINEWIDTH, abort);
    std::string type = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, polygonID, SUMO_TAG_POLY, SUMO_ATTR_TYPE, abort);
    RGBColor color = GNEAttributeCarrier::parseAttributeFromXML<RGBColor>(attrs, polygonID, SUMO_TAG_POLY, SUMO_ATTR_COLOR, abort);
    double angle = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, polygonID, SUMO_TAG_POLY, SUMO_ATTR_ANGLE, abort);
    std::string imgFile = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, polygonID, SUMO_TAG_POLY, SUMO_ATTR_IMGFILE, abort);
    bool relativePath = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, polygonID, SUMO_TAG_POLY, SUMO_ATTR_RELATIVEPATH, abort);
    // check if ID is valid
    if (SUMOXMLDefinitions::isValidTypeID(polygonID) == false) {
        WRITE_WARNING("Invalid characters for polygon ID");
        abort = true;
    }
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // check if shape must be loaded as geo attribute
        bool geo = false;
        const GeoConvHelper* gch = myGeoConvHelper != nullptr ? myGeoConvHelper : &GeoConvHelper::getFinal();
        if (attrs.getOpt<bool>(SUMO_ATTR_GEO, polygonID.c_str(), abort, false)) {
            geo = true;
            bool success = true;
            for (int i = 0; i < (int)shape.size(); i++) {
                success &= gch->x2cartesian_const(shape[i]);
            }
            if (!success) {
                WRITE_WARNING("Unable to project coordinates for polygon '" + polygonID + "'.");
                return;
            }
        }
        // check if img file is absolute
        if (imgFile != "" && !FileHelpers::isAbsolute(imgFile)) {
            imgFile = FileHelpers::getConfigurationRelative(getFileName(), imgFile);
        }
        // create polygon, or show an error if polygon already exists
        if (!myNet->getAttributeCarriers()->addPolygon(polygonID, type, color, layer, angle, imgFile, relativePath, shape, geo, fill, lineWidth, false)) {
            WRITE_WARNING("Polygon with ID '" + polygonID + "' already exists.");
        } else {
            // commit shape element insertion
            myLastInsertedElement->commitShapeInsertion(myNet->getAttributeCarriers()->getShapes().at(SUMO_TAG_POLY).at(polygonID));
        }
    }
}


void
GNEAdditionalHandler::parseAndBuildPOI(const SUMOSAXAttributes& attrs) {
    bool abort = false;
    const SumoXMLTag POITag = attrs.hasAttribute(SUMO_ATTR_LANE) ? SUMO_TAG_POILANE : SUMO_TAG_POI;
    // parse attributes of POIs
    std::string POIID = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, "", SUMO_TAG_POI, SUMO_ATTR_ID, abort);
    // POIs can be defined using a X,Y position,...
    Position pos = attrs.hasAttribute(SUMO_ATTR_X) ? GNEAttributeCarrier::parseAttributeFromXML<Position>(attrs, POIID, SUMO_TAG_POI, SUMO_ATTR_POSITION, abort) : Position::INVALID;
    // ... a Lon-Lat,...
    double lon = attrs.hasAttribute(SUMO_ATTR_LON) ? GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, POIID, SUMO_TAG_POI, SUMO_ATTR_LON, abort) : GNEAttributeCarrier::INVALID_POSITION;
    double lat = attrs.hasAttribute(SUMO_ATTR_LAT) ? GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, POIID, SUMO_TAG_POI, SUMO_ATTR_LAT, abort) : GNEAttributeCarrier::INVALID_POSITION;
    // .. or as Lane-PosLane
    std::string laneID = attrs.hasAttribute(SUMO_ATTR_LANE) ? GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, POIID, SUMO_TAG_POILANE, SUMO_ATTR_LANE, abort) : "";
    double lanePos = attrs.hasAttribute(SUMO_ATTR_POSITION) ? GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, POIID, SUMO_TAG_POILANE, SUMO_ATTR_POSITION, abort) : GNEAttributeCarrier::INVALID_POSITION;
    double lanePosLat = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, POIID, SUMO_TAG_POILANE, SUMO_ATTR_POSITION_LAT, abort);
    // continue with common parameters
    double layer = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, POIID, SUMO_TAG_POI, SUMO_ATTR_LAYER, abort);
    std::string type = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, POIID, SUMO_TAG_POI, SUMO_ATTR_TYPE, abort);
    RGBColor color = GNEAttributeCarrier::parseAttributeFromXML<RGBColor>(attrs, POIID, SUMO_TAG_POI, SUMO_ATTR_COLOR, abort);
    double angle = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, POIID, SUMO_TAG_POI, SUMO_ATTR_ANGLE, abort);
    std::string imgFile = GNEAttributeCarrier::parseAttributeFromXML<std::string>(attrs, POIID, SUMO_TAG_POI, SUMO_ATTR_IMGFILE, abort);
    bool relativePath = GNEAttributeCarrier::parseAttributeFromXML<bool>(attrs, POIID, SUMO_TAG_POI, SUMO_ATTR_RELATIVEPATH, abort);
    double width = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, POIID, SUMO_TAG_POI, SUMO_ATTR_WIDTH, abort);
    double height = GNEAttributeCarrier::parseAttributeFromXML<double>(attrs, POIID, SUMO_TAG_POI, SUMO_ATTR_HEIGHT, abort);
    // check if ID is valid
    if (SUMOXMLDefinitions::isValidTypeID(POIID) == false) {
        WRITE_WARNING("Invalid characters for POI ID");
        abort = true;
    }
    // Continue if all parameters were sucesfully loaded
    if (!abort) {
        // check if img file is absolute
        if (imgFile != "" && !FileHelpers::isAbsolute(imgFile)) {
            imgFile = FileHelpers::getConfigurationRelative(getFileName(), imgFile);
        }
        // check if lane exist
        if (laneID != "" && !myNet->retrieveLane(laneID, false)) {
            WRITE_WARNING("The lane '" + laneID + "' to use within the PoI '" + POIID + "' is not known.");
            return;
        }
        // check position
        bool useGeo = false;
        // if position is invalid, then is either a POILane or a GEOPoi
        if (pos == Position::INVALID) {
            // try computing x,y from lane,pos
            if (laneID != "") {
                // if LaneID is defined, then is a POILane
                pos = getLanePos(POIID, laneID, lanePos, lanePosLat);
            } else {
                // try computing x,y from lon,lat
                if (lat == GNEAttributeCarrier::INVALID_POSITION || lon == GNEAttributeCarrier::INVALID_POSITION) {
                    WRITE_WARNING("Either (x, y), (lon, lat) or (lane, pos) must be specified for PoI '" + POIID + "'.");
                    return;
                } else if (!GeoConvHelper::getFinal().usingGeoProjection()) {
                    WRITE_WARNING("(lon, lat) is specified for PoI '" + POIID + "' but no geo-conversion is specified for the network.");
                    return;
                }
                // set GEO Position
                pos.set(lon, lat);
                useGeo = true;
                if (!GeoConvHelper::getFinal().x2cartesian_const(pos)) {
                    WRITE_WARNING("Unable to project coordinates for PoI '" + POIID + "'.");
                    return;
                }
            }
        }
        // create POI, or show an error if POI already exists
        if (!myNet->getAttributeCarriers()->addPOI(POIID, type, color, pos, useGeo, laneID, lanePos, lanePosLat, layer, angle, imgFile, relativePath, width, height, false)) {
            WRITE_WARNING("POI with ID '" + POIID + "' already exists.");
        } else {
            // commit shape element insertion
            myLastInsertedElement->commitShapeInsertion(myNet->getAttributeCarriers()->getShapes().at(POITag).at(POIID));
        }
    }
}


void
GNEAdditionalHandler::parseParameter(const SUMOSAXAttributes& attrs) {
    // we have two cases: if we're parsing a Shape or we're parsing an Additional
    if (getLastParameterised()) {
        bool ok = true;
        std::string key;
        if (attrs.hasAttribute(SUMO_ATTR_KEY)) {
            // obtain key
            key = attrs.get<std::string>(SUMO_ATTR_KEY, nullptr, ok);
            if (key.empty()) {
                WRITE_WARNING("Error parsing key from shape parameter. Key cannot be empty");
                ok = false;
            }
            if (!SUMOXMLDefinitions::isValidParameterKey(key)) {
                WRITE_WARNING("Error parsing key from shape parameter. Key contains invalid characters");
                ok = false;
            }
        } else {
            WRITE_WARNING("Error parsing key from shape parameter. Key doesn't exist");
            ok = false;
        }
        // circumventing empty string test
        const std::string val = attrs.hasAttribute(SUMO_ATTR_VALUE) ? attrs.getString(SUMO_ATTR_VALUE) : "";
        // set parameter in last inserted additional
        if (ok) {
            WRITE_DEBUG("Inserting parameter '" + key + "|" + val + "' into shape.");
            getLastParameterised()->setParameter(key, val);
        }
    } else if (myLastInsertedElement->getLastInsertedAdditional()) {
        // first check if given additional supports parameters
        if (myLastInsertedElement->getLastInsertedAdditional()->getTagProperty().hasParameters()) {
            bool ok = true;
            std::string key;
            if (attrs.hasAttribute(SUMO_ATTR_KEY)) {
                // obtain key
                key = attrs.get<std::string>(SUMO_ATTR_KEY, nullptr, ok);
                if (key.empty()) {
                    WRITE_WARNING("Error parsing key from additional parameter. Key cannot be empty");
                    ok = false;
                }
                if (!SUMOXMLDefinitions::isValidParameterKey(key)) {
                    WRITE_WARNING("Error parsing key from additional parameter. Key contains invalid characters");
                    ok = false;
                }
            } else {
                WRITE_WARNING("Error parsing key from additional parameter. Key doesn't exist");
                ok = false;
            }
            // circumventing empty string test
            const std::string val = attrs.hasAttribute(SUMO_ATTR_VALUE) ? attrs.getString(SUMO_ATTR_VALUE) : "";
            // check double values
            if (myLastInsertedElement->getLastInsertedAdditional()->getTagProperty().hasDoubleParameters() && !GNEAttributeCarrier::canParse<double>(val)) {
                WRITE_WARNING("Error parsing value from additional float parameter. Value cannot be parsed to float");
                ok = false;
            }
            // set parameter in last inserted additional
            if (ok) {
                WRITE_DEBUG("Inserting parameter '" + key + "|" + val + "' into additional " + myLastInsertedElement->getLastInsertedAdditional()->getTagStr() + ".");
                myLastInsertedElement->getLastInsertedAdditional()->setParameter(key, val);
            }
        } else {
            WRITE_WARNING("Additionals of type '" + myLastInsertedElement->getLastInsertedAdditional()->getTagStr() + "' doesn't support parameters");
        }
    } else if (myLastInsertedElement->getLastInsertedShape()) {
        // first check if given shape supports parameters
        if (myLastInsertedElement->getLastInsertedShape()->getTagProperty().hasParameters()) {
            bool ok = true;
            std::string key;
            if (attrs.hasAttribute(SUMO_ATTR_KEY)) {
                // obtain key
                key = attrs.get<std::string>(SUMO_ATTR_KEY, nullptr, ok);
                if (key.empty()) {
                    WRITE_WARNING("Error parsing key from shape parameter. Key cannot be empty");
                    ok = false;
                }
                if (!SUMOXMLDefinitions::isValidParameterKey(key)) {
                    WRITE_WARNING("Error parsing key from shape parameter. Key contains invalid characters");
                    ok = false;
                }
            } else {
                WRITE_WARNING("Error parsing key from shape parameter. Key doesn't exist");
                ok = false;
            }
            // circumventing empty string test
            const std::string val = attrs.hasAttribute(SUMO_ATTR_VALUE) ? attrs.getString(SUMO_ATTR_VALUE) : "";
            // check double values
            if (myLastInsertedElement->getLastInsertedShape()->getTagProperty().hasDoubleParameters() && !GNEAttributeCarrier::canParse<double>(val)) {
                WRITE_WARNING("Error parsing value from shape float parameter. Value cannot be parsed to float");
                ok = false;
            }
            // set parameter in last inserted shape
            if (ok) {
                WRITE_DEBUG("Inserting parameter '" + key + "|" + val + "' into shape " + myLastInsertedElement->getLastInsertedShape()->getTagStr() + ".");
                myLastInsertedElement->getLastInsertedShape()->setParameter(key, val);
            }
        } else {
            WRITE_WARNING("Shape of type '" + myLastInsertedElement->getLastInsertedShape()->getTagStr() + "' doesn't support parameters");
        }
    } else if (myLastInsertedElement->getLastInsertedTAZElement()) {
        // first check if given TAZ supports parameters
        if (myLastInsertedElement->getLastInsertedTAZElement()->getTagProperty().hasParameters()) {
            bool ok = true;
            std::string key;
            if (attrs.hasAttribute(SUMO_ATTR_KEY)) {
                // obtain key
                key = attrs.get<std::string>(SUMO_ATTR_KEY, nullptr, ok);
                if (key.empty()) {
                    WRITE_WARNING("Error parsing key from TAZ parameter. Key cannot be empty");
                    ok = false;
                }
                if (!SUMOXMLDefinitions::isValidParameterKey(key)) {
                    WRITE_WARNING("Error parsing key from TAZ parameter. Key contains invalid characters");
                    ok = false;
                }
            } else {
                WRITE_WARNING("Error parsing key from TAZ parameter. Key doesn't exist");
                ok = false;
            }
            // circumventing empty string test
            const std::string val = attrs.hasAttribute(SUMO_ATTR_VALUE) ? attrs.getString(SUMO_ATTR_VALUE) : "";
            // check double values
            if (myLastInsertedElement->getLastInsertedTAZElement()->getTagProperty().hasDoubleParameters() && !GNEAttributeCarrier::canParse<double>(val)) {
                WRITE_WARNING("Error parsing value from TAZ float parameter. Value cannot be parsed to float");
                ok = false;
            }
            // set parameter in last inserted TAZ
            if (ok) {
                WRITE_DEBUG("Inserting parameter '" + key + "|" + val + "' into TAZ " + myLastInsertedElement->getLastInsertedTAZElement()->getTagStr() + ".");
                myLastInsertedElement->getLastInsertedTAZElement()->setParameter(key, val);
            }
        } else {
            WRITE_WARNING("TAZ of type '" + myLastInsertedElement->getLastInsertedTAZElement()->getTagStr() + "' doesn't support parameters");
        }
    } else {
        WRITE_WARNING("Parameters has to be declared within the definition of an additional, shape or TAZ element");
    }
}

// ===========================================================================
// GNEAdditionalHandler::LastInsertedElement method definitions
// ===========================================================================

void
GNEAdditionalHandler::LastInsertedElement::insertElement(SumoXMLTag tag) {
    myInsertedElements.push_back(StackElement(tag));
}


void
GNEAdditionalHandler::LastInsertedElement::commitAdditionalInsertion(GNEAdditional* additional) {
    myInsertedElements.back().additional = additional;
}


void
GNEAdditionalHandler::LastInsertedElement::commitShapeInsertion(GNEShape* shapeCreated) {
    myInsertedElements.back().shape = shapeCreated;
}


void
GNEAdditionalHandler::LastInsertedElement::commitTAZElementInsertion(GNETAZElement* TAZElementCreated) {
    myInsertedElements.back().TAZElement = TAZElementCreated;
}


void
GNEAdditionalHandler::LastInsertedElement::popElement() {
    if (!myInsertedElements.empty()) {
        myInsertedElements.pop_back();
    }
}


GNEAdditional*
GNEAdditionalHandler::LastInsertedElement::getAdditionalParent(GNENet* net, SumoXMLTag expectedTag) const {
    if (myInsertedElements.size() < 2) {
        // currently we're finding parent additional in the additional XML root
        WRITE_WARNING("A " + toString(myInsertedElements.back().tag) + " must be declared within the definition of a " + toString(expectedTag) + ".");
        return nullptr;
    } else {
        if (myInsertedElements.size() < 2) {
            // additional was hierarchically bad loaded, then return nullptr
            return nullptr;
        } else if ((myInsertedElements.end() - 2)->additional == nullptr) {
            WRITE_WARNING(toString(expectedTag) + " parent of " + toString((myInsertedElements.end() - 1)->tag) + " was not loaded sucesfully.");
            // parent additional wasn't sucesfully loaded, then return nullptr
            return nullptr;
        }
        GNEAdditional* retrievedAdditional = nullptr;
        // special case for rerouters
        if ((myInsertedElements.size() == 3) && (myInsertedElements.at(0).tag == SUMO_TAG_REROUTER) && (myInsertedElements.at(1).tag == SUMO_TAG_INTERVAL)) {
            retrievedAdditional = myInsertedElements.at(1).additional;
        } else {
            retrievedAdditional = net->retrieveAdditional((myInsertedElements.end() - 2)->tag, (myInsertedElements.end() - 2)->additional->getID(), false);
        }
        if (retrievedAdditional == nullptr) {
            // additional doesn't exist
            WRITE_WARNING("A " + toString((myInsertedElements.end() - 1)->tag) + " must be declared within the definition of a " + toString(expectedTag) + ".");
            return nullptr;
        } else if (retrievedAdditional->getTagProperty().getTag() != expectedTag) {
            // invalid parent additional
            WRITE_WARNING("A " + toString((myInsertedElements.end() - 1)->tag) + " cannot be declared within the definition of a " + retrievedAdditional->getTagStr() + ".");
            return nullptr;
        } else {
            return retrievedAdditional;
        }
    }
}


GNEShape*
GNEAdditionalHandler::LastInsertedElement::getShapeParent(GNENet* net, SumoXMLTag expectedTag) const {
    if (myInsertedElements.size() < 2) {
        // currently we're finding parent shape in the shape XML root
        WRITE_WARNING("A " + toString(myInsertedElements.back().tag) + " must be declared within the definition of a " + toString(expectedTag) + ".");
        return nullptr;
    } else {
        if (myInsertedElements.size() < 2) {
            // shape was hierarchically bad loaded, then return nullptr
            return nullptr;
        } else if ((myInsertedElements.end() - 2)->shape == nullptr) {
            WRITE_WARNING(toString(expectedTag) + " parent of " + toString((myInsertedElements.end() - 1)->tag) + " was not loaded sucesfully.");
            // parent shape wasn't sucesfully loaded, then return nullptr
            return nullptr;
        }
        GNEShape* retrievedShape = net->retrieveShape((myInsertedElements.end() - 2)->tag, (myInsertedElements.end() - 2)->shape->getID(), false);
        if (retrievedShape == nullptr) {
            // shape doesn't exist
            WRITE_WARNING("A " + toString((myInsertedElements.end() - 1)->tag) + " must be declared within the definition of a " + toString(expectedTag) + ".");
            return nullptr;
        } else if (retrievedShape->getTagProperty().getTag() != expectedTag) {
            // invalid parent shape
            WRITE_WARNING("A " + toString((myInsertedElements.end() - 1)->tag) + " cannot be declared within the definition of a " + retrievedShape->getTagStr() + ".");
            return nullptr;
        } else {
            return retrievedShape;
        }
    }
}


GNETAZElement*
GNEAdditionalHandler::LastInsertedElement::getTAZElementParent(GNENet* net, SumoXMLTag expectedTag) const {
    if (myInsertedElements.size() < 2) {
        // currently we're finding parent TAZElement in the TAZElement XML root
        WRITE_WARNING("A " + toString(myInsertedElements.back().tag) + " must be declared within the definition of a " + toString(expectedTag) + ".");
        return nullptr;
    } else {
        if (myInsertedElements.size() < 2) {
            // TAZElement was hierarchically bad loaded, then return nullptr
            return nullptr;
        } else if ((myInsertedElements.end() - 2)->TAZElement == nullptr) {
            WRITE_WARNING(toString(expectedTag) + " parent of " + toString((myInsertedElements.end() - 1)->tag) + " was not loaded sucesfully.");
            // parent TAZElement wasn't sucesfully loaded, then return nullptr
            return nullptr;
        }
        GNETAZElement* retrievedTAZElement = net->retrieveTAZElement((myInsertedElements.end() - 2)->tag, (myInsertedElements.end() - 2)->TAZElement->getID(), false);
        if (retrievedTAZElement == nullptr) {
            // TAZElement doesn't exist
            WRITE_WARNING("A " + toString((myInsertedElements.end() - 1)->tag) + " must be declared within the definition of a " + toString(expectedTag) + ".");
            return nullptr;
        } else if (retrievedTAZElement->getTagProperty().getTag() != expectedTag) {
            // invalid parent TAZElement
            WRITE_WARNING("A " + toString((myInsertedElements.end() - 1)->tag) + " cannot be declared within the definition of a " + retrievedTAZElement->getTagStr() + ".");
            return nullptr;
        } else {
            return retrievedTAZElement;
        }
    }
}


GNEAdditional*
GNEAdditionalHandler::LastInsertedElement::getLastInsertedAdditional() const {
    // ierate in reverse mode over myInsertedElements to obtain last inserted additional
    for (std::vector<StackElement>::const_reverse_iterator i = myInsertedElements.rbegin(); i != myInsertedElements.rend(); i++) {
        // we need to avoid Tag Param because isn't an additional
        if (i->tag != SUMO_TAG_PARAM) {
            return i->additional;
        }
    }
    return nullptr;
}


GNEShape*
GNEAdditionalHandler::LastInsertedElement::getLastInsertedShape() const {
    // ierate in reverse mode over myInsertedElements to obtain last inserted shape
    for (std::vector<StackElement>::const_reverse_iterator i = myInsertedElements.rbegin(); i != myInsertedElements.rend(); i++) {
        // we need to avoid Tag Param because isn't a shape
        if (i->tag != SUMO_TAG_PARAM) {
            return i->shape;
        }
    }
    return nullptr;
}


GNETAZElement*
GNEAdditionalHandler::LastInsertedElement::getLastInsertedTAZElement() const {
    // ierate in reverse mode over myInsertedElements to obtain last inserted TAZElement
    for (std::vector<StackElement>::const_reverse_iterator i = myInsertedElements.rbegin(); i != myInsertedElements.rend(); i++) {
        // we need to avoid Tag Param because isn't a TAZElement
        if (i->tag != SUMO_TAG_PARAM) {
            return i->TAZElement;
        }
    }
    return nullptr;
}


GNEAdditionalHandler::LastInsertedElement::StackElement::StackElement(SumoXMLTag _tag) :
    tag(_tag),
    additional(nullptr),
    shape(nullptr),
    TAZElement(nullptr) {
}

/****************************************************************************/
