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
/// @file    AGChild.h
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Walter Bamberger
/// @author  Michael Behrisch
/// @date    July 2010
///
// Person in age to go to school: linked to a school object
/****************************************************************************/
#pragma once
#include <config.h>

#include <iostream>
#include <vector>
#include "AGPerson.h"
#include "AGPosition.h"
#include "AGSchool.h"


// ===========================================================================
// class definitions
// ===========================================================================
class AGChild : public AGPerson {
public:
    AGChild(int age) :
        AGPerson(age),
        mySchool(nullptr) {};
    void print() const;
    bool setSchool(AGSchool* school);
    /**
     * @param schools: school vector from City object
     * @param housepos: Position of the households habitation
     * @return if a school was found corresponding to the child's age.
     */
    bool allocateASchool(std::list<AGSchool>* schools, AGPosition housePos);
    /**
     * @return if the child is now without any school
     */
    bool leaveSchool();
    bool haveASchool() const;
    AGPosition getSchoolLocation() const;
    int getSchoolOpening() const;
    int getSchoolClosing() const;

private:
    AGSchool* mySchool;
};
