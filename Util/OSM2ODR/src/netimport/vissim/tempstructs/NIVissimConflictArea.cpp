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
/// @file    NIVissimConflictArea.cpp
/// @author  Lukas Grohmann
/// @date    Aug 2015
///
// A temporary storage for conflict areas imported from Vissim
/****************************************************************************/
#include <config.h>

#include <iterator>
#include <map>
#include <string>
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include "NIVissimConflictArea.h"
#include "NIVissimConnection.h"
#include <netbuild/NBEdgeCont.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBNode.h>


// ===========================================================================
// static members
// ===========================================================================
NIVissimConflictArea::DictType NIVissimConflictArea::myDict;


// ===========================================================================
// method definitions
// ===========================================================================
NIVissimConflictArea::NIVissimConflictArea(int id,
        const std::string& link1,
        const std::string& link2,
        const std::string& status)
    : myConflictID(id), myFirstLink(link1), mySecondLink(link2), myStatus(status) {
}


NIVissimConflictArea::~NIVissimConflictArea() {}




bool
NIVissimConflictArea::dictionary(int id, const std::string& link1,
                                 const std::string& link2,
                                 const std::string& status) {
    NIVissimConflictArea* ca = new NIVissimConflictArea(id, link1, link2, status);
    if (!dictionary(id, ca)) {
        delete ca;
        return false;
    }
    return true;
}



bool
NIVissimConflictArea::dictionary(int id, NIVissimConflictArea* ca) {
    DictType::iterator i = myDict.find(id);
    if (i == myDict.end()) {
        myDict[id] = ca;
        return true;
    }
    return false;
}



NIVissimConflictArea*
NIVissimConflictArea::dictionary(int id) {
    DictType::iterator i = myDict.find(id);
    if (i == myDict.end()) {
        return nullptr;
    }
    return (*i).second;
}



NIVissimConflictArea*
NIVissimConflictArea::dict_findByLinks(const std::string& link1,
                                       const std::string& link2) {
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        if (((*i).second->myFirstLink == link1) &&
                ((*i).second->mySecondLink == link2)) {
            return (*i).second;
        }
    }
    return nullptr;
}


void
NIVissimConflictArea::clearDict() {
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        delete (*i).second;
    }
    myDict.clear();
}


void
NIVissimConflictArea::setPriorityRegulation(NBEdgeCont& ec) {
    std::map<int, NIVissimConflictArea*>::iterator it;
    for (it = myDict.begin(); it != myDict.end(); it++) {
        NIVissimConflictArea* const conflictArea = it->second;
        NIVissimConnection* const firstLink = NIVissimConnection::dictionary(StringUtils::toInt(conflictArea->getFirstLink()));
        NIVissimConnection* const secondLink = NIVissimConnection::dictionary(StringUtils::toInt(conflictArea->getSecondLink()));
        if (firstLink == nullptr || secondLink == nullptr) {
            continue;
        }
        // status == "TWOYIELDSONE"
        NIVissimConnection* priority_conn = firstLink;
        NIVissimConnection* subordinate_conn = secondLink;
        if (conflictArea->getStatus() == "ONEYIELDSTWO") {
            priority_conn = secondLink;
            subordinate_conn = firstLink;
        }
        const std::string mayDriveFrom_id = toString<int>(priority_conn->getFromEdgeID());
        const std::string mayDriveTo_id = toString<int>(priority_conn->getToEdgeID());
        const std::string mustStopFrom_id = toString<int>(subordinate_conn->getFromEdgeID());
        const std::string mustStopTo_id = toString<int>(subordinate_conn->getToEdgeID());

        NBEdge* const mayDriveFrom =  ec.retrievePossiblySplit(mayDriveFrom_id, true);
        NBEdge* const mayDriveTo =  ec.retrievePossiblySplit(mayDriveTo_id, false);
        NBEdge* const mustStopFrom =  ec.retrievePossiblySplit(mustStopFrom_id, true);
        NBEdge* const mustStopTo =  ec.retrievePossiblySplit(mustStopTo_id, false);

        if (mayDriveFrom != nullptr && mayDriveTo != nullptr && mustStopFrom != nullptr && mustStopTo != nullptr) {
            NBNode* node = mayDriveFrom->getToNode();
            node->addSortedLinkFoes(
                NBConnection(mayDriveFrom, mayDriveTo),
                NBConnection(mustStopFrom, mustStopTo));
        }
    }
}


/****************************************************************************/
