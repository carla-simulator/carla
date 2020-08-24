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
/// @file    NIVissimAbstractEdge.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// -------------------
/****************************************************************************/
#pragma once
#include <config.h>

#include <map>
#include <utils/geom/PositionVector.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 *
 */
class NIVissimAbstractEdge {
public:
    NIVissimAbstractEdge(int id, const PositionVector& geom);
    virtual ~NIVissimAbstractEdge();
    Position getGeomPosition(double pos) const;
    void splitAssigning();
    bool crossesEdge(NIVissimAbstractEdge* c) const;
    Position crossesEdgeAtPoint(NIVissimAbstractEdge* c) const;
    bool overlapsWith(const AbstractPoly& p, double offset = 0.0) const;
    virtual void setNodeCluster(int nodeid) = 0;
    bool hasNodeCluster() const;

    virtual void buildGeom() = 0;
    int getID() const;
    const PositionVector& getGeometry() const;

    void addDisturbance(int disturbance);

    const std::vector<int>& getDisturbances() const;

public:
    static bool dictionary(int id, NIVissimAbstractEdge* e);
    static NIVissimAbstractEdge* dictionary(int id);
    static void splitAndAssignToNodes();
    static std::vector<int> getWithin(const AbstractPoly& p, double offset = 0.0);
    static void clearDict();


protected:
    int myID;
    PositionVector myGeom;
    std::vector<int> myDisturbances;
    int myNode;

private:
    typedef std::map<int, NIVissimAbstractEdge*> DictType;
    static DictType myDict;
};
