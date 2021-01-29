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
/// @file    GNEChange_TAZElement.cpp
/// @author  Pablo Alvarez Lopez
/// @date    April 2020
///
// A network change in which a single TAZElement is created or deleted
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/elements/additional/GNETAZElement.h>

#include "GNEChange_TAZElement.h"

// ===========================================================================
// FOX-declarations
// ===========================================================================
FXIMPLEMENT_ABSTRACT(GNEChange_TAZElement, GNEChange, nullptr, 0)

// ===========================================================================
// member method definitions
// ===========================================================================

GNEChange_TAZElement::GNEChange_TAZElement(GNETAZElement* TAZElement, bool forward) :
    GNEChange(TAZElement, forward, TAZElement->isAttributeCarrierSelected()),
    myTAZElement(TAZElement) {
    myTAZElement->incRef("GNEChange_TAZElement");
}


GNEChange_TAZElement::~GNEChange_TAZElement() {
    myTAZElement->decRef("GNEChange_TAZElement");
    if (myTAZElement->unreferenced()) {
        // make sure that TAZElement are removed of TAZElementContainer (net) AND grid
        if (myTAZElement->getNet()->retrieveTAZElement(myTAZElement->getTagProperty().getTag(), myTAZElement->getID(), false) != nullptr) {
            // show extra information for tests
            WRITE_DEBUG("Removing " + myTAZElement->getTagStr() + " '" + myTAZElement->getID() + "' from net in ~GNEChange_TAZElement()");
            // remove TAZElement from AttributeCarreirs
            myTAZElement->getNet()->getAttributeCarriers()->deleteTAZElement(myTAZElement);
        }
        // show extra information for tests
        WRITE_DEBUG("delete " + myTAZElement->getTagStr() + " '" + myTAZElement->getID() + "' in ~GNEChange_TAZElement()");
        delete myTAZElement;
    }
}


void
GNEChange_TAZElement::undo() {
    if (myForward) {
        // show extra information for tests
        WRITE_DEBUG("Removing " + myTAZElement->getTagStr() + " '" + myTAZElement->getID() + "' from viewNet");
        // unselect if mySelectedElement is enabled
        if (mySelectedElement) {
            myTAZElement->unselectAttributeCarrier();
        }
        // remove TAZElement from net
        myTAZElement->getNet()->getAttributeCarriers()->deleteTAZElement(myTAZElement);
        // restore container
        restoreHierarchicalContainers();
    } else {
        // show extra information for tests
        WRITE_DEBUG("Adding " + myTAZElement->getTagStr() + " '" + myTAZElement->getID() + "' into viewNet");
        // select if mySelectedElement is enabled
        if (mySelectedElement) {
            myTAZElement->selectAttributeCarrier();
        }
        // Add TAZElement in net
        myTAZElement->getNet()->getAttributeCarriers()->insertTAZElement(myTAZElement);
        // restore container
        restoreHierarchicalContainers();
    }
}


void
GNEChange_TAZElement::redo() {
    if (myForward) {
        // show extra information for tests
        WRITE_DEBUG("Adding " + myTAZElement->getTagStr() + " '" + myTAZElement->getID() + "' into viewNet");
        // select if mySelectedElement is enabled
        if (mySelectedElement) {
            myTAZElement->selectAttributeCarrier();
        }
        // Add TAZElement in net
        myTAZElement->getNet()->getAttributeCarriers()->insertTAZElement(myTAZElement);
        // Add element in parents and children
        addElementInParentsAndChildren(myTAZElement);
    } else {
        // show extra information for tests
        WRITE_DEBUG("Removing " + myTAZElement->getTagStr() + " '" + myTAZElement->getID() + "' from viewNet");
        // unselect if mySelectedElement is enabled
        if (mySelectedElement) {
            myTAZElement->unselectAttributeCarrier();
        }
        // remove TAZElement from net
        myTAZElement->getNet()->getAttributeCarriers()->deleteTAZElement(myTAZElement);
        // Remove element from parents and children
        removeElementFromParentsAndChildren(myTAZElement);
    }
}


FXString
GNEChange_TAZElement::undoName() const {
    if (myForward) {
        return ("Undo create " + myTAZElement->getTagStr()).c_str();
    } else {
        return ("Undo delete " + myTAZElement->getTagStr()).c_str();
    }
}


FXString
GNEChange_TAZElement::redoName() const {
    if (myForward) {
        return ("Redo create " + myTAZElement->getTagStr()).c_str();
    } else {
        return ("Redo delete " + myTAZElement->getTagStr()).c_str();
    }
}
