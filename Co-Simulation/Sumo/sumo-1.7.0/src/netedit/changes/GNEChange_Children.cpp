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
/// @file    GNEChange_Children.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jul 2019
///
// A network change used to modify sorting of hierarchical element children
/****************************************************************************/
#include <config.h>

#include <netedit/elements/demand/GNEDemandElement.h>
#include <netedit/GNENet.h>

#include "GNEChange_Children.h"

// ===========================================================================
// FOX-declarations
// ===========================================================================
FXIMPLEMENT_ABSTRACT(GNEChange_Children, GNEChange, nullptr, 0)

// ===========================================================================
// member method definitions
// ===========================================================================

GNEChange_Children::GNEChange_Children(GNEDemandElement* demandElementParent, GNEDemandElement* demandElementChild, const Operation operation) :
    GNEChange(true, demandElementChild->isAttributeCarrierSelected()),
    myParentDemandElement(demandElementParent),
    myChildDemandElement(demandElementChild),
    myOperation(operation),
    myOriginalChildElements(demandElementParent->getChildDemandElements()),
    myEditedChildElements(demandElementParent->getChildDemandElements()) {
    myParentDemandElement->incRef("GNEChange_Children");
    // obtain iterator to demandElementChild
    auto it = std::find(myEditedChildElements.begin(), myEditedChildElements.end(), myChildDemandElement);
    // edit myEditedChildElements vector
    if (it != myEditedChildElements.end()) {
        if ((operation == Operation::MOVE_FRONT) && (it != (myEditedChildElements.end() - 1))) {
            // remove element
            it = myEditedChildElements.erase(it);
            // insert again in a different position
            myEditedChildElements.insert(it + 1, myChildDemandElement);
        } else if ((operation == Operation::MOVE_BACK) && (it != myEditedChildElements.begin())) {
            // remove element
            it = myEditedChildElements.erase(it);
            // insert again in a different position
            myEditedChildElements.insert(it - 1, myChildDemandElement);
        }
    }
}


GNEChange_Children::~GNEChange_Children() {
    myParentDemandElement->decRef("GNEChange_Children");
    // remove if is unreferenced
    if (myParentDemandElement->unreferenced()) {
        // show extra information for tests
        WRITE_DEBUG("Deleting unreferenced " + myParentDemandElement->getTagStr() + " '" + myParentDemandElement->getID() + "' in GNEChange_Children");
        // delete AC
        delete myParentDemandElement;
    }
}


void
GNEChange_Children::undo() {
    if (myForward) {
        // continue depending of myOperation
        if (myOperation == GNEChange_Children::Operation::MOVE_FRONT) {
            // show extra information for tests
            WRITE_DEBUG("Moving front " + myChildDemandElement->getTagStr() + " within demandElement parent '" + myParentDemandElement->getID() + "' in GNEChange_Children");
            // restore child demand element original vector in myChildDemandElement
//          myParentDemandElement->myChildDemandElements = myOriginalChildElements;
        } else if (myOperation == GNEChange_Children::Operation::MOVE_BACK) {
            // show extra information for tests
            WRITE_DEBUG("Moving back " + myChildDemandElement->getTagStr() + " within demandElement parent '" + myParentDemandElement->getID() + "' in GNEChange_Children");
            // restore child demand element original vector in myChildDemandElement
//          myParentDemandElement->myChildDemandElements = myOriginalChildElements;
        }
    } else {
        // continue depending of myOperation
        if (myOperation == GNEChange_Children::Operation::MOVE_FRONT) {
            // show extra information for tests
            WRITE_DEBUG("Moving front " + myChildDemandElement->getTagStr() + " within demandElement parent '" + myParentDemandElement->getID() + "' in GNEChange_Children");
            // set child demand element edited vector in myChildDemandElement
//          myParentDemandElement- myChildDemandElements = myEditedChildElements;
        } else if (myOperation == GNEChange_Children::Operation::MOVE_BACK) {
            // show extra information for tests
            WRITE_DEBUG("Moving back " + myChildDemandElement->getTagStr() + " within demandElement parent '" + myParentDemandElement->getID() + "' in GNEChange_Children");
            // set child demand element edited vector in myChildDemandElement
//          myParentDemandElement->myChildDemandElements = myEditedChildElements;
        }
    }
    // Requiere always save children
    myParentDemandElement->getNet()->requireSaveDemandElements(true);
}


void
GNEChange_Children::redo() {
    if (myForward) {
        // continue depending of myOperation
        if (myOperation == GNEChange_Children::Operation::MOVE_FRONT) {
            // show extra information for tests
            WRITE_DEBUG("Moving front " + myChildDemandElement->getTagStr() + " within demandElement parent '" + myParentDemandElement->getID() + "' in GNEChange_Children");
            // set child demand element edited vector in myChildDemandElement
//          myParentDemandElement->myChildDemandElements = myEditedChildElements;
        } else if (myOperation == GNEChange_Children::Operation::MOVE_BACK) {
            // show extra information for tests
            WRITE_DEBUG("Moving back " + myChildDemandElement->getTagStr() + " within demandElement parent '" + myParentDemandElement->getID() + "' in GNEChange_Children");
            // set child demand element edited vector in myChildDemandElement
//          myParentDemandElement->myChildDemandElements = myEditedChildElements;
        }
    } else {
        // continue depending of myOperation
        if (myOperation == GNEChange_Children::Operation::MOVE_FRONT) {
            // show extra information for tests
            WRITE_DEBUG("Moving front " + myChildDemandElement->getTagStr() + " within demandElement parent '" + myParentDemandElement->getID() + "' in GNEChange_Children");
            // restore child demand element original vector in myChildDemandElement
//          myParentDemandElement->myChildDemandElements = myOriginalChildElements;
        } else if (myOperation == GNEChange_Children::Operation::MOVE_BACK) {
            // show extra information for tests
            WRITE_DEBUG("Moving back " + myChildDemandElement->getTagStr() + " within demandElement parent '" + myParentDemandElement->getID() + "' in GNEChange_Children");
            // restore child demand element original vector in myChildDemandElement
//          myParentDemandElement->myChildDemandElements = myOriginalChildElements;
        }
    }
    // Requiere always save children
    myParentDemandElement->getNet()->requireSaveDemandElements(true);
}


FXString
GNEChange_Children::undoName() const {
    if (myForward) {
        // check myOperation
        if (myOperation == GNEChange_Children::Operation::MOVE_FRONT) {
            return ("Undo moving up " + myChildDemandElement->getTagStr()).c_str();
        } else if (myOperation == GNEChange_Children::Operation::MOVE_BACK) {
            return ("Undo moving down " + myChildDemandElement->getTagStr()).c_str();
        } else {
            return ("Invalid operation");
        }
    } else {
        // check myOperation
        if (myOperation == GNEChange_Children::Operation::MOVE_FRONT) {
            return ("Undo moving down " + myChildDemandElement->getTagStr()).c_str();
        } else if (myOperation == GNEChange_Children::Operation::MOVE_BACK) {
            return ("Undo moving up " + myChildDemandElement->getTagStr()).c_str();
        } else {
            return ("Invalid operation");
        }
    }
}


FXString
GNEChange_Children::redoName() const {
    if (myForward) {
        // check myOperation
        if (myOperation == GNEChange_Children::Operation::MOVE_FRONT) {
            return ("Redo moving front " + myParentDemandElement->getTagStr()).c_str();
        } else if (myOperation == GNEChange_Children::Operation::MOVE_BACK) {
            return ("Redo moving back " + myParentDemandElement->getTagStr()).c_str();
        } else {
            return ("Invalid operation");
        }
    } else {
        // check myOperation
        if (myOperation == GNEChange_Children::Operation::MOVE_FRONT) {
            return ("Redo moving front " + myParentDemandElement->getTagStr()).c_str();
        } else if (myOperation == GNEChange_Children::Operation::MOVE_BACK) {
            return ("Redo moving back " + myParentDemandElement->getTagStr()).c_str();
        } else {
            return ("Invalid operation");
        }
    }
}
