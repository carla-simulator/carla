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
/// @file    MSTrafficLightLogic.h
/// @author  Daniel Krajzewicz
/// @author  Eric Nicolay
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Friedemann Wesner
/// @date    Sept 2002
///
// The parent class for traffic light logics
/****************************************************************************/
#pragma once
#include <config.h>

#include <map>
#include <string>
#include <bitset>
#include <utils/common/Command.h>
#include <utils/common/Parameterised.h>
#include <microsim/MSLogicJunction.h>
#include <microsim/MSLink.h>
#include "MSPhaseDefinition.h"


// ===========================================================================
// class declarations
// ===========================================================================
class MSNet;
class MSLink;
class MSTLLogicControl;
class NLDetectorBuilder;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSTrafficLightLogic
 * @brief The parent class for traffic light logics
 */
class MSTrafficLightLogic : public Named, public Parameterised {
public:
    /// @name Structure definitions
    /// @{

    /// @brief Definition of a list of phases, being the junction logic
    typedef std::vector<MSPhaseDefinition*> Phases;

    /// @brief Definition of the list of links that are subjected to this tls
    typedef std::vector<MSLink*> LinkVector;

    /// @brief Definition of a list that holds lists of links that do have the same attribute
    typedef std::vector<LinkVector> LinkVectorVector;

    /// @brief Definition of the list of arrival lanes subjected to this tls
    typedef std::vector<MSLane*> LaneVector;

    /// @brief Definition of a list that holds lists of lanes that do have the same attribute
    typedef std::vector<LaneVector> LaneVectorVector;

    /// @brief list of vehicles
    typedef std::vector<const SUMOVehicle*> VehicleVector;
    /// @}


public:
    /** @brief Constructor
     * @param[in] tlcontrol The tls control responsible for this tls
     * @param[in] id This tls' id
     * @param[in] programID This tls' sub-id (program id)
     * @param[in] logicType This tls' type (static, actuated etc.)
     * @param[in] delay The time to wait before the first switch
     * @param[in] parameters Additional parameters (especially for actuated logics)
     */
    MSTrafficLightLogic(MSTLLogicControl& tlcontrol,
                        const std::string& id,
                        const std::string& programID,
                        const TrafficLightType logicType,
                        const SUMOTime delay,
                        const std::map<std::string, std::string>& parameters);


    /** @brief Initialises the tls with information about incoming lanes
     * @param[in] nb The detector builder
     * @exception ProcessError If something fails on initialisation
     */
    virtual void init(NLDetectorBuilder& nb);


    /// @brief Destructor
    virtual ~MSTrafficLightLogic();



    /// @name Handling of controlled links
    /// @{

    /** @brief Adds a link on building
     * @param[in] link The controlled link
     * @param[in] lane The lane this link starts at
     * @param[in] pos The link's index (signal group) within this program
     */
    virtual void addLink(MSLink* link, MSLane* lane, int pos);

    /// @brief ignore pedestrian crossing index in mesosim
    void ignoreLinkIndex(int pos);


    /** @brief Applies information about controlled links and lanes from the given logic
     *
     * If we load a logic after the network has been loaded, we do not get the information
     *  about controlled links afterwards. We have to copy them from a previously loaded logic.
     *
     * @param[in] logic The logic to use the information about controlled links/lanes from
     */
    virtual void adaptLinkInformationFrom(const MSTrafficLightLogic& logic);


    /** @brief Returns the (uncontrolled) states of the controlled links
     * @return The controlled link's states
     */
    std::map<MSLink*, LinkState> collectLinkStates() const;


    /** @brief Resets the states of controlled links
     * @param[in] vals The state of controlled links to use
     */
    void resetLinkStates(const std::map<MSLink*, LinkState>& vals) const;
    /// @}



    /// @name Switching and setting current rows
    /// @{

    /** @brief Switches to the next phase
     * @return The time of the next switch
     */
    virtual SUMOTime trySwitch() = 0;

    /// @brief called when switching programs
    virtual void activateProgram();
    virtual void deactivateProgram();

    /** @brief Applies the current signal states to controlled links
     * @param[in] t The current time
     * @return Always true
     * @see LinkState
     * @see MSLink::setTLState
     */
    bool setTrafficLightSignals(SUMOTime t) const;
    /// @}



    /// @name Static Information Retrieval
    /// @{

    /** @brief Returns this tl-logic's id
     * @return This program's id
     */
    const std::string& getProgramID() const {
        return myProgramID;
    }


    /** @brief Returns the list of lists of all lanes controlled by this tls
     * @return All lanes controlled by this tls, sorted by the signal index
     */
    const LaneVectorVector& getLaneVectors() const {
        return myLanes;
    }


    /** @brief Returns the list of lanes that are controlled by the signals at the given position
     * @param[in] i The index of the signal
     * @return The lanes controlled by the signal at the given index
     */
    const LaneVector& getLanesAt(int i) const {
        if (i < (int)myLanes.size()) {
            return myLanes[i];
        } else {
            return myEmptyLaneVector;
        }
    }


    /** @brief Returns the list of lists of all affected links
     * @return All links controlled by this tls, sorted by the signal index
     */
    const LinkVectorVector& getLinks() const {
        return myLinks;
    }


    /** @brief Returns the list of links that are controlled by the signals at the given position
     * @param[in] i The index of the signal
     * @return The links controlled by the signal at the given index
     */
    const LinkVector& getLinksAt(int i) const {
        return myLinks[i];
    }


    /** @brief Returns the index of the given link
     * @param[in] link The link to retrieve the index for
     * @return The index of the given link (-1 if it is not controlled by this tls)
     */
    int getLinkIndex(const MSLink* const link) const;


    /** @brief Returns the number of phases
     * @return The number of this tls program's phases
     */
    virtual int getPhaseNumber() const = 0;


    /** @brief Returns the phases of this tls program
     * @return The phases of this tls program
     */
    virtual const Phases& getPhases() const = 0;


    /** @brief Returns the definition of the phase from the given position within the plan
     * @param[in] givenstep The index of the phase within the plan
     * @return The definition of the phase at the given position
     */
    virtual const MSPhaseDefinition& getPhase(int givenstep) const = 0;

    /** @brief Returns the type of the logic
     * @return The type of the logic
     */
    TrafficLightType getLogicType() const {
        return myLogicType;
    }
    /// @}



    /// @name Dynamic Information Retrieval
    /// @{

    /** @brief Returns the current index within the program
     * @return The index of the current phase within the tls
     */
    virtual int getCurrentPhaseIndex() const = 0;


    /** @brief Returns the definition of the current phase
     * @return The current phase
     */
    virtual const MSPhaseDefinition& getCurrentPhaseDef() const = 0;


    /** @brief Returns the cycle time (in ms)
     * @return The (maybe changing) cycle time of this tls
     */
    SUMOTime getDefaultCycleTime() const {
        return myDefaultCycleTime;
    }

    /// @brief return the number of controlled link indices
    int getNumLinks() {
        return myNumLinks;
    }

    /** @brief Returns the assumed next switch time
     *
     * The time may change in case of adaptive/actuated traffic lights.
     * @return The assumed next switch time (simulation time)
     */
    SUMOTime getNextSwitchTime() const;


    /** @brief Returns the duration spent in the current phase
     *
     * @return The time spent in the current phase
     */
    SUMOTime getSpentDuration(SUMOTime simStep = -1) const;
    /// @}



    /// @name Conversion between time and phase
    /// @{

    /** @brief Returns the index of the logic at the given simulation step
     * @return The (estimated) index of the tls at the given simulation time step
     */
    virtual SUMOTime getPhaseIndexAtTime(SUMOTime simStep) const = 0;


    /** @brief Returns the position (start of a phase during a cycle) from of a given step
     * @param[in] index The index of the phase to return the begin of
     * @return The begin time of the phase
     */
    virtual SUMOTime getOffsetFromIndex(int index) const = 0;


    /** @brief Returns the step (the phasenumber) of a given position of the cycle
     * @param[in] offset The offset (time) for which the according phase shall be returned
     * @return The according phase
     */
    virtual int getIndexFromOffset(SUMOTime offset) const = 0;
    /// @}



    /// @name Changing phases and phase durations
    /// @{

    /** @brief Changes the duration of the next phase
     * @param[in] duration The new duration
     */
    void addOverridingDuration(SUMOTime duration);


    /** @brief Delays current phase by the given delay
     * @param[in] delay The time by which the current phase shall be delayed
     */
    void setCurrentDurationIncrement(SUMOTime delay);


    /** @brief Changes the current phase and her duration
     * @param[in] tlcontrol The responsible traffic lights control
     * @param[in] simStep The current simulation step
     * @param[in] step Index of the phase to use
     * @param[in] stepDuration The left duration of the phase
     */
    virtual void changeStepAndDuration(MSTLLogicControl& tlcontrol,
                                       SUMOTime simStep, int step, SUMOTime stepDuration) = 0;

    /// @}

    /// @brief whether this logic is selected in the GUI
    bool isSelected() const;

    /// @brief whether this logic is the active program
    bool isActive() const {
        return myAmActive;
    }

    /// @brief return vehicles that block the intersection/rail signal for vehicles that wish to pass the given linkIndex
    virtual VehicleVector getBlockingVehicles(int linkIndex) {
        UNUSED_PARAMETER(linkIndex);
        return VehicleVector();
    }

    /// @brief return vehicles that approach the intersection/rail signal and are in conflict with vehicles that wish to pass the given linkIndex
    virtual VehicleVector getRivalVehicles(int linkIndex) {
        UNUSED_PARAMETER(linkIndex);
        return VehicleVector();
    }

    /// @brief return vehicles that approach the intersection/rail signal and have priority over vehicles that wish to pass the given linkIndex
    virtual VehicleVector getPriorityVehicles(int linkIndex) {
        UNUSED_PARAMETER(linkIndex);
        return VehicleVector();
    }

protected:
    /**
     * @class SwitchCommand
     * @brief Class realising the switch between the traffic light phases
     */
    class SwitchCommand : public Command {
    public:
        /** @brief Constructor
         * @param[in] tlcontrol The responsible traffic lights control
         * @param[in] tlLogic The controlled tls logic
         * @param[in] duration Duration till next switch
         */
        SwitchCommand(MSTLLogicControl& tlcontrol,
                      MSTrafficLightLogic* tlLogic,
                      SUMOTime nextSwitch);

        /// @brief Destructor
        ~SwitchCommand();

        /** @brief Executes the regarded junction's "trySwitch"- method
         * @param[in] currentTime The current simulation time
         * @return The time after which the command shall be executed again (the time of next switch)
         */
        SUMOTime execute(SUMOTime currentTime);


        /** @brief Marks this swicth as invalid (if the phase duration has changed, f.e.)
         * @param[in] tlLogic The controlled tls logic
         */
        void deschedule(MSTrafficLightLogic* tlLogic);


        /** @brief Returns the assumed next switch time
         * @return The assumed next switch time
         */
        SUMOTime getNextSwitchTime() const {
            return myAssumedNextSwitch;
        }

        /** @brief Reschedule or deschedule the command when quick-loading state
         *
         * The implementations should return -1 if the command shall not be re-scheduled,
         *  or a value >= 0 that describe the new time at which the command
         *  shall be executed again.
         *
         * @param[in] currentTime The current simulation time
         * @param[in] execTime The time at which the command would have been executed
         * @param[in] newTime The simulation time at which the simulation is restarted
         * @return The time at which the command shall be executed again
         */
        SUMOTime shiftTime(SUMOTime currentTime, SUMOTime execTime, SUMOTime newTime);

    private:
        /// @brief The responsible traffic lights control
        MSTLLogicControl& myTLControl;

        /// @brief The logic to be executed on a switch
        MSTrafficLightLogic* myTLLogic;

        /// @brief Assumed switch time (may change in case of adaptive traffic lights)
        SUMOTime myAssumedNextSwitch;

        /// @brief Information whether this switch command is still valid
        bool myAmValid;

    private:
        /// @brief Invalidated copy constructor.
        SwitchCommand(const SwitchCommand&);

        /// @brief Invalidated assignment operator.
        SwitchCommand& operator=(const SwitchCommand&);

    };


protected:
    /// @brief The id of the logic
    const std::string myProgramID;

    /// @brief The type of the logic
    const TrafficLightType myLogicType;

    /// @brief The list of LinkVectors; each vector contains the links that belong to the same link index
    LinkVectorVector myLinks;

    /// @brief The list of LaneVectors; each vector contains the incoming lanes that belong to the same link index
    LaneVectorVector myLanes;

    /// @brief number of controlled links
    int myNumLinks;

    /// @brief A list of duration overrides
    std::vector<SUMOTime> myOverridingTimes;

    /// @brief A value for enlarge the current duration
    SUMOTime myCurrentDurationIncrement;

    /// @brief The current switch command
    SwitchCommand* mySwitchCommand;

    /// @brief The cycle time (without changes)
    SUMOTime myDefaultCycleTime;

    /// @brief An empty lane vector
    static const LaneVector myEmptyLaneVector;

    /// @brief list of indices that are ignored in mesoscopic simulatino
    std::set<int> myIgnoredIndices;

    /// @brief whether the current program is active
    bool myAmActive;

private:
    /// @brief initialize optional meso penalties
    void initMesoTLSPenalties();


private:
    /// @brief invalidated copy constructor
    MSTrafficLightLogic(const MSTrafficLightLogic& s);

    /// @brief invalidated assignment operator
    MSTrafficLightLogic& operator=(const MSTrafficLightLogic& s);

};
