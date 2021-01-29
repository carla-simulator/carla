/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2019-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSDynamicShapeUpdater.h
/// @author  Leonhard Luecken
/// @date    08 Apr 2019
///
//
/****************************************************************************/
#pragma once

#include "MSNet.h"
#include "utils/shapes/ShapeContainer.h"

/** @class DynamicShapeUpdater
 * @brief Ensures that dynamic shapes tracking traffic objects are removed when the objects are removed.
 */
class MSDynamicShapeUpdater: public MSNet::VehicleStateListener {
public:
    /// @brief Constructor
    MSDynamicShapeUpdater(ShapeContainer& shapeContainer) : myShapeContainer(shapeContainer) {};

    /// @brief Constructor
    void vehicleStateChanged(const SUMOVehicle* const vehicle, MSNet::VehicleState to, const std::string& info = "") override;

private:
    ShapeContainer& myShapeContainer;

private:
    /// @brief invalidated assignment operator
    MSDynamicShapeUpdater& operator=(const MSDynamicShapeUpdater& s) = delete;
};

