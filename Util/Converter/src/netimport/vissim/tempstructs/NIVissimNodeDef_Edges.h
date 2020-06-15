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
/// @file    NIVissimNodeDef_Edges.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
///
// -------------------
/****************************************************************************/
#pragma once
#include <config.h>


#include <string>
#include <map>
#include "NIVissimNodeParticipatingEdgeVector.h"
#include "NIVissimExtendedEdgePoint.h"
#include "NIVissimNodeDef.h"

class NIVissimNodeDef_Edges :
    public NIVissimNodeDef {
public:
    NIVissimNodeDef_Edges(int id, const std::string& name,
                          const NIVissimNodeParticipatingEdgeVector& edges);
    virtual ~NIVissimNodeDef_Edges();
    static bool dictionary(int id, const std::string& name,
                           const NIVissimNodeParticipatingEdgeVector& edges);
//    virtual void computeBounding();
//    virtual void searchAndSetConnections();
    virtual double getEdgePosition(int edgeid) const;

    /**
     *
     */
    class id_matches {
    public:
        explicit id_matches(int id) : myEdgeID(id) { }
        bool operator()(NIVissimNodeParticipatingEdge* e) {
            return e->getID() == myEdgeID;
        }
    private:
        int myEdgeID;
    };

    class lying_within_match {
    public:
        explicit lying_within_match(NIVissimNodeParticipatingEdge* e) : myEdge(e) { }
        bool operator()(NIVissimExtendedEdgePoint* e) {
            return e->getEdgeID() == myEdge->getID() &&
                   myEdge->positionLiesWithin(e->getPosition());
        }
    private:
        NIVissimNodeParticipatingEdge* myEdge;
    };

protected:
    NIVissimNodeParticipatingEdgeVector myEdges;
};
