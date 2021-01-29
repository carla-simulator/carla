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
/// @file    Helpers.h
/// @author  Martin Dippold
/// @author  Michael Behrisch
/// @date    July 2016
///
//
/****************************************************************************/


#ifndef PHEMlightHELPERS
#define PHEMlightHELPERS

#include <string>


namespace PHEMlightdll {
    class Helpers {
        //--------------------------------------------------------------------------------------------------
        // Members 
        //--------------------------------------------------------------------------------------------------
    private:
        std::string _vClass;
    public:
        const std::string&  getvClass() const;
        void setvClass(const std::string& value);
    private:
        std::string _eClass;
    public:
        const std::string&  geteClass() const;
        void seteClass(const std::string& value);
    private:
        std::string _tClass;
    public:
        const std::string&  gettClass() const;
        void settClass(const std::string& value);
    private:
        std::string _sClass;
    public:
        const std::string&  getsClass() const;
        void setsClass(const std::string& value);
    private:
        std::string _Class;
    public:
        const std::string&  getgClass() const;
        void setgClass(const std::string& value);
    private:
        std::string _ErrMsg;
    public:
        const std::string&  getErrMsg() const;
        void setErrMsg(const std::string& value);
    private:
        std::string _commentPrefix;
    public:
        const std::string&  getCommentPrefix() const;
        void setCommentPrefix(const std::string& value);
    private:
        std::string _PHEMDataV;
    public:
        const std::string&  getPHEMDataV() const;
        void setPHEMDataV(const std::string& value);

        //Get vehicle class
    private:
        bool getvclass(const std::string& VEH);

        //Get technologie Class
        bool gettclass(const std::string& VEH);

        //Get size class
        bool getsclass(const std::string& VEH);

        //Get euro class
        bool geteclass(const std::string& VEH);

        //Set complete class string
    public:
        bool setclass(const std::string& VEH);


    };
}


#endif	//#ifndef PHEMlightHELPERS
