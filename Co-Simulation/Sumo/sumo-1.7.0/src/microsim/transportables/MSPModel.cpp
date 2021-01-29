/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSPModel.cpp
/// @author  Jakob Erdmann
/// @date    Mon, 13 Jan 2014
///
// The pedestrian following model (prototype)
/****************************************************************************/
#include <config.h>

#include <cmath>
#include <algorithm>
#include <utils/options/OptionsCont.h>
#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <microsim/MSJunction.h>
#include <microsim/MSLane.h>
#include <microsim/MSGlobals.h>
#include "MSPModel_Striping.h"
#include "MSPModel_NonInteracting.h"
#include "MSPModel.h"


// ===========================================================================
// static members
// ===========================================================================
// named constants
const int MSPModel::FORWARD(1);
const int MSPModel::BACKWARD(-1);
const int MSPModel::UNDEFINED_DIRECTION(0);

// parameters shared by all models
const double MSPModel::SAFETY_GAP(1.0);
const double MSPModel::SIDEWALK_OFFSET(3);


// ===========================================================================
// MSPModel method definitions
// ===========================================================================
int
MSPModel::canTraverse(int dir, const ConstMSEdgeVector& route) {
    const MSJunction* junction = nullptr;
    for (ConstMSEdgeVector::const_iterator it = route.begin(); it != route.end(); ++it) {
        const MSEdge* edge = *it;
        if (junction != nullptr) {
            //std::cout << " junction=" << junction->getID() << " edge=" << edge->getID() << "\n";
            if (junction == edge->getFromJunction()) {
                dir = FORWARD;
            } else if (junction == edge->getToJunction()) {
                dir = BACKWARD;
            } else {
                return UNDEFINED_DIRECTION;
            }
        }
        junction = dir == FORWARD ? edge->getToJunction() : edge->getFromJunction();
    }
    return dir;
}


/****************************************************************************/
