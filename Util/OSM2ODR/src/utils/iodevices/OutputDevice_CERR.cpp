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
/// @file    OutputDevice_CERR.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    2004
///
// An output device that encapsulates cout
/****************************************************************************/
#include <config.h>

#include <iostream>
#include "OutputDevice_CERR.h"


// ===========================================================================
// static member definitions
// ===========================================================================
OutputDevice* OutputDevice_CERR::myInstance = nullptr;


// ===========================================================================
// static method definitions
// ===========================================================================
OutputDevice*
OutputDevice_CERR::getDevice() {
    // check whether the device has already been aqcuired
    if (myInstance == nullptr) {
        myInstance = new OutputDevice_CERR();
    }
    return myInstance;
}


// ===========================================================================
// method definitions
// ===========================================================================
OutputDevice_CERR::OutputDevice_CERR() {
    myFilename = "CERR";
}


OutputDevice_CERR::~OutputDevice_CERR() {
    myInstance = nullptr;
}


std::ostream&
OutputDevice_CERR::getOStream() {
    return std::cerr;
}


void
OutputDevice_CERR::postWriteHook() {
    std::cerr.flush();
}


/****************************************************************************/
