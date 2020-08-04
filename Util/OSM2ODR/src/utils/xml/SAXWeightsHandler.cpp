/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2007-2020 German Aerospace Center (DLR) and others.
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
/// @file    SAXWeightsHandler.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Fri, 30 Mar 2007
///
// An XML-handler for network weights
/****************************************************************************/
#include <config.h>

#include <utils/common/MsgHandler.h>

#include "SAXWeightsHandler.h"


// ===========================================================================
// method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// SAXWeightsHandler::ToRetrieveDefinition methods
// ---------------------------------------------------------------------------

SAXWeightsHandler::ToRetrieveDefinition::ToRetrieveDefinition(const std::string& attributeName,
        bool edgeBased, EdgeFloatTimeLineRetriever& destination) :
    myAttributeName(attributeName),
    myAmEdgeBased(edgeBased),
    myDestination(destination),
    myAggValue(0),
    myNoLanes(0),
    myHadAttribute(0) {
}


SAXWeightsHandler::ToRetrieveDefinition::~ToRetrieveDefinition() {
}

// ---------------------------------------------------------------------------
// SAXWeightsHandler methods
// ---------------------------------------------------------------------------

SAXWeightsHandler::SAXWeightsHandler(const std::vector<ToRetrieveDefinition*>& defs, const std::string& file) :
    SUMOSAXHandler(file),
    myDefinitions(defs),
    myCurrentTimeBeg(-1),
    myCurrentTimeEnd(-1) {
}


SAXWeightsHandler::SAXWeightsHandler(ToRetrieveDefinition* def, const std::string& file) :
    SUMOSAXHandler(file),
    myDefinitions({def}),
              myCurrentTimeBeg(-1),
myCurrentTimeEnd(-1) {
}


SAXWeightsHandler::~SAXWeightsHandler() {
    for (const auto& definition : myDefinitions) {
        delete definition;
    }
}


void
SAXWeightsHandler::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    switch (element) {
        case SUMO_TAG_INTERVAL: {
            bool ok = true;
            myCurrentTimeBeg = STEPS2TIME(attrs.getSUMOTimeReporting(SUMO_ATTR_BEGIN, nullptr, ok));
            myCurrentTimeEnd = STEPS2TIME(attrs.getSUMOTimeReporting(SUMO_ATTR_END, nullptr, ok));
        }
        break;
        case SUMO_TAG_EDGE: {
            bool ok = true;
            myCurrentEdgeID = attrs.getOpt<std::string>(SUMO_ATTR_ID, nullptr, ok, "");
            tryParse(attrs, true);
        }
        break;
        case SUMO_TAG_EDGEREL: {
            tryParseEdgeRel(attrs);
        }
        break;
        case SUMO_TAG_LANE: {
            tryParse(attrs, false);
        }
        break;
        default:
            break;
    }
}


void
SAXWeightsHandler::tryParse(const SUMOSAXAttributes& attrs, bool isEdge) {
    // !!!! no error handling!
    if (isEdge) {
        // process all that want values directly from the edge
        for (const auto& definition : myDefinitions) {
            if (definition->myAmEdgeBased) {
                if (attrs.hasAttribute(definition->myAttributeName)) {
                    definition->myAggValue = attrs.getFloat(definition->myAttributeName);
                    definition->myNoLanes = 1;
                    definition->myHadAttribute = true;
                } else {
                    definition->myHadAttribute = false;
                }
            } else {
                definition->myAggValue = 0;
                definition->myNoLanes = 0;
            }
        }
    } else {
        // process the current lane values
        for (const auto& definition : myDefinitions) {
            if (!definition->myAmEdgeBased) {
                try {
                    definition->myAggValue += attrs.getFloat(definition->myAttributeName);
                    definition->myNoLanes++;
                    definition->myHadAttribute = true;
                } catch (EmptyData&) {
                    WRITE_ERROR("Missing value '" + definition->myAttributeName + "' in edge '" + myCurrentEdgeID + "'.");
                } catch (NumberFormatException&) {
                    WRITE_ERROR("The value should be numeric, but is not.\n In edge '" + myCurrentEdgeID +
                                "' at time step " + toString(myCurrentTimeBeg) + ".");
                }
            }
        }
    }
}


void
SAXWeightsHandler::tryParseEdgeRel(const SUMOSAXAttributes& attrs) {
    if (attrs.hasAttribute(SUMO_ATTR_FROM) && attrs.hasAttribute(SUMO_ATTR_TO)) {
        bool ok = true;
        const std::string from = attrs.get<std::string>(SUMO_ATTR_FROM, nullptr, ok);
        const std::string to = attrs.get<std::string>(SUMO_ATTR_TO, nullptr, ok);
        for (ToRetrieveDefinition* ret : myDefinitions) {
            if (attrs.hasAttribute(ret->myAttributeName)) {
                ret->myDestination.addEdgeRelWeight(from, to,
                                                    attrs.getFloat(ret->myAttributeName),
                                                    myCurrentTimeBeg, myCurrentTimeEnd);
            }
        }
    }
}


void
SAXWeightsHandler::myEndElement(int element) {
    if (element == SUMO_TAG_EDGE) {
        for (const auto& definition : myDefinitions) {
            if (definition->myHadAttribute) {
                definition->myDestination.addEdgeWeight(myCurrentEdgeID,
                                                        definition->myAggValue / (double)definition->myNoLanes,
                                                        myCurrentTimeBeg, myCurrentTimeEnd);
            }
        }
    }
}


/****************************************************************************/
