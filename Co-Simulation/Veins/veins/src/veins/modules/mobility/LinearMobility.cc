//
// Copyright (C) 2005 Emin Ilker Cetinbas
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

// Author: Emin Ilker Cetinbas (niw3_at_yahoo_d0t_com)

#include "veins/modules/mobility/LinearMobility.h"

#include "veins/veins.h"

using namespace veins;

Define_Module(veins::LinearMobility);

void LinearMobility::initialize(int stage)
{
    BaseMobility::initialize(stage);

    EV_TRACE << "initializing LinearMobility stage " << stage << endl;

    if (stage == 0) {
        move.setSpeed(par("speed").doubleValue());
        acceleration = par("acceleration");
        angle = par("angle");
        angle = fmod(angle, 360);
    }
    else if (stage == 1) {
        stepTarget = move.getStartPos();
    }
}

void LinearMobility::fixIfHostGetsOutside()
{
    Coord dummy = Coord::ZERO;
    handleIfOutside(WRAP, stepTarget, dummy, dummy, angle);
}

/**
 * Move the host if the destination is not reached yet. Otherwise
 * calculate a new random position
 */
void LinearMobility::makeMove()
{
    EV_TRACE << "start makeMove " << move.info() << endl;

    move.setStart(stepTarget, simTime());

    stepTarget.x = (move.getStartPos().x + move.getSpeed() * cos(M_PI * angle / 180) * SIMTIME_DBL(updateInterval));
    stepTarget.y = (move.getStartPos().y + move.getSpeed() * sin(M_PI * angle / 180) * SIMTIME_DBL(updateInterval));

    move.setDirectionByTarget(stepTarget);

    EV_TRACE << "new stepTarget: " << stepTarget.info() << endl;

    // accelerate
    move.setSpeed(move.getSpeed() + acceleration * SIMTIME_DBL(updateInterval));

    fixIfHostGetsOutside();
}
