//
// Copyright (C) 2006-2018 Christoph Sommer <sommer@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#pragma once

#include "veins/base/phyLayer/AnalogueModel.h"
#include "veins/base/modules/BaseWorldUtility.h"
#include "veins/modules/obstacle/VehicleObstacleControl.h"
#include "veins/base/utils/Move.h"
#include "veins/base/messages/AirFrame_m.h"

using veins::AirFrame;
using veins::VehicleObstacleControl;

#include <cstdlib>

namespace veins {

/**
 * @brief Basic implementation of a VehicleObstacleShadowing that uses
 * SimplePathlossConstMapping (that is subclassed from SimpleConstMapping) as attenuation-Mapping.
 *
 * @ingroup analogueModels
 */
class VEINS_API VehicleObstacleShadowing : public AnalogueModel {
protected:
    /** @brief reference to global VehicleObstacleControl instance */
    VehicleObstacleControl& vehicleObstacleControl;

    /** @brief Information needed about the playground */
    const bool useTorus;

    /** @brief The size of the playground.*/
    const Coord& playgroundSize;

public:
    /**
     * @brief Initializes the analogue model. myMove and playgroundSize
     * need to be valid as long as this instance exists.
     *
     * The constructor needs some specific knowledge in order to create
     * its mapping properly:
     *
     * @param owner pointer to the cComponent that owns this AnalogueModel
     * @param vehicleObstacleControl reference to global VehicleObstacleControl module
     * @param useTorus information about the playground the host is moving in
     * @param playgroundSize information about the playground the host is moving in
     */
    VehicleObstacleShadowing(cComponent* owner, VehicleObstacleControl& vehicleObstacleControl, bool useTorus, const Coord& playgroundSize);

    /**
     * @brief Filters a specified Signal by adding an attenuation
     * over time to the Signal.
     */
    void filterSignal(Signal* signal) override;

    bool neverIncreasesPower() override
    {
        return true;
    }
};

} // namespace veins
