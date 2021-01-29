/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2013-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSSOTLPolicyDesirability.h
/// @author  Riccardo Belletti
/// @date    2014-03-20
///
// The class a low-level policy desirability
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/SwarmDebug.h>
#include <sstream>
#include <utils/common/Parameterised.h>
#include <utils/common/StringUtils.h>

/**
 * \class MSSOTLPolicyDesirability
 * \brief This class determines the desirability algorithm of a MSSOTLPolicy when
 * used in combination with a high level policy
 */
class MSSOTLPolicyDesirability: public Parameterised {

private:
    std::string myKeyPrefix;

public:

    MSSOTLPolicyDesirability(std::string keyPrefix,
                             const std::map<std::string, std::string>& parameters);
    virtual ~MSSOTLPolicyDesirability();

    /**
     * \brief Calculates the desirability of the policy
     */
    virtual double computeDesirability(double vehInMeasure,
                                       double vehOutMeasure) = 0;
    /**
     * \brief Calculates the desirability of the policy
     */
    virtual double computeDesirability(double vehInMeasure, double vehOutMeasure,
                                       double vehInDispersionMeasure,
                                       double vehOutDispersionMeasure) = 0;
    virtual std::string getMessage() = 0;
    void setKeyPrefix(std::string val) {
        myKeyPrefix = val;
    }
    std::string getKeyPrefix() {
        return myKeyPrefix;
    }

};

