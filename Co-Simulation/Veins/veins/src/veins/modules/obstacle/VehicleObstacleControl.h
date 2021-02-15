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

#include <list>

#include "veins/veins.h"

#include "veins/base/utils/AntennaPosition.h"
#include "veins/base/utils/Coord.h"
#include "veins/modules/obstacle/Obstacle.h"
#include "veins/modules/world/annotations/AnnotationManager.h"
#include "veins/base/utils/Move.h"
#include "veins/modules/obstacle/MobileHostObstacle.h"

namespace veins {

class Signal;

/**
 * VehicleObstacleControl models moving obstacles that block radio transmissions.
 *
 * Each Obstacle is a polygon.
 * Transmissions that cross one of the polygon's lines will have
 * their receive power set to zero.
 */
class VEINS_API VehicleObstacleControl : public cSimpleModule {
public:
    ~VehicleObstacleControl() override;
    void initialize(int stage) override;
    int numInitStages() const override
    {
        return 2;
    }
    void finish() override;
    void handleMessage(cMessage* msg) override;
    void handleSelfMsg(cMessage* msg);

    const MobileHostObstacle* add(MobileHostObstacle obstacle);
    void erase(const MobileHostObstacle* obstacle);

    /**
     * get distance and height of potential obstacles
     */
    std::vector<std::pair<double, double>> getPotentialObstacles(const AntennaPosition& senderPos, const AntennaPosition& receiverPos, const Signal& s) const;

    /**
     * compute attenuation due to (single) vehicle.
     * Calculate impact of vehicles as obstacles according to:
     * M. Boban, T. T. V. Vinhoza, M. Ferreira, J. Barros, and O. K. Tonguz: 'Impact of Vehicles as Obstacles in Vehicular Ad Hoc Networks', IEEE JSAC, Vol. 29, No. 1, January 2011
     *
     * @param h1: height of sender
     * @param h2: height of receiver
     * @param h: height of obstacle
     * @param d: distance between sender and receiver
     * @param d1: distance between sender and obstacle
     * @param attenuationPrototype: a prototype Signal for constructing a Signal containing the attenuation factors for each frequency
     */
    static Signal getVehicleAttenuationSingle(double h1, double h2, double h, double d, double d1, Signal attenuationPrototype);

    /**
     * compute attenuation due to vehicles.
     * Calculate impact of vehicles as obstacles according to:
     * M. Boban, T. T. V. Vinhoza, M. Ferreira, J. Barros, and O. K. Tonguz: 'Impact of Vehicles as Obstacles in Vehicular Ad Hoc Networks', IEEE JSAC, Vol. 29, No. 1, January 2011
     *
     * @param dz_vec: a vector of (distance, height) referring to potential obstacles along the line of sight, starting with the sender and ending with the receiver
     * @param attenuationPrototype: a prototype Signal for constructing a Signal containing the attenuation factors for each frequency
     */
    static Signal getVehicleAttenuationDZ(const std::vector<std::pair<double, double>>& dz_vec, Signal attenuationPrototype);

protected:
    AnnotationManager* annotations;

    using VehicleObstacles = std::list<MobileHostObstacle*>;
    VehicleObstacles vehicleObstacles;
    AnnotationManager::Group* vehicleAnnotationGroup;
    void drawVehicleObstacles(const simtime_t& t) const;
};

class VEINS_API VehicleObstacleControlAccess {
public:
    VehicleObstacleControlAccess()
    {
    }

    VehicleObstacleControl* getIfExists()
    {
        return dynamic_cast<VehicleObstacleControl*>(getSimulation()->getModuleByPath("vehicleObstacles"));
    }
};

} // namespace veins
