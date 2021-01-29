/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2010-2020 German Aerospace Center (DLR) and others.
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
/// @file    AGCar.h
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Walter Bamberger
/// @author  Michael Behrisch
/// @date    July 2010
///
// Cars owned by people of the city: included in households.
/****************************************************************************/
#pragma once
#include <config.h>

#include <iostream>
#include <string>
#include "AGAdult.h"


// ===========================================================================
// class definitions
// ===========================================================================
class AGCar {
public:
    AGCar(std::string name) :
        idName(name) {};
    AGCar(int idHH, int idCar) :
        idName(createName(idHH, idCar)) {};
    bool associateTo(AGAdult* pers);
    bool isAssociated() const;
    std::string getName() const;

private:
    std::string createName(int idHH, int idCar);

    std::string idName;
    AGAdult* currentUser;

};
