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
/// @file    GNEMoveShape.h
/// @author  Pablo Alvarez Lopez
/// @date    Mar 2020
///
// Class used for elements that own a movable shape
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/geom/PositionVector.h>


// ===========================================================================
// class definitions
// ===========================================================================

class GNEMoveShape {

public:
    /// @brief constructor
    GNEMoveShape();

    /// @gbrief start move shape
    void startMoveShape(const PositionVector& shapeBeforeMoving, const double offsetMovingShape, const double sensibility);

    /// @brief get shape before moving
    const PositionVector& getShapeBeforeMoving() const;

    /// @brief get position over shape before moving
    const Position& getPosOverShapeBeforeMoving() const;

    /// @brief get geometry point index
    const int& getGeometryPointIndex() const;

    /// @brief check if entire shape must be moved
    bool moveEntireShape() const;

private:
    /// @brief save shape Before moving
    PositionVector myShapeBeforeMoving;

    /// @brief position over shape before moving
    Position myPosOverShape;

    /// @brief geometry point index
    int myGeometryPointIndex;

    /// @brief move entire shape
    bool myMoveEntireShape;

    /// @brief Invalidated copy constructor.
    GNEMoveShape(const GNEMoveShape&) = delete;

    /// @brief Invalidated assignment operator.
    GNEMoveShape& operator=(const GNEMoveShape&) = delete;
};
