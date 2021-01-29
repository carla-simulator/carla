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
/// @file    RODFDetectorHandler.cpp
/// @author  Daniel Krajzewicz
/// @author  Eric Nicolay
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Thu, 16.03.2006
///
// A handler for loading detector descriptions
/****************************************************************************/
#include <config.h>

#include <string>
#include <utils/options/OptionsCont.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/StringUtils.h>
#include <utils/common/ToString.h>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include "RODFDetectorHandler.h"
#include "RODFNet.h"


// ===========================================================================
// method definitions
// ===========================================================================
RODFDetectorHandler::RODFDetectorHandler(RODFNet* optNet, bool ignoreErrors, RODFDetectorCon& con,
        const std::string& file)
    : SUMOSAXHandler(file),
      myNet(optNet), myIgnoreErrors(ignoreErrors), myContainer(con) {}


RODFDetectorHandler::~RODFDetectorHandler() {}


void
RODFDetectorHandler::myStartElement(int element,
                                    const SUMOSAXAttributes& attrs) {
    if (element == SUMO_TAG_DETECTOR_DEFINITION || element == SUMO_TAG_E1DETECTOR || element == SUMO_TAG_INDUCTION_LOOP) {
        try {
            bool ok = true;
            // get the id, report an error if not given or empty...
            std::string id = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
            if (!ok) {
                throw ProcessError();
            }
            std::string lane = attrs.get<std::string>(SUMO_ATTR_LANE, id.c_str(), ok);
            if (!ok) {
                throw ProcessError();
            }
            ROEdge* edge = myNet->getEdge(lane.substr(0, lane.rfind('_')));
            int laneIndex = StringUtils::toIntSecure(lane.substr(lane.rfind('_') + 1), std::numeric_limits<int>::max());
            if (edge == nullptr || laneIndex >= edge->getNumLanes()) {
                throw ProcessError("Unknown lane '" + lane + "' for detector '" + id + "' in '" + getFileName() + "'.");
            }
            double pos = attrs.get<double>(SUMO_ATTR_POSITION, id.c_str(), ok);
            std::string mml_type = attrs.getOpt<std::string>(SUMO_ATTR_TYPE, id.c_str(), ok, "");
            if (!ok) {
                throw ProcessError();
            }
            RODFDetectorType type = TYPE_NOT_DEFINED;
            if (mml_type == "between") {
                type = BETWEEN_DETECTOR;
            } else if (mml_type == "source" || mml_type == "highway_source") { // !!! highway-source is legacy (removed accoring output on 06.08.2007)
                type = SOURCE_DETECTOR;
            } else if (mml_type == "sink") {
                type = SINK_DETECTOR;
            }
            RODFDetector* detector = new RODFDetector(id, lane, pos, type);
            if (!myContainer.addDetector(detector)) {
                delete detector;
                throw ProcessError("Could not add detector '" + id + "' (probably the id is already used).");
            }
        } catch (ProcessError& e) {
            if (myIgnoreErrors) {
                WRITE_WARNING(e.what());
            } else {
                throw e;
            }
        }
    }
}


/****************************************************************************/
