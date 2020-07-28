/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2004-2020 German Aerospace Center (DLR) and others.
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
/// @file    OutputDevice_COUT.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    2004
///
// An output device that encapsulates cout
/****************************************************************************/
#include <config.h>

#include <iostream>
#include "OutputDevice_COUT.h"


// ===========================================================================
// static member definitions
// ===========================================================================
OutputDevice* OutputDevice_COUT::myInstance = nullptr;


// ===========================================================================
// static method definitions
// ===========================================================================
OutputDevice*
OutputDevice_COUT::getDevice() {
    // check whether the device has already been aqcuired
    if (myInstance == nullptr) {
        myInstance = new OutputDevice_COUT();
    }
    return myInstance;
}


// ===========================================================================
// method definitions
// ===========================================================================
OutputDevice_COUT::OutputDevice_COUT() {
    myFilename = "COUT";
}


OutputDevice_COUT::~OutputDevice_COUT() {
    myInstance = nullptr;
}


std::ostream&
OutputDevice_COUT::getOStream() {
    return std::cout;
}


void
OutputDevice_COUT::postWriteHook() {
    std::cout.flush();
}


/****************************************************************************/
