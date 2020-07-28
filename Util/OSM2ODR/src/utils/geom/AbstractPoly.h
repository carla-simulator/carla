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
/// @file    AbstractPoly.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// The base class for polygons
/****************************************************************************/
#pragma once
#include <config.h>

#include "Position.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 *
 */
class AbstractPoly {
public:
    /// @brief constructor
    AbstractPoly() { }

    /// @brief copy constructor
    AbstractPoly(const AbstractPoly&) { }

    /// @brief destructor
    virtual ~AbstractPoly() { }

    /// @brief Returns whether the AbstractPoly the given coordinate
    virtual bool around(const Position& p, double offset = 0) const = 0;

    /// @brief Returns whether the AbstractPoly overlaps with the given polygon
    virtual bool overlapsWith(const AbstractPoly& poly, double offset = 0) const = 0;

    /// @brief Returns whether the AbstractPoly is partially within the given polygon
    virtual bool partialWithin(const AbstractPoly& poly, double offset = 0) const = 0;

    /// @brief Returns whether the AbstractPoly crosses the given line
    virtual bool crosses(const Position& p1, const Position& p2) const = 0;
};
