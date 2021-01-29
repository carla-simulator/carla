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
/// @file    RODFDetFlowLoader.cpp
/// @author  Daniel Krajzewicz
/// @author  Eric Nicolay
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Thu, 16.03.2006
///
// A loader for detector flows
/****************************************************************************/
#include <config.h>

#include <string>
#include <fstream>
#include <sstream>
#include <utils/importio/LineReader.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/StringUtils.h>
#include <utils/common/UtilExceptions.h>
#include "RODFDetFlowLoader.h"


// ===========================================================================
// method definitions
// ===========================================================================
RODFDetFlowLoader::RODFDetFlowLoader(const RODFDetectorCon& dets,
                                     RODFDetectorFlows& into,
                                     SUMOTime startTime, SUMOTime endTime,
                                     SUMOTime timeOffset, SUMOTime timeScale)
    : myStorage(into), myTimeOffset(timeOffset), myTimeScale(timeScale),
      myStartTime(startTime), myEndTime(endTime), myDetectorContainer(dets),
      myHaveWarnedAboutOverridingBoundaries(false), myHaveWarnedAboutPartialDefs(false) {}



RODFDetFlowLoader::~RODFDetFlowLoader() {}


void
RODFDetFlowLoader::read(const std::string& file) {
    LineReader lr(file);
    // parse first line
    myLineHandler.reinit(lr.readLine(), ";", ";", true, true);
    // parse values
    while (lr.hasMore()) {
        std::string line = lr.readLine();
        if (line.find(';') == std::string::npos) {
            continue;
        }
        myLineHandler.parseLine(line);
        try {
            std::string detName = myLineHandler.get("detector");
            if (!myDetectorContainer.knows(detName)) {
                continue;
            }
            const double parsedTime = StringUtils::toDouble((myLineHandler.get("time"))) * myTimeScale - myTimeOffset;
            // parsing as float to handle values which would cause int overflow
            if (parsedTime < myStartTime || parsedTime >= myEndTime) {
                if (!myHaveWarnedAboutOverridingBoundaries) {
                    myHaveWarnedAboutOverridingBoundaries = true;
                    WRITE_WARNING("At least one value lies beyond given time boundaries.");
                }
                continue;
            }
            const SUMOTime time = (SUMOTime)(parsedTime + .5);
            FlowDef fd;
            fd.isLKW = 0;
            fd.qPKW = StringUtils::toDouble(myLineHandler.get("qpkw"));
            fd.vPKW = 0;
            if (myLineHandler.know("vPKW")) {
                fd.vPKW = StringUtils::toDouble(myLineHandler.get("vpkw"));
            }
            fd.qLKW = 0;
            if (myLineHandler.know("qLKW")) {
                fd.qLKW = StringUtils::toDouble(myLineHandler.get("qlkw"));
            }
            fd.vLKW = 0;
            if (myLineHandler.know("vLKW")) {
                fd.vLKW = StringUtils::toDouble(myLineHandler.get("vlkw"));
            }
            if (fd.qLKW < 0) {
                fd.qLKW = 0;
            }
            if (fd.qPKW < 0) {
                fd.qPKW = 0;
            }
            myStorage.addFlow(detName, time, fd);
            if (!myHaveWarnedAboutPartialDefs && !myLineHandler.hasFullDefinition()) {
                myHaveWarnedAboutPartialDefs = true;
                WRITE_WARNING("At least one line does not contain the correct number of columns.");
            }
            continue;
        } catch (ProcessError& e) {
            throw ProcessError(toString(e.what()) + " in '" + lr.getFileName() + "', line " + toString(lr.getLineNumber()) + ";\n"
                               + " The following values must be supplied : 'Detector', 'Time', 'qPKW'\n"
                               + " The according column names must be given in the first line of the file.");
        }
    }
}


/****************************************************************************/
