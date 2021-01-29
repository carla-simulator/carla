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
/// @file    GNENetworkElement.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2016
///
// A abstract class for networkElements
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <utils/gui/div/GUIParameterTableWindow.h>

#include "GNENetworkElement.h"


// ===========================================================================
// method definitions
// ===========================================================================

GNENetworkElement::GNENetworkElement(GNENet* net, const std::string& id, GUIGlObjectType type, SumoXMLTag tag,
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
    myMovingGeometryBoundary(),
    myShapeEdited(false) {
}


GNENetworkElement::~GNENetworkElement() {}


const std::string&
GNENetworkElement::getID() const {
    return getMicrosimID();
}


GUIGlObject*
GNENetworkElement::getGUIGlObject() {
    return this;
}


void
GNENetworkElement::setShapeEdited(const bool value) {
    myShapeEdited = value;
}


bool
GNENetworkElement::isShapeEdited() const {
    return myShapeEdited;
}


GUIParameterTableWindow*
GNENetworkElement::getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView&) {
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


void
GNENetworkElement::enableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    //
}


void
GNENetworkElement::disableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    //
}


std::string
GNENetworkElement::getPopUpID() const {
    if (myTagProperty.getTag() == SUMO_TAG_CONNECTION) {
        return getAttribute(SUMO_ATTR_FROM) + "_" + getAttribute(SUMO_ATTR_FROM_LANE) + " -> " + getAttribute(SUMO_ATTR_TO) + "_" + getAttribute(SUMO_ATTR_TO_LANE);
    } else {
        return getTagStr() + ": " + getID();
    }
}


std::string
GNENetworkElement::getHierarchyName() const {
    if (myTagProperty.getTag() == SUMO_TAG_LANE) {
        return toString(SUMO_TAG_LANE) + " " + getAttribute(SUMO_ATTR_INDEX);
    } else if (myTagProperty.getTag() == SUMO_TAG_CONNECTION) {
        return getAttribute(SUMO_ATTR_FROM_LANE) + " -> " + getAttribute(SUMO_ATTR_TO_LANE);
    } else if ((myTagProperty.getTag() == SUMO_TAG_EDGE) || (myTagProperty.getTag() == SUMO_TAG_CROSSING)) {
        return getPopUpID();
    } else {
        return getTagStr();
    }
}


void
GNENetworkElement::setEnabledAttribute(const int /*enabledAttributes*/) {
    //
}


/****************************************************************************/
