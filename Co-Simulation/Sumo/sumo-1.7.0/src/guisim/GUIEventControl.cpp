/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
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
/// @file    GUIEventControl.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Mon, 04 Feb 2008
///
// Stores time-dependant events and executes them at the proper time (guisim)
/****************************************************************************/
#include <config.h>

#include <cassert>
#include <fx.h>
#include "GUIEventControl.h"


// ===========================================================================
// member definitions
// ===========================================================================
GUIEventControl::GUIEventControl() :
    myLock(true)
{}


GUIEventControl::~GUIEventControl() {
}


void
GUIEventControl::addEvent(Command* operation, SUMOTime execTimeStep) {
    FXMutexLock locker(myLock);
    MSEventControl::addEvent(operation, execTimeStep);
}


void
GUIEventControl::execute(SUMOTime execTime) {
    FXMutexLock locker(myLock);
    MSEventControl::execute(execTime);
}


/****************************************************************************/
