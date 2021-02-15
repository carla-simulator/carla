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
/// @file    RODFRouteCont.cpp
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    Thu, 16.03.2006
///
// A container for routes
/****************************************************************************/
#include <config.h>

#include <fstream>
#include <cassert>
#include "RODFRouteDesc.h"
#include "RODFRouteCont.h"
#include "RODFNet.h"
#include <router/ROEdge.h>
#include <utils/common/ToString.h>
#include <utils/iodevices/OutputDevice.h>


// ===========================================================================
// method definitions
// ===========================================================================
RODFRouteCont::RODFRouteCont() {}


RODFRouteCont::~RODFRouteCont() {
}


void
RODFRouteCont::addRouteDesc(RODFRouteDesc& desc) {
    // routes may be duplicate as in-between routes may have different starting points
    if (find_if(myRoutes.begin(), myRoutes.end(), route_finder(desc)) == myRoutes.end()) {
        // compute route id
        setID(desc);
        myRoutes.push_back(desc);
    } else {
        RODFRouteDesc& prev = *find_if(myRoutes.begin(), myRoutes.end(), route_finder(desc));
        prev.overallProb += desc.overallProb;
    }
}


bool
RODFRouteCont::removeRouteDesc(RODFRouteDesc& desc) {
    std::vector<RODFRouteDesc>::const_iterator j = find_if(myRoutes.begin(), myRoutes.end(), route_finder(desc));
    if (j == myRoutes.end()) {
        return false;
    }
    return true;
}


bool
RODFRouteCont::save(std::vector<std::string>& saved,
                    const std::string& prependix, OutputDevice& out) {
    bool haveSavedOneAtLeast = false;
    for (std::vector<RODFRouteDesc>::const_iterator j = myRoutes.begin(); j != myRoutes.end(); ++j) {
        const RODFRouteDesc& desc = (*j);
        if (find(saved.begin(), saved.end(), desc.routename) != saved.end()) {
            continue;
        }
        saved.push_back((*j).routename);
        assert(desc.edges2Pass.size() >= 1);
        out.openTag(SUMO_TAG_ROUTE).writeAttr(SUMO_ATTR_ID, prependix + desc.routename);
        out << " edges=\"";
        for (ROEdgeVector::const_iterator k = desc.edges2Pass.begin(); k != desc.edges2Pass.end(); k++) {
            if (k != desc.edges2Pass.begin()) {
                out << ' ';
            }
            out << (*k)->getID();
        }
        out << '"';
        out.closeTag();
        haveSavedOneAtLeast = true;
    }
    return haveSavedOneAtLeast;
}


void
RODFRouteCont::sortByDistance() {
    sort(myRoutes.begin(), myRoutes.end(), by_distance_sorter());
}


void
RODFRouteCont::removeIllegal(const std::vector<ROEdgeVector >& illegals) {
    for (std::vector<RODFRouteDesc>::iterator i = myRoutes.begin(); i != myRoutes.end();) {
        RODFRouteDesc& desc = *i;
        bool remove = false;
        for (std::vector<ROEdgeVector >::const_iterator j = illegals.begin(); !remove && j != illegals.end(); ++j) {
            int noFound = 0;
            for (ROEdgeVector::const_iterator k = (*j).begin(); !remove && k != (*j).end(); ++k) {
                if (find(desc.edges2Pass.begin(), desc.edges2Pass.end(), *k) != desc.edges2Pass.end()) {
                    noFound++;
                    if (noFound > 1) {
                        remove = true;
                    }
                }
            }
        }
        if (remove) {
            i = myRoutes.erase(i);
        } else {
            ++i;
        }
    }
}


void
RODFRouteCont::setID(RODFRouteDesc& desc) const {
    std::pair<ROEdge*, ROEdge*> c(desc.edges2Pass[0], desc.edges2Pass.back());
    desc.routename = c.first->getID() + "_to_" + c.second->getID();
    if (myConnectionOccurences.find(c) == myConnectionOccurences.end()) {
        myConnectionOccurences[c] = 0;
    } else {
        myConnectionOccurences[c] = myConnectionOccurences[c] + 1;
        desc.routename = desc.routename + "_" + toString(myConnectionOccurences[c]);
    }
}


/****************************************************************************/
