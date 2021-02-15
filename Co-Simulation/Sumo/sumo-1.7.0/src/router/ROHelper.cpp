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
/// @file    ROHelper.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// Some helping methods for router
/****************************************************************************/
#include <config.h>

#include <functional>
#include <vector>
#include "ROEdge.h"
#include "ROVehicle.h"
#include "ROHelper.h"


// ===========================================================================
// class definitions
// ===========================================================================


namespace ROHelper {
void
recheckForLoops(ConstROEdgeVector& edges, const ConstROEdgeVector& mandatory) {
    // for simplicities sake, prevent removal of any mandatory edges
    // in theory these edges could occur multiple times so it might be possible
    // to delete some of them anyway.
    // XXX check for departLane, departPos, departSpeed, ....

    // removal of edge loops within the route (edge occurs twice)
    std::map<const ROEdge*, int> lastOccurence; // index of the last occurence of this edge
    for (int ii = 0; ii < (int)edges.size(); ++ii) {
        std::map<const ROEdge*, int>::iterator it_pre = lastOccurence.find(edges[ii]);
        if (it_pre != lastOccurence.end() &&
                noMandatory(mandatory, edges.begin() + it_pre->second, edges.begin() + ii)) {
            edges.erase(edges.begin() + it_pre->second, edges.begin() + ii);
            ii = it_pre->second;
        } else {
            lastOccurence[edges[ii]] = ii;
        }
    }

    // remove loops at the route's begin
    //  (vehicle makes a turnaround to get into the right direction at an already passed node)
    const RONode* start = edges[0]->getFromJunction();
    int lastStart = 0;
    for (int i = 1; i < (int)edges.size(); i++) {
        if (edges[i]->getFromJunction() == start) {
            lastStart = i;
        }
    }
    if (lastStart > 0 && noMandatory(mandatory, edges.begin(), edges.begin() + lastStart - 1)) {
        edges.erase(edges.begin(), edges.begin() + lastStart - 1);
    }
    // remove loops at the route's end
    //  (vehicle makes a turnaround to get into the right direction at an already passed node)
    const RONode* end = edges.back()->getToJunction();
    for (int i = 0; i < (int)edges.size() - 1; i++) {
        if (edges[i]->getToJunction() == end && noMandatory(mandatory, edges.begin() + i + 2, edges.end())) {
            edges.erase(edges.begin() + i + 2, edges.end());
            break;
        }
    }

    // removal of node loops (node occurs twice) is not done because these may occur legitimately
    /*
    std::vector<RONode*> nodes;
    for (ConstROEdgeVector::iterator i = edges.begin(); i != edges.end(); ++i) {
        nodes.push_back((*i)->getFromJunction());
    }
    nodes.push_back(edges.back()->getToJunction());
    bool changed = false;
    do {
        changed = false;
        for (int b = 0; b < nodes.size() && !changed; ++b) {
            RONode* bn = nodes[b];
            for (int e = b + 1; e < nodes.size() && !changed; ++e) {
                if (bn == nodes[e]) {
                    changed = true;
                    nodes.erase(nodes.begin() + b, nodes.begin() + e);
                    edges.erase(edges.begin() + b, edges.begin() + e);
                }
            }
        }
    } while (changed);
    */
}

bool
noMandatory(const ConstROEdgeVector& mandatory,
            ConstROEdgeVector::const_iterator start,
            ConstROEdgeVector::const_iterator end) {
    for (const ROEdge* m : mandatory) {
        for (auto it = start; it != end; it++) {
            if (*it == m) {
                return false;
            }
        }
    }
    return true;
}


}


/****************************************************************************/
