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
/// @file    GNEChange_TLS.cpp
/// @author  Jakob Erdmann
/// @date    July 2011
///
// A network change in which a traffic light is created or deleted
/****************************************************************************/
#include <config.h>


#include <utils/options/OptionsCont.h>
#include <netbuild/NBOwnTLDef.h>
#include <netedit/elements/network/GNEJunction.h>
#include <netedit/GNENet.h>

#include "GNEChange_TLS.h"


// ===========================================================================
// FOX-declarations
// ===========================================================================
FXIMPLEMENT_ABSTRACT(GNEChange_TLS, GNEChange, nullptr, 0)

// ===========================================================================
// member method definitions
// ===========================================================================


/// @brief constructor for creating an edge
GNEChange_TLS::GNEChange_TLS(GNEJunction* junction, NBTrafficLightDefinition* tlDef, bool forward, bool forceInsert, const std::string tlID):
    GNEChange(forward, false),
    myJunction(junction),
    myTlDef(tlDef),
    myForceInsert(forceInsert) {
    myJunction->incRef("GNEChange_TLS");
    if (myTlDef == nullptr) {
        assert(forward);
        // potential memory leak if this change is never executed
        TrafficLightType type = SUMOXMLDefinitions::TrafficLightTypes.get(OptionsCont::getOptions().getString("tls.default-type"));
        if (myJunction->getNBNode()->isTLControlled()) {
            // copy existing type
            type = (*myJunction->getNBNode()->getControllingTLS().begin())->getType();
        }
        myTlDef = new NBOwnTLDef(tlID == "" ? myJunction->getMicrosimID() : tlID, 0, type);
    }
}


GNEChange_TLS::~GNEChange_TLS() {
    myJunction->decRef("GNEChange_TLS");
    if (myJunction->unreferenced()) {
        // show extra information for tests
        WRITE_DEBUG("Deleting unreferenced " + myJunction->getTagStr() + " '" + myJunction->getID() + "' in GNEChange_TLS");
        delete myJunction;
    }
}


void
GNEChange_TLS::undo() {
    if (myForward) {
        // show extra information for tests
        WRITE_DEBUG("Removing TLS from " + myJunction->getTagStr() + " '" + myJunction->getID() + "'");
        // remove traffic light from junction
        myJunction->removeTrafficLight(myTlDef);
    } else {
        // show extra information for tests
        WRITE_DEBUG("Adding TLS into " + myJunction->getTagStr() + " '" + myJunction->getID() + "'");
        // add traffic light to junction
        myJunction->addTrafficLight(myTlDef, myForceInsert);
    }
    // enable save networkElements
    myJunction->getNet()->requireSaveNet(true);
}


void
GNEChange_TLS::redo() {
    if (myForward) {
        // show extra information for tests
        WRITE_DEBUG("Adding TLS into " + myJunction->getTagStr() + " '" + myJunction->getID() + "'");
        // add traffic light to junction
        myJunction->addTrafficLight(myTlDef, myForceInsert);
    } else {
        // show extra information for tests
        WRITE_DEBUG("Deleting TLS from " + myJunction->getTagStr() + " '" + myJunction->getID() + "'");
        // remove traffic light from junction
        myJunction->removeTrafficLight(myTlDef);
    }
    // enable save networkElements
    myJunction->getNet()->requireSaveNet(true);
}


FXString
GNEChange_TLS::undoName() const {
    if (myForward) {
        return ("Undo create " + toString(SUMO_TAG_TRAFFIC_LIGHT)).c_str();
    } else {
        return ("Undo delete " + toString(SUMO_TAG_TRAFFIC_LIGHT)).c_str();
    }
}


FXString
GNEChange_TLS::redoName() const {
    if (myForward) {
        return ("Redo create " + toString(SUMO_TAG_TRAFFIC_LIGHT)).c_str();
    } else {
        return ("Redo delete " + toString(SUMO_TAG_TRAFFIC_LIGHT)).c_str();
    }
}
