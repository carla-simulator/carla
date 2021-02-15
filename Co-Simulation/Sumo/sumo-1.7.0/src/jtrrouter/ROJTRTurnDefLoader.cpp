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
/// @file    ROJTRTurnDefLoader.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 20 Jan 2004
///
// Loader for the of turning percentages and source/sink definitions
/****************************************************************************/
#include <config.h>

#include <set>
#include <string>
#include <utils/common/FileHelpers.h>
#include <utils/xml/XMLSubSys.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringUtils.h>
#include <utils/common/ToString.h>
#include <utils/options/OptionsCont.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/vehicle/SUMOVehicleParserHelper.h>
#include <router/RONet.h>
#include "ROJTREdge.h"
#include "ROJTRTurnDefLoader.h"


// ===========================================================================
// method definitions
// ===========================================================================
ROJTRTurnDefLoader::ROJTRTurnDefLoader(RONet& net) :
    SUMOSAXHandler("turn-ratio-file"), myNet(net),
    myIntervalBegin(0), myIntervalEnd(STEPS2TIME(SUMOTime_MAX)),
    myEdge(nullptr),
    mySourcesAreSinks(OptionsCont::getOptions().getBool("sources-are-sinks")),
    myDiscountSources(OptionsCont::getOptions().getBool("discount-sources")),
    myHaveWarnedAboutDeprecatedFormat(false)
{}


ROJTRTurnDefLoader::~ROJTRTurnDefLoader() {}


void
ROJTRTurnDefLoader::myStartElement(int element,
                                   const SUMOSAXAttributes& attrs) {
    bool ok = true;
    switch (element) {
        case SUMO_TAG_INTERVAL:
            myIntervalBegin = attrs.get<double>(SUMO_ATTR_BEGIN, nullptr, ok);
            myIntervalEnd = attrs.get<double>(SUMO_ATTR_END, nullptr, ok);
            break;
        case SUMO_TAG_FROMEDGE:
            if (!myHaveWarnedAboutDeprecatedFormat) {
                myHaveWarnedAboutDeprecatedFormat = true;
                WRITE_WARNING("The turn-file format with elements " + toString(SUMO_TAG_FROMEDGE) + ", " + toString(SUMO_TAG_TOEDGE) + " is deprecated,"
                              + " please use " + toString(SUMO_TAG_EDGEREL) + " instead.");
            }
            beginFromEdge(attrs);
            break;
        case SUMO_TAG_TOEDGE:
            addToEdge(attrs);
            break;
        case SUMO_TAG_EDGEREL:
            addEdgeRel(attrs);
            break;
        case SUMO_TAG_SINK:
            if (attrs.hasAttribute(SUMO_ATTR_EDGES)) {
                std::string edges = attrs.get<std::string>(SUMO_ATTR_EDGES, nullptr, ok);
                StringTokenizer st(edges, StringTokenizer::WHITECHARS);
                while (st.hasNext()) {
                    std::string id = st.next();
                    ROEdge* edge = myNet.getEdge(id);
                    if (edge == nullptr) {
                        throw ProcessError("The edge '" + id + "' declared as a sink is not known.");
                    }
                    edge->setSink();
                }
            }
            break;
        case SUMO_TAG_FLOW: {
            const std::string flowID = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
            if (attrs.hasAttribute(SUMO_ATTR_FROM)) {
                const std::string edgeID = attrs.get<std::string>(SUMO_ATTR_FROM, nullptr, ok);
                ROEdge* edge = myNet.getEdge(edgeID);
                if (edge == nullptr) {
                    throw ProcessError("The from-edge '" + edgeID + "' in flow '" + flowID + "' is not known.");
                }
                if (mySourcesAreSinks) {
                    edge->setSink();
                }
                if (myDiscountSources) {
                    SUMOVehicleParameter* pars = SUMOVehicleParserHelper::parseFlowAttributes(SUMO_TAG_FLOW, attrs, true, 0, TIME2STEPS(3600 * 24));
                    int numVehs = 0;
                    if (pars->repetitionProbability > 0) {
                        numVehs = int(STEPS2TIME(pars->repetitionEnd - pars->depart) * pars->repetitionProbability);
                    } else {
                        numVehs = pars->repetitionNumber;
                    }
                    delete pars;
                    static_cast<ROJTREdge*>(edge)->changeSourceFlow(numVehs);
                }
            } else {
                WRITE_WARNINGF("Ignoring flow '%' without 'from'", flowID);
            }
            break;
        }
        case SUMO_TAG_SOURCE:
            if (attrs.hasAttribute(SUMO_ATTR_EDGES)) {
                std::string edges = attrs.get<std::string>(SUMO_ATTR_EDGES, nullptr, ok);
                StringTokenizer st(edges, StringTokenizer::WHITECHARS);
                while (st.hasNext()) {
                    std::string id = st.next();
                    ROEdge* edge = myNet.getEdge(id);
                    if (edge == nullptr) {
                        throw ProcessError("The edge '" + id + "' declared as a source is not known.");
                    }
                    edge->setSource();
                }
            }
            break;
        default:
            break;
    }
}


void
ROJTRTurnDefLoader::beginFromEdge(const SUMOSAXAttributes& attrs) {
    myEdge = nullptr;
    bool ok = true;
    // get the id, report an error if not given or empty...
    std::string id = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
    if (!ok) {
        return;
    }
    //
    myEdge = static_cast<ROJTREdge*>(myNet.getEdge(id));
    if (myEdge == nullptr) {
        WRITE_ERROR("The edge '" + id + "' is not known within the network (within a 'from-edge' tag).");
        return;
    }
}


void
ROJTRTurnDefLoader::addToEdge(const SUMOSAXAttributes& attrs) {
    if (myEdge == nullptr) {
        return;
    }
    bool ok = true;
    // get the id, report an error if not given or empty...
    std::string id = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
    if (!ok) {
        return;
    }
    //
    ROJTREdge* edge = static_cast<ROJTREdge*>(myNet.getEdge(id));
    if (edge == nullptr) {
        WRITE_ERROR("The edge '" + id + "' is not known within the network (within a 'to-edge' tag).");
        return;
    }
    const double probability = attrs.get<double>(SUMO_ATTR_PROB, id.c_str(), ok);
    if (ok) {
        if (probability < 0) {
            WRITE_ERROR("'probability' must be positive (in definition of to-edge '" + id + "').");
        } else {
            myEdge->addFollowerProbability(edge, myIntervalBegin, myIntervalEnd, probability);
        }
    }
}


void
ROJTRTurnDefLoader::addEdgeRel(const SUMOSAXAttributes& attrs) {
    bool ok = true;
    // get the id, report an error if not given or empty...
    std::string fromID = attrs.get<std::string>(SUMO_ATTR_FROM, nullptr, ok);
    std::string toID = attrs.get<std::string>(SUMO_ATTR_TO, nullptr, ok);
    double probability = attrs.get<double>(
                             attrs.hasAttribute(SUMO_ATTR_COUNT) && !attrs.hasAttribute(SUMO_ATTR_PROB) ? SUMO_ATTR_COUNT : SUMO_ATTR_PROB,
                             (fromID + "->" + toID).c_str(), ok);
    if (!ok) {
        return;
    }
    //
    ROJTREdge* from = static_cast<ROJTREdge*>(myNet.getEdge(fromID));
    if (from == nullptr) {
        WRITE_ERROR("The edge '" + fromID + "' is not known.");
        return;
    }
    ROJTREdge* to = static_cast<ROJTREdge*>(myNet.getEdge(toID));
    if (to == nullptr) {
        WRITE_ERROR("The edge '" + toID + "' is not known.");
        return;
    }
    if (probability < 0) {
        WRITE_ERROR("'probability' must be positive (in edgeRelation from '" + fromID + "' to '" + toID + "'.");
    } else {
        from->addFollowerProbability(to, myIntervalBegin, myIntervalEnd, probability);
    }
}


/****************************************************************************/
