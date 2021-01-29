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
/// @file    GNECalibrator.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Nov 2015
///
//
/****************************************************************************/
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/dialogs/GNECalibratorDialog.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GLIncludes.h>

#include "GNECalibrator.h"


// ===========================================================================
// member method definitions
// ===========================================================================

GNECalibrator::GNECalibrator(const std::string& id, GNENet* net, GNEEdge* edge, double pos, SUMOTime frequency,
                             const std::string& name, const std::string& output, const std::string& routeprobe) :
    GNEAdditional(id, net, GLO_CALIBRATOR, SUMO_TAG_CALIBRATOR, name, false,
{}, {edge}, {}, {}, {}, {}, {}, {}),
myPositionOverLane(pos),
myFrequency(frequency),
myOutput(output),
myRouteProbe(routeprobe) {
}


GNECalibrator::GNECalibrator(const std::string& id, GNENet* net, GNELane* lane, double pos, SUMOTime frequency,
                             const std::string& name, const std::string& output, const std::string& routeprobe) :
    GNEAdditional(id, net, GLO_CALIBRATOR, SUMO_TAG_LANECALIBRATOR, name, false,
{}, {}, {lane}, {}, {}, {}, {}, {}),
myPositionOverLane(pos),
myFrequency(frequency),
myOutput(output),
myRouteProbe(routeprobe) {
}


GNECalibrator::~GNECalibrator() {}


void
GNECalibrator::moveGeometry(const Position&) {
    // This additional cannot be moved
}


void
GNECalibrator::commitGeometryMoving(GNEUndoList*) {
    // This additional cannot be moved
}


void
GNECalibrator::updateGeometry() {
    // get shape depending of we have a edge or a lane
    if (getParentLanes().size() > 0) {
        // update geometry
        myAdditionalGeometry.updateGeometry(getParentLanes().front(), myPositionOverLane);
    } else if (getParentEdges().size() > 0) {
        // update geometry of first edge
        myAdditionalGeometry.updateGeometry(getParentEdges().front()->getLanes().front(), myPositionOverLane);
        // clear extra geometries
        myEdgeCalibratorGeometries.clear();
        // iterate over every lane and get point
        for (int i = 1; i < (int)getParentEdges().front()->getLanes().size(); i++) {
            // add new calibrator geometry
            GNEGeometry::Geometry calibratorGeometry;
            calibratorGeometry.updateGeometry(getParentEdges().front()->getLanes().at(i), myPositionOverLane);
            myEdgeCalibratorGeometries.push_back(calibratorGeometry);
        }
    } else {
        throw ProcessError("Both edges and lanes aren't defined");
    }
}


Position
GNECalibrator::getPositionInView() const {
    PositionVector shape = (getParentLanes().size() > 0) ? getParentLanes().front()->getLaneShape() : getParentEdges().front()->getLanes().at(0)->getLaneShape();
    if (myPositionOverLane < 0) {
        return shape.front();
    } else if (myPositionOverLane > shape.length()) {
        return shape.back();
    } else {
        return shape.positionAtOffset(myPositionOverLane);
    }
}


Boundary
GNECalibrator::getCenteringBoundary() const {
    return myAdditionalGeometry.getShape().getBoxBoundary().grow(10);
}


void
GNECalibrator::splitEdgeGeometry(const double splitPosition, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* newElement, GNEUndoList* undoList) {
    if (splitPosition < myPositionOverLane) {
        // change lane or edge
        if (newElement->getTagProperty().getTag() == SUMO_TAG_LANE) {
            setAttribute(SUMO_ATTR_LANE, newElement->getID(), undoList);
        } else {
            setAttribute(SUMO_ATTR_EDGE, newElement->getID(), undoList);
        }
        // now adjust start position
        setAttribute(SUMO_ATTR_POSITION, toString(myPositionOverLane - splitPosition), undoList);
    }
}


std::string
GNECalibrator::getParentName() const {
    // get parent name depending of we have a edge or a lane
    if (getParentLanes().size() > 0) {
        return getParentLanes().front()->getID();
    } else if (getParentEdges().size() > 0) {
        return getParentEdges().front()->getLanes().at(0)->getID();
    } else {
        throw ProcessError("Both myEdge and myLane aren't defined");
    }
}


void
GNECalibrator::drawGL(const GUIVisualizationSettings& s) const {
    // get values
    const double exaggeration = s.addSize.getExaggeration(s, this);
    // first check if additional has to be drawn
    if (s.drawAdditionals(exaggeration) && myNet->getViewNet()->getDataViewOptions().showAdditionals()) {
        // begin push name
        glPushName(getGlID());
        // draw first symbol
        drawCalibratorSymbol(s, exaggeration, myAdditionalGeometry.getPosition(), myAdditionalGeometry.getRotation());
        // continue with the other symbols
        for (const auto& edgeCalibratorGeometry : myEdgeCalibratorGeometries) {
            drawCalibratorSymbol(s, exaggeration, edgeCalibratorGeometry.getPosition(), edgeCalibratorGeometry.getRotation());
        }
        // pop name
        glPopName();
        // draw name
        drawName(getPositionInView(), s.scale, s.addName);
    }
}


void
GNECalibrator::openAdditionalDialog() {
    // Open calibrator dialog
    GNECalibratorDialog calibratorDialog(this);
}


std::string
GNECalibrator::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getID();
        case SUMO_ATTR_EDGE:
            return getParentEdges().front()->getID();
        case SUMO_ATTR_LANE:
            return getParentLanes().front()->getID();
        case SUMO_ATTR_POSITION:
            return toString(myPositionOverLane);
        case SUMO_ATTR_FREQUENCY:
            return time2string(myFrequency);
        case SUMO_ATTR_NAME:
            return myAdditionalName;
        case SUMO_ATTR_OUTPUT:
            return myOutput;
        case SUMO_ATTR_ROUTEPROBE:
            return myRouteProbe;
        case GNE_ATTR_SELECTED:
            return toString(isAttributeCarrierSelected());
        case GNE_ATTR_PARAMETERS:
            return getParametersStr();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


double
GNECalibrator::getAttributeDouble(SumoXMLAttr key) const {
    throw InvalidArgument(getTagStr() + " doesn't have a double attribute of type '" + toString(key) + "'");
}


void
GNECalibrator::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_ID:
        case SUMO_ATTR_EDGE:
        case SUMO_ATTR_LANE:
        case SUMO_ATTR_POSITION:
        case SUMO_ATTR_FREQUENCY:
        case SUMO_ATTR_NAME:
        case SUMO_ATTR_OUTPUT:
        case SUMO_ATTR_ROUTEPROBE:
        case GNE_ATTR_SELECTED:
        case GNE_ATTR_PARAMETERS:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }

}


bool
GNECalibrator::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            return isValidAdditionalID(value);
        case SUMO_ATTR_EDGE:
            if (myNet->retrieveEdge(value, false) != nullptr) {
                return true;
            } else {
                return false;
            }
        case SUMO_ATTR_LANE:
            if (myNet->retrieveLane(value, false) != nullptr) {
                return true;
            } else {
                return false;
            }
        case SUMO_ATTR_POSITION:
            if (canParse<double>(value)) {
                // obtain position and check if is valid
                double newPosition = parse<double>(value);
                PositionVector shape = (getParentLanes().size() > 0) ? getParentLanes().front()->getLaneShape() : getParentEdges().front()->getLanes().at(0)->getLaneShape();
                if ((newPosition < 0) || (newPosition > shape.length())) {
                    return false;
                } else {
                    return true;
                }
            } else {
                return false;
            }
        case SUMO_ATTR_FREQUENCY:
            return canParse<SUMOTime>(value);
        case SUMO_ATTR_NAME:
            return SUMOXMLDefinitions::isValidAttribute(value);
        case SUMO_ATTR_OUTPUT:
            return SUMOXMLDefinitions::isValidFilename(value);
        case SUMO_ATTR_ROUTEPROBE:
            return SUMOXMLDefinitions::isValidAdditionalID(value);
        case GNE_ATTR_SELECTED:
            return canParse<bool>(value);
        case GNE_ATTR_PARAMETERS:
            return Parameterised::areParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNECalibrator::isAttributeEnabled(SumoXMLAttr /* key */) const {
    return true;
}


std::string
GNECalibrator::getPopUpID() const {
    return getTagStr() + ": " + getID();
}


std::string
GNECalibrator::getHierarchyName() const {
    return getTagStr();
}

// ===========================================================================
// private
// ===========================================================================

void GNECalibrator::drawCalibratorSymbol(const GUIVisualizationSettings& s, const double exaggeration, const Position& pos, const double rot) const {
    // push layer matrix
    glPushMatrix();
    // translate to front
    myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, GLO_CALIBRATOR);
    // translate to position
    glTranslated(pos.x(), pos.y(), 0);
    // rotate
    glRotated(rot, 0, 0, 1);
    // scale
    glScaled(exaggeration, exaggeration, 1);
    // set drawing mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // set color
    if (drawUsingSelectColor()) {
        GLHelper::setColor(s.colorSettings.selectedAdditionalColor);
    } else {
        GLHelper::setColor(s.additionalSettings.calibratorColor);
    }
    // base
    glBegin(GL_TRIANGLES);
    glVertex2d(0 - s.additionalSettings.calibratorWidth, 0);
    glVertex2d(0 - s.additionalSettings.calibratorWidth, s.additionalSettings.calibratorHeight);
    glVertex2d(0 + s.additionalSettings.calibratorWidth, s.additionalSettings.calibratorHeight);
    glVertex2d(0 + s.additionalSettings.calibratorWidth, 0);
    glVertex2d(0 - s.additionalSettings.calibratorWidth, 0);
    glVertex2d(0 + s.additionalSettings.calibratorWidth, s.additionalSettings.calibratorHeight);
    glEnd();
    // draw text if isn't being drawn for selecting
    if (!s.drawForRectangleSelection && !s.drawForPositionSelection && s.drawDetail(s.detailSettings.calibratorText, exaggeration)) {
        // set color depending of selection status
        RGBColor textColor = drawUsingSelectColor() ? s.colorSettings.selectionColor : RGBColor::BLACK;
        // draw "C"
        GLHelper::drawText("C", Position(0, 1.5), 0.1, 3, textColor, 180);
        // draw "edge" or "lane "
        if (getParentLanes().size() > 0) {
            GLHelper::drawText("lane", Position(0, 3), .1, 1, textColor, 180);
        } else if (getParentEdges().size() > 0) {
            GLHelper::drawText("edge", Position(0, 3), .1, 1, textColor, 180);
        } else {
            throw ProcessError("Both myEdge and myLane aren't defined");
        }
    }
    // pop layer matrix
    glPopMatrix();
    // check if dotted contours has to be drawn
    if (s.drawDottedContour() || myNet->getViewNet()->getInspectedAttributeCarrier() == this) {
        GNEGeometry::drawDottedSquaredShape(true, s, pos, 2, 1, 2, 0, rot, exaggeration);
    }
    if (s.drawDottedContour() || myNet->getViewNet()->getFrontAttributeCarrier() == this) {
        GNEGeometry::drawDottedSquaredShape(false, s, pos, 2, 1, 2, 0, rot, exaggeration);
    }
}

void
GNECalibrator::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            myNet->getAttributeCarriers()->updateID(this, value);
            break;
        case SUMO_ATTR_EDGE:
            replaceAdditionalParentEdges(value);
            break;
        case SUMO_ATTR_LANE:
            replaceAdditionalParentLanes(value);
            break;
        case SUMO_ATTR_POSITION:
            myPositionOverLane = parse<double>(value);
            break;
        case SUMO_ATTR_FREQUENCY:
            myFrequency = parse<SUMOTime>(value);
            break;
        case SUMO_ATTR_NAME:
            myAdditionalName = value;
            break;
        case SUMO_ATTR_OUTPUT:
            myOutput = value;
            break;
        case SUMO_ATTR_ROUTEPROBE:
            myRouteProbe = value;
            break;
        case GNE_ATTR_SELECTED:
            if (parse<bool>(value)) {
                selectAttributeCarrier();
            } else {
                unselectAttributeCarrier();
            }
            break;
        case GNE_ATTR_PARAMETERS:
            setParametersStr(value);
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


/****************************************************************************/
