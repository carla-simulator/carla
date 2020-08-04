/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    NIVissimClosures.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// -------------------
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 *
 */
class NIVissimClosures {
public:
    NIVissimClosures(const std::string& id,
                     int from_node, int to_node,
                     std::vector<int>& overEdges);
    ~NIVissimClosures();
    static bool dictionary(const std::string& id,
                           int from_node, int to_node, std::vector<int>& overEdges);
    static bool dictionary(const std::string& name, NIVissimClosures* o);
    static NIVissimClosures* dictionary(const std::string& name);
    static void clearDict();
private:
    typedef std::map<std::string, NIVissimClosures*> DictType;
    static DictType myDict;
    const std::string myID;
    int myFromNode, myToNode;
    std::vector<int> myOverEdges;

private:
    /// @brief invalidated copy constructor
    NIVissimClosures(const NIVissimClosures& s);

    /// @brief invalidated assignment operator
    NIVissimClosures& operator=(const NIVissimClosures& s);


};
