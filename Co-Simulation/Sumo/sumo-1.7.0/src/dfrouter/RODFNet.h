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
/// @file    RODFNet.h
/// @author  Daniel Krajzewicz
/// @author  Eric Nicolay
/// @author  Michael Behrisch
/// @date    Thu, 16.03.2006
///
// A DFROUTER-network
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/options/OptionsCont.h>
#include <utils/common/StdDefs.h>
#include <utils/common/SUMOTime.h>
#include <router/ROEdge.h>
#include <router/RONet.h>
#include "RODFDetector.h"
#include "RODFRouteDesc.h"
#include "RODFRouteCont.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RODFNet
 * @brief A DFROUTER-network
 */
class RODFNet : public RONet {
public:
    /** @brief Constructor
     * @param[in] amInHighwayMode Whether search for following edges shall stop at slow edges
     */
    RODFNet(bool amInHighwayMode);


    /// @brief Destructor
    ~RODFNet();


    void buildApproachList();

    void computeTypes(RODFDetectorCon& dets,
                      bool sourcesStrict) const;
    void buildRoutes(RODFDetectorCon& det, bool keepUnfoundEnds, bool includeInBetween,
                     bool keepShortestOnly, int maxFollowingLength) const;
    double getAbsPos(const RODFDetector& det) const;

    void buildEdgeFlowMap(const RODFDetectorFlows& flows,
                          const RODFDetectorCon& detectors,
                          SUMOTime startTime, SUMOTime endTime, SUMOTime stepOffset);

    void revalidateFlows(const RODFDetectorCon& detectors,
                         RODFDetectorFlows& flows,
                         SUMOTime startTime, SUMOTime endTime, SUMOTime stepOffset);


    void removeEmptyDetectors(RODFDetectorCon& detectors,
                              RODFDetectorFlows& flows);

    void reportEmptyDetectors(RODFDetectorCon& detectors,
                              RODFDetectorFlows& flows);

    void buildDetectorDependencies(RODFDetectorCon& detectors);

    void mesoJoin(RODFDetectorCon& detectors, RODFDetectorFlows& flows);

    bool hasDetector(ROEdge* edge) const;
    const std::vector<std::string>& getDetectorList(ROEdge* edge) const;

    double getMaxSpeedFactorPKW() const {
        return myMaxSpeedFactorPKW;
    }

    double getMaxSpeedFactorLKW() const {
        return myMaxSpeedFactorLKW;
    }

    double getAvgSpeedFactorPKW() const {
        return myAvgSpeedFactorPKW;
    }

    double getAvgSpeedFactorLKW() const {
        return myAvgSpeedFactorLKW;
    }

protected:
    void revalidateFlows(const RODFDetector* detector,
                         RODFDetectorFlows& flows,
                         SUMOTime startTime, SUMOTime endTime, SUMOTime stepOffset);
    bool isSource(const RODFDetector& det,
                  const RODFDetectorCon& detectors, bool strict) const;
    bool isFalseSource(const RODFDetector& det,
                       const RODFDetectorCon& detectors) const;
    bool isDestination(const RODFDetector& det,
                       const RODFDetectorCon& detectors) const;

    ROEdge* getDetectorEdge(const RODFDetector& det) const;
    bool isSource(const RODFDetector& det, ROEdge* edge,
                  ROEdgeVector& seen, const RODFDetectorCon& detectors,
                  bool strict) const;
    bool isFalseSource(const RODFDetector& det, ROEdge* edge,
                       ROEdgeVector& seen, const RODFDetectorCon& detectors) const;
    bool isDestination(const RODFDetector& det, ROEdge* edge, ROEdgeVector& seen,
                       const RODFDetectorCon& detectors) const;

    void computeRoutesFor(ROEdge* edge, RODFRouteDesc& base, int no,
                          bool keepUnfoundEnds,
                          bool keepShortestOnly,
                          ROEdgeVector& visited, const RODFDetector& det,
                          RODFRouteCont& into, const RODFDetectorCon& detectors,
                          int maxFollowingLength,
                          ROEdgeVector& seen) const;

    void buildDetectorEdgeDependencies(RODFDetectorCon& dets) const;

    bool hasApproaching(ROEdge* edge) const;
    bool hasApproached(ROEdge* edge) const;

    bool hasInBetweenDetectorsOnly(ROEdge* edge,
                                   const RODFDetectorCon& detectors) const;
    bool hasSourceDetector(ROEdge* edge,
                           const RODFDetectorCon& detectors) const;

    struct IterationEdge {
        int depth;
        ROEdge* edge;
    };

protected:
    class DFRouteDescByTimeComperator {
    public:
        /// Constructor
        explicit DFRouteDescByTimeComperator() { }

        /// Destructor
        ~DFRouteDescByTimeComperator() { }

        /// Comparing method
        bool operator()(const RODFRouteDesc& nod1, const RODFRouteDesc& nod2) const {
            return nod1.duration_2 > nod2.duration_2;
        }
    };

private:
    /// @brief comparator for maps using edges as key, used only in myDetectorsOnEdges to make tests comparable
    struct idComp {
        bool operator()(ROEdge* const lhs, ROEdge* const rhs) const {
            return lhs->getID() < rhs->getID();
        }
    };

    /// @brief Map of edge name->list of names of this edge approaching edges
    std::map<ROEdge*, ROEdgeVector > myApproachingEdges;

    /// @brief Map of edge name->list of names of edges approached by this edge
    std::map<ROEdge*, ROEdgeVector > myApproachedEdges;

    mutable std::map<ROEdge*, std::vector<std::string>, idComp> myDetectorsOnEdges;
    mutable std::map<std::string, ROEdge*> myDetectorEdges;

    bool myAmInHighwayMode;
    mutable int mySourceNumber, mySinkNumber, myInBetweenNumber, myInvalidNumber;

    /// @brief List of ids of edges that shall not be used
    std::vector<std::string> myDisallowedEdges;


    bool myKeepTurnarounds;

    /// @brief maximum speed factor in measurements
    double myMaxSpeedFactorPKW;
    double myMaxSpeedFactorLKW;
    double myAvgSpeedFactorPKW;
    double myAvgSpeedFactorLKW;

};
