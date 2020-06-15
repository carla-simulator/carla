/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    NBTrafficLightDefinition.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// The base class for traffic light logic definitions
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <string>
#include <bitset>
#include <utility>
#include <set>
#include <utils/common/StdDefs.h>
#include <utils/common/Named.h>
#include <utils/common/VectorHelper.h>
#include <utils/common/SUMOTime.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/Parameterised.h>
#include "NBCont.h"
#include "NBConnection.h"
#include "NBConnectionDefs.h"
#include "NBLinkPossibilityMatrix.h"


// ===========================================================================
// class declarations
// ===========================================================================
class NBNode;
class OptionsCont;
class NBTrafficLightLogic;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NBTrafficLightDefinition
 * @brief The base class for traffic light logic definitions
 *
 * A base class is necessary, as we have two cases: a) the logic is given by
 *  the imported network, or b) the logic is not given and we have to compute
 *  it by ourselves. In the first case, NBLoadedTLDef should be used, in the
 *  second NBOwnTLDef.
 *
 * @see NBLoadedTLDef
 * @see NBOwnTLDef
 */
class NBTrafficLightDefinition : public Named, public Parameterised {
public:

    static const SUMOTime UNSPECIFIED_DURATION;
    static const std::string DefaultProgramID;

    /**
     * @enum TLColor
     * @brief An enumeration of possible tl-signal states
     */
    enum TLColor {
        /// @brief Signal shows red
        TLCOLOR_RED,
        /// @brief Signal shows yellow
        TLCOLOR_YELLOW,
        /// @brief Signal shows red/yellow (unused)
        TLCOLOR_REDYELLOW,
        /// @brief Signal shows green
        TLCOLOR_GREEN,
        /// @brief Signal is blinking yellow
        TLCOLOR_BLINK
    };


    /** @brief Constructor
     * @param[in] id The id of the tls
     * @param[in] junctions List of junctions controlled by this tls
     * @param[in] programID The id of the added program ("subID")
     * @param[in] offset The offset of the plan
     * @param[in] type The algorithm type for the computed traffic light
     */
    NBTrafficLightDefinition(const std::string& id,
                             const std::vector<NBNode*>& junctions,
                             const std::string& programID,
                             SUMOTime offset,
                             TrafficLightType type);


    /** @brief Constructor
     * @param[in] id The id of the tls
     * @param[in] junction The (single) junction controlled by this tls
     * @param[in] programID The id of the added program ("subID")
     * @param[in] offset The offset of the plan
     * @param[in] type The algorithm type for the computed traffic light
     */
    NBTrafficLightDefinition(const std::string& id,
                             NBNode* junction,
                             const std::string& programID,
                             SUMOTime offset,
                             TrafficLightType type);


    /** @brief Constructor
     * @param[in] id The id of the tls
     * @param[in] programID The id of the added program ("subID")
     * @param[in] offset The offset of the plan
     * @param[in] type The algorithm type for the computed traffic light
     */
    NBTrafficLightDefinition(const std::string& id, const std::string& programID,
                             SUMOTime offset,
                             TrafficLightType type);


    /// @brief Destructor
    virtual ~NBTrafficLightDefinition();


    /** @brief Computes the traffic light logic
     *
     * Does some initialisation at first, then calls myCompute to finally
     *  build the tl-logic
     *
     * @param[in] oc The options container holding options needed during the building
     * @return The built logic (may be 0)
     */
    NBTrafficLightLogic* compute(OptionsCont& oc);



    /// @name Access to controlled nodes
    /// @{

    /** @brief Adds a node to the traffic light logic
     * @param[in] node A further node that shall be controlled by the tls
     */
    virtual void addNode(NBNode* node);


    /** @brief Removes the given node from the list of controlled nodes
     * @param[in] node The node that shall not be controlled by the tls any more
     */
    virtual void removeNode(NBNode* node);

    /** @brief removes the given connection from the traffic light
     * if recontruct=true, reconstructs the logic and informs the edges for immediate use in NETEDIT
     * @note: tlIndex is not necessarily unique. we need the whole connection data here
     */
    virtual void removeConnection(const NBConnection& conn, bool reconstruct = true) {
        UNUSED_PARAMETER(conn);
        UNUSED_PARAMETER(reconstruct);
    }

    /** @brief Returns the list of controlled nodes
     * @return Controlled nodes
     */
    const std::vector<NBNode*>& getNodes() const {
        return myControlledNodes;
    }
    /// @}


    /** @brief Returns the information whether the described flow must let any other flow pass
     *
     * If the from/to connection passes only one junction (from is incoming into
     *  same node as to outgoes from) the node is asked whether the flow must brake-
     * Otherwise true is returned (recheck!)
     * "from" must be an incoming edge into one of the participating nodes!
     * @param[in] from The connection's start edge
     * @param[in] to The connection's end edge
     * @return Whether the described connection must brake (has higher priorised foes)
     */
    bool mustBrake(const NBEdge* const from, const NBEdge* const to) const;


    /** @brief Returns the information whether the described flow must let the other flow pass
     * @param[in] possProhibited The maybe prohibited connection
     * @param[in] possProhibitor The maybe prohibiting connection
     * @param[in] regardNonSignalisedLowerPriority Whether the right of way rules without traffic lights shall be regarded
     * @return Whether the second flow prohibits the first one
     * @see forbids
     */
    bool mustBrake(const NBConnection& possProhibited,
                   const NBConnection& possProhibitor,
                   bool regardNonSignalisedLowerPriority) const;

    /** @brief Returns the information whether the described flow must let any other flow pass
     * @param[in] possProhibitedFrom The maybe prohibited connection's begin
     * @param[in] possProhibitedTo The maybe prohibited connection's end
     * @param[in] possProhibitorFrom The maybe prohibiting connection's begin
     * @param[in] possProhibitorTo The maybe prohibiting connection's end
     * @param[in] regardNonSignalisedLowerPriority Whether the right of way rules without traffic lights shall be regarded
     * @return Whether the second flow prohibits the first one
     * @see forbids
     */
    bool mustBrake(const NBEdge* const possProhibitedFrom, const NBEdge* const possProhibitedTo,
                   const NBEdge* const possProhibitorFrom, const NBEdge* const possProhibitorTo,
                   bool regardNonSignalisedLowerPriority) const;


    /** @brief Returns the information whether "prohibited" flow must let "prohibitor" flow pass
     * @param[in] possProhibitedFrom The maybe prohibited connection's begin
     * @param[in] possProhibitedTo The maybe prohibited connection's end
     * @param[in] possProhibitorFrom The maybe prohibiting connection's begin
     * @param[in] possProhibitorTo The maybe prohibiting connection's end
     * @param[in] regardNonSignalisedLowerPriority Whether the right of way rules without traffic lights shall be regarded
     * @param[in] sameNodeOnly Whether the check shall only be performed if both edges are incoming to the same node
     * @return Whether the second flow prohibits the first one
     * @see forbids
     */
    bool forbids(const NBEdge* const possProhibitorFrom, const NBEdge* const possProhibitorTo,
                 const NBEdge* const possProhibitedFrom, const NBEdge* const possProhibitedTo,
                 bool regardNonSignalisedLowerPriority,
                 bool sameNodeOnly = false) const;


    /** @brief Returns the information whether the given flows cross
     * @param[in] from1 The starting edge of the first stream
     * @param[in] to1 The ending edge of the first stream
     * @param[in] from2 The starting edge of the second stream
     * @param[in] to2 The ending edge of the second stream
     * @return Whether both stream are foes (cross)
     */
    bool foes(const NBEdge* const from1, const NBEdge* const to1,
              const NBEdge* const from2, const NBEdge* const to2) const;


    /** @brief Informs edges about being controlled by a tls
     */
    virtual void setTLControllingInformation() const = 0;


    /** @brief Builds the list of participating nodes/edges/links
     */
    virtual void setParticipantsInformation();


    /** @brief Adds the given ids into the list of inner edges controlled by the tls
     * @param[in] edges The list of edge ids which shall be controlled despite lying with the jointly controlled node cluster
     */
    void addControlledInnerEdges(const std::vector<std::string>& edges);

    /** @brief Retrieve the ids of edges explicitly controlled by the tls
     */
    std::vector<std::string> getControlledInnerEdges() const;

    /** @brief Replaces occurences of the removed edge in incoming/outgoing edges of all definitions
     * @param[in] removed The removed edge
     * @param[in] incoming The edges to use instead if an incoming edge was removed
     * @param[in] outgoing The edges to use instead if an outgoing edge was removed
     */
    virtual void remapRemoved(NBEdge* removed,
                              const EdgeVector& incoming, const EdgeVector& outgoing) = 0;


    /** @brief Replaces a removed edge/lane
     * @param[in] removed The edge to replace
     * @param[in] removedLane The lane of this edge to replace
     * @param[in] by The edge to insert instead
     * @param[in] byLane This edge's lane to insert instead
     * @param[in] incoming Whether the removed edge is incoming or outgoing
     */
    virtual void replaceRemoved(NBEdge* removed, int removedLane,
                                NBEdge* by, int byLane, bool incoming) = 0;

    /// @brief patches (loaded) signal plans by modifying lane indices
    virtual void shiftTLConnectionLaneIndex(NBEdge* edge, int offset, int threshold = -1) {
        UNUSED_PARAMETER(edge);
        UNUSED_PARAMETER(offset);
        UNUSED_PARAMETER(threshold);
    }

    /** @brief Returns the list of incoming edges (must be build first)
     * @return The edges which are incoming into the tls
     */
    const EdgeVector& getIncomingEdges() const;


    /// @brief returns the controlled links (depends on previous call to collectLinks)
    const NBConnectionVector& getControlledLinks() const {
        return myControlledLinks;
    }


    /// @brief returns the controlled links (non const version)
    NBConnectionVector& getControlledLinks() {
        return myControlledLinks;
    }


    /** @brief Returns the ProgramID
     * @return The ID of the program (subID)
     */
    const std::string& getProgramID() const {
        return mySubID;
    };


    /** @brief Sets the programID
     * @param[in] programID The new ID of the program (subID)
     */
    virtual void setProgramID(const std::string& programID) {
        mySubID = programID;
    }


    /** @brief Returns the offset
     * @return Offset
     */
    SUMOTime getOffset() {
        return myOffset;
    }


    /// @brief get the algorithm type (static etc..)
    TrafficLightType getType() const {
        return myType;
    }

    /// @brief set the algorithm type (static etc..)
    virtual void setType(TrafficLightType type) {
        myType = type;
    }

    /* @brief computes whether the given stream may have green minor while the
     * other stream has green major in the same phase
     */
    bool needsCont(const NBEdge* fromE, const NBEdge* toE, const NBEdge* otherFromE, const NBEdge* otherToE) const;

    /// @brief whether the given index must yield to the foeIndex while turning right on a red light
    virtual bool rightOnRedConflict(int index, int foeIndex) const;

    /* initialize myNeedsContRelation and set myNeedsContRelationReady to true
     * This information is a byproduct of NBOwnTLDef::myCompute. All other
     * subclasses instantiate a private instance of NBOwnTLDef to answer this query */
    virtual void initNeedsContRelation() const;

    ///@brief Returns the maximum index controlled by this traffic light and assigned to a connection
    virtual int getMaxIndex() = 0;

    ///@brief Returns the maximum index controlled by this traffic light
    virtual int getMaxValidIndex() {
        return getMaxIndex();
    }

    /** @brief Computes the time vehicles may need to brake
     *
     * This time depends on the maximum speed allowed on incoming junctions.
     * It is computed as max_speed_allowed / minimum_vehicle_decleration
     */
    int computeBrakingTime(double minDecel) const;

    /// @brief whether this definition uses signal group (multiple connections with the same link index)
    virtual bool usingSignalGroups() const {
        return false;
    };

    /// @brief get ID and programID together (for convenient debugging)
    std::string getDescription() const;


protected:
    /// @brief id for temporary definitions
    static const std::string DummyID;

    /** @brief Computes the traffic light logic finally in dependence to the type
     * @param[in] brakingTime Duration a vehicle needs for braking in front of the tls
     * @return The computed logic
     */
    virtual NBTrafficLightLogic* myCompute(int brakingTime) = 0;


    /** @brief Collects the links participating in this traffic light
     * @exception ProcessError If a link could not be found
     */
    virtual void collectLinks() = 0;


    /** @brief Build the list of participating edges
     */
    virtual void collectEdges();


    // @return whether this traffic light is invalid and should be computed
    virtual bool amInvalid() const;

    /// @brief helper method for use in NBOwnTLDef and NBLoadedSUMOTLDef
    void collectAllLinks(NBConnectionVector& into);

protected:
    /// @brief The container with participating nodes
    std::vector<NBNode*> myControlledNodes;

    /// @brief The list of incoming edges
    EdgeVector myIncomingEdges;

    /// @brief The list of edges within the area controlled by the tls
    EdgeVector myEdgesWithin;

    /// @brief The list of controlled links
    NBConnectionVector myControlledLinks;

    /// @brief Set of inner edges that shall be controlled, though
    std::set<std::string> myControlledInnerEdges;

    /// @brief The tls program's subid
    std::string mySubID;

    /// @brief The offset in the program
    SUMOTime myOffset;

    /// @brief The algorithm type for the traffic light
    TrafficLightType myType;

    /// @brief data structure for caching needsCont information
    struct StreamPair {
        StreamPair(const NBEdge* _from1, const NBEdge* _to1, const NBEdge* _from2, const NBEdge* _to2):
            from1(_from1),
            to1(_to1),
            from2(_from2),
            to2(_to2) {}

        bool operator==(const StreamPair& o) const {
            return (from1 == o.from1 && to1 == o.to1
                    && from2 == o.from2 && to2 == o.to2);
        }

        bool operator<(const StreamPair& o) const {
            if (from1 != o.from1) {
                return from1 < o.from1;
            }
            if (to1 != o.to1) {
                return to1 < o.to1;
            }
            if (from2 != o.from2) {
                return from2 < o.from2;
            }
            return to2 < o.to2;
        }

        const NBEdge* from1;
        const NBEdge* to1;
        const NBEdge* from2;
        const NBEdge* to2;
    };
    typedef std::set<StreamPair> NeedsContRelation;
    mutable NeedsContRelation myNeedsContRelation;
    mutable bool myNeedsContRelationReady;

    typedef std::set<std::pair<int, int> > RightOnRedConflicts;
    mutable RightOnRedConflicts myRightOnRedConflicts;
    mutable bool myRightOnRedConflictsReady;

private:
    static std::set<NBEdge*> collectReachable(EdgeVector outer, const EdgeVector& within, bool checkControlled);


};
