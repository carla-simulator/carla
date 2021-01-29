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
/// @file    GNECandidateElement.h
/// @author  Pablo Alvarez Lopez
/// @date    May 2020
///
// class for candidate elements
/****************************************************************************/
#pragma once
#include <config.h>


// ===========================================================================
// class definitions
// ===========================================================================

class GNECandidateElement {

public:
    /// @brief Constructor
    GNECandidateElement();

    /// @brief Destructor
    ~GNECandidateElement();

    /// @brief reset candidate flags
    void resetCandidateFlags();

    /// @brief check if this element is a possible candidate
    bool isPossibleCandidate() const;

    /// @brief check if this element is a source candidate
    bool isSourceCandidate() const;

    /// @brief check if this element is a target candidate
    bool isTargetCandidate() const;

    /// @brief check if this element is a special candidate
    bool isSpecialCandidate() const;

    /// @brief check if this element is a conflicted candidate
    bool isConflictedCandidate() const;

    /// @brief set element as possible candidate
    void setPossibleCandidate(const bool value);

    /// @brief set element as source candidate
    void setSourceCandidate(const bool value);

    /// @brief set element as target candidate
    void setTargetCandidate(const bool value);

    /// @brief set element as special candidate
    void setSpecialCandidate(const bool value);

    /// @brief set element as conflicted candidate
    void setConflictedCandidate(const bool value);

    /// @name functions for reachability
    /// @{

    /// @brief get current reability (traveltime)
    double getReachability() const;

    /// @brief set current reachability (traveltime)
    void setReachability(const double reachability);

    /// @brief reset reachability (traveltime)
    void resetReachability();

    /// @}

protected:
    /// @brief flag to mark this element as possible candidate
    bool myPossibleCandidate;

    /// @brief flag to mark this element as source candidate
    bool mySourceCandidate;

    /// @brief flag to mark this element as target candidate
    bool myTargetCandidate;

    /// @brief flag to mark this element as special candidate
    bool mySpecialCandidate;

    /// @brief flag to mark this element as conflicted candidate
    bool myConflictedCandidate;

    /// @brief value for reachability
    double myReachability;

private:
    /// @brief Invalidated copy constructor.
    GNECandidateElement(const GNECandidateElement&) = delete;

    /// @brief Invalidated assignment operator
    GNECandidateElement& operator=(const GNECandidateElement& src) = delete;
};
