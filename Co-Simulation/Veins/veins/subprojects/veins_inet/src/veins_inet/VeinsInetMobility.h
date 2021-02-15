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

//
// Veins Mobility module for the INET Framework
// Based on inet::MovingMobilityBase of INET Framework v4.0.0
//

#pragma once

namespace omnetpp {
}
using namespace omnetpp;

#include "inet/mobility/base/MobilityBase.h"

#include "veins_inet/veins_inet.h"

#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

namespace veins {

class VEINS_INET_API VeinsInetMobility : public inet::MobilityBase {
public:
    VeinsInetMobility();

    virtual ~VeinsInetMobility();

    /** @brief called by class VeinsInetManager */
    virtual void preInitialize(std::string external_id, const inet::Coord& position, std::string road_id, double speed, double angle);

    virtual void initialize(int stage) override;

    /** @brief called by class VeinsInetManager */
    virtual void nextPosition(const inet::Coord& position, std::string road_id, double speed, double angle);

    virtual inet::Coord getCurrentPosition() override;
    virtual inet::Coord getCurrentVelocity() override;
    virtual inet::Coord getCurrentAcceleration() override;

    virtual inet::Quaternion getCurrentAngularPosition() override;
    virtual inet::Quaternion getCurrentAngularVelocity() override;
    virtual inet::Quaternion getCurrentAngularAcceleration() override;

    virtual std::string getExternalId() const;
    virtual TraCIScenarioManager* getManager() const;
    virtual TraCICommandInterface* getCommandInterface() const;
    virtual TraCICommandInterface::Vehicle* getVehicleCommandInterface() const;

protected:
    /** @brief The last velocity that was set by nextPosition(). */
    inet::Coord lastVelocity;

    /** @brief The last angular velocity that was set by nextPosition(). */
    inet::Quaternion lastAngularVelocity;

    mutable TraCIScenarioManager* manager = nullptr; /**< cached value */
    mutable TraCICommandInterface* commandInterface = nullptr; /**< cached value */
    mutable TraCICommandInterface::Vehicle* vehicleCommandInterface = nullptr; /**< cached value */

    std::string external_id; /**< identifier used by TraCI server to refer to this node */

protected:
    virtual void setInitialPosition() override;

    virtual void handleSelfMessage(cMessage* message) override;
};

} // namespace veins

namespace veins {
class VEINS_INET_API VeinsInetMobilityAccess {
public:
    VeinsInetMobility* get(cModule* host)
    {
        VeinsInetMobility* m = FindModule<VeinsInetMobility*>::findSubModule(host);
        ASSERT(m);
        return m;
    };
};
} // namespace veins
