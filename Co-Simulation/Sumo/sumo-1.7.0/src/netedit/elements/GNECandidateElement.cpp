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
/// @file    GNECandidateElement.cpp
/// @author  Pablo Alvarez Lopez
/// @date    May 2020
///
// class for candidate elements
/****************************************************************************/


// ===========================================================================
// included modules
// ===========================================================================

#include "GNECandidateElement.h"

// ===========================================================================
// method definitions
// ===========================================================================


GNECandidateElement::GNECandidateElement() :
    myPossibleCandidate(false),
    mySourceCandidate(false),
    myTargetCandidate(false),
    mySpecialCandidate(false),
    myConflictedCandidate(false),
    myReachability(-1) {}


GNECandidateElement::~GNECandidateElement() {}


void
GNECandidateElement::resetCandidateFlags() {
    myPossibleCandidate = false;
    mySourceCandidate = false;
    myTargetCandidate = false;
    mySpecialCandidate = false;
    myConflictedCandidate = false;
}


bool
GNECandidateElement::isPossibleCandidate() const {
    return myPossibleCandidate;
}



bool
GNECandidateElement::isSourceCandidate() const {
    return mySourceCandidate;
}


bool
GNECandidateElement::isTargetCandidate() const {
    return myTargetCandidate;
}


bool
GNECandidateElement::isSpecialCandidate() const {
    return mySpecialCandidate;
}


bool
GNECandidateElement::isConflictedCandidate() const {
    return myConflictedCandidate;
}


void
GNECandidateElement::setPossibleCandidate(const bool value) {
    myPossibleCandidate = value;
}


void
GNECandidateElement::setSourceCandidate(const bool value) {
    mySourceCandidate = value;
}


void
GNECandidateElement::setTargetCandidate(const bool value) {
    myTargetCandidate = value;
}


void
GNECandidateElement::setSpecialCandidate(const bool value) {
    mySpecialCandidate = value;
}


void
GNECandidateElement::setConflictedCandidate(const bool value) {
    myConflictedCandidate = value;
}


double
GNECandidateElement::getReachability() const {
    return myReachability;
}


void
GNECandidateElement::setReachability(const double reachability) {
    myReachability = reachability;
}


void
GNECandidateElement::resetReachability() {
    myReachability = -1;
}

/****************************************************************************/
