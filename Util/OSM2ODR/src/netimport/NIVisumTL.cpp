/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2003-2020 German Aerospace Center (DLR) and others.
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
/// @file    NIVisumTL.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Thr, 08 May 2003
///
// Intermediate class for storing visum traffic lights during their import
/****************************************************************************/
#include <config.h>

#include <string>
#include <utils/options/OptionsCont.h>
#include <utils/common/MsgHandler.h>
#include <netbuild/NBLoadedTLDef.h>
#include <netbuild/NBTrafficLightLogicCont.h>
#include <netbuild/NBEdgeCont.h>
#include "NIVisumTL.h"


// ===========================================================================
// method definitions
// ===========================================================================
NIVisumTL::NIVisumTL(const std::string& name, SUMOTime cycleTime, SUMOTime offset,
                     SUMOTime intermediateTime, bool phaseDefined)
    : myName(name), myCycleTime(cycleTime), myOffset(offset),
      myIntermediateTime(intermediateTime), myPhaseDefined(phaseDefined) {
}


NIVisumTL::~NIVisumTL() {
    for (std::map<std::string, Phase*>::iterator i = myPhases.begin(); i != myPhases.end(); ++i) {
        delete i->second;
    }
    for (std::map<std::string, SignalGroup*>::iterator i = mySignalGroups.begin(); i != mySignalGroups.end(); ++i) {
        delete i->second;
    }
}


void
NIVisumTL::addSignalGroup(const std::string& name, SUMOTime startTime, SUMOTime endTime, SUMOTime yellowTime) {
    mySignalGroups[name] = new NIVisumTL::SignalGroup(name, startTime, endTime, yellowTime);
}


void
NIVisumTL::addPhase(const std::string& name, SUMOTime startTime, SUMOTime endTime, SUMOTime yellowTime) {
    myPhases[name] = new NIVisumTL::Phase(startTime, endTime, yellowTime);
}


NIVisumTL::SignalGroup&
NIVisumTL::getSignalGroup(const std::string& name) {
    return *mySignalGroups.find(name)->second;
}


void
NIVisumTL::build(NBEdgeCont& ec, NBTrafficLightLogicCont& tlc) {
    for (std::vector<NBNode*>::iterator ni = myNodes.begin(); ni != myNodes.end(); ni++) {
        NBNode* node = (*ni);
        if (node == nullptr) {
            WRITE_WARNING("invalid node for traffic light '" + myName + "'");
            continue;
        }
        TrafficLightType type = SUMOXMLDefinitions::TrafficLightTypes.get(OptionsCont::getOptions().getString("tls.default-type"));
        NBLoadedTLDef* def = new NBLoadedTLDef(ec, node->getID(), node, myOffset, type);
        tlc.insert(def);
        def->setCycleDuration((int) myCycleTime);
        // signalgroups
        for (std::map<std::string, SignalGroup*>::iterator gi = mySignalGroups.begin(); gi != mySignalGroups.end(); gi++) {
            std::string groupName = (*gi).first;
            NIVisumTL::SignalGroup& SG = *(*gi).second;
            def->addSignalGroup(groupName);
            def->addToSignalGroup(groupName, SG.connections());
            // phases
            SUMOTime yellowTime = -1;
            if (myPhaseDefined) {
                for (std::map<std::string, Phase*>::iterator pi = SG.phases().begin(); pi != SG.phases().end(); pi++) {
                    NIVisumTL::Phase& PH = *(*pi).second;
                    def->addSignalGroupPhaseBegin(groupName, PH.getStartTime(), NBTrafficLightDefinition::TLCOLOR_GREEN);
                    def->addSignalGroupPhaseBegin(groupName, PH.getEndTime(), NBTrafficLightDefinition::TLCOLOR_RED);
                    yellowTime = MAX2(PH.getYellowTime(), yellowTime);
                };
            } else {
                def->addSignalGroupPhaseBegin(groupName, SG.getStartTime(), NBTrafficLightDefinition::TLCOLOR_GREEN);
                def->addSignalGroupPhaseBegin(groupName, SG.getEndTime(), NBTrafficLightDefinition::TLCOLOR_RED);
                yellowTime = MAX2(SG.getYellowTime(), yellowTime);
            }
            // yellowTime can be -1 if not given in the input; it will be "patched" later
            def->setSignalYellowTimes(groupName, myIntermediateTime, yellowTime);
        }
    }
}


/****************************************************************************/
