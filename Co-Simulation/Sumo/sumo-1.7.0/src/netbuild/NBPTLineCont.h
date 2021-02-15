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
/// @file    NBPTLineCont.h
/// @author  Gregor Laemmel
/// @date    Tue, 20 Mar 2017
///
// Container for NBPTLine during netbuild
/****************************************************************************/

#pragma once


#include <vector>
#include "utils/router/SUMOAbstractRouter.h"
#include "NBPTLine.h"
#include "NBEdgeCont.h"

class NBPTLineCont {

public:
    /// @brief constructor
    NBPTLineCont();

    /// @brief destructor
    ~NBPTLineCont();

    /// @brief insert new line
    void insert(NBPTLine* ptLine);

    const std::map<std::string, NBPTLine*>& getLines() const {
        return myPTLines;
    }

    void process(NBEdgeCont& ec, NBPTStopCont& sc);

    /// @brief add edges that must be kept
    void addEdges2Keep(const OptionsCont& oc, std::set<std::string>& into);

    /// @brief replace the edge with the given edge list in all lines
    void replaceEdge(const std::string& edgeID, const EdgeVector& replacement);

    /// @brief select the correct stop on superposed rail edges
    void fixBidiStops(const NBEdgeCont& ec);

    std::set<std::string>& getServedPTStops();
private:

    static const int FWD;
    static const int BWD;

    /// @brief The map of names to pt lines
    std::map<std::string, NBPTLine*> myPTLines;

    /// @brief find directional edge for all stops of the line
    void reviseStops(NBPTLine* line, const NBEdgeCont& ec, NBPTStopCont& sc);

    /* @brief find way element corresponding to the stop
     * @note: if the edge id is updated, the stop extent is recomputed */
    NBPTStop* findWay(NBPTLine* line, NBPTStop* stop, const NBEdgeCont& ec, NBPTStopCont& sc) const;

    void constructRoute(NBPTLine* myPTLine, NBEdgeCont& cont);

    std::set<std::string> myServedPTStops;

    static double getCost(const NBEdgeCont& ec, SUMOAbstractRouter<NBRouterEdge, NBVehicle>& router,
                          const NBPTStop* from, const NBPTStop* to, const NBVehicle* veh);

    static std::string getWayID(const std::string& edgeID);
};


