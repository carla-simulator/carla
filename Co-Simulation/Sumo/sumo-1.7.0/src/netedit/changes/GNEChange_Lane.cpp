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
/// @file    GNEChange_Lane.cpp
/// @author  Jakob Erdmann
/// @date    April 2011
///
// A network change in which a single lane is created or deleted
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/elements/network/GNEEdge.h>
#include <netedit/elements/network/GNELane.h>

#include "GNEChange_Lane.h"


// ===========================================================================
// FOX-declarations
// ===========================================================================
FXIMPLEMENT_ABSTRACT(GNEChange_Lane, GNEChange, nullptr, 0)


// ===========================================================================
// member method definitions
// ===========================================================================

GNEChange_Lane::GNEChange_Lane(GNEEdge* edge, const NBEdge::Lane& laneAttrs):
    GNEChange(true, false),
    myEdge(edge),
    myLane(nullptr),
    myLaneAttrs(laneAttrs),
    myRecomputeConnections(true) {
    myEdge->incRef("GNEChange_Lane");
}


GNEChange_Lane::GNEChange_Lane(GNEEdge* edge, GNELane* lane, const NBEdge::Lane& laneAttrs, bool forward, bool recomputeConnections):
    GNEChange(lane, forward, lane->isAttributeCarrierSelected()),
    myEdge(edge),
    myLane(lane),
    myLaneAttrs(laneAttrs),
    myRecomputeConnections(recomputeConnections) {
    // include both references (To edge and lane)
    myEdge->incRef("GNEChange_Lane");
    myLane->incRef("GNEChange_Lane");
}


GNEChange_Lane::~GNEChange_Lane() {
    assert(myEdge);
    myEdge->decRef("GNEChange_Lane");
    if (myEdge->unreferenced()) {
        // show extra information for tests
        WRITE_DEBUG("Deleting unreferenced " + myEdge->getTagStr() + " '" + myEdge->getID() + "' in GNEChange_Lane");
        delete myEdge;
    }
    if (myLane) {
        myLane->decRef("GNEChange_Lane");
        if (myLane->unreferenced()) {
            // show extra information for tests
            WRITE_DEBUG("Deleting unreferenced " + myLane->getTagStr() + " '" + myLane->getID() + "' in GNEChange_Lane");
            // delete lane
            delete myLane;
        }
    }
}


void
GNEChange_Lane::undo() {
    if (myForward) {
        // show extra information for tests
        if (myLane != nullptr) {
            WRITE_DEBUG("Removing " + myLane->getTagStr() + " '" + myLane->getID() + "' from " + toString(SUMO_TAG_EDGE));
            // unselect if mySelectedElement is enabled
            if (mySelectedElement) {
                myLane->unselectAttributeCarrier();
            }
            // restore container
            restoreHierarchicalContainers();
        } else {
            WRITE_DEBUG("Removing nullptr " + toString(SUMO_TAG_LANE) + " from " + toString(SUMO_TAG_EDGE));
        }
        // remove lane from edge (note: myLane can be nullptr)
        myEdge->removeLane(myLane, false);
    } else {
        // show extra information for tests
        if (myLane != nullptr) {
            WRITE_DEBUG("Adding " + myLane->getTagStr() + " '" + myLane->getID() + "' into " + toString(SUMO_TAG_EDGE));
            // select if mySelectedElement is enabled
            if (mySelectedElement) {
                myLane->selectAttributeCarrier();
            }
            // restore container
            restoreHierarchicalContainers();
        } else {
            WRITE_DEBUG("Adding nullptr " + toString(SUMO_TAG_LANE) + " into " + toString(SUMO_TAG_EDGE));
        }
        // add lane and their attributes to edge (lane removal is reverted, no need to recompute connections)
        myEdge->addLane(myLane, myLaneAttrs, false);
    }
    // enable save networkElements
    myEdge->getNet()->requireSaveNet(true);
}


void
GNEChange_Lane::redo() {
    if (myForward) {
        // show extra information for tests
        if (myLane != nullptr) {
            WRITE_DEBUG("Adding " + myLane->getTagStr() + " '" + myLane->getID() + "' into " + toString(SUMO_TAG_EDGE));
            // select if mySelectedElement is enabled
            if (mySelectedElement) {
                myLane->selectAttributeCarrier();
            }
            // add lane into parents and children
            addElementInParentsAndChildren(myLane);
        } else {
            WRITE_DEBUG("Adding nullptr " + toString(SUMO_TAG_LANE) + " into " + toString(SUMO_TAG_EDGE));
        }
        // add lane and their attributes to edge
        myEdge->addLane(myLane, myLaneAttrs, myRecomputeConnections);
    } else {
        // show extra information for tests
        if (myLane != nullptr) {
            WRITE_DEBUG("Removing " + myLane->getTagStr() + " '" + myLane->getID() + "' from " + toString(SUMO_TAG_EDGE));
            // unselect if mySelectedElement is enabled
            if (mySelectedElement) {
                myLane->unselectAttributeCarrier();
            }
            // remove lane from parents and children
            removeElementFromParentsAndChildren(myLane);
        } else {
            WRITE_DEBUG("Removing nullptr " + toString(SUMO_TAG_LANE) + " from " + toString(SUMO_TAG_EDGE));
        }
        // remove lane from edge
        myEdge->removeLane(myLane, myRecomputeConnections);
    }
    // enable save networkElements
    myEdge->getNet()->requireSaveNet(true);
}


FXString
GNEChange_Lane::undoName() const {
    if (myForward) {
        return ("Undo create " + toString(SUMO_TAG_LANE)).c_str();
    } else {
        return ("Undo delete " + toString(SUMO_TAG_LANE)).c_str();
    }
}


FXString
GNEChange_Lane::redoName() const {
    if (myForward) {
        return ("Redo create " + toString(SUMO_TAG_LANE)).c_str();
    } else {
        return ("Redo delete " + toString(SUMO_TAG_LANE)).c_str();
    }
}
