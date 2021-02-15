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
/// @file    MEInductLoop.cpp
/// @author  Daniel Krajzewicz
/// @date    Tue, May 2005
///
// An induction loop for mesoscopic simulation
/****************************************************************************/


/* =========================================================================
 * included modules
 * ======================================================================= */
#include <config.h>

#include "MEInductLoop.h"
#include <cassert>
#include <numeric>
#include <utility>
#include <limits>
#include <utils/common/WrappingCommand.h>
#include <utils/common/ToString.h>
#include <microsim/MSEdge.h>
#include <microsim/MSEventControl.h>
#include <mesosim/MESegment.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/StringUtils.h>


// ===========================================================================
// method definitions
// ===========================================================================
MEInductLoop::MEInductLoop(const std::string& id,
                           MESegment* s,
                           double positionInMeters,
                           const std::string& vTypes) :
    MSDetectorFileOutput(id, vTypes), mySegment(s),
    myPosition(positionInMeters),
    myMeanData(nullptr, mySegment->getLength(), false, nullptr) {
    myMeanData.setDescription("inductionLoop_" + id);
    s->addDetector(&myMeanData);
}


MEInductLoop::~MEInductLoop() {}


void
MEInductLoop::writeXMLOutput(OutputDevice& dev,
                             SUMOTime startTime, SUMOTime stopTime) {
    mySegment->prepareDetectorForWriting(myMeanData);
    dev.openTag(SUMO_TAG_INTERVAL).writeAttr(SUMO_ATTR_BEGIN, time2string(startTime)).writeAttr(SUMO_ATTR_END, time2string(stopTime));
    dev.writeAttr(SUMO_ATTR_ID, StringUtils::escapeXML(myID)).writeAttr("sampledSeconds", myMeanData.getSamples());
    myMeanData.write(dev, 0, stopTime - startTime, (double)mySegment->getEdge().getLanes().size(), -1.0);
    myMeanData.reset();
}


/****************************************************************************/
