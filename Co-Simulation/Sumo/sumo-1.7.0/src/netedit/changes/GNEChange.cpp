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
/// @file    GNEChange.cpp
/// @author  Jakob Erdmann
/// @date    Mar 2011
///
// The reification of a NETEDIT editing operation (see command pattern)
// inherits from FXCommand and is used to for undo/redo
/****************************************************************************/

#include "GNEChange.h"

// ===========================================================================
// FOX-declarations
// ===========================================================================

FXIMPLEMENT_ABSTRACT(GNEChange, FXCommand, nullptr, 0)

// ===========================================================================
// member method definitions
// ===========================================================================

GNEChange::GNEChange(bool forward, const bool selectedElement) :
    myForward(forward),
    mySelectedElement(selectedElement) {}


GNEChange::GNEChange(GNEHierarchicalElement* hierarchicalElement, bool forward, const bool selectedElement) :
    myForward(forward),
    mySelectedElement(selectedElement),
    myOriginalHierarchicalContainer(hierarchicalElement->getHierarchicalContainer()) {
    // get all hierarchical elements (Parents and children)
    const auto hierarchicalElements = hierarchicalElement->getAllHierarchicalElements();
    // save all hierarchical containers
    for (const auto& element : hierarchicalElements) {
        myHierarchicalContainers[element] = element->getHierarchicalContainer();
    }
}


GNEChange::~GNEChange() {}


FXuint
GNEChange::size() const {
    return 1;
}


FXString
GNEChange::undoName() const {
    return "Undo";
}


FXString
GNEChange::redoName() const {
    return "Redo";
}


void
GNEChange::undo() {}


void
GNEChange::redo() {}


void
GNEChange::restoreHierarchicalContainers() {
    // iterate over all parents and children container and restore it
    for (const auto& container : myHierarchicalContainers) {
        container.first->restoreHierarchicalContainer(container.second);
    }
}

/****************************************************************************/
