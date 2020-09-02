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
/// @file    NLJunctionControlBuilder.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 9 Jul 2001
///
// Builder of microsim-junctions and tls
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <microsim/MSJunction.h>
#include <microsim/MSRightOfWayJunction.h>
#include <microsim/MSJunctionControl.h>
#include <utils/geom/Position.h>
#include <utils/geom/PositionVector.h>
#include <microsim/traffic_lights/MSSimpleTrafficLightLogic.h>
#include <microsim/traffic_lights/MSActuatedTrafficLightLogic.h>
#include <microsim/traffic_lights/MSRailSignal.h>
#include <microsim/MSBitSetLogic.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class declarations
// ===========================================================================
class OptionsCont;
class NLDetectorBuilder;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NLJunctionControlBuilder
 * @brief Builder of microsim-junctions and tls
 *
 * NLJunctionControlBuilder is a factory for MSJunction, MSJunctionLogic, WAUT,
 *  and MSTRafficLightLogic-instances.
 *
 * @todo Refactor this class - it's too large
 * @todo Resort method by one of the topics.
 */
class NLJunctionControlBuilder {
private:
    /// @brief Definition of a lane vector
    typedef std::vector<MSLane*> LaneVector;

public:
    /** @brief Constructor
     *
     * Stores default values for extended tls reading them from the given
     *  options. Builds a MSTLLogicControl instance for myLogicControl.
     * @param[in] net The network to fill
     * @param[in] db The detector builder to use
     */
    NLJunctionControlBuilder(MSNet& net, NLDetectorBuilder& db);


    /** @brief Destructor
     *
     * Deletes previously allocated "myLogicControl" and "myJunctions" if
     *  they were not previously returned (this may happen if an error occurred).
     */
    virtual ~NLJunctionControlBuilder();


    /** @brief Begins the processing of the named junction
     *
     * @param[in] id The ID of the junction
     * @param[in] key unused?!
     * @param[in] type The type of the junction
     * @param[in] x x-position of the junction
     * @param[in] y y-position of the junction
     * @param[in] shape The shape of the junction
     * @param[in] incomingLanes List of lanes which end at this intersection
     * @param[in] internalLanes List of internal lanes across this intersection
     * @exception InvalidArgument If the junction type is not known
     * @todo Check why "key" is given
     */
    void openJunction(const std::string& id, const std::string& key,
                      const SumoXMLNodeType type,
                      const Position pos,
                      const PositionVector& shape,
                      const std::vector<MSLane*>& incomingLanes,
                      const std::vector<MSLane*>& internalLanes,
                      const std::string& name);


    /** @brief Closes (ends) the processing of the current junction
     *
     * This method throws an InvalidArgument when a junction with the same id
     *  as the current was already added or if the junction type stored in "myType"
     *  is invalid. It throws a ProcessError if the container to store the
     *  junction in was not built before.
     *
     * @exception InvalidArgument If the current id is already used or the junction type was invalid
     * @exception ProcessError If the container to store the junction in was not built before
     * @todo Throwing ProcessError would get unneeded if the container would be built by default (see prepare)
     * @todo The type of the junctions shoould probably be checked when supprted (in openJunction)
     */
    void closeJunction(const std::string& basePath);


    /** @brief Builds the MSJunctionControl which holds all of the simulations junctions
     *
     * Returns the previously built junction control ("myJunctions"). "myJunctions" is
     *  set to 0, so that it will not be destroyed by the destructor.
     *
     * @return The built junction control
     */
    MSJunctionControl* build() const;


    /** @brief Initialises a junction logic
     * @param[in] id The id of the row-logic
     */
    void initJunctionLogic(const std::string& id);


    /** @brief Adds a logic item
     *
     * Adds a logic item to the current processed logic. Throws an InvalidArgument
     *  if the current request size (myRequestSize) is 0 or lower.
     *
     * @param[in] request The request (~link) index
     * @param[in] response The response (list of higher priorised links)
     * @param[in] foes List of foes to the request
     * @param[in] cont Whether the request is followed by an internal end
     * @todo Recheck "cont"; is the description correct?
     */
    void addLogicItem(int request, const std::string& response,
                      const std::string& foes, bool cont);


    /** @brief Begins the reading of a traffic lights logic
     *
     * @param[in] id The id of the tls
     * @param[in] programID The id of the currently loaded program
     * @param[in] type The type of the tls
     * @param[in] offset The offset to start with
     * @todo Why is the type not verified?
     * @todo Recheck, describe usage of detectorOffset (where does the information come from?)
     */
    void initTrafficLightLogic(const std::string& id, const std::string& programID,
                               TrafficLightType type, SUMOTime offset);


    /** @brief Adds a phase to the currently built traffic lights logic
     *
     * @param[in] duration The duration of the phase
     * @param[in] state The state of the tls
     * @param[in] min The minimum duration of the phase
     * @param[in] max The maximum duration of the phase
     * @todo min/max is used only by one junction type. Recheck
     * @todo min/max: maybe only one type of a phase definition should be built
     */
    void addPhase(SUMOTime duration, const std::string& state, const std::vector<int>& nextPhases,
                  SUMOTime min, SUMOTime max, const std::string& name);

    /** @brief Adds a phase to the currently built traffic lights logic
     *
     * @param[in] duration The duration of the phase
     * @param[in] state The state of the tls
     * @param[in] minDuration The minimum duration of the phase
     * @param[in] maxDuration The maximum duration of the phase
     * @param[in] transient_notdecisional Specifies if this is a transient phase (true) or a decisional one (false)
     * @param[in] commit Specifies if this is a commit phase
     * @param[in] targetLanes A reference to the vector containing targeted sensor lanes for this phase, given by lane id
     */
    void addPhase(SUMOTime duration, const std::string& state, const std::vector<int>& nextPhases, SUMOTime minDuration, SUMOTime maxDuration, const std::string& name, bool transient_notdecisional, bool commit, MSPhaseDefinition::LaneIdVector* targetLanes = nullptr);


    /** @brief Returns a previously build tls logic
     *
     * @param[in] id The ID of the tls logic to return
     * @return The named logic
     * @exception InvalidArgument If the named tls logic was not built before
     */
    MSTLLogicControl::TLSLogicVariants& getTLLogic(const std::string& id)
    const;


    /** @brief Returns the built tls-logic control
     *
     * Returns the junction control ("myLogicControl") built using MSTLLogicControl::closeNetworkReading.
     * "myLogicControl" is set to 0, so that it will not be destroyed by the destructor.
     *
     * @return The built tls-logic control
     * @exception ProcessError If the traffic lights were not correct
     * @see MSTLLogicControl::closeNetworkReading
     */
    MSTLLogicControl* buildTLLogics();


    /** @brief Ends the building of a traffic lights logic
     *
     * Builds the correct type of a MSTrafficLightLogic using the stored information.
     *  Tries to add it to the used tls control. Throws an InvalidArgument if
     *  this is not possible (another tls logic with the same name exists).
     *
     * @exception InvalidArgument If another tls logic with the same name as the currently built was loaded before
     */
    virtual void closeTrafficLightLogic(const std::string& basePath);


    /** @brief Ends the building of a junction logic (row-logic)
     *
     * Rechecks values for the request and builds a MSJunctionLogic using these values.
     *  Throws and InvalidArgument if the values are invalid (error message is
     *  included).
     * Tries to add the built logic to the internal container "myLogics". If another
     *  logic with the same id exists, an InvalidArgument is thrown.
     *
     * @exception InvalidArgument If the logic's values are false or another logic with the same id was built before
     */
    void closeJunctionLogic();


    /** @brief Adds a parameter
     *
     * @param[in] key The key of the parameter
     * @param[in] value The value of the parameter
     * @todo Where are these parameter used? Describe!
     * @todo Can a parameter be overwritten?
     */
    void addParam(const std::string& key, const std::string& value);


    /** @brief Returns the active key
     * @return The active key
     */
    const std::string& getActiveKey() const;


    /** @brief Returns the active sub key
     * @return The active sub key
     */
    const std::string& getActiveSubKey() const;


    /** @brief Returns the used tls control
     *
     * This may be either the internal one ("myLogicControl"), or, if the network has
     *  been already loaded and we are loading additional tls programs, the net's logic control.
     *
     * @return The tls control to use
     */
    MSTLLogicControl& getTLLogicControlToUse() const;

    /// @brief initialize junctions after all connections have been loaded
    void postLoadInitialization();

    /// @brief try to retrieve junction by id
    MSJunction* retrieve(const std::string id);

    /// @brief return the number of phases loaded so far (for error reporting)
    int getNumberOfLoadedPhases() const {
        return (int)myActivePhases.size();
    }


protected:
    /** @brief Returns the current junction logic
     *
     * "Current" means the one with "myActiveID". If it is not built yet
     *  (not within "myLogics") an InvalidArgument is thrown.
     *
     * @return The current tls logic
     * @exception InvalidArgument If the logic was not built before
     * @todo Where is this used?
     */
    MSJunctionLogic* getJunctionLogicSecure();


protected:
    /// @name Factory methods, virtual so that other versions of the structures can be built
    /// @{

    /** @brief Builds a junction that does not use a logic
     *
     * Builds a MSNoLogicJunction
     *
     * @return The built junction
     */
    virtual MSJunction* buildNoLogicJunction();


    /** @brief Builds a junction with a logic
     *
     * Builds a MSRightOfWayJunction. Throws an exception if the logic was not built
     *  (see getJunctionLogicSecure).
     *
     * @return The built junction
     * @exception InvalidArgument If the logic of the junction was not built before
     */
    virtual MSJunction* buildLogicJunction();


    /** @brief Builds an internal junction
     *
     * Builds a MSInternalJunction
     *
     * @return The built junction
     */
    virtual MSJunction* buildInternalJunction();
    /// @}


protected:
    /// @brief The net to use
    MSNet& myNet;

    /// @brief The detector builder to use
    NLDetectorBuilder& myDetectorBuilder;

    /// @brief The switch offset within the tls
    SUMOTime myOffset;

    /// @brief The current logic type
    TrafficLightType myLogicType;

    /// @brief The right-of-way-logic of the currently chosen bitset-logic
    MSBitsetLogic::Logic myActiveLogic;

    /// @brief The description about which lanes disallow other passing the junction simultaneously
    MSBitsetLogic::Foes myActiveFoes;

    /// @brief The description about which lanes have an internal follower
    std::bitset<SUMO_MAX_CONNECTIONS> myActiveConts;

    /// @brief The current phase definitions for a simple traffic light
    MSSimpleTrafficLightLogic::Phases myActivePhases;

    /// @brief The size of the request
    int myRequestSize;

    /// @brief Counter for the inserted items
    int myRequestItemNumber;

    /// @brief The junctions controls
    mutable MSJunctionControl* myJunctions;

    /// @brief The list of the incoming lanes of the currently chosen junction
    LaneVector myActiveIncomingLanes;

    /// @brief The list of the internal lanes of the currently chosen junction
    LaneVector myActiveInternalLanes;

    /// @brief The id of the currently chosen junction
    std::string myActiveID;

    /// @brief The key of the currently chosen junction
    std::string myActiveKey, myActiveProgram;

    /// @brief The type of the currently chosen junction
    SumoXMLNodeType myType;

    /// @brief The position of the junction
    Position myPosition;

    /// @brief The absolute duration of a tls-control loop
    SUMOTime myAbsDuration;

    /// @brief The shape of the current junction
    PositionVector myShape;

    /// @brief the name of the current junction
    std::string myActiveName;

    /// @brief The container for information which junctions shall be initialised using which values
    std::vector<MSTrafficLightLogic*> myLogics2PostLoadInit;


    /// @brief The tls control to use (0 if net's tls control shall be used)
    mutable MSTLLogicControl* myLogicControl;


    /// @brief Definition of a parameter map (key->value)
    typedef std::map<std::string, std::string> StringParameterMap;

    /// @brief Parameter map (key->value)
    StringParameterMap myAdditionalParameter;


    /// @brief Map of loaded junction logics
    std::map<std::string, MSJunctionLogic*> myLogics;

    /// @brief Information whether the current logic had an error
    bool myCurrentHasError;


private:
    /** @brief invalidated copy operator */
    NLJunctionControlBuilder(const NLJunctionControlBuilder& s);

    /** @brief invalidated assignment operator */
    NLJunctionControlBuilder& operator=(const NLJunctionControlBuilder& s);

    static const int NO_REQUEST_SIZE;

    /// @brief whether the network has been loaded
    bool myNetIsLoaded;

};
