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
/// @file    MSSOTLPolicy5DStimulus.h
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
#include "MSSOTLPolicyDesirability.h"

/**
 * \class MSSOTLPolicyStimulus
 * \brief This class determines the stimulus of a MSSOTLPolicy when
 * used in combination with a high level policy.\n
 * The stimulus function is calculated as follows:\n
 * stimulus = cox * exp(-pow(pheroIn - offsetIn, 2)/divisor -pow(pheroOut - offsetOut, 2)/divisor)
 */
class MSSOTLPolicy5DStimulus: public MSSOTLPolicyDesirability {

private:

    double stimCoxDVal,
           stimOffsetInDVal, stimOffsetOutDVal, stimOffsetDispersionInDVal, stimOffsetDispersionOutDVal,
           stimDivInDVal, stimDivOutDVal, stimDivDispersionInDVal, stimDivDispersionOutDVal,
           stimCoxExpInDVal, stimCoxExpOutDVal, stimCoxExpDispersionInDVal, stimCoxExpDispersionOutDVal;

public:

    MSSOTLPolicy5DStimulus(std::string keyPrefix, const std::map<std::string, std::string>& parameters);

    double getStimCox() {
        std::string key = getKeyPrefix() + "_STIM_COX";
        return getDouble(key, stimCoxDVal);
    }
    void setStimCoxDefVal(double defVal) {
        stimCoxDVal = defVal;
    }
    double getStimOffsetIn() {
        std::string key = getKeyPrefix() + "_STIM_OFFSET_IN";
        return getDouble(key, stimOffsetInDVal);
    }
    void setStimOffsetInDefVal(double defVal) {
        stimOffsetInDVal = defVal;
    }
    double getStimOffsetOut() {
        std::string key = getKeyPrefix() + "_STIM_OFFSET_OUT";
        return getDouble(key, stimOffsetOutDVal);
    }

    void setStimOffsetOutDefVal(double defVal) {
        stimOffsetOutDVal = defVal;
    }

    double getStimOffsetDispersionIn() {
        std::string key = getKeyPrefix() + "_STIM_OFFSET_DISPERSION_IN";
        return getDouble(key, stimOffsetDispersionInDVal);
    }
    void setStimOffsetDispersionInDefVal(double defVal) {
        stimOffsetDispersionInDVal = defVal;
    }
    double getStimOffsetDispersionOut() {
        std::string key = getKeyPrefix() + "_STIM_OFFSET_DISPERSION_OUT";
        return getDouble(key, stimOffsetDispersionOutDVal);
    }
    void setStimOffsetDispersionOutDefVal(double defVal) {
        stimOffsetDispersionOutDVal = defVal;
    }
    double getStimDivisorIn() {
        std::string key = getKeyPrefix() + "_STIM_DIVISOR_IN";
        return getDouble(key, stimDivInDVal);
    }

    void setStimDivisorInDefVal(double defVal) {
        stimDivInDVal = defVal;
    }

    double getStimDivisorOut() {
        std::string key = getKeyPrefix() + "_STIM_DIVISOR_OUT";
        return getDouble(key, stimDivOutDVal);
    }

    void setStimDivisorOutDefVal(double defVal) {
        stimDivOutDVal = defVal;
    }

    double getStimDivisorDispersionIn() {
        std::string key = getKeyPrefix() + "_STIM_DIVISOR_DISPERSION_IN";
        return getDouble(key, stimDivDispersionInDVal);
    }

    void setStimDivisorDispersionInDefVal(double defVal) {
        stimDivDispersionInDVal = defVal;
    }
    double getStimDivisorDispersionOut() {
        std::string key = getKeyPrefix() + "_STIM_DIVISOR_DISPERSION_OUT";
        return getDouble(key, stimDivDispersionOutDVal);
    }

    void setStimDivisorDispersionOutDefVal(double defVal) {
        stimDivDispersionOutDVal = defVal;
    }
    double getStimCoxExpIn() {
        std::string key = getKeyPrefix() + "_STIM_COX_EXP_IN";
        return getDouble(key, stimCoxExpInDVal);
    }
    void setStimCoxExpInDefVal(double defVal) {
        stimCoxExpInDVal = defVal;
    }
    double getStimCoxExpOut() {
        std::string key = getKeyPrefix() + "_STIM_COX_EXP_OUT";
        return getDouble(key, stimCoxExpOutDVal);
    }
    void setStimCoxExpOutDefVal(double defVal) {
        stimCoxExpOutDVal = defVal;
    }

    double getStimCoxExpDispersionIn() {
        std::string key = getKeyPrefix() + "_STIM_COX_EXP_DISPERSION_IN";
        return getDouble(key, stimCoxExpDispersionInDVal);
    }
    void setStimCoxExpDispersionInDefVal(double defVal) {
        stimCoxExpDispersionInDVal = defVal;
    }
    double getStimCoxExpDispersionOut() {
        std::string key = getKeyPrefix() + "_STIM_COX_EXP_DISPERSION_OUT";
        return getDouble(key, stimCoxExpDispersionOutDVal);
    }
    void setStimCoxExpDispersionOutDefVal(double defVal) {
        stimCoxExpDispersionOutDVal = defVal;
    }
    std::string getMessage();
    /**
     *	@brief Computes stimulus function
     *  stimulus = cox * exp(-pow(pheroIn - offsetIn, 2)/divisor -pow(pheroOut - offsetOut, 2)/divisor);
     */
    virtual double computeDesirability(double vehInMeasure,
                                       double vehOutMeasure);

    virtual double computeDesirability(double vehInMeasure, double vehOutMeasure, double vehInDispersionMeasure, double vehOutDispersionMeasure);
};

