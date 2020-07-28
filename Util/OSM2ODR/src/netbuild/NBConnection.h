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
/// @file    NBConnection.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @date    Sept 2002
///
// The class holds a description of a connection between two edges
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include "NBEdge.h"


// ===========================================================================
// class declarations
// ===========================================================================
class NBNode;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NBConnection
 */
class NBConnection {
public:
    /// @brief Constructor
    NBConnection(NBEdge* from, NBEdge* to);

    /// @brief Constructor
    NBConnection(NBEdge* from, int fromLane, NBEdge* to, int toLane, int tlIndex = InvalidTlIndex, int tlIndex2 = InvalidTlIndex);

    /// @brief Constructor
    NBConnection(const std::string& fromID, NBEdge* from,
                 const std::string& toID, NBEdge* to);

    /// @brief Constructor
    NBConnection(const NBConnection& c);

    /// @brief Destructor
    virtual ~NBConnection();

    /// @brief returns the from-edge (start of the connection)
    NBEdge* getFrom() const;

    /// @brief returns the to-edge (end of the connection)
    NBEdge* getTo() const;

    /// @brief replaces the from-edge by the one given
    bool replaceFrom(NBEdge* which, NBEdge* by);

    /// @brief replaces the from-edge by the one given
    bool replaceFrom(NBEdge* which, int whichLane, NBEdge* by, int byLane);

    /// @brief replaces the to-edge by the one given
    bool replaceTo(NBEdge* which, NBEdge* by);

    /// @brief replaces the to-edge by the one given
    bool replaceTo(NBEdge* which, int whichLane, NBEdge* by, int byLane);

    /** @brief  patches lane indices refering to the given edge and above the
     * threshold by the given offset */
    void shiftLaneIndex(NBEdge* edge, int offset, int threshold = -1);

    /// @brief checks whether the edges are still valid
    bool check(const NBEdgeCont& ec);

    /// @brief returns the from-lane
    int getFromLane() const;

    /// @brief returns the to-lane
    int getToLane() const;

    /// @brief returns the index within the controlling tls or InvalidTLIndex if this link is unontrolled
    int getTLIndex() const {
        return myTlIndex;
    }
    int getTLIndex2() const {
        return myTlIndex2;
    }

    // @brief reset the tlIndex
    void setTLIndex(int tlIndex) {
        myTlIndex = tlIndex;
    }
    void setTLIndex2(int tlIndex) {
        myTlIndex2 = tlIndex;
    }

    /// @brief returns the id of the connection (!!! not really pretty)
    std::string getID() const;

    /// @brief Compares both connections in order to allow sorting
    friend bool operator<(const NBConnection& c1, const NBConnection& c2);

    /// @brief Comparison operator
    bool operator==(const NBConnection& c) const;

    /// @brief Comparison operator
    bool operator!=(const NBConnection& c) const {
        return !(*this == c);
    }

    /// @brief Output operator
    friend std::ostream& operator<<(std::ostream& os, const NBConnection& c);

    const static int InvalidTlIndex;
    const static NBConnection InvalidConnection;

private:
    /// @brief Checks whether the from-edge is still valid
    NBEdge* checkFrom(const NBEdgeCont& ec);

    /// @brief Checks whether the to-edge is still valid
    NBEdge* checkTo(const NBEdgeCont& ec);

private:
    /// @brief The from- and the to-edges
    NBEdge* myFrom, *myTo;

    /// @brief The names of both edges, needed for verification of validity
    std::string myFromID, myToID;

    /// @brief The lanes; may be -1 if no certain lane was specified
    int myFromLane, myToLane;

    // @brief the index within the controlling tls if this connection is tls-controlled
    int myTlIndex;
    /// @brief The index of the internal junction within the controlling traffic light (optional)
    int myTlIndex2;
};
