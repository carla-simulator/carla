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
/// @file    GNEChange_Attribute.cpp
/// @author  Jakob Erdmann
/// @date    Mar 2011
///
// A network change in which something is changed (for undo/redo)
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/elements/network/GNENetworkElement.h>
#include <netedit/elements/data/GNEDataSet.h>

#include "GNEChange_Attribute.h"

// ===========================================================================
// FOX-declarations
// ===========================================================================
FXIMPLEMENT_ABSTRACT(GNEChange_Attribute, GNEChange, nullptr, 0)

// ===========================================================================
// member method definitions
// ===========================================================================

GNEChange_Attribute::GNEChange_Attribute(GNEAttributeCarrier* ac, SumoXMLAttr key, const std::string& value) :
    GNEChange(true, false),
    myAC(ac),
    myKey(key),
    myForceChange(false),
    myOrigValue(ac->getAttribute(key)),
    myNewValue(value) {
    myAC->incRef("GNEChange_Attribute " + toString(myKey));
}


GNEChange_Attribute::GNEChange_Attribute(GNEAttributeCarrier* ac, SumoXMLAttr key, const std::string& value, const std::string& origValue) :
    GNEChange(true, false),
    myAC(ac),
    myKey(key),
    myForceChange(false),
    myOrigValue(origValue),
    myNewValue(value) {
    myAC->incRef("GNEChange_Attribute " + toString(myKey));
}


GNEChange_Attribute::~GNEChange_Attribute() {
    // decrease reference
    myAC->decRef("GNEChange_Attribute " + toString(myKey));
    // remove if is unreferenced
    if (myAC->unreferenced()) {
        // show extra information for tests
        WRITE_DEBUG("Deleting unreferenced " + myAC->getTagStr() + " '" + myAC->getID() + "' in GNEChange_Attribute");
        // delete AC
        delete myAC;
    }
}


void
GNEChange_Attribute::undo() {
    // show extra information for tests
    WRITE_DEBUG("Setting previous attribute " + toString(myKey) + " '" + myOrigValue + "' into " + myAC->getTagStr() + " '" + myAC->getID() + "'");
    // set original value
    myAC->setAttribute(myKey, myOrigValue);
    // certain attributes needs extra operations
    if (myKey != GNE_ATTR_SELECTED) {
        // check if updated attribute requires a update geometry
        if (myAC->getTagProperty().hasAttribute(myKey) && myAC->getTagProperty().getAttributeProperties(myKey).requireUpdateGeometry()) {
            myAC->updateGeometry();
        }
        // if is a dataelement, update attribute colors
        if (myAC->getTagProperty().isDataElement()) {
            if (myAC->getTagProperty().isGenericData()) {
                myAC->getNet()->retrieveDataSet(myAC->getAttribute(GNE_ATTR_DATASET))->updateAttributeColors();
            } else {
                myAC->getNet()->retrieveDataSet(myAC->getAttribute(SUMO_ATTR_ID))->updateAttributeColors();
            }
        }
        // check if networkElements, additional or shapes has to be saved (only if key isn't GNE_ATTR_SELECTED)
        if (myAC->getTagProperty().isNetworkElement()) {
            myAC->getNet()->requireSaveNet(true);
        } else if (myAC->getTagProperty().isAdditionalElement() || myAC->getTagProperty().isShape() || myAC->getTagProperty().isTAZElement()) {
            myAC->getNet()->requireSaveAdditionals(true);
        } else if (myAC->getTagProperty().isDemandElement()) {
            myAC->getNet()->requireSaveDemandElements(true);
        } else if (myAC->getTagProperty().isDemandElement()) {
            myAC->getNet()->requireSaveDataElements(true);
        }
    }
}


void
GNEChange_Attribute::redo() {
    // show extra information for tests
    WRITE_DEBUG("Setting new attribute " + toString(myKey) + " '" + myNewValue + "' into " + myAC->getTagStr() + " '" + myAC->getID() + "'");
    // set new value
    myAC->setAttribute(myKey, myNewValue);
    // certain attributes needs extra operations
    if (myKey != GNE_ATTR_SELECTED) {
        // check if updated attribute requires a update geometry
        if (myAC->getTagProperty().hasAttribute(myKey) && myAC->getTagProperty().getAttributeProperties(myKey).requireUpdateGeometry()) {
            myAC->updateGeometry();
        }
        // if is a dataelement, update attribute colors
        if (myAC->getTagProperty().isDataElement()) {
            if (myAC->getTagProperty().isGenericData()) {
                myAC->getNet()->retrieveDataSet(myAC->getAttribute(GNE_ATTR_DATASET))->updateAttributeColors();
            } else {
                myAC->getNet()->retrieveDataSet(myAC->getAttribute(SUMO_ATTR_ID))->updateAttributeColors();
            }
        }
        // check if networkElements, additional or shapes has to be saved (only if key isn't GNE_ATTR_SELECTED)
        if (myAC->getTagProperty().isNetworkElement()) {
            myAC->getNet()->requireSaveNet(true);
        } else if (myAC->getTagProperty().isAdditionalElement() || myAC->getTagProperty().isShape() || myAC->getTagProperty().isTAZElement()) {
            myAC->getNet()->requireSaveAdditionals(true);
        } else if (myAC->getTagProperty().isDemandElement()) {
            myAC->getNet()->requireSaveDemandElements(true);
        } else if (myAC->getTagProperty().isDemandElement()) {
            myAC->getNet()->requireSaveDataElements(true);
        }
    }
}


void
GNEChange_Attribute::forceChange() {
    myForceChange = true;
}


bool
GNEChange_Attribute::trueChange() {
    // check if we're editing the value of an attribute or changing a disjoint attribute
    if (myForceChange) {
        return true;
    } else {
        return (myOrigValue != myNewValue);
    }
}


FXString
GNEChange_Attribute::undoName() const {
    return ("Undo change " + myAC->getTagStr() + " attribute").c_str();
}


FXString
GNEChange_Attribute::redoName() const {
    return ("Redo change " + myAC->getTagStr() + " attribute").c_str();
}

/****************************************************************************/
