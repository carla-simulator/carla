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
/// @file    NIVissimNodeDef_Poly.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// -------------------
/****************************************************************************/
#pragma once
#include <config.h>


#include <string>
#include <map>
#include <utils/geom/PositionVector.h>
#include "NIVissimNodeDef_Edges.h"

class NIVissimNodeDef_Poly :
    public NIVissimNodeDef_Edges {
public:
    NIVissimNodeDef_Poly(int id, const std::string& name,
                         const PositionVector& poly);
    virtual ~NIVissimNodeDef_Poly();
    static bool dictionary(int id, const std::string& name,
                           const PositionVector& poly);
//    virtual void computeBounding();
//    double getEdgePosition(int edgeid) const;
//    void searchAndSetConnections(double offset);
private:
    PositionVector myPoly;

};
