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
/// @file    DistributionCont.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
///
// A container for distributions
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <utils/distribution/Distribution.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class DistributionCont
 * A container for distributions of different type.
 */
class DistributionCont {
public:
    /// Adds a distribution of the given type and name to the container
    static bool dictionary(const std::string& type, const std::string& id,
                           Distribution* d);

    /// retrieves the distribution described by a type and a name from the container
    static Distribution* dictionary(const std::string& type,
                                    const std::string& id);

    /// delete all stored distributions
    static void clear();

private:
    /// Definition of a map from distribution ids to distributions
    typedef std::map<std::string, Distribution*> DistDict;

    /// Definition of a map from distribution types to distribution ids to distributions
    typedef std::map<std::string, DistDict> TypedDistDict;

    /// Map from distribution types to distribution ids to distributions
    static TypedDistDict myDict;

};
