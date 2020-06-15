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
/// @file    NBTrafficLightLogicCont.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A container for traffic light definitions and built programs
/****************************************************************************/
#pragma once
#include <config.h>

#include <map>
#include <string>
#include "NBTrafficLightDefinition.h"


// ===========================================================================
// class declarations
// ===========================================================================
class OptionsCont;
class NBEdgeCont;
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NBTrafficLightLogicCont
 * @brief A container for traffic light definitions and built programs
 *
 * This container class holds definitions of traffic light logics during
 *  the loading of the network. After all information has been loaded, these
 *  definitions are used to build the traffic light logics.
 *
 * The built traffic light logics are kept stored within this container during
 *  their building and written to the network file at the end.
 *
 * @see NBTrafficLightDefinition
 * @see NBTrafficLightLogic
 */
class NBTrafficLightLogicCont {
public:
    /// @brief Constructor
    NBTrafficLightLogicCont();

    /// @brief Destructor
    ~NBTrafficLightLogicCont();

    /** @brief Initialises the storage by applying given options
     *
     * Options, mainly setting offsets, are parsed
     *  and the according internal variables are set.
     *
     * @param[in] oc The options container to read options from
     * @todo Recheck exceptions
     */
    void applyOptions(OptionsCont& oc);


    /** @brief Adds a logic definition to the dictionary
     *
     * "true" is returned if the logic is accepted - no logic with the same
     *  name and programID exists within this container.
     *
     * @param[in] logic The logic to add
     * @param[in] forceInsert If true, rename the program to make insertion succeed
     * @return Whether the logic was valid (no logic with the same id and programID is already known)
     */
    bool insert(NBTrafficLightDefinition* logic, bool forceInsert = false);


    /** @brief Removes a logic definition (and all programs) from the dictionary
     *
     * "true" is returned if the logic existed in the dictionary,
     *  otherwise "false".
     *
     * @param[in] id The id of the logic to remove
     * @return Whether the named logic was within the dictionary
     */
    bool removeFully(const std::string id);


    /** @brief Removes a program of a logic definition from the dictionary
     *
     * "true" is returned if the program existed in the dictionary,
     *  otherwise "false".
     *
     * @param[in] id The id of the logic
     * @param[in] programID The id of the program to remove
     * @param[in] del Whether the definition shall be deleted
     * @return Whether the program was within the dictionary
     */
    bool removeProgram(const std::string id, const std::string programID, bool del = true);


    /** @brief Extracts a traffic light definition from myDefinitions
     * but keeps it in myExtracted for eventual * deletion (used by NETEDIT)
     */
    void extract(NBTrafficLightDefinition* definition);


    /// @brief return the number of extracted traffic light definitions
    int getNumExtracted() const {
        return (int)myExtracted.size();
    }


    /// @brief Returns a list of all computed logics
    std::vector<NBTrafficLightLogic*> getComputed() const;


    /** @brief Computes the traffic light logics using the stored definitions and stores the results
     *
     * Goes through all stored definitions and calls "NBTrafficLightDefinition::compute"
     *  for each. Stores the result using "insert".
     *
     * @param[in] oc Options used during the computation
     * @return The number of computed tls and programs
     * @see NBTrafficLightDefinition::compute
     */
    std::pair<int, int> computeLogics(OptionsCont& oc);


    /** @brief Computes a specific traffic light logic (using by NETEDIT)
     *
     * @param[in] oc Options used during the computation
     * @return whether the logic was computed successfully
     * @see NBTrafficLightDefinition::compute
     */
    bool computeSingleLogic(OptionsCont& oc, NBTrafficLightDefinition* def);


    /** @brief Replaces occurences of the removed edge in incoming/outgoing edges of all definitions
     *
     * @param[in] removed The removed edge
     * @param[in] incoming The edges to use instead if an incoming edge was removed
     * @param[in] outgoing The edges to use instead if an outgoing edge was removed
     * @todo Recheck usage
     */
    void remapRemoved(NBEdge* removed,
                      const EdgeVector& incoming, const EdgeVector& outgoing);


    /** @brief Replaces occurences of the removed edge/lane in all definitions by the given edge
     *
     * @param[in] removed The removed edge
     * @param[in] removed The removed lane
     * @param[in] by The edge to use instead
     * @param[in] byLane The lane to use instead
     * @todo Recheck usage
     */
    void replaceRemoved(NBEdge* removed, int removedLane,
                        NBEdge* by, int byLane, bool incoming);


    /** @brief Returns the named definition
     *
     * @param[in] id The id of the definition to return
     * @param[in] programID The id of the program to return
     * @return The named definition, 0 if it is not known
     */
    NBTrafficLightDefinition* getDefinition(const std::string& id, const std::string& programID) const;


    /** @brief Returns all programs for the given tl-id
     *
     * @param[in] id The tl-id for which to return all programs
     * @return The map of programIDs to definitions
     */
    const std::map<std::string, NBTrafficLightDefinition*>& getPrograms(const std::string& id) const;


    /** @brief Returns the computed logic for the given name
     *
     * @param[in] id The id of the logic to return
     * @param[in] programID The id of the program to return
     * @return The named definition, 0 if it is not known
     */
    NBTrafficLightLogic* getLogic(const std::string& id, const std::string& programID) const;


    /** @brief Informs the edges about being controlled by a tls
     *
     * Goes through all definition, calling eachs "setParticipantsInformation" method.
     * Goes through all definition, calling eachs "setTLControllingInformation" method.
     *
     * @param[in] ec The ede control to set information into
     * @see NBTrafficLightDefinition::setParticipantsInformation
     * @see NBTrafficLightDefinition::setTLControllingInformation
     */
    void setTLControllingInformation(const NBEdgeCont& ec, const NBNodeCont& nc);

    /// @brief set OpenDRIVE signal reference parameters after all link indices are known
    void setOpenDriveSignalParameters();

    /// @brief Returns a list of all definitions (convenience for easier iteration)
    typedef std::vector<NBTrafficLightDefinition*> Definitions;
    Definitions getDefinitions() const;

private:
    /// @brief Definition of internal the container types
    typedef std::map<std::string, NBTrafficLightLogic*> Program2Logic;
    typedef std::map<std::string, Program2Logic> Id2Logics;
    typedef std::map<std::string, NBTrafficLightDefinition*> Program2Def;
    typedef std::map<std::string, Program2Def> Id2Defs;
    typedef std::vector<NBTrafficLightLogic*> Logics;

    /// @brief The container for previously computed tl-logics
    Id2Logics myComputed;

    /// @brief The container for tl-ids to their definitions
    Id2Defs myDefinitions;

    /// @brief The container for extracted definitions
    std::set<NBTrafficLightDefinition*> myExtracted;

    /// @brief List of tls which shall have an offset of T/2
    std::set<std::string> myHalfOffsetTLS;

    /// @brief List of tls which shall have an offset of T/2
    std::set<std::string> myQuarterOffsetTLS;

    static const Program2Def EmptyDefinitions;

private:

    /** @brief Destroys all stored definitions and logics
     */
    void clear();


};
