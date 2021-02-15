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
/// @file    marouter_main.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    Thu, 06 Jun 2002
///
// Main for MAROUTER
/****************************************************************************/
#include <config.h>

#ifdef HAVE_VERSION_H
#include <version.h>
#endif

#include <iostream>
#include <string>
#include <limits.h>
#include <ctime>
#include <vector>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <utils/common/FileHelpers.h>
#include <utils/common/StringUtils.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/SystemFrame.h>
#include <utils/common/RandHelper.h>
#include <utils/common/ToString.h>
#include <utils/distribution/Distribution_Points.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/iodevices/OutputDevice_String.h>
#include <utils/options/Option.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/OptionsIO.h>
#include <utils/router/RouteCostCalculator.h>
#include <utils/router/DijkstraRouter.h>
#include <utils/router/AStarRouter.h>
#include <utils/router/CHRouter.h>
#include <utils/router/CHRouterWrapper.h>
#include <utils/xml/XMLSubSys.h>
#include <od/ODCell.h>
#include <od/ODDistrict.h>
#include <od/ODDistrictCont.h>
#include <od/ODDistrictHandler.h>
#include <od/ODMatrix.h>
#include <router/ROEdge.h>
#include <router/ROLoader.h>
#include <router/RONet.h>
#include <router/RORoute.h>
#include <router/RORoutable.h>

#include "ROMAFrame.h"
#include "ROMAAssignments.h"
#include "ROMAEdgeBuilder.h"
#include "ROMARouteHandler.h"
#include "ROMAEdge.h"


// ===========================================================================
// functions
// ===========================================================================
/* -------------------------------------------------------------------------
 * data processing methods
 * ----------------------------------------------------------------------- */
/**
 * loads the net
 * The net is in this meaning made up by the net itself and the dynamic
 * weights which may be supplied in a separate file
 */
void
initNet(RONet& net, ROLoader& loader, OptionsCont& oc) {
    // load the net
    ROMAEdgeBuilder builder;
    ROEdge::setGlobalOptions(oc.getBool("weights.interpolate"));
    loader.loadNet(net, builder);
    // initialize the travel times
    /* const SUMOTime begin = string2time(oc.getString("begin"));
    const SUMOTime end = string2time(oc.getString("end"));
    for (std::map<std::string, ROEdge*>::const_iterator i = net.getEdgeMap().begin(); i != net.getEdgeMap().end(); ++i) {
        (*i).second->addTravelTime(STEPS2TIME(begin), STEPS2TIME(end), (*i).second->getLength() / (*i).second->getSpeedLimit());
    }*/
    // load the weights when wished/available
    if (oc.isSet("weight-files")) {
        loader.loadWeights(net, "weight-files", oc.getString("weight-attribute"), false, oc.getBool("weights.expand"));
    }
    if (oc.isSet("lane-weight-files")) {
        loader.loadWeights(net, "lane-weight-files", oc.getString("weight-attribute"), true, oc.getBool("weights.expand"));
    }
}


double
getTravelTime(const ROEdge* const edge, const ROVehicle* const /* veh */, double /* time */) {
    return edge->getLength() / edge->getSpeedLimit();
}


/**
 * Computes all pair shortest paths, saving them
 */
void
computeAllPairs(RONet& net, OptionsCont& oc) {
    OutputDevice::createDeviceByOption("all-pairs-output");
    OutputDevice& outFile = OutputDevice::getDeviceByOption("all-pairs-output");
    // build the router
    typedef DijkstraRouter<ROEdge, ROVehicle> Dijkstra;
    Dijkstra router(ROEdge::getAllEdges(), oc.getBool("ignore-errors"), &getTravelTime);
    ConstROEdgeVector into;
    const int numInternalEdges = net.getInternalEdgeNumber();
    const int numTotalEdges = (int)net.getEdgeNumber();
    for (int i = numInternalEdges; i < numTotalEdges; i++) {
        const Dijkstra::EdgeInfo& ei = router.getEdgeInfo(i);
        if (!ei.edge->isInternal()) {
            router.compute(ei.edge, nullptr, nullptr, 0, into);
            double fromEffort = router.getEffort(ei.edge, nullptr, 0);
            for (int j = numInternalEdges; j < numTotalEdges; j++) {
                double heuTT = router.getEdgeInfo(j).effort - fromEffort;
                outFile << heuTT;
                /*
                if (heuTT >
                        ei.edge->getDistanceTo(router.getEdgeInfo(j).edge)
                        && router.getEdgeInfo(j).traveltime != std::numeric_limits<double>::max()
                        ) {
                    std::cout << " heuristic failure: from=" << ei.edge->getID() << " to=" << router.getEdgeInfo(j).edge->getID()
                    << " fromEffort=" << fromEffort << " heuTT=" << heuTT << " airDist=" << ei.edge->getDistanceTo(router.getEdgeInfo(j).edge) << "\n";
                }
                */
            }
        }
    }
}


/**
 * Writes the travel times for a single interval
 */
void
writeInterval(OutputDevice& dev, const SUMOTime begin, const SUMOTime end, const RONet& net, const ROVehicle* const veh) {
    dev.openTag(SUMO_TAG_INTERVAL).writeAttr(SUMO_ATTR_BEGIN, time2string(begin)).writeAttr(SUMO_ATTR_END, time2string(end));
    for (std::map<std::string, ROEdge*>::const_iterator i = net.getEdgeMap().begin(); i != net.getEdgeMap().end(); ++i) {
        ROMAEdge* edge = static_cast<ROMAEdge*>(i->second);
        if (edge->getFunction() == SumoXMLEdgeFunc::NORMAL) {
            dev.openTag(SUMO_TAG_EDGE).writeAttr(SUMO_ATTR_ID, edge->getID());
            const double traveltime = edge->getTravelTime(veh, STEPS2TIME(begin));
            const double flow = edge->getFlow(STEPS2TIME(begin));
            dev.writeAttr("traveltime", traveltime);
            dev.writeAttr("speed", edge->getLength() / traveltime);
            dev.writeAttr("entered", flow);
            dev.writeAttr("flowCapacityRatio", 100. * flow / ROMAAssignments::getCapacity(edge));
            dev.closeTag();
        }
    }
    dev.closeTag();
}


/**
 * Computes the routes saving them
 */
void
computeRoutes(RONet& net, OptionsCont& oc, ODMatrix& matrix) {
    // build the router
    SUMOAbstractRouter<ROEdge, ROVehicle>* router = nullptr;
    const std::string measure = oc.getString("weight-attribute");
    const std::string routingAlgorithm = oc.getString("routing-algorithm");
    const double priorityFactor = oc.getFloat("weights.priority-factor");
    SUMOTime begin = string2time(oc.getString("begin"));
    SUMOTime end = string2time(oc.getString("end"));
    if (oc.isDefault("begin") && matrix.getBegin() >= 0) {
        begin = matrix.getBegin();
    }
    if (oc.isDefault("end") && matrix.getEnd() >= 0) {
        end = matrix.getEnd();
    }
    DijkstraRouter<ROEdge, ROVehicle>::Operation ttOp = oc.getInt("paths") > 1 ? &ROMAAssignments::getPenalizedTT : &ROEdge::getTravelTimeStatic;
    if (measure == "traveltime" && priorityFactor == 0) {
        if (routingAlgorithm == "dijkstra") {
            router = new DijkstraRouter<ROEdge, ROVehicle>(ROEdge::getAllEdges(), oc.getBool("ignore-errors"), ttOp, nullptr, false, nullptr, net.hasPermissions());
        } else if (routingAlgorithm == "astar") {
            router = new AStarRouter<ROEdge, ROVehicle>(ROEdge::getAllEdges(), oc.getBool("ignore-errors"), ttOp, nullptr, net.hasPermissions());
        } else if (routingAlgorithm == "CH") {
            const SUMOTime weightPeriod = (oc.isSet("weight-files") ?
                                           string2time(oc.getString("weight-period")) :
                                           SUMOTime_MAX);
            router = new CHRouter<ROEdge, ROVehicle>(ROEdge::getAllEdges(), oc.getBool("ignore-errors"), &ROEdge::getTravelTimeStatic, SVC_IGNORING, weightPeriod, net.hasPermissions(), false);
        } else if (routingAlgorithm == "CHWrapper") {
            const SUMOTime weightPeriod = (oc.isSet("weight-files") ?
                                           string2time(oc.getString("weight-period")) :
                                           SUMOTime_MAX);
            router = new CHRouterWrapper<ROEdge, ROVehicle>(
                ROEdge::getAllEdges(), oc.getBool("ignore-errors"), &ROEdge::getTravelTimeStatic,
                begin, end, weightPeriod, oc.getInt("routing-threads"));
        } else {
            throw ProcessError("Unknown routing Algorithm '" + routingAlgorithm + "'!");
        }
    } else {
        DijkstraRouter<ROEdge, ROVehicle>::Operation op;
        if (measure == "traveltime") {
            if (ROEdge::initPriorityFactor(priorityFactor)) {
                op = &ROEdge::getTravelTimeStaticPriorityFactor;
            } else {
                op = &ROEdge::getTravelTimeStatic;
            }
        } else if (measure == "CO") {
            op = &ROEdge::getEmissionEffort<PollutantsInterface::CO>;
        } else if (measure == "CO2") {
            op = &ROEdge::getEmissionEffort<PollutantsInterface::CO2>;
        } else if (measure == "PMx") {
            op = &ROEdge::getEmissionEffort<PollutantsInterface::PM_X>;
        } else if (measure == "HC") {
            op = &ROEdge::getEmissionEffort<PollutantsInterface::HC>;
        } else if (measure == "NOx") {
            op = &ROEdge::getEmissionEffort<PollutantsInterface::NO_X>;
        } else if (measure == "fuel") {
            op = &ROEdge::getEmissionEffort<PollutantsInterface::FUEL>;
        } else if (measure == "electricity") {
            op = &ROEdge::getEmissionEffort<PollutantsInterface::ELEC>;
        } else if (measure == "noise") {
            op = &ROEdge::getNoiseEffort;
        } else {
            op = &ROEdge::getStoredEffort;
        }
        if (measure != "traveltime" && !net.hasLoadedEffort()) {
            WRITE_WARNING("No weight data was loaded for attribute '" + measure + "'.");
        }
        router = new DijkstraRouter<ROEdge, ROVehicle>(ROEdge::getAllEdges(), oc.getBool("ignore-errors"), op, ttOp, false, nullptr, net.hasPermissions());
    }
    try {
        const RORouterProvider provider(router, nullptr, nullptr, nullptr);
        // prepare the output
        net.openOutput(oc);
        // process route definitions
        if (oc.isSet("timeline")) {
            matrix.applyCurve(matrix.parseTimeLine(oc.getStringVector("timeline"), oc.getBool("timeline.day-in-hours")));
        }
        matrix.sortByBeginTime();
        ROVehicle defaultVehicle(SUMOVehicleParameter(), nullptr, net.getVehicleTypeSecure(DEFAULT_VTYPE_ID), &net);
        ROMAAssignments a(begin, end, oc.getBool("additive-traffic"), oc.getFloat("weight-adaption"), oc.getInt("max-alternatives"), net, matrix, *router);
        a.resetFlows();
#ifdef HAVE_FOX
        // this is just to init the CHRouter with the default vehicle
        router->reset(&defaultVehicle);
        const int maxNumThreads = oc.getInt("routing-threads");
        while ((int)net.getThreadPool().size() < maxNumThreads) {
            new RONet::WorkerThread(net.getThreadPool(), provider);
        }
#endif
        std::string assignMethod = oc.getString("assignment-method");
        if (assignMethod == "UE") {
            WRITE_WARNING("Deterministic user equilibrium ('UE') is not implemented yet, using stochastic method ('SUE').");
            assignMethod = "SUE";
        }
        if (assignMethod == "incremental") {
            a.incremental(oc.getInt("max-iterations"), oc.getBool("verbose"));
        } else if (assignMethod == "SUE") {
            a.sue(oc.getInt("max-iterations"), oc.getInt("max-inner-iterations"),
                  oc.getInt("paths"), oc.getFloat("paths.penalty"), oc.getFloat("tolerance"), oc.getString("route-choice-method"));
        }
        // update path costs and output
        bool haveOutput = false;
        OutputDevice* dev = net.getRouteOutput();
        if (dev != nullptr) {
            std::vector<std::string> tazParamKeys;
            if (oc.isSet("taz-param")) {
                tazParamKeys = oc.getStringVector("taz-param");
            }
            std::map<SUMOTime, std::string> sortedOut;
            SUMOTime lastEnd = -1;
            int num = 0;
            for (const ODCell* const c : matrix.getCells()) {
                if (c->begin >= end || c->end <= begin ||
                        c->pathsVector.empty() || c->pathsVector.front()->getEdgeVector().empty()) {
                    continue;
                }
                if (lastEnd >= 0 && lastEnd <= c->begin) {
                    for (std::map<SUMOTime, std::string>::const_iterator desc = sortedOut.begin(); desc != sortedOut.end(); ++desc) {
                        dev->writePreformattedTag(desc->second);
                    }
                    sortedOut.clear();
                }
                if (c->departures.empty()) {
                    const SUMOTime b = MAX2(begin, c->begin);
                    const SUMOTime e = MIN2(end, c->end);
                    const int numVehs = int(c->vehicleNumber * (e - b) / (c->end - c->begin));
                    OutputDevice_String od(1);
                    od.openTag(SUMO_TAG_FLOW).writeAttr(SUMO_ATTR_ID, oc.getString("prefix") + toString(num++));
                    od.writeAttr(SUMO_ATTR_BEGIN, time2string(b)).writeAttr(SUMO_ATTR_END, time2string(e));
                    od.writeAttr(SUMO_ATTR_NUMBER, numVehs);
                    matrix.writeDefaultAttrs(od, oc.getBool("ignore-vehicle-type"), c);
                    od.openTag(SUMO_TAG_ROUTE_DISTRIBUTION);
                    for (RORoute* const r : c->pathsVector) {
                        r->setCosts(router->recomputeCosts(r->getEdgeVector(), &defaultVehicle, begin));
                        r->writeXMLDefinition(od, nullptr, true, false);
                    }
                    od.closeTag();
                    od.closeTag();
                    sortedOut[c->begin] += od.getString();
                } else {
                    for (std::map<SUMOTime, std::vector<std::string> >::const_iterator deps = c->departures.begin(); deps != c->departures.end(); ++deps) {
                        if (deps->first >= end || deps->first < begin) {
                            continue;
                        }
                        const std::string routeDistId = c->origin + "_" + c->destination + "_" + time2string(c->begin) + "_" + time2string(c->end);
                        for (const std::string& id : deps->second) {
                            OutputDevice_String od(1);
                            od.openTag(SUMO_TAG_VEHICLE).writeAttr(SUMO_ATTR_ID, id).writeAttr(SUMO_ATTR_DEPART, time2string(deps->first));
                            matrix.writeDefaultAttrs(od, oc.getBool("ignore-vehicle-type"), c);
                            od.openTag(SUMO_TAG_ROUTE_DISTRIBUTION);
                            for (RORoute* const r : c->pathsVector) {
                                r->setCosts(router->recomputeCosts(r->getEdgeVector(), &defaultVehicle, begin));
                                r->writeXMLDefinition(od, nullptr, true, false);
                            }
                            od.closeTag();
                            if (!tazParamKeys.empty()) {
                                od.openTag(SUMO_TAG_PARAM).writeAttr(SUMO_ATTR_KEY, tazParamKeys[0]).writeAttr(SUMO_ATTR_VALUE, c->origin).closeTag();
                                if (tazParamKeys.size() > 1) {
                                    od.openTag(SUMO_TAG_PARAM).writeAttr(SUMO_ATTR_KEY, tazParamKeys[1]).writeAttr(SUMO_ATTR_VALUE, c->destination).closeTag();
                                }
                            }
                            od.closeTag();
                            sortedOut[deps->first] += od.getString();
                        }
                    }
                }
                if (c->end > lastEnd) {
                    lastEnd = c->end;
                }
            }
            for (std::map<SUMOTime, std::string>::const_iterator desc = sortedOut.begin(); desc != sortedOut.end(); ++desc) {
                dev->writePreformattedTag(desc->second);
            }
            haveOutput = true;
        }
        if (OutputDevice::createDeviceByOption("netload-output", "meandata")) {
            if (oc.getBool("additive-traffic")) {
                writeInterval(OutputDevice::getDeviceByOption("netload-output"), begin, end, net, a.getDefaultVehicle());
            } else {
                SUMOTime lastCell = 0;
                for (std::vector<ODCell*>::const_iterator i = matrix.getCells().begin(); i != matrix.getCells().end(); ++i) {
                    if ((*i)->end > lastCell) {
                        lastCell = (*i)->end;
                    }
                }
                const SUMOTime interval = string2time(OptionsCont::getOptions().getString("aggregation-interval"));
                for (SUMOTime start = begin; start < MIN2(end, lastCell); start += interval) {
                    writeInterval(OutputDevice::getDeviceByOption("netload-output"), start, start + interval, net, a.getDefaultVehicle());
                }
            }
            haveOutput = true;
        }
        if (!haveOutput) {
            throw ProcessError("No output file given.");
        }
        // end the processing
        net.cleanup();
    } catch (ProcessError&) {
        net.cleanup();
        throw;
    }
}


/* -------------------------------------------------------------------------
 * main
 * ----------------------------------------------------------------------- */
int
main(int argc, char** argv) {
    OptionsCont& oc = OptionsCont::getOptions();
    oc.setApplicationDescription("Import O/D-matrices for macroscopic traffic assignment to generate SUMO routes");
    oc.setApplicationName("marouter", "Eclipse SUMO marouter Version " VERSION_STRING);
    int ret = 0;
    RONet* net = nullptr;
    try {
        XMLSubSys::init();
        ROMAFrame::fillOptions();
        OptionsIO::setArgs(argc, argv);
        OptionsIO::getOptions();
        if (oc.processMetaOptions(argc < 2)) {
            SystemFrame::close();
            return 0;
        }
        SystemFrame::checkOptions();
        XMLSubSys::setValidation(oc.getString("xml-validation"), oc.getString("xml-validation.net"), oc.getString("xml-validation.routes"));
        MsgHandler::initOutputOptions();
        if (!ROMAFrame::checkOptions()) {
            throw ProcessError();
        }
        RandHelper::initRandGlobal();
        // load data
        ROLoader loader(oc, false, false);
        net = new RONet();
        initNet(*net, loader, oc);
        if (oc.isSet("all-pairs-output")) {
            computeAllPairs(*net, oc);
            if (net->getDistricts().empty()) {
                delete net;
                SystemFrame::close();
                if (ret == 0) {
                    std::cout << "Success." << std::endl;
                }
                return ret;
            }
        }
        if (net->getDistricts().empty()) {
            WRITE_WARNING("No districts loaded, will use edge ids!");
        }
        // load districts
        ODDistrictCont districts;
        districts.makeDistricts(net->getDistricts());
        // load the matrix
        ODMatrix matrix(districts);
        matrix.loadMatrix(oc);
        ROMARouteHandler handler(matrix);
        matrix.loadRoutes(oc, handler);
        if (matrix.getNumLoaded() == matrix.getNumDiscarded()) {
            throw ProcessError("No valid vehicles loaded.");
        }
        if (MsgHandler::getErrorInstance()->wasInformed() && !oc.getBool("ignore-errors")) {
            throw ProcessError("Loading failed.");
        }
        MsgHandler::getErrorInstance()->clear();
        WRITE_MESSAGE(toString(matrix.getNumLoaded() - matrix.getNumDiscarded()) + " valid vehicles loaded (total seen: " + toString(matrix.getNumLoaded()) + ").");

        // build routes and parse the incremental rates if the incremental method is choosen.
        try {
            computeRoutes(*net, oc, matrix);
        } catch (XERCES_CPP_NAMESPACE::SAXParseException& e) {
            WRITE_ERROR(toString(e.getLineNumber()));
            ret = 1;
        } catch (XERCES_CPP_NAMESPACE::SAXException& e) {
            WRITE_ERROR(StringUtils::transcode(e.getMessage()));
            ret = 1;
        }
        if (MsgHandler::getErrorInstance()->wasInformed() || ret != 0) {
            throw ProcessError();
        }
    } catch (const ProcessError& e) {
        if (std::string(e.what()) != std::string("Process Error") && std::string(e.what()) != std::string("")) {
            WRITE_ERROR(e.what());
        }
        MsgHandler::getErrorInstance()->inform("Quitting (on error).", false);
        ret = 1;
    }

    delete net;
    SystemFrame::close();
    if (ret == 0) {
        std::cout << "Success." << std::endl;
    }
    return ret;
}


/****************************************************************************/
