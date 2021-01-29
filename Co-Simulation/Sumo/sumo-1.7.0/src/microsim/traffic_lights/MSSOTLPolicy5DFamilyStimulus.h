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
/// @file    MSSOTLPolicy5DFamilyStimulus.h
/// @author  Riccardo Belletti
/// @author  Simone Bacchilega
/// @date    2014-09-30
///
// The class the low-level policy stimulus
/****************************************************************************/
#pragma once
#include <config.h>

//#define SWARM_DEBUG
#include <utils/common/SwarmDebug.h>
#include <utils/common/MsgHandler.h>
#include <sstream>
#include <cmath>
#include <stdlib.h>
#include <string.h>
#include "MSSOTLPolicy5DStimulus.h"

/**
 * \class MSSOTLPolicy5DListStimulus
 * \brief This class determines the stimulus of a MSSOTLPolicy when
 * used in combination with a high level policy.\n
 * The stimulus function is calculated as follows:\n
 * stimulus = cox * exp(-pow(pheroIn - offsetIn, 2)/divisor -pow(pheroOut - offsetOut, 2)/divisor)
 */
class MSSOTLPolicy5DFamilyStimulus: public MSSOTLPolicyDesirability {

private:

    /*double stimCoxDVal,
    stimOffsetInDVal, stimOffsetOutDVal, stimOffsetDispersionInDVal, stimOffsetDispersionOutDVal,
    stimDivInDVal, stimDivOutDVal, stimDivDispersionInDVal, stimDivDispersionOutDVal,
    stimCoxExpInDVal, stimCoxExpOutDVal, stimCoxExpDispersionInDVal, stimCoxExpDispersionOutDVal;*/

    std::map<std::string, std::string> default_values;
    std::vector<std::string> params_names;
    std::vector<MSSOTLPolicy5DStimulus*> family;

    //std::vector<std::string> inline StringSplit(const std::string &source, const char *delimiter = " ", bool keepEmpty = false);

public:

    MSSOTLPolicy5DFamilyStimulus(std::string keyPrefix, const std::map<std::string, std::string>& parameters);

    std::string getMessage();

    std::vector<MSSOTLPolicy5DStimulus*> getFamilies() {
        return family;
    }

    /*
     *	@brief Computes stimulus function
     *  stimulus = cox * exp(-pow(pheroIn - offsetIn, 2)/divisor -pow(pheroOut - offsetOut, 2)/divisor);
     */
    virtual double computeDesirability(double vehInMeasure,
                                       double vehOutMeasure);

    virtual double computeDesirability(double vehInMeasure, double vehOutMeasure, double vehInDispersionMeasure, double vehOutDispersionMeasure);
};

