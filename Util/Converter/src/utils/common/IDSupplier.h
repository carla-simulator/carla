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
/// @file    IDSupplier.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @date    Sept 2002
///
// A class that generates enumerated and prefixed string-ids
/****************************************************************************/
#pragma once
#include <string>
#include <vector>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class IDSupplier
 * This class builds string ids by adding an increasing numerical value to a
 * previously given string
 */
class IDSupplier {
public:
    /// Constructor
    IDSupplier(const std::string& prefix = "", long long int begin = 0);

    /** @brief Constructor
     * @param[in] prefix The string to use as ID prefix
     * @param[in] knownIDs List of IDs that should never be returned by this
     * IDSupplier
     **/
    IDSupplier(const std::string& prefix, const std::vector<std::string>& knownIDs);

    /// Destructor
    ~IDSupplier();

    /// Returns the next id
    std::string getNext();

    /// make sure that the given id is never supplied
    void avoid(const std::string& id);

private:
    /// The current index
    long long int myCurrent;

    /// The prefix to use
    std::string myPrefix;

};
