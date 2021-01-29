/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
// activitygen module
// Copyright 2010 TUM (Technische Universitaet Muenchen, http://www.tum.de/)
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
/// @file    AGBus.h
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Walter Bamberger
/// @date    July 2010
///
// A bus driving in the city
/****************************************************************************/
#pragma once
#include <config.h>

#include <iostream>
#include <string>


// ===========================================================================
// class definitions
// ===========================================================================
class AGBus {
public:
    AGBus(std::string name, int depTime) :
        myName(name),
        myDepartureTime(depTime) {};
    AGBus(int depTime) :
        myDepartureTime(depTime) {};
    void setName(std::string name);
    int getDeparture();
    std::string getName();
    void print() const;

private:
    std::string myName;
    int myDepartureTime;
};
