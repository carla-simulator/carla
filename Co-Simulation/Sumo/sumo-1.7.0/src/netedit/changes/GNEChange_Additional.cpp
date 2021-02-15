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
/// @file    GNEChange_Additional.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Nov 2015
///
// A network change in which a busStop is created or deleted
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/elements/network/GNEJunction.h>
#include <netedit/elements/additional/GNEAdditional.h>

#include "GNEChange_Additional.h"

// ===========================================================================
// FOX-declarations
// ===========================================================================
FXIMPLEMENT_ABSTRACT(GNEChange_Additional, GNEChange, nullptr, 0)

// ===========================================================================
// member method definitions
// ===========================================================================

GNEChange_Additional::GNEChange_Additional(GNEAdditional* additional, bool forward) :
    GNEChange(additional, forward, additional->isAttributeCarrierSelected()),
    myAdditional(additional),
    myPath(additional->getPath()) {
    myAdditional->incRef("GNEChange_Additional");
}


GNEChange_Additional::~GNEChange_Additional() {
    myAdditional->decRef("GNEChange_Additional");
    if (myAdditional->unreferenced()) {
        // show extra information for tests
        WRITE_DEBUG("Deleting unreferenced " + myAdditional->getTagStr() + " '" + myAdditional->getID() + "'");
        // make sure that additional isn't in net before removing
        if (myAdditional->getNet()->getAttributeCarriers()->additionalExist(myAdditional)) {
            // delete additional from net
            myAdditional->getNet()->getAttributeCarriers()->deleteAdditional(myAdditional);
            // remove element from path (used by E2 multilane detectors)
            for (const auto& pathElement : myPath) {
                pathElement.getLane()->removePathAdditionalElement(myAdditional);
                if (pathElement.getJunction()) {
                    pathElement.getJunction()->removePathAdditionalElement(myAdditional);
                }
            }
        }
        delete myAdditional;
    }
}


void
GNEChange_Additional::undo() {
    if (myForward) {
        // show extra information for tests
        WRITE_DEBUG("Removing " + myAdditional->getTagStr() + " '" + myAdditional->getID() + "' in GNEChange_Additional");
        // unselect if mySelectedElement is enabled
        if (mySelectedElement) {
            myAdditional->unselectAttributeCarrier();
        }
        // delete additional from net
        myAdditional->getNet()->getAttributeCarriers()->deleteAdditional(myAdditional);
        // remove element from path
        for (const auto& pathElement : myPath) {
            pathElement.getLane()->removePathAdditionalElement(myAdditional);
            if (pathElement.getJunction()) {
                pathElement.getJunction()->removePathAdditionalElement(myAdditional);
            }
        }
        // restore container
        restoreHierarchicalContainers();
    } else {
        // show extra information for tests
        WRITE_DEBUG("Adding " + myAdditional->getTagStr() + " '" + myAdditional->getID() + "' in GNEChange_Additional");
        // select if mySelectedElement is enabled
        if (mySelectedElement) {
            myAdditional->selectAttributeCarrier();
        }
        // insert additional into net
        myAdditional->getNet()->getAttributeCarriers()->insertAdditional(myAdditional);
        // restore container
        restoreHierarchicalContainers();
    }
    // Requiere always save additionals
    myAdditional->getNet()->requireSaveAdditionals(true);
}


void
GNEChange_Additional::redo() {
    if (myForward) {
        // show extra information for tests
        WRITE_DEBUG("Adding " + myAdditional->getTagStr() + " '" + myAdditional->getID() + "' in GNEChange_Additional");
        // select if mySelectedElement is enabled
        if (mySelectedElement) {
            myAdditional->selectAttributeCarrier();
        }
        // insert additional into net
        myAdditional->getNet()->getAttributeCarriers()->insertAdditional(myAdditional);
        // add additional in parent elements
        addElementInParentsAndChildren(myAdditional);
    } else {
        // show extra information for tests
        WRITE_DEBUG("Removing " + myAdditional->getTagStr() + " '" + myAdditional->getID() + "' in GNEChange_Additional");
        // unselect if mySelectedElement is enabled
        if (mySelectedElement) {
            myAdditional->unselectAttributeCarrier();
        }
        // delete additional from net
        myAdditional->getNet()->getAttributeCarriers()->deleteAdditional(myAdditional);
        // remove element from path
        for (const auto& pathElement : myPath) {
            pathElement.getLane()->removePathAdditionalElement(myAdditional);
            if (pathElement.getJunction()) {
                pathElement.getJunction()->removePathAdditionalElement(myAdditional);
            }
        }
        // remove additional from parents and children
        removeElementFromParentsAndChildren(myAdditional);
    }
    // Requiere always save additionals
    myAdditional->getNet()->requireSaveAdditionals(true);
}


FXString
GNEChange_Additional::undoName() const {
    if (myForward) {
        return ("Undo create " + myAdditional->getTagStr()).c_str();
    } else {
        return ("Undo delete " + myAdditional->getTagStr()).c_str();
    }
}


FXString
GNEChange_Additional::redoName() const {
    if (myForward) {
        return ("Redo create " + myAdditional->getTagStr()).c_str();
    } else {
        return ("Redo delete " + myAdditional->getTagStr()).c_str();
    }
}
