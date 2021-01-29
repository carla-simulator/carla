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
/// @file    GUILane.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// Representation of a lane in the micro simulation (gui-version)
/****************************************************************************/
#include <config.h>

#include <string>
#include <utility>
#include <fx.h>
#include <utils/geom/GeomHelper.h>
#include <utils/geom/Position.h>
#include <microsim/logging/FunctionBinding.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StdDefs.h>
#include <utils/geom/GeomHelper.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/windows/GUISUMOAbstractView.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <microsim/MSGlobals.h>
#include <microsim/MSLane.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/MSInsertionControl.h>
#include <microsim/MSVehicleTransfer.h>
#include <microsim/MSNet.h>
#include <microsim/MSEdgeWeightsStorage.h>
#include <microsim/devices/MSDevice_Routing.h>
#include <mesosim/MELoop.h>
#include <mesosim/MESegment.h>
#include "GUILane.h"
#include "GUIEdge.h"
#include "GUIVehicle.h"
#include "GUINet.h"

#ifdef HAVE_OSG
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4275) // do not warn about the DLL interface for OSG
#endif
#include <osg/Geometry>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif

//#define GUILane_DEBUG_DRAW_WALKING_AREA_VERTICES
//#define GUILane_DEBUG_DRAW_VERTICES
//#define GUILane_DEBUG_DRAW_FOE_INTERSECTIONS

// ===========================================================================
// static member declaration
// ===========================================================================
const RGBColor GUILane::MESO_USE_LANE_COLOR(0, 0, 0, 0);


// ===========================================================================
// method definitions
// ===========================================================================
GUILane::GUILane(const std::string& id, double maxSpeed, double length,
                 MSEdge* const edge, int numericalID,
                 const PositionVector& shape, double width,
                 SVCPermissions permissions, int index, bool isRampAccel,
                 const std::string& type) :
    MSLane(id, maxSpeed, length, edge, numericalID, shape, width, permissions, index, isRampAccel, type),
    GUIGlObject(GLO_LANE, id),
#ifdef HAVE_OSG
    myGeom(0),
#endif
    myAmClosed(false),
    myLock(true) {
    if (MSGlobals::gUseMesoSim) {
        myShape = splitAtSegments(shape);
        assert(fabs(myShape.length() - shape.length()) < POSITION_EPS);
        assert(myShapeSegments.size() == myShape.size());
    }
    myShapeRotations.reserve(myShape.size() - 1);
    myShapeLengths.reserve(myShape.size() - 1);
    myShapeColors.reserve(myShape.size() - 1);
    int e = (int) myShape.size() - 1;
    for (int i = 0; i < e; ++i) {
        const Position& f = myShape[i];
        const Position& s = myShape[i + 1];
        myShapeLengths.push_back(f.distanceTo2D(s));
        myShapeRotations.push_back(RAD2DEG(atan2(s.x() - f.x(), f.y() - s.y())));
    }
    //
    myHalfLaneWidth = myWidth / 2.;
    myQuarterLaneWidth = myWidth / 4.;
}


GUILane::~GUILane() {
    // just to quit cleanly on a failure
    if (myLock.locked()) {
        myLock.unlock();
    }
}


// ------ Vehicle insertion ------
void
GUILane::incorporateVehicle(MSVehicle* veh, double pos, double speed, double posLat,
                            const MSLane::VehCont::iterator& at,
                            MSMoveReminder::Notification notification) {
    FXMutexLock locker(myLock);
    MSLane::incorporateVehicle(veh, pos, speed, posLat, at, notification);
}


// ------ Access to vehicles ------
const MSLane::VehCont&
GUILane::getVehiclesSecure() const {
    myLock.lock();
    return myVehicles;
}


void
GUILane::releaseVehicles() const {
    myLock.unlock();
}


void
GUILane::planMovements(const SUMOTime t) {
    FXMutexLock locker(myLock);
    MSLane::planMovements(t);
}

void
GUILane::setJunctionApproaches(const SUMOTime t) const {
    FXMutexLock locker(myLock);
    MSLane::setJunctionApproaches(t);
}


void
GUILane::executeMovements(const SUMOTime t) {
    FXMutexLock locker(myLock);
    MSLane::executeMovements(t);
}


MSVehicle*
GUILane::removeVehicle(MSVehicle* remVehicle, MSMoveReminder::Notification notification, bool notify) {
    FXMutexLock locker(myLock);
    return MSLane::removeVehicle(remVehicle, notification, notify);
}


void
GUILane::removeParking(MSVehicle* remVehicle) {
    FXMutexLock locker(myLock);
    return MSLane::removeParking(remVehicle);
}


void
GUILane::swapAfterLaneChange(SUMOTime t) {
    FXMutexLock locker(myLock);
    MSLane::swapAfterLaneChange(t);
}


void
GUILane::integrateNewVehicles() {
    FXMutexLock locker(myLock);
    MSLane::integrateNewVehicles();
}


void
GUILane::detectCollisions(SUMOTime timestep, const std::string& stage) {
    FXMutexLock locker(myLock);
    MSLane::detectCollisions(timestep, stage);
}


double
GUILane::setPartialOccupation(MSVehicle* v) {
    FXMutexLock locker(myLock);
    return MSLane::setPartialOccupation(v);
}


void
GUILane::resetPartialOccupation(MSVehicle* v) {
    FXMutexLock locker(myLock);
    MSLane::resetPartialOccupation(v);
}


// ------ Drawing methods ------
void
GUILane::drawLinkNo(const GUIVisualizationSettings& s) const {
    int noLinks = (int)myLinks.size();
    if (noLinks == 0) {
        return;
    }
    // draw all links
    if (getEdge().isCrossing()) {
        // draw indices at the start and end of the crossing
        MSLink* link = MSLinkContHelper::getConnectingLink(*getLogicalPredecessorLane(), *this);
        PositionVector shape = getShape();
        shape.extrapolate(0.5); // draw on top of the walking area
        GLHelper::drawTextAtEnd(toString(link->getIndex()), shape, 0, s.drawLinkJunctionIndex, s.scale);
        GLHelper::drawTextAtEnd(toString(link->getIndex()), shape.reverse(), 0, s.drawLinkJunctionIndex, s.scale);
        return;
    }
    // draw all links
    double w = myWidth / (double) noLinks;
    double x1 = myHalfLaneWidth;
    for (int i = noLinks; --i >= 0;) {
        double x2 = x1 - (double)(w / 2.);
        GLHelper::drawTextAtEnd(toString(myLinks[MSGlobals::gLefthand ? noLinks - 1 - i : i]->getIndex()), getShape(), x2, s.drawLinkJunctionIndex, s.scale);
        x1 -= w;
    }
}


void
GUILane::drawTLSLinkNo(const GUIVisualizationSettings& s, const GUINet& net) const {
    int noLinks = (int)myLinks.size();
    if (noLinks == 0) {
        return;
    }
    if (getEdge().isCrossing()) {
        // draw indices at the start and end of the crossing
        MSLink* link = MSLinkContHelper::getConnectingLink(*getLogicalPredecessorLane(), *this);
        int linkNo = net.getLinkTLIndex(link);
        // maybe the reverse link is controlled separately
        int linkNo2 = net.getLinkTLIndex(myLinks.front());
        // otherwise, use the same index as the forward link
        if (linkNo2 < 0) {
            linkNo2 = linkNo;
        }
        if (linkNo >= 0) {
            PositionVector shape = getShape();
            shape.extrapolate(0.5); // draw on top of the walking area
            GLHelper::drawTextAtEnd(toString(linkNo2), shape, 0, s.drawLinkTLIndex, s.scale);
            GLHelper::drawTextAtEnd(toString(linkNo), shape.reverse(), 0, s.drawLinkTLIndex, s.scale);
        }
        return;
    }
    // draw all links
    double w = myWidth / (double) noLinks;
    double x1 = myHalfLaneWidth;
    for (int i = noLinks; --i >= 0;) {
        double x2 = x1 - (double)(w / 2.);
        int linkNo = net.getLinkTLIndex(myLinks[MSGlobals::gLefthand ? noLinks - 1 - i : i]);
        if (linkNo < 0) {
            continue;
        }
        GLHelper::drawTextAtEnd(toString(linkNo), getShape(), x2, s.drawLinkTLIndex, s.scale);
        x1 -= w;
    }
}


void
GUILane::drawLinkRules(const GUIVisualizationSettings& s, const GUINet& net) const {
    int noLinks = (int)myLinks.size();
    if (noLinks == 0) {
        drawLinkRule(s, net, nullptr, getShape(), 0, 0);
        return;
    }
    if (getEdge().isCrossing()) {
        // draw rules at the start and end of the crossing
        MSLink* link = MSLinkContHelper::getConnectingLink(*getLogicalPredecessorLane(), *this);
        MSLink* link2 = myLinks.front();
        if (link2->getTLLogic() == nullptr) {
            link2 = link;
        }
        PositionVector shape = getShape();
        shape.extrapolate(0.5); // draw on top of the walking area
        drawLinkRule(s, net, link2, shape, 0, myWidth);
        drawLinkRule(s, net, link, shape.reverse(), 0, myWidth);
        return;
    }
    // draw all links
    const double w = myWidth / (double) noLinks;
    double x1 = myEdge->getToJunction()->getType() == SumoXMLNodeType::RAIL_SIGNAL ? -myWidth * 0.5 : 0;
    for (int i = 0; i < noLinks; ++i) {
        double x2 = x1 + w;
        drawLinkRule(s, net, myLinks[MSGlobals::gLefthand ? noLinks - 1 - i : i], getShape(), x1, x2);
        x1 = x2;
    }
    // draw stopOffset for passenger cars
    if (myStopOffsets.size() != 0 && (myStopOffsets.begin()->first & SVC_PASSENGER) != 0) {
        const double stopOffsetPassenger = myStopOffsets.begin()->second;
        const Position& end = myShape.back();
        const Position& f = myShape[-2];
        const double rot = RAD2DEG(atan2((end.x() - f.x()), (f.y() - end.y())));
        GLHelper::setColor(s.getLinkColor(LINKSTATE_MAJOR));
        glPushMatrix();
        glTranslated(end.x(), end.y(), 0);
        glRotated(rot, 0, 0, 1);
        glTranslated(0, stopOffsetPassenger, 0);
        glBegin(GL_QUADS);
        glVertex2d(-myHalfLaneWidth, 0.0);
        glVertex2d(-myHalfLaneWidth, 0.2);
        glVertex2d(myHalfLaneWidth, 0.2);
        glVertex2d(myHalfLaneWidth, 0.0);
        glEnd();
        glPopMatrix();
    }
}


void
GUILane::drawLinkRule(const GUIVisualizationSettings& s, const GUINet& net, MSLink* link, const PositionVector& shape, double x1, double x2) const {
    const Position& end = shape.back();
    const Position& f = shape[-2];
    const double rot = RAD2DEG(atan2((end.x() - f.x()), (f.y() - end.y())));
    if (link == nullptr) {
        if (static_cast<GUIEdge*>(myEdge)->showDeadEnd()) {
            GLHelper::setColor(GUIVisualizationColorSettings::SUMO_color_DEADEND_SHOW);
        } else {
            GLHelper::setColor(GUIVisualizationSettings::getLinkColor(LINKSTATE_DEADEND));
        }
        glPushMatrix();
        glTranslated(end.x(), end.y(), 0);
        glRotated(rot, 0, 0, 1);
        glBegin(GL_QUADS);
        glVertex2d(-myHalfLaneWidth, 0.0);
        glVertex2d(-myHalfLaneWidth, 0.5);
        glVertex2d(myHalfLaneWidth, 0.5);
        glVertex2d(myHalfLaneWidth, 0.0);
        glEnd();
        glPopMatrix();
    } else {
        glPushMatrix();
        glTranslated(end.x(), end.y(), 0);
        glRotated(rot, 0, 0, 1);
        // select glID
        switch (link->getState()) {
            case LINKSTATE_TL_GREEN_MAJOR:
            case LINKSTATE_TL_GREEN_MINOR:
            case LINKSTATE_TL_RED:
            case LINKSTATE_TL_REDYELLOW:
            case LINKSTATE_TL_YELLOW_MAJOR:
            case LINKSTATE_TL_YELLOW_MINOR:
            case LINKSTATE_TL_OFF_BLINKING:
            case LINKSTATE_TL_OFF_NOSIGNAL:
                glPushName(net.getLinkTLID(link));
                break;
            case LINKSTATE_MAJOR:
            case LINKSTATE_MINOR:
            case LINKSTATE_EQUAL:
            default:
                glPushName(getGlID());
                break;
        }
        GLHelper::setColor(GUIVisualizationSettings::getLinkColor(link->getState()));
        if (!(drawAsRailway(s) || drawAsWaterway(s)) || link->getState() != LINKSTATE_MAJOR) {
            // the white bar should be the default for most railway
            // links and looks ugly so we do not draw it
            double scale = isInternal() ? 0.5 : 1;
            if (myEdge->getToJunction()->getType() == SumoXMLNodeType::RAIL_SIGNAL) {
                scale *= MAX2(s.laneWidthExaggeration, s.junctionSize.getExaggeration(s, this, 10));
            }
            glScaled(scale, scale, 1);
            glBegin(GL_QUADS);
            glVertex2d(x1 - myHalfLaneWidth, 0.0);
            glVertex2d(x1 - myHalfLaneWidth, 0.5);
            glVertex2d(x2 - myHalfLaneWidth, 0.5);
            glVertex2d(x2 - myHalfLaneWidth, 0.0);
            glEnd();
        }
        glPopName();
        glPopMatrix();
    }
}

void
GUILane::drawArrows() const {
    if (myLinks.size() == 0) {
        return;
    }
    // draw all links
    const Position& end = getShape().back();
    const Position& f = getShape()[-2];
    const double rot = RAD2DEG(atan2((end.x() - f.x()), (f.y() - end.y())));
    glPushMatrix();
    glColor3d(1, 1, 1);
    glTranslated(end.x(), end.y(), 0);
    glRotated(rot, 0, 0, 1);
    if (myWidth < SUMO_const_laneWidth) {
        glScaled(myWidth / SUMO_const_laneWidth, 1, 1);
    }
    for (std::vector<MSLink*>::const_iterator i = myLinks.begin(); i != myLinks.end(); ++i) {
        LinkDirection dir = (*i)->getDirection();
        LinkState state = (*i)->getState();
        if (state == LINKSTATE_DEADEND || dir == LinkDirection::NODIR) {
            continue;
        }
        switch (dir) {
            case LinkDirection::STRAIGHT:
                GLHelper::drawBoxLine(Position(0, 4), 0, 2, .05);
                GLHelper::drawTriangleAtEnd(Position(0, 4), Position(0, 1), (double) 1, (double) .25);
                break;
            case LinkDirection::TURN:
                GLHelper::drawBoxLine(Position(0, 4), 0, 1.5, .05);
                GLHelper::drawBoxLine(Position(0, 2.5), 90, .5, .05);
                GLHelper::drawBoxLine(Position(0.5, 2.5), 180, 1, .05);
                GLHelper::drawTriangleAtEnd(Position(0.5, 2.5), Position(0.5, 4), (double) 1, (double) .25);
                break;
            case LinkDirection::TURN_LEFTHAND:
                GLHelper::drawBoxLine(Position(0, 4), 0, 1.5, .05);
                GLHelper::drawBoxLine(Position(0, 2.5), -90, 1, .05);
                GLHelper::drawBoxLine(Position(-0.5, 2.5), -180, 1, .05);
                GLHelper::drawTriangleAtEnd(Position(-0.5, 2.5), Position(-0.5, 4), (double) 1, (double) .25);
                break;
            case LinkDirection::LEFT:
                GLHelper::drawBoxLine(Position(0, 4), 0, 1.5, .05);
                GLHelper::drawBoxLine(Position(0, 2.5), 90, 1, .05);
                GLHelper::drawTriangleAtEnd(Position(0, 2.5), Position(1.5, 2.5), (double) 1, (double) .25);
                break;
            case LinkDirection::RIGHT:
                GLHelper::drawBoxLine(Position(0, 4), 0, 1.5, .05);
                GLHelper::drawBoxLine(Position(0, 2.5), -90, 1, .05);
                GLHelper::drawTriangleAtEnd(Position(0, 2.5), Position(-1.5, 2.5), (double) 1, (double) .25);
                break;
            case LinkDirection::PARTLEFT:
                GLHelper::drawBoxLine(Position(0, 4), 0, 1.5, .05);
                GLHelper::drawBoxLine(Position(0, 2.5), 45, .7, .05);
                GLHelper::drawTriangleAtEnd(Position(0, 2.5), Position(1.2, 1.3), (double) 1, (double) .25);
                break;
            case LinkDirection::PARTRIGHT:
                GLHelper::drawBoxLine(Position(0, 4), 0, 1.5, .05);
                GLHelper::drawBoxLine(Position(0, 2.5), -45, .7, .05);
                GLHelper::drawTriangleAtEnd(Position(0, 2.5), Position(-1.2, 1.3), (double) 1, (double) .25);
                break;
            default:
                break;
        }
    }
    glPopMatrix();
}


void
GUILane::drawLane2LaneConnections(double exaggeration) const {
    Position centroid;
    if (exaggeration > 1) {
        centroid = myEdge->getToJunction()->getShape().getCentroid();
    }
    for (std::vector<MSLink*>::const_iterator i = myLinks.begin(); i != myLinks.end(); ++i) {
        const MSLane* connected = (*i)->getLane();
        if (connected == nullptr) {
            continue;
        }
        GLHelper::setColor(GUIVisualizationSettings::getLinkColor((*i)->getState()));
        glBegin(GL_LINES);
        Position p1 = myEdge->isWalkingArea() ? getShape().getCentroid() : getShape()[-1];
        Position p2 = connected->getEdge().isWalkingArea() ? connected->getShape().getCentroid() : connected->getShape()[0];
        if (exaggeration > 1) {
            p1 = centroid + ((p1 - centroid) * exaggeration);
            p2 = centroid + ((p2 - centroid) * exaggeration);
        }
        glVertex2d(p1.x(), p1.y());
        glVertex2d(p2.x(), p2.y());
        glEnd();
        GLHelper::drawTriangleAtEnd(p1, p2, (double) .4, (double) .2);
    }
}


void
GUILane::drawGL(const GUIVisualizationSettings& s) const {
    glPushMatrix();
    glPushName(getGlID());
    const bool isCrossing = myEdge->isCrossing();
    const bool isWalkingArea = myEdge->isWalkingArea();
    const bool isInternal = isCrossing || isWalkingArea || myEdge->isInternal();
    bool mustDrawMarkings = false;
    double exaggeration = s.laneWidthExaggeration;
    if (MSGlobals::gUseMesoSim) {
        GUIEdge* myGUIEdge = dynamic_cast<GUIEdge*>(myEdge);
        exaggeration *= s.edgeScaler.getScheme().getColor(myGUIEdge->getScaleValue(s.edgeScaler.getActive()));
    } else {
        exaggeration *= s.laneScaler.getScheme().getColor(getScaleValue(s.laneScaler.getActive()));
    }
    const bool hasRailSignal = myEdge->getToJunction()->getType() == SumoXMLNodeType::RAIL_SIGNAL;
    const bool detailZoom = s.scale * exaggeration > 5;
    const bool drawDetails = (detailZoom || s.junctionSize.minSize == 0 || hasRailSignal) && !s.drawForRectangleSelection;
    const bool drawRails = drawAsRailway(s);
    if (isCrossing || isWalkingArea) {
        // draw internal lanes on top of junctions
        glTranslated(0, 0, GLO_JUNCTION + 0.1);
    } else if (isWaterway(myPermissions)) {
        // draw waterways below normal roads
        glTranslated(0, 0, getType() - 0.2);
    } else {
        glTranslated(0, 0, getType());
    }
    // set lane color
    const RGBColor color = setColor(s);
    if (MSGlobals::gUseMesoSim) {
        myShapeColors.clear();
        const std::vector<RGBColor>& segmentColors = static_cast<const GUIEdge*>(myEdge)->getSegmentColors();
        if (segmentColors.size() > 0) {
            // apply segment specific shape colors
            //std::cout << getID() << " shape=" << myShape << " shapeSegs=" << toString(myShapeSegments) << "\n";
            for (int ii = 0; ii < (int)myShape.size() - 1; ++ii) {
                myShapeColors.push_back(segmentColors[myShapeSegments[ii]]);
            }
        }
    }
    // recognize full transparency and simply don't draw
    bool hiddenBidi = myEdge->getBidiEdge() != nullptr && myEdge->getNumericalID() > myEdge->getBidiEdge()->getNumericalID();
    if (color.alpha() != 0 && s.scale * exaggeration > s.laneMinSize) {
        // scale tls-controlled lane2lane-arrows along with their junction shapes
        double junctionExaggeration = 1;
        if (!isInternal
                && myEdge->getToJunction()->getType() <= SumoXMLNodeType::RAIL_CROSSING
                && (s.junctionSize.constantSize || s.junctionSize.exaggeration > 1)) {
            junctionExaggeration = MAX2(1.001, s.junctionSize.getExaggeration(s, this, 4));
        }
        // draw lane
        // check whether it is not too small
        if (s.scale * exaggeration < 1. && junctionExaggeration == 1 && s.junctionSize.minSize != 0) {
            if (!isInternal || hasRailSignal) {
                if (myShapeColors.size() > 0) {
                    GLHelper::drawLine(myShape, myShapeColors);
                } else {
                    GLHelper::drawLine(myShape);
                }
            }
            glPopMatrix();
        } else {
            GUINet* net = (GUINet*) MSNet::getInstance();
            const bool spreadSuperposed = s.spreadSuperposed && myEdge->getBidiEdge() != nullptr && drawRails;
            if (hiddenBidi && !spreadSuperposed) {
                // do not draw shape
            } else if (drawRails) {
                // draw as railway: assume standard gauge of 1435mm when lane width is not set
                // draw foot width 150mm, assume that distance between rail feet inner sides is reduced on both sides by 39mm with regard to the gauge
                // assume crosstie length of 181% gauge (2600mm for standard gauge)
                PositionVector shape = myShape;
                const double width = myWidth;
                double halfGauge = 0.5 * (width == SUMO_const_laneWidth ?  1.4350 : width) * exaggeration;
                if (spreadSuperposed) {
                    try {
                        shape.move2side(halfGauge * 0.8);
                    } catch (InvalidArgument&) {}
                    halfGauge *= 0.4;
                }
                const double halfInnerFeetWidth = halfGauge - 0.039 * exaggeration;
                const double halfRailWidth = detailZoom ? (halfInnerFeetWidth + 0.15 * exaggeration) : SUMO_const_halfLaneWidth;
                const double halfCrossTieWidth = halfGauge * 1.81;
                if (myShapeColors.size() > 0) {
                    GLHelper::drawBoxLines(shape, myShapeRotations, myShapeLengths, myShapeColors, halfRailWidth);
                } else {
                    GLHelper::drawBoxLines(shape, myShapeRotations, myShapeLengths, halfRailWidth);
                }
                // Draw white on top with reduced width (the area between the two tracks)
                if (detailZoom) {
                    glColor3d(1, 1, 1);
                    glTranslated(0, 0, .1);
                    GLHelper::drawBoxLines(shape, myShapeRotations, myShapeLengths, halfInnerFeetWidth);
                    setColor(s);
                    GLHelper::drawCrossTies(shape, myShapeRotations, myShapeLengths, 0.26 * exaggeration, 0.6 * exaggeration, halfCrossTieWidth, s.drawForRectangleSelection);
                }
            } else if (isCrossing) {
                if (s.drawCrossingsAndWalkingareas && (s.scale > 3.0 || s.junctionSize.minSize == 0)) {
                    glTranslated(0, 0, .2);
                    GLHelper::drawCrossTies(myShape, myShapeRotations, myShapeLengths, 0.5, 1.0, getWidth() * 0.5, s.drawForRectangleSelection);
                    glTranslated(0, 0, -.2);
                }
            } else if (isWalkingArea) {
                if (s.drawCrossingsAndWalkingareas && (s.scale > 3.0 || s.junctionSize.minSize == 0)) {
                    glTranslated(0, 0, .2);
                    if (s.scale * exaggeration < 20.) {
                        GLHelper::drawFilledPoly(myShape, true);
                    } else {
                        GLHelper::drawFilledPolyTesselated(myShape, true);
                    }
                    glTranslated(0, 0, -.2);
#ifdef GUILane_DEBUG_DRAW_WALKING_AREA_VERTICES
                    GLHelper::debugVertices(myShape, 80 / s.scale);
#endif
                }
            } else {
                // we draw the lanes with reduced width so that the lane markings below are visible
                // (this avoids artifacts at geometry corners without having to
                // compute lane-marking intersection points)
                const double halfWidth = isInternal ? myQuarterLaneWidth : (myHalfLaneWidth - SUMO_const_laneMarkWidth / 2);
                mustDrawMarkings = !isInternal && myPermissions != 0 && myPermissions != SVC_PEDESTRIAN && exaggeration == 1.0 && !isWaterway(myPermissions);
                const int cornerDetail = drawDetails && !isInternal ? (int)(s.scale * exaggeration) : 0;
                const double offset = halfWidth * MAX2(0., (exaggeration - 1)) * (MSGlobals::gLefthand ? -1 : 1);
                if (myShapeColors.size() > 0) {
                    GLHelper::drawBoxLines(myShape, myShapeRotations, myShapeLengths, myShapeColors, halfWidth * exaggeration, cornerDetail, offset);
                } else {
                    GLHelper::drawBoxLines(myShape, myShapeRotations, myShapeLengths, halfWidth * exaggeration, cornerDetail, offset);
                }
            }
#ifdef GUILane_DEBUG_DRAW_VERTICES
            GLHelper::debugVertices(myShape, 80 / s.scale);
#endif
#ifdef GUILane_DEBUG_DRAW_FOE_INTERSECTIONS
            if (myEdge->isInternal() && gSelected.isSelected(getType(), getGlID())) {
                debugDrawFoeIntersections();
            }
#endif
            glPopMatrix();
            // draw details
            if ((!isInternal || isCrossing || !s.drawJunctionShape) && (drawDetails || s.drawForRectangleSelection || junctionExaggeration > 1)) {
                glPushMatrix();
                glTranslated(0, 0, GLO_JUNCTION); // must draw on top of junction shape
                glTranslated(0, 0, .5);
                if (drawDetails) {
                    if (s.showLaneDirection) {
                        if (drawRails) {
                            // improve visibility of superposed rail edges
                            GLHelper::setColor(setColor(s).changedBrightness(100));
                        } else {
                            glColor3d(0.3, 0.3, 0.3);
                        }
                        if (!isCrossing || s.drawCrossingsAndWalkingareas) {
                            drawDirectionIndicators(exaggeration, spreadSuperposed);
                        }
                    }
                    if (!isInternal || isCrossing
                            // controlled internal junction
                            || (getLinkCont().size() != 0 && getLinkCont()[0]->isInternalJunctionLink() && getLinkCont()[0]->getTLLogic() != nullptr)) {
                        if (MSGlobals::gLateralResolution > 0 && s.showSublanes && !hiddenBidi && (myPermissions & ~(SVC_PEDESTRIAN | SVC_RAIL_CLASSES)) != 0) {
                            // draw sublane-borders
                            const double offsetSign = MSGlobals::gLefthand ? -1 : 1;
                            GLHelper::setColor(color.changedBrightness(51));
                            for (double offset = -myHalfLaneWidth; offset < myHalfLaneWidth; offset += MSGlobals::gLateralResolution) {
                                GLHelper::drawBoxLines(myShape, myShapeRotations, myShapeLengths, 0.01, 0, -offset * offsetSign);
                            }
                        }
                        if (s.showLinkDecals && !drawRails && !drawAsWaterway(s) && myPermissions != SVC_PEDESTRIAN) {
                            drawArrows();
                        }
                        glTranslated(0, 0, 1000);
                        if (s.drawLinkJunctionIndex.show) {
                            drawLinkNo(s);
                        }
                        if (s.drawLinkTLIndex.show) {
                            drawTLSLinkNo(s, *net);
                        }
                        glTranslated(0, 0, -1000);
                    }
                    glTranslated(0, 0, .1);
                }
                // make sure link rules are drawn so tls can be selected via right-click
                if (s.showLinkRules && (drawDetails || s.drawForRectangleSelection)
                        && !isWalkingArea
                        && (!myEdge->isInternal() || getLinkCont()[0]->isInternalJunctionLink())) {
                    drawLinkRules(s, *net);
                }
                if ((drawDetails || junctionExaggeration > 1) && s.showLane2Lane) {
                    //  draw from end of first to the begin of second but respect junction scaling
                    drawLane2LaneConnections(junctionExaggeration);
                }
                glPopMatrix();
            }
        }
        if (mustDrawMarkings && drawDetails && s.laneShowBorders && !hiddenBidi) { // needs matrix reset
            drawMarkings(s, exaggeration);
        }
        if (drawDetails && isInternal && s.showBikeMarkings && myPermissions == SVC_BICYCLE && exaggeration == 1.0 && s.showLinkDecals && s.laneShowBorders && !hiddenBidi) {
            drawBikeMarkings();
        }
    } else {
        glPopMatrix();
    }
    // draw vehicles
    if (s.scale * s.vehicleSize.getExaggeration(s, nullptr) > s.vehicleSize.minSize) {
        // retrieve vehicles from lane; disallow simulation
        const MSLane::VehCont& vehicles = getVehiclesSecure();
        for (MSLane::VehCont::const_iterator v = vehicles.begin(); v != vehicles.end(); ++v) {
            if ((*v)->getLane() == this) {
                static_cast<const GUIVehicle* const>(*v)->drawGL(s);
            } // else: this is the shadow during a continuous lane change
        }
        // draw parking vehicles
        for (std::set<const MSVehicle*>::const_iterator v = myParkingVehicles.begin(); v != myParkingVehicles.end(); ++v) {
            static_cast<const GUIVehicle* const>(*v)->drawGL(s);
        }
        // allow lane simulation
        releaseVehicles();
    }
    glPopName();
}


void
GUILane::drawMarkings(const GUIVisualizationSettings& s, double scale) const {
    glPushMatrix();
    glTranslated(0, 0, GLO_EDGE);
    setColor(s);
    // optionally draw inverse markings
    if (myIndex > 0 && (myEdge->getLanes()[myIndex - 1]->getPermissions() & myPermissions) != 0) {
        double mw = (myHalfLaneWidth + SUMO_const_laneMarkWidth) * scale;
        double mw2 = (myHalfLaneWidth - SUMO_const_laneMarkWidth) * scale;
        if (MSGlobals::gLefthand) {
            mw *= -1;
            mw2 *= -1;
        }
        int e = (int) getShape().size() - 1;
        for (int i = 0; i < e; ++i) {
            glPushMatrix();
            glTranslated(getShape()[i].x(), getShape()[i].y(), 2.1);
            glRotated(myShapeRotations[i], 0, 0, 1);
            for (double t = 0; t < myShapeLengths[i]; t += 6) {
                const double length = MIN2((double)3, myShapeLengths[i] - t);
                glBegin(GL_QUADS);
                glVertex2d(-mw, -t);
                glVertex2d(-mw, -t - length);
                glVertex2d(-mw2, -t - length);
                glVertex2d(-mw2, -t);
                glEnd();
            }
            glPopMatrix();
        }
    }
    // draw white boundings and white markings
    glColor3d(1, 1, 1);
    GLHelper::drawBoxLines(
        getShape(),
        getShapeRotations(),
        getShapeLengths(),
        (myHalfLaneWidth + SUMO_const_laneMarkWidth) * scale);
    glPopMatrix();
}


void
GUILane::drawBikeMarkings() const {
    // draw bike lane markings onto the intersection
    glColor3d(1, 1, 1);
    const int e = (int) getShape().size() - 1;
    const double markWidth = 0.1;
    const double mw = myHalfLaneWidth;
    for (int i = 0; i < e; ++i) {
        glPushMatrix();
        glTranslated(getShape()[i].x(), getShape()[i].y(), GLO_JUNCTION + 0.4);
        glRotated(myShapeRotations[i], 0, 0, 1);
        for (double t = 0; t < myShapeLengths[i]; t += 0.5) {
            // left and right marking
            for (int side = -1; side <= 1; side += 2) {
                glBegin(GL_QUADS);
                glVertex2d(side * mw, -t);
                glVertex2d(side * mw, -t - 0.35);
                glVertex2d(side * (mw + markWidth), -t - 0.35);
                glVertex2d(side * (mw + markWidth), -t);
                glEnd();
            }
        }
        glPopMatrix();
    }
}

void
GUILane::drawDirectionIndicators(double exaggeration, bool spreadSuperposed) const {
    glPushMatrix();
    glTranslated(0, 0, GLO_EDGE);
    int e = (int) getShape().size() - 1;
    const double widthFactor = spreadSuperposed ? 0.4 : 1;
    const double w = MAX2(POSITION_EPS, myWidth * widthFactor);
    const double w2 = MAX2(POSITION_EPS, myHalfLaneWidth * widthFactor);
    const double w4 = MAX2(POSITION_EPS, myQuarterLaneWidth * widthFactor);
    const double sideOffset = spreadSuperposed ? w * -0.5 : 0;
    for (int i = 0; i < e; ++i) {
        glPushMatrix();
        glTranslated(getShape()[i].x(), getShape()[i].y(), 0.1);
        glRotated(myShapeRotations[i], 0, 0, 1);
        for (double t = 0; t < myShapeLengths[i]; t += w) {
            const double length = MIN2(w2, myShapeLengths[i] - t) * exaggeration;
            glBegin(GL_TRIANGLES);
            glVertex2d(sideOffset, -t - length);
            glVertex2d(sideOffset - w4 * exaggeration, -t);
            glVertex2d(sideOffset + w4 * exaggeration, -t);
            glEnd();
        }
        glPopMatrix();
    }
    glPopMatrix();
}


void
GUILane::debugDrawFoeIntersections() const {
    glPushMatrix();
    glColor3d(1.0, 0.3, 0.3);
    const double orthoLength = 0.5;
    const MSLink* link = getLinkCont().front();
    const std::vector<const MSLane*>& foeLanes = link->getFoeLanes();
    const std::vector<std::pair<double, double> >& lengthsBehind = link->getLengthsBehindCrossing();
    if (foeLanes.size() == lengthsBehind.size()) {
        for (int i = 0; i < (int)foeLanes.size(); ++i) {
            const MSLane* l = foeLanes[i];
            Position pos = l->geometryPositionAtOffset(l->getLength() - lengthsBehind[i].second);
            PositionVector ortho = l->getShape().getOrthogonal(pos, 10, true, orthoLength);
            if (ortho.length() < orthoLength) {
                ortho.extrapolate(orthoLength - ortho.length(), false, true);
            }
            GLHelper::drawLine(ortho);
            //std::cout << "foe=" << l->getID() << " lanePos=" << l->getLength() - lengthsBehind[i].second << " pos=" << pos << "\n";
        }
    }
    glPopMatrix();
}


// ------ inherited from GUIGlObject
GUIGLObjectPopupMenu*
GUILane::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUIGLObjectPopupMenu(app, parent, *this);
    buildPopupHeader(ret, app);
    buildCenterPopupEntry(ret);
    //
    new FXMenuCommand(ret, "Copy edge name to clipboard", nullptr, ret, MID_COPY_EDGE_NAME);
    buildNameCopyPopupEntry(ret);
    buildSelectionPopupEntry(ret);
    //
    buildShowParamsPopupEntry(ret, false);
    const double pos = interpolateGeometryPosToLanePos(myShape.nearest_offset_to_point25D(parent.getPositionInformation()));
    const double height = myShape.positionAtOffset(pos).z();
    new FXMenuCommand(ret, ("pos: " + toString(pos) + " height: " + toString(height)).c_str(), nullptr, nullptr, 0);
    new FXMenuSeparator(ret);
    buildPositionCopyEntry(ret, false);
    new FXMenuSeparator(ret);
    if (myAmClosed) {
        if (myPermissionChanges.empty()) {
            new FXMenuCommand(ret, "Reopen lane", nullptr, &parent, MID_CLOSE_LANE);
            new FXMenuCommand(ret, "Reopen edge", nullptr, &parent, MID_CLOSE_EDGE);
        } else {
            new FXMenuCommand(ret, "Reopen lane (override rerouter)", nullptr, &parent, MID_CLOSE_LANE);
            new FXMenuCommand(ret, "Reopen edge (override rerouter)", nullptr, &parent, MID_CLOSE_EDGE);
        }
    } else {
        new FXMenuCommand(ret, "Close lane", nullptr, &parent, MID_CLOSE_LANE);
        new FXMenuCommand(ret, "Close edge", nullptr, &parent, MID_CLOSE_EDGE);
    }
    new FXMenuCommand(ret, "Add rerouter", nullptr, &parent, MID_ADD_REROUTER);
    new FXMenuSeparator(ret);
    // reachability menu
    FXMenuPane* reachableByClass = new FXMenuPane(ret);
    ret->insertMenuPaneChild(reachableByClass);
    new FXMenuCascade(ret, "Select reachable", GUIIconSubSys::getIcon(GUIIcon::FLAG), reachableByClass);
    for (auto i : SumoVehicleClassStrings.getStrings()) {
        new FXMenuCommand(reachableByClass, i.c_str(), nullptr, &parent, MID_REACHABILITY);
    }
    return ret;
}


GUIParameterTableWindow*
GUILane::getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView&) {
    GUIParameterTableWindow* ret = new GUIParameterTableWindow(app, *this);
    // add items
    ret->mkItem("maxspeed [m/s]", false, getSpeedLimit());
    ret->mkItem("length [m]", false, myLength);
    ret->mkItem("width [m]", false, myWidth);
    ret->mkItem("street name", false, myEdge->getStreetName());
    ret->mkItem("stored traveltime [s]", true, new FunctionBinding<GUILane, double>(this, &GUILane::getStoredEdgeTravelTime));
    ret->mkItem("loaded weight", true, new FunctionBinding<GUILane, double>(this, &GUILane::getLoadedEdgeWeight));
    ret->mkItem("routing speed [m/s]", true, new FunctionBinding<MSEdge, double>(myEdge, &MSEdge::getRoutingSpeed));
    ret->mkItem("brutto occupancy [%]", true, new FunctionBinding<GUILane, double>(this, &GUILane::getBruttoOccupancy, 100.));
    ret->mkItem("netto occupancy [%]", true, new FunctionBinding<GUILane, double>(this, &GUILane::getNettoOccupancy, 100.));
    ret->mkItem("pending insertions [#]", true, new FunctionBinding<GUILane, double>(this, &GUILane::getPendingEmits));
    ret->mkItem("edge type", false, myEdge->getEdgeType());
    ret->mkItem("type", false, myLaneType);
    ret->mkItem("priority", false, myEdge->getPriority());
    ret->mkItem("distance [km]", false, myEdge->getDistance() / 1000);
    ret->mkItem("allowed vehicle class", false, getVehicleClassNames(myPermissions));
    ret->mkItem("disallowed vehicle class", false, getVehicleClassNames(~myPermissions));
    ret->mkItem("permission code", false, myPermissions);
    if (myEdge->getBidiEdge() != nullptr) {
        ret->mkItem("bidi-edge", false, myEdge->getBidiEdge()->getID());
    }
    for (const auto& kv : myEdge->getParametersMap()) {
        ret->mkItem(("edgeParam:" + kv.first).c_str(), false, kv.second);
    }
    ret->closeBuilding();
    return ret;
}


Boundary
GUILane::getCenteringBoundary() const {
    Boundary b;
    b.add(myShape[0]);
    b.add(myShape[-1]);
    b.grow(10);
    // ensure that vehicles and persons on the side are drawn even if the edge
    // is outside the view
    return b;
}


const PositionVector&
GUILane::getShape() const {
    return myShape;
}


const std::vector<double>&
GUILane::getShapeRotations() const {
    return myShapeRotations;
}


const std::vector<double>&
GUILane::getShapeLengths() const {
    return myShapeLengths;
}


double
GUILane::firstWaitingTime() const {
    return myVehicles.size() == 0 ? 0 : myVehicles.back()->getWaitingSeconds();
}


double
GUILane::getEdgeLaneNumber() const {
    return (double) myEdge->getLanes().size();
}


double
GUILane::getStoredEdgeTravelTime() const {
    MSEdgeWeightsStorage& ews = MSNet::getInstance()->getWeightsStorage();
    if (!ews.knowsTravelTime(myEdge)) {
        return -1;
    } else {
        double value(0);
        ews.retrieveExistingTravelTime(myEdge, STEPS2TIME(MSNet::getInstance()->getCurrentTimeStep()), value);
        return value;
    }
}


double
GUILane::getLoadedEdgeWeight() const {
    MSEdgeWeightsStorage& ews = MSNet::getInstance()->getWeightsStorage();
    if (!ews.knowsEffort(myEdge)) {
        return -1;
    } else {
        double value(-1);
        ews.retrieveExistingEffort(myEdge, STEPS2TIME(MSNet::getInstance()->getCurrentTimeStep()), value);
        return value;
    }
}


RGBColor
GUILane::setColor(const GUIVisualizationSettings& s) const {
    // setting and retrieving the color does not work in OSGView so we return it explicitliy
    RGBColor col;
    if (MSGlobals::gUseMesoSim && static_cast<const GUIEdge*>(myEdge)->getMesoColor() != MESO_USE_LANE_COLOR) {
        col = static_cast<const GUIEdge*>(myEdge)->getMesoColor();
    } else {
        const GUIColorer& c = s.laneColorer;
        if (!setFunctionalColor(c, col) && !setMultiColor(s, c, col)) {
            col = c.getScheme().getColor(getColorValue(s, c.getActive()));
        }
    }
    GLHelper::setColor(col);
    return col;
}


bool
GUILane::setFunctionalColor(const GUIColorer& c, RGBColor& col, int activeScheme) const {
    if (activeScheme < 0) {
        activeScheme = c.getActive();
    }
    switch (activeScheme) {
        case 0:
            if (myEdge->isCrossing()) {
                // determine priority to decide color
                MSLink* link = MSLinkContHelper::getConnectingLink(*getLogicalPredecessorLane(), *this);
                if (link->havePriority() || link->getTLLogic() != nullptr) {
                    col = RGBColor(230, 230, 230);
                } else {
                    col = RGBColor(26, 26, 26);
                }
                GLHelper::setColor(col);
                return true;
            } else {
                return false;
            }
        case 18: {
            double hue = GeomHelper::naviDegree(myShape.beginEndAngle()); // [0-360]
            col = RGBColor::fromHSV(hue, 1., 1.);
            GLHelper::setColor(col);
            return true;
        }
        case 30: { // taz color
            col = c.getScheme().getColor(0);
            std::vector<RGBColor> tazColors;
            for (MSEdge* e : myEdge->getPredecessors()) {
                if (e->isTazConnector() && e->knowsParameter("tazColor")) {
                    tazColors.push_back(RGBColor::parseColor(e->getParameter("tazColor")));
                }
            }
            for (MSEdge* e : myEdge->getSuccessors()) {
                if (e->isTazConnector() && e->knowsParameter("tazColor")) {
                    tazColors.push_back(RGBColor::parseColor(e->getParameter("tazColor")));
                }
            }
            if (tazColors.size() > 0) {
                int randColor = RandHelper::rand((int)tazColors.size(), RGBColor::getColorRNG());
                col = tazColors[randColor];
            }
            GLHelper::setColor(col);
            return true;
        }
        default:
            return false;
    }
}


bool
GUILane::setMultiColor(const GUIVisualizationSettings& s, const GUIColorer& c, RGBColor& col) const {
    const int activeScheme = c.getActive();
    myShapeColors.clear();
    switch (activeScheme) {
        case 22: // color by height at segment start
            for (PositionVector::const_iterator ii = myShape.begin(); ii != myShape.end() - 1; ++ii) {
                myShapeColors.push_back(c.getScheme().getColor(ii->z()));
            }
            // osg fallback (edge height at start)
            col = c.getScheme().getColor(getColorValue(s, 21));
            return true;
        case 24: // color by inclination  at segment start
            for (int ii = 1; ii < (int)myShape.size(); ++ii) {
                const double inc = (myShape[ii].z() - myShape[ii - 1].z()) / MAX2(POSITION_EPS, myShape[ii].distanceTo2D(myShape[ii - 1]));
                myShapeColors.push_back(c.getScheme().getColor(inc));
            }
            col = c.getScheme().getColor(getColorValue(s, 23));
            return true;
        default:
            return false;
    }
}


double
GUILane::getColorValue(const GUIVisualizationSettings& s, int activeScheme) const {
    switch (activeScheme) {
        case 0:
            switch (myPermissions) {
                case SVC_PEDESTRIAN:
                    return 1;
                case SVC_BICYCLE:
                    return 2;
                case 0:
                    // forbidden road or green verge
                    return myEdge->getPermissions() == 0 ? 10 : 3;
                case SVC_SHIP:
                    return 4;
                case SVC_AUTHORITY:
                    return 8;
                default:
                    break;
            }
            if (myEdge->isTazConnector()) {
                return 9;
            } else if (isRailway(myPermissions)) {
                return 5;
            } else if ((myPermissions & SVC_PASSENGER) != 0) {
                if ((myPermissions & (SVC_RAIL_CLASSES & ~SVC_RAIL_FAST)) != 0 && (myPermissions & SVC_SHIP) == 0) {
                    return 6;
                } else {
                    return 0;
                }
            } else {
                return 7;
            }
        case 1:
            return isLaneOrEdgeSelected();
        case 2:
            return (double)myPermissions;
        case 3:
            return getSpeedLimit();
        case 4:
            return getBruttoOccupancy();
        case 5:
            return getNettoOccupancy();
        case 6:
            return firstWaitingTime();
        case 7:
            return getEdgeLaneNumber();
        case 8:
            return getCO2Emissions() / myLength;
        case 9:
            return getCOEmissions() / myLength;
        case 10:
            return getPMxEmissions() / myLength;
        case 11:
            return getNOxEmissions() / myLength;
        case 12:
            return getHCEmissions() / myLength;
        case 13:
            return getFuelConsumption() / myLength;
        case 14:
            return getHarmonoise_NoiseEmissions();
        case 15: {
            return getStoredEdgeTravelTime();
        }
        case 16: {
            MSEdgeWeightsStorage& ews = MSNet::getInstance()->getWeightsStorage();
            if (!ews.knowsTravelTime(myEdge)) {
                return -1;
            } else {
                double value(0);
                ews.retrieveExistingTravelTime(myEdge, 0, value);
                return 100 * myLength / value / getSpeedLimit();
            }
        }
        case 17: {
            // geometrical length has no meaning for walkingAreas since it describes the outer boundary
            return myEdge->isWalkingArea() ? 1 :  1 / myLengthGeometryFactor;
        }
        case 19: {
            return getLoadedEdgeWeight();
        }
        case 20: {
            return myEdge->getPriority();
        }
        case 21: {
            // color by z of first shape point
            return getShape()[0].z();
        }
        case 23: {
            // color by incline
            return (getShape()[-1].z() - getShape()[0].z()) / getLength();
        }
        case 25: {
            // color by average speed
            return getMeanSpeed();
        }
        case 26: {
            // color by average relative speed
            return getMeanSpeed() / myMaxSpeed;
        }
        case 27: {
            // color by routing device assumed speed
            return myEdge->getRoutingSpeed();
        }
        case 28:
            return getElectricityConsumption() / myLength;
        case 29:
            return getPendingEmits();
        case 31: {
            // by numerical edge param value
            try {
                return StringUtils::toDouble(myEdge->getParameter(s.edgeParam, "0"));
            } catch (NumberFormatException&) {
                try {
                    return StringUtils::toBool(myEdge->getParameter(s.edgeParam, "0"));
                } catch (BoolFormatException&) {
                    return -1;
                }
            }
        }
        case 32: {
            // by numerical lane param value
            try {
                return StringUtils::toDouble(getParameter(s.laneParam, "0"));
            } catch (NumberFormatException&) {
                try {
                    return StringUtils::toBool(getParameter(s.laneParam, "0"));
                } catch (BoolFormatException&) {
                    return -1;
                }
            }
        }
        case 33: {
            // by edge data value
            return GUINet::getGUIInstance()->getEdgeData(myEdge, s.edgeData);
        }
        case 34: {
            return myEdge->getDistance();
        }
        case 35: {
            return fabs(myEdge->getDistance());
        }
        case 36: {
            return myReachability;
        }
        case 37: {
            return myRNGIndex % MSGlobals::gNumSimThreads;
        }
    }
    return 0;
}


double
GUILane::getScaleValue(int activeScheme) const {
    switch (activeScheme) {
        case 0:
            return 0;
        case 1:
            return isLaneOrEdgeSelected();
        case 2:
            return getSpeedLimit();
        case 3:
            return getBruttoOccupancy();
        case 4:
            return getNettoOccupancy();
        case 5:
            return firstWaitingTime();
        case 6:
            return getEdgeLaneNumber();
        case 7:
            return getCO2Emissions() / myLength;
        case 8:
            return getCOEmissions() / myLength;
        case 9:
            return getPMxEmissions() / myLength;
        case 10:
            return getNOxEmissions() / myLength;
        case 11:
            return getHCEmissions() / myLength;
        case 12:
            return getFuelConsumption() / myLength;
        case 13:
            return getHarmonoise_NoiseEmissions();
        case 14: {
            return getStoredEdgeTravelTime();
        }
        case 15: {
            MSEdgeWeightsStorage& ews = MSNet::getInstance()->getWeightsStorage();
            if (!ews.knowsTravelTime(myEdge)) {
                return -1;
            } else {
                double value(0);
                ews.retrieveExistingTravelTime(myEdge, 0, value);
                return 100 * myLength / value / getSpeedLimit();
            }
        }
        case 16: {
            return 1 / myLengthGeometryFactor;
        }
        case 17: {
            return getLoadedEdgeWeight();
        }
        case 18: {
            return myEdge->getPriority();
        }
        case 19: {
            // scale by average speed
            return getMeanSpeed();
        }
        case 20: {
            // scale by average relative speed
            return getMeanSpeed() / myMaxSpeed;
        }
        case 21:
            return getElectricityConsumption() / myLength;
        case 22:
            return MSNet::getInstance()->getInsertionControl().getPendingEmits(this);
    }
    return 0;
}


bool
GUILane::drawAsRailway(const GUIVisualizationSettings& s) const {
    return isRailway(myPermissions) && s.showRails && (!s.drawForRectangleSelection || s.spreadSuperposed);
}


bool
GUILane::drawAsWaterway(const GUIVisualizationSettings& s) const {
    return isWaterway(myPermissions) && s.showRails && !s.drawForRectangleSelection; // reusing the showRails setting
}


#ifdef HAVE_OSG
void
GUILane::updateColor(const GUIVisualizationSettings& s) {
    if (myGeom == 0) {
        // not drawn
        return;
    }
    const RGBColor col = setColor(s);
    osg::Vec4ubArray* colors = dynamic_cast<osg::Vec4ubArray*>(myGeom->getColorArray());
    (*colors)[0].set(col.red(), col.green(), col.blue(), col.alpha());
    myGeom->setColorArray(colors);
}
#endif


void
GUILane::closeTraffic(bool rebuildAllowed) {
    MSGlobals::gCheckRoutes = false;
    if (myAmClosed) {
        myPermissionChanges.clear(); // reset rerouters
        resetPermissions(CHANGE_PERMISSIONS_GUI);
    } else {
        setPermissions(SVC_AUTHORITY, CHANGE_PERMISSIONS_GUI);
    }
    myAmClosed = !myAmClosed;
    if (rebuildAllowed) {
        getEdge().rebuildAllowedLanes();
        for (MSEdge* const pred : getEdge().getPredecessors()) {
            pred->rebuildAllowedTargets();
        }
    }
}


PositionVector
GUILane::splitAtSegments(const PositionVector& shape) {
    assert(MSGlobals::gUseMesoSim);
    int no = MELoop::numSegmentsFor(myLength, OptionsCont::getOptions().getFloat("meso-edgelength"));
    const double slength = myLength / no;
    PositionVector result = shape;
    double offset = 0;
    for (int i = 0; i < no; ++i) {
        offset += slength;
        Position pos = shape.positionAtOffset(offset);
        int index = result.indexOfClosest(pos);
        if (pos.distanceTo(result[index]) > POSITION_EPS) {
            index = result.insertAtClosest(pos, false);
        }
        while ((int)myShapeSegments.size() < index) {
            myShapeSegments.push_back(i);
        }
        //std::cout << "splitAtSegments " << getID() << " no=" << no << " i=" << i << " offset=" << offset << " index=" << index << " segs=" << toString(myShapeSegments) << " resultSize=" << result.size() << " result=" << toString(result) << "\n";
    }
    while (myShapeSegments.size() < result.size()) {
        myShapeSegments.push_back(no - 1);
    }
    return result;
}

bool
GUILane::isSelected() const {
    return gSelected.isSelected(GLO_LANE, getGlID());
}

bool
GUILane::isLaneOrEdgeSelected() const {
    return isSelected() || gSelected.isSelected(GLO_EDGE, dynamic_cast<GUIEdge*>(myEdge)->getGlID());
}

double
GUILane::getPendingEmits() const {
    return MSNet::getInstance()->getInsertionControl().getPendingEmits(this);
}


/****************************************************************************/
