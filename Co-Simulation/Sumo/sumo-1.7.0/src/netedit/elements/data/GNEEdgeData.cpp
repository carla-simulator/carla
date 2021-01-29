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
/// @file    GNEEdgeData.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2020
///
// class for edge data
/****************************************************************************/


// ===========================================================================
// included modules
// ===========================================================================
#include <config.h>

#include <netedit/GNEUndoList.h>
#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEViewParent.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/frames/data/GNEEdgeDataFrame.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GLIncludes.h>

#include "GNEEdgeData.h"
#include "GNEDataInterval.h"


// ===========================================================================
// member method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEEdgeData - methods
// ---------------------------------------------------------------------------

GNEEdgeData::GNEEdgeData(GNEDataInterval* dataIntervalParent, GNEEdge* edgeParent, const std::map<std::string, std::string>& parameters) :
    GNEGenericData(SUMO_TAG_MEANDATA_EDGE, GLO_EDGEDATA, dataIntervalParent, parameters,
{}, {edgeParent}, {}, {}, {}, {}, {}, {}) {
}


GNEEdgeData::~GNEEdgeData() {}


const RGBColor&
GNEEdgeData::getColor() const {
    if (myNet->getViewNet()->getEditModes().dataEditMode == DataEditMode::DATA_EDGEDATA) {
        // get selected data interval and filtered attribute
        const GNEDataInterval* dataInterval = myNet->getViewNet()->getViewParent()->getEdgeDataFrame()->getIntervalSelector()->getDataInterval();
        const std::string filteredAttribute = myNet->getViewNet()->getViewParent()->getEdgeDataFrame()->getAttributeSelector()->getFilteredAttribute();
        // continue if there is a selected data interval and filtered attribute
        if (dataInterval && (filteredAttribute.size() > 0)) {
            // obtain minimum and maximum value
            const double minValue = dataInterval->getSpecificAttributeColors().at(myTagProperty.getTag()).getMinValue(filteredAttribute);
            const double maxValue = dataInterval->getSpecificAttributeColors().at(myTagProperty.getTag()).getMaxValue(filteredAttribute);
            // get value
            const double value = parse<double>(getParameter(filteredAttribute, "0"));
            // return color
            return GNEViewNetHelper::getRainbowScaledColor(minValue, maxValue, value);
        }
    }
    // return default color
    return RGBColor::RED;
}


bool
GNEEdgeData::isGenericDataVisible() const {
    // first check if we're in supermode data
    if (!myNet->getViewNet()->getEditModes().isCurrentSupermodeData()) {
        return false;
    }
    // get current data edit mode
    DataEditMode dataMode = myNet->getViewNet()->getEditModes().dataEditMode;
    // check if we have to filter generic data
    if ((dataMode == DataEditMode::DATA_INSPECT) || (dataMode == DataEditMode::DATA_DELETE) || (dataMode == DataEditMode::DATA_SELECT)) {
        return isVisibleInspectDeleteSelect();
    } else if (myDataIntervalParent->getNet()->getViewNet()->getViewParent()->getEdgeDataFrame()->shown()) {
        // get selected data interval and filtered attribute
        const GNEDataInterval* dataInterval = myNet->getViewNet()->getViewParent()->getEdgeDataFrame()->getIntervalSelector()->getDataInterval();
        const std::string filteredAttribute = myNet->getViewNet()->getViewParent()->getEdgeDataFrame()->getAttributeSelector()->getFilteredAttribute();
        // check interval
        if ((dataInterval != nullptr) && (dataInterval != myDataIntervalParent)) {
            return false;
        }
        // check attribute
        if ((filteredAttribute.size() > 0) && (getParametersMap().count(filteredAttribute) == 0)) {
            return false;
        }
        // all checks ok, then return true
        return true;
    } else {
        // GNEEdgeDataFrame hidden, then return false
        return false;
    }
}


void
GNEEdgeData::updateGeometry() {
    // calculate generic data path
    calculateGenericDataLanePath(getParentEdges());
}


Position
GNEEdgeData::getPositionInView() const {
    return getParentEdges().front()->getPositionInView();
}


void
GNEEdgeData::writeGenericData(OutputDevice& device) const {
    // open device (don't use SUMO_TAG_MEANDATA_EDGE)
    device.openTag(SUMO_TAG_EDGE);
    // write edge ID
    device.writeAttr(SUMO_ATTR_ID, getParentEdges().front()->getID());
    // iterate over attributes
    for (const auto& attribute : getParametersMap()) {
        // write attribute (don't use writeParams)
        device.writeAttr(attribute.first, attribute.second);
    }
    // close device
    device.closeTag();
}


bool
GNEEdgeData::isGenericDataValid() const {
    return true;
}


std::string
GNEEdgeData::getGenericDataProblem() const {
    return "";
}


void
GNEEdgeData::fixGenericDataProblem() {
    throw InvalidArgument(getTagStr() + " cannot fix any problem");
}


void
GNEEdgeData::drawGL(const GUIVisualizationSettings& /*s*/) const {
    // Nothing to draw
}


void
GNEEdgeData::drawPartialGL(const GUIVisualizationSettings& s, const GNELane* lane, const double offsetFront) const {
    // get lane width
    const double laneWidth = s.addSize.getExaggeration(s, lane) * (lane->getParentEdge()->getNBEdge()->getLaneWidth(lane->getIndex()) * 0.5);
    // Start drawing adding an gl identificator
    glPushName(getGlID());
    // Add a draw matrix
    glPushMatrix();
    // Start with the drawing of the area traslating matrix to origin
    myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, GLO_EDGEDATA, offsetFront);
    // Set orange color
    GLHelper::setColor(RGBColor::BLACK);
    // draw box lines
    GNEGeometry::drawLaneGeometry(myNet->getViewNet(), lane->getLaneShape(), lane->getShapeRotations(), lane->getShapeLengths(), {}, laneWidth);
    // translate to top
    glTranslated(0, 0, 0.01);
    // Set color
    if (isAttributeCarrierSelected()) {
        GLHelper::setColor(s.colorSettings.selectedEdgeDataColor);
    } else {
        GLHelper::setColor(getColor());
    }
    // draw interne box lines
    GNEGeometry::drawLaneGeometry(myNet->getViewNet(), lane->getLaneShape(), lane->getShapeRotations(), lane->getShapeLengths(), {}, laneWidth - 0.1);
    // Pop last matrix
    glPopMatrix();
    // Pop name
    glPopName();
    // draw filtered attribute
    if (getParentEdges().front()->getLanes().front() == lane) {
        drawFilteredAttribute(s, lane->getLaneShape(), myNet->getViewNet()->getViewParent()->getEdgeDataFrame()->getAttributeSelector()->getFilteredAttribute());
    }
    // check if shape dotted contour has to be drawn
    if (s.drawDottedContour() || (myNet->getViewNet()->getInspectedAttributeCarrier() == this)) {
        GNEGeometry::drawDottedContourEdge(true, s, lane->getParentEdge(), true, true);
    }
}


void
GNEEdgeData::drawPartialGL(const GUIVisualizationSettings& /*s*/, const GNELane* /*fromLane*/, const GNELane* /*toLane*/, const double /*offsetFront*/) const {
    // EdgeDatas don't use drawPartialGL over junction
}


Boundary
GNEEdgeData::getCenteringBoundary() const {
    return getParentEdges().front()->getCenteringBoundary();
}


std::string
GNEEdgeData::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getParentEdges().front()->getID();
        case GNE_ATTR_DATASET:
            return myDataIntervalParent->getDataSetParent()->getID();
        case GNE_ATTR_SELECTED:
            return toString(isAttributeCarrierSelected());
        case GNE_ATTR_PARAMETERS:
            return getParametersStr();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


double
GNEEdgeData::getAttributeDouble(SumoXMLAttr key) const {
    throw InvalidArgument(getTagStr() + " doesn't have a double attribute of type '" + toString(key) + "'");
}


void
GNEEdgeData::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case GNE_ATTR_SELECTED:
        case GNE_ATTR_PARAMETERS:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEEdgeData::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case GNE_ATTR_SELECTED:
            return canParse<bool>(value);
        case GNE_ATTR_PARAMETERS:
            return Parameterised::areParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEEdgeData::enableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    // Nothing to enable
}


void
GNEEdgeData::disableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    // Nothing to disable enable
}


bool GNEEdgeData::isAttributeEnabled(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return false;
        default:
            return true;
    }
}


std::string
GNEEdgeData::getPopUpID() const {
    return getTagStr();
}


std::string
GNEEdgeData::getHierarchyName() const {
    return getTagStr() + ": " + getParentEdges().front()->getID();
}


void
GNEEdgeData::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case GNE_ATTR_SELECTED:
            if (parse<bool>(value)) {
                selectAttributeCarrier();
            } else {
                unselectAttributeCarrier();
            }
            break;
        case GNE_ATTR_PARAMETERS:
            setParametersStr(value);
            // update attribute colors
            myDataIntervalParent->getDataSetParent()->updateAttributeColors();
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEEdgeData::setEnabledAttribute(const int /*enabledAttributes*/) {
    throw InvalidArgument("Nothing to enable");
}

/****************************************************************************/
