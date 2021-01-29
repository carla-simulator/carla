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
/// @file    GNEChange_DemandElement.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2019
///
// A network change in which a busStop is created or deleted
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/elements/network/GNEJunction.h>
#include <netedit/elements/network/GNEEdge.h>
#include <netedit/elements/demand/GNEDemandElement.h>
#include <netedit/frames/demand/GNEVehicleTypeFrame.h>
#include <netedit/GNEViewParent.h>
#include <netedit/GNEViewNet.h>

#include "GNEChange_DemandElement.h"

// ===========================================================================
// FOX-declarations
// ===========================================================================
FXIMPLEMENT_ABSTRACT(GNEChange_DemandElement, GNEChange, nullptr, 0)

// ===========================================================================
// member method definitions
// ===========================================================================

GNEChange_DemandElement::GNEChange_DemandElement(GNEDemandElement* demandElement, bool forward) :
    GNEChange(demandElement, forward, demandElement->isAttributeCarrierSelected()),
    myDemandElement(demandElement),
    myPath(demandElement->getPath()) {
    myDemandElement->incRef("GNEChange_DemandElement");
}


GNEChange_DemandElement::~GNEChange_DemandElement() {
    myDemandElement->decRef("GNEChange_DemandElement");
    if (myDemandElement->unreferenced()) {
        // show extra information for tests
        WRITE_DEBUG("Deleting unreferenced " + myDemandElement->getTagStr() + " '" + myDemandElement->getID() + "'");
        // make sure that element isn't in net before removing
        if (myDemandElement->getNet()->getAttributeCarriers()->demandElementExist(myDemandElement)) {
            // remove demand element of network
            myDemandElement->getNet()->getAttributeCarriers()->deleteDemandElement(myDemandElement);
            // remove element from path
            for (const auto& pathElement : myPath) {
                pathElement.getLane()->removePathDemandElement(myDemandElement);
                if (pathElement.getJunction()) {
                    pathElement.getJunction()->removePathDemandElement(myDemandElement);
                }
            }
        }
        delete myDemandElement;
    }
}


void
GNEChange_DemandElement::undo() {
    if (myForward) {
        // show extra information for tests
        WRITE_DEBUG("Removing " + myDemandElement->getTagStr() + " '" + myDemandElement->getID() + "' in GNEChange_DemandElement");
        // unselect if mySelectedElement is enabled
        if (mySelectedElement) {
            myDemandElement->unselectAttributeCarrier();
        }
        // delete demand element from net
        myDemandElement->getNet()->getAttributeCarriers()->deleteDemandElement(myDemandElement);
        // remove element from path
        for (const auto& pathElement : myPath) {
            pathElement.getLane()->removePathDemandElement(myDemandElement);
            if (pathElement.getJunction()) {
                pathElement.getJunction()->removePathDemandElement(myDemandElement);
            }
        }
        // restore container
        restoreHierarchicalContainers();
    } else {
        // show extra information for tests
        WRITE_DEBUG("Adding " + myDemandElement->getTagStr() + " '" + myDemandElement->getID() + "' in GNEChange_DemandElement");
        // select if mySelectedElement is enabled
        if (mySelectedElement) {
            myDemandElement->selectAttributeCarrier();
        }
        // insert demand element into net
        myDemandElement->getNet()->getAttributeCarriers()->insertDemandElement(myDemandElement);
        // compute demand element path
        myDemandElement->computePath();
        // restore container
        restoreHierarchicalContainers();
    }
    // update vehicle type selector if demand element is a VType and vehicle type Frame is shown
    if ((myDemandElement->getTagProperty().getTag() == SUMO_TAG_VTYPE) && myDemandElement->getNet()->getViewNet()->getViewParent()->getVehicleTypeFrame()->shown()) {
        myDemandElement->getNet()->getViewNet()->getViewParent()->getVehicleTypeFrame()->getVehicleTypeSelector()->refreshVehicleTypeSelector();
    }
    // update stack labels
    if (myOriginalHierarchicalContainer.getParents<std::vector<GNEEdge*> >().size() > 0) {
        myOriginalHierarchicalContainer.getParents<std::vector<GNEEdge*> >().front()->updateVehicleStackLabels();
    }
    // Requiere always save elements
    myDemandElement->getNet()->requireSaveDemandElements(true);
}


void
GNEChange_DemandElement::redo() {
    if (myForward) {
        // show extra information for tests
        WRITE_DEBUG("Adding " + myDemandElement->getTagStr() + " '" + myDemandElement->getID() + "' in GNEChange_DemandElement");
        // select if mySelectedElement is enabled
        if (mySelectedElement) {
            myDemandElement->selectAttributeCarrier();
        }
        // insert demand element into net
        myDemandElement->getNet()->getAttributeCarriers()->insertDemandElement(myDemandElement);
        // compute demand element path
        myDemandElement->computePath();
        // add demand element in parents and children
        addElementInParentsAndChildren(myDemandElement);
    } else {
        // show extra information for tests
        WRITE_DEBUG("Removing " + myDemandElement->getTagStr() + " '" + myDemandElement->getID() + "' in GNEChange_DemandElement");
        // unselect if mySelectedElement is enabled
        if (mySelectedElement) {
            myDemandElement->unselectAttributeCarrier();
        }
        // delete demand element from net
        myDemandElement->getNet()->getAttributeCarriers()->deleteDemandElement(myDemandElement);
        // remove element from path
        for (const auto& pathElement : myPath) {
            pathElement.getLane()->removePathDemandElement(myDemandElement);
            if (pathElement.getJunction()) {
                pathElement.getJunction()->removePathDemandElement(myDemandElement);
            }
        }
        // remove demand element from parents and children
        removeElementFromParentsAndChildren(myDemandElement);
    }
    // update vehicle type selector if demand element is a VType and vehicle type Frame is shown
    if ((myDemandElement->getTagProperty().getTag() == SUMO_TAG_VTYPE) && myDemandElement->getNet()->getViewNet()->getViewParent()->getVehicleTypeFrame()->shown()) {
        myDemandElement->getNet()->getViewNet()->getViewParent()->getVehicleTypeFrame()->getVehicleTypeSelector()->refreshVehicleTypeSelector();
    }
    // update stack labels
    if (myOriginalHierarchicalContainer.getParents<std::vector<GNEEdge*> >().size() > 0) {
        myOriginalHierarchicalContainer.getParents<std::vector<GNEEdge*> >().front()->updateVehicleStackLabels();
    }
    // Requiere always save elements
    myDemandElement->getNet()->requireSaveDemandElements(true);
}


FXString
GNEChange_DemandElement::undoName() const {
    if (myForward) {
        return ("Undo create " + myDemandElement->getTagStr()).c_str();
    } else {
        return ("Undo delete " + myDemandElement->getTagStr()).c_str();
    }
}


FXString
GNEChange_DemandElement::redoName() const {
    if (myForward) {
        return ("Redo create " + myDemandElement->getTagStr()).c_str();
    } else {
        return ("Redo delete " + myDemandElement->getTagStr()).c_str();
    }
}
