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
/// @file    GUIEdge.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    Sept 2002
///
// A road/street connecting two junctions (gui-version)
/****************************************************************************/
#include <config.h>

#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <fx.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/windows/GUIMainWindow.h>
#include <utils/gui/windows/GUISUMOAbstractView.h>
#include <utils/geom/GeomHelper.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <microsim/MSBaseVehicle.h>
#include <microsim/MSEdge.h>
#include <microsim/MSJunction.h>
#include <microsim/MSLaneChanger.h>
#include <microsim/MSInsertionControl.h>
#include <microsim/MSGlobals.h>
#include <microsim/logging/CastingFunctionBinding.h>
#include <microsim/logging/FunctionBinding.h>
#include "GUITriggeredRerouter.h"
#include "GUIEdge.h"
#include "GUIVehicle.h"
#include "GUINet.h"
#include "GUILane.h"
#include "GUIPerson.h"
#include "GUIContainer.h"

#include <mesogui/GUIMEVehicleControl.h>
#include <mesogui/GUIMEVehicle.h>
#include <mesosim/MESegment.h>
#include <mesosim/MELoop.h>
#include <mesosim/MEVehicle.h>


GUIEdge::GUIEdge(const std::string& id, int numericalID,
                 const SumoXMLEdgeFunc function,
                 const std::string& streetName, const std::string& edgeType, int priority,
                 double distance)
    : MSEdge(id, numericalID, function, streetName, edgeType, priority, distance),
      GUIGlObject(GLO_EDGE, id) {}


GUIEdge::~GUIEdge() {
    // just to quit cleanly on a failure
    if (myLock.locked()) {
        myLock.unlock();
    }
}

void
GUIEdge::closeBuilding() {
    MSEdge::closeBuilding();
    bool hasNormalSuccessors = false;
    for (const MSEdge* out : getSuccessors()) {
        if (!out->isTazConnector()) {
            hasNormalSuccessors = true;
            break;
        }
    }
    myShowDeadEnd = (!isTazConnector() && !hasNormalSuccessors && getToJunction()->getOutgoing().size() > 0
                     && (getPermissions() & ~SVC_PEDESTRIAN) != 0
                     && (getToJunction()->getOutgoing().size() > 1 ||
                         getToJunction()->getOutgoing().front()->getToJunction() != getFromJunction()));
}

MSLane&
GUIEdge::getLane(int laneNo) {
    assert(laneNo < (int)myLanes->size());
    return *((*myLanes)[laneNo]);
}


std::vector<GUIGlID>
GUIEdge::getIDs(bool includeInternal) {
    std::vector<GUIGlID> ret;
    ret.reserve(MSEdge::myDict.size());
    for (MSEdge::DictType::const_iterator i = MSEdge::myDict.begin(); i != MSEdge::myDict.end(); ++i) {
        const GUIEdge* edge = dynamic_cast<const GUIEdge*>(i->second);
        assert(edge);
        if (includeInternal || edge->isNormal()) {
            ret.push_back(edge->getGlID());
        }
    }
    return ret;
}


double
GUIEdge::getTotalLength(bool includeInternal, bool eachLane) {
    double result = 0;
    for (MSEdge::DictType::const_iterator i = MSEdge::myDict.begin(); i != MSEdge::myDict.end(); ++i) {
        const MSEdge* edge = i->second;
        if (includeInternal || !edge->isInternal()) {
            // @note needs to be change once lanes may have different length
            result += edge->getLength() * (eachLane ? edge->getLanes().size() : 1);
        }
    }
    return result;
}


Boundary
GUIEdge::getBoundary() const {
    Boundary ret;
    if (!isTazConnector()) {
        for (std::vector<MSLane*>::const_iterator i = myLanes->begin(); i != myLanes->end(); ++i) {
            ret.add((*i)->getShape().getBoxBoundary());
        }
    } else {
        // take the starting coordinates of all follower edges and the endpoints
        // of all successor edges
        for (MSEdgeVector::const_iterator it = mySuccessors.begin(); it != mySuccessors.end(); ++it) {
            const std::vector<MSLane*>& lanes = (*it)->getLanes();
            for (std::vector<MSLane*>::const_iterator it_lane = lanes.begin(); it_lane != lanes.end(); ++it_lane) {
                ret.add((*it_lane)->getShape().front());
            }
        }
        for (MSEdgeVector::const_iterator it = myPredecessors.begin(); it != myPredecessors.end(); ++it) {
            const std::vector<MSLane*>& lanes = (*it)->getLanes();
            for (std::vector<MSLane*>::const_iterator it_lane = lanes.begin(); it_lane != lanes.end(); ++it_lane) {
                ret.add((*it_lane)->getShape().back());
            }
        }
    }
    ret.grow(10);
    return ret;
}


GUIGLObjectPopupMenu*
GUIEdge::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUIGLObjectPopupMenu(app, parent, *this);
    buildPopupHeader(ret, app);
    buildCenterPopupEntry(ret);
    buildNameCopyPopupEntry(ret);
    buildSelectionPopupEntry(ret);
    if (MSGlobals::gUseMesoSim) {
        buildShowParamsPopupEntry(ret);
    }
    MESegment* segment = getSegmentAtPosition(parent.getPositionInformation());
    new FXMenuCommand(ret, ("segment: " + toString(segment->getIndex())).c_str(), nullptr, nullptr, 0);
    buildPositionCopyEntry(ret, false);
    return ret;
}


GUIParameterTableWindow*
GUIEdge::getParameterWindow(GUIMainWindow& app,
                            GUISUMOAbstractView& parent) {
    GUIParameterTableWindow* ret = nullptr;
    ret = new GUIParameterTableWindow(app, *this);
    // add edge items
    ret->mkItem("length [m]", false, (*myLanes)[0]->getLength());
    ret->mkItem("allowed speed [m/s]", false, getAllowedSpeed());
    ret->mkItem("brutto occupancy [%]", true, new FunctionBinding<GUIEdge, double>(this, &GUIEdge::getBruttoOccupancy, 100.));
    ret->mkItem("mean vehicle speed [m/s]", true, new FunctionBinding<GUIEdge, double>(this, &GUIEdge::getMeanSpeed));
    ret->mkItem("flow [veh/h/lane]", true, new FunctionBinding<GUIEdge, double>(this, &GUIEdge::getFlow));
    ret->mkItem("routing speed [m/s]", true, new FunctionBinding<MSEdge, double>(this, &MSEdge::getRoutingSpeed));
    ret->mkItem("#vehicles", true, new CastingFunctionBinding<GUIEdge, int, int>(this, &MSEdge::getVehicleNumber));
    // add segment items
    MESegment* segment = getSegmentAtPosition(parent.getPositionInformation());
    ret->mkItem("segment index", false, segment->getIndex());
    ret->mkItem("segment queues", false, segment->numQueues());
    ret->mkItem("segment length [m]", false, segment->getLength());
    ret->mkItem("segment allowed speed [m/s]", false, segment->getEdge().getSpeedLimit());
    ret->mkItem("segment jam threshold [%]", false, segment->getRelativeJamThreshold() * 100);
    ret->mkItem("segment brutto occupancy [%]", true, new FunctionBinding<MESegment, double>(segment, &MESegment::getRelativeOccupancy, 100));
    ret->mkItem("segment mean vehicle speed [m/s]", true, new FunctionBinding<MESegment, double>(segment, &MESegment::getMeanSpeed));
    ret->mkItem("segment flow [veh/h/lane]", true, new FunctionBinding<MESegment, double>(segment, &MESegment::getFlow));
    ret->mkItem("segment #vehicles", true, new CastingFunctionBinding<MESegment, int, int>(segment, &MESegment::getCarNumber));
    ret->mkItem("segment leader leave time", true, new FunctionBinding<MESegment, double>(segment, &MESegment::getEventTimeSeconds));
    ret->mkItem("segment headway [s]", true, new FunctionBinding<MESegment, double>(segment, &MESegment::getLastHeadwaySeconds));
    ret->mkItem("segment entry blocktime [s]", true, new FunctionBinding<MESegment, double>(segment, &MESegment::getEntryBlockTimeSeconds));

    // close building
    ret->closeBuilding();
    return ret;
}


Boundary
GUIEdge::getCenteringBoundary() const {
    Boundary b = getBoundary();
    // ensure that vehicles and persons on the side are drawn even if the edge
    // is outside the view
    b.grow(10);
    return b;
}

const std::string
GUIEdge::getOptionalName() const {
    return myStreetName;
}

void
GUIEdge::drawGL(const GUIVisualizationSettings& s) const {
    if (s.hideConnectors && myFunction == SumoXMLEdgeFunc::CONNECTOR) {
        return;
    }
    glPushName(getGlID());
    // draw the lanes
    if (MSGlobals::gUseMesoSim) {
        setColor(s);
    }
    for (std::vector<MSLane*>::const_iterator i = myLanes->begin(); i != myLanes->end(); ++i) {
        static_cast<GUILane*>(*i)->drawGL(s);
    }
    if (MSGlobals::gUseMesoSim) {
        if (s.scale * s.vehicleSize.getExaggeration(s, nullptr) > s.vehicleSize.minSize) {
            drawMesoVehicles(s);
        }
    }
    glPopName();
    // (optionally) draw the name and/or the street name
    const bool drawEdgeName = s.edgeName.show && myFunction == SumoXMLEdgeFunc::NORMAL;
    const bool drawInternalEdgeName = s.internalEdgeName.show && myFunction == SumoXMLEdgeFunc::INTERNAL;
    const bool drawCwaEdgeName = s.cwaEdgeName.show && (myFunction == SumoXMLEdgeFunc::CROSSING || myFunction == SumoXMLEdgeFunc::WALKINGAREA);
    const bool drawStreetName = s.streetName.show && myStreetName != "";
    const bool drawEdgeValue = s.edgeValue.show && (myFunction == SumoXMLEdgeFunc::NORMAL
                               || (myFunction == SumoXMLEdgeFunc::INTERNAL && !s.drawJunctionShape)
                               || ((myFunction == SumoXMLEdgeFunc::CROSSING || myFunction == SumoXMLEdgeFunc::WALKINGAREA) && s.drawCrossingsAndWalkingareas));
    if (drawEdgeName || drawInternalEdgeName || drawCwaEdgeName || drawStreetName || drawEdgeValue) {
        GUILane* lane1 = dynamic_cast<GUILane*>((*myLanes)[0]);
        GUILane* lane2 = dynamic_cast<GUILane*>((*myLanes).back());
        if (lane1 != nullptr && lane2 != nullptr) {
            const bool spreadSuperposed = s.spreadSuperposed && getBidiEdge() != nullptr && lane2->drawAsRailway(s);
            Position p = lane1->getShape().positionAtOffset(lane1->getShape().length() / (double) 2.);
            p.add(lane2->getShape().positionAtOffset(lane2->getShape().length() / (double) 2.));
            p.mul(.5);
            if (spreadSuperposed) {
                // move name to the right of the edge and towards its beginning
                const double dist = 0.6 * s.edgeName.scaledSize(s.scale);
                const double shiftA = lane1->getShape().rotationAtOffset(lane1->getShape().length() / (double) 2.) - DEG2RAD(135);
                Position shift(dist * cos(shiftA), dist * sin(shiftA));
                p.add(shift);
            }
            double angle = s.getTextAngle(lane1->getShape().rotationDegreeAtOffset(lane1->getShape().length() / (double) 2.) + 90);
            if (drawEdgeName) {
                drawName(p, s.scale, s.edgeName, angle);
            } else if (drawInternalEdgeName) {
                drawName(p, s.scale, s.internalEdgeName, angle);
            } else if (drawCwaEdgeName) {
                drawName(p, s.scale, s.cwaEdgeName, angle);
            }
            if (drawStreetName) {
                GLHelper::drawTextSettings(s.streetName, getStreetName(), p, s.scale, angle);
            }
            if (drawEdgeValue) {
                const int activeScheme = s.getLaneEdgeMode();
                std::string value = "";
                if (activeScheme == 31) {
                    // edge param, could be non-numerical
                    value = getParameter(s.edgeParam, "");
                } else if (activeScheme == 32) {
                    // lane param, could be non-numerical
                    value = lane2->getParameter(s.laneParam, "");
                } else {
                    // use numerical value value of leftmost lane to hopefully avoid sidewalks, bikelanes etc
                    const double doubleValue = (MSGlobals::gUseMesoSim
                                                ? getColorValue(s, activeScheme)
                                                : lane2->getColorValue(s, activeScheme));
                    const RGBColor color = (MSGlobals::gUseMesoSim ? s.edgeColorer : s.laneColorer).getScheme().getColor(doubleValue);
                    if (doubleValue != s.MISSING_DATA
                            && color.alpha() != 0
                            && (!s.edgeValueHideCheck || doubleValue > s.edgeValueHideThreshold)) {
                        value = toString(doubleValue);
                    }
                }
                if (value != "") {
                    GLHelper::drawTextSettings(s.edgeValue, value, p, s.scale, angle);
                }
            }
        }
    }
    if (s.scale * s.personSize.getExaggeration(s, nullptr) > s.personSize.minSize) {
        FXMutexLock locker(myLock);
        for (std::set<MSTransportable*>::const_iterator i = myPersons.begin(); i != myPersons.end(); ++i) {
            GUIPerson* person = dynamic_cast<GUIPerson*>(*i);
            assert(person != 0);
            person->drawGL(s);
        }
    }
    if (s.scale * s.containerSize.getExaggeration(s, nullptr) > s.containerSize.minSize) {
        FXMutexLock locker(myLock);
        for (std::set<MSTransportable*>::const_iterator i = myContainers.begin(); i != myContainers.end(); ++i) {
            GUIContainer* container = dynamic_cast<GUIContainer*>(*i);
            assert(container != 0);
            container->drawGL(s);
        }
    }
}


void
GUIEdge::drawMesoVehicles(const GUIVisualizationSettings& s) const {
    GUIMEVehicleControl* vehicleControl = GUINet::getGUIInstance()->getGUIMEVehicleControl();
    if (vehicleControl != nullptr) {
        // draw the meso vehicles
        vehicleControl->secureVehicles();
        FXMutexLock locker(myLock);
        int laneIndex = 0;
        for (std::vector<MSLane*>::const_iterator msl = myLanes->begin(); msl != myLanes->end(); ++msl, ++laneIndex) {
            GUILane* l = static_cast<GUILane*>(*msl);
            // go through the vehicles
            double segmentOffset = 0; // offset at start of current segment
            for (MESegment* segment = MSGlobals::gMesoNet->getSegmentForEdge(*this);
                    segment != nullptr; segment = segment->getNextSegment()) {
                const double length = segment->getLength();
                if (laneIndex < segment->numQueues()) {
                    // make a copy so we don't have to worry about synchronization
                    std::vector<MEVehicle*> queue = segment->getQueue(laneIndex);
                    const int queueSize = (int)queue.size();
                    double vehiclePosition = segmentOffset + length;
                    // draw vehicles beginning with the leader at the end of the segment
                    double xOff = 0;
                    for (int i = 0; i < queueSize; ++i) {
                        GUIMEVehicle* veh = static_cast<GUIMEVehicle*>(queue[queueSize - i - 1]);
                        const double vehLength = veh->getVehicleType().getLengthWithGap();
                        while (vehiclePosition < segmentOffset) {
                            // if there is only a single queue for a
                            // multi-lane edge shift vehicles and start
                            // drawing again from the end of the segment
                            vehiclePosition += length;
                            xOff += 2;
                        }
                        const Position p = l->geometryPositionAtOffset(vehiclePosition);
                        const double angle = l->getShape().rotationAtOffset(l->interpolateLanePosToGeometryPos(vehiclePosition));
                        veh->drawOnPos(s, p, angle);
                        vehiclePosition -= vehLength;
                    }
                }
                segmentOffset += length;
            }
            glPopMatrix();
        }
        vehicleControl->releaseVehicles();
    }
}



double
GUIEdge::getAllowedSpeed() const {
    return (*myLanes)[0]->getSpeedLimit();
}


double
GUIEdge::getRelativeSpeed() const {
    return getMeanSpeed() / getAllowedSpeed();
}


void
GUIEdge::setColor(const GUIVisualizationSettings& s) const {
    myMesoColor = RGBColor(0, 0, 0); // default background color when using multiColor
    const GUIColorer& c = s.edgeColorer;
    if (!setFunctionalColor(c) && !setMultiColor(c)) {
        myMesoColor = c.getScheme().getColor(getColorValue(s, c.getActive()));
    }
}


bool
GUIEdge::setFunctionalColor(const GUIColorer& c) const {
    const int activeScheme = c.getActive();
    int activeMicroScheme = -1;
    switch (activeScheme) {
        case 0:
            activeMicroScheme = 0; // color uniform
            break;
        case 9:
            activeMicroScheme = 18; // color by angle
            break;
        case 17:
            activeMicroScheme = 30; // color by TAZ
            break;
        default:
            return false;
    }
    GUILane* guiLane = static_cast<GUILane*>(getLanes()[0]);
    return guiLane->setFunctionalColor(c, myMesoColor, activeMicroScheme);
}


bool
GUIEdge::setMultiColor(const GUIColorer& c) const {
    const int activeScheme = c.getActive();
    mySegmentColors.clear();
    switch (activeScheme) {
        case 10: // alternating segments
            for (MESegment* segment = MSGlobals::gMesoNet->getSegmentForEdge(*this);
                    segment != nullptr; segment = segment->getNextSegment()) {
                mySegmentColors.push_back(c.getScheme().getColor(segment->getIndex() % 2));
            }
            //std::cout << getID() << " scheme=" << c.getScheme().getName() << " schemeCols=" << c.getScheme().getColors().size() << " thresh=" << toString(c.getScheme().getThresholds()) << " segmentColors=" << mySegmentColors.size() << " [0]=" << mySegmentColors[0] << " [1]=" << mySegmentColors[1] <<  "\n";
            return true;
        case 11: // by segment jammed state
            for (MESegment* segment = MSGlobals::gMesoNet->getSegmentForEdge(*this);
                    segment != nullptr; segment = segment->getNextSegment()) {
                mySegmentColors.push_back(c.getScheme().getColor(segment->getRelativeOccupancy() <= segment->getRelativeJamThreshold() ? 0 : 1));
            }
            return true;
        case 12: // by segment occupancy
            for (MESegment* segment = MSGlobals::gMesoNet->getSegmentForEdge(*this);
                    segment != nullptr; segment = segment->getNextSegment()) {
                mySegmentColors.push_back(c.getScheme().getColor(segment->getRelativeOccupancy()));
            }
            return true;
        case 13: // by segment speed
            for (MESegment* segment = MSGlobals::gMesoNet->getSegmentForEdge(*this);
                    segment != nullptr; segment = segment->getNextSegment()) {
                mySegmentColors.push_back(c.getScheme().getColor(segment->getMeanSpeed()));
            }
            return true;
        case 14: // by segment flow
            for (MESegment* segment = MSGlobals::gMesoNet->getSegmentForEdge(*this);
                    segment != nullptr; segment = segment->getNextSegment()) {
                mySegmentColors.push_back(c.getScheme().getColor(3600 * segment->getCarNumber() * segment->getMeanSpeed() / segment->getLength()));
            }
            return true;
        case 15: // by segment relative speed
            for (MESegment* segment = MSGlobals::gMesoNet->getSegmentForEdge(*this);
                    segment != nullptr; segment = segment->getNextSegment()) {
                mySegmentColors.push_back(c.getScheme().getColor(segment->getMeanSpeed() / getAllowedSpeed()));
            }
            return true;
        default:
            return false;
    }
}


double
GUIEdge::getColorValue(const GUIVisualizationSettings& /*s*/, int activeScheme) const {
    switch (activeScheme) {
        case 1:
            return gSelected.isSelected(getType(), getGlID());
        case 2:
            return (double)getFunction();
        case 3:
            return getAllowedSpeed();
        case 4:
            return getBruttoOccupancy();
        case 5:
            return getMeanSpeed();
        case 6:
            return getFlow();
        case 7:
            return getRelativeSpeed();
        case 8:
            return getRoutingSpeed();
        case 16:
            return MSNet::getInstance()->getInsertionControl().getPendingEmits(getLanes()[0]);
    }
    return 0;
}


double
GUIEdge::getScaleValue(int activeScheme) const {
    switch (activeScheme) {
        case 1:
            return gSelected.isSelected(getType(), getGlID());
        case 2:
            return getAllowedSpeed();
        case 3:
            return getBruttoOccupancy();
        case 4:
            return getMeanSpeed();
        case 5:
            return getFlow();
        case 6:
            return getRelativeSpeed();
        case 7:
            return MSNet::getInstance()->getInsertionControl().getPendingEmits(getLanes()[0]);
    }
    return 0;
}


MESegment*
GUIEdge::getSegmentAtPosition(const Position& pos) {
    const PositionVector& shape = getLanes()[0]->getShape();
    const double lanePos = shape.nearest_offset_to_point2D(pos);
    return MSGlobals::gMesoNet->getSegmentForEdge(*this, lanePos);
}



void
GUIEdge::closeTraffic(const GUILane* lane) {
    const std::vector<MSLane*>& lanes = getLanes();
    const bool isClosed = lane->isClosed();
    for (std::vector<MSLane*>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
        GUILane* l = dynamic_cast<GUILane*>(*i);
        if (l->isClosed() == isClosed) {
            l->closeTraffic(false);
        }
    }
    rebuildAllowedLanes();
    for (MSEdge* const pred : getPredecessors()) {
        pred->rebuildAllowedTargets();
    }
}


void
GUIEdge::addRerouter() {
    MSEdgeVector edges;
    edges.push_back(this);
    GUITriggeredRerouter* rr = new GUITriggeredRerouter(getID() + "_dynamic_rerouter", edges, 1, "", false, 0, "",
            GUINet::getGUIInstance()->getVisualisationSpeedUp());

    MSTriggeredRerouter::RerouteInterval ri;
    ri.begin = MSNet::getInstance()->getCurrentTimeStep();
    ri.end = SUMOTime_MAX;
    ri.edgeProbs.add(&MSTriggeredRerouter::mySpecialDest_keepDestination, 1.);
    rr->myIntervals.push_back(ri);

    // trigger rerouting for vehicles already on this edge
    const std::vector<MSLane*>& lanes = getLanes();
    for (std::vector<MSLane*>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
        const MSLane::VehCont& vehicles = (*i)->getVehiclesSecure();
        for (MSLane::VehCont::const_iterator v = vehicles.begin(); v != vehicles.end(); ++v) {
            if ((*v)->getLane() == (*i)) {
                rr->notifyEnter(**v, MSMoveReminder::NOTIFICATION_JUNCTION);
            } // else: this is the shadow during a continuous lane change
        }
        (*i)->releaseVehicles();
    }
}


bool
GUIEdge::isSelected() const {
    return gSelected.isSelected(GLO_EDGE, getGlID());
}


/****************************************************************************/
