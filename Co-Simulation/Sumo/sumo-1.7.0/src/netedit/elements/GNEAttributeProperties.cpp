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
/// @file    GNEAttributeProperties.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2020
///
// Abstract Base class for tag properties used in GNEAttributeCarrier
/****************************************************************************/


// ===========================================================================
// included modules
// ===========================================================================

#include "GNEAttributeProperties.h"
#include "GNETagProperties.h"


// ===========================================================================
// method definitions
// ===========================================================================

GNEAttributeProperties::GNEAttributeProperties() :
    myAttribute(SUMO_ATTR_NOTHING),
    myTagPropertyParent(nullptr),
    myAttrStr(toString(SUMO_ATTR_NOTHING)),
    myAttributeProperty(STRING),
    myDefinition(""),
    myDefaultValue(""),
    myAttrSynonym(SUMO_ATTR_NOTHING),
    myMinimumRange(0),
    myMaximumRange(0) {}


GNEAttributeProperties::GNEAttributeProperties(const SumoXMLAttr attribute, const int attributeProperty, const std::string& definition, std::string defaultValue) :
    myAttribute(attribute),
    myTagPropertyParent(nullptr),
    myAttrStr(toString(attribute)),
    myAttributeProperty(attributeProperty),
    myDefinition(definition),
    myDefaultValue(defaultValue),
    myAttrSynonym(SUMO_ATTR_NOTHING),
    myMinimumRange(0),
    myMaximumRange(0) {
    // empty definition aren't valid
    if (definition.empty()) {
        throw FormatException("Missing definition for AttributeProperty '" + toString(attribute) + "'");
    }
    // if default value isn't empty, but attribute doesn't support default values, throw exception.
    if (!defaultValue.empty() && !(attributeProperty & DEFAULTVALUESTATIC)) {
        throw FormatException("AttributeProperty for '" + toString(attribute) + "' doesn't support default values");
    }
    // default value cannot be static and mutables at the same time
    if ((attributeProperty & DEFAULTVALUESTATIC) && (attributeProperty & DEFAULTVALUEMUTABLE)) {
        throw FormatException("Default value for attribute '" + toString(attribute) + "' cannot be static and mutable at the same time");
    }
    // Attributes that can write optionally their values in XML must have either a static or a mutable efault value
    if ((attributeProperty & XMLOPTIONAL) && !((attributeProperty & DEFAULTVALUESTATIC) || (attributeProperty & DEFAULTVALUEMUTABLE))) {
        throw FormatException("Attribute '" + toString(attribute) + "' requires a either static or mutable default value");
    }
    // Attributes cannot be flowdefinition and enabilitablet at the same time
    if ((attributeProperty & FLOWDEFINITION) && (attributeProperty & ACTIVATABLE)) {
        throw FormatException("Attribute '" + toString(attribute) + "' cannot be flowdefinition and activatable at the same time");
    }
}


GNEAttributeProperties::~GNEAttributeProperties() {}


void
GNEAttributeProperties::checkAttributeIntegrity() {
    // check that positive attributes correspond only to a int, floats or SUMOTimes
    if (isPositive() && !(isInt() || isFloat() || isSUMOTime())) {
        throw FormatException("Only int, floats or SUMOTimes can be positive");
    }
    // check that secuential attributes correspond to a list
    if (isSecuential() && !isList()) {
        throw FormatException("Secuential property only is compatible with list properties");
    }
    // check that synonym attribute isn't nothing
    if (hasAttrSynonym() && (myAttrSynonym == SUMO_ATTR_NOTHING)) {
        throw FormatException("synonym attribute cannot be nothing");
    }
    // check that ranges are valid
    if (hasAttrRange()) {
        if (myMinimumRange == myMaximumRange) {
            throw FormatException("empty range");
        } else if ((myMinimumRange == 0) && (myMaximumRange == 0)) {
            throw FormatException("non-defined range");
        } else if ((myMaximumRange - myMinimumRange) <= 0) {
            throw FormatException("invalid range");
        }
    }
    // check that positive attributes correspond only to a int, floats or SUMOTimes
    if (isOptional() && !(hasStaticDefaultValue() || hasMutableDefaultValue())) {
        throw FormatException("if attribute is optional, must have either a static or dynamic default value");
    }
}


void
GNEAttributeProperties::setDiscreteValues(const std::vector<std::string>& discreteValues) {
    if (isDiscrete()) {
        myDiscreteValues = discreteValues;
    } else {
        throw FormatException("AttributeProperty doesn't support discrete values values");
    }
}


void
GNEAttributeProperties::setSynonym(const SumoXMLAttr synonym) {
    if (hasAttrSynonym()) {
        myAttrSynonym = synonym;
    } else {
        throw FormatException("AttributeProperty doesn't support synonyms");
    }
}


void
GNEAttributeProperties::setRange(const double minimum, const double maximum) {
    if (hasAttrRange()) {
        myMinimumRange = minimum;
        myMaximumRange = maximum;
        // check that given range is valid
        if (myMinimumRange == myMaximumRange) {
            throw FormatException("empty range");
        } else if ((myMinimumRange == 0) && (myMaximumRange == 0)) {
            throw FormatException("non-defined range");
        } else if ((myMaximumRange - myMinimumRange) <= 0) {
            throw FormatException("invalid range");
        }
    } else {
        throw FormatException("AttributeProperty doesn't support ranges");
    }
}


void
GNEAttributeProperties::setTagPropertyParent(GNETagProperties* tagPropertyParent) {
    myTagPropertyParent = tagPropertyParent;
}


SumoXMLAttr
GNEAttributeProperties::getAttr() const {
    return myAttribute;
}


const std::string&
GNEAttributeProperties::getAttrStr() const {
    return myAttrStr;
}


const GNETagProperties&
GNEAttributeProperties::getTagPropertyParent() const {
    return *myTagPropertyParent;
}


int
GNEAttributeProperties::getPositionListed() const {
    for (auto i = myTagPropertyParent->begin(); i != myTagPropertyParent->end(); i++) {
        if (i->getAttr() == myAttribute) {
            return (int)(i - myTagPropertyParent->begin());
        }
    }
    throw ProcessError("Attribute wasn't found in myTagPropertyParent");
}


const std::string&
GNEAttributeProperties::getDefinition() const {
    return myDefinition;
}


const std::string&
GNEAttributeProperties::getDefaultValue() const {
    return myDefaultValue;
}


std::string
GNEAttributeProperties::getDescription() const {
    std::string pre;
    std::string type;
    std::string plural;
    std::string last;
    // pre type
    if ((myAttributeProperty & LIST) != 0) {
        pre += "list of ";
        if ((myAttributeProperty & VCLASS) != 0) {
            plural = "es";
        } else {
            plural = "s";
        }
    }
    if ((myAttributeProperty & POSITIVE) != 0) {
        pre += "positive ";
    }
    if ((myAttributeProperty & DISCRETE) != 0) {
        pre += "discrete ";
    }
    if ((myAttributeProperty & XMLOPTIONAL) != 0) {
        pre += "optional ";
    }
    if ((myAttributeProperty & UNIQUE) != 0) {
        pre += "unique ";
    }
    if ((myAttributeProperty & VCLASSES) != 0) {
        pre += "vclasses ";
    }
    // type
    if ((myAttributeProperty & INT) != 0) {
        type = "integer";
    }
    if ((myAttributeProperty & FLOAT) != 0) {
        type = "float";
    }
    if ((myAttributeProperty & SUMOTIME) != 0) {
        type = "SUMOTime";
    }
    if ((myAttributeProperty & BOOL) != 0) {
        type = "boolean";
    }
    if ((myAttributeProperty & STRING) != 0) {
        type = "string";
    }
    if ((myAttributeProperty & POSITION) != 0) {
        type = "position";
    }
    if ((myAttributeProperty & COLOR) != 0) {
        type = "color";
    }
    if ((myAttributeProperty & VCLASS) != 0) {
        type = "VClass";
    }
    if ((myAttributeProperty & FILENAME) != 0) {
        type = "filename";
    }
    if ((myAttributeProperty & PROBABILITY) != 0) {
        type = "probability";
        last = "[0, 1]";
    }
    if ((myAttributeProperty & ANGLE) != 0) {
        type = "angle";
        last = "[0, 360]";
    }
    return pre + type + plural + last;
}


const std::vector<std::string>&
GNEAttributeProperties::getDiscreteValues() const {
    return myDiscreteValues;
}


SumoXMLAttr
GNEAttributeProperties::getAttrSynonym() const {
    if (hasAttrSynonym()) {
        return myAttrSynonym;
    } else {
        throw ProcessError("Attr doesn't support synonym");
    }
}


double
GNEAttributeProperties::getMinimumRange() const {
    if (hasAttrRange()) {
        return myMinimumRange;
    } else {
        throw ProcessError("Attr doesn't support range");
    }
}


double
GNEAttributeProperties::getMaximumRange() const {
    if (hasAttrRange()) {
        return myMaximumRange;
    } else {
        throw ProcessError("Attr doesn't support range");
    }
}


bool
GNEAttributeProperties::hasStaticDefaultValue() const {
    return (myAttributeProperty & DEFAULTVALUESTATIC) != 0;
}


bool
GNEAttributeProperties::hasMutableDefaultValue() const {
    return (myAttributeProperty & DEFAULTVALUEMUTABLE) != 0;
}


bool
GNEAttributeProperties::hasAttrSynonym() const {
    return (myAttributeProperty & SYNONYM) != 0;
}

bool
GNEAttributeProperties::hasAttrRange() const {
    return (myAttributeProperty & RANGE) != 0;
}


bool
GNEAttributeProperties::isInt() const {
    return (myAttributeProperty & INT) != 0;
}


bool
GNEAttributeProperties::isFloat() const {
    return (myAttributeProperty & FLOAT) != 0;
}


bool
GNEAttributeProperties::isSUMOTime() const {
    return (myAttributeProperty & SUMOTIME) != 0;
}


bool
GNEAttributeProperties::isBool() const {
    return (myAttributeProperty & BOOL) != 0;
}


bool
GNEAttributeProperties::isString() const {
    return (myAttributeProperty & STRING) != 0;
}


bool
GNEAttributeProperties::isposition() const {
    return (myAttributeProperty & POSITION) != 0;
}


bool
GNEAttributeProperties::isProbability() const {
    return (myAttributeProperty & PROBABILITY) != 0;
}


bool
GNEAttributeProperties::isNumerical() const {
    return (myAttributeProperty & (INT | FLOAT | SUMOTIME)) != 0;
}


bool
GNEAttributeProperties::isPositive() const {
    return (myAttributeProperty & POSITIVE) != 0;
}


bool
GNEAttributeProperties::isColor() const {
    return (myAttributeProperty & COLOR) != 0;
}


bool
GNEAttributeProperties::isFilename() const {
    return (myAttributeProperty & FILENAME) != 0;
}


bool
GNEAttributeProperties::isVClass() const {
    return (myAttributeProperty & VCLASS) != 0;
}


bool
GNEAttributeProperties::isSVCPermission() const {
    return ((myAttributeProperty & LIST) != 0) && ((myAttributeProperty & VCLASS) != 0);
}


bool
GNEAttributeProperties::isList() const {
    return (myAttributeProperty & LIST) != 0;
}


bool
GNEAttributeProperties::isSecuential() const {
    return (myAttributeProperty & SECUENCIAL) != 0;
}


bool
GNEAttributeProperties::isUnique() const {
    return (myAttributeProperty & UNIQUE) != 0;
}


bool
GNEAttributeProperties::isOptional() const {
    return (myAttributeProperty & XMLOPTIONAL) != 0;
}

bool
GNEAttributeProperties::isDiscrete() const {
    return (myAttributeProperty & DISCRETE) != 0;
}


bool
GNEAttributeProperties::isVClasses() const {
    return (myAttributeProperty & VCLASSES) != 0;
}


bool
GNEAttributeProperties::isExtended() const {
    return (myAttributeProperty & EXTENDED) != 0;
}


bool
GNEAttributeProperties::requireUpdateGeometry() const {
    return (myAttributeProperty & UPDATEGEOMETRY) != 0;
}


bool
GNEAttributeProperties::isActivatable() const {
    return (myAttributeProperty & ACTIVATABLE) != 0;
}


bool
GNEAttributeProperties::isComplex() const {
    return (myAttributeProperty & COMPLEX) != 0;
}


bool
GNEAttributeProperties::isFlowDefinition() const {
    return (myAttributeProperty & FLOWDEFINITION) != 0;
}

/****************************************************************************/
