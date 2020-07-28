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
/// @file    ShapeContainer.cpp
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Sept 2002
///
// Storage for geometrical objects, sorted by the layers they are in
/****************************************************************************/
#include <config.h>

#include <fstream>
#include <stdlib.h>
#include <iostream>
#include <utility>
#include <string>
#include <cmath>
#include <utils/common/NamedObjectCont.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/ToString.h>
#include <utils/common/StdDefs.h>
#include <utils/common/ParametrisedWrappingCommand.h>
#include "PolygonDynamics.h"
#include "ShapeContainer.h"


// Debug defines
//#define DEBUG_DYNAMIC_SHAPES

// ===========================================================================
// method definitions
// ===========================================================================
ShapeContainer::ShapeContainer() {}

ShapeContainer::~ShapeContainer() {
    for (auto& p : myPolygonUpdateCommands) {
        p.second->deschedule();
    }
    myPolygonUpdateCommands.clear();

    for (auto& p : myPolygonDynamics) {
        delete p.second;
    }
    myPolygonDynamics.clear();

}

bool
ShapeContainer::addPolygon(const std::string& id, const std::string& type,
                           const RGBColor& color, double layer,
                           double angle, const std::string& imgFile, bool relativePath,
                           const PositionVector& shape, bool geo, bool fill, double lineWidth, bool ignorePruning) {
    return add(new SUMOPolygon(id, type, color, shape, geo, fill, lineWidth, layer, angle, imgFile, relativePath), ignorePruning);
}


PolygonDynamics*
ShapeContainer::addPolygonDynamics(double simtime,
                                   std::string polyID,
                                   SUMOTrafficObject* trackedObject,
                                   const std::vector<double>& timeSpan,
                                   const std::vector<double>& alphaSpan,
                                   bool looped,
                                   bool rotate) {

#ifdef DEBUG_DYNAMIC_SHAPES
    std::cout << simtime << " ShapeContainer::addPolygonDynamics() called for polygon '" << polyID << "'" << std::endl;
#endif

    SUMOPolygon* p = myPolygons.get(polyID);
    if (p == nullptr) {
#ifdef DEBUG_DYNAMIC_SHAPES
        std::cout << "   polygon '" << polyID << "' doesn't exist!" << std::endl;
#endif
        return nullptr;
    }
    // remove eventually existent previously
    removePolygonDynamics(polyID);

    // Add new dynamics
    PolygonDynamics* pd = new PolygonDynamics(simtime, p, trackedObject, timeSpan, alphaSpan, looped, rotate);
    myPolygonDynamics.insert(std::make_pair(polyID, pd));

    // Add tracking information
    if (trackedObject != nullptr) {
        auto i = myTrackingPolygons.find(pd->getTrackedObjectID());
        if (i == myTrackingPolygons.end()) {
            myTrackingPolygons.insert(std::make_pair(pd->getTrackedObjectID(), std::set<const SUMOPolygon*>({p})));
        } else {
            i->second.insert(p);
        }
    }
    return pd;
}


bool
ShapeContainer::removePolygonDynamics(const std::string& polyID) {
    SUMOPolygon* p = myPolygons.get(polyID);
    if (p == nullptr) {
        return false;
    }
    auto d = myPolygonDynamics.find(polyID);
    if (d != myPolygonDynamics.end()) {
#ifdef DEBUG_DYNAMIC_SHAPES
        std::cout << "   Removing dynamics of polygon '" << polyID << "'" << std::endl;
#endif
        const std::string& trackedObjID = d->second->getTrackedObjectID();
        if (trackedObjID != "") {
            // Remove tracking information
            auto i = myTrackingPolygons.find(trackedObjID);
            assert(i != myTrackingPolygons.end());
            assert(i->second.find(p) != i->second.end());
            i->second.erase(p);
            // Remove highlighting information
            clearHighlights(trackedObjID, p);
        }
        delete d->second;
        myPolygonDynamics.erase(d);
        // Clear existing polygon dynamics commands before adding new dynamics
        cleanupPolygonDynamics(polyID);
        return true;
    } else {
        return false;
    }
}


bool
ShapeContainer::addPOI(const std::string& id, const std::string& type, const RGBColor& color, const Position& pos, bool geo,
                       const std::string& lane, double posOverLane, double posLat, double layer, double angle,
                       const std::string& imgFile, bool relativePath, double width, double height, bool ignorePruning) {
    return add(new PointOfInterest(id, type, color, pos, geo, lane, posOverLane, posLat, layer, angle, imgFile, relativePath, width, height), ignorePruning);
}


bool
ShapeContainer::removePolygon(const std::string& id, bool /* useLock */) {
#ifdef DEBUG_DYNAMIC_SHAPES
    std::cout << "ShapeContainer: Removing Polygon '" << id << "'" << std::endl;
#endif
    removePolygonDynamics(id);
    return myPolygons.remove(id);
}


bool
ShapeContainer::removePOI(const std::string& id) {
    return myPOIs.remove(id);
}


void
ShapeContainer::movePOI(const std::string& id, const Position& pos) {
    PointOfInterest* p = myPOIs.get(id);
    if (p != nullptr) {
        static_cast<Position*>(p)->set(pos);
    }
}


void
ShapeContainer::reshapePolygon(const std::string& id, const PositionVector& shape) {
    SUMOPolygon* p = myPolygons.get(id);
    if (p != nullptr) {
        p->setShape(shape);
    }
}


bool
ShapeContainer::add(SUMOPolygon* poly, bool /* ignorePruning */) {
    if (!myPolygons.add(poly->getID(), poly)) {
        delete poly;
        return false;
    }
    return true;
}


bool
ShapeContainer::add(PointOfInterest* poi, bool /* ignorePruning */) {
    if (!myPOIs.add(poi->getID(), poi)) {
        delete poi;
        return false;
    }
    return true;
}


void
ShapeContainer::cleanupPolygonDynamics(const std::string& id) {
    auto j = myPolygonUpdateCommands.find(id);
    if (j != myPolygonUpdateCommands.end()) {
        j->second->deschedule();
        myPolygonUpdateCommands.erase(j);
    }
}


SUMOTime
ShapeContainer::polygonDynamicsUpdate(SUMOTime t, PolygonDynamics* pd) {
    SUMOTime next = pd->update(t);
    if (next == 0) {
        // Dynamics have expired => remove polygon
        myPolygonUpdateCommands[pd->getPolygonID()]->deschedule();
        // Don't aquire lock (in GUI case GUIShapeContainer::polygonDynamicsUpdate() does this)
        removePolygon(pd->getPolygonID(), false);
    }
    return next;
}

void
ShapeContainer::registerHighlight(const std::string& objectID, const int type, const std::string& polygonID) {
    std::string toRemove = "";
    clearHighlight(objectID, type, toRemove);
    if (toRemove != "") {
        removePolygon(toRemove);
    }
    auto i = myHighlightPolygons.find(objectID);
    if (i == myHighlightPolygons.end()) {
        myHighlightPolygons.insert(std::make_pair(objectID, std::map<int, std::string>({std::make_pair(type, polygonID)})));
    } else {
        i->second.insert(std::make_pair(type, polygonID));
    }
    myHighlightedObjects.insert(std::make_pair(polygonID, objectID));
}

void
ShapeContainer::clearHighlight(const std::string& objectID, const int type, std::string& toRemove) {
    auto i = myHighlightPolygons.find(objectID);
    if (i != myHighlightPolygons.end()) {
        auto j = i->second.find(type);
        if (j != i->second.end()) {
            toRemove = j->second;
            myHighlightedObjects.erase(toRemove);
            i->second.erase(j);
            if (i->second.empty()) {
                myHighlightPolygons.erase(i);
            }
        }
    }
}

void
ShapeContainer::clearHighlights(const std::string& objectID, SUMOPolygon* p) {
    auto i = myHighlightPolygons.find(objectID);
    if (i != myHighlightPolygons.end()) {
        auto j = i->second.begin();
        while (j != i->second.end()) {
            if (j->second == p->getID()) {
                i->second.erase(j);
                break;
            } else {
                ++j;
            }
        }
        if (i->second.empty()) {
            myHighlightPolygons.erase(i);
        }
    }
}

void
ShapeContainer::addPolygonUpdateCommand(std::string polyID, ParametrisedWrappingCommand<ShapeContainer, PolygonDynamics*>* cmd) {
    myPolygonUpdateCommands.insert(std::make_pair(polyID, cmd));
}


void
ShapeContainer::removeTrackers(std::string objectID) {
    auto i = myTrackingPolygons.find(objectID);
    if (i != myTrackingPolygons.end()) {
#ifdef DEBUG_DYNAMIC_SHAPES
        std::cout << " Removing tracking polygons for object '" << objectID << "'" << std::endl;
#endif
        while (!i->second.empty()) {
            removePolygon((*i->second.begin())->getID());
        }
        myTrackingPolygons.erase(i);
    }
}


/****************************************************************************/
