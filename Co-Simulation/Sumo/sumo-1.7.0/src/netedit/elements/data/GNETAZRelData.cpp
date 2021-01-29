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
/// @file    GNETAZRelData.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2020
///
// class for TAZ relation data
/****************************************************************************/


// ===========================================================================
// included modules
// ===========================================================================
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEViewParent.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/frames/data/GNETAZRelDataFrame.h>

#include "GNETAZRelData.h"
#include "GNEDataInterval.h"


// ===========================================================================
// member method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNETAZRelData - methods
// ---------------------------------------------------------------------------

GNETAZRelData::GNETAZRelData(GNEDataInterval* dataIntervalParent, GNETAZElement* fromTAZ, GNETAZElement* toTAZ,
                             const std::map<std::string, std::string>& parameters) :
    GNEGenericData(SUMO_TAG_TAZREL, GLO_TAZRELDATA, dataIntervalParent, parameters,
{}, {}, {}, {}, {}, {fromTAZ, toTAZ}, {}, {}) {
}


GNETAZRelData::~GNETAZRelData() {}


const RGBColor&
GNETAZRelData::getColor() const {
    return RGBColor::ORANGE;
}


bool
GNETAZRelData::isGenericDataVisible() const {
    // first check if we're in supermode data
    if (!myNet->getViewNet()->getEditModes().isCurrentSupermodeData()) {
        return false;
    }
    // obtain pointer to TAZ data frame (only for code legibly)
    const GNETAZRelDataFrame* TAZRelDataFrame = myNet->getViewNet()->getViewParent()->getTAZRelDataFrame();
    // get current data edit mode
    DataEditMode dataMode = myNet->getViewNet()->getEditModes().dataEditMode;
    // check if we have to filter generic data
    if ((dataMode == DataEditMode::DATA_INSPECT) || (dataMode == DataEditMode::DATA_DELETE) || (dataMode == DataEditMode::DATA_SELECT)) {
        return isVisibleInspectDeleteSelect();
    } else if (TAZRelDataFrame->shown()) {
        // check interval
        if ((TAZRelDataFrame->getIntervalSelector()->getDataInterval() != nullptr) &&
                (TAZRelDataFrame->getIntervalSelector()->getDataInterval() != myDataIntervalParent)) {
            return false;
        }
        // check attribute
        if ((TAZRelDataFrame->getAttributeSelector()->getFilteredAttribute().size() > 0) &&
                (getParametersMap().count(TAZRelDataFrame->getAttributeSelector()->getFilteredAttribute()) == 0)) {
            return false;
        }
        // all checks ok, then return true
        return true;
    } else {
        // GNETAZRelDataFrame hidden, then return false
        return false;
    }
}


void
GNETAZRelData::updateGeometry() {
    // nothing to update
}


Position
GNETAZRelData::getPositionInView() const {
    return getParentTAZElements().front()->getPositionInView();
}


void
GNETAZRelData::writeGenericData(OutputDevice& device) const {
    // open device
    device.openTag(SUMO_TAG_TAZREL);
    // write from
    device.writeAttr(SUMO_ATTR_FROM, getParentTAZElements().front()->getID());
    // write to
    device.writeAttr(SUMO_ATTR_TO, getParentTAZElements().back()->getID());
    // iterate over attributes
    for (const auto& attribute : getParametersMap()) {
        // write attribute (don't use writeParams)
        device.writeAttr(attribute.first, attribute.second);
    }
    // close device
    device.closeTag();
}


bool
GNETAZRelData::isGenericDataValid() const {
    return true;
}


std::string
GNETAZRelData::getGenericDataProblem() const {
    return "";
}


void
GNETAZRelData::fixGenericDataProblem() {
    throw InvalidArgument(getTagStr() + " cannot fix any problem");
}


void
GNETAZRelData::drawGL(const GUIVisualizationSettings& /*s*/) const {
    // Nothing to draw
}


void
GNETAZRelData::drawPartialGL(const GUIVisualizationSettings& /*s*/, const GNELane* /*lane*/, const double /*offsetFront*/) const {
    //
}


void
GNETAZRelData::drawPartialGL(const GUIVisualizationSettings& /*s*/, const GNELane* /*fromLane*/, const GNELane* /*toLane*/, const double /*offsetFront*/) const {
    //
}


Boundary
GNETAZRelData::getCenteringBoundary() const {
    return getParentTAZElements().front()->getCenteringBoundary();
}


std::string
GNETAZRelData::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getParentTAZElements().front()->getID();
        case SUMO_ATTR_FROM:
            return getParentTAZElements().front()->getID();
        case SUMO_ATTR_TO:
            return getParentTAZElements().back()->getID();
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
GNETAZRelData::getAttributeDouble(SumoXMLAttr key) const {
    throw InvalidArgument(getTagStr() + " doesn't have a double attribute of type '" + toString(key) + "'");
}


void
GNETAZRelData::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_FROM:
        case SUMO_ATTR_TO:
        case GNE_ATTR_SELECTED:
        case GNE_ATTR_PARAMETERS:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNETAZRelData::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_FROM:
        case SUMO_ATTR_TO:
            return SUMOXMLDefinitions::isValidNetID(value) && (myNet->retrieveTAZElement(SUMO_TAG_TAZ, value, false) != nullptr);
        case GNE_ATTR_SELECTED:
            return canParse<bool>(value);
        case GNE_ATTR_PARAMETERS:
            return Parameterised::areParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNETAZRelData::enableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    // Nothing to enable
}


void
GNETAZRelData::disableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    // Nothing to disable enable
}


bool GNETAZRelData::isAttributeEnabled(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return false;
        default:
            return true;
    }
}


std::string
GNETAZRelData::getPopUpID() const {
    return getTagStr();
}


std::string
GNETAZRelData::getHierarchyName() const {
    return getTagStr() + ": " + getParentTAZElements().front()->getID() + "->" + getParentTAZElements().back()->getID();
}


void
GNETAZRelData::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_FROM: {
            // change first TAZ
            replaceFirstParentTAZElement(SUMO_TAG_TAZ, value);
            break;
        }
        case SUMO_ATTR_TO: {
            // change last TAZ
            replaceLastParentTAZElement(SUMO_TAG_TAZ, value);
            break;
        }
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
GNETAZRelData::setEnabledAttribute(const int /*enabledAttributes*/) {
    throw InvalidArgument("Nothing to enable");
}

/****************************************************************************/
