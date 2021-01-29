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
/// @file    GNETagProperties.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2020
///
// Abstract Base class for tag properties used in GNEAttributeCarrier
/****************************************************************************/


// ===========================================================================
// included modules
// ===========================================================================

#include "GNETagProperties.h"


// ===========================================================================
// static members
// ===========================================================================

const size_t GNETagProperties::MAXNUMBEROFATTRIBUTES = 128;

// ===========================================================================
// method definitions
// ===========================================================================

GNETagProperties::GNETagProperties() :
    myTag(SUMO_TAG_NOTHING),
    myTagType(0),
    myTagProperty(0),
    myIcon(GUIIcon::EMPTY),
    myTagSynonym(SUMO_TAG_NOTHING) {
}


GNETagProperties::GNETagProperties(SumoXMLTag tag, int tagType, int tagProperty, GUIIcon icon, const std::vector<SumoXMLTag>& masterTags, SumoXMLTag tagSynonym) :
    myTag(tag),
    myTagStr(toString(tag)),
    myTagType(tagType),
    myTagProperty(tagProperty),
    myIcon(icon),
    myMasterTags(masterTags),
    myTagSynonym(tagSynonym) {
}


GNETagProperties::~GNETagProperties() {}


SumoXMLTag
GNETagProperties::getTag() const {
    return myTag;
}


const std::string&
GNETagProperties::getTagStr() const {
    return myTagStr;
}


void
GNETagProperties::checkTagIntegrity() const {
    // check that element must ist at least networkElement, Additional, or shape
    if (!isNetworkElement() && !isAdditionalElement() && !isShape() && !isTAZElement() && !isDemandElement() && !isDataElement()) {
        throw ProcessError("element must be at leas networkElement, additional, TAZ, shape, demandElement or dataElement");
    }
    // check that element only is networkElement, Additional, or shape at the same time
    if ((isNetworkElement() + isAdditionalElement() + isShape() + isTAZElement() + isDemandElement() + isDataElement()) > 1) {
        throw ProcessError("element can be only a networkElement, additional, TAZ, shape, demandElement or dataElement at the same time");
    }
    // if element can mask the start and end position, check that bot attributes exist
    if (canMaskStartEndPos() && (!hasAttribute(SUMO_ATTR_STARTPOS) || !hasAttribute(SUMO_ATTR_ENDPOS))) {
        throw ProcessError("If attribute mask the start and end position, bot attribute has to be defined");
    }
    // check that synonym tag isn't nothing
    if (hasTagSynonym() && (myTagSynonym == SUMO_TAG_NOTHING)) {
        throw FormatException("Synonym tag cannot be nothing");
    }
    // check that synonym was defined
    if (!hasTagSynonym() && (myTagSynonym != SUMO_TAG_NOTHING)) {
        throw FormatException("Tag doesn't support synonyms");
    }
    // check that master tag is valid
    if (isSlave() && myMasterTags.empty()) {
        throw FormatException("Master tags cannot be empty");
    }
    // check that master was defined
    if (!isSlave() && !myMasterTags.empty()) {
        throw FormatException("Tag doesn't support master elements");
    }
    // check integrity of all attributes
    for (auto attributeProperty : myAttributeProperties) {
        attributeProperty.checkAttributeIntegrity();
        // check that if attribute is vehicle classes, own a combination of Allow/disallow attibute
        if (attributeProperty.isVClasses()) {
            if ((attributeProperty.getAttr() != SUMO_ATTR_ALLOW) && (attributeProperty.getAttr() != SUMO_ATTR_DISALLOW)) {
                throw ProcessError("Attributes aren't combinables");
            } else if ((attributeProperty.getAttr() == SUMO_ATTR_ALLOW) && !hasAttribute(SUMO_ATTR_DISALLOW)) {
                throw ProcessError("allow need a disallow attribute in the same tag");
            } else if ((attributeProperty.getAttr() == SUMO_ATTR_DISALLOW) && !hasAttribute(SUMO_ATTR_ALLOW)) {
                throw ProcessError("disallow need an allow attribute in the same tag");
            }
        }
    }
}


const std::string&
GNETagProperties::getDefaultValue(SumoXMLAttr attr) const {
    // iterate over attribute properties
    for (const auto& i : myAttributeProperties) {
        if (i.getAttr() == attr) {
            if (!i.hasStaticDefaultValue()) {
                throw ProcessError("attribute '" + i.getAttrStr() + "' doesn't have a default value");
            } else {
                return i.getDefaultValue();
            }
        }
    }
    throw ProcessError("Attribute '" + toString(attr) + "' not defined");
}


void
GNETagProperties::addAttribute(const GNEAttributeProperties& attributeProperty) {
    if (isAttributeDeprecated(attributeProperty.getAttr())) {
        throw ProcessError("Attribute '" + attributeProperty.getAttrStr() + "' is deprecated and cannot be inserted");
    } else if ((myAttributeProperties.size() + 1) >= MAXNUMBEROFATTRIBUTES) {
        throw ProcessError("Maximum number of attributes for tag " + attributeProperty.getAttrStr() + " exceeded");
    } else {
        // Check that attribute wasn't already inserted
        for (auto i : myAttributeProperties) {
            if (i.getAttr() == attributeProperty.getAttr()) {
                throw ProcessError("Attribute '" + attributeProperty.getAttrStr() + "' already inserted");
            }
        }
        // insert AttributeProperties in vector
        myAttributeProperties.push_back(attributeProperty);
        myAttributeProperties.back().setTagPropertyParent(this);
    }
}


void
GNETagProperties::addDeprecatedAttribute(SumoXMLAttr attr) {
    // Check that attribute wasn't already inserted
    for (auto i : myAttributeProperties) {
        if (i.getAttr() == attr) {
            throw ProcessError("Attribute '" + toString(attr) + "' is deprecated but was inserted in list of attributes");
        }
    }
    // add it into myDeprecatedAttributes
    myDeprecatedAttributes.push_back(attr);
}


const GNEAttributeProperties&
GNETagProperties::getAttributeProperties(SumoXMLAttr attr) const {
    // iterate over attribute properties
    for (const auto& i : myAttributeProperties) {
        if ((i.getAttr() == attr) || (i.hasAttrSynonym() && (i.getAttrSynonym() == attr))) {
            return i;
        }
    }
    // throw error if these attribute doesn't exist
    throw ProcessError("Attribute '" + toString(attr) + "' doesn't exist");
}


std::vector<GNEAttributeProperties>::const_iterator
GNETagProperties::begin() const {
    return myAttributeProperties.begin();
}


std::vector<GNEAttributeProperties>::const_iterator
GNETagProperties::end() const {
    return myAttributeProperties.end();
}


int
GNETagProperties::getNumberOfAttributes() const {
    return (int)myAttributeProperties.size();
}


GUIIcon
GNETagProperties::getGUIIcon() const {
    return myIcon;
}


const std::vector<SumoXMLTag>&
GNETagProperties::getMasterTags() const {
    return myMasterTags;
}


SumoXMLTag
GNETagProperties::getTagSynonym() const {
    if (hasTagSynonym()) {
        return myTagSynonym;
    } else {
        throw ProcessError("Tag doesn't have synonym");
    }
}


bool
GNETagProperties::hasAttribute(SumoXMLAttr attr) const {
    // iterate over attribute properties
    for (const auto& i : myAttributeProperties) {
        if (i.getAttr() == attr) {
            return true;
        }
    }
    return false;
}


bool
GNETagProperties::isNetworkElement() const {
    return (myTagType & NETWORKELEMENT) != 0;
}


bool
GNETagProperties::isAdditionalElement() const {
    return (myTagType & ADDITIONALELEMENT) != 0;
}

bool
GNETagProperties::isShape() const {
    return (myTagType & SHAPE) != 0;
}


bool
GNETagProperties::isTAZElement() const {
    return (myTagType & TAZELEMENT) != 0;
}


bool
GNETagProperties::isDemandElement() const {
    return (myTagType & DEMANDELEMENT) != 0;
}


bool
GNETagProperties::isDataElement() const {
    return (myTagType & DATAELEMENT) != 0;
}


bool
GNETagProperties::isStoppingPlace() const {
    return (myTagType & STOPPINGPLACE) != 0;
}


bool
GNETagProperties::isDetector() const {
    return (myTagType & DETECTOR) != 0;
}


bool
GNETagProperties::isVehicleType() const {
    return (myTagType & VTYPE) != 0;
}


bool
GNETagProperties::isVehicle() const {
    return (myTagType & VEHICLE) != 0;
}

bool
GNETagProperties::isRoute() const {
    return (myTagType & ROUTE) != 0;
}


bool
GNETagProperties::isStop() const {
    return (myTagType & STOP) != 0;
}


bool
GNETagProperties::isPerson() const {
    return (myTagType & PERSON) != 0;
}


bool
GNETagProperties::isPersonPlan() const {
    return (myTagType & PERSONPLAN) != 0;
}


bool
GNETagProperties::isPersonTrip() const {
    return (myTagType & PERSONTRIP) != 0;
}


bool
GNETagProperties::isWalk() const {
    return (myTagType & WALK) != 0;
}


bool
GNETagProperties::isRide() const {
    return (myTagType & RIDE) != 0;
}


bool
GNETagProperties::isPersonStop() const {
    return (myTagType & PERSONSTOP) != 0;
}


bool
GNETagProperties::isGenericData() const {
    return (myTagType & GENERICDATA) != 0;
}


bool
GNETagProperties::isSlave() const {
    return (myTagProperty & SLAVE) != 0;
}


bool
GNETagProperties::isSymbol() const {
    return (myTagType & SYMBOL) != 0;
}


bool
GNETagProperties::isDrawable() const {
    return (myTagProperty & DRAWABLE) != 0;
}


bool
GNETagProperties::isSelectable() const {
    return (myTagProperty & SELECTABLE) != 0;
}


bool
GNETagProperties::canBlockMovement() const {
    return (myTagProperty & BLOCKMOVEMENT) != 0;
}


bool
GNETagProperties::canBlockShape() const {
    return (myTagProperty & BLOCKSHAPE) != 0;
}


bool
GNETagProperties::canCloseShape() const {
    return (myTagProperty & CLOSESHAPE) != 0;
}


bool
GNETagProperties::hasGEOPosition() const {
    return (myTagProperty & GEOPOSITION) != 0;
}


bool
GNETagProperties::hasGEOShape() const {
    return (myTagProperty & GEOSHAPE) != 0;
}


bool
GNETagProperties::hasTagSynonym() const {
    return (myTagProperty & SYNONYM) != 0;
}


bool
GNETagProperties::hasDialog() const {
    return (myTagProperty & DIALOG) != 0;
}


bool
GNETagProperties::hasMinimumNumberOfChildren() const {
    return (myTagProperty & MINIMUMCHILDREN) != 0;
}


bool
GNETagProperties::hasParameters() const {
    // note: By default all Tags supports parameters, except Tags with "NOPARAMETERS"
    return (myTagProperty & NOPARAMETERS) == 0;
}


bool
GNETagProperties::hasDoubleParameters() const {
    return (myTagProperty & PARAMETERSDOUBLE) != 0;
}


bool
GNETagProperties::isPlacedInRTree() const {
    return (myTagProperty & RTREE) != 0;
}


bool
GNETagProperties::canBeReparent() const {
    return (myTagProperty & REPARENT) != 0;
}


bool
GNETagProperties::canWriteChildrenSeparate() const {
    return (myTagProperty & WRITECHILDRENSEPARATE) != 0;
}


bool
GNETagProperties::canMaskStartEndPos() const {
    return (myTagProperty & MASKSTARTENDPOS) != 0;
}


bool
GNETagProperties::canMaskXYZPositions() const {
    return (myTagProperty & MASKXYZPOSITION) != 0;
}


bool
GNETagProperties::canCenterCameraAfterCreation() const {
    return (myTagProperty & CENTERAFTERCREATION) != 0;
}


bool
GNETagProperties::personPlanStartEdge() const {
    return (myTagProperty & PERSONPLAN_START_EDGE) != 0;
}


bool
GNETagProperties::personPlanEndEdge() const {
    return (myTagProperty & PERSONPLAN_END_EDGE) != 0;
}


bool
GNETagProperties::personPlanStartBusStop() const {
    return (myTagProperty & PERSONPLAN_START_BUSSTOP) != 0;
}


bool
GNETagProperties::personPlanEndBusStop() const {
    return (myTagProperty & PERSONPLAN_END_BUSSTOP) != 0;
}


bool
GNETagProperties::personPlanStartStop() const {
    return (myTagProperty & PERSONPLAN_START_STOP) != 0;
}


bool
GNETagProperties::personPlanEndStop() const {
    return (myTagProperty & PERSONPLAN_END_STOP) != 0;
}


bool
GNETagProperties::embebbedRoute() const {
    return (myTagProperty & EMBEDDED_ROUTE) != 0;
}


bool
GNETagProperties::isAttributeDeprecated(SumoXMLAttr attr) const {
    return (std::find(myDeprecatedAttributes.begin(), myDeprecatedAttributes.end(), attr) != myDeprecatedAttributes.end());
}

/****************************************************************************/
