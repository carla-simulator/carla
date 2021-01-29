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
/// @file    GUIShapeContainer.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    08.10.2009
///
// Storage for geometrical objects extended by mutexes
/****************************************************************************/
#include <config.h>

#include "GUIShapeContainer.h"
#include <utils/common/MsgHandler.h>
#include <utils/shapes/PolygonDynamics.h>
#include <foreign/rtree/SUMORTree.h>
#include <utils/gui/globjects/GUIPolygon.h>
#include <utils/gui/globjects/GUIPointOfInterest.h>


// ===========================================================================
// method definitions
// ===========================================================================
GUIShapeContainer::GUIShapeContainer(SUMORTree& vis) :
    myVis(vis),
    myAllowReplacement(false) {
}


GUIShapeContainer::~GUIShapeContainer() {}


bool
GUIShapeContainer::addPOI(const std::string& id, const std::string& type, const RGBColor& color, const Position& pos, bool geo,
                          const std::string& lane, double posOverLane, double posLat, double layer, double angle,
                          const std::string& imgFile, bool relativePath, double width, double height, bool /* ignorePruning */) {
    GUIPointOfInterest* p = new GUIPointOfInterest(id, type, color, pos, geo, lane, posOverLane, posLat, layer, angle, imgFile, relativePath, width, height);
    FXMutexLock locker(myLock);
    if (!myPOIs.add(id, p)) {
        if (myAllowReplacement) {
            GUIPointOfInterest* oldP = dynamic_cast<GUIPointOfInterest*>(myPOIs.get(id));
            myVis.removeAdditionalGLObject(oldP);
            myPOIs.remove(id);
            myPOIs.add(id, p);
            WRITE_WARNING("Replacing POI '" + id + "'");
        } else {
            delete p;
            return false;
        }
    }
    myVis.addAdditionalGLObject(p);
    return true;
}


bool
GUIShapeContainer::addPolygon(const std::string& id, const std::string& type,
                              const RGBColor& color, double layer,
                              double angle, const std::string& imgFile, bool relativePath,
                              const PositionVector& shape, bool geo, bool fill, double lineWidth, bool /* ignorePruning */) {
    GUIPolygon* p = new GUIPolygon(id, type, color, shape, geo, fill, lineWidth, layer, angle, imgFile, relativePath);
    FXMutexLock locker(myLock);
    if (!myPolygons.add(id, p)) {
        if (myAllowReplacement) {
            GUIPolygon* oldP = dynamic_cast<GUIPolygon*>(myPolygons.get(id));
            myVis.removeAdditionalGLObject(oldP);
            myPolygons.remove(id);
            myPolygons.add(id, p);
            WRITE_WARNING("Replacing polygon '" + id + "'");
        } else {
            delete p;
            return false;
        }
    }
    myVis.addAdditionalGLObject(p);
    return true;
}


PolygonDynamics*
GUIShapeContainer::addPolygonDynamics(double simtime,
                                      std::string polyID,
                                      SUMOTrafficObject* trackedObject,
                                      const std::vector<double>& timeSpan,
                                      const std::vector<double>& alphaSpan,
                                      bool looped,
                                      bool rotate) {
    PolygonDynamics* pd = ShapeContainer::addPolygonDynamics(simtime, polyID, trackedObject, timeSpan, alphaSpan, looped, rotate);
    if (pd != nullptr) {
        pd->setRTree(&myVis);
    }
    return pd;
}


SUMOTime
GUIShapeContainer::polygonDynamicsUpdate(SUMOTime t, PolygonDynamics* pd) {
    FXMutexLock locker(myLock);
    GUIPolygon* p = dynamic_cast<GUIPolygon*>(pd->getPolygon());
    assert(p != nullptr);
    myVis.removeAdditionalGLObject(p);
    SUMOTime next = ShapeContainer::polygonDynamicsUpdate(t, pd);
    if (next != 0) {
        // Update polygon position in RTree
        myVis.addAdditionalGLObject(p);
    }
    return next;
}


bool
GUIShapeContainer::removePolygon(const std::string& id, bool useLock) {
    GUIPolygon* p = dynamic_cast<GUIPolygon*>(myPolygons.get(id));
    if (p == nullptr) {
        return false;
    }
    FXMutexLock* locker = nullptr;
    if (useLock) {
        locker = new FXMutexLock(myLock);
    }
    myVis.removeAdditionalGLObject(p);
    bool succ = ShapeContainer::removePolygon(id);
    delete locker;
    return succ;
}


bool
GUIShapeContainer::removePOI(const std::string& id) {
    FXMutexLock locker(myLock);
    GUIPointOfInterest* p = dynamic_cast<GUIPointOfInterest*>(myPOIs.get(id));
    if (p == nullptr) {
        return false;
    }
    myVis.removeAdditionalGLObject(p);
    return myPOIs.remove(id);
}


void
GUIShapeContainer::movePOI(const std::string& id, const Position& pos) {
    FXMutexLock locker(myLock);
    GUIPointOfInterest* p = dynamic_cast<GUIPointOfInterest*>(myPOIs.get(id));
    if (p != nullptr) {
        myVis.removeAdditionalGLObject(p);
        static_cast<Position*>(p)->set(pos);
        myVis.addAdditionalGLObject(p);
    }
}


void
GUIShapeContainer::reshapePolygon(const std::string& id, const PositionVector& shape) {
    FXMutexLock locker(myLock);
    GUIPolygon* p = dynamic_cast<GUIPolygon*>(myPolygons.get(id));
    if (p != nullptr) {
        myVis.removeAdditionalGLObject(p);
        p->setShape(shape);
        myVis.addAdditionalGLObject(p);
    }
}



std::vector<GUIGlID>
GUIShapeContainer::getPOIIds() const {
    FXMutexLock locker(myLock);
    std::vector<GUIGlID> ret;
    for (const auto& poi : getPOIs()) {
        ret.push_back(static_cast<GUIPointOfInterest*>(poi.second)->getGlID());
    }
    return ret;
}


std::vector<GUIGlID>
GUIShapeContainer::getPolygonIDs() const {
    FXMutexLock locker(myLock);
    std::vector<GUIGlID> ret;
    for (const auto& poly : getPolygons()) {
        ret.push_back(static_cast<GUIPolygon*>(poly.second)->getGlID());
    }
    return ret;
}


/****************************************************************************/
