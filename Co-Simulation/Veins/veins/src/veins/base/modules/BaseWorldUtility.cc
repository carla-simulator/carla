//
// Copyright (C) 2006 Parallel and Distributed Systems Group (PDS) at Technische Universiteit Delft, The Netherlands.
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

// author:      Tom Parker
// description: basic world utility class
//              provides world-required values

#include "veins/base/modules/BaseWorldUtility.h"
#include "veins/base/utils/FindModule.h"
#include "veins/base/connectionManager/BaseConnectionManager.h"

using namespace veins;

Define_Module(veins::BaseWorldUtility);

BaseWorldUtility::BaseWorldUtility()
    : isInitialized(false)
{
}

void BaseWorldUtility::initialize(int stage)
{
    if (stage == 0) {
        initializeIfNecessary();
    }
    else if (stage == 1) {
        // check if necessary modules are there
        // Connection Manager
        if (!FindModule<BaseConnectionManager*>::findGlobalModule()) {
            throw cRuntimeError("Could not find a connection manager module in the network!");
        }
    }
}

void BaseWorldUtility::initializeIfNecessary()
{
    if (isInitialized) return;

    use2DFlag = par("use2D");
    playgroundSize = Coord(par("playgroundSizeX").doubleValue(), par("playgroundSizeY").doubleValue(), use2DFlag ? 0. : par("playgroundSizeZ").doubleValue());

    if (playgroundSize.x < 0) {
        throw cRuntimeError("Playground size in X direction is invalid: (%f). Should be greater than or equal to zero.", playgroundSize.x);
    }
    if (playgroundSize.y < 0) {
        throw cRuntimeError("Playground size in Y direction is invalid: (%f). Should be greater than or equal to zero.", playgroundSize.y);
    }
    if (!use2DFlag && playgroundSize.z < 0) {
        throw cRuntimeError("Playground size in Z direction is invalid: (%f). Should be greater than or equal to zero.", playgroundSize.z);
    }

    useTorusFlag = par("useTorus");

    airFrameId = 0;

    isInitialized = true;
}

Coord BaseWorldUtility::getRandomPosition()
{
    initializeIfNecessary();

    return Coord(uniform(0, playgroundSize.x), uniform(0, playgroundSize.y), use2DFlag ? 0. : uniform(0, playgroundSize.z));
}
