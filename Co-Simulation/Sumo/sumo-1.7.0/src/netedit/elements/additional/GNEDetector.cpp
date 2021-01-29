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
/// @file    GNEDetector.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Nov 2015
///
//
/****************************************************************************/
#include <config.h>

#include <netedit/elements/network/GNELane.h>
#include <netedit/elements/network/GNEEdge.h>
#include <netedit/elements/additional/GNEAdditionalHandler.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GLIncludes.h>

#include "GNEDetector.h"


// ===========================================================================
// member method definitions
// ===========================================================================

GNEDetector::GNEDetector(const std::string& id, GNENet* net, GUIGlObjectType type, SumoXMLTag tag,
                         double pos, const std::string& freq, const std::string& filename, const std::string& vehicleTypes,
                         const std::string& name, bool friendlyPos, bool blockMovement, const std::vector<GNELane*>& parentLanes) :
    GNEAdditional(id, net, type, tag, name, blockMovement, {}, {}, parentLanes, {}, {}, {}, {}, {}),
              myPositionOverLane(pos),
              myFreq(freq),
              myFilename(filename),
              myVehicleTypes(vehicleTypes),
myFriendlyPosition(friendlyPos) {
}


GNEDetector::GNEDetector(GNEAdditional* additionalParent, GNENet* net, GUIGlObjectType type, SumoXMLTag tag,
                         double pos, const std::string& freq, const std::string& filename, const std::string& name, bool friendlyPos,
                         bool blockMovement, const std::vector<GNELane*>& parentLanes) :
    GNEAdditional(net, type, tag, name, blockMovement, {}, {}, parentLanes, {additionalParent}, {}, {}, {}, {}),
myPositionOverLane(pos),
myFreq(freq),
myFilename(filename),
myFriendlyPosition(friendlyPos) {
}


GNEDetector::~GNEDetector() {}


double
GNEDetector::getPositionOverLane() const {
    return myPositionOverLane;
}


GNELane*
GNEDetector::getLane() const {
    return getParentLanes().front();
}


Position
GNEDetector::getPositionInView() const {
    return getLane()->getLaneShape().positionAtOffset(getGeometryPositionOverLane());
}


Boundary
GNEDetector::getCenteringBoundary() const {
    if (getParentLanes().size() > 1) {
        return myAdditionalSegmentGeometry.getBoxBoundary().grow(10);
    } else if (myAdditionalGeometry.getShape().size() > 0) {
        return myAdditionalGeometry.getShape().getBoxBoundary().grow(10);
    } else {
        Boundary b;
        b.add(myAdditionalGeometry.getPosition());
        b.grow(10);
        return b;
    }
}


void
GNEDetector::splitEdgeGeometry(const double splitPosition, const GNENetworkElement* originalElement,
                               const GNENetworkElement* newElement, GNEUndoList* undoList) {
    // only split geometry of E2 multilane detectors
    if (myTagProperty.getTag() == SUMO_TAG_E2DETECTOR_MULTILANE) {
        // obtain new list of E2 lanes
        std::string newE2Lanes = getNewListOfParents(originalElement, newElement);
        // update E2 Lanes
        if (newE2Lanes.size() > 0) {
            setAttribute(SUMO_ATTR_LANES, newE2Lanes, undoList);
        }
    } else if (splitPosition < myPositionOverLane) {
        // change lane
        setAttribute(SUMO_ATTR_LANE, newElement->getID(), undoList);
        // now adjust start position
        setAttribute(SUMO_ATTR_POSITION, toString(myPositionOverLane - splitPosition), undoList);
    }
}


double
GNEDetector::getGeometryPositionOverLane() const {
    double fixedPos = myPositionOverLane;
    const double len = getLane()->getParentEdge()->getNBEdge()->getFinalLength();
    if (fixedPos < 0) {
        fixedPos += len;
    }
    GNEAdditionalHandler::checkAndFixDetectorPosition(fixedPos, len, true);
    return fixedPos * getLane()->getLengthGeometryFactor();
}


double
GNEDetector::getAttributeDouble(SumoXMLAttr key) const {
    throw InvalidArgument(getTagStr() + " doesn't have a double attribute of type '" + toString(key) + "'");
}


std::string
GNEDetector::getParentName() const {
    return getLane()->getID();
}


std::string
GNEDetector::getPopUpID() const {
    return getTagStr() + ": " + getID();
}


std::string
GNEDetector::getHierarchyName() const {
    return getTagStr();
}


void
GNEDetector::drawE1Shape(const GUIVisualizationSettings& s, const double exaggeration, const double scaledWidth,
                         const RGBColor& mainColor, const RGBColor& secondColor) const {
    // push matrix
    glPushMatrix();
    // set line width
    glLineWidth(1.0);
    // translate to center geometry
    glTranslated(myAdditionalGeometry.getPosition().x(), myAdditionalGeometry.getPosition().y(), 0);
    // rotate
    glRotated(-myBlockIcon.getRotation(), 0, 0, 1);
    // scale
    glScaled(exaggeration, exaggeration, 1);
    // set main color
    GLHelper::setColor(mainColor);
    // begin draw square
    glBegin(GL_QUADS);
    // draw square
    glVertex2d(-1.0,  2);
    glVertex2d(-1.0, -2);
    glVertex2d(1.0, -2);
    glVertex2d(1.0,  2);
    // end draw square
    glEnd();
    // move top
    glTranslated(0, 0, .01);
    // begin draw line
    glBegin(GL_LINES);
    // draw lines
    glVertex2d(0, 2 - .1);
    glVertex2d(0, -2 + .1);
    // end draw line
    glEnd();
    // outline if isn't being drawn for selecting
    if ((scaledWidth * exaggeration > 1) && !s.drawForRectangleSelection) {
        // set main color
        GLHelper::setColor(secondColor);
        // set polygon mode
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        // begin draw square
        glBegin(GL_QUADS);
        // draw square
        glVertex2f(-1.0,  2);
        glVertex2f(-1.0, -2);
        glVertex2f(1.0, -2);
        glVertex2f(1.0,  2);
        // end draw square
        glEnd();
        // rotate 90 degrees
        glRotated(90, 0, 0, -1);
        //set polygon mode
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // begin draw line
        glBegin(GL_LINES);
        // draw line
        glVertex2d(0, 1.7);
        glVertex2d(0, -1.7);
        // end draw line
        glEnd();
    }
    // pop matrix
    glPopMatrix();
}


void
GNEDetector::drawDetectorLogo(const GUIVisualizationSettings& s, const double exaggeration,
                              const std::string& logo, const RGBColor& textColor) const {
    if (!s.drawForRectangleSelection && !s.drawForPositionSelection) {
        // Push matrix
        glPushMatrix();
        // translate to center geometry
        glTranslated(myBlockIcon.getPosition().x(), myBlockIcon.getPosition().y(), 0);
        // rotate
        glRotated((myBlockIcon.getRotation() * -1) + 90, 0, 0, 1);
        // move
        glTranslated(-1, 0, 0);
        // scale text
        glScaled(exaggeration, exaggeration, 1);
        // draw E1 logo
        GLHelper::drawText(logo, Position(), .1, 1.5, textColor);
        // pop matrix
        glPopMatrix();
    }
}


/****************************************************************************/
