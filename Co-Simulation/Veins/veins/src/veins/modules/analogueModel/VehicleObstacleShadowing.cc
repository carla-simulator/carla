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

#include "veins/modules/analogueModel/VehicleObstacleShadowing.h"

using namespace veins;

VehicleObstacleShadowing::VehicleObstacleShadowing(cComponent* owner, VehicleObstacleControl& vehicleObstacleControl, bool useTorus, const Coord& playgroundSize)
    : AnalogueModel(owner)
    , vehicleObstacleControl(vehicleObstacleControl)
    , useTorus(useTorus)
    , playgroundSize(playgroundSize)
{
    if (useTorus) throw cRuntimeError("VehicleObstacleShadowing does not work on torus-shaped playgrounds");
}

void VehicleObstacleShadowing::filterSignal(Signal* signal)
{
    auto senderPos = signal->getSenderPoa().pos.getPositionAt();
    auto receiverPos = signal->getReceiverPoa().pos.getPositionAt();

    auto potentialObstacles = vehicleObstacleControl.getPotentialObstacles(signal->getSenderPoa().pos, signal->getReceiverPoa().pos, *signal);

    if (potentialObstacles.size() < 1) return;

    double senderHeight = senderPos.z;
    double receiverHeight = receiverPos.z;
    potentialObstacles.insert(potentialObstacles.begin(), std::make_pair(0, senderHeight));
    potentialObstacles.emplace_back(senderPos.distance(receiverPos), receiverHeight);

    auto attenuationDB = VehicleObstacleControl::getVehicleAttenuationDZ(potentialObstacles, Signal(signal->getSpectrum()));

    EV_TRACE << "t=" << simTime() << ": Attenuation by vehicles is " << attenuationDB << std::endl;

    // convert from "dB loss" to a multiplicative factor
    Signal attenuation(attenuationDB.getSpectrum());
    for (uint16_t i = 0; i < attenuation.getNumValues(); i++) {
        attenuation.at(i) = pow(10.0, -attenuationDB.at(i) / 10.0);
    }

    *signal *= attenuation;
}
