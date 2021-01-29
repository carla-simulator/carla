/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2016-2020 German Aerospace Center (DLR) and others.
// PHEMlight module
// Copyright 2016 Technische Universitaet Graz, https://www.tugraz.at/
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
/// @file    Helpers.cpp
/// @author  Martin Dippold
/// @author  Michael Behrisch
/// @date    July 2016
///
//
/****************************************************************************/


#include "Helpers.h"
#include "Constants.h"


namespace PHEMlightdll {

    const std::string& Helpers::getvClass() const {
        return _vClass;
    }

    void Helpers::setvClass(const std::string& value) {
        _vClass = value;
    }

    const std::string& Helpers::geteClass() const {
        return _eClass;
    }

    void Helpers::seteClass(const std::string& value) {
        _eClass = value;
    }

    const std::string& Helpers::gettClass() const {
        return _tClass;
    }

    void Helpers::settClass(const std::string& value) {
        _tClass = value;
    }

    const std::string& Helpers::getsClass() const {
        return _sClass;
    }

    void Helpers::setsClass(const std::string& value) {
        _sClass = value;
    }

    const std::string& Helpers::getgClass() const {
        return _Class;
    }

    void Helpers::setgClass(const std::string& value) {
        _Class = value;
    }

    const std::string& Helpers::getErrMsg() const {
        return _ErrMsg;
    }

    void Helpers::setErrMsg(const std::string& value) {
        _ErrMsg = value;
    }

    const std::string& Helpers::getCommentPrefix() const {
        return _commentPrefix;
    }

    void Helpers::setCommentPrefix(const std::string& value) {
        _commentPrefix = value;
    }

    const std::string& Helpers::getPHEMDataV() const {
        return _PHEMDataV;
    }

    void Helpers::setPHEMDataV(const std::string& value) {
        _PHEMDataV = value;
    }

    bool Helpers::getvclass(const std::string& VEH) {
        // Set the drive train efficency
        Constants::setDRIVE_TRAIN_EFFICIENCY(Constants::DRIVE_TRAIN_EFFICIENCY_All);

        //Get the vehicle class
        if (VEH.find(Constants::strPKW) != std::string::npos) {
            _vClass = Constants::strPKW;
            return true;
        }
        else if (VEH.find(Constants::strLNF) != std::string::npos) {
            _vClass = Constants::strLNF;
            return true;
        }
        else if (VEH.find(Constants::strLKW) != std::string::npos) {
            _vClass = Constants::strLKW;
            return true;
        }
        else if (VEH.find(Constants::strLSZ) != std::string::npos) {
            _vClass = Constants::strLSZ;
            return true;
        }
        else if (VEH.find(Constants::strRB) != std::string::npos) {
            _vClass = Constants::strRB;
            return true;
        }
        else if (VEH.find(Constants::strLB) != std::string::npos) {
            _vClass = Constants::strLB;
            Constants::setDRIVE_TRAIN_EFFICIENCY(Constants::DRIVE_TRAIN_EFFICIENCY_CB);
            return true;
        }
        else if (VEH.find(Constants::strMR2) != std::string::npos) {
            _vClass = Constants::strMR2;
            return true;
        }
        else if (VEH.find(Constants::strMR4) != std::string::npos) {
            _vClass = Constants::strMR4;
            return true;
        }
        else if (VEH.find(Constants::strKKR) != std::string::npos) {
            _vClass = Constants::strKKR;
            return true;
        }
        //Should never happens
        _ErrMsg = std::string("Vehicle class not defined! (") + VEH + std::string(")");
        return false;
    }

    bool Helpers::gettclass(const std::string& VEH) {
        if ((int)VEH.find(std::string("_") + Constants::strDiesel) > 0) {
            if ((int)VEH.find(std::string("_") + Constants::strHybrid) > 0) {
                _tClass = Constants::strDiesel + std::string("_") + Constants::strHybrid;
                return true;
            }
            else {
                _tClass = Constants::strDiesel;
                return true;
            }

        }
        else if ((int)VEH.find(std::string("_") + Constants::strGasoline) > 0) {
            if ((int)VEH.find(std::string("_") + Constants::strHybrid) > 0) {
                _tClass = Constants::strGasoline + std::string("_") + Constants::strHybrid;
                return true;
            }
            else {
                _tClass = Constants::strGasoline;
                return true;
            }
        }
        else if ((int)VEH.find(std::string("_") + Constants::strCNG) > 0) {
            _tClass = Constants::strCNG;
            return true;
        }
        else if ((int)VEH.find(std::string("_") + Constants::strBEV) > 0) {
            _tClass = Constants::strBEV;
            return true;
        }
        //Should never happens
        _ErrMsg = std::string("Fuel class not defined! (") + VEH + std::string(")");
        return false;
    }

    bool Helpers::getsclass(const std::string& VEH) {
        if (VEH.find(Constants::strLKW) != std::string::npos) {
            if ((int)VEH.find(std::string("_") + Constants::strSII) > 0) {
                _sClass = Constants::strSII;
                return true;
            }
            else if ((int)VEH.find(std::string("_") + Constants::strSI) > 0) {
                _sClass = Constants::strSI;
                return true;
            }
            else {
                //Should never happen
                _ErrMsg = std::string("Size class not defined! (") + VEH + std::string(")");
                return false;
            }
        }
        else if (VEH.find(Constants::strLNF) != std::string::npos) {
            if ((int)VEH.find(std::string("_") + Constants::strSIII) > 0) {
                _sClass = Constants::strSIII;
                return true;
            }
            else if ((int)VEH.find(std::string("_") + Constants::strSII) > 0) {
                _sClass = Constants::strSII;
                return true;
            }
            else if ((int)VEH.find(std::string("_") + Constants::strSI) > 0) {
                _sClass = Constants::strSI;
                return true;
            }
            else {
                _ErrMsg = std::string("Size class not defined! (") + VEH.substr((int)VEH.rfind("\\"), VEH.length() - (int)VEH.rfind("\\")) + std::string(")");
                return false;
            }
        }
        else {
            _sClass = "";
            return true;
        }
    }

    bool Helpers::geteclass(const std::string& VEH) {
        if ((int)VEH.find(std::string("_") + Constants::strEU) > 0) {
            if ((int)VEH.find("_", (int)VEH.find(std::string("_") + Constants::strEU) + 1) > 0) {
                _eClass = Constants::strEU + VEH.substr((int)VEH.find(std::string("_") + Constants::strEU) + 3, (int)VEH.find("_", (int)VEH.find(std::string("_") + Constants::strEU) + 1) - ((int)VEH.find(std::string("_") + Constants::strEU) + 3));
                return true;
            }
            else if ((int)VEH.find(".", (int)VEH.find(std::string("_") + Constants::strEU) + 1) > 0) {
                _eClass = Constants::strEU + VEH.substr((int)VEH.find(std::string("_") + Constants::strEU) + 3, (int)VEH.find(".", (int)VEH.find(std::string("_") + Constants::strEU) + 1) - ((int)VEH.find(std::string("_") + Constants::strEU) + 3));
                return true;
            }
            else {
                _eClass = Constants::strEU + VEH.substr((int)VEH.find(std::string("_") + Constants::strEU) + 3, VEH.length() - ((int)VEH.find(std::string("_") + Constants::strEU) + 3));
                return true;
            }
        }
        else if ((int)VEH.find(std::string("_") + Constants::strBEV) > 0) {
            _eClass = "";
            return true;
        }
        //Should never happens
        _ErrMsg = std::string("Euro class not defined! (") + VEH + std::string(")");
        return false;
    }

    bool Helpers::setclass(const std::string& VEH) {
        if (getvclass(VEH)) {
            _Class = _vClass;
        }
        else {
            return false;
        }
        if (getsclass(VEH)) {
            if (_sClass != "") {
                _Class = _Class + std::string("_") + getsClass();
            }
        }
        else {
            return false;
        }
        if (gettclass(VEH)) {
            _Class = _Class + std::string("_") + gettClass();
        }
        else {
            return false;
        }
        if (geteclass(VEH)) {
            if (_eClass != "") {
                _Class = _Class + std::string("_") + geteClass();
            }
        }
        else {
            return false;
        }
        return true;
    }
}
