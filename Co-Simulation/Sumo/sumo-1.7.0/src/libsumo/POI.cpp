/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2017-2020 German Aerospace Center (DLR) and others.
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
/// @file    POI.cpp
/// @author  Daniel Krajzewicz
/// @author  Mario Krumnow
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Robert Hilbrich
/// @date    30.05.2012
///
// C++ TraCI client API implementation
/****************************************************************************/
#include <config.h>

#include <utils/shapes/PointOfInterest.h>
#include <utils/shapes/ShapeContainer.h>
#include <utils/geom/GeomHelper.h>
#include <microsim/MSNet.h>
#include <libsumo/TraCIConstants.h>
#include "Polygon.h"
#include "POI.h"
#include "Helper.h"


namespace libsumo {
// ===========================================================================
// static member initializations
// ===========================================================================
SubscriptionResults POI::mySubscriptionResults;
ContextSubscriptionResults POI::myContextSubscriptionResults;
NamedRTree* POI::myTree(nullptr);


// ===========================================================================
// static member definitions
// ===========================================================================
std::vector<std::string>
POI::getIDList() {
    std::vector<std::string> ids;
    ShapeContainer& shapeCont = MSNet::getInstance()->getShapeContainer();
    shapeCont.getPOIs().insertIDs(ids);
    return ids;
}


int
POI::getIDCount() {
    return (int)getIDList().size();
}


std::string
POI::getType(const std::string& poiID) {
    return getPoI(poiID)->getShapeType();
}


TraCIColor
POI::getColor(const std::string& poiID) {
    return Helper::makeTraCIColor(getPoI(poiID)->getShapeColor());
}


TraCIPosition
POI::getPosition(const std::string& poiID, const bool includeZ) {
    return Helper::makeTraCIPosition(*getPoI(poiID), includeZ);
}


double
POI::getWidth(const std::string& poiID) {
    return getPoI(poiID)->getWidth();
}


double
POI::getHeight(const std::string& poiID) {
    return getPoI(poiID)->getHeight();
}


double
POI::getAngle(const std::string& poiID) {
    return getPoI(poiID)->getShapeNaviDegree();
}


std::string
POI::getImageFile(const std::string& poiID) {
    return getPoI(poiID)->getShapeImgFile();
}


std::string
POI::getParameter(const std::string& poiID, const std::string& key) {
    return getPoI(poiID)->getParameter(key, "");
}


LIBSUMO_GET_PARAMETER_WITH_KEY_IMPLEMENTATION(POI)


void
POI::setType(const std::string& poiID, const std::string& type) {
    getPoI(poiID)->setShapeType(type);
}


void
POI::setPosition(const std::string& poiID, double x, double y) {
    // try to retrieve so that the correct error is generated for unknown poiIDs
    getPoI(poiID);
    MSNet::getInstance()->getShapeContainer().movePOI(poiID, Position(x, y));
}


void
POI::setColor(const std::string& poiID, const TraCIColor& c) {
    getPoI(poiID)->setShapeColor(Helper::makeRGBColor(c));
}


void
POI::setWidth(const std::string& poiID, double width) {
    getPoI(poiID)->setWidth(width);
}


void
POI::setHeight(const std::string& poiID, double height) {
    getPoI(poiID)->setHeight(height);
}


void
POI::setAngle(const std::string& poiID, double angle) {
    getPoI(poiID)->setShapeNaviDegree(angle);
}


void
POI::setImageFile(const std::string& poiID, const std::string& imageFile) {
    getPoI(poiID)->setShapeImgFile(imageFile);
}


bool
POI::add(const std::string& poiID, double x, double y, const TraCIColor& color, const std::string& poiType, int layer, const std::string& imgFile, double width, double height, double angle) {
    ShapeContainer& shapeCont = MSNet::getInstance()->getShapeContainer();
    bool ok = shapeCont.addPOI(poiID, poiType,
                               Helper::makeRGBColor(color),
                               Position(x, y), false, "", 0, 0, (double)layer,
                               angle,
                               imgFile,
                               Shape::DEFAULT_RELATIVEPATH,
                               width,
                               height);
    if (ok && myTree != nullptr) {
        PointOfInterest* p = shapeCont.getPOIs().get(poiID);
        const float cmin[2] = {(float)p->x(), (float)p->y()};
        const float cmax[2] = {(float)p->x(), (float)p->y()};
        myTree->Insert(cmin, cmax, p);
    }
    return ok;
}


bool
POI::remove(const std::string& poiID, int /* layer */) {
    ShapeContainer& shapeCont = MSNet::getInstance()->getShapeContainer();
    PointOfInterest* p = shapeCont.getPOIs().get(poiID);
    if (p != nullptr && myTree != nullptr) {
        const float cmin[2] = {(float)p->x(), (float)p->y()};
        const float cmax[2] = {(float)p->x(), (float)p->y()};
        myTree->Remove(cmin, cmax, p);
    }
    return shapeCont.removePOI(poiID);
}


void
POI::highlight(const std::string& poiID, const TraCIColor& col, double size, const int alphaMax, const double duration, const int type) {
    // NOTE: Code is duplicated in large parts in Vehicle.cpp
    PointOfInterest* poi = getPoI(poiID);

    // Center of the highlight circle
    Position* center = dynamic_cast<Position*>(poi);
    // Size of the highlight circle
    if (size <= 0) {
        size = sqrt(poi->getHeight() * poi->getHeight() + poi->getWidth() * poi->getWidth()) * 0.7;
    }
    // Make polygon shape
    const unsigned int nPoints = 34;
    const PositionVector circlePV = GeomHelper::makeRing(size, size + 1., *center, nPoints);
    TraCIPositionVector circle = Helper::makeTraCIPositionVector(circlePV);

#ifdef DEBUG_DYNAMIC_SHAPES
    std::cout << SIMTIME << " Vehicle::highlight() for vehicle '" << vehicleID << "'\n"
              << " circle: " << circlePV << std::endl;
#endif

    // Find a free polygon id
    int i = 0;
    std::string polyID = poi->getID() + "_hl" + toString(i);
    while (Polygon::exists(polyID)) {
        polyID = poi->getID() + "_hl" + toString(++i);
    }
    // Line width
    double lw = 0.;
    // Layer
    double lyr = 0.;
    if (MSNet::getInstance()->isGUINet()) {
        lyr = poi->getShapeLayer();
        lyr += (type + 1) / 257.;
    }
    // Make Polygon
    Polygon::addHighlightPolygon(poiID, type, polyID, circle, col, true, "highlight", (int)lyr, lw);

    // Animation time line
    double maxAttack = 1.0; // maximal fade-in time
    std::vector<double> timeSpan;
    if (duration > 0.) {
        timeSpan = {0, MIN2(maxAttack, duration / 3.), 2.*duration / 3., duration};
    }
    // Alpha time line
    std::vector<double> alphaSpan;
    if (alphaMax > 0.) {
        alphaSpan = {0., (double) alphaMax, ((double) alphaMax) / 3., 0.};
    }
    // Attach dynamics
    Polygon::addDynamics(polyID, "", timeSpan, alphaSpan, false, false);
}


void
POI::setParameter(const std::string& poiID, const std::string& key, const std::string& value) {
    PointOfInterest* p = getPoI(poiID);
    p->setParameter(key, value);
}



LIBSUMO_SUBSCRIPTION_IMPLEMENTATION(POI, POI)


PointOfInterest*
POI::getPoI(const std::string& id) {
    PointOfInterest* sumoPoi = MSNet::getInstance()->getShapeContainer().getPOIs().get(id);
    if (sumoPoi == nullptr) {
        throw TraCIException("POI '" + id + "' is not known");
    }
    return sumoPoi;
}


NamedRTree*
POI::getTree() {
    if (myTree == nullptr) {
        myTree = new NamedRTree();
        ShapeContainer& shapeCont = MSNet::getInstance()->getShapeContainer();
        for (const auto& i : shapeCont.getPOIs()) {
            const float cmin[2] = {(float)i.second->x(), (float)i.second->y()};
            const float cmax[2] = {(float)i.second->x(), (float)i.second->y()};
            myTree->Insert(cmin, cmax, i.second);
        }
    }
    return myTree;
}

void
POI::cleanup() {
    delete myTree;
    myTree = nullptr;
}


void
POI::storeShape(const std::string& id, PositionVector& shape) {
    shape.push_back(*getPoI(id));
}


std::shared_ptr<VariableWrapper>
POI::makeWrapper() {
    return std::make_shared<Helper::SubscriptionWrapper>(handleVariable, mySubscriptionResults, myContextSubscriptionResults);
}


bool
POI::handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper) {
    switch (variable) {
        case TRACI_ID_LIST:
            return wrapper->wrapStringList(objID, variable, getIDList());
        case ID_COUNT:
            return wrapper->wrapInt(objID, variable, getIDCount());
        case VAR_TYPE:
            return wrapper->wrapString(objID, variable, getType(objID));
        case VAR_COLOR:
            return wrapper->wrapColor(objID, variable, getColor(objID));
        case VAR_POSITION:
            return wrapper->wrapPosition(objID, variable, getPosition(objID));
        case VAR_POSITION3D:
            return wrapper->wrapPosition(objID, variable, getPosition(objID, true));
        case VAR_WIDTH:
            return wrapper->wrapDouble(objID, variable, getWidth(objID));
        case VAR_HEIGHT:
            return wrapper->wrapDouble(objID, variable, getHeight(objID));
        case VAR_ANGLE:
            return wrapper->wrapDouble(objID, variable, getAngle(objID));
        case VAR_IMAGEFILE:
            return wrapper->wrapString(objID, variable, getImageFile(objID));
        default:
            return false;
    }
}


}


/****************************************************************************/
