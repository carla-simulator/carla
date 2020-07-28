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
/// @file    Distribution_Points.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// The description of a distribution by a curve
/****************************************************************************/
#pragma once
#include <config.h>

#include "RandomDistributor.h"
#include "Distribution.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class Distribution_Points
 * A description of a distribution that uses a set of points in a 2d-space
 *  to describe the values (each points x-value) and their possibilities
 *  (each points y-value)
 */
class Distribution_Points :
    public Distribution, public RandomDistributor<double> {
public:
    /// Constructor
    Distribution_Points(const std::string& id);

    /// Destructor
    virtual ~Distribution_Points();

    /** @brief Draw a sample of the distribution.
    *
    * A random sample is drawn according to the assigned probabilities.
    *
    * @param[in] which The random number generator to use; the static one will be used if 0 is passed
    * @return the drawn member
    */
    double sample(std::mt19937* which = 0) const {
        return get(which);
    }

    /// Returns the maximum value of this distribution
    double getMax() const;

    /// Returns the string representation of this distribution
    std::string toStr(std::streamsize accuracy) const;
};
