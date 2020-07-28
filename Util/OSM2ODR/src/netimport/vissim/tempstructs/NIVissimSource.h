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
/// @file    NIVissimSource.h
/// @author  Daniel Krajzewicz
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
class NIVissimSource {
public:
    NIVissimSource(const std::string& id, const std::string& name,
                   const std::string& edgeid);
    ~NIVissimSource();
    static bool dictionary(const std::string& id, const std::string& name,
                           const std::string& edgeid);
    static bool dictionary(const std::string& id, NIVissimSource* o);
    static NIVissimSource* dictionary(const std::string& id);
    static void clearDict();
private:
    std::string myID;
    std::string myName;
    std::string myEdgeID;

private:
    typedef std::map<std::string, NIVissimSource*> DictType;
    static DictType myDict;
};
