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
/// @file    OutputDevice_CERR.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    2004
///
// An output device that encapsulates cerr
/****************************************************************************/
#pragma once
#include <config.h>

#include "OutputDevice.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class OutputDevice_CERR
 * @brief An output device that encapsulates cerr
 */
class OutputDevice_CERR : public OutputDevice {
public:
    /** @brief Returns the single cerr instance
     *
     * Creates and returns the cerr device.
     */
    static OutputDevice* getDevice();


protected:
    /// @name Methods that override/implement OutputDevice-methods
    /// @{

    /** @brief Returns the associated ostream
     * @return cerr
     */
    std::ostream& getOStream();


    /** @brief Called after every write access.
     *
     * Calls flush on stderr.
     */
    virtual void postWriteHook();
    /// @}


private:
    /** @brief Constructor
     * @exception IOError Should not be thrown by this implementation
     */
    OutputDevice_CERR();


    /// @brief Destructor
    ~OutputDevice_CERR();


private:
    /// @brief my singular instance
    static OutputDevice* myInstance;


};
