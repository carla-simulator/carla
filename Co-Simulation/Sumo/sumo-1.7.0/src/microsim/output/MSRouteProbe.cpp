/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2008-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSRouteProbe.cpp
/// @author  Michael Behrisch
/// @author  Daniel Krajzewicz
/// @author  Tino Morenz
/// @author  Jakob Erdmann
/// @date    Thu, 04.12.2008
///
// Writes route distributions at a certain edge
/****************************************************************************/
#include <config.h>

#include <string>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSGlobals.h>
#include <microsim/MSRoute.h>
#include <microsim/MSVehicle.h>
#include <utils/common/ToString.h>
#include <utils/iodevices/OutputDevice.h>
#include <mesosim/MELoop.h>
#include <mesosim/MESegment.h>
#include "MSRouteProbe.h"


// ===========================================================================
// method definitions
// ===========================================================================
MSRouteProbe::MSRouteProbe(const std::string& id, const MSEdge* edge, const std::string& distID, const std::string& lastID,
                           const std::string& vTypes) :
    MSDetectorFileOutput(id, vTypes), MSMoveReminder(id),
    myEdge(edge) {
    myCurrentRouteDistribution = std::make_pair(distID, MSRoute::distDictionary(distID));
    if (myCurrentRouteDistribution.second == 0) {
        myCurrentRouteDistribution.second = new RandomDistributor<const MSRoute*>();
        MSRoute::dictionary(distID, myCurrentRouteDistribution.second, false);
    }
    myLastRouteDistribution = std::make_pair(lastID, MSRoute::distDictionary(lastID));
    if (MSGlobals::gUseMesoSim) {
        MESegment* seg = MSGlobals::gMesoNet->getSegmentForEdge(*edge);
        while (seg != nullptr) {
            seg->addDetector(this);
            seg = seg->getNextSegment();
        }
        return;
    }
    for (std::vector<MSLane*>::const_iterator it = edge->getLanes().begin(); it != edge->getLanes().end(); ++it) {
        (*it)->addMoveReminder(this);
    }
}


MSRouteProbe::~MSRouteProbe() {
}


bool
MSRouteProbe::notifyEnter(SUMOTrafficObject& veh, MSMoveReminder::Notification reason, const MSLane* /* enteredLane */) {
    if (!vehicleApplies(veh)) {
        return false;
    }
    if (reason != MSMoveReminder::NOTIFICATION_SEGMENT && reason != MSMoveReminder::NOTIFICATION_LANE_CHANGE) {
        SUMOVehicle* vehicle = dynamic_cast<SUMOVehicle*>(&veh);
        if (vehicle != nullptr) {
            if (myCurrentRouteDistribution.second->add(&vehicle->getRoute(), 1.)) {
                vehicle->getRoute().addReference();
            }
        }
    }
    return false;
}


void
MSRouteProbe::writeXMLOutput(OutputDevice& dev,
                             SUMOTime startTime, SUMOTime stopTime) {
    if (myCurrentRouteDistribution.second->getOverallProb() > 0) {
        dev.openTag("routeDistribution") << " id=\"" << getID() + "_" + time2string(startTime) << "\"";
        const std::vector<const MSRoute*>& routes = myCurrentRouteDistribution.second->getVals();
        const std::vector<double>& probs = myCurrentRouteDistribution.second->getProbs();
        for (int j = 0; j < (int)routes.size(); ++j) {
            const MSRoute* r = routes[j];
            dev.openTag("route") << " id=\"" << r->getID() + "_" + time2string(startTime) << "\" edges=\"";
            for (MSRouteIterator i = r->begin(); i != r->end(); ++i) {
                if (i != r->begin()) {
                    dev << " ";
                }
                dev << (*i)->getID();
            }
            dev << "\" probability=\"" << probs[j] << "\"";
            dev.closeTag();
        }
        dev.closeTag();
        if (myLastRouteDistribution.second != 0) {
            MSRoute::checkDist(myLastRouteDistribution.first);
        }
        myLastRouteDistribution = myCurrentRouteDistribution;
        myCurrentRouteDistribution.first = getID() + "_" + toString(stopTime);
        myCurrentRouteDistribution.second = new RandomDistributor<const MSRoute*>();
        MSRoute::dictionary(myCurrentRouteDistribution.first, myCurrentRouteDistribution.second, false);
    }
}


void
MSRouteProbe::writeXMLDetectorProlog(OutputDevice& dev) const {
    dev.writeXMLHeader("routes", "routes_file.xsd");
}


const MSRoute*
MSRouteProbe::sampleRoute(bool last) const {
    if (myLastRouteDistribution.second == 0 || !last) {
        if (myCurrentRouteDistribution.second->getOverallProb() > 0) {
            return myCurrentRouteDistribution.second->get();
        }
        return nullptr;
    }
    return myLastRouteDistribution.second->get();
}
