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
/// @file    NIVissimClosures.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// -------------------
/****************************************************************************/
#include <config.h>

#include <string>
#include <utils/common/VectorHelper.h>
#include "NIVissimClosures.h"


NIVissimClosures::DictType NIVissimClosures::myDict;

NIVissimClosures::NIVissimClosures(const std::string& id,
                                   int from_node, int to_node,
                                   std::vector<int>& overEdges)
    : myID(id), myFromNode(from_node), myToNode(to_node),
      myOverEdges(overEdges) {}


NIVissimClosures::~NIVissimClosures() {}


bool
NIVissimClosures::dictionary(const std::string& id,
                             int from_node, int to_node,
                             std::vector<int>& overEdges) {
    NIVissimClosures* o = new NIVissimClosures(id, from_node, to_node,
            overEdges);
    if (!dictionary(id, o)) {
        delete o;
        return false;
    }
    return true;
}


bool
NIVissimClosures::dictionary(const std::string& name, NIVissimClosures* o) {
    DictType::iterator i = myDict.find(name);
    if (i == myDict.end()) {
        myDict[name] = o;
        return true;
    }
    return false;
}


NIVissimClosures*
NIVissimClosures::dictionary(const std::string& name) {
    DictType::iterator i = myDict.find(name);
    if (i == myDict.end()) {
        return nullptr;
    }
    return (*i).second;
}



void
NIVissimClosures::clearDict() {
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        delete (*i).second;
    }
    myDict.clear();
}


/****************************************************************************/
