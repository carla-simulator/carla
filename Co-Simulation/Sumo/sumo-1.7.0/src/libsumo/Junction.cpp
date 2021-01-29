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
/// @file    Junction.cpp
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
#include <microsim/MSNet.h>
#include <microsim/MSJunctionControl.h>
#include <libsumo/TraCIConstants.h>
#include "Helper.h"
#include "Junction.h"


namespace libsumo {
// ===========================================================================
// static member initializations
// ===========================================================================
SubscriptionResults Junction::mySubscriptionResults;
ContextSubscriptionResults Junction::myContextSubscriptionResults;
NamedRTree* Junction::myTree(nullptr);


// ===========================================================================
// member definitions
// ===========================================================================
std::vector<std::string>
Junction::getIDList() {
    std::vector<std::string> ids;
    MSNet::getInstance()->getJunctionControl().insertIDs(ids);
    return ids;
}


int
Junction::getIDCount() {
    return (int)getIDList().size();
}


TraCIPosition
Junction::getPosition(const std::string& junctionID, const bool includeZ) {
    return Helper::makeTraCIPosition(getJunction(junctionID)->getPosition(), includeZ);
}


TraCIPositionVector
Junction::getShape(const std::string& junctionID) {
    return Helper::makeTraCIPositionVector(getJunction(junctionID)->getShape());
}


MSJunction*
Junction::getJunction(const std::string& id) {
    MSJunction* j = MSNet::getInstance()->getJunctionControl().get(id);
    if (j == nullptr) {
        throw TraCIException("Junction '" + id + "' is not known");
    }
    return j;
}


LIBSUMO_SUBSCRIPTION_IMPLEMENTATION(Junction, JUNCTION)


NamedRTree*
Junction::getTree() {
    if (myTree == nullptr) {
        myTree = new NamedRTree();
        for (const auto& i : MSNet::getInstance()->getJunctionControl()) {
            Boundary b = i.second->getShape().getBoxBoundary();
            const float cmin[2] = {(float) b.xmin(), (float) b.ymin()};
            const float cmax[2] = {(float) b.xmax(), (float) b.ymax()};
            myTree->Insert(cmin, cmax, i.second);
        }
    }
    return myTree;
}

void
Junction::cleanup() {
    delete myTree;
    myTree = nullptr;
}

void
Junction::storeShape(const std::string& id, PositionVector& shape) {
    shape.push_back(getJunction(id)->getPosition());
}


std::shared_ptr<VariableWrapper>
Junction::makeWrapper() {
    return std::make_shared<Helper::SubscriptionWrapper>(handleVariable, mySubscriptionResults, myContextSubscriptionResults);
}


bool
Junction::handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper) {
    switch (variable) {
        case TRACI_ID_LIST:
            return wrapper->wrapStringList(objID, variable, getIDList());
        case ID_COUNT:
            return wrapper->wrapInt(objID, variable, getIDCount());
        case VAR_POSITION:
            return wrapper->wrapPosition(objID, variable, getPosition(objID));
        default:
            return false;
    }
}


}


/****************************************************************************/
