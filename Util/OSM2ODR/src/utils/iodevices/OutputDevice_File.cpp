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
/// @file    OutputDevice_File.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    2004
///
// An output device that encapsulates an ofstream
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <cstring>
#include <cerrno>
#ifdef HAVE_ZLIB
#include <foreign/zstr/zstr.hpp>
#endif
#include <utils/common/UtilExceptions.h>
#include "OutputDevice_File.h"


// ===========================================================================
// method definitions
// ===========================================================================
OutputDevice_File::OutputDevice_File(const std::string& fullName, const bool compressed)
    : OutputDevice(0, fullName), myFileStream(nullptr) {
#ifdef WIN32
    if (fullName == "/dev/null") {
        myFileStream = new std::ofstream("NUL");
        if (!myFileStream->good()) {
            delete myFileStream;
            throw IOError("Could not redirect to NUL device (" + std::string(std::strerror(errno)) + ").");
        }
        return;
    }
#endif
#ifdef HAVE_ZLIB
    if (compressed) {
        try {
            myFileStream = new zstr::ofstream(fullName.c_str(), std::ios_base::out);
        } catch (zstr::Exception& e) {
            throw IOError("Could not build output file '" + fullName + "' (" + e.what() + ").");
        }
    } else {
        myFileStream = new std::ofstream(fullName.c_str(), std::ios_base::out);
    }
#else
    UNUSED_PARAMETER(compressed);
    myFileStream = new std::ofstream(fullName.c_str(), binary ? std::ios::binary : std::ios_base::out);
#endif
    if (!myFileStream->good()) {
        delete myFileStream;
        throw IOError("Could not build output file '" + fullName + "' (" + std::strerror(errno) + ").");
    }
}


OutputDevice_File::~OutputDevice_File() {
    delete myFileStream;
}


std::ostream&
OutputDevice_File::getOStream() {
    return *myFileStream;
}


/****************************************************************************/
