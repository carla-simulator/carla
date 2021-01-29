//
// Copyright (C) 2006-2017 Christoph Sommer <sommer@ccs-labs.org>
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
// Veins Mobility module for the INET Framework (i.e., implementing inet::IMobility)
// Based on inet::MobilityBase of INET Framework v3.4.0
//

#pragma once

#undef INET_IMPORT
#include "inet/common/ModuleAccess.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/EulerAngles.h"
#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/mobility/contract/IMobility.h"

#include "veins_inet/veins_inet.h"

namespace veins {

class VEINS_INET_API VeinsInetMobility : public cSimpleModule, public inet::IMobility {
public:
    VeinsInetMobility();

public:
    virtual void preInitialize(std::string external_id, const inet::Coord& position, std::string road_id, double speed, double angle);
    virtual void nextPosition(const inet::Coord& position, std::string road_id, double speed, double angle);

public:
    virtual double getMaxSpeed() const override;

    virtual inet::Coord getCurrentPosition() override;
    virtual inet::Coord getCurrentSpeed() override;
    virtual inet::EulerAngles getCurrentAngularPosition() override;
    virtual inet::EulerAngles getCurrentAngularSpeed() override
    {
        return inet::EulerAngles::ZERO;
    }

    virtual inet::Coord getConstraintAreaMax() const override
    {
        return constraintAreaMax;
    }
    virtual inet::Coord getConstraintAreaMin() const override
    {
        return constraintAreaMin;
    }

protected:
    cModule* visualRepresentation;
    const inet::CanvasProjection* canvasProjection;

    inet::Coord constraintAreaMin, constraintAreaMax;

    inet::Coord lastPosition;
    inet::Coord lastSpeed;
    inet::EulerAngles lastOrientation;

protected:
    virtual int numInitStages() const override
    {
        return inet::NUM_INIT_STAGES;
    }

    virtual void initialize(int stage) override;

    virtual void handleMessage(cMessage* msg) override;

    virtual void updateVisualRepresentation();

    virtual void emitMobilityStateChangedSignal();
};

} // namespace veins
