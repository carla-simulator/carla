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
/// @file    GNEShape.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2017
///
// Abstract class for Shapes uses in netedit
/****************************************************************************/
#include <config.h>

#include <utils/gui/images/GUITextureSubSys.h>
#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <utils/gui/globjects/GLIncludes.h>

#include "GNEShape.h"


// ===========================================================================
// method definitions
// ===========================================================================

GNEShape::GNEShape(GNENet* net, SumoXMLTag tag, bool movementBlocked,
                   const std::vector<GNEJunction*>& junctionParents,
                   const std::vector<GNEEdge*>& edgeParents,
                   const std::vector<GNELane*>& laneParents,
                   const std::vector<GNEAdditional*>& additionalParents,
                   const std::vector<GNEShape*>& shapeParents,
                   const std::vector<GNETAZElement*>& TAZElementParents,
                   const std::vector<GNEDemandElement*>& demandElementParents,
                   const std::vector<GNEGenericData*>& genericDataParents) :
    GNEHierarchicalElement(net, tag, junctionParents, edgeParents, laneParents, additionalParents, shapeParents, TAZElementParents, demandElementParents, genericDataParents),
    myBlockMovement(movementBlocked) {
}


GNEShape::~GNEShape() {}


bool
GNEShape::isMovementBlocked() const {
    return myBlockMovement;
}


void
GNEShape::draw(const Position& pos, double layer, double size) const {
    if (myNet->getViewNet()->showLockIcon()) {
        // Start pushing matrix
        glPushMatrix();
        // Traslate to middle of shape
        glTranslated(pos.x(), pos.y(), layer + 0.1);
        // Rotate 180 degrees
        glRotated(180, 0, 0, 1);
        // Set draw color
        glColor3d(1, 1, 1);
        // Draw icon depending of the selection status
        if (mySelected) {
            if (myBlockMovement) {
                // Draw lock texture if shape is movable, is blocked and is selected
                GUITexturesHelper::drawTexturedBox(GUITextureSubSys::getTexture(GNETEXTURE_LOCKSELECTED), size);
            } else {
                // Draw empty texture if shape is movable, isn't blocked and is selected
                GUITexturesHelper::drawTexturedBox(GUITextureSubSys::getTexture(GNETEXTURE_EMPTYSELECTED), size);
            }
        } else {
            if (myBlockMovement) {
                // Draw lock texture if shape is movable and is blocked
                GUITexturesHelper::drawTexturedBox(GUITextureSubSys::getTexture(GNETEXTURE_LOCK), size);
            } else {
                // Draw empty texture if shape is movable and isn't blocked
                GUITexturesHelper::drawTexturedBox(GUITextureSubSys::getTexture(GNETEXTURE_EMPTY), size);
            }
        }
        // Pop matrix
        glPopMatrix();
    }
}


void
GNEShape::enableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    //
}


void
GNEShape::disableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    //
}


std::string
GNEShape::getPopUpID() const {
    return getTagStr() + ": " + getID();
}


std::string
GNEShape::getHierarchyName() const {
    return getTagStr();
}


void
GNEShape::replaceShapeParentLanes(const std::string& value) {
    replaceParentElements(this, parse<std::vector<GNELane*> >(getNet(), value));
}


void
GNEShape::setEnabledAttribute(const int /*enabledAttributes*/) {
    //
}


/****************************************************************************/
