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
/// @file    MSSOTLPolicy5DFamilyStimulus.cpp
/// @author  Riccardo Belletti
/// @author  Simone Bacchilega
/// @date    2014-09-30
///
// The class for Swarm-based low-level policy
/****************************************************************************/
#include <config.h>

#include <utils/common/StringTokenizer.h>
#include "MSSOTLPolicy5DFamilyStimulus.h"


// ===========================================================================
// method definitions
// ===========================================================================
MSSOTLPolicy5DFamilyStimulus::MSSOTLPolicy5DFamilyStimulus(std::string keyPrefix,
        const std::map<std::string, std::string>& parameters) :
    MSSOTLPolicyDesirability(keyPrefix, parameters) {

    default_values["_STIM_COX"] = "1";
    default_values["_STIM_OFFSET_IN"] = "1";
    default_values["_STIM_OFFSET_OUT"] = "1";
    default_values["_STIM_OFFSET_DISPERSION_IN"] = "1";
    default_values["_STIM_OFFSET_DISPERSION_OUT"] = "1";
    default_values["_STIM_DIVISOR_IN"] = "1";
    default_values["_STIM_DIVISOR_OUT"] = "1";
    default_values["_STIM_DIVISOR_DISPERSION_IN"] = "1";
    default_values["_STIM_DIVISOR_DISPERSION_OUT"] = "1";
    default_values["_STIM_COX_EXP_IN"] = "0";
    default_values["_STIM_COX_EXP_OUT"] = "0";
    default_values["_STIM_COX_EXP_DISPERSION_IN"] = "0";
    default_values["_STIM_COX_EXP_DISPERSION_OUT"] = "0";

    params_names.push_back("_STIM_COX");
    params_names.push_back("_STIM_OFFSET_IN");
    params_names.push_back("_STIM_OFFSET_OUT");
    params_names.push_back("_STIM_OFFSET_DISPERSION_IN");
    params_names.push_back("_STIM_OFFSET_DISPERSION_OUT");
    params_names.push_back("_STIM_DIVISOR_IN");
    params_names.push_back("_STIM_DIVISOR_OUT");
    params_names.push_back("_STIM_DIVISOR_DISPERSION_IN");
    params_names.push_back("_STIM_DIVISOR_DISPERSION_OUT");
    params_names.push_back("_STIM_COX_EXP_IN");
    params_names.push_back("_STIM_COX_EXP_OUT");
    params_names.push_back("_STIM_COX_EXP_DISPERSION_IN");
    params_names.push_back("_STIM_COX_EXP_DISPERSION_OUT");


    int size_family = int(getDouble(keyPrefix + "_SIZE_FAMILY", 1));
    DBG(

        std::ostringstream str;
        str << keyPrefix << "\n" << "size fam" << size_family;
        WRITE_MESSAGE(str.str());
    )

    std::vector< std::map <std::string, std::string > > sliced_maps;

    for (int i = 0; i < size_family; i++) {
        sliced_maps.push_back(std::map<std::string, std::string>());
    }

    //For each param list, slice values
    for (int i = 0; i < (int)params_names.size(); i ++) {
        std::string key = keyPrefix + params_names[i];
        std::string param_list = getParameter(key, default_values[params_names[i]]);
        std::vector<std::string> tokens = StringTokenizer(param_list, ";").getVector();

        for (int token_counter = 0; token_counter < size_family; ++token_counter) {
            if (token_counter >= (int)tokens.size()) {
                std::ostringstream errorMessage;
                errorMessage << "Error in " << key << ": not enough tokens.";
                WRITE_ERROR(errorMessage.str());
                assert(-1);
            }
            DBG(
                std::ostringstream str;
                str << "found token " << tokens[token_counter] << " position " << token_counter;
                WRITE_MESSAGE(str.str());
            )
            sliced_maps[token_counter][key] = tokens[token_counter];
        }
    }

    for (int i = 0; i < size_family; i++) {
        std::map<std::string, std::string>& ref_map = sliced_maps[i];
        family.push_back(new MSSOTLPolicy5DStimulus(keyPrefix, ref_map));
    }

}


double MSSOTLPolicy5DFamilyStimulus::computeDesirability(double vehInMeasure, double vehOutMeasure, double vehInDispersionMeasure, double vehOutDispersionMeasure) {
    /*DBG(
    		std::ostringstream str;
    		str << "cox=" << getStimCox() << ", cox_exp_in=" << getStimCoxExpIn() << ", cox_exp_out=" << getStimCoxExpOut()
    				<< ", off_in=" << getStimOffsetIn() << ", off_out=" << getStimOffsetOut() << ", div_in=" << getStimDivisorIn() << ", div_out=" << getStimDivisorOut(); WRITE_MESSAGE(str.str());)
     */
    //		it seems to be not enough, a strange segmentation fault appears...
    //	 if((getStimCoxExpIn()!=0.0 && getStimDivisorIn()==0.0)||(getStimCoxExpOut()!=0.0 && getStimDivisorOut()==0.0)){

    double best_stimulus = -1;
    for (std::vector<MSSOTLPolicy5DStimulus*>::const_iterator it  = family.begin(); it != family.end(); it++) {
        double temp_stimulus = (*it)->computeDesirability(vehInMeasure, vehOutMeasure, vehInDispersionMeasure, vehOutDispersionMeasure);
        DBG(
            std::ostringstream str;
            str << "STIMULUS: " << temp_stimulus;
            WRITE_MESSAGE(str.str());
        )
        if (temp_stimulus > best_stimulus) {
            best_stimulus = temp_stimulus;
        }
    }

    DBG(
        std::ostringstream str;
        str << "BEST STIMULUS: " << best_stimulus;
        WRITE_MESSAGE(str.str());
    )
    return best_stimulus;
}


double MSSOTLPolicy5DFamilyStimulus::computeDesirability(double vehInMeasure, double vehOutMeasure) {

    return computeDesirability(vehInMeasure, vehOutMeasure, 0, 0);
}

std::string MSSOTLPolicy5DFamilyStimulus::getMessage() {
    std::ostringstream ot;
    for (int i = 0; i < (int)family.size(); i++) {
        ot << " gaussian " << i << ":" << family[i]->getMessage();
    }
    return ot.str();
}

/*
std::vector<std::string> inline MSSOTLPolicy5DFamilyStimulus::StringSplit(const std::string &source, const char *delimiter = " ", bool keepEmpty = false)
{
    std::vector<std::string> results;

    int prev = 0;
    std::string::size_type next = 0;

    while ((next = source.find_first_of(delimiter, prev)) != std::string::npos)
    {
        if (keepEmpty || (next - prev != 0))
        {
            results.push_back(source.substr(prev, next - prev));
        }
        prev = next + 1;
    }

    if (prev < source.size())
    {
        results.push_back(source.substr(prev));
    }

    return results;
}
*/
