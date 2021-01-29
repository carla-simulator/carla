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
/// @file    RODFDetector.h
/// @author  Daniel Krajzewicz
/// @author  Eric Nicolay
/// @author  Michael Behrisch
/// @date    Thu, 16.03.2006
///
// Class representing a detector within the DFROUTER
/****************************************************************************/
#pragma once
#include <config.h>

#include <map>
#include <string>
#include <vector>
#include <set>
#include <utils/common/SUMOTime.h>
#include <utils/distribution/RandomDistributor.h>
#include <utils/common/Named.h>
#include "RODFRouteCont.h"


// ===========================================================================
// class declarations
// ===========================================================================
class RODFRouteCont;
class RODFDetectorFlows;
class ROEdge;
class RODFEdge;
class RODFDetectorCon;
class RODFNet;
struct RODFRouteDesc;
class OutputDevice;
class SUMOVTypeParameter;


// ===========================================================================
// enumerations
// ===========================================================================
/**
 * @enum RODFDetectorType
 * @brief Numerical representation of different detector types
 */
enum RODFDetectorType {
    /// A not yet defined detector
    TYPE_NOT_DEFINED = 0,

    /// A detector which had to be discarded (!!!)
    DISCARDED_DETECTOR,

    /// An in-between detector
    BETWEEN_DETECTOR,

    /// A source detector
    SOURCE_DETECTOR,
    SINK_DETECTOR
};


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RODFDetector
 * @brief Class representing a detector within the DFROUTER
 */
class RODFDetector : public Named {
public:
    /** @brief Constructor
     *
     * This constructor is used when detectors are read from a file
     *
     * @param[in] id The id of the detector
     * @param[in] laneID The id of the lane the detector is placed at
     * @param[in] pos The position of the detector at the lane
     * @param[in] type The df-router type of the detector
     * @see RODFDetectorType
     */
    RODFDetector(const std::string& id, const std::string& laneID,
                 double pos, const RODFDetectorType type);


    /** @brief Constructor
     *
     * This constructor is used when detectors shall be joined
     *
     * @param[in] id The id of the detector
     * @param[in] f A detector from which routes shall be copied
     */
    RODFDetector(const std::string& id, const RODFDetector& f);


    /// @brief Destructor
    ~RODFDetector();



    /// @name Atomar getter methods
    /// @{

    /** @brief Returns the id of the lane this detector is placed on
     * @return The id of the lane this detector is placed on
     */
    const std::string& getLaneID() const {
        return myLaneID;
    };


    /** @brief Returns the id of the edge this detector is placed on
     * @return The id of the edge this detector is placed on
     */
    std::string getEdgeID() const {
        return myLaneID.substr(0, myLaneID.rfind('_'));
    }


    /** @brief Returns the position at which the detector lies
     * @return The position of the detector at the lane
     */
    double getPos() const {
        return myPosition;
    };


    /** @brief Returns the type of the detector
     * @return This detector's type
     * @see RODFDetectorType
     */
    RODFDetectorType getType() const {
        return myType;
    };
    /// @}


    void setType(RODFDetectorType type);
    void addRoute(RODFRouteDesc& nrd);
    void addRoutes(RODFRouteCont* routes);
    bool hasRoutes() const;
    const std::vector<RODFRouteDesc>& getRouteVector() const;
    void addPriorDetector(const RODFDetector* det);
    void addFollowingDetector(const RODFDetector* det);
    const std::set<const RODFDetector*>& getPriorDetectors() const;
    const std::set<const RODFDetector*>& getFollowerDetectors() const;


    /// @name Writing methods
    /// @{

    bool writeEmitterDefinition(const std::string& file,
                                const std::map<SUMOTime, RandomDistributor<int>* >& dists,
                                const RODFDetectorFlows& flows,
                                SUMOTime startTime, SUMOTime endTime, SUMOTime stepOffset,
                                bool includeUnusedRoutes, double scale,
                                bool insertionsOnly, double defaultSpeed) const;
    bool writeRoutes(std::vector<std::string>& saved,
                     OutputDevice& out);
    void writeSingleSpeedTrigger(const std::string& file,
                                 const RODFDetectorFlows& flows,
                                 SUMOTime startTime, SUMOTime endTime, SUMOTime stepOffset,
                                 double defaultSpeed);
    void writeEndRerouterDetectors(const std::string& file);
    /// @}

    void buildDestinationDistribution(const RODFDetectorCon& detectors,
                                      SUMOTime startTime, SUMOTime endTime, SUMOTime stepOffset,
                                      const RODFNet& net,
                                      std::map<SUMOTime, RandomDistributor<int>* >& into) const;

    void computeSplitProbabilities(const RODFNet* net, const RODFDetectorCon& detectors,
                                   const RODFDetectorFlows& flows,
                                   SUMOTime startTime, SUMOTime endTime, SUMOTime stepOffset);

    const std::vector<std::map<RODFEdge*, double> >& getSplitProbabilities() const {
        return mySplitProbabilities;
    }

protected:
    int getFlowFor(const ROEdge* edge, SUMOTime time) const;
    double computeDistanceFactor(const RODFRouteDesc& rd) const;


protected:
    std::string myLaneID;
    double myPosition;
    RODFDetectorType myType;
    RODFRouteCont* myRoutes;
    std::set<const RODFDetector*> myPriorDetectors, myFollowingDetectors;
    std::vector<std::map<RODFEdge*, double> > mySplitProbabilities;
    std::map<std::string, RODFEdge*> myRoute2Edge;


private:
    /// @brief Invalidated copy constructor
    RODFDetector(const RODFDetector& src);

    /// @brief Invalidated assignment operator
    RODFDetector& operator=(const RODFDetector& src);

};


/**
 * @class RODFDetectorCon
 * @brief A container for RODFDetectors
 */
class RODFDetectorCon {
public:
    RODFDetectorCon();
    ~RODFDetectorCon();
    bool addDetector(RODFDetector* dfd);
    void removeDetector(const std::string& id);
    bool detectorsHaveCompleteTypes() const;
    bool detectorsHaveRoutes() const;
    const std::vector<RODFDetector*>& getDetectors() const;
    void save(const std::string& file) const;
    void saveAsPOIs(const std::string& file) const;
    void saveRoutes(const std::string& file) const;

    const RODFDetector& getDetector(const std::string& id) const;
    RODFDetector& getModifiableDetector(const std::string& id) const;
    const RODFDetector& getAnyDetectorForEdge(const RODFEdge* const edge) const;

    bool knows(const std::string& id) const;
    void writeEmitters(const std::string& file,
                       const RODFDetectorFlows& flows,
                       SUMOTime startTime, SUMOTime endTime, SUMOTime stepOffset,
                       const RODFNet& net,
                       bool writeCalibrators, bool includeUnusedRoutes,
                       double scale,
                       bool insertionsOnly);

    void writeEmitterPOIs(const std::string& file,
                          const RODFDetectorFlows& flows);

    void writeSpeedTrigger(const RODFNet* const net, const std::string& file,
                           const RODFDetectorFlows& flows,
                           SUMOTime startTime, SUMOTime endTime, SUMOTime stepOffset);

    void writeValidationDetectors(const std::string& file,
                                  bool includeSources, bool singleFile, bool friendly);
    void writeEndRerouterDetectors(const std::string& file);

    int getAggFlowFor(const ROEdge* edge, SUMOTime time, SUMOTime period,
                      const RODFDetectorFlows& flows) const;

    void guessEmptyFlows(RODFDetectorFlows& flows);

    void mesoJoin(const std::string& nid, const std::vector<std::string>& oldids);

    void setSpeedFactorAndDev(SUMOVTypeParameter& type, double maxFactor, double avgFactor, double dev, bool forceDev);

protected:
    /** @brief Clears the given distributions map, deleting the timed distributions
     * @param[in] dists The distribution map to clear
     */
    void clearDists(std::map<SUMOTime, RandomDistributor<int>* >& dists) const;


protected:
    std::vector<RODFDetector*> myDetectors;
    std::map<std::string, RODFDetector*> myDetectorMap;
    std::map<std::string, std::vector<RODFDetector*> > myDetectorEdgeMap;

private:
    /// @brief Invalidated copy constructor
    RODFDetectorCon(const RODFDetectorCon& src);

    /// @brief Invalidated assignment operator
    RODFDetectorCon& operator=(const RODFDetectorCon& src);

};
