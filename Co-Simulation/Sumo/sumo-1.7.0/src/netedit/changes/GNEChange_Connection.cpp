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
/// @file    GNEChange_Connection.cpp
/// @author  Jakob Erdmann
/// @date    May 2011
///
// A network change in which a single connection is created or deleted
/****************************************************************************/
#include <config.h>

#include <netedit/elements/network/GNEEdge.h>
#include <netedit/GNENet.h>

#include "GNEChange_Connection.h"

// ===========================================================================
// FOX-declarations
// ===========================================================================
FXIMPLEMENT_ABSTRACT(GNEChange_Connection, GNEChange, nullptr, 0)

// ===========================================================================
// member method definitions
// ===========================================================================


GNEChange_Connection::GNEChange_Connection(GNEEdge* edge, NBEdge::Connection nbCon, bool selected, bool forward) :
    GNEChange(forward, selected),
    myEdge(edge),
    myNBEdgeConnection(nbCon) {
}


GNEChange_Connection::~GNEChange_Connection() {
}


void
GNEChange_Connection::undo() {
    if (myForward) {
        // show extra information for tests
        WRITE_DEBUG("Removing " + toString(SUMO_TAG_CONNECTION) + " '" +
                    myEdge->getNBEdge()->getLaneID(myNBEdgeConnection.fromLane) + "->" + myNBEdgeConnection.toEdge->getLaneID(myNBEdgeConnection.toLane) + "' from " +
                    toString(SUMO_TAG_EDGE) + " '" + myEdge->getID() + "'");
        // remove connection from edge
        myEdge->removeConnection(myNBEdgeConnection);
    } else {
        // show extra information for tests
        std::string selected = mySelectedElement ? ("a previously selected ") : ("");
        WRITE_DEBUG("Adding " + selected + toString(SUMO_TAG_CONNECTION) + " '" +
                    myEdge->getNBEdge()->getLaneID(myNBEdgeConnection.fromLane) + "->" + myNBEdgeConnection.toEdge->getLaneID(myNBEdgeConnection.toLane) + "' into " +
                    toString(SUMO_TAG_EDGE) + " '" + myEdge->getID() + "'");
        // add connection into edge
        myEdge->addConnection(myNBEdgeConnection, mySelectedElement);
    }
    // enable save networkElements
    myEdge->getNet()->requireSaveNet(true);
}


void
GNEChange_Connection::redo() {
    if (myForward) {
        // show extra information for tests
        std::string selected = mySelectedElement ? ("a previously selected ") : ("");
        WRITE_DEBUG("Adding " + selected + toString(SUMO_TAG_CONNECTION) + " '" +
                    myEdge->getNBEdge()->getLaneID(myNBEdgeConnection.fromLane) + "->" + myNBEdgeConnection.toEdge->getLaneID(myNBEdgeConnection.toLane) + "' into " +
                    toString(SUMO_TAG_EDGE) + " '" + myEdge->getID() + "'");
        // add connection into edge
        myEdge->addConnection(myNBEdgeConnection, mySelectedElement);
    } else {
        // show extra information for tests
        WRITE_DEBUG("Removing " + toString(SUMO_TAG_CONNECTION) + " '" +
                    myEdge->getNBEdge()->getLaneID(myNBEdgeConnection.fromLane) + "->" + myNBEdgeConnection.toEdge->getLaneID(myNBEdgeConnection.toLane) + "' from " +
                    toString(SUMO_TAG_EDGE) + " '" + myEdge->getID() + "'");
        // remove connection from edge
        myEdge->removeConnection(myNBEdgeConnection);
    }
    // enable save networkElements
    myEdge->getNet()->requireSaveNet(true);
}


FXString
GNEChange_Connection::undoName() const {
    if (myForward) {
        return ("Undo create " + toString(SUMO_TAG_CONNECTION)).c_str();
    } else {
        return ("Undo delete " + toString(SUMO_TAG_CONNECTION)).c_str();
    }
}


FXString
GNEChange_Connection::redoName() const {
    if (myForward) {
        return ("Redo create " + toString(SUMO_TAG_CONNECTION)).c_str();
    } else {
        return ("Redo delete " + toString(SUMO_TAG_CONNECTION)).c_str();
    }
}
