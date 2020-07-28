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
/// @file    NBLoadedTLDef.h
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @date    Fri, 29.04.2005
///
// A loaded (complete) traffic light logic
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <string>
#include <set>
#include "NBTrafficLightDefinition.h"
#include "NBNode.h"
#include <utils/common/SUMOTime.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NBLoadedTLDef
 * @brief A loaded (complete) traffic light logic
 */
class NBLoadedTLDef : public NBTrafficLightDefinition {
public:
    /** @class SignalGroup
     * @brief A single signal group, may control several connections
     */
    class SignalGroup : public Named {
    public:
        /** @brief Constructor
         * @param[in] id The id of the signal group
         */
        SignalGroup(const std::string& id);

        /// @brief Destructor
        ~SignalGroup();

        /** @brief Inserts a controlled connection
         * @param[in] c The connection to be controlled by this signal group
         */
        void addConnection(const NBConnection& c);

        /** @brief Sets the begin of a phase
         * @param[in] time The time at which the phase starts
         * @param[in] color The color of this signal starting at the given time
         */
        void addPhaseBegin(SUMOTime time, TLColor color);

        /** @brief Sets the times for redyellow and yellow
         * @param[in] tRedYellowe The duration of the redyellow phase
         * @param[in] tYellow The duration of the yellow phase
         */
        void setYellowTimes(SUMOTime tRedYellowe, SUMOTime tYellow);

        /** @brief Returns the times at which the signal switches
         * @param[in] cycleDuration The duration of the complete cycle
         * @return The switch times of this signal
         */
        std::vector<double> getTimes(SUMOTime cycleDuration) const;

        /** @brief Sorts the phases */
        void sortPhases();

        /** @brief Returns the number of links (connection) controlled by this signal
         * @return The number of links controlled by this signal
         */
        int getLinkNo() const;

        /** @brief Returns whether vehicles on controlled links may drive at the given time
         * @param[in] time The regarded time
         * @return Whether vehicles may drive at this time
         */
        bool mayDrive(SUMOTime time) const;

        /** @brief Returns whether controlled links have yellow at the given time
         * @param[in] time The regarded time
         * @return Whether controlled links are yellow at this time
         */
        bool hasYellow(SUMOTime time) const;

        /** @brief Returns whether the given connection is controlled by this signal
         * @param[in] from The connection's start edge
         * @param[in] from The connection's end edge
         * @return Whether the connection is controlled by this signal
         */
        bool containsConnection(NBEdge* from, NBEdge* to) const;

        /** @brief Returns whether this signal controls the given edge
         * @param[in] from The incoming edge
         * @return Whether this edge is controlled by this signal
         */
        bool containsIncoming(NBEdge* from) const;

        /** @brief Replaces the given incoming edge by the others given
         * @param[in] which The edge to replace
         * @param[in] by The replacements
         */
        void remapIncoming(NBEdge* which, const EdgeVector& by);

        /** @brief Returns whether this signal controls a connection where the given edge is the destination
         * @param[in] from The outgoing edge
         * @return Whether this edge's predecessing edge is controlled by this signal
         */
        bool containsOutgoing(NBEdge* to) const;

        /** @brief Replaces the given outgoing edge by the others given
         * @param[in] which The edge to replace
         * @param[in] by The replacements
         */
        void remapOutgoing(NBEdge* which, const EdgeVector& by);

        /** @brief Returns the connection at the given index
         * @param[in] pos The position within this signal
         * @return The connection at the given index
         */
        const NBConnection& getConnection(int pos) const;

        /** @brief Sets the yellow time
         * @param[in] tyellow The yellow time to set in seconds
         * @param[in] forced Whether resetting tyellow was forced by the user by setting "tls.yellow.patch-small"
         */
        void patchTYellow(int tyellow, bool forced);

        /** @brief Replaces a removed edge/lane
         * @param[in] removed The edge to replace
         * @param[in] removedLane The lane of this edge to replace
         * @param[in] by The edge to insert instead
         * @param[in] byLane This edge's lane to insert instead
         */
        void remap(NBEdge* removed, int removedLane, NBEdge* by, int byLane);

        /** @class PhaseDef
         * @brief Definition of a single, loaded phase
         */
        class PhaseDef {
        public:
            /** @brief Constructor
             * @param[in] time The begin time of this phase
             * @param[in] color A signal's color from this time
             */
            PhaseDef(SUMOTime time, TLColor color)
                : myTime(time), myColor(color) { }

            /// @brief The begin time of this phase
            SUMOTime myTime;
            /// @brief A signal's color from this time
            TLColor myColor;
        };

        /** @class phase_by_time_sorter
         * @brief Sorts phases by their begin time
         */
        class phase_by_time_sorter {
        public:
            /// @brief Constructor
            explicit phase_by_time_sorter() { }

            /** @brief Sorts phases by their begin time
             * @param[in] p1 a phase definition
             * @param[in] p2 a phase definition
             */
            int operator()(const PhaseDef& p1, const PhaseDef& p2) {
                return p1.myTime < p2.myTime;
            }
        };

    private:
        /// @brief Connections controlled by this signal
        NBConnectionVector myConnections;
        /// @brief The phases of this signal
        std::vector<PhaseDef> myPhases;
        /// @brief The times of redyellow and yellow
        SUMOTime myTRedYellow, myTYellow;
    };



    /// @brief Definition of the container for signal groups
    typedef std::map<std::string, SignalGroup*> SignalGroupCont;


    /** @brief Constructor
     * @param[in] id The id of the tls
     * @param[in] junctions Junctions controlled by this tls
     * @param[in] offset The offset of the plan
     * @param[in] type The algorithm type for the computed traffic light
     */
    NBLoadedTLDef(const NBEdgeCont& ec, const std::string& id,
                  const std::vector<NBNode*>& junctions, SUMOTime offset,
                  TrafficLightType type);


    /** @brief Constructor
     * @param[in] id The id of the tls
     * @param[in] junction The junction controlled by this tls
     * @param[in] offset The offset of the plan
     * @param[in] type The algorithm type for the computed traffic light
     */
    NBLoadedTLDef(const NBEdgeCont& ec, const std::string& id, NBNode* junction, SUMOTime offset,
                  TrafficLightType type);


    /** @brief Constructor
     * @param[in] id The id of the tls
     * @param[in] offset The offset of the plan
     * @param[in] type The algorithm type for the computed traffic light
     */
    NBLoadedTLDef(const NBEdgeCont& ec, const std::string& id, SUMOTime offset,
                  TrafficLightType type);


    /// @brief Destructor
    ~NBLoadedTLDef();


    /** @brief Returns the signal group which is responsible for the given connection
     * @param[in] from The connection's start edge
     * @param[in] to The connection's end edge
     * @return The signal group which controls the given connection
     */
    SignalGroup* findGroup(NBEdge* from, NBEdge* to) const;


    /** @brief Sets the duration of a cycle
     * @param[in] cycleDur The duration of the cycle
     */
    void setCycleDuration(int cycleDur);


    /** @brief Adds a signal group
     * @param[in] id The id of the signal group
     */
    void addSignalGroup(const std::string& id);


    /** @brief Adds a connection to a signal group
     * @param[in] groupid The id of the signal group to add the connection to
     * @param[in] connection The connection to add
     */
    bool addToSignalGroup(const std::string& groupid,
                          const NBConnection& connection);


    /** @brief Adds a list of connections to a signal group
     * @param[in] groupid The id of the signal group to add the connections to
     * @param[in] connections The connections to add
     */
    bool addToSignalGroup(const std::string& groupid,
                          const NBConnectionVector& connections);


    /** @brief Sets the information about the begin of a phase
     * @param[in] groupid The id of the signal group to add the phase to
     * @param[in] time The time the phase starts at
     * @param[in] color The color of the signal during this phase
     */
    void addSignalGroupPhaseBegin(const std::string& groupid,
                                  SUMOTime time, TLColor color);


    /** @brief Sets the times the light is yellow or red/yellow
     * @param[in] groupid The id of the signal group to add the phase to
     * @param[in] tRedYellow The duration of redyellow
     * @param[in] tYellow The duration of yellow
     */
    void setSignalYellowTimes(const std::string& groupid,
                              SUMOTime tRedYellow, SUMOTime tYellow);


    /// @name Public methods from NBTrafficLightDefinition-interface
    /// @{

    /** @brief Informs edges about being controlled by a tls
     * @param[in] ec The container of edges
     * @see NBTrafficLightDefinition::setTLControllingInformation
     */
    void setTLControllingInformation() const;


    /** @brief Replaces occurences of the removed edge in incoming/outgoing edges of all definitions
     * @param[in] removed The removed edge
     * @param[in] incoming The edges to use instead if an incoming edge was removed
     * @param[in] outgoing The edges to use instead if an outgoing edge was removed
     * @see NBTrafficLightDefinition::remapRemoved
     */
    void remapRemoved(NBEdge* removed,
                      const EdgeVector& incoming, const EdgeVector& outgoing);

    /* initialize myNeedsContRelation and set myNeedsContRelationReady to true */
    void initNeedsContRelation() const;

    ///@brief Returns the maximum index controlled by this traffic light
    int getMaxIndex();

    /// @}


protected:
    /// @name Protected methods from NBTrafficLightDefinition-interface
    /// @{

    /** @brief Computes the traffic light logic finally in dependence to the type
     * @param[in] brakingTime Duration a vehicle needs for braking in front of the tls in seconds
     * @return The computed logic
     * @see NBTrafficLightDefinition::myCompute
     */
    NBTrafficLightLogic* myCompute(int brakingTimeSeconds);

    /** @brief Builds the list of participating nodes/edges/links
     * @see NBTrafficLightDefinition::setParticipantsInformation
     */
    void setParticipantsInformation();


    /** @brief Collects the nodes participating in this traffic light
     * @see NBTrafficLightDefinition::collectNodes
     */
    void collectNodes();


    /** @brief Collects the links participating in this traffic light
     * @exception ProcessError If a link could not be found
     * @see NBTrafficLightDefinition::collectLinks
     */
    void collectLinks();


    /** @brief Returns the information whether a connection must brake, given a phase
     * @param[in] possProhibited The connection to investigate
     * @param[in] state The state
     * @param[in] strmpos The index of this connection within the masks
     * @return Whether the given connection must brake
     */
    bool mustBrake(const NBConnection& possProhibited,
                   const std::string& state,
                   int strmpos) const;


    /** @brief Replaces a removed edge/lane
     * @param[in] removed The edge to replace
     * @param[in] removedLane The lane of this edge to replace
     * @param[in] by The edge to insert instead
     * @param[in] byLane This edge's lane to insert instead
     * @see NBTrafficLightDefinition::replaceRemoved
     */
    void replaceRemoved(NBEdge* removed, int removedLane,
                        NBEdge* by, int byLane, bool incoming);
    /// @}

private:
    /** @brief Builds the phase for a given time
     * @param[in] ec The edge control to use
     * @param[in] time The time to build the phase for
     * @return The phase of this tls for the given time
     */
    std::string buildPhaseState(int time) const;

    // pointer to the NBEdgeCont for checking edges
    const NBEdgeCont* myEdgeCont;


private:
    /// @brief Controlled signal groups
    SignalGroupCont mySignalGroups;

    /// @brief The duration of a single cycle
    int myCycleDuration;


};
