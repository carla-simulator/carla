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
/// @file    NBPTLine.h
/// @author  Gregor Laemmel
/// @author  Nikita Cherednychek
/// @date    Tue, 20 Mar 2017
///
// The representation of one direction of a single pt line
/****************************************************************************/
#pragma once


#include <string>
#include <vector>
#include <map>
#include "NBEdge.h" // Cherednychek

// ===========================================================================
// class declarations
// ===========================================================================
class OutputDevice;
class NBPTStop;
class NBEdgeCont;

class NBPTLine {

public:
    explicit NBPTLine(const std::string& id, const std::string& name,
                      const std::string& type, const std::string& ref, int interval, const std::string& nightService,
                      SUMOVehicleClass vClass);

    void addPTStop(NBPTStop* pStop);

    const std::string& getLineID() const {
        return myPTLineId;
    }

    const std::string& getName() const {
        return myName;
    }

    const std::string& getType() const {
        return myType;
    }

    std::vector<NBPTStop*> getStops();
    void write(OutputDevice& device, NBEdgeCont& ec);
    void addWayNode(long long int way, long long int node);

    void setMyNumOfStops(int numStops);

    /// @brief get line reference (not unique)
    const std::string& getRef() const {
        return myRef;
    }

    void replaceStops(std::vector<NBPTStop*> stops) {
        myPTStops = stops;
    }
    /// @brief get stop edges
    std::vector<NBEdge*> getStopEdges(const NBEdgeCont& ec) const;

    /// @brief return first valid edge of myRoute (if it doest not lie after the first stop)
    NBEdge* getRouteStart(const NBEdgeCont& ec) const;

    /// @brief return last valid edge of myRoute (if it doest not lie before the last stop)
    NBEdge* getRouteEnd(const NBEdgeCont& ec) const;

    /// @brief replace the given stop
    void replaceStop(NBPTStop* oldStop, NBPTStop* newStop);

    /// @brief replace the edge with the given edge list
    void replaceEdge(const std::string& edgeID, const EdgeVector& replacement);

    void setName(const std::string& name) {
        myName = name;
    }

private:
    std::string myName;
    std::string myType;
    std::vector<NBPTStop*> myPTStops;

private:
    std::map<std::string, std::vector<long long int> > myWaysNodes;
    std::vector<std::string> myWays;
public:
    const std::vector<std::string>& getMyWays() const;
    std::vector<long long int>* getWaysNodes(std::string wayId);
private:

    std::string myCurrentWay;
    std::string myPTLineId;
    std::string myRef;

    // @brief the service interval in minutes
    int myInterval;

    std::string myNightService;
    SUMOVehicleClass myVClass;

public:
    void setEdges(const std::vector<NBEdge*>& edges);
private:
    // route of ptline
    std::vector<NBEdge*> myRoute;
public:
    const std::vector<NBEdge*>& getRoute() const;
private:

    int myNumOfStops;
};


