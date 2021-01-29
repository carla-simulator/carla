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
/// @file    GNEDialogACChooser.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Apr 2018
///
// Class for the window that allows to choose a street, junction or vehicle
/****************************************************************************/
#include <config.h>

#include <utils/gui/globjects/GUIGlObjectStorage.h>
#include <netedit/GNENet.h>
#include <netedit/GNEViewParent.h>
#include <netedit/elements/network/GNEJunction.h>

#include "GNEDialogACChooser.h"


// ===========================================================================
// method definitions
// ===========================================================================

GNEDialogACChooser::GNEDialogACChooser(GNEViewParent* viewParent, FXIcon* icon, const std::string& title, const std::vector<GNEAttributeCarrier*>& ACs):
    GUIDialog_GLObjChooser(viewParent, icon, title.c_str(), std::vector<GUIGlID>(), GUIGlObjectStorage::gIDStorage),
    myACs(ACs),
    myViewParent(viewParent),
    myLocateTLS(title.find("TLS") != std::string::npos) {
    // @note refresh must be called here because the base class constructor cannot
    // call the virtual function getObjectName
    std::vector<GUIGlID> ids;
    for (auto ac : ACs) {
        ids.push_back(dynamic_cast<GUIGlObject*>(ac)->getGlID());
    }
    refreshList(ids);
}


GNEDialogACChooser::~GNEDialogACChooser() {
    myViewParent->eraseACChooserDialog(this);
}


void
GNEDialogACChooser::toggleSelection(int listIndex) {
    GNEAttributeCarrier* ac = myACs[listIndex];
    if (ac->isAttributeCarrierSelected()) {
        ac->unselectAttributeCarrier();
    } else {
        ac->selectAttributeCarrier();
    }
}


std::string
GNEDialogACChooser::getObjectName(GUIGlObject* o) const {
    if (myLocateTLS) {
        GNEJunction* junction = dynamic_cast<GNEJunction*>(o);
        assert(junction != nullptr);
        const std::set<NBTrafficLightDefinition*>& defs = junction->getNBNode()->getControllingTLS();
        assert(defs.size() > 0);
        NBTrafficLightDefinition* tlDef = *defs.begin();
        if (tlDef->getID() == o->getMicrosimID()) {
            return o->getMicrosimID();
        } else {
            return tlDef->getID() + " (" + o->getMicrosimID() + ")";
        }
    } else {
        return GUIDialog_GLObjChooser::getObjectName(o);
    }
}


/****************************************************************************/
