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
/// @file    ROVehicle.cpp
/// @author  Daniel Krajzewicz
/// @author  Axel Wegener
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Sept 2002
///
// A vehicle as used by router
/****************************************************************************/
#include <config.h>

#include <string>
#include <iostream>
#include <utils/common/StringUtils.h>
#include <utils/common/ToString.h>
#include <utils/common/MsgHandler.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/vehicle/SUMOVTypeParameter.h>
#include <utils/options/OptionsCont.h>
#include <utils/iodevices/OutputDevice.h>
#include "RORouteDef.h"
#include "RORoute.h"
#include "ROHelper.h"
#include "RONet.h"
#include "ROLane.h"
#include "ROVehicle.h"


// ===========================================================================
// method definitions
// ===========================================================================
ROVehicle::ROVehicle(const SUMOVehicleParameter& pars,
                     RORouteDef* route, const SUMOVTypeParameter* type,
                     const RONet* net, MsgHandler* errorHandler)
    : RORoutable(pars, type), myRoute(route) {
    getParameter().stops.clear();
    if (route != nullptr && route->getFirstRoute() != nullptr) {
        for (std::vector<SUMOVehicleParameter::Stop>::const_iterator s = route->getFirstRoute()->getStops().begin(); s != route->getFirstRoute()->getStops().end(); ++s) {
            addStop(*s, net, errorHandler);
        }
    }
    for (std::vector<SUMOVehicleParameter::Stop>::const_iterator s = pars.stops.begin(); s != pars.stops.end(); ++s) {
        addStop(*s, net, errorHandler);
    }
    if (pars.via.size() != 0) {
        // via takes precedence over stop edges
        // XXX check for inconsistencies #2275
        myStopEdges.clear();
        for (std::vector<std::string>::const_iterator it = pars.via.begin(); it != pars.via.end(); ++it) {
            assert(net->getEdge(*it) != 0);
            myStopEdges.push_back(net->getEdge(*it));
        }
    }
}


void
ROVehicle::addStop(const SUMOVehicleParameter::Stop& stopPar, const RONet* net, MsgHandler* errorHandler) {
    const ROEdge* stopEdge = net->getEdgeForLaneID(stopPar.lane);
    assert(stopEdge != 0); // was checked when parsing the stop
    if (stopEdge->prohibits(this)) {
        if (errorHandler != nullptr) {
            errorHandler->inform("Stop edge '" + stopEdge->getID() + "' does not allow vehicle '" + getID() + "'.");
        }
        return;
    }
    // where to insert the stop
    std::vector<SUMOVehicleParameter::Stop>::iterator iter = getParameter().stops.begin();
    ConstROEdgeVector::iterator edgeIter = myStopEdges.begin();
    if (stopPar.index == STOP_INDEX_END || stopPar.index >= static_cast<int>(getParameter().stops.size())) {
        if (getParameter().stops.size() > 0) {
            iter = getParameter().stops.end();
            edgeIter = myStopEdges.end();
        }
    } else {
        if (stopPar.index == STOP_INDEX_FIT) {
            const ConstROEdgeVector edges = myRoute->getFirstRoute()->getEdgeVector();
            ConstROEdgeVector::const_iterator stopEdgeIt = std::find(edges.begin(), edges.end(), stopEdge);
            if (stopEdgeIt == edges.end()) {
                iter = getParameter().stops.end();
                edgeIter = myStopEdges.end();
            } else {
                while (iter != getParameter().stops.end()) {
                    if (edgeIter > stopEdgeIt || (edgeIter == stopEdgeIt && iter->endPos >= stopPar.endPos)) {
                        break;
                    }
                    ++iter;
                    ++edgeIter;
                }
            }
        } else {
            iter += stopPar.index;
            edgeIter += stopPar.index;
        }
    }
    getParameter().stops.insert(iter, stopPar);
    myStopEdges.insert(edgeIter, stopEdge);
}


ROVehicle::~ROVehicle() {}


const ROEdge*
ROVehicle:: getDepartEdge() const {
    return myRoute->getFirstRoute()->getFirst();
}


void
ROVehicle::computeRoute(const RORouterProvider& provider,
                        const bool removeLoops, MsgHandler* errorHandler) {
    SUMOAbstractRouter<ROEdge, ROVehicle>& router = provider.getVehicleRouter(getVClass());
    std::string noRouteMsg = "The vehicle '" + getID() + "' has no valid route.";
    RORouteDef* const routeDef = getRouteDefinition();
    // check if the route definition is valid
    if (routeDef == nullptr) {
        errorHandler->inform(noRouteMsg);
        myRoutingSuccess = false;
        return;
    }
    RORoute* current = routeDef->buildCurrentRoute(router, getDepartureTime(), *this);
    if (current == nullptr || current->size() == 0) {
        delete current;
        if (current == nullptr || !routeDef->discardSilent()) {
            errorHandler->inform(noRouteMsg);
        }
        myRoutingSuccess = false;
        return;
    }
    // check whether we have to evaluate the route for not containing loops
    if (removeLoops) {
        const ROEdge* requiredStart = (getParameter().departPosProcedure == DepartPosDefinition::GIVEN
                                       || getParameter().departLaneProcedure == DepartLaneDefinition::GIVEN ? current->getEdgeVector().front() : 0);
        const ROEdge* requiredEnd = (getParameter().arrivalPosProcedure == ArrivalPosDefinition::GIVEN
                                     || getParameter().arrivalLaneProcedure == ArrivalLaneDefinition::GIVEN ? current->getEdgeVector().back() : 0);
        current->recheckForLoops(getMandatoryEdges(requiredStart, requiredEnd));
        // check whether the route is still valid
        if (current->size() == 0) {
            delete current;
            errorHandler->inform(noRouteMsg + " (after removing loops)");
            myRoutingSuccess = false;
            return;
        }
    }
    // add built route
    routeDef->addAlternative(router, this, current, getDepartureTime());
    myRoutingSuccess = true;
}


ConstROEdgeVector
ROVehicle::getMandatoryEdges(const ROEdge* requiredStart, const ROEdge* requiredEnd) const {
    ConstROEdgeVector mandatory;
    if (requiredStart) {
        mandatory.push_back(requiredStart);
    }
    for (const ROEdge* e : getStopEdges()) {
        if (e->isInternal()) {
            // the edges before and after the internal edge are mandatory
            const ROEdge* before = e->getNormalBefore();
            const ROEdge* after = e->getNormalAfter();
            if (mandatory.size() == 0 || after != mandatory.back()) {
                mandatory.push_back(before);
                mandatory.push_back(after);
            }
        } else {
            if (mandatory.size() == 0 || e != mandatory.back()) {
                mandatory.push_back(e);
            }
        }
    }
    if (requiredEnd) {
        if (mandatory.size() < 2 || mandatory.back() != requiredEnd) {
            mandatory.push_back(requiredEnd);
        }
    }
    return mandatory;
}


void
ROVehicle::saveAsXML(OutputDevice& os, OutputDevice* const typeos, bool asAlternatives, OptionsCont& options) const {
    if (typeos != nullptr && getType() != nullptr && !getType()->saved) {
        getType()->write(*typeos);
        getType()->saved = true;
    }
    if (getType() != nullptr && !getType()->saved) {
        getType()->write(os);
        getType()->saved = asAlternatives;
    }

    const bool writeTrip = options.exists("write-trips") && options.getBool("write-trips");
    const bool writeGeoTrip = writeTrip && options.getBool("write-trips.geo");
    const bool writeJunctions = writeTrip && options.getBool("write-trips.junctions");
    // write the vehicle (new style, with included routes)
    getParameter().write(os, options, writeTrip ? SUMO_TAG_TRIP : SUMO_TAG_VEHICLE);

    // save the route
    if (writeTrip) {
        const ConstROEdgeVector edges = myRoute->getFirstRoute()->getEdgeVector();
        const ROEdge* from = nullptr;
        const ROEdge* to = nullptr;
        if (edges.size() > 0) {
            if (edges.front()->isTazConnector()) {
                if (edges.size() > 1) {
                    from = edges[1];
                    if (from->isTazConnector() && writeJunctions && edges.front()->getSuccessors().size() > 0) {
                        // routing was skipped
                        from = edges.front()->getSuccessors(getVClass()).front();
                    }
                }
            } else {
                from = edges[0];
            }
            if (edges.back()->isTazConnector()) {
                if (edges.size() > 1) {
                    to = edges[edges.size() - 2];
                    if (to->isTazConnector() && writeJunctions && edges.back()->getPredecessors().size() > 0) {
                        // routing was skipped
                        to = edges.back()->getPredecessors().front();
                    }
                }
            } else {
                to = edges[edges.size() - 1];
            }
        }
        if (from != nullptr) {
            if (writeGeoTrip) {
                Position fromPos = from->getLanes()[0]->getShape().positionAtOffset2D(0);
                if (GeoConvHelper::getFinal().usingGeoProjection()) {
                    os.setPrecision(gPrecisionGeo);
                    GeoConvHelper::getFinal().cartesian2geo(fromPos);
                    os.writeAttr(SUMO_ATTR_FROMLONLAT, fromPos);
                    os.setPrecision(gPrecision);
                } else {
                    os.writeAttr(SUMO_ATTR_FROMXY, fromPos);
                }
            } else if (writeJunctions) {
                os.writeAttr(SUMO_ATTR_FROMJUNCTION, from->getFromJunction()->getID());
            } else {
                os.writeAttr(SUMO_ATTR_FROM, from->getID());
            }
        }
        if (to != nullptr) {
            if (writeGeoTrip) {
                Position toPos = to->getLanes()[0]->getShape().positionAtOffset2D(to->getLanes()[0]->getShape().length2D());
                if (GeoConvHelper::getFinal().usingGeoProjection()) {
                    os.setPrecision(gPrecisionGeo);
                    GeoConvHelper::getFinal().cartesian2geo(toPos);
                    os.writeAttr(SUMO_ATTR_TOLONLAT, toPos);
                    os.setPrecision(gPrecision);
                } else {
                    os.writeAttr(SUMO_ATTR_TOXY, toPos);
                }
            } else if (writeJunctions) {
                os.writeAttr(SUMO_ATTR_TOJUNCTION, to->getToJunction()->getID());
            } else {
                os.writeAttr(SUMO_ATTR_TO, to->getID());
            }
        }
        if (getParameter().via.size() > 0) {
            std::vector<std::string> viaOut;
            SumoXMLAttr viaAttr = (writeGeoTrip
                                   ? (GeoConvHelper::getFinal().usingGeoProjection() ? SUMO_ATTR_VIALONLAT : SUMO_ATTR_VIAXY)
                                   : (writeJunctions ? SUMO_ATTR_VIAJUNCTIONS : SUMO_ATTR_VIA));
            for (const std::string& viaID : getParameter().via) {
                const ROEdge* viaEdge = RONet::getInstance()->getEdge(viaID);
                if (viaEdge->isTazConnector()) {
                    if (viaEdge->getPredecessors().size() == 0) {
                        continue;
                    }
                    // XXX used edge that was used in route
                    viaEdge = viaEdge->getPredecessors().front();
                }
                assert(viaEdge != nullptr);
                if (writeGeoTrip) {
                    Position viaPos = viaEdge->getLanes()[0]->getShape().positionAtOffset2D(viaEdge->getLanes()[0]->getShape().length2D() / 2);
                    if (GeoConvHelper::getFinal().usingGeoProjection()) {
                        GeoConvHelper::getFinal().cartesian2geo(viaPos);
                        viaOut.push_back(toString(viaPos, gPrecisionGeo));
                    } else {
                        viaOut.push_back(toString(viaPos, gPrecision));
                    }
                } else if (writeJunctions) {
                    viaOut.push_back(viaEdge->getToJunction()->getID());
                } else {
                    viaOut.push_back(viaEdge->getID());
                }
            }
            os.writeAttr(viaAttr, viaOut);
        }
    } else {
        myRoute->writeXMLDefinition(os, this, asAlternatives, options.getBool("exit-times"));
    }
    for (std::vector<SUMOVehicleParameter::Stop>::const_iterator stop = getParameter().stops.begin(); stop != getParameter().stops.end(); ++stop) {
        stop->write(os);
    }
    getParameter().writeParams(os);
    os.closeTag();
}


/****************************************************************************/
