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
/// @file    GNEMoveShape.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Mar 2020
///
// Class used for elements that own a movable shape
/****************************************************************************/
#include <netedit/elements/network/GNEEdge.h>

#include "GNEMoveShape.h"


GNEMoveShape::GNEMoveShape() :
    myGeometryPointIndex(0),
    myMoveEntireShape(0) {
}


void
GNEMoveShape::startMoveShape(const PositionVector& shapeBeforeMoving, const double offsetMovingShape, const double sensibility) {
    // update shape before moving
    myShapeBeforeMoving = shapeBeforeMoving;
    // check if entire shape must be moved
    if (offsetMovingShape == -1) {
        myMoveEntireShape = true;
    } else if (myShapeBeforeMoving.size() > 0) {
        myMoveEntireShape = false;
        // calculate position over shape
        if (offsetMovingShape <= 0) {
            myPosOverShape = myShapeBeforeMoving.front();
        } else if (offsetMovingShape >= getShapeBeforeMoving().length()) {
            myPosOverShape = myShapeBeforeMoving.back();
        } else {
            myPosOverShape = shapeBeforeMoving.positionAtOffset2D(offsetMovingShape);
        }
        // reset geometry point index
        myGeometryPointIndex = -1;
        // check if in shapeBeforeMoving, for the given offsetMovingShape, there is a geometry point
        for (int i = 0; i < (int)shapeBeforeMoving.size(); i++) {
            if (shapeBeforeMoving[i].distanceSquaredTo2D(myPosOverShape) <= sensibility) {
                myGeometryPointIndex = i;
            }
        }
    }
}


const PositionVector&
GNEMoveShape::getShapeBeforeMoving() const {
    return myShapeBeforeMoving;
}


const Position&
GNEMoveShape::getPosOverShapeBeforeMoving() const {
    return myPosOverShape;
}


const int&
GNEMoveShape::getGeometryPointIndex() const {
    return myGeometryPointIndex;
}


bool
GNEMoveShape::moveEntireShape() const {
    return myMoveEntireShape;
}

/****************************************************************************/
