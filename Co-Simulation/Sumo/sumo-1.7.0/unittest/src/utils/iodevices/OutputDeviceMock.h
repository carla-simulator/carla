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
/// @file    OutputDeviceMock.h
/// @author  Matthias Heppner
/// @author  Michael Behrisch
/// @date    2009-11-23
///
//
/****************************************************************************/

#ifndef OutputDeviceMock_h
#define OutputDeviceMock_h


// ===========================================================================
// included modules
// ===========================================================================
#include <config.h>

#include <fstream>
#include <utils/iodevices/OutputDevice.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class OutputDeviceMock
 * Mock Implementation for Unit Tests
 *
 */
class OutputDeviceMock : public OutputDevice {
public:
    /** @brief Constructor
     */
    OutputDeviceMock() {}

    /// @brief Destructor
    ~OutputDeviceMock()  {}


    /** @brief Returns the current content as a string
     */
    std::string getString()  {
        return myStream.str();
    }

protected:
    /** @brief Returns the associated ostream
     */
    std::ostream& getOStream()  {
        return myStream;
    }

private:
    /// the string stream
    std::ostringstream myStream;

};


#endif

/****************************************************************************/

