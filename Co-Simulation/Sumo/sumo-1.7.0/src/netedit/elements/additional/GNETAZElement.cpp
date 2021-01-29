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
/// @file    GNETAZElement.cpp
/// @author  Pablo Alvarez Lopez
/// @date    April 2020
///
// Abstract class for TAZElements uses in netedit
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>

#include "GNETAZElement.h"

// ===========================================================================
// member method definitions
// ===========================================================================

GNETAZElement::GNETAZElement(const std::string& id, GNENet* net, GUIGlObjectType type, SumoXMLTag tag, bool blockMovement,
                             const std::vector<GNEJunction*>& junctionParents,
                             const std::vector<GNEEdge*>& edgeParents,
                             const std::vector<GNELane*>& laneParents,
                             const std::vector<GNEAdditional*>& additionalParents,
                             const std::vector<GNEShape*>& shapeParents,
                             const std::vector<GNETAZElement*>& TAZElementParents,
                             const std::vector<GNEDemandElement*>& demandElementParents,
                             const std::vector<GNEGenericData*>& genericDataParents) :
    GUIGlObject(type, id),
    GNEHierarchicalElement(net, tag, junctionParents, edgeParents, laneParents, additionalParents, shapeParents, TAZElementParents, demandElementParents, genericDataParents),
    myBlockMovement(blockMovement) {
}


GNETAZElement::GNETAZElement(GNETAZElement* TAZElementParent, GNENet* net, GUIGlObjectType type, SumoXMLTag tag, bool blockMovement,
                             const std::vector<GNEJunction*>& junctionParents,
                             const std::vector<GNEEdge*>& edgeParents,
                             const std::vector<GNELane*>& laneParents,
                             const std::vector<GNEAdditional*>& additionalParents,
                             const std::vector<GNEShape*>& shapeParents,
                             const std::vector<GNETAZElement*>& TAZElementParents,
                             const std::vector<GNEDemandElement*>& demandElementParents,
                             const std::vector<GNEGenericData*>& genericDataParents) :
    GUIGlObject(type, TAZElementParent->generateChildID(tag)),
    GNEHierarchicalElement(net, tag, junctionParents, edgeParents, laneParents, additionalParents, shapeParents, TAZElementParents, demandElementParents, genericDataParents),
    myBlockMovement(blockMovement) {
}


GNETAZElement::~GNETAZElement() {}


const std::string&
GNETAZElement::getID() const {
    return getMicrosimID();
}


GUIGlObject*
GNETAZElement::getGUIGlObject() {
    return this;
}


std::string
GNETAZElement::generateChildID(SumoXMLTag childTag) {
    int counter = (int)getChildTAZElements().size();
    while (myNet->retrieveTAZElement(childTag, getID() + toString(childTag) + toString(counter), false) != nullptr) {
        counter++;
    }
    return (getID() + toString(childTag) + toString(counter));
}


bool
GNETAZElement::isTAZElementBlocked() const {
    return myBlockMovement;
}


GUIGLObjectPopupMenu*
GNETAZElement::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUIGLObjectPopupMenu(app, parent, *this);
    // build header
    buildPopupHeader(ret, app);
    // build menu command for center button and copy cursor position to clipboard
    buildCenterPopupEntry(ret);
    buildPositionCopyEntry(ret, false);
    // buld menu commands for names
    new FXMenuCommand(ret, ("Copy " + getTagStr() + " name to clipboard").c_str(), nullptr, ret, MID_COPY_NAME);
    new FXMenuCommand(ret, ("Copy " + getTagStr() + " typed name to clipboard").c_str(), nullptr, ret, MID_COPY_TYPED_NAME);
    new FXMenuSeparator(ret);
    // build selection and show parameters menu
    myNet->getViewNet()->buildSelectionACPopupEntry(ret, this);
    buildShowParamsPopupEntry(ret);
    return ret;
}


GUIParameterTableWindow*
GNETAZElement::getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView&) {
    // Create table
    GUIParameterTableWindow* ret = new GUIParameterTableWindow(app, *this);
    // Iterate over attributes
    for (const auto& i : myTagProperty) {
        // Add attribute and set it dynamic if aren't unique
        if (i.isUnique()) {
            ret->mkItem(i.getAttrStr().c_str(), false, getAttribute(i.getAttr()));
        } else {
            ret->mkItem(i.getAttrStr().c_str(), true, getAttribute(i.getAttr()));
        }
    }
    // close building
    ret->closeBuilding();
    return ret;
}

// ---------------------------------------------------------------------------
// GNETAZElement - protected methods
// ---------------------------------------------------------------------------

bool
GNETAZElement::isValidTAZElementID(const std::string& newID) const {
    if (SUMOXMLDefinitions::isValidAdditionalID(newID) && (myNet->retrieveTAZElement(myTagProperty.getTag(), newID, false) == nullptr)) {
        return true;
    } else {
        return false;
    }
}


void
GNETAZElement::enableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    //
}


void
GNETAZElement::disableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    //
}


bool
GNETAZElement::checkChildTAZElementRestriction() const {
    // throw exception because this function mus be implemented in child (see GNEE3Detector)
    throw ProcessError("Calling non-implemented function checkChildTAZElementRestriction during saving of " + getTagStr() + ". It muss be reimplemented in child class");
}


void
GNETAZElement::setEnabledAttribute(const int /*enabledAttributes*/) {
    //
}


/****************************************************************************/
