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
/// @file    NBRequest.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Tue, 20 Nov 2001
///
// This class computes the logic of a junction
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <map>
#include <bitset>
#include "NBConnectionDefs.h"
#include "NBContHelper.h"
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class declarations
// ===========================================================================
class NBEdge;
class NBJunctionTypeIO;
class NBTrafficLightLogic;
class OptionsCont;
class NBTrafficLightDefinition;
class NBEdgeCont;
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NBRequest
 * Given a special node, this class builds the logic of this (junction)
 * regarding the relationships between the incoming and outgoing edges and
 * their priorities. The junction's logic is saved when it does not yet exist.
 */
class NBRequest {
public:
    /** constructor
        The parameter are the logic's lists of edges (all, incoming only and
        outgoing only edges). By now no further informations are needed to
        describe the junctions. These parameter must not be changed during the
        logic's building */
    NBRequest(const NBEdgeCont& ec,
              NBNode* junction,
              const EdgeVector& all,
              const EdgeVector& incoming,
              const EdgeVector& outgoing,
              const NBConnectionProhibits& loadedProhibits);

    /// @brief destructor
    ~NBRequest();

    /// @brief builds the bitset-representation of the logic
    void buildBitfieldLogic();

    /// @brief @brief returns the number of the junction's lanes and the number of the junction's links in respect. @note: only connected lanes are counted
    std::pair<int, int> getSizes() const;

    /** @brief Returns the information whether "prohibited" flow must let "prohibitor" flow pass
     * @param[in] possProhibitorFrom The maybe prohibiting connection's begin
     * @param[in] possProhibitorTo The maybe prohibiting connection's end
     * @param[in] possProhibitedFrom The maybe prohibited connection's begin
     * @param[in] possProhibitedTo The maybe prohibited connection's end
     * @return Whether the first flow prohibits the second one
     */
    bool mustBrake(const NBEdge* const possProhibitorFrom, const NBEdge* const possProhibitorTo,
                   const NBEdge* const possProhibitedFrom, const NBEdge* const possProhibitedTo) const;


    /** @brief Returns the information whether the described flow must let any other flow pass
     * @param[in] from The connection's start edge
     * @param[in] to The connection's end edge
     * @param[in] fromLane The connection starting lane
     * @param[in] includePedCrossings Whether braking due to a pedestrian crossing counts
     * @return Whether the described connection must brake (has higher priorised foes)
     */
    bool mustBrake(const NBEdge* const from, const NBEdge* const to, int fromLane, int toLane, bool includePedCrossings) const;

    /** @brief Returns the information whether the described flow must brake for the given crossing
     * @param[in] node The parent node of this request
     * @param[in] from The connection's start edge
     * @param[in] to The connection's end edge
     * @param[in] crossing The pedestrian crossing to check
     * @return Whether the described connection must brake (has higher priorised foes)
     */
    static bool mustBrakeForCrossing(const NBNode* node, const NBEdge* const from, const NBEdge* const to, const NBNode::Crossing& crossing);

    /** @brief Returns the information whether the given flows cross
     * @param[in] from1 The starting edge of the first stream
     * @param[in] to1 The ending edge of the first stream
     * @param[in] from2 The starting edge of the second stream
     * @param[in] to2 The ending edge of the second stream
     * @return Whether both stream are foes (cross)
     */
    bool foes(const NBEdge* const from1, const NBEdge* const to1,
              const NBEdge* const from2, const NBEdge* const to2) const;


    /** @brief Returns the information whether "prohibited" flow must let "prohibitor" flow pass
     * @param[in] possProhibitorFrom The maybe prohibiting connection's begin
     * @param[in] possProhibitorTo The maybe prohibiting connection's end
     * @param[in] possProhibitedFrom The maybe prohibited connection's begin
     * @param[in] possProhibitedTo The maybe prohibited connection's end
     * @param[in] regardNonSignalisedLowerPriority Whether the right of way rules without traffic lights shall be regarded
     * @return Whether the first flow prohibits the second one
     */
    bool forbids(const NBEdge* const possProhibitorFrom, const NBEdge* const possProhibitorTo,
                 const NBEdge* const possProhibitedFrom, const NBEdge* const possProhibitedTo,
                 bool regardNonSignalisedLowerPriority) const;

    /// @brief writes the XML-representation of the logic as a bitset-logic XML representation
    void computeLogic(const bool checkLaneFoes);

    void writeLogic(OutputDevice& into) const;

    const std::string& getFoes(int linkIndex) const;
    const std::string& getResponse(int linkIndex) const;

    /// @brief prints the request
    friend std::ostream& operator<<(std::ostream& os, const NBRequest& r);

    /// @brief reports warnings if any occurred
    static void reportWarnings();

    /// @brief whether the given connections crosses the foe connection from the same lane and must yield
    bool rightTurnConflict(const NBEdge* from, const NBEdge::Connection& con,
                           const NBEdge* prohibitorFrom, const NBEdge::Connection& prohibitorCon) const;

    /// @brief whether multple connections from the same edge target the same lane
    bool mergeConflict(const NBEdge* from, const NBEdge::Connection& con,
                       const NBEdge* prohibitorFrom, const NBEdge::Connection& prohibitorCon, bool foes) const;

    /// @brief whether opposite left turns intersect
    bool oppositeLeftTurnConflict(const NBEdge* from, const NBEdge::Connection& con,
                                  const NBEdge* prohibitorFrom,  const NBEdge::Connection& prohibitorCon, bool foes) const;


    /// @brief whether there are conflicting streams of traffic at this node
    bool hasConflict() const;

private:
    /** sets the information that the edge from1->to1 blocks the edge
        from2->to2 (is higher priorised than this) */
    void setBlocking(NBEdge* from1, NBEdge* to1, NBEdge* from2, NBEdge* to2);

    /** @brief computes the response of a certain lane
        Returns the next link index within the junction */
    int computeLaneResponse(NBEdge* from, int lane, int pos, const bool checkLaneFoes);

    /** @brief computes the response of a certain crossing
        Returns the next link index within the junction */
    int computeCrossingResponse(const NBNode::Crossing& crossing, int pos);

    /** @brief Writes the response of a certain link
     *
     * For the link (described by the connected edges and lanes), the response in dependence
     *  to all other links of this junction is computed. Herefor, the method
     *  goes through all links of this junction and writes a '0' if the link
     *  is not blocked by the currently investigated one, or '1' if it is.
     *
     * In the case "mayDefinitelyPass" is true, the link will not be disturbed by
     *  any other (special case for on-ramps).
     *
     * @param[in] tlIndex The tl index of the connection for which the responses shall be written
     * @param[in] from The link's starting edge
     * @param[in] to The link's destination edge
     * @param[in] fromLane The link's starting lane
     * @param[in] toLane The link's destination lane
     * @param[in] mayDefinitelyPass Whether this link is definitely not disturbed
     * @return the response string
     * @exception IOError not yet implemented
     */
    std::string getResponseString(const NBEdge* const from, const NBEdge::Connection& c, const bool checkLaneFoes) const;


    /** writes which participating links are foes to the given */
    std::string getFoesString(NBEdge* from, NBEdge* to,
                              int fromLane, int toLane, const bool checkLaneFoes) const;


    /** @brief Returns the index to the internal combination container for the given edge combination
     *
     * If one of the edges is not known, -1 is returned.
     *
     * @param[in] from The starting edge (incoming to this logic)
     * @param[in] to The destination edge (outgoing from this logic)
     * @return The index within the internal container
     */
    int getIndex(const NBEdge* const from, const NBEdge* const to) const;


    /// @brief returns the distance between the incoming (from) and the outgoing (to) edge clockwise in edges
    int distanceCounterClockwise(NBEdge* from, NBEdge* to);

    /// @brief computes the relationships between links outgoing right of the given link */
    void computeRightOutgoingLinkCrossings(NBEdge* from, NBEdge* to);

    /// @brief computes the relationships between links outgoing left of the given link
    void computeLeftOutgoingLinkCrossings(NBEdge* from, NBEdge* to);


    void resetSignalised();

    /// @brief reset foes it the number of lanes matches (or exceeds) the number of incoming connections for an edge
    void resetCooperating();

    /// @brief whether the given connections must be checked for lane conflicts due to the vClasses involved
    bool checkLaneFoesByClass(const NBEdge::Connection& con,
                              const NBEdge* prohibitorFrom,  const NBEdge::Connection& prohibitorCon) const;

    /// @brief whether the given connections must be checked for lane conflicts due to disjunct target lanes
    bool checkLaneFoesByCooperation(const NBEdge* from, const NBEdge::Connection& con,
                                    const NBEdge* prohibitorFrom,  const NBEdge::Connection& prohibitorCon) const;

    /** @brief return whether the given laneToLane connections prohibit each other
     * under the assumption that the edge2edge connections are in conflict
     */
    bool laneConflict(const NBEdge* from, const NBEdge* to, int toLane, const NBEdge* prohibitorFrom, const NBEdge* prohibitorTo, int prohibitorToLane) const;


    /// @brief return to total number of edge-to-edge connections of this request-logic
    inline int numLinks() const;

private:
    /// @brief the node the request is assigned to
    NBNode* myJunction;

    /// @brief all (icoming and outgoing) of the junctions edges
    const EdgeVector& myAll;

    /// @brief edges incoming to the junction
    const EdgeVector& myIncoming;

    /// @brief edges outgoing from the junction
    const EdgeVector& myOutgoing;

    /// @brief definition of a container to store boolean informations about a link into
    typedef std::vector<bool> LinkInfoCont;

    /// @brief definition of a container for link(edge->edge) X link(edge->edge) combinations (size = |myIncoming|*|myOutgoing|)
    typedef std::vector<LinkInfoCont> CombinationsCont;

    /// @brief the link X link blockings
    CombinationsCont  myForbids;

    /// @brief the link X link is done-checks
    CombinationsCont  myDone;

    /// @brief precomputed right-of-way matrices for each lane-to-lane link
    std::vector<std::string> myFoes;
    std::vector<std::string> myResponse;
    std::vector<bool> myHaveVia;

private:
    static int myGoodBuilds, myNotBuild;

    /// @brief Invalidated assignment operator
    NBRequest& operator=(const NBRequest& s) = delete;
};
