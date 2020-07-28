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
/// @file    Distribution_Parameterized.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A distribution described by parameters such as the mean value and std-dev
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <random>

#include "Distribution.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class Distribution_Parameterized
 * A description of distribution by the distribution's mean value and a
 *  standard deviation.
 * Incomplete and unused yet. This class should be overridden by derived
 *  classes
 */
class Distribution_Parameterized : public Distribution {

public:
    /// @brief Constructor for standard normal distribution
    Distribution_Parameterized(const std::string& id, double mean, double deviation);

    /// @brief Constructor for normal distribution with cutoff
    Distribution_Parameterized(const std::string& id, double mean, double deviation, double min, double max);

    /// @brief Destructor
    virtual ~Distribution_Parameterized();

    /// @brief Overwrite by parsable distribution description
    void parse(const std::string& description, const bool hardFail);

    /** @brief Draw a sample of the distribution.
    *
    * A random sample is drawn according to the assigned probabilities.
    *
    * @param[in] which The random number generator to use; the static one will be used if 0 is passed
    * @return the drawn member
    */
    double sample(std::mt19937* which = 0) const;

    /// @brief Returns the maximum value of this distribution
    double getMax() const;

    /// @brief Returns the parameters of this distribution
    std::vector<double>& getParameter();

    /// @brief Returns the unmodifiable parameters of this distribution
    const std::vector<double>& getParameter() const;

    /// @brief check whether the distribution is valid
    bool isValid(std::string& error);

    /// @brief Returns the string representation of this distribution
    std::string toStr(std::streamsize accuracy) const;

private:
    /// @brief The distribution's parameters
    std::vector<double> myParameter;
};
