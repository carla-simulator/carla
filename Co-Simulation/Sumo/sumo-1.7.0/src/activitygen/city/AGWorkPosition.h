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
/// @file    AGWorkPosition.h
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Walter Bamberger
/// @author  Michael Behrisch
/// @date    July 2010
///
// Location and schedules of a work position: linked with one adult
/****************************************************************************/
#pragma once
#include <config.h>

#include "AGPosition.h"
#include <stdexcept>


// ===========================================================================
// class declarations
// ===========================================================================
class AGStreet;
class AGAdult;
class AGDataAndStatistics;


// ===========================================================================
// class definitions
// ===========================================================================
// TODO: Change name to AGWorkPlace?
// TODO: Counter for free work positions should be in City
// TODO: Change name of openingTime to something like startHour or openingHour
class AGWorkPosition {
public:
    AGWorkPosition(AGDataAndStatistics* ds, const AGStreet& inStreet);
    AGWorkPosition(AGDataAndStatistics* ds, const AGStreet& inStreet, double pos);
    ~AGWorkPosition();

    void take(AGAdult* ad);
    void let();
    bool isTaken() const;

    AGPosition getPosition() const;
    int getOpening() const;
    int getClosing() const;

    void print() const;

private:
    static int generateOpeningTime(const AGDataAndStatistics& ds);
    static int generateClosingTime(const AGDataAndStatistics& ds);

private:
    AGDataAndStatistics* myStatData;
    AGPosition myLocation;
    AGAdult* myAdult;
    int myOpeningTime;
    int myClosingTime;
};
