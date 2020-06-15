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
/// @file    NIVissimDistrictConnection.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    End of 2002
///
// An edge imported from Vissim together for a container for
/****************************************************************************/
#pragma once
#include <config.h>

#include <map>
#include <string>
#include <utils/geom/Position.h>


class NBDistrictCont;
class NBEdgeCont;


// ===========================================================================
// class definitions
// ===========================================================================
class NIVissimDistrictConnection {
public:
    /// Contructor
    NIVissimDistrictConnection(int id, const std::string& name,
                               const std::vector<int>& districts, const std::vector<double>& percentages,
                               int edgeid, double position,
                               const std::vector<std::pair<int, int> >& assignedVehicles);

    // Destructor
    ~NIVissimDistrictConnection();

    /** @brief Returns the position
        The position yields from the edge geometry and the place the connection is plaed at */
    Position geomPosition() const;

    /// Returns the id of the connection
    int getID() const {
        return myID;
    }

    /// Returns the position of the connection at the edge
    double getPosition() const {
        return myPosition;
    }

    double getMeanSpeed() const;

public:
    /// Inserts the connection into the dictionary after building it
    static bool dictionary(int id, const std::string& name,
                           const std::vector<int>& districts, const std::vector<double>& percentages,
                           int edgeid, double position,
                           const std::vector<std::pair<int, int> >& assignedVehicles);

    /// Inserts the build connection to the dictionary
    static bool dictionary(int id, NIVissimDistrictConnection* o);

    /// Returns the named dictionary
    static NIVissimDistrictConnection* dictionary(int id);

    /// Builds the nodes that belong to a district
    static void dict_BuildDistrictNodes(NBDistrictCont& dc,
                                        NBNodeCont& nc);

    /// Builds the districts
    static void dict_BuildDistricts(NBDistrictCont& dc,
                                    NBEdgeCont& ec, NBNodeCont& nc);

    /** @brief Returns the connection to a district placed at the given node
        Yep, there onyl should be one, there is no need to build a single edge as connection between two parking places */
    static NIVissimDistrictConnection* dict_findForEdge(int edgeid);

    /// Clears the dictionary
    static void clearDict();

    static void dict_BuildDistrictConnections();

    static void dict_CheckEdgeEnds();


private:
    void checkEdgeEnd();
    double getRealSpeed(int distNo) const;

private:
    /// The id of the connections
    int myID;

    /// The name of the connections
    std::string myName;

    /// The connected districts
    std::vector<int> myDistricts;

    /// Definition of a map of how many vehicles should leave to a certain district
    typedef std::map<int, double> DistrictPercentages;

    /// A map how many vehicles (key, amount) should leave to a district (key)
    DistrictPercentages myPercentages;

    /// The id of the connected edge
    int myEdgeID;

    /// The position on the edge
    double myPosition;

    /// The vehicles using this connection
    std::vector<std::pair<int, int> > myAssignedVehicles;

private:
    /// Definition of a dictionary of district connections
    typedef std::map<int, NIVissimDistrictConnection*> DictType;

    /// District connection dictionary
    static DictType myDict;

    /// Map from ditricts to connections
    static std::map<int, std::vector<int> > myDistrictsConnections;

};
