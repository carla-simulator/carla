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
/// @file    HelpersPHEMlight.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Nikolaus Furian
/// @date    Sat, 20.04.2013
///
// Helper methods for PHEMlight-based emission computation
/****************************************************************************/
#include <config.h>

#include <limits>
#include <cmath>
#ifdef INTERNAL_PHEM
#include "PHEMCEPHandler.h"
#include "PHEMConstants.h"
#endif
#include <foreign/PHEMlight/cpp/Constants.h>
#include <utils/options/OptionsCont.h>
#include "HelpersPHEMlight.h"

// idle speed is usually given in rpm (but may depend on electrical consumers). Actual speed depends on the gear so this number is only a rough estimate
#define IDLE_SPEED (10 / 3.6)

// ===========================================================================
// method definitions
// ===========================================================================
HelpersPHEMlight::HelpersPHEMlight() : PollutantsInterface::Helper("PHEMlight"), myIndex(PHEMLIGHT_BASE) {
}


SUMOEmissionClass
HelpersPHEMlight::getClassByName(const std::string& eClass, const SUMOVehicleClass vc) {
    if (eClass == "unknown" && !myEmissionClassStrings.hasString("unknown")) {
        myEmissionClassStrings.addAlias("unknown", getClassByName("PC_G_EU4", vc));
    }
    if (eClass == "default" && !myEmissionClassStrings.hasString("default")) {
        myEmissionClassStrings.addAlias("default", getClassByName("PC_G_EU4", vc));
    }
    if (myEmissionClassStrings.hasString(eClass)) {
        return myEmissionClassStrings.get(eClass);
    }
    if (eClass.size() < 6) {
        throw InvalidArgument("Unknown emission class '" + eClass + "'.");
    }
    int index = myIndex++;
    const std::string type = eClass.substr(0, 3);
    if (type == "HDV" || type == "LB_" || type == "RB_" || type == "LSZ" || eClass.find("LKW") != std::string::npos) {
        index |= PollutantsInterface::HEAVY_BIT;
    }
    myEmissionClassStrings.insert(eClass, index);
#ifdef INTERNAL_PHEM
    if (type == "HDV" || type == "LCV" || type == "PC_" || !PHEMCEPHandler::getHandlerInstance().Load(index, eClass)) {
#endif
        std::vector<std::string> phemPath;
        phemPath.push_back(OptionsCont::getOptions().getString("phemlight-path") + "/");
        if (getenv("PHEMLIGHT_PATH") != nullptr) {
            phemPath.push_back(std::string(getenv("PHEMLIGHT_PATH")) + "/");
        }
        if (getenv("SUMO_HOME") != nullptr) {
            phemPath.push_back(std::string(getenv("SUMO_HOME")) + "/data/emissions/PHEMlight/");
        }
        myHelper.setCommentPrefix("c");
        myHelper.setPHEMDataV("V4");
        myHelper.setclass(eClass);
        if (!myCEPHandler.GetCEP(phemPath, &myHelper)) {
            myEmissionClassStrings.remove(eClass, index);
            myIndex--;
            throw InvalidArgument("File for PHEM emission class " + eClass + " not found.\n" + myHelper.getErrMsg());
        }
        myCEPs[index] = myCEPHandler.getCEPS().find(myHelper.getgClass())->second;
#ifdef INTERNAL_PHEM
    }
#endif
    std::string eclower = eClass;
    std::transform(eclower.begin(), eclower.end(), eclower.begin(), tolower);
    myEmissionClassStrings.addAlias(eclower, index);
    return index;
}


SUMOEmissionClass
HelpersPHEMlight::getClass(const SUMOEmissionClass base, const std::string& vClass, const std::string& fuel, const std::string& eClass, const double weight) const {
    std::string eClassOffset = "0";
    if (eClass.length() == 5 && eClass.substr(0, 4) == "Euro") {
        if (eClass[4] >= '0' && eClass[4] <= '6') {
            eClassOffset = eClass.substr(4, 1);
        }
    }
    std::string desc;
    if (vClass == "Passenger") {
        desc = "PKW_";
        if (fuel == "Gasoline") {
            desc += "G_";
        } else if (fuel == "Diesel") {
            desc += "D_";
        } else if (fuel == "HybridGasoline") {
            desc = "H_" + desc + "G_";
        } else if (fuel == "HybridDiesel") {
            desc = "H_" + desc + "G_";
        }
        desc += "EU" + eClassOffset;
    } else if (vClass == "Moped") {
        desc = "KKR_G_EU" + eClassOffset;
    } else if (vClass == "Motorcycle") {
        desc = "MR_G_EU" + eClassOffset;
        if (fuel == "Gasoline2S") {
            desc += "_2T";
        } else {
            desc += "_4T";
        }
    } else if (vClass == "Delivery") {
        desc = "LNF_";
        if (fuel == "Gasoline") {
            desc += "G_";
        } else if (fuel == "Diesel") {
            desc += "D_";
        }
        desc += "EU" + eClassOffset + "_I";
        if (weight > 1305.) {
            desc += "I";
            if (weight > 1760.) {
                desc += "I";
            }
        }
    } else if (vClass == "UrbanBus") {
        desc = "LB_D_EU" + eClassOffset;
    } else if (vClass == "Coach") {
        desc = "RB_D_EU" + eClassOffset;
    } else if (vClass == "Truck") {
        desc = "Solo_LKW_D_EU" + eClassOffset + "_I";
        if (weight > 1305.) {
            desc += "I";
        }
    } else if (vClass == "Trailer") {
        desc = "LSZ_D_EU" + eClassOffset;
    }
    if (myEmissionClassStrings.hasString(desc)) {
        return myEmissionClassStrings.get(desc);
    }
    return base;
}


std::string
HelpersPHEMlight::getAmitranVehicleClass(const SUMOEmissionClass c) const {
    const std::string name = myEmissionClassStrings.getString(c);
    if (name.find("KKR_") != std::string::npos) {
        return "Moped";
    } else if (name.find("RB_") != std::string::npos) {
        return "Coach";
    } else if (name.find("LB_") != std::string::npos) {
        return "UrbanBus";
    } else if (name.find("LNF_") != std::string::npos) {
        return "Delivery";
    } else if (name.find("LSZ_") != std::string::npos) {
        return "Trailer";
    } else if (name.find("MR_") != std::string::npos) {
        return "Motorcycle";
    } else if (name.find("LKW_") != std::string::npos) {
        return "Truck";
    }
    return "Passenger";
}


std::string
HelpersPHEMlight::getFuel(const SUMOEmissionClass c) const {
    const std::string name = myEmissionClassStrings.getString(c);
    std::string fuel = "Gasoline";
    if (name.find("_D_") != std::string::npos) {
        fuel = "Diesel";
    }
    if (name.find("H_") != std::string::npos) {
        fuel = "Hybrid" + fuel;
    }
    return fuel;
}


int
HelpersPHEMlight::getEuroClass(const SUMOEmissionClass c) const {
    const std::string name = myEmissionClassStrings.getString(c);
    if (name.find("_EU1") != std::string::npos) {
        return 1;
    } else if (name.find("_EU2") != std::string::npos) {
        return 2;
    } else if (name.find("_EU3") != std::string::npos) {
        return 3;
    } else if (name.find("_EU4") != std::string::npos) {
        return 4;
    } else if (name.find("_EU5") != std::string::npos) {
        return 5;
    } else if (name.find("_EU6") != std::string::npos) {
        return 6;
    }
    return 0;
}


double
HelpersPHEMlight::getWeight(const SUMOEmissionClass c) const {
    const std::string name = myEmissionClassStrings.getString(c);
    if (name.find("LNF_") != std::string::npos) {
        if (name.find("_III") != std::string::npos) {
            return 2630.;
        } else if (name.find("_II") != std::string::npos) {
            return 1532.;
        } else if (name.find("_I") != std::string::npos) {
            return 652.;
        }
    }
    if (name.find("Solo_LKW_") != std::string::npos) {
        if (name.find("_II") != std::string::npos) {
            return 8398.;
        } else if (name.find("_I") != std::string::npos) {
            return 18702.;
        }
    }
    return -1.;
}


double
HelpersPHEMlight::getEmission(const PHEMCEP* oldCep, PHEMlightdll::CEP* currCep, const std::string& e, const double p, const double v) const {
    if (oldCep != nullptr) {
        return oldCep->GetEmission(e, p, v);
    }
    return currCep->GetEmission(e, p, v, &myHelper);
}


double
HelpersPHEMlight::getModifiedAccel(const SUMOEmissionClass c, const double v, const double a, const double slope) const {
    PHEMlightdll::CEP* currCep = myCEPs.count(c) == 0 ? 0 : myCEPs.find(c)->second;
    if (currCep != nullptr) {
        return v == 0.0 ? 0.0 : MIN2(a, currCep->GetMaxAccel(v, slope));
    }
    return a;
}


double
HelpersPHEMlight::compute(const SUMOEmissionClass c, const PollutantsInterface::EmissionType e, const double v, const double a, const double slope, const std::map<int, double>* /* param */) const {
    const double corrSpeed = MAX2(0.0, v);
    double power = 0.;
#ifdef INTERNAL_PHEM
    const PHEMCEP* const oldCep = PHEMCEPHandler::getHandlerInstance().GetCep(c);
    if (oldCep != nullptr) {
        if (v > IDLE_SPEED && a < oldCep->GetDecelCoast(corrSpeed, a, slope, 0)) {
            // coasting without power use only works if the engine runs above idle speed and
            // the vehicle does not accelerate beyond friction losses
            return 0;
        }
        power = oldCep->CalcPower(corrSpeed, a, slope);
    }
#else
    const PHEMCEP* const oldCep = 0;
#endif
    PHEMlightdll::CEP* currCep = myCEPs.count(c) == 0 ? 0 : myCEPs.find(c)->second;
    if (currCep != nullptr) {
        const double corrAcc = getModifiedAccel(c, corrSpeed, a, slope);
        if (currCep->getFuelType() != PHEMlightdll::Constants::strBEV && corrAcc < currCep->GetDecelCoast(corrSpeed, corrAcc, slope) && corrSpeed > PHEMlightdll::Constants::ZERO_SPEED_ACCURACY) {
            // the IDLE_SPEED fix above is now directly in the decel coast calculation.
            return 0;
        }
        power = currCep->CalcPower(corrSpeed, corrAcc, slope);
    }
    const std::string& fuelType = oldCep != nullptr ? oldCep->GetVehicleFuelType() : currCep->getFuelType();
    switch (e) {
        case PollutantsInterface::CO:
            return getEmission(oldCep, currCep, "CO", power, corrSpeed) / SECONDS_PER_HOUR * 1000.;
        case PollutantsInterface::CO2:
            if (oldCep != nullptr) {
                return getEmission(oldCep, currCep, "FC", power, corrSpeed) * 3.15 / SECONDS_PER_HOUR * 1000.;
            }
            return currCep->GetCO2Emission(getEmission(nullptr, currCep, "FC", power, corrSpeed),
                                           getEmission(nullptr, currCep, "CO", power, corrSpeed),
                                           getEmission(nullptr, currCep, "HC", power, corrSpeed), &myHelper) / SECONDS_PER_HOUR * 1000.;
        case PollutantsInterface::HC:
            return getEmission(oldCep, currCep, "HC", power, corrSpeed) / SECONDS_PER_HOUR * 1000.;
        case PollutantsInterface::NO_X:
            return getEmission(oldCep, currCep, "NOx", power, corrSpeed) / SECONDS_PER_HOUR * 1000.;
        case PollutantsInterface::PM_X:
            return getEmission(oldCep, currCep, "PM", power, corrSpeed) / SECONDS_PER_HOUR * 1000.;
        case PollutantsInterface::FUEL: {
            if (fuelType == PHEMlightdll::Constants::strDiesel) { // divide by average diesel density of 836 g/l
                return getEmission(oldCep, currCep, "FC", power, corrSpeed) / 836. / SECONDS_PER_HOUR * 1000.;
            } else if (fuelType == PHEMlightdll::Constants::strGasoline) { // divide by average gasoline density of 742 g/l
                return getEmission(oldCep, currCep, "FC", power, corrSpeed) / 742. / SECONDS_PER_HOUR * 1000.;
            } else if (fuelType == PHEMlightdll::Constants::strBEV) {
                return 0;
            } else {
                return getEmission(oldCep, currCep, "FC", power, corrSpeed) / SECONDS_PER_HOUR * 1000.; // surely false, but at least not additionally modified
            }
        }
        case PollutantsInterface::ELEC:
            if (fuelType == PHEMlightdll::Constants::strBEV) {
                return getEmission(oldCep, currCep, "FC", power, corrSpeed) / SECONDS_PER_HOUR * 1000.;
            }
            return 0;
    }
    // should never get here
    return 0.;
}


/****************************************************************************/
