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
/// @file    MSSOTLPolicy5DStimulus.cpp
/// @author  Riccardo Belletti
/// @author  Simone Bacchilega
/// @date    2014-09-30
///
// The class for Swarm-based low-level policy
/****************************************************************************/

#include "MSSOTLPolicy5DStimulus.h"

MSSOTLPolicy5DStimulus::MSSOTLPolicy5DStimulus(std::string keyPrefix,
        const std::map<std::string, std::string>& parameters) :
    MSSOTLPolicyDesirability(keyPrefix, parameters) {

    stimCoxDVal = 1;
    stimOffsetInDVal = 1;
    stimOffsetOutDVal = 1;
    stimOffsetDispersionInDVal = 1;
    stimOffsetDispersionOutDVal = 1;
    stimDivInDVal = 1;
    stimDivOutDVal = 1;
    stimDivDispersionInDVal = 1;
    stimDivDispersionOutDVal = 1;
    stimCoxExpInDVal = 0;
    stimCoxExpOutDVal = 0;
    stimCoxExpDispersionInDVal = 0;
    stimCoxExpDispersionOutDVal = 0;
}

double MSSOTLPolicy5DStimulus::computeDesirability(double vehInMeasure, double vehOutMeasure, double vehInDispersionMeasure, double vehOutDispersionMeasure) {
    DBG(
        std::ostringstream str;
        str << "cox=" << getStimCox() << ", cox_exp_in=" << getStimCoxExpIn() << ", cox_exp_out=" << getStimCoxExpOut()
        << ", off_in=" << getStimOffsetIn() << ", off_out=" << getStimOffsetOut() << ", div_in=" << getStimDivisorIn() << ", div_out=" << getStimDivisorOut(); WRITE_MESSAGE(str.str());)

    //		it seems to be not enough, a strange segmentation fault appears...
    //	 if((getStimCoxExpIn()!=0.0 && getStimDivisorIn()==0.0)||(getStimCoxExpOut()!=0.0 && getStimDivisorOut()==0.0)){
    if (getStimDivisorIn() == 0 || getStimDivisorOut() == 0) {
        std::ostringstream errorMessage;
        errorMessage << "INCORRECT VALUES" << "\nStimCoxExpIn="
                     << getStimCoxExpIn() << ", StimDivisorIn=" << getStimDivisorIn()
                     << ", StimCoxExpOut=" << getStimCoxExpOut()
                     << ", StimDivisorOut=" << getStimDivisorOut();
        WRITE_ERROR(errorMessage.str());
        assert(-1);
        return -1;
    } else {
        double stimulus = getStimCox()
                          * exp(
                              -getStimCoxExpIn()
                              * pow(vehInMeasure - getStimOffsetIn(), 2)
                              / getStimDivisorIn()
                              - getStimCoxExpOut()
                              * pow(vehOutMeasure - getStimOffsetOut(), 2)
                              / getStimDivisorOut()
                              - getStimCoxExpDispersionIn()
                              * pow(vehInDispersionMeasure - getStimOffsetDispersionIn(), 2)
                              / getStimDivisorDispersionIn()
                              - getStimCoxExpDispersionOut()
                              * pow(vehOutDispersionMeasure - getStimOffsetDispersionOut(), 2)
                              / getStimDivisorDispersionOut()

                          );
        return stimulus;
    }
}

double MSSOTLPolicy5DStimulus::computeDesirability(double vehInMeasure, double vehOutMeasure) {

    return computeDesirability(vehInMeasure, vehOutMeasure, 0, 0);
}
std::string  MSSOTLPolicy5DStimulus::getMessage() {
    std::ostringstream _str;
    _str << " stimCox " << getStimCox()
         << " StimOffsetIn " << getStimOffsetIn()
         << " StimOffsetOut " << getStimOffsetOut()
         << " StimDivisorIn " << getStimDivisorIn()
         << " StimDivisorOut " << getStimDivisorOut()
         << " StimCoxExpIn " << getStimCoxExpIn()
         << " StimCoxExpOut " << getStimCoxExpOut()
         << " .";
    return _str.str();
}
