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
/// @file    Polygon.cpp
/// @author  Gregor L\"ammel
/// @date    15.03.2017
///
// C++ TraCI client API implementation
/****************************************************************************/
#include <microsim/MSNet.h>
#include <microsim/MSEventControl.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/transportables/MSTransportableControl.h>
#include <microsim/MSDynamicShapeUpdater.h>
#include <libsumo/TraCIConstants.h>
#include <utils/shapes/SUMOPolygon.h>
#include <utils/shapes/PolygonDynamics.h>
#include <utils/shapes/ShapeContainer.h>
#include <utils/common/ParametrisedWrappingCommand.h>

#include "Polygon.h"
#include "Helper.h"


namespace libsumo {
// ===========================================================================
// static member initializations
// ===========================================================================
SubscriptionResults Polygon::mySubscriptionResults;
ContextSubscriptionResults Polygon::myContextSubscriptionResults;
NamedRTree* Polygon::myTree(nullptr);


// ===========================================================================
// static member definitions
// ===========================================================================
std::vector<std::string>
Polygon::getIDList() {
    std::vector<std::string> ids;
    ShapeContainer& shapeCont = MSNet::getInstance()->getShapeContainer();
    shapeCont.getPolygons().insertIDs(ids);
    return ids;
}


int
Polygon::getIDCount() {
    return (int)getIDList().size();
}


std::string
Polygon::getType(const std::string& polygonID) {
    return getPolygon(polygonID)->getShapeType();
}


TraCIPositionVector
Polygon::getShape(const std::string& polygonID) {
    SUMOPolygon* p = getPolygon(polygonID);
    return Helper::makeTraCIPositionVector(p->getShape());
}


bool
Polygon::getFilled(const std::string& polygonID) {
    return getPolygon(polygonID)->getFill();
}

double
Polygon::getLineWidth(const std::string& polygonID) {
    return getPolygon(polygonID)->getLineWidth();
}

TraCIColor
Polygon::getColor(const std::string& polygonID) {
    SUMOPolygon* p = getPolygon(polygonID);
    return Helper::makeTraCIColor(p->getShapeColor());
}


std::string
Polygon::getParameter(const std::string& polygonID, const std::string& key) {
    return getPolygon(polygonID)->getParameter(key, "");
}


LIBSUMO_GET_PARAMETER_WITH_KEY_IMPLEMENTATION(Polygon)


void
Polygon::setType(const std::string& polygonID, const std::string& setType) {
    SUMOPolygon* p = getPolygon(polygonID);
    p->setShapeType(setType);
}


void
Polygon::setShape(const std::string& polygonID, const TraCIPositionVector& shape) {
    PositionVector positionVector = Helper::makePositionVector(shape);
    getPolygon(polygonID); // just to check whether it exists
    ShapeContainer& shapeCont = MSNet::getInstance()->getShapeContainer();
    shapeCont.reshapePolygon(polygonID, positionVector);
}


void
Polygon::setColor(const std::string& polygonID, const TraCIColor& c) {
    getPolygon(polygonID)->setShapeColor(Helper::makeRGBColor(c));
}


void
Polygon::add(const std::string& polygonID, const TraCIPositionVector& shape, const TraCIColor& color, bool fill, const std::string& polygonType, int layer, double lineWidth) {
    ShapeContainer& shapeCont = MSNet::getInstance()->getShapeContainer();
    PositionVector pShape = Helper::makePositionVector(shape);
    RGBColor col = Helper::makeRGBColor(color);
    if (!shapeCont.addPolygon(polygonID, polygonType, col, (double)layer, Shape::DEFAULT_ANGLE, Shape::DEFAULT_IMG_FILE, Shape::DEFAULT_RELATIVEPATH, pShape, false, fill, lineWidth)) {
        throw TraCIException("Could not add polygon '" + polygonID + "'");
    }
    if (myTree != nullptr) {
        SUMOPolygon* p = shapeCont.getPolygons().get(polygonID);
        Boundary b = p->getShape().getBoxBoundary();
        const float cmin[2] = {(float) b.xmin(), (float) b.ymin()};
        const float cmax[2] = {(float) b.xmax(), (float) b.ymax()};
        myTree->Insert(cmin, cmax, p);
    }
}


void
Polygon::addHighlightPolygon(const std::string& objectID, const int type, const std::string& polygonID, const TraCIPositionVector& shape, const TraCIColor& color, bool fill, const std::string& polygonType, int layer, double lineWidth) {
    add(polygonID, shape, color, fill, polygonType, layer, lineWidth);
    MSNet::getInstance()->getShapeContainer().registerHighlight(objectID, type, polygonID);
}


void
Polygon::addDynamics(const std::string& polygonID, const std::string& trackedID, const std::vector<double>& timeSpan, const std::vector<double>& alphaSpan, bool looped, bool rotate) {
    if (timeSpan.empty()) {
        if (trackedID == "") {
            throw TraCIException("Could not add polygon dynamics for polygon '" + polygonID + "': dynamics underspecified (either a tracked object ID or a time span have to be provided).");
        }
        if (looped) {
            throw TraCIException("Could not add polygon dynamics for polygon '" + polygonID + "': looped==true requires time line of positive length.");
        }
    }
    if (timeSpan.size() == 1) {
        throw TraCIException("Could not add polygon dynamics for polygon '" + polygonID + "': time span cannot have length one.");
    } else if (timeSpan.size() > 0 && timeSpan[0] != 0.0) {
        throw TraCIException("Could not add polygon dynamics for polygon '" + polygonID + "': first element of time span must be zero.");
    }
    if (timeSpan.size() != alphaSpan.size() && alphaSpan.size() != 0) {
        throw TraCIException("Could not add polygon dynamics for polygon '" + polygonID + "': alpha span must have length zero or equal to time span length.");
    }
    if (timeSpan.size() >= 2) {
        for (unsigned int i = 1; i < timeSpan.size(); ++i) {
            if (timeSpan[i - 1] > timeSpan[i]) {
                throw TraCIException("Could not add polygon dynamics for polygon '" + polygonID + "': entries of time span must be ordered ascendingly.");
            }
        }
    }

    SUMOTrafficObject* obj = getTrafficObject(trackedID);
    ShapeContainer& shapeCont = MSNet::getInstance()->getShapeContainer();
    PolygonDynamics* pd = shapeCont.addPolygonDynamics(SIMTIME, polygonID, obj, timeSpan, alphaSpan, looped, rotate);
    if (pd == nullptr) {
        throw TraCIException("Could not add polygon dynamics for polygon '" + polygonID + "': polygon doesn't exist.");
    }
    // Ensure existence of a DynamicShapeUpdater
    if (MSNet::getInstance()->getDynamicShapeUpdater() == nullptr) {
        MSNet::VehicleStateListener* listener = dynamic_cast<MSNet::VehicleStateListener*>(MSNet::getInstance()->makeDynamicShapeUpdater());
        MSNet::getInstance()->addVehicleStateListener(listener);
    }

    // Schedule the regular polygon update
    auto cmd = new ParametrisedWrappingCommand<ShapeContainer, PolygonDynamics*>(&shapeCont, pd, &ShapeContainer::polygonDynamicsUpdate);
    shapeCont.addPolygonUpdateCommand(pd->getPolygonID(), cmd);
    MSNet::getInstance()->getEndOfTimestepEvents()->addEvent(cmd, SIMSTEP);
}


void
Polygon::remove(const std::string& polygonID, int /* layer */) {
    // !!! layer not used yet (shouldn't the id be enough?)
    ShapeContainer& shapeCont = MSNet::getInstance()->getShapeContainer();
    if (myTree != nullptr) {
        SUMOPolygon* p = shapeCont.getPolygons().get(polygonID);
        if (p != nullptr) {
            Boundary b = p->getShape().getBoxBoundary();
            const float cmin[2] = {(float) b.xmin(), (float) b.ymin()};
            const float cmax[2] = {(float) b.xmax(), (float) b.ymax()};
            myTree->Remove(cmin, cmax, p);
        }
    }
    if (!shapeCont.removePolygon(polygonID)) {
        throw TraCIException("Could not remove polygon '" + polygonID + "'");
    }
}


void
Polygon::setFilled(std::string polygonID, bool filled) {
    SUMOPolygon* p = getPolygon(polygonID);
    p->setFill(filled);
}

void
Polygon::setLineWidth(std::string polygonID, double lineWidth) {
    SUMOPolygon* p = getPolygon(polygonID);
    p->setLineWidth(lineWidth);
}


SUMOPolygon*
Polygon::getPolygon(const std::string& id) {
    SUMOPolygon* p = MSNet::getInstance()->getShapeContainer().getPolygons().get(id);
    if (p == nullptr) {
        throw TraCIException("Polygon '" + id + "' is not known");
    }
    return p;
}


SUMOTrafficObject*
Polygon::getTrafficObject(const std::string& id) {
    if (id == "") {
        return nullptr;
    }
    MSNet* net = MSNet::getInstance();
    // First try to find a vehicle with the given id
    SUMOVehicle* sumoVehicle = net->getVehicleControl().getVehicle(id);
    if (sumoVehicle != nullptr) {
        return static_cast<SUMOTrafficObject*>(sumoVehicle);
    }
    MSTransportable* transportable = net->getPersonControl().get(id);
    if (transportable != nullptr) {
        return static_cast<SUMOTrafficObject*>(transportable);
    } else {
        throw TraCIException("Traffic object '" + id + "' is not known");
    }
}


void
Polygon::setParameter(const std::string& polygonID, const std::string& key, const std::string& value) {
    SUMOPolygon* p = getPolygon(polygonID);
    p->setParameter(key, value);
}


LIBSUMO_SUBSCRIPTION_IMPLEMENTATION(Polygon, POLYGON)


NamedRTree*
Polygon::getTree() {
    if (myTree == nullptr) {
        myTree = new NamedRTree();
        ShapeContainer& shapeCont = MSNet::getInstance()->getShapeContainer();
        for (const auto& i : shapeCont.getPolygons()) {
            Boundary b = i.second->getShape().getBoxBoundary();
            const float cmin[2] = {(float) b.xmin(), (float) b.ymin()};
            const float cmax[2] = {(float) b.xmax(), (float) b.ymax()};
            myTree->Insert(cmin, cmax, i.second);
        }
    }
    return myTree;
}

void
Polygon::cleanup() {
    delete myTree;
    myTree = nullptr;
}

void
Polygon::storeShape(const std::string& id, PositionVector& shape) {
    shape = getPolygon(id)->getShape();
}


std::shared_ptr<VariableWrapper>
Polygon::makeWrapper() {
    return std::make_shared<Helper::SubscriptionWrapper>(handleVariable, mySubscriptionResults, myContextSubscriptionResults);
}


bool
Polygon::handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper) {
    switch (variable) {
        case TRACI_ID_LIST:
            return wrapper->wrapStringList(objID, variable, getIDList());
        case ID_COUNT:
            return wrapper->wrapInt(objID, variable, getIDCount());
        case VAR_TYPE:
            return wrapper->wrapString(objID, variable, getType(objID));
        case VAR_COLOR:
            return wrapper->wrapColor(objID, variable, getColor(objID));
        case VAR_FILL:
            return wrapper->wrapInt(objID, variable, getFilled(objID));
        case VAR_WIDTH:
            return wrapper->wrapDouble(objID, variable, getLineWidth(objID));
        default:
            return false;
    }
}


bool
Polygon::exists(std::string polyID) {
    SUMOPolygon* p = MSNet::getInstance()->getShapeContainer().getPolygons().get(polyID);
    return p != nullptr;
}
}


/****************************************************************************/
