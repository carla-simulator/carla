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
/// @file    Position.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Axel Wegener
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A position in the 2D- or 3D-world
/****************************************************************************/
#include <config.h>

#include <limits>
#include "Position.h"


// ===========================================================================
// static member definitions
// ===========================================================================

// Position 1Mio km below the surface should suffice for signaling invalidity inside the solar system
const Position Position::INVALID(
    - 1024 * 1024 * 1024,
    - 1024 * 1024 * 1024,
    - 1024 * 1024 * 1024);


Position
Position::rotateAround2D(double rad, const Position& origin) {
    const double s = sin(rad);
    const double c = cos(rad);
    Position p = (*this) - origin;
    return Position(
               p.x() * c - p.y() * s,
               p.x() * s + p.y() * c) + origin;

}
