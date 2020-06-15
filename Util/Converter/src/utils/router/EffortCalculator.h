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
/// @file    EffortCalculator.h
/// @author  Michael Behrisch
/// @date    2018-08-21
///
// The EffortCalculator is an interface for additional edge effort calculators
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/Parameterised.h>


// ===========================================================================
// class definitions
// ===========================================================================
/// @brief the effort calculator interface
class EffortCalculator {

public:

    /** Pass the set of all edges in the routing query to the effortCalculator **/
    virtual void init(const std::vector<std::string>& edges) = 0;

    /** Add information about stops **/
    virtual void addStop(const int stopEdge, const Parameterised& params) = 0;

    /** Return the effort of a given edge **/
    virtual double getEffort(const int numericalID) const = 0;

    /** Update the effort of the edge **/
    virtual void update(const int edge, const int prev, const double length) = 0;

    /** Set the effort of the first edge in the query to zero **/
    virtual void setInitialState(const int edge)  = 0;

    /** basic output facility to inform about effort at this edge **/
    virtual std::string output(const int edge) const = 0;

};
